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

#include <sstream>

#include "afxdialogex.h"
#include <TlHelp32.h>
#include <Psapi.h>
#pragma comment(lib, "version")

#include "Resource.h"
#include "ConfigDlg.h"
#include "Config.h"
#include "StringUtils.h"
#include "ConfigParams.h"

#include "ShortcutsDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	const int SHORCUT_HOTKEY = 100;
	const int MAX_DISPLAY_ITEMS = 20;
	const int LISTBOX_OFFSET = 2;
	const int LISTBOX_HEIGHT = 4;
	const int ITEM_HEIGHT = 10;
	const std::wstring DEFAULT_CONFIG(L"_config.conf");
	const std::wstring DEFAULT_HOTKEY(L"Win A");
	const COLORREF DEFAULT_TEXT_COL(RGB(0, 0, 0));
	const COLORREF DEFAULT_SHORTCUT_COL(RGB(255, 0, 0));

	template<typename T>
	struct Pos
	{
		Pos(T _x, T _y)
			: x(_x), y(_y) {}
		Pos(std::wstring str)
		{
			std::wstringstream ss(str);
			ss >> x;
			ss >> y;
		}
		T x;
		T y;
	};

	template<typename T>
	inline std::wostream& operator<<(std::wostream& os, const Pos<T>& obj) 
	{ 
	  os << obj.x << L" " << obj.y;
	  return os;
	}
}

BEGIN_MESSAGE_MAP(ShortcutsDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ACTIVATE()
	ON_EN_CHANGE(IDC_EDIT1, &ShortcutsDlg::OnEnChangeEntry)
	ON_WM_HOTKEY()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_BN_CLICKED(IDC_SETTINGS_BTN, &ShortcutsDlg::OnBnClickedSettingsBtn)
END_MESSAGE_MAP()

ShortcutsDlg::ShortcutsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(ShortcutsDlg::IDD, pParent),
	config(new Config(DEFAULT_CONFIG))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	futureItems = std::async([](){ return new MenuItems(); });
}

ShortcutsDlg::~ShortcutsDlg()
{
	/* Make sure we unregister the hotkey. */
	UnregisterHotKey(GetSafeHwnd(), SHORCUT_HOTKEY);
}

void ShortcutsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, entryBox);
	DDX_Control(pDX, IDC_LIST1, shortcutList);
	DDX_Control(pDX, IDC_SETTINGS_BTN, settingsBtn);
}

BOOL ShortcutsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	/* Set icon for settings button. */
	HICON settingsIcon = (HICON)LoadImage(AfxGetInstanceHandle(), 
		MAKEINTRESOURCE(IDI_SETTINGS), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
	settingsBtn.SetIcon(settingsIcon);

	/* Get configuration hotkey (or default if not set). */
	KeyCombi key(config->GetSetParam<KeyCombi>(HOTKEY_PARAM, KeyCombi(DEFAULT_HOTKEY)));

	shortcutList.SetTextColour(config->GetSetParam<COLORREF>(TEXT_COL_PARAM, DEFAULT_TEXT_COL));
	shortcutList.SetShortcutColour(config->GetSetParam<COLORREF>(SHORTCUT_COL_PARAM, DEFAULT_SHORTCUT_COL));

	/* Hot key for displaying window. */
	RegisterHotKey(GetSafeHwnd(), SHORCUT_HOTKEY, key.mods(), key.key());

	setInitialPosition();

	switchWinState(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void ShortcutsDlg::OnPaint()
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
HCURSOR ShortcutsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ShortcutsDlg::OnEnChangeEntry()
{
	/* Get menu items object from async task if not already done. */
	if( !items )
	{
		items = std::move(std::unique_ptr<MenuItems>(futureItems.get()));
	}

	/* Set hint text. */
	std::wstring hintApp = currApp;
	std::wstring hint = L"No shortcuts available";
	if( !currApp.empty() )
	{
		hintApp[0] = ::toupper(hintApp[0]);
		hint = L"Shortcuts for " + hintApp;
	}
	entryBox.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)hint.c_str());

	/* Get text from entry box. */
	CString search;
	entryBox.GetWindowTextW(search);
	
	/* Split entry box text into words. */
	std::wstring text = search.GetBuffer();
	text = trim(text);
	std::transform(std::begin(text), std::end(text), std::begin(text), ::tolower);
	std::vector<std::wstring> words;
	std::wstringstream iss(text);
	std::wstring word;
	while (std::getline(iss, word, L' '))
	{
		trim(word);
		if( !word.empty() )
		{
			words.push_back(word);
		}
	}

	/* Get items. */
	selectedItems = items->GetItems(currApp, currVersion[0], words);
	shortcutList.ResetContent();

	/* Pass words to list box. */
	shortcutList.SetSearchWords(words);

	/* Resize list box and dialog. */
	int numItems = min(selectedItems.size(), MAX_DISPLAY_ITEMS);
	int listH = (numItems > 0) ? LISTBOX_HEIGHT + numItems * ITEM_HEIGHT : 0;
	CRect newListRect(0, 0, 0, listH);
	MapDialogRect(&newListRect);
	CRect listRect;
	shortcutList.GetWindowRect(&listRect);
	ScreenToClient(&listRect);
	listRect.bottom = listRect.top + newListRect.bottom;
	shortcutList.SetWindowPos(nullptr, 0, 0, listRect.Width(), listRect.Height(),
		SWP_NOZORDER | SWP_NOMOVE);

	CRect dlgRect;
	GetWindowRect(&dlgRect);
	ScreenToClient(&dlgRect);
	if( numItems > 0 )
	{
		dlgRect.bottom = listRect.bottom;
	}
	else
	{
		dlgRect.bottom = listRect.top - LISTBOX_OFFSET;
	}
	SetWindowPos(nullptr, 0, 0, dlgRect.Width(), dlgRect.Height(),
		SWP_NOZORDER | SWP_NOMOVE);

	/* Refill list box. */
	for(auto &i : selectedItems)
	{
		shortcutList.AddString(i.desc, i.keys.str(L"+"));
	}

	/* Select first item. */
	shortcutList.SetCurSel(0);
}

