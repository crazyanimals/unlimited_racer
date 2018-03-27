//
// Author:			Roman Margold
// 
// GraphicTest is a program used for testing purposes while developing new graphic engine features.
//
// Program parameters:
//
//   -fname    name specifies the .map file which to load (main game map file)
//   -dname    name specifies the demo file that is used to run in demo mode.
//				Demo mode is not user-controlable and is simulating standard run by processing
//				instructions passed to it by script demo file. For interpreting the script file
//				the ScriptInterpreter module is used. This is useful when creating a benchmark
//				that will test engine speed on different computers. When writing a demo script,
//				it's recommended to use some delay before starting time counting.
//              Demo file is looked for in demos path (set in configuration file). If not found,
//				the actual directory is used.

//#define GROBJECT_USE_OLD_WORLDMAT_STYLE

#include "stdafx.h"
#include "GraphicTest.h"
#include "..\\Client\\CClient.h"

using namespace graphic;



// help dialog structure 
class CMyOwnDialog
{
public:
	CMyOwnDialog() {
			ridBasicFont = -1;
			ridTitleFont = -1;
			pGUIDialog = NULL;
			idBackgroundTexture = -1;
	}
	~CMyOwnDialog() {	
			if ( ridBasicFont > 0 ) ResourceManager->ReleaseResource( ridBasicFont );
			if ( ridTitleFont > 0 ) ResourceManager->ReleaseResource( ridTitleFont );
			if ( idBackgroundTexture > 0 ) ResourceManager->ReleaseResource( idBackgroundTexture ); 
			SAFE_DELETE( pGUIDialog );
	}
	
	CGUIDialog			*	pGUIDialog;
	GUICONTROLID			TestLabel, TestLabel2;
	GUICONTROLID			TestButton1;
	GUICONTROLID			TestCheckBox1, TestCheckBox2;
	GUICONTROLID			TestTrackBar;
	GUICONTROLID			TestRadio1, TestRadio2, TestRadio3;
	GUICONTROLID			TestEdit;
	GUICONTROLID			TestCombo;
	GUICONTROLID			TestMenu;
	GUICONTROLID			TestIList;
	GUICONTROLID			TestImage;
	GUICONTROLDECLARATION	Declaration;
	GUIDIALOGDECLARATION	DialogDeclaration;
	resManNS::RESOURCEID	idBackgroundTexture;
	resManNS::RESOURCEID	ridBasicFont, ridTitleFont;
} ;



// predefined Global Variables:
HWND		hWind;
HINSTANCE	hInst;				// current instance
TCHAR		szWindowClass[100]; // window class name

// own global variables
CTimeCounter		Timer; // time counter
resManNS::CResourceManager *	ResourceManager = NULL;
CGraphic		 *	Graphic = NULL;
CScene			 *	Scene = NULL;
CString				MapFileName;
CString				TerrainFileName, TrackFileName;
CString				MapName;
LPGROBJECTMESH		AutoMesh, TreeMesh, LampMesh, LampMesh2, AutoMesh2, Lamp3, Lamp4;
UINT				uiMaxPlayersCount;
CString				DemoFileName;
bool				DemoMode = false;
CMyOwnDialog	*	pMainDialog = NULL;
bool				bInMenu;
int					PrimaryMCursor;
CClient			*	Client;




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
			MapFileName = text;
			break;
		case 'd':
			sscanf( (LPCTSTR) str, "-d%s", text );
			DemoFileName = text;
			DemoMode = true;
			break;
		}
	}

	GlobalFree( args );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function used as an "ExternalFunctionCall" by ScriptInterpreter
