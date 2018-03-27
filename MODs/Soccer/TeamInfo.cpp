//
// Implementation of some methods from the TeamInfo class
//
// (c) 2007 Branislav Repcek
//

#include "stdafx.h"
#include "BoundingBox.h"
#include "TeamInfo.h"

namespace soccer
{
    TeamInfo::TeamInfo(const ATL::CString &team_name) :
    name(team_name),
    score(0)
    {
    }

    bool TeamInfo::InsideHome(const D3DXVECTOR3 &point, int &award) const
    {
        for (std::vector< BoxObject >::const_iterator it = objects.begin(); it != objects.end(); ++it)
        {
            if (it->GetBoundingBox().IsInside(point))
            {
                award = it->GetScoreBonus();
                return true;
            }
        }

        award = 0;
        return false;
    }

    bool TeamInfo::InsideHome(const D3DXVECTOR3 &center, float radius, int &award) const
    {
        for (std::vector< BoxObject >::const_iterator it = objects.begin(); it != objects.end(); ++it)
        {
            if (it->GetBoundingBox().IsInside(center, radius))
            {
                award = it->GetScoreBonus();
                return true;
            }
        }

        award = 0;
        return false;
    }

    bool TeamInfo::InsideHomeFull(const std::vector< BallData > &ball_data, int &award) const
    {
        for (std::vector< BallData >::const_iterator it = ball_data.begin(); it != ball_data.end(); ++it)
        {
            if (InsideHome(it->GetPosition(), it->GetRadius(), award))
            {
                return true;
            }
        }

        award = 0;
        return false;
    }

    bool TeamInfo::InsideHomeCenter(const std::vector< BallData > &ball_data, int &award) const
    {
        int index = 0;
        for (std::vector< BallData >::const_iterator it = ball_data.begin(); it != ball_data.end(); ++it, ++index)
        {
            if (InsideHome(it->GetPosition(), award))
            {
                return true;
            }
        }

        award = 0;
        return false;
    }
}
