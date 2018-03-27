#include "CSteeringAI.h"
#include "..\Globals\Configuration.h"

using namespace AI_NS;


float UpdateTimeClip;
float CarMaxSpeed;
float CarSpeedTreshold;
float CarSpeedChangeProbability;
float CarSpeedChange;
float MassAvoidanceCoef;
float LeadTime;
int   CarTimeBeforeRestart;
float CarTrackProgress;
float LeakThrough;
float DistanceTolerance;
int   MaxWaypointsMissed;
float MapToPathLocalDistance;
float DefaultRadius;     
float MaxWaypointDistance;
bool  OptimizeWaypointPlacement;


CSteeringAI::CSteeringAI() {
    pCars = NULL;
}



CSteeringAI::~CSteeringAI() {
    pRM->ReleaseResource(ridMap);
    delete[] pCars;
    // pRM->ReleaseResource(ridVehicle);
}

int CSteeringAI::InitAI(InitStruct is, float fRadius) {
    vAICarIndexes = is.vCI;
    size_t i;
    CAtlString sLogName;
    
    pRM       = is.pRM;
    pPhysics  = is.pPhysics;
    iLapCount = is.iLapCount;
    
    OUTMSG("\tConfiguration", 1);
    // load parameters from configuration
    UpdateTimeClip                = Configuration.UpdateTimeClip;
    CarMaxSpeed                   = Configuration.CarMaxSpeed * ONE_METER;
    CarSpeedTreshold              = Configuration.CarSpeedTreshold * CarMaxSpeed;
    CarSpeedChangeProbability     = Configuration.CarSpeedChangeProbability;
    CarSpeedChange                = Configuration.CarSpeedChange;
    MassAvoidanceCoef             = Configuration.MassAvoidanceCoef;
    LeadTime                      = Configuration.LeadTime;
    CarTimeBeforeRestart          = Configuration.CarTimeBeforeRestart;
    CarTrackProgress              = Configuration.CarTrackProgress * ONE_METER;
    LeakThrough                   = Configuration.LeakThrough;
    DistanceTolerance             = Configuration.DistanceTolerance * ONE_METER;
    MaxWaypointsMissed            = Configuration.MaxWaypointsMissed;
    MapToPathLocalDistance        = Configuration.MapToPathLocalDistance * ONE_METER;
    DefaultRadius                 = Configuration.DefaultRadius * ONE_METER;
    OptimizeWaypointPlacement     = Configuration.OptimizeWaypointPlacement;
    MaxWaypointDistance           = Configuration.MaxWaypointDistance * ONE_METER;

    OUTMSG("\tCreating cars", 1);
    pCars = new CAIVehicle[MAX_PLAYERS]; 

    for (i= 0; i < vAICarIndexes.size(); i++) {
        sLogName.Format("%i_carlog.txt", (int) i);
        pCars[vAICarIndexes[i]].ClearRestartStructures();
//        pCars[vAICarIndexes[i]].StartLogging(sLogName);
    }
    
    if (!pRM || !pPhysics) {
        ErrorHandler.HandleError(ERRGENERIC, "CSteeringAI::InitAI()", "Invalid Physics or RM pointers!");
        return 0;
    }

    OUTMSG("\tLoading resource", 1);
	if ((ridMap = pRM->LoadResource(is.sMapName, RES_Map, true, NULL)) < 0) {
        ErrorHandler.HandleError(ERRNOTFOUND, "CSteeringAI::InitAI()", "Could not load map");
        return 0;
    }

    OUTMSG("\tGetting Map", 1);
    if (!(pMap = pRM->GetMap(ridMap))) {
        ErrorHandler.HandleError(ERRNOTFOUND, "CSteeringAI::InitAI()", "Could not obtain map");
        return 0;
    }
    
    OUTMSG("\tFinding waypoint path", 1);
    if (FindBestWaypointPath(vBestWaypointPath) != ERRNOERROR) {
        ErrorHandler.HandleError(ERRGENERIC, "CSterringAI::InitAI()",  "Could not find waypoint path");
        return 0;
    } 

    vOptimizedWaypointPath = vBestWaypointPath;

    OUTMSG("\tOptimizing placement", 1);
    if (OptimizeWaypointPlacement)
        OptimizePlacement(vOptimizedWaypointPath);
    
    OUTMSG("\tNonOptimizedPath.Initialize", 1);
    if (!AINonOptimizedPath.Initialize(vBestWaypointPath)) {
        ErrorHandler.HandleError(ERRGENERIC, "CSterringAI::InitAI()",  "Could not initialize AINonOptimizedPath");
        return 0;
    }
    
    OUTMSG("\tAIPath.Initialize", 1);
    if (!AIPath.Initialize(vOptimizedWaypointPath)) {
        ErrorHandler.HandleError(ERRGENERIC, "CSterringAI::InitAI()",  "Could not initialize AIPath");
        return 0;
    }
    
    OUTMSG("\tInitObjects", 1);
    InitObjects();

    return 1;
}

