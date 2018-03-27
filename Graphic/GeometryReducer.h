/*
	CGeometryReducer:		This class performs a geometry reduction based on removing multiple vertices.
							Multiple vertex is defined as a group of vertices in which no vertex is isolated;
							isolated vertex in group G is such a vertex V that there's no vertex U element of G
							that U != V and dist(U,V) < Epsilon. Epsilon is a constant here that defines how 
							close vertices have to be, to form a group - multiple vertex.

							This class lets you fill it with some geometry data (faces and vertices) and then
							perform a reduction of count of vertices/faces. The geometry quality reduction is not
							necessary (you can set Epsilon to zero), but can be performed if you need it.

							The output of this class is a newly created geometry that contains no multiple vertices.

							You can attach your own custom data to every face or vertex, so you will later
							find out which one it was. Geometry reduction is not performed on group of vertices
							that contains different custom data (only pointer is considered). Also faces are
							not removed under the same circumstances. So it is highly recommended to attach
							the custom data reasonably so that the reduction can be effective. You can turn this
							feature off, so that even vertices/faces with different custom data are removed.
							In such case, however, it's not defined which data will be used when joining vertices/faces.

							No two faces can be compound of the same vertex triplet. The graph don't have to be 
							enclosed in the space, that means - there could be some edges that are a part of 
							ONE face only. However, no edge can be a part to less than one, or more than two faces.

							WARNING: This class is hardly finished! There's no UserData support, and many other missing features.

	Creation Date:			19.1.2006
	Last Update:			24.4.2006
	Author:					Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"


namespace graphic
{

// used constants
#define GRC_MIN_REQUIREMENTS		16
#define GRC_MIN_INDICES_COUNT		(256 * GRC_MIN_REQUIREMENTS)
#define GRC_MIN_VERTICES_COUNT		(128 * GRC_MIN_REQUIREMENTS)
#define GRC_MIN_FACES_COUNT			(64 * GRC_MIN_REQUIREMENTS)
#define GRC_MIN_GROUPS_COUNT		(64 * GRC_MIN_REQUIREMENTS)
#define GRC_MIN_GROUP_SIZE			8



	/////////////////////////////
	// GEOMETRY REDUCER CLASS
	/////////////////////////////
	class CGeometryReducer
	{
	public:
		friend int	__cdecl GRCSortKernelX( void * context, const void * v0, const void * v1 );
		friend int	__cdecl GRCSortKernelZ( void * context, const void * v0, const void * v1 );
		
		// forward declarations
		struct GRC_VERTEX;
		struct GRC_VERTEXGROUP;
		struct GRC_EDGE;
		struct GRC_FACE;


		
		// TYPES & STRUCTURES
		typedef int					GRC_ID;
		
		typedef DWORD				GRC_INDEX; // 32bit vertex index
		
		typedef LPVOID				GRC_USERDATA; // long pointer to any custom data

		typedef std::vector<GRC_VERTEXGROUP>	GRC_VGROUP_LIST;
		typedef	GRC_VGROUP_LIST::iterator		GRC_VGROUP_ITER;


		enum GRC_STATE
		{
			GRS_UNDEFINED	= 0, // object is in this state before Init() is called
			GRS_INITIALIZED	= 1, // this state is set after Init() is called, but before CloseInput()
			GRS_PREPARED	= 2, // after CloseInput() call, the object is in PREPARED state; then ReduceGeometry() method can be called
			GRS_FORCE_DWORD	= 0xffffffff // this is here only to force compiler to comile it to DWORD structure
		};


		// single vertex structure; each vertex is joining exactly one group of close vertices
		// vertex is initialized with position and custom data; vertex normal is computed later
		// as an average of normals of surrounding faces
		struct GRC_VERTEX
		{	
			GRC_ID			id; // vertex unique ID
			GRC_ID			newID; // ID to the field of new (regenerated) vertices - user only during colapsing process
			D3DXVECTOR3		position; // 3D local coordinate
			GRC_ID			idUserData; // user data; vertices with different UserData ID won't be joined !!!
			D3DXVECTOR3		normal; // computed normal of the vertex
			GRC_ID			idGroup; // group ID of which this vertex is a member
			BOOL			bDontTouch; // if set to true, this vertex can't be removed, joined, replaced or moved
			int				iReserved; // global purpose variable
		};

		
		// edge structure is a compound of exactly two vertices and points to 1-2 attached faces
		// it's natively oriented so is directs from the lesser index to the higher index
		struct GRC_EDGE
		{
			GRC_ID			id; // edge unique ID
			GRC_ID			ind0, ind1; // IDs of vertex indices in index buffer
			GRC_ID			face0, face1; // face IDs; face0 is on the left side when v0 is at the bottom and v1 on top
			GRC_ID			idUserData; // ID of the attached user data block
			BOOL			bLeftFaceDefined, bRightFaceDefined; // specifies which faces are defined
			int				iFaceCount; // count of attached faces <0,2>
		};


		// face is a compound of exactly three vertices and three edges; both appears in clockwise order
		// face normal is computed from vertices position
		struct GRC_FACE
		{
			GRC_ID			id; // face unique ID
			GRC_ID			ind[3]; // IDs of vertex indices in clockwise order
			GRC_ID			edge[3]; // IDs of edges in clockwise order
			D3DXVECTOR3		normal; // face normal vector
			GRC_ID			idUserData; // ID of the attached user data block
		};

		
		struct GRC_VERTEXGROUP;
		typedef GRC_VERTEXGROUP * LPGRC_VERTEXGROUP;

		// group of close vertices (vertices of one group will be joined together)
		struct GRC_VERTEXGROUP
		{
			GRC_ID				id; // group unique id
			GRC_ID				newID; // id used only during vertex colapsing process - points to the active
			int					iCount; // count of vertices
			int					iMemAllocated; // size of allocated memory in bytes
			GRC_ID			*	pVertices; // pointer to vertex indices

								GRC_VERTEXGROUP();
								~GRC_VERTEXGROUP();
			HRESULT				Init( int count = 0 );
			inline void			Free() { SAFE_DELETE_ARRAY( pVertices ); }
			HRESULT				Alloc( int count = 0 );
			HRESULT				AddVertex( GRC_ID vid );
			HRESULT				JoinGroup( LPGRC_VERTEXGROUP pGroup );
			
		};


		// GRC_VERTEXINITDATA structure is used in AddFace method for definition of vertices being added
		struct GRC_VERTEXINITDATA
		{
			D3DXVECTOR3		Position; // vertex 3D coordinates
			BOOL			bDontTouch; // set this to true to suppress possible vertex reduction
			LPVOID			pUserData; // user data pointer, set to NULL to disable user data for this vertex
		};


		// structure used in GetD3DBuffers as a definition of output data format
		struct GRC_OUTPUTDATA
		{
			// input params
			IDirect3DDevice9 *	pD3DDevice; // pointer to a valid D3D9 device
			UINT				uiVertexStride; // size of the vertex structure in bytes
			DWORD				dwNormalOffset; // this parameter must contain the offset of the Normal data in the vertex declaration (in bytes)
			DWORD				dwVBufferUsage; // usage parameter for the vertex buffer creataion method
			DWORD				dwIBufferUsage; // usage parameter for the index buffer creataion method
			D3DPOOL				VBufferPool; // pool used for vertex buffer
			D3DPOOL				IBufferPool; // pool used for index buffer
			DWORD				dwFVF; // flexible vertex format for vertex buffer creation method
			D3DFORMAT			IBufferFormat; // format of the index buffer

			// output params
			int					iVertexCount; // contains count of vertices
			int					iIndexCount; // contains count of indices
		};




		// PUBLIC METHODS
							CGeometryReducer();
							~CGeometryReducer();

		HRESULT				Init( int facesCount = 0, int indicesCount = 0, int verticesCount = 0 );
		HRESULT				AddFace( LPVOID, GRC_VERTEXINITDATA *, GRC_VERTEXINITDATA *, GRC_VERTEXINITDATA * );
		HRESULT				CloseInput();
		HRESULT				ReduceGeometry( float epsilon, bool optimizefaces, int width, int depth, int division );
		HRESULT				GetD3DBuffers( GRC_OUTPUTDATA *, IDirect3DVertexBuffer9 **, IDirect3DIndexBuffer9 ** );
		HRESULT				GetUserData(); //TODO: THIS IS NOT IMPLEMENTED YET !!!!!


	private:
		// PRIVATE PROPERTIES
		GRC_STATE			State;					
							
		int					iFaceCount; // count of items (not whole allocated memory has to be used)
		int					iIndexCount; // count of items (not whole allocated memory has to be used)
		int					iVertexCount; // count of items (not whole allocated memory has to be used)
		int					iEdgeCount; // count of items (not whole allocated memory has to be used)
		int					iGroupCount; // count of items (not whole allocated memory has to be used)
		int					iNewVertCount; // count of items (not whole allocated memory has to be used)
		int					iUserDataCount; // count of items (not whole allocated memory has to be used)
		
		int					iActiveGroups; // count of groups being used
							
		int					iVertMemAllocated; // size of allocated memory in bytes
		int					iIndMemAllocated; // size of allocated memory in bytes
		int					iEdgeMemAllocated; // size of allocated memory in bytes
		int					iFaceMemAllocated; // size of allocated memory in bytes
		int					iGroupMemAllocated; // size of allocated memory in bytes
		int					iNewVertMemAllocated; // size of allocated memory in bytes
		int					iUserDataMemAllocated; // size of allocated memory in bytes

		GRC_VERTEX		*	pVertices;
		GRC_INDEX		*	pIndices;
		GRC_EDGE		*	pEdges;
		GRC_FACE		*	pFaces;
		GRC_USERDATA	*	pUserDataBlocks;
		GRC_VERTEXGROUP	*	pGroups;
		GRC_INDEX		*	pTempIndicesX; // used only for sorting function
		GRC_INDEX		*	pTempIndicesZ; // used only for sorting function
		GRC_VERTEX		*	pNewVertices; // new vertices generated from vertex groups
				


		// PRIVATE METHODS
		void				Free();
		HRESULT				AddVertex( GRC_VERTEXINITDATA * pVert );
		HRESULT				AddEdge( GRC_ID ind0, GRC_ID ind1, GRC_ID face, GRC_ID * edge );
		HRESULT				AddUserDataBlock( LPVOID pUserData, GRC_ID * returnedId );
		bool				ConnectedGraph();
		HRESULT				JoinGroups( GRC_VERTEX * vert0, GRC_VERTEX * vert1 );
		void				AddNewFace( GRC_ID ind0, GRC_ID ind1, GRC_ID ind2 );

		static HRESULT		SatisfyMemoryRequirements( LPVOID * pBuf, int * actualSize, int requestedSize );

	} ;

	typedef CGeometryReducer	*	LPGEOMETRYREDUCER;


} // end of namespace
