//////////////////////////////////////////////////////////////////////////////////////////////
//
// CCar
// TODO: - complete this ...
//	
//	car must be oriented in our project along z axis ... documentation note
//
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "Engine.h"
#include "Wheel.h"
#include "PhysicsMesh.h"

namespace physics
{
	class CPhysics;

    typedef std::vector<CWheel *> WheelsVector;

	#define CLUTCH_QUANTUM 0.1f	// kvantum spojky, jak rychle se macka a rozevira

	class CCar : public CRigidBody
	{
	// CCar structures
	struct MassPos
	{
		float mass;
		float pos;
	};

	// CCar properties
	public:
		WheelsVector poweredWheels;
		WheelsVector wheels;

		CWheel *GetWheel (UINT i) {	return wheels[i]; }
		CEngine * engine;

		int gearBoxType;

		//float _maxWheelAngle;
		dVector _force;
	private: 
		//#define WHEEL_ANGLES_COUNT 9
		#define MAX_ANGLE 30.0f // ve stupnich
		//#define CAR_WIDTH 1.65f
		//#define CAR_LENGTH 2.364f

		//float angles[2][WHEEL_ANGLES_COUNT];
		//int curAngle;
		NewtonJoint * vehicleJoint;
		NewtonBody * m_cameraBody;

		dVector velocity;
		float _speed;
		
		float m_currentBrake;
		bool handBrake;
		float currentHandBrake;

		float m_wheelRpm;

		BYTE m_dirOfMovement;

		float frontalArea;

		float car_w_width;
		float car_w_length;
		float cogx;

		float shift_up;
		float shift_down;

		dVector carCamPos;
		
		float m_currClutchRatio;	// 1=clutchFullyPushed, 0=clutchFullyOpened

		CPhysics * physics;

	// CCar methods
	public:
		CCar() {};
		// class destructor
		~CCar();

		HRESULT Init(NewtonWorld * world, CResources * resources, InitCar * initCar, CPhysics * physics);

		HRESULT Reset();

		dVector GetCamPos (void);

		void Release();

		void Keys(char keys[]);

		bool Airborne();

		void RestartCar();

		void RestartCar(float x, float z, dVector & heading);//, UINT type);

		void RestartCar(float x, float y, float z, dVector & heading);

		NewtonJoint * VehicleJoint() { return vehicleJoint; };

		float Speed() { return _speed; };

		int Gear() { return (int)engine->GetGear(); };

		void SetForce(dVector force) { _force = force; };

		// set steering in degrees - input is clamped between maximum and minimum
		// value that is available
		void SetSteer(float value) { Steer(CLAMP_BETWEEN(value, -MAX_ANGLE, MAX_ANGLE)/MAX_ANGLE); };
		
		// set throttle - input is clamped between 1.0f and 0.0f
		void SetThrottle(float value) { Throttle(CLAMP_BETWEEN(value,0.0f,1.0f)); };

		// set brakes - input is clamped between 1.0f and 0.0f
		void SetBrakes(float value) { Brakes(CLAMP_BETWEEN(value,0.0f,1.0f)); };

		// set handbrake - input is clamped either to value 1.0f or to 0.0f
		void SetHandBrake(float value) { HandBrake(CLAMP_TO(value,0.0f,1.0f)); };

		void SetThrottleBrakes(float value1, float value2);

		static void UpdateCameraCallBack(const NewtonBody * nBody);

	private:
		void ApplyForceAndTorque_Callback();

		void SetTransform_Callback();

        static void WheelUpdateCallbackStatic(const NewtonJoint * vehicleJoint);

		void WheelUpdate_Callback();

		void Steer(float value);

		void Throttle(float value);

		void Brakes(float value);

		void HandBrake(float value);

		void FindMasses(MassPos * points, UINT min, UINT max, MassPos center);

		void UpdateGearBox(void);

		void IsInWorldBoundingBox(dVector & position);
	};
}; // namespace physics