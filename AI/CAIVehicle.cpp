#define _CRT_RAND_S
#include <stdlib.h>
#include "CAIVehicle.h"

float fRandom01() {
    unsigned int ui;
    rand_s(&ui);
    return ((float) ui) / ((float) UINT_MAX);
}

using namespace AI_NS;

CAIVehicle::CAIVehicle() {
    iLastValidWaypoint = 0;
    fMinSpeed = 0.0; 
    fMaxSpeed = CarMaxSpeed;
    fSpeedAdjust =  (float) (1 + (fRandom01() - 0.5f) * 2 * CarSpeedChange);
    iRoundCount = 0;
    fTimeAccumulator = 0;
    vDesiredVelocity = V3::Zero();
    bRestartVehicle  = false;
    fLog = NULL;
    pvLastPosition = new V3[CarTimeBeforeRestart];
    pfLastProgress = new float[CarTimeBeforeRestart];
    ClearRestartStructures();
}

CAIVehicle::~CAIVehicle() {
    if (fLog) fclose(fLog);
    delete[] pvLastPosition;
    delete[] pfLastProgress;
}

CAIVehicle::CAIVehicle(const CAIVehicle &v) {
    int i;
    fMinSpeed           = v.fMinSpeed;
    fMaxSpeed           = v.fMaxSpeed;
    fSpeedAdjust        = v.fSpeedAdjust;
    iLastValidWaypoint  = v.iLastValidWaypoint;
    ivWaypointsPassed   = v.ivWaypointsPassed;
    vForward            = v.vForward;
    vDesiredVelocity    = v.vDesiredVelocity;
    bRestartVehicle     = v.bRestartVehicle;
    fTimeAccumulator    = v.fTimeAccumulator;
    fLog                = v.fLog;
    pvLastPosition = new V3[CarTimeBeforeRestart];
    pfLastProgress = new float[CarTimeBeforeRestart];
    for (i = 0; i < CarTimeBeforeRestart; i++) {
        pvLastPosition[i] = v.pvLastPosition[i];
        pfLastProgress[i] = v.pfLastProgress[i];
    }

}

void    CAIVehicle::UpdatePositionOnMap(const CAIPath * path) {
    // what waypoints are nearby?
    IVec ivWaypointIndexes = path->MapPointToWaypoints(Position());
    int i, j, size = (int) ivWaypointIndexes.size();
    if (fLog) {
        fprintf_s(fLog, "[%f, %f, %f] - nearby waypoints:\n", Position().x, Position().y, Position().z);
        for (i = 0; i < size; i++)
            fprintf_s(fLog, "#%i - [%f, %f, %f], distance: %f\n", ivWaypointIndexes[i], 
                                                                  path->wvWaypoints[ivWaypointIndexes[i]].point.x,
                                                                  path->wvWaypoints[ivWaypointIndexes[i]].point.y,
                                                                  path->wvWaypoints[ivWaypointIndexes[i]].point.z,
                                                                  Position().Distance(path->wvWaypoints[ivWaypointIndexes[i]].point)
                                                                  );
    }
    // we are lost, no waypoints nearby
    if (!size) return;
    // let's find waypoint we are nearby to, which is just after iLastValidWaypoint
    for (i = 0; i < size && ivWaypointIndexes[i] <= iLastValidWaypoint; i++);
    // there is no such waypoint, quit
    if  (i == size) return;
    // now find which waypoints on path we actually passed around, with a tolerance - we can miss few of them
    for (; i < size && ivWaypointIndexes[i] <= iLastValidWaypoint + MaxWaypointsMissed; i++) {
        // add all of them there, including the missed ones
        for (j = iLastValidWaypoint + 1; j <= ivWaypointIndexes[i]; j++)
            ivWaypointsPassed.push_back(j);            
        // update iLastValidWaypoint
        iLastValidWaypoint = ivWaypointIndexes[i];
    }
    // did we get to the end of the list and is the last nearby waypoint the path's last waypoint?
    if (i == size && ivWaypointIndexes[i-1] == path->wvWaypoints.size() - 1) {
        // it is, we have just crossed the finish line - increase round count
        iRoundCount++;
        // set iLastValidWaypoint to beginning and mark it as passed
        iLastValidWaypoint = 0;
        ivWaypointsPassed.push_back(0);
        // repeat the whole thing form beginning
        for (i = 1; i < size && ivWaypointIndexes[i] == iLastValidWaypoint + MaxWaypointsMissed; i++) {
            for (j = iLastValidWaypoint + 1; j <= ivWaypointIndexes[i]; j++)
                ivWaypointsPassed.push_back(j);            
            iLastValidWaypoint = ivWaypointIndexes[i];
        }
    }    
}

