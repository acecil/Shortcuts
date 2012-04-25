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

#include <map>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include "StringUtils.h"

#include "KeyCombi.h"

using namespace std;

namespace
{
	map<unsigned char, wstring> initModStrings();
	map<wstring, unsigned char> initModifiers();
	map<wstring, unsigned char> initKeys();
	bool getMod(std::wstring text, unsigned char& mod);
	bool getKey(std::wstring text, unsigned char& key);

	map<unsigned char, wstring> modStrings(initModStrings());
	map<wstring, unsigned char> modifiers(initModifiers());
	map<wstring, unsigned char> keys(initKeys());
}

KeyCombi::KeyCombi(unsigned char mods, unsigned char key)
	: _mods(mods), _key(key)
{
	/* Find key in map. */
	for(auto& i: keys)
	{
		if( i.second == key )
		{
			_keystr = i.first;
			_keystr[0] = ::toupper(_keystr[0]);
			break;
		}
	}
}

KeyCombi::KeyCombi(wstring text)
	: _mods(0), _key(0)
{
	wstringstream ss(text);
	wstring part;
	while(getline(ss, part, L' '))
	{
		transform(begin(part), end(part), begin(part), ::tolower);
		trim(part);
		unsigned char gmod = 0;
		unsigned char gk = 0;
		if( getMod(part, gmod) )
		{
			_mods += gmod;
		}
		else if( getKey(part, gk) )
		{
			_key = gk;
			_keystr = part;
			_keystr[0] = ::toupper(_keystr[0]);
		}
	}
}

KeyCombi::~KeyCombi(void)
{
}

wstring KeyCombi::str(wstring sep) const
{
	wstringstream ss;

	/* Get modifier keys as a string. */
	for(unsigned char i = 0; i < 8; ++i)
	{
		unsigned char bit = 1 << i;
		if( _mods & bit )
		{
			if( modStrings.find(bit) == end(modStrings) )
			{
				throw new invalid_argument("Bad modifier key");
			}
			ss << modStrings[bit]; 
			ss << sep;
		}
	}

	/* Get key as a string. */
	ss << keystr();

	return ss.str();
}

namespace
{
	bool getMod(wstring text, unsigned char& mod)
	{
		auto& it = modifiers.find(text);
		if( it == end(modifiers) )
		{
			return false;
		}
		mod = it->second;
		return true;
	}

	bool getKey(wstring text, unsigned char& key)
	{
		auto& it = keys.find(text);
		if( it == end(keys) )
		{
			return false;
		}
		key = it->second;
		return true;
	}

	map<unsigned char, wstring> initModStrings()
	{
		map<unsigned char, wstring> modStrings;
		modStrings[MOD_CONTROL] = L"Ctrl";
		modStrings[MOD_ALT] = L"Alt";
		modStrings[MOD_SHIFT] = L"Shift";
		modStrings[MOD_WIN] = L"Win";
		return modStrings;
	}

