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
#include "shortcuts.h"
#include "ListBox.h"

namespace {
struct ListItem
{
	wstring shortcut;
};
}

IMPLEMENT_DYNAMIC(ListBox, CListBox)
	
BEGIN_MESSAGE_MAP(ListBox, CListBox)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

ListBox::ListBox()
{

}

ListBox::~ListBox()
{
}

void ListBox::AddString(wstring description, wstring shortcut)
{
	/* Pass description as string for base class. */
	int idx = CListBox::AddString(description.c_str());

	/* Create ListItem and add as pointer. */
	ListItem *ptr = new ListItem();
	ptr->shortcut = shortcut;
	SetItemDataPtr(idx, ptr);
}

void ListBox::DeleteString(wstring description)
{
	/* Find index for item. */
	int loc = FindString(0, description.c_str());
	if( loc == LB_ERR )
	{
		/* String not found - nothing to do. */
		return;
	}

	/* Get item data pointer and delete. */
	ListItem *ptr = static_cast<ListItem*>(GetItemDataPtr(loc));
	delete ptr;

	/* Delete string. */
	CListBox::DeleteString(loc);
}

void ListBox::ResetContent()
{
	/* Delete all data. */
	for(int i = 0; i < GetCount(); ++i)
	{
		ListItem *ptr = static_cast<ListItem*>(GetItemDataPtr(i));
		delete ptr;
	}

	/* Call base class. */
	CListBox::ResetContent();
}

void ListBox::OnDestroy()
{
	/* Must call base class destroy. */
	CListBox::OnDestroy();

	/* Delete all data. */
	for(int i = 0; i < GetCount(); ++i)
	{
		ListItem *ptr = static_cast<ListItem*>(GetItemDataPtr(i));
		delete ptr;
	}
}

void ListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	/* TODO: Draw item ourselves. */
	CListBox::DrawItem(lpDrawItemStruct);
}

void ListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	/* TODO: Measure item ourselves. */
	CListBox::MeasureItem(lpMeasureItemStruct);
}






