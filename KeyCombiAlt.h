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
#include "KeyCombiMulti.h"

class KeyCombiAlt
{
public:
	KeyCombiAlt(){}
	KeyCombiAlt(std::wstring text);
	~KeyCombiAlt();
	std::wstring str(std::wstring sep) const;

	size_t size() { return _keys.size(); }
	KeyCombiMulti& front() { return _keys.front(); }
	const KeyCombiMulti& front() const { return _keys.front(); }

private:
	std::vector<KeyCombiMulti> _keys;
};

inline std::wostream& operator<<(std::wostream& os, const KeyCombiAlt& obj) 
{ 
  os << obj.str(L" ");
  return os;
} 

