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
#include "afxwin.h"

#include "ColourButton.h"
#include "Config.h"

class ConfigDlg : public CDialogEx
{
public:
	ConfigDlg(CWnd* pParent,
			  const Config& oldCfg);
	virtual ~ConfigDlg();
	Config GetChanges() const;

// Dialog Data
	enum { IDD = IDD_CONFIG_DIALOG };

protected:
	HICON m_hIcon;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnPaint();
	HCURSOR OnQueryDragIcon();
	void OnBnClickedCloseBtn();
	DECLARE_MESSAGE_MAP()

private:
	const Config& oldConfig;
	Config configChanges;
	CStatic versionText;
	CButton winModCheck;
	CButton ctrlModCheck;
	CButton altModCheck;
	CButton shiftModCheck;
	CComboBox keyCombo;
	ColourButton textColBtn;
	ColourButton shortcutColBtn;

	void OnNMClickGithubLink(NMHDR *pNMHDR, LRESULT *pResult);
	void OnBnClickedWinCheck();
	void OnBnClickedCtrlCheck();
	void OnBnClickedAltCheck();
	void OnBnClickedShiftCheck();
	void OnCbnSelchangeLaunchCombo();
	void OnBnClickedTextColBtn();
	void OnBnClickedShortcutColBtn();
	void setHotkey();

	template<typename T>
	T getParam(std::wstring name)
	{
		T oldParam = oldConfig.GetParam<T>(name);
		return configChanges.GetParam<T>(name, oldParam);
	}
};


