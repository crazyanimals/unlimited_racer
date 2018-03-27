#pragma once

#include <math.h>
#include <vector>
#include "AI_Common.h"



namespace AI_NS {

    class CAIPath {
    private:
        float   fRadius, * pLengths, fTotalPathLength;
        V3    * pNormals;
        int     iPointCount;
    public:
        WVec        wvWaypoints;
        int         Initialize              (WVec & waypoints, float radius = DefaultRadius);
        V3          MapPointToPath          (const V3 & point, float & outside) const;
        V3          MapPathDistanceToPoint  (const float path_distance, float & min_speed, float & max_speed) const;
        IVec        MapPointToWaypoints     (const V3 & point, float fDistanceTolerance = DistanceTolerance) const;
        float       MapPointToPathDistance  (const V3 & point, int iLastValidWaypoint) const;
        float       HowFarOutsidePath       (const V3 & point) const;
        bool        IsInsidePath            (const V3 & point) const;
        float       PointToSegmentDistance  (const V3& point, int segment_index, V3 & segment_point, float & segment_projection) const;
        float       GetTotalPathLength      () const;
        V3          GetNormal               (const int iSegment) const;
    };
};