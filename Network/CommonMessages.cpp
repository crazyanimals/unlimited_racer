#include "stdafx.h"
#include "CommonMessages.h"


using namespace Network;



HRESULT Network::SetMsgID( CNetMsg * Msg, int ID )
{
	HRESULT hr;

	hr = Msg->setNetMsg( NETMSG_PLAYER_ID, (BYTE *) &ID, sizeof(int) );
	if ( FAILED(hr) ) ERRORMSG(hr, "Network::SetMsgID()", "Unable to set net message.");
	
	return ERRNOERROR;
};


// Set map file name into message
HRESULT Network::SetMsgMap( CNetMsg * Msg, CString * MapName )
{
	HRESULT		hr;
	BYTE		Data[256];

	strcpy_s( (char *) Data, 256, *MapName );
	hr = Msg->setNetMsg( NETMSG_PLAYER_MAP_NAME, Data, min( MapName->GetLength() + 1, 256 ) );
	if ( FAILED(hr) ) ERRORMSG(hr, "Network::SetMsgMap()", "Unable to set net message.");

	return ERRNOERROR;
};


// Set game name message
HRESULT Network::SetMsgGameName( CNetMsg * Msg, CString * GameName )
{
	HRESULT	hr;
	BYTE	Data[256];

	strcpy_s( (char *) Data, 256, *GameName );
	hr = Msg->setNetMsg( NETMSG_GAME_NAME, Data, min( GameName->GetLength() + 1, 256 ) );
	if ( FAILED(hr) ) ERRORMSG(hr, "Network::SetMsgGameName()", "Unable to set net message.");

	return ERRNOERROR;
};


// Set game name message
HRESULT Network::SetMsgGameType( CNetMsg * Msg, CString * GameType )
{
	HRESULT	hr;
	BYTE	Data[256];

	strcpy_s( (char *) Data, 256, *GameType );
	hr = Msg->setNetMsg( NETMSG_GAME_TYPE, Data, min( GameType->GetLength() + 1, 256 ) );
	if ( FAILED(hr) ) ERRORMSG(hr, "Network::SetMsgGameType()", "Unable to set net message.");

	return ERRNOERROR;
};


// Set chat message
HRESULT Network::SetMsgChat( CNetMsg * Msg, CString * ChatMessage )
{
	HRESULT hr;
	BYTE	Data[1024];

	strcpy_s( (char *) Data, 1024, *ChatMessage );
	hr = Msg->setNetMsg( NETMSG_CHAT, Data, min( ChatMessage->GetLength() + 1, 1024 ) );
	if ( FAILED(hr ) ) ERRORMSG(hr, "Network::SetMsgChat()", "Unable to set net message.");

	return ERRNOERROR;
};


// composes a custom network message with a specified string of maximal length of 1024 characters
HRESULT	Network::SetMsgString( CNetMsg * Msg, WORD msgID, CString & text )
{
	HRESULT hr;
	BYTE	Data[1024];

	strcpy_s( (char *) Data, 1024, text );
	hr = Msg->setNetMsg( msgID, Data, min( text.GetLength() + 1, 1024 ) );
	if ( FAILED(hr ) ) ERRORMSG(hr, "Network::SetMsgString()", "Unable to set net message.");

	return ERRNOERROR;
};


// Computes and sets map CRCs message
HRESULT Network::SetMsgMapCRCs( CNetMsg * Msg, std::multiset<DWORD> * MapCRCs )
{
	HRESULT hr;

	hr = SetMsgCRCs( Msg, MapCRCs );
	if ( FAILED(hr ) ) ERRORMSG(hr, "Network::SetMsgMapCRCs()", "Unable to set net message.");
	
	return ERRNOERROR;
};


