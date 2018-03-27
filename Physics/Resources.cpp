#include "Resources.h"
#include "PhysicsMesh.h"

using namespace physics;

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CResources::CResources()
{
	rsm = NULL;
	nWorld = NULL;
	loadedResources.clear();
	loadedMeshes.clear();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CResources::~CResources()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CResources::Init(resManNS::CResourceManager * rsm, NewtonWorld * world)
{
	this->rsm = rsm;
	nWorld = world;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CResources::Release()
{
	if(loadedResources.size())
		ReleaseAllResources();

	if(loadedMeshes.size())
		ReleaseMeshes();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CResources::AddResource(resManNS::RESOURCEID resourceID) 
{
	loadedResources.push_back(resourceID);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CResources::ReleaseResources()
{
	for(UINT i=3; i<loadedResources.size(); i++)
		rsm->ReleaseResource(loadedResources[i]);
	loadedResources.clear();
};

void CResources::ReleaseAllResources()
{
	for(UINT i=0; i<loadedResources.size(); i++)
		rsm->ReleaseResource(loadedResources[i]);
	loadedResources.clear();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
MESHID CResources::LoadDynamicMesh(CAtlString meshName, dVector * scaling, bool bCreateCollision)
{
	CPhysicsMesh * mesh;
	MESHID meshID = LoadMesh(meshName, scaling);

	if(meshID == -1)
		return meshID;

	mesh = loadedMeshes[meshID]->mesh;

	if(bCreateCollision)
		if(!loadedMeshes[meshID]->collision)
		{
			OUTMSG("Creating collision "+meshName+" ...", 3);
			loadedMeshes[meshID]->collision = 
				NewtonCreateConvexHull(nWorld, mesh->numVertices, &mesh->pVertices[0][0], sizeof(dVector), NULL);
		}

	return meshID;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
MESHID CResources::LoadStaticMesh(CAtlString meshName, dVector * scaling, bool bCreateCollision)
{
	CPhysicsMesh * mesh;
	NewtonCollision * collision;
	MESHID meshID = LoadMesh(meshName, scaling);

	if(meshID == -1)
		return meshID;

	mesh = loadedMeshes[meshID]->mesh;

	if(bCreateCollision)
		if(!loadedMeshes[meshID]->collision)
		{
			OUTMSG("Creating collision "+meshName+" ...", 3);
			collision = NewtonCreateTreeCollision(nWorld, NULL);
			loadedMeshes[meshID]->collision = collision;
			if(meshName == "")
				CreateCollisionFromBox(collision, mesh);
			else
				CreateCollisionFromMesh(collision, mesh);
		}

	return meshID;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CPhysicsMesh * CResources::GetMesh(MESHID meshID)
{
	return loadedMeshes[meshID]->mesh;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CResources::ReleaseMeshes()
{
	for(UINT i=0; i<loadedMeshes.size(); i++)
	{
		ReleasePhysicsMesh(loadedMeshes[i]);
	}
	loadedMeshes.clear();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
NewtonCollision * CResources::GetCollision(MESHID meshID)
{
	return loadedMeshes[meshID]->collision;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
int CResources::FindMesh(CAtlString meshName, dVector * scaling)
{
	for(UINT i=0; i<loadedMeshes.size(); i++)
	{
		if(loadedMeshes[i]->name == meshName)
			if(loadedMeshes[i]->scaling.m_x == scaling->m_x &&
				loadedMeshes[i]->scaling.m_y == scaling->m_y &&
				loadedMeshes[i]->scaling.m_z == scaling->m_z)
				return i;
	}
	return -1;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CResources::_PhysicsMesh * CResources::CreatePhysicsMesh(CAtlString meshName, dVector * scaling)
{
	HRESULT hr;
	_PhysicsMesh * _physicsMesh;

	__SAFE_NEW(_physicsMesh, _PhysicsMesh, "CResources::CreatePhysicsMesh()", NULL);
	_physicsMesh->name = meshName;
	_physicsMesh->scaling = dVector(*scaling);
	_physicsMesh->collision = NULL;
	__SAFE_NEW(_physicsMesh->mesh, CPhysicsMesh(this), "CResources::CreatePhysicsMesh()", NULL);

	if((hr = _physicsMesh->mesh->Load(meshName, scaling)) != ERRNOERROR)
	{
		ReleasePhysicsMesh(_physicsMesh);
		__ERRORMSG(hr, "CResourcess::CreatePhysicsMesh()", "", NULL);
	}

	return _physicsMesh;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
MESHID CResources::LoadMesh(CAtlString meshName, dVector * scaling)
{
	MESHID meshID;
	_PhysicsMesh * _physicsMesh;

	meshID = FindMesh(meshName, scaling);
	OUTMSG("Requesting "+meshName+" ...", 3);
	// mesh not found
	if(meshID == -1)
	{
		OUTMSG("Loading "+meshName+" ...", 3); 
		if((_physicsMesh = CreatePhysicsMesh(meshName, scaling)) == NULL)
		{
			ReleasePhysicsMesh(_physicsMesh);
			__ERRORMSG(ERRGENERIC, "CResources::GetMesh()", "", -1);
		}
		loadedMeshes.push_back(_physicsMesh);
		meshID = MESHID(loadedMeshes.size())-1;
	}

	return meshID;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CResources::ReleasePhysicsMesh(_PhysicsMesh * _physicsMesh)
{
	if(_physicsMesh)
	{
		SAFE_DELETE(_physicsMesh->mesh);
		if(_physicsMesh->collision)
			NewtonReleaseCollision(nWorld, _physicsMesh->collision);
	}
	SAFE_DELETE(_physicsMesh);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CResources::CreateCollisionFromBox(NewtonCollision * collision, CPhysicsMesh * mesh)
{
	float left = mesh->minBox.m_x;
	float front = mesh->minBox.m_z;
	float bottom = mesh->minBox.m_y;
	float right = mesh->maxBox.m_x;
	float back = mesh->maxBox.m_z;
	float top = mesh->maxBox.m_y;
	
	NewtonTreeCollisionBeginBuild(collision);

	// bottom
	CreateCollisionFromBoxFace(collision, dVector(left, bottom, front), dVector(right, bottom, front), dVector(left, bottom, back));
	CreateCollisionFromBoxFace(collision, dVector(right, bottom, front), dVector(right, bottom, back), dVector(left, bottom, back));
	// front
	CreateCollisionFromBoxFace(collision, dVector(left, bottom, front), dVector(right, bottom, front), dVector(left, top, front));
	CreateCollisionFromBoxFace(collision, dVector(right, bottom, front), dVector(right, top, front), dVector(left, top, front));
	// right
	CreateCollisionFromBoxFace(collision, dVector(right, bottom, front), dVector(right, bottom, back), dVector(right, top, front));
	CreateCollisionFromBoxFace(collision, dVector(right, bottom, back), dVector(right, top, back), dVector(right, top, front));
	// back
	CreateCollisionFromBoxFace(collision, dVector(right, bottom, back), dVector(left, bottom, back), dVector(right, top, back));
	CreateCollisionFromBoxFace(collision, dVector(left, bottom, back), dVector(left, top, back), dVector(right, top, back));
	// left
	CreateCollisionFromBoxFace(collision, dVector(left, bottom, back), dVector(left, bottom, front), dVector(left, top, back));
	CreateCollisionFromBoxFace(collision, dVector(left, bottom, front), dVector(left, top, front), dVector(left, top, back));
	// top
	CreateCollisionFromBoxFace(collision, dVector(left, top, front), dVector(right, top, front), dVector(right, top, back));
	CreateCollisionFromBoxFace(collision, dVector(right, top, back), dVector(left, top, back), dVector(left, top, front));

	NewtonTreeCollisionEndBuild(collision, 0);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CResources::CreateCollisionFromBoxFace(NewtonCollision * collision, dVector p0, dVector p1, dVector p2)
{
	dVector points[3];

	points[0] = p0;
	points[1] = p1;
	points[2] = p2;

	NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), 1);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CResources::CreateCollisionFromMesh(NewtonCollision * collision, CPhysicsMesh * mesh)
{
	dVector point[3];
	UINT index;

	NewtonTreeCollisionBeginBuild(collision);

	for(UINT i=0; i<mesh->numFaces; i++)
	{
		index = 3*i;
		for(UINT j=0; j<3; j++)
		{
				point[j].m_x = mesh->pVertices[mesh->pIndices[index+2-j]][0];
				point[j].m_y = mesh->pVertices[mesh->pIndices[index+2-j]][1];
				point[j].m_z = mesh->pVertices[mesh->pIndices[index+2-j]][2];
		}
		NewtonTreeCollisionAddFace(collision, 3, &point[0].m_x, sizeof(dVector), 1);
	}

	NewtonTreeCollisionEndBuild(collision, 1);
};