/*
	 BoundingBox Modeler v0.1
	==========================

	an easy tool for viewing and editing bounding boxes for any input .X file (DX mesh)
	
	program parameters:
	-f@@@@@@ - @@@@@@ means file name (or path) of .X file to be loaded
	-F@@@@@@ - @@@@@@ means file name (or path) of a stored bounding box

*/

#include "stdafx.h"
#include "BBModeler.h"


// Global Variables:
HWND				hWind;
HINSTANCE			hInst;								// current instance
TCHAR				szTitle[BBM_MAX_LOADSTRING];				// The title bar text
TCHAR				szWindowClass[BBM_MAX_LOADSTRING];		// the main window class name


CTimeCounter		Timer; // time counter
CResourceManager	*ResourceManager;
graphic::CGraphic	*Graphic;		   
graphic::CScene		*Scene;
CString				ModelFileName;
CString				BoxFileName;
graphic::LPGROBJECTMESH			Mesh;
graphic::LPGROBJECTBOUNDINGBOX	BoundingBox;
FLOAT				RotationStep = 10.0f;
graphic::CCamera	Camera;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// parses all arguments from command line to program parameters
//
//////////////////////////////////////////////////////////////////////////////////////////////
void ParseArgs()
{
	int		argv;
	LPWSTR	*args;
	char	text[520];
	CString	ret;
	int		ind;
	bool	BoxFileNameSpecified = false;

	args = CommandLineToArgvW( GetCommandLineW(), &argv );
	if ( !args ) return;
	for ( int i = 1; i < argv; i++ )
	{
		CString str = args[i];
		if ( str.GetLength() < 2 ) continue;
        switch ( str[1] )
		{
		case 'f':
			sscanf( (LPCTSTR) str, "-f%s", text );
			ModelFileName = text;
			if ( BoxFileNameSpecified ) break;
			ret = text;
			ind = ret.ReverseFind( '.' );
			if ( ( ind = ret.ReverseFind( '.' ) ) < 0 ) BoxFileName = ModelFileName + ".box";
			else BoxFileName = ModelFileName.Left( ind ) + ".box";
			break;
		case 'F':
			sscanf( (LPCTSTR) str, "-F%s", text );
			BoxFileName = text;
			BoxFileNameSpecified = true;
			break;
		}
	}

	GlobalFree( args );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// windows Main function
// contains only calls to RegisterClass, InitInstance and main program loop
//
//////////////////////////////////////////////////////////////////////////////////////////////
int APIENTRY _tWinMain(	HINSTANCE hInstance,
						HINSTANCE hPrevInstance,
						LPTSTR    lpCmdLine,
						int       nCmdShow )
{
	MSG msg;
	HACCEL hAccelTable;

	MyRegisterClass( hInstance );

	// Perform application initialization:
	if ( !InitInstance( hInstance, nCmdShow ) ) return FALSE;

	hAccelTable = LoadAccelerators( hInstance, (LPCTSTR) IDC_BBMODELER );

	// Main message loop:
	while ( true )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) 
		{
			if ( 0 == GetMessage( &msg, NULL, 0, 0 ) ) return (int) msg.wParam;
			else if( !TranslateAccelerator( hWind, hAccelTable, &msg ) )
			{
				TranslateMessage( &msg ); 
				DispatchMessage( &msg );
			}                                       
		}
		else 
		{
			Graphic->RenderScene();
		}
	}

	return (int) msg.wParam;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: MyRegisterClass()
