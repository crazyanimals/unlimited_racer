#include "stdafx.h"
#include "initFunctions.h"


using namespace graphic;



HRESULT CheckConfiguration()
{
	//TODO: This must also check the graphic configuration depending on HW abilities.
	//TODO: when is st bad set default settings
	

	return ERRNOERROR;
};


HRESULT initialization()
{
    HRESULT hr,hr1;
	//initialization Error handler
	ErrorHandler.Init( ErrOutputCallBack );
	//ErrorHandler.MsgOut( "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" );

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); //for direct play
	if (hr) ERRORMSG(hr,"initFunctions::initialization()","new Error");

	SAFE_NEW(Graphic,CGraphic(),"initFunctions::initialization()")
	SAFE_NEW(Scene,CScene(),"initFunctions::initialization()")
	SAFE_NEW(ResourceManager,resManNS::CResourceManager(),"initFunctions::initialization()")
	SAFE_NEW(Physics,physics::CPhysics(),"initFunctions::initialization()")
	SAFE_NEW(Server,CServer(),"initFunctions::initialization()")
	SAFE_NEW(Client,CClient(),"initFunctions::initialization()")
	
    DebugOutput.Init( DebugOutputCallBack );
	
	InitializeCriticalSection(&g_csServerList);
	
	if( ! (Graphic && Scene && ResourceManager && Physics && Client && Server))
		ERRORMSG(ERROUTOFMEMORY,"initFunctions::initialization()","new Error");

	Timer.Start(); //timer initialization

	hr = Configuration.Load( CONFIGFILENAME );
	hr1 = Configuration.Load( KEYCONFIGFILENAME );
	if ( hr || hr1 ) 
	{
		ERRORMSG( ERRGENERIC, "initFunctions::initialization()", "Configuration data can not be loaded." );
	}

	if( CheckConfiguration() ) ERRORMSG(ERRINVALIDCONFIGURATION,"initFunctions::initialization()","Bad configuration");

	DebugLevel = Configuration.DebugLevel;

	
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
	ResourceManager->SetResourcePath( CONFIG_OBJECTSPATH, RES_MainObject );
	ResourceManager->SetResourcePath( CONFIG_OBJECTSPATH, RES_GameObject );
	ResourceManager->SetResourcePath( CONFIG_LIGHTSPATH, RES_LightObject );
	ResourceManager->SetResourcePath( CONFIG_SKYSYSTEMPATH, RES_SkySystem );
	ResourceManager->SetResourcePath( CONFIG_TEXTURESETPATH, RES_TextureSet );
	ResourceManager->SetResourcePath( CONFIG_OBJECTSPATH, RES_Waypoints );
	ResourceManager->SetResourcePath( CONFIG_SOUNDSPATH, RES_WaveBank );
	ResourceManager->SetResourcePath( CONFIG_SOUNDSPATH, RES_SoundBank );
	
	//OUT messages
	OUTMSG( "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n", 1 );
	OUTS( "Application started at ", 1 );
	OUTDATETIMEN( 1 );
	OUTS( "Configuration information loaded successfully from file '", 2 );
	OUTS( CONFIGFILENAME, 2 );
	OUTSN( "'.", 2 );


	
	hr = Physics->Init(ResourceManager);
	if(FAILED(hr))
		ERRORMSG(hr, "initFunctions::initialization()", "Physics initialization failed");


	// Init game info
	hr = GameInfo.Init( &Configuration, ResourceManager );
	if (FAILED(hr) )
		ERRORMSG(hr, "initFunctions::initialization()", "Unable to init game info");


	//if(gInput.init(hInst)) //Direct input initialization
	//	ERRORMSG(ERRGENERIC,"gInput.init()","Graphic initialization failed");


	// initialize the sound engine
	hr = InitSoundEngine();
	if ( hr ) ERRORMSG( hr, "initFunctions::initialization()", "Unable to initialize sound engine." );


	return ERRNOERROR;
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


