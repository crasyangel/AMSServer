#include "stdafx.h"

#include "MFCAPP.h"


/////////////////////////////////////////////////////////////////////////////
// MFCAPP construction

MFCAPP::MFCAPP(u_short useport):
	CWinApp()
{
	serverport = useport;
}


/////////////////////////////////////////////////////////////////////////////
// MFCAPP initialization

BOOL MFCAPP::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	MFCLoginGUI logingui;
	AMS_DBUG("logingui has been create!\n"); 

	int nresponse = 0;
	nresponse = logingui.DoModal();

	if (nresponse == IDOK)
	{
		MFCMainGUI maingui(serverport);
		AMS_DBUG("maingui has been create!\n");
		m_pMainWnd = &maingui;
		nresponse = maingui.DoModal();
		if (nresponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nresponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
	}
	else if (nresponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
