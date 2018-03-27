/*
	GrObjectMesh:	This class inherits from GrObjectBase class. It is used to load and work with packs of X files.
					Every such pack contains a bundle of LODs of a single model. 

					Read all comments in GrObjectBase.h file before using this!!!

	Creation Date:	26.10.2003
	Last Update:	17.12.2005
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GrObjectType.h"
#include "BoundingBox.h"
#include "GrObjectBase.h"
#include "GrObjectBillboard.h"
#include "ShadowVolume.h"
#include "Light.h"


namespace graphic
{

	
	class CGrObjectMesh : public CGrObjectBase
	{
		// type & structure definitions

		typedef std::map<CString,CString>	STRREMAP;
		typedef std::pair<CString,CString>	STRREMAP_PAIR;
		typedef STRREMAP::iterator			STRREMAP_ITER;
		
		// following structure is used for loading model definition from modelDef file
		struct LOADINGSTRUCT
		{
			bool				bDefined; // true if this LOD is already defined
			STRREMAP			TexRemapTable; // this structure remaps texture names (defines substitutions)
			CString				ModelFileName; // .x file name
		} ;

		// MeshSubset structure defines texture usage of mesh polygons subset
		struct MeshSubset
		{
			resManNS::RESOURCEID			ridOrigTexture; // RID of the originally loaded texture - stored for proper deallocation in destructor
			resManNS::RESOURCEID			ridNewTexture; // RID of the substituted texture - can be -1, in such case the ridOrigTexture is used
			
			// GetTextureID() method returns RID of the texture that should be used on this polygon subset
			inline resManNS::RESOURCEID	GetTextureID() { return ridNewTexture >= 0 ? ridNewTexture : ridOrigTexture; }
		} ;

		// single LOD definition
		struct ModelLOD
		{
			resManNS::RESOURCEID		ridXFile; // RID of the resManNS::XFile resource associated with this LOD
			UINT			uiSubsetsCount; // count of subsets
			MeshSubset	*	pSubsets; // array of polygon subset definitons
		} ;

		enum LOADINGSTATETYPE
		{
			LS_Undefined,
			LS_LODTag,
			LS_TexturesTag,
			LS_Skip
		} ;


	public:
		// public properties
		BOOL				bUsePredefinedMaterial;


		// constructors & destructors
							CGrObjectMesh();
							~CGrObjectMesh();
		
		// public methods
		inline virtual int	GetObjectType() { return GROBJECT_TYPE_MESH; }; // returns type of object

		HRESULT				Init(	BOOL Static, 
									CString fileName,
									UINT LODcount );
		HRESULT				InitShadowVolumes( D3DXVECTOR3 * pLightPos, bool _static = false );

		void				Free();

		
		HRESULT				Render();
		HRESULT				RenderShadowVolume( int iWhichSide = 0 );
		
		void				SetMaterial( D3DMATERIAL9 *mtrl );
		inline void			SetLOD( UINT lod ) { uiLODToRender = min( lod, uiLODCount - 1 ); }; // sets LOD level which to render
		LPD3DXMESH			GetMesh( int iLOD = -1 );

		inline void			SetLight( CLight * _Light ) {Light = _Light; };
		inline CLight	*	GetLight() { return Light; };
		inline CGrObjectMesh * GetWheel( UINT WheelNum ) { return Wheels[min(WheelNum, 3) ]; }
		inline void			SetWheel( UINT WheelNum, CGrObjectMesh * Wheel ) { Wheels[min(WheelNum, 3) ] = Wheel; }

		// Billboards for this object
		std::vector<CGrObjectBillboard *> Billboards;
		std::vector< resManNS::LightObject *> BillboardsObjects;

	protected:
		// methods
		void				PreInitThis();
		void				PreInit();

		// private properties
		ModelLOD			pLODs[MODEL_LOD_COUNT]; // LOD definitions (with model RIDs)
		CShadowVolume	*	ShadowVolumes[MODEL_LOD_COUNT]; // shadow volume objects created for every LOD
		D3DMATERIAL9		OptionalMaterial; // optional material is used if UsePredefinedMaterial is false
		UINT				uiLODCount; // count of LODs actually available
		UINT				uiLODToRender; // specifies which LOD variant to render
		LOADINGSTRUCT		pLoadingStructs[MODEL_LOD_COUNT]; // structure used only when model definition is being loaded
		LOADINGSTATETYPE	LoadingState; // defines the state in which the loading process is
		int					iLoadingActiveLOD; // specifies the LOD index that is just being loaded 
		CLight			*	Light; // Pointer to light
		CGrObjectMesh	*	Wheels[4]; // Wheel meshes for the car
		
		// private methods
		void static CALLBACK	LoadModelDefinitionCallBack( LPNAMEVALUEPAIR pPair );
		void CALLBACK			LoadModelDefinitionCallBackNonStatic( LPNAMEVALUEPAIR pPair );

	} ;


	typedef CGrObjectMesh	*LPGROBJECTMESH;


} // end of namespace