/*
	GameMod:		CGameMod class is an abstract class describing and covering all of the functionality
					of one particular game modification (= game MOD). Every such MOD is thus an implementation
					of this interface stored in a DLL.

					When writing a new module, you have to implement the CGameMod class and create one its instance.
					Moreover, you have to implement two functions in your DLL:
					 - DWORD GetInterfaceVersion()
					 - CGameMod * GetModulePointer()
					Without these function the library will not be accepted by loader (CModWorkshop).
					Also you have to implement DLL_PROCESS_ATTACH event in the DllMain function with initialization
					of ErrorHandler and DebugOutput objects if you want to use output macros and functions like 
					ERRORMSG, OUTMSG and so on.
					
					The first one returns GAMEMOD_INTERFACE_VERSION (as defined below) which was used when
					building the particular module. This function is important to maintain compatibility between
					game MODs and the loader interface. This number must be exactly the same as the one used in 
					the game. (Notice, that the game itself can be of ANY version as long as it uses the same 
					game-MOD interface.)
					
					The second function (GetModulePointer) must return a valid pointer to an instance of object
					implementing your game modification (CGameMod). It is highly recommended to perform any 
					initializations in the DLL load time and when anything get wrong, return zero when this
					function is being called. The caller must then stop the execution/use of your module.

	Creation Date:	15.11.2006
	Last Update:	3.3.2007
	Author:			Roman Margold
*/

#pragma once

#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\Client\CMultiplayerGameInfo.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\Physics\Physics.h"
#include "..\GraphicObjects\Scene.h"
#include "..\main\CommonDefs.h"
#include "..\main\SoundBankCar.h"
#include "..\Network\CNetMsg.h"
#include "..\Network\CNetServer.h"
#include "..\Network\CNetClient.h"



// interface version macro defines version of the GameMOD interface which is used either
//   by the particular MOD or the game itself; this is supposed to change rarely.
//   every game module being loaded is tested against this interface version number
//   and only modules with the exactly same number are accepted
#define GAMEMOD_INTERFACE_VERSION	0x00010000



/* controls are being sent as an array of chars; common controls (game regular) are sent in low order
	chars, whether custom keys are being sent in higher indices; this value defines the base index from
	which custom MOD keys are placed in the array: (see main/controls.h for details) */
#define CUSTOM_KEY_INDEX		4



// following macros will signify your module, uncomment them, change them accordingly to your needs 
//	and keep them up to date
//#define MODULE_VERSION		0x00010000		// version specified by DWORD number
//#define MODULE_VERSION_STR	"1.0"			// version specified by string
//#define MODULE_NAME			"Free Ride"		// MOD name
//#define MODULE_PRIMARYID		"FREERIDE"		// MOD ID!
//#define MODULE_CONFIG_FNAME	"FreeRide.cfg"	// MOD configuration file name
//#define MODULE_ADV_DIALOG_FNAME	""			// game dialog file name
//#define MODULE_HUD_FNAME		"FreeRide.HUD"	// HUD definition file name

/*	maximal count of custom keys supported by game = the max count which can be used by any MOD;
	If you want to change this macro, you have to change controls.h and controls.cpp also, where
	specific number of ACTIONs are created and tested !! */
#define MAX_CUSTOM_KEYS			8





