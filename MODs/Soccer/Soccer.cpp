//
// Soccer.cpp : Defines the entry point for the DLL application.
//
// (c) 2006-2007 Branislav Repcek, Roman Margold
//

#include <fstream>

#include <string>
#include <sstream>
#include <iomanip>

#include "stdafx.h"
#include "Soccer.h"
#include "../../Main/GlobalSounds.h"
#include "../../GraphicObjects/GrObjectBase.h"
#include "../../Network/CommonMessages.h"
#include "../../Network/CNetMsg.h"

#include "GameState.h"
#include "BoundingBox.h"
#include "PlayField.h"
#include "TeamInfo.h"
#include "BallData.h"

CSoccerMod *	pSoccerMod; // exported object
bool			bLoadFault; // is set to true when library initialization failed

using namespace std;

// Amount of time (ms) before the ball is reset to its starting position after the goal has been scored
const int BALL_RESET_TIME = 10000;

// Number of seconds before the game can begin
const int GAME_WARMUP_TIME = 12;

// Number of seconds before the game really ends (after some winnin/losing/end game condition has been met)
const int GAME_WARMDOWN_TIME = 10;

/* Find property and query its value

   props: list of properties to search through
   name: name of the property to search for
   result: reference to the name-value pair which will receive value of the property if it has been found

   return: true if property with given name is in the list, false otherwise. If property has been found, its value
           is returned in the result parameter. If property has not been found, result parameter is not modified.
 */
bool FindProperty(const std::vector< NAMEVALUEPAIR > &props, const CString &name, NAMEVALUEPAIR &result);

/* Test if property is in the property list

   props: list of properties to search through
   name: name of property to look for

   returns: true if property with given name is in the list, false otherwise
 */
bool HasProperty(const std::vector< NAMEVALUEPAIR > &props, const CString &name);

// Convert value to CString
template< typename T >
CString ConvertToCStr(T value)
{
    stringstream stream;
    stream << value;

    return CString(stream.str().c_str());
}

// Uncomment to output additional data to the different file than the main log
//#define CUSTOM_LOG

#ifdef CUSTOM_LOG
ofstream log_file;
#endif

#ifdef _MANAGED
#pragma managed(push, off)
#endif

// Entry point for the DLL
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        pSoccerMod = new CSoccerMod;
        bLoadFault = !pSoccerMod;
        ErrorHandler.Init(ErrOutputCallBack);
        DebugOutput.Init(DebugOutputCallBack);
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

// returns: loader interface version used to write/build this module.
SOCCER_API DWORD GetInterfaceVersion()
{
    return GAMEMOD_INTERFACE_VERSION;
};

// returns: pointer to the SoccerMod object.
SOCCER_API CGameMod * GetModulePointer()
{
	if ( bLoadFault ) return NULL;

	return (CGameMod *) pSoccerMod;
};

CSoccerMod::CSoccerMod() :
teams(soccer::TeamInfo(TEAM_NAME_FIRST), soccer::TeamInfo(TEAM_NAME_SECOND)),
play_field_init(false),
controls_enabled(true)
{
#ifdef CUSTOM_LOG
    log_file.open("log_file", ios_base::out | ios_base::app);
    log_file << "========================================================" << endl;
#endif

	csVersion = MODULE_VERSION_STR;
	dwVersion = MODULE_VERSION;
	csName = MODULE_NAME;
    csPrimaryID = MODULE_PRIMARYID;
    csConfigFileName = MODULE_CONFIG_FNAME;
    csAdvDialogFileName = MODULE_ADV_DIALOG_FNAME;
	csDescription = "Unlimited Soccer is a crazy UR modification which allows human players"
	                " to play soccer in various cars. So, no FOOT in this football other than"
	                " the one on the pedal! (Doesn't contain AI players.)";
	csHUDFileName = MODULE_HUD_FNAME;

    SupportedMODs.clear();
    SupportedMODs.push_back(csPrimaryID);

    Configuration.bAI = false;
    Configuration.bMultiPlayer = true;
    Configuration.bSinglePlayer = false;
    Configuration.uiMinTeamCount = Configuration.uiMaxTeamCount = 2; // todo
    Configuration.bTeamNameCustomizable = false;
    Configuration.uiMaxPlayers = 8;
    Configuration.uiMinPlayersPerTeam = 0;
	Configuration.uiMaxPlayersPerTeam = 4;

    Configuration.uiCustomKeysCount = CUSTOM_KEY_COUNT;
    for (size_t i = 0; i < CUSTOM_KEY_COUNT; ++i)
    {
        CustomKeyNames.push_back(CUSTOM_KEY_NAMES[i]);
    }

    TeamNames.push_back(teams.first.GetName());
    TeamNames.push_back(teams.second.GetName());

	Configuration.bHasAdvancedDialog = false;
	Configuration.bHasHUDPlayerList = false;
	Configuration.bHasHUDScore = true;
	Configuration.bHasHUDInfoMessage = true;
	Configuration.bHasHUDTime = true;
};

