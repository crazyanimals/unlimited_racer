/*
	GraphicModes:	Contains the CGraphicMode class definition that covers all methods needed to enumerate
					all graphic modes available on default graphic adapter. It gets a pointer to callback
					function that can be used to define own functionality or procedure to be called for every
					mode found.
	Creation Date:	7.7.2004
	Last Update:	7.7.2004
	Author:			Roman Margold

*/


#pragma once

#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"


namespace graphic
{

	// one certain mode (without depth specification)
	struct DISPLAYMODE 
	{
		UINT					Width, Height;
		UINT					RefreshRate;
		BOOL					Hardware;		 // hardware support for this mode
	} ;
	typedef DISPLAYMODE			*LPDISPLAYMODE;

	
	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// CGraphicModes class stores all modes of specified format
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CGraphicModes
	{
	public:
		LPDISPLAYMODE			pModes;
		UINT					cModes;
		D3DFORMAT				Format; // modes format
		
		// methods
								CGraphicModes();
								~CGraphicModes();

		HRESULT					Init( LPDIRECT3D9 Direct3D, UINT modesCount, D3DFORMAT format,  void (CALLBACK *pEnumCB) ( LPDISPLAYMODE, D3DDISPLAYMODE * ) = NULL ); 
												/* allocates a memory for all modes; if CB func is not specified 
												 (that is, the default NULL value is used), the  standard CB enum func is used */
		HRESULT					EnumerateModes(); // finds all modes and calls a callback function for each found
		void					SetCallBackFunc( void (CALLBACK *pEnumCB) ( LPDISPLAYMODE, D3DDISPLAYMODE * ) ); // sets the CB function pointer
		static void				CALLBACK EnumCallBack( LPDISPLAYMODE pMyMode, D3DDISPLAYMODE	*d3dMode ); // standard CB enumeration function

	private:

		LPDIRECT3D9				Direct3D; // pointer to Direct3D object
		void					(CALLBACK *pEnumCBFunc) ( LPDISPLAYMODE, D3DDISPLAYMODE * ); // stored pointer to enum callback function
		BOOL					D3DCreated; // contains true if the object was created when the Init method was called
	} ;



}