/*
//RACER MOD ONLY:
// Set checkpoint position message
HRESULT Network::SetMsgCheckpointPosition( CNetMsg * Msg, D3DXVECTOR3 * Position )
{
	HRESULT hr;

	hr = Msg->setNetMsg( NETMSG_CHECKPOINT_POSITION, (BYTE *) Position, sizeof(*Position) );
	if ( FAILED(hr ) ) ERRORMSG(hr, "Network::SetMsgCheckpointPosition()", "Unable to set net message.");

	return ERRNOERROR;
};


// set laps gone message
HRESULT	Network::SetMsgLapsGone( CNetMsg * Msg, UINT ID, int LapsGone )
{
	HRESULT		hr;
	const UINT	Size = sizeof(UINT) + sizeof(int);
	BYTE		Data[Size];

	*(UINT *)Data = ID;
	*(int *)(Data + sizeof(UINT)) = LapsGone;

	hr = Msg->setNetMsg( NETMSG_LAPS_GONE, Data, Size );
	if ( FAILED(hr ) ) ERRORMSG( hr, "Network::SetMsgLapsGone()", "Unable to set net message." );

	return ERRNOERROR;
};


HRESULT Network::SetMsgRacePosition( CNetMsg * Msg, UINT ID, UINT RacePosition )
{
	const UINT	Size = 2 * sizeof(UINT);
	BYTE		Data[Size];
	HRESULT		hr;

	((UINT *)Data)[0] = ID;
	((UINT *)Data)[1] = RacePosition;

	hr = Msg->setNetMsg( NETMSG_RACE_POSITION, Data, Size );
	if ( FAILED(hr) ) ERRORMSG(hr, "Network::SetMsgRacePosition()", "Unable to set net message.");

	return ERRNOERROR;
};
*/


HRESULT Network::SetMsgGameSettings( CNetMsg * Msg, int TimeHours, int TimeMinutes )
{
	const DWORD		Size = 2 * sizeof(int);
	BYTE			Data[Size];
	HRESULT			hr;

	((int *)Data)[0] = TimeHours;
	((int *)Data)[1] = TimeMinutes;

/*
//RACER MOD ONLY:
	(* (BOOL *) DataPtr) = Laps;
	DataPtr += sizeof(BOOL);
	(* (int *) DataPtr) = NumLaps;
*/
	hr = Msg->setNetMsg( NETMSG_MAP_SETTINGS, Data, Size );
	if ( FAILED(hr) ) ERRORMSG(hr, "Network::SetMsgGameSettings()", "Unable to set net message.");
	
	return ERRNOERROR;
};


// Sets msg to CRC message
HRESULT Network::SetMsgCRCs( CNetMsg * Msg, std::multiset<DWORD> * CRCs )
{
	DWORD	Size;
	DWORD	Count;
	DWORD * Data;
	std::multiset<DWORD>::iterator	It;
	HRESULT hr;

	Size = (DWORD) CRCs->size();
	Data = new DWORD[Size + 1];
	if ( !Data ) ERRORMSG( ERROUTOFMEMORY, "Network::SetMsgCRCs()", "Unable to allocate place for message." );
	
	(* Data) = Size;
	Count = 1;
	for ( It = CRCs->begin(); It != CRCs->end(); It++, Count++ )
		*(Data + Count) = (*It);
		
	hr = Msg->setNetMsg( NETMSG_PLAYER_CRC, (BYTE *) Data, sizeof(DWORD) * (Size + 1) );
	if ( FAILED(hr) )
	{
		SAFE_DELETE_ARRAY(Data);
		ERRORMSG(hr, "Network::SetMsgCRCs()", "Unable to set net message.");
	}

	SAFE_DELETE_ARRAY(Data);

	return ERRNOERROR;
};


