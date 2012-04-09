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

#include <memory>
#include <future>
using namespace std;

#include "EditBox.h"
#include "MenuItems.h"

// ShortcutsDlg dialog
class ShortcutsDlg : public CDialogEx
{
// Construction
public:
	ShortcutsDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~ShortcutsDlg();

// Dialog Data
	enum { IDD = IDD_SHORTCUTS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	void OnPaint();
	HCURSOR OnQueryDragIcon();
	void OnEnChangeEntry();
	virtual void OnOK();
	virtual void OnCancel();
	void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
	future<MenuItems*> futureItems;
	unique_ptr<MenuItems> items;
	vector<Item> selectedItems;
	HWND currWin;
	wstring currApp;
	EditBox entryBtn;
	CListBox shortcutList;

	void switchWinState();
	wstring getProcFocus(HWND &hwnd);
};
