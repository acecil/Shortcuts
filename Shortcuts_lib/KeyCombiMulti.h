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
#include <vector>
#include <ostream>
#include "KeyCombi.h"

class KeyCombiMulti
{
public:
	KeyCombiMulti(){};
	KeyCombiMulti(std::wstring text);
	~KeyCombiMulti();
	std::wstring str(std::wstring sep) const;
	
	std::vector<KeyCombi>::iterator begin() { return _keys.begin(); }
	std::vector<KeyCombi>::iterator end() { return _keys.end(); }
	std::vector<KeyCombi>::const_iterator begin() const { return _keys.cbegin(); }
	std::vector<KeyCombi>::const_iterator end() const { return _keys.cend(); }
	void push_back(const KeyCombi& item) { _keys.push_back(item); }
	void push_back(KeyCombi&& item) { _keys.push_back(item); }

private:
	std::vector<KeyCombi> _keys;
};

inline std::wostream& operator<<(std::wostream& os, const KeyCombiMulti& obj) 
{ 
  os << obj.str(L" ");
  return os;
} 

