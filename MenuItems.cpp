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

#include "StringUtils.h"
#include "KeyCombiAlt.h"
#include "MenuItems.h"

using namespace std::tr2::sys;
using namespace std;

namespace
{
	const wchar_t PART_DELIM(L'|');
	const wstring CONFIG_DIR(L"Config");
	const wchar_t *STATS_FILENAME = L"_stats.conf";

	struct EnumWinDesc
	{
		vector<HWND> hwnds;
		DWORD procId;
	};
	
	struct TopWinInfo
	{
		HWND hwnd;
		DWORD procId;
	};

	struct ItemList
	{
		wstring path;
		map<wstring, Item> items;
	};

	struct MenuItem
	{
		UINT command;
		wstring text;
		bool accelerator;
	};

	void getAllMenuItems(vector<MenuItem>& menuItems, bool acc, std::wstring item, HMENU menu);
	BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam);	
}

struct MenuItems::impl
{
	map<unsigned char, unsigned char> modvk;
	map<wstring, ItemList> allitems;
};

MenuItems::MenuItems()
	: pimpl(new impl())
{
	/* Fill string -> modifier/key maps. */
	pimpl->modvk[MOD_CONTROL] = VK_CONTROL;
	pimpl->modvk[MOD_ALT] = VK_MENU;
	pimpl->modvk[MOD_SHIFT] = VK_SHIFT;
	pimpl->modvk[MOD_WIN] = VK_LWIN;

	/* Load configuration files. */
	wpath p(initial_path<wpath>());
	p /= CONFIG_DIR;
	for(wdirectory_iterator di(p); di != wdirectory_iterator(); ++di)
	{
		/* Ignore directories. */
		if( is_directory(di->status()) ) continue;

		/* Get the full path. */
		wpath configPath(CONFIG_DIR);
		wstring fullpath = configPath / di->path();

		/* Ignore everthing except text files. */
		if( !endsWith(fullpath, L".txt") ) continue;

		/* Get application name. */
		wstring appname = di->path().filename();
		trim(appname);
		transform(begin(appname), end(appname), begin(appname), ::tolower);
		ItemList &il = pimpl->allitems[appname.substr(0, appname.length() - 4)];
		il.path = fullpath;
		auto& items = il.items;

		/* Read the file line by line. */
		wifstream fs(fullpath);
		wstring line;
		while(getline(fs, line))
		{
			/* Ignore empty lines and lines starting with '#'. */
			wstring trimLine = trim(line);
			if( trimLine.empty() || (trimLine[0] == '#') )
			{
				continue;
			}

			/* Parse each line of the file into segments. */
			wstringstream ss(line);
			wstring segment;
			vector<wstring> segments;
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
	wpath pstat(initial_path<wpath>());
	pstat /= CONFIG_DIR;
	pstat /= STATS_FILENAME;
	wifstream fst(pstat.file_string());
	wstring sline;
	while(getline(fst, sline))
	{
		/* Skip comment and empty lines. */
		wstring trimLine = trim(sline);
		if( sline.empty() || (sline[0] == '#') )
		{
			continue;
		}

		/* Split line on delimiter to separate app/shortcut/count. */
		wstring spart;
		wstringstream sps(sline);
		vector<wstring> sparts;
		while(getline(sps, spart, PART_DELIM))
		{
			sparts.push_back(trim(spart));
		}
		if( sparts.size() > 2 )
		{
			/* We have an app, a shortcut, and a count. */
			transform(begin(sparts[0]), end(sparts[0]), begin(sparts[0]), ::tolower);
			transform(begin(sparts[1]), end(sparts[1]), begin(sparts[1]), ::tolower);
			auto& it = pimpl->allitems.find(sparts[0]);
			if( it == end(pimpl->allitems) )
			{
				/* App not found. */
				continue;
			}
			auto& iit = it->second.items.find(sparts[1]);
			if( iit == end(it->second.items) )
			{
				wstringstream cnt(sparts[2]);
				cnt >> iit->second.count;
			}
		}
	}
}

MenuItems::~MenuItems()
{
	Save();
}

void MenuItems::UpdateMenuItems(std::wstring application, HWND currWin)
{
	/* Find top-level window for the current process. */
	DWORD procId;
	::GetWindowThreadProcessId(currWin, &procId);
	TopWinInfo topWinInfo = { NULL, procId };
	::EnumWindows(enumWindowsProc, (LPARAM)&topWinInfo);
	ItemList& items = pimpl->allitems[application];
	vector<MenuItem> menuItems;
	getAllMenuItems(menuItems, true, wstring(), ::GetMenu(topWinInfo.hwnd));
	getAllMenuItems(menuItems, true, wstring(), ::GetSystemMenu(topWinInfo.hwnd, FALSE));

	/* Add menu items to item list. */
	for(auto& m : menuItems)
	{
		vector<wchar_t> acc;
		wstring desc(m.text);
		/* Split text into name and shortcut. */
		auto ampPos = wstring::npos;
		while( (ampPos = desc.find(L"&")) != wstring::npos )
		{
			if( m.accelerator )
			{
				acc.push_back(desc[ampPos+1]);
			}
			desc.replace(ampPos, 1, L"");
		}
		wstring shDesc(desc);
		wstring shstring;
		auto mit = desc.find('\t');
		if( mit != wstring::npos )
		{
			shDesc = desc.substr(0, mit);
			shstring = desc.substr(mit + 1, wstring::npos);
			replace(begin(shstring), end(shstring), L'+', L' ');
		}

		KeyCombiAlt shShort(shstring);
		if( shShort.empty() && !acc.empty() )
		{
			/* Use accelerators for shortcut. */
			wstring alt(L"Alt ");
			KeyCombiMulti keyMulti;
			for(auto &i : acc)
			{
				keyMulti.push_back(KeyCombi(alt + wstring(1, i)));
				alt.clear();
			}
			shShort.push_back(keyMulti);
		}

		/* Use text for name and description. */
		Item& item = items.items[shDesc];
		item.desc = shDesc;
		item.name = shDesc;
		item.keys = shShort;
		item.command = m.command;
	}
}

bool MenuItems::IsConfigAvailable(wstring application)
{
	auto &it = pimpl->allitems.find(application);
	if( it == end(pimpl->allitems) )
	{
		return false;
	}
	return !it->second.items.empty();
}

vector<Item> MenuItems::GetItems(wstring application, vector<wstring> words)
{
	/* Show no items when no text. */
	if(words.empty())
	{
		return vector<Item>();
	}

	auto& ait = pimpl->allitems.find(application);
	if( ait == pimpl->allitems.end() )
	{
		/* Application not found. */
		return vector<Item>();
	}
	auto& items = ait->second.items;
	set<Item> matches;

	/* Try to match all words from name, description and shortcut. */
	for(auto &it : items)
	{
		wstring lname = it.second.name;
		wstring desc = it.second.desc;
		wstring shortcut = it.second.keys.str(L" + ");
		transform(begin(lname), end(lname), begin(lname), ::tolower);
		transform(begin(desc), end(desc), begin(desc), ::tolower);
		transform(begin(shortcut), end(shortcut), begin(shortcut), ::tolower);

		bool foundAll = true;
		for(auto& w : words)
		{
			bool found = false;
			if(lname.find(w) != wstring::npos)
			{
				found = true;
			}
			if(desc.find(w) != wstring::npos)
			{
				found = true;
			}
			if(shortcut.find(w) != wstring::npos)
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

	vector<Item> matchVec;
	copy(matches.rbegin(), matches.rend(), back_inserter(matchVec));
	return matchVec;
}

void MenuItems::Launch(HWND hwnd, wstring application, Item item)
{
	/* Show window. */
	SetForegroundWindow(hwnd);

	Sleep(15);

	/* Send keys/command for shortcut/menu item. */
	if( item.command != 0 )
	{
		::SendMessage(hwnd, WM_COMMAND, item.command, 0);
	}
	else if( item.keys.size() > 0 )
	{
		for(auto& k: item.keys.front())
		{
			vector<INPUT> inputs;
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
			UINT numSent = 0;
			while( numSent != inputs.size() )
			{
				numSent += ::SendInput(inputs.size() - numSent, &inputs[numSent], sizeof(INPUT));
			}

			/* Sleep for a short time to gaurantee both keydown and keyup messages
			 * are received separately.
			 */
			Sleep(50);
		
			for(auto& i: inputs)
			{
				i.ki.dwFlags = KEYEVENTF_KEYUP;
			}
			/* Send key ups. */
			numSent = 0;
			while( numSent != inputs.size() )
			{
				numSent += ::SendInput(inputs.size() - numSent, &inputs[numSent], sizeof(INPUT));
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

	/* Save items. */
	Save();
}

void MenuItems::Save()
{
	wpath pstat(initial_path<wpath>());
	pstat /= CONFIG_DIR;
	pstat /= STATS_FILENAME;
	wofstream fst(pstat.file_string(), ios::trunc | ios::out);
	for(auto& app : pimpl->allitems)
	{
		for(auto& s : app.second.items)
		{
			fst << app.first << " | ";
			fst << s.second.name << " | ";
			fst << s.second.count << endl;
		}
	}
}

namespace
{
	void getAllMenuItems(vector<MenuItem>& menuItems, bool acc, std::wstring name, HMENU menu)
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
				vector<TCHAR> menuItemString(menuItemInfo.cch + 1);
				++menuItemInfo.cch;
				menuItemInfo.dwTypeData = &menuItemString[0];
				if( GetMenuItemInfo(menu, c, TRUE, &menuItemInfo) != 0 )
				{
					wstring menuname(menuItemInfo.dwTypeData);
					wstring subname(name);
					if( !subname.empty() )
					{
						subname += L" → ";
					}
					if( menuname.find(L'&') == wstring::npos )
					{
						acc = false;
					}
					subname += menuname;
					HMENU subMenu = NULL;
					if((subMenu = ::GetSubMenu(menu, c)) != NULL)
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

