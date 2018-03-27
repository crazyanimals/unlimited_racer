//
// Soccer game events
//
// (c) 2007 Branislav Repcek
//

#ifndef GAME_EVENT_INCLUDED
#define GAME_EVENT_INCLUDED

#include "stdafx.h"
#include "../../Network/CNetServer.h"
#include "../../Network/CommonMessages.h"
#include "../../Network/CNetMsg.h"

namespace soccer
{
    // Id of the game event
    enum GameEventID
    {
        EVENT_GENERIC,
        EVENT_COUNTDOWN,
        EVENT_GOAL,
        EVENT_OUT,
        EVENT_GAME_START,
        EVENT_GAME_END_YOU_WON,
        EVENT_GAME_END_YOU_LOST,
        EVENT_5MIN_REMAINING,
        EVENT_3MIN_REMAINING,
        EVENT_1MIN_REMAINING,
        EVENT_30SEC_REMAINING
    };

    // Game event class
    class GameEvent
    {
    public:
        // Create event from the network message
        explicit GameEvent(const ::Network::CNetMsg *message)
        {
            memcpy((void *) &data, const_cast< ::Network::CNetMsg * >(message)->getData(), sizeof(Data));
        }

        // Create event
        explicit GameEvent(GameEventID id, int value = 0)
        {
            data.event_id = id;
            data.value = value;
        }

        // Get id of the event
        GameEventID GetEventID(void) const
        {
            return data.event_id;
        }

        // Get value of the event
        int GetValue(void) const
        {
            return data.value;
        }

        // Send this event to all players on the server
        bool SendMessageAll(::Network::CNetServer *server) const
        {
            if (server == NULL)
            {
                return false;
            }

            ::Network::CNetMsg message;
            
            if (!BuildMessage(message))
            {
                return false;
            }

            HRESULT res = server->SendMsg(&message, -1);
            
            return SUCCEEDED(res) == TRUE;
        }

        // Send message to all players of given team on the given server
        bool SendMessageTeam(size_t team_id, ::Network::CNetServer *server, CMultiPlayerGameInfo *game_info) const
        {
            if (server == NULL)
            {
                return false;
            }

            ::Network::CNetMsg message;
            
            if (!BuildMessage(message))
            {
                return false;
            }

            for (unsigned int i = 0; i < MAX_PLAYERS; ++i)
            {
                if (!game_info->GetPlayerActive(i))
                {
                    continue;
                }

                if (team_id == game_info->GetPlayerTeamID(i))
                {
                    if (FAILED(server->SendMsg(&message, static_cast< int >(i))))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        // Send message to the player on the given server
        bool SendMessagePlayer(int player_id, ::Network::CNetServer *server) const
        {
            if (server == NULL)
            {
                return false;
            }

            ::Network::CNetMsg message;
            
            if (!BuildMessage(message))
            {
                return false;
            }

            return SUCCEEDED(server->SendMsg(&message, player_id)) == TRUE;
        }

    private:

        // Build network message containing event's data
        bool BuildMessage(::Network::CNetMsg &message) const
        {
            HRESULT hres = message.setNetMsg(NETMSG_SOCCER_GAME_EVENT, (BYTE *) &data, sizeof(Data));
            
            if (FAILED(hres))
            {
                false;
            }

            return true;
        }

        // Data storage
        struct Data
        {
            GameEventID event_id;  // event id
            int value;             // event value
        };

        // Event data
        Data data;
    };
}

#endif
