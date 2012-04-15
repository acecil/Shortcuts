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
#include "Shortcuts.h"

#include <algorithm>

#include "StringUtils.h"

using namespace std;

bool isspace(const wstring& str)
{
	auto it = begin(str);
	do {
		if (it == end(str)) return true;
	} while (*it >= 0 && *it <= 0x7f && isspace(*(it++)));
	return false;
}

wstring &ltrim(wstring &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](int i){ return !isspace(i); }));
	return s;
}

wstring &rtrim(wstring &s) {
	s.erase(find_if(s.rbegin(), s.rend(), [](int i){ return !isspace(i); }).base(), s.end());
	return s;
}

wstring &trim(wstring &s) {
	return ltrim(rtrim(s));
}

bool endsWith (wstring const &fullString, wstring const &ending)
{
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}
 
void replaceAll(wstring &s, const wstring &from, const wstring &to)
{
	size_t pos = 0;
	while((pos = s.find(from, pos)) != wstring::npos) {
		s.replace(pos, from.length(), to);
		pos += to.length();
	}
}

unsigned char countbits(unsigned char b)
{
	unsigned char count;
	for(count = 0; b != 0; count++)
	{
		b &= b - 1;
	}

	return count;
}