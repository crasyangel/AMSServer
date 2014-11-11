#include "stdafx.h"


#include "MFCMainGUI.h"


/////////////////////////////////////////////////////////////////////////////
// MFCMainGUI dialog


MFCMainGUI::MFCMainGUI(u_short useport, CWnd* pParent /*=NULL*/)
	: CDialog(MFCMainGUI::IDD, pParent)
{
	serverruning = FALSE;
	serverport = useport;
	server = new HttpServer(serverport);

	//{{AFX_DATA_INIT(MFCMainGUI)
	projectid = _T("");
	vendorid  = _T("");
	//}}AFX_DATA_INIT
}

MFCMainGUI::~MFCMainGUI()
{
	delete server;
}

void MFCMainGUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MFCMainGUI)
	DDX_Text(pDX, IDC_EDIT_ProjectID, projectid);
	DDX_Text(pDX, IDC_EDIT_VendorID, vendorid);
	DDX_Text(pDX, IDC_EDIT_CSV, csvfilepath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MFCMainGUI, CDialog)
	//{{AFX_MSG_MAP(CToolsDlg)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_BUTTON_CHOOSECSV, OnChooseCsv)
	ON_BN_CLICKED(IDC_BUTTON_SUBMIT, SubmitSqlData)
	ON_BN_CLICKED(IDC_BUTTON_SERVER_START, StartServer)
	ON_BN_CLICKED(IDC_BUTTON_SERVER_STOP, StopServer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void MFCMainGUI::OnOk() 
{  
	//Do nothing for "Enter" key
}  

void MFCMainGUI::OnCancel() 
{  
	//Do nothing for "Esc" key
}  


void MFCMainGUI::OnSysCommand(UINT nID, LPARAM lParam )    //X button handle  
{  
	// close the dialog 
	if(SC_CLOSE == nID)
	{
		StopServer();
		EndDialog(IDCANCEL); 
		CDialog::OnClose();  
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}

}  

BOOL MFCMainGUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_BUTTON_SERVER_STOP)->EnableWindow(FALSE);

	return TRUE;
}


BOOL MFCMainGUI::VerifyCsvFile() 
{
	CString extension = csvfilepath.Right(4);
	CString rightextension = ".csv";

	if(0 == extension.CompareNoCase(rightextension))
	{
		AMS_DBUG("the file has a right extension\n");
		return TRUE;
	}
		
	return FALSE;
}

void MFCMainGUI::OnChooseCsv()
{
	UpdateData(TRUE);
	CFileDialog hFileDlg(TRUE,NULL,NULL,OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_READONLY,
		TEXT("CSV文件(*.csv)|*.csv|所有文件(*.*)|*.*"),NULL);
	hFileDlg.m_ofn.nFilterIndex=1;
	hFileDlg.m_ofn.hwndOwner=m_hWnd;
	hFileDlg.m_ofn.lStructSize=sizeof(OPENFILENAME);
	hFileDlg.m_ofn.lpstrTitle=TEXT("选择CSV文件...\0");
	hFileDlg.m_ofn.nMaxFile=MAX_PATH;
	if(hFileDlg.DoModal() == IDOK)
	{
		csvfilepath = hFileDlg.GetPathName();
		UpdateData(FALSE);
	}

}

void MFCMainGUI::SubmitSqlData()
{
	UpdateData(TRUE);

	if("" == projectid)
	{
		AfxMessageBox(_T("请输入项目ID"));
		GetDlgItem(IDC_EDIT_ProjectID)->SetFocus();
		return;
	}

	if("" == vendorid)
	{
		AfxMessageBox(_T("请输入制造商ID"));
		GetDlgItem(IDC_EDIT_VendorID)->SetFocus();
		return;
	}

	if("" == csvfilepath)
	{
		AfxMessageBox(_T("请选择CSV文件，或者输入全路径"));
		GetDlgItem(IDC_EDIT_CSV)->SetFocus();
		return;
	}

	if(FALSE == VerifyCsvFile())
	{
		csvfilepath = "";
		UpdateData(FALSE);
		AfxMessageBox(_T("文件格式错误，请选择.csv文件"));
		GetDlgItem(IDC_EDIT_CSV)->SetFocus();
		return;
	}
	
	//TODO: connect to sql



	if(serverruning)
	{
		StopServer();
		StartServer();
	}
}

void MFCMainGUI::StartServer()
{
	if(FALSE == IOCPServer::GetSingletonInstance()->Start())
	{
		AMS_DBUG("Server Start failed at port: %d\n", serverport);
		return;
	}
	AMS_DBUG("Server Start succeed at port: %d\n", serverport);

	serverruning = TRUE;
	GetDlgItem(IDC_BUTTON_SERVER_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_SERVER_STOP)->EnableWindow(TRUE);
}

void MFCMainGUI::StopServer()
{
	IOCPServer::GetSingletonInstance()->Stop();
	AMS_DBUG("Server Stop succeed at port: %d\n", serverport);

	serverruning = FALSE;
	GetDlgItem(IDC_BUTTON_SERVER_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_SERVER_START)->EnableWindow(TRUE);
}