void CSoccerMod::SetDefaultConfig()
{
    for (size_t i = 0; i < CUSTOM_KEY_COUNT; ++i)
    {
        CustomKeys[i] = CUSTOM_KEY_DEFAULT_BINDINGS[i];
    }

    DebugLevel = 0;
}

HRESULT CSoccerMod::ReloadConfig()
{
    SetDefaultConfig();

    HRESULT hres = NAMEVALUEPAIR::Load(csConfigFileName, LoadCallbackAdapter, this);
    if (hres) OUTMSG("ERROR: Unable to load Soccer configuartion.", 1);

    bConfigLoaded = true;

    return ERRNOERROR;
}

HRESULT CSoccerMod::SaveConfig()
{
    ofstream config_file;

    config_file.open(MODULE_CONFIG_FNAME);
    if (!config_file.is_open())
    {
		ErrorHandler.HandleError(ERRCANNOTOPENFORWRITE, "CSoccerMod::SaveConfig()", MODULE_CONFIG_FNAME);
		return ERRGENERIC;
    }
    
    for (size_t i = 0; i < CUSTOM_KEY_COUNT; ++i)
    {
        CString str = CConfig::UnParseKeyBindConfigPair(CustomKeys[i]);

        config_file << CUSTOM_KEY_CONFIG_NAMES[i] << " = " << str << endl;
    }

    config_file << "DebugLevel = " << DebugLevel << endl;

    return ERRNOERROR;
}

HRESULT CSoccerMod::Init(CGameMod::INITSTRUC *pInitStruc)
{
    HRESULT res = CGameMod::Init(pInitStruc);
    if (res)
    {
        ERRORMSG(res, "CSoccerMod::Init()", "Default init failed.");
    }

    ClearGameData();

    return ERRNOERROR;
}

void CSoccerMod::ClearGameData(bool OnlyGamePlayRelated)
{
    match_time = 0;
    teams.first.Reset();
    teams.second.Reset();
    play_field_init = false;

    ResetBalls();
    match_time = 0;
    SendToAll("");
    SendToAll("", NETMSG_HUD_SCORE);
    game_state.Start();

    CGameMod::ClearGameData(OnlyGamePlayRelated);
}

void CSoccerMod::UpdateGameStatus(APPRUNTIME dT)
{
    match_time += dT;

    UpdateBalls();

    switch (game_state.state)
    {
    case soccer::GameState::NORMAL_GAME: // normal game -> test if someone scored
        NormalGameUpdate();
        break;

    case soccer::GameState::AFTER_GOAL: // someone just scored, show some text messages
		AfterGoalGameUpdate();
        break;

    case soccer::GameState::AFTER_OUT:
        AfterOutGameUpdate();
        break;

    case soccer::GameState::GAME_START:
        GameStart();
        break;

    case soccer::GameState::GAME_END:
        GameEnd();
        break;
    }

    if (game_state.state != soccer::GameState::GAME_END)
    {
        TestEndConditions();
    }

    if (game_state.state == soccer::GameState::NORMAL_GAME)
    {
        RemainingTimeUpdate();
    }

    if (game_state.state != game_state.GAME_END)
    {
        stringstream stream;

        stream.fill('0');
        int time_s = match_time / 1000 - GAME_WARMUP_TIME;

        if (time_s >= 0)
        {
            stream << (time_s / 60) << ":" << setw(2) << (time_s % 60);
        }
        else
        {
            stream << "--:--";
        }
     
        SendToAll(CString(stream.str().c_str()), NETMSG_HUD_TIME);
    }

    //stringstream stream;
    //stream << balls[0].GetPosition().x << "," << balls[0].GetPosition().y << "," << balls[0].GetPosition().z;
    //SendToAll(CString(stream.str().c_str()));
}

bool TestRange(int value, int min_v, int max_v)
{
    return (value >= min_v) && (value <= max_v);
}

