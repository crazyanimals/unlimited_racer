/*
	Mouse:			This module is used for managing graphic representation of mouse cursor. Two classes are prepared
					to provide wide range of usable methods.
					CMouseCursor is a class that defines single mouse cursor - it's texture, hotspot and so on.
					CMouse class contains a set of CMouseCursor s and methods to maintain them. In any time
					you can easily swap loaded cursor for another one.
	Creation Date:	27.11.2004
	Last Update:	27.11.2004
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"



namespace graphic
{

	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// class containing single mouse cursor
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CMouseCursor
	{
	public:
								CMouseCursor();
								~CMouseCursor();
		
		virtual HRESULT			Init( LPDIRECT3DDEVICE9	pD3DDev, resManNS::CResourceManager *pResMan, LPCTSTR fileName ); // loads a mouse sprite from a file 
		virtual HRESULT			Draw( int X, int Y ); // Draws a cursor to specified screen coordinates

		inline void 			OnLostDevice() { pDXSprite->OnLostDevice(); return; };
		inline void 			OnResetDevice() { pDXSprite->OnResetDevice(); return; };

	private:
		CString					TextureFileName;
		resManNS::RESOURCEID				idTexture;
		LPD3DXSPRITE			pDXSprite;
		bool					bAnimated; // must be false in this version
		int						iHotSpotX, iHotSpotY; // HotSpot coordinates relative to sprite origin = left-up corner
		
		resManNS::CResourceManager	*	pResourceManager; // pointer to the resource manager object
		LPDIRECT3DDEVICE9		pD3DDevice;
	} ;


	
	typedef std::vector<CMouseCursor *>	MOUSECURSORSLIST;
	typedef MOUSECURSORSLIST::iterator	CURSOR_ITER;



	
	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// mouse object class
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CMouse 
	{
	public:
								CMouse();
								CMouse( resManNS::CResourceManager * pResMan, LPDIRECT3DDEVICE9 pD3DDev ); // initializing constructor
								~CMouse();
		
		HRESULT					Init( resManNS::CResourceManager * pResMan, LPDIRECT3DDEVICE9 pD3DDev ); // initializes this object
		
		inline int				GetPosX() const { return iPosX; }; // returns X position of a mouse
		inline int				GetPosY() const { return iPosY; }; // returns Y position of a mouse
		inline void				SetPosX( int X ) { iPosX = X; }; // sets the X position of a mouse cursor
		inline void				SetPosY( int Y ) { iPosY = Y; }; // sets the Y position of a mouse cursor
		inline void				SetPosXY( int X, int Y ) { iPosX = X; iPosY = Y; }; // sets both, X and Y, positions of a mouse cursor
		
		HRESULT					DrawCursor() const; // draws an actual cursor sprite to actual coordinates

		HRESULT					AddCursor( LPCTSTR fileName, int * index, bool bSetAsActual = true ); // adds a new cursor

		void					SelectCursor( int index ); // selects cursor from a queue to be actual
		void					SetCursor( CMouseCursor * pNewCur ) { pActualCursor = pNewCur; }; // sets the actual cursor without adding it to the list
		inline int				GetCursorsCount() const { return (int) CursorsList.size(); }; // returns count of cursors
		int						GetActualCursorIndex(); // returns actual cursor index

		void					OnLostDevice(); // Must be called when D3D device is lost
		void					OnResetDevice(); // Must be called when D3D device is reset
		
	private:
		int						iPosX, iPosY; // x and y mouse cursor coordinates
		CMouseCursor		*	pActualCursor; // pointer to the actual cursor
		MOUSECURSORSLIST		CursorsList; // list of cursors
		resManNS::CResourceManager	*	pResourceManager; // pointer to the resource manager object
		LPDIRECT3DDEVICE9		pD3DDevice;
	} ;


} // end of namespace