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

#include <string>
using namespace std;

bool isspace(const wstring& str);
wstring &ltrim(wstring &s);
wstring &rtrim(wstring &s);
wstring &trim(wstring &s);
bool endsWith (wstring const &fullString, wstring const &ending);
void replaceAll(wstring &s, const wstring &from, const wstring &to);
unsigned char countbits(unsigned char b);