void CSoccerMod::RemainingTimeUpdate(void)
{
    // Test this only on games that have time limit
    if (game_options.GetGameType() == soccer::GAME_TIME_CAP)
    {
        static soccer::GameEventID last_event_sent = soccer::EVENT_GENERIC;
        static int clear_on = -1;

        int time_remaining = static_cast< int >(game_options.GetTimeCap()) - (match_time / 1000 - GAME_WARMUP_TIME);

        if (time_remaining < clear_on)
        {
            clear_on = -1;
            SendToAll("");
        }

        if (TestRange(time_remaining, 299, 300) && (last_event_sent != soccer::EVENT_5MIN_REMAINING))
        {
            soccer::GameEvent(soccer::EVENT_5MIN_REMAINING).SendMessageAll(pNetServer);
            SendToAll("5 minutes remaining...");
            last_event_sent = soccer::EVENT_5MIN_REMAINING;
            clear_on = time_remaining - 2;
            return;
        }

        if (TestRange(time_remaining, 179, 180) && (last_event_sent != soccer::EVENT_3MIN_REMAINING))
        {
            soccer::GameEvent(soccer::EVENT_3MIN_REMAINING).SendMessageAll(pNetServer);
            SendToAll("3 minutes remaining...");
            last_event_sent = soccer::EVENT_3MIN_REMAINING;
            clear_on = time_remaining - 2;
            return;
        }
        if (TestRange(time_remaining, 59, 60) && (last_event_sent != soccer::EVENT_1MIN_REMAINING))
        {
            soccer::GameEvent(soccer::EVENT_1MIN_REMAINING).SendMessageAll(pNetServer);
            SendToAll("1 minute remaining...");
            last_event_sent = soccer::EVENT_1MIN_REMAINING;
            clear_on = time_remaining - 2;
            return;
        }
        if (TestRange(time_remaining, 29, 30) && (last_event_sent != soccer::EVENT_30SEC_REMAINING))
        {
            soccer::GameEvent(soccer::EVENT_30SEC_REMAINING).SendMessageAll(pNetServer);
            SendToAll("30 seconds remaining...");
            last_event_sent = soccer::EVENT_30SEC_REMAINING;
            clear_on = time_remaining - 2;
            return;
        }

        if ((time_remaining <= 5) && (time_remaining > 0))
        {
            static int last_time_remaing = 0;

            if (last_time_remaing != time_remaining)
            {
                soccer::GameEvent(soccer::EVENT_COUNTDOWN, time_remaining).SendMessageAll(pNetServer);
                SendToAll(ConvertToCStr(time_remaining));
            }
            last_time_remaing = time_remaining;
        }
    }
}

void CSoccerMod::NormalGameUpdate(void)
{    
    if (TestOut())
    {
        return;
    }

    if (UpdateScore())
    {
        SendToAll(ScoreString(), NETMSG_HUD_SCORE);
    }
}

void CSoccerMod::GameStart(void)
{
    static int last_countdown = -1;
    int countdown = std::max< int >(match_time / 1000, 0);

    if (countdown < GAME_WARMUP_TIME)
    {
        if (last_countdown != countdown)
        {
            if (countdown == 0)
            {
                soccer::GameEvent(soccer::EVENT_GAME_START).SendMessageAll(pNetServer);
                SendToAll("Prepare for battle!");
            }
            else if (countdown > 1)
            {
                soccer::GameEvent(soccer::EVENT_COUNTDOWN, GAME_WARMUP_TIME - countdown).SendMessageAll(pNetServer);
                SendToAll(ConvertToCStr(GAME_WARMUP_TIME - countdown));
            }
        }

        last_countdown = countdown;
    }
    else
    {
        game_state.Game(match_time);
        SendToAll("");
        SendToAll(ScoreString(), NETMSG_HUD_SCORE);

        ResetBalls();
        return;
    }
}

void CSoccerMod::GameEnd(void)
{
}

bool CSoccerMod::UpdateScore(void)
{
    int score_award = 0;
    bool result = false;

    if (teams.first.InsideHomeCenter(balls, score_award))
    {
        // Second team scored
#ifdef CUSTOM_LOG
        log_file << teams.second.GetName() << " scored: " << score_award << endl;
#endif
        teams.second.AddScore(score_award);
        game_state.Goal(match_time, &teams.second, score_award);

        if ((game_options.GetGameType() != soccer::GAME_SCORE_CAP) || 
            (static_cast< int >(game_options.GetScoreCap()) > teams.second.GetScore()))
        {
            // Send event only it this is not the winning goal
            soccer::GameEvent(soccer::EVENT_GOAL).SendMessageAll(pNetServer);
        }
        result = true;
    }
    else if (teams.second.InsideHomeCenter(balls, score_award))
    {
        // First team scored
#ifdef CUSTOM_LOG
        log_file << teams.first.GetName() << " scored: " << score_award << endl;
#endif
        teams.first.AddScore(score_award);
        game_state.Goal(match_time, &teams.first, score_award);
        
        if ((game_options.GetGameType() != soccer::GAME_SCORE_CAP) || 
            (static_cast< int >(game_options.GetScoreCap()) > teams.first.GetScore()))
        {
            // Send event only it this is not the winning goal
            soccer::GameEvent(soccer::EVENT_GOAL).SendMessageAll(pNetServer);
        }
        result = true;
    }

    return result;
}

bool CSoccerMod::TestOut(void)
{
    for (vector< soccer::BallData >::const_iterator it = balls.begin(); it != balls.end(); ++it)
    {
        if (play_field.IsOut(it->GetPosition().x, it->GetPosition().z))
        {
            game_state.Out(match_time);

            soccer::GameEvent(soccer::EVENT_OUT).SendMessageAll(pNetServer);
            return true;
        }
    }

    return false;
}

