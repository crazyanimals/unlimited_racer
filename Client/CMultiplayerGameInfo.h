#pragma once

/*
Multiplayer game info - contains informations needed to start multiplayer
is used to fill in multiplayer dialog
is used by server and client for communication
by Pavel Celba, Roman Margold
10. 12. 2006
*/

#include "../Globals/GlobalsIncludes.h"
#include "../Network/CNetMsg.h"
#include "../ResourceManager/ResourceManager.h"
//#include "IntegrityChecker.h"
#include "GameInfo.h"


//extern CGameInfo GameInfo;

class CMultiPlayerGameInfo
{
public:
	// Multiplayer game info states
	enum States
	{
		PHASE_ZERO,
		PHASE_ONE,
		PHASE_TWO,
		PHASE_THREE,
		PHASE_FOUR, // loading game
		PHASE_FIVE // game loaded and ready to start on all clients and server
	};


	// constructor
						CMultiPlayerGameInfo();

	// Clear all game info 
	void				Clear( BOOL OnlyGamePlayRelated = FALSE );
	HRESULT 			Init( resManNS::CResourceManager * _ResourceManager, CGameInfo * _pGameInfo );

	void				SetID( int NewID ) { Id = NewID; };
	int					GetID() const {	return Id; };
	
	void				SetGameName( CString & Name ) {	GameName = Name; };
	CString &			GetGameName() { return GameName; };
	
	void				SetState( States _State ) {	State = _State; };
	States				GetState() const { return State; };
	
	HRESULT				SetMapFileName( CString & Name ); // also checks whether file name is existing and computes map CRCs
	HRESULT				SetMapFileName( CString & Name, std::multiset<DWORD> MapCRC ); // Only sets file name along with its CRCs; no integrity check or existence check
	CString &			GetMapFileName() { return MapFileName; };
	std::multiset<DWORD> & GetMapCRCs() { return MapCRCs; };

	BOOL				CompareMapFileName( std::multiset<DWORD> & CRCs );

	HRESULT				FetchAllCars(); // Adds all available cars filenames and computes their CRCs

	unsigned int		GetCarNum() const {	return (unsigned int) AllowedCarsFileNames.size(); };

	// Compares cars filenames and their CRCs
	// Excludes every car from local allowed cars list, which is not in client file names or hasn't same CRC
	HRESULT				CompareCarsFileNames();

	// Sets all cars filenames 
	// Iterates through active player car filenames and when car doesn't exist
	// changes car name to ""
	HRESULT				SetAllowedCarsFileNames( std::vector<CString> & Names );
	std::vector<CString> & GetAllowedCarsFileNames() { return AllowedCarsFileNames; }; // Get all allowed car filenames

	// Sets client car names
	HRESULT				SetClientCarNames( unsigned int ID, std::vector<CString> & Names )
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::SetClientCarNames()", "Invalid player ID.");

		AllClientCars[ID] = Names;

