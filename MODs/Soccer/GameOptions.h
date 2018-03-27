#ifndef GAME_OPTIONS_INCLUDED
#define GAME_OPTIONS_INCLUDED

#include "stdafx.h"
#include <strstream>

namespace soccer
{
    enum GameType
    {
        GAME_ENDLESS,
        GAME_SCORE_CAP,
        GAME_TIME_CAP
    };

    enum GameEnd
    {
        END_NO_END, // :)
        END_TEAM1_WON,
        END_TEAM2_WON
    };

    // Game options (eg. winning conditions...)
    class GameOptions
    {
    public:
        explicit GameOptions(GameType gt = GAME_ENDLESS, size_t score_max = 10, size_t time_max = 600) :
        score_cap(score_max),
        time_cap(time_max),
        game_type(gt)
        {
        }

        GameType GetGameType(void) const
        {
            return game_type;
        }

        void SetGameType(GameType type)
        {
            game_type = type;
        }

        bool SetGameType(const CString &value)
        {
            if (value.CompareNoCase("Endless") == 0)
            {
                game_type = GAME_ENDLESS;
                return true;
            }

            if (value.CompareNoCase("ScoreCap") == 0)
            {
                game_type = GAME_SCORE_CAP;
                return true;
            }

            if (value.CompareNoCase("TimeCap") == 0)
            {
                game_type = GAME_TIME_CAP;
                return true;
            }

            game_type = GAME_ENDLESS;
            return false;
        }

        CString GetGameTypeString(void) const
        {
            switch (game_type)
            {
            case GAME_ENDLESS:
                return "Endless";

            case GAME_SCORE_CAP:
                return "ScoreCap";

            case GAME_TIME_CAP:
                return "TimeCap";
            }
        }

        size_t GetScoreCap(void) const
        {
            return score_cap;
        }

        size_t SetScoreCap(size_t new_cap)
        {
            if (new_cap == 0)
            {
                score_cap = 10;
            }
            else
            {
                score_cap = new_cap;
            }

            return score_cap;
        }

        size_t GetTimeCap(void) const
        {
            return time_cap;
        }

        size_t SetTimeCap(size_t new_cap)
        {
            if (new_cap < 60)
            {
                time_cap = 60;
            }
            else
            {
                time_cap = new_cap;
            }

            return time_cap;
        }

        GameEnd TestConditions(APPRUNTIME time, const std::pair< TeamInfo, TeamInfo > &teams)
        {
            switch (game_type)
            {
            case GAME_ENDLESS:
                return END_NO_END;

            case GAME_SCORE_CAP:
                if (teams.first.GetScore() >= static_cast< int >(score_cap))
                {
                    return END_TEAM1_WON;
                }
                if (teams.second.GetScore() >= static_cast< int >(score_cap))
                {
                    return END_TEAM2_WON;
                }
                return END_NO_END;

            case GAME_TIME_CAP:
                if (time / 1000 >= time_cap)
                {
                    if (teams.first.GetScore() >= teams.second.GetScore())
                    {
                        return END_TEAM1_WON;
                    }
                    else if (teams.second.GetScore() > teams.first.GetScore())
                    {
                        return END_TEAM2_WON;
                    }
                }
                return END_NO_END; // time's not up or both teams have the same score
            }

            return END_NO_END;
        }

    private:
        size_t score_cap;
        size_t time_cap;
        GameType game_type;
    };
}

#endif
