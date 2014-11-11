#pragma once

#include "stdafx.h"

#include "MFCMainGUI.h"
#include "MFCLoginGUI.h"

class MFCAPP : public CWinApp
{
public:
	MFCAPP(u_short useport);

private:
	u_short serverport;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GUIAPP)
public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
};