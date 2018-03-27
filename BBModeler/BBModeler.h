#pragma once

#include "resource.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\GraphicObjects\GraphicObjectsIncludes.h"
#include "..\Graphic\Graphic.h"
#include "..\Graphic\Camera.h"


// constants
#define BBM_MAX_LOADSTRING		100


// global variables
extern CResourceManager		*ResourceManager;
extern graphic::CGraphic	*Graphic;
extern graphic::CScene		*Scene;
extern CTimeCounter			Timer;
extern CString				ModelFileName;
extern FLOAT				RotationStep;



// functions
ATOM				MyRegisterClass( HINSTANCE hInstance );
BOOL				InitInstance( HINSTANCE, int );
LRESULT CALLBACK	WndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK	About( HWND, UINT, WPARAM, LPARAM );
void CALLBACK		BBMDebugOutputCallBack( CString msg, int level );
void CALLBACK		BBMErrOutputCallBack( CString msg, int level );
void				ProcessMouseMoveMsg( WPARAM wParam, LPARAM lParam );
void				RotateModel( LPPOINT move );


