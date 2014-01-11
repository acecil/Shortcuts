//	
//	(c) 2012 by Andrew Gascoyne-Cecil.
//	
//	This file is part of Shortcuts.
//
//	Shortcuts is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//	
//	Shortcuts is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//	
//	You should have received a copy of the GNU General Public License
//	along with Shortcuts.  If not, see <http://www.gnu.org/licenses/>.
//

#include "stdafx.h"

#include <tlhelp32.h>
#include <VersionHelpers.h>

#include <map>
#include <list>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <functional> 
#include <locale>
#include <set>
#include <iterator>
#include <tuple>
#include <vector>

#include "Config.h"
#include "StringUtils.h"
#include "KeyCombiAlt.h"
#include "MenuItems.h"

using std::tr2::sys::wpath;
using std::tr2::sys::initial_path;
using std::tr2::sys::wdirectory_iterator;
using std::tr2::sys::is_directory;

namespace
{
	const wchar_t PART_DELIM(L'|');
	const std::wstring CONFIG_DIR(L"Config");
	const wchar_t *STATS_FILENAME = L"_stats.conf";
	const std::wstring WINDOWS(L"windows");

	struct EnumWinDesc
	{
		std::vector<HWND> hwnds;
		DWORD procId;
	};
	
	struct TopWinInfo
	{
		HWND hwnd;
		DWORD procId;
	};

	struct ItemList
	{
		std::wstring path;
		std::map<std::wstring, Item> items;
	};

	struct MenuItem
	{
		UINT command;
		std::wstring text;
		bool accelerator;
	};

	void getAllMenuItems(std::vector<MenuItem>& menuItems, bool acc, std::wstring item, HMENU menu);
	BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam);
}

struct MenuItems::impl
{
	std::map<unsigned char, unsigned char> modvk;
	std::map<std::wstring, ItemList> allitems;
};

MenuItems::MenuItems()
	: pimpl(new impl())
{
	/* Fill string -> modifier/key std::maps. */
	pimpl->modvk[MOD_CONTROL] = VK_CONTROL;
	pimpl->modvk[MOD_ALT] = VK_MENU;
	pimpl->modvk[MOD_SHIFT] = VK_SHIFT;
	pimpl->modvk[MOD_WIN] = VK_LWIN;

	/* Load configuration files. */
	wpath p(Config::GetAppDataFolder());
	p /= CONFIG_DIR;
	for (wdirectory_iterator di(p); di != wdirectory_iterator(); ++di)
	{
		/* Ignore directories. */
		if (is_directory(di->status())) continue;

		/* Get the full path. */
		std::wstring fullpath = di->path();

		/* Ignore everthing except text files. */
		if( !endsWith(fullpath, L".txt") ) continue;

		/* Get application name. */
		std::wstring appname = di->path().filename();
		trim(appname);
		std::transform(std::begin(appname), std::end(appname), std::begin(appname), ::tolower);
		ItemList &il = pimpl->allitems[appname.substr(0, appname.length() - 4)];
		il.path = fullpath;
		auto& items = il.items;

		/* Read the file line by line. */
		std::wifstream fs(fullpath);
		std::wstring line;
		while(std::getline(fs, line))
		{
			/* Ignore empty lines and lines starting with '#'. */
			std::wstring trimLine = trim(line);
			if( trimLine.empty() || (trimLine[0] == '#') )
			{
				continue;
			}

			/* Parse each line of the file into segments. */
			std::wstringstream ss(line);
			std::wstring segment;
			std::vector<std::wstring> segments;
			while(getline(ss, segment, PART_DELIM))
			{
				segments.push_back(segment);
			}

			/* Parse item name. */
			if(segments.size() > 1)
			{
				/* We have a shortcut. */
				Item item;
				item.name = trim(segments[0]);
				item.count = 0;
				item.command = 0;

				/* Parse description. */
				item.desc = trim(segments[1]);

				/* We have shortcut keys. */
				if(segments.size() > 2)
				{
					item.keys = KeyCombiAlt(segments[2]);
				}

				/* Save the item in the item list. */
				items[item.name] = item;
			}
		}
	}

	/* Load stats file. */
	wpath pstat(Config::GetAppDataFolder());
	pstat /= CONFIG_DIR;
	pstat /= STATS_FILENAME;
	std::wifstream fst(pstat.file_string());
	std::wstring sline;
	while(std::getline(fst, sline))
	{
		/* Skip comment and empty lines. */
		std::wstring trimLine = trim(sline);
		if( sline.empty() || (sline[0] == '#') )
		{
			continue;
		}

		/* Split line on delimiter to separate app/shortcut/count. */
		std::wstring spart;
		std::wstringstream sps(sline);
		std::vector<std::wstring> sparts;
		while(getline(sps, spart, PART_DELIM))
		{
			sparts.push_back(trim(spart));
		}
		if( sparts.size() > 2 )
		{
			/* We have an app, a shortcut, and a count. */
			std::transform(std::begin(sparts[0]), std::end(sparts[0]), std::begin(sparts[0]), ::tolower);
			std::transform(std::begin(sparts[1]), std::end(sparts[1]), std::begin(sparts[1]), ::tolower);
			auto& it = pimpl->allitems.find(sparts[0]);
			if( it == end(pimpl->allitems) )
			{
				/* App not found. */
				continue;
			}
			auto& iit = it->second.items.find(sparts[1]);
			if( iit != end(it->second.items) )
			{
				std::wstringstream cnt(sparts[2]);
				cnt >> iit->second.count;
			}
		}
	}
}

