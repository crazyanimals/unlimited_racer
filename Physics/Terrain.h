//////////////////////////////////////////////////////////////////////////////////////////////
//
// CTerrain
// TODO: complete this ...
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

#include "..\Globals\Configuration.h"
#include "PhysicsPlate.h"

namespace physics
{
	class CPhysics;

	class CTerrain
	{
		struct PhysicsPlateData
		{
			UINT index;
			UINT sizeX;
			UINT sizeY;
			UINT platesX;
			UINT platesY;
		};

	// CTerrain properties
	private:
		CResources * resources;
		NewtonWorld * nWorld;
		UINT sizeX, sizeY;

		UINT physicsSurfacesCount;
		CPhysicsSurface * physicsSurfaces;

		UINT platesSizeX, platesSizeY;
		UINT physicsPlatesCount;
		CPhysicsPlate * physicsPlates;
		
		UINT mmSizeX, mmSizeY;
		UINT materialsMapSize;
		BYTE * materialsMap;

		NewtonBody * worldBoundingBox;
		
		UserData userData;

	// CTerrain methods
	public:
		CTerrain() { worldBoundingBox = NULL; };

		~CTerrain();

		void Release();

		void Init(NewtonWorld * world, CResources * resources);

		HRESULT Load(CAtlString terrainName, CPhysics * physics, int bboxID);

		BYTE & MaterialsMap(UINT index) { return materialsMap[index]; };

		BYTE & MaterialsMap(UINT x, UINT y) { return materialsMap[mmSizeX*y+x]; };

		int MaterialsMapf(float x, float y) 
		{
			int index = mmSizeX*(int)floorf(y)+(int)floorf(x);
			if(index > (int)(mmSizeX*mmSizeY-1))
			{
				OUTS("Out of bound:\t", 1); OUTIN(index, 1);
				return 0;
			}
			if(index < 0)
			{
				OUTS("Out of bound:\t", 1); OUTIN(index, 1);
				return 0;
			}
			return materialsMap[index]; 
		};

		dVector WorldSize() { return dVector(sizeX*TERRAIN_PLATE_WIDTH_M, WORLD_TOP, sizeY*TERRAIN_PLATE_WIDTH_M); };

		void Debug_Print(CAtlString fileName);

        UINT GetTerrainWidthSquares(void) const
        {
            return sizeX;
        }

        UINT GetTerrainHeightSquares(void) const
        {
            return sizeY;
        }

	private:
		HRESULT LoadPhysicsSurfaces(resManNS::Terrain * resTerrain);

		void SetMaterialsMap(resManNS::PhysicalTexture * resPT, UINT posX, UINT posY, UINT rotation);

		HRESULT LoadPhysicsPlates(resManNS::Terrain * resTerrain, CPhysics * physics);

		HRESULT LoadPhysicsPlate(resManNS::Terrain * resTerrain, PhysicsPlateData * phPlateData);

		void CreateWorldBoundingBox(int bboxID);

		void CreateWorldBoundingBoxFace(NewtonCollision * collision, dVector p0, dVector p1, dVector p2);
	};
}; // namespace physics

// This code is no longer used
#if 0 

struct _VERTEX {
	float x, y, z;
	float nx, ny, nz;
	float tu, tv;
};

class CPlateCollision
{
// CPlateCollision properties
private:
	NewtonCollision * collision;
	NewtonWorld * world;
// CPlateCollision methods
private:
	void Copy(dVector * points, int point, _VERTEX * vert, int index);
	void Copy1(dVector * points, int point, _VERTEX * vert, int index);

public:
	CPlateCollision();
	~CPlateCollision();

	void Create(NewtonWorld * world, int segsx, int segsz, _VERTEX * vert, UINT bPlanar = 0);
	inline NewtonCollision * Collision() { return collision; };
	void Release();
};

class CPlate
{
// CPlate properties
private:
	NewtonBody * nBody;
	UserData userData;
	
public:
	CPhysics * physics;
// CPlate methods
private:
public:
	~CPlate();

	void Create(NewtonWorld * world, NewtonCollision * collision, int rotation, dVector * position, CPhysics * physics);
	void Release();
};

class CTerrain
{
// CTerrain properties
private:
	CResources * resources;
	NewtonWorld * nWorld;
	int plateCollisionsCount;
	CPlateCollision * plateCollisions;
	int platesCount;
	CPlate * plates;

	UINT sizeX, sizeY;
	UINT mmSizeX, mmSizeY;
	BYTE * materialsMap;
	int materialsMapSize;

	UserData userData;

public:

	/////	alternative		/////
private:
	int physicsPlatesCount;
	CPhysicsPlate * physicsPlates;
	int _platesCount;
	_CPlate * _plates;
	
	/////	alternative		/////

// CTerrain methods
private:
	HRESULT LoadPlateCollisions(resManNS::Terrain * resTerrain);
	void CreateWorldBoundingBox();

	/////	alternative		/////		
private:
	HRESULT LoadPhysicsPlates(resManNS::Terrain * resTerrain);

public:
	HRESULT _Load(CAtlString terrainName, CPhysics * physics);

	/////	alternative		/////

public:
	~CTerrain();

	HRESULT Init(NewtonWorld * world, CResources * resources);
	HRESULT Load(CAtlString terrainName, CPhysics * physics);
	void Release();

	BYTE & MaterialsMap(UINT index) { return materialsMap[index]; };
	BYTE & MaterialsMap(UINT x, UINT y) { return materialsMap[mmSizeX*y+x]; };
	BYTE MaterialsMapf(float x, float y) { return materialsMap[mmSizeX*(int)floorf(y)+(int)floorf(x)]; };
};

#endif
