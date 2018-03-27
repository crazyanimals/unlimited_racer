#include "PhysicsSurface.h"

using namespace physics;

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CPhysicsSurface::CPhysicsSurface()
{
	pVertices = NULL;
	segsCountX = segsCountZ = vertsCount = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CPhysicsSurface::~CPhysicsSurface()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsSurface::Release()
{
	SAFE_DELETE_ARRAY(pVertices);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsSurface::Copy(dVector * points, UINT point, UINT sx, UINT sz, UINT rot, dVector position)
{
	UINT index = Rotation(DIRECTION_N, sx, sz);
	points[point].m_x = CENTIMETER_2_METER(pVertices[index].m_x + position.m_x);
	points[point].m_y = CENTIMETER_2_METER(pVertices[Rotation(rot, sx, sz)].m_y + position.m_y);
	points[point].m_z = CENTIMETER_2_METER(pVertices[index].m_z + position.m_z);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CPhysicsSurface::Load(CResources * resources, resManNS::RESOURCEID resGrPlateID)
{
	SURFACEVERTEX * vert;
	HRESULT hr;
	resManNS::GrPlate * resGrPlate;
	resManNS::RESOURCEID resGrSurfaceID;
	resManNS::GrSurface * resGrSurface;

	resGrPlate = resources->rsm->GetGrPlate(resGrPlateID);
	if(!resGrPlate) ERRORMSG(ERRNOTFOUND, "CPhysicsSurface::Load()", "Could not obtain grplate object.");
    
	// use desired lod (TERRAIN_LOD_PHYSICS), if not available use best lod
	resGrSurfaceID = resGrPlate->LoD[(TERRAIN_LOD_PHYSICS < resGrPlate->uiLoDCount ? TERRAIN_LOD_PHYSICS : resGrPlate->uiLoDCount-1)];
	resGrSurface = resources->rsm->GetGrSurface(resGrSurfaceID);
	if(!resGrSurface) ERRORMSG(ERRNOTFOUND, "CPhysicsSurface::Load()", "Could not obtain terrain surface data.");

	if((hr = resGrSurface->pVertices->Lock(0, 0, (LPVOID *) &vert, 0)) != ERRNOERROR)
		ERRORMSG(hr, "CPhysicsSurface::Load()", "Locking of vertex buffer failed.");

	vertsCount = resGrSurface->VertsCount;
	segsCountX = resGrSurface->SegsCountX;
	segsCountZ = resGrSurface->SegsCountZ;

	SAFE_NEW_ARRAY(pVertices, dVector, vertsCount, "CPhysicsSurface::Load()");
	
	for(UINT i=0; i<vertsCount; i++)
	{
		pVertices[i].m_x = vert[i].x;
		pVertices[i].m_y = vert[i].y;
		pVertices[i].m_z = vert[i].z;
	}
	
	if((hr = resGrSurface->pVertices->Unlock()) != ERRNOERROR)
		ERRORMSG(hr, "CPhysicsSurface::Load()", "Unlocking of vertex buffer failed.");

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// computes new index
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
UINT CPhysicsSurface::Rotation(UINT rot, UINT x, UINT y)
{
	switch(rot)
	{
	case DIRECTION_N:
		return y * (segsCountZ + 1) + x;
	case DIRECTION_E:
		return x * (segsCountX + 1) + (segsCountX - y); 
	case DIRECTION_S:
		return (segsCountZ - y) * (segsCountX + 1) + (segsCountX - x);
	case DIRECTION_W:
		return (segsCountZ - x) * (segsCountX + 1) + y;
	}
    return y * (segsCountX + 1) + x;
};