MenuItems::~MenuItems()
{
	Save();
}

void MenuItems::UpdateMenuItems(std::wstring application, HWND currWin, WORD currVersion)
{
	/* Find top-level window for the current process. */
	DWORD procId;
	::GetWindowThreadProcessId(currWin, &procId);
	TopWinInfo topWinInfo = { nullptr, procId };
	::EnumWindows(enumWindowsProc, (LPARAM)&topWinInfo);

	std::wstring verApp = application + std::to_wstring(currVersion);
	auto it = pimpl->allitems.find(verApp);
	if (it == pimpl->allitems.end())
	{
		it = pimpl->allitems.find(application);
	}
	if (it == pimpl->allitems.end())
	{
		pimpl->allitems[application] = ItemList();
		it = pimpl->allitems.find(application);
	}

	std::vector<MenuItem> menuItems;
	getAllMenuItems(menuItems, true, std::wstring(), ::GetMenu(topWinInfo.hwnd));
	getAllMenuItems(menuItems, true, std::wstring(), ::GetSystemMenu(topWinInfo.hwnd, FALSE));

	/* Add menu items to item list. */
	for(auto& m : menuItems)
	{
		bool sysCommand = false;
		std::vector<std::wstring> acc;
		std::wstring desc(m.text);
		/* Split text into name and shortcut. */
		if( m.accelerator && (desc.find(L"→") == std::wstring::npos) )
		{
			acc.push_back(L"spacebar");
			sysCommand = true;
		}
		auto ampPos = std::wstring::npos;
		while( (ampPos = desc.find(L"&")) != std::wstring::npos )
		{
			if( m.accelerator )
			{
				acc.push_back(std::wstring(1, desc[ampPos+1]));
			}
			desc.replace(ampPos, 1, L"");
		}
		std::wstring shDesc(desc);
		std::wstring shstring;
		auto mit = desc.find('\t');
		if( mit != std::wstring::npos )
		{
			shDesc = desc.substr(0, mit);
			shstring = desc.substr(mit + 1, std::wstring::npos);
			std::replace(std::begin(shstring), std::end(shstring), L'+', L' ');
		}

		KeyCombiAlt shShort(shstring);
		if( shShort.empty() && !acc.empty() )
		{
			/* Use accelerators for shortcut. */
			std::wstring alt(L"Alt ");
			KeyCombiMulti keyMulti;
			for(auto &i : acc)
			{
				keyMulti.push_back(KeyCombi(alt + i));
				alt.clear();
			}
			shShort.push_back(keyMulti);
		}

		/* Use text for name and description. */
		if (it != pimpl->allitems.end())
		{
			Item& item = it->second.items[shDesc];
			item.desc = shDesc;
			item.name = shDesc;
			item.keys = shShort;
			item.command = m.command;
			item.sysCommand = sysCommand;
		}
	}
}

