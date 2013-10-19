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
#include <map>
#include <algorithm>
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

	map<size_t, size_t> findWords(wstring text, vector<wstring> words);
	void drawString(CDC* dc, CFont* font, CFont* bFont, wstring text, map<size_t, size_t> locs, LPRECT lpRect, UINT format);
}

IMPLEMENT_DYNAMIC(ListBox, CListBox)

BEGIN_MESSAGE_MAP(ListBox, CListBox)
END_MESSAGE_MAP()

ListBox::ListBox()
	: _fontsSet(FALSE),
	_textCol(::GetSysColor(COLOR_WINDOWTEXT)),
	_shortcutCol(RGB(255, 0, 0))
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
	if (!item)
	{
		return;
	}

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
	COLORREF frontColor(_textCol);
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
	lpDrawItemStruct->rcItem.right -= 5;

	/* Search for words in shortcut. */
	map<size_t, size_t> slocs = findWords(item->_shortcut, _words);;
	
	/* Search for words in description. */
	map<size_t, size_t> dlocs = findWords(item->_description, _words);
	
	/* Calculate rectangle for shortcut. */
	CRect shortRect(lpDrawItemStruct->rcItem);
	drawString(dc, &_normalFont, &_boldFont, item->_shortcut, slocs, &shortRect, DT_CALCRECT);

	/* Draw the description - cropping to prevent overlap with shortcut. */
	CRect descRect(lpDrawItemStruct->rcItem);
	descRect.right -= shortRect.Width() + 5;
	drawString(dc, &_normalFont, &_boldFont, item->_description, dlocs, &descRect, 0);

	/* Draw the shortcut - red and right aligned. */
	CRect fullRect(lpDrawItemStruct->rcItem);
	int shortW = shortRect.Width();
	shortRect.left += fullRect.Width() - shortW;
	shortRect.right += fullRect.Width() - shortW;
	SetColor setFgColor2(::SetTextColor, *dc, _shortcutCol);
	drawString(dc, &_normalFont, &_boldFont, item->_shortcut, slocs, &shortRect, 0);
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

namespace
{
	map<size_t, size_t> findWords(wstring text, vector<wstring> words)
	{
		map<size_t, size_t> locs;
		transform(begin(text), end(text), begin(text), ::tolower);
		for(auto& i: words)
		{
			size_t idx = text.find(i);
			while(idx != std::string::npos)
			{
				locs[idx] = max(locs[idx], i.size());
				idx = text.find(i, idx + 1);
			}
		}
		/* Merge overlaps. */
		if( !locs.empty() )
		{
			auto& prev = begin(locs);
			auto& curr = begin(locs);
			++curr;
			for(; curr != end(locs); )
			{
				if((prev->first + prev->second) > curr->first)
				{
					/* Merge curr into prev and remove curr. */
					prev->second = max(prev->first + prev->second, curr->first + curr->second) - prev->first;

					locs.erase(curr++);
					prev = curr;
					--prev;
				}
				else
				{
					++curr;
					++prev;
				}
			}
		}
		return locs;
	}

	void drawString(CDC* dc, CFont* font, CFont* bFont, wstring text, map<size_t, size_t> locs, LPRECT lpRect, UINT format)
	{
		CRect rect(*lpRect);
		int right = rect.left;
		const wchar_t *rtext = text.c_str();
		const size_t textLen = text.length();
		size_t lastIdx = 0;
		for(auto& l: locs)
		{
			size_t idx = l.first;
			size_t len = l.second;

			if( idx > lastIdx )
			{
				dc->SelectObject(font);
				CRect frect(rect);
				dc->DrawText(rtext + lastIdx, idx - lastIdx, &frect, format | DT_CALCRECT);
				if( !(format & DT_CALCRECT) )
				{
					dc->DrawText(rtext + lastIdx, idx - lastIdx, &rect, format);
				}
				rect.left = min(rect.right, frect.right);
				right = rect.left;
			}

			dc->SelectObject(bFont);
			CRect wrect(rect);
			dc->DrawText(rtext + idx, len, &wrect, format | DT_CALCRECT);
			if( !(format & DT_CALCRECT) )
			{
				dc->DrawText(rtext + idx, len, &rect, format);
			}
			rect.left = min(rect.right, wrect.right);
			right = rect.left;

			lastIdx = idx + len;
		}
		/* Draw any remaining portion. */
		dc->SelectObject(font);
		if( lastIdx < textLen )
		{
			CRect erect(rect);
			dc->DrawText(rtext + lastIdx, textLen - lastIdx, &erect, format | DT_CALCRECT);
			if( !(format & DT_CALCRECT) )
			{
				dc->DrawText(rtext + lastIdx, textLen - lastIdx, &rect, format);
			}
			right = min(rect.right, erect.right);
		}
		lpRect->right = right;
	}
}