void CSoccerMod::AfterGoalGameUpdate(void)
{
    static int last_countdown = 0;
    int reset_countdown = match_time - game_state.state_change;

    if (reset_countdown > BALL_RESET_TIME - 1000)
    {
        // ball needs to be reset
        ResetBalls();
        game_state.Game(match_time); // switch game state back to normal
        SendToAll(""); // reset info message to empty string
    }
    else
    {
        reset_countdown = ((BALL_RESET_TIME - 1000) - reset_countdown) / 1000;
        stringstream stream;

        if ((reset_countdown > BALL_RESET_TIME / 1000 - 6) && (game_state.which_team != NULL)) // last five seconds
        {
            stream << game_state.which_team->GetName();
            if ((game_state.score_delta == -1) || (game_state.score_delta == 1))
            {
                stream << " scored " << game_state.score_delta << " point!";
            }
            else if (game_state.score_delta == 0)
            {
                stream << " hit the decoy!";
            }
            else
            {
                stream << " scored " << game_state.score_delta << " points!";
            }
        }
        else
        {
            stream << "Ball reset in " << (1 + reset_countdown) << "...";
            if (reset_countdown != last_countdown)
            {
                soccer::GameEvent(soccer::EVENT_COUNTDOWN, 1 + reset_countdown).SendMessageAll(pNetServer);
            }
            last_countdown = reset_countdown;
        }

        CString message_text(stream.str().c_str());
        SendToAll(message_text);
    }
}

void CSoccerMod::AfterOutGameUpdate(void)
{
    static int last_countdown = 0;
    int reset_countdown = match_time - game_state.state_change;

    if (reset_countdown > BALL_RESET_TIME - 1000)
    {
        // ball needs to be reset
        ResetBalls();
        game_state.Game(match_time); // switch game state back to normal
        SendToAll(""); // reset info message to empty string
    }
    else
    {
        reset_countdown = ((BALL_RESET_TIME - 1000) - reset_countdown) / 1000;

        if (reset_countdown > BALL_RESET_TIME / 1000 - 6) // last 5 seconds
        {
            SendToAll(CString("OUT!"));
        }
        else
        {
            stringstream stream;

            stream << "Ball reset in " << (1 + reset_countdown) << "...";

            CString message_text(stream.str().c_str());
            SendToAll(message_text);

            if (last_countdown != reset_countdown)
            {
                soccer::GameEvent(soccer::EVENT_COUNTDOWN, 1 + reset_countdown).SendMessageAll(pNetServer);
            }
            last_countdown = reset_countdown;
        }
    }
}

bool CSoccerMod::ServerGameFinished()
{
    return (game_state.state == soccer::GameState::GAME_END) && (match_time > game_state.state_change + GAME_WARMDOWN_TIME * 1000);
}

void CSoccerMod::TestEndConditions(void)
{
    int mt = match_time - GAME_WARMUP_TIME * 1000;
    static soccer::GameEventID first_id = soccer::EVENT_GENERIC;
    static soccer::GameEventID second_id = soccer::EVENT_GENERIC;

    switch (game_options.TestConditions((mt <= 0 ? 0 : mt), teams))
    {
    case soccer::END_NO_END:
        return;

    case soccer::END_TEAM1_WON:
#ifdef CUSTOM_LOG
        if (game_state.state != soccer::GameState::GAME_END)
        {
            log_file << "Team1 won" << endl;
            log_file << "  time: " << match_time - GAME_WARMUP_TIME * 1000 << endl;
            log_file << "  match_time: " << match_time << endl;
        }
#endif
        if (first_id != soccer::EVENT_GAME_END_YOU_WON)
        {
            soccer::GameEvent(soccer::EVENT_GAME_END_YOU_WON).SendMessageTeam(TEAM_ID_FIRST, pNetServer, pServerGameInfo);
            first_id = soccer::EVENT_GAME_END_YOU_WON;
        }
        if (second_id != soccer::EVENT_GAME_END_YOU_LOST)
        {
            soccer::GameEvent(soccer::EVENT_GAME_END_YOU_LOST).SendMessageTeam(TEAM_ID_SECOND, pNetServer, pServerGameInfo);
            second_id = soccer::EVENT_GAME_END_YOU_LOST;
        }

        if (game_state.state != soccer::GameState::GAME_END)
        {
            SendToAll("Red team won the match!");
            ResetVelocities();
            game_state.End(match_time);
        }
        return;

    case soccer::END_TEAM2_WON:
#ifdef CUSTOM_LOG
        if (game_state.state != soccer::GameState::GAME_END)
        {
            log_file << "Team2 won" << endl;
            log_file << "  time: " << match_time - GAME_WARMUP_TIME * 1000 << endl;
            log_file << "  match_time: " << match_time << endl;
        }
#endif
        if (first_id != soccer::EVENT_GAME_END_YOU_LOST)
        {
            soccer::GameEvent(soccer::EVENT_GAME_END_YOU_LOST).SendMessageTeam(TEAM_ID_FIRST, pNetServer, pServerGameInfo);
            first_id = soccer::EVENT_GAME_END_YOU_LOST;
        }
        if (second_id != soccer::EVENT_GAME_END_YOU_WON)
        {
            soccer::GameEvent(soccer::EVENT_GAME_END_YOU_WON).SendMessageTeam(TEAM_ID_SECOND, pNetServer, pServerGameInfo);
            second_id = soccer::EVENT_GAME_END_YOU_WON;
        }
        
        if (game_state.state != soccer::GameState::GAME_END)
        {
            SendToAll("Blue team won the match!");
            ResetVelocities();
            game_state.End(match_time);
        }
        return;
    }
}