bool MenuItems::IsConfigAvailable(std::wstring application)
{
	auto &it = pimpl->allitems.find(application);
	if( it == end(pimpl->allitems) )
	{
		return false;
	}
	return !it->second.items.empty();
}

std::vector<Item> MenuItems::GetItems(std::wstring application, WORD currVersion, std::vector<std::wstring> words)
{
	/* Show no items when no text. */
	if (words.empty())
	{
		return std::vector<Item>();
	}

	/* Get standard windows shortcuts for the current windows version. */
	std::map<std::wstring, Item> allItems = GetWindowsItems();
	
	LoadItems(allItems, application, currVersion);

	/* Try to match all words from name, description and shortcut. */
	std::set<Item> matches;
	for (auto &it : allItems)
	{
		std::wstring lname = it.second.name;
		std::wstring desc = it.second.desc;
		std::wstring shortcut = it.second.keys.str(L" + ");
		std::transform(std::begin(lname), std::end(lname), std::begin(lname), ::tolower);
		std::transform(std::begin(desc), std::end(desc), std::begin(desc), ::tolower);
		std::transform(std::begin(shortcut), std::end(shortcut), std::begin(shortcut), ::tolower);

		bool foundAll = true;
		for(auto& w : words)
		{
			bool found = false;
			if(lname.find(w) != std::wstring::npos)
			{
				found = true;
			}
			if(desc.find(w) != std::wstring::npos)
			{
				found = true;
			}
			if(shortcut.find(w) != std::wstring::npos)
			{
				found = true;
			}
			if(!found)
			{
				foundAll = false;
				break;
			}
		}
		if(foundAll)
		{
			matches.insert(it.second);
		}
	}

	std::vector<Item> matchVec;
	std::copy(matches.rbegin(), matches.rend(), std::back_inserter(matchVec));
	return matchVec;
}

void MenuItems::Launch(HWND hwnd, std::wstring application, Item item)
{
	/* Show window. */
	SetForegroundWindow(hwnd);

	Sleep(100);

	/* Send keys/command for shortcut/menu item. */
	if( item.command != 0 )
	{
		::SendMessage(hwnd, item.sysCommand ? WM_SYSCOMMAND : WM_COMMAND,
			item.command, 0);
	}
	else if( item.keys.size() > 0 )
	{
		for(auto& k: item.keys.front())
		{
			std::vector<INPUT> inputs;
			for(unsigned char c = 1; c != 0; c <<= 1)
			{
				if(c & k.mods())
				{
					INPUT input = {0};
					input.type = INPUT_KEYBOARD;
					input.ki.wVk = pimpl->modvk[c];
					inputs.push_back(input);
				}
			}
			INPUT input = {0};
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = k.key();
			inputs.push_back(input);

			/* Send key downs. */
			for (auto& i : inputs)
			{
				while (::SendInput(1, &i, sizeof(INPUT)) == 0);
				Sleep(50);
			}

			/* Send key ups. */
			for (auto& i : inputs)
			{
				i.ki.dwFlags = KEYEVENTF_KEYUP;
				while (::SendInput(1, &i, sizeof(INPUT)) == 0);
				Sleep(50);
			}
		}
	}

	/* Increase count for this item. */
	auto& items = pimpl->allitems[application];
	for(auto& i: items.items)
	{
		/* Assume item name is unique within app. */
		if(i.second.name == item.name)
		{
			++i.second.count;
		}
	}
	auto& winItems = pimpl->allitems[L"windows"];
	for (auto& i : winItems.items)
	{
		/* Assume item name is unique within app. */
		if (i.second.name == item.name)
		{
			++i.second.count;
		}
	}

	/* Save items. */
	Save();
}

