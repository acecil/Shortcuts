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
#include "Shortcuts.h"
#include "ShortcutsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ShortcutsApp

BEGIN_MESSAGE_MAP(ShortcutsApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// ShortcutsApp construction

ShortcutsApp::ShortcutsApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only ShortcutsApp object

ShortcutsApp theApp;


// ShortcutsApp initialization

BOOL ShortcutsApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	ShortcutsDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