void CSoccerMod::ServerProcessKey(UINT carID, UINT keyID, bool pressed)
{
    switch ( keyID )
    {
    case KeyAttack: 
        if (pressed)
        {
            pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_ATTACK, 0, 0, NULL);
        }
        break;
    case KeyHorn:
        if (pressed)
        {
            pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_CARHORNRANDOM, 0, 0, NULL);
        }
        break;

    case KeyDefend:
        if (pressed)
        {
            ResetBalls();
        }
        break;
    }
}

void CSoccerMod::LoadCallbackAdapter(NAMEVALUEPAIR *name_value)
{
    ((CSoccerMod *) name_value->ReservedPointer)->LoadCallback(name_value);
}

void CSoccerMod::LoadCallback(NAMEVALUEPAIR *name_value)
{
    if (name_value->GetName().CompareNoCase(OPTION_GAME_TYPE) == 0)
    {
        game_options.SetGameType(name_value->GetString());
        return;
    }

    if (name_value->GetName().Compare(OPTION_SCORE_CAP) == 0)
    {
        game_options.SetScoreCap(static_cast< size_t >(name_value->GetInt()));
        return;
    }

    if (name_value->GetName().Compare(OPTION_TIME_CAP) == 0)
    {
        game_options.SetTimeCap(static_cast< size_t >(name_value->GetInt()));
        return;
    }

    for (size_t i = 0; i < CUSTOM_KEY_COUNT; ++i)
    {
        if (name_value->GetName().CompareNoCase(CUSTOM_KEY_CONFIG_NAMES[i]) == 0)
        {
            CustomKeys[i] = CConfig::ParseKeyBindConfigPair(name_value->GetString());
            break;
        }
    }
}

HRESULT	CSoccerMod::ServerInitObject(OBJECT_LIST *pObjects, PAIR_LIST *pPairs)
{
    if (HasProperty(*pPairs, PROPERTY_PLAYGROUND))
    {
        if (!AddPlayground(pObjects, pPairs))
        {
            return ERRINVALIDDATA;
        }
    }

    if (HasProperty(*pPairs, PROPERTY_FOOTBALL))
    {
        return InitializeBall(pObjects, pPairs) ? ERRNOERROR : ERRINVALIDDATA;
    }

    if (HasProperty(*pPairs, PROPERTY_HOME))
    {
        return InitializeTeam(pObjects, pPairs) ? ERRNOERROR : ERRINVALIDDATA;
    }

    if (HasProperty(*pPairs, PROPERTY_START_POS))
    {
        return AddStartingPosition(pObjects, pPairs) ? ERRNOERROR : ERRINVALIDDATA;
    }
    
    return ERRNOERROR;
}

bool CSoccerMod::InitializeBall(OBJECT_LIST *pObjects, PAIR_LIST *pPairs)
{
#ifdef CUSTOM_LOG
    log_file << "football " << (balls.size() + 1) << endl;
#endif

    int ball_item_index = -1;

    for (OBJECT_LIST::const_iterator it = pObjects->begin(); it != pObjects->end(); ++it)
    {
        if (it->bDynamic)
        {
            ball_item_index = static_cast< int >(it - pObjects->begin());
            break;
        }
    }

    if (ball_item_index < 0)
    {
#ifdef CUSTOM_LOG
        log_file << "  No dynamic object found for ball" << endl;
        return false;
#endif
    }

    NewtonCollision *pNCollision;
    dMatrix Matrix;
    D3DXVECTOR3 vmin, vmax;

    physics::CDynamicObject *ball_object = pPhysics->dynamicObjects[(*pObjects)[ball_item_index].iIndex];

    // obtain the collision volume
    pNCollision = NewtonBodyGetCollision(ball_object->NBody());

    // set the collision world matrix
    Matrix = ball_object->Matrix();

    // calculate convex bounding box
    NewtonCollisionCalculateAABB(pNCollision, (dFloat *) (&Matrix), (dFloat *) (&vmin), (dFloat *) (&vmax));

    balls.push_back(soccer::BallData((*pObjects)[0].iIndex, ball_object->Position(), (vmax.x - vmin.x) * 0.5f));
    balls.back().Update(pPhysics);

#ifdef CUSTOM_LOG
    log_file << "  index: " << balls.back().GetIndex() << endl;
    log_file << "  pos: " << balls.back().GetPosition().x << ", " << balls.back().GetPosition().y << ", " << balls.back().GetPosition().z << endl;
    log_file << "  radius: " << balls.back().GetRadius() << endl;
#endif

    return true;
}

