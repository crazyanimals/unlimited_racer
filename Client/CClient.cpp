#include "stdafx.h"
#include "CClient.h"
#include "../main/dialogs.h"


extern CRITICAL_SECTION	g_csServerList;
extern CMyDialogs			*pDialogs;

#define INIT_CAM_TYPE CAM_BEHIND

// constructor
CClient::CClient()
{
	clientState = CS_NULL;
	netClient = 0;

	for(int i=0; i<MAX_PLAYERS; i++)
		vCarsMesh[i] = NULL;

	SinglePlayer = TRUE;

	// PJ //////////////////////////////
	
	////////// By Venca_X for camera ///////////
	cameraType = INIT_CAM_TYPE;
	this->ChangeCamera();
	
	//buttonState = false;
	oldPos.m_x = oldPos.m_y = oldPos.m_z = 0.0f;
	cameraTimer = 0;
	behindCamera = false;
	// PJ //////////////////////////////
}

// destructor
CClient::~CClient()
{
	SAFE_DELETE(netClient);
}


void CClient::ReceivePing(){
	
	ping = (GetTickCount() - pingtime)/ 2;
	pingtime = 0;
}
/////////////////////////////////////////
// return 0 ok.. return -1 connection lost a socket was closed
int CClient::SendPing()
{
	HRESULT hr;

	if(pingtime != 0)  {
		netClient->Disconnect();
		return -1;
	}
	Network::CNetMsg msg;
	pingtime = GetTickCount();
	hr = msg.setNetMsg(NETMSG_PING,NULL,NULL);
	if (FAILED(hr) )
		ERRORMSG(hr, "CClient::SendPing()", "Unable to set net message.");
	hr = netClient->SendMsg(&msg);
	if ( hr )
		ERRORMSG(hr, "CClient::SendPing()", "Error while sending message.")
	return 0;
}


