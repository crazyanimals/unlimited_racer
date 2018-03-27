#pragma once

#include <math.h>
#include <vector>
#include <CString>
#include "V3.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\Globals\Globals.h"

#define PI  3.14159265358979323846f
#define SQRT2                           1.4142135623731f

// if true and distance between 2 waypoints is greater than this, add new waypoints between them
#define OPTIMIZE_WAYPOINT_PLACEMENT     false
#define MAX_WAYPOINT_DISTANCE           (5 * ONE_METER)




extern float UpdateTimeClip;
extern float CarMaxSpeed;
extern float CarSpeedTreshold;
extern float CarSpeedChangeProbability;
extern float CarSpeedChange;
extern float MassAvoidanceCoef;
extern float LeadTime;
extern int   CarTimeBeforeRestart;
extern float CarTrackProgress;
extern float LeakThrough;
extern float DistanceTolerance;
extern int   MaxWaypointsMissed;
extern float MapToPathLocalDistance;
extern float DefaultRadius;   
extern bool  OptimizeWaypointPlacement;
extern float MaxWaypointDistance;

namespace AI_NS {
    
    typedef std::vector<int> IVec;

    typedef struct __WaypointStruct {
        V3    point;
        float fMinSpeed, fMaxSpeed;
    } WaypointStruct;

    typedef std::vector<WaypointStruct> WVec;
};

float fRandom01();
