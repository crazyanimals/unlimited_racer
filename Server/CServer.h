#pragma once


#include "stdafx.h"
#include "..\Network\CNetServer.h"
#include "..\Network\CNetMsg.h"
#include "..\Network\CommonMessages.h"
#include "..\Globals\Configuration.h"
#include "..\Physics\Physics.h"
#include "..\Client\MapInit.h"
#include "..\main\controls.h"
#include "..\Client\CMultiplayerGameInfo.h"
#include "..\AI\CSteeringAI.h"
#include "..\GameMods\GameMod.h"
#include "..\GameMods\ModWorkshop.h"


#define SS_GROUP_MASK 0xff0
#define SS_SG_START 0x10
#define SS_MG_START 0x110
#define SS_MG_START_LOAD 0x120
#define SS_GAME_PLAY 0x20

#define START_DELAY 1600



class CServer{
		
	private:
		HWND					hWnd;
		Network::CNetServer	*	netServer;
		physics::CPhysics	*	Physics;
		resManNS::CResourceManager * ResourceManager;
	
		APPRUNTIME			lastTime;
		CGameMod		*	pGameMod;
		CMODWorkshop	*	pMODWorkshop;
	
	public:
		inline CServer()
		{
			netServer = 0;
			Timer = 0;
			lastTime = 0;
			serverState = 0;
			IPList = NULL;
			AI = NULL;
			CheckpointDistance = (UINT) (50.0f * ONE_METER);
			CheckpointSphere = 13 * ONE_METER;
			GameStatusUpdateInterval = 100;
//RACER MOD ONLY:			bControl_blocked = 0;
		}

		inline ~CServer()
		{
			SAFE_DELETE(netServer);
			SAFE_DELETE(Timer);
			SAFE_DELETE(AI);
			// free IP list
			if (IPList)
			{
				freeaddrinfo(IPList);
				IPList = NULL;
			}
		}
		
	public:	 
		CString				sSessionName;
		//RACER MOD ONLY: int					iStartCounter;
		UINT_PTR			timerPtr;
		CTimeCounter	*	Timer;

		D3DXVECTOR4 camera_position;

		HRESULT				init(	physics::CPhysics *,
									HWND hWnd, 
									resManNS::CResourceManager * ResManager, 
									CGameInfo * pGameInfo,
									CMODWorkshop * _pMODWorkshop,
									void (CALLBACK *LoadingProgress) (CString) ); // initialization  CServer
		physics::CPhysics *	GetPhysics() { return Physics; };
		void				SetGameMOD( CGameMod * _pMOD ) { pGameMod = _pMOD; };
		CGameMod		*	GetGameMOD() const { return pGameMod; };
		HRESULT				StartServerSingle();
		// Starts server multiplayer
		// Returns -2, when selected map is corrupted
		HRESULT				StartServerMulti(CString sessionName, CString MapFileName, CGameMod * pGameMod);
		HRESULT				StartServerMultiLoad();
		HRESULT				StopSession();
	
		// Changes server state and performs associated operations
		HRESULT				ChangeServerState(int NewState);
		// returns server state
		inline int			GetServerState() {return serverState; }
		
		// Sends, that server returned to lobby, so all others must return to lobby
		inline void			SendServerReturnedToLobby()
		{
			Network::CNetMsg Msg;
			BOOL ToLobby = TRUE;

			MultiPlayerInfo.SetServerReturnedToLobby(TRUE);
			Msg.setNetMsg(NETMSG_RETURNTOLOBBY, (BYTE *) &ToLobby, sizeof(BOOL) );
			netServer->SendMsg(&Msg);
		}

		Network::CNetServer	* GetNetServer() const { return netServer; };
		

		HRESULT				updateGame();
		LRESULT				ServerMessageHandler( WPARAM wParam, LPARAM lParam);
		
