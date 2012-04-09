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
using namespace std::tr2::sys;

#include "MenuItems.h"

static wchar_t PART_DELIM = '|';
static wchar_t ALT_KEY_DELIM = ';';
static wchar_t KEY_DELIM = ',';
static wchar_t *STATS_FILENAME = L"_stats.conf";

struct EnumWinDesc
{
	vector<HWND> hwnds;
	DWORD procId;
};

struct ItemList
{
	wstring path;
	list<Item> items;
};

struct MenuItems::impl
{
	map<wstring, Modifier> modifiers;
	map<wstring, unsigned char> keys;
	map<unsigned char, unsigned char> modvk;
	map<wstring, ItemList> allitems;
};

static bool isspace(const wstring& str);
static inline wstring &ltrim(wstring &s);
static inline wstring &rtrim(wstring &s);
static inline wstring &trim(wstring &s);
static bool endsWith (wstring const &fullString, wstring const &ending);
static void replaceAll(wstring &s, const wstring &from, const wstring &to);
static unsigned char countbits(unsigned char b);

MenuItems::MenuItems()
	: pimpl(new impl())
{
	/* Fill string -> modifier/key maps. */
	pimpl->modifiers[L"ctrl"] = KEY_CTRL;
	pimpl->modifiers[L"control"] = KEY_CTRL;
	pimpl->modifiers[L"alt"] = KEY_ALT;
	pimpl->modifiers[L"shift"] = KEY_SHIFT;
	pimpl->modifiers[L"shft"] = KEY_SHIFT;
	pimpl->modifiers[L"super"] = KEY_SUPER;
	pimpl->modifiers[L"win"] = KEY_SUPER;
	pimpl->modifiers[L"windows"] = KEY_SUPER;
	pimpl->modvk[KEY_CTRL] = VK_CONTROL;
	pimpl->modvk[KEY_ALT] = VK_MENU;
	pimpl->modvk[KEY_SHIFT] = VK_SHIFT;
	pimpl->modvk[KEY_SUPER] = VK_LWIN;
	pimpl->keys[L"a"] = 'A';
	pimpl->keys[L"b"] = 'B';
	pimpl->keys[L"c"] = 'C';
	pimpl->keys[L"d"] = 'D';
	pimpl->keys[L"e"] = 'E';
	pimpl->keys[L"f"] = 'F';
	pimpl->keys[L"g"] = 'G';
	pimpl->keys[L"h"] = 'H';
	pimpl->keys[L"i"] = 'I';
	pimpl->keys[L"j"] = 'J';
	pimpl->keys[L"k"] = 'K';
	pimpl->keys[L"l"] = 'L';
	pimpl->keys[L"m"] = 'M';
	pimpl->keys[L"n"] = 'N';
	pimpl->keys[L"o"] = 'O';
	pimpl->keys[L"p"] = 'P';
	pimpl->keys[L"q"] = 'Q';
	pimpl->keys[L"r"] = 'R';
	pimpl->keys[L"s"] = 'S';
	pimpl->keys[L"t"] = 'T';
	pimpl->keys[L"u"] = 'U';
	pimpl->keys[L"v"] = 'V';
	pimpl->keys[L"w"] = 'W';
	pimpl->keys[L"x"] = 'X';
	pimpl->keys[L"y"] = 'Y';
	pimpl->keys[L"z"] = 'Z';
	pimpl->keys[L"0"] = '0';
	pimpl->keys[L"1"] = '1';
	pimpl->keys[L"2"] = '2';
	pimpl->keys[L"3"] = '3';
	pimpl->keys[L"4"] = '4';
	pimpl->keys[L"5"] = '5';
	pimpl->keys[L"6"] = '6';
	pimpl->keys[L"7"] = '7';
	pimpl->keys[L"8"] = '8';
	pimpl->keys[L"9"] = '9';
	pimpl->keys[L"+"] = VK_ADD;
	pimpl->keys[L"add"] = VK_ADD;
	pimpl->keys[L"plus"] = VK_ADD;
	pimpl->keys[L"-"] = VK_SUBTRACT;
	pimpl->keys[L"subtract"] = VK_SUBTRACT;
	pimpl->keys[L"minus"] = VK_SUBTRACT;
	pimpl->keys[L"tab"] = VK_TAB;
	pimpl->keys[L"pgup"] = VK_PRIOR;
	pimpl->keys[L"pageup"] = VK_PRIOR;
	pimpl->keys[L"pgdown"] = VK_NEXT;
	pimpl->keys[L"pagedown"] = VK_NEXT;
	pimpl->keys[L"f1"] = VK_F1;
	pimpl->keys[L"f2"] = VK_F2;
	pimpl->keys[L"f3"] = VK_F3;
	pimpl->keys[L"f4"] = VK_F4;
	pimpl->keys[L"f5"] = VK_F5;
	pimpl->keys[L"f6"] = VK_F6;
	pimpl->keys[L"f7"] = VK_F7;
	pimpl->keys[L"f8"] = VK_F8;
	pimpl->keys[L"f9"] = VK_F9;
	pimpl->keys[L"f10"] = VK_F10;
	pimpl->keys[L"f11"] = VK_F11;
	pimpl->keys[L"f12"] = VK_F12;
	pimpl->keys[L"left"] = VK_LEFT;
	pimpl->keys[L"right"] = VK_RIGHT;
	pimpl->keys[L"up"] = VK_UP;
	pimpl->keys[L"down"] = VK_DOWN;
	pimpl->keys[L"back"] = VK_BACK;
	pimpl->keys[L"backspace"] = VK_BACK;
	pimpl->keys[L"home"] = VK_HOME;
	pimpl->keys[L"esc"] = VK_ESCAPE;
	pimpl->keys[L"escape"] = VK_ESCAPE;
	pimpl->keys[L"del"] = VK_DELETE;
	pimpl->keys[L"delete"] = VK_DELETE;
	pimpl->keys[L"enter"] = VK_RETURN;
	pimpl->keys[L"return"] = VK_RETURN;
	pimpl->keys[L"ret"] = VK_RETURN;
	pimpl->keys[L"space"] = VK_SPACE;
	pimpl->keys[L"spacebar"] = VK_SPACE;
	pimpl->keys[L"home"] = VK_HOME;
	pimpl->keys[L"end"] = VK_END;
	pimpl->keys[L"insert"] = VK_INSERT;
	pimpl->keys[L"."] = VK_OEM_PERIOD;
	pimpl->keys[L"period"] = VK_OEM_PERIOD;
	pimpl->keys[L","] = VK_OEM_COMMA;
	pimpl->keys[L"comma"] = VK_OEM_COMMA;
	pimpl->keys[L"/"] = VK_DIVIDE;
	pimpl->keys[L"slash"] = VK_DIVIDE;
	pimpl->keys[L"divide"] = VK_DIVIDE;
	pimpl->keys[L"\\"] = VK_OEM_102;
	pimpl->keys[L"backslash"] = VK_OEM_102;
	pimpl->keys[L"pause"] = VK_PAUSE;
	pimpl->keys[L"break"] = VK_PAUSE;
	pimpl->keys[L"insert"] = VK_INSERT;
	pimpl->keys[L"ins"] = VK_INSERT;
	pimpl->keys[L"["] = VK_OEM_4;
	pimpl->keys[L"{"] = VK_OEM_4;
	pimpl->keys[L"]"] = VK_OEM_6;
	pimpl->keys[L"}"] = VK_OEM_6;
	pimpl->keys[L"'"] = VK_OEM_7;
	pimpl->keys[L"`"] = VK_OEM_3;

	/* Load configuration files. */
	wpath p(initial_path<wpath>());
	for(wdirectory_iterator di(p); di != wdirectory_iterator(); ++di)
	{
		/* Ignore directories. */
		if( is_directory(di->status()) ) continue;

		/* Get the full path. */
		wstring fullpath = di->path().file_string();

		/* Ignore everthing except text files. */
		if( !endsWith(fullpath, L".txt") ) continue;

		/* Get application name. */
		wstring appname = di->path().filename();
		trim(appname);
		transform(begin(appname), end(appname), begin(appname), ::tolower);
		ItemList &il = pimpl->allitems[appname.substr(0, appname.length() - 4)];
		il.path = fullpath;
		list<Item> &items = il.items;

		/* Read the file line by line. */
		wifstream fs(fullpath);
		wstring line;
		while(getline(fs, line))
		{
			/* Parse each line of the file into segments. */
			wstringstream ss(line);
			wstring segment;
			vector<wstring> segments;
			while(getline(ss, segment, PART_DELIM))
			{
				segments.push_back(segment);
			}

			/* Parse item name. */
			if(segments.size() > 0)
			{
				/* We have a shortcut. */
				Item item;
				item.name = trim(segments[0]);
				item.count = 0;

				/* We have a description. */
				if(segments.size() > 1)
				{
					item.desc = trim(segments[1]);
				}

				/* We have shortcut keys. */
				if(segments.size() > 2)
				{
					wstring mkeysstr;
					wstringstream ss2(segments[2]);
					while(getline(ss2, mkeysstr, ALT_KEY_DELIM))
					{
						wstring keysstr;
						wstringstream ss3(mkeysstr);
						vector<pair<unsigned char, unsigned char>> keylist;
						while(getline(ss3, keysstr, KEY_DELIM))
						{
							transform(begin(keysstr), end(keysstr), begin(keysstr), ::tolower);
							wstringstream ss3(trim(keysstr));
							wstring keystr;
							unsigned char mod = KEY_NOMOD;
							unsigned char k = 0;
							while(ss3 >> keystr)
							{
								auto modit = pimpl->modifiers.find(keystr);
								auto keyit = pimpl->keys.find(keystr);
								if( modit != end(pimpl->modifiers) )
								{
									mod += modit->second;
								}
								else if( keyit != end(pimpl->keys) )
								{
									k = keyit->second;
								}
								else
								{
									/* Key not found. */
								}
							}
							keylist.push_back(make_pair(mod, k));
						}
						item.keys.push_back(keylist);
					}
				}

				/* Save the item in the item list. */
				items.push_back(item);
			}
		}
	}

	/* Load stats file. */
	wpath pstat(initial_path<wpath>());
	pstat /= STATS_FILENAME;
	wifstream fst(pstat.file_string());
	wstring sline;
	while(getline(fst, sline))
	{
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
			for(auto& i : it->second.items)
			{
				wstring lname = i.name;
				transform(begin(lname), end(lname), begin(lname), ::tolower);
				if( lname == sparts[1] )
				{
					wstringstream cnt(sparts[2]);
					cnt >> i.count;
					break;
				}
			}
		}
	}
}

