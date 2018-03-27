//////////////////////////////////////////////////////////////////////////////////////////////
//
// CRigidBody
//
// TODO: complete this ...
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

#include "PhysicsMacros.h"
#include "InitStructures.h"
#include "UserData.h"
#include "Resources.h"

#include "Materials.h"

namespace physics
{
	class CRigidBody
	{
	// CRigidBody properties
	private:
		float floor_y;
		

	protected:
        UserData userData;
		NewtonBody * nBody;
		dVector scaling;
		dMatrix nBodyMatrix;
        bool alwaysActive;
		bool changed;

		float minBox_y;
		float bboxDiameter;

		CMaterials *m_materials;

	// CRigidBody methods
	protected:
		// class constructor
		CRigidBody() {}

		// class destructor
		//~CRigidBody();

		static void ApplyForceAndTorque_CallbackStatic(const NewtonBody * nBody);
        virtual void ApplyForceAndTorque_Callback();

		static void SetTransform_CallbackStatic(const NewtonBody * body, const float * matrix);
		virtual void SetTransform_Callback();

		static float RayCast_CallbackStatic(const NewtonBody * nBody, const float * normal, int collisionID, void * userData, float intersetParam);

		float RayCast_Callback(/*const float * normal, int collisionID, void * userData,*/ float intersetParam);

		float GetHeight(float x, float z);

	private:

	public:
		void Position(float x, float y, float z);
		void Position(float x, float z);
		//void Position(dVector & vector);
		//dVector Position();

		void Rotation(float x, float y, float z);
		//void Rotation(dVector & vector);
		//void RotationY(float y);

		void PlaceInWorld(dVector & position, dVector & rotation);

		dMatrix Matrix();

		void Scaling(float x, float y, float z) { scaling.m_x = x; scaling.m_y = y; scaling.m_z = z; };
		void Scaling(dVector & vector) { scaling = vector; };

		NewtonBody * NBody() { return nBody; };

		bool Active() { if(alwaysActive) return true; else return NewtonBodyGetSleepingState(nBody)!=0; };
		bool Changed() { return changed; };
		void Changed(bool changed) { this->changed = changed; };

		dVector Heading();
		dVector Velocity();
		dVector Position();
		float Mass();
		float BBoxDiameter();
		dMatrix GetBody();
		void SetBody( dMatrix  _nBodyMatrix);

	};
}; // namespace physics