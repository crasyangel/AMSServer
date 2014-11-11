#pragma once

#include "stdafx.h"

#include "resource.h"

#include <ShlObj.h>
#include <afxdlgs.h>

#include "HttpServer.h"

class MFCMainGUI : public CDialog
{
	// Construction
public:
	MFCMainGUI(u_short useport, CWnd* pParent = NULL);   // standard constructor
	virtual ~MFCMainGUI(void);

	// Dialog Data
	//{{AFX_DATA(MFCMainGUI)
	enum { IDD = IDD_DIALOG_GUI };

private:
	u_short serverport;
	BOOL serverruning;
	IOCPServer *server;

private:
	CString	projectid;
	CString	vendorid;
	CString csvfilepath;
	//}}AFX_DATA

private:
	BOOL VerifyCsvFile(void); 
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MFCMainGUI)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MFCMainGUI)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam );
	afx_msg void OnOk(void);
	afx_msg void OnCancel(void);
	afx_msg BOOL OnInitDialog(void);
	afx_msg void OnChooseCsv(void);
	afx_msg void SubmitSqlData(void);
	afx_msg void StartServer(void);
	afx_msg void StopServer(void);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};