void ShortcutsDlg::OnOK()
{
	/* Get menu items object from async task if not already done. */
	if( !items )
	{
		items = std::move(std::unique_ptr<MenuItems>(futureItems.get()));
	}

	/* Get the currently selected item from shortcutList. */
	int currItemIdx = shortcutList.GetCurSel();
	if( (0 <= currItemIdx) && (currItemIdx < (int)selectedItems.size()) )
	{
		Item currItem = selectedItems[currItemIdx];
		std::async([&](Item item){ items->Launch(currWin, currApp, item); }, currItem);
	}

	/* Hide window. */
	switchWinState(false);
}

void ShortcutsDlg::OnCancel()
{
	switchWinState(false);
}

void ShortcutsDlg::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
	/* Hide dialog when another window is activated. */
	if( nState == WA_INACTIVE )
	{
		switchWinState(false);
	}
}

void ShortcutsDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if( nHotKeyId == SHORCUT_HOTKEY )
	{
		switchWinState(!IsWindowVisible());
	}
}

void ShortcutsDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if((nChar == VK_UP) || (nChar == VK_DOWN))
	{
		/* Pass to list box. */
		shortcutList.SendMessage(WM_KEYDOWN, nChar, (nRepCnt << 16) | (nFlags & 0xFF));
	}
	else
	{
		/* Pass to base class. */
		CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void ShortcutsDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if((nChar == VK_UP) || (nChar == VK_DOWN))
	{
		/* Pass to list box. */
		shortcutList.SendMessage(WM_KEYUP, nChar, (nRepCnt << 16) | (nFlags & 0xFF));
	}
	else
	{
		/* Pass to base class. */
		CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
	}
}