bool CSoccerMod::InitializeTeam(OBJECT_LIST *pObjects, PAIR_LIST *pPairs)
{
#ifdef CUSTOM_LOG
    log_file << "home cage" << endl;
#endif

    NAMEVALUEPAIR team_id;
    bool result = FindProperty(*pPairs, PROPERTY_TEAM_ID, team_id);

    if (!result)
    {
        // no team specification -> invalid data
        return false;
    }

    NAMEVALUEPAIR score_award;
    result = FindProperty(*pPairs, PROPERTY_SCORE_AWARD, score_award);
    int score_award_points = 1;

    if (result)
    {
        // There's custom score award for this home cage
        stringstream stream;

        stream << score_award.GetString();
        stream >> score_award_points;

        if (stream.bad())
        {
#ifdef CUSTOM_LOG
            log_file << "Custom award error: \"" << score_award.GetString() << "\"" << endl;
#endif
            score_award_points = 1;
        }
    }

    soccer::TeamInfo &current_team = (team_id.GetString() == "0" ? teams.first : teams.second);
    soccer::BoxObject &current_box = current_team.AddHomeCageObject(score_award_points);

    current_team.ResetScore();

#ifdef CUSTOM_LOG
    log_file << "  current_id: " << team_id.GetString() << endl;
    log_file << "  current_team: " << current_team.GetName() << endl;
    log_file << "  obj_count: " << pObjects->size() << endl;
    log_file << "  award: " << score_award_points << endl;
#endif

    for (OBJECT_LIST::const_iterator it = pObjects->begin(); it != pObjects->end(); ++it)
    {
        if (it->bDynamic)
        {
            // skip dynamic objects, not supported yet
            continue;
        }

        physics::CStaticObject *goal_object = pPhysics->staticObjects[it->iIndex];

        D3DXVECTOR3 bbox_min;
        D3DXVECTOR3 bbox_max;

        NewtonBodyGetAABB(goal_object->NBody(), (float *) &bbox_min, (float *) &bbox_max);

        current_box.GetBoundingBox().HomeAddPoint(bbox_min);
        current_box.GetBoundingBox().HomeAddPoint(bbox_max);
        current_box.GetObjectIDs().push_back(it->iIndex);
    }

#ifdef CUSTOM_LOG
    log_file << "  bbox.ll: " << current_box.GetBoundingBox().GetBoundingBoxLL().x << ", " 
                              << current_box.GetBoundingBox().GetBoundingBoxLL().y << ", " 
                              << current_box.GetBoundingBox().GetBoundingBoxLL().z << endl;
    log_file << "  bbox.ur: " << current_box.GetBoundingBox().GetBoundingBoxUR().x << ", " 
                              << current_box.GetBoundingBox().GetBoundingBoxUR().y << ", " 
                              << current_box.GetBoundingBox().GetBoundingBoxUR().z << endl;
#endif

    return true;
}

bool CSoccerMod::AddPlayground(OBJECT_LIST *pObjects, PAIR_LIST *pPairs)
{
    static size_t count = 0;

    ++count;

    if (!play_field_init)
    {
#ifdef CUSTOM_LOG
        log_file << "AddPlayground Init" << endl;
#endif

        play_field_init = true;
        play_field = soccer::PlayField(pPhysics->terrain.GetTerrainWidthSquares(), pPhysics->terrain.GetTerrainHeightSquares());

        play_field.SetDimensions(soccer::SQUARE_SIZE * play_field.GetWidth(), soccer::SQUARE_SIZE * play_field.GetHeight());

#ifdef CUSTOM_LOG
        log_file << " playground size: " << play_field.GetWidth() << ", " << play_field.GetHeight() << endl;
#endif
    }

    // for each object set value of playfield bitmap
    for (OBJECT_LIST::const_iterator it = pObjects->begin(); it != pObjects->end(); ++it)
    {
        play_field(static_cast< size_t >(it->iMapX), static_cast< size_t >(it->iMapY)) = soccer::PlayField::SQUARE_PLAYGROUND;
    }

    return true;
}

bool CSoccerMod::AddStartingPosition(OBJECT_LIST *pObjects, PAIR_LIST *pPairs)
{
#ifdef CUSTOM_LOG
    log_file << "Start pos" << endl;
    log_file << "  objects: " << pObjects->size() << endl;
#endif
    
    // TODO:
    //for (OBJECT_LIST::const_iterator it = pObjects->begin(); it != pObjects->end(); ++it)
    //{

    //}

    return true;
}