		return ERRNOERROR;
	}

	// Adds client car CRC
	inline HRESULT AddClientCarCRCs(unsigned int ID, std::multiset<DWORD> & CRCs)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::AddClientCarCRCs()", "Invalid player ID.");

		AllClientCarsCRCs[ID].push_back(CRCs);

		return ERRNOERROR;
	}

	// Clear client car and CRCs
	inline HRESULT ClearClientCarAndCRCs(unsigned int ID)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::ClearClientCarAndCRCs()", "Invalid player ID.");

		AllClientCars[ID].clear();
		AllClientCarsCRCs[ID].clear();

		return ERRNOERROR;
	}

	// Get client car names size
	inline HRESULT GetClientCarsSize(unsigned int ID, unsigned int & Size)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetClientCarsSize()", "Invalid player ID.");

		Size = (unsigned int) AllClientCars[ID].size();

		return ERRNOERROR;
	}
	
	// Get client car CRCs size
	inline HRESULT GetClientCarsCRCsSize(unsigned int ID, unsigned int & Size)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetClientCarsCRCsSize()", "Invalid player ID.");

		Size = (unsigned int) AllClientCarsCRCs[ID].size();

		return ERRNOERROR;
	}

	// Sets player status; teamID should be greater or equal to zero; however, if not specified,
	//  set teamID to -1 which means "don't change"
	HRESULT SetPlayerStatus(	unsigned int ID,
								CString & PlayerName, 
								CString & PlayerCar, 
								int teamID,
								BOOL NewReady, 
								BOOL NewActive = true );

	// Get Player name
	inline HRESULT GetPlayerName(unsigned int ID, CString & PlayerName)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetPlayerName()", "Invalid player ID.");

		PlayerName = PlayerNames[ID];

		return ERRNOERROR;
	}

	// Get player car
	inline HRESULT GetPlayerCar(unsigned int ID, CString & PlayerCar)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetPlayerCar()", "Invalid player ID.");

		PlayerCar = CarFileNames[ID];

		return ERRNOERROR;
	}

	// Get player ready
	inline HRESULT GetPlayerReady(unsigned int ID, BOOL & PlayerReady)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetPlayerReady()", "Invalid player ID.");

		PlayerReady = Ready[ID];

		return ERRNOERROR;
	}

	void SetPlayerActive( unsigned int ID, BOOL NewActive );
	
	void SetPlayerTeamID( unsigned int playerID, unsigned int teamID );
	
	inline UINT GetPlayerTeamID( UINT playerID ) const 
	{
		if ( playerID >= MAX_PLAYERS ) 
			ErrorHandler.HandleError(ERRINVALIDPARAMETER,"CMultiPlayerGameInfo::GetPlayerTeamID()", "Invalid player ID.");

		return TeamID[playerID];
	}

	inline HRESULT GetPlayerActive(unsigned int ID, BOOL & PlayerActive)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetPlayerActive()", "Invalid player ID.");

		PlayerActive = Active[ID];

		return ERRNOERROR;
	}

	inline BOOL GetPlayerActive(unsigned int ID)
	{
		if ( ID >= MAX_PLAYERS ) 
			ErrorHandler.HandleError(ERRINVALIDPARAMETER,"CMultiPlayerGameInfo::GetPlayerActive()", "Invalid player ID.");

		return Active[ID];
	}

	// Get player loaded
	inline HRESULT GetPlayerLoaded(unsigned int ID, BOOL & PlayerLoaded)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetPlayerLoaded()", "Invalid player ID.");

		PlayerLoaded = Loaded[ID];

		return ERRNOERROR;
	}

	// Set player loaded
	inline HRESULT SetPlayerLoaded(unsigned int ID, BOOL PlayerLoaded)
	{
			if (ID >= MAX_PLAYERS)
				ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::SetPlayerLoaded", "Invalid player ID.");

			Loaded[ID] = PlayerLoaded;

			return ERRNOERROR;
	}


	HRESULT				SetGameSettings( int Hour, int Minutes );
	int					GetHours() const { return DayTimeHours; };
	int					GetMinutes() const { return DayTimeMinutes; };


	// Set return to lobby
	inline HRESULT SetReturnToLobby( unsigned int ID, BOOL ToLobby )
	{
		if (ID >= MAX_PLAYERS )
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::SetReturnToLobby()", "Invalid player ID.");

		ReturnToLobby[ID] = ToLobby;

		return ERRNOERROR;
	}

	// Get return to lobby
	inline HRESULT GetReturnToLobby( unsigned int ID, BOOL & ToLobby )
	{
		if (ID >= MAX_PLAYERS )
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetReturnToLobby()", "Invalid player ID.");

		ToLobby = ReturnToLobby[ID];
		
		return ERRNOERROR;
	}

	// Set server returned to lobby
	inline HRESULT SetServerReturnedToLobby(BOOL ToLobby)
	{
		ServerReturnedToLobby = ToLobby;

		return ERRNOERROR;
	}

	BOOL				GetServerReturnedToLobby() const { return ServerReturnedToLobby; };

	// Set ping time 
	inline HRESULT SetPingTime(unsigned int ID, DWORD NewPingTime)
	{
		if (ID >= MAX_PLAYERS)
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::SetPingTime()", "Invalid player ID.");

		PingTime[ID] = NewPingTime;

		return ERRNOERROR;
	}

	// Set ping 
	inline HRESULT SetPing(unsigned int ID, DWORD NewPing)
	{
			if (ID >= MAX_PLAYERS)
				ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::SetPing", "Invalid player ID.");

			Ping[ID] = NewPing;

			return ERRNOERROR;
	}

	// get ping time
	inline HRESULT GetPingTime(unsigned int ID, DWORD & ActualPingTime)
	{
			if (ID >= MAX_PLAYERS)
				ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetPingTime()", "Invalid player ID.");

			ActualPingTime = PingTime[ID];

			return ERRNOERROR;
	}

	// get ping
	inline HRESULT GetPing(unsigned int ID, DWORD & ActualPing)
	{
			if (ID >= MAX_PLAYERS)
				ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetPing()", "Invalid player ID.");

			ActualPing = Ping[ID];

			return ERRNOERROR;
	}

	inline HRESULT AddChatMessage(CString & ChatMessage)
	{
		ChatMessages.push_back(ChatMessage);

		return ERRNOERROR;
	}

	// get number of chat messages
	inline UINT GetChatMessagesNumber()
	{
		return (UINT) ChatMessages.size();
	}

	// get chat message number
	inline HRESULT GetChatMessage(unsigned int Number, CString & ChatMessage)
	{
		if (Number >= ChatMessages.size() )
			ERRORMSG(-1, "CMultiPlayerGameInfo::GetChatMessage()", "Number of chat message is too high.");

		ChatMessage = ChatMessages[Number];

		return ERRNOERROR;
	}

	// Set, whether player is AI or not
	inline HRESULT SetPlayerAI(unsigned int ID, BOOL AI)
	{
		if (ID >= MAX_PLAYERS )
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::SetPlayerAI()", "Invalid player ID.");

		IsAI[ID] = AI;

		return ERRNOERROR;
	}

	// Get, whether player is AI or not
	inline HRESULT GetPlayerAI(unsigned int ID, BOOL & AI)
	{	
		if (ID >= MAX_PLAYERS )
			ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::GetPlayerAI()", "Invalid player ID.");

		AI = IsAI[ID];

		return ERRNOERROR;
	}

	// Get map ID
	inline resManNS::RESOURCEID GetMapID() const { return MapID; };

	inline UINT GetPlayersCount() const { return PlayerCount; };
	inline UINT GetTeamPlayersCount( UINT teamID ) const { return TeamPlayerCount[teamID] ;};