//////////////////////////////////////////////////
// this structure describes particular game MOD
struct MODCONFIG
{
	bool		bAI; // AI supported; when bMultiplayer is 0, this must also be 0
	bool		bMultiPlayer; // game MOD also available for multiple players
	bool		bSinglePlayer; // game also available in singlaplayer mode (multiplayer mode can still be possible however)
	UINT		uiMaxPlayers; // maximal count of players in multiplayer game; must be greater or equal to uiMinTeamCount * uiMinPlayersPerTeam
	UINT		uiMaxTeamCount; /* count of teams supported; 0 means that this game is not team based, 
									1 means almost the same now, but can change in the future; 
									the GetTeamNames method MUST return as many names as this property contains */
	UINT		uiMinTeamCount; /* minimal count of teams, this must be less or equal to uiMaxTeamCount;
									mostly it has the same value, but can be used to have more options */
	UINT		uiMinPlayersPerTeam; // minimal count of players per team
	UINT		uiMaxPlayersPerTeam; // maximal count of players per team
	UINT		uiCustomKeysCount; // count of custom keys supported
	bool		bTeamNameCustomizable; // if set, team names can be changed by players or whoever
	bool		bHasAdvancedDialog; // it this set, there's a dialog available for the game setting
	bool		bHasHUDPlayerList; // if set, the player list HUD item is displayed during game
	bool		bHasHUDScore; /* if set, the 'score' HUD item is displayed (it's a simple single-line 
									text field which can be used in any needed way, not just score display) */
	bool		bHasHUDInfoMessage; // if set, the 'info message' HUD item is displayed during game
	bool		bHasHUDTime; // if set, the 'time' HUD item is displayed during game
	
		//TODO: musi se pridat cela rada funkci pro editor! on musi vedet jaky ma vytvorit dialog a 
		//		co ten dialog bude popisovat, co se v nem ma nastavovat!
		//		nejlepe me ted napada asi seznam dvojic: Jmeno polozky, typ polozky (mozna jeste defaultni hodnota, rozsah, ...)
		//			a v editoru v dialogu by se pak nastavovaly
		// HA!!! ted me napadlo, ze by se mapa nemusela tak moc menit, kdyby se do ni pridaly sekce
		//		kazda sekce by se tykala nejakeho typu mapy; pokud by tam dana sekce nebyla, pak nic zvlastniho...
		//		pokud by tam byla, zpracovala by se pouze pro dany tom MODu, vyhoda je v tom, ze by tam byly nejenom
		//		polozky pro objekty (musel by tam tedy byt druhy seznam objektu), ale take polozky pro mapu jako celek...
		//	PROMYSLET!
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// Main class represeting the game MOD
//
//////////////////////////////////////////////////////////////////////////////////////////////
class CGameMod abstract 
{
public:
	
	// DEFINED TYPES
	
	// OBJECT defines single mesh placed into the world
	struct OBJECT
	{
		OBJECT( bool bDyn, int ind ) { bDynamic = bDyn; iIndex = ind; }
		OBJECT( int mapPosX, int mapPosY ) { bSurface = true; iMapX = mapPosX; iMapY = mapPosY; }

		bool		bDynamic;
		bool		bSurface; // this object has no model - just a surface texture
		int			iIndex;
		int			iMapX, iMapY; // map coordinates of the object; valid only if bSurface is true !!!
	};

	typedef std::vector<CString>		STRING_LIST;
	typedef std::vector<int>			INT_LIST;
	typedef std::vector<OBJECT>			OBJECT_LIST;
	typedef std::vector<NAMEVALUEPAIR>	PAIR_LIST;
	
	// structure used during the initialization of the MOD
	struct INITSTRUC
	{
		resManNS::CResourceManager	*	pResourceManager;
		physics::CPhysics			*	pPhysics;
		graphic::CScene				*	pScene;
		IXACTEngine					*	pSoundEngine; // engine used to play sounds
		COMMON_SOUNDS				*	pCommonSounds; // pointer to the COMMON_SOUNDS structure
		Network::CNetClient			*	pNetClient;
		Network::CNetServer			*	pNetServer; // not a server structure, but just a network interface 
	};




	// INITIALIZING METHODS
	/*
		All the module information should be set here, among others the SupportedMODs list by adding
		 csPrimaryID into it and the Configuration structure by proper info! */
						CGameMod();

	/* Init method is called by the external (main) application to define some properties.
	   It is NECESARRY to implement this method so it can be called repeatedly. An example
	   of this request is starting new game after another one. In that case the same
	   game MOD instance is used and thus all game info MUST be reset.
	   The default implementation just sets all pointers provided by 'pInitStruc' parameter. */
	virtual HRESULT		Init( INITSTRUC * pInitStruc );

