#include "stdafx.h"
#include "ShadowVolume.h"


using namespace graphic;



//////////////////////////////////////////////////////////////////////////////////////////////
//
// CShadowVolume constructor zeroes the buffer
//
//////////////////////////////////////////////////////////////////////////////////////////////
CShadowVolume::CShadowVolume()
{
	this->D3DDevice = NULL;
	//this->pVertices = NULL;
	this->pEdgeLists = NULL;
	this->uiNumVertices = 0;
	this->uiVerticesAllocated = 0;
	this->uiEdgeListsAllocated = 0;
	this->uiNumBorderEdges = 0;
	pVertexBuffer = NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// CShadowVolume destructor frees the vertices memory
//
//////////////////////////////////////////////////////////////////////////////////////////////
CShadowVolume::~CShadowVolume()
{
	//SAFE_FREE( pVertices );
	if ( pEdgeLists ) delete[] pEdgeLists;
	SAFE_RELEASE( pVertexBuffer );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// CEdgeList constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CShadowVolume::CEdgeList::CEdgeList()
{
	pEdges = (MESHEDGE *) malloc( 2 * sizeof(MESHEDGE) );
	if ( !pEdges )
	{
		ErrorHandler.HandleError(	ERROUTOFMEMORY,
									"CShadowVolume::CEdgeList::CEdgelist()",
									"Edge list could not be created because of insufficient memory." );
		uiAllocatedCount = 0;
		return;
	}

	uiAllocatedCount = 2;
	uiCount = 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// if the specified edge (oriented edge that is specified by vertex indices ind0-ind1) doesn't
// exist in pEdges list, this will add it to this list of silhouette edges, otherwise this will
// remove it, because it's not a silhouette edge
//
// if there is no more space in the array, it automatically grows up;
//
// returns updated count of edges that are stored as possibly border-edges
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CShadowVolume::CEdgeList::AddEdge( UINT &uiNumEdges, WORD ind0, WORD ind1 )
{
	for ( UINT i = 0; i < uiCount; i++ )
	{
		if ( ( pEdges[i].i2 == ind1 && pEdges[i].i1 == ind0 ) ||
			 ( pEdges[i].i1 == ind1 && pEdges[i].i2 == ind0 ) )
		{ 
			// the same egde found, both of them are throwed out
			pEdges[i].i1 = pEdges[uiCount - 1].i1;
			pEdges[i].i2 = pEdges[uiCount - 1].i2;
			uiNumEdges--;
			uiCount--;

			return ERRNOERROR;
		}
	}

	// this edge hasn't a twin, it's to be added to the list
	// but check first, if ther's enough space
	if ( uiCount == uiAllocatedCount )
	{
		pEdges = (MESHEDGE *) realloc( pEdges, 2 * uiAllocatedCount * sizeof(MESHEDGE) );
		if ( !pEdges ) ERRORMSG(	ERROUTOFMEMORY, 
									"CShadowVolume::CEdgeList::AddEdge()",
									"Can not add edge due to unsuccessful attempt to reallocate memory." );
		uiAllocatedCount *= 2;
	}

	pEdges[uiCount].i1 = ind0;
	pEdges[uiCount].i2 = ind1;
	uiCount++;
	uiNumEdges++;

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// creates a volume from mesh data and light position
// pObjMat specifies the worldmat of object that is processed
// use bStatic parameter to tell that this shadow volume will be static and therefore it's
//   computation could take longer time and therefore could be more precise
// use bDirectional parameter to tell that the light source is directional and so the 
//   pLightPos vector represents light's direction in place of light's source position
//
// this returns a nonzero error value when any error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CShadowVolume::Init(	LPDIRECT3DDEVICE9 Direct3DDevice,
								LPD3DXMESH pMesh,
								D3DXVECTOR3 *pLightPos,
								D3DXMATRIXA16 *pObjMat,
								bool bStatic,
								bool bDirectional )
{
	MESHVERTEX		*	pMeshVertices;
	TRANSFORMEDVERTEX *	pTransVertices;
	WORD			*	pMeshIndices;
	DWORD			*	pMeshIndicesDW; // double word version used when the buffer has more than 64k vertices
	BOOL				bDoubleSizedIB;
	UINT				uiNumFaces, uiNumVerticesInVB;
	HRESULT				hr;
	D3DXMATRIXA16		matrix;
	D3DXVECTOR3			lightPos;
	D3DXVECTOR3			lightDir;
	DWORD				ind0, ind1, ind2; // indices to vertex buffer
	D3DXVECTOR3			vert0, vert1, vert2; // vertices of a triangle
	D3DXVECTOR3			vec0, vec1, vec2, vec3, norm; // vectors
	D3DXVECTOR3			zero = D3DXVECTOR3( 0, 0, 0 );



	this->D3DDevice = Direct3DDevice;
	if ( !bStatic ) WorldMat = *pObjMat;
	else D3DXMatrixIdentity( &WorldMat );
	
	if ( bDirectional )
	{
		lightDir = *pLightPos;
		//D3DXVec3Normalize( &lightDir, pLightPos );
		//lightDir *= 100 * ONE_METER;
	}


	uiNumFaces = pMesh->GetNumFaces();
	uiNumVerticesInVB = pMesh->GetNumVertices();

	bDoubleSizedIB = uiNumVerticesInVB > 65535;

	
	// first, allocate memory for edge lists
	if ( uiEdgeListsAllocated < uiNumVerticesInVB )
	{
		if ( pEdgeLists ) // free the edgelist field, because it exists but is of insufficient size
		{
			delete[] pEdgeLists;
			pEdgeLists = NULL;
		}
		pEdgeLists = new CEdgeList[uiNumVerticesInVB];
		uiEdgeListsAllocated = uiNumVerticesInVB;
	}
	else
	{
		for ( UINT i = 0; i < uiNumVerticesInVB; i++ ) pEdgeLists[i].uiCount = 0;
	}
	
	if ( !pEdgeLists )
	{
		uiEdgeListsAllocated = 0;	
		ERRORMSG( ERROUTOFMEMORY, "CShadowVolume::Init()", "Shadow volume can not be created because of insufficient memory." );
	}


	

	// prepare mesh vertex buffers
	hr = pMesh->LockVertexBuffer( D3DLOCK_READONLY, (LPVOID *) &pMeshVertices );
	if ( bDoubleSizedIB ) hr += pMesh->LockIndexBuffer( D3DLOCK_READONLY, (LPVOID *) &pMeshIndicesDW );
	else hr += pMesh->LockIndexBuffer( D3DLOCK_READONLY, (LPVOID *) &pMeshIndices );

	if ( hr ) ERRORMSG( ERRGENERIC, "CShadowVolume::Init()", "One of the mesh buffers could not be locked." );

	
	uiNumBorderEdges = 0; // count of edges in pEdges field - potential border edges
	



	// Add all silhouette edges to the edge list, but their processing depends on 
	//   shadow volume stationariness:
	//     If the shadowing object is not static, the shadow volume is probably being
	//       recomputed very often and therefore it should be done as fast as possible
	//     Another way is performing more precise computations that minimizes shadow volume size
	if ( !bStatic ) 
	{
		
		// transform the light into object's space
		D3DXMatrixInverse( &matrix, NULL, &WorldMat );
		if ( bDirectional )
		{
			D3DXVec3TransformCoord( &vert1, &lightDir, &matrix );
			D3DXVec3TransformCoord( &vert0, &zero, &matrix );
			D3DXVec3Normalize( &lightDir, &(D3DXVECTOR3( vert1 - vert0 )) );
			lightDir *= 20 * ONE_METER;
			lightPos = -10*lightDir;
		}
		else
		{
			D3DXVec3TransformCoord( &lightPos, pLightPos, &matrix );
			lightDir = -lightPos;
    	}



		// now, process each face of mesh
		// WARNING! it's not certain that the index buffer is made of TRIANGLELIST !
 		for ( UINT i = 0; i < uiNumFaces; i++ )
		{		
			// store triangle (face) vertex indices
			if ( bDoubleSizedIB ) 
			{
				ind0 = pMeshIndicesDW[i*3];
				ind1 = pMeshIndicesDW[i*3 + 1];
				ind2 = pMeshIndicesDW[i*3 + 2];
			}
			else
			{
				ind0 = pMeshIndices[i*3];
				ind1 = pMeshIndices[i*3 + 1];
				ind2 = pMeshIndices[i*3 + 2];
			}
			
			// store triangle (face) vertex coordinates
			vert0 = pMeshVertices[ind0].pos;
			vert1 = pMeshVertices[ind1].pos;
			vert2 = pMeshVertices[ind2].pos;

			// compute face normal
			vec0 = vert0 - vert1;
			vec1 = vert2 - vert1;
			D3DXVec3Cross( &norm, &vec0, &vec1 );

			// check the angle between light vector and face normal vector
			if ( D3DXVec3Dot( &norm, &lightPos ) >= 0.0f )
			{
				pEdgeLists[ind0 < ind1 ? ind0 : ind1].AddEdge( uiNumBorderEdges, (WORD) ind0, (WORD) ind1 );
				pEdgeLists[ind1 < ind2 ? ind1 : ind2].AddEdge( uiNumBorderEdges, (WORD) ind1, (WORD) ind2 );
				pEdgeLists[ind2 < ind0 ? ind2 : ind0].AddEdge( uiNumBorderEdges, (WORD) ind2, (WORD) ind0 );
			}
		}

	} // end of dynamic shadow
	else // creating static shadow volume
	{
		pTransVertices = new TRANSFORMEDVERTEX[uiNumVerticesInVB];
		if ( !pTransVertices ) ( ERROUTOFMEMORY, "CShadowVolume::Init()", "Shadow volume can not be created because of insufficient memory." );

		if ( !bDirectional )
		{
			lightPos = *pLightPos;
			D3DXVec3TransformCoord(	&zero, &zero, pObjMat );
			lightDir = zero - lightPos;
		}
		else
		{
			D3DXVec3TransformCoord(	&zero, &zero, pObjMat );
			lightPos = zero - 10 * ONE_KILOMETER * lightDir;
		}
		
		// first, transform all vertices
		D3DXVec3TransformCoordArray(	&pTransVertices[0].pos,
										sizeof( TRANSFORMEDVERTEX ),
										&pMeshVertices[0].pos,
										sizeof( MESHVERTEX ),
										pObjMat,
										uiNumVerticesInVB );
		
		// now, process each face of the mesh
		// WARNING! it's not certain that the index buffer is made of TRIANGLELIST !
 		for ( UINT i = 0; i < uiNumFaces; i++ )
		{		
			// store triangle (face) vertex indices
			if ( bDoubleSizedIB ) 
			{
				ind0 = pMeshIndicesDW[i*3];
				ind1 = pMeshIndicesDW[i*3 + 1];
				ind2 = pMeshIndicesDW[i*3 + 2];
			}
			else
			{
				ind0 = pMeshIndices[i*3];
				ind1 = pMeshIndices[i*3 + 1];
				ind2 = pMeshIndices[i*3 + 2];
			}

			// store triangle (face) vertex coordinates
			vert0 = pTransVertices[ind0].pos;
			vert1 = pTransVertices[ind1].pos;
			vert2 = pTransVertices[ind2].pos;

			// compute face normal
			vec0 = vert0 - vert1;
			vec1 = vert2 - vert1;
			D3DXVec3Cross( &norm, &vec0, &vec1 );

			// check the angle between light vector and face normal vector
			if ( D3DXVec3Dot( &norm, &(D3DXVECTOR3(lightPos - zero)) ) >= 0.0f )
			{
				pEdgeLists[ind0 < ind1 ? ind0 : ind1].AddEdge( uiNumBorderEdges, (WORD) ind0, (WORD) ind1 );
				pEdgeLists[ind1 < ind2 ? ind1 : ind2].AddEdge( uiNumBorderEdges, (WORD) ind1, (WORD) ind2 );
				pEdgeLists[ind2 < ind0 ? ind2 : ind0].AddEdge( uiNumBorderEdges, (WORD) ind2, (WORD) ind0 );
			}
		}
	} // silhouette edges added to the list
	

	
	
	
	uiNumVertices = uiNumBorderEdges * 6;

	// create vertex buffer for shadow volume vertices
	if ( uiVerticesAllocated < uiNumVertices )
	{
		SAFE_RELEASE( pVertexBuffer );
		hr = Direct3DDevice->CreateVertexBuffer( uiNumVertices * sizeof(D3DXVECTOR3), D3DUSAGE_WRITEONLY, D3DFVF_XYZ, D3DPOOL_MANAGED, &pVertexBuffer, NULL );
		if ( hr ) 
		{
			uiVerticesAllocated = 0;
			pMesh->UnlockVertexBuffer();
			pMesh->UnlockIndexBuffer();
			ERRORMSG( ERRGENERIC, "CShadowVolume::Init()", "Shadow volume vertex buffer could not be created." );
		}
		
		uiVerticesAllocated = uiNumVertices;
	}
	
	hr = pVertexBuffer->Lock( 0, 0, (LPVOID *) &pVertices, D3DLOCK_NOSYSLOCK );
	if ( hr ) ERRORMSG( ERRGENERIC, "CShadowVolume::Init()", "Shadow volume vertex buffer could not be accessed." );
	
	



	// now, process all founded silhouette edges to create buffer of vertices
	// preprocess all vertex coordinates
	// again, this process depends on shadow volume stationariness
	if ( !bStatic )
	{
        if ( !bDirectional ) for ( UINT j = 0; j < uiNumVerticesInVB; j++ ) pEdgeLists[j].backVertex = pMeshVertices[j].pos - lightPos * 10;
		else for ( UINT j = 0; j < uiNumVerticesInVB; j++ ) pEdgeLists[j].backVertex = pMeshVertices[j].pos + lightDir;

		
		for ( UINT j = 0, vertInd = 0; j < uiNumVerticesInVB; j++ )
		{
			for ( UINT i = 0; i < pEdgeLists[j].uiCount; i++ )
			{
				pVertices[vertInd++] = pMeshVertices[pEdgeLists[j].pEdges[i].i1].pos;
				pVertices[vertInd++] = pMeshVertices[pEdgeLists[j].pEdges[i].i2].pos;
				pVertices[vertInd++] = pEdgeLists[pEdgeLists[j].pEdges[i].i1].backVertex;

				pVertices[vertInd++] = pMeshVertices[pEdgeLists[j].pEdges[i].i2].pos;
				pVertices[vertInd++] = pEdgeLists[pEdgeLists[j].pEdges[i].i2].backVertex;
				pVertices[vertInd++] = pEdgeLists[pEdgeLists[j].pEdges[i].i1].backVertex;
			}
		}
	}
	else // static shadow volume
	{
		D3DXVECTOR3		dir;
		FLOAT			param;

		for ( UINT j = 0; j < uiNumVerticesInVB; j++ )
		{
/*			pEdgeLists[j].backVertex = pTransVertices[j].pos - lightPos + pTransVertices[j].pos;
		
		
			if ( pEdgeLists[j].backVertex.y < -10 * ONE_CENTIMETER )
			{*/

				if ( bDirectional ) dir = lightDir;
				else
				{
					dir = pTransVertices[j].pos - lightPos;
					D3DXVec3Normalize( &dir, &dir );
					dir *= 100 * ONE_METER;
				}
				param = (- 10 * ONE_METER - pTransVertices[j].pos.y) / dir.y;
				
				pEdgeLists[j].backVertex.y = -10 * ONE_METER;
				pEdgeLists[j].backVertex.x = pTransVertices[j].pos.x + dir.x * param;
				pEdgeLists[j].backVertex.z = pTransVertices[j].pos.z + dir.z * param;
//			}
		}

		for ( UINT j = 0, vertInd = 0; j < uiNumVerticesInVB; j++ )
		{
			for ( UINT i = 0; i < pEdgeLists[j].uiCount; i++ )
			{
				pVertices[vertInd++] = pTransVertices[pEdgeLists[j].pEdges[i].i1].pos;
				pVertices[vertInd++] = pTransVertices[pEdgeLists[j].pEdges[i].i2].pos;
				pVertices[vertInd++] = pEdgeLists[pEdgeLists[j].pEdges[i].i1].backVertex;

				pVertices[vertInd++] = pTransVertices[pEdgeLists[j].pEdges[i].i2].pos;
				pVertices[vertInd++] = pEdgeLists[pEdgeLists[j].pEdges[i].i2].backVertex;
				pVertices[vertInd++] = pEdgeLists[pEdgeLists[j].pEdges[i].i1].backVertex;
			}
		}

		delete[] pTransVertices;
	}



	pMesh->UnlockVertexBuffer();
	pMesh->UnlockIndexBuffer();

	pVertexBuffer->Unlock();


	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders the shadow volume
// use the iWhichSide parameter to define which polygons should be rendered:
//   if the iWhichSide is positive, only front sided polygons are rendered
//   if it is negative, only back sided polygons are rendered
//   if iWhichSide is zero, all (back and front faced) polygons are rendered (default value)
//
// returns zero when successfull or nonzero error code
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CShadowVolume::Render( int iWhichSide )
{
	CGrObjectBase::Shader->SetWorldMatrix( &WorldMat );
	
	if ( iWhichSide > 0 ) FastRender();
	else if ( iWhichSide < 0 )
	{
		D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW ); 
		FastRender();
	    D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	}
	else
	{
		D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ); 
		FastRender();
		D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW ); 
	}

	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders the shadow volume polygons without managing any options or state
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CShadowVolume::FastRender()
{
	D3DDevice->SetFVF( D3DFVF_XYZ );
	D3DDevice->SetStreamSource( 0, pVertexBuffer, 0, sizeof( D3DXVECTOR3 ) );
	return D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, uiNumVertices / 3 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
