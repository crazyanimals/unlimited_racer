#include "PhysicsMesh.h"

using namespace physics;

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CPhysicsMesh::CPhysicsMesh(CResources * resources) 
{
	resXFileID = -1;
	this->resources = resources;
	pVertices = NULL;
	pIndices = NULL;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CPhysicsMesh::~CPhysicsMesh()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsMesh::Release()
{
	SAFE_DELETE_ARRAY(pVertices);

	SAFE_DELETE_ARRAY(pIndices);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CPhysicsMesh::Load(CAtlString model, dVector * scaling)
{
	HRESULT hr;
	iLoadingState = 0;
	CAtlString path = resources->rsm->GetResourcePath(RES_Model);

	hr = NAMEVALUEPAIR::Load(path + model, LoadModelDefinition_CallbackStatic, this);
	if(hr)
		ERRORMSG(ERRGENERIC, "CPhysicsMesh::Load()", "Some model parts could not be loaded.");

	resManNS::XFile * pXFile = resources->rsm->GetXFile(resXFileID);
    if(!pXFile)
		ERRORMSG(ERRGENERIC, "CPhysicsMesh::Load()", "Invalid model pointer.");

	LPD3DXMESH pMesh = pXFile->Mesh;

	numVertices = (int)pMesh->GetNumVertices();
	MESHVERTEX * pMeshVertices;

	if((hr = pMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID *) &pMeshVertices)) != ERRNOERROR)
		ERRORMSG(hr, "CPhysicsMesh::Load()", "Locking of vertex buffer failed.");
	  
	SAFE_NEW_ARRAY(pVertices, dVector, numVertices, "CPhysicsMesh::Load()");

	minBox = dVector(CENTIMETER_2_METER(pMeshVertices[0].pos[0])*scaling->m_x,
		CENTIMETER_2_METER(pMeshVertices[0].pos[1])*scaling->m_z,
		CENTIMETER_2_METER(pMeshVertices[0].pos[2])*scaling->m_z);
	maxBox = dVector(CENTIMETER_2_METER(pMeshVertices[0].pos[0])*scaling->m_x,
		CENTIMETER_2_METER(pMeshVertices[0].pos[1])*scaling->m_y,
		CENTIMETER_2_METER(pMeshVertices[0].pos[2])*scaling->m_z);

	for(UINT i=0; i<numVertices; i++)
	{
		pVertices[i].m_x = CENTIMETER_2_METER(pMeshVertices[i].pos[0])*scaling->m_x;
		pVertices[i].m_y = CENTIMETER_2_METER(pMeshVertices[i].pos[1])*scaling->m_y;
		pVertices[i].m_z = CENTIMETER_2_METER(pMeshVertices[i].pos[2])*scaling->m_z;

		minBox.m_x = minBox.m_x > pVertices[i].m_x ? pVertices[i].m_x : minBox.m_x;
		minBox.m_y = minBox.m_y > pVertices[i].m_y ? pVertices[i].m_y : minBox.m_y;
		minBox.m_z = minBox.m_z > pVertices[i].m_z ? pVertices[i].m_z : minBox.m_z;

		maxBox.m_x = maxBox.m_x < pVertices[i].m_x ? pVertices[i].m_x : maxBox.m_x;
		maxBox.m_y = maxBox.m_y < pVertices[i].m_y ? pVertices[i].m_y : maxBox.m_y;
		maxBox.m_z = maxBox.m_z < pVertices[i].m_z ? pVertices[i].m_z : maxBox.m_z;		
	}

	if((hr = pMesh->UnlockVertexBuffer()) != ERRNOERROR)
		ERRORMSG(hr, "CPhysicsMesh::Load()", "Unlocking of vertex buffer failed.");

	numFaces = pMesh->GetNumFaces();
	LPVOID * pMeshIndices;

	if((hr = pMesh->LockIndexBuffer(D3DLOCK_READONLY, (LPVOID *) &pMeshIndices)) != ERRNOERROR)
		ERRORMSG(hr, "CPhysicsMesh::Load()", "Locking of index buffer failed.");

	SAFE_NEW_ARRAY(pIndices, DWORD, numFaces*3, "CPhysicsMesh::Load()");

	for(UINT i=0; i<numFaces*3; i++)
		if(numVertices > 65535)
			pIndices[i] = ((DWORD *)pMeshIndices)[i];
		else
			pIndices[i] = ((WORD *)pMeshIndices)[i];

	if((hr = pMesh->UnlockIndexBuffer()) != ERRNOERROR)
		ERRORMSG(hr , "CPhysicsMesh::Load()", "Unlocking of index buffer failed.");

	dVector size = maxBox - minBox;
	diameter = sqrt(size.m_x*size.m_x + size.m_y*size.m_y + size.m_z*size.m_z)*0.5f;

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CPhysicsMesh::LoadModelDefinition_CallbackStatic(LPNAMEVALUEPAIR pPair)
{
	((CPhysicsMesh *) (pPair->ReservedPointer))->LoadModelDefinition_Callback(pPair);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicsMesh::LoadModelDefinition_Callback(LPNAMEVALUEPAIR pPair)
{
	CAtlString	name = pPair->GetName();
	DWORD		dwParam;
	CAtlString	string;
	
	// TODO: check error messages
	#define _THROW_ERR(code, str)	{	ErrorHandler.HandleError(code, "CPhysicsMesh::LoadModelDefinition_Callback()", str); \
	pPair->ThrowError(); return; }

	name.MakeLower();
	

	if(name == "" || iLoadingState == 2) return;

    string.Format("lod%i", MODEL_LOD_COUNT-1);

	if(name == string && iLoadingState == 0 && pPair->GetFlags() == NVPTYPE_TAGBEGIN) iLoadingState = 1;

	if(name == string && iLoadingState == 0 && pPair->GetFlags() == NVPTYPE_TAGEND) iLoadingState = 0;

	if(name == "model" && iLoadingState == 1 && pPair->GetFlags() & NVPTYPE_VARIABLE)
	{
		//load model
		dwParam = D3DXMESH_MANAGED;

		resXFileID = resources->rsm->LoadResource(pPair->GetString(), RES_XFile, &dwParam);
			if(resXFileID < 0) _THROW_ERR(ERRGENERIC, "XFile could not be loaded.")
		resources->AddResource(resXFileID);
		iLoadingState = 2;
	}
	
	return;

	#undef _THROW_ERR
};