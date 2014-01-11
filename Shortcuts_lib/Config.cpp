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

using std::tr2::sys::wpath;
using std::tr2::sys::initial_path;

namespace
{
	const std::wstring AUTHOR(L"acecil");
	const std::wstring APP_NAME(L"Shortcuts");
	const std::wstring CONFIG_DIR(L"Config");
};

Config::Config(std::wstring filename)
{
	if( filename.empty() )
	{
		/* No config to read. */
		return;
	}

	wpath p(GetAppDataFolder());
	p /= CONFIG_DIR;
	p /= filename;
	std::wifstream fst(p.file_string());
	std::wstring sline;
	while (std::getline(fst, sline))
	{
		/* Skip comment and empty lines. */
		std::wstring trimLine = trim(sline);
		if( sline.empty() || (sline[0] == '#') )
		{
			continue;
		}

		/* Check line matches format. */
		std::wregex rx(L"([^=]+)=(.*)", std::regex_constants::extended);
		std::wsmatch match;
		if (!std::regex_match(sline, match, rx))
		{
			/* Ignore invalid lines. */
			continue;
		}
		
		/* Save key/value pair. */
		std::wstring key(match[1]);
		std::wstring value(match[2]);
		_params[trim(key)] = trim(value);
	}
}

Config::~Config(void)
{
}

void Config::Save(std::wstring filename)
{
	wpath p(GetAppDataFolder());
	p /= CONFIG_DIR;
	p /= filename;
	std::wofstream fst(p.file_string());
	for(auto& i: _params)
	{
		fst << i.first << L" = " << i.second << std::endl;
	}
}

wpath Config::GetAppDataFolder()
{
	wpath p(initial_path<wpath>());
	wchar_t szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, szPath)))
	{
		p = szPath;
		p /= AUTHOR;
		p /= APP_NAME;
		if (!is_directory(p))
		{
			p = initial_path<wpath>();
		}
	}
	return p;
}
