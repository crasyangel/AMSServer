#include "stdafx.h"

#include "MFCLoginGUI.h"


/////////////////////////////////////////////////////////////////////////////
// MFCLoginGUI dialog


MFCLoginGUI::MFCLoginGUI(CWnd* pParent /*=NULL*/)
	: CDialog(MFCLoginGUI::IDD, pParent)
{
	//{{AFX_DATA_INIT(MFCLoginGUI)
	username = _T("");
	passwd	 = _T("");
	//}}AFX_DATA_INIT
}

void MFCLoginGUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MFCLoginGUI)
	DDX_Text(pDX, IDC_EDIT_USERNAME, username);
	DDX_Text(pDX, IDC_EDIT_PASSWD, passwd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MFCLoginGUI, CDialog)
	//{{AFX_MSG_MAP(CToolsDlg)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, OnLogin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void MFCLoginGUI::OnOk() 
{  
	//Do nothing for "Enter" key
}  

void MFCLoginGUI::OnCancel() 

{  
	//Do nothing for "Esc" key
}  

void MFCLoginGUI::OnSysCommand(UINT nID, LPARAM lParam )    //X button handle  
{  
	// close the dialog 
	if(SC_CLOSE == nID)
	{
		EndDialog(IDCANCEL); 
		CDialog::OnClose();  
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
	
}  

/////////////////////////////////////////////////////////////////////////////
// MFCLoginGUI message handlers

void MFCLoginGUI::OnLogin ()
{
	UpdateData(TRUE);

	if("" == username)
	{
		AfxMessageBox(_T("请输入用户名"));
		GetDlgItem(IDC_EDIT_USERNAME)->SetFocus();
		return;
	}

	if("" == passwd)
	{
		AfxMessageBox(_T("请输入密码"));
		GetDlgItem(IDC_EDIT_PASSWD)->SetFocus();
		return;
	}

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//TODO: connect to sql
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	CDialog::OnOK( );
}