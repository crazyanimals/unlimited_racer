/*
	Terrain:				CGrObjectSkyBox is a CGrObjectBase's descendant and is used to surround the scene with a textured box
							making an illusion of far places outside of the scene.

							Read all comments in GrObjectBase.h file before using this!!!

	Creation Date:			17.3.2004
	Last Update:			17.12.2005
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


// sides of the skybox
#define _SKYBOX_NORTH		0
#define _SKYBOX_EAST		1
#define _SKYBOX_SOUTH		2
#define _SKYBOX_WEST		3
#define _SKYBOX_UP			4
#define _SKYBOX_BOTTOM		5


namespace graphic
{

	class CGrObjectSkyBox : public CGrObjectBase
	{
	private:
		// types
		struct VERTEX
		{
			float		x, y, z;
			float		tu, tv;
		} ;
		static const DWORD	VertexFVF = ( D3DFVF_XYZ | D3DFVF_TEX1 );

	public:
		// constructors & destructors
							CGrObjectSkyBox();
							~CGrObjectSkyBox();

		// methods
		inline int			GetObjectType() { return GROBJECT_TYPE_SKYBOX; }; // returns type of object
		HRESULT				Init(	BOOL Static,
									CString fileName,
									UINT LODcount ); // loads 6 sky box textures
		HRESULT				Render(); // renders the terrain

	private:
		resManNS::RESOURCEID			pTextureID[6]; // array storing all 6 texture IDs
		LPDIRECT3DVERTEXBUFFER9	vertexBuffer; // vertex buffer containing box vertices

	} ;

	
	typedef CGrObjectSkyBox	*LPGROBJECTSKYBOX;

} // end of namespace