HRESULT initGraphic(HINSTANCE hInstance,int nCmdShow){
	CLight					Light;
	HRESULT					hr = S_OK;
	graphic::GRAPHICINIT	grInit;

	
	Graphic->Scene = Scene;

	grInit.hWnd = hWnd;
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
	if ( hr ) ERRORMSG( ERRGENERIC, "initFunctions::initGraphic()", "Graphic device initialization failed." );

	CGrObjectBase::StaticInit(Graphic->GetD3DDevice(), Graphic->GetShader(), ResourceManager, Graphic->GetInvisibleTexture() );

	OUTMSG( "Graphic device initialization and mode change successfully done.", 1 );
	
	// Graphic initialization computed count of LODs for particular object types... save it for future use
	Configuration.ModelLODCount = Graphic->GetModelLODCount();
	Configuration.TerrainLODCount = Graphic->GetTerrainLODCount();
	Configuration.GrassLODCount = Graphic->GetGrassLODCount();


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
	Graphic->SetRenderLightGlows( Configuration.RenderLightGlows);
	Graphic->SetShadowComplexity( Configuration.ShadowComplexity );
	Graphic->SetRenderHUD(Configuration.RenderHUD);
	Graphic->SetRenderGrass( Configuration.RenderGrass );
	Graphic->SetRenderSkyBox( Configuration.RenderSky );
	Graphic->SetWireframe( Configuration.WireFrameMode );
	Graphic->SetRenderBoundingEnvelopes( Configuration.RenderBoundingEnvelopes );
	Graphic->SetGamma( Configuration.GammaFactor );
	Graphic->SetCarPerPixelLight( Configuration.CarPerPixelLight );

	
	//hr = Graphic->InitLightMaps();

	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );
	
	ShowCursor( FALSE );
	Graphic->SetMouseVisible();
	Graphic->SetRenderEnabled(TRUE);

	return hr;
};


HRESULT initDialogs(){
	// INIT GRAPHIC USER INTERFACE
	HRESULT hr = ERRNOERROR;
	// create new CDialog object
	pDialogs = new CMyDialogs();
	if ( !pDialogs ) ERRORMSG( ERROUTOFMEMORY, "initFunctions::initDialogs()", "Unsuccessful GUI dialog creation." );
	hr = pDialogs->init(Graphic,ResourceManager,&Timer,hWnd);
	if (hr) ERRORMSG(hr, "initFunctions::initDialogs()", "Dialogs could not be initialized." );
	pDialogs->RecenterDialogs();

	hr = Graphic->LoadMouseCursor( CONFIG_CURSORSPATH + "arrow.cur", &(pDialogs->PrimaryMCursor) );

	return hr;
};


HRESULT startSinglePlayer()
{
	HRESULT				hr;
	CGameMod::INT_LIST	keysList;


	hr = Client->initHUD();
	if ( hr ) ERRORMSG(ERRGENERIC, "initFunctions::startMultiPlayerGame()","Unable to initialize HUD.");


	CHANGE_ACTDIALOG(pWaitDialog);
	Graphic->SetMouseVisible( false );

	Graphic->RenderDialogOnly();
	gameState = GS_PLAY;

	// Load server
	if (Server->StartServerSingle())
	{
		ERRORMSG(ERRGENERIC,"initFunctions::startSinglePlayer()","Server start failed");
	}
	
	// Load client
	if( Client->ClientStartSingleLoad() )
	{
		ERRORMSG(ERRGENERIC,"initFunctions::startLocalPlayer()","Client start failed");
	}
	
	OUTMSG("Client was connected.",1);
	CHANGE_ACTDIALOG(pInGameDialog);
	pDialogs->hideDialog();


	// perform game MOD custom actions
	hr = pGameMod->SPStart();
	if ( hr ) ERRORMSG(hr, "initFunctions::startSinglePlayer()", "Unable to start game MOD.");

	
	// reset key mapping so it could be remapped by game MOD change
	keysList.clear();
	//for ( UINT i = 0; i < ModConfig.uiCustomKeysCount; i++ ) keysList.push_back( i );
	pGameMod->GetCustomKeys( &keysList );

	hr = gInput.RefreshActionMapping( &Configuration, &keysList );
	if ( hr ) ERRORMSG( ERRGENERIC, "initFunctions::startSinglePlayer()", "Unable to perform key binding." );
	

	return ERRNOERROR;
};

