#include "stdafx.h"
#include "Racer.h"


// exported object
CRacerMod	*	pRacerMod;
bool			bLoadFault; // is set to true when library initialization failed



//////////////////////////////////////////////////////////////////////////////////////////////
//
// DLL main function
//
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		pRacerMod = new CRacerMod;
		bLoadFault = !pRacerMod;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif



//////////////////////////////////////////////////////////////////////////////////////////////
//
// GetInterfaceVersion returns the loader interface version used to write/build this module.
//
//////////////////////////////////////////////////////////////////////////////////////////////
RACER_API DWORD GetInterfaceVersion()
{
	return GAMEMOD_INTERFACE_VERSION;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// GetModulePointer returns a pointer to the RacerMod object.
//
//////////////////////////////////////////////////////////////////////////////////////////////
RACER_API CGameMod * GetModulePointer()
{
	if ( bLoadFault ) return NULL;

	return (CGameMod *) pRacerMod;
};




//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//
//				C R a c e r M o d       i m p l e m e n t a t i o n
//
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Standard constructor. Use this place to (re)define any static data.
//
//////////////////////////////////////////////////////////////////////////////////////////////
CRacerMod::CRacerMod()
{
	csVersion = MODULE_VERSION_STR;
	dwVersion = MODULE_VERSION;
	csName = MODULE_NAME;
	csDescription = "Basic Racing MOD is a basic game type in which you drive your car along";
	csDescription += " a prepared track. The only objective is to be the fastest among all players.";
	csDescription += " However, it's not so easy since you must not miss any obstacle. This MOD";
	csDescription += " contains AI driver players.";
	
	SupportedMODs.clear();
	SupportedMODs.push_back( "BRACER1" );

	ZeroMemory( &Configuration, sizeof( Configuration ) );
	Configuration.bAI = true;
	Configuration.bMultiPlayer = true;
	Configuration.bSinglePlayer = true;
	Configuration.uiTeamCount = 0;
	
	TeamNames.clear();
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// free all non-persistent (=single game dependent) objects
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CRacerMod::ClearGameData( bool OnlyGamePlayRelated )
{
	atLastServerAction = 0;
	atActualTime = 0;

	NumLaps = 0;

	for ( UINT i = 0; i < MAX_PLAYERS; i++ )
	{
		LastCheckpoint[i] = -1;
		RacePositions[i] = -1;
		CanUpdateCheckpoints[i] = TRUE;
		BestLapTime[i] = 4294967284;
		RaceTime[i] = 4294967284;
	}

	ZeroMemory( LapsGone,		MAX_PLAYERS * sizeof(*LapsGone) );
	ZeroMemory( LapStartTime,	MAX_PLAYERS * sizeof(*LapStartTime) );

	RaceStartTime = 0;
	
	// Clear final position
	FinalPosition.clear();
	FinalRaceTime.clear();
	FinalBestLapTime.clear();

	// clear original Multiplayer game info structure
	pGameInfo->Clear( OnlyGamePlayRelated );

};



//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
