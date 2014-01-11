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

#include "Resource.h"
#include "EditBox.h"
#include "ListBox.h"
#include "MenuItems.h"
#include "afxwin.h"

class Config;

// ShortcutsDlg dialog
class ShortcutsDlg : public CDialogEx
{
// Construction
public:
	ShortcutsDlg(CWnd* pParent = nullptr);	// standard constructor
	virtual ~ShortcutsDlg();

// Dialog Data
	enum { IDD = IDD_SHORTCUTS_DIALOG };

protected:
	HICON m_hIcon;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnPaint();
	HCURSOR OnQueryDragIcon();
	void OnEnChangeEntry();
	virtual void OnOK();
	virtual void OnCancel();
	void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
	std::future<MenuItems*> futureItems;
	std::unique_ptr<MenuItems> items;
	std::vector<Item> selectedItems;
	HWND currWin;
	std::wstring currApp;
	WORD currVersion[4];
	EditBox entryBox;
	CButton settingsBtn;
	ListBox shortcutList;
	std::unique_ptr<Config> config;

	void setInitialPosition();
	void switchWinState(bool show);
	std::wstring getProcFocus(HWND &hwnd);
	void OnBnClickedSettingsBtn();

};
