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
#include "ConfigDlg.h"
#include "Config.h"
#include "KeyCombi.h"
#include "MenuItems.h"
#include "ConfigParams.h"
#include "VERSION"

#include <string>

static const wchar_t APP_NAME[]{ L"Shortcuts" };
static const wchar_t AUTHOR[]{ L"Andrew Gascoyne-Cecil" };

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(ConfigDlg, CDialogEx)
	ON_BN_CLICKED(ID_CLOSE_BTN, &ConfigDlg::OnBnClickedCloseBtn)
	ON_BN_CLICKED(IDC_WIN_CHECK, &ConfigDlg::OnBnClickedWinCheck)
	ON_BN_CLICKED(IDC_CTRL_CHECK, &ConfigDlg::OnBnClickedCtrlCheck)
	ON_BN_CLICKED(IDC_ALT_CHECK, &ConfigDlg::OnBnClickedAltCheck)
	ON_BN_CLICKED(IDC_SHIFT_CHECK, &ConfigDlg::OnBnClickedShiftCheck)
	ON_CBN_SELCHANGE(IDC_LAUNCH_COMBO, &ConfigDlg::OnCbnSelchangeLaunchCombo)
	ON_BN_CLICKED(ID_TEXT_COL_BTN, &ConfigDlg::OnBnClickedTextColBtn)
	ON_BN_CLICKED(ID_SHORTCUT_COL_BTN, &ConfigDlg::OnBnClickedShortcutColBtn)
	ON_NOTIFY(NM_CLICK, IDC_GITHUB_LINK, &ConfigDlg::OnNMClickGithubLink)
END_MESSAGE_MAP()

ConfigDlg::ConfigDlg(CWnd* pParent,
					 const Config& oldCfg)
		: CDialogEx(ConfigDlg::IDD, pParent),
		oldConfig(oldCfg)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

ConfigDlg::~ConfigDlg()
{
}

Config ConfigDlg::GetChanges() const
{
	return configChanges;
}

void ConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VERSION_TEXT, versionText);
	DDX_Control(pDX, IDC_WIN_CHECK, winModCheck);
	DDX_Control(pDX, IDC_CTRL_CHECK, ctrlModCheck);
	DDX_Control(pDX, IDC_ALT_CHECK, altModCheck);
	DDX_Control(pDX, IDC_SHIFT_CHECK, shiftModCheck);
	DDX_Control(pDX, IDC_LAUNCH_COMBO, keyCombo);
	DDX_Control(pDX, ID_TEXT_COL_BTN, textColBtn);
	DDX_Control(pDX, ID_SHORTCUT_COL_BTN, shortcutColBtn);
}

BOOL ConfigDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	/* Set initial state. */
	KeyCombi key(oldConfig.GetParam<KeyCombi>(HOTKEY_PARAM));

	CString versionStr;
	versionStr.Format(L"%s V%s by %s", APP_NAME, SHORTCUTS_VERSION_WSTRING, AUTHOR);
	versionText.SetWindowText(versionStr);
	winModCheck.SetCheck((key.mods() & MOD_WIN) ? BST_CHECKED : BST_UNCHECKED);
	ctrlModCheck.SetCheck((key.mods() & MOD_CONTROL) ? BST_CHECKED : BST_UNCHECKED);
	shiftModCheck.SetCheck((key.mods() & MOD_SHIFT) ? BST_CHECKED : BST_UNCHECKED);
	altModCheck.SetCheck((key.mods() & MOD_ALT) ? BST_CHECKED : BST_UNCHECKED);

	int sel = keyCombo.FindStringExact(0, key.keystr().c_str());
	if( sel != CB_ERR )
	{
		keyCombo.SetCurSel(sel);
	}

	textColBtn.SetColour(getParam<COLORREF>(TEXT_COL_PARAM));
	shortcutColBtn.SetColour(getParam<COLORREF>(SHORTCUT_COL_PARAM));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void ConfigDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR ConfigDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ConfigDlg::OnNMClickGithubLink(NMHDR* pNMHDR, LRESULT *pResult)
{
	if (!pNMHDR)
	{
		*pResult = 0;
		return;
	}
	if (pNMHDR->code != NM_CLICK)
	{
		*pResult = 0;
		return;
	}

	NMLINK* link = reinterpret_cast<NMLINK*>(pNMHDR);
	LITEM& item = link->item;

	/* Open browser with link. */
	ShellExecute(nullptr, L"open", item.szUrl, nullptr, nullptr, SW_SHOWNORMAL);
	*pResult = 0;
}

void ConfigDlg::OnBnClickedCloseBtn()
{
	/* Exit application. */
	exit(0);
}

void ConfigDlg::OnBnClickedWinCheck()
{
	setHotkey();
}

void ConfigDlg::OnBnClickedCtrlCheck()
{
	setHotkey();
}

void ConfigDlg::OnBnClickedAltCheck()
{
	setHotkey();
}

void ConfigDlg::OnBnClickedShiftCheck()
{
	setHotkey();
}

void ConfigDlg::OnCbnSelchangeLaunchCombo()
{
	setHotkey();
}

void ConfigDlg::setHotkey()
{
	int selKey = keyCombo.GetCurSel();
	if( selKey == CB_ERR )
	{
		return;
	}
	std::wstring hotkey;
	if( winModCheck.GetCheck() == BST_CHECKED )
	{
		hotkey += L"Win ";
	}
	if( ctrlModCheck.GetCheck() == BST_CHECKED )
	{
		hotkey += L"Ctrl ";
	}
	if( altModCheck.GetCheck() == BST_CHECKED )
	{
		hotkey += L"Alt ";
	}
	if( shiftModCheck.GetCheck() == BST_CHECKED )
	{
		hotkey += L"Shift ";
	}
	CString keyString;
	keyCombo.GetLBText(selKey, keyString);
	hotkey += keyString;

	configChanges.SetParam(HOTKEY_PARAM, hotkey);
}

void ConfigDlg::OnBnClickedTextColBtn()
{
	CColorDialog dlg(getParam<COLORREF>(TEXT_COL_PARAM));
	if( dlg.DoModal() == IDOK )
	{
		configChanges.SetParam<COLORREF>(TEXT_COL_PARAM, dlg.GetColor());
		textColBtn.SetColour(dlg.GetColor());
	}
}

void ConfigDlg::OnBnClickedShortcutColBtn()
{
	CColorDialog dlg(getParam<COLORREF>(SHORTCUT_COL_PARAM));
	if( dlg.DoModal() == IDOK )
	{
		configChanges.SetParam<COLORREF>(SHORTCUT_COL_PARAM, dlg.GetColor());
		shortcutColBtn.SetColour(dlg.GetColor());
	}
}

