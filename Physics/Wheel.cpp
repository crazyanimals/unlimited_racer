

#include "Resources.h" // for resources structures


#include "Wheel.h"
#include "Car.h"

using namespace physics;

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CWheel::~CWheel()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// CWheel initialization
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CWheel::Init(
					 NewtonWorld * world,
					 float carMass, 
					 CResources * resources,
					 InitWheel * initWheel, 
					 CMaterials *mats, UINT i)
{
	parameter = 0.0f;
	currentSurfaceMaterialCoefs = & mats->GetCurrentMutator()->materialCoefs[0];

	nBody = NULL;
	wheelID = NULL;
	alwaysActive = true;
	changed = false;
	powered = initWheel->powered;
	scaling = dVector(initWheel->scale);
	this->m_materials = mats;
	vehicleJoint = ((CCar *)initWheel->car)->VehicleJoint();

	m_effic = mats->GetCurrentMutator()->breaksEfficiecy;

	t_torque = 0.0f;
	t_brakes = 0.0f;
	t_steer = 0.0f;

	t_speed = -431602080.0f;

	currentBrake = 0.0f;

	//dMatrix matrix (GetIdentityMatrix());

	MESHID meshID = resources->LoadDynamicMesh(initWheel->model, &scaling, false);
	if(meshID == -1) ERRORMSG(ERRGENERIC, "CWheel::Init()", "");

	CPhysicsMesh * mesh = resources->GetMesh(meshID);
	if(!mesh) ERRORMSG(ERRGENERIC, "CWheel::Init()", "");

	dVector size (mesh->maxBox - mesh->minBox);
	dVector origin ((mesh->maxBox + mesh->minBox).Scale(0.5f));

	width = size.m_x;

	float wheelDiameter = (size.m_z + size.m_y)* 0.5f;
	radius = wheelDiameter * 0.5f;

	dMatrix wheelPosition (GetIdentityMatrix());

	wheelPosition = wheelPosition * dgYawMatrix(D3DX_PI/2 + (int)initWheel->rotated*2*D3DX_PI/2);

	wheelPosition.m_posit = initWheel->position;

	wheelPosition.m_posit.m_x = CENTIMETER_2_METER(wheelPosition.m_posit.m_x)*scaling.m_x;
	wheelPosition.m_posit.m_y = CENTIMETER_2_METER(wheelPosition.m_posit.m_y)*scaling.m_y;
	wheelPosition.m_posit.m_z = CENTIMETER_2_METER(wheelPosition.m_posit.m_z)*scaling.m_z;

	float wheelMass = initWheel->mass; //5

	/*
	float wheelSuspensionShock = initWheel->suspensionShock; //163
	float wheelSuspensionSpring = initWheel->suspensionSpring; //6000
	float wheelSuspensionLength = CENTIMETER_2_METER(initWheel->suspensionLength); // 0.12
	*/
	//float wheelMass = initWheel->mass;
	//float wheelSuspensionLength = CENTIMETER_2_METER(initWheel->suspensionLength);
	//float wheelSuspensionSpring = (initWheel->carMassOnWheel*abs(GRAVITY))/wheelSuspensionLength;
	//float wheelSuspensionShock = sqrt(wheelSuspensionSpring)*initWheel->suspensionCoef;	
	//float wheelMass = 5.0f;
	//float wheelSuspensionShock = 163.0f;
	//float wheelSuspensionSpring = 6000.0f;
	//float wheelSuspensionLength = 0.12f;
	//float wheelSuspensionLength = 0.5f;
	//float wheelSuspensionSpring = (500.0f * 10.0f) / wheelSuspensionLength;
	//float wheelSuspensionShock = 2.0f * sqrt(wheelSuspensionSpring);
	//float wheelSuspensionLength = 0.05f;
	//float wheelSuspensionSpring = 1.0f;
	//float wheelSuspensionShock = 1.0f;


	dVector wheelPin(0.0f, 0.0f, -1.0f);

	userData.dataType = PHYSICS_WHEEL;
	userData.data = this;

	// set suspension behaviour
#define MEZERA_KOLO_AUTO 0.05f		// 5 cm
	float wheelSuspensionLength = radius + MEZERA_KOLO_AUTO;

#define K 1.5f
	float wheelSuspensionSpring = ((carMass/4)*mats->GetCurrentMutator()->gravity)/wheelSuspensionLength;
	float wheelSuspensionShock = sqrt (wheelSuspensionSpring) * K;

#define WHEEL_MASS_RATIO (1.0f / 50.0f)
	wheelMass = carMass * WHEEL_MASS_RATIO;
	

	//wheelID = NewtonVehicleAddTire(vehicleJoint, &wheelPosition[0][0], &wheelPin[0], wheelMass, width*0.8f, radius,
	//	wheelSuspensionShock, wheelSuspensionSpring, wheelSuspensionLength, &userData, WHEEL_COLLITION_ID);
	wheelID = NewtonVehicleAddTire(vehicleJoint, &wheelPosition[0][0], &wheelPin[0], wheelMass, width, radius,
		wheelSuspensionShock, wheelSuspensionSpring, wheelSuspensionLength, &userData, WHEEL_COLLITION_ID+i);


	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CWheel::Reset()
{
	t_torque = 0.0f;
	t_brakes = 0.0f;
	t_steer = 0.0f;

	parameter = 0.0f;

	t_speed = -431602080.0f;

	currentBrake = 0.0f;

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CWheel::Release()
{
	if(wheelID) 
	{
		NewtonVehicleRemoveTire(vehicleJoint, wheelID); 
		wheelID = NULL; 
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CWheel::SetWheelPhysics()
{

	NewtonVehicleSetTireTorque(vehicleJoint, wheelID, t_torque);

	_lateral = NewtonVehicleGetTireLateralSpeed(vehicleJoint, wheelID);
	_longitudinal = NewtonVehicleGetTireLongitudinalSpeed(vehicleJoint, wheelID);

	// count RPM of the wheel
	// i use formula: rmp = (omega*(360/2PI)[->to degree/s]*60[->to degree/min]/360[to round/min])
	// omega * (30 / PI) after shorting
	float omega = NewtonVehicleGetTireOmega(vehicleJoint, wheelID);	
	m_RPM = omega * (30 / D3DX_PI);

	//t_speed = abs(_longitudinal);

	// Set the maximum side slip velocity for the tire to be considered to lose grip.
	// je to v m/s a znamena to, kolik musi byt lateralni(dostrany vychylujici) rychlost
	// aby doslo ke smyku v dusedku toho, ze pneu uz nesedi...
#define LOST_GRIP_SPEED 10.0f
	NewtonVehicleSetTireMaxSideSleepSpeed (vehicleJoint, wheelID, LOST_GRIP_SPEED);//t_speed*0.2*Configuration.RoadSpeedCoef);	

	// maximum side speed before the vehicle is considered to loose side traction.
	// je to v m/s a znamena to, kolik musi byt podelna rychlost, aby doslo ke smyku pneu
	// v dusledku hrabani, nebo brzdeni
#define LOST_TRACTION_SPEED 5.0f
	
	NewtonVehicleSetTireMaxLongitudinalSlideSpeed (
		vehicleJoint,
		wheelID,
		LOST_TRACTION_SPEED// * (1 / currentSurfaceMaterialCoefs->fSoftness)
		);	
		

	///////////////////////////////////////////////////
	// Steering
	///////////////////////////////////////////////////
#define STEER_SPEED_COEF 0.025f
	float currentSteer = NewtonVehicleGetTireSteerAngle(vehicleJoint, wheelID);
	NewtonVehicleSetTireSteerAngle(vehicleJoint, wheelID, currentSteer + (t_steer - currentSteer) * STEER_SPEED_COEF);

	////////////////////////////////////////////////
	// Breaking system
	// is mutual exclusive to torque system, so we cam apply them simutaneously
	////////////////////////////////////////////////
#define FRICTION_TORQUE 100.0f
#define BREAKING_TORQUE 1000.0f
#define HANDBREAK_TORQUE 7000.0f

#define BREAKING_COEF 0.5f
#define HANDBREAKING_COEF 0.2f

	float breakingSpeed;
	float breaktorque;

	// simulate rolling friction and engine friction
	breaktorque = FRICTION_TORQUE;

	float maxBrakeAccel = NewtonVehicleTireCalculateMaxBrakeAcceleration(vehicleJoint, wheelID);

	if (t_brakes)	// we are using breaks
	{ 		
		if (t_brakes==HANDBREAK_VALUE)
		{
			// HANDBREAKING_COEF is enough to bring full stop
			breakingSpeed = HANDBREAKING_COEF * maxBrakeAccel;

			// but to full stop big torque is needed
			breaktorque += HANDBREAK_TORQUE;
		}
		else
		{
			breakingSpeed = t_brakes * maxBrakeAccel * BREAKING_COEF * m_effic;
			breaktorque += BREAKING_TORQUE * m_effic;		// and add some break torque
		}
	}
	else			// apply value & engine friction
	{
		if (maxBrakeAccel < 0)
			//breaktorque *= -1;	// invert sign of break torque
			breakingSpeed = -FRICTION_TORQUE;
		else
			breakingSpeed = FRICTION_TORQUE;
	}

	// apply values
	NewtonVehicleTireSetBrakeAcceleration(vehicleJoint, wheelID, breakingSpeed, breaktorque);


	// Set the coefficient that tell the engine how much
	// of the lateral force can be absorbed by the tire.
	// todle kolik sily pneu dokaze absorbovat - jak rychle se auto da rozkejvat
#define NORMAL_LOST_GRIP_COEF 10.9f
	NewtonVehicleSetTireSideSleepCoeficient (vehicleJoint, wheelID, NORMAL_LOST_GRIP_COEF);

	if (NewtonVehicleTireLostTraction (vehicleJoint, wheelID))
	{
		int SmykPodel = 1;		
#define SMYK_TRENI 1.0f
		OUTMSG( "Hrabacka", 1 )
		// Set the coefficient that tell the engine how much of the longitudinal 
		// force can be absorbed by the tire.
		// pri smyku mala -> 
		//NewtonVehicleSetTireLongitudinalSlideCoeficient(vehicleJoint, wheelID, SMYK_TRENI);
	}
	else
	{
		
	}

	// check if we got smyk :)
	if (NewtonVehicleTireLostSideGrip (vehicleJoint, wheelID))
	{
		int i = 0;	
		OUTMSG( "Smyk!!!!", 1 )
//#define SLIP_LOST_GRIP_COEF 0.9f
		//NewtonVehicleSetTireSideSleepCoeficient (vehicleJoint, wheelID, SLIP_LOST_GRIP_COEF);
	}
	else
	{
		
	}

#if 0
	//int a =0 ;
	//if(powered)// && LIES_AROUND(torque, 0.0f, 0.01f))//(-0.01f < torque && torque < 0.01f))
	//{
	//	//brakeAcceleration = NewtonVehicleTireCalculateMaxBrakeAcceleration(vehicleJoint, wheelID);
	//	resistance = SIGNUM(omega)*20.0f*(_longitudinal/60.0f);
	//	NewtonVehicleTireSetBrakeAcceleration(vehicleJoint, wheelID, resistance, 10000.0f);
	//	a = 1;
	//}

	////if(type)
	////{
	////	NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*200.0f);
	//	//// zadni
	//	//// trava
	//	//if(material == 3)
	//	//{
	//	//	NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*0.2f);	
	//	//	//NewtonVehicleSetTireLongitudinalSlideCoeficient(vehicleJoint, wheelID, 0.0f);
	//	//	//NewtonVehicleSetTireMaxLongitudinalSlideSpeed(vehicleJoint, wheelID, 15.0f);
	//	//	//NewtonVehicleSetTireSideSleepCoeficient(vehicleJoint, wheelID, 0);
	//	//	//NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*5.5f);
	//	}
	//	//// silnice
	//	//if(material == 6)
	//	//{
	//	//	NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*0.02f);	
	//	//	//NewtonVehicleSetTireLongitudinalSlideCoeficient(vehicleJoint, wheelID, 0.0f);
	//	//	//NewtonVehicleSetTireMaxLongitudinalSlideSpeed(vehicleJoint, wheelID, 15.0f);
	//	//	//NewtonVehicleSetTireSideSleepCoeficient(vehicleJoint, wheelID, 0f);
	//	//	//NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*5.5f);
	//	//}
	////}
	////else
	////{
	////	NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*0.2f);
	//	//// predni
	//	//// trava
	//	//if(material == 3)
	//	//{
	//	//	NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*0.2f);	
	//	//	//NewtonVehicleSetTireLongitudinalSlideCoeficient(vehicleJoint, wheelID, 0.0f);
	//	//	//NewtonVehicleSetTireMaxLongitudinalSlideSpeed(vehicleJoint, wheelID, 15.0f);
	//	//	//NewtonVehicleSetTireSideSleepCoeficient(vehicleJoint, wheelID, 0);
	//	//	//NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*5.5f);
	//	//}
	//	//// silnice
	//	//if(material == 6)
	//	//{
	//	//	NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*1.6f);	
	//	//	//NewtonVehicleSetTireLongitudinalSlideCoeficient(vehicleJoint, wheelID, 0.0f);
	//	//	//NewtonVehicleSetTireMaxLongitudinalSlideSpeed(vehicleJoint, wheelID, 15.0f);
	//	//	//NewtonVehicleSetTireSideSleepCoeficient(vehicleJoint, wheelID, 0f);
	//	//	//NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*5.5f);
	//	//}

	////}

	////OUTFN(t_speed,1);

	////NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, t_speed*0.2);	

	////NewtonVehicleSetTireSideSleepCoeficient(vehicleJoint, wheelID, 0.1f);
	//

	////Radim NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, 5.5f);
	////NewtonVehicleSetTireMaxSideSleepSpeed(vehicleJoint, wheelID, 20.0f);//_lateral*0.1f);
	////NewtonVehicleSetTireSideSleepCoeficient(vehicleJoint, wheelID, parameter);
	////NewtonVehicleSetTireLongitudinalSlideCoeficient(vehicleJoint, wheelID, parameter);
	////NewtonVehicleSetTireSideSleepCoeficient(vehicleJoint, wheelID, parameter);
	////
	////NewtonVehicleSetTireMaxLongitudinalSlideSpeed(vehicleJoint, wheelID, 10.0f);

	////if(t_speed < 0.1f)
	////	t_speed = 0.0f;
	////NewtonVehicleSetTireSideSleepCoeficient(vehicleJoint, wheelID, t_speed*0.5f);

	////NewtonVehicleSetTireMaxLongitudinalSlideSpeed(vehicleJoint, wheelID, t_speed * 0.05f);
	////NewtonVehicleSetTireLongitudinalSlideCoeficient(vehicleJoint, wheelID, t_speed * 0.05f);

#endif
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CWheel::SetTransform()
{
	NewtonVehicleGetTireMatrix(vehicleJoint, wheelID, &nBodyMatrix[0][0]);
	dMatrix rot = dgYawMatrix(D3DX_PI/2);
	nBodyMatrix = rot * nBodyMatrix;
	changed = true;
};
