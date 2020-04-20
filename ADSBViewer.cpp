#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "ADSBViewer.h"
#include "MainFrm.h"
#include "AboutDialog.h"
#include "ADSBViewerDoc.h"
#include "ADSBViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CADSBViewerApp
BEGIN_MESSAGE_MAP(CADSBViewerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CADSBViewerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// CADSBViewerApp construction
CADSBViewerApp::CADSBViewerApp() noexcept
{
	SetAppID(_T("JWS Soft/Hard.ADSBViewer.Viewer.1.0.1"));
}

// The one and only CADSBViewerApp object
CADSBViewerApp theApp;

// CADSBViewerApp initialization
BOOL CADSBViewerApp::InitInstance()
{
	CWinApp::InitInstance();

	EnableTaskbarInteraction(FALSE);

	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("ADS-B Viewer, JWS Soft/Hard"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CADSBViewerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CADSBViewerView));
	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo)) return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

// App command to run the dialog
void CADSBViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
