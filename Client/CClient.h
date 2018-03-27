#pragma once

#include "stdafx.h"
#include "..\Network\CNetClient.h"
#include "..\Network\CNetMsg.h"
#include "..\Network\CommonMessages.h"
#include "..\Globals\Configuration.h"
#include "..\Globals\NameValuePair.h"
#include "..\Graphic\Graphic.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\GraphicObjects\Scene.h"
#include "..\GraphicObjects\GrObjectMesh.h"
#include "..\GraphicObjects\GrObjectTerrainPlate.h"
#include "..\GameMods\GameMod.h"
#include "..\GameMods\ModWorkshop.h"
#include "MapInit.h"
#include "HUD.h"
#include "Sound.h"
#include "CMultiplayerGameInfo.h"



#define CS_SG_START				0x10
//#define CS_SG_WAITING_FOR_CAR 0x11
//#define CS_SG_WAITING_FOR_MAP 0x12
#define CS_GAME_PLAY			0x20
#define CS_MG_ENUMERATE_HOSTS	0x40
#define CS_MG_START				0x110
#define CS_MG_START_LOAD		0x120
#define CS_RACE_RESULT			0x130
#define CS_GROUP_MASK			0xff0
#define CS_NULL					0

extern CClient				*	Client;

	// PJ //////////////////////////////
	#define CAM_COUNT		7
	enum eCameraType {
		CAM_BEHIND,
		CAM_BEHIND_1,
		CAM_LEFTSIDE,
		CAM_REVERSE,
		CAM_RIGHTSIDE,
		CAM_FREE,
		CAM_FLY_BY
	};

class CClient{
	
	private:
		HWND hWnd;
		graphic::CGrObjectMesh * vCarsMesh[MAX_PLAYERS];
		std::vector< graphic::CGrObjectMesh *> vObjectsMesh;
	//  std::vector< DPNMSG_ENUM_HOSTS_RESPONSE > vServerList;
	
	public:
		CClient();
		~CClient();
		//std::vector< DPNMSG_ENUM_HOSTS_RESPONSE > vServerList;

	
		CHUD				  HUD; // Head-up display
		CSoundSystem		  Sound; // Game sound system
		DWORD				  ping;

		// Is called, when new song starts playing
		static void SongStaticCallback(void *);
		// Is called, when new song starts playing
		void SongCallback();
		// Is called N seconds after new song starts playing
		static void SongSecondsStaticCallback(void * Ptr);
		void SongSecondsCallback();
		
		// Timer
		CTimeCounter Timer;

		// Sets start times for race
		void SetStartTimes();

		Network::CNetClient	*	GetNetClient() const { return netClient; };

	private:
		Network::CNetClient	* netClient;
		graphic::CGraphic	* Graphic;
		graphic::CScene		* Scene;
		resManNS::CResourceManager	* ResourceManager;
		
		


	public:
	//	int clientState;

		HRESULT			init(	graphic::CGraphic *,
								graphic::CScene *,
								resManNS::CResourceManager *,
								HWND hWnd,
								CGameInfo * pGameInfo,
								CMODWorkshop * _pMODWorkshop,
								void (CALLBACK * LoadingProgress) (CString) ); // initialization CClient	
		void			SetGameMOD( CGameMod * _pMOD ) { pGameMod = _pMOD; };
		CGameMod	*	GetGameMOD() const { return pGameMod; };
		HRESULT			initHUD();
		HRESULT			release();
		HRESULT			ConnectToLocalServer(); //starting single player
		HRESULT			ConnectToServer(CString ip,u_short port);
		HRESULT			SendControls(char *pKeys,DWORD size);
		BOOL			isConnected();
		inline BOOL		IsSinglePlayer() {return SinglePlayer; }
		// changes client state + performs various operations associated with client state change
		HRESULT			ChangeState(int NewState);
		// returs client state
		inline int		GetClientState() {return clientState; }

