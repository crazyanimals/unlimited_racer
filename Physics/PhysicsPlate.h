//////////////////////////////////////////////////////////////////////////////////////////////
//
// CPhysicsSurface
//
// TODO: complete this ...
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

#include "UserData.h"
#include "PhysicsSurface.h"

namespace physics
{
	class CPhysics;

	class CPhysicsPlate
	{
	// CPhysicsPlate properties
	public:
		UINT posX;
		UINT posY;
		UINT sizeX;
		UINT sizeY;

		CPhysics * physics;
	private:
		NewtonBody * nBody;
		NewtonCollision * collision;

		UserData userData;

	// CPhysicsPlate methods
	public:
		CPhysicsPlate();

		~CPhysicsPlate();

		void Release();

		void ReleaseCollision();

		void Begin(CPhysics * physics, UINT posX, UINT posY, UINT sizeX, UINT sizeY);

		void AddPlanar(CPhysicsSurface * phSurface, UINT rotation, dVector position);

		void AddNonPlanar(CPhysicsSurface * phSurface, UINT rotation, dVector position);

		void End();

	private:
		void Add(UINT count, dVector * vector);
	};
}; // namespace physics