	/* This call is used to set the pointer to CMultiPlayerGameInfo object which contains basic
		info common for all MODs and thus is implemented inside of the game itself. */
	virtual void		SetServerGameInfoObject( CMultiPlayerGameInfo * _pGameInfo ) { pServerGameInfo = _pGameInfo; };
	virtual void		SetClientGameInfoObject( CMultiPlayerGameInfo * _pGameInfo ) { pClientGameInfo = _pGameInfo; };

	/* ClearGameData method is called when the game ends, so it can contain some free calls
		etc. However, the game object still exists and it is possible that the game will
		be created again, thus this should also keep all data which can be used later (and free
		those in destructor) - use this to reset value and so on. */
	virtual void		ClearGameData( bool OnlyGamePlayRelated = false );




	// MODULE INFORMATION

	/* GetModuleVersion can return the module version number - informative character only,
	    it's not mandatory and doesn't have any special meaning. */
	virtual DWORD		GetModuleVersion() const { return dwVersion; };

	/* GetModuleVersionStr can return the module version string - informative character only,
	    it's not mandatory and doens't have any special meaning. */
	virtual CString		GetModuleVersionStr() const { return csVersion; };

	/* GetModulePrimaryID MUST return string which is used by the Limited Editor for signing 
		maps created for this module. This string must be unique against other modules and also versions!
		It should also be shorter than MAX_GAME_NAME_LENGTH characters (defined in CNetMsg.h file), which was 30.
		Also it should not contain characters used elsewhere with special meanings like ;,(){}...
		Better use only alphanumeric characters. */
	virtual CString		GetModulePrimaryID() const { return csPrimaryID; };

	/* GetModuleName should return module name. It's not mandatory, but is highly recommended. */
	virtual CString		GetModuleName() const { return csName; };

	/* GetModuleDescription can return the module text description. It's only optional and 
		is used for the user to get some insight. */
	virtual CString		GetModuleDescription() const { return csDescription; };




	// MODULE CONFIGURATION

	/* GetModuleCfgFileName MUST return the configuration file name if there is such one (or empty string). */
	virtual CString		GetModuleCfgFileName() const { return csConfigFileName; };

	/* GetModuleHUDFileName MUST return the file name of HUD configuration if there is such one (or empty string)
		which leads in default HUD definition usage. */
	virtual CString		GetModuleHUDFileName() const { return csHUDFileName; };

	/* GetSupportedMapIDs method returns ordered list of all map types that are supported by this module.
		This list is ordered by importance: when some map is usable by multiple MODs and more of
		them are implemented (supported) by this module, the first one appearing in the importance list 
		is used. This is mainly usable when making versions of single module. You can then implement your
		module so it accepts maps for older version also, but prefers the highest version available. */
	virtual void		GetSupportedMapIDs( STRING_LIST * pList ) const { if ( pList ) *pList = SupportedMODs; };

	/* GetConfiguration method returns the configuration structure filled with data describing
		this game MOD. See declaration of MODCONFIG structure. */
	virtual void		GetConfiguration( MODCONFIG * pConfig ) const { if ( pConfig ) *pConfig = Configuration; };
	virtual const MODCONFIG & GetConfiguration() const { return this->Configuration; };

	/* GetTeamNames method is used when the game type supports teams mode, when players are 
		divided into groups. More options are available regarding teams, but this method returns just
		predefined team names. Even if your module doesn't support teams, you must implement this method
		and return empty list.
		If bTeamNameCustomizable is not set, than this method MUST always return as many names as the value of 
		uiMaxTeamCount configuration property. If it IS set, then this method CAN return the same count
		of names, or can return 0. In such case team players will be forced to fill up the name. */
	virtual void		GetTeamNames( STRING_LIST * pList ) const { 
		if ( pList ) *pList = TeamNames; 
	};

	/* GetCustomKeyNames method is used when there are custom keys used by this game type. This method
		must return their names or short description of their purpose. (e.g. "Fire", "Jump", "Use" ... )
		The list must always return as many names as the value of uiCustomKeysCount configuration property.
		!!! Moreover, this method must be used in a very specific way !!! - the list MUST already 
		contain as many items as are excepted that it will return (Config.uiCustomKeysCount) and this 
		method only changes their values! */
	virtual void		GetCustomKeyNames( STRING_LIST * pList ) const;

