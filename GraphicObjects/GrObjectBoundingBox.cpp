#include "stdafx.h"
#include "GrObjectBoundingBox.h"

using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectBoundingBox::CGrObjectBoundingBox()
{
	// call inherited constructor
	VertexBuffer = NULL;
	IndexBuffer = NULL;
	uiNodeLevelsCount = 0;
	uiNodesCount = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectBoundingBox::~CGrObjectBoundingBox()
{
	SAFE_RELEASE( VertexBuffer );
	SAFE_RELEASE( IndexBuffer );
	// call inherited destructor
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads the box from a specified file and creates the proper vertex and index buffers
// returns a nonzero value when any error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectBoundingBox::Init(	BOOL Static,
									CString fileName,
									UINT LODcount )
{
	HRESULT		hr;

	// use inherited initializator
	hr = this->CGrObjectBase::Init( Static );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectBoundingBox::Init()", "An error occurs while initializing global parts of the object." );


	hr = ReInit( fileName );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectBoundingBox::Init()", "Could not initialize bounding box object." );


	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all needed states and renders the box hierarchy, then restores all saved state
// returns a nonzero value when any error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectBoundingBox::Render()
{
	D3DMATERIAL9	material;

#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	if ( WorldMatNeedRecount ) CountWorldMat();
#endif
	
	// place an object to scene
	if ( bVertexShader )
		Shader->SetWorldMatrix( &WorldMat );
	else
		D3DDevice->SetTransform( D3DTS_WORLD, &WorldMat );

	D3DDevice->SetFVF( VertexFVF );
	D3DDevice->SetStreamSource( 0, VertexBuffer, 0, sizeof( VERTEX ) );
	D3DDevice->SetIndices( IndexBuffer );

	D3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
	D3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	D3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
	D3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	ZeroMemory( &material, sizeof( material ) );
	material.Ambient.r = 1.0;
	material.Ambient.g = 1.0;
	material.Ambient.b = 0.0;
	material.Ambient.a = 1.0;
	material.Diffuse.r = 1.0;
	material.Diffuse.g = 1.0;
	material.Diffuse.b = 0.0;
	material.Diffuse.a = 1.0;

	D3DDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
	D3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
	D3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
	D3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );

	if ( bVertexShader )
		Shader->SetVSMaterial(&material);
	else
		D3DDevice->SetMaterial( &material );
	
	D3DDevice->DrawIndexedPrimitive( D3DPT_LINELIST, 0, 0, 8 * uiNodesCount, 0, uiNodesCount * 12 );


	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases actual buffers and performs new initialization with loading new bounding
// box from specified file
// can't be used befor calling Init() method !!!!!
//
// returns a nonzero value when any error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectBoundingBox::ReInit( LPCTSTR fileName )
{
	HRESULT		hr;

	hr = LoadFromFile( fileName );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectBoundingBox::Init()", "Could not initialize bounding box object." );


	hr = InitBuffers();
	if ( hr ) ERRORMSG( hr, "CGrObjectBoundingBox::LoadFromFile()", "Could not initialize buffers." );

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads the bounding box hierarchy from a file and inits the attached buffer 
// returns a nonzero value when any error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectBoundingBox::LoadFromFile( LPCTSTR fileName )
{
	SAFE_DELETE( BoundingBox );
	
	BoundingBox = CBoundingBox::Load( fileName );
	if ( !BoundingBox ) ERRORMSG( ERRGENERIC, "CGrObjectBoundingBox::LoadFromFile()", "Could not load the bounding box." );

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// creates vertex and index buffers and fills them with a proper bounding box vertices data
// returns a nonzero value when any error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectBoundingBox::InitBuffers()
{
	HRESULT				hr;
	VERTEX			*	v;
	DWORD			*	ind;
	D3DXMATRIX			mat;
	D3DXVECTOR3			vec;
	CBoundingBox	*	pBox;
	std::vector<CBoundingBox*>::iterator	iter;
	std::vector<BBSINGLENODE>				Boxes;
	std::vector<BBSINGLENODE>::iterator		nodeIter;
	BBSINGLENODE		BoxNode;
	UINT				nodesCount, vertIndex, index;



	// create vertex buffer
	hr = D3DDevice->CreateVertexBuffer(	8 * BoundingBox->GetNodesCount() * sizeof( VERTEX ), 
										D3DUSAGE_WRITEONLY, VertexFVF, D3DPOOL_MANAGED, &VertexBuffer, 0);
	if ( hr ) ERRORMSG( hr, "CGrObjectBoundingBox::InitBuffers()", "Couldn't create bounding box's vertex buffer." );

	
	// create index buffer
	hr = D3DDevice->CreateIndexBuffer(	24 * BoundingBox->GetNodesCount() * sizeof( DWORD ), 
										D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &IndexBuffer, 0);
	if ( hr ) ERRORMSG( hr, "CGrObjectBoundingBox::InitBuffers()", "Couldn't create bounding box's index buffer." );
				

	// lock vertex buffer to be able to insert vertices into it
	hr = VertexBuffer->Lock( 0, 0, (LPVOID *) &v, 0 );
	if ( hr ) ERRORMSG( hr, "CGrObjectBoundingBox::InitBuffers()", "Couldn't lock bounding box's vertex buffer." );
	


	// lock index buffer to be able to insert indices into it
	hr = IndexBuffer->Lock(0, 0, (void **) &ind, 0);
	if ( hr ) ERRORMSG( hr, "CGrObjectBoundingBox::InitBuffers()", "Couldn't lock bounding box's index buffer." );
	




	// now, fill the vertex buffer with transformed vertices and the index buffer with indices
	// walking thru the BB tree hierarchy level by level using an intermediate treelevel-sorted queue
	NodesCountOnEachLevel.clear();
	nodesCount = uiNodesCount = 0;
	uiNodeLevelsCount = 1;

	BoxNode.Level = 1;
	BoxNode.pBox = BoundingBox;
	BoxNode.Matrix = BoundingBox->Matrix;

	Boxes.push_back( BoxNode );
	
	vertIndex = index = 0;
	nodeIter = Boxes.begin();
	while ( nodeIter != Boxes.end() )
	{
		// test, if a new nodes level (tree layer) starts here
		if ( uiNodeLevelsCount < nodeIter->Level ) 
		{
			NodesCountOnEachLevel.push_back( nodesCount );
			nodesCount = 0;
			uiNodeLevelsCount++;
		}
		
		// add all child nodes to the queue
		pBox = nodeIter->pBox;
		for ( iter = pBox->pNodes.begin(); iter != pBox->pNodes.end(); iter++ )
		{
			BoxNode.Level = nodeIter->Level + 1;
			BoxNode.Matrix = nodeIter->Matrix * (*iter)->Matrix;
			BoxNode.pBox = *iter;
			Boxes.push_back( BoxNode );
			nodeIter = Boxes.begin();
		}

		// add this node's box (its vertices) to the vertex buffer
		for ( UINT i = 0; i < 8; i++ )
		{
			D3DXVec3TransformCoord( &vec, &pBox->Corners[i],  &nodeIter->Matrix );
			v[vertIndex].x = vec.x;
			v[vertIndex].y = vec.y;
			v[vertIndex].z = vec.z;
			vertIndex++;
		}

		// add indices of lines vertices to the index buffer
		ind[index++] = vertIndex - 8; ind[index++] = vertIndex - 7;
		ind[index++] = vertIndex - 7; ind[index++] = vertIndex - 6;
		ind[index++] = vertIndex - 6; ind[index++] = vertIndex - 5;
		ind[index++] = vertIndex - 5; ind[index++] = vertIndex - 8;

		ind[index++] = vertIndex - 4; ind[index++] = vertIndex - 3;
		ind[index++] = vertIndex - 3; ind[index++] = vertIndex - 2;
		ind[index++] = vertIndex - 2; ind[index++] = vertIndex - 1;
		ind[index++] = vertIndex - 1; ind[index++] = vertIndex - 4;

		ind[index++] = vertIndex - 8; ind[index++] = vertIndex - 3;
		ind[index++] = vertIndex - 7; ind[index++] = vertIndex - 4;
		ind[index++] = vertIndex - 6; ind[index++] = vertIndex - 1;
		ind[index++] = vertIndex - 5; ind[index++] = vertIndex - 2;
 
		// increase counters
		nodeIter = Boxes.begin();
		nodeIter = Boxes.erase( nodeIter );
		nodesCount++;
		uiNodesCount++;
	} 

	NodesCountOnEachLevel.push_back( nodesCount ); // last node level addition
	uiNodeLevelsCount++;


	
	// lock both buffers
	VertexBuffer->Unlock();
	IndexBuffer->Unlock();


	return ERRNOERROR;
}
