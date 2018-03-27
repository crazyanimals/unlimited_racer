#include "PhysicsPlate.h"
#include "Physics.h"

using namespace physics;

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CPhysicsPlate::CPhysicsPlate()
{
	collision = NULL;
	nBody = NULL;
	physics = NULL;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CPhysicsPlate::~CPhysicsPlate()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsPlate::Release()
{
	ReleaseCollision();
	if(nBody)
	{
		NewtonDestroyBody(physics->World(), nBody);
		nBody = NULL;
	}
	physics = NULL;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsPlate::ReleaseCollision()
{
	if(collision)
	{
		NewtonReleaseCollision(physics->World(), collision);
		collision = NULL;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsPlate::Begin(CPhysics * physics, UINT posX, UINT posY, UINT sizeX, UINT sizeY)
{
	this->physics = physics;
	this->posX = posX;
	this->posY = posY;
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	collision = NewtonCreateTreeCollision(physics->World(), NULL);

	NewtonTreeCollisionBeginBuild(collision);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsPlate::AddPlanar(CPhysicsSurface * phSurface, UINT rotation, dVector position)
{
	dVector points[4];

	phSurface->Copy(points, 0, 0, 0, rotation, position);
	phSurface->Copy(points, 1, phSurface->segsCountX, 0, rotation, position);
	phSurface->Copy(points, 2, phSurface->segsCountX, phSurface->segsCountZ, rotation, position);
	phSurface->Copy(points, 3, 0, phSurface->segsCountZ, rotation, position);

	Add(4, points);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsPlate::AddNonPlanar(CPhysicsSurface * phSurface, UINT rotation, dVector position)
{
	dVector points[3];

	for(UINT sz=0; sz<phSurface->segsCountZ; sz++)
		for(UINT sx=0; sx<phSurface->segsCountX; sx++)
		{
			phSurface->Copy(points, 0, sx, sz, rotation, position); 
			phSurface->Copy(points, 1, sx+1, sz, rotation, position);
			phSurface->Copy(points, 2, sx, sz+1, rotation, position);

			Add(3, points);

			phSurface->Copy(points, 0, sx+1, sz, rotation, position);
			phSurface->Copy(points, 1, sx+1, sz+1, rotation, position);
			phSurface->Copy(points, 2, sx, sz+1, rotation, position);

			Add(3, points);
		}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsPlate::End()
{
	NewtonTreeCollisionEndBuild(collision, 0);

	nBody = NewtonCreateBody(physics->World(), collision);

	userData.dataType = PHYSICS_TERRAIN;
	userData.data = this;
	NewtonBodySetUserData(nBody, &userData);

	NewtonBodySetMaterialGroupID(nBody, physics->materials.GetMaterialID (physics::MAT_GRASS) );

	NewtonBodySetMatrix(nBody, &GetIdentityMatrix()[0][0]);

	ReleaseCollision();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsPlate::Add(UINT count, dVector * vector)
{
	NewtonTreeCollisionAddFace(collision, count, &vector->m_x, sizeof(dVector), 1);
};