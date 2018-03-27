//////////////////////////////////////////////////////////////////////////////////////////////
//
// CPhysicsSurface
//
// TODO: complete this ...
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

#include "PhysicsMacros.h"
#include "Resources.h"

namespace physics
{		
	class CPhysicsSurface
	{
		// CPhysicsSurface structures
		struct SURFACEVERTEX
		{
			float x, y, z;
			float nx, ny, nz;
			float tu, tv;
		};

		// CPhysicsSurface properties
	public:
		dVector * pVertices;
		UINT segsCountX;
		UINT segsCountZ;
		UINT vertsCount;

		// CPhysicsSurface methods
	public:
		CPhysicsSurface();

		~CPhysicsSurface();

		void Release();

		void Copy(dVector * points, UINT point, UINT sx, UINT sz, UINT rot, dVector position);

		HRESULT Load(CResources * resources, resManNS::RESOURCEID resGrPlateID);	
	
	private:
		UINT Rotation(UINT rot, UINT x, UINT y);
	};
}; // namespace physics