void ShortcutsDlg::setInitialPosition()
{
	CRect rect;
	GetWindowRect(&rect);
	auto defX(::GetSystemMetrics(SM_CXSCREEN) / 2 - rect.Width() / 2);
	auto defY(::GetSystemMetrics(SM_CYSCREEN) / 3);
	auto initialPos(config->GetParam<Pos<int>>(INITIAL_POS_PARAM, Pos<int>(defX, defY)));
	SetWindowPos(nullptr, initialPos.x, initialPos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void ShortcutsDlg::switchWinState(bool show)
{
	/* Get the current application that has focus. */
	if(show)
	{
		std::wstring app = getProcFocus(currWin);
		if( (app.length() > 4) && (app.substr(app.length() - 4, 4) == L".exe") )
		{
			app = app.substr(0, app.length() - 4);
		}
		if( currApp != app )
		{
			/* Clear entry box. */
			entryBox.SetWindowText(L"");
		}

		/* Get full path to application. */
		DWORD procId = 0;
		::GetWindowThreadProcessId(currWin, &procId);
		HANDLE hProc = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procId);
		WCHAR path[_MAX_PATH];
		::GetModuleFileNameEx(hProc, nullptr, path, _MAX_PATH);
		::CloseHandle(hProc);

		/* Get application version. */
		DWORD verHandle = 0;
		DWORD size = ::GetFileVersionInfoSize(path, &verHandle);
		auto verInfo = std::make_unique<BYTE[]>(size);
		if (::GetFileVersionInfo(path, verHandle, size, verInfo.get()))
		{
			UINT len = 0;
			void *vsfiv = nullptr;
			::VerQueryValue(verInfo.get(), L"\\", &vsfiv, &len);
			auto vsfi = static_cast<VS_FIXEDFILEINFO*>(vsfiv);
			currVersion[0] = HIWORD(vsfi->dwFileVersionMS);
			currVersion[1] = LOWORD(vsfi->dwFileVersionMS);
			currVersion[2] = HIWORD(vsfi->dwFileVersionLS);
			currVersion[3] = LOWORD(vsfi->dwFileVersionLS);
		}
		

		/* Ignore our own configuration window. */
		if( app != L"shortcuts" )
		{
			currApp = app;

			/* Update stored list of menu items. */
			items->UpdateMenuItems(currApp, currWin, currVersion[0]);
		}
	}

	/* Prepopulate list. */
	OnEnChangeEntry();

	/* Show or hide our window. */
	ShowWindow(show ? SW_SHOW : SW_HIDE);
	if(show)
	{
		entryBox.SetFocus();
		/* Select text in the edit box. */
		entryBox.SetSel(0, entryBox.GetWindowTextLengthW());
		SetForegroundWindow();
	}
	else
	{
		SetWindowLong(GetSafeHwnd(), GWL_EXSTYLE, GetWindowLong(GetSafeHwnd(), GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
		ShowWindow(SW_SHOW);
		ShowWindow(SW_HIDE);
	}
}

std::wstring ShortcutsDlg::getProcFocus(HWND &hwnd)
{
    hwnd = ::GetForegroundWindow();
	DWORD procId;
	::GetWindowThreadProcessId(hwnd, &procId);
	HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	std::wstring procName;
	if(snapshot)
	{
		PROCESSENTRY32 process = {0};
		process.dwSize = sizeof(process);
		if(Process32First(snapshot, &process))
		{
			do
			{
				if( process.th32ProcessID == procId )
				{
					procName = process.szExeFile;
					break;
				}

			} while(Process32Next(snapshot, &process));
		}
		CloseHandle(snapshot);
	}
	/* Convert process name to lower case. */
	transform(begin(procName), end(procName), begin(procName), ::tolower);
	return procName;
}

void ShortcutsDlg::OnBnClickedSettingsBtn()
{
	/* Display configuration dialog. */
	ConfigDlg configDlg(this, *config);
	if( configDlg.DoModal() == IDOK )
	{
		Config configChanges(configDlg.GetChanges());
		
		/* Go through configuration applying changes. */
		for(auto& c: configChanges)
		{
			/* Apply change. */
			if( c.first == HOTKEY_PARAM )
			{
				KeyCombi key(configChanges.GetParam<KeyCombi>(HOTKEY_PARAM));
				UnregisterHotKey(GetSafeHwnd(), SHORCUT_HOTKEY);
				RegisterHotKey(GetSafeHwnd(), SHORCUT_HOTKEY, key.mods(), key.key());
			}
			else if( c.first == TEXT_COL_PARAM )
			{
				shortcutList.SetTextColour(configChanges.GetParam<COLORREF>(TEXT_COL_PARAM));
			}
			else if( c.first == SHORTCUT_COL_PARAM )
			{
				shortcutList.SetShortcutColour(configChanges.GetParam<COLORREF>(SHORTCUT_COL_PARAM));
			}

			/* Save config change. */
			config->SetParam(c.first, c.second);
		}

		/* Save configuration. */
		config->Save(DEFAULT_CONFIG);
	}
}

