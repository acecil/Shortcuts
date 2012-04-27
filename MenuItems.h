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
#include <list>
#include <vector>
#include <memory>
#include "KeyCombiAlt.h"

struct Item
{
	std::wstring name;
	std::wstring desc;
	KeyCombiAlt keys;
	unsigned int count;
	WORD command;
};

inline bool operator<(const Item& lhs, const Item& rhs)
{
	if( lhs.count == rhs.count )
	{
		return (lhs.name > rhs.name);
	}
	return (lhs.count < rhs.count);
}

class MenuItems
{
public:
	explicit MenuItems();
	virtual ~MenuItems();

	void UpdateMenuItems(std::wstring application, HWND currWin);
	bool IsConfigAvailable(std::wstring application);
	std::vector<Item> GetItems(std::wstring application, std::vector<std::wstring> text);
	void Launch(HWND hwnd, std::wstring application, Item item);
	void Save();
	
private:
	struct impl; std::unique_ptr<impl> pimpl;

};
