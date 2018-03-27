// MapEditor.h : main header file for the MapEditor application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CMapEditorApp:
// See MapEditor.cpp for the implementation of this class
//

class CMapEditorApp : public CWinApp
{
public:
	CMapEditorApp();
	~CMapEditorApp();
	
// Overrides
public:
	virtual BOOL InitInstance();
	
// Implementation
	
	afx_msg void OnAppAbout();
	
	
	DECLARE_MESSAGE_MAP()
};

extern CMapEditorApp theApp;