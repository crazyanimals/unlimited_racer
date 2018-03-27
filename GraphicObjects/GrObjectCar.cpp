#include "stdafx.h"
#include "GrObjectCar.h"

using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders the object with per pixel specular lighting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectCar::RenderSpecular()
{
	LPD3DXMESH			Mesh;
	resManNS::XFile	*	pXFile;
	resManNS::Texture *	pTextureStruct;
	resManNS::Texture *	pTextureStructOld;
	vector<UINT>	*	ExtendedTable;

	
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	if ( WorldMatNeedRecount ) CountWorldMat();
#endif
	
	// place an object to scene
	Shader->SetWorldMatrix( &WorldMat );


	pXFile = ResourceManager->GetXFile( pLODs[uiLODToRender].ridXFile ); // find xFile resource

#ifdef MY_DEBUG_VERSION
	if ( !pXFile ) ERRORMSG( ERRNOTFOUND, "CGrObjectMesh::Render()", "Invalid model pointer." );
#endif // defined MY_DEBUG_VERSION

	Mesh = pXFile->Mesh; // obtain a Mesh pointer
	
	ExtendedTable = Shader->GetExtendedTable( Shader->Material );
	
	// render an object
	pTextureStructOld = NULL;

	for ( DWORD i = 0; i < pXFile->cMtrl; i++ ) // draw each subset
	{
		if ( bUsePredefinedMaterial )
		{
			Shader->SetPSValue( ExtendedTable->at(0), &pXFile->Material[i].Diffuse, 16 );
			Shader->SetPSValue( ExtendedTable->at(1), &pXFile->Material[i].Specular, 16 );
		}
		else 
		{
			Shader->SetPSValue( ExtendedTable->at(0), &OptionalMaterial.Diffuse, 16 );
			Shader->SetPSValue( ExtendedTable->at(1), &OptionalMaterial.Specular, 16 );
		}
		
		// use a texture if it is defined
		if ( pLODs[uiLODToRender].pSubsets[i].GetTextureID() > 0 ) 
		{
			pTextureStruct = ResourceManager->GetTexture( pLODs[uiLODToRender].pSubsets[i].GetTextureID() );
			// change texture only if it is differs from the last one
			if ( pTextureStruct != pTextureStructOld )
				D3DDevice->SetTexture( 1, pTextureStruct->texture );

			// render a subset of vertices
			Mesh->DrawSubset( i );

			pTextureStructOld = pTextureStruct;
		}
	}


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
