/*
	GrObjectTerrainPlate:	This class inherits from GrObjectBase class. It is used to store one terrain plate object.
							This is not a terrain plate prototype, this contains a certain instance of it.
							Among others, it has a property that specifies which LOD to use when render.
							.grPlate is an appropriate file extension (format)

							Read all comments in GrObjectBase.h file before using this!!!

	Creation Date:			23.12.2003
	Last Update:			29.4.2006
	Author:					Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GrObjectType.h"
#include "GrObjectMesh.h"
#include "BoundingBox.h"
#include "GrObjectBase.h"
#include "Light.h"



#define MAX_ANGLE_DIFFERENCE		0.005f
#define MAX_COSINE_DIFFERENCE		0.001f

#define TERRAIN_TRANSLATION_HACK	30.0f // used for second pass terrain rendering - sealing hole in terrain 


namespace graphic
{


	class CGrObjectTerrainPlate : public CGrObjectBase
	{
	public:
	
		friend class CGrObjectTerrain;

		// types
		struct VERTEX // VERTEX format used by all terrain geometry
		{
			float		x, y, z;
			float		nx, ny, nz;
			float		tu, tv;
		} ;

		struct VERTEX_DX // the same as above, but rather using DX structures
		{
			D3DXVECTOR3	pos;
			D3DXVECTOR3	normal;
			D3DXVECTOR2	texcoord;
		} ;

		static const DWORD	VertexFVF = ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );

		
		// constructors & destructors
							CGrObjectTerrainPlate();
							~CGrObjectTerrainPlate();
		
		// object identification method
		inline virtual int	GetObjectType() { return GROBJECT_TYPE_TERRAINPLATE; }; // returns type of object


		// initializing methods
		HRESULT				Init(	BOOL Static,
									CString fileName,
									UINT LODcount );

		HRESULT				InitLightMap( CLight * Light );

		void				Free();


		// rendering methods
		HRESULT				RenderLightMap(); // renders the plate for the light map texture
		HRESULT				Render() { return Render( false ); }
		HRESULT				Render( bool secondPass ); // sets all needed states and renders the object, then restores all saved state

		
		// redefine inherited method
		virtual void		InitialWorldPlacement( D3DXVECTOR3 & position, D3DXVECTOR3 & rotation, D3DXVECTOR3 & scale );

		
		// resource assignment methods
		void				SetPlate( resManNS::RESOURCEID resID ); // assigns the geometry pack with all LoDs to this object
		void				SetTexture( resManNS::RESOURCEID textureID ); // change texture on object
		HRESULT				LoadTexture( CString fileName ); // loads a texture and assigns to this plate as the primary layer
		void				SetRoadTexture( resManNS::RESOURCEID ridTex ); // sets the road texture (second layer)
		HRESULT				LoadRoadTexture( CString fileName ); // loads a texture and assigns to this plate as the secondary layer

		
		// property setting methods
		void				SetLOD( UINT lod ) { uiLODToRender = lod; }; // sets LOD level which to render
		void				SetMaterial( D3DMATERIAL9 *mtrl ); // sets a material, the default material is gray
		void				SetTextureRotation( float angle ); // sets the rotation angle of texture
		void				SetRoadTextureRotation( float angle ); // sets the rotation angle of road (=secondary) texture
		void				SetPlanar( bool planar = true ) { bPlanar = planar; } // sets the planar property
		void				SetConcave( bool concave = true ) { bConcave = concave; } // sets this plate concave flag

		bool				IsPlanar() const { return bPlanar; } // says whether this plate is planar
		bool				IsConcave() const { return bConcave; } // says whether this plate is concave

		inline UINT			GetLODCount() const { return uiLODCount; } // returns count of LODs loaded for this object

		
		// visibility methods
		void				SetVisibilityParams( char * bitField, char bitMask );
		void				SetVisible() { *pVisibilityBitField |= cVisibilityBitMask; }; // tells that this object interferes the viewing frustum
		void				SetInvisible() { *pVisibilityBitField &= ~cVisibilityBitMask; }; // tells that this object doesn't interfere the viewing frustum
		BOOL				IsVisible() { return (*pVisibilityBitField & cVisibilityBitMask); } // returns the on-screen visibility status
		
		
		// lightmap stuff
		IDirect3DTexture9 * GetLightMap() const { return LightMapTexture; }; // Gets light map texture
		CLight *			GetLamp() const { return Lamp; }; // get lamp light 
		void				SetLightMap( IDirect3DTexture9 * LightMap ) { LightMapTexture = LightMap; } // sets lightmap texture
		void				SetLamp( CLight * _Lamp ) { Lamp = _Lamp; } // sets the light for light map

		// object stuff
		inline std::vector<CGrObjectMesh *> * GetObjects() { return &Objects; }

		resManNS::RESOURCEID		GetTerrainType() const { return TerrainID[ uiLODCount - 1]; }; // returns resource ID o the highest used LOD of surface 
		resManNS::RESOURCEID		GetLODResourceID( UINT Lod ) const { return TerrainID[Lod]; }; // Gets certain lod resource ID


	
	protected:
		// methods
		void				PreInitThis();
		void				PreInit();


	private:
		// private properties
		resManNS::RESOURCEID			TerrainID[TERRAIN_LOD_COUNT]; // index to ResourceManager indexing a terrain object
		resManNS::RESOURCEID			ridGrPlate; // ID of the LoD's pack
		resManNS::RESOURCEID			TextureID; // texture ID as pointer to ResourceManager
		resManNS::RESOURCEID			ridRoadTexture; // ID of the texture of road (track) - second texture - not necessarry

		D3DMATERIAL9		Material;
		LPDIRECT3DTEXTURE9	LightMapTexture; // field's lightmap is available only if lamp is on the field (bContainsLamp)

		bool				bPlanar; // says whether this terrain plate is planar
		bool				bConcave; // set to true when the plate is concave
		UINT				uiLODToRender; // specifies which LOD variant to render
		UINT				uiLODCount; // count of LODs actually available

		D3DXMATRIX			TextureMatrix; // matrix used to rotate texture
		D3DXMATRIX			RoadTextureMatrix; // matrix used to rotate texture
		BOOL				bUseTextureRotation; // true if texture is rotated
		BOOL				bUseRoadTextureRotation; // true if secondary texture is rotated
		FLOAT				fTextureRotAngle, fRoadTextureRotAngle;
		FLOAT				fRotationAngle;

		D3DXMATRIX			TranslatedMatrix; // auxiliary matrix used during second render pass
		D3DXVECTOR3			TranslatedPosition; // translation hack for sealing terrain holes

		CLight *			Lamp; // Pointer to lamp light
		std::vector<CGrObjectMesh *> Objects; // Pointer to array of objects on terrain plate

		char			*	pVisibilityBitField; // pointer to the character in bit field that describes visibility of 8 adjacent fields
		char				cVisibilityBitMask; // specifies the exact bit in relevant character inside VisibilitiyBitField
									

		// private methods (internal use)
		void				ComputeTextureRotation(); // recomputes texture rotation matrix
		void				ComputeRoadTextureRotation(); // recomputes secondary texture rotation matrix

	} ;


	typedef CGrObjectTerrainPlate	*LPGROBJECTTERRAINPLATE;


} // end of namespace