HRESULT Network::SetMsgAllCarsFileNames( CNetMsg * Msg, std::vector<CString> * AllCarsFileNames )
{
	DWORD		Size;
	DWORD		StringSize;
	BYTE	*	Data;
	BYTE	*	DataPtr;
	HRESULT		hr;


	Size = sizeof(DWORD);

	// compute length
	for ( UINT i = 0; i < AllCarsFileNames->size(); i++ )
		Size += (*AllCarsFileNames)[i].GetLength() + 1 + sizeof(DWORD);

	Data = new BYTE[Size];
	if ( !Data ) ERRORMSG(ERROUTOFMEMORY, "Network::SetMsgAllCarsFileNames()", "Unable to allocate place for message.");

	(*(DWORD *)Data) = (DWORD) AllCarsFileNames->size();
	DataPtr = Data + sizeof(DWORD);
	for ( UINT i = 0; i < AllCarsFileNames->size(); i++ )
	{
		StringSize = (*AllCarsFileNames)[i].GetLength() + 1;
		(* (DWORD *) DataPtr) = StringSize;
		DataPtr += sizeof(DWORD);
		strcpy_s( (char *) DataPtr, StringSize, (*AllCarsFileNames)[i] );
		DataPtr += StringSize;
	}

	hr = Msg->setNetMsg( NETMSG_PLAYER_CAR_NAMES, Data, Size );
	if ( FAILED(hr) )
	{
		SAFE_DELETE_ARRAY(Data);
		ERRORMSG(hr, "Network::SetMsgAllCarsFileNames()", "Unable to set net message.");
	}

	SAFE_DELETE_ARRAY(Data);

	return ERRNOERROR;
};


// Set car CRCs message
HRESULT Network::SetMsgCarCRCs( CNetMsg * Msg, UINT CarNum, std::vector<std::multiset<DWORD>> * AllowedCarsCRCs )
{
	HRESULT		hr;

	if ( CarNum >= AllowedCarsCRCs->size() )
		ERRORMSG(ERRINVALIDPARAMETER, "Network::SetMsgCarCRCs()", "Car number too high.");

	hr = SetMsgCRCs( Msg, &( (*AllowedCarsCRCs)[CarNum] ) );
	if ( FAILED(hr) ) ERRORMSG(hr, "Network::SetMsgCarCRCs()", "Unable to set net message.");
	

	return ERRNOERROR;
};


// Set player status message
// it's sextet (Id, PlayerName, PlayerCar, TeamID, Ready, Active)
HRESULT Network::SetMsgPlayerStatus(	CNetMsg * Msg,
										UINT ID, 
										CString * Name,
										CString * Car,
										int TeamID,
										BOOL Ready,
										BOOL Active )
{
	BYTE *	Data;
	BYTE *	DataPtr;
	DWORD	PlayerNameSize;
	DWORD	PlayerCarSize;
	DWORD	Size;
	HRESULT hr;

	
	if ( ID >= MAX_PLAYERS ) 
		ERRORMSG(ERRINVALIDPARAMETER, "Network::SetMsgPlayerStatus()", "Invalid player ID.");

	PlayerNameSize = Name->GetLength() + 1;
	PlayerCarSize = Car->GetLength() + 1;
	Size = sizeof(ID) + PlayerNameSize + PlayerCarSize + sizeof(PlayerNameSize) + sizeof(PlayerCarSize) +
			sizeof(TeamID) + sizeof(Ready) + sizeof(Active);
	Data = new BYTE[Size];
	if ( !Data ) ERRORMSG(ERROUTOFMEMORY, "Network::SetMsgPlayerStatus()", "Unable to allocate place for message.");

	(* (unsigned int * ) Data) = ID;
	
	DataPtr = Data + sizeof(UINT);
	(* (DWORD *) DataPtr) = PlayerNameSize;

	DataPtr += sizeof(PlayerNameSize);
	strcpy_s( (char *) DataPtr, PlayerNameSize, *Name );
	
	DataPtr += PlayerNameSize;
	(* (DWORD *) DataPtr) = PlayerCarSize;

	DataPtr += sizeof(PlayerCarSize);
	strcpy_s( (char *) DataPtr, PlayerCarSize, *Car );

	DataPtr += PlayerCarSize;
	(* (int *) DataPtr) = TeamID;

	DataPtr += sizeof(TeamID);
	(* (BOOL *) DataPtr) = Ready;

	DataPtr += sizeof(Ready);
	(* (BOOL *) DataPtr) = Active;

	hr = Msg->setNetMsg( NETMSG_PLAYER_STATUS, Data,  Size );
	if ( FAILED(hr) )
	{
		SAFE_DELETE_ARRAY(Data);
		ERRORMSG(hr, "Network::SetMsgPlayerStatus()", "Unable to set net message.");
	}
	
	SAFE_DELETE_ARRAY(Data);


	return ERRNOERROR;
};





