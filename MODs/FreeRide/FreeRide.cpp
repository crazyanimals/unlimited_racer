#include "stdafx.h"
#include "FreeRide.h"


// exported objects
CFreeRideMod *	pFreeRideMod;
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
		pFreeRideMod = new CFreeRideMod;
		bLoadFault = !pFreeRideMod;
		ErrorHandler.Init( ErrOutputCallBack );
	    DebugOutput.Init( DebugOutputCallBack );
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
FREERIDE_API DWORD GetInterfaceVersion()
{
	return GAMEMOD_INTERFACE_VERSION;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// GetModulePointer returns a pointer to the FreeRideMod object.
//
//////////////////////////////////////////////////////////////////////////////////////////////
FREERIDE_API CGameMod * GetModulePointer()
{
	if ( bLoadFault ) return NULL;

	return (CGameMod *) pFreeRideMod;
};




//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//
//				C F r e e R i d e M o d       i m p l e m e n t a t i o n
//
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Standard constructor. Use this place to (re)define any static data.
//
//////////////////////////////////////////////////////////////////////////////////////////////
CFreeRideMod::CFreeRideMod()
{
	// inherited constructor automatically called here

	// basic info setting
	csVersion = MODULE_VERSION_STR;
	dwVersion = MODULE_VERSION;
	csName = MODULE_NAME;
	csPrimaryID = MODULE_PRIMARYID;
	csConfigFileName = MODULE_CONFIG_FNAME;
	csAdvDialogFileName = MODULE_ADV_DIALOG_FNAME;
	csHUDFileName = MODULE_HUD_FNAME;

	csDescription = "Free Ride is a very simple mode in which you can freely drive your car";
	csDescription += " throu a map. No objective, no aim, no AI driven players.";
	
	SupportedMODs.clear();
	SupportedMODs.push_back( csPrimaryID );

	// fill up the configuration structure
	ZeroMemory( &Configuration, sizeof( Configuration ) );
	Configuration.bAI = false;
	Configuration.bMultiPlayer = true;
	Configuration.bSinglePlayer = true;
	Configuration.uiMaxTeamCount = Configuration.uiMaxTeamCount = 0;
	TeamNames.clear();
	Configuration.bTeamNameCustomizable = false;
	Configuration.uiMaxPlayers = 8;
	Configuration.uiMaxPlayersPerTeam = Configuration.uiMinPlayersPerTeam = 0;
	Configuration.uiCustomKeysCount = CUSTOM_KEY_COUNT; // must be less than MAX_CUSTOM_KEYS
	CustomKeyNames.clear();
	CustomKeyNames.push_back( "Horn:" );
	Configuration.bHasAdvancedDialog = false;
	Configuration.bHasHUDPlayerList = false;
	Configuration.bHasHUDScore = false;
	Configuration.bHasHUDInfoMessage = false;
	Configuration.bHasHUDTime = false;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// restores the default configuration
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CFreeRideMod::SetDefaultConfig()
{
	CustomKeys[KEY_HORN] = 'h';
	DebugLevel = 0;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// reloads configuration from the configuration file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CFreeRideMod::ReloadConfig()
{
	HRESULT	hr;

	// reload default setting first
	SetDefaultConfig();

	hr = NAMEVALUEPAIR::Load( csConfigFileName, LoadCallBackStatic, this );		
	if ( hr ) OUTMSG( "ERROR: Unable to load FreeRide module configuration.", 1 );

	bConfigLoaded = true;

	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function, calls the nonstatic version that could access object's properties
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CFreeRideMod::LoadCallBackStatic( NAMEVALUEPAIR * nameValue )
{
	((CFreeRideMod *) nameValue->ReservedPointer)->LoadCallBack( nameValue );
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function, stores one configuration pair to any member
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CFreeRideMod::LoadCallBack( NAMEVALUEPAIR * pPair )
{
	CString		name = pPair->GetName();
	
	name.MakeUpper();

	// key binding config
	if ( name == _T("KEYHORN") )
		CustomKeys[KEY_HORN] = CConfig::ParseKeyBindConfigPair( pPair->GetString() );

	if ( name == _T("DEBUGLEVEL") )
		DebugLevel = pPair->GetInt();

};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// saves the configuration into the default configuration file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CFreeRideMod::SaveConfig()
{
	FILE	*	f;
	CString		str;

	fopen_s( &f, MODULE_CONFIG_FNAME, "wt" );
	if ( !f )
	{
		ErrorHandler.HandleError( ERRCANNOTOPENFORWRITE, "CFreeRideMod::SaveConfig()", MODULE_CONFIG_FNAME );
		return ERRGENERIC;
	}

	str = CConfig::UnParseKeyBindConfigPair( CustomKeys[KEY_HORN] );
	fprintf_s( f, "KeyHorn = %s\n", str );
	fprintf_s( f, "DebugLevel = %i\n", DebugLevel );

	fclose( f );
	
	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// Some initializing stuff can occur here; the game info must be reset
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CFreeRideMod::Init( INITSTRUC * pInitStruc )
{
	HRESULT		hr;
	
	hr = this->CGameMod::Init( pInitStruc );
	if ( hr ) ERRORMSG( hr, "CFreeRideMod::Init()", "Default initialization failed." );

	ClearGameData();

	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// free all non-persistent (=single game dependent) objects
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CFreeRideMod::ClearGameData( bool OnlyGamePlayRelated )
{
	atLastServerAction = 0;
	atActualTime = 0;

	this->CGameMod::ClearGameData( OnlyGamePlayRelated );
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// This method controls the game.
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CFreeRideMod::UpdateGameStatus( APPRUNTIME dT )
{
	atActualTime += dT;

	// place code here

	atLastServerAction = atActualTime;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns a constant reference to MOD configuration object
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CFreeRideMod::ServerProcessKey( UINT carID, UINT keyID, bool pressed )
{
	switch ( keyID )
	{
	case KEY_HORN:
		if ( pressed ) pCommonSounds->pPrimarySoundBank->Play( XACT_CUE_CAR_CARHORNRANDOM, 0, 0, NULL );
		break;
	}
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