		// ping sending
		static void CALLBACK PingTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time);
		int				SendPing();
		void			ReceivePing();
		void			StartPing(int ptime=10000);
		void			StopPing();

		void CameraUpdate (dMatrix matrix);

		// Send return to lobby request to server
		inline void		SendReturnToLobbyRequest()
		{
			Network::CNetMsg Msg;
			BOOL ToLobby = TRUE;
			Msg.setNetMsg(NETMSG_RETURNTOLOBBY, (BYTE *) &ToLobby, sizeof(BOOL) );
			netClient->SendMsg(&Msg);
		}

		// Enumerating hosts functions 
		HRESULT			SendServersInfoDemand();
		HRESULT			StartBroadcast();
		HRESULT			StopBroadcast();

		HRESULT			ClientStartMultiLoad();
		// Loads all needed things for single player game
		HRESULT			ClientStartSingleLoad();
		


		// Updates player status message
		HRESULT			UpdatePlayerStatus(CString & PlayerName, CString & CarFileName, int TeamID, BOOL Ready = false);

		// Adds chat message and send it to server
		HRESULT			AddChatMessage(CString & ChatMessage);

		// Disconnect from server
		HRESULT			DisconnectFromServer();

		// SERVER LIST MAINTENANCE
		// Adds server to the list
		HRESULT			AddServer( Network::SERVER_INFO & ServerInfo);

		// Updates server list in order to exclude dead servers
		// TimeOut is in miliseconds
		HRESULT			UpdateServers(int TimeOut = 10000);

		// Dialog server info
		struct DialogServerInfo
		{
			char name[MAX_GAME_NAME_LENGTH];
			char type[MAX_GAME_NAME_LENGTH]; // specifies the game MOD = game type
			u_short port;
			u_long addr;
		};

		// Structure to maintain server info updates
		struct CServerInfoWithTime
		{
			DialogServerInfo ServerInfo;
			DWORD Time;
			inline bool operator<(const CServerInfoWithTime & ServerInfo2) const
			{ return (*this).ServerInfo.addr < ServerInfo2.ServerInfo.addr; }
		};

		// Server list with time
		set<CServerInfoWithTime> ServerList;

		graphic::CScene * GetScene(){ return Scene; };
		inline std::vector< graphic::CGrObjectMesh *> * GetvObjectsMesh(){ return &vObjectsMesh;}
		inline graphic::CGrObjectMesh ** GetvCarsMesh() {return vCarsMesh; }
	
		//Functions to load
		
		//Message handlers
		LRESULT			ClientMessageHandler(WPARAM wParam, LPARAM lParam); 

		// Multiplayer game info - used to store all important data for connection with server
		CMultiPlayerGameInfo MultiPlayerInfo;
	
	private:
		
		HRESULT setLook();
		//Message handlers 
		HRESULT ClientMultiEnumMH(WPARAM, LPARAM);
		HRESULT ClientMultiStartMH(WPARAM, LPARAM);
		HRESULT ClientSingleStartMH (WPARAM, LPARAM);		
		HRESULT ClientGameMH(WPARAM, LPARAM);
		HRESULT ClientMultiStartLoadMH(WPARAM, LPARAM);
		HRESULT	ClientRaceResultMH(WPARAM, LPARAM);
		
		// camera updating ////////////// by Venca_X
		inline void UpdateCam_Smooth( const D3DXMATRIX &);

	private:
		DWORD				pingtime;
		UINT_PTR			PingTimer;
		resManNS::RESOURCEID MapID;
		CGameMod		*	pGameMod;
		CMODWorkshop	*	pMODWorkshop;
		UINT				CarNumber;
		// Client state
		int					clientState;
		// True, when game is single player
		BOOL				SinglePlayer;

		// Loading progress callback
		void (CALLBACK * ProgressCallback) (CString);

		//#define CAM_BEHIND_1	1
		//#define CAM_FREE		2
		//#define CAM_LEFTSIDE	3
		//#define CAM_RIGHTSIDE	4
		//#define CAM_FLY_BY		4


		#define CAMERA_TIMER	200
		
		//bool buttonState;
		dVector oldPos;
		UINT cameraTimer;
		bool behindCamera;
		public:
		enum eCameraType cameraType;
		D3DXMATRIX m_posMatrix;
		
		/////////////////////////////////////////////////////////////////////
		// By Venca_X for camera
		D3DXVECTOR4 lastCarPos;
		D3DXVECTOR4 lastCameraPos;

		// cam properties
			
		// The distances in the x-z plane to the target
		float m_distance_min;
		float m_distance_shift;
		// Height we want the camera above the target
		float m_cam_height;
		// speed of reciding of camera according run speed
		float m_reciding_speed;
		// speed of floating of camera. 1 means emediately change to new pos without lerp
		// lower vals-> slower floating . Ideal value is 0.1
		float m_camera_agressivness;
		//////////////////////////////////////////////////////////////////////


		dVector m_rrrPos;
		
		void ChangeCamera(void)
		{
					if (cameraType == CAM_FLY_BY)
						cameraType = CAM_BEHIND;
					else
						cameraType = (eCameraType) ((int)cameraType + 1);
	
					// initial values. Theese are nice, Don't change it!!!
					switch (cameraType)
					{
					case CAM_BEHIND:
						m_distance_min = 5.0f;
						m_distance_shift = 5.0f;
						m_cam_height = 2.0f;
						m_reciding_speed = 0.000005f;						
						m_camera_agressivness = 0.02f;;	//for debuging use 0.2, for release 0.06
						break;

					case CAM_BEHIND_1:
						m_distance_min = 7.0f;
						m_distance_shift = 8.0f;
						m_cam_height = 2.5f;
						m_reciding_speed = 0.000005f;
						m_camera_agressivness = 0.02f;;
						break;

					case CAM_REVERSE:
						m_distance_min = -8.0f;
						m_distance_shift = 8.0f;
						m_cam_height = 2.0f;
						m_reciding_speed = 0.000005f;
						m_camera_agressivness = 0.02f;;
						break;
					}
					
					//buttonState = true;
					behindCamera = false;

					cameraTimer = CAMERA_TIMER;
		}
};
extern CClient * Client;