// performs all actions done by demo commands when the program is running in demo mode
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CALLBACK DemoExternalCallback( CString &retVal, int ParamCount, CString params[] )
{
	CVariable			var( "a" );
	MSG					msg;
	D3DXVECTOR3			vec;

/*
	List of commands (functions) used by demo script:

	void render(int);		// renders the scene and dispatches windows messages; this call an amount of times
	int getTime(void);		// returns app run time (in miliseconds)
	void writeI(int);		// same as OUTI
	void writeIN(int);		// same as OUTIN
	void writeS(string);	// same as OUTS
	void writeSN(string);	// same as OUTSN
	void writeF(float);		// same as OUTF
	void writeFN(float);	// same as OUTFN
	void userAction(string)	// performs user action like moving forward etc.
*/

	retVal = "0";

	if ( params[0] == "getTime" )
	{
		if ( ParamCount > 0 ) return ERR_TOOMANYPARAMETERS;

		var.SetValue( (int) Timer.GetRunTime() );
		
		retVal = var.GetString();
	}
	
	
	// render the scene and dispatch messages - just do all those boring things :)
	if ( params[0] == "render" )
	{
		CCamera		cam;
		D3DXVECTOR3	vec;
		
		if ( ParamCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var.SetValue( params[1] );
		if ( var.GetInt() < 0 ) 
		{
			retVal = "-1";
			return ERRNOERROR;
		}
		
		// runs the cycle for an amout of times
		for ( int i = 0; i < var.GetInt(); i++ )
		{
			// dispatch all messages cumulated in the queue
			while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE) ) 
			{
				if ( 0 == GetMessage(&msg, NULL, 0, 0) ) exit( 0 );
				else
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
			}

			Graphic->GetCamera( &cam );
			cam.GetEyePtVector( &vec );
			Graphic->SetMapPosX( (int) (vec.x / TERRAIN_PLATE_WIDTH) );
			Graphic->SetMapPosZ( (int) (vec.z / TERRAIN_PLATE_WIDTH) );
        
			Graphic->RenderScene();
		}
	}


	// performs user action
	if ( params[0] == "userAction" ) 
	{
		if ( ParamCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 1 ) return ERR_TOOMANYPARAMETERS;

		if ( params[1] == "KeyForward" )		Graphic->MoveCamera( CM_FORWARD );
		if ( params[1] == "KeyBackward" )		Graphic->MoveCamera( CM_BACKWARD );
		if ( params[1] == "KeyMoveLeft" )		Graphic->MoveCamera( CM_LEFT );
		if ( params[1] == "KeyMoveRight" )		Graphic->MoveCamera( CM_RIGHT );
		if ( params[1] == "KeyTurnLeft" )		Graphic->RotateCamera( CR_YAWLEFT );
		if ( params[1] == "KeyTurnRight" )		Graphic->RotateCamera( CR_YAWRIGHT );
		if ( params[1] == "KeyTurnUp" )			Graphic->RotateCamera( CR_PITCHUP );
		if ( params[1] == "KeyTurnDown" )		Graphic->RotateCamera( CR_PITCHDOWN );
	}

	
	// debug output functions
	if ( params[0] == "writeI" || params[0] == "writeIN" || params[0] == "writeS" 
		|| params[0] == "writeSN" || params[0] == "writeF" || params[0] == "writeFN" )
	{
	
		if ( ParamCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var.SetValue( params[1] );

		if ( params[0] == "writeI" )			OUTI( var.GetInt(), 2 );
		if ( params[0] == "writeIN" )			OUTIN( var.GetInt(), 2 );
		if ( params[0] == "writeS" )			OUTS( var.GetString(), 2 );
		if ( params[0] == "writeSN" )			OUTSN( var.GetString(), 2 );
		if ( params[0] == "writeF" )			OUTF( var.GetFloat(), 2 );
		if ( params[0] == "writeFN" )			OUTFN( var.GetFloat(), 2 );
	}


	// setting camera params
	if ( params[0] == "setEyePtVec" )
	{
		if ( ParamCount < 3 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 3 ) return ERR_TOOMANYPARAMETERS;
		
		var.SetValue( params[1] );
		vec.x = var.GetFloat();
		var.SetValue( params[2] );
		vec.y = var.GetFloat();
		var.SetValue( params[3] );
		vec.z = var.GetFloat();
		Graphic->SetEyePtVector( vec );
	}

	if ( params[0] == "setLookAtVec" )
	{
		if ( ParamCount < 3 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 3 ) return ERR_TOOMANYPARAMETERS;
		
		var.SetValue( params[1] );
		vec.x = var.GetFloat();
		var.SetValue( params[2] );
		vec.y = var.GetFloat();
		var.SetValue( params[3] );
		vec.z = var.GetFloat();
		Graphic->SetLookAtVector( vec );
	}

	if ( params[0] == "setUpVec" )
	{
		if ( ParamCount < 3 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 3 ) return ERR_TOOMANYPARAMETERS;
		
		var.SetValue( params[1] );
		vec.x = var.GetFloat();
		var.SetValue( params[2] );
		vec.y = var.GetFloat();
		var.SetValue( params[3] );
		vec.z = var.GetFloat();
		Graphic->SetUpVector( vec );
	}


	return ERRNOERROR;	
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
	MSG			msg;
	HACCEL		hAccel;
	HRESULT		hr;
	char		str[512];
	CString		retValue;
	UINT		lineNo;
	APPRUNTIME	uiLastDialogTimerCall = Timer.GetStartTime();
	APPRUNTIME	uiNewTime;


	MyRegisterClass( hInstance );

	// Perform application initialization:
	if ( !InitInstance( hInstance, nCmdShow ) ) 
	{
		ErrorHandler.HandleError( ERRGENERIC, "GraphicTest::_tWinMain()", "Could not initialize basic program structures." );
		return 0;
	}

	hAccel = LoadAccelerators( hInstance, (LPCTSTR) IDC_GRAPHICTEST );


	// runs in DEMO mode
	if ( DemoMode )
	{
		// interpret script; look for the file in two possible directories (demos dir and actual dir)
		hr = InterpretScript( CONFIG_DEMOSPATH + DemoFileName, DemoExternalCallback, retValue );
		if ( hr == ERR_FILENOTFOUND ) hr = InterpretScript( DemoFileName, DemoExternalCallback, retValue );
		switch ( hr )
		{
		case ERR_FILENOTFOUND:
			sprintf( str, "Demo file '%s' not found.", CONFIG_DEMOSPATH + DemoFileName );
			ERRORMSG( ERRGENERIC, "GraphicTest::InitInstance()", str );
		case ERR_WRONGINTERPRETVERSION:
			sprintf( str, "Demo file '%s' not found.", CONFIG_DEMOSPATH + DemoFileName );
			ERRORMSG( ERRGENERIC, "GraphicTest::InitInstance()", "Demo file needs newer version of ScriptInterpreter." );
		}

		
		// check for possible error
		if ( CInterpretingError::PickError( hr, retValue, lineNo ) ) {
			sprintf( str, "An error occured while interpreting the demo script. Line %d: %s\n", lineNo, (LPCTSTR) retValue );
			ERRORMSG( ERRGENERIC, "GraphicTest::_tWinMain()", str );
		}

		// post a message to a queue that we want to close app (and therefore free all objects)
		PostMessage( hWind, WM_DESTROY, 0, 0 );
		
		// stay alive until the quit message is handlel
		while ( true )
		{
			if ( !GetMessage(&msg, NULL, 0, 0) ) return (int) msg.wParam;
			else
			{
				TranslateMessage( &msg ); 
				DispatchMessage( &msg );
			}                                       
		}
	}
	else //DestroyWindow( hWind ); // TODO: zrusit, pouze testovaci ucely
	// Main message loop (not DEMO mode)
	while ( true )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) 
		{
			if ( 0 == GetMessage(&msg, NULL, 0, 0) ) return (int)msg.wParam;
			else if( !TranslateAccelerator( hWind, hAccel, &msg ) )
			{
				TranslateMessage( &msg ); 
				DispatchMessage( &msg );
			}                                       
		}
		else 
		{
			CCamera		cam;
			D3DXVECTOR3	vec;
			
			//AutoMesh->SetRotation( 0, (float) Timer.GetRunTime() / 1500.0f, 0 );
			Graphic->GetCamera( &cam );
			cam.GetEyePtVector( &vec );
			Graphic->SetMapPosX( (int) (vec.x / TERRAIN_PLATE_WIDTH) );
			Graphic->SetMapPosZ( (int) (vec.z / TERRAIN_PLATE_WIDTH) );

			Graphic->RenderScene();
		}

		if ( uiLastDialogTimerCall + 50 <= ( uiNewTime = Timer.GetRunTime() )  )
		{
			PostMessage( hWind, GUIMSG_TIMER, 0, 0 );
		}

		uiLastDialogTimerCall = uiNewTime;
	}

	return (int) msg.wParam;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// registers a window class
