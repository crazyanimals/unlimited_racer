#include "Car.h"
#include "Physics.h"
#include "../main/Globals.h"	// physics, etc.

using namespace physics;

//////////////////////////////////////////////////////////////////////////////////////////////
//
// class destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CCar::~CCar()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// class initialization
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CCar::Init(NewtonWorld * world, CResources * resources, InitCar * initCar, CPhysics * physics)
{
	HRESULT hr;

	try
	{
		_force = dVector(0.0f, 0.0f, 0.0f);
		_speed = 0.0f;
		m_materials = &physics->materials;
		this->physics = physics;

		// create engine
		SAFE_NEW(engine, CEngine(), "CCar::Init()")
		if(hr = engine->Init(&initCar->engine, m_materials))
			ERRORMSG(hr, "CCar::Init()", "");		

		wheels.clear();
		alwaysActive = true;
		scaling = dVector(initCar->scale);
		nBodyMatrix = GetIdentityMatrix();

		gearBoxType = GEARBOX_AUTOMATIC;
		//gearBoxType = GEARBOX_MANUAL;
		shift_up = initCar->shift_up;
		shift_down = initCar->shift_down;

		MESHID meshID = resources->LoadDynamicMesh(initCar->model, &scaling, true);
		if(meshID == -1) ERRORMSG(ERRGENERIC, "CCar::Init()", "");
		
		CPhysicsMesh * mesh = resources->GetMesh(meshID);
		if(!mesh) ERRORMSG(ERRGENERIC, "CCar::Init()", "");

		bboxDiameter = mesh->diameter;
		
		// car frontal area calculation
		dVector size (mesh->maxBox - mesh->minBox);
		frontalArea = size.m_x * size.m_y * 0.9f;
		
		//dMatrix t (GetIdentityMatrix());
		//t.m_posit = (mesh->maxBox + mesh->minBox).Scale(0.5);
	    
		// TODO: remove fixed value
		NewtonCollision * collision = resources->GetCollision(meshID);
		if(!collision) ERRORMSG(ERRGENERIC, "CCar::Init()", "");
		NewtonConvexCollisionSetUserID(collision, CHASIS_COLLITION_ID);
		
		nBody = NewtonCreateBody(world, collision);

		userData.dataType = PHYSICS_CAR;
		userData.data = this;
		userData.material0 = initCar->material;
		userData.material1 = initCar->wheel_material;
		NewtonBodySetUserData(nBody, &userData);

		// set car's material
		NewtonBodySetMaterialGroupID(nBody, physics->materials.GetMaterialID (initCar->material));

		NewtonBodySetForceAndTorqueCallback(nBody, ApplyForceAndTorque_CallbackStatic);	
		NewtonBodySetTransformCallback(nBody, SetTransform_CallbackStatic);
		
		dVector inertia, origin, origin1;
		NewtonConvexCollisionCalculateInertialMatrix(collision, &inertia.m_x, &origin.m_x);

		initCar->cog.m_x = CENTIMETER_2_METER(initCar->cog.m_x)*scaling.m_x;
		initCar->cog.m_y = CENTIMETER_2_METER(initCar->cog.m_y)*scaling.m_y;
		initCar->cog.m_z = CENTIMETER_2_METER(initCar->cog.m_z)*scaling.m_z;
		origin1 = origin + initCar->cog;

		NewtonBodySetMassMatrix(nBody, initCar->mass, initCar->mass*inertia.m_x, initCar->mass*inertia.m_y, initCar->mass*inertia.m_z);
		NewtonBodySetCentreOfMass(nBody, &origin1.m_x);

		cogx = origin1.m_x;

		NewtonBodySetAutoFreeze(nBody, 0);

		NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);

		car_w_length = abs(CENTIMETER_2_METER(initCar->wheels[0].position.m_z-initCar->wheels[2].position.m_z))*scaling.m_z;
		car_w_width = abs(CENTIMETER_2_METER(initCar->wheels[0].position.m_x-initCar->wheels[1].position.m_x))*scaling.m_x;
	  
		//wheels initialization

		dVector updir (nBodyMatrix.m_up);
		//updir.m_w = 1.0f;

		vehicleJoint = NewtonConstraintCreateVehicle(world, &updir[0], nBody);

		//NewtonJointSetStiffness(vehicleJoint, 1.0f);

		NewtonVehicleSetTireCallback(vehicleJoint, WheelUpdateCallbackStatic);

		CWheel * wheel;

		for(int i=0; i<(int)initCar->wheels.size(); i++)
		{
			SAFE_NEW(wheel, CWheel(), "CCar::Init()");
			initCar->wheels[i].car = this;
			if((hr = wheel->Init(world, initCar->mass, resources, &initCar->wheels[i], &physics->materials, i)) != ERRNOERROR)
			{
				SAFE_DELETE(wheel);
				ERRORMSG(hr, "CCar::Init()", "");
			}

			// insert it into wheelVectors according properties
			wheels.push_back (wheel);
			if (initCar->wheels[i].powered)
			{
				poweredWheels.push_back (wheel);
			}

			wheels[i]->dynamicObjectsIndex = (int)physics->dynamicObjects.size();
			physics->dynamicObjects.push_back((CDynamicObject *)wheels[i]);
			if(i<2)
				wheels[i]->type = 0;
			else
				wheels[i]->type = 1;
		}
		
		NewtonBodySetLinearDamping(nBody, 0.0f);
		//dVector aDamp (0.1f, 0.1f, 0.1f);
		//NewtonBodySetAngularDamping(nBody, &aDamp.m_x);

		this->Reset();

		// create camera body to simulate camera on spring
#define RADIUS 1.0f
		collision = NewtonCreateSphere (world, RADIUS, RADIUS, RADIUS, NULL);
		m_cameraBody = NewtonCreateBody (world, collision);
		//NewtonConvexCollisionCalculateInertialMatrix (collision, &inertia.m_x, &origin.m_x);

#define CAM_WEIGHT 20.0f
		//NewtonBodySetMassMatrix (m_cameraBody, CAM_WEIGHT, inertia.m_x, inertia.m_y, inertia.m_z);
		//NewtonBodySetCentreOfMass (m_cameraBody, &origin.m_x);
		
		NewtonBodySetUserData (m_cameraBody, (void*)this);
		//NewtonBodySetAutoFreeze(m_cameraBody, 0);

		// cam pos in accordance to car
		float initCarPos[3] = {0.0f, 0.0f, -10.0f};
		carCamPos = dVector(initCarPos);

		dMatrix initMat;
		initMat = GetIdentityMatrix();
		initMat.m_posit.m_y = 2000.0f;
		NewtonBodySetMatrix(m_cameraBody, &initMat[0][0]);

		NewtonBodySetForceAndTorqueCallback (m_cameraBody, UpdateCameraCallBack);

		//int state = NewtonBodyGetSleepingState (m_cameraBody);

	}
	catch(...)
	{
		ERRORMSG(ERRGENERIC, "CCar::Init()", "Car initialization failed (Newton)");
	}

	return ERRNOERROR;
}

