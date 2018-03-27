/*
	Scene:			This class is representing whole scene. Is passed to Graphic object to be rendered.
					Stores a couple of various buffers that contains pointers to graphic scene objects, meshes, triangles and so on.
	Creation Date:	12.11.2003
	Last Update:	2.3.2007
	Author:			Roman Margold
*/

#pragma once

#include "stdafx.h"
#include "GrObjectMesh.h"
#include "GrObjectTerrainPlate.h"
#include "GrObjectTerrain.h"
#include "GrObjectSkyBox.h"
#include "GrObjectSkySystem.h"
#include "GrObjectGrass.h"
#include "GrObjectCar.h"
#include "Light.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"


namespace graphic
{

	typedef std::vector<LPGROBJECTBASE>	GROBJECTS;
	typedef GROBJECTS::iterator			GROBJECTS_ITER;

	typedef std::vector<resManNS::RESOURCEID>	RESOURCES;
	typedef RESOURCES::iterator			RESOURCES_ITER;

	typedef std::vector<CLight * >		LIGHTS;
	typedef LIGHTS::iterator			LIGHTS_ITER;



	class CScene
	{
	friend class CGraphic;

	// defines a prototype for all grass fields that has the same terrain plate and grass type
	// thus these two items defines the unique ID for grass object
	struct GRASSID
		{
			CString				csFileName; // grass file name
			resManNS::RESOURCEID			ridSurface; // RID of the highest used LOD of the surface
			CGrObjectGrass	*	pGrass;
		};
		
		typedef std::vector<GRASSID>		GRASSPROTOTYPES;
		typedef GRASSPROTOTYPES::iterator	GRASS_ITER;

	public:
		// variables
		BOOL				Exists; // Indicates, whether scene was loaded or not

		CGrObjectTerrain 	Map;

		CGrObjectSkySystem  SkySystem; // sky system

		GROBJECTS			StaticObjects; // a list of all static renderable objects
		GROBJECTS			DynamicObjects; // a list of all dynamic renderable objecs
		GROBJECTS			TransparentObjects; // a list of all (possibly/partially) transparent renderable objecs
		GROBJECTS			Cars; /* list of all car objects
									THESE POINTERS MUST ALSO BE PRESENT IN DynamicObjects LIST */
		RESOURCES			Resources; // list of resource id's - mainly textures
		LIGHTS				Lights; // scene lights of all type, but without a Sun and sprite lights
		CLight *			AmbientLight; // ambient light in scene
		CLight *			SunLight; // sun directional light in scene

		D3DXVECTOR3			FogColor;

		
		D3DMATERIAL9		GreenMaterial, BlackMaterial, RedMaterial, 
							WhiteMaterial, BlueMaterial, YellowMaterial, 
							CyanMaterial, MagentaMaterial, BlendedMaterial, GrayMaterial;

		D3DMATERIAL9		FreeUseMaterial; // a material usable for any purposes
		
		// methods
							CScene();
							~CScene(); // frees every graphic objects stored in Scene

		HRESULT				Init( resManNS::CResourceManager *ResManager ); // inits the Scene object

		HRESULT				InitGrassMap( UINT lodcount );

		HRESULT				AddGrassToField( CString fileName, UINT x, UINT z );

		void				SetMaterial( FLOAT r, FLOAT g, FLOAT b, FLOAT a = 1.0f ); // fills the FreeUseMateril structure with this color

		void				Free();
		void				FreeLights();
		void				FreeObjects();

	private:
		resManNS::CResourceManager *	ResourceManager; // stored pointer to a ResourceManager object
		LPGROBJECTGRASS	*	pGrassFields;
		GRASSPROTOTYPES		GrassList; // list of all grass prototypes
		UINT				uiGrassLODCount; // count of LODs used for grass
	
	} ;


	typedef CScene			*LPSCENE;


} // end of namespace graphic

