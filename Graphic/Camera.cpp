#include "stdafx.h"
#include "Camera.h"

using namespace graphic;

//////////////////////////////////////////////////////////////////////////////////////////////
// sets EyePt vector
//////////////////////////////////////////////////////////////////////////////////////////////
void CCamera::SetEyePtVector(D3DXVECTOR3 Vector)
{
	EyePt = Vector;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// sets EyePt vector
//////////////////////////////////////////////////////////////////////////////////////////////
void CCamera::SetEyePtVector(float x, float y, float z)
{
	SetEyePtVector( D3DXVECTOR3(x,y,z) );
}
//////////////////////////////////////////////////////////////////////////////////////////////
// sets LookAt vector
//////////////////////////////////////////////////////////////////////////////////////////////
void CCamera::SetLookAtVector(D3DXVECTOR3 Vector)
{
	LookAt = Vector;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// sets UpVector vector
//////////////////////////////////////////////////////////////////////////////////////////////
void CCamera::SetUpVector(D3DXVECTOR3 Vector)
{
	UpVector = Vector;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// moves camera in a direction [FORWARD,BACKWARD,LEFT,RIGHT,UP,DOWN]
//////////////////////////////////////////////////////////////////////////////////////////////
void CCamera::MoveCamera(int Direction)
{
	D3DXVECTOR3 Vec;

	switch(Direction)
	{
	case CM_FORWARD:
		D3DXVec3Normalize(&Vec, D3DXVec3Subtract(&Vec, &LookAt, &EyePt));	// A = Normalize(LookAt - EyePt)
		D3DXVec3Scale(&Vec, &Vec, CM_STEP);									// A = CM_STEP * A
		D3DXVec3Add(&EyePt, &EyePt, &Vec);									// EyePt = EyePt + A
		D3DXVec3Add(&LookAt, &LookAt, &Vec);								// LookAt = LookAt + A
		break;
	case CM_BACKWARD:
		D3DXVec3Normalize(&Vec, D3DXVec3Subtract(&Vec, &EyePt, &LookAt));	// A = Normalize(EyePt - LookAt)
		D3DXVec3Scale(&Vec, &Vec, CM_STEP);									// A = CM_STEP * A
		D3DXVec3Add(&EyePt, &EyePt, &Vec);									// EyePt = EyePt + A
		D3DXVec3Add(&LookAt, &LookAt, &Vec);								// LookAt = LookAt + A
		break;
	case CM_LEFT:
		D3DXVec3Normalize(&Vec, D3DXVec3Cross(&Vec, D3DXVec3Subtract(&Vec, &LookAt, &EyePt), &UpVector));
														// A = Normalize((LookAt - EyePt) x UpVector)
		D3DXVec3Scale(&Vec, &Vec, CM_STEP);				// A = CM_STEP * A
		D3DXVec3Add(&EyePt, &EyePt, &Vec);				// EyePt = EyePt + A
		D3DXVec3Add(&LookAt, &LookAt, &Vec);			// LookAt = LookAT + A
		break;
	case CM_RIGHT:
		D3DXVec3Normalize(&Vec, D3DXVec3Cross(&Vec, &UpVector, D3DXVec3Subtract(&Vec, &LookAt, &EyePt)));
														// A = Normalize(UpVector x (LookAt - EyePt))
		D3DXVec3Scale(&Vec, &Vec, CM_STEP);				// A = CM_STEP * A
		D3DXVec3Add(&EyePt, &EyePt, &Vec);				// EyePt = EyePt + A
		D3DXVec3Add(&LookAt, &LookAt, &Vec);			// LookAt = LookAt + A
		break;
	case CM_UP:
		D3DXVec3Normalize(&Vec, &UpVector);				// A = Normalize(UpVector)
		D3DXVec3Scale(&Vec, &Vec, CM_STEP);				// A = CM_STEP * A
		D3DXVec3Add(&EyePt, &EyePt, &Vec);				// EyePt = EyePt + A
		D3DXVec3Add(&LookAt, &LookAt, &Vec);			// LookAt = LookAt + A
		break;
	case CM_DOWN:
		Vec = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXVec3Normalize(&Vec, D3DXVec3Subtract(&Vec, &Vec, &UpVector));	// A = Normalize(-UpVector)
		D3DXVec3Scale(&Vec, &Vec, CM_STEP);									// A = CM_STEP * A
		D3DXVec3Add(&EyePt, &EyePt, &Vec);									// EyePt = EyePt + A
		D3DXVec3Add(&LookAt, &LookAt, &Vec);								// LookAt = LookAt + A
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// rotates camera [ROLL,PITCH,YAW]
//////////////////////////////////////////////////////////////////////////////////////////////
void CCamera::RotateCamera(int Direction)
{
	D3DXVECTOR3 Vec3, Vec3EL, Vec3a;
	D3DXVECTOR4 Vec4;
	D3DXMATRIX Mat;

	switch(Direction)
	{
	case CR_ROLLLEFT:
		D3DXVec3Subtract(&Vec3, &LookAt, &EyePt);					// A[3] = LookAt - EyePt
		D3DXMatrixRotationAxis(&Mat, &Vec3, D3DXToRadian(CR_STEP));	// M[4x4] - rotation matrix(+)
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);					// B[4] = A[3+1].M[4x4]
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);				// A[3] = B[4]
		break;
	case CR_ROLLRIGHT:
		D3DXVec3Subtract(&Vec3, &LookAt, &EyePt);					// A[3] = LookAt - EyePt
		D3DXMatrixRotationAxis(&Mat, &Vec3, D3DXToRadian(-CR_STEP));	// M[4x4] - rotation matrix(-)
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);					// B[4] = A[3+1].M[4x4]
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);				// A[3] = B[4]
		break;
	case CR_PITCHUP:
		D3DXVec3Cross(&Vec3, &UpVector, D3DXVec3Subtract(&Vec3EL, &LookAt, &EyePt));
																	// A[3] = UpVector x (LookAt - EyePt)
		D3DXMatrixRotationAxis(&Mat, &Vec3, D3DXToRadian(-CR_STEP));	// M[4x4] - rotation matrix(-)
		D3DXVec3Transform(&Vec4, &Vec3EL, &Mat);					// B[4] = A[3+1].M[4x4]
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);					// C[3] = B[4]
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);						// LookAt = EyePt + C[3]
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);					// B[4] = UpVector[3+1].M[4x4]
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);				// UpVector = B[4]
		break;
	case CR_PITCHDOWN:
		D3DXVec3Cross(&Vec3, &UpVector, D3DXVec3Subtract(&Vec3EL, &LookAt, &EyePt));
																	// A[3] = UpVector x (LookAt - EyePt)
		D3DXMatrixRotationAxis(&Mat, &Vec3, D3DXToRadian(CR_STEP));	// M[4x4] - rotation matrix(+)
		D3DXVec3Transform(&Vec4, &Vec3EL, &Mat);					// B[4] = A[3+1].M[4x4]
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);					// C[3] = B[4]
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);						// LookAt = EyePt + C[3]
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);					// B[4] = UpVector[3+1].M[4x4]
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);				// UpVector = B[4]
		break;
	case CR_YAWLEFT:
		/*D3DXVec3Subtract(&Vec3, &LookAt, &EyePt);						// A[3] = LookAt - EyePt
		D3DXMatrixRotationAxis(&Mat, &UpVector, D3DXToRadian(-CR_STEP));	// M[4x4] - rotation matrix(-)
		D3DXVec3Transform(&Vec4, &Vec3, &Mat);							// B[4] = A[3].M[4x4]
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);						// C[3] = B[4]
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);							// LookAt = EyePt + C[3]
		break;*/
		Vec3a = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXVec3Subtract(&Vec3, &LookAt, &EyePt);
		D3DXMatrixRotationAxis(&Mat, &Vec3a, D3DXToRadian(-CR_STEP));
		D3DXVec3Transform(&Vec4, &Vec3, &Mat);
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);
		break;
	case CR_YAWRIGHT:
		/*D3DXVec3Subtract(&Vec3, &LookAt, &EyePt);						// A[3] = LookAt - EyePt
		D3DXMatrixRotationAxis(&Mat, &UpVector, D3DXToRadian(CR_STEP));	// M[4x4] - rotation matrix(+)
		D3DXVec3Transform(&Vec4, &Vec3, &Mat);							// B[4] = A[3].M[4x4]
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);						// C[3] = B[4]
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);							// LookAt = EyePt + C[3]
		break;*/
		Vec3a = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXVec3Subtract(&Vec3, &LookAt, &EyePt);
		D3DXMatrixRotationY(&Mat, D3DXToRadian(CR_STEP));
		D3DXMatrixRotationAxis(&Mat, &Vec3a, D3DXToRadian(CR_STEP));
		D3DXVec3Transform(&Vec4, &Vec3, &Mat);
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// rotates camera - mouse [PITCH, YAW]
//////////////////////////////////////////////////////////////////////////////////////////////
void CCamera::RotateCameraM(int Direction, float Angle)
{
	D3DXVECTOR3 Vec3, Vec3EL, Vec3a;
	D3DXVECTOR4 Vec4;
	D3DXMATRIX Mat;

	switch(Direction)
	{
	case CR_PITCHUP:
		D3DXVec3Cross(&Vec3, &UpVector, D3DXVec3Subtract(&Vec3EL, &LookAt, &EyePt));
																	// A[3] = UpVector x (LookAt - EyePt)
		D3DXMatrixRotationAxis(&Mat, &Vec3, D3DXToRadian(Angle));	// M[4x4] - rotation matrix(-)
		D3DXVec3Transform(&Vec4, &Vec3EL, &Mat);					// B[4] = A[3+1].M[4x4]
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);					// C[3] = B[4]
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);						// LookAt = EyePt + C[3]
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);					// B[4] = UpVector[3+1].M[4x4]
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);				// UpVector = B[4]
		break;
	case CR_PITCHDOWN:
		D3DXVec3Cross(&Vec3, &UpVector, D3DXVec3Subtract(&Vec3EL, &LookAt, &EyePt));
																	// A[3] = UpVector x (LookAt - EyePt)
		D3DXMatrixRotationAxis(&Mat, &Vec3, D3DXToRadian(Angle));	// M[4x4] - rotation matrix(+)
		D3DXVec3Transform(&Vec4, &Vec3EL, &Mat);					// B[4] = A[3+1].M[4x4]
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);					// C[3] = B[4]
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);						// LookAt = EyePt + C[3]
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);					// B[4] = UpVector[3+1].M[4x4]
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);				// UpVector = B[4]
		break;
	case CR_YAWLEFT:
		Vec3a = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXVec3Subtract(&Vec3, &LookAt, &EyePt);
		D3DXMatrixRotationAxis(&Mat, &Vec3a, D3DXToRadian(Angle));
		D3DXVec3Transform(&Vec4, &Vec3, &Mat);
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);
		break;
	case CR_YAWRIGHT:
		Vec3a = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXVec3Subtract(&Vec3, &LookAt, &EyePt);
		D3DXMatrixRotationY(&Mat, D3DXToRadian(CR_STEP));
		D3DXMatrixRotationAxis(&Mat, &Vec3a, D3DXToRadian(Angle));
		D3DXVec3Transform(&Vec4, &Vec3, &Mat);
		Vec3 = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);
		D3DXVec3Add(&LookAt, &EyePt, &Vec3);
		D3DXVec3Transform(&Vec4, &UpVector, &Mat);
		UpVector = D3DXVECTOR3(Vec4.x, Vec4.y, Vec4.z);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// returns EyePt vector
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 * CCamera::GetEyePtVector(D3DXVECTOR3 *vec)
{
	*vec = EyePt;
	return vec;
}

////////////////////////////
// RESET
////////////////////////////
void
CCamera::Reset (void)
{
	EyePt = D3DXVECTOR3 (0, 0, 1);
	LookAt = D3DXVECTOR3 (0, 0, 0);
	UpVector = D3DXVECTOR3 (0, 1, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// returns LookAt vector
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 * CCamera::GetLookAtVector(D3DXVECTOR3 *vec)
{
	*vec = LookAt;
	return vec;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// returns UpVector vector
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 * CCamera::GetUpVector(D3DXVECTOR3 *vec)
{
	*vec = UpVector;
	return vec;
}