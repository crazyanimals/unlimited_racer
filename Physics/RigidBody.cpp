#include "RigidBody.h"

using namespace physics;

//// class constructor
//CRigidBody::CRigidBody()
//{
//	nBody = NULL;
//}
//
//// class destructor
//CRigidBody::~CRigidBody()
//{
//}

#pragma message(WARNING_MSG("is NBody() necessary????"))
#pragma message(WARNING_MSG("nBodyMatrix = matrix !!!!"))


//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CRigidBody::RayCast_CallbackStatic(const NewtonBody * nBody, const float * normal, int collisionID, void * userData, float intersetParam)
{
	//CRigidBody * body = (CRigidBody *) ((UserData *) NewtonBodyGetUserData(nBody))->data;
	CRigidBody * body = (CRigidBody *) ((UserData *)userData)->data;
	
	return body->RayCast_Callback(/*normal, collisionID, userData,*/ intersetParam);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CRigidBody::RayCast_Callback(/*const float * normal, int collisionID, void * userData,*/ float intersetParam)
{
	floor_y = (WORLD_TOP-WORLD_BOTTOM) * intersetParam;
	return intersetParam;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CRigidBody::GetHeight(float x, float z)
{
	dVector bottom = dVector(x, WORLD_BOTTOM, z);
	dVector top = dVector(x, WORLD_TOP, z);
	NewtonWorldRayCast(NewtonBodyGetWorld(nBody), &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	if(floor_y == 100.0f)
	{
		bottom = dVector(x+0.2f, WORLD_BOTTOM, z);
		top = dVector(x+0.2f, WORLD_TOP, z);
		NewtonWorldRayCast(NewtonBodyGetWorld(nBody), &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	}
	if(floor_y == 100.0f)
	{
		bottom = dVector(x-0.2f, WORLD_BOTTOM, z);
		top = dVector(x-0.2f, WORLD_TOP, z);
		NewtonWorldRayCast(NewtonBodyGetWorld(nBody), &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	}
	if(floor_y == 100.0f)
	{
		bottom = dVector(x, WORLD_BOTTOM, z+0.2f);
		top = dVector(x, WORLD_TOP, z+0.2f);
		NewtonWorldRayCast(NewtonBodyGetWorld(nBody), &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	}
	if(floor_y == 100.0f)
	{
		bottom = dVector(x, WORLD_BOTTOM, z-0.2f);
		top = dVector(x, WORLD_TOP, z-0.2f);
		NewtonWorldRayCast(NewtonBodyGetWorld(nBody), &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);
	}
	CAtlString msg;
	if(floor_y == 100.0f)
	{
		msg.Format("GetHeight failure: height %5.2f at coords [%5.2f,%5.2f]", floor_y, x, z);
		OUTMSG(msg, 1);
	}
	return floor_y;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CRigidBody::Position(float x, float y, float z)
{	
	//dVector bottom = dVector(x, WORLD_BOTTOM, z);
	//dVector top = dVector(x, WORLD_TOP, z);
	//NewtonWorldRayCast(NewtonBodyGetWorld(nBody), &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);

	dMatrix matrix;
	dMatrix trMatrix(GetIdentityMatrix());

	NewtonBodyGetMatrix(nBody, &matrix[0][0]);
	
	trMatrix.m_posit.m_x = x;
	//trMatrix.m_posit.m_y = y;
	trMatrix.m_posit.m_y = y;
	//trMatrix.m_posit.m_y = 15.0f;
	trMatrix.m_posit.m_z = z;

	matrix = trMatrix;

	nBodyMatrix = matrix;
	NewtonBodySetMatrix(nBody, &matrix[0][0]);

};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CRigidBody::Position(float x, float z)
{	
	//dVector bottom = dVector(x, WORLD_BOTTOM, z);
	//dVector top = dVector(x, WORLD_TOP, z);
	//NewtonWorldRayCast(NewtonBodyGetWorld(nBody), &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);

	dMatrix matrix;
	dMatrix trMatrix(GetIdentityMatrix());

	NewtonBodyGetMatrix(nBody, &matrix[0][0]);
	
	trMatrix.m_posit.m_x = x;
	//trMatrix.m_posit.m_y = y;
	trMatrix.m_posit.m_y = GetHeight(x, z);
	//trMatrix.m_posit.m_y = 15.0f;
	trMatrix.m_posit.m_z = z;

	matrix = matrix * trMatrix;

	nBodyMatrix = matrix;
	NewtonBodySetMatrix(nBody, &matrix[0][0]);

};

//
////////////////////////////////////////////////////////////////////////////////////////////////
////
//// 
////
////////////////////////////////////////////////////////////////////////////////////////////////
//void CRigidBody::Position(dVector & vector)
//{
//	dMatrix matrix;
//	dMatrix trMatrix(GetIdentityMatrix());
//
//	NewtonBodyGetMatrix(nBody, &matrix[0][0]);
//	
//	trMatrix.m_posit = vector;
//
//	matrix = matrix * trMatrix;
//
//	nBodyMatrix = matrix;
//	NewtonBodySetMatrix(nBody, &matrix[0][0]);
//};
//
////////////////////////////////////////////////////////////////////////////////////////////////
////
//// 
////
////////////////////////////////////////////////////////////////////////////////////////////////
//dVector CRigidBody::Position()
//{
//	dMatrix matrix;
//
//	NewtonBodyGetMatrix(nBody, &matrix[0][0]);
//
//	return matrix.m_posit;
//};
//
////////////////////////////////////////////////////////////////////////////////////////////////
////
//// 
////
////////////////////////////////////////////////////////////////////////////////////////////////
void CRigidBody::Rotation(float x, float y, float z)
{
	dMatrix matrix;
	dMatrix scMatrix(GetIdentityMatrix());

	NewtonBodyGetMatrix(nBody, &matrix[0][0]);

	scMatrix.m_front = scMatrix.m_front.Scale(scaling.m_x);
	scMatrix.m_up = scMatrix.m_up.Scale(scaling.m_y);
	scMatrix.m_right = scMatrix.m_right.Scale(scaling.m_z);

	//matrix = scMatrix * dgRollMatrix(z) * dgPitchMatrix(x) * dgYawMatrix(y);
    matrix = matrix * dgRollMatrix(z) * dgPitchMatrix(x) * dgYawMatrix(y);

	nBodyMatrix = matrix;
	NewtonBodySetMatrix(nBody, &matrix[0][0]);
};


//
////////////////////////////////////////////////////////////////////////////////////////////////
////
//// 
////
////////////////////////////////////////////////////////////////////////////////////////////////
//void CRigidBody::Rotation(dVector & vector)
//{
//	dMatrix matrix;
//	dMatrix scMatrix(GetIdentityMatrix());
//
//	NewtonBodyGetMatrix(nBody, &matrix[0][0]);
//
//	scMatrix.m_front = scMatrix.m_front.Scale(scaling.m_x);
//	scMatrix.m_up = scMatrix.m_up.Scale(scaling.m_y);
//	scMatrix.m_right = scMatrix.m_right.Scale(scaling.m_z);
//
//	//matrix = scMatrix * dgRollMatrix(vector.m_z) * dgPitchMatrix(vector.m_x) * dgYawMatrix(vector.m_y);
//    matrix = matrix * dgRollMatrix(vector.m_z) * dgPitchMatrix(vector.m_x) * dgYawMatrix(vector.m_y);
//
//	nBodyMatrix = matrix;
//	NewtonBodySetMatrix(nBody, &matrix[0][0]);
//};
//
////////////////////////////////////////////////////////////////////////////////////////////////
////
//// 
////
////////////////////////////////////////////////////////////////////////////////////////////////
//void CRigidBody::RotationY(float y)
//{
//	dMatrix matrix;
//
//	NewtonBodyGetMatrix(nBody, &matrix[0][0]);
//
//	matrix = matrix * dgYawMatrix(y);
//
//	nBodyMatrix = matrix;
//	NewtonBodySetMatrix(nBody, &matrix[0][0]);
//};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CRigidBody::PlaceInWorld(dVector & position, dVector & rotation)
{
	//dVector bottom = dVector(position.m_x, WORLD_BOTTOM, position.m_z);
	//dVector top = dVector(position.m_x, WORLD_TOP, position.m_z);
	//NewtonWorldRayCast(NewtonBodyGetWorld(nBody), &bottom.m_x, &top.m_x, RayCast_CallbackStatic, &userData);

	position.m_y += GetHeight(position.m_x, position.m_z) - minBox_y;
		//floor_y - minBox_y;
	position.m_w = 1.0f;

	nBodyMatrix = GetIdentityMatrix();

	nBodyMatrix = nBodyMatrix * dgRollMatrix(rotation.m_z) * dgPitchMatrix(rotation.m_x) * dgYawMatrix(rotation.m_y);

	//#pragma message(WARNING_MSG("here must be position.m_w"))
	nBodyMatrix.m_posit = position;
	NewtonBodySetMatrix(nBody, &nBodyMatrix[0][0]);

	//dVector position1 = dVector(position.m_x, position.m_y, position.m_z);
	//dVector floor = dVector(position.m_x, -10.0f, position.m_z);
	//position1.m_y = 10.0f;
	//NewtonWorldRayCast(NewtonBodyGetWorld(nBody), &position1.m_x, &floor.m_x, RayCastPlacement, NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
dMatrix CRigidBody::Matrix()
{
	dMatrix matrix (nBodyMatrix);
	matrix.m_posit = nBodyMatrix.m_posit.Scale(ONE_METER);
	return matrix;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CRigidBody::ApplyForceAndTorque_CallbackStatic(const NewtonBody * nBody)
{
    CRigidBody * body = (CRigidBody *) ((UserData *) NewtonBodyGetUserData(nBody))->data;

    body->ApplyForceAndTorque_Callback();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CRigidBody::ApplyForceAndTorque_Callback()
{
	float mass;
	float Ixx;
	float Iyy;
	float Izz;

	NewtonBodyGetMassMatrix(nBody, &mass, &Ixx, &Iyy, &Izz);

	dVector g_force(0.0f, mass * (-m_materials->GetCurrentMutator()->gravity), 0.0f);
	
	NewtonBodySetForce(nBody, &g_force.m_x);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CRigidBody::SetTransform_CallbackStatic(const NewtonBody * nBody, const float * matrix)
{
	CRigidBody * body = (CRigidBody *) ((UserData *) NewtonBodyGetUserData(nBody))->data;

	body->SetTransform_Callback();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CRigidBody::SetTransform_Callback()
{
	NewtonBodyGetMatrix(nBody, &nBodyMatrix[0][0]);
	changed = true;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
dVector CRigidBody::Heading()
{
	dVector heading(0.0f, 0.0f, 1.0f);
	return nBodyMatrix.RotateVector(heading);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
dVector CRigidBody::Velocity()
{
	dVector velocity;
	NewtonBodyGetVelocity(nBody, &velocity.m_x);
	return velocity;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
dVector CRigidBody::Position()
{
	return nBodyMatrix.m_posit;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
dMatrix CRigidBody::GetBody()
{
	return nBodyMatrix;
};

void CRigidBody::SetBody( dMatrix  _nBodyMatrix)
{
	nBodyMatrix=_nBodyMatrix;
};
//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CRigidBody::Mass()
{
	float mass;
	float Ixx;
	float Iyy;
	float Izz;
	NewtonBodyGetMassMatrix(nBody, &mass, &Ixx, &Iyy, &Izz);
	return mass;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CRigidBody::BBoxDiameter()
{
	return bboxDiameter;
};