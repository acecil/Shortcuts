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

class ListBox : public CListBox
{
	DECLARE_DYNAMIC(ListBox)

public:
	ListBox();
	virtual ~ListBox();

	void SetSearchWords(std::vector<std::wstring> words) { _words = words; }
	void AddString(std::wstring description, std::wstring shortcut);
	void SetTextColour(COLORREF textCol) { _textCol = textCol; }
	void SetShortcutColour(COLORREF shortcutCol) { _shortcutCol = shortcutCol; }

protected:
	DECLARE_MESSAGE_MAP()

private:
	bool _fontsSet;
	CFont _normalFont;
	CFont _boldFont;
	COLORREF _textCol;
	COLORREF _shortcutCol;
	std::vector<std::wstring> _words;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
};