HRESULT CSoccerMod::ClientMPStart()
{
#ifdef CUSTOM_LOG
    log_file << "ClientMPStart" << endl;
    log_file << "  dynamic: " << pScene->DynamicObjects.size() << endl;
    log_file << "  static: " << pScene->StaticObjects.size() << endl;
#endif

    controls_enabled = true;
    D3DXMATRIX team_matrix[2];

    D3DXMatrixIdentity(&team_matrix[0]);
    D3DXMatrixIdentity(&team_matrix[1]);

    team_matrix[0]._22 = team_matrix[0]._33 = 0.3f;
    team_matrix[0]._41 = 0.3f;
    team_matrix[1]._11 = team_matrix[1]._22 = 0.3f;
    team_matrix[1]._43 = 0.3f;

#ifdef CUSTOM_LOG

    log_file << "PlayerIDs {c,s}: ";
    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        log_file << "{";
        if (pClientGameInfo->GetPlayerActive((unsigned int ) i))
        {
            log_file << pClientGameInfo->GetPlayerTeamID((UINT) i);
        }
        else
        {
            log_file << "X";
        }

        if (pServerGameInfo->GetPlayerActive((unsigned int ) i))
        {
            log_file << pServerGameInfo->GetPlayerTeamID((UINT) i);
        }
        else
        {
            log_file << "X";
        }

        log_file << "}";
    }
    log_file << endl;
#endif

    for (int i = 0; i < MAX_PLAYERS; i++)
    {

        if (!pClientGameInfo->GetPlayerActive(i))
        {
            continue;
        }

        if (pClientGameInfo->GetPlayerTeamID(i) == 0)
        {
            pScene->Cars[i]->SetColorModifier(&team_matrix[0]);
        }
        else
        {
            pScene->Cars[i]->SetColorModifier(&team_matrix[1]);
        }
    }

    // TODO: write shaders for static objects too, and then this will work...
    //for (vector< soccer::TeamInfo::BoxObject >::iterator home_it = teams.first.GetHomeCageObjects().begin();
    //     home_it != teams.first.GetHomeCageObjects().end();
    //     ++home_it)
    //{
    //    for (std::vector< int >::const_iterator obj_it = home_it->GetObjectIDs().begin();
    //         obj_it != home_it->GetObjectIDs().end();
    //         ++obj_it)
    //    {
    //        pScene->StaticObjects[*obj_it]->SetColorModifier(&team_matrix[0]);
    //    }
    //}

    //for (vector< soccer::TeamInfo::BoxObject >::iterator home_it = teams.second.GetHomeCageObjects().begin();
    //     home_it != teams.second.GetHomeCageObjects().end();
    //     ++home_it)
    //{
    //    for (std::vector< int >::const_iterator obj_it = home_it->GetObjectIDs().begin();
    //         obj_it != home_it->GetObjectIDs().end();
    //         ++obj_it)
    //    {
    //        pScene->StaticObjects[*obj_it]->SetColorModifier(&team_matrix[1]);
    //    }
    //}

#ifdef CUSTOM_LOG
    for (size_t y = 0; y < play_field.GetHeight(); ++y)
    {
        for (size_t x = 0; x < play_field.GetWidth(); ++x)
        {
            log_file << (play_field(x, y) == soccer::PlayField::SQUARE_PLAYGROUND ? 'X' : '.');
        }

        log_file << endl;
    }
#endif

    return ERRNOERROR;
}

HRESULT CSoccerMod::ServerMPStart(void)
{
    ResetBalls();
    match_time = 0;
    SendToAll("");
    SendToAll("", NETMSG_HUD_SCORE);
    game_state.Start();
    play_field_init = false;
    teams.first.ResetScore();
    teams.second.ResetScore();

    return ERRNOERROR;
}

void CSoccerMod::UpdateBalls(void)
{
    for (vector< soccer::BallData >::iterator it = balls.begin(); it != balls.end(); ++it)
    {
        it->Update(pPhysics);
    }
}

void CSoccerMod::ResetBalls(void)
{
    static const float ZERO_VELOCITY[3] = {0.0f, 0.0f, 0.0f};

    for (vector< soccer::BallData >::iterator it = balls.begin(); it != balls.end(); ++it)
    {
        physics::CDynamicObject *object = pPhysics->dynamicObjects[it->GetIndex()];
        object->Position(it->GetStartPos().x, it->GetStartPos().y, it->GetStartPos().z);

        NewtonBody *object_body = object->NBody();
        
        NewtonBodySetVelocity(object_body, ZERO_VELOCITY);
        NewtonBodySetOmega(object_body, ZERO_VELOCITY);
        object->Changed(true);
    }
}

void CSoccerMod::ResetVelocities(void)
{
    static const float ZERO_VELOCITY[3] = {0.0f, 0.0f, 0.0f};

    for (physics::CDynamicObjectVector::iterator it = pPhysics->dynamicObjects.begin(); it != pPhysics->dynamicObjects.end(); ++it)
    {
        NewtonBody *object_body = (*it)->NBody();
        
        if (object_body == NULL)
        {
            continue;
        }

        NewtonBodySetVelocity(object_body, ZERO_VELOCITY);
        NewtonBodySetOmega(object_body, ZERO_VELOCITY);
        (*it)->Changed(true);
    }

    for (size_t i = 0; i < MAX_PLAYERS; ++i)
    {
        if (pPhysics->cars[i] == NULL)
        {
            continue;
        }

        NewtonBody *object_body = pPhysics->cars[i]->NBody();
        if (object_body == NULL)
        {
            continue;
        }
        NewtonBodySetVelocity(object_body, ZERO_VELOCITY);
        NewtonBodySetOmega(object_body, ZERO_VELOCITY);
        pPhysics->cars[i]->Changed(true);
    }
}

