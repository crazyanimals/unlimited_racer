#pragma once


#include "V3.h"
#include "CAIObject.h"
#include "CAIPath.h"
#include "AI_common.h"




namespace AI_NS {
    class CAIVehicle : public CAIObject {
    private:
        IVec        ivWaypointsPassed;
        int         iLastValidWaypoint, iRoundCount;
        V3          vForward, vDesiredVelocity, vSide;
        float       fTimeAccumulator;
        V3        * pvLastPosition;
        float     * pfLastProgress;
        bool        bRestartVehicle;
        float       fMinSpeed, fMaxSpeed, fSpeedAdjust;
    public:        
        FILE      * fLog;

                    CAIVehicle();
                    CAIVehicle(const CAIVehicle & v);
                   ~CAIVehicle();
      
        void        SetForward              (const V3 & forward);
        void        SetSide                 (const V3 & side);
        void        UpdatePositionOnMap     (const CAIPath * path);
        void        Update                  (const float fPredictionTime, const ObjectVec & objects, const CAIPath * path);
        V3          Forward                 () const;
        V3          Side                    () const;
        V3          CombinedSteering        (const float fPredictionTime, const ObjectVec & objects, const CAIPath * path);
        V3          SteerToFollowPath       (const float fPredictionTime, const CAIPath * path);
        V3          SteerToAvoidObjects     (const float fMinTimeToCollision   , const ObjectVec & objects) ;
        V3          SteerToAvoidCloseObjects(const float fMinSeparationDistance, const ObjectVec & others)  ;
        V3          AvoidCollision          (float fMaxCollisionTime, const ObjectVec & objects);
        //V3          SteerForSeek            (const V3 & vTarget) const;
        //V3          AdjustRawSteeringForce  (const V3 & vForce) const;
        void        GetDesiredVelocity      (V3 & vel, float & min_speed, float & max_speed) const;
        //void        ApplySteeringForce      (const V3 & vForce, const float fPredictionTime);
        void        RaceInfo(int & _LastValidWaypoint, int & _RoundsFinished) const;
        bool        NeedRestart() const;
        void        ClearRestartStructures();
        void        StartLogging(CAtlString sLogName);
    };
    
};