	map<wstring, unsigned char> initModifiers()
	{
		map<wstring, unsigned char> modifiers;
		modifiers[L"ctrl"] = MOD_CONTROL;
		modifiers[L"control"] = MOD_CONTROL;
		modifiers[L"alt"] = MOD_ALT;
		modifiers[L"shift"] = MOD_SHIFT;
		modifiers[L"shft"] = MOD_SHIFT;
		modifiers[L"super"] = MOD_WIN;
		modifiers[L"win"] = MOD_WIN;
		modifiers[L"windows"] = MOD_WIN;
		return modifiers;
	}
	map<wstring, unsigned char> initKeys()
	{
		map<wstring, unsigned char> keys;
		keys[L"a"] = 'A';
		keys[L"b"] = 'B';
		keys[L"c"] = 'C';
		keys[L"d"] = 'D';
		keys[L"e"] = 'E';
		keys[L"f"] = 'F';
		keys[L"g"] = 'G';
		keys[L"h"] = 'H';
		keys[L"i"] = 'I';
		keys[L"j"] = 'J';
		keys[L"k"] = 'K';
		keys[L"l"] = 'L';
		keys[L"m"] = 'M';
		keys[L"n"] = 'N';
		keys[L"o"] = 'O';
		keys[L"p"] = 'P';
		keys[L"q"] = 'Q';
		keys[L"r"] = 'R';
		keys[L"s"] = 'S';
		keys[L"t"] = 'T';
		keys[L"u"] = 'U';
		keys[L"v"] = 'V';
		keys[L"w"] = 'W';
		keys[L"x"] = 'X';
		keys[L"y"] = 'Y';
		keys[L"z"] = 'Z';
		keys[L"0"] = '0';
		keys[L"1"] = '1';
		keys[L"2"] = '2';
		keys[L"3"] = '3';
		keys[L"4"] = '4';
		keys[L"5"] = '5';
		keys[L"6"] = '6';
		keys[L"7"] = '7';
		keys[L"8"] = '8';
		keys[L"9"] = '9';
		keys[L"+"] = VK_ADD;
		keys[L"add"] = VK_ADD;
		keys[L"plus"] = VK_ADD;
		keys[L"-"] = VK_SUBTRACT;
		keys[L"subtract"] = VK_SUBTRACT;
		keys[L"minus"] = VK_SUBTRACT;
		keys[L"tab"] = VK_TAB;
		keys[L"pgup"] = VK_PRIOR;
		keys[L"pageup"] = VK_PRIOR;
		keys[L"pgdown"] = VK_NEXT;
		keys[L"pagedown"] = VK_NEXT;
		keys[L"f1"] = VK_F1;
		keys[L"f2"] = VK_F2;
		keys[L"f3"] = VK_F3;
		keys[L"f4"] = VK_F4;
		keys[L"f5"] = VK_F5;
		keys[L"f6"] = VK_F6;
		keys[L"f7"] = VK_F7;
		keys[L"f8"] = VK_F8;
		keys[L"f9"] = VK_F9;
		keys[L"f10"] = VK_F10;
		keys[L"f11"] = VK_F11;
		keys[L"f12"] = VK_F12;
		keys[L"left"] = VK_LEFT;
		keys[L"right"] = VK_RIGHT;
		keys[L"up"] = VK_UP;
		keys[L"down"] = VK_DOWN;
		keys[L"back"] = VK_BACK;
		keys[L"backspace"] = VK_BACK;
		keys[L"home"] = VK_HOME;
		keys[L"esc"] = VK_ESCAPE;
		keys[L"escape"] = VK_ESCAPE;
		keys[L"del"] = VK_DELETE;
		keys[L"delete"] = VK_DELETE;
		keys[L"enter"] = VK_RETURN;
		keys[L"return"] = VK_RETURN;
		keys[L"ret"] = VK_RETURN;
		keys[L"space"] = VK_SPACE;
		keys[L"spacebar"] = VK_SPACE;
		keys[L"home"] = VK_HOME;
		keys[L"end"] = VK_END;
		keys[L"insert"] = VK_INSERT;
		keys[L"."] = VK_OEM_PERIOD;
		keys[L"period"] = VK_OEM_PERIOD;
		keys[L","] = VK_OEM_COMMA;
		keys[L"comma"] = VK_OEM_COMMA;
		keys[L"/"] = VK_DIVIDE;
		keys[L"slash"] = VK_DIVIDE;
		keys[L"divide"] = VK_DIVIDE;
		keys[L"\\"] = VK_OEM_102;
		keys[L"backslash"] = VK_OEM_102;
		keys[L"pause"] = VK_PAUSE;
		keys[L"break"] = VK_PAUSE;
		keys[L"insert"] = VK_INSERT;
		keys[L"ins"] = VK_INSERT;
		keys[L"["] = VK_OEM_4;
		keys[L"{"] = VK_OEM_4;
		keys[L"]"] = VK_OEM_6;
		keys[L"}"] = VK_OEM_6;
		keys[L"'"] = VK_OEM_7;
		keys[L"`"] = VK_OEM_3;
		return keys;
	}
}