	/* GetAdvDialogFileName returns the file name of .dlgDef file which stores definition of advanced
		dialog. This method can return empty string if Configuration.bHasAdvancedDialog is false. */
	virtual CString		GetAdvDialogFileName() const { return csAdvDialogFileName; };




	// ACTUAL CONFIGURATION HANDLING

	/* GetCustomKeys method is used when there are custom keys used by this game type. This method
		must return their currently set values or default values if not set yet.
		!!! Moreover, this method must be used in a very specific way !!! - the list MUST already 
		contain as many	items as are excepted that it will return (Config.uiCustomKeysCount) and this
		method only changes their values! */
	virtual void		GetCustomKeys( INT_LIST * pList );

	/* SetCustomKeys method is called to set the custom key binding as defined in the main program. 
		This method must be called with as many keys as the uiCustomKeysCount config property defines.
		Each key is defined by its integer key code. */
	virtual void		SetCustomKeys( INT_LIST * pList );

	/* SetDefaultConfig method must set all module dependent configuration to some default values.
		This method must be implemented! */
	virtual void		SetDefaultConfig() = 0;

	/* ReloadConfig method is used to force the module to reload its custom configuration. The first load 
		should be done automatically in any call which uses the configuration data. Notice, that this
		configuration is MOD dependent and has nothing to do with the MODCONFIG structure which is 
		the same for all MODs.
		This method must be implemented and should set the bConfigLoaded flag. */
	virtual HRESULT		ReloadConfig() = 0;

	/* SaveConfig method must be implemented and should store the actual configuration to some file. */
	virtual HRESULT		SaveConfig() = 0;




	// METHODS CALLED FROM MAIN (& Co.)

	/* SPStart method is called at the begining of single player when both server and client are already 
		loaded and initialized, dialog set to the InGameDialog and disabled (if you want to show another dialog
		it must be later reset to this one). In main game it is called by startSinglePlayer function. */
	virtual HRESULT		SPStart() { return ERRNOERROR; };

	/* GetCarPosition method is called by both server and client during cars (players) initialization.
		For each player (either AI or human) this method is called with the player index and asks for the
		initial position and rotation of the player in the world. If these positions are not known, this method
		must return false and in such case the default location (read from map starting location data) is
		used instead. This can be used when starting locations are defined in other than the default way.
		Only Y member of rotation vector is being used however.
		If you don't want to specify Y coordinate (height) which can be hard to compute, you can set the pos.y 
		to a negative value. In such case the y coordinate will be computed by physical engine to fit on the 
		surface. 
		(WARN! This means you CAN NOT use negative values for y coordinate to specify position below surface.) */
	virtual bool		GetCarPosition( int carIndex, D3DXVECTOR3 & pos, D3DXVECTOR3 & rot ) { return false; };




	// SERVER SIDE METHODS

	/* UpdateGameStatus method corresponds to the CServer::UpdateGameStatus and should perform all the
		actions related to operating the game. This method is called time to time and accepts the game-time
		difference as a parameter. In game it is called by UpdateGameStatus method before as a first action. */
	virtual void		UpdateGameStatus( APPRUNTIME dT ) {};
	
	/* ServerKeyProcessing method is called when controls are received from client. ID specifies car (player)
		identifier and keys parameter has the meaing as described in main\controls.h. Lower chars are used
		for game regular controls and higher chars (starting on CUSTOM_KEY_INDEX) are used for MOD keys.
		Each bit represents one key - 1 stands for 'pressed'. */
	virtual	void		ServerKeyProcessing( UINT carID, char * keys );

	/* ServerSPStart method is called after server initialization for single player game and when 
		the physics is already precomputed. In main game it is called by StartServerSingle. */
	virtual HRESULT		ServerSPStart() { return ERRNOERROR; };

