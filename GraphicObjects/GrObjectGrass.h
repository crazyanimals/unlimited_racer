/*
	GrObjectGrass:	CGrObjectGrass class represents a group of dynamic low-poly "2D" objects composed of several quads.
					This group of objects is capable of time- and position- dependent animation.
					Also supports LOD (defined by count of segments = jointed quads).

					Read all comments in GrObjectBase.h file before using this!!!

	Creation Date:	27.7.2005
	Last Update:	24.12.2005
	Author:			Roman Margold
*/



#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GrObjectType.h"
#include "GrObjectBase.h"
#include "GrObjectTerrainPlate.h"



namespace graphic
{
	// forward declaration
	class CGrObjectGrass;


	// used for listing grass items for later processing when loading (initializing) grass group (CGrObjectGrass)
	struct GRASSITEMTOPROCESS
	{
		UINT		uiGrassItemIndex; // index to grass item prototypes list
		resManNS::RESOURCEID	ridTexture; // used texture RID
		float		x, y, z; // position
		float		Rotation; // rotation around the Y axis
		float		Scale; // uniform scaling
	} ;
	

	typedef vector<GRASSITEMTOPROCESS>		TOPROCESS_LIST;
	typedef TOPROCESS_LIST::iterator		TOPROCESS_ITER;
	typedef vector<resManNS::RESOURCEID>				RID_LIST;
	typedef RID_LIST::iterator				RID_ITER;
	
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// structure used when loading .grass file for passing parameters to callback methods
	//    and for driving the whole loading process
	// 
	//////////////////////////////////////////////////////////////////////////////////////////////
	struct GRASSLOADINGSTRUCT
	{
		// PROPERTIES

		// processing control properties
		CGrObjectGrass		*	pGrassObject; // pointer to calling CGrObjectGrass instance
		TOPROCESS_LIST		*	pItemsToProcess; // pointer to a list of items that should be processed later
		bool					bTextureFileDefined; // is set to true the first time texture file name is defined in the file
		bool					bInItemTag; // is set to true if an ITEM tag is beeing processed
		UINT					uiLODsRequested; // use to tell loader how many LODs are requested to load
		
		// global properties
		CString					csTextureFileName; // global resManNS::Texture property
		CString					csDefaultItemFileName; // global ItemFile property
		
		// ITEM object properties
		CString					csItemFile; // Item.File property
		float					fItemX, fItemY, fItemZ; // Item.XYZ properties
		float					fItemRotation; // Item.Rotation property
		float					fItemScale; // Item.Scale property - uses uniform scaling


		// METHODS
		inline void UseItemDefaults() // sets all properties related to ITEM object to default values
		{
			csItemFile = "";
			fItemX = 500;
			fItemY = 0;
			fItemZ = 500;
			fItemRotation = 0;
			fItemScale = 1;
		}
		
		inline void UseGlobalDefaults() // sets all global properties to default values
		{
			csTextureFileName = "";
			csDefaultItemFileName = "";
		}
		
		inline void	UseDefaults() { UseItemDefaults(); UseGlobalDefaults(); } // sets all properties to default values
		
		inline void Init( CGrObjectGrass * point, TOPROCESS_LIST * toProcessList, UINT lods )
			// sets all defaults and inits control properties - should be called befor used to Load operation
		{
			pGrassObject = point;
			pItemsToProcess = toProcessList;
			bTextureFileDefined = false;
			bInItemTag = false;
			uiLODsRequested = lods;
			UseDefaults();
		}
	} ;



