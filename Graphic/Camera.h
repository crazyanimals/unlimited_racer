/*
	Camera:			Class used to set camera position. It provides easy to use methods to move and rotate with camera object (that is viewer position).
	Creation Date:	14.11.2003
	Last Update:	22.3.2004
	Author:			Pavel Janda

*/


#pragma once

#include "stdafx.h"


// macros for camera move
#define CM_FORWARD		0 
#define CM_BACKWARD		1 
#define CM_LEFT			2 
#define CM_RIGHT		3 
#define CM_UP			4
#define CM_DOWN			5

#define CM_STEP			100.0f

// macros for camera rotation
#define CR_ROLLLEFT		0
#define CR_ROLLRIGHT	1
#define CR_PITCHUP		2
#define CR_PITCHDOWN	3
#define CR_YAWLEFT		4
#define CR_YAWRIGHT		5

#define CR_STEP			6.0f

namespace graphic
{

// CCamera class
	class CCamera
	{
	public:
		// public methods
		void SetEyePtVector( D3DXVECTOR3 Vector ); // sets EyePt vector
		void SetEyePtVector( float x, float y, float z ); // sets EyePt vector
		void SetLookAtVector( D3DXVECTOR3 Vector ); // sets LookAt vector
		void SetLookAtVector( float x, float y, float z ) { SetLookAtVector( D3DXVECTOR3( x,y,z ) ); }; // sets LookAt vector
		void SetUpVector( D3DXVECTOR3 Vector ); // sets UpVector vector
		void SetUpVector( float x, float y, float z ) { SetUpVector( D3DXVECTOR3( x,y,z ) ); }; // sets UpVector vector
		void MoveCamera( int Direction ); //moves a camera
		void RotateCamera( int Direction ); //rotates a camera
		void RotateCameraM( int Direction, float Angle ); //rotates a camera - mouse
		D3DXVECTOR3 * GetEyePtVector( D3DXVECTOR3 *vec ); // returns EyePt vector
		D3DXVECTOR3 * GetLookAtVector( D3DXVECTOR3 *vec ); // returns LookAt vector
		D3DXVECTOR3 * GetUpVector( D3DXVECTOR3 *vec ); // returns UpVector vector
		D3DXVECTOR3 GetEyePtVector() { return EyePt; }; // returns EyePt vector
		D3DXVECTOR3 GetLookAtVector() { return LookAt; }; // returns LookAt vector
		D3DXVECTOR3 GetUpVector() { return UpVector; }; // returns UpVector vector

		void Reset (void);

	private:
		// variables
		D3DXVECTOR3 EyePt, LookAt, UpVector;
	};

} // end of namespace graphic