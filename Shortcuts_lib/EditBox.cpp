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
#include "EditBox.h"


// EditBox

IMPLEMENT_DYNAMIC(EditBox, CEdit)

BEGIN_MESSAGE_MAP(EditBox, CEdit)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

EditBox::EditBox()
{

}

EditBox::~EditBox()
{
}

void EditBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if( (nChar == VK_UP) || (nChar == VK_DOWN) )
	{
		/* Pass to parent. */
		CWnd *parent = GetParent();
		if(parent)
		{
			parent->SendMessage(WM_KEYDOWN, nChar, (nFlags << 16) | (nRepCnt & 0xFF));
		}
	}
	else
	{
		/* Pass to base class. */
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void EditBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if( (nChar == VK_UP) || (nChar == VK_DOWN) )
	{
		/* Pass to parent. */
		CWnd *parent = GetParent();
		if(parent)
		{
			parent->SendMessage(WM_KEYUP, nChar, (nFlags << 16) | (nRepCnt & 0xFF));
		}
	}
	else
	{
		/* Pass to base class. */
		CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
	}
}


// EditBox message handlers