	//////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// CGrassItems class covers vertex buffer with all grass item prototypes (shrubs) and methods
	//    for rendering and adding shrubs
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CGrassItems
	{
	public:		
		// constructor & destructor
								CGrassItems();
								~CGrassItems();

		// public methods
		HRESULT					Init( resManNS::CResourceManager * pResMan );
		HRESULT					Add( LPCTSTR fileName, UINT maxLOD );
		void					Free();

		inline resManNS::RESOURCEID		GetItemRID( UINT index ) const { return Items[index]; }; // returns RID of the specified prototype
		inline UINT				GetItemsCount() const { return uiItemsCount; }; // returns count of grass prototypes actually stored

	private:
		resManNS::CResourceManager	*	pResourceManager;
		UINT					uiItemsCount;
		RID_LIST				Items;

	} ;



	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// CGrObjectGrass - main class representing all LODs of one grass group, which is covering one
	//   particular terrain field type with particular rotation
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CGrObjectGrass :  public CGrObjectBase
	{
	public:
		typedef vector<UINT>					INDICES_LIST;
		typedef INDICES_LIST::iterator			INDICES_ITER;

		// grass vertex structure
		struct VERTEX
		{
			D3DXVECTOR3			Coord; // 3D coordinates
			D3DXVECTOR2			TexCoord; // 2D texturing coordinates - mapped to TEX0
			float				SinusY, CosinusY; // sinus and cosinus of rotation around Y axis - mapped to TEX1.uv
			float				Segment; // specifies vertical placement (segment position) <0,1> - mapped to TEX1.w
			D3DXVECTOR3			RPCoord; // coordinates of the relative point [0,0,0] (mapped to TEX2)
		} ;
		
		
		// vertex format definition
		static const DWORD		VertexFVF = (	D3DFVF_XYZ | D3DFVF_TEX3 |  
												D3DFVF_TEXCOORDSIZE2(0) | 
												D3DFVF_TEXCOORDSIZE3(1) | 
												D3DFVF_TEXCOORDSIZE3(2) );


		// constructors & destructors
								CGrObjectGrass();
								~CGrObjectGrass();
		
		// public methods
		inline virtual int		GetObjectType() { return GROBJECT_TYPE_GRASS; }; // returns the object's type

		HRESULT					Init(	BOOL Static, 
										CString fileName,
										UINT LODcount,
										CGrObjectTerrainPlate * pTerrainPlate );

		HRESULT					Render();
		HRESULT					Render( UINT uiLOD );
		
		inline void				SetMaterial( D3DMATERIAL9 *mtrl ) { Material = *mtrl; }; // can be used to change the default material
		inline void				SetLOD( UINT lod ) { uiLODToRender = min( lod, uiLODCount - 1 ); }; // sets the LOD that should be used for rendering

		const D3DMATERIAL9 *	GetMaterialPtr() { return &Material; }; // returns a pointer to material structure
	
	private:		
		CGrassItems				ItemPrototypes; // contains all the grass shrubs used in this grass field
		
		
		UINT					uiLODCount; // count of LODs actually available
		UINT					uiObjectsCount; // specifies the count of object in the group
		UINT					uiLODToRender; // specifies which LOD variant to render
		int						iMapPosX, iMapPosZ; // position in the map (field's coordinates)
		UINT					InitialIndices[MODEL_LOD_COUNT+1]; // specifies the indices of first item in index buffer of each LOD
		UINT					InitialVertices[MODEL_LOD_COUNT+1]; // specifies the indices of first item in vertex buffer of each LOD

		D3DMATERIAL9			Material; // initialized by default value, but could be updated any time
		resManNS::RESOURCEID	ridTexture; // stores an RID of texture that is used for all objects in the group
		
		INDICES_LIST			ItemsList; // complete list of all grass items - list of indices to prototypes list
		LPDIRECT3DVERTEXBUFFER9	pVertexBuffer; // this vertex buffer contains a geometry of ALL items in this group
		LPDIRECT3DINDEXBUFFER9	pIndexBuffer; // indexing a vertex buffer with complete geometry of ALL items in this group


		// private methods
		void static CALLBACK	LoadCallBack( LPNAMEVALUEPAIR pPair );
		float					GetSurfaceHeight( resManNS::GrSurface * pSurface, float x, float z );

	} ;


	typedef CGrObjectGrass	*LPGROBJECTGRASS;


} // end of namespace graphic
