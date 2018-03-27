#include "stdafx.h"
#include "BoundingEnvelopes.h"

using namespace graphic;



//////////////////////////////////////////////////////////////////////////////////////////////
//
// static Start method is a callback function that clears object's structures
// when calling this, pass a valid pointer to an appropriate CBoundingEnvelopes object
//
// when successfull, this returns zero, otherwise an error code is returned
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CALLBACK CBoundingEnvelopes::Start( CBoundingEnvelopes * _this )
{
#ifdef MY_DEBUG_VERSION
	if ( _this->State == ST_Undefined )
	{
		OUTMSG( "Warning: CBoundingEnvelopes::Start() - structure not initialized!", 1 );
		return ERRNOTPREPARED;
	}
#endif

	_this->Triangles.clear();
	SAFE_RELEASE( _this->pVertexBuffer );
	_this->State = ST_Started;

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// static Finish method is a callback function that creates a vertex buffer and fills it with
//   data obtained by preceding ProcessFace calls
// this also shifts the object to the ST_Prepared state 
// when calling this, pass a valid pointer to an appropriate CBoundingEnvelopes objects
//
// when successfull, this returns zero, otherwise an error code is returned
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CALLBACK CBoundingEnvelopes::Finish( CBoundingEnvelopes * _this )
{
#ifdef MY_DEBUG_VERSION
	if ( _this->State != ST_Started )
	{
		OUTMSG( "Warning: CBoundingEnvelopes::Finish() - not prepared for conversion!", 1 );
		return ERRNOTPREPARED;
	}
#endif

	HRESULT			hr;
	VERTEX		*	v;
	int				index;
	TRIANGLE_ITER	iter;
	UINT			count;

	count = (UINT) _this->Triangles.size();
	if ( count <= 0 ) count = 1;

	hr = _this->D3DDevice->CreateVertexBuffer( count * 3 * sizeof( VERTEX ), 0, VertexFVF, D3DPOOL_MANAGED, &(_this->pVertexBuffer), NULL );
	if ( hr ) ERRORMSG( hr, "CBoundingEnvelopes::Finish()", "Bounding volume vertex buffer could not be created." );

	hr = _this->pVertexBuffer->Lock( 0, 0, (LPVOID *) &v, 0 );
	if ( hr ) ERRORMSG( hr, "CBoundingEnvelopes::Finish()", "Bounding volume vertex buffer could not be locked." );
    
	index = 0;
	for ( iter = _this->Triangles.begin(); iter != _this->Triangles.end(); iter++ )
	{
		v[index].x = iter->verts[0].x;
		v[index].y = iter->verts[0].y;
		v[index++].z = iter->verts[0].z;

		v[index].x = iter->verts[1].x;
		v[index].y = iter->verts[1].y;
		v[index++].z = iter->verts[1].z;

		v[index].x = iter->verts[2].x;
		v[index].y = iter->verts[2].y;
		v[index++].z = iter->verts[2].z;
	}
    
	_this->pVertexBuffer->Unlock();

	_this->State = ST_Prepared;

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// static ProcessFace method is a callback function that processes single face
//   the face could be any planar convex polygon, so this adds n-2 triangles to the Triangles
//   list, where n is the number of vertices of the polygon
// you have to set iVertCount parameter to the count of vertices (n) and pass vertex coordinates
//   in a field of double triads in clockwise order (to define face normal)
// when calling this, pass a valid pointer to an appropriate CBoundingEnvelopes object (_this)
//
// when successfull, this returns zero, otherwise an error code is returned
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CALLBACK CBoundingEnvelopes::ProcessFace( CBoundingEnvelopes * _this, UINT uiVertCount, float * pBuf )
{
	TRIANGLE	tri;
	
	for ( UINT i = 2; i < uiVertCount; i++ )
	{
		tri.verts[0].x = (float) pBuf[0];
		tri.verts[0].y = (float) pBuf[1];
		tri.verts[0].z = (float) pBuf[2];
		
		tri.verts[1].x = (float) pBuf[(i-1)*3];
		tri.verts[1].y = (float) pBuf[(i-1)*3 + 1];
		tri.verts[1].z = (float) pBuf[(i-1)*3 + 2];

		tri.verts[2].x = (float) pBuf[i*3];
		tri.verts[2].y = (float) pBuf[i*3 + 1];
		tri.verts[2].z = (float) pBuf[i*3 + 2];
		_this->Triangles.push_back( tri );
	}

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CBoundingEnvelopes::CBoundingEnvelopes()
{
	State = ST_Undefined;
	pVertexBuffer = NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CBoundingEnvelopes::~CBoundingEnvelopes()
{
	Triangles.clear();
	State = ST_Undefined;
	SAFE_RELEASE( pVertexBuffer );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this intializes the object
//
// when successfull, this returns zero, otherwise an error code is returned
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CBoundingEnvelopes::Init( LPDIRECT3DDEVICE9 _D3DDevice )
{
	D3DDevice = _D3DDevice;
	
	Triangles.clear();
	State = ST_Initialized;
	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Render this method renders all the bounding envelopes as wireframes
//
// when successfull, this returns zero, otherwise an error code is returned
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CBoundingEnvelopes::Render()
{
#ifdef MY_DEBUG_VERSION
	if ( State != ST_Prepared )
	{
		OUTMSG( "Warning: CBoundingEnvelopes::Render() - not prepared for rendering!", 1 );
		return ERRNOTPREPARED;
	}
#endif

	DWORD		_stat1, _stat2;

	D3DDevice->GetRenderState( D3DRS_FILLMODE, &_stat1 );
	D3DDevice->GetRenderState( D3DRS_CULLMODE, &_stat2 );

	D3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	D3DDevice->SetFVF( VertexFVF );
	D3DDevice->SetStreamSource( 0, pVertexBuffer, 0, sizeof( VERTEX ) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, (UINT) Triangles.size() );

	D3DDevice->SetRenderState( D3DRS_FILLMODE, _stat1 );
	D3DDevice->SetRenderState( D3DRS_CULLMODE, _stat2 );

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
