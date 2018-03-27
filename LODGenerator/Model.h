/*
	Model:				Class that is used to store the whole 3d model in special representation an can do some oparations
						with him, like loading from XFile and generate less quaility model (LOD).
						There are some importent paramteres which has an influence to the quality and appearance of the output model:
							- maximal face angle - specifies a maximal angle between face normals that signs an egde between two faces as "border"
							- plane distance - maximal distance of a vertex to its "average plane" to be deleted from mesh
							- edge distance - maximal distance of a vertex on border edge from the new edge to be deleted from mesh
							- optimal criterion to create edge when making hole triangulation
	Creation Date:		30.3.2004
	Last Update:		1.4.2004
	Author:				Roman Margold

*/

#pragma once



#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"



// define edge types
#define ET_UNDEFINED		0
#define ET_NORMAL			1
#define ET_IMPORTANT		2
#define ET_BORDER			3
#define ET_COMPLEX			4

// define vertex types
#define VT_UNDEFINED		0
#define VT_SIMPLE			1
#define VT_EDGE				2
#define VT_CORNER			3
#define VT_BORDER			4
#define VT_COMPLEX			5



// new types
typedef std::list<UINT>				INDICES;
typedef std::list<UINT>::iterator	INDICES_ITER;


struct VERTEX
{
						VERTEX() { type = VT_UNDEFINED; cutOff = false; };					
						~VERTEX() { edges.clear(); faces.clear(); };
	
	D3DXVECTOR3			pos; // vertex pocition ( == radius vector )
	INDICES				edges; // list of edge's indices
	INDICES				faces; // list of face's indices
	BYTE				type; // vertex type
	D3DXVECTOR2			textureCoords; // 2-D coordinates of a texture
	bool				cutOff; // true signalizes that this vertex is to be cutted off
} ;


struct EDGE
{
						EDGE() { bValid = false; ucNumFaces = 0; type = ET_UNDEFINED; };
	
	UINT				vertices[2]; // vertex indices ( oriented edgde v0->v1 )
	UINT				faces[2]; // face's indices ( f0 is on the left, f1 on the right in edge direction )
	BYTE				type; // edge type
	float				angle; // angle between edge's faces
	BYTE				ucNumFaces; // count of incident faces (0-2)
	bool				bValid; // specifies if this edge is valid
} ;


struct FACE
{
						FACE() { cutOff = false; };
	
	UINT				vertices[3]; // vertex's indices
	UINT				edges[3]; // edge's indices
	D3DXVECTOR3			normal; // normal vector of this face
	float				area; // size of the area of this face
	UINT				materialIndex; // material index
	bool				cutOff;
} ;


// material structure - used only to store XFile data
struct MATERIAL
{
	D3DXVECTOR4			diffuseColor;
    FLOAT				power;
    D3DXVECTOR3			specularColor;
    D3DXVECTOR3			emissiveColor;
	CString				textureFileName;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
// main class storing whole mesh data
//////////////////////////////////////////////////////////////////////////////////////////////
class CModel
{
public:
						CModel();
						~CModel();
	HRESULT				LoadFromXFile( LPCTSTR fileName );
	HRESULT				SaveToXFile( LPCTSTR fileName );
	HRESULT				Reduce( CModel *pModel, UINT angle, float distEdge, float distPlane, float criterion );
	bool				IsValid();
	HRESULT				PrecomputeData();

private:
	UINT				uiNumVertices;
	UINT				uiNumFaces;
	UINT				uiNumEdges;
	UINT				uiNumMaterials;
	VERTEX				*pVertices;
	EDGE				*pEdges;
	FACE				*pFaces;
	MATERIAL			*pMaterials;
	bool				bIsPrecomputed; // specifies whether the values are already precomputed

	bool				ReadLine( FILE *f, char *s );
	UINT				AddEdge( UINT face, UINT ind0, UINT ind1 );
	HRESULT				BuildUpEdges();
	void				AnalyzeItems( float angle );
	void				ReplaceEdgeIndexInFace( UINT face, UINT oldEdge, UINT newEdge );
	void				PerformTriangulation( UINT vertex );

} ;
