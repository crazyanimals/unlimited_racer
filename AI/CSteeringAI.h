#pragma once

#include "..\ResourceManager\ResourceManager.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\Globals\Globals.h"
#include "AI_Common.h"
#include "CAIVehicle.h"
#include "CAIPath.h"
#include "..\Physics\Physics.h"


namespace AI_NS {

    typedef struct __InitStruct {
        resManNS::CResourceManager * pRM;
        CAtlString sMapName;
        IVec vCI; //car indexes
        physics::CPhysics * pPhysics;
        int iLapCount;
    } InitStruct;

    typedef struct __PathPointStruct {
        float fDistance;
        int iPathIndex;
        V3  vEnd;
        IVec vWaypointIndexes;
    } PathPointStruct;

    class CSteeringAI {
    private:
        IVec vAICarIndexes;
        resManNS::RESOURCEID         ridMap;
        resManNS::Map              * pMap;
        resManNS::CResourceManager * pRM;
        physics::CPhysics          * pPhysics;
        int iObjectCount, iLapCount;
        CAIVehicle  * pCars;
        ObjectVec   vObjects;
        CAIPath AIPath, AINonOptimizedPath;
        WVec vBestWaypointPath, vOptimizedWaypointPath;

    public:
                CSteeringAI();
               ~CSteeringAI();

		void        AdjustXYZ(float &x, float &y, float &z, UINT uiPathIndex) const;
        HRESULT     FindBestWaypointPath(WVec &vResult, bool bOptimize = true) const;
        void        OptimizePlacement(WVec &vResult);
        int         InitAI(InitStruct is, float fRadius = DefaultRadius);
        void        Update(float fTime);  
        void        UpdateCar(float fTime, int iCarIndex);
//        void        LoadCar(int iCarIndex);
        void        SaveCar(int iCarIndex) const;
        void        GetRaceInfo(int iCarIndex, int & iLastValidWaypoint, int & iRoundsFinished) const;
        void        LoadObjects();
        void        LoadCars();
        void        InitObjects();
        CAIPath   * GetAIPath();
    };
};
