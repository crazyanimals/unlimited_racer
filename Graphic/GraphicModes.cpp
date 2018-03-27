#include "stdafx.h"
#include "GraphicModes.h"

using namespace graphic;

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//				   C G r a p h i c M o d e s      c l a s s
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGraphicModes::CGraphicModes()
{
	pModes = pModes = NULL;
	D3DCreated = FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor - releases the D3D object if it was created
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGraphicModes::~CGraphicModes()
{
	SAFE_FREE( pModes )
	SAFE_FREE( pModes )
	if ( D3DCreated ) SAFE_RELEASE( Direct3D ) 
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// allocates a memory for all modes; if CB func is not specified 
// (that is, the default NULL value is used), the standard CB enum func is used
//
// Direct3D parameter can be NULL, the new D3D object is created in that case and released in
// the destructor
// this method could be called repeatedly on the same object
//
// returns nonzero value when any error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphicModes::Init( LPDIRECT3D9 Direct3D, UINT modesCount, D3DFORMAT format,
							void (CALLBACK *pEnumCB) ( LPDISPLAYMODE, D3DDISPLAYMODE * ) )
{
	cModes = modesCount;
	Format = format;
	if ( pEnumCB ) pEnumCBFunc = pEnumCB;
	else pEnumCBFunc = EnumCallBack;
	
	// release possible previous instances of D3D object
	if ( D3DCreated ) SAFE_RELEASE( this->Direct3D )

	if ( Direct3D )
	{
		// create a D3D object
		this->Direct3D = Direct3DCreate9( D3D_SDK_VERSION );
		if ( !(this->Direct3D) ) ERRORMSG( ERRD3DGENERIC, "CGraphicModes::Init()", "Cannot create D3D object." );
		D3DCreated = TRUE;
	}
	else 
	{
		D3DCreated = FALSE;
		this->Direct3D = Direct3D;
	}


	pModes = (LPDISPLAYMODE) malloc( cModes * sizeof(DISPLAYMODE) );
	if ( !pModes )
	{
		ErrorHandler.HandleError( ERROUTOFMEMORY, "CGraphicModes::Init()", "" );
		return ERROUTOFMEMORY;
	}

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// finds all modes and calls a specified callback function for each found
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphicModes::EnumerateModes()
{
	D3DDISPLAYMODE	d3dMode;

	for ( DWORD i = 0; i < cModes; i++ )
	{
		Direct3D->EnumAdapterModes( D3DADAPTER_DEFAULT, Format, i, &d3dMode );
		pEnumCBFunc( &(pModes[i]), &d3dMode );
	}

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the CB function pointer
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphicModes::SetCallBackFunc( void (CALLBACK *pEnumCB) ( LPDISPLAYMODE, D3DDISPLAYMODE * ) )
{
	pEnumCBFunc = pEnumCB;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// standard CB enumeration function
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CGraphicModes::EnumCallBack( LPDISPLAYMODE pMyMode, D3DDISPLAYMODE *d3dMode )
{
	pMyMode->Width = d3dMode->Width;
	pMyMode->Height = d3dMode->Height;
	pMyMode->RefreshRate = d3dMode->RefreshRate;
//TODO:CheckDeviceType()	
	pMyMode->Hardware = true; // this is not correct, this should be verified by CheckDeviceType() call
}

			   
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
