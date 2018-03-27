#include "Object.h"

using namespace physics;

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//
// CObject CObject CObject CObject CObject CObject
//
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//
// CStaticObject CStaticObject CStaticObject CStaticObject
//
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
CStaticObject::~CStaticObject()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// after class object created
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CStaticObject::Init(NewtonWorld *world, CResources * resources, InitObject *initObject, CMaterials *mat)
{
	try
	{
		nBody = NULL;
		alwaysActive = false;
		changed = false;
		scaling = dVector(initObject->scale);   
		nBodyMatrix = GetIdentityMatrix();
		m_materials = mat;
		
		MESHID meshID = resources->LoadStaticMesh(initObject->model, &scaling, true);
		if(meshID == -1) ERRORMSG(ERRGENERIC, "CStaticObject::Init()", "");

		CPhysicsMesh * mesh = resources->GetMesh(meshID);
		if(!mesh) ERRORMSG(ERRGENERIC, "CStaticObject::Init()", "");

		bboxDiameter = mesh->diameter;
		minBox_y = mesh->minBox.m_y;
	    
		NewtonCollision * collision = resources->GetCollision(meshID);
		if(!collision) ERRORMSG(ERRGENERIC, "CStaticObject::Init()", "");
		nBody = NewtonCreateBody(world, collision);

		NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);

		userData.dataType = PHYSICS_STATIC_OBJECT;
		userData.material0 = initObject->material;
		userData.data = this;
		NewtonBodySetUserData(nBody, &userData);

		NewtonBodySetMaterialGroupID(nBody, mat->GetMaterialID (initObject->material));
	}
	catch(...)
	{
		ERRORMSG(ERRGENERIC, "CStaticObject::Init()", "Static object initialization failed (Newton)");
	}

    return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CStaticObject::Reset()
{
	nBodyMatrix = GetIdentityMatrix();

	NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CStaticObject::Release()
{
	if(nBody) 
	{
		NewtonDestroyBody(NewtonBodyGetWorld(nBody), nBody); 
		nBody = NULL; 
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//
// CDynamicObject CDynamicObject CDynamicObject CDynamicObject
//
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
CDynamicObject::~CDynamicObject()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// after class object created
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CDynamicObject::Init(NewtonWorld * world, CResources * resources, InitObject * initObject, CMaterials *mat)
{
	try
	{
		nBody = NULL;
		alwaysActive = false;
		changed = false;
		scaling = dVector(initObject->scale);
		nBodyMatrix = GetIdentityMatrix();
		m_materials = mat;


		MESHID meshID = resources->LoadDynamicMesh(initObject->model, &scaling, true);
		if(meshID == -1) ERRORMSG(ERRGENERIC, "CDynamicObject::Init()", "");

		CPhysicsMesh * mesh = resources->GetMesh(meshID);
		if(!mesh) ERRORMSG(ERRGENERIC, "CDynamicObject::Init()", "");

		bboxDiameter = mesh->diameter;
		minBox_y = mesh->minBox.m_y;

		// TODO: remove fixed value
		//NewtonConvexCollisionSetUserID(collision, 0x400);
		NewtonCollision * collision = resources->GetCollision(meshID);
		if(!collision) ERRORMSG(ERRGENERIC, "CDynamicObject::Init()", "");
		nBody = NewtonCreateBody(world, collision);

		userData.dataType = PHYSICS_DYNAMIC_OBJECT;
		userData.material0 = initObject->material;
		userData.data = this;
		NewtonBodySetUserData(nBody, &userData);

		NewtonBodySetMaterialGroupID(nBody, mat->GetMaterialID (initObject->material));

		NewtonBodySetForceAndTorqueCallback(nBody, ApplyForceAndTorque_CallbackStatic);
		NewtonBodySetTransformCallback(nBody, SetTransform_CallbackStatic);

		dVector inertia, origin;
		NewtonConvexCollisionCalculateInertialMatrix(collision, &inertia.m_x, &origin.m_x);

		initObject->cog.m_x = CENTIMETER_2_METER(initObject->cog.m_x);
		initObject->cog.m_y = CENTIMETER_2_METER(initObject->cog.m_y);
		initObject->cog.m_z = CENTIMETER_2_METER(initObject->cog.m_z);
		origin += initObject->cog;

		NewtonBodySetMassMatrix(nBody, initObject->mass, initObject->mass*inertia.m_x, initObject->mass*inertia.m_y, initObject->mass*inertia.m_z);
		NewtonBodySetCentreOfMass(nBody, &origin.m_x);

		NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);
	}
	catch(...)
	{
		ERRORMSG(ERRGENERIC, "CDynamicObject::Init()", "Dynamic object initialization failed (Newton)");
	}

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CDynamicObject::Reset()
{
	nBodyMatrix = GetIdentityMatrix();

	NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CDynamicObject::Release()
{
	if(nBody) 
	{
		NewtonDestroyBody(NewtonBodyGetWorld(nBody), nBody); 
		nBody = NULL; 
	}
};