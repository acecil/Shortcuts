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

#include <filesystem>
#include <fstream>
#include <regex>
#include "StringUtils.h"
#include "Config.h"

using namespace std::tr2::sys;
using namespace std;

namespace
{
	const wstring CONFIG_DIR(L"Config");
};

Config::Config(wstring filename)
{
	if( filename.empty() )
	{
		/* No config to read. */
		return;
	}

	wpath p(initial_path<wpath>());
	p /= CONFIG_DIR;
	p /= filename;
	wifstream fst(p.file_string());
	wstring sline;
	while(getline(fst, sline))
	{
		/* Skip comment and empty lines. */
		wstring trimLine = trim(sline);
		if( sline.empty() || (sline[0] == '#') )
		{
			continue;
		}

		/* Check line matches format. */
		wregex rx(L"([^=]+)=(.*)", regex_constants::extended);
		wsmatch match;
		if( !regex_match(sline, match, rx) )
		{
			/* Ignore invalid lines. */
			continue;
		}
		
		/* Save key/value pair. */
		wstring key(match[1]);
		wstring value(match[2]);
		_params[trim(key)] = trim(value);
	}
}

Config::~Config(void)
{
}

void Config::Save(std::wstring filename)
{
	wpath p(initial_path<wpath>());
	p /= CONFIG_DIR;
	p /= filename;
	wofstream fst(p.file_string());
	for(auto& i: _params)
	{
		fst << i.first << L" = " << i.second << endl;
	}
}

