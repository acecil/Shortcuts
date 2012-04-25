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

#include <sstream>
#include <algorithm>
#include "StringUtils.h"

#include "KeyCombiMulti.h"

using namespace std;

namespace
{
	const wchar_t KEY_DELIM(L',');
}

KeyCombiMulti::KeyCombiMulti(wstring text)
{
	wstringstream ss(text);
	wstring part;
	while(getline(ss, part, KEY_DELIM))
	{
		transform(part.begin(), part.end(), part.begin(), ::tolower);
		trim(part);
		_keys.push_back(KeyCombi(part));
	}
}


KeyCombiMulti::~KeyCombiMulti(void)
{
}

wstring KeyCombiMulti::str(wstring sep) const
{
	wstringstream ss;
	bool first = true;
	for(auto& i: _keys)
	{
		if( first )
		{
			first = false;
		}
		else
		{
			ss << KEY_DELIM << L" ";
		}
		ss << i.str(sep);
	}

	return ss.str();
}