	/* ServerMPStart method is called after server initialization for multiplayer game and when 
		the physics is already precomputed. In main game it is called by StartServerMultiLoad. */
	virtual HRESULT		ServerMPStart() { return ERRNOERROR; };

	/* ServerSendCustomInitialData method sends initial data from the server to all clients. These data are
		those that are not being sent automatically (static or dynamic objects), but rather virtual data
		like checkpoints etc. In main game this is called by CheckAllPlayersLoaded and
		ServerSingleStartMH functions. */
	virtual HRESULT		ServerSendCustomInitialData() { return ERRNOERROR; };
	
	/* Message handling routines are designed in a special way. Each one can implement custom network messages
		and these types of messages can be sent between the client and the server. These functions (handlers)
		are being called from main game in appropriate piece of code (message handling section) when any message
		of a special type is caught. Each such message type (16 bit integer) must have the highest bit set =
		= must contain the NETMSG_GAMEMOD macro. Of course, this routine can handle several types of messages. 

		This method must check the msg->getType() value which contains the message type identifier.

		ServerMPLobbyMH handler is called from ServerMultiStartMH during the time when players are connecting
			to game and choosing options.
		ServerMPStartMH is called by ServerMultiStartLoadMH function when waiting for players to load and
			when initial data are being sent from server to client.
		ServerGameMH (the most important one) is called by ServerGameMH function during the game itself. */
	virtual	HRESULT		ServerMPLobbyMH( WPARAM wParam, LPARAM lParam, Network::CNetMsg * msg ) { return ERRNOERROR; };
	virtual	HRESULT		ServerMPStartMH( WPARAM wParam, LPARAM lParam, Network::CNetMsg * msg ) { return ERRNOERROR; };
	virtual	HRESULT		ServerGameMH( WPARAM wParam, LPARAM lParam, Network::CNetMsg * msg ) { return ERRNOERROR; };

	/* ServerInitObject method is called during map loading on server's side (by MapPhInit function) for every 
		map object which has extra properties defined for this particular game MOD.
		The pObjects list contains indices of mesh objects that represents the map object being processed.
		Thus these indices can be used to point into Physics->staticObjects or Physics->dynamicObjects arrays.
		pPairs parameter contains all name=value pairs of all advanced properties for this particular MOD.
		Surface textures are send as separate objects also with bSurface property set. These contains valid
		coordinates so they can be located in the map. */
	virtual HRESULT		ServerInitObject( OBJECT_LIST * pObjects, PAIR_LIST * pPairs ) { return ERRNOERROR; };

	/* ServerGameFinished method should return true when the server decided that the game is over for some
		reason (for example time limit exceeded, game objectives reached or whatever). During the game this 
		should always return false. Once this returns true the game is permanently over and game states
		of all clients are changed to CS_RACE_RESULT and for every client the ClientGameOver method is called. */
	virtual bool		ServerGameFinished() { return false; };




	// CLIENT SIDE METHODS

	/* ClientSPStart method is called after client initialization for single player game and when 
		all data are loaded. In main game it is called by ClientStartSingleLoad. */
	virtual HRESULT		ClientSPStart() { return ERRNOERROR; };

	/* ClientMPStart method is called after client initialization for multiplayer game and when 
		all data are loaded. In main game it is called by ClientStartMultiLoad. */
	virtual HRESULT		ClientMPStart() { return ERRNOERROR; };

