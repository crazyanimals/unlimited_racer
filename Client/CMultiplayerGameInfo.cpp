/*
CMultiplayer game info class implementation
by Pavel Celba
22. 2. 2006
*/

#include "CMultiplayerGameInfo.h"

using namespace std;

// Init
HRESULT CMultiPlayerGameInfo::Init( resManNS::CResourceManager * _ResourceManager, CGameInfo * _pGameInfo )
{
	ResourceManager = _ResourceManager;
//	Checker.Init(ResourceManager);

	pGameInfo = _pGameInfo;

	return ERRNOERROR;
}

// constructor
CMultiPlayerGameInfo::CMultiPlayerGameInfo()
{
	MapID = 0;
	CarIDs.clear();
	Clear();
}

// Clear all game info
// When OnlyGamePlayRelated is TRUE -> setting is as in multiplayer lobby
void CMultiPlayerGameInfo::Clear(BOOL OnlyGamePlayRelated)
{
	int i;

	if (!OnlyGamePlayRelated)
	{
		Id = -2;
		State = PHASE_ZERO;
		GameName = "";
		MapFileName = "";
		MapCRCs.clear();
		ZeroMemory(Active, sizeof(Active[0]) * MAX_PLAYERS);
		ZeroMemory( TeamPlayerCount, sizeof(TeamPlayerCount[0]) * MAX_PLAYERS );
		ZeroMemory( TeamID, sizeof(TeamID[0]) * MAX_PLAYERS );
		PlayerCount = 0;
		for (i = 0; i < MAX_PLAYERS; i++)
		{
			PlayerNames[i] = "";
			CarFileNames[i] = "";
		}
		AllowedCarsCRCs.clear();
		AllowedCarsFileNames.clear();
		ZeroMemory(IsAI, sizeof(IsAI[0]) * MAX_PLAYERS);
		DayTimeHours = DayTimeMinutes = 0;
		ChatMessages.clear();
		// Unload loaded map
		if (MapID > 0)
			ResourceManager->ReleaseResource(MapID);
		// Unload loaded cars
		if (CarIDs.size() > 0)
		{
			for (i = 0; i < (int) CarIDs.size(); i++)
				ResourceManager->ReleaseResource(CarIDs[i]);
			CarIDs.clear();
		}
	}
	else
	{
		State = PHASE_THREE;
	}

	ZeroMemory( Loaded,			MAX_PLAYERS * sizeof(*Loaded) );
	ZeroMemory( Ready,			MAX_PLAYERS * sizeof(*Ready) );
	ZeroMemory( ReturnToLobby,	MAX_PLAYERS * sizeof(*ReturnToLobby) );
	
	ServerReturnedToLobby = FALSE;
}


HRESULT CMultiPlayerGameInfo::ComputeMapCRCs()
{
	//set<CIntegrityChecker::FileNameCRC> CRCSet;

	// Compute
	MapCRCs.clear();
	if (MapFileName != "")
	{
		// Load map and compute CRCs
		MapID = ResourceManager->LoadResource(MapFileName, RES_Map, true, 0);
		if (FAILED(MapID) ) 
			ERRORMSG(MapID, "CMultiPlayerGameInfo::ComputeMapCRCs()", CString("Unable to load map or compute CRCS for map filename:") + MapFileName);

		// Get CRCs
		MapCRCs = ResourceManager->GetCRCMultiset(MapID);
	}

	return ERRNOERROR;
}

// Set map file name 
// Checks whether file name is existing and computes map CRCs
HRESULT CMultiPlayerGameInfo::SetMapFileName(CString & Name)
{
	unsigned int i;
	BOOL Exists = false;
	HRESULT Result;

	// Check if file name exists
	for (i = 0; i < pGameInfo->MapNames.size(); i++)
		if (pGameInfo->MapNames[i].FileName == Name)
		{
			Exists = true;
			break;
		}

	// Return error, but don't write something into log,
	// because it's perfectly ok, that map is not present
	if (!Exists) return ERRFILENOTFOUND;

	MapFileName = Name;

	// compute CRC
	Result = ComputeMapCRCs();
	if (FAILED(Result) )
		ERRORMSG(Result, "CMultiPlayerGameInfo::SetMapFileName()", CString("Unable to compute CRCs for map file name: ") + Name);

	return ERRNOERROR;
}

// Only sets file name along with its CRCs
// No integrity check or existence check
HRESULT CMultiPlayerGameInfo::SetMapFileName(CString & Name, std::multiset<DWORD> MapCRC)
{
	MapFileName = Name;
	MapCRCs = MapCRC;

	return ERRNOERROR;
}

// Compares map CRCs
BOOL CMultiPlayerGameInfo::CompareMapFileName(std::multiset<DWORD> & CRCs)
{
	return CompareOrderedCRCs(MapCRCs, CRCs);
}

// Adds all available cars filenames and computes their CRCs
HRESULT CMultiPlayerGameInfo::FetchAllCars()
{
	unsigned int i;
	std::multiset<DWORD> CRCs;
	resManNS::RESOURCEID CarID;

	AllowedCarsFileNames.clear();
	AllowedCarsCRCs.clear();
	CarIDs.clear();

	for (i = 0; i < pGameInfo->CarNames.size(); i++)
	{
		// Compute car CRC
		CRCs.clear();
		
		CarID = ResourceManager->LoadResource(pGameInfo->CarNames[i].FileName, RES_CarObject, true, 0);
		if (FAILED(CarID) )
			continue;

		CarIDs.push_back(CarID);
		AllowedCarsFileNames.push_back(pGameInfo->CarNames[i].FileName);
		AllowedCarsCRCs.push_back(CRCs);
	}
	
	return ERRNOERROR;
}

