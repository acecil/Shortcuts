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
using namespace std;

enum Modifier : unsigned char
{
	KEY_NOMOD	= 0x0,
	KEY_CTRL	= 0x1,
	KEY_SHIFT	= 0x2,
	KEY_ALT		= 0x4,
	KEY_SUPER   = 0x8
};

struct Item
{
	wstring name;
	wstring desc;
	list<vector<pair<unsigned char, unsigned char>>> keys;
	unsigned int count;
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

	vector<Item> GetItems(wstring application, wstring text);
	void Launch(HWND hwnd, wstring application, Item item);
	wstring KeysFromItem(Item item, wstring sep=L" ");
	wstring ItemToString(Item item);
	void Save();

private:
	struct impl; unique_ptr<impl> pimpl;
};