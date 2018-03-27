#include "Model.h"


//////////////////////////////////////////////////////////////////////////////////////////////
// constructor zeroes some pointers
//////////////////////////////////////////////////////////////////////////////////////////////
CModel::CModel()
{
	pEdges = NULL;
	pVertices = NULL;
	pFaces = NULL;
	pMaterials = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// destructor frees some structures
//////////////////////////////////////////////////////////////////////////////////////////////
CModel::~CModel()
{
	SAFE_DELETE_ARRAY( pEdges );
	SAFE_DELETE_ARRAY( pVertices );
	SAFE_DELETE_ARRAY( pFaces );
	SAFE_DELETE_ARRAY( pMaterials );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// reads one line from a text file, returns false if the end of a file was reached
// also in that case, s contains the read string
//////////////////////////////////////////////////////////////////////////////////////////////
bool CModel::ReadLine( FILE *f, char *s )
{
	UINT	i = 0;
	size_t	hr;

	hr = fread( &s[i], 1, 1, f );
	if ( !hr )
	{
		s[i] = 0;
		return false;
	}

	
	while ( s[i] != '\n' )
	{
		i++;
		hr = fread( &s[i], 1, 1, f );
		if ( !hr )
		{
			s[i] = 0;
			return false;
		}
	}
	s[i] = 0;
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// loads a model from DirectX eXtension file
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CModel::LoadFromXFile( LPCTSTR fileName )
{
	FILE		*f;
	char		_str[1025];
	CString		str;
	int			ok, pomI;
	int			materialInd = -1;


	bIsPrecomputed = false;
	uiNumVertices = 0;
	uiNumEdges = 0;
	uiNumFaces = 0;
	uiNumMaterials = 0;

	
	f = fopen( fileName, "rt" );
	if ( !f ) return ERRFILENOTFOUND;

	
	// reads lines of file until the end of the file is reached
	while ( ReadLine( f, _str ) )
	{
		str = " ";
		str += _str;
		
		// mesh data block found
		if ( str.Find( " Mesh ") != -1 ) 
		{
			// read vertices
			if ( !fscanf( f, "%li;\n", &pomI ) ) return ERRFILECORRUPTED;
			
			uiNumVertices = pomI;
			
			pVertices = new VERTEX[uiNumVertices];
			if ( !pVertices ) return ERROUTOFMEMORY;

			ok = 0;
			for ( UINT i = 0; i < uiNumVertices - 1; i++ )
			{
				ok += fscanf(	f, "%f;%f;%f;,'\n", 
								&(pVertices[i].pos.x),
								&(pVertices[i].pos.y),
								&(pVertices[i].pos.z) );
			}

			ok += fscanf(	f, "%f;%f;%f;;'\n", 
							&(pVertices[uiNumVertices - 1].pos.x),
							&(pVertices[uiNumVertices - 1].pos.y),
							&(pVertices[uiNumVertices - 1].pos.z) );
			if ( ok != 3*uiNumVertices ) return ERRFILECORRUPTED;



			// read faces
			if ( !fscanf( f, "%li;\n", &pomI ) ) return ERRFILECORRUPTED;
			
			uiNumFaces = pomI;
			
			pFaces = new FACE[uiNumFaces];
			if ( !pFaces ) return ERROUTOFMEMORY;

			for ( UINT i = 0; i < uiNumFaces - 1; i++ )
			{
				ok = fscanf( f, "%li;", &pomI );
				if ( !ok || pomI != 3 ) return ERRFILECORRUPTED;
				
				ok = fscanf( f, "%lu,%lu,%lu;,'\n", 
							&(pFaces[i].vertices[0]),
							&(pFaces[i].vertices[1]),
							&(pFaces[i].vertices[2]) );
				if ( ok != 3 ) return ERRFILECORRUPTED;
			}

			ok = fscanf( f, "%li;", &pomI );
			if ( !ok || pomI != 3 ) return ERRFILECORRUPTED;
				
			ok = fscanf(	f, "%lu,%lu,%lu;;'\n", 
							&(pFaces[uiNumFaces - 1].vertices[0]),
							&(pFaces[uiNumFaces - 1].vertices[1]),
							&(pFaces[uiNumFaces - 1].vertices[2]) );
			if ( ok != 3 ) return ERRFILECORRUPTED;
		}



		// TextureCoords data block found (should be in Mesh data block)
		if ( str.Find( " MeshTextureCoords " ) != -1 )
		{
			if ( !fscanf( f, "%li;\n", &pomI ) ) return ERRFILECORRUPTED;
			if ( pomI != uiNumVertices ) return ERRFILECORRUPTED;
			
			ok = 0;
			for ( UINT i = 0; i < uiNumVertices - 1; i++ )
			{
				ok += fscanf(	f, "%f;%f;,\n", 
								&(pVertices[i].textureCoords.x), 
								&(pVertices[i].textureCoords.y) ); 
			}

			ok += fscanf(	f, "%f;%f;;\n", 
							&(pVertices[uiNumVertices - 1].textureCoords.x), 
							&(pVertices[uiNumVertices - 1].textureCoords.y) ); 
			if ( ok != 2*uiNumVertices ) return ERRFILECORRUPTED;
		}



		// MaterialList found (should be in MEsh data block)
		if ( str.Find( " MeshMaterialList " ) != -1 )
		{
			if ( !fscanf( f, "%li;\n", &pomI ) ) return ERRFILECORRUPTED;
			if ( !pomI ) return ERRFILECORRUPTED;
			uiNumMaterials = pomI;
			
			if ( !fscanf( f, "%li;\n", &pomI ) ) return ERRFILECORRUPTED;
			if ( pomI != uiNumFaces ) return ERRFILECORRUPTED;


			pMaterials = new MATERIAL[uiNumMaterials];
			if ( !pMaterials ) return ERROUTOFMEMORY;

			// read material indices
			for ( UINT i = 0; i < uiNumFaces - 1; i++ )
				if ( !fscanf( f, "%lu,\n", &(pFaces[i].materialIndex) ) ) return ERRFILECORRUPTED;
	
			if ( !fscanf( f, "%lu;\n", &(pFaces[uiNumFaces - 1].materialIndex) ) ) return ERRFILECORRUPTED;
		}



		// Material found (should be in MaterialList data block)
		if ( str.Find( " Material " ) != -1 )
		{
			if ( materialInd > (int) uiNumMaterials ) return ERRFILECORRUPTED;
			
			materialInd++;

			ok = fscanf(	f, "%f;%f;%f;%f;;\n",
							&(pMaterials[materialInd].diffuseColor.x),
							&(pMaterials[materialInd].diffuseColor.y),
							&(pMaterials[materialInd].diffuseColor.z),
							&(pMaterials[materialInd].diffuseColor.w) );

			ok += fscanf( f, "%f;\n", &(pMaterials[materialInd].power) );

			ok += fscanf(	f, "%f;%f;%f;;\n",
							&(pMaterials[materialInd].specularColor.x),
							&(pMaterials[materialInd].specularColor.y),
							&(pMaterials[materialInd].specularColor.z) );

			ok += fscanf(	f, "%f;%f;%f;;\n",
							&(pMaterials[materialInd].emissiveColor.x),
							&(pMaterials[materialInd].emissiveColor.y),
							&(pMaterials[materialInd].emissiveColor.z) );

			if ( ok != 11 ) return ERRFILECORRUPTED;

			pMaterials[materialInd].textureFileName = "";
		}



		// Texture file name found (should be in Material data block)
		if ( str.Find( " TextureFilename " ) != -1 )
		{
			if ( !ReadLine( f, _str ) ) return ERRFILECORRUPTED;

			int		ind1, ind2;
			
			str = _str;
			ind1 = str.Find( _T( '"' ) );
			ind2 = str.ReverseFind( _T( '"' ) );
			if ( ind1 == ind2 || ind1 == -1 || ind2 == -1 ) return ERRFILECORRUPTED;
			
			pMaterials[materialInd].textureFileName = str.Mid( ind1 + 1, ind2 - ind1 - 1 );
		}
	}
	

	fclose( f );

	if ( !uiNumVertices || !uiNumFaces || !uiNumMaterials ) return ERRFILECORRUPTED;

    return ERRNOERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// precomputes all numbers used when reducing mesh complexity
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CModel::PrecomputeData()
{
	HRESULT			hr;
	D3DXVECTOR3		vec0, vec1, vec2;
	
	if ( hr = BuildUpEdges() ) return hr;

	
	// add edge-indices to vertices
	for ( UINT i = 0; i < uiNumEdges; i++ )
	{
		pVertices[pEdges[i].vertices[0]].edges.push_back( i );
		pVertices[pEdges[i].vertices[1]].edges.push_back( i );
	}
	

	// add face-indices to vertices and compute face normals and areas
	for ( UINT i = 0; i < uiNumFaces; i++ )
	{
		pVertices[pFaces[i].vertices[0]].faces.push_back( i );
		pVertices[pFaces[i].vertices[1]].faces.push_back( i );
		pVertices[pFaces[i].vertices[2]].faces.push_back( i );

		vec0 = pVertices[pFaces[i].vertices[0]].pos - pVertices[pFaces[i].vertices[1]].pos;
		vec1 = pVertices[pFaces[i].vertices[2]].pos - pVertices[pFaces[i].vertices[1]].pos;
		
		D3DXVec3Cross( &vec2, &vec0, &vec1 );
		D3DXVec3Normalize( &pFaces[i].normal, &vec2 );

		pFaces[i].area = 0.5f * D3DXVec3Length( &pFaces[i].normal );
	}


	// compute edge angles
	for ( UINT i = 0; i < uiNumEdges; i++ )
	{
		FLOAT	kosinus, dot;
		
		dot = D3DXVec3Dot( & pFaces[pEdges[i].faces[0]].normal, & pFaces[pEdges[i].faces[1]].normal );
		kosinus = dot / ( D3DXVec3Length( & pFaces[pEdges[i].faces[0]].normal ) *
					D3DXVec3Length( & pFaces[pEdges[i].faces[1]].normal ) );

		pEdges[i].angle = acos( kosinus );
	}


	bIsPrecomputed = true;

	return ERRNOERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// builds up whole edge-list from face-list and than reduces them to only the valid ones
// that were in pairs (now only one of them remains)
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CModel::BuildUpEdges()
{
	pEdges = new EDGE[3 * uiNumFaces];
	if ( !pEdges ) return ERROUTOFMEMORY;

	uiNumEdges = 0;


	// add all edges to a list
	for ( UINT i = 0; i < uiNumFaces; i++ )
	{
		pFaces[i].edges[0] = AddEdge( i, pFaces[i].vertices[0], pFaces[i].vertices[1] );
		pFaces[i].edges[1] = AddEdge( i, pFaces[i].vertices[1], pFaces[i].vertices[2] );
		pFaces[i].edges[2] = AddEdge( i, pFaces[i].vertices[2], pFaces[i].vertices[0] );
	}

	

	// put off all duplicates of another edges and all single edges
	UINT	count = uiNumEdges;
	for ( UINT i = 0; i < count; i++ )
	{
		if ( !pEdges[i].bValid ) continue;

		bool	found = false;
		
		for ( UINT j = i + 1; j < count; j++ )
		{
			if ( !pEdges[j].bValid || pEdges[j].vertices[0] != pEdges[i].vertices[0] ||
				pEdges[j].vertices[1] != pEdges[i].vertices[1] ) continue;
			// same edge found:
			found = true;
			pEdges[j].bValid = false;
			pEdges[i].ucNumFaces++;
			pEdges[i].faces[1] = pEdges[j].faces[0];
			uiNumEdges--;
			// edge index is invalid, repair face's information:
			ReplaceEdgeIndexInFace( pEdges[i].faces[1], j, i );

			break;
		}
		
		if ( !found )
		{
			pEdges[i].bValid = false;
			uiNumEdges--;
		}
	}

	// if resultant count of edges is wrong:
	if ( 3 * uiNumFaces != 2 * uiNumEdges ) return ERRWRONGMESHFORMAT;



	// reorder edges so the valid will be first in list
	UINT	l = 0, r = count - 1;
	while ( l < r )
	{
		// find invalid edge on the left and valid on the right
		while ( pEdges[l].bValid ) l++;
		while ( !pEdges[r].bValid ) r--;
		
		// edge index is going to be invalid, repair face's information:
		ReplaceEdgeIndexInFace( pEdges[r].faces[0], r, l );
		ReplaceEdgeIndexInFace( pEdges[r].faces[1], r, l );

		// move the edge
		pEdges[l] = pEdges[r];
		pEdges[r].bValid = false;
	}


	return ERRNOERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// use this method if any edge is changing its location (index) to another place
// this will set the face's index to an edge to a new index
//////////////////////////////////////////////////////////////////////////////////////////////
void CModel::ReplaceEdgeIndexInFace( UINT face, UINT oldEdge, UINT newEdge )
{
	for ( UINT i = 0; i < 3; i++ ) 
		if ( pFaces[face].edges[i] == oldEdge) pFaces[face].edges[i] = newEdge;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// adds a face's edge to an edge-list and increase uiNumEdges
// returns new uiNumEdges count
//////////////////////////////////////////////////////////////////////////////////////////////
UINT CModel::AddEdge( UINT face, UINT ind0, UINT ind1 )
{
	pEdges[uiNumEdges].vertices[0] = ind0 < ind1 ? ind0 : ind1;
	pEdges[uiNumEdges].vertices[1] = ind0 < ind1 ? ind1 : ind0;
	pEdges[uiNumEdges].faces[0] = face;
	pEdges[uiNumEdges].bValid = true;
	pEdges[uiNumEdges].ucNumFaces = 1;
	
	return uiNumEdges++;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// checks whether the loaded mesh is valid, that is - it has only edge-pairs, no single 
// or multiple edges;
// checks, if this volume is manifold
//////////////////////////////////////////////////////////////////////////////////////////////
bool CModel::IsValid()
{
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// saves a model to DirectX eXtension file
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CModel::SaveToXFile( LPCTSTR fileName )
{
	FILE	*f;
	CString	str;
	HRESULT	hr;

#define BADFILERETURN	{ fclose( f ); return ERRFILEWRITE; };

	
	f = fopen( fileName, "wt" );
	if ( !f ) return ERRCANNOTOPENFORWRITE;

	
	hr = fprintf( f, "xof 0303txt 0032\n\nMesh {\n %u;\n", uiNumVertices );
	if ( hr < 0 ) BADFILERETURN;



	// write vertices coordinates
	for ( UINT i = 0; i < uiNumVertices - 1; i++ )
	{
		hr = fprintf(	f, " %f;%f;%f;,\n",
						pVertices[i].pos.x,
						pVertices[i].pos.y,
						pVertices[i].pos.z );
		if ( hr < 0 ) BADFILERETURN;
	}

	hr = fprintf(	f, " %f;%f;%f;;\n",
					pVertices[uiNumVertices - 1].pos.x,
					pVertices[uiNumVertices - 1].pos.y,
					pVertices[uiNumVertices - 1].pos.z );
	if ( hr < 0 ) BADFILERETURN;
	
	

	// write faces (vertex-indices)
	fprintf( f, " %u;\n", uiNumFaces );
	for ( UINT i = 0; i < uiNumFaces - 1; i++ )
	{
		hr = fprintf(	f, " %u;%u,%u,%u;,\n", 3,
						pFaces[i].vertices[0],
						pFaces[i].vertices[1],
						pFaces[i].vertices[2] );
		if ( hr < 0 ) BADFILERETURN;
	}

	hr = fprintf(	f, " %u;%u,%u,%u;;\n", 3,
					pFaces[uiNumFaces - 1].vertices[0],
					pFaces[uiNumFaces - 1].vertices[1],
					pFaces[uiNumFaces - 1].vertices[2] );
	if ( hr < 0 ) BADFILERETURN;



	// write material data
	hr = fprintf(	f, "\n MeshMaterialList {\n  %u;\n  %u;\n", uiNumMaterials, uiNumFaces );
	if ( hr < 0 ) BADFILERETURN;

	for ( UINT i = 0; i < uiNumFaces - 1; i++ )
	{
		hr = fprintf( f, "  %u,\n", pFaces[i].materialIndex );
		if ( hr < 0 ) BADFILERETURN;
	}

	hr = fprintf( f, "  %u;\n\n", pFaces[uiNumFaces - 1].materialIndex );
	if ( hr < 0 ) BADFILERETURN;

	
	for ( UINT i = 0; i < uiNumMaterials; i++ )
	{
		hr = fprintf(	f, "  Material {\n   %f;%f;%f;%f;;\n   %f;\n   %f;%f;%f;;\n   %f;%f;%f;;\n",
						pMaterials[i].diffuseColor.x,
						pMaterials[i].diffuseColor.y,
						pMaterials[i].diffuseColor.z,
						pMaterials[i].diffuseColor.w,
						pMaterials[i].power,
						pMaterials[i].specularColor.x,
						pMaterials[i].specularColor.y,
						pMaterials[i].specularColor.z,
						pMaterials[i].emissiveColor.x,
						pMaterials[i].emissiveColor.y,
						pMaterials[i].emissiveColor.z );
		if ( hr < 0 ) BADFILERETURN;

		if ( pMaterials[i].textureFileName != "" )
		{
			hr = fprintf( f, "\n   TextureFilename {\n    \"%s\";\n   }\n", pMaterials[i].textureFileName );
			if ( hr < 0 ) BADFILERETURN;
		}

		hr = fprintf( f, "  }\n\n" );
		if ( hr < 0 ) BADFILERETURN;
	}



	// write normals data
	hr = fprintf( f, " }\n\n MeshNormals {\n  %u;\n", uiNumFaces );
	if ( hr < 0 ) BADFILERETURN;

	for ( UINT i = 0; i < uiNumFaces - 1; i++ )
	{
		hr = fprintf(	f, "  %f;%f;%f;,\n", 
						pFaces[i].normal.x, 
						pFaces[i].normal.y, 
						pFaces[i].normal.z );
		if ( hr < 0 ) BADFILERETURN;
	}

	hr = fprintf(	f, "  %f;%f;%f;;\n  %u;\n", 
					pFaces[uiNumFaces - 1].normal.x, 
					pFaces[uiNumFaces - 1].normal.y, 
					pFaces[uiNumFaces - 1].normal.z,
					uiNumFaces );
	if ( hr < 0 ) BADFILERETURN;


	for ( UINT i = 0; i < uiNumFaces - 1; i++ )
	{
		hr = fprintf( f, "  3;%u,%u,%u;,\n", i, i, i );
		if ( hr < 0 ) BADFILERETURN;
	}

	hr = fprintf(	f, "  3;%u,%u,%u;;\n }\n\n MeshTextureCoords {\n  %u;\n",
					uiNumFaces - 1,
					uiNumFaces - 1,
					uiNumFaces - 1,
					uiNumVertices );
	if ( hr < 0 ) BADFILERETURN;
	


	// write texture coordinates
	for ( UINT i = 0; i < uiNumVertices - 1; i++ )
	{
		hr = fprintf(	f, "  %f;%f;,\n",
						pVertices[i].textureCoords.x, 
						pVertices[i].textureCoords.y );
		if ( hr < 0 ) BADFILERETURN;
	}

	hr = fprintf(	f, "  %f;%f;;\n }\n}",
					pVertices[uiNumVertices - 1].textureCoords.x, 
					pVertices[uiNumVertices - 1].textureCoords.y );
	if ( hr < 0 ) BADFILERETURN;



	fclose( f );
	
	
	return ERRNOERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// performs a faces reduction controlled by specified parameters
// a new CModel object is created, filled with generated data and a pointer to it
// returned as first parameter
// see the header of Model.h for further information
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CModel::Reduce( CModel *pModel, UINT angle, float distEdge, float distPlane, float criterion )
{
	FLOAT			fPhi = angle * D3DX_PI / 180.f;
	FLOAT			AreaSume, Distance;
	D3DXVECTOR3		NormalsSume, NormalVec, RadiusSume, RadiusVec;
	

	// copy the model
	pModel->pEdges = new EDGE[uiNumEdges];
	pModel->pVertices = new VERTEX[uiNumVertices];
	pModel->pFaces = new FACE[uiNumFaces];
	pModel->pMaterials = new MATERIAL[uiNumMaterials];

	if ( !pEdges || !pVertices || !pFaces || !pMaterials ) return ERROUTOFMEMORY;

	for ( UINT i = 0; i < uiNumFaces; i++ ) pModel->pFaces[i] = pFaces[i];
	for ( UINT i = 0; i < uiNumMaterials; i++ ) pModel->pMaterials[i] = pMaterials[i];
	for ( UINT i = 0; i < uiNumVertices; i++ ) pModel->pVertices[i] = pVertices[i];
	for ( UINT i = 0; i < uiNumEdges; i++ ) pModel->pEdges[i] = pEdges[i];
	pModel->bIsPrecomputed = bIsPrecomputed;


	

	
	if ( !bIsPrecomputed ) PrecomputeData();

	AnalyzeItems( fPhi );

	
	AreaSume = 0.0f;
	NormalsSume = RadiusSume = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

	for ( UINT i = 0; i < uiNumVertices; i++ )
	{
		if ( pVertices[i].type != VT_SIMPLE ) continue;
		
		for ( INDICES_ITER iter = pVertices[i].faces.begin(); iter != pVertices[i].faces.end(); iter++ )
		{
			D3DXVECTOR3		V0, V1, V2, P0;

			// coordinates of vertices of the triangle
			V0 = pVertices[pFaces[*iter].vertices[0]].pos;
			V1 = pVertices[pFaces[*iter].vertices[1]].pos;
			V2 = pVertices[pFaces[*iter].vertices[2]].pos;

			// get the center of one from the triangle sides
			D3DXVec3Lerp( &P0, &V1, &V2, 0.5f );

			// compute 2/3 of triangle median (the center of the triangle)
			D3DXVec3Lerp( &RadiusVec, &P0, &V0, 2.0f / 3.0f );
			
			// increase sume of the areas of polygons around this vertex
			AreaSume += pFaces[*iter].area; 

			// sume of the radius vectors of centers of triangles around this vertex
			RadiusSume += pFaces[*iter].area * RadiusVec; 

			// increase sume of triangle normals
			NormalsSume += pFaces[*iter].area * pFaces[*iter].normal; 
		}
		
		NormalsSume /= AreaSume;
		
		// we need these to compute average plane
		NormalVec = NormalsSume / D3DXVec3Length( &NormalsSume );
		RadiusVec = RadiusSume / AreaSume;

		D3DXVECTOR3	pomVec = pVertices[i].pos - RadiusVec;

		// compute distance to average plane
		Distance = abs( D3DXVec3Dot( &NormalVec, &pomVec ) );

		if ( Distance < distPlane ) pVertices[i].cutOff = true;
	}



	for ( UINT i = 0; i < uiNumVertices; i++ )
	{
		if ( pVertices[i].cutOff ) pModel->PerformTriangulation( i );
	}



	return ERRNOERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// performs triangulation of a hole that will arise when the vertex will be cutted off
//////////////////////////////////////////////////////////////////////////////////////////////
void CModel::PerformTriangulation( UINT vertex )
{
	
}

//////////////////////////////////////////////////////////////////////////////////////////////
// marks all edges and vertices with its proper types on the base of the size of an angle
// between two adjacnet faces and specified limiting angle
//////////////////////////////////////////////////////////////////////////////////////////////
void CModel::AnalyzeItems( float angle )
{
	// mark "important" edges
	for ( UINT i = 0; i < uiNumEdges; i++ )
	{
		if ( pEdges[i].angle < angle ) pEdges[i].type = ET_NORMAL;
		else pEdges[i].type = ET_IMPORTANT;
	}

	// mark "important" vertices
	for ( UINT i = 0; i < uiNumVertices; i++ )
	{
		UINT	count = 0; // count of "Important" edges this vertex is lying on

		for ( INDICES_ITER iter = pVertices[i].edges.begin(); iter != pVertices[i].edges.end(); iter++ )
		{
			if ( pEdges[*iter].type == ET_IMPORTANT ) count++;
		}

		if ( !count ) pVertices[i].type = VT_SIMPLE;
		else if ( count < 3 ) pVertices[i].type = VT_EDGE;
		else pVertices[i].type = VT_CORNER;

		pVertices[i].cutOff = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