void CSteeringAI::Update(float fTime) {
    int i, iLaps, iLVW;
    dVector heading;
    V3 vNormal;
    float x, y, z, alfa;
    // clip update time
    fTime = fTime > UpdateTimeClip ? UpdateTimeClip : fTime;
    // load dynamic objects' positions & stuff
    LoadObjects();
    LoadCars();
    for (i = 0; i < (int) vAICarIndexes.size(); i++) {
//        LoadCar(vAICarIndexes[i]);
        UpdateCar(fTime, vAICarIndexes[i]);
        SaveCar(vAICarIndexes[i]);
        pCars[vAICarIndexes[i]].RaceInfo(iLVW, iLaps);
        if (pCars[vAICarIndexes[i]].NeedRestart() && iLapCount > iLaps) {
            x = AIPath.wvWaypoints[iLVW+1].point.x / ONE_METER;
			y = AIPath.wvWaypoints[iLVW+1].point.y / ONE_METER;
            z = AIPath.wvWaypoints[iLVW+1].point.z / ONE_METER;
			vNormal = AIPath.GetNormal(iLVW+2).SetYtoZero().Normalize();

			if(LIES_BETWEEN(vNormal.x, -0.001f, 0.001f) && LIES_BETWEEN(vNormal.y, -0.001f, 0.001f) && LIES_BETWEEN(vNormal.z, -0.001f, 0.001f))
				vNormal = V3::Zero();

            if (vNormal == V3::Zero()) {
                alfa = fRandom01() * PI * 2;
                vNormal = V3(sinf(alfa), 0.0f, cosf(alfa));
            }
            heading = dVector(vNormal.x, vNormal.y, vNormal.z);
            pPhysics->cars[vAICarIndexes[i]]->RestartCar(x, y, z, heading);
            pCars[vAICarIndexes[i]].ClearRestartStructures();
        }
    }
}

int iState = 0;

void CSteeringAI::UpdateCar(float fTime, int iCarIndex) {
    if (iCarIndex >= MAX_PLAYERS) return;

    /*float fSliding = (pCars[iCarIndex].Velocity().Length() > ONE_METER) ? (pCars[iCarIndex].Velocity().Normalize() - pCars[iCarIndex].Forward().Normalize()).Length() : 0;
    if (fSliding < SLIDING_ACCEPTANCE 
        && !pPhysics->cars[iCarIndex]->Airborne()) */
    {
        pCars[iCarIndex].Update(fTime, vObjects, &AIPath);
    } 
}

V3  dVtoV3(const dVector & vec) {
    return V3(vec.m_x, vec.m_y, vec.m_z);
}

V3  TransformVector(const V3 & vSource, const dMatrix & mat) {
    dVector vec = dVector(vSource.x, vSource.y, vSource.z);
    vec = mat.TransformVector(vec);
    return dVtoV3(vec);
}