HRESULT startMultiServer( CString name,BOOL dedicated, CString MapFileName, CGameMod * pGameMod )
{
	CAtlString jmeno=_T("127.0.0.1");	
	
	if( Server->StartServerMulti(name, MapFileName, pGameMod) )
		ERRORMSG(ERRGENERIC,"initFunctions::startMultiServer()","Server start failed");
	
	if ( Server->StartBroadcast())
		ERRORMSG(ERRGENERIC,"initFunctions::startMultiServer()","Brodcast start failed");

	if (!dedicated){
		if ( Client->ConnectToServer(jmeno, Configuration.netPort) )
		ERRORMSG(ERRGENERIC,"initFunctions::startMultiServer()","Client start failed");
	}

	return ERRNOERROR;
};

HRESULT startMultiPlayerGame(){
	HRESULT				Result;
	CGameMod::INT_LIST	keysList;


	Result = Client->initHUD();
	if ( Result ) ERRORMSG(ERRGENERIC, "initFunctions::startMultiPlayerGame()","Unable to initialize HUD.");

	Result = Server->StartServerMultiLoad();
	if (FAILED(Result) ) 
		ERRORMSG(Result, "initFunctions::startMultiPlayerGame()","Unable to start server.");

	
	// reset key mapping as it could be remapped by game MOD change
	keysList.clear();
	//for ( UINT i = 0; i < ModConfig.uiCustomKeysCount; i++ ) keysList.push_back( i );
	pGameMod->GetCustomKeys( &keysList );

	Result = gInput.RefreshActionMapping( &Configuration, &keysList );
	if ( Result ) ERRORMSG( ERRGENERIC, "initFunctions::startMultiPlayerGame()", "Unable to perform key binding." );


	return ERRNOERROR;
};


HRESULT stopMultiServer(){
	//TODO: TO DO
	Server->StopBroadcast();
	return ERRNOERROR;

};

HRESULT stopServer()
{
	//Server->StopPing();
	//Server->StopSession();
	//Server->StopBroadcast();
	Server->ChangeServerState(0);

	return ERRNOERROR;
};

HRESULT startGame(){

	pDialogs->pActDialog = pDialogs->pMainDialog;
	pDialogs->showDialog();
	return ERRNOERROR;

};

HRESULT freeAll(){
	
	HRESULT hr=ERRNOERROR;
	
	Client->ChangeState(NULL);
	Server->ChangeServerState(NULL);
	DebugOutput.Init( DebugOutputCallBack );
	ReleaseSoundEngine();
	SAFE_DELETE(pDialogs);
	SAFE_DELETE(Physics);
	SAFE_DELETE(Server);
	SAFE_DELETE(Client);
	SAFE_DELETE(Graphic);
	SAFE_DELETE(Scene);
	SAFE_DELETE(ResourceManager);
	DeleteCriticalSection(&g_csServerList);
		
	return hr;
};

