/*
	This file contains the CGameMod implementation for free ride game as a plugin into Unlimited Racer.
	For detailed info about this class see the GameMod.h file.

	Creation Date:	26.11.2006
	Last Update:	8.1.2007
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\..\GameMods\GameMod.h"
#include "..\..\Globals\GlobalsIncludes.h"
#include "..\..\main\GlobalSounds.h"


#ifdef FREERIDE_EXPORTS
#define FREERIDE_API __declspec(dllexport)
#else
#define FREERIDE_API __declspec(dllimport)
#endif


// following macros are signifying this module and its version, keep them up to date and change
//	accordingly to your (new) module
#define MODULE_VERSION		0x00010000
#define MODULE_VERSION_STR	"1.0"
#define MODULE_NAME			"Free Ride"
#define MODULE_PRIMARYID	"FREERIDE"
#define MODULE_CONFIG_FNAME	"FreeRide.cfg"	// MOD configuration file name
#define MODULE_ADV_DIALOG_FNAME	"" // game dialog file name
#define MODULE_HUD_FNAME	""	// HUD definition file name

// custom keys:
#define CUSTOM_KEY_COUNT	1
#define KEY_HORN			0



// mandatory function exports
extern "C" FREERIDE_API DWORD		GetInterfaceVersion();
extern "C" FREERIDE_API CGameMod *	GetModulePointer();




//////////////////////////////////////////////////////////////////////////////////////////////
//
// main class exported from the FreeRide.dll, represents the game MOD
//
//////////////////////////////////////////////////////////////////////////////////////////////
class FREERIDE_API CFreeRideMod : CGameMod
{
public:
					CFreeRideMod();

	HRESULT			Init( INITSTRUC * pInitStruc );
	
	void			ClearGameData( bool OnlyGamePlayRelated = false );

	HRESULT			ReloadConfig();
	HRESULT			SaveConfig();
	void			SetDefaultConfig();

	// server methods
	void			UpdateGameStatus( APPRUNTIME dT );


private:
	// MOD dependent properties
	APPRUNTIME		atLastServerAction, atActualTime;

	// private methods inherited
	void			ServerProcessKey( UINT carID, UINT keyID, bool pressed ); 

	// private methods (MOD dependent)
	static void CALLBACK	LoadCallBackStatic( NAMEVALUEPAIR * nameValue );
	void			LoadCallBack( NAMEVALUEPAIR * pPair );
};




// exported CGameMod instance
extern CFreeRideMod  *	pFreeRideMod;
extern bool				bLoadFault;