//
//////////////////////////////////////////////////////////////////////////////////////////////
ATOM MyRegisterClass( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;

	LoadString( hInstance, IDC_GRAPHICTEST, szWindowClass, 100 );

	wcex.cbSize = sizeof( WNDCLASSEX ); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC) WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon( hInstance, (LPCTSTR) IDI_GRAPHICTEST );
	wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= (HBRUSH) ( COLOR_WINDOW + 1 );
	wcex.lpszMenuName	= 0;//(LPCTSTR) IDC_GRAPHICTEST;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon( wcex.hInstance, (LPCTSTR) IDI_SMALL );

	return RegisterClassEx( &wcex );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Checks whether the configuration data are valid.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CheckConfiguration()
{
	//TODO: This must also check the graphic configuration depending on HW abilities.
	

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function for outputting debug messages to both - graphical and file output
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK GrDebugOutputCallback( CAtlString msg, int level )
{
	if ( level > DebugLevel ) return;
	
	FILE	*fil;
	
	fil = fopen( DEBUGOUTPUTFILENAME, _T("at") );
	fprintf( fil, _T("%s"), (LPCTSTR) msg );
	fclose( fil );

	if ( Graphic ) Graphic->AddDebugMessage( msg );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Returns LODMap file name
//
//////////////////////////////////////////////////////////////////////////////////////////////
CString CreateLODMapFileName( LODMAPTYPE type, int level )
{
	CString csType;
	CString csLevel;

	switch ( type )
	{
	case LTTerrain : csType = "terrain_"; break;
	case LTModel : csType = "model_"; break;
	case LTGrass : csType = "grass_"; break;
	default : csType = "undefined_";
	}

	switch ( level )
	{
	case 0: csLevel = "verylow.lodmap"; break;
	case 1: csLevel = "low.lodmap"; break;
	case 2: csLevel = "medium.lodmap"; break;
	case 3: csLevel = "high.lodmap"; break;
	case 4: csLevel = "veryhigh.lodmap"; break;
	default: csLevel = "undefined.lodmap"; break;
	}

	return csType + csLevel;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// InitInstance makes all initializing steps:
//	1] Inits Error and Debug outputs, ProgramTimer
//	2] Parses program arguments and loads and cheks configuration data
//	3] Creates a window
//	4] Creates a ResourceManager
//	5] Initializes a graphic object and GUI dialog
//  6] Creates a Scene objects, then loads map and all needed data
//	7] Sets viewing parameters
//
// returns true when all of the initialization parts were successful
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	HRESULT		hr, hr1;
	resManNS::RESOURCEID MapID;

	hInst = hInstance; // Store instance handle in our global variable


	// timer initialization
	Timer.Start(); 
	// error handler and debug output initialization
	ErrorHandler.Init( ErrOutputCallBack );
	ErrorHandler.MsgOut( "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" );
    DebugOutput.Init( DebugOutputCallBack );




	// load configuration and parse program arguments
	hr = Configuration.Load( CONFIGFILENAME );
	hr1 = Configuration.Load( KEYCONFIGFILENAME );
	if ( hr || hr1 ) 
	{
		ErrorHandler.HandleError( ERRGENERIC, "GraphicTest::WndProc()", "Configuration data can not be loaded." );
		return FALSE;
	}
	MapFileName = Configuration.MapFileName;
	DebugLevel = Configuration.DebugLevel;

	ParseArgs(); // ! parse program arguments

//TODO: Nejdrive musime znat vsechna dostupna rozliseni a schopnosti HW, abychom mohli kontrolovat konfiguraci.
	hr = CheckConfiguration();
	if ( hr )
	{
		CString		msg = "Check the '";
		msg += CONFIGFILENAME;
		msg += "' configuration file.";
		ErrorHandler.HandleError( ERRINVALIDCONFIGURATION, "GraphicTest::InitInstance()", msg );
		return FALSE;
	}



	// output some debug data...
	OUTMSG( "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n", 1 );
	OUTS( "Application started at ", 1 );
	OUTDATETIME( 1 );
	if ( DemoMode ) { OUTSN( " in DEMO mode." , 1); }
	else { OUTSN( " in STANDARD mode.", 1 ); }
	OUTS( "Configuration information loaded successfully from file '", 2 );
	OUTS( CONFIGFILENAME, 2 );
	OUTSN( "'.", 2 );
	OUTS( "Resolution: ", 1 );
	OUTI( Configuration.WindowWidth, 1 );
	OUTS( " x ", 1 );
	OUTI( Configuration.WindowHeight, 1 );
	OUTS( " x ", 1 );
	OUTI( Configuration.ColorDepth, 1 );
	if ( Configuration.FullScreen ) { OUTSN( ", full-screen.", 1 ); } else { OUTSN( ", windowed.", 1 ); }
	OUTS( "Terrain LOD: ", 2 );
	OUTIN( Configuration.TerrainLODCount, 2 );




	// CREATE WINDOW
	if ( Configuration.FullScreen ) hWind = CreateWindowEx( WS_EX_TOPMOST,
															szWindowClass, _T("Stunts - test application"), 
															WS_POPUP,
															CW_USEDEFAULT, 
															CW_USEDEFAULT,
															Configuration.WindowWidth, 
															Configuration.WindowHeight, 
															NULL, 
															NULL, 
															hInstance, 
															NULL );
	else hWind = CreateWindow(	szWindowClass, 
								_T("Stunts - test application"), 
								WS_OVERLAPPEDWINDOW, 
								CW_USEDEFAULT, 
								0,
								Configuration.WindowWidth, 
								Configuration.WindowHeight,
								NULL, 
								NULL, 
								hInstance, 
								NULL );
	
	if ( !hWind ) return FALSE;
	OUTMSG( "Main window created successfully.", 2 );



	// ResourceManager initialization
	ResourceManager = new resManNS::CResourceManager;
	ResourceManager->SetResourcePath( CONFIG_MODELSPATH, RES_Model );//car model isnt only car model
	ResourceManager->SetResourcePath( CONFIG_MODELSPATH, RES_XFile );
	ResourceManager->SetResourcePath( CONFIG_MODELSPATH, RES_GrassItem );
	ResourceManager->SetResourcePath( CONFIG_MAPSPATH, RES_Terrain );
	ResourceManager->SetResourcePath( CONFIG_TERRAINSPATH, RES_GrSurface );
	ResourceManager->SetResourcePath( CONFIG_TERRAINSPATH, RES_GrPlate );
	ResourceManager->SetResourcePath( CONFIG_TEXTURESPATH, RES_Texture );
	ResourceManager->SetResourcePath( CONFIG_LODDEFINITIONSPATH, RES_LoDMap );
	ResourceManager->SetResourcePath( CONFIG_DIALOGSPATH, RES_DlgDeclaration );
	ResourceManager->SetResourcePath( CONFIG_DIALOGSPATH, RES_DlgDefinition );
	ResourceManager->SetResourcePath( CONFIG_FONTSPATH, RES_FontDef );
	ResourceManager->SetResourcePath( CONFIG_SHADERSPATH, RES_PixelShader );
	ResourceManager->SetResourcePath( CONFIG_SHADERSPATH, RES_VertexShader );
	ResourceManager->SetResourcePath( CONFIG_MAPSPATH, RES_Map );
	ResourceManager->SetResourcePath( CONFIG_PHTEXTURESPATH, RES_PhysicalTexture );
    ResourceManager->SetResourcePath( CONFIG_OBJECTSPATH, RES_EngineObject  );
	ResourceManager->SetResourcePath( CONFIG_CARSPATH, RES_CarObject );
	ResourceManager->SetResourcePath( CONFIG_OBJECTSPATH, RES_MainObject);
	ResourceManager->SetResourcePath( CONFIG_OBJECTSPATH, RES_GameObject);
	ResourceManager->SetResourcePath( CONFIG_LIGHTSPATH, RES_LightObject);
	ResourceManager->SetResourcePath( CONFIG_SKYSYSTEMPATH, RES_SkySystem);
	ResourceManager->SetResourcePath( CONFIG_TEXTURESETPATH, RES_TextureSet );



	
	// CREATE GRAPHIC OBJECT and initialize it, resource manager has to be initialized first !!!
	graphic::GRAPHICINIT		grInit;
	Graphic = new CGraphic();

	grInit.hWnd = hWind;
	grInit.Width = Configuration.WindowWidth;
	grInit.Height = Configuration.WindowHeight;
	grInit.ColorDepth = Configuration.ColorDepth;
	grInit.FullScreen = Configuration.FullScreen;
	grInit.TerrainLOD = CreateLODMapFileName( LTTerrain, Configuration.TerrainComplexity );
	grInit.ModelLOD = CreateLODMapFileName( LTModel, Configuration.ModelComplexity );
	grInit.GrassLOD = CreateLODMapFileName( LTGrass, Configuration.GrassComplexity );
	grInit.pResourceManager = ResourceManager;
	grInit.LightGlows = Configuration.RenderLightGlows;
	grInit.LOD0TerrainTexWidth = Configuration.LOD0TerrainTexWidth;
	grInit.LOD0TerrainTexHeight = Configuration.LOD0TerrainTexHeight;
	grInit.AnisotropicFiltering = Configuration.AnisotropicFiltering;
	grInit.Antialiasing = Configuration.Antialiasing;
	grInit.MultisampleQuality = Configuration.MultisampleQuality;
	grInit.WaitVSync = Configuration.WaitVSync;

	hr = Graphic->Init( &grInit );
	if ( hr )
	{
		ErrorHandler.HandleError( ERRGENERIC, "GraphicTest::InitInstance()", "Graphic device initialization failed." );
		return FALSE;
	}

	// Graphic initialization computed count of LODs for particular object types... save it for future use
	Configuration.ModelLODCount = Graphic->GetModelLODCount();
	Configuration.TerrainLODCount = Graphic->GetTerrainLODCount();
	Configuration.GrassLODCount = Graphic->GetGrassLODCount();

	Graphic->SetDebugMsgParams( 0, 100, Configuration.WindowWidth, Configuration.WindowHeight, DT_TOP | DT_RIGHT );
	Graphic->SetMaxDebugMsgs( 6 );
	DebugOutput.Init( GrDebugOutputCallback );

	OUTMSG( "Graphic device initialization and mode change successfully done.", 1 );


	// Init static variables of GrObjectBase
	CGrObjectBase::StaticInit( Graphic->GetD3DDevice(), 
							   Graphic->GetShader(), 
							   ResourceManager,
							   Graphic->GetInvisibleTexture() );

	Graphic->LoadMouseCursor( CONFIG_CURSORSPATH + "arrow.cur", &PrimaryMCursor );





	
	// INIT GRAPHIC USER INTERFACE
	
	// create new CDialog object
	pMainDialog = new CMyOwnDialog();
	if ( !pMainDialog )
	{
		ErrorHandler.HandleError( ERROUTOFMEMORY, "GraphicTest::InitInstance()", "Unsuccessful custom dialog creation." );
		return FALSE;
	}
	pMainDialog->pGUIDialog = new CGUIDialog();
	if ( !(pMainDialog->pGUIDialog) )
	{
		ErrorHandler.HandleError( ERROUTOFMEMORY, "GraphicTest::InitInstance()", "Unsuccessful GUI dialog creation." );
		return FALSE;
	}


	hr = pMainDialog->pGUIDialog->InitFromFile( CONFIG_DIALOGSPATH + "GraphicTest.dlgDef", hWind, Graphic->GetD3DDevice(), ResourceManager, &Timer );
	if ( hr )
	{
		ErrorHandler.HandleError( hr, "GraphicTest::InitInstance()", "Main dialog object could not be initialized." );
		return FALSE;
	}

	pMainDialog->TestLabel = (pMainDialog->pGUIDialog->GetControl( "TopLabel" ))->GetUID();
	pMainDialog->TestCheckBox1 = (pMainDialog->pGUIDialog->GetControl( "Trans1CheckBox" ))->GetUID();
	pMainDialog->TestCheckBox2 = (pMainDialog->pGUIDialog->GetControl( "Trans2CheckBox" ))->GetUID();
	pMainDialog->TestLabel2 = (pMainDialog->pGUIDialog->GetControl( "TransLabel" ))->GetUID();
	pMainDialog->TestTrackBar = (pMainDialog->pGUIDialog->GetControl( "TrackBar" ))->GetUID();
	pMainDialog->TestRadio1 = (pMainDialog->pGUIDialog->GetControl( "Radio1" ))->GetUID();
	pMainDialog->TestRadio2 = (pMainDialog->pGUIDialog->GetControl( "Radio2" ))->GetUID();
	pMainDialog->TestRadio3 = (pMainDialog->pGUIDialog->GetControl( "Radio3" ))->GetUID();
	pMainDialog->TestButton1 = (pMainDialog->pGUIDialog->GetControl( "CloseButton" ))->GetUID();
	pMainDialog->TestEdit = (pMainDialog->pGUIDialog->GetControl( "EditBox" ))->GetUID();
	pMainDialog->TestCombo = (pMainDialog->pGUIDialog->GetControl( "Combo" ))->GetUID();
	pMainDialog->TestIList = (pMainDialog->pGUIDialog->GetControl( "FileList" ))->GetUID();
	pMainDialog->TestMenu = (pMainDialog->pGUIDialog->GetControl( "Popup" ))->GetUID();
	pMainDialog->TestImage = (pMainDialog->pGUIDialog->GetControl( "Image" ))->GetUID();

	((CGUIControlButton *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestButton1 ))->lpOnChange = OnCloseButtonClick;
	
	((CGUIControlImage * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestImage ))->lpOnMouseClick = OnImageClick;

	((CGUIControlCheckBox * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCheckBox1 ))->lpOnChange = OnCheckBox1Change;
	((CGUIControlCheckBox * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCheckBox2 ))->lpOnChange = OnCheckBox2Change;
	((CGUIControlTrackBar * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestTrackBar ))->lpOnChange = OnTrackBarChange;

	((CGUIControlRadioButton * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestRadio1 ))->lpOnChange = OnRadioChange;

	((CGUIControlRadioButton * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestRadio2 ))->lpOnChange = OnRadioChange;
	((CGUIControlRadioButton * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestRadio3 ))->lpOnChange = OnRadioChange;

	((CGUIControlComboBox * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->AddLine( " Red text " );
	((CGUIControlComboBox * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->AddLine( " Green text " );
	((CGUIControlComboBox * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->AddLine( " Blue text " );
	((CGUIControlComboBox * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->AddLine( " White text " );
	((CGUIControlComboBox * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->SetValue( 3 );
	((CGUIControlComboBox * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->lpOnChange = OnComboChange;

	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Lines.push_back( " Set name invisible " );
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Lines.push_back( " Set name visible " );
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Lines.push_back( " Disable name editing " );
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Lines.push_back( " Enable name editing " );
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Lines.push_back( "  " );
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Lines.push_back( " Close this menu " );
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Lines.push_back( "  " );
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Lines.push_back( " Close dialog " );
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->SetValue( 1 );
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->lpOnChange = OnMenuSelect;
	((CGUIControlPopupMenu * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->lpOnLostFocus = OnMenuLostFocus;

	_finddata_t			FData;
	intptr_t			FInfoHandle;
	CGUIControlIconedList::CLine	listboxLine;

	listboxLine.idIconTexture = pMainDialog->pGUIDialog->GetBackgroundTexture();
	listboxLine.tpIcon.iPosY = 381;
	listboxLine.tpIcon.iWidth = 20;
	listboxLine.tpIcon.iHeight = 20;

	if ( (FInfoHandle = _findfirst( "*", &FData )) != -1 )
	{
		while ( !_findnext( FInfoHandle, &FData ) )
		{
			listboxLine.csText = FData.name;
			if ( FData.attrib & _A_SUBDIR ) listboxLine.tpIcon.iPosX = 450;
			else listboxLine.tpIcon.iPosX = 480;
			((CGUIControlIconedList * ) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestIList ))->Lines.push_back( listboxLine );
		}
	}
	_findclose( FInfoHandle );


	pMainDialog->pGUIDialog->lpOnRButtonDown = OnRButtonClick;
	pMainDialog->pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed

	// pass the proper dialog object pointer to Graphic object
	//Graphic->SetDialog( pMainDialog->pGUIDialog );





	// SCENE INITIALIZING, data loading
	
	// create the CScene object
	Scene = new CScene;
	
	// load and init the scene from .map file
	hr = MapInit( MapFileName, Scene, ResourceManager, 0 );
	if ( hr )
	{
		ErrorHandler.HandleError( ERRGENERIC, "GraphicTest::InitInstance()", "Scene initialization wasn't successfull." );
		return FALSE;
	}

	// Obtain map ID
	MapID = ResourceManager->LoadResource( MapFileName, RES_Map, true, 0 );


	hr = loadCar( Configuration.CarFileName, 0, 0, ResourceManager, NULL, MapID, 0, Scene  );
	if ( hr )
	{
		ErrorHandler.HandleError( ERRGENERIC, "GraphicTest::InitInstance()", "Unable to initialize car object." );
		return FALSE;
	}



	//Scene->SkySystem.SetTimeAcceleration(1600.0f);
	Scene->SkySystem.SetTimeAcceleration( 0 );
	//Scene->SkySystem.SetSkyTime( 0.67f );



	Graphic->Scene = Scene;


	
	// Register lights with shadow mapping
	//Graphic->RegisterLights();
	//Graphic->InitLightMaps();



	// set position and viewing parameters
	FLOAT	halfWidth = (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH / 2;
	FLOAT	halfDepth = (float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH / 2;

	Graphic->SetEyePtVector( halfWidth, 3000, halfDepth - 5 * TERRAIN_PLATE_WIDTH );
	Graphic->SetLookAtVector( halfWidth, 0, halfDepth );
	Graphic->SetUpVector( 0.0f, 1.0f, 0.0f );
	Graphic->SetProjectionParameters( FIELD_OF_VIEW_Y, 1.33f, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE );
	
	Graphic->SetShowingFPS( Configuration.ShowFPS );
	Graphic->SetShowingDebugMessages( Configuration.ShowDebugMsgs );
	Graphic->SetMsgFontSize( Configuration.MsgFontSize );
	Graphic->SetDebugFontSize( Configuration.DebugFontSize );
	Graphic->SetMsgFontName( Configuration.MsgFontName );
	Graphic->SetDebugFontName( Configuration.DebugFontName );
	Graphic->SetMsgFontColor( Configuration.MsgFontColor );
	Graphic->SetDebugFontColor( Configuration.DebugFontColor );
	Graphic->SetRenderAmbientLight( Configuration.RenderAmbientLight );
	Graphic->SetRenderDirectionalLight( Configuration.RenderDirectionalLight );
	Graphic->SetRenderDynamicObjects( Configuration.RenderDynamicObjects );
	Graphic->SetRenderStaticObjects( Configuration.RenderStaticObjects );
	Graphic->SetRenderTerrain( Configuration.RenderTerrain );
	Graphic->SetRenderFarObjects( Configuration.RenderFarObjects);
	Graphic->SetMapDependentRendering( Configuration.MapDependentRendering );
	Graphic->SetUseDynamicLights( Configuration.DynamicLights );
	Graphic->SetRenderLightGlows( Configuration.RenderLightGlows );
	Graphic->SetRenderHUD(false);
	Graphic->SetRenderGrass( Configuration.RenderGrass );
	Graphic->SetRenderSkyBox( Configuration.RenderSky );
	Graphic->SetShadowComplexity( Configuration.ShadowComplexity );
	Graphic->SetWireframe( Configuration.WireFrameMode );
	Graphic->SetRenderBoundingEnvelopes( false );
	Graphic->SetGamma( Configuration.GammaFactor );
	Graphic->SetCarPerPixelLight( Configuration.CarPerPixelLight );


	// Init light maps
	//hr = Graphic->InitLightMaps();
	if ( hr )
	{
		ErrorHandler.HandleError(hr, "GraphicTest::InitInstance()", "An error occurs during lamp light maps initialization.");
		return FALSE;
	}

	hr = Graphic->InitNormalMap();
	if ( hr )
	{
		ErrorHandler.HandleError( hr, "CGraphic::InitInstance()", "Unable to init terrain normal map." );
		return FALSE;
	}

	// perform terrain optimalization
	hr = Graphic->PerformTerrainOptimalization();
	if ( hr )
	{
		ErrorHandler.HandleError(hr, "GraphicTest::InitInstance()", "An error occurs during perfomance of terrain optimalization.");
		return FALSE;
	}


    



	// RUNS IN DEMO MODE
	if ( DemoMode )
	{
		int			majVer, minVer;
		CString		verDesc;
		char		str[512];
		
		// check the DLL version
		GetDLLVersion( majVer, minVer, verDesc );

		sprintf( str, "ScriptInterpreter v.%d.%d %s loaded.", majVer, minVer, (LPCTSTR)verDesc );
		OUTSN( str, 1 );

		if ( SI_MAJOR_VERSION_NEEDED > majVer || SI_MAJOR_VERSION_NEEDED == majVer && SI_MINOR_VERSION_NEEDED > minVer )
		{
			sprintf( str, "Obsolete version of ScriptInterpreter used. For proper interpretation you need v.%d.%d at least.", SI_MAJOR_VERSION_NEEDED, SI_MINOR_VERSION_NEEDED );
			ErrorHandler.HandleError( ERRGENERIC, "GraphicTest::InitInstance()", str );
			return FALSE;
		}
	}

	
	
	bInMenu = false;

	Graphic->Activate();
	Graphic->SetRenderEnabled( TRUE );

	ShowWindow( hWind, nCmdShow );
	UpdateWindow( hWind );

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// just a WndProc - window handling procedure
//
//////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int xPos, yPos, dxPos, dyPos;
	static int i = 0;
	static int iHeight, iWidth;
	static int xWinPos, yWinPos;
	RECT rect;

	
	// app runs in DEMO mode and has its own MainWindowProc
	if ( DemoMode ) return WndProcDemoMode( hWnd, message, wParam, lParam );


	switch ( message ) 
	{
	case WM_CREATE:
		GetWindowRect( hWnd, &rect );
		xWinPos = rect.left;
		yWinPos = rect.top;
		SetCursorPos( xWinPos+iWidth/2, yWinPos+iHeight/2 );
		ShowCursor( FALSE );
		break;
	case WM_COMMAND:
		wmId    = LOWORD( wParam ); 
		wmEvent = HIWORD( wParam ); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox( hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About );
			break;
		case IDM_EXIT:
			DestroyWindow( hWnd );
			break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;
	case WM_CHAR:
		if ( bInMenu )
			if ( pMainDialog->pGUIDialog->WndProc( hWnd, message, wParam, lParam ) ) break;
	case WM_KEYDOWN:
		if ( bInMenu )
			if ( pMainDialog->pGUIDialog->WndProc( hWnd, message, wParam, lParam ) ) break;
		pMainDialog->pGUIDialog->SetPosition( pMainDialog->pGUIDialog->GetPositionX() + 1, pMainDialog->pGUIDialog->GetPositionX() );
		KeyMessages( wParam );
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
		if ( bInMenu ) pMainDialog->pGUIDialog->WndProc( hWnd, message, wParam, lParam );
		break;
	case GUIMSG_RADIOBUTTONCHOOSEN:
	case GUIMSG_COMBOBOXROLLDOWN:
		if ( bInMenu ) pMainDialog->pGUIDialog->WndProc( hWnd, message, wParam, lParam );
		break;
	case WM_MOVE:
		xWinPos = LOWORD( lParam );
		yWinPos = HIWORD( lParam );
		break;
	case WM_MOUSEMOVE:
		xPos = LOWORD( lParam ); 
		yPos = HIWORD( lParam );
    
		if ( bInMenu )
		{
			Graphic->SetMousePositionXY( xPos, yPos );
			pMainDialog->pGUIDialog->WndProc( hWnd, message, wParam, lParam );
		}
		else 
		{
			if ( xPos != iWidth/2 || yPos != iHeight/2)
			{
				dxPos = xPos - iWidth/2;
				dyPos = yPos - iHeight/2;
			
				SetCursorPos( xWinPos+iWidth/2, yWinPos+iHeight/2 );
					
				if ( dxPos > 0 ) Graphic->RotateCameraM( CR_YAWLEFT, float(dxPos/10) );
				if ( dxPos < 0 ) Graphic->RotateCameraM( CR_YAWRIGHT, float(dxPos/10) );
				if ( dyPos < 0 ) Graphic->RotateCameraM( CR_PITCHUP, float(dyPos/10) );
				if ( dyPos > 0 ) Graphic->RotateCameraM( CR_PITCHDOWN, float(dyPos/10) );		
			}
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint( hWnd, &ps );
		EndPaint( hWnd, &ps );
		break;
	case WM_DESTROY:
		DebugOutput.Init( DebugOutputCallBack );
		SAFE_DELETE( Scene )
		SAFE_DELETE( pMainDialog )
		SAFE_DELETE( Graphic )
		OUTMSG( "Main graphic object released.", 1 )
		SAFE_DELETE( ResourceManager )
		OUTMSG( "ResourceManager released.", 1 )
		OUTS( "Going to close application at ", 1 )
		OUTDATETIMEN( 1 );
		OUTS( "Application run time: ", 2 )
		OUTF( (float) Timer.GetRunTime() / 1000, 2 )
		OUTSN( " s", 2 );
		ShowCursor( TRUE );
		PostQuitMessage( 0 );
		break;
	case WM_SIZE:
		iWidth = LOWORD( lParam );
		iHeight = HIWORD( lParam );
		if ( !Configuration.FullScreen || Graphic )
			Graphic->SetProjectionParameters( 
				FIELD_OF_VIEW_Y, 
				(float) LOWORD(lParam) / (float) HIWORD(lParam),
				NEAR_CLIPPING_PLANE,
				FAR_CLIPPING_PLANE
			);
		if ( Graphic ) Graphic->SetScreenSize( iWidth, iHeight ); // Graphic object must already be initialized
		break;
	case GUIMSG_TIMER:
		if ( bInMenu ) pMainDialog->pGUIDialog->WndProc( hWnd, message, wParam, lParam );
		break;
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// main WindowProc used when the application is running in DEMO mode
//
//////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProcDemoMode( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	int			wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC			hdc;
	static int iHeight, iWidth;


	switch ( message ) 
	{
	case WM_COMMAND:
		wmId    = LOWORD( wParam ); 
		wmEvent = HIWORD( wParam ); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox( hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About );
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
		DebugOutput.Init( DebugOutputCallBack );
		SAFE_DELETE( Scene )
		SAFE_DELETE( Graphic )
		SAFE_DELETE( pMainDialog )
		OUTMSG( "Main graphic object released.", 1 )
		SAFE_DELETE( ResourceManager )
		OUTMSG( "ResourceManager released.", 1 )
		OUTS( "Going to close application at ", 1 )
		OUTDATETIMEN( 1 );
		OUTS( "Application run time: ", 2 )
		OUTF( (float) Timer.GetRunTime() / 1000, 2 )
		OUTSN( " s", 2 );
		ShowCursor( TRUE );
		PostQuitMessage( 0 );
		break;
	case WM_SIZE:
		iWidth = LOWORD( lParam );
		iHeight = HIWORD( lParam );
		if ( !Configuration.FullScreen || Graphic )
			Graphic->SetProjectionParameters( 
				FIELD_OF_VIEW_Y, 
				(float) LOWORD(lParam) / (float) HIWORD(lParam),
				NEAR_CLIPPING_PLANE,
				FAR_CLIPPING_PLANE
			);
		if ( Graphic ) Graphic->SetScreenSize( iWidth, iHeight ); // Graphic object must already be initialized
		break;
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message )
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if ( LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL ) 
		{
			EndDialog( hDlg, LOWORD(wParam) );
			return TRUE;
		}
		break;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Process keyboard messages dependently on program configuration
//
//////////////////////////////////////////////////////////////////////////////////////////////
void KeyMessages( WPARAM wParam )
{
	if ( wParam == Configuration.KeyForward )	Graphic->MoveCamera( CM_FORWARD );
	if ( wParam == Configuration.KeyBackward )	Graphic->MoveCamera( CM_BACKWARD );
	if ( wParam == Configuration.KeyMoveLeft )	Graphic->MoveCamera( CM_LEFT );
	if ( wParam == Configuration.KeyMoveRight )	Graphic->MoveCamera( CM_RIGHT );
	if ( wParam == Configuration.KeyTurnLeft )	Graphic->RotateCamera( CR_YAWLEFT );
	if ( wParam == Configuration.KeyTurnRight )	Graphic->RotateCamera( CR_YAWRIGHT );
	if ( wParam == Configuration.KeyTurnUp )	Graphic->RotateCamera( CR_PITCHUP );
	if ( wParam == Configuration.KeyTurnDown )	Graphic->RotateCamera( CR_PITCHDOWN );
	
	if ( wParam == Configuration.KeyMenu )
	{
		if ( bInMenu )
		{
			Graphic->SetDialog( NULL );
			Graphic->SetMouseVisible( false );
			bInMenu = false;
		}
		else
		{
			pMainDialog->pGUIDialog->Reset();
			Graphic->SetDialog( pMainDialog->pGUIDialog );
			Graphic->SetMouseVisible();
			bInMenu = true;
		}
	}

	if ( wParam == VK_SPACE )
	{
		if ( Timer.IsActive() ) Timer.Pause(); 
		else Timer.Continue();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Next functions are processing various dialog events
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CALLBACK OnCloseButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{
	Graphic->SetDialog( NULL );
	Graphic->SetMouseVisible( false );
	bInMenu = false;
	
	return ERRNOERROR;
}

HRESULT CALLBACK OnCheckBox1Change( CGUIControlBase * pObject, LPARAM lParam )
{
	CGUIControlTrackBar	 * pTrack = (CGUIControlTrackBar *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestTrackBar );

	if ( ((CGUIControlCheckBox *) pObject)->IsChecked() ) pMainDialog->pGUIDialog->SetTransparency( pTrack->GetValue() );
	else pMainDialog->pGUIDialog->SetTransparency( 0.0f );

	return ERRNOERROR;
}

HRESULT CALLBACK OnCheckBox2Change( CGUIControlBase * pObject, LPARAM lParam )
{
	if ( ((CGUIControlCheckBox *) pObject)->IsChecked() ) pMainDialog->pGUIDialog->SetControlsTransparency( 0.5f );
	else pMainDialog->pGUIDialog->SetControlsTransparency( 0.0f );

	return ERRNOERROR;
}

HRESULT CALLBACK OnTrackBarChange( CGUIControlBase * pObject, LPARAM lParam )
{
	CGUIControlCheckBox	 * pCheck = (CGUIControlCheckBox *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCheckBox1 );

	if ( pCheck->IsChecked() ) pMainDialog->pGUIDialog->SetTransparency( pObject->GetValueFloat() );
	else pMainDialog->pGUIDialog->SetTransparency( 0.0f );

	return ERRNOERROR;
}

HRESULT CALLBACK OnRButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{
	if ( pObject ) return ERRNOERROR;
	
	((CGUIControlPopupMenu *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->SetVisible();
	((CGUIControlPopupMenu *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Enable();
	((CGUIControlPopupMenu *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->SetValue( 0 );
	((CGUIControlPopupMenu *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->SetPosition( LOWORD( lParam ), HIWORD( lParam) );
	pMainDialog->pGUIDialog->SetFocusTo( pMainDialog->TestMenu );

	return ERRNOERROR;
}																														  

HRESULT CALLBACK OnMenuSelect( graphic::CGUIControlBase * pObject, LPARAM lParam )
{
	CVariable	val;

	val.SetValue( *((CString *) lParam) );

	switch ( val.GetInt() )
	{
	case 0:
		((CGUIControlEdit *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestEdit ))->SetInvisible();
		break;
	case 1:
		((CGUIControlEdit *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestEdit ))->SetVisible();
		break;
	case 2:
		((CGUIControlEdit *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestEdit ))->Disable();
		break;
	case 3:
		((CGUIControlEdit *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestEdit ))->Enable();
		break;
	case 4:
	case 6:
        return ERRNOERROR;
	case 7:
		OnCloseButtonClick( NULL, 0 );
		break;
	}

	OnMenuLostFocus( NULL, 0 );
	pMainDialog->pGUIDialog->SetFocusTo( -1 );

	return ERRNOERROR;
}

HRESULT CALLBACK OnMenuLostFocus( graphic::CGUIControlBase * pObject, LPARAM lParam )
{
	((CGUIControlPopupMenu *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->SetInvisible();
	((CGUIControlPopupMenu *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestMenu ))->Disable();
	
	return ERRNOERROR;
}

HRESULT CALLBACK OnRadioChange( graphic::CGUIControlBase * pObject, LPARAM lParam )
{
	CGUIControlRadioButton * pRadio = (CGUIControlRadioButton *) pObject;

	if ( pRadio->GetUID() == pMainDialog->TestRadio1 ) 
		((CGUIControlListBox *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestIList ))->SetLinesCount( 3 );
	if ( pRadio->GetUID() == pMainDialog->TestRadio2 ) 
		((CGUIControlListBox *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestIList ))->SetLinesCount( 4 );
	if ( pRadio->GetUID() == pMainDialog->TestRadio3 ) 
		((CGUIControlListBox *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestIList ))->SetLinesCount( 5 );

	return ERRNOERROR;
}

HRESULT CALLBACK OnComboChange( graphic::CGUIControlBase * pObject, LPARAM lParam )
{
	CVariable	val;

	val.SetValue( *((CString *) lParam) );

	switch ( val.GetInt() )
	{
	case 0:
		((CGUIControlComboBox *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->SetBasicTextColor( 0xffff0000 );
		break;
	case 1:
		((CGUIControlComboBox *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->SetBasicTextColor( 0xff00ff00 );
		break;
	case 2:
		((CGUIControlComboBox *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->SetBasicTextColor( 0xff0000ff );
		break;
	case 3:
		((CGUIControlComboBox *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestCombo ))->SetBasicTextColor( 0xffd0d0d0 );
		break;
	}

	return ERRNOERROR;
}

HRESULT CALLBACK OnImageClick( graphic::CGUIControlBase * pObject, LPARAM lParam )
{
	static char	chr = 0;
	CString		str;
	
	if ( chr == 3 ) chr = 0;
	else chr++;
	
	switch ( chr )
	{
	case 0:
	default: str = "terrain\\grass_w.jpg"; break;
	}
	
	((CGUIControlImage *) pMainDialog->pGUIDialog->GetControl( pMainDialog->TestImage ))->LoadImage( str, NULL );
	
	
	return ERRNOERROR;
}
