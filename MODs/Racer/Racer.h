/*
	This file contains the CGameMod implementation for the basic racer game as a plugin into Unlimited Racer.
	For detailed info about this class see the GameMod.h file.

	Creation Date:	26.11.2006
	Last Update:	26.11.2006
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\..\GameMods\GameMod.h"


#ifdef RACER_EXPORTS
#define RACER_API __declspec(dllexport)
#else
#define RACER_API __declspec(dllimport)
#endif


// version of this module (doesn't have any connection to the loader interface version)
//  - has only informative character
#define MODULE_VERSION		0x00010000
#define MODULE_VERSION_STR	"1.0"
#define MODULE_NAME			"Basic Racing"



// mandatory function exports
extern "C" RACER_API DWORD		GetInterfaceVersion();
extern "C" RACER_API CGameMod *	GetModulePointer();


// main class exported from the FreeRide.dll
class RACER_API CRacerMod : CGameMod
{
public:
					CRacerMod();

	DWORD			GetModuleVersion() const { return dwVersion; };
	CString			GetModuleVersionStr() const { return csVersion; };
	CString			GetModuleName() const { return csName; };
	CString			GetModuleDescription() const { return csDescription; };
	void			GetSupportedMapIDs( STRING_LIST * pList ) const { if ( pList ) *pList = SupportedMODs; };
	void			GetConfiguration( MODCONFIG * pConfig ) const { if ( pConfig ) *pConfig = Configuration; };
	void			GetTeamNames( STRING_LIST * pList ) const { if ( pList ) *pList = TeamNames; };

private:
	// properties
	CString			csVersion;
	DWORD			dwVersion;
	CString			csName;
	CString			csDescription;
	STRING_LIST		SupportedMODs;
	MODCONFIG		Configuration;
	STRING_LIST		TeamNames;

};


// exported CGameMod instance
extern CRacerMod	*	pRacerMod;
extern bool				bLoadFault;



//////////////////////////////////////////////////////////////////////////////////////////////
//
//  CMODGameInfo class stores mainly the server side data about the game and its progress
//
//////////////////////////////////////////////////////////////////////////////////////////////
class RACER_API CRacerGameInfo : CMODGameInfo
{
public:
	
	// MOD methods
	void			UpdateLastCheckpoint( UINT ID, BOOL FromStart = FALSE ) 
							{ LastCheckpoint[ID] = FromStart ? 0 : LastCheckpoint[ID]+1; };
	void			UpdateBestLapTime( UINT ID, APPRUNTIME LapTime )
							{ if ( BestLapTime[ID] > LapTime ) BestLapTime[ID] = LapTime; };



	// MOD dependent properties
	STR_LIST		FinalPosition;
	std::vector<APPRUNTIME>	FinalRaceTime;
	std::vector<APPRUNTIME>	FinalBestLapTime;
	int				LastCheckpoint[MAX_PLAYERS]; // Last crossed checkpoint
	BOOL			CanUpdateCheckpoints[MAX_PLAYERS]; /* Whether checkpoints can be updated;
								is used to solve problem with updating checkpoint too early */
	int				LapsGone[MAX_PLAYERS]; // Number of laps a player finished already
	UINT			RacePositions[MAX_PLAYERS]; // Positions of the player in the race
	int				NumLaps; // Number of laps to go
	
	// Time statistics
	APPRUNTIME		LapStartTime[MAX_PLAYERS];
	APPRUNTIME		BestLapTime[MAX_PLAYERS];
	APPRUNTIME		RaceTime[MAX_PLAYERS];

};