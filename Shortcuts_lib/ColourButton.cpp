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
#include "ColourButton.h"

IMPLEMENT_DYNAMIC(ColourButton, CButton)

BEGIN_MESSAGE_MAP(ColourButton, CButton)
	ON_WM_THEMECHANGED()
END_MESSAGE_MAP()


ColourButton::ColourButton()
	: _colour(RGB(0, 0, 0))
{
	OpenTheme();
}

ColourButton::~ColourButton()
{
	CloseTheme();
}

void ColourButton::SetColour(COLORREF colour)
{
	_colour = colour;
	Invalidate();
}

void ColourButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect clientRect(lpDrawItemStruct->rcItem);
	
	BOOL bIsPressed =	(lpDrawItemStruct->itemState & ODS_SELECTED);
	BOOL bIsFocused  = (lpDrawItemStruct->itemState & ODS_FOCUS);
	BOOL bIsDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED);
	BOOL bDrawFocusRect = !(lpDrawItemStruct->itemState & ODS_NOFOCUSRECT);

	if(_theme)
	{
		DWORD state = (bIsPressed)?PBS_PRESSED:PBS_NORMAL;
		if(state == PBS_NORMAL)
		{
			if(bIsFocused)
			{
				state = PBS_DEFAULTED;
			}
		}

		if(IsThemeBackgroundPartiallyTransparent(_theme, 
			BP_PUSHBUTTON, state))
		{
			DrawThemeParentBackground(lpDrawItemStruct->hwndItem, 
				lpDrawItemStruct->hDC, 
				&clientRect);
		}

		DrawThemeBackground(_theme, 
			lpDrawItemStruct->hDC, BP_PUSHBUTTON, 
			state, &clientRect, &clientRect);
	}
	else
	{
		DrawFrameControl(lpDrawItemStruct->hDC, &clientRect,
			DFC_BUTTON, lpDrawItemStruct->itemState);
	}
	
	CBrush brush(_colour);
	int offset = 5;
	clientRect.left += offset;
	clientRect.right -= offset;
	clientRect.top += offset;
	clientRect.bottom -= offset;
	::FillRect(lpDrawItemStruct->hDC, &clientRect, brush);
}

LRESULT ColourButton::OnThemeChanged()
{
	CloseTheme();
	OpenTheme();
	return 0;
}