		void				StartPing(int pingtime=10000);
		void				StopPing();
		int					SendPing(int id=-1);
		HRESULT				ReceivePing(int id);
		BOOL				isConnected(int id){return netServer->isConnected(id);};
		HRESULT				DisconnectPlayer(int id=-1);
		static void CALLBACK PingTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time);

		int					AddPlayer();
		int					AutoAssignTeam();

		// Adds chat message and send it to all clients
		HRESULT				AddChatMessage(CString & ChatMessage);
		
		// Updates game settings and send it to all clients
		HRESULT				UpdateGameSettings( int Hours, int Minutes );

		// Checks whether all players are loaded
		HRESULT				CheckAllPlayersLoaded();

		// Computes check points positions
		HRESULT				ComputeCheckpointsPositions();
		// Inits AI after map is loaded
		HRESULT				InitAI();

		///////////////////////////////////////
		// Broadcast host enumeration:
		
		HRESULT				SendBroadcast();
		HRESULT				StartBroadcast();
		HRESULT				StopBroadcast();
		
		// Kick player ID from server
		HRESULT				Kick(unsigned int ID);

		// sends positions of static objects
		HRESULT				SendAllStaticObjectsPositions();

		// sends positions of dynamic objects and cars
		HRESULT				SendAllDynamicObjectsPositions();
		
//RACER MOD ONLY:		BOOL				bControl_blocked;
		// Multiplayer game info - used to store all important data for connection with client
		CMultiPlayerGameInfo MultiPlayerInfo;

		// Returns checkpoint distance
		inline UINT			GetCheckpointDistance() { return CheckpointDistance; }
		// Sets checkpoint distance
		inline void			SetCheckpointDistance(UINT NewDistance) { CheckpointDistance = NewDistance; }

		// returns checkpoint active sphere
		inline float		GetCheckpointSphere() { return CheckpointSphere; }
		// Sets checkpoint sphere
		inline void			SetCheckpointSphere(float NewSphere) { CheckpointSphere = NewSphere; }

		// returns game status update interval
		inline UINT			GetGameStatusUpdateInterval() { return GameStatusUpdateInterval; }
		// Sets game status update interval
		inline void			SetGameStatusUpdateInterval(UINT NewInterval)
		{
			if (NewInterval >= 10 && NewInterval < 333)
				GameStatusUpdateInterval = NewInterval;
		}
		static void CALLBACK StartingProcedure(HWND Hwnd, UINT Msg,UINT_PTR IdEvent, DWORD Time);

	private:
		struct RacePositionStruct
		{
			UINT ID;
			UINT LapsGone;
			UINT LastWaypoint;
			float Distance;
		}; 

		std::vector<int>	players;
		UINT_PTR			PingTimer;
		
		HRESULT				ServerGameMH(WPARAM, LPARAM);
		HRESULT				ServerSingleStartMH(WPARAM, LPARAM);
		HRESULT				ServerMultiStartMH(WPARAM, LPARAM);
		HRESULT				ServerMultiStartLoadMH(WPARAM,LPARAM);

		HRESULT				sendAllStartingData();

		// Server state
		int					serverState;

		// progress callback function
		void (CALLBACK *ProgressCallback) (CString);

		// IP list - list of all IPs of all networks computer is connected to
		struct addrinfo *	IPList;

		// AI and game part
		// AI
		AI_NS::CSteeringAI * AI;

		// Checkpoints positions for controling car position on track
		std::vector<AI_NS::V3> CheckpointsPositions;
	
		// Nearest waypoints on the path to the chechpoints
		std::vector<UINT>	NearestWaypoints;

		// Waypoints
		AI_NS::WVec			Waypoints;

		// approx. distance between checkpoints
		UINT				CheckpointDistance;

		// Sphere of checkpoint active area
		float				CheckpointSphere;

		// Game status update interval in ms
		UINT				GameStatusUpdateInterval;

		// Computes center of object according to x and z using map positions
		AI_NS::V3			ComputeObjectCheckpoint(AI_NS::WVec * Waypoints, UINT StartWaypoint, UINT WaypointCount, UINT & OnePastMiddleIndex);

		// Updates checkpoint status of all cars
		// Sends positions of new checkpoints to the client
		// Updates laps gone
		HRESULT				UpdateGameStatus(int dt);

		// Updates race positions
		HRESULT				UpdateRacePositions();

		// Find nearest waypoint number along correct path
		HRESULT				FindNearestWaypointAlongPath(AI_NS::V3 CarPosition, UINT RefWaypoint, float MaxDistance, UINT & NearesWaypoint, float & SmallestDistance);
};

extern CServer * Server;