// Gets map file name from message
HRESULT Network::GetMsgName( CNetMsg * Msg, CString & Name )
{
	Name = CString( (char *) Msg->getData() );
	
	return ERRNOERROR;
};


/*
//RACER MOD ONLY:
// Get checkpoint position message
HRESULT Network::GetMsgCheckpointPosition( CNetMsg * Msg, D3DXVECTOR3 & Position )
{
	Position = *(D3DXVECTOR3 *) Msg->getData();

	return ERRNOERROR;
};


// Get laps gone message
HRESULT Network::GetMsgLapsGone( CNetMsg * Msg, UINT & ID, int & LapsGone )
{

	ID = *(UINT *) Msg->getData();
	LapsGone = * (int *)  ((BYTE *) Msg->getData() + sizeof(UINT) );

	return ERRNOERROR;
};


// Get race position message
HRESULT Network::GetMsgRacePosition( CNetMsg * Msg, UINT & ID, UINT & RacePosition )
{
	ID = * (UINT *) Msg->getData();
	RacePosition = * (UINT *) ( (BYTE *) Msg->getData() + sizeof(UINT) );

	return ERRNOERROR;
};
*/


// Gets CRCs message
HRESULT Network::GetMsgCRCs( CNetMsg * Msg, std::multiset<DWORD> & CRCs )
{
	BYTE *	Data;
	DWORD	Size;

	Data = (BYTE *) Msg->getData();
	Size = * ((DWORD *) Data);
	Data += sizeof(DWORD);

	for ( UINT i = 0; i < Size; i++, Data += sizeof(DWORD) )
		CRCs.insert( *(DWORD *) Data );
		
	return ERRNOERROR;
};


// Gets all cars filenames
HRESULT Network::GetMsgAllCarsFileNames( CNetMsg * Msg, std::vector<CString> & Names )
{
	DWORD	Size, StringSize;
	BYTE *	Data;
	CString Name;

	Data = (BYTE *) Msg->getData();
	Size = * ((DWORD *) Data);
	Data += sizeof(DWORD);

	for ( UINT i = 0; i < Size; i++ )
	{	
		StringSize = * (DWORD *) Data;
		Data += sizeof(DWORD);
		Name = CString( (char *) Data );
		Names.push_back( Name );
		Data += StringSize;
	}

	return ERRNOERROR;
};


// Get player status kvintet message
HRESULT Network::GetMsgPlayerStatus(	CNetMsg * Msg,
										UINT & ID,
										CString & Name,
										CString & Car,
										int & TeamID,
										BOOL & Ready,
										BOOL & Active )
{
	BYTE *	Data;
	DWORD	StringSize;

	Data = (BYTE *) Msg->getData();
	ID = * (UINT *) Data;
	
	Data += sizeof(UINT);
	StringSize = * (DWORD *) Data;
	
	Data +=sizeof(DWORD);
	Name = CString( (char*) Data );
	
	Data += StringSize;
	StringSize = * (DWORD *) Data;
	
	Data += sizeof(DWORD);
	Car = CString( (char*) Data );
	
	Data += StringSize;
	TeamID = * (int *) Data;

	Data += sizeof(TeamID);
	Ready = * (BOOL *) Data;
	
	Data += sizeof(BOOL);
	Active = * (BOOL *) Data;

	return ERRNOERROR;
};


// Get game settings message
HRESULT Network::GetMsgGameSettings( CNetMsg * Msg, int & TimeHours, int & TimeMinutes )
{
	BYTE *	Data;
	
	Data = (BYTE *) Msg->getData();
	TimeHours = * (int *) Data;
	
	Data += sizeof(int);
	TimeMinutes = * (int *) Data;

/*
//RACER MOD ONLY:
	Data += sizeof(BOOL);
	NumberOfLaps = * (int *) Data;
*/

	return ERRNOERROR;
};


