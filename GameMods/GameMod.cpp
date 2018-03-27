#include "stdafx.h"
#include "GameMod.h"



//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGameMod::CGameMod()
{
	// configuration file not loaded yet:
	bConfigLoaded = false;

	pServerGameInfo = NULL;
	pClientGameInfo = NULL;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGameMod::Init( INITSTRUC * pInitStruc )
{
	if ( !pInitStruc ) ERRORMSG( ERRINVALIDPARAMETER, "CGameMod::Init()", "Invalid init structure pointer." );

	pResourceManager = pInitStruc->pResourceManager;
	pPhysics = pInitStruc->pPhysics;
	pScene = pInitStruc->pScene;
	pSoundEngine = pInitStruc->pSoundEngine;
	pCommonSounds = pInitStruc->pCommonSounds;
	pNetClient = pInitStruc->pNetClient;
	pNetServer = pInitStruc->pNetServer;

	if ( !pResourceManager || !pPhysics || !pScene || !pSoundEngine || !pCommonSounds
		|| !pNetServer || !pNetClient )
		ERRORMSG( ERRINVALIDPARAMETER, "CGameMod::Init()", "" );

	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGameMod::GetCustomKeyNames( STRING_LIST * pList ) const
{
	if ( !pList || pList->size() < Configuration.uiCustomKeysCount ) return;
	
	for ( UINT i = 0; i < Configuration.uiCustomKeysCount; i++ )
		(*pList)[i] = CustomKeyNames[i];
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGameMod::GetCustomKeys( INT_LIST * pList )
{
	if ( !bConfigLoaded ) ReloadConfig();
	if ( !pList || pList->size() < Configuration.uiCustomKeysCount ) return;

	//pList->clear();
	for ( UINT i = 0; i < Configuration.uiCustomKeysCount; i++ )
		//pList->push_back( CustomKeys[i] );
		(*pList)[i] = CustomKeys[i];
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGameMod::SetCustomKeys( INT_LIST * pList )
{
	if ( !pList ) return;
	if ( pList->size() != Configuration.uiCustomKeysCount ) return;

	for ( UINT i = 0; i < Configuration.uiCustomKeysCount; i++ ) 
		CustomKeys[i] = (*pList)[i];
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGameMod::ClearGameData( bool OnlyGamePlayRelated )
{
	if ( pServerGameInfo ) pServerGameInfo->Clear( OnlyGamePlayRelated );
	if ( pClientGameInfo ) pClientGameInfo->Clear( OnlyGamePlayRelated );
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGameMod::ServerKeyProcessing( UINT carID, char * keys )
{	
	BYTE	bites;

	for ( UINT i = 0; i < Configuration.uiCustomKeysCount; i++ )
	{
		// custom keys starts from index CUSTOM_KEY_INDEX
		bites = (BYTE) keys[CUSTOM_KEY_INDEX + (i / 8)]; 
		// take only one bit at once = one key; in the order from the lowest to the highest bit
		bites &= 1 << (i % 8);
		
		// perform appropriate action
		if ( bites ) ServerProcessKey( carID, i, bites ? true : false );
	}
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////