void    CAIVehicle::Update(const float fPredictionTime, const ObjectVec & objects, const CAIPath * path) {
    int i;
    float fDistance = 0;
    // are we stuck?
    for (i = 1; i < CarTimeBeforeRestart; i++)
        fDistance += pvLastPosition[i-1].Distance(pvLastPosition[i]);
    if (fDistance < CarTrackProgress || pfLastProgress[CarTimeBeforeRestart -1] - pfLastProgress[0] < CarTrackProgress * 2)
        bRestartVehicle = true;
    // update stuck information:
    fTimeAccumulator += fPredictionTime;
    while (fTimeAccumulator >= 1.0) {
        fTimeAccumulator -= 1.0;
        for (i = 1; i < CarTimeBeforeRestart; i++) {
            pvLastPosition[i-1] = pvLastPosition[i];
            pfLastProgress[i-1] = pfLastProgress[i];
        }
        pvLastPosition[CarTimeBeforeRestart -1] = Position();
        pfLastProgress[CarTimeBeforeRestart -1] = path->MapPointToPathDistance(path->wvWaypoints[iLastValidWaypoint].point, iLastValidWaypoint) + path->GetTotalPathLength() * iRoundCount;
    }
    if (fRandom01() < CarSpeedChangeProbability) fSpeedAdjust = (float) (1 + (fRandom01() - 0.5f) * 2 * CarSpeedChange);
    // compute steering force
    vDesiredVelocity = CombinedSteering(fPredictionTime, objects, path);
    // update position on map to compute race state
    UpdatePositionOnMap(path);
};


V3      CAIVehicle::Forward() const {
    return vForward;
}

V3      CAIVehicle::CombinedSteering(const float fPredictionTime, const ObjectVec & objects, const CAIPath * path) {
    // move forward
    V3 vSteering;
    V3 vPathFollow;
    V3 vCollisionAvoidance = V3::Zero();

    fMinSpeed = 0;
    fMaxSpeed = CarMaxSpeed;

    // avoid collisions?
    if (LeakThrough > fRandom01()) 
        vCollisionAvoidance = AvoidCollision(LeadTime * 4, objects);

    // just in case we do not need to avoid a collision. 
    // As a side effect, this call sets appropriate fMinSpeed & fMaxSpeed according to waypoint we are currently at.
    vPathFollow = SteerToFollowPath (LeadTime, path);

    if (vCollisionAvoidance != V3::Zero()) vSteering = vCollisionAvoidance;
    else vSteering = vPathFollow;

    return vSteering;
}

V3      CAIVehicle::SteerToAvoidCloseObjects(const float fMinSeparationDistance, const ObjectVec &objects) {
    size_t i;
    for (i = 0; i < objects.size(); i++) {
        CAIObject * object = objects[i];
        if (object && object != this && object->Mass() * MassAvoidanceCoef > Mass()) {
            V3 vOffset = Position() - object->Position();
            if (vOffset.Length() < fMinSeparationDistance + Radius() + object->Radius()) {
                fMinSpeed = CarMaxSpeed;
                return vOffset.PerpendicularComponent(Forward());
            }
        }
    }

    // otherwise return zero
    return V3::Zero();
}


