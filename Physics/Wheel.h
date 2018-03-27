//////////////////////////////////////////////////////////////////////////////////////////////
//
// CWheel
// TODO: - complete this ...
//	
//////////////////////////////////////////////////////////////////////////////////////////////
// if params 0.0f set to default

#pragma once

#include "stdafx.h"
#include "Rigidbody.h"
#include "PhysicsMesh.h"

namespace physics
{
    class CCar;

#define HANDBREAK_VALUE 5.0f

	class CWheel : CRigidBody
	{
	// CWheel properties
	public:
		int dynamicObjectsIndex;

	private:
		float width;
		float radius;
		bool powered;

		float t_torque;
		float t_brakes;
		float t_steer;

		float t_handbrake;

		float t_speed;

		float m_RPM;

		void * wheelID;
		NewtonJoint * vehicleJoint;

		float currentBrake;

		float m_effic;

		resManNS::MaterialsCoef *currentSurfaceMaterialCoefs;

	public:
		float _omega, _lateral, _longitudinal;

		

		float parameter;

		int type;

	// CWheel methods
	public:
		~CWheel();

		HRESULT Init(NewtonWorld * world, float carMass, CResources * resources, InitWheel * initWheel, CMaterials *mats, UINT i);

		HRESULT Reset();

		void Release();

		void SetWheelPhysics();

		void SetTransform();

		void Torque(float torque) { t_torque = torque; }; // set torque
		void Brakes(float brakes) { t_brakes = brakes; }; // set brakes
		//void HandBrake(float handbrake) { t_handbrake = handbrake; }; 
		void Steer(float steer) { t_steer = steer; }; // set steer
		bool Powered() { return powered; }; // get powered
		void * WheelID() { return wheelID; };

		void Speed(float speed) { t_speed = speed; };
		float Radius() { return radius; };

		dMatrix _Matrix() { return Matrix(); };

		inline void SetMaterial (resManNS::MaterialsCoef * mat) { currentSurfaceMaterialCoefs = mat; }
		inline float GetCurrRPM (void) { return m_RPM; }

		void IncParameter() { /*if(parameter < 1.0f) parameter += 0.01f;*/ };
		void DecParameter() { /*if(parameter > 0.0f) parameter -= 0.01f;*/ };

	private:
	};
}; // namespace physics