//void CSteeringAI::LoadCar(int iCarIndex) {
//    dVector pos, vel, fwd;
//    dMatrix mat;
//    V3 vSide;
//    
//    if (iCarIndex >= MAX_PLAYERS || !pPhysics) return;
//
//    pos = pPhysics->cars[iCarIndex]->Position();
//    vel = pPhysics->cars[iCarIndex]->Velocity();
//    fwd = pPhysics->cars[iCarIndex]->Heading();
//    mat = pPhysics->cars[iCarIndex]->Matrix();
//    mat.m_posit = dVector(0.0, 0.0, 0.0);
//    
//    vSide = TransformVector(V3(1.0, 0.0, 0.0), mat);
//
//    pCars[iCarIndex].SetPosition(V3(pos.m_x, pos.m_y, pos.m_z) * ONE_METER);
//    pCars[iCarIndex].SetVelocity(V3(vel.m_x, vel.m_y, vel.m_z) * ONE_METER);
//    pCars[iCarIndex].SetForward (V3(fwd.m_x, fwd.m_y, fwd.m_z));
//    pCars[iCarIndex].SetSide    (vSide);
//    pCars[iCarIndex].SetMass    (pPhysics->cars[iCarIndex]->Mass());
//    pCars[iCarIndex].SetRadius  (pPhysics->cars[iCarIndex]->BBoxDiameter() * ONE_METER);
//}

void CSteeringAI::LoadCars() {
    int i;
    dVector pos, vel, fwd;
    dMatrix mat;
    V3 vSide;

    if (!pPhysics) return;

    for (i = 0; i < MAX_PLAYERS; i++) {
        if (!pPhysics->cars[i]) continue;

        pos = pPhysics->cars[i]->Position();
        vel = pPhysics->cars[i]->Velocity();
        fwd = pPhysics->cars[i]->Heading();
        mat = pPhysics->cars[i]->Matrix();
        mat.m_posit = dVector(0.0, 0.0, 0.0);
        
        vSide = TransformVector(V3(1.0, 0.0, 0.0), mat);

        pCars[i].SetPosition(V3(pos.m_x, pos.m_y, pos.m_z) * ONE_METER);
        pCars[i].SetVelocity(V3(vel.m_x, vel.m_y, vel.m_z) * ONE_METER);
        pCars[i].SetForward (V3(fwd.m_x, fwd.m_y, fwd.m_z));
        pCars[i].SetSide    (vSide);
        pCars[i].SetMass    (pPhysics->cars[i]->Mass());
        pCars[i].SetRadius  (pPhysics->cars[i]->BBoxDiameter() * ONE_METER);
    }
}

