#include "Physics.h"

using namespace physics;

// class constructor
//CPhysics::CPhysics()
//{
//};
//
//// class destructor
//CPhysics::~CPhysics()
//{
//}
// TODO: collision objects - nacitani podle typu + scaling k danemu objektu

graphic::CBoundingEnvelopes * CPhysics::bEnvelope;
HRESULT (CALLBACK * CPhysics::Start)(graphic::CBoundingEnvelopes * bEnvelope);
HRESULT (CALLBACK * CPhysics::Finish)(graphic::CBoundingEnvelopes * bEnvelope);
HRESULT (CALLBACK * CPhysics::ProcessFace)(graphic::CBoundingEnvelopes * bEnvelope, UINT uiVertCount, float * pBuf);

//SpecialEffectStruct * CPhysics::g_currentEffect;

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// call after class creation - class initialization
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CPhysics::Init(resManNS::CResourceManager * rsm)
{
	userData.data = this;

	for(int i=0; i<MAX_PLAYERS; i++)
		cars[i] = NULL;
	staticObjects.clear();
	dynamicObjects.clear();

	HRESULT hr = ERRNOERROR;

	nWorld = NewtonCreate(NULL, NULL);

	NewtonWorldSetUserData(nWorld, &userData);

	resources.Init(rsm, nWorld);

	NewtonSetSolverModel(nWorld, Configuration.SolverModel); //default 1

	NewtonSetFrictionModel(nWorld, Configuration.FrictionModel); // default 1

	terrain.Init(nWorld, &resources);

	hr = materials.Init(nWorld, &resources, this);
	if(hr) ERRORMSG(hr, "CPhysics::Init()", "Materials initialization failed");
	// TODO: if error tak zrusit ReleaseResources();

	accumulator = 0.0f;

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// debuggraphics initialization
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CPhysics::Debug_Init(graphic::CBoundingEnvelopes * _bEnvelope, HRESULT (CALLBACK * start)(graphic::CBoundingEnvelopes * bEnvelope), HRESULT (CALLBACK * finish)(graphic::CBoundingEnvelopes * bEnvelope), HRESULT (CALLBACK * processface)(graphic::CBoundingEnvelopes * bEnvelope, UINT uiVertCount, float * pBuf))
{
	bEnvelope = _bEnvelope;
	Start = start;
	Finish = finish;
	ProcessFace = processface;

	return ERRNOERROR;
};

// call before game start

////////////////////////////////////////////////////////////////////////////////////////////
//
// call after finishing game
//
////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CPhysics::Clean()
{
	for(int i=0; i<MAX_PLAYERS; i++)
	{
		SAFE_DELETE(cars[i]);
	}

	for(int i=0; i<(int)dynamicObjects.size(); i++)
	{
		SAFE_DELETE(dynamicObjects[i]);
	}
	dynamicObjects.clear();

	for(int i=0; i<(int)staticObjects.size(); i++)
	{
		SAFE_DELETE(staticObjects[i]);
	}
	staticObjects.clear();

	terrain.Release();

	//CleanUpMaterials();

	resources.ReleaseResources();

	return ERRNOERROR;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// should be called before class destruction
//
////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CPhysics::Destroy()
{
	NewtonDestroy(nWorld);

	return ERRNOERROR;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// updates world
//
////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CPhysics::Update(int dt)
{
	try
	{
		accumulator += (dt/1000.0f);

		while(accumulator >= UPDATE_STEP)
		{
			NewtonUpdate(nWorld, UPDATE_STEP);
			accumulator -= UPDATE_STEP;
		}

 		NewtonJoint * vehicle = cars[0]->VehicleJoint();
		if(Configuration.RenderBoundingEnvelopes)
			this->Debug_ShowCollisions();
	}
	catch(...)
	{
		ERRORMSG(ERRGENERIC, "CPhysics::Update()", "Physics update failure");
	}
#if 0
	//CAtlString msg;
	//msg.Format("%f\n", (float)dt/1000.0f);
	//OUTMSG(msg, 1);

	//dMatrix mat;
	//NewtonBodyGetMatrix(levelBody, &mat[0][0]);

	//NewtonBodyGetMatrix(cars[0]->NBody(), &mat[0][0]);

    //dVector vel;
    //NewtonBodyGetVelocity(cars[0]->NBody(), &vel.m_x);

    //if(abs(vel.m_x) < VELOCITY_LIMIT && abs(vel.m_y) < VELOCITY_LIMIT && abs(vel.m_z) < VELOCITY_LIMIT)
    //{
    //    if(cars[0]->frames == FRAMES-1)
    //        cars[0]->IsMoving(false);
    //    else
    //        cars[0]->frames++;
    //}
    //else
    //{
    //    cars[0]->frames = 0;
    //    cars[0]->IsMoving(true);        
    //}

    //CAtlString message;
	//message.Format(_T("[%f, %f, %f] - %i\n"), vel.m_x, vel.m_y, vel.m_z, cars[0]->IsMoving());  
    //OUTMSG(message, 1);

	
	
	//float omega[4];
	//int i=0;
	//CAtlString strFloat;

	//for(int i=0; i<4; i++)
	//{
	//	omega[i] = NewtonVehicleGetTireOmega(vehicle, cars[0]->wheels[i]->WheelID());
	//}

	//message = "##";
	//for(void * wheelID = NewtonVehicleGetFirstTireID(vehicle); wheelID; wheelID = NewtonVehicleGetNextTireID(vehicle, wheelID))
	//{
	//	omega[i] = NewtonVehicleGetTireOmega(vehicle, wheelID);
	//	i++;
	////	strFloat.Format(" %f", omega);
	////	message.Append(strFloat);
	//}
	//message.Append(" ##\n");
	//OUTMSG(message, 1);

	//float omega_mean = (omega[0] + omega[1])*0.5f;
	//float rpm = omega_mean * 30 / D3DX_PI;

	//int sg[4];
	//int t[4];
	//int a[4];
	//float nl[4];
	//float lat[4];
	//float lon[4];
	//void * wheelid;
	//float p;
	//dMatrix wheel_matrix;
	//int m[4];
	//for(int i=0; i<4; i++)
	//{
	//	wheelid = cars[0]->wheels[i]->WheelID();
	//	sg[i] = NewtonVehicleTireLostSideGrip(vehicle, wheelid);
	//	t[i] = NewtonVehicleTireLostTraction(vehicle, wheelid);
	//	a[i] = NewtonVehicleTireIsAirBorne(vehicle, wheelid);
	//	nl[i] = NewtonVehicleGetTireNormalLoad(vehicle, wheelid);
	//	lat[i] = NewtonVehicleGetTireLateralSpeed(vehicle, wheelid);
	//	lon[i] = NewtonVehicleGetTireLongitudinalSpeed(vehicle, wheelid);

	//	wheel_matrix = cars[0]->wheels[i]->_Matrix();
	//	m[i] = terrain.MaterialsMapf(CENTIMETER_2_METER(wheel_matrix.m_posit.m_x), CENTIMETER_2_METER(wheel_matrix.m_posit.m_z));
	//	cars[0]->wheels[i]->SetMaterial(m[i]);
	//}

	//p = cars[0]->wheels[0]->parameter;

	//dVector pos;
	//dMatrix mat;
	//NewtonBodyGetMatrix(cars[0]->NBody(), &mat[0][0]);
	//pos = mat.m_posit;

	//dVector delta = dVector(pos.m_x-oldPos.m_x, pos.m_y-oldPos.m_y, pos.m_z-oldPos.m_z);
	//float length = sqrt(delta.m_x*delta.m_x + delta.m_y*delta.m_y + delta.m_z*delta.m_z);

	//dVector velVec;
	//float vel;
	//dVector forceVec;
	//float time = (float)dt / 1000.0f;

	//data = (data * 10 - data + time)/10;

	//for(int i=0; i<MAX_PLAYERS; i++)
	//	if(cars[i])
	//	{
	//		//NewtonBodyGetVelocity(cars[i]->NBody(), &velVec.m_x);
	//		//vel = sqrt(velVec.m_x*velVec.m_x + velVec.m_y*velVec.m_y + velVec.m_z*velVec.m_z);
	//		//NewtonBodyGetForce(cars[i]->NBody(), &forceVec.m_x);

	//		//float torque = cars[i]->engine->Update(cars[i]->throttle, 0.0f, vel, 1);
	//		float rpm = NewtonVehicleGetTireOmega(vehicle, cars[i]->wheels[2]->WheelID());
	//		//float torque = cars[i]->engine->Update(0.0f, rpm, cars[i]->Speed(), 1);

	//		//cars[i]->wheels[0]->Torque(-torque);
	//		//cars[i]->wheels[1]->Torque(-torque);

	//		oldPos = pos;

	//	CAtlString msg;
	//	//msg.Format("Velocity: %5.1f Throttle: %5.1f Gear: %5d Rpm: %10.2f WheelRpm: %10.2f NewRPM: %10.2f Torque: %10.2f\n", 3.6f*vel, throttle, gear, rpm, rpm1, newRPM, torque);
	//	//msg.Format("Speed[%5.1f] MySpeed[%5.1f] Refresh[%5d] Newton[%5.1f]", 3.6f*vel, 3.6f*(length/data), dt, NewtonGetTimeStep(World()));
	//	//msg.Format("Pos[%5.1f %5.1f %5.1f]", oldPos.m_x, oldPos.m_y, oldPos.m_z);
	//	//msg.Format("Speedmps[%5.1f] Speedkmph[%5.1f] MWA[%5.1f] Length[%5.1f] Data[%15.10f] Time[%15.10f] Dt[%5.1f]", vel, 3.6f*vel, cars[0]->_maxWheelAngle, (float)length, data, time, (float)dt);

		//msg.Format("M[%3d %3d %3d %3d] P[%7.2f] LON[%7.1f %7.1f %7.1f %7.1f] LAT[%7.1f %7.1f %7.1f %7.1f] T[%3d %3d %3d %3d] SG[%3d %3d %3d %3d] A[%3d %3d %3d %3d]", m[0], m[1], m[2], m[3], p, lon[0], lon[1], lon[2], lon[3], lat[0
		//	], lat[1], lat[2], lat[3], t[0], t[1], t[2], t[3], sg[0], sg[1], sg[2], sg[3], a[0], a[1], a[2], a[3]);

	//	//msg.Format("NL[%7.1f %7.1f %7.1f %7.1f | %7.1f %7.1f | %7.1f ]", nl[0], nl[1], nl[2], nl[3], nl[0]+nl[1], nl[2]+nl[3], nl[0]+nl[1]+nl[2]+nl[3]);
	//	//OUTMSG(msg, 1);
	//	}
	//float _scale = 0.016f * (1.0f - cars[0]->Speed() / 60.0f);
	//float scale = _scale > 0.001f ? _scale : 0.001f;

	//CAtlString msg;
	//msg.Format("Speed[%5.1f] _Scale[%5.3f] Scale[%5.3f]", cars[0]->Speed(), _scale, scale);
	//OUTMSG(msg, 1);

	//CAtlString msg;
	//for(UINT i=0; i<MAX_PLAYERS; i++)
	//	if(cars[i])
	//	{
	//		for(UINT w=0; w<cars[i]->wheels.size(); w++)
	//			if(cars[i]->wheels[w]->Powered())
	//				cars[i]->wheels[w]->Torque(cars[i]->engine->Update(cars[i]->wheels[w]->_omega))
	//	}
#endif

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads map ...
//
//////////////////////////////////////////////////////////////////////////////////////////////
//HRESULT CPhysics::LoadMap(CAtlString terrainName)
//{
//	//terrain.Load(terrainName, this);
//
//	
//
//
//	return ERRNOERROR;
//};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// function for debugging purposes only - iterates through all NewtonBody objects in world
// and calls appropriate function that shows graphics
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysics::Debug_ShowCollisions()
{
    Start(bEnvelope);
	NewtonWorldForEachBodyDo(nWorld, Debug_ShowBodyCollision);
    Finish(bEnvelope);
	//Debug_ShowBodyCollision(this->cars[0]->NBody());
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function - called for each body
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysics::Debug_ShowBodyCollision(const NewtonBody * body)
{    
	UserData * data = ((UserData *)NewtonBodyGetUserData(body));
	CWheel * wheel;CCar * car;CStaticObject * sobject;CDynamicObject * dobject;
	//CPhysicsPlate * plate;
	dMatrix mat;
	//float minX, minZ, maxX, maxZ;
	switch(data->dataType)
	{
	//case PHYSICS_TERRAIN:
	//	plate = (CPhysicsPlate *)data->data;
	//	car = plate->physics->cars[0];
	//	NewtonBodyGetMatrix(car->NBody(), &mat[0][0]);

	//	minX = PLATE_SIZE * plate->posX * TERRAIN_PLATE_WIDTH_M;
	//	minZ = PLATE_SIZE * plate->posY * TERRAIN_PLATE_WIDTH_M;
	//	maxX = PLATE_SIZE * (plate->posX+1) * TERRAIN_PLATE_WIDTH_M;
	//	maxZ = PLATE_SIZE * (plate->posY+1) * TERRAIN_PLATE_WIDTH_M;

	//	if(LIES_BETWEEN(mat.m_posit.m_x, minX, maxX) && LIES_BETWEEN(mat.m_posit.m_z, minZ, maxZ))
	//		NewtonBodyForEachPolygonDo(body, Debug_CollisionPolygon);
	//	//if(m2.m_posit.m_x-30.0f < m1.m_posit.m_x && m1.m_posit.m_x < m2.m_posit.m_x+30.0f && m2.m_posit.m_z-30.0f < m1.m_posit.m_z && m1.m_posit.m_z < m2.m_posit.m_z+30.0f)
	//	//{
	//	//	NewtonBodyForEachPolygonDo(body, Debug_CollisionPolygon);
	//	//}
	//	break;
	case PHYSICS_WHEEL:
		NewtonBodyForEachPolygonDo(body, Debug_CollisionPolygon);
		wheel = (CWheel *)data->data;
		break;
	case PHYSICS_CAR:
		NewtonBodyForEachPolygonDo(body, Debug_CollisionPolygon);
		car = (CCar *)data->data;
		break;
	case PHYSICS_STATIC_OBJECT:
		NewtonBodyForEachPolygonDo(body, Debug_CollisionPolygon);
		sobject = (CStaticObject *)data->data;
		break;
	case PHYSICS_DYNAMIC_OBJECT:
		NewtonBodyForEachPolygonDo(body, Debug_CollisionPolygon);
		dobject = (CDynamicObject *)data->data;
		break;
	case PHYSICS_WORLD_BOUNDING_BOX:
		NewtonBodyForEachPolygonDo(body, Debug_CollisionPolygon);
		break;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function - called for each polygon of a body
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysics::Debug_CollisionPolygon(const NewtonBody * body, int vertexCount, const float * FaceArray, int faceId)
{
	float * faceArray = new float[vertexCount*3];
	if(faceArray)
	{
		for(int i=0; i<vertexCount*3; i++)
			faceArray[i] = FaceArray[i] * ONE_METER;
		ProcessFace(bEnvelope, vertexCount, faceArray);
		SAFE_DELETE_ARRAY(faceArray);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CPhysics::RayCast_CallbackStatic(const NewtonBody * nBody, const float * normal, int collisionID, void * userData, float intersetParam)
{
	CPhysics * physics = (CPhysics *) ((UserData *)userData)->data;

	return physics->RayCast_Callback(/*normal, collisionID, userData,*/ intersetParam);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CPhysics::RayCast_Callback(float intersetParam)
{
	getHeightParam = (WORLD_TOP-WORLD_BOTTOM) * intersetParam;
	return intersetParam;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CPhysics::GetHeight(float x, float z)
{
	dVector bottom = dVector(x, WORLD_BOTTOM, z);
	dVector top = dVector(x, WORLD_TOP, z);
	NewtonWorldRayCast(nWorld, &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	if(getHeightParam == 100.0f)
	{
		bottom = dVector(x+0.2f, WORLD_BOTTOM, z);
		top = dVector(x+0.2f, WORLD_TOP, z);
		NewtonWorldRayCast(nWorld, &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	}
	if(getHeightParam == 100.0f)
	{
		bottom = dVector(x-0.2f, WORLD_BOTTOM, z);
		top = dVector(x-0.2f, WORLD_TOP, z);
		NewtonWorldRayCast(nWorld, &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	}
	if(getHeightParam == 100.0f)
	{
		bottom = dVector(x, WORLD_BOTTOM, z+0.2f);
		top = dVector(x, WORLD_TOP, z+0.2f);
		NewtonWorldRayCast(nWorld, &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	}
	if(getHeightParam == 100.0f)
	{
		bottom = dVector(x, WORLD_BOTTOM, z-0.2f);
		top = dVector(x, WORLD_TOP, z-0.2f);
		NewtonWorldRayCast(nWorld, &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	}
	CAtlString msg;
	if(getHeightParam == 100.0f)
	{
		msg.Format("GetHeight failure: height %5.2f at coords [%5.2f,%5.2f]", getHeightParam, x, z);
		OUTMSG(msg, 1);
	}
	return getHeightParam;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysics::ReleaseStartUpData()
{
	resources.ReleaseMeshes();
};

// This code is no longer used
#if 0
////////////////////////////////////////////////////////////////////////////////////////////
//
// initializes materials
//
////////////////////////////////////////////////////////////////////////////////////////////
//HRESULT CPhysics::InitMaterials()
//{
//	//// TODO: change this -> Softness, Elasticity, Frictions (st, dyn) and Collidable param needed
//	//RESOURCEID resFrictionCoefTableID = rsm->LoadResource("", RES_FrictionCoefTable);
//	//if(resFrictionCoefTableID < 1) ERRORMSG(ERRGENERIC, "CPhysics::InitMaterials()", "Could not load frictioncoeftable object.");
//
//	//FrictionCoefTable * fct = rsm->GetFrictionCoefTable(resFrictionCoefTableID);
//	//if(!fct) ERRORMSG(ERRNOTFOUND, "CPhysics::InitMaterials()", "Could not obtain frictioncoeftable object.");
//	//
//	//int fctuiTableSize = 5;
//
//	//int defaultID = NewtonMaterialGetDefaultGroupID(nWorld);
//
//	//NewtonMaterialSetDefaultSoftness(nWorld, defaultID, defaultID, 0.05f);
//	//NewtonMaterialSetDefaultElasticity(nWorld, defaultID, defaultID, 0.4f);
//	//NewtonMaterialSetDefaultCollidable(nWorld, defaultID, defaultID, 1);
//	//NewtonMaterialSetDefaultFriction(nWorld, defaultID, defaultID, 1.0f, 0.5f);
//	//NewtonMaterialSetCollisionCallback(nWorld, defaultID, defaultID, NULL, NULL, NULL, NULL);
//
//	//materialSize = fctuiTableSize + 1;//fct->uiTableSize + 1;
//	//materialID = new int[materialSize];
//
//	//materialID[materialSize-1] = defaultID;
//
//	//for(int i=0; i<materialSize; i++)
//	//	materialID[i] = NewtonMaterialCreateGroupID(nWorld);
//
//	//for(int i=0; i<fct->uiTableSize; i++)
//	//	for(int j=0; j<fct->uiTableSize; j++)
//	//	{
//	//		NewtonMaterialSetDefaultElasticity(nWorld, materialID[i], materialID[j], data);
//	//		NewtonMaterialSetDefaultFriction(nWorld, materialID{i], materialID[j], data1, data2);
//	//		NewtonMaterialSetCollisionCallback(nWorld, materialID[i], materialID[j], NULL, NULL, NULL, NULL);
//	//		 and more
//	//	}
//	
//
//	int defaultID;
//
//	wood_wood.Init("wood_wood");
//	wood_metal.Init("wood_metal");
//	wood_level.Init("wood_level");
//	metal_metal.Init("metal_metal");
//	metal_level.Init("metal_level");
//	vehicle_level.Init("vehicle_level");
//
//	defaultID = NewtonMaterialGetDefaultGroupID(nWorld);
//
//	woodID = NewtonMaterialCreateGroupID(nWorld);
//	metalID = NewtonMaterialCreateGroupID(nWorld);
//	levelID = NewtonMaterialCreateGroupID(nWorld);
//	vehicleID = NewtonMaterialCreateGroupID(nWorld);
//	characterID = NewtonMaterialCreateGroupID(nWorld);
//
//	NewtonMaterialSetDefaultSoftness (nWorld, defaultID, defaultID, 0.05f);
//	NewtonMaterialSetDefaultElasticity (nWorld, defaultID, defaultID, 0.4f);
//	NewtonMaterialSetDefaultCollidable (nWorld, defaultID, defaultID, 1);
//	NewtonMaterialSetDefaultFriction (nWorld, defaultID, defaultID, 1.0f, 0.5f);
//	NewtonMaterialSetCollisionCallback (nWorld, defaultID, defaultID, &wood_wood, GenericContactBegin, GenericContactProcess, GenericContactEnd);
//	//NewtonMaterialSetCollisionCallback (nWorld, woodID, woodID, &wood_wood, NULL, NULL, NULL);
//
//	// set the material properties for wood on wood
//	NewtonMaterialSetDefaultElasticity (nWorld, woodID, woodID, 0.3f);
//	NewtonMaterialSetDefaultFriction (nWorld, woodID, woodID, 1.1f, 0.7f);
//	NewtonMaterialSetCollisionCallback (nWorld, woodID, woodID, &wood_wood, GenericContactBegin, GenericContactProcess, GenericContactEnd); 
//	//NewtonMaterialSetCollisionCallback (nWorld, woodID, woodID, &wood_wood, NULL, NULL, NULL);
//
//	// set the material properties for wood on metal
//	NewtonMaterialSetDefaultElasticity (nWorld, woodID, metalID, 0.5f);
//	NewtonMaterialSetDefaultFriction (nWorld, woodID, metalID, 0.8f, 0.6f);
//	NewtonMaterialSetCollisionCallback (nWorld, woodID, metalID, &wood_metal, GenericContactBegin, GenericContactProcess, GenericContactEnd); 
//	//NewtonMaterialSetCollisionCallback (nWorld, woodID, metalID, &wood_metal, NULL, NULL, NULL);
//
//	// set the material properties for wood on levell
//	NewtonMaterialSetDefaultElasticity (nWorld, woodID, levelID, 0.3f);
//	NewtonMaterialSetDefaultFriction (nWorld, woodID, levelID, 0.9f, 0.3f);
//	NewtonMaterialSetCollisionCallback (nWorld, woodID, levelID, &wood_level, GenericContactBegin, GenericContactProcess, GenericContactEnd); 
//	//NewtonMaterialSetCollisionCallback (nWorld, woodID, levelID, &wood_level, NULL, NULL, NULL);
//
//	// set the material properties for metal on metal
//	NewtonMaterialSetDefaultElasticity (nWorld, metalID, metalID, 0.7f);
//	NewtonMaterialSetDefaultFriction (nWorld, metalID, metalID, 0.5f, 0.2f);
//	NewtonMaterialSetCollisionCallback (nWorld, metalID, metalID, &metal_metal, GenericContactBegin, GenericContactProcess, GenericContactEnd); 
//	//NewtonMaterialSetCollisionCallback (nWorld, metalID, metalID, &metal_metal, NULL, NULL, NULL);
//
//	// set the material properties for metal on level
//	NewtonMaterialSetDefaultElasticity (nWorld, metalID, levelID, 0.4f);
//	NewtonMaterialSetDefaultFriction (nWorld, metalID, levelID, 0.6f, 0.4f);
//	NewtonMaterialSetCollisionCallback (nWorld, metalID, levelID, &metal_level, GenericContactBegin, GenericContactProcess, GenericContactEnd); 
//	//NewtonMaterialSetCollisionCallback (nWorld, metalID, levelID, &metal_level, NULL, NULL, NULL);
//
//	// create the charater material interactions
//	NewtonMaterialSetDefaultElasticity (nWorld, woodID, characterID, 0.3f);
//	NewtonMaterialSetDefaultFriction (nWorld, woodID, characterID, 1.1f, 0.7f);
//	NewtonMaterialSetCollisionCallback (nWorld, woodID, characterID, &wood_wood, GenericContactBegin, GenericContactProcess, GenericContactEnd); 
//	//NewtonMaterialSetCollisionCallback (nWorld, woodID, characterID, &wood_wood, NULL, NULL, NULL);
//
//	// set the material properties for metal on metal
//	NewtonMaterialSetDefaultElasticity (nWorld, metalID, characterID, 0.7f);
//	NewtonMaterialSetDefaultFriction (nWorld, metalID, characterID, 0.5f, 0.2f);
//	NewtonMaterialSetCollisionCallback (nWorld, metalID, characterID, &metal_metal, GenericContactBegin, GenericContactProcess, GenericContactEnd); 
//	//NewtonMaterialSetCollisionCallback (nWorld, metalID, characterID, &metal_metal, NULL, NULL, NULL);
//
//	// set the material properties for character on level 
//	NewtonMaterialSetDefaultElasticity (nWorld, levelID, characterID, 0.1f);
//	NewtonMaterialSetDefaultFriction (nWorld, levelID, characterID, 0.6f, 0.6f);
//	NewtonMaterialSetCollisionCallback (nWorld, levelID, characterID, &metal_level, GenericContactBegin, CharacterContactProcess, GenericContactEnd); 
//	//NewtonMaterialSetCollisionCallback (nWorld, levelID, characterID, &metal_level, NULL, NULL, NULL);
//
//	// set the material properties for vehicle on level
//
//	NewtonMaterialSetDefaultElasticity (nWorld, levelID, vehicleID, 0.1f);
////	NewtonMaterialSetDefaultFriction (nWorld, levelID, vehicleID, 1.0f, 1.0f);
//	NewtonMaterialSetCollisionCallback (nWorld, levelID, vehicleID, &vehicle_level, GenericContactBegin, VehicleContactProcess, GenericContactEnd); 
//	//NewtonMaterialSetCollisionCallback (nWorld, levelID, vehicleID, &vehicle_level, NULL, NULL, NULL);
//	
//	return ERRNOERROR;
//};
//
//HRESULT CPhysics::CleanUpMaterials()
//{
//	wood_wood.Destroy();
//	wood_metal.Destroy();
//	wood_level.Destroy();
//	metal_metal.Destroy();
//	metal_level.Destroy();
//
//	vehicle_level.Destroy();
//
//	return ERRNOERROR;
//};
//
//////////////////////////////////////////////////////////////////////////////////////////////
//
//	struct _VERTEX {
//		float x, y, z;
//		float nx, ny, nz;
//		float tu, tv;
//	} * vert;
//
//	RESOURCEID resTerrainID = rsm->LoadResource(terrainName, RES_Terrain);
//	if(resTerrainID < 1) ERRORMSG(ERRGENERIC, "CPhysics::LoadMap()", "Could not load terrain object.");
//
//	Terrain * resTerrain = rsm->GetTerrain(resTerrainID);
//	if(!resTerrain) ERRORMSG(ERRNOTFOUND, "CPhysics::LoadMap()", "Could not obtain terrain object.");
//
//	int size_x = resTerrain->uiSizeX;
//	int size_y = resTerrain->uiSizeY;
//
//	NewtonCollision * collision = NewtonCreateTreeCollision(nWorld, NULL);
//
//	NewtonTreeCollisionBeginBuild(collision);
//
//	TerrainPlateInfo * resPlateInfo;
//	RESOURCEID resGrPlateID;
//	GrPlate * resGrPlate;
//	RESOURCEID resGrSurfaceID;
//	GrSurface * resGrSurface;
//	int segsx, segsz;
////	int rotation = resPlateInfo->uiPlateRotation;
//
//	for(int iy=0; iy<size_y; iy++)
//		for(int ix=0; ix<size_x; ix++)
////	for(int iy=49; iy<52; iy++)
////		for(int ix=49; ix<52; ix++)
//		{
//			resPlateInfo = &resTerrain->pTerrainPlateInfo[size_x*iy + ix];
//
//			resGrPlateID = resTerrain->pGrPlateIDs[resPlateInfo->uiGrPlateIndex];
//			
//			resGrPlate = rsm->GetGrPlate(resGrPlateID);
//			if(!resGrPlate) ERRORMSG(ERRNOTFOUND, "Physics::LoadMap()", "Could not obtain grplate object.");
//
//			resGrSurfaceID = resGrPlate->LoD[resGrPlate->uiLoDCount-2];
//			resGrSurface = rsm->GetGrSurface(resGrSurfaceID);
//
//			resGrSurface->pVertices->Lock(0, 0, (LPVOID *) &vert, 0);
//
//			segsx = resGrSurface->SegsCountX;
//			segsz = resGrSurface->SegsCountZ;
//
//			int rotation = resPlateInfo->uiPlateRotation;
//			int index;
//
//			float height = resPlateInfo->uiHeight * TERRAIN_PLATE_HEIGHT;
//			float xposition = TERRAIN_PLATE_WIDTH / 2 + TERRAIN_PLATE_WIDTH * ix;
//			float zposition = TERRAIN_PLATE_WIDTH / 2 + TERRAIN_PLATE_WIDTH * iy;
//
//			dVector point[4];
//
//			if(resGrPlate->bPlanar)
//			{
//				//point[0].m_x = vert[0].x + xposition;
//				//point[0].m_y = vert[0].y + height;
//				//point[0].m_z = vert[0].z + zposition;
//
//				//point[3].m_x = vert[segsx].x + xposition;
//				//point[3].m_y = vert[segsx].y + height;
//				//point[3].m_z = vert[segsx].z + zposition;
//			
//				//point[2].m_x = vert[(segsx+1)*(segsz+1)-1].x + xposition;
//				//point[2].m_y = vert[(segsx+1)*(segsz+1)-1].y + height;
//				//point[2].m_z = vert[(segsx+1)*(segsz+1)-1].z + zposition;
//
//				//point[1].m_x = vert[segsz*(segsx+1)].x + xposition;
//				//point[1].m_y = vert[segsz*(segsx+1)].y + height;
//				//point[1].m_z = vert[segsz*(segsx+1)].z + zposition;
//
//				//NewtonTreeCollisionAddFace(collision, 4, &point[0].m_x, sizeof (dVector), 1);
//
//				point[0].m_x = vert[0].x + xposition;
//				point[0].m_y = vert[0].y + height;
//				point[0].m_z = vert[0].z + zposition;
//
//				point[1].m_x = vert[segsx].x + xposition;
//				point[1].m_y = vert[segsx].y + height;
//				point[1].m_z = vert[segsx].z + zposition;
//
//				point[2].m_x = vert[segsz*(segsx+1)].x + xposition;
//				point[2].m_y = vert[segsz*(segsx+1)].y + height;
//				point[2].m_z = vert[segsz*(segsx+1)].z + zposition;
//
//				NewtonTreeCollisionAddFace(collision, 3, &point[0].m_x, sizeof (dVector), 1);
//
//				point[0].m_x = vert[segsx].x + xposition;
//				point[0].m_y = vert[segsx].y + height;
//				point[0].m_z = vert[segsx].z + zposition;
//
//				point[2].m_x = vert[segsz*(segsx+1)].x + xposition;
//				point[2].m_y = vert[segsz*(segsx+1)].y + height;
//				point[2].m_z = vert[segsz*(segsx+1)].z + zposition;
//
//				point[1].m_x = vert[(segsx+1)*(segsz+1)-1].x + xposition;
//				point[1].m_y = vert[(segsx+1)*(segsz+1)-1].y + height;
//				point[1].m_z = vert[(segsx+1)*(segsz+1)-1].z + zposition;
//
//				NewtonTreeCollisionAddFace(collision, 3, &point[0].m_x, sizeof (dVector), 1);				
//				
//			}
//			else
//			{
//				for(int sz=0; sz<segsz; sz++)
//					for(int sx=0; sx<segsx; sx++)
//					{						
//						// z * width + x
//						// y * segsx + x			
//
//						// [ sx, sz ]
//						// [ sx + 1, sz ]
//						// [ sx, sz + 1 ]
//						
//						//point[0].m_x = vert[sz * segsx + sx].x;
//						//point[0].m_y = vert[sz * segsx + sx].y;
//						//point[0].m_z = vert[sz * segsx + sx].z;
//
//						//point[1].m_x = vert[sz * segsx + (sx+1)].x;
//						//point[1].m_y = vert[sz * segsx + (sx+1)].y;
//						//point[1].m_z = vert[sz * segsx + (sx+1)].z;
//						//
//						//point[2].m_x = vert[(sz+1) * segsx + sx].x;
//						//point[2].m_y = vert[(sz+1) * segsx + sx].y;
//						//point[2].m_z = vert[(sz+1) * segsx + sx].z;
//						
//						index = Rotation(0, sx, sz, segsx, segsz);
//						point[0].m_x = vert[index].x + xposition;
//						point[0].m_y = vert[Rotation(rotation, sx, sz, segsx, segsz)].y + height;
//						point[0].m_z = vert[index].z + zposition;
//
//						index = Rotation(0, sx+1, sz, segsx, segsz);
//						point[1].m_x = vert[index].x + xposition;
//						point[1].m_y = vert[Rotation(rotation, sx+1, sz, segsx, segsz)].y + height;
//						point[1].m_z = vert[index].z + zposition;
//						
//						index = Rotation(0, sx, sz+1, segsx, segsz);
//						point[2].m_x = vert[index].x + xposition;
//						point[2].m_y = vert[Rotation(rotation, sx, sz+1, segsx, segsz)].y + height;
//						point[2].m_z = vert[index].z + zposition;
//
//						NewtonTreeCollisionAddFace(collision, 3, &point[0].m_x, sizeof (dVector), 1);
//
//						// [ sx + 1, sz ]
//						// [ sx, sz + 1 ]
//						// [ sx + 1, sz + 1 ]
//						
//						//point[0].m_x = vert[sz * segsx + (sx+1)].x;
//						//point[0].m_y = vert[sz * segsx + (sx+1)].y;
//						//point[0].m_z = vert[sz * segsx + (sx+1)].z;
//
//						//point[1].m_x = vert[(sz+1) * segsx + sx].x;
//						//point[1].m_y = vert[(sz+1) * segsx + sx].y;
//						//point[1].m_z = vert[(sz+1) * segsx + sx].z;
//						//
//						//point[2].m_x = vert[(sz+1) * segsx + (sx+1)].x;
//						//point[2].m_y = vert[(sz+1) * segsx + (sx+1)].y;
//						//point[2].m_z = vert[(sz+1) * segsx + (sx+1)].z;
//						
//						index = Rotation(0, sx+1, sz, segsx, segsz);
//						point[0].m_x = vert[index].x + xposition;
//						point[0].m_y = vert[Rotation(rotation, sx+1, sz, segsx, segsz)].y + height;
//						point[0].m_z = vert[index].z + zposition;
//
//						index = Rotation(0, sx, sz+1, segsx, segsz);
//						point[2].m_x = vert[index].x + xposition;
//						point[2].m_y = vert[Rotation(rotation, sx, sz+1, segsx, segsz)].y + height;
//						point[2].m_z = vert[index].z + zposition;
//						
//						index = Rotation(0, sx+1, sz+1, segsx, segsz);
//						point[1].m_x = vert[index].x + xposition;
//						point[1].m_y = vert[Rotation(rotation, sx+1, sz+1, segsx, segsz)].y + height;
//						point[1].m_z = vert[index].z + zposition;
//
//						NewtonTreeCollisionAddFace(collision, 3, &point[0].m_x, sizeof (dVector), 1);
//					}
//			}
//
//			resGrSurface->pVertices->Unlock();
//		}
//
//	NewtonTreeCollisionEndBuild(collision, 0);
//
//    levelBody = NewtonCreateBody(nWorld, collision);
//
//    userData.dataType = PHYSICS_TERRAIN;
//    userData.data = this;
//
//    NewtonBodySetUserData(levelBody, &userData);
//
//    dVector boxP0; 
//	dVector boxP1;
//    dMatrix matrix (GetIdentityMatrix());
//
//	NewtonCollisionCalculateAABB (collision, &matrix[0][0], &boxP0.m_x, &boxP1.m_x); 
//
//	boxP0.m_x -= 1000.0f;
//	boxP0.m_y -= 1000.0f;
//	boxP0.m_z -= 1000.0f;
//	boxP1.m_x += 1000.0f;
//	boxP1.m_y += 100000.0f;
//	boxP1.m_z += 1000.0f;
//
//	// set the world size
//	NewtonSetWorldSize (nWorld, &boxP0.m_x, &boxP1.m_x);  
//
//
//	NewtonReleaseCollision(nWorld, collision);
//
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//
// this callback is called when the two aabb boxes of the collisiong object overlap
//int CPhysics::GenericContactBegin (const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1)
//{
//
//	// get the pointer to the special effect struture
//	g_currentEffect = (SpecialEffectStruct *)NewtonMaterialGetMaterialPairUserData (material);
//
//	// save the collisiong bodies
//	g_currentEffect->m_body0 = (NewtonBody*) body0;
//	g_currentEffect->m_body1 = (NewtonBody*) body1;
//
//	// clear the contact normal speed 
//	g_currentEffect->m_contactMaxNormalSpeed = 0.0f;
//
//	// clear the contact sliding speed
//	g_currentEffect->m_contactMaxTangentSpeed = 0.0f;
//
//
//	// return one the tell Newton the application wants to proccess this contact
//	return 1;
//};
//
// this callback is called for every contact between the two bodies
//int CPhysics::GenericContactProcess (const NewtonMaterial* material, const NewtonContact* contact)
//{
//	dFloat speed0;
//	dFloat speed1;
//	dVector normal;
//
//	// Get the maximun normal speed of this impact. this can be used for particels of playing collision sound
//	speed0 = NewtonMaterialGetContactNormalSpeed (material, contact);
//	if (speed0 > g_currentEffect->m_contactMaxNormalSpeed) {
//		// save the position of the contact (for 3d sound of particles effects)
//		g_currentEffect->m_contactMaxNormalSpeed = speed0;
//		NewtonMaterialGetContactPositionAndNormal (material, &g_currentEffect->m_position.m_x, &normal.m_x);
//	}
//
//	// get the maximun of the two sliding contact speed
//	speed0 = NewtonMaterialGetContactTangentSpeed (material, contact, 0);
//	speed1 = NewtonMaterialGetContactTangentSpeed (material, contact, 1);
//	if (speed1 > speed0) {
//		speed0 = speed1;
//	}
//
//	// Get the maximun tangent speed of this contact. this can be used for particles(sparks) of playing scratch sounds 
//	if (speed0 > g_currentEffect->m_contactMaxTangentSpeed) {
//		// save the position of the contact (for 3d sound of particles effects)
//		g_currentEffect->m_contactMaxTangentSpeed = speed0;
//		NewtonMaterialGetContactPositionAndNormal (material, &g_currentEffect->m_position.m_x, &normal.m_x);
//	}
//
//	
//	#ifdef DEBUG_NEWTON
//	// the application can implement some kind of contact debug here
////	if (m_showDegug) {
////		if (m_contactCount < NEWTON_MAX_CONTACTS) {
////			dVector point;
////			dVector normal;	
////			NewtonMaterialGetContactPositionAndNormal (material, &point.X, &normal.X);
////			m_contactNormals[m_contactCount * 2] = point * NewtonToIrr;
////			m_contactNormals[m_contactCount * 2 + 1] = m_contactNormals[m_contactCount * 2] + normal * (NewtonToIrr * 0.5f);
////			m_contactCount ++;
////		}
////	}
//	#endif
//
//	// return one to tell Newton we want to accept this contact
//	return 1;
//}
//
// this function is call affter all contacts for this pairs is proccesed
//void CPhysics::GenericContactEnd (const NewtonMaterial* material)
//{
//	#define MIN_CONTACT_SPEED 15
//	#define MIN_SCRATCH_SPEED 5
//
//	// if the max contact speed is larger than some minumum value. play a sound
//	if (g_currentEffect->m_contactMaxNormalSpeed > MIN_CONTACT_SPEED) {
//		g_currentEffect->PlayImpactSound (g_currentEffect->m_contactMaxNormalSpeed - MIN_CONTACT_SPEED);
//	}
//
//	// if the max contact speed is larger than some minumum value. play a sound
//	if (g_currentEffect->m_contactMaxNormalSpeed > MIN_SCRATCH_SPEED) {
//		g_currentEffect->PlayScratchSound (g_currentEffect->m_contactMaxNormalSpeed - MIN_SCRATCH_SPEED);
//	}
//
//	// implement here any other effects
//};
//
// this is use to constomized the chatarer control contact vectors
//int CPhysics::CharacterContactProcess (const NewtonMaterial* material, const NewtonContact* contact)
//{
//	dFloat mass;
//	dFloat Ixx;
//	dFloat Iyy;
//	dFloat Izz;
//	NewtonBody* sphere;
//	
//	// apply the default behaviuor
//	GenericContactProcess (material, contact);
//
//	// get the spherical body, it is the body with non zero mass
//	// this way to determine the body is quit and dirty but it no safe, it only work in this
//	// case because one of the two bodies is the terrain which we now have infinite mass.
//	// a better way is by getting the user data an finding some object identifier stored with the user data.
//	sphere = g_currentEffect->m_body0;
//	NewtonBodyGetMassMatrix (g_currentEffect->m_body0, &mass, &Ixx, &Iyy, &Izz);
//	if (mass == 0.0f) {
//		sphere = g_currentEffect->m_body1;
//		NewtonBodyGetMassMatrix (g_currentEffect->m_body1, &mass, &Ixx, &Iyy, &Izz);
//	}
//
//	// align the tangent contact direction with the velocity vector of the ball
//	dVector posit;
//	dVector normal;
//	
//	dVector velocity;
//	NewtonBodyGetVelocity(sphere, &velocity.m_x);
//	NewtonMaterialGetContactPositionAndNormal (material, &posit.m_x, &normal.m_x);
//
//	// calculate ball velocity perpendicular to the contact normal
//	dVector tangentVelocity (velocity - normal.Scale (normal % velocity));
//
//	// align the tangent at the contact point with the tangent velocity vector of the ball
//	NewtonMaterialContactRotateTangentDirections (material, &tangentVelocity.m_x);
//	
//
//
//	// return one to tell Newton we want to accept this contact
//	return 1;
//};
//
// this is use to constomized the chatarer control contact vectors
//int CPhysics::VehicleContactProcess (const NewtonMaterial* material, const NewtonContact* contact)
//{
//	int collisionID;
//	int faceMaterialID;
//	dFloat mass;
//	dFloat Ixx;
//	dFloat Iyy;
//	dFloat Izz;
//	NewtonBody* car;
//	
//	// apply the default behaviuor
////	GenericContactProcess (material, contact);
//
//	// get the car, it is the body with non zero mass
//	// this way to determine the body is quit and dirty but it no safe, it only works in this
//	// case because one of the two bodies is the terrain which we now have infinite mass.
//	// a better way is by getting the user data an finding some object identifier stored with the user data.
//
//	car = g_currentEffect->m_body0;
//	NewtonBodyGetMassMatrix (g_currentEffect->m_body0, &mass, &Ixx, &Iyy, &Izz);
//	if (mass == 0.0f) {
//		car = g_currentEffect->m_body1;
//	}
//
//
//	collisionID = NewtonMaterialGetBodyCollisionID (material, car);
//
//	switch (collisionID) 
//	{
//		// the car body is colliding
//		case CHASIS_COLLITION_ID:
//		{
//			// get the contact user data assigned to the terrain face generating the contact
//			faceMaterialID = NewtonMaterialGetContactFaceAttribute (material);
//
//			// used the face ID to identifine the terrain material and apply per face material properties
//			switch(faceMaterialID) {
//				case 0:
//					//set the friction values depending on the fase material
//					NewtonMaterialSetContactStaticFrictionCoef (material, 0.7f, 0);
//					NewtonMaterialSetContactStaticFrictionCoef (material, 0.7f, 1);
//					NewtonMaterialSetContactKineticFrictionCoef (material, 0.3f, 0);
//					NewtonMaterialSetContactKineticFrictionCoef (material, 0.3f, 1);
//					// maybe svae the contact position to generate skid marks, smokes, any special effect
//					// ....
//
//					break;
//								
//				default:
//					//set the friction values dep[ending on the fase material
//					NewtonMaterialSetContactStaticFrictionCoef (material, 0.7f, 0);
//					NewtonMaterialSetContactStaticFrictionCoef (material, 0.7f, 1);
//					NewtonMaterialSetContactKineticFrictionCoef (material, 0.3f, 0);
//					NewtonMaterialSetContactKineticFrictionCoef (material, 0.3f, 1);
//					// maybe svae the contact position to generate skid marks, smokes, any special effect
//					// ....
//
//			}
//
//
//			// maybe save the speed to generate impact sound
//			break;
//		}
//			
//
//		// a tire body is colliding
//		case TIRE_COLLITION_ID:
//		{
//			// get the contact user data assigned to the terrain face generating the contact
//			// get the contact user data assigned to the terrain face generating the contact
//			faceMaterialID = NewtonMaterialGetContactFaceAttribute (material);
//
//			// used the face ID to identifine the terrain material and apply per face material properties
//			switch(faceMaterialID) 
//			{
//				case 0:
//					// used the max allowable friction coeficenst 2.0f in Newton
//					// set the friction values depending on the face material
//					// first axis is the longitudilal tire friction (in general lateral and longitudinal friction should be equal)
//					// but the apliication can play with this values to generate specials effects
//					// first axis is the longitudilal tire friction
//					NewtonMaterialSetContactStaticFrictionCoef (material, 2.0f, 0);
//					NewtonMaterialSetContactKineticFrictionCoef (material, 1.9f, 0);
//
//					// second axis is the lateral tire friction (in general lateral and longitudinal friction should be equal)
//					// but the aplication can play with this values to generate specials effects
//					NewtonMaterialSetContactStaticFrictionCoef (material, 2.0f, 1);
//					NewtonMaterialSetContactKineticFrictionCoef (material, 1.9f, 1);
//
//					// maybe save the speed to generate impact sound
//					// .....
//
//					break;
//
//				default:
//					//set the friction values depending on the face material
//					// first axis is the longitudilal tire friction (in general lateral and longitudinal friction should be equal)
//					// but the apliication can play with this values to generate specials effects
//					// first axis is the longitudilal tire friction
//					NewtonMaterialSetContactStaticFrictionCoef (material, 2.0f, 0);
//					NewtonMaterialSetContactKineticFrictionCoef (material, 1.9f, 0);
//
//					// second axis is the lateral tire friction (in general lateral and longitudinal friction should be equal)
//					// but the aplication can play with this values to generate specials effects
//					NewtonMaterialSetContactStaticFrictionCoef (material, 2.0f, 1);
//					NewtonMaterialSetContactKineticFrictionCoef (material, 1.9f, 1);
//
//					// maybe svae the contact position to generate skid marks, smokes, any special effect
//					// ....
//			}
//			
//			break;
//		}
//	}
//
//	// return one to tell Newton we want to accept this contact
//	return 1;
//};
//
//////////////////////////////////////////////////////////////////////////////////////////////
#endif