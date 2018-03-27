// MapEditor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MapEditor.h"
#include "MainFrm.h"

#include "MapEditorDoc.h"
#include "MapEditorView.h"
#include "EditorProperties.h"
#include "MapDialogs.h"
#include "..\globals\Configuration.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CMapEditorApp

BEGIN_MESSAGE_MAP(CMapEditorApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)



	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// CMapEditorApp construction

CMapEditorApp::CMapEditorApp()
{
	//EnableHtmlHelp();

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}
CMapEditorApp::~CMapEditorApp()
{	
	//free(gProp);
	// Place all significant initialization in InitInstance
}

// The one and only CMapEditorApp object

CMapEditorApp theApp;

// CMapEditorApp initialization

BOOL CMapEditorApp::InitInstance()
{
	gProp.loadProp();
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();
	
	CWinApp::InitInstance();
	
	ErrorHandler.Init( ErrOutputCallBack );
	//ErrorHandler.MsgOut( "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ MAP EDITOR @@@@@@@@@@@@@@@@@@@@@@@@@" );

	
	// initialize the ModWorkshop class
	gProp.ModWorkshop.GetModsList( "Mods", NULL );

	
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	AfxInitRichEdit2();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	//SetRegistryKey(_T("Map Editor"));
	//LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMapEditorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMapEditorView));
	
	if (!pDocTemplate)
		return FALSE;
	
	AddDocTemplate(pDocTemplate);
	// Parse command line for standard shell commands, DDE, file open
	
	CCommandLineInfo cmdInfo;
	//ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	CString str;
	CWnd * wnd;
	wnd =AfxGetMainWnd();
	if(wnd){
		str.LoadStringA(AFX_IDS_APP_TITLE);
		wnd->SetWindowText(str);
	}


	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CMapEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CMapEditorApp message handlers
