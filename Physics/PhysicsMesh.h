//////////////////////////////////////////////////////////////////////////////////////////////
//
// CPhysicsMesh
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
	class CPhysicsMesh
	{
		// CPhysicsMesh structures
		struct MESHVERTEX
		{
			D3DXVECTOR3 pos, norm;
			FLOAT tu, tv;
		};

		// CPhysicsMesh properties
	public:
		UINT numVertices;
		UINT numFaces;
		dVector * pVertices;
		DWORD * pIndices;
		dVector minBox;
		dVector maxBox;
		float diameter;

	private:
		UINT iLoadingState;
		resManNS::RESOURCEID resXFileID;
		CResources * resources;

		// CPhysicsMesh methods
	public:
		CPhysicsMesh(CResources * resources);

		~CPhysicsMesh();

		void Release();

		HRESULT Load(CAtlString model, dVector * scaling);

	private:
		static void CALLBACK LoadModelDefinition_CallbackStatic(LPNAMEVALUEPAIR pPair);

		void LoadModelDefinition_Callback(LPNAMEVALUEPAIR pPair);
	};
}; // namespace physics