#include "CAIPath.h"

using namespace AI_NS;

int     CAIPath::Initialize(WVec & waypoints, float radius) {
    wvWaypoints = waypoints;
    fRadius = radius;
    fTotalPathLength = 0;
    wvWaypoints.push_back(wvWaypoints[0]);
    iPointCount = (int) wvWaypoints.size();
    pLengths = new float [iPointCount];
    pNormals = new V3    [iPointCount];

    // first and last waypoint are identical
    pNormals[0] = V3::Zero();
    pLengths[0] = 0;
    // loop over all points
    for (int i = 1; i < iPointCount; i++)  {
        // compute the segment length
        pNormals[i] = wvWaypoints[i].point - wvWaypoints[i - 1].point;
        pLengths[i] = pNormals[i].Length();

        // find the normalized vector parallel to the segment
        pNormals[i] = pNormals[i].Normalize();

        // keep running total of segment lengths
        fTotalPathLength += pLengths[i];
    }  
    return 1;
}

V3      CAIPath::MapPointToPath(const V3 &point, float &outside) const {
    float fMinDistance = FLT_MAX, d, dummy;
    V3 vOnPath, vSegmentPoint;

    // loop over all segments, find the one nearest to the given point
    for (int i = 1; i < iPointCount; i++)  {
        d = PointToSegmentDistance (point, i, vSegmentPoint, dummy);
        if (d < fMinDistance) {
            fMinDistance = d;
            vOnPath = vSegmentPoint;
        }
    }

    // measure how far original point is outside the Pathway's "tube"
    outside = vOnPath.Distance(point) - fRadius;

    // return point on path
    return vOnPath;
}

V3      CAIPath::MapPathDistanceToPoint(const float path_distance, float & min_speed, float & max_speed) const {
    int i;
    float remaining = path_distance;
    // clip or wrap given path distance according to cyclic flag
    //if (bCyclic) 
        remaining = (float) fmod (path_distance, fTotalPathLength);
    //else {
    //    if (path_distance < 0) return wvWaypoints[0].point;
    //    if (path_distance >= fTotalPathLength) return wvWaypoints[iPointCount-1].point;
    //}

    // step through segments, subtracting off segment lengths until
    // locating the segment that contains the original pathDistance.
    // Interpolate along that segment to find 3d point value to return.
    for (i = 1; i < iPointCount && pLengths[i] <= remaining; i++)
        remaining -= pLengths[i];
    min_speed  = wvWaypoints[i-1].fMinSpeed + (remaining / pLengths[i]) * (wvWaypoints[i].fMinSpeed - wvWaypoints[i-1].fMinSpeed);
    max_speed  = wvWaypoints[i-1].fMaxSpeed + (remaining / pLengths[i]) * (wvWaypoints[i].fMaxSpeed - wvWaypoints[i-1].fMaxSpeed);
    return wvWaypoints[i-1].point + pNormals[i] * remaining;
}

float   CAIPath::MapPointToPathDistance(const V3 & point, int iLastValidWaypoint) const {
    float d;
    float fMinDistance = FLT_MAX;
    float fSegmentLengthTotal = 0;
    float fSegmentProjection;
    float fPathDistance = 0;
    V3    vSegmentPoint;
    int i;

    for (i = 1; i < iLastValidWaypoint; i++)
        fSegmentLengthTotal += pLengths[i];
    
    for (
        i = iLastValidWaypoint; 
        i < iPointCount + iLastValidWaypoint && wvWaypoints[i % iPointCount].point.Distance(wvWaypoints[iLastValidWaypoint].point) < MapToPathLocalDistance;
        i++
        ) 
    {
        d = PointToSegmentDistance (point, i % iPointCount, vSegmentPoint, fSegmentProjection);
        if (d < fMinDistance)  {
            fMinDistance = d;
            fPathDistance = fSegmentLengthTotal + fSegmentProjection;
        }
        fSegmentLengthTotal += pLengths[i % iPointCount];
    }

    // return distance along path of onPath point
    return fPathDistance;
}

float   CAIPath::PointToSegmentDistance(const V3 & point, int segment_index, V3 & segment_point, float & segment_projection) const {
    
    segment_projection = pNormals[segment_index].DotProduct(point - wvWaypoints[(segment_index - 1 + iPointCount) % iPointCount].point); 

    // handle boundary cases: when projection is not on segment, the
    // nearest point is one of the endpoints of the segment
    if (segment_projection < 0)  {
        segment_point = wvWaypoints[(segment_index - 1 + iPointCount) % iPointCount].point;
        segment_projection = 0;
    } else if (segment_projection > pLengths[segment_index])  {
        segment_point = wvWaypoints[segment_index].point;
        segment_projection = pLengths[segment_index];
    } else {
            // otherwise nearest point is projection point on segment
            segment_point = pNormals[segment_index] * segment_projection;
            segment_point += wvWaypoints[(segment_index - 1 + iPointCount) % iPointCount].point;
    }
    return point.Distance(segment_point);
}

float   CAIPath::HowFarOutsidePath(const V3 &point) const {
    float outside;
    MapPointToPath(point, outside);
    return outside;
}

bool    CAIPath::IsInsidePath(const V3 &point) const {
    float outside; 
    MapPointToPath(point, outside);
    return outside < 0;
}

IVec    CAIPath::MapPointToWaypoints(const V3 &point, float fDistanceTolerance) const {
    int i;
    IVec ivResult;
    for (i = 0; i < (int) wvWaypoints.size(); i++)
        if (point.Distance(wvWaypoints[i].point) < fDistanceTolerance)
            ivResult.push_back(i);
    return ivResult;
}

float   CAIPath::GetTotalPathLength() const {
    return fTotalPathLength;
}

V3      CAIPath::GetNormal(int iSegment) const {
    iSegment = ((iSegment % iPointCount) + iPointCount) % iPointCount;
    return pNormals[iSegment];
}