#include "stdafx.h"
#include "main.h"
#include "..\Globals\Base64Coder.h"	



// global variables
CServer				*	Server = NULL;
CClient				*	Client = NULL;
CGameInfo				GameInfo;
CGameInput				gInput;
CTimeCounter			Timer; //time counter
resManNS::CResourceManager	* ResourceManager = NULL;
graphic::CGraphic	*	Graphic = NULL;
graphic::CScene		*	Scene = NULL;
physics::CPhysics	*	Physics = NULL;
CMyDialogs			*	pDialogs = NULL;
CRITICAL_SECTION		g_csServerList;
int						gameState;
CMODWorkshop			GameModsWrk; // object used to access game modifications
CGameMod			*	pGameMod = NULL; // pointer to the choosen game MOD object
MODCONFIG				ModConfig; // configuration of the choosen game MOD
IXACTEngine			*	pSoundEngine = NULL;
COMMON_SOUNDS			CommonSounds = { -1, -1, 0 };


TCHAR		szWindowClass[255];
TCHAR		szApplicationName[256];
HWND		hWnd;
HINSTANCE	hInst;



//////////////////////////////////////////////////////////////////////////
//
// Changes resolution
//
//////////////////////////////////////////////////////////////////////////
HRESULT ChangeResolution( graphic::GRAPHICINIT * grInit )
{
	HRESULT hr;
	
	// Release resources
	pDialogs->OnLostDevice();
	Graphic->OnLostDevice();
	Client->HUD.OnLostDevice();
	
	// Change resolution
	hr = pDialogs->pGraphic->ChangeResolution( grInit );
	if (FAILED(hr) ) ERRORMSG(hr, "OnGSOkClick", "Unable to change resolution");
	
	// Reallocate resources
	pDialogs->OnResetDevice();
	hr = Graphic->OnResetDevice();
	if (FAILED(hr) ) ERRORMSG(hr, "OnChangeResolution()", "Unable to reset graphic device.");
	Client->HUD.OnResetDevice();

	// Recenter dialogs
	pDialogs->RecenterDialogs();
	
	return 0;
};

//////////////////////////////////////////////////////////////////////////
//
// Validates, whether graphic device was lost
// and maintains runtine connected with it
//
//////////////////////////////////////////////////////////////////////////
HRESULT ValidateLostDevice()
{
	HRESULT hr;
	static BOOL DeviceLost = FALSE; // Used to call OnLostDevice only once, when device is lost

	// Validate, whether device is lost or not
	hr =  Graphic->ValidateLostDevice();
	if ( hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICENOTRESET)
	{
		if (!DeviceLost)
		{
			// Disable rendering since graphic device is lost
			Graphic->SetRenderEnabled(FALSE);
			// Call OnLostDevice
			Graphic->OnLostDevice();
			pDialogs->OnLostDevice();
			Client->HUD.OnLostDevice();

			// pause timer
			Timer.Pause();
			Client->Timer.Pause();
		}

		DeviceLost = TRUE;

		// try to reset device
		if (hr == D3DERR_DEVICENOTRESET)
		{
			hr = Graphic->ResetDevice();
			if (hr == D3DERR_DEVICELOST) return 0;
			if (FAILED(hr) ) ERRORMSG(hr, "ValidateLostDevice", "Unable to reset graphic device");
		}

		if (DeviceLost && hr == D3D_OK)
		{
			// call after reset device functions
			Graphic->OnResetDevice();
			pDialogs->OnResetDevice();
			Client->HUD.OnResetDevice();

			// Enable rendering since graphic device is reset and ready
			Graphic->SetRenderEnabled(TRUE);

			// resume timer
			Timer.Continue();
			Client->Timer.Continue();
			
			DeviceLost = FALSE;
		}

	}
	else if ( FAILED(hr) ) ERRORMSG( hr, "ValidateLostDevice", "Unable to recover from lost device" );

	
	return 0;	
};