V3      CAIVehicle::SteerToAvoidObjects(const float fMinTimeToCollision, const ObjectVec & objects) {  
    // first priority is to prevent immediate interpenetration
    const V3 vSeparation = SteerToAvoidCloseObjects (5 * ONE_METER, objects);
    if (vSeparation != V3::Zero()) 
        return vSeparation;

    // otherwise, go on to consider potential future collisions
    float steer = 0;
    CAIObject * threat = NULL;

    // Time (in seconds) until the most immediate collision threat found
    // so far.  Initial value is a threshold: don't look more than this
    // many frames into the future.
    float fMinTime = fMinTimeToCollision;

    // for each of the other vehicles, determine which (if any)
    // pose the most immediate threat of collision.
    for (size_t i = 0; i < objects.size(); i++) {
        CAIObject * object = objects[i];
        if (object && object != this)  {	
            // predicted time until nearest approach of "this" and "other"
            float fTime = PredictNearestApproachTime (object);

            // If the time is in the future, sooner than any other
            // threatened collision...
            if ((fTime >= 0) && (fTime < fMinTime))  
                // if the two will be close enough to collide,
                // make a note of it
                if (ComputeNearestApproachPositions (object, fTime) < Radius() + object->Radius())  {
                    fMinTime = fTime;
                    threat = object;
                }
        }
    }

    // if a potential collision was found, compute steering to avoid
    if (threat != NULL)  {
        if (fLog) {
            fprintf_s(fLog, "Steering away from object at position [%f, %f, %f]\n", threat->Position().x,
                                                                                    threat->Position().y, 
                                                                                    threat->Position().z); 
        }
        // parallel: +1, perpendicular: 0, anti-parallel: -1
        float parallelness = Forward().DotProduct(threat->Forward());
        float angle = 0.707f;

        if (parallelness < -angle)   {
            // anti-parallel "head on" paths:
            // steer away from future threat position
            V3 offset = (threat->Position() + threat->Velocity() * fMinTime) - Position();
            steer = (offset.DotProduct(Side()) > 0) ? -1.0f : 1.0f;
        }  else if (parallelness > angle)  
                // parallel paths: steer away from threat
                    steer = ((threat->Position() - Position()).DotProduct(Side()) > 0) ? -1.0f : 1.0f;
                // perpendicular paths: steer behind threat
           else     steer = (Side().DotProduct(threat->Velocity()) > 0) ? -1.0f : 1.0f;
    }

    return Side() * steer;
}

V3  CAIVehicle::SteerToFollowPath(const float fPredictionTime, const CAIPath *path) {
    V3 vResult;
    // our goal will be offset from our path distance by this amount
    const float fPathDistanceOffset = fPredictionTime * Speed();

    // predict our future position
    const V3 vFuturePosition = PredictFuturePosition (fPredictionTime);

    // measure distance along path of our current and predicted positions
    const float fNowPathDistance     = path->MapPointToPathDistance(Position(), iLastValidWaypoint);
    const float fFuturePathDistance  = path->MapPointToPathDistance(vFuturePosition, iLastValidWaypoint);

    // are we facing in the correction direction?
    const bool bRightway = (fNowPathDistance < fFuturePathDistance);

    //// find the point on the path nearest the predicted future position
    float fOutside, fDummy;
    //we need just fOutside
    path->MapPointToPath(vFuturePosition, fOutside);

    // no steering is required if (a) our future position is inside
    // the path tube and (b) we are facing in the correct direction
    
    vResult = ((fOutside < 0) && bRightway) ?
        // all is fine, keep the current velocity
        Velocity()
        :
        // otherwise we need to steer towards a target point obtained
        // by adding fPathDistanceOffset to our current path position
        // return steering to seek target on path
        //SteerForSeek (path->MapPathDistanceToPoint(fNowPathDistance + fPathDistanceOffset));*/
        (path->MapPathDistanceToPoint(fNowPathDistance + fPathDistanceOffset, fDummy, fDummy) - Position()) / fPredictionTime;

        path->MapPathDistanceToPoint(fNowPathDistance, fMinSpeed, fMaxSpeed);

    if (fLog) 
        fprintf_s(fLog, 
                  "iLVW = %i, P[%f, %f, %f], Steering to [%f, %f, %f] \n", 
                  iLastValidWaypoint,
                  Position().x, 
                  Position().y,
                  Position().z,
                  (vResult * fPredictionTime + Position()).x,
                  (vResult * fPredictionTime + Position()).y,
                  (vResult * fPredictionTime + Position()).z
                  );

    return vResult;
}
//
//V3  CAIVehicle::SteerForSeek(const V3 &vTarget) const {
//    return (vTarget - Position()) - Velocity();
//}