public:
	// PRIVATE PROPERTIES

	// identifying properties & global setting
	CGameInfo		*	pGameInfo;
	int					Id; // Current player's id - for server it's -1
	CString				GameName; // Game name
	CString				MapFileName; // Map file name
	std::multiset<DWORD> MapCRCs; // Map CRCs
	int					DayTimeHours, DayTimeMinutes; // Time of day, when race takes event
	std::vector<CString> ChatMessages; // Chat messages
	APPRUNTIME			RaceStartTime;

	// info 'bout all players
	BOOL				Loaded[MAX_PLAYERS]; // Whether is player loaded
	BOOL				IsAI[MAX_PLAYERS]; // Whether player is computere artificial inteligence
	CString				PlayerNames[MAX_PLAYERS]; // Player names
	CString				CarFileNames[MAX_PLAYERS]; // Car file names
	BOOL				ReturnToLobby[MAX_PLAYERS]; // Whether client demands return to lobby
	BOOL				Ready[MAX_PLAYERS]; // Whether player is ready
	DWORD				PingTime[MAX_PLAYERS]; // Ping time
	DWORD				Ping[MAX_PLAYERS]; // ping send

private:
	BOOL				Active[MAX_PLAYERS];// Whether certain player position is active
	UINT				TeamID[MAX_PLAYERS]; // team ID for every player
	UINT				PlayerCount; // actual count of active players
	UINT				TeamPlayerCount[MAX_PLAYERS]; // count of players in each team (there can not be more teams than MAX_PLAYERS

public:
	// status
	BOOL				ServerReturnedToLobby; // Whether server returned to lobby
	States				State; // Inicialization state

	// server restrictions
	std::vector<CString> AllowedCarsFileNames; // All allowed car filenames
	std::vector<std::multiset<DWORD>> AllowedCarsCRCs; // allowed cars CRCs
	std::vector<CString> AllClientCars[MAX_PLAYERS]; // All car filenames from all clients
	std::vector<std::multiset<DWORD>> AllClientCarsCRCs[MAX_PLAYERS]; // CRCs of all cars of all clients - server need this for minimal set computation

	// used resources info
	resManNS::CResourceManager * ResourceManager; // Resource manager
	resManNS::RESOURCEID MapID; // map resource ID
	std::vector<resManNS::RESOURCEID> CarIDs; // loaded car IDs


	
	// PRIVATE METHODS
	HRESULT				ComputeMapCRCs(); // Computes Map CRC

	// Iterates through active player car filenames and when car doesn't exist
	//  changes car name to ""
	HRESULT				CheckCarFileNames();
	HRESULT				CheckCarFileNames(unsigned int i);
	
    // compares orderd CRC multisets
    BOOL				CompareOrderedCRCs(std::multiset<DWORD> & MultiSet1, std::multiset<DWORD> & MultiSet2) const;
    
};