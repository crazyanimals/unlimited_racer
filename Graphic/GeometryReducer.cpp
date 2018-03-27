#include "stdafx.h"
#include "GeometryReducer.h"


using namespace graphic;



//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGeometryReducer::CGeometryReducer()
{
	State = GRS_UNDEFINED;
	
	pVertices = NULL;
	pIndices = NULL;
	pEdges = NULL;
	pFaces = NULL;
	pGroups = NULL;
	pNewVertices = NULL;
	pUserDataBlocks = NULL;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor frees all allocated memory
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGeometryReducer::~CGeometryReducer()
{
	Free();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// frees all allocated structures
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGeometryReducer::Free()
{
	if ( pVertices ) { delete [] (char *) pVertices; pVertices = NULL; }
	if ( pIndices ) { delete [] (char *) pIndices; pIndices = NULL; }
	if ( pEdges ) { delete [] (char *) pEdges; pEdges = NULL; }
	if ( pNewVertices ) { delete [] (char *) pNewVertices; pNewVertices = NULL; }
	if ( pUserDataBlocks ) { delete [] (char *) pUserDataBlocks; pUserDataBlocks = NULL; }
	for ( int i = 0; i < iGroupCount; i++ ) pGroups[i].Free();
	if ( pGroups ) { delete [] (char *) pGroups; pGroups = NULL; };

	iFaceCount = 0;
	iIndexCount = 0;
	iVertexCount = 0;
	iEdgeCount = 0;
	iGroupCount = 0;
	iNewVertCount = 0;
	iUserDataCount = 0;

	iVertMemAllocated = 0;
	iIndMemAllocated = 0;
	iEdgeMemAllocated = 0;
	iFaceMemAllocated = 0;
	iGroupMemAllocated = 0;
	iNewVertMemAllocated = 0;
	iUserDataMemAllocated = 0;

	State = GRS_UNDEFINED;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Frees all possibly allocated structures and initializes them again to meet the requirements
//   specified by function parameters; this method is designed so it could be called several
//   times during the object's lifetime, it always resets all states and begins with a new
//   geometry definition.
// This also set the object's state to INITIALIZED; without calling this you can't use any
//   methods like AddFace() ReduceGeometry etc.
//
// Use the faceCount parameter to help this function to estimate maximal vertex count.
//   If you don't know the count of facess you will input, set this parameter to zero or don't
//   use it at all. The same for indicesCount and verticesCount parameters. All will work
//   even if your estimation will be lesser that the real count afterwards. 
//
// Function returns a zero when the initialization finished successfully, or returns nonzero
//   error code.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::Init( int facesCount, int indicesCount, int verticesCount )
{
	HRESULT		hr;
	int			toLoad;
	
	
	Free(); // deinit all structures & set default values

	toLoad = sizeof(GRC_FACE) * ( facesCount ? facesCount : GRC_MIN_FACES_COUNT );
	hr = SatisfyMemoryRequirements( (LPVOID *) &pFaces, &iFaceMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::Init()", "Can't allocate appropriate memory for geometry faces." );

	toLoad = sizeof(GRC_INDEX) * ( indicesCount ? indicesCount : GRC_MIN_INDICES_COUNT );
	hr = SatisfyMemoryRequirements( (LPVOID *) &pIndices, &iIndMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::Init()", "Can't allocate appropriate memory for indexing data." );

	toLoad = sizeof(GRC_VERTEX) * ( verticesCount ? verticesCount : GRC_MIN_VERTICES_COUNT );
	hr = SatisfyMemoryRequirements( (LPVOID *) &pVertices, &iVertMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::Init()", "Can't allocate appropriate memory for vertex data." );

	toLoad = sizeof(GRC_EDGE) * ( indicesCount ? indicesCount : GRC_MIN_INDICES_COUNT ) / 2;
	hr = SatisfyMemoryRequirements( (LPVOID *) &pEdges, &iEdgeMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::Init()", "Can't allocate appropriate memory for mesh data." );

	hr = SatisfyMemoryRequirements( (LPVOID *) &pUserDataBlocks, &iUserDataMemAllocated, sizeof(GRC_USERDATA) );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::Init()", "Can't allocate appropriate memory for custom user data." );

	State = GRS_INITIALIZED;

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// AddFace() method adds a single face to the geometry
// This method can be called only if the object is in INITIALIZED state (that means an Init()
//   method was already called)
//
// Specify vertices in clockwise order. This is necessary for proper face normal computation.
//
// You can attach your custom data to the face or to any of its vertex by setting pUserData 
//   parameter. If you set this parameter to NULL, no data are attached. Consider adding
//   custom data cautiously, because faces and vertices with different UserData pointer 
//   attached can't be joined/removed. This functionality can be turned off, but in such case,
//   it's not defined which data will be used when joining vertices/faces.
//   On the other hand, you can suppress any vertex removal by setting the bDontTouch property
//   of this vertex GRC_VERTEXINITDATA structure to true.
//   
// Function returns zero when the face was added successfully, or returns nonzero error code.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::AddFace(	LPVOID pUserData,
									GRC_VERTEXINITDATA * vert0,
									GRC_VERTEXINITDATA * vert1,
									GRC_VERTEXINITDATA * vert2 )
{
	HRESULT			hr;
	int				toLoad;
	D3DXVECTOR3		normal, vec1, vec2;


	// check this object's state
	if ( State != GRS_INITIALIZED ) 
		ERRORMSG( ERRNOTPREPARED, "CGeometryReducer::AddFace()", 
			"Can't add face. GeometryReducer object is not in INITIALIZED state." );

	
	// allocate needed memory
	toLoad = sizeof( GRC_FACE ) * ( iFaceCount + 1 );
	hr = SatisfyMemoryRequirements( (LPVOID *) &pFaces, &iFaceMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::AddFace()", "Can't allocate appropriate memory for geometry faces." );

	toLoad = sizeof( GRC_VERTEX ) * ( iVertexCount + 3 );
	hr = SatisfyMemoryRequirements( (LPVOID *) &pVertices, &iVertMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::AddFace()", "Can't allocate appropriate memory for vertex data." );

	toLoad = sizeof( GRC_INDEX ) * ( iIndexCount + 3 );
	hr = SatisfyMemoryRequirements( (LPVOID *) &pIndices, &iIndMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::AddFace()", "Can't allocate appropriate memory for indexing data." );

	toLoad = sizeof( GRC_EDGE ) * ( iEdgeCount + 3 );
	hr = SatisfyMemoryRequirements( (LPVOID *) &pEdges, &iEdgeMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::AddFace()", "Can't allocate appropriate memory for mesh data." );


	// add new vertices
	hr = AddVertex( vert0 );
	hr += AddVertex( vert1 );
	hr += AddVertex( vert2 );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGeometryReducer::AddFace()", "Unable to add any more vertices." );

	// set vertex normals to face normal
	vec1 = pVertices[iVertexCount - 3].position - pVertices[iVertexCount - 2].position;
	vec2 = pVertices[iVertexCount - 1].position - pVertices[iVertexCount - 2].position;
	D3DXVec3Cross( &normal, &vec2, &vec1 );
	pVertices[iVertexCount - 3].normal = normal;
	pVertices[iVertexCount - 2].normal = normal;
	pVertices[iVertexCount - 1].normal = normal;
	
	// add new indices
	pIndices[iIndexCount++] = iVertexCount - 3;
	pIndices[iIndexCount++] = iVertexCount - 2;
	pIndices[iIndexCount++] = iVertexCount - 1;

	// add new edges
	hr = AddEdge( iIndexCount-3, iIndexCount-2, iFaceCount, &(pFaces[iFaceCount].edge[0]) );
	hr += AddEdge( iIndexCount-2, iIndexCount-1, iFaceCount, &(pFaces[iFaceCount].edge[1]) );
	hr += AddEdge( iIndexCount-1, iIndexCount-3, iFaceCount, &(pFaces[iFaceCount].edge[2]) );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGeometryReducer::AddFace()", "Unable to add mesh edge." );

	// add new face
	hr = AddUserDataBlock( pUserData, &(pFaces[iFaceCount].idUserData) );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGeometryReducer::AddFace()", "Unable to attach user data." );

	pFaces[iFaceCount].id = iFaceCount;
	pFaces[iFaceCount].ind[0] = iIndexCount - 3;
	pFaces[iFaceCount].ind[1] = iIndexCount - 2;
	pFaces[iFaceCount].ind[2] = iIndexCount - 1;

	iFaceCount++;


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// performs consistency check on the inputted data and builds some internal structures upon them
// this method must be called after last triangle is added and before ReduceGeometry call
//
// reurns zero on success or nonzero error code which also covers all types of wrong input data
//   for example diconnected or empty graph
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::CloseInput()
{
	int		groupsToLoad;
	HRESULT	hr;


	if ( State != GRS_INITIALIZED ) 
		ERRORMSG( ERRNOTPREPARED, "CGeometryReducer::CloseInput()", 
			"Can't finalize data input. GeometryReducer object is not in INITIALIZED state." );


	// empty graph check
	if ( !iFaceCount || !iVertexCount || !iEdgeCount ) 
		ERRORMSG( ERRINVALIDDATA, "CGeometryReducer::CloseInput()", "Empty graph not allowed." );

	// disconnected graph check
	//if ( !ConnectedGraph() ) ERRORMSG( ERRINVALIDDATA, "CGeometryReducer::CloseInput()", "Graph must be connected." );

	// reset vertex groups in vertex data
	for ( int i = 0; i < iVertexCount; i++ ) pVertices[i].idGroup = -1;

	// create vertex groups
	groupsToLoad = max( iVertexCount >> 2, GRC_MIN_GROUPS_COUNT );
	hr = SatisfyMemoryRequirements( (LPVOID *) &pGroups, &iGroupMemAllocated, groupsToLoad * sizeof(GRC_VERTEXGROUP) );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::CloseInput()", "Can't allocate appropriate memory for vertex groups." );
	
	//for ( int i = 0; i < groupsToLoad; i++ ) 
	//{
	//	if ( pGroups[i].Init() ) ERRORMSG( ERRGENERIC, "CGeometryReducer::CloseInput()", "Cannot init vertex group." );
	//}


	State = GRS_PREPARED;

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// processes the whole graph and tells whether the graph is connected
// this can be a bit time consuming, since the time complexity is quadratic for vertex count
// uses the iReserved vertex property, so the property value is undefined after this call
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGeometryReducer::ConnectedGraph()
{
	std::vector<GRC_ID>	stack;
	GRC_ID				id;

	// THE WHOLE THIS FUNCTION USES THE FACT THAT iVertexCount == iIndexCount

	// set all vertices to different components 
	for ( int i = 0; i < iVertexCount; i++ ) pVertices[i].iReserved = i;

	stack.clear();
	stack.push_back( 0 );

	// mark all reachable vertices
	while ( !stack.empty() )
	{
		id = stack.back();
		stack.pop_back();
	
		for ( int i = 0; i < iEdgeCount; i++ ) // try out all edges to find adjacent vertices
		{
			if ( pEdges[i].ind0 == id && pVertices[pIndices[pEdges[i].ind1]].iReserved )
			{ // found an edge leading from this vertex to a vertex that hasn't been visited yet
				pVertices[pIndices[pEdges[i].ind1]].iReserved = pVertices[pIndices[pEdges[i].ind0]].iReserved;
				stack.push_back( pEdges[i].ind1 );
			}
			else if ( pEdges[i].ind1 == id && pVertices[pIndices[pEdges[i].ind0]].iReserved )
			{ // found an edge leading to this vertex from a vertex that hasn't been visited yet
				pVertices[pIndices[pEdges[i].ind0]].iReserved = pVertices[pIndices[pEdges[i].ind1]].iReserved;
				stack.push_back( pEdges[i].ind0 );
			}
		}
	}


	// find an unvisited vertex
	for ( int i = 0; i < iVertexCount; i++ ) 
		if ( pVertices[i].iReserved ) return false;


	return true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// performs the TERRAIN(!) geometry reduction
// this method doesn't work on any data, can be used only for terrain now
// use width and depth parameters to define count of the dimension of the terrain (in fields)
// use division parameter to define granularity of plates; e.g. divison == 3 means 8 segments
//
// first this generates vertex groups (find all close vertices and join them together to group)
// then for each ungrouped vertex and for each group a new vertice is created, the position
//   and normal of the new vertice is made up as an arithmetic average value of all vertices
//   from appropriate group
// finally, vertices are placed into two dimensional grid and new faces are created between
//   adjacent vertices; only one optimization is done in this phase - horizontal (parallel 
//   with x axis) stripes of terrain plates are made up when a line of adjacent plates are all
//   planar and in one height - such a stripe is substituted by single quad
//
// you can set 'optimizefaces' to false and thus turn off the final face count reduction however 
//
// return 0 on success, otherwise returns non-zero error code
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::ReduceGeometry(	float epsilon,
											bool optimizefaces,
											int width,
											int depth,
											int division )
{
	struct __MAPFIELD
	{
		bool	bDivided; // plate is subdidived, thus can't be planar and optimized
		bool	bPlanar; // plate is planar, can be joined with other planar fields
		bool	bProcessed; // plate was already processed
	};

	int			id;
	HRESULT		hr;
	D3DXVECTOR3	normal, position;
	GRC_ID	*	pTemp;
	int			ix, iz;
	int			iWidth, iDepth; // dimension of the terrain auxiliary matrix (with all subdivisions)
	__MAPFIELD *pFields;
	D3DXPLANE	dxplane;
	int			iDelta = 1 << division; // 'count of subvertices between two main vertices' + 1



	if ( State != GRS_PREPARED ) 
		ERRORMSG( ERRNOTPREPARED, "CGeometryReducer::ReduceGeometry()", 
			"Can't perform geometry reduction. GeometryReducer object is not in PREPARED state." );
	



	//////////////////////////////////////////
	// GENERATING VERTEX GROUPS (CLOSE VERTICES)
	
	
	// will use two temporary (sorted) index arrays
	pTempIndicesX = new GRC_INDEX[iVertexCount];
	pTempIndicesZ = new GRC_INDEX[iVertexCount];
	if ( !pTempIndicesX || !pTempIndicesZ ) ERRORMSG( ERROUTOFMEMORY, "CGeometryReducer::ReduceGeometry()", "" );

	// fill them with vertex indices
	for ( int i = 0; i < iVertexCount; i++ )
	{
		pTempIndicesX[i] = i;
		pTempIndicesZ[i] = i;
	}

	// sort those temporary arrays
	qsort_s( pTempIndicesX, iVertexCount, sizeof( *pTempIndicesX ), GRCSortKernelX, this );
	qsort_s( pTempIndicesZ, iVertexCount, sizeof( *pTempIndicesZ ), GRCSortKernelZ, this );


	// join groups of vertices close in Z axis
	for ( ix = 0; ix < iVertexCount; ix++ )
	{
		for ( iz = ix + 1; iz < iVertexCount && abs( pVertices[pTempIndicesX[ix]].position.z -
			pVertices[pTempIndicesX[iz]].position.z ) <= epsilon; iz++ )
		{
			if ( abs( pVertices[pTempIndicesX[ix]].position.y - pVertices[pTempIndicesX[iz]].position.y ) <= epsilon )
				if ( JoinGroups( &pVertices[pTempIndicesX[ix]], &pVertices[pTempIndicesX[iz]] ) )
					ERRORMSG( ERRGENERIC, "CGeometryReducer::ReduceGeometry()", "Groups cannot be unified." );
		}
	}


	// join groups of vertices close in Z axis
	for ( iz = 0; iz < iVertexCount; iz++ )
	{
		for ( ix = iz + 1; ix < iVertexCount && abs( pVertices[pTempIndicesZ[iz]].position.x -
			pVertices[pTempIndicesZ[ix]].position.x ) <= epsilon; ix++ )
		{
			if ( abs( pVertices[pTempIndicesZ[iz]].position.y - pVertices[pTempIndicesZ[ix]].position.y ) <= epsilon )
				JoinGroups( &pVertices[pTempIndicesZ[iz]], &pVertices[pTempIndicesZ[ix]] );
		}
	}


	SAFE_DELETE_ARRAY( pTempIndicesX );
	SAFE_DELETE_ARRAY( pTempIndicesZ );





	//////////////////////////////////////////
	// CREATING VERTICES FROM GROUPS


	// get the count of groups actually used
	iActiveGroups = 0;
	for ( int i = 0; i < iGroupCount; i++ ) 
		if ( pGroups[i].iCount ) iActiveGroups++;
	
	iNewVertCount = iActiveGroups;
	for ( int i = 0; i < iVertexCount; i++ ) 
		if ( pVertices[i].idGroup == -1 ) iNewVertCount++;

	iNewVertCount += 4; // TEST: for adding lower polygon


	// create a new vertex for each group
	hr = SatisfyMemoryRequirements( (LPVOID *) &pNewVertices, &iNewVertMemAllocated, iNewVertCount * sizeof( GRC_VERTEX ) );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGeometryReducer::ReduceGeometry()", "Cannot generate new vertices." ); 

	id = 0;
	for ( int i = 0; i < iGroupCount; i++ )
	{
		if ( pGroups[i].iCount )
		{
			pNewVertices[id].id = id;
			pNewVertices[id].bDontTouch = false;
			pNewVertices[id].idGroup = i;
			
			// compute vertex normal and position as arithmetic average
			position = D3DXVECTOR3( 0, 0, 0 );
			normal = D3DXVECTOR3( 0, 0, 0 );

			for ( int j = 0; j < pGroups[i].iCount; j++ )
			{
				position += pVertices[pGroups[i].pVertices[j]].position;
				normal += pVertices[pGroups[i].pVertices[j]].normal;
			}

			pNewVertices[id].position = position / (float) pGroups[i].iCount;
			pNewVertices[id].normal = normal / (float) pGroups[i].iCount;
			
			pGroups[i].newID = id;
			id++;
		}
		else pGroups[i].newID = -1;
	}

		
	// also copy all ungrouped vertices
	for ( int i = 0; i < iVertexCount; i++ ) 
	{
		if ( pVertices[i].idGroup == -1 ) // ungrouped vertex
		{
			pNewVertices[id].id = id;
			pNewVertices[id].bDontTouch = false;
			pNewVertices[id].idGroup = -1;
			pNewVertices[id].position = pVertices[i].position;
			pNewVertices[id].normal = pVertices[i].normal;

			pVertices[i].newID = id;
			id++;
		}
		else pVertices[i].newID = pGroups[pVertices[i].idGroup].newID;
	}

	
	// settle vertices to standard terrain positions
	for ( int i = 0; i < iNewVertCount; i++ )
	{
		// add PLATE_WIDTH / 16 and then divide by (PLATE_WIDTH / 8), multiply back by the same value and it 
		//   should return appropriate value alligned to terrain plate grid (subdivided to 8*8 squares)
		pNewVertices[i].position.x = (float) ( 
				  ( ((int) pNewVertices[i].position.x) + ( ((int) TERRAIN_PLATE_WIDTH) >> (division + 1) ) ) / 
				  ( ((int) TERRAIN_PLATE_WIDTH) >> division ) * ( ((int) TERRAIN_PLATE_WIDTH) >> division )
			 );
		pNewVertices[i].position.z = (float) ( 
				  ( ((int) pNewVertices[i].position.z) + ( ((int) TERRAIN_PLATE_WIDTH) >> (division + 1) ) ) / 
				  ( ((int) TERRAIN_PLATE_WIDTH) >> division ) * ( ((int) TERRAIN_PLATE_WIDTH) >> division )
			 );
		//pNewVertices[i].position.y = (float) ( 
		//		  ( ((int) pNewVertices[i].position.y) + ( ((int) TERRAIN_PLATE_HEIGHT) >> 1 ) ) / 
		//			((int) TERRAIN_PLATE_HEIGHT) * ((int) TERRAIN_PLATE_HEIGHT)
		//	 );
	}

 



	//////////////////////////////////////////
	// GENERATE NEW FACES


	// create a new auxiliary vertex map
	iWidth = ( width << division ) + 1;
	iDepth = ( depth << division ) + 1;
	pTemp = new GRC_ID[(iWidth + 1) * (iDepth + 1)];
	if ( !pTemp ) ERRORMSG( ERROUTOFMEMORY, "CGeometryReducer::ReduceGeometry()", "Unable to create temporary index array." );

	
	// set indices to vertex map
	for ( int i = 0; i < iWidth * iDepth; i++ )
		pTemp[i] = -1;

	for ( int i = 0; i < iNewVertCount; i++ )
	{
		ix = ( (int) pNewVertices[i].position.x ) / ( ((int) TERRAIN_PLATE_WIDTH) >> division );
		iz = ( (int) pNewVertices[i].position.z ) / ( ((int) TERRAIN_PLATE_WIDTH) >> division );
		if ( ix >= 0 && ix < iWidth && iz >= 0 && iz < iDepth ) 
			pTemp[iz * iWidth + ix] = i;
	}


	// create fields auxiliary map
	pFields = new __MAPFIELD[width * depth];
	if ( !pFields ) ERRORMSG( ERROUTOFMEMORY, "CGeometryReducer::ReduceGeometry()", "Unable to create temporary structure." );

	ZeroMemory( pFields, sizeof(*pFields) * width * depth );

	
	// find planar and subdivided fields
	if ( optimizefaces )
	{
		for ( iz = 0; iz < depth; iz++ )
		{
			int index0 = (iz << division) * iWidth; // left near vertex
			int index1 = index0 + iDelta * iWidth; // left far vertex
			int index2 = index1 + iDelta; // right far vertex
			int index3 = index0 + iDelta; // right near vertex
			int index4 = index0 + iWidth * (iDelta >> 1) + (iDelta >> 1); // center vertex

			for ( ix = 0; ix < width; ix++ )
			{
				// check for field subdivision
				if ( division > 0 && pTemp[index4] != -1 )
					pFields[iz * depth + ix].bDivided = true;

				// check for field planarity
				//D3DXPlaneFromPoints(	&dxplane, &pNewVertices[pTemp[index0]].position, 
				//						&pNewVertices[pTemp[index1]].position,
				//						&pNewVertices[pTemp[index2]].position );
				//if ( abs( D3DXPlaneDot( &plane, &pNewVertices[pTemp[index3]].position ) ) <= epsilon )
				//	pFields[iz * depth + ix].bPlanar = true;
				
				// only plates that are parallel with y axis are considered planar
				if ( abs( pNewVertices[pTemp[index0]].position.y - pNewVertices[pTemp[index1]].position.y ) <= epsilon &&
					 abs( pNewVertices[pTemp[index2]].position.y - pNewVertices[pTemp[index3]].position.y ) <= epsilon &&
					 abs( pNewVertices[pTemp[index0]].position.y - pNewVertices[pTemp[index2]].position.y ) <= epsilon )
					pFields[iz * depth + ix].bPlanar = true;
				

				index0 += iDelta;
				index1 += iDelta;
				index2 += iDelta;
				index3 += iDelta;
				index4 += iDelta;
			}
		}
	}


	// reset all structures
	iVertexCount = 0;
	iIndexCount = 0;
	iEdgeCount = 0;
	iFaceCount = 0;


	// create compound faces - find larger planar areas
	if ( optimizefaces )
	{
		for ( iz = 0; iz < depth; iz++ )
		{
			for ( ix = 0; ix < width; ix++ )
			{
				if ( pFields[iz * depth + ix].bPlanar && !pFields[iz * depth + ix].bDivided )
				{
					int len = 0;
					for ( int dx = ix + 1; dx < width; dx++ )
					{
						if ( !pFields[iz * depth + dx].bPlanar || pFields[iz * depth + dx].bDivided ) break;
						pFields[iz * depth + dx].bProcessed = true;
						len++;
					}
					
					if ( len ) // larger horizontal area found -> replace it with two faces only
					{
						int index1 = (iz << division) * iWidth + (ix << division);
						int index2 = index1 + iDelta * iWidth; 
						AddNewFace( pTemp[index1], pTemp[index2], pTemp[index2 + iDelta * (len + 1)] );
						AddNewFace( pTemp[index2 + iDelta * (len + 1)], pTemp[index1 + iDelta * (len + 1)], pTemp[index1] );
						pFields[iz * depth + ix].bProcessed = true;
					}

					ix += len;
				}
			}
		}
	}


	// just recreate all the rest faces - no further reduction is being done here at all
	for ( iz = 0; iz < depth; iz++ )
	{
		for ( ix = 0; ix < width; ix++ )
		{
			// discover the granularity used for this particular field
			int div = 0;
			for ( int i = division - 1; i >= 0; i-- ) 
			{
				if ( pTemp[((iz << division) + (1 << i)) * iWidth + (ix << division) + (1 << i)] != -1 )
					div++;
					else break;
			}
			
			// create new faces
			int index = (iz << division) * iWidth + (ix << division);
			int index2;
			int delta = 1 << (division - div);

			for ( int dz = 0; dz < (1 << div); dz++ )
			{
				index2 = index;
				for ( int dx = 0; dx < (1 << div); dx++ ) if ( !pFields[iz*width + ix].bProcessed )
				{
					AddNewFace( pTemp[index2], pTemp[index2 + delta * iWidth], pTemp[index2 + delta * iWidth + delta] );
					AddNewFace( pTemp[index2 + delta * iWidth + delta], pTemp[index2 + delta], pTemp[index2] );
					index2 += delta;
				}
				index += delta * iWidth;
			}
		}
	}



	SAFE_DELETE_ARRAY( pTemp );
	SAFE_DELETE_ARRAY( pFields );

	//pNewVertices = pVertices;
	//pVertices = NULL;
	//iNewVertCount = iVertexCount;


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a new face to the renewed list of faces; parameters are only indices to the vertices
//   and this method actually adds all three indices into index buffer in given order
//   and creates a new face which is added to the faces list
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGeometryReducer::AddNewFace( GRC_ID ind0, GRC_ID ind1, GRC_ID ind2 )
{
	pFaces[iFaceCount].id = iFaceCount;
	pFaces[iFaceCount].ind[0] = ind0;
	pFaces[iFaceCount].ind[1] = ind1;
	pFaces[iFaceCount].ind[2] = ind2;
	
	pFaces[iFaceCount].normal = pNewVertices[ind0].normal;
	pFaces[iFaceCount].normal += pNewVertices[ind1].normal;
	pFaces[iFaceCount].normal += pNewVertices[ind2].normal;
	pFaces[iFaceCount].normal /= 3.0f;

	pIndices[iIndexCount++] = ind0;
	pIndices[iIndexCount++] = ind1;
	pIndices[iIndexCount++] = ind2;

	iFaceCount++;

	return;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Joins groups used by two specified vertices together or adds a vertex to existing group.
// If both vertices has group ID set to -1, a new group is created and both vertices are placed
//   into this new group.
// If the second vertex (vert1) has group ID set to -1, but the first one has valid group ID, 
//   the second one is simply added to the group of the first vertex (vert0). And vice versa.
// If the second vertex has valid group ID, all vertice from this group are moved to the
//   group of vert0 and then the group is cleared out. However, this group IS NOT removed! 
//   so all the remaining group indices are still valid (even if this one is dead forever).
//
// !!!! THIS FUNCTION WORKS DIRECTLY WITH VERTEXBUFFER BECAUSE IT USES THE FACT THAT
//   iVertexCount == iIndexCount AND THAT BOTH FIELDS STILL USES THE SAME ORDER
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::JoinGroups( GRC_VERTEX * vert0, GRC_VERTEX * vert1 )
{
	int		toLoad, size, id;
	GRC_ID *pId;
	

	// create a new group if both vertices are ungrouped
	if ( vert0->idGroup == -1 && vert1->idGroup == -1 )
	{
		toLoad = sizeof( GRC_VERTEXGROUP ) * ( iGroupCount + 1 );
		if ( SatisfyMemoryRequirements( (LPVOID *) &pGroups, &iGroupMemAllocated, toLoad ) )
			ERRORMSG( ERRGENERIC, "CGeometryReducer::JoinGroups()", "Cannot creaty any more groups." );
		if ( pGroups[iGroupCount].Init() ) 
			ERRORMSG( ERRGENERIC, "CGeometryReducer::JoinGroups()", "Cannot init vertex group." );		

		pGroups[iGroupCount].Alloc();
		pGroups[iGroupCount].id = iGroupCount;
		pGroups[iGroupCount].AddVertex( vert0->id );
		pGroups[iGroupCount].AddVertex( vert1->id );
		vert0->idGroup = iGroupCount;
		vert1->idGroup = iGroupCount;
		iGroupCount++;
		return ERRNOERROR;
	}

	// maybe both vertices are already in the same group:
	if ( vert0->idGroup == vert1->idGroup ) return ERRNOERROR;

	// is the second vertex in ANY group at all?
	if ( vert1->idGroup == -1 )
	{
		pGroups[vert0->idGroup].AddVertex( vert1->id );
		vert1->idGroup = vert0->idGroup;
		return ERRNOERROR;
	}

	// is the first vertex in ANY group at all?
	if ( vert0->idGroup == -1 )
	{
		pGroups[vert1->idGroup].AddVertex( vert0->id );
		vert0->idGroup = vert1->idGroup;
		return ERRNOERROR;
	}

	// both vertices has their own group, so move all vertices from second group to the first one
	pId = pGroups[vert1->idGroup].pVertices;
	size = pGroups[vert1->idGroup].iCount;
	id = vert1->idGroup;

	for ( int i = 0; i < size; i++, pId++ )
		pVertices[*pId].idGroup = vert0->idGroup;
	
	if ( pGroups[vert0->idGroup].JoinGroup( &pGroups[id] ) ) 
		ERRORMSG( ERRGENERIC, "CGeometryReducer::JoinGroups()", "Cannot merge groups." );		

	pGroups[id].iCount = 0;


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// GetD3DBuffers method creates vertex and index buffers defined by properties in Params structure.
// This structure is also used for ouput - members iVertexCount and iIndexCount are filled 
//   by valid values. The rest of parameters defines how these buffers should be created.
// The caller is responsible for releasing of created buffers.
// This method can't be called before CloseInit() call at least. If you want to obtain
//   user data associated with vertices stored in these buffers, you have to call method
//   GetUserData() which will create and fill buffers with these data.
//
// Zero is returned when there is no error during buffers creation, otherwise a nonzero
//   error code is returned.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::GetD3DBuffers(	GRC_OUTPUTDATA * params, 
											IDirect3DVertexBuffer9 ** pVertexBuffer,
											IDirect3DIndexBuffer9 ** pIndexBuffer )
{
	typedef std::map<GRC_ID,int>		ID_MAP;
	typedef ID_MAP::iterator			ID_MAP_ITER;
	typedef std::pair<GRC_ID,int>		MAPPAIR;
	typedef std::pair<ID_MAP_ITER,bool>	INSERTRESPAIR;

	HRESULT			hr;
	char		*	pVBuffer;
	char		*	pIBuffer;
	int				IBind, VBind;
	DWORD		*	pIBuffer32;
	WORD		*	pIBuffer16;
	ID_MAP			IDmap;
	ID_MAP_ITER		iter;
	INSERTRESPAIR	ResPair;
	bool			b32bitIB;



	// check parameters validity
	if ( State != GRS_PREPARED ) 
		ERRORMSG( ERRNOTPREPARED, "CGeometryReducer::GetD3DBuffers()", 
			"Can't return geometry data. GeometryReducer object is not in PREPARED state." );

	if ( !(params->dwFVF & (D3DFVF_XYZ | D3DFVF_NORMAL)) )
		ERRORMSG( ERRINVALIDPARAMETER, "CGeometryReducer::GetD3DBuffers()", 
			"Flexible vertex format must have at least D3DFVF_XYZ and D3DFVF_NORMAL flags set." );
	
	if ( !params->dwNormalOffset ) 
		ERRORMSG( ERRINVALIDPARAMETER, "CGeometryReducer::GetD3DBuffers()", 
			"Offset of the Normal property in the vertex declaration structure is not set!" );



	// create vertex buffer
	hr = params->pD3DDevice->CreateVertexBuffer(	
				params->uiVertexStride * iNewVertCount,
				params->dwVBufferUsage,
				params->dwFVF,
				params->VBufferPool,
				pVertexBuffer,
				NULL );

	if ( hr ) ERRORMSG( hr, "CGeometryReducer::GetD3DBuffers()", "" );

	// create index buffer
	hr = params->pD3DDevice->CreateIndexBuffer(	
				iIndexCount * ( params->IBufferFormat & D3DFMT_INDEX32 ? sizeof(DWORD) : sizeof(WORD) ),
				params->dwIBufferUsage,
				params->IBufferFormat,
				params->IBufferPool,
				pIndexBuffer,
				NULL );

	if ( hr ) ERRORMSG( hr, "CGeometryReducer::GetD3DBuffers()", "" );


	
	// fill both buffers with valid data (only the necessary data are copied)
	(*pVertexBuffer)->Lock( 0, 0, (void **) &pVBuffer, 0 );
	(*pIndexBuffer)->Lock( 0, 0, (void **) &pIBuffer, 0 );

	IBind = VBind = 0;
	pIBuffer32 = (DWORD *) pIBuffer;
	pIBuffer16 = (WORD *) pIBuffer;
	b32bitIB = params->IBufferFormat & D3DFMT_INDEX32 ? true : false;
	IDmap.clear();

	
	// process each face
	for ( int i = 0; i < iFaceCount; i++ ) 
	{

		// process each face vertex index
		for ( int j = 0; j < 3; j++ ) 
		{
			iter = IDmap.find( pFaces[i].ind[j] );
			if ( iter == IDmap.end() ) // this index is not used yet
			{
				ResPair = IDmap.insert( MAPPAIR( pFaces[i].ind[j], VBind ) );

				// copy vertex position
				memcpy( &(pVBuffer[VBind * params->uiVertexStride]), 
						&pNewVertices[pFaces[i].ind[j]].position, 
						sizeof( pNewVertices[pFaces[i].ind[j]].position ) ); 
				
				// copy vertex normal
				//pNewVertices[pFaces[i].ind[j]].normal = D3DXVECTOR3( 0, 1, 0 );
				memcpy( &(pVBuffer[VBind * params->uiVertexStride + params->dwNormalOffset]),
						&pNewVertices[pFaces[i].ind[j]].normal,
						sizeof( pNewVertices[pFaces[i].ind[j]].normal ) ); 

				VBind++;
				iter = ResPair.first;
			}

			// save 16- or 32-bit indice
			if ( b32bitIB ) pIBuffer32[IBind++] = (DWORD) iter->second;
			else pIBuffer16[IBind++] = (WORD) iter->second;
		}
		
	}


	(*pVertexBuffer)->Unlock();
	(*pIndexBuffer)->Unlock();

	params->iVertexCount = iNewVertCount;
	params->iIndexCount = iIndexCount;


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// This method checks whether there's enough space in the allocated memory block to fulfill
//   specified memory request. If there's not enough space, the memory block is resized to
//   satisfy the request. During this process the original data pointer can change.
//
// Use pBuf pointer to specify actually allocated memory block. Pointer to newly allocated
//   memory block will be also returned in this parameter. Also specify the actual amount of
//   memory allocated (in bytes) in actualSize parameter. This value can be changed by this
//   call, so you have to store the returned value of actualSize parameter. For the memory
//   size request use the requestedSize parameter.
//
// If requestedSize is less or equivalent to actualSize, no changes are made,
//   no (de)allocation is performed and the function immediatelly returns 0. 
//
// This function returns 0 if everything went ok. When there was any error, the return value
//   is proper error code, pBuf pointer is set to NULL and the value of actualSize parameter
//   is set to 0.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::SatisfyMemoryRequirements( LPVOID * pBuf, int * actualSize, int requestedSize )
{
	char	*	pNewBuf;
	int			originalSize;
	
	// there's enough space for fulfill the request
	if ( requestedSize <= *actualSize ) return ERRNOERROR;

	if ( !pBuf ) ERRORMSG( ERRINVALIDPARAMETER, "CGeometryReducer::SatisfyMemoryRequirements()", "Invalid memory block pointer." );
	if ( requestedSize < 1 ) ERRORMSG( ERRINVALIDPARAMETER, "CGeometryReducer::SatisfyMemoryRequirements()", "Invalid size requirement." );
	if ( *actualSize < 0 ) ERRORMSG( ERRINVALIDPARAMETER, "CGeometryReducer::SatisfyMemoryRequirements()", "Invalid actual size information." );
	
	pNewBuf = (char *) *pBuf;
	originalSize = *actualSize;

	if ( !pNewBuf )
	{
		*pBuf = new char[requestedSize];

		if ( *pBuf ) *actualSize = requestedSize;
		else
		{
			*actualSize = 0;
			ERRORMSG( ERROUTOFMEMORY, "CGeometryReducer::SatisfyMemoryRequirements()", "" );
		}
	}
	else
	{
		int		minSize = 1;
		while ( minSize < requestedSize ) minSize *= 2;
		pNewBuf = new char[minSize];
		
		if ( pNewBuf )
		{
			// move the original data to the newly created buffer
			memcpy( pNewBuf, *pBuf, originalSize );
			delete [] (*pBuf);
			*actualSize = minSize;
			*pBuf = pNewBuf;
		}
		else ERRORMSG( ERROUTOFMEMORY, "CGeometryReducer::SatisfyMemoryRequirements()", "" );
	}


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// AddVertex method adds a new vertex into vertex buffer and increases vertices counter.
// This method assumes that there's enough space in the vertex buffer, so it doesn't allocate
//   any memory. If there's not enough space, this returns a nonzero error code. Otherwise 
//   it returns 0.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::AddVertex( GRC_VERTEXINITDATA * pVert )
{
	HRESULT			hr;


	if ( (int) sizeof(GRC_VERTEX) * (iVertexCount+1) > iVertMemAllocated )
		ERRORMSG( ERRGENERIC, "CGeometryReducer::AddVertex()", "Not enough memory allocated. Can't add any more vertices." );

	hr = AddUserDataBlock( pVert->pUserData, &(pVertices[iVertexCount].idUserData) );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGeometryReducer::AddVertex()", "Unable to attach user data." );

	pVertices[iVertexCount].id = iVertexCount;
	//pVertices[iVertexCount].idGroup = iVertexCount;
	pVertices[iVertexCount].bDontTouch = pVert->bDontTouch;
	pVertices[iVertexCount].position = pVert->Position;

	iVertexCount++;	

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// finds specified pointer among user data blocks list; if the specified pointer doesn't
//   exist, a new one is added into the list and a index of the new one is returned in
//   returnedId parameter; if it already exists, its id is returned in the returnedId 
//   parameter and no new item is added
// 
// This function can fail when the block is not found and a creation of the new one fails.
//   In such case this returns nonzero error code, otherwise this returns zero.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::AddUserDataBlock( LPVOID pUserData, GRC_ID * returnedId )
{
	int				ind;
	HRESULT			hr;


	for ( ind = 0; ind < iUserDataCount; ind++ ) 
		if ( pUserDataBlocks[ind] == pUserData ) break;

	if ( ind < iUserDataCount ) *returnedId = ind;
	else 
	{
		hr = SatisfyMemoryRequirements( (LPVOID *) &pUserDataBlocks, &iUserDataMemAllocated, (iUserDataCount + 1) * sizeof(GRC_USERDATA) );
		if ( hr ) ERRORMSG( hr, "CGeometryReducer::AddUserDataBlock()", "Can't allocate appropriate memory for custom user data." );
	
		pUserDataBlocks[iUserDataCount] = pUserData;
		*returnedId = iUserDataCount;
		iUserDataCount++;
	}

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// This methods checks whether there's such an edge in the list. It looks for edges that
//   consists of the same vertices. If such an edge doesn't exist, a new edge is added into
//   the list with a left face set to the one specified by face parameter. However, if such
//   an edge already exists, a check is made whether the existing edge has only one attached
//   face. Then the second face is set on this found edge.
//
// In edge parameter this method returns the ID of the newly created or edited edge.
//
// This method assumes that there's enough memory for it to add. If there's not, this returns 
//   an error value.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::AddEdge( GRC_ID ind0, GRC_ID ind1, GRC_ID face, GRC_ID * edge )
{
	bool	bReverted = false;
	GRC_ID	index = -1;

	
	// check for degenereated edge
	if ( pIndices[ind0] == pIndices[ind1] ) 
		ERRORMSG( ERRINVALIDPARAMETER, "CGeometryReducer::AddEdge()", "Degenerated edge." ); 

	// check for proper edge orientation
	if ( ind0 > ind1 )
	{
		ind0 += ind1;
		ind1 = ind0 - ind1;
		ind0 = ind0 - ind1;
		bReverted = true;
	}

	// find similar edge
	for ( int i = 0; i < iEdgeCount; i++ )
		if ( pEdges[i].ind0 == ind0 && pEdges[i].ind1 == ind1 ) { index = i; break; }
	
	
	if ( index < 0 ) // no similar edge found, add new one
	{	
		if ( (int) sizeof(GRC_EDGE) * (iEdgeCount+1) > iEdgeMemAllocated )
			ERRORMSG( ERRGENERIC, "CGeometryReducer::AddEdge()", "Not enough memory allocated. Can't add any more edges." );

		pEdges[iEdgeCount].id = iEdgeCount;
		pEdges[iEdgeCount].idUserData = NULL;
		pEdges[iEdgeCount].ind0 = ind0;
		pEdges[iEdgeCount].ind1 = ind1;
		pEdges[iEdgeCount].face0 = bReverted ? -1 : face;
		pEdges[iEdgeCount].face1 = bReverted ? face : -1;
		pEdges[iEdgeCount].iFaceCount = 1;
		pEdges[iEdgeCount].bLeftFaceDefined = !bReverted;
		pEdges[iEdgeCount].bRightFaceDefined = bReverted;
		
		*edge = iEdgeCount;
		iEdgeCount++;
	}
	else // old edge will be updated
	{
		if ( !bReverted )
		{
			if ( pEdges[index].bLeftFaceDefined )
				ERRORMSG( ERRINVALIDPARAMETER, "CGeometryReducer::AddEdge()", "Duplicate edge not allowed." ); 
			pEdges[index].face0 = face;
			pEdges[index].bLeftFaceDefined = true;
		}
		else
		{
			if ( pEdges[index].bRightFaceDefined )
				ERRORMSG( ERRINVALIDPARAMETER, "CGeometryReducer::AddEdge()", "Duplicate edge not allowed." ); 
			pEdges[index].face1 = face;
			pEdges[index].bRightFaceDefined = true;
		}

		pEdges[index].iFaceCount++;
		*edge = index;
	}


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//
//								GRC_VERTEXGROUP MEMBER FUNCTIONS
//
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor & destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGeometryReducer::GRC_VERTEXGROUP::GRC_VERTEXGROUP()
{
	id = -1;
	iCount = 0;
	iMemAllocated = 0;
	pVertices = NULL;
};

CGeometryReducer::GRC_VERTEXGROUP::~GRC_VERTEXGROUP()
{
	Free();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// first this method resets all class members and then tries to allocate buffer for
//   'count' vertices; you can skip the count parameter, in such case GRC_MIN_GROUP_SIZE will 
//   be used instead 
//
// returns 0 on success or non-zero error code
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::GRC_VERTEXGROUP::Init( int count )
{
	id = -1;
	iCount = 0;
	iMemAllocated = 0;
	pVertices = NULL;

	return Alloc( count );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// check whether there's enough memory for storing 'count' vertices and if the actually 
//   allocated memory is not sufficient, this tries to allocate bigger buffer
// you can skip the count parameter, in such case GRC_MIN_GROUP_SIZE will be used instead 
//
// returns 0 on success or non-zero error code
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::GRC_VERTEXGROUP::Alloc( int count )
{
	int		toLoad;
	HRESULT	hr;

	toLoad = sizeof( GRC_ID ) * ( count ? count : GRC_MIN_GROUP_SIZE );
	hr = SatisfyMemoryRequirements( (LPVOID *) &pVertices, &iMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::GRC_VERTEXGROUP::Alloc()", 
		"Can't allocate appropriate memory for group vertices." );

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a vertex to this group; if the actually allocated memory is not sufficient, this tries
//   to allocate bigger buffer
// returns 0 on success or non-zero error code
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::GRC_VERTEXGROUP::AddVertex( GRC_ID vid )
{
	int		toLoad;
	HRESULT	hr;

	toLoad = sizeof( GRC_ID ) * (iCount + 1);
	hr = CGeometryReducer::SatisfyMemoryRequirements( (LPVOID *) &pVertices, &iMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::GRC_VERTEXGROUP::AddVertex()", 
		"Can't allocate appropriate memory for group vertices." );
		
	pVertices[iCount++] = vid;

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// copy all vertices from specified group to this one; if the actually allocated memory is not 
//   sufficient, this tries to allocate bigger buffer
// returns 0 on success or non-zero error code
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGeometryReducer::GRC_VERTEXGROUP::JoinGroup( LPGRC_VERTEXGROUP pGroup )
{
	int		toLoad;
	HRESULT	hr;
	
	toLoad = sizeof( GRC_ID ) * (iCount + pGroup->iCount);
	hr = CGeometryReducer::SatisfyMemoryRequirements( (LPVOID *) &pVertices, &iMemAllocated, toLoad );
	if ( hr ) ERRORMSG( hr, "CGeometryReducer::GRC_VERTEXGROUP::JoinGroup()", 
		"Can't allocate appropriate memory for group vertices." );
		
	for ( int i = 0; i < pGroup->iCount; i++ ) 
		pVertices[iCount++] = pGroup->pVertices[i];
	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//
//								NON CLASS MEMBERS
//
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
//
// the kernel function for quicksort; sorts vertices by their X coordinate 
//
//////////////////////////////////////////////////////////////////////////////////////////////
int __cdecl graphic::GRCSortKernelX( void * context, const void * v0, const void * v1 )
{
	if ( ((CGeometryReducer *)context)->pVertices[*((int*)v0)].position.x < 
		((CGeometryReducer *)context)->pVertices[*((int*)v1)].position.x )
		return -1;

	if ( ((CGeometryReducer *)context)->pVertices[*((int*)v0)].position.x > 
		((CGeometryReducer *)context)->pVertices[*((int*)v1)].position.x )
		return 1;

	if ( ((CGeometryReducer *)context)->pVertices[*((int*)v0)].position.z < 
		((CGeometryReducer *)context)->pVertices[*((int*)v1)].position.z )
		return -1;

	if ( ((CGeometryReducer *)context)->pVertices[*((int*)v0)].position.z > 
		((CGeometryReducer *)context)->pVertices[*((int*)v1)].position.z )
		return 1;

	return 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// the kernel function for quicksort; sorts vertices by their Z coordinate 
//
//////////////////////////////////////////////////////////////////////////////////////////////
int __cdecl graphic::GRCSortKernelZ( void * context, const void * v0, const void * v1 )
{
	if ( ((CGeometryReducer *)context)->pVertices[*((int*)v0)].position.z < 
		((CGeometryReducer *)context)->pVertices[*((int*)v1)].position.z )
		return -1;

	if ( ((CGeometryReducer *)context)->pVertices[*((int*)v0)].position.z > 
		((CGeometryReducer *)context)->pVertices[*((int*)v1)].position.z )
		return 1;

	if ( ((CGeometryReducer *)context)->pVertices[*((int*)v0)].position.x < 
		((CGeometryReducer *)context)->pVertices[*((int*)v1)].position.x )
		return -1;

	if ( ((CGeometryReducer *)context)->pVertices[*((int*)v0)].position.x > 
		((CGeometryReducer *)context)->pVertices[*((int*)v1)].position.x )
		return 1;

	return 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
