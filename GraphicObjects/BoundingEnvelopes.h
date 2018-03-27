/*
	BoundingEnvelopes:	This class is used for storage and rendering of all bounding boxes and bounding volumes in the 
						scene. It is supposed to be filled with actualized list of faces every frame and this class renders 
						them. All data are supposed to be obtained in world coordinates.
						The whole functionality is as follows:
						  1) INIT - this object has to be initialized with D3DDevice pointer
						  2) START FRAME - the Start() method has to be called before any face is added
						  3) ADD FACE - for evry face a ProcessFace() method is called that triangulates the face and add all triangles to the list
						  4) FINISH - a Finish() method is called so that this object can create and fill the vertex buffer with proper data
						  5) RENDER - Render method is called so all the prepared data are rendered
					
	Creation Date:		8.11.2005
	Last Update:		8.11.2005
	Author:				Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\Graphic\Shaders.h"
#include "GrObjectBase.h"



namespace graphic
{


	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// CBoundingEnvelopes class
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CBoundingEnvelopes
	{
		// simple pretransformed vertex with world coordinates
		struct VERTEX
		{
			float			x, y, z;
		} ;
		static const DWORD	VertexFVF = ( D3DFVF_XYZ );

		// single triangle structure contains 3 vertices in world coordinates
		struct TRIANGLE 
		{
			D3DXVECTOR3			verts[3];
		} ;

		typedef std::vector<TRIANGLE>			TRIANGLELIST;
		typedef std::vector<TRIANGLE>::iterator	TRIANGLE_ITER;
		typedef enum StateType
		{
			ST_Undefined,
			ST_Initialized,
			ST_Started,
			ST_Prepared
		} ;

	public:
								CBoundingEnvelopes();
								~CBoundingEnvelopes();

		HRESULT					Init( LPDIRECT3DDEVICE9 _D3DDevice );
		
		static HRESULT CALLBACK	Start( CBoundingEnvelopes * _this );
		static HRESULT CALLBACK	Finish( CBoundingEnvelopes * _this );
		static HRESULT CALLBACK	ProcessFace( CBoundingEnvelopes * _this, UINT uiVertCount, float * pBuf );
		
		HRESULT					Render();
		               
		
	private:
		LPDIRECT3DDEVICE9		D3DDevice; // pointer to a D3DDevice object
		TRIANGLELIST			Triangles; // list of triangles
		StateType				State; // defines the state this object is in ()
		LPDIRECT3DVERTEXBUFFER9	pVertexBuffer; // contain vertex data when the object is in ST_Prepared state

	} ;

	
	typedef CBoundingEnvelopes	*LPBOUNDINGENVELOPES;

} // end of namespace