void MenuItems::Save()
{
	wpath pstat(initial_path<wpath>());
	pstat /= CONFIG_DIR;
	pstat /= STATS_FILENAME;
	std::wofstream fst(pstat.file_string(), std::ios::trunc | std::ios::out);
	for(auto& app : pimpl->allitems)
	{
		for(auto& s : app.second.items)
		{
			fst << app.first << " | ";
			fst << s.second.name << " | ";
			fst << s.second.count << std::endl;
		}
	}
}

std::map<std::wstring, Item> MenuItems::GetWindowsItems()
{
	std::map<std::wstring, Item> items;

	/* Load common windows shortcuts. */
	if (IsWindowsXPOrGreater())
	{
		LoadItems(items, WINDOWS + L"xp");
	}
	if (IsWindowsVistaOrGreater())
	{
		LoadItems(items, WINDOWS + L"vista");
	}
	if (IsWindows7OrGreater())
	{
		LoadItems(items, WINDOWS + L"7");
	}
	if (IsWindows8OrGreater())
	{
		LoadItems(items, WINDOWS + L"8");
	}
	if (IsWindows8Point1OrGreater())
	{
		LoadItems(items, WINDOWS + L"8.1");
	}

	return items;
}

void MenuItems::LoadItems(std::map<std::wstring, Item> &items, std::wstring app, WORD currVersion)
{
	auto it = pimpl->allitems.find(app);
	if (it != pimpl->allitems.end())
	{
		auto &nitems = it->second.items;
		items.insert(std::begin(nitems), std::end(nitems));
	}
	it = pimpl->allitems.find(app + std::to_wstring(currVersion));
	if (it != pimpl->allitems.end())
	{
		auto &nitems = it->second.items;
		items.insert(std::begin(nitems), std::end(nitems));
	}
}

namespace
{
	void getAllMenuItems(std::vector<MenuItem>& menuItems, bool acc, std::wstring name, HMENU menu)
	{
		int menuItemCount = GetMenuItemCount(menu);
		for(int c = 0; c < menuItemCount; ++c)
		{
			MENUITEMINFO menuItemInfo = {0};
			menuItemInfo.cbSize = sizeof(MENUITEMINFO);
			menuItemInfo.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
			if( (GetMenuItemInfo(menu, c, TRUE, &menuItemInfo) != 0)
				&& (menuItemInfo.cch > 0) 
				&& !(menuItemInfo.fState & MFS_DISABLED) )
			{
				std::vector<TCHAR> menuItemString(menuItemInfo.cch + 1);
				++menuItemInfo.cch;
				menuItemInfo.dwTypeData = &menuItemString[0];
				if( GetMenuItemInfo(menu, c, TRUE, &menuItemInfo) != 0 )
				{
					std::wstring menuname(menuItemInfo.dwTypeData);
					std::wstring subname(name);
					if( !subname.empty() )
					{
						subname += L" → ";
					}
					if( menuname.find(L'&') == std::wstring::npos )
					{
						acc = false;
					}
					subname += menuname;
					HMENU subMenu = nullptr;
					if((subMenu = ::GetSubMenu(menu, c)) != nullptr)
					{
						getAllMenuItems(menuItems, acc, subname, subMenu);
					}
					else
					{
						MenuItem menuItem = { menuItemInfo.wID, subname, acc };
						menuItems.push_back(menuItem);
					}
				}
			}
		}
	}

	BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam)
	{
		TopWinInfo* topWinInfo = (TopWinInfo*)lParam;
		DWORD procId = 0;
		::GetWindowThreadProcessId(hwnd, &procId);
		if( (procId == topWinInfo->procId) &&
			::GetMenu(hwnd) )
		{
			topWinInfo->hwnd = hwnd;
			return FALSE;
		}

		return TRUE;
	}
}

