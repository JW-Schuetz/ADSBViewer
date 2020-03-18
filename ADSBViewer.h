#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CADSBViewerApp:
class CADSBViewerApp : public CWinApp
{
public:
	CADSBViewerApp() noexcept;

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	UINT  m_nAppLook = 0;
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CADSBViewerApp theApp;