// Frees game play resources - map and cars (mainly)
HRESULT FreeGamePlayResources(BOOL OnlyToLobby)
{

	if (!OnlyToLobby)
	{
		// Disconnect client from server
		Client->ChangeState(CS_NULL);
	}
	else
	{
		Client->ChangeState(CS_MG_START);
	}

	
	// if server is running on this computer
	//if (Server->GetServerState() == SS_GAME_PLAY)
	//{
		// Stop server
		//Server->StopSession();
		if (!OnlyToLobby)
		{
			Server->ChangeServerState(0);
		}
		else
		{
			Server->ChangeServerState(SS_MG_START);
		}
		
		// Free physics
		Physics->Clean();
	//}

	gameState = GS_START;

	// Free scene
	Graphic->Scene->Free();
	// Free graphics
	Graphic->Free();

	// Stop playing music
	Client->Sound.PlayMusic(FALSE);

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////
//
// InitSoundEngine creates all needed XACT objects and initializes them.
// Use params to define which XACT library should be used.
//
//////////////////////////////////////////////////////////////////////////
HRESULT InitSoundEngine( bool bDebugMode, bool bAuditionMode )
{
    HRESULT		hr;
    DWORD		dwCreationFlags = 0;


    // initialize COM and set the apartment to multithreaded mode
	hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );  // COINIT_APARTMENTTHREADED will work too
    
	if ( FAILED(hr) ) ERRORMSG( hr, "initFunctions::InitSoundEngine()", "COM initialization failed." );



	// set initialization flags
    if( bAuditionMode ) dwCreationFlags |= XACT_FLAG_API_AUDITION_MODE;
    if( bDebugMode ) dwCreationFlags |= XACT_FLAG_API_DEBUG_MODE;

    // init the main XACT engine object
	hr = XACTCreateEngine( dwCreationFlags, &pSoundEngine );
    
	if ( FAILED(hr) || !pSoundEngine ) 
		ERRORMSG( hr, "initFunctions::InitSoundEngine()", "XACT engine creation failed." );

    
	// Initialize & create the XACT runtime 
    XACT_RUNTIME_PARAMETERS xrParams = {0};
    xrParams.lookAheadTime = 250;
	xrParams.fnNotificationCallback = XACTNotificationCallback;
    
	hr = pSoundEngine->Initialize( &xrParams );
    
	if ( FAILED(hr) ) ERRORMSG( hr, "initFunctions::InitSoundEngine()", "XACT engine initialization failed." );



	// load the default common sound files
	CommonSounds.ridPrimaryWaveBank = ResourceManager->LoadResource( "Car.xwb", RES_WaveBank, false, pSoundEngine );
	CommonSounds.ridPrimarySoundBank = ResourceManager->LoadResource( "Car.xsb", RES_SoundBank, false, pSoundEngine );
	if ( CommonSounds.ridPrimarySoundBank < 0 || CommonSounds.ridPrimaryWaveBank < 0 )
		ERRORMSG( ERRNOTFOUND, "initFunctions::InitSoundEngine()", "Could not load common sound resources." );

	resManNS::SoundBank	*	SndBank = ResourceManager->GetSoundBank( CommonSounds.ridPrimarySoundBank );
	resManNS::WaveBank	*	WvBank = ResourceManager->GetWaveBank( CommonSounds.ridPrimaryWaveBank );
	if ( !SndBank || !WvBank ) ERRORMSG( ERRNOTFOUND, "initFunctions::InitSoundEngine()", "Could not obtain common sound resources." );
	CommonSounds.pPrimarySoundBank = SndBank->pSoundBank;
	CommonSounds.pPrimaryWaveBank = WvBank->pWaveBank;


	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////
//
// ReleaseSoundEngine frees the XACT object.
//
//////////////////////////////////////////////////////////////////////////
void ReleaseSoundEngine()
{
	if ( CommonSounds.ridPrimarySoundBank >= 0 ) 
		ResourceManager->ReleaseResource( CommonSounds.ridPrimarySoundBank );
	if ( CommonSounds.ridPrimaryWaveBank >= 0 ) 
		ResourceManager->ReleaseResource( CommonSounds.ridPrimaryWaveBank );

	if ( pSoundEngine )
	{ 
		pSoundEngine->ShutDown();
		pSoundEngine->Release();
	}

    CoUninitialize();
};