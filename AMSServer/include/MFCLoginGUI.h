#pragma once

#include "stdafx.h"

#include "resource.h"

class MFCLoginGUI : public CDialog
{
	// Construction
public:
	MFCLoginGUI(CWnd* pParent = NULL);   // standard constructor

	// Dialog Data
	//{{AFX_DATA(MFCLoginGUI)
	enum { IDD = IDD_DIALOG_LOGIN };

private:	
	CString	username;
	CString	passwd;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MFCLoginGUI)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MFCLoginGUI)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam );
	afx_msg void OnOk(void);
	afx_msg void OnCancel(void);
	afx_msg void OnLogin(void);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};