void CClient::StartPing(int ptime){
	pingtime = 0;
	PingTimer = SetTimer( 0,0, ptime,PingTimerProc);

}
void CClient::StopPing(){
	pingtime = 0;
	KillTimer(0,PingTimer);
	
}
void CALLBACK CClient::PingTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time){
	HRESULT hr;
	hr = Client->SendPing();
	if (FAILED(hr) )
	{
		Client->ChangeState(0);
		ErrorHandler.HandleError(hr, "CClient::PingTimerProc()", "Unable to send ping.");
	}
	
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// initHUD method must be called when game MOD has been already chosen, this loads the HUD
//  objects as is defined by the MOD
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CClient::initHUD()
{
	HRESULT		hr;
	CString		name;

	
	// init the HUD structure
	hr = HUD.Init(& (Graphic->HUDCore), this->ResourceManager);
	if (FAILED(hr) ) ERRORMSG(hr, "CClient::initHUD()", "Unable to initialize HUD.");


	// if HUD file is not specified by game MOD, the default one is used instead
	name = pGameMod->GetModuleHUDFileName();
	if ( !name.GetLength() ) name = Configuration.HUD;


	// Load and register HUD
	hr = Client->HUD.LoadHUD( CONFIG_HUDPATH + name );
	if (FAILED(hr) ) ERRORMSG(hr, "CClient::initHUD()", "Unable to load HUD from file.");

	hr = Client->HUD.Register();
	if (FAILED(hr) ) ERRORMSG(hr, "CClient::initHUD()", "Unable to register HUD with HUD core.");


	// show/hide objects as MOD config demands
    Client->HUD.NowPlaying.SetVisibility(false);
    Client->HUD.Score.SetVisibility(pGameMod->GetConfiguration().bHasHUDScore);
    Client->HUD.PlaceList.SetVibibility(pGameMod->GetConfiguration().bHasHUDPlayerList);
	Client->HUD.InfoMessage.SetVisibility(pGameMod->GetConfiguration().bHasHUDInfoMessage);
	Client->HUD.Time.SetVisibility(pGameMod->GetConfiguration().bHasHUDTime);

	return ERRNOERROR;
};


///////////////////////////////////////////////
// If is connected return true else false
BOOL CClient::isConnected(){
	return netClient->isConnected();
}



HRESULT CClient::init(	graphic::CGraphic *grp,
						graphic::CScene *scn,
						resManNS::CResourceManager *rsm,
						HWND _hWnd, 
						CGameInfo * pGameInfo, 
						CMODWorkshop * _pMODWorkshop,
						void (CALLBACK * LoadingProgress) (CString))
{
	HRESULT hr;
	
	hWnd = _hWnd;
	Graphic = grp;
	Scene = scn;
	ResourceManager = rsm;
	netClient =  new Network::CNetClient(hWnd);
	clientState = CS_NULL;
	ProgressCallback = LoadingProgress;
	pMODWorkshop = _pMODWorkshop;
	


	if (!netClient) ERRORMSG(ERROUTOFMEMORY	,_T("Create instance CNetClient"),_T("Failed to init client"));
	
	if (!netClient->Init()){
		OUTMSG("Client was succesfully inicialized", 2 );
	}
	else {
		ERRORMSG(ERRGENERIC,_T("initClient()"),_T("Failed to init client"));
	};


	// Init game sound system
	hr = Sound.Init();
	if (FAILED(hr) ) ERRORMSG(hr, "CClient::init()", "Unable to initialize game sound system.");

	// Init multiplayer game info
	hr = MultiPlayerInfo.Init(ResourceManager,pGameInfo);
	if (FAILED(hr) ) ERRORMSG(hr, "CClient::init()", "Unable to initialize multiplayer game info.");

	Sound.SetNewSongCallback(SongStaticCallback, (void *) this);
	// Set N seconds callback on 15 seconds after song starts to play
	// Used for hiding now playing info on HUD
	Sound.SetNewSongSecondsCallback(SongSecondsStaticCallback, (void *) this, 15);

	// Set music volume
	Sound.SetMusicVolume(Configuration.MusicVolume);

	// Start timer
	Timer.Start();

	return 0;
};

HRESULT CClient::release(){
	for(int i=0; i<MAX_PLAYERS; i++)
		vCarsMesh[i] = NULL;
	vObjectsMesh.clear();
	return ERRNOERROR;
};

/*
HRESULT CClient::StartEnumServers(){
//	netClient->EnumDirectPlayServers();
	return ERRNOERROR;
};

HRESULT CClient::StopEnumServers(){
	return ERRNOERROR;
};
*/
HRESULT CClient::ConnectToLocalServer(){
	
	CAtlString jmeno=_T("127.0.0.1");
	
	ChangeState(CS_SG_START);
	
	return netClient->ConnectToServer(jmeno,Configuration.netPort);
};
HRESULT CClient::ConnectToServer(CString IP, u_short port)
{
	HRESULT Result;
	
	Result = netClient->ConnectToServer(IP,port);
	if (Result < 0)
		return Result;

	ChangeState(CS_MG_START);

	return ERRNOERROR;
}
/*
HRESULT CClient::ConnectToMultiServer(){
	/*
	CAtlString jmeno=_T("localhost");
	
	clientState = CS_SG_START;
	
	if( initClientNetwork() )
		ERRORMSG(ERRGENERIC,_T("ConnetToSingleServer()"),_T("Failed to init client"));
	
	
	if ( netClient->CreateHostAddress(&jmeno))
		ERRORMSG(ERRGENERIC,"ConnetToSingleServer()","Failed creating Host Addres");
	
	netClient->ConnectToSession();
	*/
//	return 0;
//}

HRESULT CClient::ClientMessageHandler(WPARAM wParam, LPARAM lParam)
{
	
	HRESULT hr = 0;

	switch(clientState & CS_GROUP_MASK){
		
		case CS_MG_ENUMERATE_HOSTS:
			return ClientMultiEnumMH(wParam,lParam);
		
		case CS_MG_START:
			return ClientMultiStartMH(wParam,lParam);
			
		case CS_SG_START :
			return ClientSingleStartMH(wParam,lParam );
			
		case CS_MG_START_LOAD:
			return ClientMultiStartLoadMH(wParam, lParam );

		case CS_GAME_PLAY:
			return ClientGameMH(wParam,lParam );

		case CS_RACE_RESULT:
			return ClientRaceResultMH(wParam, lParam);
	}

    return 0;

};



HRESULT CClient::setLook(){
	
	HRESULT hr = S_OK;
 	//hr = Graphic->InitLightMaps();
	//if (hr) ERRORMSG(hr, "CClient::setLook()", "An error occurs during lamp light maps initialization.");

	hr = Graphic->InitNormalMap();
	if ( hr ) ERRORMSG( hr, "CClient::setLook()", "Unable to init terrain normal map." );


	// perform terrain optimalization
	hr = Graphic->PerformTerrainOptimalization();
	if (hr) ERRORMSG(hr, "CClient::setLook()", "An error occurs during perfomance of terrain optimalization.");

	// place camera to centre of map...
#define INITIAL_HEIGHT 20000.0f		// 2km above ground
	lastCameraPos = D3DXVECTOR4((float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH / -2, 
								INITIAL_HEIGHT, 
								(float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH / -2,
								1
								);
/*
	// theese values are counted by UpdateCamera, though they needn't to be set explicitly... 
	Graphic->GetCamera()->SetLookAtVector(	(float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH / 2, 
								INITIAL_HEIGHT-1.0f,
								(float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH / 2 );
	Graphic->GetCamera()->SetUpVector( 0.0f, 1.0f, 0.0f );
	/*Graphic->SetEyePtVector( -3000.0f, 3000.0f, -3000.0f);
	Graphic->SetLookAtVector( 0.0f, 2000.0f, 0.0f);
	Graphic->SetUpVector( 0.0f, 1.0f, 0.0f);*/

	// PLAY SOUND!!!
	hr = Sound.PlayMusic(TRUE);
	if (FAILED(hr) ) ERRORMSG(hr, "CClient::setLook()", "Error playing music.");

	
	
	return hr;
};

HRESULT CClient::ClientGameMH(WPARAM wParam, LPARAM lParam)
{

	HRESULT hr;
	D3DXVECTOR3 pos;
	D3DXVECTOR3 rot;
	Network::MSG_CAR	msgcar;
	Network::CNetMsg msg;
	graphic::CGrObjectMesh * Mesh,*WMesh;
	int LapsGone;
	UINT ID;
	CString PlayerName;
	UINT RacePosition;
	UINT i;
	APPRUNTIME PlayerLapStartTime;
	APPRUNTIME RaceStartTime;
	CString	cstr;

		
	switch( WSAGETSELECTEVENT(lParam)){
		case FD_READ:

			dMatrix matrix,wmatrix;
			D3DXMATRIX trMat,wtrmat ;
			D3DXMATRIX pomMat;

			while(netClient->ReceiveMsg((SOCKET) wParam,&msg)>0)
			{
				switch(msg.getType())
				{
					case NETMSG_PING://send ping back
						hr = msg.setNetMsg(NETMSG_PING_BACK,NULL,NULL);
						if (FAILED(hr) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientGameMH()", "Unable to set net message.");
						}
						hr = netClient->SendMsgToSocket(&msg,(SOCKET) wParam);
						if (FAILED(hr) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientGameMH()", "Unable to send msg to socket.");
						}
						break;
					case NETMSG_PING_BACK://receive ping
						ReceivePing();
						break;

					case NETMSG_CAR_POSITION:
						Mesh = vCarsMesh[msg.getIndex()];
						
						Scene->Map.UnBoundObject(Mesh);
						msgcar = *((Network::MSG_CAR *) msg.getIndexedData());

						// camera position
						m_rrrPos = msgcar.camPos;

						// car pos
						matrix = msgcar.mxCar;
						
						// this i suppose is transformation matrix of player's car
						trMat = ((D3DXMATRIX &)matrix);

						// make member copy
						m_posMatrix = trMat;
						
						Mesh->SetWorldMat(trMat);

						// Update all billboards bound to object
						for (i = 0; i < (UINT) Mesh->Billboards.size(); i++)
						{
							D3DXMatrixTranslation(&pomMat, Mesh->BillboardsObjects[i]->x, Mesh->BillboardsObjects[i]->y, Mesh->BillboardsObjects[i]->z);
							pomMat = pomMat * trMat;
							Mesh->Billboards[i]->SetPosition(pomMat._41, pomMat._42, pomMat._43);
						}
						
						Scene->Map.BoundObject(Mesh);
						
						for(i = 0; i < (UINT)msgcar.iWheelNum;i++){
								WMesh = vObjectsMesh[msgcar.idWheel[i]];
								Scene->Map.UnBoundObject(WMesh);
								wmatrix = msgcar.mxWheel[i];
								wtrmat=((D3DXMATRIX &)wmatrix);
								WMesh->SetWorldMat(wtrmat);
								Scene->Map.BoundObject(WMesh);
							}

						if(msg.getIndex() == MultiPlayerInfo.GetID())
						{
							CameraUpdate (matrix);
						}

						//if(Configuration.RenderBoundingEnvelopes == 0)
						//{
						//	//Graphic->SetEyePtVector(matrix.m_posit.m_x+dir.x, matrix.m_posit.m_y+dir.y, matrix.m_posit.m_z+dir.z);
						//	//Graphic->SetLookAtVector(matrix.m_posit.m_x, matrix.m_posit.m_y, matrix.m_posit.m_z);
						//	//Graphic->SetUpVector(0.0f, 1.0f, 0.0f);
						//}
						break;
					case NETMSG_OBJECT_POSITION:
						
						Mesh = vObjectsMesh[msg.getIndex()];
						
						Scene->Map.UnBoundObject(Mesh);

						matrix = *((dMatrix *) msg.getIndexedData());
						trMat=((D3DXMATRIX &)matrix);
						Mesh->SetWorldMat(trMat);

						Scene->Map.BoundObject(Mesh);

						break;
					case NETMSG_CAR_GEAR:
						HUD.Gear.SetGear( *((int *) msg.getData()) - 1);
						break;

					case NETMSG_CAR_SPEED:
						HUD.Speedometer.SetSpeed( (int) (* ((float *) msg.getData())*3.6f));
						break;

					case NETMSG_ENGINE_RPM:
						HUD.Speedometer.SetRPM( (* (float *) msg.getData()));
						break;

					case NETMSG_HUD_SCORE:
						Network::GetMsgName( &msg, cstr );
						HUD.Score.SetText( cstr );
						break;

                    case NETMSG_HUD_INFO_TEXT:
                        Network::GetMsgName(&msg, cstr);
                        HUD.InfoMessage.SetText(cstr);
                        break;

                    case NETMSG_HUD_TIME:
                        Network::GetMsgName(&msg, cstr);
                        HUD.Time.SetText(cstr);
                        break;

						/*
				//RACER MOD ONLY:
					case NETMSG_CHECKPOINT_POSITION:
						MultiPlayerInfo.GetMsgCheckpointPosition(&msg, pos);
						Graphic->SetCheckpointPosition(pos);
						Graphic->SetRenderCheckpoints(TRUE);
						break;
					case NETMSG_LAPS_GONE:
						MultiPlayerInfo.GetMsgLapsGone(&msg, ID, LapsGone);
						MultiPlayerInfo.SetLapsGone(ID, LapsGone);
						MultiPlayerInfo.GetPlayerName(ID, PlayerName);
						MultiPlayerInfo.GetRacePosition(ID, RacePosition);
						HUD.PlaceList.SetRaceState(ID, PlayerName, LapsGone, RacePosition);

						// Update lap time
						MultiPlayerInfo.GetLapStartTime(ID, PlayerLapStartTime);
						MultiPlayerInfo.UpdateBestLapTime(ID, Timer.GetRunTime() - PlayerLapStartTime);
						MultiPlayerInfo.SetLapStartTime(ID, Timer.GetRunTime() );

						// Test whether player completed race (when lap mode)
						if (MultiPlayerInfo.GetLapMode() && MultiPlayerInfo.GetNumberOfLaps() <= LapsGone)
						{
							// update race time
							RaceStartTime = MultiPlayerInfo.GetRaceStartTime();
							MultiPlayerInfo.SetRaceTime(ID, Timer.GetRunTime() - RaceStartTime);
							MultiPlayerInfo.GetBestLapTime(ID, PlayerLapStartTime);
							
							// add to final standings
							MultiPlayerInfo.FinalPosition.push_back(PlayerName);
							MultiPlayerInfo.FinalRaceTime.push_back(Timer.GetRunTime() - RaceStartTime);
							MultiPlayerInfo.FinalBestLapTime.push_back(PlayerLapStartTime);
						}
						break;
					case NETMSG_RACE_POSITION:
						MultiPlayerInfo.GetMsgRacePosition(&msg, ID, RacePosition);
						MultiPlayerInfo.SetRacePosition(ID, RacePosition);
						MultiPlayerInfo.GetPlayerName(ID, PlayerName);
						MultiPlayerInfo.GetLapsGone(ID, LapsGone);
						HUD.PlaceList.SetRaceState(ID, PlayerName, LapsGone, RacePosition);
						break;
				*/
					case NETMSG_RETURNTOLOBBY:
						MultiPlayerInfo.SetServerReturnedToLobby(TRUE);
						break;
				/*
				//RACER MOD ONLY:
					case NETMSG_SHOW_DIALOG_3:
						CHANGE_ACTDIALOG(pStartDialog);
						
						pDialogs->showDialog();
						Graphic->SetMouseVisible( false );
						pDialogs->pStartDialog->CDLabel->SetValue("3");
						break;
					case NETMSG_SHOW_DIALOG_2:
						pDialogs->pStartDialog->CDLabel->SetValue("2");
						break;
					case NETMSG_SHOW_DIALOG_1:
						pDialogs->pStartDialog->CDLabel->SetValue("1");
						
						break;
					case NETMSG_SHOW_DIALOG_GO:
						pDialogs->pStartDialog->CDLabel->SetValue("GO");
						pDialogs->allowkeystogame=TRUE;
						break;
					case NETMSG_HIDE_DIALOG:
						pDialogs->allowkeystogame=FALSE;
						pDialogs->hideDialog();
						CHANGE_ACTDIALOG(pInGameDialog);
						break;
				*/
				}

                // perform custom game MOD actions
                if (msg.getType() & NETMSG_GAMEMOD)
                {
                    pGameMod->ClientGameMH(wParam, lParam, &msg);
                }

                msg.Release();
			}
			break;
	}
		
	return ERRNOERROR;
};

///////////////////////////
// camera updating
///////////////////////////
void
CClient::CameraUpdate (dMatrix matrix)
{
	D3DXMATRIX pomMat;
	D3DXVECTOR4 dir, dir1;

	switch(cameraType)
	{
	case CAM_BEHIND:
		this->UpdateCam_Smooth( m_posMatrix);
		break;

	case CAM_BEHIND_1:
		this->UpdateCam_Smooth( m_posMatrix);
		break;

	case CAM_FREE:
		break;

	case CAM_LEFTSIDE:
		dir = D3DXVECTOR4(-1000.0f, 350.0f, 0.0f, 1.0f);
		dir1 = D3DXVECTOR4(0.0f, 250.0f, 0.0f, 1.0f);
		pomMat = m_posMatrix;
		pomMat._41 = pomMat._42 = pomMat._43 = 0.0f;
		D3DXVec4Transform(&dir, &dir, &pomMat);
		D3DXVec4Transform(&dir1, &dir1, &pomMat);
		Graphic->SetEyePtVector(matrix.m_posit.m_x+dir.x, matrix.m_posit.m_y+dir.y, matrix.m_posit.m_z+dir.z);
		Graphic->SetLookAtVector(matrix.m_posit.m_x+dir1.x, matrix.m_posit.m_y+dir1.y, matrix.m_posit.m_z+dir1.z);
		Graphic->SetUpVector(matrix.m_up.m_x, matrix.m_up.m_y, matrix.m_up.m_z);
		break;

	case CAM_REVERSE:
		this->UpdateCam_Smooth( m_posMatrix);
		break;

	case CAM_RIGHTSIDE:
		dir = D3DXVECTOR4(1000.0f, 350.0f, 0.0f, 1.0f);
		dir1 = D3DXVECTOR4(0.0f, 250.0f, 0.0f, 1.0f);
		pomMat = m_posMatrix;
		pomMat._41 = pomMat._42 = pomMat._43 = 0.0f;
		D3DXVec4Transform(&dir, &dir, &pomMat);
		D3DXVec4Transform(&dir1, &dir1, &pomMat);
		Graphic->SetEyePtVector(matrix.m_posit.m_x+dir.x, matrix.m_posit.m_y+dir.y, matrix.m_posit.m_z+dir.z);
		Graphic->SetLookAtVector(matrix.m_posit.m_x+dir1.x, matrix.m_posit.m_y+dir1.y, matrix.m_posit.m_z+dir1.z);
		Graphic->SetUpVector(matrix.m_up.m_x, matrix.m_up.m_y, matrix.m_up.m_z);
		break;

	case CAM_FLY_BY:
		pomMat = m_posMatrix;
		pomMat._41 = pomMat._42 = pomMat._43 = 0.0f;
		if(cameraTimer < CAMERA_TIMER)
		{
			Graphic->SetEyePtVector(oldPos.m_x, oldPos.m_y, oldPos.m_z);
			Graphic->SetLookAtVector(matrix.m_posit.m_x, matrix.m_posit.m_y, matrix.m_posit.m_z);
			Graphic->SetUpVector(0.0f, 1.0f, 0.0f);
			cameraTimer++;
		}
		else
		{
			dir = D3DXVECTOR4(0.0f, pomMat._42 + 200.0f, 3000.0f, 1.0f);
			D3DXVec4Transform(&dir, &dir, &pomMat);
			oldPos = matrix.m_posit + dVector(dir.x, dir.y, dir.z);
			cameraTimer = 0;
		}
		break;
	}
		//wheels
}

HRESULT	CClient::ClientRaceResultMH(WPARAM wParam, LPARAM lParam)
{
	Network::CNetMsg msg;
	HRESULT hr;
	int LapsGone;
	UINT ID;
	CString PlayerName;
	UINT RacePosition;
	APPRUNTIME PlayerLapStartTime;
	APPRUNTIME RaceStartTime;


	switch( WSAGETSELECTEVENT(lParam)){
		case FD_READ:

			while(netClient->ReceiveMsg((SOCKET) wParam,&msg)>0)
			{
				switch(msg.getType())
				{
					case NETMSG_PING://send ping back
						hr = msg.setNetMsg(NETMSG_PING_BACK,NULL,NULL);
						if (FAILED(hr) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientRaceResultMH()", "Unable to set net message.");
						}
						hr = netClient->SendMsgToSocket(&msg,(SOCKET) wParam);
						if (FAILED(hr) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientRaceResultMH()", "Unable to send message to socket.");
						}
						break;
					case NETMSG_PING_BACK://receive ping
						ReceivePing();
						break;
					case NETMSG_RETURNTOLOBBY:
						MultiPlayerInfo.SetServerReturnedToLobby(TRUE);
						break;
					/*
					//RACER MOD ONLY:
					case NETMSG_LAPS_GONE:
						MultiPlayerInfo.GetMsgLapsGone(&msg, ID, LapsGone);
						MultiPlayerInfo.SetLapsGone(ID, LapsGone);
						MultiPlayerInfo.GetPlayerName(ID, PlayerName);
						MultiPlayerInfo.GetRacePosition(ID, RacePosition);
						HUD.PlaceList.SetRaceState(ID, PlayerName, LapsGone, RacePosition);

						// Update lap time
						MultiPlayerInfo.GetLapStartTime(ID, PlayerLapStartTime);
						MultiPlayerInfo.UpdateBestLapTime(ID, Timer.GetRunTime() - PlayerLapStartTime);
						MultiPlayerInfo.SetLapStartTime(ID, Timer.GetRunTime() );

						// Test whether player completed race (when lap mode)
						if (MultiPlayerInfo.GetLapMode() && MultiPlayerInfo.GetNumberOfLaps() <= LapsGone)
						{
							// update race time
							RaceStartTime = MultiPlayerInfo.GetRaceStartTime();
							MultiPlayerInfo.SetRaceTime(ID, Timer.GetRunTime() - RaceStartTime);
							MultiPlayerInfo.GetBestLapTime(ID, PlayerLapStartTime);
							
							// add to final standings
							MultiPlayerInfo.FinalPosition.push_back(PlayerName);
							MultiPlayerInfo.FinalRaceTime.push_back(Timer.GetRunTime() - RaceStartTime);
							MultiPlayerInfo.FinalBestLapTime.push_back(PlayerLapStartTime);
						}
						break;
					*/
				}	


				// perform custom game MOD actions
				if ( msg.getType() & NETMSG_GAMEMOD ) pGameMod->ClientGameResultsMH( wParam, lParam, &msg );


				msg.Release();
			}
			break;
	}
		
	return ERRNOERROR;
}

HRESULT CClient::ClientMultiEnumMH(WPARAM wParam, LPARAM lParam){
	
	Network::CNetMsg msg;
	Network::SERVER_INFO ServInfo;

	switch( WSAGETSELECTEVENT(lParam)){
		case FD_READ:

			while(netClient->ReceiveMsg((SOCKET) wParam,&msg)>0){
			
				switch(msg.getType()){
						case NETMSG_BROADCAST_SERVER_INFO :
						
							ServInfo = *((Network::SERVER_INFO *)msg.getData());
							AddServer(ServInfo);
							break;
					

					}	
				msg.Release();
			}
			break;
	}
		
	return ERRNOERROR;

};

HRESULT CClient::ClientMultiStartMH(WPARAM wParam, LPARAM lParam){


	HRESULT				hr = ERRNOERROR;	
	Network::CNetMsg	msg;
	int					id;
	CString				Name;
	int					NumLaps, Hours, Minutes;
	BOOL				Laps;
	std::multiset<DWORD> CRCs;
	std::vector<CString> Names;
	unsigned int		i;
	CString				PlayerName;
	CString				CarName;
	BOOL				Active;
	BOOL				Ready;
	unsigned int		ID;
	int					TeamID;
	graphic::CGrObjectMesh *	grOM=NULL;
	CGameMod		*	_pGMOD;


	switch( WSAGETSELECTEVENT(lParam)){
		case FD_CONNECT:
			break;
		case FD_CLOSE:
			Client->ChangeState(0);
			break;

		case FD_READ:

			while(netClient->ReceiveMsg((SOCKET) wParam,&msg)>0)
			{
				switch(msg.getType())
				{
					case NETMSG_ACCEPTED:
						// Clear all previous info
						MultiPlayerInfo.Clear();
						MultiPlayerInfo.SetState(CMultiPlayerGameInfo::PHASE_ONE);

						break;
					case NETMSG_PING://send ping back
						hr = msg.setNetMsg(NETMSG_PING_BACK,NULL,NULL);
						if (FAILED(hr) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientMultiStartMH()", "Unable to set net message.");
						}
						hr = netClient->SendMsgToSocket(&msg,(SOCKET) wParam);
						if (FAILED(hr ) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientMultiStartMH()", "Unable to send message to socket.");
						}
						break;
					case NETMSG_PING_BACK://receive ping
						ReceivePing();
						break;

					case NETMSG_PLAYER_ID:
						// Set Id	
						id = *((int *) msg.getData());
						MultiPlayerInfo.SetID(id);
						break;
					case NETMSG_GAME_NAME:
						// Store game name
						Network::GetMsgName(&msg, Name);
						MultiPlayerInfo.SetGameName(Name);
						break;
					case NETMSG_GAME_TYPE:
						// Store game type
						Network::GetMsgName( &msg, Name );
						_pGMOD = pMODWorkshop->FindMOD( Name );
						if ( !_pGMOD ) 
						{
							CString	csName, csCar;
							csName = csCar = "";
							MultiPlayerInfo.SetPlayerStatus( MultiPlayerInfo.GetID(), csName, csCar, -1, false, false );
							Network::SetMsgPlayerStatus( &msg, MultiPlayerInfo.GetID(), &csName, &csCar, -1, false, false );
							netClient->SendMsg( &msg );
							Client->ChangeState( 0 );							
							ERRORMSG(ERRINCOMPATIBLE, "CClient::ClientMultiStartHM()", "You don't have a proper version of game module used on the server.");
						}
						else pGameMod = _pGMOD;
						break;
					case NETMSG_PLAYER_MAP_NAME:
						// Set map name
						Network::GetMsgName(&msg, Name);
						hr = MultiPlayerInfo.SetMapFileName(Name);
						if (FAILED(hr) )
						{
							CString	csName, csCar;
							csName = csCar = "";
							MultiPlayerInfo.SetPlayerStatus( MultiPlayerInfo.GetID(), csName, csCar, -1, false, false );
							Network::SetMsgPlayerStatus( &msg, MultiPlayerInfo.GetID(), &csName, &csCar, -1, false, false );
							netClient->SendMsg( &msg );
							Client->ChangeState(0);							
							ERRORMSG(0, "CClient::ClientMultiStartHM()", "Map is not existing or not same or corrupted.");
						}
						break;
					case NETMSG_PLAYER_CRC:
						// Compare map CRCs
						CRCs.clear();
						Network::GetMsgCRCs(&msg, CRCs);
						hr = MultiPlayerInfo.CompareMapFileName(CRCs);
						if (FAILED(hr) )
						{
							Client->ChangeState(0);
							break;

						}
						
						MultiPlayerInfo.SetState(CMultiPlayerGameInfo::PHASE_TWO);
						// Get all car filenames and compute their CRCs
						hr = MultiPlayerInfo.FetchAllCars();
						if (FAILED(hr) )
						{
							Client->ChangeState(0);
							break;
						}

						// Send all car filenames to server
						hr = Network::SetMsgAllCarsFileNames(&msg, &MultiPlayerInfo.AllowedCarsFileNames );
						if (FAILED(hr ) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientMultiStartMH()", "Unable to set message.");
						}
						hr = netClient->SendMsg(&msg);
						if (FAILED(hr ) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientMultiStartMH()", "Unable to send message.");
						}

						// Send their CRCs to server
						for (i = 0; i < MultiPlayerInfo.GetCarNum(); i++)
						{
							hr = Network::SetMsgCarCRCs(&msg, i, &MultiPlayerInfo.AllowedCarsCRCs );
							if (FAILED(hr) )
							{
								Client->ChangeState(0);
								ERRORMSG(hr, "CClient::ClientMultiStartMH()", "Unable to set message.");
							}
							hr = netClient->SendMsg(&msg);
							if (FAILED(hr ) )
							{
								Client->ChangeState(0);
								ERRORMSG(hr, "CClient::ClientMultiStartMH()", "Unable to send message.");
							}
						}
						break;
					case NETMSG_PLAYER_CAR_NAMES:
						// Set allowed car names and control player car names
						Network::GetMsgAllCarsFileNames(&msg, Names);
						MultiPlayerInfo.SetAllowedCarsFileNames(Names);
						MultiPlayerInfo.SetState(CMultiPlayerGameInfo::PHASE_THREE);
						break;
					case NETMSG_MAP_SETTINGS:
						// Update game settings
						Network::GetMsgGameSettings(&msg, Hours, Minutes);
						MultiPlayerInfo.SetGameSettings(Hours, Minutes);
						break;
					case NETMSG_PLAYER_STATUS:
						// Updata player status
						Network::GetMsgPlayerStatus(&msg, ID, PlayerName, CarName, TeamID, Ready, Active);
						MultiPlayerInfo.SetPlayerStatus(ID, PlayerName, CarName, TeamID, Ready, Active);
						break;
					case NETMSG_CHAT:
						// Add chat message
						Network::GetMsgName(&msg, Name);
						MultiPlayerInfo.AddChatMessage(Name);
						break;
					case NETMSG_MULTIGAME_STARTLOAD:
						// Set state to loading 
						MultiPlayerInfo.SetState(CMultiPlayerGameInfo::PHASE_FOUR);
						break;
				}


				// perform custom game MOD actions
				if ( msg.getType() & NETMSG_GAMEMOD ) pGameMod->ClientMPLobbyMH( wParam, lParam, &msg );


				msg.Release();
			}
			break;
	}
		
	return ERRNOERROR;

	/*switch( dwMessageType )
    {
		case DPN_MSGID_ENUM_HOSTS_RESPONSE:
			PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg;
            pEnumHostsResponseMsg = (PDPNMSG_ENUM_HOSTS_RESPONSE)pvMessage;

             Take note of the host response
            AddResponce( pEnumHostsResponseMsg );
            break;
	}*/

	return hr;

};

HRESULT CClient::ClientMultiStartLoadMH(WPARAM wParam, LPARAM lParam)
{
	Network::CNetMsg msg;
	BYTE * Data;
	BYTE * Pos;
	UINT Size;
	UINT i, e;
	dMatrix matrix;
	D3DXMATRIX trMat;
	D3DXMATRIX pomMat;
	D3DXVECTOR3 CheckpointPosition;
	graphic::CGrObjectBase * BaseObj;
	graphic::CGrObjectMesh * MeshObj;
	HRESULT hr;

	
	switch( WSAGETSELECTEVENT(lParam)){
		case FD_CLOSE:
			Client->ChangeState(0);
			break;
		case FD_READ:

			while(netClient->ReceiveMsg((SOCKET) wParam,&msg)>0){
			
				switch(msg.getType()){
					case NETMSG_PLAYER_LOADED:
						HUD.PlaceList.ClearRaceState();
						// Set start times for race
						SetStartTimes();
						// all players are loaded - start game
						ChangeState(CS_GAME_PLAY);
						// Set ping back to normal time
						StopPing();
						StartPing();

						// set state to everything loaded
						MultiPlayerInfo.SetState(CMultiPlayerGameInfo::PHASE_FIVE);
						break;
					case NETMSG_STATIC_POSITION:
						Data = (BYTE *) msg.getData();
						Size = (* (UINT *) Data);
						Pos = Data + sizeof(UINT);
						for (i = 0; i < Size; i++)
						{
							matrix = * ((dMatrix *) Pos);
							trMat=((D3DXMATRIX &)matrix);
							BaseObj = Scene->StaticObjects[i];
							BaseObj->SetWorldMat(trMat);
							// Update all billboards bound to object
							if (BaseObj->GetObjectType() == GROBJECT_TYPE_MESH || BaseObj->GetObjectType() == GROBJECT_TYPE_CAR)
							{
								MeshObj = (graphic::CGrObjectMesh *) BaseObj;
								for (e = 0; e < (UINT) MeshObj->Billboards.size(); e++)
								{
									D3DXMatrixTranslation(&pomMat, MeshObj->BillboardsObjects[e]->x, 
										MeshObj->BillboardsObjects[e]->y, 
										MeshObj->BillboardsObjects[e]->z);
									pomMat = pomMat * trMat;
									MeshObj->Billboards[e]->SetPosition(pomMat._41, pomMat._42, pomMat._43);
								}
								if (MeshObj->GetLight() )
								{
									MeshObj->GetLight()->Position = D3DXVECTOR3(MeshObj->GetLight()->Position.x,
										MeshObj->GetLight()->Position.y + trMat._42,
										MeshObj->GetLight()->Position.z);
								}
							}
							
							Scene->Map.BoundObject((graphic::CGrObjectMesh * )Scene->StaticObjects[i]);
							Pos += sizeof(dMatrix);
						}

						break;
				/*
				//RACER MOD ONLY:
					case NETMSG_CHECKPOINT_POSITION:
							MultiPlayerInfo.GetMsgCheckpointPosition(&msg, CheckpointPosition);
							Graphic->SetCheckpointPosition(CheckpointPosition);
							Graphic->SetRenderCheckpoints(TRUE);
					break;
				*/
					case NETMSG_PING://send ping back
						hr = msg.setNetMsg(NETMSG_PING_BACK,NULL,NULL);
						if (FAILED(hr) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientMultiStartLoadMH()", "Unable to set message.");
						}
						hr = netClient->SendMsgToSocket(&msg,(SOCKET) wParam);
						if (FAILED(hr) )
						{
							Client->ChangeState(0);
							ERRORMSG(hr, "CClient::ClientMultiStartLoadMH()", "Unable to send message.");
						}
						break;
					case NETMSG_PING_BACK://receive ping
						ReceivePing();
						break;
					case NETMSG_CAR_POSITION:
						MeshObj = vCarsMesh[msg.getIndex() ];
						
						Scene->Map.UnBoundObject(MeshObj);
						
						matrix = *((dMatrix *) msg.getIndexedData());
						trMat=((D3DXMATRIX &)matrix);
						MeshObj->SetWorldMat(trMat);

						// Update all billboards bound to object
						for (i = 0; i < (UINT) MeshObj->Billboards.size(); i++)
						{
							D3DXMatrixTranslation(&pomMat, MeshObj->BillboardsObjects[i]->x, MeshObj->BillboardsObjects[i]->y, MeshObj->BillboardsObjects[i]->z);
							pomMat = pomMat * trMat;
							MeshObj->Billboards[i]->SetPosition(pomMat._41, pomMat._42, pomMat._43);
						}
						
						Scene->Map.BoundObject(MeshObj);
						break;
					case NETMSG_OBJECT_POSITION:
							
						MeshObj = vObjectsMesh[msg.getIndex()];
						
						Scene->Map.UnBoundObject(MeshObj);

						matrix = *((dMatrix *) msg.getIndexedData());
						trMat=((D3DXMATRIX &)matrix);
						MeshObj->SetWorldMat(trMat);

						Scene->Map.BoundObject(MeshObj);

						break;
				}	


				// perform custom game MOD actions
				if ( msg.getType() & NETMSG_GAMEMOD ) pGameMod->ClientMPStartMH( wParam, lParam, &msg );


				msg.Release();
			}
			break;
	}
		
	return ERRNOERROR;
	
}

HRESULT CClient::SendControls(char *pKeys,DWORD size){
	Network::CNetMsg msg;
	HRESULT hr;
	hr = msg.setNetMsg(NETMSG_CONTROLS,(BYTE *) pKeys,size );
	if (FAILED(hr) )
	{
		Client->ChangeState(0);
		ERRORMSG(hr, "CClient::SendControls()", "Unable to set net message.");
	}

	hr = netClient->SendMsg(&msg);
	if (FAILED(hr) )
	{
		Client->ChangeState(0);
		ERRORMSG(hr, "CClient::SendControls()", "Unable to send message.");
	}

	return ERRNOERROR;

};

HRESULT CClient::ClientSingleStartMH(WPARAM wParam, LPARAM lParam)

{	
    HRESULT				hr = S_OK;
    graphic::CGrObjectMesh * grOM=NULL;
    Network::CNetMsg	msg, msgSend;
	
	BYTE * Data;
	BYTE * Pos;
	UINT Size;
	UINT i, e;
	dMatrix matrix;
	D3DXMATRIX trMat;
	D3DXMATRIX pomMat;
	D3DXVECTOR3 CheckpointPosition;
	graphic::CGrObjectBase * BaseObj;
	graphic::CGrObjectMesh * MeshObj;

	switch(WSAGETSELECTEVENT(lParam))
	{
		case FD_CONNECT:
			break;
		
		case FD_READ:
			while(netClient->ReceiveMsg((SOCKET) wParam,&msg)>0)
			{
				switch(msg.getType())
				{
					case NETMSG_ACCEPTED:
						//msgSend.setNetMsg(NETMSG_PLAYER_READY,NULL,0);
						//netClient->SendMsg(&msgSend);
						//OUTMSG("Client was accepted a sent ready.",3);
						
						HUD.PlaceList.ClearRaceState();

						// Set start times for race
						SetStartTimes();
					
						// Set state to game play
						ChangeState(CS_GAME_PLAY);
						break;
					case NETMSG_STATIC_POSITION:
						Data = (BYTE *) msg.getData();
						Size = * ((UINT *) Data);
						Pos = Data + sizeof(UINT);
						for (i = 0; i < Size; i++)
						{
							matrix = * ((dMatrix *) Pos);
							trMat=((D3DXMATRIX &)matrix);
							BaseObj = Scene->StaticObjects[i];
							BaseObj->SetWorldMat(trMat);
							// Update all billboards bound to object
							if (BaseObj->GetObjectType() == GROBJECT_TYPE_MESH || BaseObj->GetObjectType() == GROBJECT_TYPE_CAR)
							{
								MeshObj = (graphic::CGrObjectMesh *) BaseObj;
								for (e = 0; e < (UINT) MeshObj->Billboards.size(); e++)
								{
									D3DXMatrixTranslation(&pomMat, MeshObj->BillboardsObjects[e]->x, 
										MeshObj->BillboardsObjects[e]->y, 
										MeshObj->BillboardsObjects[e]->z);
									pomMat = pomMat * trMat;
									MeshObj->Billboards[e]->SetPosition(pomMat._41, pomMat._42, pomMat._43);
									if (MeshObj->GetLight() )
									{
										MeshObj->GetLight()->Position = D3DXVECTOR3(MeshObj->GetLight()->Position.x,
											MeshObj->GetLight()->Position.y + trMat._42,
											MeshObj->GetLight()->Position.z);
									}
								}
							}
							
							Scene->Map.BoundObject((graphic::CGrObjectMesh * )Scene->StaticObjects[i]);
							Pos += sizeof(dMatrix);
						}

						break;
				/*
				//RACER MOD ONLY:

					case NETMSG_CHECKPOINT_POSITION:
						MultiPlayerInfo.GetMsgCheckpointPosition(&msg, CheckpointPosition);
						Graphic->SetCheckpointPosition(CheckpointPosition);
						Graphic->SetRenderCheckpoints(TRUE);
					break;
				*/
					case NETMSG_CAR_POSITION:
						MeshObj = vCarsMesh[msg.getIndex() ];
						
						Scene->Map.UnBoundObject(MeshObj);
						
						matrix = *((dMatrix *) msg.getIndexedData());
						trMat=((D3DXMATRIX &)matrix);
						MeshObj->SetWorldMat(trMat);

						// Update all billboards bound to object
						for (i = 0; i < (UINT) MeshObj->Billboards.size(); i++)
						{
							D3DXMatrixTranslation(&pomMat, MeshObj->BillboardsObjects[i]->x, MeshObj->BillboardsObjects[i]->y, MeshObj->BillboardsObjects[i]->z);
							pomMat = pomMat * trMat;
							MeshObj->Billboards[i]->SetPosition(pomMat._41, pomMat._42, pomMat._43);
						}
						
						Scene->Map.BoundObject(MeshObj);
						break;
					case NETMSG_OBJECT_POSITION:
						
						MeshObj = vObjectsMesh[msg.getIndex()];
						
						Scene->Map.UnBoundObject(MeshObj);

						matrix = *((dMatrix *) msg.getIndexedData());
						trMat=((D3DXMATRIX &)matrix);
						MeshObj->SetWorldMat(trMat);

						Scene->Map.BoundObject(MeshObj);

						break;
					/*
					case NETMSG_PLAYER_CAR_NAME:
						hr=loadCar((char *)msg.getData(), CarNumber, ResourceManager, this, MapID);
						vCarsMesh.push_back((graphic::CGrObjectMesh *) Scene->DynamicObjects.back() );
						// TODO: add load car fail test and resolution
						OUTMSG("Car was loaded",3);
						CarNumber++;
						
						break;

					case NETMSG_PLAYER_MAP_NAME :
						hr = MapInit((LPCTSTR) msg.getData(),Scene,ResourceManager, this);
						// Clear car order
						vCarsMesh.clear();
						// Set car number to zero
						CarNumber = 0;
						// Obtain map ID
						MapID = ResourceManager->LoadResource( (LPCTSTR) msg.getData(), RES_Map, true, 0);
						OUTMSG("Map was loaded",3);
						setLook();
						clientState=CS_GAME_PLAY;
						break;
					*/

				}


				// perform custom game MOD actions
				if ( msg.getType() & NETMSG_GAMEMOD ) pGameMod->ClientSPStartMH( wParam, lParam, &msg );


				msg.Release();
			}
			

			break;
	}

    return hr;
};
HRESULT CClient::SendServersInfoDemand(){
	
	HRESULT				hr = ERRNOERROR;
	Network::CNetMsg	msg;

	hr = msg.setNetMsg(NETMSG_BROADCAST_DEMAND,NULL,NULL);
	if (FAILED(hr) )
		ERRORMSG(hr, "CClient::SendServersInfoDemand()", "Unable to set message.");
	
	hr = netClient->SendBroadcastMsg(&msg,Configuration.netBroadPort);
	if (FAILED(hr) )
		ERRORMSG(hr, "CClient::SendServersInfoDemand()", "Unable to send broadcast message.");
	return hr;
}

HRESULT CClient::StartBroadcast(){	 
	return netClient->StartBroadcast(Configuration.netBroadPort);
}

HRESULT CClient::StopBroadcast(){
	return netClient->StopBroadcast();
}
//void CClient::ClearServerList()
//{
//	// Clean up Host list
//    EnterCriticalSection(&g_csServerList);
//	vServerList.clear();
//    LeaveCriticalSection(&g_csServerList);
//};

//void CClient::AddResponce(PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg){
//	
//	
//	HRESULT hr = S_OK;
//    BOOL    bFound;
//	DWORD aktTime;
//	DWORD  NoResponseTime = 3000;
///*	size_t size;
//	DWORD i;
//	
//	GUID aplGuid = pEnumHostsResponseMsg->pApplicationDescription->guidInstance;
//	
//
//	size = vServerList.size();
//	i = 0;
//	bFound=false;
//	aktTime = GetTickCount();
//
//	EnterCriticalSection(&g_csServerList);
//	while( i <size ) {
//		if (vServerList[i].pApplicationDescription->guidInstance == aplGuid) {
//			vServerList[i].pvUserContext = (PVOID) aktTime;
//			bFound = true;
//			i++;
//		}
//		else {
//			if ( (aktTime - (DWORD) vServerList[i].pvUserContext) > NoResponseTime ){
//		//		vServerList.erase(i);
//				size --;
//
//			} 
//			else i++;
//		}
//	}
//*/
//	vector <DPNMSG_ENUM_HOSTS_RESPONSE >::iterator iter;
//
//	GUID aplGuid = pEnumHostsResponseMsg->pApplicationDescription->guidInstance;
//
//	iter = vServerList.begin();
//
//	bFound = false;
//	aktTime = GetTickCount();
//
//	EnterCriticalSection(&g_csServerList);
//
//	while( iter != vServerList.end() ) {
//		
//		if ( (*iter).pApplicationDescription->guidInstance == aplGuid) {
//			(*iter).pvUserContext = (PVOID) aktTime;
//			bFound = true;
//			iter++;
//		}
//		else {
//			if ( (aktTime - (DWORD) (*iter).pvUserContext) > NoResponseTime ){
//				iter = vServerList.erase(iter);
//			} 
//			else iter++;
//		}
//	}
//
//	if ( !bFound ){
//		pEnumHostsResponseMsg->pvUserContext=(PVOID) aktTime;
//		vServerList.push_back(* pEnumHostsResponseMsg);
//	}
//	
//	LeaveCriticalSection(&g_csServerList);
//};

// Is called, when new song starts playing
void CClient::SongStaticCallback(void * Ptr)
{
	CClient * ClientPtr = (CClient *) Ptr;

	ClientPtr->SongCallback();
}

// Is called, when new song starts playing
void CClient::SongCallback()
{
	if (Sound.GetTitle() != "" || Sound.GetArtist() != "")
	{
		HUD.NowPlaying.SetVisibility(true);
		HUD.NowPlaying.SetNowPlayingText(Sound.GetArtist(), Sound.GetTitle() );
	}

}

// Is called N seconds after new song starts playing
void CClient::SongSecondsStaticCallback(void * Ptr)
{
	CClient * ClientPtr = (CClient *) Ptr;

	ClientPtr->SongSecondsCallback();
}

void CClient::SongSecondsCallback()
{
	HUD.NowPlaying.SetVisibility(false);
}

// SERVER LIST MAINTENANCE
// Adds server to the list
HRESULT CClient::AddServer(Network::SERVER_INFO & ServerInfo)
{
	HRESULT Result;
	pair<set<CServerInfoWithTime>::iterator, bool> InsertResult;

	DialogServerInfo DialogInfo;

	// adress info structures
	addrinfo Hints;
	addrinfo * AddressInfo = NULL;
	sockaddr_in * SocketAddress;

	
	// Fill in dialog server info
	strcpy_s( DialogInfo.name, MAX_GAME_NAME_LENGTH, ServerInfo.name );
	strcpy_s( DialogInfo.type, MAX_GAME_NAME_LENGTH, ServerInfo.type );
	DialogInfo.port = ServerInfo.port;

	if (!ServerInfo.Address)
	{
		ZeroMemory(&Hints, sizeof(addrinfo) );
		Hints.ai_family = AF_INET;
		Hints.ai_socktype = SOCK_DGRAM;
		Hints.ai_protocol = IPPROTO_UDP;

		// Translate host name into IP
		Result = getaddrinfo(ServerInfo.addr, NULL, &Hints, &AddressInfo);
		if (Result != 0)
		{
			OUTMSG(CString("Unable to get IP address from host name: ") + CString(ServerInfo.addr), 1 );
			return ERRNOERROR;
		}
			
		SocketAddress = (sockaddr_in*) AddressInfo->ai_addr;
		DialogInfo.addr = SocketAddress->sin_addr.S_un.S_addr;
	}
	else
	{
		DialogInfo.addr = inet_addr(ServerInfo.addr);
	}

	// Prepare structure to save in server list
	CServerInfoWithTime ServerInfoTime;
	ServerInfoTime.ServerInfo = DialogInfo;
	ServerInfoTime.Time = GetTickCount();

	// insert into server list
	InsertResult = ServerList.insert(ServerInfoTime);
	// Refresh time, if the server is in the server info
	if (! InsertResult.second)
		InsertResult.first->Time = GetTickCount();

	// Free address info
	freeaddrinfo(AddressInfo);

	return ERRNOERROR;
}

// Updates server list in order to exclude dead servers
// TimeOut is in miliseconds
HRESULT CClient::UpdateServers(int TimeOut)
{
	int TimeDif;
	set<CServerInfoWithTime>::iterator It;

	for (It = ServerList.begin(); It != ServerList.end(); It++)
	{
		TimeDif = ( (int) GetTickCount() - (int) (*It).Time - TimeOut);
		if (  TimeDif > 0 )
		{
			It = ServerList.erase(It);
			if (It == ServerList.end() ) break;
		}
	}

	return ERRNOERROR;
}

// Updates player status message
HRESULT CClient::UpdatePlayerStatus(CString & PlayerName, CString & CarFileName, int TeamID, BOOL Ready)
{
	UINT				ID;
	Network::CNetMsg	Msg;
	HRESULT				hr;

	ID = MultiPlayerInfo.GetID();
	MultiPlayerInfo.SetPlayerStatus( ID, PlayerName, CarFileName, TeamID, Ready, true );
	hr = Network::SetMsgPlayerStatus( &Msg, ID, &PlayerName, &CarFileName, TeamID, Ready, true );
	if (FAILED(hr) )
		ERRORMSG(hr, "CClient::UpdatePlayerStatus()", "Unable to set message.");
	
	hr = netClient->SendMsg(&Msg);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CClient::UpdatePlayerStatus()", "Unable to send message.");

	return ERRNOERROR;
}

// Disconnect from server
HRESULT CClient::DisconnectFromServer()
{
	netClient->Disconnect();

	return ERRNOERROR;
}

// Adds chat message and send it to server
HRESULT CClient::AddChatMessage(CString & ChatMessage)
{
	Network::CNetMsg Msg;
	CString PlayerName;
	CString Message;
	UINT ID;
	ID = MultiPlayerInfo.GetID();
	MultiPlayerInfo.GetPlayerName(ID, PlayerName);

	Message = PlayerName + CString(": ") + ChatMessage;
	MultiPlayerInfo.AddChatMessage(Message);
	Network::SetMsgChat( &Msg, &Message );
	netClient->SendMsg(&Msg);

	return ERRNOERROR;
}

HRESULT CClient::ClientStartMultiLoad()
{
	Network::CNetMsg msg;
	UINT i;
	BOOL Active;
	CString CarName;
	graphic::CGrObjectMesh * grOM=NULL;
	HRESULT hr;

	SinglePlayer = FALSE;

	if (ProgressCallback) ProgressCallback("Loading...");

	// Start map and cars loading
	ChangeState(CS_MG_START_LOAD);

	// change ping for longer time in order loading have enough time
	StopPing();
	//StartPing(60000);

	// Map loading
	hr = MapInit(MultiPlayerInfo.GetMapFileName(),Scene,ResourceManager, ProgressCallback, this);
	if (FAILED(hr) )
		ERRORMSG(hr, "CClient::ClientStartMultiLoad()", CString("Unable to load map filename: ") + MultiPlayerInfo.GetMapFileName() );
	OUTMSG("Map was loaded",3);
	
	// Obtain MapID
	MapID = ResourceManager->LoadResource(MultiPlayerInfo.GetMapFileName(), RES_Map, true, 0);

	// Load cars
	for(i=0; i < MAX_PLAYERS; i++)
		vCarsMesh[i] = NULL;
	CarNumber = 0;
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		MultiPlayerInfo.GetPlayerActive(i, Active);
		if (Active)
		{
			MultiPlayerInfo.GetPlayerCar(i, CarName);
			hr=loadCar(CarName, CarNumber, i, ResourceManager, this, MapID, ProgressCallback);
			if (hr)
			{
				ResourceManager->ReleaseResource(MapID);
				ERRORMSG(-1, "CClient::ClientStartMultiLoad()", CString("Unable to load car filename:") + CarName);
			}
			OUTMSG("Car was loaded",3);
			CarNumber++;
		}
	}

	ResourceManager->ReleaseResource(MapID);

	hr = setLook();
	if (FAILED(hr) )
		ERRORMSG(hr, "CClient::ClientStartMultiLoad()", CString("Unable to do load finalization.") );

	// Send message to server, that everything was loaded
	msg.setNetMsg(NETMSG_PLAYER_LOADED, NULL, NULL);
	netClient->SendMsg(&msg);

	// Enable/disable rendering of checkpoints
//RACER MOD ONLY:
	//if (! MultiPlayerInfo.GetLapMode() ) Graphic->SetRenderCheckpoints(FALSE);

	if (ProgressCallback) ProgressCallback("Waiting for others...");


	// perform game MOD custom actions
	hr = pGameMod->ClientMPStart();
	if ( hr ) ERRORMSG(hr, "CClient::ClientStartMultiLoad()", "Unable to start game MOD client.");


	return ERRNOERROR;
}

// Loads all needed things for single player game
HRESULT CClient::ClientStartSingleLoad()
{
	HRESULT Result;
	resManNS::RESOURCEID MapID;
	BOOL PlayerActive;
	CString PlayerCar;
	int CarNumber;

	SinglePlayer = TRUE;

	if (ProgressCallback) ProgressCallback("Loading...");
	Result = ConnectToLocalServer();
	if (FAILED(Result) )
		ERRORMSG(Result, "CClient::ClientStartSingleLoad()", "Unable to connect to local server.");

	// Load map
	Result = MapInit(MultiPlayerInfo.GetMapFileName(),Scene,ResourceManager,ProgressCallback, this);
	if (FAILED(Result) )
		ERRORMSG(Result, "CClient::ClientStartSingleLoad()", "Unable to load map.");

	Result = setLook();
	if (FAILED(Result) )
		ERRORMSG(Result, "CClient::ClientStartSingleLoad()", "Unable to perform map rendering preparations.");

	OUTMSG("Map was loaded",3);

	// Obtain map ID
	MapID = ResourceManager->LoadResource( MultiPlayerInfo.GetMapFileName(), RES_Map, true, 0);
	
	// Set player ID to be zero
	MultiPlayerInfo.SetID(0);

	// Load player car
	for(int i=0; i<MAX_PLAYERS; i++)
		vCarsMesh[i] = NULL;
	
	CarNumber = 0;
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
		if (PlayerActive)
		{
			MultiPlayerInfo.GetPlayerCar(i, PlayerCar);
			Result=loadCar(PlayerCar, CarNumber, i, ResourceManager, this, MapID, ProgressCallback);		
			if (FAILED(Result) )
			{
				ResourceManager->ReleaseResource(MapID);
				ERRORMSG(Result, "CClient::ClientStartSingleLoad()", "Unable to load player car for graphics.");
			}		
			CarNumber++;
		}
		
	}
	
	OUTMSG("Cars was loaded",3);
	
	// Enable/disable rendering of checkpoints
//RACER MOD ONLY:
	//if (! MultiPlayerInfo.GetLapMode() ) Graphic->SetRenderCheckpoints(FALSE);

	OUTS( "Graphic memory available: ", 2 );
	OUTI( Graphic->GetAvailableTextureMem(), 2 );
	OUTSN( " kB", 2 );

	ResourceManager->ReleaseResource(MapID);

	// perform game MOD custom actions
	Result = pGameMod->ClientSPStart();
	if ( Result ) ERRORMSG(Result, "CClient::ClientStartSingleLoad()", "Unable to start game MOD client.");


	return ERRNOERROR;
}

// changes client state + performs various operations associated with client state change
HRESULT CClient::ChangeState(int NewState)
{
	// Check whether its needed to 
	// 1) disconnect from server
	// 2) clear multiplayer game info
	// 3) stop ping
	if (clientState == CS_GAME_PLAY || 
		clientState == CS_MG_START ||
		clientState == CS_MG_START_LOAD ||
		clientState == CS_SG_START || 
		clientState == CS_RACE_RESULT)
		
	{
		if (NewState != CS_GAME_PLAY &&
			NewState != CS_MG_START &&
			NewState != CS_MG_START_LOAD && 
			NewState != CS_SG_START && 
			NewState != CS_RACE_RESULT)
		{

			StopPing();
			DisconnectFromServer();
			MultiPlayerInfo.Clear();
		}
	}

	// Check whether partial multiplayer game info clear is needed
	if (clientState == CS_GAME_PLAY ||
		clientState == CS_RACE_RESULT)
	{
		if (NewState == CS_MG_START)
			MultiPlayerInfo.Clear(TRUE);
	}

	// Check whether to start or stop broadcast
	if (NewState == CS_MG_ENUMERATE_HOSTS)
	{
		StartBroadcast();
		SendServersInfoDemand();
	}
	if (clientState == CS_MG_ENUMERATE_HOSTS && NewState != CS_MG_ENUMERATE_HOSTS)
		StopBroadcast();
		

	// remember new state
	clientState = NewState;

	return ERRNOERROR;
}

// Sets start times for race
void CClient::SetStartTimes()
{
/*
//RACER MOD ONLY:
	BOOL PlayerActive;
	UINT i;

	
	// Set race start time
	MultiPlayerInfo.SetRaceStartTime(Timer.GetRunTime() );

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
		if (PlayerActive)
			MultiPlayerInfo.SetLapStartTime(i, Timer.GetRunTime() );
	}
*/
}

////////////////////////////
// Camera Update - SMOOTH
//
// We always place the camera behind the target, so we get current wanted position
// and the we smoothly interpolate new pos between current pos and wanted pos
////////////////////////////
void
CClient::UpdateCam_Smooth( const D3DXMATRIX &mat)
{
	
	D3DXVECTOR4 curCarPos = D3DXVECTOR4( 0, 0, 0, 1);

	D3DXMATRIX pomMat = mat;

	// compute curr car position from matrix
	D3DXVec4Transform( &curCarPos, &curCarPos, &pomMat);
	
	/////////////////////////////////////////
	// reciding camera depending on speed
	// compute lenght of last position - current pos vector...
	D3DXVECTOR4 diff = ( curCarPos - lastCarPos);
	
	/*
	// ... and magicaly scale it
	float len = m_reciding_speed * D3DXVec4LengthSq( &(diff));
	if( len > 1.0f)
	{
		len = 1.0f;	// edge case
	}
	
	// compute current camera distance from car origin
	float dist = len * len * m_distance_shift*ONE_METER + m_distance_min*ONE_METER;
	*/
	/////////////////////////////////////////

	// lets make orientation matrix by zeroing of trnaslation fragment
	pomMat._41 = pomMat._42 = pomMat._43 = 0.0f;

	// compute wanted current camera position through cas orientation matrix
	D3DXVECTOR4 wantedPos = D3DXVECTOR4( 0, 0, -m_distance_min*ONE_METER, 1);
	D3DXVec4Transform(&wantedPos, &wantedPos, &pomMat);

	// store only x, z coords -> projection to xz plane
	wantedPos.y = 0.0f;
	
	// The target we are following
	// the target is the player's car. Always
	D3DXVECTOR4 lookAt = curCarPos;
	lookAt.y += 1.5f * ONE_METER;	// translation of look at point
	Graphic->SetLookAtVector((D3DXVECTOR3)lookAt);
	Graphic->SetUpVector(0.0f, 1.0f, 0.0f);
	
	D3DXVECTOR4 interpolatedCurrPos;
	D3DXVec4Lerp( &interpolatedCurrPos, &lastCameraPos, &wantedPos, m_camera_agressivness);

	// store current camera pos vector (origin = car)
	lastCameraPos = interpolatedCurrPos;

	// add height -> always camera is in constant height above the car
	interpolatedCurrPos.y += (m_cam_height*ONE_METER);
	
	// set camera eye positioin
	Graphic->SetEyePtVector((D3DXVECTOR3)(interpolatedCurrPos+curCarPos));	

	// save current car position
	lastCarPos = curCarPos;
}