	/* See the comments of server message handling methods (e.g.ServerGameMH).

		ServerGameMH handler is called during the game itself by ClientGameMH function.
		ServerGameResultsMH is called in the end of the game when the results are being displayed (ClientRaceResultMH).
		ClientMPLobbyMH is called during the process of starting a new server and waiting for player and final
			configuration (ClientMultiStartMH).
		ClientMPStartMH is called befor the very start of game when initialization data are being sent between
			server and client and when server waits for players to load (ClientMultiStartLoadMH).
		ClientSPStartMH is called befor the very start of game when initialization data are being sent between
			server and client (ClientSingleStartMH). */
	virtual	HRESULT		ClientGameMH( WPARAM wParam, LPARAM lParam, Network::CNetMsg * msg ) { return ERRNOERROR; };
	virtual	HRESULT		ClientGameResultsMH( WPARAM wParam, LPARAM lParam, Network::CNetMsg * msg ) { return ERRNOERROR; };
	virtual	HRESULT		ClientMPLobbyMH( WPARAM wParam, LPARAM lParam, Network::CNetMsg * msg ) { return ERRNOERROR; };
	virtual	HRESULT		ClientMPStartMH( WPARAM wParam, LPARAM lParam, Network::CNetMsg * msg ) { return ERRNOERROR; };
	virtual	HRESULT		ClientSPStartMH( WPARAM wParam, LPARAM lParam, Network::CNetMsg * msg ) { return ERRNOERROR; };

	/* ClientInitObject method is called during map loading on client's side (by MapInit function) for every 
		map object which has extra properties defined for this particular game MOD.
		The pObjects list contains indices of mesh objects that represents the map object being processed.
		Thus these indices can be used to point into Scene->StaticObjects or Scene->DynamicObjects arrays.
		pPairs parameter contains all name=value pairs of all advanced properties for this particular MOD.
		Surface textures are send as separate objects also with bSurface property set. These contains valid
		coordinates so they can be located in the map. */
	virtual HRESULT		ClientInitObject( OBJECT_LIST * pObjects, PAIR_LIST * pPairs ) { return ERRNOERROR; };

	/* Implement ClientControlsEnabled method to disable client input during particular game time or as a
		response to some event. By default this method returns true which means that client controls are 
		processed and sent to server. If this method returns false, the user input which is not processed 
		on client side is ignored. */
	virtual bool		ClientControlsEnabled() { return true; };

	/* Implement this method if you want to perform some action at the end of the game. This method is called
		on every client when it receives a message about game over. The return value says whether the 'results'
		dialog should be displayed or not (by default this method simply returns true to display results dialog
		without other actions). */
	virtual bool		ClientGameOver() { return true; };




protected:
	// properties for inheritance
	
	// external object access pointer
	resManNS::CResourceManager	*	pResourceManager;
	physics::CPhysics			*	pPhysics;
	graphic::CScene				*	pScene;
	IXACTEngine					*	pSoundEngine; // engine used to play sounds
	COMMON_SOUNDS				*	pCommonSounds; // pointer to the COMMON_SOUNDS structure
	Network::CNetClient			*	pNetClient;
	Network::CNetServer			*	pNetServer; // not a server structure, but just a network interface 

	// module info
	CString						csVersion;
	DWORD						dwVersion;
	CString						csPrimaryID;
	CString						csName;
	CString						csDescription;
	STRING_LIST					SupportedMODs;
	MODCONFIG					Configuration;
	STRING_LIST					TeamNames;
	STRING_LIST					CustomKeyNames;
	CString						csConfigFileName;
	CString						csAdvDialogFileName;
	CString						csHUDFileName;

	// actual configuration
	int							CustomKeys[MAX_CUSTOM_KEYS];
	bool						bConfigLoaded;

	// server and client info
	CMultiPlayerGameInfo	*	pServerGameInfo;
	CMultiPlayerGameInfo	*	pClientGameInfo;



	// PRIVATE METHODS

	/* ServerProcessKey method should perform some action according to the specified key state.
		carID parameter specifies ID of the player, keyID is an index of the custom MOD key and
		pressed parameter tells whether the specified key is pressed or not. Thus we can handle even
		state changes. */
	virtual void		ServerProcessKey( UINT carID, UINT keyID, bool pressed ) {}; 


};



//////////////////////////////////////////////////////////////////////////////////////////////
//   Other types
//////////////////////////////////////////////////////////////////////////////////////////////
typedef CGameMod	*	LPGAMEMOD;


// declaration of the only two mandatory functions which have to be in ANY game MOD DLL
typedef CGameMod * (*LPGAMEMODOBJPTR)(); // returns the CGameMod object pointer
typedef DWORD		(*LPGAMEMODVERSION)(); // returns the interface version used in the module