void CAIVehicle::RaceInfo(int & _LastValidWaypoint, int & _RoundsFinished) const {
    _LastValidWaypoint = iLastValidWaypoint;
    _RoundsFinished = iRoundCount;
}


void  CAIVehicle::GetDesiredVelocity(V3 & vel, float & min_speed, float & max_speed) const {
    vel = vDesiredVelocity;
    min_speed = fMinSpeed * fSpeedAdjust;
    max_speed = fMaxSpeed * fSpeedAdjust;
}

void CAIVehicle::SetForward(const AI_NS::V3 &forward) {
    vForward = forward;
}

bool    CAIVehicle::NeedRestart() const {
    return bRestartVehicle;
}

void    CAIVehicle::ClearRestartStructures() {
    int i;
    bRestartVehicle = false;
    for (i = 0; i < CarTimeBeforeRestart; i++) {
        pvLastPosition[i] = V3(-999999, -999999, -999999);
        pfLastProgress[i] = -9999.0f;
    }
    pvLastPosition[CarTimeBeforeRestart - 1] = Position();
    pfLastProgress[CarTimeBeforeRestart - 1] = 0.0f;
}

void    CAIVehicle::StartLogging(CAtlString sLogName) {
    if (fLog) fclose(fLog);
    fopen_s(&fLog, sLogName, "w");
}

//V3 LimitMaxDeviationAngle (const V3 & vSource, const float fCosineOfConeAngle, const V3 & vBasis) {       
//    float fSourceLength = vSource.Length();
//    if (fSourceLength == 0) return vSource;
//
//    // measure the angular diviation of "vSource" from "vBasis"
//    V3 vDir = vSource / fSourceLength;
//    float fCosineOfSourceAngle = vDir.DotProduct(vBasis);
//
//    // Simply return "vSource" if it already meets the angle criteria.
//    // (note: we hope this top "if" gets compiled out since the flag
//    // is a constant when the function is inlined into its caller)
//
//	// vSource vector is already inside the cone, just return it
//	if (fCosineOfSourceAngle >= fCosineOfConeAngle) return vSource;
//
//    // find normalized portion of "vSource" that is perpendicular to "vBasis"
//    V3 vUnitPerp = vSource.PerpendicularComponent(vBasis).Normalize();
//
//    // construct a new vector whose length equals the vSource vector,
//    // and lies on the intersection of a plane (formed the vSource and
//    // vBasis vectors) and a cone (whose axis is "vBasis" and whose
//    // angle corresponds to fCosineOfConeAngle)
//    return (vBasis * fCosineOfConeAngle + vUnitPerp * sqrtf(1 - (fCosineOfConeAngle * fCosineOfConeAngle))) * vSource.Length();        
//}
//
//V3  CAIVehicle::AdjustRawSteeringForce(const V3 &vForce) const {
//    float fMaxAdjustedSpeed = 0.2f * CarMaxSpeed;
//    float fRange = Speed() / fMaxAdjustedSpeed;
//    float fCosine = -1.0f + 2.0f * pow (fRange, 20);
//    
//    if ((Speed() > fMaxAdjustedSpeed) || (vForce == V3::Zero()))  return vForce;
//    else return LimitMaxDeviationAngle (vForce, fCosine, Forward());
// 
//}

float Clip(const float fValue, const float fMin, const float fMax) {
    if (fValue < fMin) return fMin;
    if (fValue > fMax) return fMax;
    return fValue;
}

//void CAIVehicle::ApplySteeringForce(const V3 & vForce, const float fPredictionTime) {
//    V3 vAdjustedForce = /*AdjustRawSteeringForce*/Velocity() - vForce;
//   
//    //vAdjustedForce = (vAdjustedForce.Length() > CAR_MAX_FORCE) ? (vAdjustedForce.Normalize() * CAR_MAX_FORCE) : vAdjustedForce;
//    //vDesiredVelocity = Velocity() + vAdjustedForce /* Mass()*/ * 10 * fPredictionTime;
//    vDesiredVelocity = vForce;
//    if (fLog) fprintf_s(fLog, "Velocity = [%f, %f, %f], DesiredVelocity = [%f, %f, %f]\n", Velocity().x,
//                                                                                           Velocity().y,
//                                                                                           Velocity().z,
//                                                                                           vDesiredVelocity.x,
//                                                                                           vDesiredVelocity.y,
//                                                                                           vDesiredVelocity.z
//                                                                                           );
//    if (fLog) fprintf_s(fLog, "fTime = %f, vAdjustedForce = [%f, %f, %f]\n", fPredictionTime,
//                                                                               vAdjustedForce.x,
//                                                                               vAdjustedForce.y,
//                                                                               vAdjustedForce.z
//                                                                                              );
//}

