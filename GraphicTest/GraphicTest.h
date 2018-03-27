#pragma once

#include "stdafx.h"
#include "resource.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\GraphicObjects\Scene.h"
#include "..\GraphicObjects\GrObjectMesh.h"
#include "..\Graphic\Graphic.h"
#include "..\Graphic\GUIDialog.h"
#include "..\Graphic\Camera.h"
#include "..\Client\MapInit.h"


// include ScriptInterpreter
#ifdef SCRIPTINTERPRETER_EXPORTS
#undef SCRIPTINTERPRETER_EXPORTS
#endif
#include "..\ScriptInterpreter\ScriptInterpreter.h"
#include "..\ScriptInterpreter\Variables.h"


#define SI_MINOR_VERSION_NEEDED	0
#define SI_MAJOR_VERSION_NEEDED	1



// global variables
extern resManNS::CResourceManager	*ResourceManager;
extern graphic::CGraphic	*Graphic;
extern graphic::CScene		*Scene;
extern CTimeCounter			Timer;
extern CString				MapFileName;


enum LODMAPTYPE
{
	LTTerrain,
	LTModel,
	LTGrass
};


// functions
ATOM				MyRegisterClass( HINSTANCE hInstance );
BOOL				InitInstance( HINSTANCE, int );
LRESULT CALLBACK	WndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK	About( HWND, UINT, WPARAM, LPARAM );
void				KeyMessages( WPARAM wParam );
HRESULT				CheckConfiguration();
CString				CreateLODMapFileName( LODMAPTYPE type, int level );
LRESULT CALLBACK	WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK	WndProcDemoMode( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

HRESULT CALLBACK	OnCloseButtonClick( graphic::CGUIControlBase * pObject, LPARAM lParam );
HRESULT CALLBACK	OnCheckBox1Change( graphic::CGUIControlBase * pObject, LPARAM lParam );
HRESULT CALLBACK	OnCheckBox2Change( graphic::CGUIControlBase * pObject, LPARAM lParam );
HRESULT CALLBACK	OnTrackBarChange( graphic::CGUIControlBase * pObject, LPARAM lParam );
HRESULT CALLBACK	OnRButtonClick( graphic::CGUIControlBase * pObject, LPARAM lParam );
HRESULT CALLBACK	OnMenuSelect( graphic::CGUIControlBase * pObject, LPARAM lParam );
HRESULT CALLBACK	OnMenuLostFocus( graphic::CGUIControlBase * pObject, LPARAM lParam );
HRESULT CALLBACK	OnRadioChange( graphic::CGUIControlBase * pObject, LPARAM lParam );
HRESULT CALLBACK	OnComboChange( graphic::CGUIControlBase * pObject, LPARAM lParam );
HRESULT CALLBACK	OnImageClick( graphic::CGUIControlBase * pObject, LPARAM lParam );
