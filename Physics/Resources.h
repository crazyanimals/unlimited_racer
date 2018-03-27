//////////////////////////////////////////////////////////////////////////////////////////////
//
// CResources
// TODO: complete this ...
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

#include "..\ResourceManager\ResourceManager.h"

namespace physics
{
	class CPhysicsMesh;

	typedef std::vector<resManNS::RESOURCEID> RESOURCEIDVector;
	typedef signed int MESHID;
	
	class CResources
	{
	// CResources structures
		struct _PhysicsMesh
		{
			CPhysicsMesh * mesh;
			NewtonCollision * collision;
			CAtlString name;
			dVector scaling;
		};

		typedef std::vector<_PhysicsMesh *> _PhysicsMeshVector;

	// CResources properties
	public:
		resManNS::CResourceManager * rsm;

	private:
		NewtonWorld * nWorld;

		RESOURCEIDVector loadedResources;
		
		_PhysicsMeshVector loadedMeshes;

	// CResources methods
	public:
		CResources();

		~CResources();

		void Init(resManNS::CResourceManager * rsm, NewtonWorld * world);

		void Release();

		void AddResource(resManNS::RESOURCEID resourceID);

		void ReleaseResources();

		void ReleaseAllResources (void);

		MESHID LoadDynamicMesh(CAtlString meshName, dVector * scaling, bool bCreateCollision);

		MESHID LoadStaticMesh(CAtlString meshName, dVector * scaling, bool bCreateCollision);

		CPhysicsMesh * GetMesh(MESHID meshID);

		void ReleaseMeshes();

		NewtonCollision * GetCollision(MESHID meshID);

	private:
		MESHID FindMesh(CAtlString meshName, dVector * scaling);

		MESHID LoadMesh(CAtlString meshName, dVector * scaling);

		_PhysicsMesh * CreatePhysicsMesh(CAtlString meshName, dVector * scaling);

		void ReleasePhysicsMesh(_PhysicsMesh * _physicsMesh);

		void CreateCollisionFromBox(NewtonCollision * collision, CPhysicsMesh * mesh);

		void CreateCollisionFromBoxFace(NewtonCollision * collision, dVector p0, dVector p1, dVector p2);

		void CreateCollisionFromMesh(NewtonCollision * collision, CPhysicsMesh * mesh);
	};
}; // namespace physics