dVector
CCar::GetCamPos (void)
{
	dMatrix m;
	NewtonBodyGetMatrix (m_cameraBody, &m[0][0]);

	return m.m_posit;
}

void
CCar::UpdateCameraCallBack (const NewtonBody * nBody)
{
	// get car
	CCar *car = (CCar *) NewtonBodyGetUserData (nBody);
	
	// caculate wanted camera position
	dMatrix carPos;
	NewtonBodyGetMatrix (car->NBody(), &carPos[0][0]);

	dVector behind = car->carCamPos;

	carPos.RotateVector (behind);

	dVector wantedPos = carPos.m_posit + behind;

	// now get curr camBody pos
	dMatrix currCam;
	NewtonBodyGetMatrix (nBody, &currCam[0][0]);

	dVector currForce = currCam.m_posit - wantedPos;

	NewtonBodySetForce (nBody, &currForce[0]);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// class reset
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CCar::Reset()
{
	_force = dVector(0.0f, 0.0f, 0.0f);
	_speed = 0.0f;
	//torque = false;

	m_currentBrake = 0.0f;
	handBrake = false;
	currentHandBrake = 0.0f;		
	
	m_currClutchRatio = 1.0f;	// clutch pushed

	engine->Reset();

	nBodyMatrix = GetIdentityMatrix();

	NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);

	NewtonVehicleReset(vehicleJoint);

	// neco s pneumatikama

	//curAngle = 0;

	for(int i=0; i<(int)wheels.size(); i++)
	{
		wheels[i]->Reset();
	}

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// class destruction
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CCar::Release()
{
	SAFE_DELETE(engine);

	if(nBody) 
	{
		NewtonWorld * world = NewtonBodyGetWorld(nBody);

		for(int i=0; i<(int)wheels.size(); i++)
		{
			physics->dynamicObjects[wheels[i]->dynamicObjectsIndex] = NULL;
			SAFE_DELETE(wheels[i]);
		}
		wheels.clear();

		if(vehicleJoint)
		{
			NewtonDestroyJoint(world, vehicleJoint);
			vehicleJoint = NULL;
		}
		
		NewtonDestroyBody(world, nBody); nBody = NULL;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// set car controls
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CCar::Keys(char keys[])
{
	// steering
	Steer(CONTROL_VALUE(keys[0]));

	// acceleration & breaking & shifting
	SetThrottleBrakes (CONTROL_VALUE(keys[1]), CONTROL_VALUE(keys[2]));

	// solve shifting
	// if manual transmition, pressing shift down means shifting lower gear
	// if automatic, it means shifting neutral & explicitLauning disabling
	physics::eGears gear = engine->GetGear();
	switch (gearBoxType)
	{
	case GEARBOX_AUTOMATIC:
		if(keys[3] & (char) KEY_UPSHIFT)
		{
			// only from neutral
			if (gear == physics::GEAR_NEUTRAL)
			{
				engine->ShiftUp ();
				engine->clutchState = physics::CLUTCH_UNPUSHING;
				engine->m_burningOut = BURN_AUTO;
			}
		}
		if(keys[3] & (char) KEY_DOWNSHIFT)
		{
			// only from NON neutral
			if (gear != physics::GEAR_NEUTRAL)
			{
				engine->ShiftGear (physics::GEAR_NEUTRAL);
				engine->m_burningOut = physics::BURN_MANUAL;
			}
		}
		break;

	case GEARBOX_MANUAL:
		// shift up
		if(keys[3] & (char) KEY_UPSHIFT)
		{
			switch (gear)
			{
			case physics::GEAR_NEUTRAL:
				engine->clutchState = physics::CLUTCH_UNPUSHING;
				engine->ShiftUp();
				break;

			case physics::GEAR_6:
				// nic.. CHaaaa nahoru to nejde
				break;

			default:
				engine->ShiftUp();
				break;
			}
		}
		// shift down
		if(keys[3] & (char) KEY_DOWNSHIFT)
		{
			if (gear != physics::GEAR_BACKWRD)
				engine->ShiftDown();
		}
		break;
	};

	// handbrake
	if(keys[3] & (char) KEY_HANDBRAKE)
	{
		// handbreaked wheels are 2 last (rear)
		size_t wheelSize = wheels.size();
		wheels[wheelSize-1]->Brakes (HANDBREAK_VALUE);
		wheels[wheelSize-2]->Brakes (HANDBREAK_VALUE);
	}

	// restart
	if(keys[3] & (char) KEY_RESTART)
	{
		RestartCar();
	}
/*
	// shift up
	if(keys[3] & (char) KEY_UPSHIFT)
	{
		for(UINT i=0; i<4; i++)
			wheels[i]->IncParameter();

	}
	// shift down
	if(keys[3] & (char) KEY_DOWNSHIFT)
	{
		for(UINT i=0; i<4; i++)
			wheels[i]->DecParameter();
	}
*/
};

void
CCar::SetThrottleBrakes(float accel, float deccel)
{
	physics::eGears gear = engine->GetGear();

	switch (gear)
	{
	case physics::GEAR_BACKWRD:
		// use throtle as break and vice versa
		Brakes(accel);
		Throttle(deccel);
		break;

	case physics::GEAR_NEUTRAL:
		if( !(accel && deccel) )	// na neutral se zarovne nepridava a nebrzdi
		{			
			if (accel)
			{
				Throttle(accel);
				m_dirOfMovement = 1;
			}
			else if (deccel)
			{
				Throttle(deccel);
				m_dirOfMovement = 0;
			}
			else
			{
				Throttle(0.0f);	// zadnej plyn
				Brakes(0.0f);	// zadna brzda
			}
		}
		break;

	default: // forward moving
		// throttle
		Throttle(accel);
		// brakes
		Brakes(deccel);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CCar::Airborne()
{
	bool result = true;
	for(UINT i=0; i<wheels.size(); i++)
		result = result && (NewtonVehicleTireIsAirBorne(vehicleJoint, wheels[i]->WheelID()) ? true : false);
	return result;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CCar::RestartCar()
{
	dVector position = nBodyMatrix.m_posit;
	dVector heading = nBodyMatrix.m_right;

	RestartCar(nBodyMatrix.m_posit.m_x, nBodyMatrix.m_posit.m_z, heading);
#if 0
	//Reset();

	//heading.m_y = 0.0f;
	//nBodyMatrix = dgGrammSchmidt(dVector(heading.m_z, 0.0f, -heading.m_x));
	//nBodyMatrix.m_up = dVector(0.0f, 1.0f, 0.0f);

	//bool found = false;
	//bool result;
	//dVector _position;
	//dVector diff;
	//float distance;

	//while(!found)
	//{
	//	position = dVector(position.m_x, physics->GetHeight(position.m_x, position.m_z) + 2.0f, position.m_z, 1.0f);
	//	IsInWorldBoundingBox(position);

	//	result = true;
	//	for(UINT i=0; i<MAX_PLAYERS; i++)
	//	{
	//		if(physics->cars[i])
	//		{
	//			_position = physics->cars[i]->Matrix().m_posit;
	//			_position = _position.Scale(0.01f);
	//			_position.m_w = 1.0f;

	//			diff = position - _position;
	//			distance = sqrt(diff.m_x*diff.m_x+diff.m_y*diff.m_y+diff.m_z*diff.m_z);
	//			if(distance < (physics->cars[i]->BBoxDiameter() + bboxDiameter))
	//				result = false;				
	//		}
	//	}
	//	for(UINT i=0; i<physics->staticObjects.size(); i++)
	//	{
	//		_position = physics->staticObjects[i]->Matrix().m_posit;
	//		_position = _position.Scale(0.01f);
	//		_position.m_w = 1.0f;

	//		diff = position - _position;
	//		distance = sqrt(diff.m_x*diff.m_x+diff.m_y*diff.m_y+diff.m_z*diff.m_z);
	//		if(distance < (physics->staticObjects[i]->BBoxDiameter() + bboxDiameter))
	//			result = false;
	//	}
	//	for(UINT i=0; i<physics->dynamicObjects.size(); i++)
	//	{
	//		_position = physics->dynamicObjects[i]->Matrix().m_posit;
	//		_position = _position.Scale(0.01f);
	//		_position.m_w = 1.0f;

	//		diff = position - _position;
	//		distance = sqrt(diff.m_x*diff.m_x+diff.m_y*diff.m_y+diff.m_z*diff.m_z);
	//		if(distance < (physics->dynamicObjects[i]->BBoxDiameter() + bboxDiameter))
	//			result = false;
	//	}
	//	if(result)
	//		found = true;
	//	else
	//	{
	//		int x = rand() % 2;
	//		int y = rand() % 2;
	//		position = position + dVector(float(-5 + x*10), 0.0f, float(-5 + y*10));
	//		IsInWorldBoundingBox(position);
	//	}
	//}


	//dVector cog;
	//NewtonBodyGetCentreOfMass(nBody, &cog.m_x);

	//dVector worldSize = physics->GetWorldSize();

	//dVector worldCenter = worldSize.Scale(0.5f);
	//worldCenter.m_y = 0.0f;
	//dVector object_center = dVector(worldCenter-position);
	//float length = sqrt(object_center.m_x*object_center.m_x+object_center.m_y*object_center.m_y+object_center.m_z*object_center.m_z);
	//object_center = object_center.Scale(5.0f/length);
	//position += object_center;
	//position = worldCenter;

	//nBodyMatrix.m_posit = dVector(position.m_x+cog.m_x, physics->GetHeight(position.m_x, position.m_z)+cog.m_y+2.0f, position.m_z+cog.m_z, 1.0f);

	//nBodyMatrix.m_posit = dVector(position.m_x, physics->GetHeight(position.m_x, position.m_z)+2.0f, position.m_z, 1.0f);
	//
	//if(nBodyMatrix.m_posit.m_x-bboxDiameter < 0.0f)
	//	nBodyMatrix.m_posit.m_x += bboxDiameter;
	//if(nBodyMatrix.m_posit.m_x+bboxDiameter > worldSize.m_x)
	//	nBodyMatrix.m_posit.m_x -= bboxDiameter;
	//if(nBodyMatrix.m_posit.m_y-bboxDiameter < 0.0f)
	//	nBodyMatrix.m_posit.m_y += bboxDiameter;
	//if(nBodyMatrix.m_posit.m_y+bboxDiameter > worldSize.m_y)
	//	nBodyMatrix.m_posit.m_y -= bboxDiameter;
	//if(nBodyMatrix.m_posit.m_z-bboxDiameter < 0.0f)
	//	nBodyMatrix.m_posit.m_z += bboxDiameter;
	//if(nBodyMatrix.m_posit.m_z+bboxDiameter > worldSize.m_z)
	//	nBodyMatrix.m_posit.m_z -= bboxDiameter;

	//nBodyMatrix.m_posit = position;
	//NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);

	//dVector reset (0.0f, 0.0f, 0.0f);
	//NewtonBodySetVelocity(nBody, &reset.m_x);
	//NewtonBodySetOmega(nBody, &reset.m_x);
#endif
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// car restart, x,z - position in meters, heading - front vector of a car
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CCar::RestartCar(float x, float z, dVector & heading)//, UINT type)
{
	dVector position (x, 0.0f, z);

	Reset();
		
	heading.m_y = 0.0f;
	nBodyMatrix = dgGrammSchmidt(dVector(heading.m_z, 0.0f, -heading.m_x));
	nBodyMatrix.m_up = dVector(0.0f, 1.0f, 0.0f);


	bool found = false;
	bool result;
	dVector _position;
	dVector diff;
	float distance;

	int cyclesCount = 0;
	while(!found)
	{
		position = dVector(position.m_x, physics->GetHeight(position.m_x, position.m_z) + 2.0f, position.m_z, 1.0f);
		IsInWorldBoundingBox(position);

		result = true;
		for(UINT i=0; i<MAX_PLAYERS; i++)
		{
			if(physics->cars[i])
			{
				_position = nBodyMatrix.m_posit;
				_position = _position.Scale(0.01f);
				_position.m_w = 1.0f;

				diff = position - _position;
				distance = sqrt(diff.m_x*diff.m_x+diff.m_y*diff.m_y+diff.m_z*diff.m_z);
				if(distance < (physics->cars[i]->BBoxDiameter() + bboxDiameter))
					result = false;				
			}
		}

		for(UINT i=0; i<physics->staticObjects.size(); i++)
		{
			_position = physics->staticObjects[i]->Matrix().m_posit;
			_position = _position.Scale(0.01f);
			_position.m_w = 1.0f;

			diff = position - _position;
			distance = sqrt(diff.m_x*diff.m_x+diff.m_y*diff.m_y+diff.m_z*diff.m_z);
			if(distance < (physics->staticObjects[i]->BBoxDiameter() + bboxDiameter))
				result = false;
		}
		for(UINT i=0; i<physics->dynamicObjects.size(); i++)
		{
			if(physics->dynamicObjects[i]->NBody())
			{
				_position = physics->dynamicObjects[i]->Matrix().m_posit;
				_position = _position.Scale(0.01f);
				_position.m_w = 1.0f;

				diff = position - _position;
				distance = sqrt(diff.m_x*diff.m_x+diff.m_y*diff.m_y+diff.m_z*diff.m_z);
				if(distance < (physics->dynamicObjects[i]->BBoxDiameter() + bboxDiameter))
					result = false;
			}
		}

		if(result)
			found = true;
		else
		{
			int _x = rand() % 2;
			int _y = rand() % 2;
			position = position + dVector(float(-5 + _x*10), 0.0f, float(-5 + _y*10));
			IsInWorldBoundingBox(position);
		}
		cyclesCount++;
		if(cyclesCount > 100)
		{
			OUTMSG("Restart car failure.",1);
			break;
		}
	}

	//CAtlString msg;
	//msg.Format("CCar::RestartCar() - cyclesCount: %d", cyclesCount);
	//OUTMSG(msg, 1);
	
	//dVector cog;
	//NewtonBodyGetCentreOfMass(nBody, &cog.m_x);

	//dVector worldSize = physics->GetWorldSize();

	//dVector worldCenter = worldSize.Scale(0.5f);
	//worldCenter.m_y = 0.0f;
	//dVector object_center = dVector(worldCenter-position);
	//float length = sqrt(object_center.m_x*object_center.m_x+object_center.m_y*object_center.m_y+object_center.m_z*object_center.m_z);
	//object_center = object_center.Scale(5.0f/length);
	//position += object_center;

	//nBodyMatrix.m_posit = dVector(x+cog.m_x, physics->GetHeight(x, z)+cog.m_y+2.0f, z+cog.m_z, 1.0f);

	//nBodyMatrix.m_posit = dVector(position.m_x, physics->GetHeight(position.m_x, position.m_z)+2.0f, position.m_z, 1.0f);

	//if(nBodyMatrix.m_posit.m_x-bboxDiameter < 0.0f)
	//	nBodyMatrix.m_posit.m_x += bboxDiameter;
	//if(nBodyMatrix.m_posit.m_x+bboxDiameter > worldSize.m_x)
	//	nBodyMatrix.m_posit.m_x -= bboxDiameter;
	//if(nBodyMatrix.m_posit.m_y-bboxDiameter < 0.0f)
	//	nBodyMatrix.m_posit.m_y += bboxDiameter;
	//if(nBodyMatrix.m_posit.m_y+bboxDiameter > worldSize.m_y)
	//	nBodyMatrix.m_posit.m_y -= bboxDiameter;
	//if(nBodyMatrix.m_posit.m_z-bboxDiameter < 0.0f)
	//	nBodyMatrix.m_posit.m_z += bboxDiameter;
	//if(nBodyMatrix.m_posit.m_z+bboxDiameter > worldSize.m_z)
	//	nBodyMatrix.m_posit.m_z -= bboxDiameter;
	
	nBodyMatrix.m_posit = position;
	NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);

	dVector reset (0.0f, 0.0f, 0.0f);
	NewtonBodySetVelocity(nBody, &reset.m_x);
	NewtonBodySetOmega(nBody, &reset.m_x);	
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CCar::RestartCar(float x, float y, float z, dVector & heading)
{
	dVector position (x, y, z);

	Reset();
		
	heading.m_y = 0.0f;
	nBodyMatrix = dgGrammSchmidt(dVector(heading.m_z, 0.0f, -heading.m_x));

	//CAtlString _msg;
	//_msg.Format("HEADING [%f %f %f]", heading.m_x, heading.m_y, heading.m_z);
	//OUTMSG(_msg, 1);
	//_msg.Format("RESTART [%0.2f %0.2f %0.2f %0.2f|%0.2f %0.2f %0.2f %0.2f|%0.2f %0.2f %0.2f %0.2f|%0.2f %0.2f %0.2f %0.2f]",
	//	nBodyMatrix.m_front.m_x,nBodyMatrix.m_front.m_y,nBodyMatrix.m_front.m_z,nBodyMatrix.m_front.m_w,
	//	nBodyMatrix.m_right.m_x,nBodyMatrix.m_right.m_y,nBodyMatrix.m_right.m_z,nBodyMatrix.m_right.m_w,
	//	nBodyMatrix.m_up.m_x,nBodyMatrix.m_up.m_y,nBodyMatrix.m_up.m_z,nBodyMatrix.m_up.m_w,
	//	nBodyMatrix.m_posit.m_x,nBodyMatrix.m_posit.m_y,nBodyMatrix.m_posit.m_z,nBodyMatrix.m_posit.m_w);
	//OUTMSG(_msg, 1);

	nBodyMatrix.m_up = dVector(0.0f, 1.0f, 0.0f);

	bool found = false;
	bool result;
	dVector _position;
	dVector diff;
	float distance;

	int cyclesCount = 0;
	while(!found)
	{
		position = dVector(position.m_x, position.m_y/*physics->GetHeight(position.m_x, position.m_z)*/ + 2.0f, position.m_z, 1.0f);
		IsInWorldBoundingBox(position);

		result = true;
		for(UINT i=0; i<MAX_PLAYERS; i++)
		{
			if(physics->cars[i])
			{
				_position = nBodyMatrix.m_posit;
				_position = _position.Scale(0.01f);
				_position.m_w = 1.0f;

				diff = position - _position;
				distance = sqrt(diff.m_x*diff.m_x+diff.m_y*diff.m_y+diff.m_z*diff.m_z);
				if(distance < (physics->cars[i]->BBoxDiameter() + bboxDiameter))
					result = false;				
			}
		}

		//for(UINT i=0; i<physics->staticObjects.size(); i++)
		//{
		//	_position = physics->staticObjects[i]->Matrix().m_posit;
		//	_position = _position.Scale(0.01f);
		//	_position.m_w = 1.0f;

		//	diff = position - _position;
		//	distance = sqrt(diff.m_x*diff.m_x+diff.m_y*diff.m_y+diff.m_z*diff.m_z);
		//	if(distance < (physics->staticObjects[i]->BBoxDiameter() + bboxDiameter))
		//		result = false;
		//}
		//for(UINT i=0; i<physics->dynamicObjects.size(); i++)
		//{
		//	if(physics->dynamicObjects[i]->NBody())
		//	{
		//		_position = physics->dynamicObjects[i]->Matrix().m_posit;
		//		_position = _position.Scale(0.01f);
		//		_position.m_w = 1.0f;

		//		diff = position - _position;
		//		distance = sqrt(diff.m_x*diff.m_x+diff.m_y*diff.m_y+diff.m_z*diff.m_z);
		//		if(distance < (physics->dynamicObjects[i]->BBoxDiameter() + bboxDiameter))
		//			result = false;
		//	}
		//}

		if(result)
			found = true;
		else
		{
			//int _x = rand() % 2;
			int _y = rand() % 2;
			position = position + dVector(0.0f, 5.0f, 0.0f);//dVector(float(-5 + _x*10), 0.0f, float(-5 + _y*10));
			IsInWorldBoundingBox(position);
		}
		cyclesCount++;
		if(cyclesCount > 100)
		{
			OUTMSG("Restart car failure.",1);
			break;
		}
	}
///////////////////////////////

	nBodyMatrix.m_posit = position;
	NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);

	dVector reset (0.0f, 0.0f, 0.0f);
	NewtonBodySetVelocity(nBody, &reset.m_x);
	NewtonBodySetOmega(nBody, &reset.m_x);	
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CCar::ApplyForceAndTorque_Callback()
{
	float mass;
	float Ixx;
	float Iyy;
	float Izz;

//	dVector omega;
//#define OMEGA_MAX_VALUE 25.0f
//	NewtonBodyGetOmega(nBody, &omega.m_x);
//	if(omega.m_x > OMEGA_MAX_VALUE)
//		omega.m_x = OMEGA_MAX_VALUE;
//	if(omega.m_x < -OMEGA_MAX_VALUE)
//		omega.m_x = -OMEGA_MAX_VALUE;
//	if(omega.m_y > OMEGA_MAX_VALUE)
//		omega.m_y = OMEGA_MAX_VALUE;
//	if(omega.m_y < -OMEGA_MAX_VALUE)
//		omega.m_y = -OMEGA_MAX_VALUE;
//	if(omega.m_z > OMEGA_MAX_VALUE)
//		omega.m_z = OMEGA_MAX_VALUE;
//	if(omega.m_z < -OMEGA_MAX_VALUE)
//		omega.m_z = -OMEGA_MAX_VALUE;
//	NewtonBodySetOmega(nBody, &omega.m_x);

	// get velocity
	NewtonBodyGetVelocity(nBody, &velocity.m_x);
	velocity.m_w = 1.0f;

	D3DXVECTOR4 v = D3DXVECTOR4(velocity.m_x, velocity.m_y, velocity.m_z, 1.0f);
	_speed = D3DXVec4Length (&v);

	//float spd = abs(_lat_speed)/10.0f > 2.0f ? 2.0f : abs(_lat_speed)/10.0f;

	//dVector f_force(0.0f, mass * GRAVITY * spd, 0.0f);


	NewtonBodyGetMassMatrix(nBody, &mass, &Ixx, &Iyy, &Izz);

	float coef = Airborne() ? 1.4f : 1.0f;

	//mass *= (1.0f + _speed / 20.0f);
	dVector g_force(0.0f, coef * mass * (-m_materials->GetCurrentMutator()->gravity), 0.0f);

	dVector f_force(0.0f, mass * (-m_materials->GetCurrentMutator()->gravity) * (_speed *0.01f), 0.0f);
	f_force =  nBodyMatrix.TransformVector(f_force);

	g_force = g_force + f_force;
	
	NewtonBodySetForce(nBody, &g_force.m_x);
	float f = 0.5f * 0.33f * frontalArea * AIR_DENSITY * _speed;//* _speed;

	velocity.Scale(f);

	NewtonBodyAddForce(nBody, &velocity[0]);

	dVector f1;
	if(_speed < 8.0f)
		f1 = dVector(velocity.Scale(-100.0f));
	else
		f1 = dVector(velocity.Scale(-50.0f));

	NewtonBodyAddForce(nBody, &f1.m_x);

	dMatrix wheel_matrix;

	//update gearBox if no clutch action taken
	switch (engine->clutchState)
	{
	case CLUTCH_PUSHING:
		m_currClutchRatio += CLUTCH_QUANTUM;
		if (m_currClutchRatio >= 1.0)
		{
			// stop pushing
			m_currClutchRatio = 1.0;
			engine->clutchState = CLUTCH_NOACTION;
			engine->ShiftGear (GEAR_NEUTRAL);
		}
		break;

	case CLUTCH_UNPUSHING:
		m_currClutchRatio -= CLUTCH_QUANTUM;
		if (m_currClutchRatio <= 0.0)
		{
			// stop pushing
			m_currClutchRatio = 0.0;
			engine->clutchState = CLUTCH_NOACTION;
		}
		break;

	case CLUTCH_NOACTION:
		this->UpdateGearBox();
		break;
	}

	// update engine
	engine->Update (m_wheelRpm, m_currClutchRatio);

	float totalRPM = 0.0f;
	//update powered wheels values
	for(UINT w=0; w<poweredWheels.size(); w++)
	{
		// add wheel's RPM to total
		totalRPM += wheels[w]->GetCurrRPM ();

		// set torque
		wheels[w]->Torque (engine->GetCurrTorque());
	}
	
	// make mean of totalRPM
	m_wheelRpm = totalRPM / (float)poweredWheels.size();

	// make absolut value
	if (m_wheelRpm < 0)
		m_wheelRpm *= -1;

	//#define VEC_MAGN	6.0f
	//		dVector f1;
	//		if(__speed == 0.0f)
	//			f1 = dVector(0.0f, 0.0f, 0.0f);
	//		else
	//			f1 = dVector(VEC_MAGN*velocity.m_x/_speed, VEC_MAGN*velocity.m_y/_speed, VEC_MAGN*velocity.m_z/_speed);



		//CAtlString msg;
		//msg.Format("Air resistance[%10.2f]", f*_speed);
		//OUTMSG(msg,1);
	//}

	//dMatrix matrix;
	//NewtonBodyGetMatrix(nBody, &matrix[0][0]);
	

	//NewtonBodyAddForce(nBody, &force.m_x);
	//dVector force;
	//NewtonBodyGetForce(nBody, &force.m_x);

	// engine update

	//	wheel_matrix = cars[0]->wheels[i]->_Matrix();
	//	m[i] = terrain.MaterialsMapf(CENTIMETER_2_METER(wheel_matrix.m_posit.m_x), CENTIMETER_2_METER(wheel_matrix.m_posit.m_z));
	//	cars[0]->wheels[i]->SetMaterial(m[i]);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CCar::SetTransform_Callback()
{
	CRigidBody::SetTransform_Callback();

	dVector com;
	NewtonBodyGetCentreOfMass(nBody, &com.m_x);

	for(int i=0; i<(int)wheels.size(); i++)
	{
		wheels[i]->SetTransform();
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CCar::WheelUpdateCallbackStatic(const NewtonJoint * vehicleJoint)
{
	// go in wheel container and call SetWheelPhisics for every wheel
    CWheel * wheel;
    for(void * wheelID = NewtonVehicleGetFirstTireID(vehicleJoint); wheelID; wheelID = NewtonVehicleGetNextTireID(vehicleJoint, wheelID))
    {
        wheel = (CWheel *) ((UserData *) NewtonVehicleGetTireUserData(vehicleJoint, wheelID))->data;
		wheel->SetWheelPhysics();

		//sg[i] = NewtonVehicleTireLostSideGrip(vehicleJoint, wheelID);
		//t[i] = NewtonVehicleTireLostTraction(vehicleJoint, wheelID);
		//a[i] = NewtonVehicleTireIsAirBorne(vehicleJoint, wheelID);
		//i++;
    }
	//int sg[4];
	//int t[4];
	//int a[4];
	//void * wheelid;
	//NewtonJoint * vehicle = this->VehicleJoint();
	//for(int i=0; i<4; i++)
	//{
	//	wheelid = this->wheels[i]->WheelID();
	//	sg[i] = NewtonVehicleTireLostSideGrip(vehicle, wheelid);
	//	t[i] = NewtonVehicleTireLostTraction(vehicle, wheelid);
	//	a[i] = NewtonVehicleTireIsAirBorne(vehicle, wheelid);
	//}

	//CAtlString msg;
	//msg.Format("T[%3d %3d %3d %3d] SG[%3d %3d %3d %3d] A[%3d %3d %3d %3d]", t[0], t[1], t[2], t[3], sg[0], sg[1], sg[2], sg[3], a[0], a[1], a[2], a[3]);
	//OUTMSG(msg, 1);
};

void CCar::Steer(float value)
{
	//float _scale = 1.0f - _speed / 30.0f;
	//float scale = _scale > 0.1f ? _scale : 0.1f;

	//if(_speed > 5.0f)
	//	scale = 0.9f;
	//if(_speed > 10.0f)
	//	scale = 0.6f;
	//if(_speed > 15.0f)
	//	scale = 0.5f;
	//if(_speed > 20.0f)
	//	scale = 0.4f;
	//if(_speed > 25.0f)
	//	scale = 0.3f;
#define MIN_SPEED_FOR_SCALING (20.0f / 3.6f)	// 20 km/h
#define SPEED_SCALING_COEF 4.0f
	float scale = SPEED_SCALING_COEF / (_speed+MIN_SPEED_FOR_SCALING);

	float value0 = 0.0f, value1 = 0.0f;

	value = value*DEG2RAD(MAX_ANGLE)*scale;
/*
	if(value != 0.0f)
	{
		

		if(value > 0.0f)
		{
			value1 = value;
			value0 = atan(car_w_length/((car_w_length / tan(value))+car_w_width));
				//atan(CAR_LENGTH/((CAR_LENGTH / tan(angles[0][i]))+CAR_WIDTH));
		}
		else
		{
			value0 = value;
			value1 = atan(car_w_length/((car_w_length / tan(value))+car_w_width));
		}
		
	}
	else
		value = 0.0f;

		*/

	wheels[0]->Steer(value);
	wheels[1]->Steer(value);

	wheels[0]->Speed(_speed);
	wheels[1]->Speed(_speed);
};

void CCar::Throttle(float value)
{

	#define THROTTLE_INCREASING_COEF 0.2f	// jak rychle se macka plyn
	#define THROTTLE_DECREASING_COEF 0.1f	// jak rychle se vraci plyn

	float currThrottle = engine->GetThrottle();

	if (value >= 0.5)	// pushing throttle pedal
	{
		currThrottle += THROTTLE_INCREASING_COEF;
	}
	else				// NO pushing
	{
		currThrottle -= THROTTLE_DECREASING_COEF;
	}

	// clamp between 0-1
	FUNGUJICI_CLAMP_BETWEEN(currThrottle, 0.0f, 1.0f);
	
	// set engine
	engine->SetThrottle(currThrottle);
};

void CCar::Brakes(float value)
{
	#define BREAK_INCREASING_COEF 0.2f	// jak rychle se macka brzda
	#define BREAK_DECREASING_COEF 0.1f	// jak rychle se vraci brzda

	if (value >= 0.5)	// pushing break pedal
	{
		m_currentBrake += BREAK_INCREASING_COEF;
	}
	else				// NO pushing
	{
		m_currentBrake -= BREAK_DECREASING_COEF;
	}

	// clamp between 0-1
	FUNGUJICI_CLAMP_BETWEEN(m_currentBrake, 0.0f, 1.0f);

	// set vals in all wheels
	for (UINT i=0; i < wheels.size (); i++)
			wheels[i]->Brakes (m_currentBrake);
};

void CCar::HandBrake(float value)
{
	//if(value != 0.0f)


	wheels[2]->Brakes(value);
	wheels[2]->parameter = value;
	wheels[3]->Brakes(value);
	wheels[3]->parameter = value;
};

void CCar::FindMasses(MassPos * points, UINT min, UINT max, MassPos center)
{
	UINT i=min;
	UINT j;
	MassPos center1, center2;
	center1.pos = center2.pos = 0.0f;
	while(points[i].pos > center.pos) i++;

	for(j=min; j<i; j++) center1.pos += points[j].pos;
	center1.pos /= (i-min);

	for(j=i; j<max+1; j++) center2.pos += points[j].pos;
	center2.pos /= (max-i+1);

	center1.mass = ((center.pos-center2.pos)/((center1.pos-center.pos)+(center.pos-center2.pos)))*center.mass;
	center2.mass = ((center1.pos-center.pos)/((center1.pos-center.pos)+(center.pos-center2.pos)))*center.mass;

	if(i-min == 1)
		points[min].mass = center1.mass;
	else
		FindMasses(points, min, i-1, center1);
	if(max-i+1 == 1)
		points[i].mass = center2.mass;
	else
		FindMasses(points, i, max, center2);
};

///////////////////////////////////////////
// does UpdateGearBox
///////////////////////////////////////////
void CCar::UpdateGearBox( void)
{
	physics::eGears gear = engine->GetGear();
	switch (gear)
	{
	case GEAR_BACKWRD:
		// always automaticaly laung 1st gear after going backward
		engine->m_burningOut = BURN_AUTO;
		break;

	case GEAR_NEUTRAL:
		switch (m_dirOfMovement)
		{
		case 1:		// forward
			switch (engine->m_burningOut)
			{
			case physics::BURN_AUTO:				
				// wait for right rpms & shift up
				if (engine->m_engineRpm > engine->m_lowRPM)
				{
					engine->ShiftUp();
					engine->clutchState = CLUTCH_UNPUSHING;
				}
				break;

			case BURN_MANUAL:	// no automatic launching of 1st gear
				// do nothing, manualy shift to 1st
				break;
			}
			break;

		case 0:	// backward
			if (engine->m_engineRpm > engine->m_lowRPM)
			{
				engine->ShiftDown();
				engine->clutchState = CLUTCH_UNPUSHING;
			}
			break;
		}
		break;

	case physics::GEAR_1:	// from 1st gear
		// accelering
		if( (gearBoxType == GEARBOX_AUTOMATIC) && (engine->m_engineRpm >= engine->m_hiRPM) )
				engine->ShiftUp();
		break;

	default: // forward
		// if automatic gearbox
		if (gearBoxType == GEARBOX_AUTOMATIC)
		{
			switch(gear)
			{
			case physics::GEAR_6:
				// on this gear it can onlu shift down
				if(engine->m_engineRpm <= engine->m_lowRPM)
					engine->ShiftDown();
				break;
			
			default:
				if(engine->m_engineRpm >= engine->m_hiRPM)
					engine->ShiftUp();
				if(engine->m_engineRpm <= engine->m_lowRPM)
					engine->ShiftDown();
				break;
			}
		}
	}
}

void CCar::IsInWorldBoundingBox(dVector & position)
{
	dVector worldSize = physics->GetWorldSize();

	if(position.m_x-bboxDiameter < 0.0f)
		position.m_x += bboxDiameter;
	if(position.m_x+bboxDiameter > worldSize.m_x)
		position.m_x -= bboxDiameter;
	if(position.m_y-bboxDiameter < 0.0f)
		position.m_y += bboxDiameter;
	if(position.m_y+bboxDiameter > worldSize.m_y)
		position.m_y -= bboxDiameter;
	if(position.m_z-bboxDiameter < 0.0f)
		position.m_z += bboxDiameter;
	if(position.m_z+bboxDiameter > worldSize.m_z)
		position.m_z -= bboxDiameter;
};

#if 0
//void CCar::Steering(float value)
//{
//	float _scale = 1.0f - _speed / 30.0f; // 60.0f
//	float scale = _scale > 0.1f ? _scale : 0.1f;
//	if(value > 0.0f)
//		value = (MAX_ANGLE*D3DX_PI/180.0f) * scale;
//	else if(value < 0.0f)
//		value = -(MAX_ANGLE*D3DX_PI/180.0f) * scale;
//	else
//		value = 0.0f;
//
//	wheels[0]->Steer(value);
//	wheels[1]->Steer(value);
//
//	wheels[0]->Speed(_speed);
//	wheels[1]->Speed(_speed);
//	//steering
//	//if(_speed > 0)
//	//	_maxWheelAngle = 9;
//	//if(_speed > 9)
//	//	_maxWheelAngle = 6;	
//	//if(_speed > 25)
//	//	_maxWheelAngle = 3;
//
//	//if(value > 0)
//	//{
//	//	if(curAngle < _maxWheelAngle-1)
//	//		curAngle++;
//	//}
//	//else if(value < 0)
//	//{
//	//	if(curAngle > -_maxWheelAngle+1)
//	//	curAngle--;
//	//}
//	//else
//	//{
//	//	if(curAngle > 0)
//	//		curAngle--;
//	//	else if(curAngle < 0)
//	//		curAngle++;
//	//}
//	//if(curAngle >= 0)
//	//{
//	//	wheels[0]->Steer(angles[1][curAngle]);
//	//	wheels[1]->Steer(angles[0][curAngle]);
//	//}
//	//else
//	//{
//	//	wheels[0]->Steer(-angles[0][-curAngle]);
//	//	wheels[1]->Steer(-angles[1][-curAngle]);
//	//}
//
//};
//
//void CCar::WheelTorque(float value)
//{
//#define MAX_TORQUE 2000.0f
//	if(value>0.0f)
//		value = -MAX_TORQUE * (1.0f - _speed/60.0f);
//	else if(value<0.0f)
//		value = MAX_TORQUE*0.5f * (1.0f - _speed/60.0f);
//	else
//		value = 0.0f;
//
//	
//	float _value = currentTorque + (value - currentTorque) * 0.5f;
//	//CAtlString msg;
//	//msg.Format("%10.2f", value);
//	//OUTMSG(msg, 1);
//
//	wheels[0]->Torque(_value);
//	wheels[1]->Torque(_value);
//};
//
//void CCar::WheelBrake(float value)
//{
//	wheels[0]->Brakes(value);
//	wheels[1]->Brakes(value);
//};
#endif

#if 0

	//if(keys[0] > 0)
	//	Steering(1.0f);
	//else if(keys[0] < 0)
	//	Steering(-1.0f);
	//else
	//	Steering(0.0f);
	//if(keys[1])
	//	WheelTorque(1.0f);
	//else if(keys[2])
	//	WheelTorque(-1.0f);
	//else
	//	WheelTorque(0.0f);
	//
	////for(int i=0; i<6; i++)
	////this->keys[i] = keys[i];
	//
	////force = dVector(0.0f, 0.0f, 0.0f);

	//// steering
	//if(_speed > 0)
	//	_maxWheelAngle = 9;
	//if(_speed > 9)
	//	_maxWheelAngle = 6;	
	//if(_speed > 25)
	//	_maxWheelAngle = 3;

	//if(keys[0] > 0)
	//{
	//	if(curAngle < _maxWheelAngle-1)
	//		curAngle++;
	//}
	//else if(keys[0] < 0)
	//{
	//	if(curAngle > -_maxWheelAngle+1)
	//	curAngle--;
	//}
	//else
	//{
	//	if(curAngle > 0)
	//		curAngle--;
	//	else if(curAngle < 0)
	//		curAngle++;
	//}
	//if(curAngle >= 0)
	//{
	//	wheels[0]->Steer(angles[1][curAngle]);
	//	wheels[1]->Steer(angles[0][curAngle]);
	//}
	//else
	//{
	//	wheels[0]->Steer(-angles[0][-curAngle]);
	//	wheels[1]->Steer(-angles[1][-curAngle]);
	//}
	//
	////if(keys[0] > 0)
	////{
	////	wheels[0]->Steer(0.5f);
	////	wheels[1]->Steer(0.5f);
	////}
	////else if(keys[0] < 0)
	////{
	////	wheels[0]->Steer(-0.5f);
	////	wheels[1]->Steer(-0.5f);		
	////}
	////else
	////{
	////	wheels[0]->Steer(0.0f);
	////	wheels[1]->Steer(0.0f);
	////}

	//// acceleration & brake
	////if(gearBoxType)
	////{
	////	// manual
	////}
	////else
	////{
	////	// automatic
	////	if(engine->Gear() > 1)
	////	{
	////		engine->Throttle(keys[1]);
	////		// brakes keys[2]
	////	}
	////	if(engine->Gear() == 0)
	////	{
	////		engine->Throttle(-keys[2]);
	////		// brakes keys[1]
	////	}
	////	if(engine->Gear() == 1)
	////	{
	////		if(keys[1])
	////			engine->Throttle(keys[1]);
	////		else
	////			engine->Throttle(-keys[2]);
	////	}
	////}

	////if(keys[1])
	////{
	////	for(int i=0; i<(int)wheels.size(); i++)
	////		if(wheels[i]->Powered())
	////			wheels[i]->Torque(-2000.0f);
	////}
	////else if(keys[2])
	////{
	////	for(int i=0; i<(int)wheels.size(); i++)
	////		if(wheels[i]->Powered())
	////			wheels[i]->Torque(1500.0f);
	////}
	////else
	////{
	////	for(int i=0; i<(int)wheels.size(); i++)
	////		if(wheels[i]->Powered())
	////			wheels[i]->Torque(0.0f);
	////}

	//if(keys[3] & (char) KEY_UPSHIFT)
	//{
	//	// shift up
	//}
	//if(keys[3] & (char) KEY_DOWNSHIFT)
	//{
	//	// shift down
	//}
	//if(keys[3] & (char) KEY_BRAKE)
	//{
	//	// handbrake
	//	wheels[0]->Brakes(1.0f);
	//	wheels[1]->Brakes(1.0f);
	//}
	//else
	//{
	//	wheels[0]->Brakes(0.0f);
	//	wheels[1]->Brakes(0.0f);
	//}


	//// handbrake

	//// gear up & down
#endif