void    CAIVehicle::SetSide(const V3 &side) {
    vSide = side.Normalize();
}

V3      CAIVehicle::Side() const {
    return vSide;
}

inline void swap(float &a, float &b) {
    float c;
    c = a;
    a = b;
    b = c;
}

inline bool inside(float a, float ad, float b, float bd, float & c, float & cd) {
    bool in1, in2;

    if (a - ad > b - bd) {
        swap(a , b );
        swap(ad, bd);
    }

    in1 = b - bd <= a + ad;
    in2 = b + bd <= a + ad;

    if (in1 && in2) {
        c  = b ;
        cd = bd;
        return true;
    }

    if (!in1) return false;
    
    cd = ((a + ad) - (b - bd)) / 2;
    c  = b - bd + cd;

    return true;
}

V3      CAIVehicle::AvoidCollision(float fMaxCollisionTime, const ObjectVec & objects) {
    CAIObject * threat = NULL, * object;
    V3 p1, p2, offset, v1, v2, rel_v;
    size_t size, i;
    float fMinCollisionTime = FLT_MAX, t, td, r, r1, r2, tx, tz, txd, tzd, steer = 0;

    size = objects.size();
    r1 = Radius();
    v1 = Velocity().SetYtoZero();
    p1 = Position().SetYtoZero();
    if (!v1.x && !v1.z) return V3::Zero();

    for (i = 0; i < size; i++) {
        object = objects[i];
        if (!object || object == this || object->Mass() * MassAvoidanceCoef < Mass()) continue;
        r2 = object->Radius();
        v2 = object->Velocity().SetYtoZero();
        p2 = object->Position().SetYtoZero();
        r = r1 + r2;
        offset = p1 - p2;
        rel_v  = v1 - v2;
        // let's assume that object is stationary and is placed exactly at [0,0,0]
        tx = rel_v.x ? - offset.x / rel_v.x : 0;
        tz = rel_v.z ? - offset.z / rel_v.z : 0;
        // are they both non-zero?
        if (tx * tz) {
            txd = abs(r / rel_v.x);
            tzd = abs(r / rel_v.z);
            // do we go close enough?
            if (!inside(tx, txd, tz, tzd, t, td)) continue;
        // is at least one of them 0?
        } else // are both 0?
            if (tx == tz) continue;
            // only one is 0
            else {
                t = tx + tz;
                td = abs(r / (rel_v.x + rel_v.z));
            }
        // is the collision behind us?
        if (t < 0 || t - td > fMaxCollisionTime) continue;

        if (t < fMinCollisionTime && object->Speed() <= Speed()) {
            threat = object;
            fMinCollisionTime = t;
        }
    }

    if (threat)  {
        if (fLog) {
            fprintf_s(fLog, "Steering away from object at position [%f, %f, %f]\n", threat->Position().x,
                                                                                    threat->Position().y, 
                                                                                    threat->Position().z); 
        }

        // parallel: +1, perpendicular: 0, anti-parallel: -1
        float parallelness = Forward().DotProduct(threat->Forward());
        float angle = 0.707f;

        if (parallelness < -angle)   {
            // anti-parallel "head on" paths:
            // steer away from future threat position
            offset = (threat->Position() + threat->Velocity() * t) - Position();
            steer = (offset.DotProduct(Side()) > 0) ? -1.0f : 1.0f;
        }  else if (parallelness > angle)  
                // parallel paths: steer away from threat
                    steer = ((threat->Position() - Position()).DotProduct(Side()) > 0) ? -1.0f : 1.0f;
                // perpendicular paths: steer behind threat
           else     steer = (Side().DotProduct(threat->Velocity()) > 0) ? -1.0f : 1.0f;
    }
    return Side().SetYtoZero() * CarMaxSpeed * steer;
}