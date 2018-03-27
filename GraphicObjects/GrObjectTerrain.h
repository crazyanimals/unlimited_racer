/*
	Terrain:				CTerrain class stores whole terrain info and provides some methods to work with it.

							Read all comments in GrObjectBase.h file before using this!!!

	Creation Date:			24.12.2003
	Last Update:			30.4.2006
	Author:					Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GrObjectType.h"
#include "BoundingBox.h"
#include "GrObjectBase.h"
#include "GrObjectTerrainPlate.h"
#include "GrObjectSkyBox.h"
#include "GrObjectMesh.h"



#define BORDER_WALL_HEIGHT		(4 * ONE_METER)  // in meters
#define BORDER_WALL_THICKNESS      (60 * ONE_CENTIMETER)

#define TVC_MIN_DIFFERENCE		0.0000001f	// minimal difference allowed during terrain visibility computation


namespace graphic
{

	struct TERRAINFIELD
	{
		CGrObjectTerrainPlate TerrainPlate; // terrain plate object
		UINT				Height; // height level of field (zero based)
		UINT				Rotation; // Rotation*D3DX_PI/2 specifies the rotation angle of this field
		UINT				TextureRotation; // Rotation*D3DX_PI/2 specifies the rotation angle of primary texture
		resManNS::RESOURCEID			TextureID; // texture ID obtained by ResourceManager
		BOOL				Lamp; // specifies whether this field contains a lamp object iluminating it

							TERRAINFIELD() { TextureID = -1; }
	} ;
	
	typedef TERRAINFIELD	*LPTERRAINFIELD;

	
	// map bounding box vertices
	struct MAPBBVERTEX
	{
		float x, y, z;	
	} ;

	struct MAPBBVERTEXTRANS
	{
		float x, y, z, w;
	} ;


	// help structure used for map visibility testing
	// contains information about ray that is casted from camera position
	struct RAY
	{
		D3DXVECTOR3			coord; // world coordinates
		D3DXVECTOR3			cameraPos; // position in camera space
		D3DXVECTOR3			intersectPoint; // point of intersection with the plane
		bool				bCrossed;
		bool				bOriginal; // true if this ray was originally proper, false if it had to be validated by CreateAdditionalIntersectionPoint()
		int					iDirection; // -1 if this ray is directed downwards, 0 if is horizontall and +1 if it directs upwards
		int					iPosX, iPosZ; // map position
	} ;


	//////////////////////////////////////////////////////////////////////////////////////////////
	// main Terrain class
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CGrObjectTerrain : public CGrObjectBase
	{
	public:
		static const WORD		MAPBB_FVF;		// FV format of map's bounding box
		static const WORD		MAPBBTRANS_FVF;		// FV format of map's transformed bounding box
		static const D3DXMATRIX	IdentityMatrix; // identity matrix for free use :o)
		
		// constructors & destructors
								CGrObjectTerrain();
								~CGrObjectTerrain();

		// public methods
		inline virtual int		GetObjectType() { return GROBJECT_TYPE_TERRAIN; }; // returns type of object
		
		HRESULT					Init(	BOOL Static, 
										CString fileName,
										UINT LODcount ); // loads a terrain from file
		HRESULT					InitBorders();

		void					Free();


		HRESULT					Render(); // renders the terrain
		HRESULT					RenderBorders();
		
		HRESULT					RenderField( int x, int z, UINT LOD = 0, bool secondPass = false );
		HRESULT					RenderSection( int x1, int z1, int x2, int z2 );
		HRESULT					RenderSection( int x, int z, int range );

		CGrObjectTerrainPlate * GetPlate( int x, int z ); // returns terrain plate object pointer
		resManNS::__Terrain	*	GetTerrainResource(); // returns a pointer to terrain resource as is stored in resource manager

		inline UINT				GetWidth() { return SegsCountX; }; // returns count of fields in x-direction (width)
		inline UINT				GetDepth() { return SegsCountZ; }; // returns count of fields in z-direction (depth)
		UINT					GetFieldHeight( int x, int z ); // returns field height measured in terrain levels - zero based
		UINT					GetFieldHeight( MAPPOSITION *pos ); // returns field height measured in terrain levels - zero based
 		UINT					GetFieldRotation( int x, int z); // returns field rotation - 4 positions
		UINT					GetFieldRotation( MAPPOSITION * pos); // returns field rotation - 4 positions
		inline UINT				GetLODCount() const { return uiLODCount; }; // returns count of LODs 
		
		// returns field texture id
		inline resManNS::RESOURCEID	GetFieldTextureID( const int x, const int z)
		{
			if ( x < 0 || x >= (int) SegsCountX || z < 0 || z >= (int) SegsCountZ ) return ERRNOERROR;

			return pFields[z * SegsCountZ + x].TextureID;
		}

		void					SetWholeMapInvisible(); // sets all of the fields of the map invisibile (visibility status to FALSE)
		HRESULT					ComputeTerrainVisibility(	D3DVIEWPORT9 *viewPort,
															D3DXMATRIX *,
															FLOAT fovX,
															FLOAT fovY,
															FLOAT ZNear,
															FLOAT ZFar ); // determines the terrain fields visibility
		void					SetFieldVisible( int x, int z ); // sets the specified field visibility status to TRUE
		void					SetFieldInvisible( int x, int z ); // sets the specified field visibility status to FALSE
		void					SetFieldVisible( MAPPOSITION *pos ); // sets the specified field visibility status to TRUE
		void					SetFieldInvisible( MAPPOSITION *pos ); // sets the specified field visibility status to FALSE
		BOOL					IsFieldVisible( int index ); // returns true if the specified field visibility status is TRUE
		BOOL					IsFieldVisible( int x, int z ); // returns true if the specified field visibility status is TRUE
		BOOL					IsFieldVisible( MAPPOSITION *pos ); // returns true if the specified field visibility status is TRUE
		BOOL					ObjectIsVisible( MAPPOSITIONLIST *posList ); // returns true if any of the map positions has a visibility status TRUE

		void					BoundObject(CGrObjectMesh * Mesh); // sets object pointer according to object map position list on certain terrain plate
		void					UnBoundObject(CGrObjectMesh * Mesh); // deletes object pointer according to object map position list on certain terrain plates

	protected:
		// methods
		void					PreInitThis();
		void					PreInit();


	private:
		// private variables
		UINT					SegsCountX, SegsCountZ; // count of fields in width and depth
		LPTERRAINFIELD			pFields; // array of terrain fields
		int						*pLeftBorder, *pRightBorder; // auxiliary fields used for computing map visibility
		int						*pLeftBorderBackup, *pRightBorderBackup; // backup of auxiliary fields used for computing map visibility
		D3DXVECTOR3				IntersectionPoints[4]; // coordinates of points of intersection of view frustum and lower surface plane
		char				*	pVisibilityBitField; // field of bits describing visibility of all fields
		resManNS::RESOURCEID	ridTerrain; // stored terrain resource ID
		UINT					uiLODCount; // stores count of LODs that are used


		// private methods
		BOOL					PointIsInVisibleScreen( FLOAT x, FLOAT y, D3DVIEWPORT9 *viewPort ); // says whether the specified point in screen space is visible
		void					MakeTriangleVisible( RAY * rays ); 
		void					MakeLineVisible( RAY * ray1, RAY * ray2 ); // makes all fields lying on line between ray1 and ray2 visible
		HRESULT					PlaneSurfaceIntersection( D3DXVECTOR3 * point1, D3DXVECTOR3 * point2, D3DXPLANE * plane, FLOAT y ); // computes intersection line of general plane and horizontal plane
		int						RayIntersectionWithMapBorders( D3DXVECTOR3 * intersector, D3DXVECTOR3 * point1, D3DXVECTOR3 * point2 );
		void					CreateAdditionalIntersectionPoint( RAY * rayOk, RAY * rayNotOk, D3DXPLANE * plane, D3DXVECTOR3 * cameraPos );
		UINT					TerrainVisibilityForSinglePlane( RAY * rays, D3DXMATRIX * mat, D3DXVECTOR3 * cameraPos, FLOAT planeY ); // computes the terrain visibility for single horizontal plane


		// Terrain border wall stuff
		IDirect3DVertexBuffer9 *pBorderVBuffer; // geometry of the border wall
		IDirect3DIndexBuffer9 *	pBorderIBuffer; // index buffer for the border wall
		IDirect3DTexture9	*	pBorderTexture; // border wall texture - only an obtained pointer, this MUST NOT be released upon object death
		UINT					uiBorderVertCount; // count of vertices in pBorderVBuffer;
		UINT					uiBorderIndCount; // count of indices in pBorderIBuffer;
		D3DMATERIAL9			StandardMaterial;

		static const DWORD		BorderVertexFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

		struct BORDERVERTEX
		{
			D3DXVECTOR3 Pos;
			D3DXVECTOR3 Normal;
			D3DXVECTOR2 TexCoord;
		};

		void					GetBorderVertex( int platex, int platez, int index, D3DXVECTOR3 * pos );

	} ;											  

	
	typedef CGrObjectTerrain	*	LPGROBJECTTERRAIN;


} // end of namespace

