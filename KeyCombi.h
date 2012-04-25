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
#pragma once

#include <string>
#include <ostream>

class KeyCombi
{
public:
	explicit KeyCombi(unsigned char mods=0, unsigned char key=0);
	explicit KeyCombi(std::wstring text);
	~KeyCombi();
	std::wstring str(std::wstring sep) const;
	unsigned char mods(void) const { return _mods; }
	unsigned char key(void) const { return _key; }
	std::wstring keystr(void) const { return _keystr; }

private:
	unsigned char _mods;
	unsigned char _key;
	std::wstring _keystr;
};

inline std::wostream& operator<<(std::wostream& os, const KeyCombi& obj) 
{ 
  os << obj.str(L" ");
  return os;
} 
