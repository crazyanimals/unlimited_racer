/*
	This file contains the CGameMod implementation for car-soccer game as a plugin into Unlimited Racer.
	Two functions are mandatory for export - GetInterfaceVersion (compatibility issues), GetModulePointer (object
	pointer retrieval) - and one instance of the implemented CGameMod object (=CSoccerMod) must be also exported.
	For more precise description, see comments in GameMods.h file.

	Creation Date:	23.11.2006
	Last Update:	5.2.2007
	Author:			Branislav Repcek
*/

#pragma once

#include "stdafx.h"
#include "../../GameMods/GameMod.h"
//#include "../../main/GlobalSounds.h"
#include "../../Network/CommonMessages.h"
#include "../../Network/CNetMsg.h"
#include "TeamInfo.h"
#include "BallData.h"
#include "PlayField.h"
#include "GameEvent.h"
#include "GameState.h"
#include "GameOptions.h"

#ifdef SOCCER_EXPORTS
#define SOCCER_API __declspec(dllexport)
#else
#define SOCCER_API __declspec(dllimport)
#endif


// version of this module (doesn't have any connection to the loader interface version)
//  - has only informative character
#define MODULE_VERSION          0x00009000
#define MODULE_VERSION_STR      "0.9 alpha"
#define MODULE_NAME             "Unlimited Soccer"
#define MODULE_PRIMARYID        "SOCCER1"
#define MODULE_CONFIG_FNAME	    "Soccer.cfg"
#define MODULE_ADV_DIALOG_FNAME ""
#define MODULE_HUD_FNAME		"Soccer.HUD"	// HUD definition file name


// mandatory function exports
extern "C" SOCCER_API DWORD GetInterfaceVersion();
extern "C" SOCCER_API CGameMod *GetModulePointer();

const size_t CUSTOM_KEY_COUNT = 4;
const char *CUSTOM_KEY_NAMES[CUSTOM_KEY_COUNT] = {"Attack", "Defend", "Goal", "Horn"};
const char *CUSTOM_KEY_CONFIG_NAMES[CUSTOM_KEY_COUNT] = {"KeyAttack", "KeyDefend", "KeyGoal", "KeyHorn"};
const char CUSTOM_KEY_DEFAULT_BINDINGS[CUSTOM_KEY_COUNT] = {'U', 'I', 'O', 'H'};
const WORD CUSTOM_KEY_SOUNDS[CUSTOM_KEY_COUNT] = {0, 1, 2, 3}; // TODO change sounds

const CString TEAM_NAME_FIRST = "Red Team";
const CString TEAM_NAME_SECOND = "Blue Team";

const CString OPTION_GAME_TYPE = "GameType";
const CString OPTION_SCORE_CAP = "ScoreCap";
const CString OPTION_TIME_CAP = "TimeCap";

const CString PROPERTY_PLAYGROUND = "playground";
const CString PROPERTY_HOME = "home";
const CString PROPERTY_TEAM_ID = "team_id";
const CString PROPERTY_FOOTBALL = "football";
const CString PROPERTY_START_POS = "start_pos";
const CString PROPERTY_START_ROT = "start_rot";
const CString PROPERTY_SCORE_AWARD = "score_award";

const size_t TEAM_ID_FIRST = 0;
const size_t TEAM_ID_SECOND = 1;

// Custom keys for the soccer game
enum SoccerCustomKeys
{
    KeyAttack,
    KeyDefend,
    KeyGoal,
    KeyHorn
};

#pragma warning(push)
#pragma warning(disable: 4275) // disable warning about "non-dll interface class" :/

// main class exported from the Soccer.dll
class SOCCER_API CSoccerMod : CGameMod
{
public:
    CSoccerMod();

    HRESULT Init(INITSTRUC *pInitStruc);

    virtual HRESULT ReloadConfig();
    virtual HRESULT SaveConfig();
    virtual void SetDefaultConfig();
	
	virtual void ClearGameData(bool OnlyGamePlayRelated = false);
    virtual void UpdateGameStatus(APPRUNTIME dT);

    virtual HRESULT ServerMPStart(void);
    virtual HRESULT ServerInitObject( OBJECT_LIST * pObjects, PAIR_LIST * pPairs );
    
    virtual bool ClientControlsEnabled();
    virtual HRESULT ClientMPStart();
    virtual HRESULT ClientGameMH(WPARAM wParam, LPARAM lParam, ::Network::CNetMsg *msg);

private:
    APPRUNTIME                                      match_time; // Match time.
    std::pair< soccer::TeamInfo, soccer::TeamInfo > teams;      // Details about teams.
    std::vector< soccer::BallData >                 balls;      // THE game with balls ;)
    soccer::GameState                               game_state; // State of the game
    soccer::PlayField                               play_field; // Playground (used to determine outs).
    soccer::GameOptions                             game_options; // game options
    bool                                            play_field_init; // oh, ugly :(
    bool                                            controls_enabled;

    // Initialize ball from the given object
    bool InitializeBall(OBJECT_LIST *pObjects, PAIR_LIST *pPairs);

    // Initialize goal area of a team from the given object
    bool InitializeTeam(OBJECT_LIST *pObjects, PAIR_LIST *pPairs);

    // Add playground square.
    bool AddPlayground(OBJECT_LIST *pObjects, PAIR_LIST *pPairs);

    // Add starting position square.
    bool AddStartingPosition(OBJECT_LIST *pObjects, PAIR_LIST *pPairs);

    // Process key event
    void ServerProcessKey(UINT carID, UINT keyID, bool pressed);

    // Generate string with current score
    CString ScoreString(void) const;

    // Adapter for load callback function
    static void CALLBACK LoadCallbackAdapter(NAMEVALUEPAIR *name_value);

    // Load callback method
    void LoadCallback(NAMEVALUEPAIR *name_value);

    // Update positions of all ball objects
    void UpdateBalls(void);

    // Reset all balls to their starting positions
    void ResetBalls(void);

    /* Update score if someone placed the ball into the home cage
       
       returns: true if someone scored (even if the score award is 0)
     */
    bool UpdateScore(void);

    /* Test if an out occured
       
       returns: true if an out occured
     */
    bool TestOut(void);

    // Handles game beginning (countdown etc...)
    void GameStart(void);

    // Handle game ending
    void GameEnd(void);

    // Send message to all players.
    void SendToAll(const CString &message, WORD msg_id = NETMSG_HUD_INFO_TEXT);

    // Called by the UpdateGameStatus during the normal game
    void NormalGameUpdate(void);

    // Called by the UpdateGameStatus during short time interval after the goal has been scored
    void AfterGoalGameUpdate(void);

    // Called by the UpdateGameStatus during short time after the ball left the playground (out)
    void AfterOutGameUpdate(void);

    // Inform players about time remaining
    void RemainingTimeUpdate(void);

    // Is game finished?
    bool ServerGameFinished();

    // Test if the games should end now.
    void TestEndConditions(void);

    // Reset velocities of all dynamic objects in the map
    void ResetVelocities(void);
};
#pragma warning(pop)

// exported CGameMod instance
extern CSoccerMod*  pSoccerMod;
extern bool         bLoadFault;