LRESULT CALLBACK WndProc(HWND hWind, UINT msg, WPARAM wParam, LPARAM lParam)
{  
	
	int xPos, yPos;
	switch (msg)
    {
		case WM_WSAASYNCC:
			return Client->ClientMessageHandler(wParam, lParam);
			
			
		case WM_WSAASYNCS:
			return Server->ServerMessageHandler(wParam, lParam);

		case WM_CREATE:
			
			SetTimer( hWind, TIMER_CONTROLS, 1000/30, NULL );
			SetTimer( hWind,  TIMER_PING, 2000,NULL );
			return DefWindowProc(hWind, msg, wParam, lParam);

		case WM_DESTROY:
			
			PostQuitMessage (0) ;
			break ;
		
		
		case WM_KEYDOWN:
			if (Client && Client->GetClientState() == CS_GAME_PLAY)
			{
				if( wParam == VK_ESCAPE ){
					if ( pDialogs->visible ) pDialogs->hideDialog();
					else 
					{
						CHANGE_ACTDIALOG(pInGameDialog);
						pDialogs->showDialog();
					}
				}
			}
			/*{	PostQuitMessage (0) ;
					// Exit program. Let the dialog handle this
					break;
				}*/
			
		//	if (wParam == VK_F1 )
		//	{
				
		//	}

			// Capture screen shot on print screen key
			if (wParam == VK_F2)
			{
				CaptureScreenShot();
			}
			
			break;
		
     
	
	}
	/////////////////////////////////////
	//CONTROLS
	//////////////////////////////////////
	// IN DIALOG
	if(pDialogs && pDialogs->visible){
		pDialogs->pActDialog->pGUIDialog->WndProc( hWnd, msg, wParam, lParam );
		
		switch(msg){
			case WM_MOUSEMOVE:
				xPos = LOWORD( lParam ); 
				yPos = HIWORD( lParam );
				Graphic->SetMousePositionXY( xPos, yPos );
				break;
		}
		
	}
	/////////////////////////////////////
	// IN GAME
	if( (!pDialogs || !pDialogs->visible || pDialogs->allowkeystogame) && pGameMod && pGameMod->ClientControlsEnabled() )
	{
		if( Client && ((Client->GetClientState() & CS_GROUP_MASK) == CS_GAME_PLAY) && GetFocus() )
			switch(msg){
		
				case WM_TIMER:
					 switch (wParam) 
					{ 
				        case  TIMER_CONTROLS: 
							
							RECT rect;		
							GetWindowRect( hWnd, &rect );
			
							SetCursorPos(  rect.left/2 + rect.right/2, rect.top/2+rect.bottom/2 );
							gInput.CheckInput();
							if (Client->cameraType == CAM_FREE)
							gInput.React(Graphic);
							
							Client->SendControls(gInput.KeysToSend,sizeof(gInput.KeysToSend));
							// Test whether client is connected to server
							if (!Client->isConnected() )
							{
								FreeGamePlayResources();
								pDialogs->showDialog();
								SHOWMESSAGEBOX("NOT CONNECTED TO SERVER", "You are no longer connected to server.", pDialogs->pMainDialog);
							}

							if ( pGameMod->ServerGameFinished() ) {
								Client->ChangeState( CS_RACE_RESULT );
								if ( pGameMod->ClientGameOver() ) {
									pDialogs->showDialog();
									CHANGE_ACTDIALOG(pRaceResultDialog);
								}
							}
/*
//RACER MOD ONLY:
							// Test whether it is an end of the race
							if (Client->MultiPlayerInfo.GetLapMode() )
							{
								int ID;
								int LapsGone;
								ID = Client->MultiPlayerInfo.GetID();
								Client->MultiPlayerInfo.GetLapsGone(ID, LapsGone);
								if (LapsGone >= Client->MultiPlayerInfo.GetNumberOfLaps() )
								{
									pDialogs->showDialog();
									CHANGE_ACTDIALOG(pRaceResultDialog);
									Client->ChangeState(CS_RACE_RESULT);

								}
							}
*/
							// Test whether server requested clients to return to lobby
							if (Client->MultiPlayerInfo.GetServerReturnedToLobby() )
							{
								FreeGamePlayResources(TRUE);
								pDialogs->showDialog();
								SHOWMESSAGEBOX("SERVER RETURNED TO LOBBY", "Game is over, server returned to lobby.", pDialogs->pMultiStartGameDialog);
							}

							break;
					} 
					
	
		}
	};
		


	return DefWindowProc(hWind, msg, wParam, lParam);

};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// WndRegisterClass registers the window class and creates a window.
//
//////////////////////////////////////////////////////////////////////////////////////////////
ATOM WndRegisterClass( HINSTANCE hInstance )
{
	
	LoadString( hInstance, IDS_WINDOWS_CLASS, szWindowClass, 256 );
	WNDCLASSEX wcex;
    
	wcex.cbSize = sizeof( WNDCLASSEX ); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC) WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= (HBRUSH) ( COLOR_WINDOW + 1 );
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	=  szWindowClass;
	wcex.hIconSm		= NULL;

	RegisterClassEx( &wcex );


	//Graphic Messages
	OUTS( "Resolution: ", 1 );
	OUTI( Configuration.WindowWidth, 1 );
	OUTS( " x ", 1 );
	OUTI( Configuration.WindowHeight, 1 );
	OUTS( " x ", 1 );
	OUTI( Configuration.ColorDepth, 1 );
	if ( Configuration.FullScreen ) { OUTSN( ", full-screen.", 1 ); } else { OUTSN( ", windowed.", 1 ); }

	LoadString( hInstance, IDS_APPNAME, szApplicationName, 256 );
	//////////////////////////////
	// create window

	if ( Configuration.FullScreen ) hWnd = CreateWindowEx( WS_EX_TOPMOST,
															szWindowClass, szApplicationName, 
															WS_POPUP,
															CW_USEDEFAULT, 
															CW_USEDEFAULT,
															Configuration.WindowWidth, 
															Configuration.WindowHeight, 
															NULL, 
															NULL, 
															hInstance, 
															NULL );
	else hWnd = CreateWindow(	szWindowClass, 
								szApplicationName, 
								WS_OVERLAPPEDWINDOW, 
								CW_USEDEFAULT, 
								0,
								Configuration.WindowWidth, 
								Configuration.WindowHeight,
								NULL, 
								NULL, 
								hInstance, 
								NULL );
	
	
	if ( !hWnd ) 
	{ 
		ErrorHandler.HandleError( ERRGENERIC, "main::WndRegisterClass()", "Unbale to create main program window." );  
		return FALSE;
	}
	

	return TRUE;
}



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
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// windows MAIN function
//
//////////////////////////////////////////////////////////////////////////////////////////////
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
#ifdef MEMLEAK_DEBUG
	InitAllocCheck(ACOutput_XML, TRUE, 0);
	{ // NEMAZAT
#endif

	MSG							msg;
	HRESULT						hr;
	graphic::GROBJECTS_ITER		listik;
	CGameMod::INITSTRUC			initStruc;

	
	gameState	= GS_START;
	hInst = hInstance;

	
	Timer.Start();
	InitCommonControls();
		

	if (initialization()) 	// Configuration load, Server,client,timer,errors, ressource manager, AI initialization 
		ERRORMSG(ERRGENERIC,"main::_tWinMain()","Initialization failed");
	
	if (WndRegisterClass( hInstance ) == FALSE) //register window class
		ERRORMSG(ERRGENERIC,"initFunctions::initialization()","Program window creation failed.");
	OUTMSG( "Main window created successfully.", 2 );


	// Client & Server initialization
	if( Server->init(Physics,hWnd, ResourceManager, &GameInfo, &GameModsWrk, CWaitDialog::LoadingProgress) ) ERRORMSG(ERRGENERIC,"initFunctions::initGraphic()","Server initialization failed");
	if( Client->init(Graphic,Scene,ResourceManager,hWnd, &GameInfo, &GameModsWrk, CWaitDialog::LoadingProgress) ) ERRORMSG(ERRGENERIC,"initFunctions::initGraphic()","Client initialization failed");

	// Set music play list into sound system
	Client->Sound.SetMusicList(GameInfo.MusicNames);


	// initialize list of available game MODs
	ZeroMemory( &initStruc, sizeof(initStruc) );
	initStruc.pResourceManager = ResourceManager;
	initStruc.pPhysics = Physics;
	initStruc.pCommonSounds = &CommonSounds;
	initStruc.pScene = Scene;
	initStruc.pSoundEngine = pSoundEngine;
	initStruc.pNetClient = Client->GetNetClient();
	initStruc.pNetServer = Server->GetNetServer();

	GameModsWrk.GetModsList( "Mods", &initStruc );
	for ( int i = 0; i < GameModsWrk.GetModCount(); i++ )
	{
		GameModsWrk[i]->SetServerGameInfoObject( &Server->MultiPlayerInfo );
		GameModsWrk[i]->SetClientGameInfoObject( &Client->MultiPlayerInfo );
	}
	pGameMod = NULL;


	//Direct input initialization
	gInput.hWnd = hWnd;
	if ( gInput.init(hInstance) ) ERRORMSG(ERRGENERIC,"initFunctions::initGraphic()","DirectInput initialization failed"); 

	// Set key mapping as it is in our configuration
	hr = gInput.RefreshActionMapping( &Configuration, NULL );
	if ( FAILED(hr) ) ERRORMSG(hr, "initFunctions::initGraphic()", "Unable to set our key configuration into direct input");
	

	if (initGraphic(hInstance,nCmdShow)) 		// Graphic initialization
		ERRORMSG(ERRGENERIC,"main::_tWinMain()","Graphic initialization failed");

	Physics->Debug_Init(Graphic->GetBoundingEnvelopesObj(), Graphic->GetBoundingEnvelopesObj()->Start, Graphic->GetBoundingEnvelopesObj()->Finish, Graphic->GetBoundingEnvelopesObj()->ProcessFace);
	
	if (initDialogs())
		ERRORMSG(ERRGENERIC,"main::_tWinMain()","Dialogs initialization failed");

	//CommonSounds.pPrimarySoundBank->Play( XACT_SOUNDBANK_CAR_CARHORNRANDOM, 0, 0, NULL );



	// debug for physics
	Graphic->SetDebugMsgParams( 0, 100, Configuration.WindowWidth, Configuration.WindowHeight, DT_TOP | DT_RIGHT );
	Graphic->SetMaxDebugMsgs( 4 );
	/*Graphic->SetMaxDebugMsgs( 50 );
	Graphic->SetDebugFontSize( 4 );
	Graphic->SetDebugFontColor( 0xffff0000 );
	Graphic->SetDebugFontName( "Courier" );*/
	DebugOutput.Init( GrDebugOutputCallback );

	
	//Init Dialog menu();
	//Start Dialog();

	//if (startSinglePlayer()) ERRORMSG(ERRGENERIC,"startSinglePlayer()","Start single failed");
	
	if (startGame()) ERRORMSG(ERRGENERIC,"main::_tWinMain()","Start game failed");
	graphic::CCamera		cam;
	D3DXVECTOR3	vec;
	
	//Server->HostSession();
	//Client->connectTo();
	// Main message loop:
	while ( gameState )
	{
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
		{	
			if ( msg.message == WM_QUIT ) gameState = 0;
			TranslateMessage( &msg ); 	
			DispatchMessage( &msg );
		}
		else
		{
			switch ( gameState & GS_GROUP_MASK )
			{
				case GS_START:
					// Validate, whether device is lost or not
					hr = ValidateLostDevice();
					if (FAILED(hr) ) ERRORMSG(hr, "main::_tWinMain()", "Unable to recover from lost device");
					
					Graphic->RenderDialogOnly();
					hr = Client->Sound.Update();
					if (FAILED(hr) ) ERRORMSG(hr, "main::_tWinMain()", "Unable to update sound system.");

					// Update server, if it is gameplay
					//if (Server->GetServerState() == SS_GAME_PLAY)
					//	Server->updateGame();
					break;

				case GS_PLAY:
					switch (Client->GetClientState() & CS_GROUP_MASK)
					{
						case CS_GAME_PLAY:
						case CS_RACE_RESULT: 
							// Validate, whether device is lost or not
							hr = ValidateLostDevice();
							if (FAILED(hr) ) ERRORMSG(hr, "main::_tWinMain()", "Unable to recover from lost device");
							hr = Client->Sound.Update();
							if (FAILED(hr) ) ERRORMSG(hr, "main::_tWinMain()", "Unable to update sound system.");

							Graphic->GetCamera( &cam );
							cam.GetEyePtVector( &vec );
							Graphic->SetMapPosX( (int) (vec.x / TERRAIN_PLATE_WIDTH) );
							Graphic->SetMapPosZ( (int) (vec.z / TERRAIN_PLATE_WIDTH) );
						
							for ( listik = Scene->DynamicObjects.begin(); listik != Scene->DynamicObjects.end(); listik++ )
								(*listik)->SetMapPositionAsWorldPosition();

							Graphic->RenderScene();
							hr = Server->updateGame();
							if (FAILED(hr) )
							{
								FreeGamePlayResources();
								ErrorHandler.HandleError(hr, "WinMain()", "Error while updating game");
								SHOWMESSAGEBOX("SERVER ERROR", "There was an error on server. Server stopped.", pDialogs->pMainDialog);
								break;
							}

							break;
					}
					break;
			}

			pSoundEngine->DoWork();
		}
	}


	freeAll();
	
#ifdef MEMLEAK_DEBUG
	} // NEMAZAT
	DeInitAllocCheck();
#endif

	return (int) msg.wParam;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function used for handling XACT (sound playback) notifications
//
//////////////////////////////////////////////////////////////////////////////////////////////
void WINAPI XACTNotificationCallback( const XACT_NOTIFICATION * pNotification )
{
	switch ( pNotification->type )
	{
	case XACTNOTIFICATIONTYPE_CUESTOP: 
		// Handle the cue stop notification 	
		break;
	case XACTNOTIFICATIONTYPE_SOUNDBANKDESTROYED:
		// destroy associtated resources
		if ( pNotification->pvContext ) SAFE_DELETE_ARRAY( (LPVOID) pNotification->pvContext );
		break;
	case XACTNOTIFICATIONTYPE_WAVEBANKDESTROYED:
		// destroy associtated resources
		if ( pNotification->pvContext ) UnmapViewOfFile( (LPVOID) pNotification->pvContext );
		break;
	}
};