HRESULT CSoccerMod::ClientGameMH(WPARAM wParam, LPARAM lParam, ::Network::CNetMsg *msg)
{
    soccer::GameEvent game_event = soccer::GameEvent(msg);

    switch (game_event.GetEventID())
    {
    case soccer::EVENT_GENERIC:
        // do nothing
#ifdef CUSTOM_LOG
        log_file << "EVENT_GENERIC" << endl;
#endif
        break;

    case soccer::EVENT_COUNTDOWN:
#ifdef CUSTOM_LOG
        log_file << "EVENT_COUNTDOWN" << endl;
#endif
        {
            int value = game_event.GetValue() + static_cast< int >(XACT_CUE_CAR_COUNTDOWN1) - 1;
            if ((value > XACT_CUE_CAR_COUNTDOWN10) || (value < XACT_CUE_CAR_COUNTDOWN1))
            {
                break;
            }

            pCommonSounds->pPrimarySoundBank->Play(static_cast< XACTINDEX >(value), 0, 0, NULL);
        }
        break;

    case soccer::EVENT_GOAL:
#ifdef CUSTOM_LOG
        log_file << "EVENT_GOAL" << endl;
#endif
        pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_GODLIKE, 0, 0, NULL);
        break;

    case soccer::EVENT_OUT:
#ifdef CUSTOM_LOG
        log_file << "EVENT_OUT" << endl;
#endif
        pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_HOMMERDOH, 0, 0, NULL);
        break;

    case soccer::EVENT_GAME_START:
#ifdef CUSTOM_LOG
        log_file << "EVENT_GAME_START" << endl;
#endif
        pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_PREPARE, 0, 0, NULL);
        break;

    case soccer::EVENT_GAME_END_YOU_LOST:
#ifdef CUSTOM_LOG
        log_file << "EVENT_GAME_END_YOU_LOST" << endl;
#endif
        pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_LOSTMATCH, 0, 0, NULL);
        controls_enabled = false;
        break;

    case soccer::EVENT_GAME_END_YOU_WON:
#ifdef CUSTOM_LOG
        log_file << "EVENT_GAME_END_YOU_WON" << endl;
#endif
        pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_WINNERCONGRAT, 0, 0, NULL);
        controls_enabled = false;
        break;

    case soccer::EVENT_5MIN_REMAINING:
#ifdef CUSTOM_LOG
        log_file << "EVENT_5MIN_REMAINING" << endl;
#endif
        pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_COUNTDOWN5MIN, 0, 0, NULL);
        break;

    case soccer::EVENT_3MIN_REMAINING:
#ifdef CUSTOM_LOG
        log_file << "EVENT_3MIN_REMAINING" << endl;
#endif
        pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_COUNTDOWN3MIN, 0, 0, NULL);
        break;

    case soccer::EVENT_1MIN_REMAINING:
#ifdef CUSTOM_LOG
        log_file << "EVENT_1MIN_REMAINING" << endl;
#endif
        pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_COUNTDOWN1MIN, 0, 0, NULL);
        break;

    case soccer::EVENT_30SEC_REMAINING:
#ifdef CUSTOM_LOG
        log_file << "EVENT_30SEC_REMAINING" << endl;
#endif
        pCommonSounds->pPrimarySoundBank->Play(XACT_CUE_CAR_COUNTDOWN30SEC, 0, 0, NULL);
        break;
    }

    return ERRNOERROR;
}

bool CSoccerMod::ClientControlsEnabled()
{
    //return controls_enabled;
    return true;
}

void CSoccerMod::SendToAll(const CString &message_text, WORD msg_id)
{
    Network::CNetMsg message;
    Network::SetMsgString(&message, msg_id, const_cast< CString & >(message_text));

    pNetServer->SendMsg(&message, -1);
}

CString CSoccerMod::ScoreString(void) const
{
    stringstream stream;

    stream << teams.first.GetScore() << ":" << teams.second.GetScore();

    return CString(stream.str().c_str());
}

bool FindProperty(const std::vector< NAMEVALUEPAIR > &props, const CString &name, NAMEVALUEPAIR &result)
{
    for (std::vector< NAMEVALUEPAIR >::const_iterator it = props.begin(); it != props.end(); ++it)
    {
        if (it->GetName() == name)
        {
            result = *it;
            return true;
        }
    }

    return false;
}

bool HasProperty(const std::vector< NAMEVALUEPAIR > &props, const CString &name)
{
    for (std::vector< NAMEVALUEPAIR >::const_iterator it = props.begin(); it != props.end(); ++it)
    {
        if (it->GetName() == name)
        {
            return true;
        }
    }

    return false;
}