//
//  Registers the window class.
//
//////////////////////////////////////////////////////////////////////////////////////////////
ATOM MyRegisterClass( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;

	// Initialize global strings
	LoadString( hInstance, IDS_APP_TITLE, szTitle, BBM_MAX_LOADSTRING );
	LoadString( hInstance, IDC_BBMODELER, szWindowClass, BBM_MAX_LOADSTRING );


	wcex.cbSize = sizeof( WNDCLASSEX ); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC) WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon( hInstance, (LPCTSTR) IDI_BBMODELER );
	wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= (HBRUSH) (COLOR_APPWORKSPACE + 1);
	wcex.lpszMenuName	= (LPCTSTR) IDC_BBMODELER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon( wcex.hInstance, (LPCTSTR) IDI_SMALL );

	return RegisterClassEx( &wcex );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function for ErrorHandler to output error msgs
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK BBMErrOutputCallBack( CString msg, int level )
{
	FILE	*fil;
	
	fil = fopen( "BBModelerError.log", _T( "at" ) );
	fprintf( fil, _T( "%s" ), (LPCTSTR) msg );
	fclose( fil );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function for output debug msgs when level is equal or less than the app DebugLevel 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK BBMDebugOutputCallBack( CString msg, int level )
{
	if ( level > DebugLevel ) return;
	
	FILE	*fil;
	
	fil = fopen( "BBModelerDebug.log", _T("at") );
	fprintf( fil, _T("%s"), (LPCTSTR) msg );
	fclose( fil );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// InitInstance makes all initializing steps:
//	1] Inits Error and Debug outputs, ProgramTimer
//	2] Parses program arguments and loads and cheks configuration data
//	3] Creates a window
//	4] Initializes a graphic object
//	5] Creates a ResourceManager and Scene object
//	6] Sets viewing parameters
//
// returns true when all of the initialization parts were successful
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	HRESULT				hr;
	graphic::CLight	*	pLight;


	hInst = hInstance; // Store instance handle in our global variable


	// timer initialization
	Timer.Start(); 

	// error handler and debug output initialization
	ErrorHandler.Init( BBMErrOutputCallBack );
	ErrorHandler.MsgOut( "\n@@@@@@@@@@@@@@@@@@@@@@@@    B B M o d e l e r    v 1 . 0   @@@@@@@@@@@@@@@@@@@@@@@@@" );
    DebugOutput.Init( BBMDebugOutputCallBack );

	ModelFileName = "";
	DebugLevel = 4;
	ParseArgs(); // ! parse program arguments



	// Create window
	hWind = CreateWindow(	szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 
							0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL );
	
	if ( !hWind ) return FALSE;
	OUTMSG( "Main window created successfully.", 2 );



	// ResourceManager initialization
	ResourceManager = new CResourceManager;
	ResourceManager->SetResourcePath( ".\\", RES_XFile );
	ResourceManager->SetResourcePath( ".\\", RES_GrSurface );
	ResourceManager->SetResourcePath( ".\\", RES_GrPlate );
	ResourceManager->SetResourcePath( ".\\", RES_Texture );
	ResourceManager->SetResourcePath( ".\\", RES_LoDMap );



	// CREATE GRAPHIC OBJECT and initialize it, resource manager has to be initialized first !!!
	Graphic = new graphic::CGraphic();

	graphic::GRAPHICINIT			grInit;

	ZeroMemory( &grInit, sizeof( grInit ) );
	grInit.hWnd = hWind;
	grInit.Width = 600;
	grInit.Height = 400;
	grInit.ColorDepth = 32;
	grInit.FullScreen = FALSE;
	grInit.TerrainLOD = 1;
	grInit.pResourceManager = ResourceManager;
	grInit.LightGlows = FALSE;
	
	hr = Graphic->Init( &grInit );
	if ( hr )
	{
		ErrorHandler.HandleError( ERRGENERIC, "GraphicTest::InitInstance()", "Graphic device initialization failed." );
		return FALSE;
	}
	OUTMSG( "Graphic device initialization successfully done.", 1 );

 	// Init static variables of GrObjectBase
	graphic::CGrObjectBase::StaticInit( Graphic->GetD3DDevice(), Graphic->GetShader(), ResourceManager );




	// SCENE INITIALIZATION
	Scene = new graphic::CScene;
	Scene->Init( ResourceManager );
	
	Mesh = new graphic::CGrObjectMesh;
	BoundingBox = new graphic::CGrObjectBoundingBox;
	ResourceManager->SetResourcePath( ".\\Data\\Models\\", RES_XFile );
	ResourceManager->SetResourcePath( ".\\Data\\Textures\\", RES_Texture );
	
	// load mesh object
	hr = Mesh->Init( FALSE, "auto.x", 5 );
	if ( hr ) ERRORMSG( hr, "BBModeler::InitInstance()", "Couldn't initialize mesh object." );

	// load bounding box object
	hr = BoundingBox->Init( FALSE, "auto.box", 5 );
	if ( hr ) ERRORMSG( hr, "BBModeler::InitInstance()", "Couldn't initialize bounding box object." );


	Mesh->UsePredefinedMaterial = FALSE;
	Mesh->SetMaterial( &(Scene->WhiteMaterial) );
	Mesh->SetPosition( 0, 0, 0 );
	BoundingBox->SetScaling( 10.0f );
	Scene->StaticObjects.push_back( Mesh );
	Scene->StaticObjects.push_back( BoundingBox );


    Graphic->Scene = Scene;
	
	// insert lights into the scene
	pLight = new graphic::CLight;
	if ( !pLight )
	{
		ErrorHandler.HandleError( ERROUTOFMEMORY, "BBModeler::InitInstance()", "Can't add a light into scene." );
		return FALSE;
	}
	pLight->SetStandardLight( LT_DIRECTIONAL );
	pLight->SetColor( 0.4f, 0.4f, 0.4f, 1.0f );
	pLight->Position = D3DXVECTOR3( 10000, 10000, 0 );
	pLight->Direction = D3DXVECTOR3( -1.0f, -1.0f, 0.0f );
	Scene->Lights.push_back( pLight );

	pLight = new graphic::CLight;
	if ( !pLight )
	{
		ErrorHandler.HandleError( ERROUTOFMEMORY, "BBModeler::InitInstance()", "Can't add a light into scene." );
		return FALSE;
	}
	pLight->SetStandardLight( LT_AMBIENT );
	pLight->SetColor( 0.2f, 0.2f, 0.2f, 1.0f );
	Scene->Lights.push_back( pLight );
  

	// set position and viewing parameters
	Graphic->SetEyePtVector( 0, 0, - TERRAIN_PLATE_WIDTH );
	Graphic->SetLookAtVector( 0, 0, 1 );
	Graphic->SetUpVector( 0, 1, 0 );
	Graphic->SetProjectionParameters( FIELD_OF_VIEW_Y, 1.33f, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE );
	Graphic->SetShowingFPS( TRUE );
	Graphic->SetShowingDebugMessages( TRUE );
	Graphic->SetRenderTerrain( FALSE );
	Graphic->SetRenderLightGlows( FALSE );
	Graphic->SetMapDependentRendering( FALSE );
	Graphic->SetUseDynamicLights( FALSE );
	Graphic->SetShadowComplexity( SC_NONE );
	Graphic->SetMsgFontColor( 0xffffffff );
	Graphic->SetDebugFontColor( 0xffffff00 );

	Camera.SetEyePtVector( 0, 0, 0 );
	Camera.SetLookAtVector( 0, 0, -1 );
	Camera.SetUpVector( 0, 1, 0 );


	ShowWindow( hWind, nCmdShow );
	UpdateWindow( hWind );

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
//  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	int iHeight, iWidth;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch ( message ) 
	{
	case WM_COMMAND:
		wmId    = LOWORD( wParam ); 
		wmEvent = HIWORD( wParam ); 
		// Parse the menu selections:
		switch ( wmId )
		{
		case IDM_ABOUT:
			DialogBox( hInst, (LPCTSTR) IDD_ABOUTBOX, hWnd, (DLGPROC) About );
			break;
		case IDM_EXIT:
			DestroyWindow( hWnd );
			break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint( hWnd, &ps );
		EndPaint( hWnd, &ps );
		break;
	case WM_DESTROY:
		SAFE_DELETE( Scene )
		SAFE_DELETE( Graphic )
		SAFE_DELETE( ResourceManager )
		PostQuitMessage( 0 );
		break;
	case WM_SIZE:
		iWidth = LOWORD( lParam );
		iHeight = HIWORD( lParam );
		Graphic->SetProjectionParameters(	FIELD_OF_VIEW_Y, (float) iWidth / (float) iHeight,
                                            NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE	);
		Graphic->SetScreenSize( iWidth, iHeight );
		break;
	case WM_MOUSEMOVE:
		ProcessMouseMoveMsg( wParam, lParam );
		break;
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Processes the WM_MOUSEMOVE message - checks the pressed system and mouse buttons
// and calls apropriate methods
//
//////////////////////////////////////////////////////////////////////////////////////////////
void ProcessMouseMoveMsg( WPARAM wParam, LPARAM lParam )
{
	static BOOL		wasCalled = FALSE;
	static POINT	lPos;
	POINT			nPos, dPos;

	if ( !wasCalled )
	{
        wasCalled = TRUE;
		GetCursorPos( &lPos );
	}
	
	nPos.x = LOWORD( lParam );
	nPos.y = HIWORD( lParam );
	dPos.x = nPos.x - lPos.x;
	dPos.y = nPos.y - lPos.y;
	lPos = nPos;

	if ( wParam & MK_RBUTTON ) RotateModel( &dPos ); 
	
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Rotates the model on the base of mouse cursor movement
//
//////////////////////////////////////////////////////////////////////////////////////////////
void RotateModel( LPPOINT move )
{
	if ( !Mesh ) return;

	D3DXVECTOR3		rot( 0, 0, 0 );
	D3DXVECTOR3		look, eye, up;
	D3DXMATRIX		mat;

	rot = Mesh->GetRotation();

	rot.x -= RotationStep * (FLOAT) move->y / 1000.0f;
	rot.y -= RotationStep * (FLOAT) move->x / 1000.0f;
/*
	if ( rot.x > 0 ) Camera.RotateCameraM( CR_PITCHUP, rot.x );
	else Camera.RotateCameraM( CR_PITCHDOWN, -rot.x );

	//Camera.GetLookAtVector( &look );
	//Camera.GetEyePtVector( &eye );
	//Camera.GetUpVector( &up );

	D3DXMatrixLookAtLH( &mat, Camera.GetEyePtVector( &eye ), Camera.GetLookAtVector( &look ), Camera.GetUpVector( &up ) );

*/	
// HERE will be Rotation vector computing on the base of eye , look and up vectors

	Mesh->SetRotation( rot );
	BoundingBox->SetRotation( rot );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Message handler for about box.
//
//////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message )
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL ) 
		{
			EndDialog( hDlg, LOWORD( wParam ) );
			return TRUE;
		}
		break;
	}
	return FALSE;
}
