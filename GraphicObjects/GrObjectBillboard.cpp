#include "stdafx.h"
#include "GrObjectBillboard.h"

using namespace graphic;


int CGrObjectBillboard::iReferences = 0;
IDirect3DVertexBuffer9 * CGrObjectBillboard::pVertexBuffer = NULL;
D3DXVECTOR3 CGrObjectBillboard::ViewerPosition = D3DXVECTOR3( 0, 0, 0 );


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectBillboard::CGrObjectBillboard()
{
	// call inherited constructor

	// sets all properties to default values
	PreInitThis();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectBillboard::~CGrObjectBillboard()
{
	Free();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// private method called by constructor
// this initializes all internal structures to default values
// !!! this doesn't call inherited PreInit() method
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::PreInitThis()
{
	ridTexture = -1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this initializes all internal structures to default values
// !!! also calls PreInit() method of all ancestors
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::PreInit()
{
	// calls inherited PreInit() method
	this->CGrObjectBase::PreInit();

	// init own data
	PreInitThis();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases all structures created by Init() or other calls
// this is called by destructor, but could be used separately for this object's reuse
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::Free()
{
	if ( ridTexture != -1 ) ResourceManager->ReleaseResource( ridTexture );
	if ( --iReferences <= 0 ) SAFE_RELEASE( pVertexBuffer );

	// call inherited dealocator
	this->CGrObjectBase::Free();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this method loads the specified texture, the static parameter is irrelevant as like as
//   the LODcount - this object is neither a static nor dynamic - it's always faced towards
//   the viewer; it can be moved throughout the time, but it is not supposed to
// this also creates the vertex buffer, but only when called by the first instance, all
//   the rest of instances of this class only increases usage counter 'iReferences'
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectBillboard::Init(	BOOL Static,
									CString texFileName,
									UINT LODcount )
{
	HRESULT			hr;

	hr = InternalInit( Static, LODcount );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectBillboard::Init()", "An error occurs while initializing global parts of the object." );

	// load the billboard texture
	ridTexture = ResourceManager->LoadResource( texFileName, RES_Texture, true, NULL );
	if ( ridTexture < 0 ) ERRORMSG( ERRGENERIC, "CGrObjectBillboard::Init()", "Unable to load billboard texture." );


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this method loads the specified texture, the static parameter is irrelevant as like as
//   the LODcount - this object is neither a static nor dynamic - it's always faced towards
//   the viewer; it can be moved throughout the time, but it is not supposed to
// this also creates the vertex buffer, but only when called by the first instance, all
//   the rest of instances of this class only increases usage counter 'iReferences'
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectBillboard::Init(	BOOL Static,
									resManNS::RESOURCEID rid,
									UINT LODcount )
{
	HRESULT			hr;

	hr = InternalInit( Static, LODcount );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectBillboard::Init()", "An error occurs while initializing global parts of the object." );

	// load the billboard texture
	ResourceManager->DuplicateResource( rid );
	ridTexture = rid;
	if ( ridTexture < 0 ) ERRORMSG( ERRGENERIC, "CGrObjectBillboard::Init()", "Unable to init billboard texture." );


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this method performs the main part of all Init methods - the static parameter is irrelevant 
//   as like as the LODcount - this object is neither a static nor dynamic - it's always faced 
//   towards the viewer; it can be moved throughout the time, but it is not supposed to
// this also creates the vertex buffer, but only when called by the first instance, all
//   the rest of instances of this class only increases usage counter 'iReferences'
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectBillboard::InternalInit( BOOL Static, UINT LODcount )
{
	HRESULT			hr;
	VERTEX		*	pVertices;
	

	// use inherited initializator
	hr = this->CGrObjectBase::Init( Static );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectBillboard::InternalInit()", "An error occurs while initializing global parts of the object." );

	// sets all properties to default values
	PreInitThis();

	
	// create the billboard vertex buffer and fill it with proper data (simple unitary quad)
	if ( iReferences <= 0 ) 
	{
		hr = D3DDevice->CreateVertexBuffer( 4 * sizeof(VERTEX), 0, VertexFVF, D3DPOOL_MANAGED, &pVertexBuffer, NULL );
		if ( hr ) ERRORMSG( hr, "CGrObjectBillboard::InternalInit()", "Unable to create vertex buffer for billboards." );

		hr = pVertexBuffer->Lock( 0, 0, (LPVOID *) &pVertices, 0 );
		if ( hr ) ERRORMSG( hr, "CGrObjectBillboard::InternalInit()", "Unable to create initialize vertex buffer for billboards." );

		pVertices[0].pos = D3DXVECTOR3( -0.5f, -0.5f, 0 );
		pVertices[0].texcoord = D3DXVECTOR2( 0, 1 );
		pVertices[1].pos = D3DXVECTOR3( -0.5f,  0.5f, 0 );
		pVertices[1].texcoord = D3DXVECTOR2( 0, 0 );
		pVertices[2].pos = D3DXVECTOR3(  0.5f, -0.5f, 0 );
		pVertices[2].texcoord = D3DXVECTOR2( 1, 1 );
		pVertices[3].pos = D3DXVECTOR3(  0.5f,  0.5f, 0 );
		pVertices[3].texcoord = D3DXVECTOR2( 1, 0 );

		pVertexBuffer->Unlock();
	}
	
	iReferences++;


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// rotates the object to be face towards the camera and renders it
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectBillboard::Render()
{
	resManNS::Texture *	pTextureStruct;

	CountWorldMat();

	Shader->SetWorldMatrix( &WorldMat );
	
	pTextureStruct = ResourceManager->GetTexture( ridTexture );
	D3DDevice->SetTexture( 1, pTextureStruct->texture );
	D3DDevice->SetStreamSource( 0, pVertexBuffer, 0, sizeof( VERTEX ) );
	D3DDevice->SetFVF( VertexFVF );
	
	return D3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// DERIVED METHODS THAT HANDLES OBJECT'S WORLD COORDINATES MUST BE REDEFINED
//
//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////
// InitialWorldPlacement can be now called as many times as you want 
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::InitialWorldPlacement( D3DXVECTOR3 & position, D3DXVECTOR3 & rotation, D3DXVECTOR3 & scale )
{
	Position = position;
	Scaling.x = scale.x;
	Scaling.y = scale.y;
	WorldMatNeedRecount = true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of the standard SetPosition method - works in both modes the same way
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::SetPosition( FLOAT x, FLOAT y, FLOAT z )
{
	Position = D3DXVECTOR3( x, y, z );
	WorldMatNeedRecount = true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of the standard SetPosition method - works in both modes the same way
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::SetPosition( D3DXVECTOR3 & vec )
{
	Position = vec;
	WorldMatNeedRecount = true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of the standard SetScaling method - changes scaling only in X and Y axis
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::SetScaling( FLOAT x, FLOAT y )
{
	Scaling.x = x;
	Scaling.y = y;
	WorldMatNeedRecount = true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of the standard SetScaling method - changes scaling only in X and Y axis
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::SetScaling( FLOAT scale )
{
	Scaling.x = scale;
	Scaling.y = scale;
	WorldMatNeedRecount = true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Sets the object width, which actually means this changes the X component of Scale vector
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::SetWidth( FLOAT width )
{
	Scaling.x = width;
	WorldMatNeedRecount = true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Sets the object height, which actually means this changes the Y component of Scale vector
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::SetHeight( FLOAT height )
{
	Scaling.y = height;
	WorldMatNeedRecount = true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Sets the actual viewer position for all billboard instances
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::SetViewerPosition( D3DXVECTOR3 & eyePt )
{
	ViewerPosition = eyePt;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of standard method counts the world matrix on the base of object's world
//   position, 2D scale vector AND viewer position
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBillboard::CountWorldMat()
{
	D3DXMATRIX				mat, mat2, mat3, mat4;
	D3DXVECTOR3				vec, look;
	FLOAT					r, a, b;

	
	// compute billboard rotation
	look = ViewerPosition - Position;

	r = sqrtf( look.x * look.x + look.z * look.z );
	a = acosf( ABS( look.z / r ) );
	r = sqrtf( look.z * look.z + look.y * look.y );
	b = acosf( ABS( look.z / r ) );

	Rotation.x = look.y * look.z < 0 ? b : -b;
	Rotation.y = look.x * look.z > 0 ? a : -a;
	

	// perform   Scale * RotY * RotX * Trans   multiplication and set the result as WorldMat
	D3DXMatrixRotationX( &mat, Rotation.x );				// set RotX matrix
	D3DXMatrixRotationY( &mat2, Rotation.y );				// set RotY matrix
	D3DXMatrixMultiply( &mat3, &mat2, &mat );				// compute RotY * RotX
	D3DXMatrixScaling( &mat4, Scaling.x, Scaling.y, 1 );	// set Scale matrix
	D3DXMatrixMultiply( &mat, &mat4, &mat3 );				// compute Scale * Rot
	D3DXMatrixTranslation( &mat4, Position.x, Position.y, Position.z ); // set Translation matrix
	D3DXMatrixMultiply( &WorldMat, &mat, &mat4 );			// compute Scale * Rot * Trans


	SetMapPositionAsWorldPosition();
	WorldMatNeedRecount = FALSE;

};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of the standard GetPosition method - doesn't have two modes
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 CGrObjectBillboard::GetPosition()
{
	return Position;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of the standard GetScaling method - returns only X and Y, Z is always 1
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 CGrObjectBillboard::GetScaling()
{
	return D3DXVECTOR3( Scaling.x, Scaling.y, 1.0f );
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Version of the standard GetScaling method, but returns only 2D vector (X,Y)
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR2 CGrObjectBillboard::GetScalingXY()
{
	return D3DXVECTOR2( Scaling.x, Scaling.y );
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of the standard GetRotation method - returns only recomputed X and Y angles,
//   Z angle is always zero. Moreover, the WorldMat is always recomputed by this call.
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 CGrObjectBillboard::GetRotation()
{
	CountWorldMat();
	
	return D3DXVECTOR3( Rotation.x, Rotation.y, 0 );
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of the standard GetWorldMat method - this works in both modes the same way.
// Moreover, the WorldMat is always recomputed by this call.
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXMATRIX CGrObjectBillboard::GetWorldMat()
{
	CountWorldMat();

	return WorldMat;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Redefinition of the standard GetWorld method - this works in both modes the same way.
// Moreover, the WorldMat is always recomputed by this call.
//////////////////////////////////////////////////////////////////////////////////////////////
const D3DXMATRIX * CGrObjectBillboard::GetWorldMatPtr()
{
	CountWorldMat();

	return &WorldMat;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
