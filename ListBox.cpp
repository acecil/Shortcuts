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

#include <functional>
#include <memory>
#include <set>
using namespace std;

namespace {
	struct ListItem
	{
		ListItem(wstring description, wstring shortcut)
			: _description(description), _shortcut(shortcut) {}
		wstring _description;
		wstring _shortcut;
	};

	class SetColor
	{
	public:
		SetColor(function<COLORREF __stdcall(HDC, COLORREF)> f, HDC hdc, COLORREF color)
			: _f(f), _hdc(hdc), _oldColor(f(hdc, color)) {}
		~SetColor()
		{
			_f(_hdc, _oldColor);
		}
	private:
		function<COLORREF __stdcall(HDC, COLORREF)> _f;
		HDC _hdc;
		COLORREF _oldColor;
	};

	class AutoDC
	{
	public:
		AutoDC(CWnd* wnd) 
			: _wnd(wnd), _dc(wnd ? wnd->GetDC() : NULL) {}
		~AutoDC()
		{
			if( _wnd && _dc )
			{
				_wnd->ReleaseDC(_dc);
			}
		}
		CDC* operator->() const { return _dc; }
	private:
		CWnd* _wnd;
		CDC* _dc;
	};
}

IMPLEMENT_DYNAMIC(ListBox, CListBox)

BEGIN_MESSAGE_MAP(ListBox, CListBox)
END_MESSAGE_MAP()

ListBox::ListBox()
	: _fontsSet(FALSE)
{
}

ListBox::~ListBox()
{
}

void ListBox::AddString(wstring description, wstring shortcut)
{
	/* Create ListItem and add as pointer. */
	CListBox::AddString((LPCTSTR)(new ListItem(description, shortcut)));
}

void ListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ListItem* item(reinterpret_cast<ListItem*>(lpDrawItemStruct->itemData));

	CDC* dc(CDC::FromHandle(lpDrawItemStruct->hDC));

	if( !_fontsSet )
	{
		CFont *font = GetFont();
		LOGFONT logFont;
		font->GetLogFont(&logFont);
		_normalFont.CreateFontIndirectW(&logFont);
		logFont.lfWeight = FW_BOLD;
		_boldFont.CreateFontIndirectW(&logFont);
		_fontsSet = TRUE;
	}

	/* Choose colors. */
	COLORREF frontColor(::GetSysColor(COLOR_WINDOWTEXT));
	COLORREF backColor(::GetSysColor(COLOR_WINDOW));

	/* If this item is selected, set the background color 
	 * and the text color to appropriate values.
	 */
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
		(lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		frontColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		backColor = ::GetSysColor(COLOR_HIGHLIGHT);
	}

	/* Safely set colors. */
	SetColor setFgColor(::SetTextColor, *dc, frontColor);
	SetColor setBgColor(::SetBkColor, *dc, backColor);

	/* Erase background to correct color. */
	dc->FillSolidRect(&lpDrawItemStruct->rcItem, backColor);

	/* Inset text. */
	lpDrawItemStruct->rcItem.left += 5;
	
	/* Calculate rectangle for shortcut. */
	CRect shortRect(lpDrawItemStruct->rcItem);
	dc->DrawText(item->_shortcut.c_str(), -1, &shortRect, DT_CALCRECT);

	/* Search for words in description. */
	set<pair<size_t, size_t>> locs;
	for(auto& i: _words)
	{
		size_t idx = item->_description.find(i);
		locs.insert(make_pair(idx, i.size()));
	}

	/* Draw the description - cropping to prevent overlap with shortcut. */
	CRect descRect(lpDrawItemStruct->rcItem);
	descRect.right -= shortRect.Width() + 5;

	size_t lastIdx = 0;
	for(auto& l: locs)
	{
		size_t idx = l.first;
		size_t len = l.second;

		if( idx > lastIdx )
		{
			SetFont(&_normalFont);
			CRect rect(descRect);
			wstring sub(item->_description.substr(lastIdx, idx - lastIdx));
			dc->DrawText(sub.c_str(), -1, &rect, DT_CALCRECT);
			dc->DrawText(sub.c_str(), -1, &rect, 0);
			descRect.left = rect.right;
		}

		//SetFont(&_boldFont);
		CRect wrect(descRect);
		wstring sub(item->_description.substr(idx, len));
		//dc->DrawText(sub.c_str(), -1, &wrect, DT_CALCRECT);
		//dc->DrawText(sub.c_str(), -1, &wrect, 0);
		descRect.left = wrect.right;

		lastIdx = idx + len;
	}
	/* Draw any remaining portion. */
	SetFont(&_normalFont);
	wstring sub(item->_description.substr(lastIdx, string::npos));
	//dc->DrawText(sub.c_str(), -1, &descRect, 0);

	/* Draw the shortcut - red and right aligned. */
	SetColor setFgColor2(::SetTextColor, *dc, RGB(255, 0, 0));
	dc->DrawText(item->_shortcut.c_str(), -1, &lpDrawItemStruct->rcItem, DT_RIGHT);
}

void ListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	ListItem* item(reinterpret_cast<ListItem*>(lpMeasureItemStruct->itemData));
	
	/* Get rectangle for item. */
	CRect rect;
	GetItemRect(lpMeasureItemStruct->itemID, &rect);

	/* Safely get device context. */
	AutoDC dc(this);

	/* Measure text. */
	lpMeasureItemStruct->itemHeight = dc->DrawText(item->_description.c_str(), -1, rect, DT_CALCRECT); 
}

void ListBox::DeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	/* Delete data pointer. */
	ListItem* ptr(reinterpret_cast<ListItem*>(lpDeleteItemStruct->itemData));
	delete ptr;
}
