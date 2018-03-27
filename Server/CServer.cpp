#include "CServer.h"
#include "..\Globals\TimeCounter.h"


extern CServer * Server;



LRESULT CServer::ServerMessageHandler(WPARAM wParam, LPARAM lParam){
	
	switch(serverState & SS_GROUP_MASK){

		case SS_SG_START:
			
			return ServerSingleStartMH(wParam,lParam);
			

		case SS_MG_START: 
		
			return ServerMultiStartMH(wParam,lParam);
			
		case SS_MG_START_LOAD:

			return ServerMultiStartLoadMH(wParam,lParam);

		case SS_GAME_PLAY:

			return ServerGameMH(wParam,lParam);
			
		
	}

	return 0;
}


void CServer::StartPing(int pingtime)
{
	for(int i = 0; i< MAX_CONNECTIONS;i++)
		MultiPlayerInfo.SetPingTime(i,0);
	
	PingTimer = SetTimer( 0,0,pingtime,PingTimerProc);
}
void CServer::StopPing()
{
	for(int i = 0; i< MAX_CONNECTIONS;i++)
		MultiPlayerInfo.SetPingTime(i,0);
	
	KillTimer(0,PingTimer);
}
void CALLBACK CServer::PingTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time)
{
	Server->SendPing(-1);
}
HRESULT CServer::ReceivePing(int id)
{	
	DWORD pingtime,ping;
	MultiPlayerInfo.GetPingTime(id,pingtime);
	ping = (GetTickCount() - pingtime)/ 2;
	MultiPlayerInfo.SetPingTime(id,0);
	MultiPlayerInfo.SetPing(id,ping);
	return ERRNOERROR;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return 0 ok..return -1 connection lost a socket was closed 
// id= -1 send to all players
int CServer::SendPing(int id)
{	
	Network::CNetMsg msg;
	int i;
	HRESULT hr;
	DWORD pingTime;

	if (id == -1)
	{
		for( i = 0 ; i < MAX_CONNECTIONS;i++)
		{
			if(isConnected(i))
			{
				MultiPlayerInfo.GetPingTime(i,pingTime);
				if(pingTime != 0)  
				{
					DisconnectPlayer(i);
					MultiPlayerInfo.SetPingTime(i,0);
					continue;
				}
				
				MultiPlayerInfo.SetPingTime(i,GetTickCount());
				hr = msg.setNetMsg(NETMSG_PING,NULL,NULL);
				if (FAILED(hr) )
					ERRORMSG(ERRGENERIC, "CServer::SendPing()", "Unable to set net message.");
				hr = netServer->SendMsg(&msg,i);
				if (FAILED(hr ) )
					ERRORMSG(ERRGENERIC, "CServer::SendPing()", "Unable to send net message.");
			}
			
		}

	}
	else
	{
		if (isConnected(id) )
		{
			MultiPlayerInfo.GetPingTime(id,pingTime);
			if(pingTime != 0)  
			{
				DisconnectPlayer(id);
				MultiPlayerInfo.SetPingTime(id,0);
				return 0;
			}

			MultiPlayerInfo.SetPingTime(id,GetTickCount());
			hr = msg.setNetMsg(NETMSG_PING,NULL,NULL);
			if (FAILED(hr) )
				ERRORMSG(hr, "CServer::SendPing()", "Unable to set net message.");
			hr = netServer->SendMsg(&msg,id);
			if (FAILED(hr ) )
				ERRORMSG(hr, "CServer::SendPing()", "Unable to send net message.");
		}
	}
	
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////
// return 0 
// id= -1 disconnect all players
HRESULT CServer::DisconnectPlayer(int id)
{
	Network::CNetMsg	Msg;
	CString				PlayerName, CarFileName;
	int i;
	
	if(id >=0) 
	{
		// Set not active
		PlayerName = CarFileName = CString("");
		MultiPlayerInfo.SetPlayerStatus(id, PlayerName, CarFileName, -1, false, false);
		Network::SetMsgPlayerStatus( &Msg, id, &PlayerName, &CarFileName, -1, false, false );
		
		netServer->ClosePlayer(id);

		// Send to all players info about kicking
		netServer->SendMsg(&Msg);
		
		// recompute allowed car data
		MultiPlayerInfo.ClearClientCarAndCRCs(id);
		MultiPlayerInfo.CompareCarsFileNames();
		// Send allowed car data message to all client
		Network::SetMsgAllCarsFileNames( &Msg, &(MultiPlayerInfo.AllowedCarsFileNames) );
		netServer->SendMsg(&Msg);
		
	}
	if(id == -1) 
		for(i = 0; i<MAX_CONNECTIONS;i++){
			if(isConnected(i))
				netServer->ClosePlayer(i);
		}
	return ERRNOERROR;

}
//////////////////////////////////////////////////////////////
// return -1 when server is full return -2 for INVALID_SOCKET
int CServer::AddPlayer()
{
	int					id;
	Network::CNetMsg	msg;
	BOOL				PlayerActive;
	UINT				i;
	HRESULT				hr;
	CString				cstr;

	id = netServer->AcceptPlayer();
	
	// if ID is accepted, than send:
	// 0) Write down, that player is active
	// 0.5) Game type
	// 1) Map file name
	// 2) Map CRCs
	// 3) Map settings
	if (id >= 0)
		
	{
		hr = msg.setNetMsg(NETMSG_ACCEPTED ,NULL, NULL);
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		
		hr = netServer->SendMsg(&msg,id);
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}

		// Set player active
		MultiPlayerInfo.SetPlayerStatus(id, CString(""), CString(""), 0, false, true);
		MultiPlayerInfo.SetPlayerAI(id, FALSE);

		// Send ID to the client
		hr = Network::SetMsgID(&msg, id);
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		hr = netServer->SendMsg(&msg, id);
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		// Send game name
		hr = Network::SetMsgGameName( &msg, &(MultiPlayerInfo.GameName) );
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		hr = netServer->SendMsg(&msg, id);
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		// Send game type
		cstr = pGameMod->GetModulePrimaryID();
		hr = Network::SetMsgGameType( &msg, &cstr );
		if ( FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		hr = netServer->SendMsg( &msg, id );
		if ( FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		// Send map file name
		hr = Network::SetMsgMap( &msg, &(MultiPlayerInfo.MapFileName) );
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		hr = netServer->SendMsg(&msg, id);
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		// Send map CRCs
		hr = Network::SetMsgMapCRCs( &msg, &(MultiPlayerInfo.MapCRCs) );
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		hr = netServer->SendMsg(&msg, id);
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		// send map settings
		hr = Network::SetMsgGameSettings( &msg, MultiPlayerInfo.GetHours(), MultiPlayerInfo.GetMinutes() );
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		hr = netServer->SendMsg(&msg, id);
		if (FAILED(hr) )
		{
			DisconnectPlayer(id);
			OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
			return -1;
		}
		// send active player status
		for (i = 0; i < MAX_PLAYERS; i++)
		{
			MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
			if (PlayerActive)
			{
				hr = Network::SetMsgPlayerStatus( &msg, i, &(MultiPlayerInfo.PlayerNames[i]), 
								(&MultiPlayerInfo.CarFileNames[i]), MultiPlayerInfo.GetPlayerTeamID(i),
								MultiPlayerInfo.Ready[i], MultiPlayerInfo.GetPlayerActive(i) );
				if (FAILED(hr) )
				{
					DisconnectPlayer(id);
					OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
					return -1;
				}
				if (i == id)
				{
					hr = netServer->SendMsg(&msg);
					if (FAILED(hr) )
					{
						DisconnectPlayer(id);
						OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
						return -1;
					}
				}
				else
				{
					hr = netServer->SendMsg(&msg, id);
					if (FAILED(hr) )
					{
						DisconnectPlayer(id);
						OUTS("CServer::AddPlayer(), error occured during adding player. Player disconnected.", 1);
						return -1;
					}
				}
			}
		}	
			
	}


	return id;

};




HRESULT CServer::StopSession(){
	return netServer->StopSession();
}


HRESULT CServer::init(	physics::CPhysics * Ph,
						HWND _hWnd,
						resManNS::CResourceManager * ResManager,
						CGameInfo * pGameInfo,
						CMODWorkshop * _pMODWorkshop,
						void (CALLBACK *LoadingProgress) (CString) )
{	
	hWnd=_hWnd;
	Physics = Ph;
	netServer = new Network::CNetServer(hWnd);
	ResourceManager = ResManager;
	pGameMod = NULL;
	pMODWorkshop = _pMODWorkshop;
	ProgressCallback = LoadingProgress;
	IPList = NULL;
//RACER MOD ONLY:	bControl_blocked = false;
	HRESULT hr;
	
	serverState = 0;

	if (!netServer) ERRORMSG(ERROUTOFMEMORY, "CServer::init()", "Unable to allocate place for net server.");

    Timer = new CTimeCounter();
	if (! Timer) 
	{
		SAFE_DELETE(netServer);
		ERRORMSG(ERROUTOFMEMORY, "CServer::init()", "Unable to allocate place for time counter.");
	}

    Timer->Start();

	if (!netServer->Init()){
		OUTMSG("Server was succesfully inicialized.", 2 );
	}
	else 
	{
		SAFE_DELETE(netServer);
		SAFE_DELETE(Timer);
		ERRORMSG(ERRGENERIC,_T("CServer::init()"),_T("Failed to init server"));
	};

	// Init multiplayerinfo 
	hr = MultiPlayerInfo.Init(ResourceManager,pGameInfo);
	if (FAILED(hr) )
	{
		SAFE_DELETE(netServer);
		SAFE_DELETE(Timer);
		ERRORMSG(hr, "CServer::Init()", "Unable to init multiplayer game info.");
	}
	
	return 0;
};



HRESULT CServer::StartServerSingle()
{
	HRESULT Result;	
	resManNS::RESOURCEID MapID;
	UINT i;
	UINT CarNumber;
	BOOL PlayerActive;
	CString PlayerCar;

	if (ProgressCallback) ProgressCallback("Loading...");
	ChangeServerState(SS_SG_START);
	
	if (netServer->HostSession(Configuration.netPort))
		ERRORMSG(ERRGENERIC,_T("CServer::StartServerSingle()"),_T("Failed to start server"));
	netServer->StopBroadcast();
	OUTMSG("Session was created.",1);

	OUTMSG("Loading physics started ...", 1);
	// Load map for physics
	Result = MapPhInit(MultiPlayerInfo.GetMapFileName(), Physics, ResourceManager, ProgressCallback, pGameMod);
	if (FAILED(Result) )
		ERRORMSG(Result, "CServer::StartServerSingle()", "Unable to load map for physics.");
							
	// Obtain MapID
	MapID = ResourceManager->LoadResource(MultiPlayerInfo.GetMapFileName(), RES_Map, true, 0);

	// Load cars for physics
	CarNumber = 0;
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
		if (PlayerActive)
		{
			MultiPlayerInfo.GetPlayerCar(i, PlayerCar);
			Result = loadPhCar(PlayerCar, CarNumber, i, Physics, ResourceManager, MapID, pGameMod, ProgressCallback);
			if (FAILED(Result) )
			{
				ResourceManager->ReleaseResource(MapID);
				ERRORMSG(Result, "CServer::StartServerSingle()", "Unable to load car for physics.");
			}
			CarNumber++;
		}
	}
	OUTMSG("Loading physics finished ...", 1);

	// Init AI
/*
//RACER MOD ONLY:
	if (MultiPlayerInfo.GetLapMode() )
	{
        OUTMSG("Loading AI ...", 1);	
	    if (ProgressCallback) ProgressCallback("Loading AI...");
	    Result = InitAI();
        OUTMSG("Finished!", 1);
	    if (FAILED(Result) )
	    {
		    ResourceManager->ReleaseResource(MapID);
		    ERRORMSG(Result, "CServer::StartServerSingle()", "Unable to initialize AI.");
	    }

	    // Compute checkpoints
        OUTMSG("Computing Checkpoints ...", 1);
	    Result = ComputeCheckpointsPositions();
        OUTMSG("Finished!", 1);
	    if (FAILED(Result) )
	    {
		    ResourceManager->ReleaseResource(MapID);
		    ERRORMSG(Result, "CServer::StartServerSingle()", "Unable to compute checkpoints.");
	    }


	}
*/
	ResourceManager->ReleaseResource(MapID);
	
	if (ProgressCallback) ProgressCallback("Releasing physics startup data...");
	// release data used for physics initialization
	Physics->ReleaseStartUpData();
	if (ProgressCallback) ProgressCallback("Precomputing physics...");
	// Update physics, so that all objects fall onto the ground
	Result = Physics->Update(8000);
	if (FAILED(Result) )
		ERRORMSG(Result, "CServer::StartServerSingle()", "Unable to update physics.");

/*
//RACER MOD ONLY:
	bControl_blocked=TRUE;
	Server->timerPtr=SetTimer(0,0,100,StartingProcedure);
	iStartCounter = 0;
*/

	// perform game MOD custom actions
	Result = pGameMod->ServerSPStart();
	if ( Result ) ERRORMSG(Result, "CServer::StartServerSingle()", "Unable to start game MOD server.");
	

	return ERRNOERROR;
};


HRESULT CServer::StartServerMulti( CString name, CString MapFileName, CGameMod * pGameMod ){
	HRESULT		Result;
	char		Buf[255];
	int			newID;
	//if (initNetwork())
	//	ERRORMSG(ERRGENERIC,_T("initNetwork()"),_T("Init network failed"));;
	
	// Set game name
	MultiPlayerInfo.SetGameName(name);
	
	// Set map file name
	Result = MultiPlayerInfo.SetMapFileName(MapFileName);
	if (FAILED(Result) )
		return -2;

	// Add AI players if AI supported by game MOD
	if ( pGameMod->GetConfiguration().bAI )
	{
		for (UINT i = 0; i < MAX_OPPONENTS; i++)
			if (Configuration.OpponentsFileNames[i] != "")
			{
				newID = AutoAssignTeam();
				if ( newID < 0 ) break; // not enough space for another player
				sprintf_s( Buf, 255, "%i", i + 1 );
				MultiPlayerInfo.SetPlayerAI(MAX_PLAYERS - 1 - i, true);
				MultiPlayerInfo.SetPlayerStatus(MAX_PLAYERS - 1 - i, CString("Computer ") + Buf, Configuration.OpponentsFileNames[i], newID, true, true);
			}
	}	

	if ( netServer->HostSession(Configuration.netPort))
		ERRORMSG(ERRGENERIC,"CServer::StartServerMulti()",_T("Failed to start server"));

	sSessionName = name;
	ChangeServerState(SS_MG_START);

	MultiPlayerInfo.SetState(CMultiPlayerGameInfo::PHASE_THREE);

	return ERRNOERROR;
}

HRESULT CServer::sendAllStartingData(){
	
	HRESULT hr = S_OK;
	
	Network::CNetMsg message;
	
	// send map name
	hr = message.setNetMsg( NETMSG_PLAYER_MAP_NAME,
					(BYTE *) Configuration.MapFileName.GetBuffer(), 
					Configuration.MapFileName.GetLength()+1);

	hr = netServer->SendMsg(&message);
	
	// send car name
	hr = message.setNetMsg( NETMSG_PLAYER_CAR_NAME,
					(BYTE *) Configuration.CarFileName.GetBuffer(), 
					Configuration.CarFileName.GetLength()+1);
	
	hr = netServer->SendMsg(&message);
	
	return hr;
};


/* AutoAssignTeam method returns a team ID for new player - this method tries to keep
	teams balanced - with equal count of members. Returns -1 if there is not enough space
	for another player. */
int CServer::AutoAssignTeam()
{
	MODCONFIG	config;
	UINT		min = MultiPlayerInfo.GetTeamPlayersCount( 0 );
	UINT		ind = 0;
	
	pGameMod->GetConfiguration( &config );

	for ( UINT i = 0; i < config.uiMinTeamCount; i++ )
		if ( MultiPlayerInfo.GetTeamPlayersCount( i ) < min )
		{
			ind = 0;
			min = MultiPlayerInfo.GetTeamPlayersCount( i );
		}

	// return -1 if no more space for this player
	if ( min >= config.uiMaxPlayersPerTeam ) return -1;
	if ( MultiPlayerInfo.GetPlayersCount() >= config.uiMaxPlayers ) return -1;

	return (int) ind;
};



HRESULT CServer::ServerGameMH(WPARAM wParam, LPARAM lParam){
	
	
	char *allKeys;
	Network::CNetMsg msg;
	int id;
	switch(WSAGETSELECTEVENT(lParam))
	{
		case FD_READ:
			
			while (netServer->ReceiveMsg((SOCKET) wParam,&msg)>0){
	
				switch(msg.getType()){
					case NETMSG_PING:
						msg.setNetMsg(NETMSG_PING_BACK,NULL,NULL);
						netServer->SendMsgToSocket((SOCKET) wParam,&msg);
						break;

					case NETMSG_PING_BACK:
						id = netServer->GetPlayerID((SOCKET) wParam);
						ReceivePing(id);
						break;
					
					case NETMSG_CONTROLS:
						id = netServer->GetPlayerID((SOCKET) wParam);
						allKeys  = (char *) msg.getData();
						Physics->cars[id]->Keys(allKeys);
						pGameMod->ServerKeyProcessing( (UINT) id, allKeys );
						
						break;
					case NETMSG_RETURNTOLOBBY:
						id = netServer->GetPlayerID((SOCKET) wParam);
						MultiPlayerInfo.SetReturnToLobby(id, TRUE);
						break;
				}

				// perform custom game MOD actions
				if ( msg.getType() & NETMSG_GAMEMOD ) pGameMod->ServerGameMH( wParam, lParam, &msg );


				msg.Release();
			}
			break;
	}

	return 0;
};

HRESULT CServer::ServerMultiStartLoadMH(WPARAM wParam, LPARAM lParam)
{	
	
	Network::CNetMsg msg;
	int id;
	
	switch(WSAGETSELECTEVENT(lParam))
	{	
		
		
		case FD_CLOSE:
			break;
		
		case FD_READ:
			
			while(netServer->ReceiveMsg((SOCKET) wParam,&msg)>0){
			
				switch(msg.getType()){
					case NETMSG_PING:
						msg.setNetMsg(NETMSG_PING_BACK,NULL,NULL);
						netServer->SendMsgToSocket((SOCKET) wParam,&msg);
						break;
					case NETMSG_PING_BACK:
						id = netServer->GetPlayerID((SOCKET) wParam);
						ReceivePing(id);
						break;
					case NETMSG_PLAYER_LOADED:
					// Set player loaded
					id = netServer->GetPlayerID(wParam);	
					MultiPlayerInfo.SetPlayerLoaded(id, TRUE);

					// Check whether all players were loaded
					CheckAllPlayersLoaded();

					break;
				}

				// perform custom game MOD actions
				if ( msg.getType() & NETMSG_GAMEMOD ) pGameMod->ServerMPStartMH( wParam, lParam, &msg );

				msg.Release();
							
			}

			break;
	}


	return 0;
};

HRESULT CServer::ServerSingleStartMH(WPARAM wParam, LPARAM lParam)

{	
	Network::CNetMsg msg;
	int id;
	HRESULT hr;
	
	switch(WSAGETSELECTEVENT(lParam))
	{	
		
		
		case FD_ACCEPT:
			id = netServer->AcceptPlayer();
			if (id < 0)
				ERRORMSG(0, "CServer::ServerSingleStart()", "Player disconnected due to problem with player accept.");
			
			ChangeServerState(SS_GAME_PLAY);
			// Send static object positions
			hr = SendAllStaticObjectsPositions();
			if (FAILED(hr) )
			{
				DisconnectPlayer(id);
				ERRORMSG(0, "CServer::ServerSingleStart()", "Player disconnected due to problem with sending static objects positions.");
			}
			
			// Send dynamic object positions
			hr = SendAllDynamicObjectsPositions();
			if (FAILED(hr ) )
			{
				DisconnectPlayer(id);
				ERRORMSG(0, "CServer::ServerSingleStart()", "Player disconnected due to problem with sending dynamic objects positions.");
			}

/*
//RACER MOD ONLY:
			// Send first checkpoint position
			if (MultiPlayerInfo.GetLapMode() && CheckpointsPositions.size() != 0)
			{
				hr = MultiPlayerInfo.SetMsgCheckpointPosition(&msg, 
					D3DXVECTOR3(CheckpointsPositions[0].x, CheckpointsPositions[0].y, CheckpointsPositions[0].z) );
				if (FAILED(hr) )
				{
					DisconnectPlayer(id);
					ERRORMSG(0, "CServer::ServerSingleStart()", "Player disconnected do to problem with setting message.");
				}
				hr = netServer->SendMsg(&msg, id);
				if (FAILED(hr) )
				{
					DisconnectPlayer(id);
					ERRORMSG(0, "CServer::ServerSingleStart()", "Player disconnected do to problem with sending message.");
				}
			}
*/

			// perform game MOD custom actions
			hr = pGameMod->ServerSendCustomInitialData();
			if ( hr )
			{
				DisconnectPlayer();
				ERRORMSG(0, "CServer::CheckAllPlayersLoaded()", "All players disconnected due to unable to set net message.");
			}


			hr = msg.setNetMsg(NETMSG_ACCEPTED ,NULL, NULL);
			if (FAILED(hr) )
			{
				DisconnectPlayer(id);
				ERRORMSG(0, "CServer::ServerSingleStart()", "Player disconnected do to problem with setting message.");
			}
			hr = netServer->SendMsg(&msg,id);
			if (FAILED(hr) )
			{
				DisconnectPlayer(id);
				ERRORMSG(0, "CServer::ServerSingleStart()", "Player disconnected do to problem with sending message.");
			}

			// init timer
			lastTime = Timer->GetRunTime();
			break;
		
		case FD_READ:
			
			while(netServer->ReceiveMsg((SOCKET) wParam,&msg)>0){
				msg.Release();
			}

			break;
	}


	return 0;
};


HRESULT CServer::ServerMultiStartMH(WPARAM wParam, LPARAM lParam)

{	
	
	Network::CNetMsg		msg;
	int						id;
	std::multiset<DWORD>	CRCs;
	std::vector<CString>	Names;
	unsigned int			Size1, Size2;
	unsigned int			i, e;
	BOOL					PlayerActive;
	BOOL					PlayerAI;
	CString					PlayerName;
	CString					PlayerCar;
	CString					ChatMessage;
	BOOL					PlayerReady;
	BOOL					IsAICarValid;
	unsigned int			ID;
	int						teamID;

	
	switch(WSAGETSELECTEVENT(lParam))
	{
		case FD_ACCEPT:
			id = AddPlayer();
		
			break;
		case FD_CLOSE:
			ID = netServer->GetPlayerID(wParam);
			DisconnectPlayer(ID);

			break;
		case FD_READ:
			while(netServer->ReceiveMsg((SOCKET) wParam,&msg)>0)
			{
				switch(msg.getType())
				{
					case NETMSG_PING:
						msg.setNetMsg(NETMSG_PING_BACK,NULL,NULL);
						netServer->SendMsgToSocket((SOCKET) wParam,&msg);
						break;
					case NETMSG_PING_BACK:
						id = netServer->GetPlayerID((SOCKET) wParam);
						ReceivePing(id);
						break;
					case NETMSG_BROADCAST_DEMAND:
						SendBroadcast();
						break;
					
					case NETMSG_PLAYER_DATA:
					
						break;
					case NETMSG_PLAYER_CAR_NAME:
					
						break;
					case NETMSG_PLAYER_READY:
					
				/*		MapPhInit(Configuration.MapFileName, Physics);

						/////////////
						// LOAD CAR
						physics::CCar * pomCar;
						// NASKLADAT AUTA
						pomCar = loadPhCar(Configuration.CarFileName, Physics);	
						
						Physics->cars.push_back(pomCar);
						sendAllStartingData();
						serverState= SS_GAME_PLAY;*/
						break;
					case NETMSG_PLAYER_CAR_NAMES:
						id = netServer->GetPlayerID(wParam);
						MultiPlayerInfo.ClearClientCarAndCRCs(id);
						// Store car names for further use
						Network::GetMsgAllCarsFileNames( &msg, Names );
						MultiPlayerInfo.SetClientCarNames(id, Names);
					break;
					case NETMSG_PLAYER_CRC:
						id = netServer->GetPlayerID(wParam);
						Network::GetMsgCRCs( &msg, CRCs );
						// Store car CRCs
						MultiPlayerInfo.AddClientCarCRCs(id, CRCs);
						// if not all CRCs
						MultiPlayerInfo.GetClientCarsCRCsSize(id, Size1);
						MultiPlayerInfo.GetClientCarsSize(id, Size2);
						if (Size1 != Size2)
							break;
						// compute new minimal car set
						MultiPlayerInfo.CompareCarsFileNames();
						// send new minimal car set to all active players
						// and kick AI players with unsupported car
						for (i = 0; i < MAX_PLAYERS; i++)
						{
							MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
							if ( PlayerActive)
							{
								MultiPlayerInfo.GetPlayerAI(i, PlayerAI);
								if (PlayerAI)
								{
									MultiPlayerInfo.GetPlayerCar(i, PlayerCar);
									IsAICarValid = false;
									for (e = 0; e < MultiPlayerInfo.GetAllowedCarsFileNames().size(); e++)
										if (PlayerCar == MultiPlayerInfo.GetAllowedCarsFileNames().at(e) )
										{
											IsAICarValid = true;
											break;
										}
									if (!IsAICarValid)
									{
										MultiPlayerInfo.SetPlayerStatus(i, CString(""), CString(""), -1, false, false);
										MultiPlayerInfo.SetPlayerAI(i, false);
										// send update to all players, that computer was kicked out
										CString		csName, csCar;
										csName = csCar = "";
										Network::SetMsgPlayerStatus( &msg, i, &csName, &csCar, -1, false, false );
										netServer->SendMsg(&msg);
									}
								}

								Network::SetMsgAllCarsFileNames( &msg, &MultiPlayerInfo.AllowedCarsFileNames );
								netServer->SendMsg(&msg, i);
							}
						}
						break;
					case NETMSG_PLAYER_STATUS:
						// Save player status
						Network::GetMsgPlayerStatus( &msg, ID, PlayerName, PlayerCar, teamID, PlayerReady, PlayerActive );
						MultiPlayerInfo.SetPlayerStatus( ID, PlayerName, PlayerCar, teamID, PlayerReady, PlayerActive );
						
						// Send to all other players
						id = netServer->GetPlayerID(wParam);
						for (i = 0; i < MAX_PLAYERS; i++)
							if (id != i)
							{
								MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
								if (PlayerActive)
									netServer->SendMsg(&msg, i);
							}
						
						break;
					case NETMSG_CHAT:
						// Save chat message
						Network::GetMsgName( &msg, ChatMessage );
						MultiPlayerInfo.AddChatMessage(ChatMessage);

						// send to all other players
						id = netServer->GetPlayerID(wParam);
						for (i = 0; i < MAX_PLAYERS; i++)
							if (id != i)
							{
								MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
								if (PlayerActive)
									netServer->SendMsg(&msg, i);
								
							}
						break;

				}

				
				// perform custom game MOD actions
				if ( msg.getType() & NETMSG_GAMEMOD ) pGameMod->ServerMPLobbyMH( wParam, lParam, &msg );


				msg.Release();
			}
			

			break;
	}


	return 0;
};

HRESULT CServer::updateGame(){

	HRESULT hr=ERRNOERROR;
	//char allKeys[4];
	dMatrix mtrx[MAX_PLAYERS];

	if(this->serverState == SS_GAME_PLAY)
	{
		    APPRUNTIME	newTime, dt;//, T2, Tim;

			if(!lastTime)
				lastTime = Timer->GetRunTime();

            /*Tim = */newTime = Timer->GetRunTime();
            dt = newTime - lastTime;
			lastTime = newTime;

			// Updates physics for game
			if(dt > 250)
				dt = 250;
//RACER MOD ONLY:
/*			if(bControl_blocked){
				for(int i=0; i<MAX_PLAYERS; i++){
					if(Physics->cars[i])
					{
						
						mtrx[i] = Physics->cars[i]->GetBody();
						
					//allKeys[0] = 0;allKeys[1]=0;allKeys[2]=0;allKeys[3]=0x00000010;
					//Physics->cars[i]->Keys(allKeys);
					}
				}
			}
*/
			hr = Physics->Update(dt);

//RACER MOD ONLY:			if ( !bControl_blocked && AI ) AI->Update( (float)dt / 1000.0f );
//RACER MOD ONLY:
/*
			if ( bControl_blocked )
			{
				for(int i=0; i<MAX_PLAYERS; i++){
					if(Physics->cars[i])
					{
						Physics->cars[i]->SetBody(mtrx[i]);
						//Physics->cars[i]->Rotation(rot[i].m_w,rot[i].m_y,rot[i].m_z);
					//Physics->cars[i]->Position(pos[i].m_x,pos[i].m_y,pos[i].m_z);
					//pos[i] = Physics->cars[i]->Position();
					//allKeys[0] = 0;allKeys[1]=0;allKeys[2]=0;allKeys[3]=0x00000010;
					//Physics->cars[i]->Keys(allKeys);
					}
				}
			}
*/
			if (FAILED(hr) )
				ERRORMSG(hr, "CServer::updateGame()", "Error while updating physics.");
			//T2 = Timer->GetRunTime();
			//OUTS("Update Physics:\t",1); OUTIN(T2 - Tim, 1);

			// Update AI
			
			//Tim = Timer->GetRunTime();
			//OUTS("Update AI:\t",1); OUTIN(Tim - T2, 1);

			// Updates game status - checkpoints, laps, ...
			hr = UpdateGameStatus(dt);
			if ( FAILED(hr) ) ERRORMSG(hr, "CServer::updateGame()", "Error while updating game status.");
           
			//T2 = Timer->GetRunTime();
			//OUTS("Update game:\t",1); OUTIN(T2 - Tim, 1);

	//         WORD time = GetTickCount();
   //         int dt = time - lastTime;
   //         OUTIN(dt, 1);
			//Physics->Update(dt);
   //         WORD T2 = GetTickCount();
   //         dt = T2 - time;
   //         OUTIN(dt, 1);
   //         lastTime = time;

            
   //     WORD time = GetTickCount();
			//Physics->Update(time - lastTime);
			//lastTime = time;
			Network::CNetMsg msg;
            dVector ObjPos;
			dMatrix ObjMat;
			resManNS::Map * MapObj;
            Network::MSG_CAR msgcar;
			UINT j;
			MapObj = ResourceManager->GetMap(MultiPlayerInfo.GetMapID() );
			if (!MapObj)
				ERRORMSG(-1, "CServer::updateGame()", "Unable to retrieve map object.");

			
			for(int i=0; i<MAX_PLAYERS; i++)
				if(Physics->cars[i])
				{
					//CAtlString _msg;
					//_msg.Format("Position %i [%0.2f %0.2f %0.2f %0.2f|%0.2f %0.2f %0.2f %0.2f|%0.2f %0.2f %0.2f %0.2f|%0.2f %0.2f %0.2f %0.2f]", i,
					//	Physics->cars[i]->Matrix().m_front.m_x,Physics->cars[i]->Matrix().m_front.m_y,Physics->cars[i]->Matrix().m_front.m_z,Physics->cars[i]->Matrix().m_front.m_w,
					//	Physics->cars[i]->Matrix().m_right.m_x,Physics->cars[i]->Matrix().m_right.m_y,Physics->cars[i]->Matrix().m_right.m_z,Physics->cars[i]->Matrix().m_right.m_w,
					//	Physics->cars[i]->Matrix().m_up.m_x,Physics->cars[i]->Matrix().m_up.m_y,Physics->cars[i]->Matrix().m_up.m_z,Physics->cars[i]->Matrix().m_up.m_w,
					//	Physics->cars[i]->Matrix().m_posit.m_x,Physics->cars[i]->Matrix().m_posit.m_y,Physics->cars[i]->Matrix().m_posit.m_z,Physics->cars[i]->Matrix().m_posit.m_w);
					//OUTMSG(_msg, 1);

					msgcar.mxCar = Physics->cars[i]->Matrix();
					ObjPos = msgcar.mxCar.m_posit;
					if (ObjPos.m_x < 0) ObjPos.m_x = 0;
					if (ObjPos.m_z < 0) ObjPos.m_z = 0;
					if (ObjPos.m_x > MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_x = MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f;
					if (ObjPos.m_z > MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_z = MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f;
					msgcar.mxCar.m_posit = ObjPos;

					// camera position
					// send camera position
					msgcar.camPos = Physics->cars[i]->GetCamPos();
				
					//add wheels
					for(j=0;j<Physics->cars[i]->wheels.size() && j < MAX_CAR_WHEELS;j++){
						msgcar.mxWheel[j]=Physics->cars[i]->wheels[j]->_Matrix();
						msgcar.idWheel[j]=Physics->cars[i]->wheels[j]->dynamicObjectsIndex;
						ObjPos = msgcar.mxWheel[j].m_posit;
						if (ObjPos.m_x < 0) ObjPos.m_x = 0;
						if (ObjPos.m_z < 0) ObjPos.m_z = 0;
						if (ObjPos.m_x > MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_x = MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f;
						if (ObjPos.m_z > MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_z = MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f;
						msgcar.mxWheel[j].m_posit = ObjPos;

					}
					msgcar.iWheelNum =j;
					
					msg.setNetMsg(NETMSG_CAR_POSITION,i,(BYTE *) &msgcar,sizeof(Network::MSG_CAR));
					netServer->SendMsg(&msg);
				}

			//CAtlString mes, mstr;
			//mes = "##";
			for(int i=0; i<(int)Physics->dynamicObjects.size(); i++)
			{
				if(Physics->dynamicObjects[i]->Changed() && Physics->dynamicObjects[i]->NBody())
				{
					Physics->dynamicObjects[i]->Changed(false);
					ObjMat = Physics->dynamicObjects[i]->Matrix();
					ObjPos = ObjMat.m_posit;
					if (ObjPos.m_x < 0) ObjPos.m_x = 0;
					if (ObjPos.m_z < 0) ObjPos.m_z = 0;
					if (ObjPos.m_x > MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_x = MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f;
					if (ObjPos.m_z > MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_z = MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f;
					ObjMat.m_posit = ObjPos;
					//mstr.Format(" %d", i);
					//mes.Append(mstr);
					hr = msg.setNetMsg(NETMSG_OBJECT_POSITION,i,
						(BYTE *) &ObjMat,
						sizeof(dMatrix));
				netServer->SendMsg(&msg);
				}
			}

			// send camera positions

			netServer->SendMsg(&msg);
			
			//mes.Append(" ##\n");
			//OUTMSG(mes, 1);
	}
		//netServer->SendMessage();
		
	return hr;
};



HRESULT CServer::SendBroadcast(){
	
	HRESULT hr= ERRNOERROR;
	Network::CNetMsg msg;
	addrinfo * ListItem;
	BOOL Failed = true;
	
	Network::SERVER_INFO serverInfo;
	
	sprintf_s( serverInfo.name, MAX_GAME_NAME_LENGTH, "%s", sSessionName );
	sprintf_s( serverInfo.type, MAX_GAME_NAME_LENGTH, "%s", pGameMod->GetModulePrimaryID() );
	gethostname( serverInfo.addr, MAX_HOST_NAME_LENGTH - 1 );
	serverInfo.port = Configuration.netPort;
	serverInfo.Address = false;
	
	msg.setNetMsg( NETMSG_BROADCAST_SERVER_INFO, (BYTE *) &serverInfo, sizeof(serverInfo) );

	if (IPList)
	{
		ListItem = IPList;
		serverInfo.Address = true;
		do
		{
			strcpy(serverInfo.addr, inet_ntoa( ((sockaddr_in *) ListItem->ai_addr)->sin_addr) );
			msg.setNetMsg(NETMSG_BROADCAST_SERVER_INFO,(BYTE *) &serverInfo,sizeof(serverInfo));
			netServer->BindBroadcast((sockaddr_in *) ListItem->ai_addr, Configuration.netBroadPort);
			hr = netServer->SendBroadcastMsg(&msg,Configuration.netBroadPort);
			if (hr == 0)
				Failed = false;
			ListItem = IPList->ai_next;
		}
		while (ListItem != 0);
		if (Failed) 
			return -1;
		else
			return 0;
	}
	else
		return netServer->SendBroadcastMsg(&msg,Configuration.netBroadPort);

	return ERRNOERROR;
};

HRESULT CServer::StartBroadcast()
{
	struct addrinfo aiHints;
	char CompName[256];
	HRESULT hr;

	// Free IP list
	if (IPList)
	{
		freeaddrinfo(IPList);
		IPList = NULL;
	}

	// obtain all IPs computer is assigned in networks
	// fill in hints
	memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = AF_INET;
	aiHints.ai_socktype = SOCK_DGRAM;
	aiHints.ai_protocol = IPPROTO_UDP;
	aiHints.ai_flags = AI_PASSIVE;

	// get name of this computer
	gethostname(CompName, 255);

	// obtain IPList
	hr = getaddrinfo(CompName, 0, &aiHints, &IPList);

	return netServer->StartBroadcast(Configuration.netBroadPort);

};

HRESULT CServer::StopBroadcast(){
	return netServer->StopBroadcast();
}

// Adds chat message and send it to all clients
HRESULT CServer::AddChatMessage(CString & ChatMessage)
{
	
	//BOOL PlayerActive;
	Network::CNetMsg Msg;

	MultiPlayerInfo.AddChatMessage(ChatMessage);
	Network::SetMsgChat( &Msg, &ChatMessage );
	netServer->SendMsg(&Msg);

	return ERRNOERROR;
}

// Updates game settings and send it to all clients
HRESULT CServer::UpdateGameSettings( int Hours, int Minutes )
{
//RACER MOD ONLY: (must update ALL MOD dependent settings )
	Network::CNetMsg Msg;
	MultiPlayerInfo.SetGameSettings( Hours, Minutes );
	Network::SetMsgGameSettings( &Msg, Hours, Minutes );
	netServer->SendMsg(&Msg);

	return ERRNOERROR;
}

HRESULT CServer::StartServerMultiLoad()
{
	
	HRESULT result;
	Network::CNetMsg msg;
	BOOL b;
	CString carfilename;
	resManNS::RESOURCEID MapID;
	UINT CarNumber;


	if (ProgressCallback) ProgressCallback("Loading...");
	ChangeServerState(SS_MG_START_LOAD);
	// Set state to loading 
	MultiPlayerInfo.SetState(CMultiPlayerGameInfo::PHASE_FOUR);
	
	msg.setNetMsg(NETMSG_MULTIGAME_STARTLOAD,NULL,NULL);
	netServer->SendMsg(&msg);
	// change ping for longer time in order loading have enough time
	StopPing();
	//StartPing(60000);

	OUTMSG("Loading physics started ...", 1);
	// Initialize physics for map
	result = MapPhInit(MultiPlayerInfo.GetMapFileName(), Physics, ResourceManager, ProgressCallback, pGameMod);
	if (FAILED(result) )
		ERRORMSG(result, "CServer::StartServerMultiLoad()", CString("Unable to initialize physics for map filename:") + MultiPlayerInfo.GetMapFileName() );

	// Obtain map ID
	MapID = ResourceManager->LoadResource(MultiPlayerInfo.GetMapFileName(), RES_Map, true, 0);

	CarNumber = 0;
	for(int i=0;i < MAX_PLAYERS;i++){
		
		MultiPlayerInfo.GetPlayerActive(i,b);
		if(b) {
			MultiPlayerInfo.GetPlayerCar(i,carfilename);
			// SKLADAT V PORADI NA STARTU
			result = loadPhCar(carfilename, CarNumber, i, Physics, ResourceManager, MapID, pGameMod, ProgressCallback);
			if (FAILED(result) )
			{
				ResourceManager->ReleaseResource(MapID);
				ERRORMSG(result, "CServer::StartServerMultiLoad()", CString("Unable to load physics for car filename:") + carfilename);
			}
			CarNumber++;
			players.push_back(i);
		}
	}
	OUTMSG("Loading physics finished ...", 1);

	// Init AI
/*
//RACER MOD ONLY:
	if (MultiPlayerInfo.GetLapMode() )
	{
	
		if (ProgressCallback) ProgressCallback("Loading AI...");
		result = InitAI();
		if (FAILED(result) )
		{
			ResourceManager->ReleaseResource(MapID);
			ERRORMSG(result, "CServer::StartServerMultiLoad()", "Unable to initialize AI.");
		}

		// Compute checkpoints
		result = ComputeCheckpointsPositions();
		if (FAILED(result) )
		{
			ResourceManager->ReleaseResource(MapID);
			ERRORMSG(result, "CServer::StartServerMultiLoad()", "Unable to compute checkpoints.");
		}

		
	}
*/

	ResourceManager->ReleaseResource(MapID);

	///////////////////
	// apply mutator
	///////////////////
	Physics->materials.ApplyCurrentMutator();

	if (ProgressCallback) ProgressCallback("Releasing physics startup data...");
	// release data used for physics initialization
	Physics->ReleaseStartUpData();
	if (ProgressCallback) ProgressCallback("Precomputing physics...");
	// Update physics, so that all objects fall onto the ground
	result = Physics->Update(8000);
	if (FAILED(result) )
		ERRORMSG(result, "CServer::StartServerMultiLoad()", "Unable to update physics.");
	

	// perform game MOD custom actions
	result = pGameMod->ServerMPStart();
	if ( result ) ERRORMSG(result, "CServer::StartServerMultiLoad()", "Unable to start game MOD server.");
	

	if (ProgressCallback) ProgressCallback("Server loaded. Waiting...");

	return 0;

};

// sends positions of static objects
HRESULT CServer::SendAllStaticObjectsPositions()
{
	Network::CNetMsg msg;
	BYTE * Buf;
	BYTE * Pos;
	int Size;
	HRESULT hr;

	Size = (int) Physics->staticObjects.size() * sizeof(dMatrix) + sizeof(UINT);
	Buf = new BYTE[Size];
	if (!Buf)
		ERRORMSG(ERROUTOFMEMORY, "CServer::SendAllStaticObjectsPositions()", "Unable to allocate place for message buffer.");
	
	*((UINT *) Buf) = (UINT) Physics->staticObjects.size();
	Pos = Buf + sizeof(UINT);
	for (UINT e = 0; e < Physics->staticObjects.size(); e++)
	{
		*((dMatrix *) Pos) = Physics->staticObjects[e]->Matrix();
		Pos += sizeof(dMatrix);
	}
	
	hr = msg.setNetMsg(NETMSG_STATIC_POSITION, Buf, Size);
	if (FAILED(hr) )
		ERRORMSG(hr, "CServer::SendAllStaticObjectsPositions()", "Unable to set net message.");
	hr = netServer->SendMsg(&msg);
	if (FAILED(hr) )
		ERRORMSG(hr, "CServer::SendAllStaticObjectsPositions()", "Unable to send net message.");
	SAFE_DELETE_ARRAY(Buf);

	return ERRNOERROR;
};

// sends positions of all dynamic objects and cars
HRESULT CServer::SendAllDynamicObjectsPositions()
{
	Network::CNetMsg msg;
	dMatrix ObjMat;
	dVector ObjPos;
	resManNS::Map * MapObj;
	HRESULT hr;
            
	MapObj = ResourceManager->GetMap(MultiPlayerInfo.GetMapID() );
	if (!MapObj)
		ERRORMSG(-1, "CServer::SendAllDynamicObjectsPositions()", "Unable to retrieve map object.");

	for(int i=0; i<MAX_PLAYERS; i++)
		if(Physics->cars[i])
		{
			ObjMat = Physics->cars[i]->Matrix();
			ObjPos = ObjMat.m_posit;
			if (ObjPos.m_x < 0) ObjPos.m_x = 0;
			if (ObjPos.m_z < 0) ObjPos.m_z = 0;
			if (ObjPos.m_x > MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_x = MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f;
			if (ObjPos.m_z > MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_z = MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f;
			ObjMat.m_posit = ObjPos;

			hr = msg.setNetMsg(NETMSG_CAR_POSITION,i,(BYTE *) &ObjMat,sizeof(dMatrix));
			if (FAILED(hr) )
				ERRORMSG(-1, "CServer::SendAllDynamicObjectsPositions()", "Unable to set net message.");
			hr = netServer->SendMsg(&msg);
			if (FAILED(hr) )
				ERRORMSG(-1, "CServer::SendAllDynamicObjectsPositions()", "Unable to send net message.");
		}

	for(int i=0; i<(int)Physics->dynamicObjects.size(); i++)
	{
		ObjMat = Physics->dynamicObjects[i]->Matrix();
		ObjPos = ObjMat.m_posit;
		if (ObjPos.m_x < 0) ObjPos.m_x = 0;
		if (ObjPos.m_z < 0) ObjPos.m_z = 0;
		if (ObjPos.m_x > MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_x = MapObj->uiSegsCountX * TERRAIN_PLATE_WIDTH - 0.1f;
		if (ObjPos.m_z > MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f) ObjPos.m_z = MapObj->uiSegsCountZ * TERRAIN_PLATE_WIDTH - 0.1f;
		ObjMat.m_posit = ObjPos;
		hr = msg.setNetMsg(NETMSG_OBJECT_POSITION,i,
			(BYTE *) &ObjMat,
			sizeof(dMatrix));
		if (FAILED(hr) )
			ERRORMSG(-1, "CServer::SendAllDynamicObjectsPositions()", "Unable to set net message.");
		hr = netServer->SendMsg(&msg);
		if (FAILED(hr) )
			ERRORMSG(-1, "CServer::SendAllDynamicObjectsPositions()", "Unable to send net message.");
	}

	return ERRNOERROR;
}

// Changes server state and performs associated operations
HRESULT CServer::ChangeServerState(int NewState)
{
	// check whether it's not needed to:
	// 1) clear multiplayer game info
	// 2) stop session
	// 3) stop ping
	// 4) stop broadcast
	if (serverState == SS_MG_START ||
		serverState == SS_MG_START_LOAD ||
		serverState == SS_GAME_PLAY ||
		serverState == SS_SG_START)
	{
		if (NewState != SS_MG_START &&
			NewState != SS_MG_START_LOAD &&
			NewState != SS_GAME_PLAY &&
			NewState != SS_SG_START)
		{
			StopSession();
			StopPing();
			StopBroadcast();
			MultiPlayerInfo.Clear();
		}
	}

	// Check if partial clear of multiplayer game info is needed
	if (serverState == SS_GAME_PLAY && NewState == SS_MG_START)
		MultiPlayerInfo.Clear(TRUE);


	serverState = NewState;

	return ERRNOERROR;
}

// Checks whether all players are loaded
HRESULT CServer::CheckAllPlayersLoaded()
{
	BOOL AllPlayersLoaded;
	BOOL TempB, PlayerAI;
	UINT i;
	Network::CNetMsg msg;
	HRESULT hr;

	// check if state is loading
	if (serverState != SS_MG_START_LOAD ) return 0;

	// check if all players are loaded
	AllPlayersLoaded = TRUE;
	for (i = 0; i < players.size(); i++)
	{
		MultiPlayerInfo.GetPlayerActive(players[i], TempB);
		MultiPlayerInfo.GetPlayerAI(players[i], PlayerAI);
		if (TempB && ! PlayerAI)
		{
			MultiPlayerInfo.GetPlayerLoaded(players[i], TempB);
			if (! TempB)
			{
				AllPlayersLoaded = false;
				break;
			}
		}
	}

	// All players loaded -> inform players
	if (AllPlayersLoaded)
	{
		// Send all static objects positions
		hr = SendAllStaticObjectsPositions();
		if (FAILED(hr) )
		{
			DisconnectPlayer();
			ERRORMSG(0, "CServer::CheckAllPlayersLoaded()","All players disconnected due to unable to send static objects positions");
		}

		hr = SendAllDynamicObjectsPositions();
		if (FAILED(hr) )
		{
			DisconnectPlayer();
			ERRORMSG(0, "CServer::CheckAllPlayersLoaded()", "All players disconnected due to unable to send dynamic objects positions");
		}

		// Send first checkpoint position
/*
//RACER MOD ONLY:
		if (MultiPlayerInfo.GetLapMode() && CheckpointsPositions.size() != 0)
		{
			hr = MultiPlayerInfo.SetMsgCheckpointPosition(&msg, 
				D3DXVECTOR3(CheckpointsPositions[0].x, CheckpointsPositions[0].y, CheckpointsPositions[0].z) );
			if (FAILED(hr) )
			{
				DisconnectPlayer();
				ERRORMSG(0, "CServer::CheckAllPlayersLoaded()", "All players disconnected due to unable to set net message");
			}
			hr = netServer->SendMsg(&msg);
			if (FAILED(hr) )
			{	
				DisconnectPlayer();
				ERRORMSG(0, "CServer::CheckAllPlayersLoaded()", "All players disconnected due to unable to send net message");
			}
		}
*/

		// perform game MOD custom actions
		hr = pGameMod->ServerSendCustomInitialData();
		if ( hr )
		{
			DisconnectPlayer();
			ERRORMSG(0, "CServer::CheckAllPlayersLoaded()", "All players disconnected due to unable to set net message.");
		}


		// send info to clients that all players are loaded
		hr = msg.setNetMsg(NETMSG_PLAYER_LOADED, NULL, NULL);
		if (FAILED(hr) )
		{
			DisconnectPlayer();
			ERRORMSG(0, "CServer::CheckAllPlayersLoaded()", "All players disconnected due to unable to set net message");
		}
		hr = netServer->SendMsg(&msg);
		if (FAILED(hr) )
		{
			DisconnectPlayer();
			ERRORMSG(0, "CServer::CheckAllPlayersLoaded()", "All players disconnected due to unable to send net message");
		}
		
		// Set ping back to normal time
		StopPing();
		StartPing();

		ChangeServerState(SS_GAME_PLAY);
		// Set state to everything loaded
		MultiPlayerInfo.SetState(CMultiPlayerGameInfo::PHASE_FIVE);
/*
//RACER MOD ONLY:
		bControl_blocked=TRUE;
		Server->timerPtr=SetTimer(0,0,100,StartingProcedure);
		iStartCounter = 0;
*/
		// init timer
		lastTime = Timer->GetRunTime();
	}

	return ERRNOERROR;
};

void CALLBACK CServer::StartingProcedure(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time){
/*
//RACER MOD ONLY:
	Network::CNetMsg msg;
	if(Server->iStartCounter == 0) Server->iStartCounter = Server->Timer->GetRunTime();
	int aktdiv = (Server->Timer->GetRunTime() - Server->iStartCounter)/ (int) START_DELAY ;
	
	if(aktdiv < 1){
		
		msg.setNetMsg(NETMSG_SHOW_DIALOG_3,NULL,NULL);
	}
	else if(aktdiv < 2){
		msg.setNetMsg(NETMSG_SHOW_DIALOG_2,NULL,NULL);
	}
	else if(aktdiv < 3){
		msg.setNetMsg(NETMSG_SHOW_DIALOG_1,NULL,NULL);
	}
	else if(aktdiv < 4){
		Server->bControl_blocked=FALSE;
		msg.setNetMsg(NETMSG_SHOW_DIALOG_GO,NULL,NULL);
	}
	else {
		KillTimer(NULL, Server->timerPtr);
		msg.setNetMsg(NETMSG_HIDE_DIALOG,NULL,NULL);
		Server->iStartCounter = 0;
	}
	Server->netServer->SendMsg(&msg);
*/
};

// Computes check points positions
HRESULT CServer::ComputeCheckpointsPositions()
{
	resManNS::Map * MapObj;
	resManNS::MainObject * MainObj;
	resManNS::Waypoints * WaypointObj;
	AI_NS::V3 Checkpoint;
	UINT OnePastMiddleIndex;
	AI_NS::V3 WaypointDistance;
	resManNS::RESOURCEID ObjID;
	
	UINT i, j, k;
	UINT Distance;
	UINT WayPos = 0;
	BOOL WaypointMatch, AllWaypointsMatched;
	float x, y, z;
	UINT TrackPos = 0;
	UINT SquareTrackPos;

	// Get map object
	MapObj = ResourceManager->GetMap(MultiPlayerInfo.GetMapID() );
	if (! MapObj)
		ERRORMSG(-1, "CServer::ComputeCheckpointsPositions()", "Unable to retrieve map object in order to compute checkpoints");

	// Clear
	CheckpointsPositions.clear();
	NearestWaypoints.clear();

	if (Waypoints.size() == 0) return ERRNOERROR;
	
	Distance = CheckpointDistance;
	WaypointDistance = Waypoints[0].point;
	// itarate through all objects and generate checkpoints in appropriate distance
	for (i = 0; i < MapObj->uiPathPointsCount; i++)
	{
		ObjID = MapObj->pridObjects[MapObj->puiPlacedObjectIndexes[MapObj->puiPathPointIndexes[i]]];
		MainObj = ResourceManager->GetMainObject(ObjID);
		
		WaypointMatch = FALSE;
		SquareTrackPos = TrackPos;
		// try set by set of waypoint indexes and try to map it on the path
		// in order to find right set of waypoints used in the path
		for (j = 0; j < MainObj->pvWaypointsIDs->size(); j++)
		{
			TrackPos = SquareTrackPos;
			AllWaypointsMatched = TRUE;
			WaypointObj = ResourceManager->GetWaypoints(MainObj->pvWaypointsIDs->at(j));
			for (k = 0; k < WaypointObj->x->size(); k++)
			{
				// Recompute waypoint x,z coords into world coords
				x = WaypointObj->x->at(k);
				y = WaypointObj->y->at(k);
				z = WaypointObj->z->at(k);
				AI->AdjustXYZ(x, y, z, i);

				// Test against track
				if (TrackPos == Waypoints.size() ) break;
				if ( abs(Waypoints[TrackPos].point.x - x) < 0.001 && abs(Waypoints[TrackPos].point.y - y) < 0.001 && abs(Waypoints[TrackPos].point.z - z) < 0.001 )
				{
					TrackPos++;
				}
				else 
				{
					AllWaypointsMatched = FALSE;
					break;
				}
			}

			if (AllWaypointsMatched)
			{	
				WaypointMatch = TRUE;
				break;
			}
		}

		if (WaypointMatch)
		{
			// we have in WaypointObj set of waypoints, which matched
			// now we will compute distaces between waypoints and will
			// control whether it exceed checkpoint distance or is
			// on object which must have a checkpoint
			// add checkpoint
			if (Distance >= CheckpointDistance || MainObj->bHasCheckpoint)
			{
					// find point from which starting object waypoint has approx. same distace as
					// ending object waypoint (through waypoints)
					Checkpoint = ComputeObjectCheckpoint(&Waypoints, SquareTrackPos, (UINT) WaypointObj->x->size(), OnePastMiddleIndex);
					CheckpointsPositions.push_back(Checkpoint);
					NearestWaypoints.push_back(OnePastMiddleIndex);
					Distance = (UINT) Checkpoint.Distance(Waypoints[OnePastMiddleIndex].point);
					WaypointDistance = Waypoints[OnePastMiddleIndex].point;

					// compute distance till end of the square
					for (k = OnePastMiddleIndex; k < SquareTrackPos + WaypointObj->x->size(); k++)
					{
						Distance += (UINT) WaypointDistance.Distance( Waypoints[k].point);
						WaypointDistance = Waypoints[k].point;
					}
			}
			else
			{
				for (j = SquareTrackPos + 1; j < SquareTrackPos + WaypointObj->x->size(); j++)
				{
					Distance += (UINT) WaypointDistance.Distance( Waypoints[j].point);
					WaypointDistance = Waypoints[j].point;
					
					if (Distance >= CheckpointDistance)
					{
						// find point from which starting object waypoint has approx. same distace as
						// ending object waypoint (through waypoints)
						Checkpoint = ComputeObjectCheckpoint(&Waypoints, SquareTrackPos, (UINT) WaypointObj->x->size(), OnePastMiddleIndex);
						CheckpointsPositions.push_back(Checkpoint);
						NearestWaypoints.push_back(OnePastMiddleIndex);
						Distance = (UINT) Checkpoint.Distance(Waypoints[OnePastMiddleIndex].point);
						WaypointDistance = Waypoints[OnePastMiddleIndex].point;

						// compute distance till end of the square
						for (k = OnePastMiddleIndex; k < SquareTrackPos + WaypointObj->x->size(); k++)
						{
							Distance += (UINT) WaypointDistance.Distance( Waypoints[k].point);
							WaypointDistance = Waypoints[k].point;
						}

						break;
					}
				}
			}
			
			// add distance between last waypoint and first waypoint of the next square
			if (Waypoints.size() > SquareTrackPos + WaypointObj->x->size())
			{
				Distance += (UINT) WaypointDistance.Distance( Waypoints[SquareTrackPos + WaypointObj->x->size() ].point);
				WaypointDistance = Waypoints[SquareTrackPos + WaypointObj->x->size() ].point;
			}
			
		}
		else 
			continue;

	}


	

	return ERRNOERROR;
}

// Computes center of object according to x and z using map positions
// WaypointCount >= 1
AI_NS::V3 CServer::ComputeObjectCheckpoint(AI_NS::WVec * Waypoints, UINT StartWaypoint, UINT WaypointCount, UINT & OnePastMiddleIndex)
{
	UINT i;
	UINT Distance, HalfDistance;
	AI_NS::V3 Waypoint;
	
	if (WaypointCount == 1) 
	{
		OnePastMiddleIndex = StartWaypoint;
		return Waypoints->at(StartWaypoint).point;
	}

	// Compute distance from start to end
	Distance = 0;
	Waypoint = Waypoints->at(StartWaypoint).point;
	for (i = StartWaypoint + 1; i < StartWaypoint + WaypointCount; i++)
	{
		Distance += (UINT) Waypoint.Distance(Waypoints->at(i).point);
		Waypoint = Waypoints->at(i).point;
	}

	// find half distance
	HalfDistance = Distance / 2;
	Distance = 0;
	Waypoint = Waypoints->at(StartWaypoint).point;
	for (i = StartWaypoint + 1; i < StartWaypoint + WaypointCount; i++)
	{
		Distance += (UINT) Waypoint.Distance(Waypoints->at(i).point);
		Waypoint = Waypoints->at(i).point;
		if (Distance >= HalfDistance)
		{
			// interpolate between this and previous point
			OnePastMiddleIndex = i;
			return Waypoints->at(i -1).point.Interpolate(Waypoint, 0.5f);
		}
	}
	
	// Only when something goes wrong
	OUTS("Computation of object checkpoint is not correct, this is bad.", 1);
	OnePastMiddleIndex = StartWaypoint;
	return Waypoints->at(StartWaypoint).point;
};


// Inits AI after map is loaded
HRESULT	CServer::InitAI()
{
/*
//RACER MOD ONLY:
	int i;
	BOOL Active, PlayerAI;
	HRESULT hr;
	AI_NS::InitStruct InitAIStruct;

	if (AI) SAFE_DELETE(AI);


	// Create AI
    OUTMSG("Creating AI",1);
	AI = new AI_NS::CSteeringAI;
	if (!AI) 
		ERRORMSG(ERROUTOFMEMORY, "CServer::InitAI()", "Not enough memory to initialize AI.");

    OUTMSG("Filling Initstruct",1);
	InitAIStruct.pRM = ResourceManager;
	InitAIStruct.sMapName = MultiPlayerInfo.GetMapFileName();
	InitAIStruct.pPhysics = Physics;
    InitAIStruct.iLapCount = MultiPlayerInfo.GetNumberOfLaps();

	// AI car indexes
    OUTMSG("Setting car indexes",1);
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		MultiPlayerInfo.GetPlayerActive(i, Active);
		if (Active)
		{
			MultiPlayerInfo.GetPlayerAI(i, PlayerAI);
			if (PlayerAI)
				InitAIStruct.vCI.push_back(i);
		}
	}

	// Init AI
    OUTMSG("Initialiazing AI",1);
	hr = AI->InitAI(InitAIStruct);
	if (FAILED(hr) )
		ERRORMSG(hr, "CServer::InitAI()", "Error while initializing AI.");

	// Obtain waypoints on the track
	Waypoints.clear();
    // try optimized pathfinding
    OUTMSG("Trying to find optimized waypoint path",1);
	hr = AI->FindBestWaypointPath(Waypoints, true);
    // failed, try safer and slower version
    if (FAILED(hr) ) {
        OUTMSG("Trying to find non-optimized waypoint path",1);
        hr = AI->FindBestWaypointPath(Waypoints, false);
    }
	if (FAILED(hr) )
		ERRORMSG(hr, "CServer::InitAI()", "Unable to get waypoints vector.");
*/
	return ERRNOERROR;
};


// Updates checkpoint status of all cars
// Sends positions of new checkpoints to the client
HRESULT CServer::UpdateGameStatus(int dt)
{
	int Checkpoint;
	BOOL FromStart;
	BOOL PlayerActive, PlayerAI;
	BOOL CanUpdate;
	UINT i;
	float Distance;
	dMatrix CarMatrix;
	AI_NS::V3 CheckpointPos;
	Network::CNetMsg Msg;
	int LapsGone;
	float CarSpeed;
	int CarGear;
	int Temp;
	static UINT Time = 0;


	// perform the game MOD update...
	pGameMod->UpdateGameStatus( (APPRUNTIME) dt );

	// ... and then perform all the common stuff:

	// Check if update is needed
	Time += (UINT) dt;
	if (Time < GameStatusUpdateInterval) return ERRNOERROR;
	Time = 0;


	// Send car speeds and gears ALSO the RPMs
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
		if (Physics->cars[i] && PlayerActive)
		{
			// car speed
			CarSpeed = Physics->cars[i]->Speed();
			Msg.setNetMsg(NETMSG_CAR_SPEED,(BYTE *) &CarSpeed, sizeof(float) );
			netServer->SendMsg(&Msg, i);

			// car engine RPM
			CarSpeed = Physics->cars[i]->engine->GetCurrRPM();
			Msg.setNetMsg(NETMSG_ENGINE_RPM,(BYTE *) &CarSpeed, sizeof(float) );
			netServer->SendMsg(&Msg, i);

			// car engine gear
			CarGear = Physics->cars[i]->Gear(); 
			Msg.setNetMsg(NETMSG_CAR_GEAR,(BYTE *) &CarGear, sizeof(int) );
			netServer->SendMsg(&Msg, i);
		}
	}


	/*
	//RACER MOD ONLY:
	if (!AI) return ERRNOERROR;
	if (CheckpointsPositions.size() ==  0) return ERRNOERROR;
	if (! MultiPlayerInfo.GetLapMode() ) return ERRNOERROR;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
		MultiPlayerInfo.GetPlayerAI(i, PlayerAI);
		if (PlayerActive)
		{
			// Update checkpoint status, if player is not AI
			if (!PlayerAI)
			{
				FromStart = FALSE;
				MultiPlayerInfo.GetLastCheckpoint(i, Checkpoint);
				MultiPlayerInfo.GetCanUpdateCheckpoints(i, CanUpdate);
				if (CanUpdate)
				{
					Checkpoint++; // next checkpoint
					if (Checkpoint >= (int) CheckpointsPositions.size() )
					{
						Checkpoint = 0;
						FromStart = TRUE;
					}

					// Check whether player is within checkpoint sphere
					// and so player passed that checkpoint
					CarMatrix = Physics->cars[i]->Matrix();
					CheckpointPos = CheckpointsPositions[Checkpoint];
					Distance = (CarMatrix.m_posit.m_x -  CheckpointPos.x) * (CarMatrix.m_posit.m_x - CheckpointPos.x) +
						(CarMatrix.m_posit.m_y - CheckpointPos.y) * (CarMatrix.m_posit.m_y - CheckpointPos.y) +
						(CarMatrix.m_posit.m_z - CheckpointPos.z) * (CarMatrix.m_posit.m_z - CheckpointPos.z);

					if (Distance <= CheckpointSphere * CheckpointSphere)
					{
						// update and send new checkpoint position
						MultiPlayerInfo.UpdateLastCheckpoint(i, FromStart);
						// set no update
						MultiPlayerInfo.SetCanUpdateCheckpoints(i, FALSE);

						Checkpoint++;
						if (Checkpoint >= (int) CheckpointsPositions.size() ) Checkpoint = 0;
						CheckpointPos = CheckpointsPositions[Checkpoint];
						MultiPlayerInfo.SetMsgCheckpointPosition(&Msg, D3DXVECTOR3(CheckpointPos.x, CheckpointPos.y, CheckpointPos.z) );
						netServer->SendMsg(&Msg, i);
						if (FromStart)
						{
							// Update and send info about laps gone
							MultiPlayerInfo.UpdateLapsGone(i);
							MultiPlayerInfo.GetLapsGone(i, LapsGone);
							MultiPlayerInfo.SetMsgLapsGone(&Msg,i, LapsGone);
							// Send it to all players
							netServer->SendMsg(&Msg);
						}
					}
				}
				else
				{
					// Check whether player is outside last checkpoint
					CarMatrix = Physics->cars[i]->Matrix();
					CheckpointPos = CheckpointsPositions[Checkpoint];
					Distance = (CarMatrix.m_posit.m_x -  CheckpointPos.x) * (CarMatrix.m_posit.m_x - CheckpointPos.x) +
						(CarMatrix.m_posit.m_y - CheckpointPos.y) * (CarMatrix.m_posit.m_y - CheckpointPos.y) +
						(CarMatrix.m_posit.m_z - CheckpointPos.z) * (CarMatrix.m_posit.m_z - CheckpointPos.z);

					if (Distance >= CheckpointSphere * CheckpointSphere)
						MultiPlayerInfo.SetCanUpdateCheckpoints(i, TRUE);

				}
			}
			else
			{
				// Obtain desired info about AI player
				AI->GetRaceInfo(i, Temp, LapsGone);
				// Check if correct information about laps gone is in multiplayer info
				MultiPlayerInfo.GetLapsGone(i, Temp);
				if (Temp != LapsGone)
				{
					// update laps
					MultiPlayerInfo.SetLapsGone(i, LapsGone);
					// send update to all clients
					MultiPlayerInfo.SetMsgLapsGone(&Msg, i, LapsGone);
					netServer->SendMsg(&Msg);
				}
			}
		}	
	}

	UpdateRacePositions();

	*/

	return ERRNOERROR;
};


// Updates race positions
HRESULT CServer::UpdateRacePositions()
{
/*
//RACER MOD ONLY:
	UINT i, e;
	BOOL PlayerActive;
	BOOL PlayerAI;
	int LastValidWaypoint;
	int LapsGone;
	UINT PositionWaypoint;
	AI_NS::V3 CarPosition;
	dMatrix CarMatrix;
	float SmallestDistance;
	Network::CNetMsg Msg;
	UINT InsertPosition;


	RacePositionStruct RacePositions[MAX_PLAYERS];
	RacePositionStruct RacePosition;

	if (!AI) return ERRNOERROR;

	// Clear Race positions
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		RacePositions[i].ID = -1;
		RacePositions[i].LapsGone = 1000000;
		RacePositions[i].LastWaypoint = 1000000;
		RacePositions[i].Distance = FLT_MAX;
	}


	// Iterate through all players and write down their laps gone and 
	// number of nearest waypoint along correct path
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
		if (PlayerActive)
		{
			MultiPlayerInfo.GetPlayerAI(i, PlayerAI);
			// Car position
			CarMatrix = Physics->cars[i]->Matrix();
			CarPosition = AI_NS::V3(CarMatrix.m_posit.m_x, CarMatrix.m_posit.m_y, CarMatrix.m_posit.m_z);
			if (PlayerAI)
			{
				// Obtain desired info about AI player
				AI->GetRaceInfo(i, LastValidWaypoint, LapsGone);
			}
			else
			{
				MultiPlayerInfo.GetLapsGone(i, LapsGone);
				// Obtain checkpoint
				MultiPlayerInfo.GetLastCheckpoint(i, LastValidWaypoint);
				if (LastValidWaypoint == -1)
					LastValidWaypoint = 0;
				// Obtain nearest waypoint behind checkpoint along the path
				LastValidWaypoint = NearestWaypoints[LastValidWaypoint];
			}
			
			// Find nearest waypoint and distance to it
			FindNearestWaypointAlongPath(CarPosition, LastValidWaypoint, 20 * ONE_METER, PositionWaypoint, SmallestDistance);
			
			// Create race position
			RacePosition.ID = i;
			RacePosition.LapsGone = LapsGone;
			RacePosition.LastWaypoint = PositionWaypoint;
			RacePosition.Distance = SmallestDistance;

			// sort it into race positions
			// Sorting criteria are:
			// 1) Laps gone
			// 2) "Last waypoint" reached by the car
			// 3) Distance from the last waypoint
			// find insert position
			InsertPosition = -1;
			for (e = 0; e < MAX_PLAYERS; e++)
			{
				if (RacePositions[e].ID == -1)
				{
					InsertPosition = e;
					break;
				}
				if (RacePositions[e].LapsGone < RacePosition.LapsGone)
				{
					InsertPosition = e;
					break;
				}
				else if (RacePositions[e].LapsGone == RacePosition.LapsGone)
				{
					if (RacePositions[e].LastWaypoint < RacePosition.LastWaypoint)
					{
						InsertPosition = e;
						break;
					}
					else if (RacePositions[e].LastWaypoint == RacePosition.LastWaypoint)
					{
						//if (RacePositions[e].Distance > RacePosition.Distance)
						//{
						//	InsertPosition = e;
						//	break;
						//}
						
					}

				}

			}
			// move all succeeding and insert
			for (e = MAX_PLAYERS - 1; e > InsertPosition; e--)
				RacePositions[e] = RacePositions[e - 1];
			RacePositions[InsertPosition] = RacePosition;
		}	
	}

	// Compare with multiplayer info race position and if not same send new race position
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (RacePositions[i].ID == -1) break;
		MultiPlayerInfo.GetRacePosition(RacePositions[i].ID, e);
		if (e != i)
		{
			MultiPlayerInfo.SetRacePosition(RacePositions[i].ID, i);
			MultiPlayerInfo.SetMsgRacePosition(&Msg, RacePositions[i].ID, i);
			netServer->SendMsg(&Msg);
		}
	}
*/
	return ERRNOERROR;
}

// Find nearest waypoint number along correct path
// Uses waypoints vector
// All info must be correst, waypoints vector must be filled with something
// MaxDistance > 0
HRESULT CServer::FindNearestWaypointAlongPath(AI_NS::V3 CarPosition, UINT RefWaypoint, float MaxDistance, UINT & NearestWaypoint, float & SmallestDistance)
{
	AI_NS::V3 BaseWaypoint;
	UINT i;
	float Distance;
	float CarDistance;
	SmallestDistance = FLT_MAX;

	BaseWaypoint = Waypoints[RefWaypoint].point;
	// check waypoints to backward (with BaseWaypoint included)
	i = RefWaypoint;
	Distance = 0;
	while (Distance <= MaxDistance)
	{
		CarDistance = Waypoints[i].point.Distance(CarPosition);
		if (CarDistance < SmallestDistance)
		{
			SmallestDistance = CarDistance;
			NearestWaypoint = i;
		}

		if (i == 0) break;
		i--;
		Distance += Waypoints[i + 1].point.Distance( Waypoints[i].point );
	}

	// check waypoints to forward (ommiting BaseWaypoint)
	i = RefWaypoint + 1;
	if (i >= Waypoints.size()) return ERRNOERROR;
	Distance = BaseWaypoint.Distance( Waypoints[i].point );
	while (Distance <= MaxDistance)
	{
		CarDistance = Waypoints[i].point.Distance(CarPosition);
		if (CarDistance < SmallestDistance)
		{
			SmallestDistance = CarDistance;
			NearestWaypoint = i;
		}
		i++;
		if (i >= Waypoints.size()) break;
		Distance += Waypoints[i - 1].point.Distance( Waypoints[i].point );
	}
		
	return ERRNOERROR;
}