// Sets all cars filenames 
// Iterates through active player car filenames and when cars doesn't exists
// changes car name to ""
HRESULT CMultiPlayerGameInfo::SetAllowedCarsFileNames( std::vector<CString> & Names )
{
	AllowedCarsFileNames = Names;

	CheckCarFileNames();

	return ERRNOERROR;
}


// Compares cars filenames and their CRCs
// Excludes every car from local allowed cars list, which is not in client file names or hasn't same CRC
HRESULT CMultiPlayerGameInfo::CompareCarsFileNames()
{
	unsigned int i, e, j;
	BOOL Exists;
	BOOL First = true;

	AllowedCarsFileNames.clear();
	AllowedCarsCRCs.clear();

	for (i = 0; i < MAX_PLAYERS; i++)
		if ( GetPlayerActive(i) && ! IsAI[i] )
			if (First)
			{
				AllowedCarsFileNames = AllClientCars[i];
				AllowedCarsCRCs = AllClientCarsCRCs[i];
				First = false;
			}
			else
			{
				for (e = 0; e < AllowedCarsFileNames.size(); e++)
				{
					Exists = false;
					for (j = 0; j < AllClientCars[i].size(); j++)
						if (AllClientCars[i].at(j) == AllowedCarsFileNames[e] )
						{
							Exists = CompareOrderedCRCs(AllowedCarsCRCs[e], AllClientCarsCRCs[i].at(j) );
							break;	
						}
					
					if (!Exists)
					{
						AllowedCarsFileNames.erase(AllowedCarsFileNames.begin() + e);
						AllowedCarsCRCs.erase(AllowedCarsCRCs.begin() + e);
						e--;
					}
				}
			}

	return ERRNOERROR;
}


// set the player's team ID (assigns a player to a team)
void CMultiPlayerGameInfo::SetPlayerTeamID( unsigned int playerID, unsigned int teamID )
{
	if ( playerID >= MAX_PLAYERS || teamID >= MAX_PLAYERS )
		ErrorHandler.HandleError( ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::SetPlayerTeamID()", "Invalid player or team ID." );

	// if the player is not active, just change the team ID, otherwise update the player counts in appropriate teams
	if ( GetPlayerActive(playerID) ) 
	{
		TeamPlayerCount[TeamID[playerID]]--;
		TeamPlayerCount[teamID]++;
	}
	
	TeamID[playerID] = teamID;
};

	
// Sets player status; teamID should be greater or equal to zero; however, if not specified,
//  set teamID to -1 which means "don't change"
HRESULT CMultiPlayerGameInfo::SetPlayerStatus(	unsigned int ID, 
												CString & PlayerName, 
												CString & PlayerCar, 
												int teamID,
												BOOL NewReady, 
												BOOL NewActive )
{
	if (ID >= MAX_PLAYERS)
		ERRORMSG(ERRINVALIDPARAMETER, "CMultiPlayerGameInfo::SetPlayerStatus()", "Invalid player ID.");

	PlayerNames[ID] = PlayerName;
	CarFileNames[ID] = PlayerCar;
	Ready[ID] = NewReady;
	SetPlayerActive( ID, NewActive );
	if ( teamID >= 0 ) SetPlayerTeamID( ID, (UINT) teamID );

	if (! IsAI[ID] )
		CheckCarFileNames(ID);

	return ERRNOERROR;
};


void CMultiPlayerGameInfo::SetPlayerActive( unsigned int ID, BOOL NewActive )
{
	if ( ID >= MAX_PLAYERS ) 
		ErrorHandler.HandleError(ERRINVALIDPARAMETER,"CMultiPlayerGameInfo::SetPlayerActive()", "Invalid player ID.");
	
	// the value is gonna be changed -> change counters also
	if ( NewActive != Active[ID] )
	{
		if ( NewActive )
		{
			TeamPlayerCount[TeamID[ID]]++;
			PlayerCount++;
		}
		else
		{
			TeamPlayerCount[TeamID[ID]]--;
			PlayerCount--;
		}
	}

	Active[ID] = NewActive;
};


// Iterates through active player car filenames and when car doesn't exist
// changes car name to ""
HRESULT CMultiPlayerGameInfo::CheckCarFileNames()
{
	unsigned int i;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		CheckCarFileNames(i);
	}

	return ERRNOERROR;
};


HRESULT CMultiPlayerGameInfo::CheckCarFileNames(unsigned int i)
{
	unsigned int e;
	BOOL Exists = false;
	
	for (e = 0; e < AllowedCarsFileNames.size(); e++)
		if (AllowedCarsFileNames[e] == CarFileNames[i])
		{
			Exists = true;
			break;
		}

	if (!Exists) CarFileNames[i] = "";

	return ERRNOERROR;
}

// Sets game settings
HRESULT CMultiPlayerGameInfo::SetGameSettings(int Hour, int Minutes)
{
	DayTimeHours = Hour;
	DayTimeMinutes = Minutes;
/*
//RACER MOD ONLY:
	Laps = LapMode;
	NumLaps = NumberOfLaps;
*/
	return ERRNOERROR;
}

BOOL CMultiPlayerGameInfo::CompareOrderedCRCs(std::multiset<DWORD> & MultiSet1, std::multiset<DWORD> & MultiSet2) const 
{
	multiset<DWORD>::const_iterator It1, It2;

	if (MultiSet1.size() != MultiSet2.size() ) return FALSE;

	for (It1 = MultiSet1.begin(), It2 = MultiSet2.begin(); It1 != MultiSet1.end(); It1++, It2++)
		if ( (*It1) != (*It2) )
			return FALSE;

	return TRUE;
}
