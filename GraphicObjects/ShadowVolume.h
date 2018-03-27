/*
	ShadowVolume:	CShadowVolume is created as a volume that is caused by a (light,object) pair. So every light and every object in the scene
					will have one. It will be computed from mesh data and rendered as a half-transparent gray object when covering any face in the scene.
	Creation Date:	25.3.2004
	Last Update:	9.5.2005
	Author:			Roman Margold
*/

#pragma once

#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "GrObjectBase.h"



namespace graphic
{

	// main shadow volume class
	class CShadowVolume
	{

		struct MESHEDGE // structure used to store one edge
		{
			WORD				i1, i2; // indices of vertices, i1 is always less then i2 ( for sorting purposes )
		} ;

		// edge list that can dynamically grow
		class CEdgeList 
		{
		public:
								CEdgeList();
								~CEdgeList() { SAFE_FREE( pEdges ); } ;
			HRESULT				AddEdge( UINT &uiNumEdges, WORD ind0, WORD ind1 ); // adds an edge to a list, grows if needed, returns new count of edges

			MESHEDGE			*pEdges;
			UINT				uiCount; // count of edges in this list
			UINT				uiAllocatedCount; // count of edge fields already allocated
			D3DXVECTOR3			backVertex; // back vertex of shadow polygon made up by an edge with this vertex
		} ;

		
		struct MESHVERTEX // vertex structure of the mesh; WARNING! this structure can differ in some x files
		{
			D3DXVECTOR3		pos, norm;
			FLOAT			tu, tv;
		} ;


		struct TRANSFORMEDVERTEX // vertex structure of the transofrmed mesh
		{
			D3DXVECTOR3		pos;
		} ;


	public:
							CShadowVolume();
							~CShadowVolume();

		HRESULT				Init(	LPDIRECT3DDEVICE9 DirectD3DDevice,
									LPD3DXMESH pMesh,
									D3DXVECTOR3 *pLightPos,
									D3DXMATRIXA16 *pObjMat,
									bool bStatic = false,
									bool bDirectional = false );
		HRESULT				Render( int iWhichSide = 0 );

	private:
		LPDIRECT3DDEVICE9	D3DDevice; // pointer to valid Direct3DDevice9 obtainde in Init method
		D3DXMATRIXA16		WorldMat; // world transforamation matrix of the shadowing object (the one that throws a shadow)
		D3DXVECTOR3			*pVertices; // vertex coordinates are specifying shadow volume faces in thriads (3k, 3k+1, 3k+2)
		CEdgeList			*pEdgeLists; // field of edge lists
		UINT				uiNumVertices; // actual count of vertices
		UINT				uiVerticesAllocated; // count of memory fields that has been allocated for vertices
		UINT				uiEdgeListsAllocated; // count of memory fields that has been allocated for edge lists
		UINT				uiNumBorderEdges; // count of border edges
		LPDIRECT3DVERTEXBUFFER9	pVertexBuffer; // shadow volume vertices

		
		void				AddEdge( WORD* pEdges, UINT& uiNumEdges, WORD ind0, WORD ind1 );
		HRESULT				FastRender();

	} ;

} // end of namespace graphic