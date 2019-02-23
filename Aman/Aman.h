// Aman.h : main header file for the Aman DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAmanApp
// See Aman.cpp for the implementation of this class
//

class CAmanApp : public CWinApp
{
public:
	CAmanApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
