//
// Header file for the GameState class
//
// (c) 2007 Branislav Repcek
//

#ifndef GAME_STATE_INCLUDED
#define GAME_STATE_INCLUDED

#include "stdafx.h"
#include "../../Globals/GlobalsIncludes.h"

namespace soccer
{
    // Stores details about current game state
    class GameState
    {
    public:
        // Create new state object
        GameState(void) :
        state(GAME_START),
        state_change(0),
        which_team(NULL),
        score_delta(0)
        {
        }

        // Game states
        enum State
        {
            NORMAL_GAME, // "Normal game", nothing interesting happened
            AFTER_GOAL,  // Right after goal has been scored and before ball reset
            AFTER_OUT,   // Right after the ball has been pushed out of the playground
            GAME_START,  // Just after the game has begun
            GAME_END     // Game has ended
        };

        void Goal(APPRUNTIME time, TeamInfo *team, int delta = 1)
        {
            state_change = time;
            state = AFTER_GOAL;
            which_team = team;
            score_delta = delta;
        }

        void Out(APPRUNTIME time)
        {
            state = AFTER_OUT;
            state_change = time;
        }

        void Game(APPRUNTIME time)
        {
            state = NORMAL_GAME;
            state_change = time;
        }

        void Start(void)
        {
            state = GAME_START;
            state_change = 0;
            score_delta = 0;
            which_team = NULL;
        }

        void End(APPRUNTIME time)
        {
            state = GAME_END;
            state_change = time;
            score_delta = 0;
            which_team = NULL;
        }

        APPRUNTIME state_change;  // last state flip time
        State      state;         // current game state
        TeamInfo   *which_team;   // players of which team caused an event, used only when state is AFTER_GOAL
        int        score_delta;   // how much has score changed
    };
}

#endif
