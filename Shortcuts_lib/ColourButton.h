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

class ColourButton : public CButton
{
	DECLARE_DYNAMIC(ColourButton)

public:
	ColourButton();
	virtual ~ColourButton();

	void SetColour(COLORREF colour);

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	LRESULT OnThemeChanged();
	DECLARE_MESSAGE_MAP()

private:
	COLORREF _colour;
	HTHEME _theme;

	void OpenTheme() { _theme = OpenThemeData(GetSafeHwnd(), L"Button"); }
	void CloseTheme() { if(_theme) { CloseThemeData(_theme); _theme = nullptr; } }
};