void CSteeringAI::SaveCar(int iCarIndex) const {
    V3 vOV, vDV;
    dMatrix inv_mat;
    int _, iLaps;
    float fTangent, fF, fS, fMinSpeed, fMaxSpeed;


    if (iCarIndex >= MAX_PLAYERS || !pPhysics) {
        ErrorHandler.HandleError(ERRGENERIC, "CSteeringAI::SaveCar()", "Unable to save car - wrong index or pPhysics");
        return;
    }

    pCars[iCarIndex].RaceInfo(_, iLaps);

    if (iLaps >= iLapCount) {
        pPhysics->cars[iCarIndex]->SetHandBrake(1.0);
        return;
    }

    inv_mat = pPhysics->cars[iCarIndex]->Matrix().Inverse();
    inv_mat.m_posit = dVector(0, 0, 0);

    pCars[iCarIndex].GetDesiredVelocity(vDV, fMinSpeed, fMaxSpeed);

    vDV = TransformVector(vDV                        , inv_mat).SetYtoZero();
    vOV = TransformVector(pCars[iCarIndex].Velocity(), inv_mat).SetYtoZero();

    fF = (vOV.z > fMaxSpeed) ? -1.0f : 1.0f;
    
    fTangent = atan2(vDV.x, vDV.z) / PI * 180; 

    if (vDV.Length() > CarSpeedTreshold) fS =  fTangent;
    else fS = 0;

    if (fS >  45) { 
        fS =  45;
        if (vOV.z > 35 * ONE_METER)
            fF = -1.0;
    }
    if (fS < -45) {
        fS = -45;
        if (vOV.z > 35 * ONE_METER)
            fF = -1.0;
    }

    if (pCars[iCarIndex].fLog) {
        fprintf_s(pCars[iCarIndex].fLog, "XXXXXXXXXXX S A V I N G   C A R XXXXXXXXXXXXXXX\n");
        fprintf_s(pCars[iCarIndex].fLog, "P  =[%f, %f, %f]\n", pCars[iCarIndex].Position().x,
                                                               pCars[iCarIndex].Position().y,
                                                               pCars[iCarIndex].Position().z);
        fprintf_s(pCars[iCarIndex].fLog, "OV =[%f, %f, %f]\n", vOV.x, vOV.y, vOV.z);
        fprintf_s(pCars[iCarIndex].fLog, "DV =[%f, %f, %f]\n", vDV.x, vDV.y, vDV.z);
        fprintf_s(pCars[iCarIndex].fLog, "Steering = %f, Forward = %f\n", fS, fF);
        fprintf_s(pCars[iCarIndex].fLog, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    }

    
    pPhysics->cars[iCarIndex]->SetSteer(fS);
    pPhysics->cars[iCarIndex]->SetHandBrake(0.0);    

    if (fF > 0) {
        //pPhysics->cars[iCarIndex]->SetThrottle(fF);
        //pPhysics->cars[iCarIndex]->SetBrakes(0.0);
		pPhysics->cars[iCarIndex]->SetThrottleBrakes(fF, 0.0f);

    } else {
        //pPhysics->cars[iCarIndex]->SetThrottle(0);
        //pPhysics->cars[iCarIndex]->SetBrakes(fF);
		pPhysics->cars[iCarIndex]->SetThrottleBrakes(0.0f, -fF);
    }
}

void CSteeringAI::InitObjects() {
    CAIObject * poNew;
    size_t i, size;
    dVector pos, vel;

    if (!pPhysics) {
        ErrorHandler.HandleError(ERRGENERIC, "CSteeringAI::InitObjects()", "pPhysics not initialized!");
        return;
    }

    size = pPhysics->dynamicObjects.size();
    for (i = 0; i < size; i++) {
        poNew = NULL;
        if (pPhysics->dynamicObjects[i]->NBody()) { // if it is not a wheel, init it
            poNew = new CAIObject();

            pos = pPhysics->dynamicObjects[i]->Position();
            vel = pPhysics->dynamicObjects[i]->Velocity();

            poNew->SetMass    (pPhysics->dynamicObjects[i]->Mass());
            poNew->SetRadius  (pPhysics->dynamicObjects[i]->BBoxDiameter() * ONE_METER);
            poNew->SetPosition(dVtoV3(pos) * ONE_METER);
            poNew->SetVelocity(dVtoV3(vel) * ONE_METER);
        }
        vObjects.push_back(poNew);
    }

    for (i = 0; i < MAX_PLAYERS; i++)
        vObjects.push_back(pCars + i);
}

void CSteeringAI::LoadObjects() {
    size_t i, size;
    dVector pos, vel;

    if (!pPhysics) {
        ErrorHandler.HandleError(ERRGENERIC, "CSteeringAI::LoadObjects()", "pPhysics not initialized!");
        return;
    }
    
    size = pPhysics->dynamicObjects.size();
    if (size != (vObjects.size() - MAX_PLAYERS)) {
        ErrorHandler.HandleError(ERRGENERIC, "CSteeringAI::LoadObjects()", "# of objects differ!");
        return;
    }
    for (i = 0; i < size; i++) 
        if (vObjects[i]) {
            pos = pPhysics->dynamicObjects[i]->Position();
            vel = pPhysics->dynamicObjects[i]->Velocity();

            vObjects[i]->SetMass    (pPhysics->dynamicObjects[i]->Mass());
            vObjects[i]->SetRadius  (pPhysics->dynamicObjects[i]->BBoxDiameter() * ONE_METER);
            vObjects[i]->SetPosition(dVtoV3(pos) * ONE_METER);
            vObjects[i]->SetVelocity(dVtoV3(vel) * ONE_METER);
        }
}

HRESULT CSteeringAI::FindBestWaypointPath(WVec &vResult, bool bOptimize) const {
    PathPointStruct * ppsNew, * ppsTmp;
    resManNS::MainObject * pmoTmp;
    resManNS::Waypoints  * pwpTmp;
    WaypointStruct wps;
    CAtlString sFmt;
    std::vector<PathPointStruct *> vQueue;
    size_t qi, qj, qs, qsj;
    float fx, fy, fz, fDist, fMinDist;
    V3 vTmp;

    if (vBestWaypointPath.size()) {
        vResult = vBestWaypointPath;
        return ERRNOERROR;
    }

    if (!pMap->uiPlacedObjectsCount) HE(ERRNOTFOUND, "CSteeringAI::FindBestWaypointPath()", "No placed objects!");

    pmoTmp = pRM->GetMainObject(pMap->pridObjects[pMap->puiPlacedObjectIndexes[pMap->puiPathPointIndexes[0]]]);
    if (!pmoTmp) {
        sFmt.Format("Cannot get MainObject with ID=%u", pMap->pridObjects[pMap->puiPlacedObjectIndexes[pMap->puiPathPointIndexes[0]]]);
        HE(ERRNOTFOUND, "CSteeringAI::FindBestWaypointPath()", sFmt);
    }
    qs = pmoTmp->pvWaypointsIDs->size();
    for (qi = 0;  qi < qs; qi++) { // inital placement of starting waypoints into queue
        pwpTmp = pRM->GetWaypoints(pmoTmp->pvWaypointsIDs->at(qi));
        ppsNew = NULL; ppsNew = new PathPointStruct;
        if (!pwpTmp || !ppsNew) {
            for (qi--; qi >= 0; qi--)
                delete vQueue[qi];
            if (ppsNew) delete ppsNew;
            sFmt.Format("Cannot get Waypoints with ID=%u", pmoTmp->pvWaypointsIDs->at(qi));
            HE(ERRNOTFOUND, "CSteeringAI::FindBestWaypointPath()", sFmt);
        }
        ppsNew->fDistance = 0;
        ppsNew->iPathIndex = 0;
        fx = pwpTmp->x->at(pwpTmp->x->size() - 1); 
        fy = pwpTmp->y->at(pwpTmp->y->size() - 1); 
        fz = pwpTmp->z->at(pwpTmp->z->size() - 1); 
        AdjustXYZ(fx, fy, fz, 0);
        ppsNew->vEnd = V3(fx, fy, fz);
        ppsNew->vWaypointIndexes.push_back((int) qi);
        vQueue.push_back(ppsNew);  
    }
    qs = vQueue.size(); ppsTmp = NULL;
    while (qs) {
        ppsTmp = vQueue[0]; vQueue.erase(vQueue.begin()); qs--;// take the one with smallest difference and greatest progress out
        ppsTmp->iPathIndex++; // increase path index
        if (ppsTmp->iPathIndex == pMap->uiPathPointsCount) break; // are we at the end?
        pmoTmp = pRM->GetMainObject(pMap->pridObjects[pMap->puiPlacedObjectIndexes[pMap->puiPathPointIndexes[ppsTmp->iPathIndex]]]);
        if (!pmoTmp) { // if we did not get MainObject
            while (qs) {
                delete vQueue[0];
                vQueue.erase(vQueue.begin());
                qs--;
            }
            sFmt.Format("Cannot get MainObject with ID=%u", pMap->pridObjects[pMap->puiPlacedObjectIndexes[pMap->puiPathPointIndexes[ppsTmp->iPathIndex]]]);
            HE(ERRNOTFOUND, "CSteeringAI::FindBestWaypointPath()", sFmt);
        }
        qsj = pmoTmp->pvWaypointsIDs->size();
        
        fMinDist = FLT_MAX;
        for (qi = 0; qi < qsj; qi++) { // for all waypoints that are there do :
            pwpTmp = pRM->GetWaypoints(pmoTmp->pvWaypointsIDs->at(qi));
            if (!pwpTmp) {
                while (qs) {
                    delete vQueue[0];
                    vQueue.erase(vQueue.begin());
                    qs--;
                }
                if (ppsNew) delete ppsNew;
                sFmt.Format("OOM / Cannot get Waypoints with ID=%u", pmoTmp->pvWaypointsIDs->at(qi));
                HE(ERRNOTFOUND, "CSteeringAI::FindBestWaypointPath()", sFmt); 
            }
            fx = pwpTmp->x->at(0); 
            fy = pwpTmp->y->at(0);
            fz = pwpTmp->z->at(0); 
            AdjustXYZ(fx, fy, fz, ppsTmp->iPathIndex);

            fDist = ppsTmp->vEnd.Distance(V3(fx, fy, fz));

            if (fDist < fMinDist)
                fMinDist = fDist;
        }


        for (qi = 0; qi < qsj; qi++) { // for all waypoints that are there do :
            pwpTmp = pRM->GetWaypoints(pmoTmp->pvWaypointsIDs->at(qi));
            ppsNew = NULL; ppsNew = new PathPointStruct;

            if (!ppsNew) {
                while (qs) {
                    delete vQueue[0];
                    vQueue.erase(vQueue.begin());
                    qs--;
                }
                if (ppsNew) delete ppsNew;
                sFmt.Format("OOM / Cannot get Waypoints with ID=%u", pmoTmp->pvWaypointsIDs->at(qi));
                HE(ERRNOTFOUND, "CSteeringAI::FindBestWaypointPath()", sFmt); 
            }
            // compute absolute xyz position of end & beginning of the current waypoint and compute distance from previous pathpoint to beggining of this one

            fx = pwpTmp->x->at(0); 
            fy = pwpTmp->y->at(0);
            fz = pwpTmp->z->at(0); 
            AdjustXYZ(fx, fy, fz, ppsTmp->iPathIndex);
            
            fDist = ppsTmp->vEnd.Distance(V3(fx, fy, fz));

            if (fMinDist < 5 * ONE_METER) {
                if (fDist > 5 * ONE_METER) {
                    delete ppsNew;
                    continue;
                }
            } else if (bOptimize) {
                PathPointStruct * pps;
                vector<PathPointStruct * >::iterator q_tmp = vQueue.begin();
               
                while (q_tmp != vQueue.end()) {
                    pps = *q_tmp;
                    if (pps->iPathIndex < ppsTmp->iPathIndex - 2 && pps->fDistance > ppsTmp->fDistance) {
                        q_tmp = vQueue.erase(q_tmp);
                        qs--;
                    } else q_tmp++;
                }
            }

            ppsNew->fDistance = ppsTmp->fDistance + fDist;
            ppsNew->iPathIndex = ppsTmp->iPathIndex;
            ppsNew->vWaypointIndexes = ppsTmp->vWaypointIndexes;
            ppsNew->vWaypointIndexes.push_back((int) qi);


            fx = pwpTmp->x->at(pwpTmp->x->size() - 1); 
            fy = pwpTmp->y->at(pwpTmp->y->size() - 1); 
            fz = pwpTmp->z->at(pwpTmp->z->size() - 1); 
            AdjustXYZ(fx, fy, fz, ppsTmp->iPathIndex);
            ppsNew->vEnd = V3(fx, fy, fz);
            for ( // find where insert new PathPointStruct to
                qj = 0
                ;
                qj < qs && // aren't we at the end?
                (
                    (vQueue[qj]->fDistance <  ppsNew->fDistance) || // there are better results ...  
                    (vQueue[qj]->fDistance == ppsNew->fDistance && vQueue[qj]->iPathIndex > ppsNew->iPathIndex) // or the same but with better progress
                )
                ;
                qj++ // move to the next one
            );
            vQueue.insert(vQueue.begin() + qj, ppsNew); // finally, insert it there...
            qs++;
        }
        delete ppsTmp; ppsTmp = NULL;
    }
    if (!ppsTmp || ppsTmp->iPathIndex != pMap->uiPathPointsCount) 
        HE(ERRNOTFOUND, "CSteeringAI::FindBestWaypointPath()", "Path not found");

    for (qi = 0; qi < pMap->uiPathPointsCount; qi++) {
        //load correct MainObject
        pmoTmp = pRM->GetMainObject(pMap->pridObjects[pMap->puiPlacedObjectIndexes[pMap->puiPathPointIndexes[qi]]]);  
        sFmt.Format("OOM / Cannot get Waypoints with ID=%u", pmoTmp->pvWaypointsIDs->at(ppsTmp->vWaypointIndexes[qi]));
        if (!pmoTmp) HE(ERRNOTFOUND, "CSteeringAI::FindBestWaypointPath()", sFmt);
        // chose right waypoint sequence
        pwpTmp = pRM->GetWaypoints(pmoTmp->pvWaypointsIDs->at(ppsTmp->vWaypointIndexes[qi]));
        
        qs = pwpTmp->x->size(); 

        for (qj = 0; qj < qs; qj++) {
            wps.point = V3( pwpTmp->x->at(qj), pwpTmp->y->at(qj), pwpTmp->z->at(qj));
            wps.fMaxSpeed = pwpTmp->max_speed->at(qj) * ONE_METER;
            wps.fMinSpeed = pwpTmp->min_speed->at(qj) * ONE_METER;
            if (wps.fMinSpeed <= 0) wps.fMinSpeed = 0;
            if (wps.fMaxSpeed <= 0) wps.fMaxSpeed = CarMaxSpeed;
            AdjustXYZ(wps.point.x, wps.point.y, wps.point.z, (UINT) qi);
            vResult.push_back(wps);
        }
    }
  
    //FILE * fr = fopen("Waypoints.txt", "w");
    //for (qi = 0; qi < vResult.size(); qi++) 
    //    fprintf_s(fr, "%.0f\t%.0f\t%0.f\n", vResult[qi].point.x, vResult[qi].point.y, vResult[qi].point.z);
    //fclose(fr);

    return ERRNOERROR;
}

void CSteeringAI::AdjustXYZ(float &x, float &y, float &z, UINT uiPathIndex) const {
    int iOrientation, i, px, pz;
    UINT size, shift;
    float nx, nz;
    resManNS::MainObject * pMO;
    resManNS::GameObject * pGO;
    
    if (!pMap || !pRM) return;
    pMO = pRM->GetMainObject(pMap->pridObjects[pMap->puiPlacedObjectIndexes[pMap->puiPathPointIndexes[uiPathIndex]]]);
    if (!pMO) return;
    
    iOrientation = pMap->puiPlacedObjectOrientations[pMap->puiPathPointIndexes[uiPathIndex]];
    px           = pMap-> piPlacedObjectX           [pMap->puiPathPointIndexes[uiPathIndex]];
    pz           = pMap-> piPlacedObjectZ           [pMap->puiPathPointIndexes[uiPathIndex]];
    size = 1; 

    for (i = 0; i < (int) pMO->pvObjectIDs->size(); i++) {
		pGO = pRM->GetGameObject(pMO->pvObjectIDs->at(i));
        if (!pGO) return;
		if ((pGO->sizeX + pGO->posX) > size)  size = (pGO->sizeX + pGO->posX) ;
		if ((pGO->sizeY + pGO->posY) > size)  size = (pGO->sizeY + pGO->posY) ;
	}

	for (i = 0; i < (int) pMO->pvRoadIDs->size(); i++) {
		pGO = pRM->GetGameObject(pMO->pvRoadIDs->at(i));
        if (!pGO) return;
		if ((pGO->sizeX + pGO->posX) > size)  size = (pGO->sizeX + pGO->posX);
		if ((pGO->sizeY + pGO->posY) > size)  size = (pGO->sizeY + pGO->posY);
	}

	shift = (size-1)/2;

	switch( iOrientation % 4)
	{
		case 0:		
			nx = (float) TERRAIN_PLATE_WIDTH * px + x;
			nz = (float) TERRAIN_PLATE_WIDTH * pz + z;
			break;
		
		case 1:
			nx = (float) TERRAIN_PLATE_WIDTH * px +  z;
			nz = (float) TERRAIN_PLATE_WIDTH * pz +  TERRAIN_PLATE_WIDTH*(size) - x;
			break;
		
		case 2:
			nx = (float) TERRAIN_PLATE_WIDTH * px +  TERRAIN_PLATE_WIDTH*(size) - x;
			nz = (float) TERRAIN_PLATE_WIDTH * pz +  TERRAIN_PLATE_WIDTH*(size) - z;
			break;

		case 3:
			nx = (float) TERRAIN_PLATE_WIDTH * px + TERRAIN_PLATE_WIDTH*(size) - z;
			nz = (float) TERRAIN_PLATE_WIDTH * pz + x;
			break;
    }
    
    x = (nx - shift * TERRAIN_PLATE_WIDTH);
    z = (nz - shift * TERRAIN_PLATE_WIDTH);
    y += pPhysics->GetHeight(x / ONE_METER, z / ONE_METER) * ONE_METER;
}

void CSteeringAI::GetRaceInfo(int iCarIndex, int & iLastValidWaypoint, int & iRoundsFinished) const {
    int i, iLVW, iNO;
    float min = FLT_MAX;
    V3 vDummy;

    pCars[iCarIndex].RaceInfo(iLVW, iRoundsFinished);
    
    if (!OptimizeWaypointPlacement) {
        iLastValidWaypoint = iLVW;
        return;
    }

    for (i = 0, iNO = 0; i <= iLVW; i++)
        if (AIPath.wvWaypoints[i].point == AINonOptimizedPath.wvWaypoints[iNO].point) iNO++;
    //for (i = 1; i < AINonOptimizedPath.wvWaypoints.size(); i++) {
    //    d = AINonOptimizedPath.PointToSegmentDistance(AIPath.wvWaypoints[iLVW].point, i, vDummy, fDummy);
    //    if (d < min) {
    //        iseg = i;
    //        min = d;
    //    }
    //}
    iLastValidWaypoint = --iNO;
}

void CSteeringAI::OptimizePlacement(WVec &vResult) {
    size_t qi, qj, qs;
    WaypointStruct wps;
    for (qi = 1; qi < vResult.size(); qi++) {
        float fDiff = vResult[qi].point.Distance(vResult[qi-1].point) / MAX_WAYPOINT_DISTANCE;
        if (fDiff < 1) continue;
        qs = (size_t) ceilf(fDiff);
        for (qj = qs - 1; qj > 0; qj--) {
            fDiff = ((float) qj ) / ((float) qs);
            wps.fMaxSpeed = vResult[qi-1].fMaxSpeed + (vResult[qi].fMaxSpeed - vResult[qi-1].fMaxSpeed) * fDiff;
            wps.fMinSpeed = vResult[qi-1].fMinSpeed + (vResult[qi].fMinSpeed - vResult[qi-1].fMinSpeed) * fDiff;
            wps.fMaxSpeed *= ONE_METER;
            wps.fMinSpeed *= ONE_METER;
            wps.point     = vResult[qi-1].point     + (vResult[qi].point     - vResult[qi-1].point    ) * fDiff;
            vResult.insert(vResult.begin() + qi, wps);
        }
    }
}