MenuItems::~MenuItems()
{
	Save();
}

vector<Item> MenuItems::GetItems(wstring application, wstring text)
{
	transform(begin(text), end(text), begin(text), ::tolower);
	auto& all = pimpl->allitems[application];
	auto& items = all.items;
	set<Item> matches;

	/* Split words from text into vector. */
	vector<wstring> words;
	wstringstream iss(text);
	wstring word;
	while(getline(iss, word, L' '))
	{
		trim(word);
		if( !word.empty() )
		{
			words.push_back(word);
		}
	}

	/* Try to match all words from name, descriptio and shortcut. */
	for(auto &it : items)
	{
		wstring lname = it.name;
		wstring desc = it.desc;
		wstring shortcut = KeysFromItem(it, L" + ");
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
			matches.insert(it);
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

	/* Send keys for shortcut. */
	if( item.keys.size() > 0 )
	{
		for(auto& k: item.keys.front())
		{
			vector<INPUT> inputs;
			for(unsigned char c = 1; c != 0; c <<= 1)
			{
				if(c & k.first)
				{
					INPUT input = {0};
					input.type = INPUT_KEYBOARD;
					input.ki.wVk = pimpl->modvk[c];
					inputs.push_back(input);
				}
			}
			INPUT input = {0};
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = k.second;
			inputs.push_back(input);

			/* Send key downs. */
			::SendInput(inputs.size(), &inputs[0], sizeof(INPUT));

			Sleep(15);
		
			for(auto& i: inputs)
			{
				i.ki.dwFlags = KEYEVENTF_KEYUP;
			}
			/* Send key ups. */
			::SendInput(inputs.size(), &inputs[0], sizeof(INPUT));
		}
	}

	/* Increase count for this item. */
	auto& items = pimpl->allitems[application];
	for(auto& i: items.items)
	{
		/* Assume item name is unique within app. */
		if(i.name == item.name)
		{
			++i.count;
		}
	}

	/* Save items. */
	Save();
}

wstring MenuItems::KeysFromItem(Item item, wstring sep)
{
	wstring keystr;
	bool firstlist = true;
	for(auto& kl : item.keys)
	{
		bool first = true;
		if(firstlist)
		{
			firstlist = false;
		}
		else
		{
			keystr += L"; ";
		}
		for(auto& k : kl)
		{
			if(first)
			{
				first = false;
			}
			else
			{
				keystr += L", ";
			}
			bool done = false;
			if(k.first & KEY_CTRL)
			{
				if(done) keystr += sep;
				keystr += L"Ctrl";
				done = true;
			}
			if(k.first & KEY_SHIFT)
			{
				if(done) keystr += sep;
				keystr += L"Shift";
				done = true;
			}
			if(k.first & KEY_ALT)
			{
				if(done) keystr += sep;
				keystr += L"Alt";
				done = true;
			}
			if(k.first & KEY_SUPER)
			{
				if(done) keystr += sep;
				keystr += L"Win";
				done = true;
			}
			if(done) keystr += sep;
			for(auto &l : pimpl->keys)
			{
				if(l.second == k.second)
				{
					wstring ch = l.first;
					ch[0] = ::toupper(ch[0]);
					keystr += ch;
					break;
				}
			}
		}
	}
	return keystr;
}

wstring MenuItems::ItemToString(Item item)
{
	wstring disp(item.name);
	disp += L" (";
	disp += KeysFromItem(item, L" + ");
	disp += L")";
	return disp;
}

void MenuItems::Save()
{
	wpath pstat(initial_path<wpath>());
	pstat /= STATS_FILENAME;
	wofstream fst(pstat.file_string(), ios::trunc | ios::out);
	for(auto& app : pimpl->allitems)
	{
		for(auto& s : app.second.items)
		{
			fst << app.first << " | ";
			fst << s.name << " | ";
			fst << s.count << endl;
		}
	}
}

static bool isspace(const wstring& str)
{
	auto it = begin(str);
	do {
		if (it == end(str)) return true;
	} while (*it >= 0 && *it <= 0x7f && isspace(*(it++)));
	return false;
}

static inline wstring &ltrim(wstring &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](int i){ return !isspace(i); }));
	return s;
}

static inline wstring &rtrim(wstring &s) {
	s.erase(find_if(s.rbegin(), s.rend(), [](int i){ return !isspace(i); }).base(), s.end());
	return s;
}

static inline wstring &trim(wstring &s) {
	return ltrim(rtrim(s));
}

static bool endsWith (wstring const &fullString, wstring const &ending)
{
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}
 
static void replaceAll(wstring &s, const wstring &from, const wstring &to)
{
	size_t pos = 0;
	while((pos = s.find(from, pos)) != wstring::npos) {
		s.replace(pos, from.length(), to);
		pos += to.length();
	}
}

static unsigned char countbits(unsigned char b)
{
	unsigned char count;
	for(count = 0; b != 0; count++)
	{
		b &= b - 1;
	}

	return count;
}