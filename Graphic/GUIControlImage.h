/*
	GUIControlImage:	Graphic User Interface image class derives functionality directly from GUIControlBase and
						represents area for custom image loading.
						Can't get focus and is not stopable.
	Creation Date:		16.1.2005
	Last Update:		16.1.2005
	Author:				Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GUIControlBase.h"



namespace graphic
{

	class CGUIControlImage : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;

	public:
		// constructors & destructor
								CGUIControlImage();
								~CGUIControlImage();

		// public methods
		HRESULT					Render();

		virtual HRESULT			LoadImage( LPCTSTR fileName, GUITEXTUREFILEPOSITION *pTexPos );
		virtual HRESULT			SetImage( resManNS::RESOURCEID ridTexture, GUITEXTUREFILEPOSITION *pTexPos );

		virtual inline void		OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice(); 
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual inline void		OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  CGUIControlBase::OnResetDevice();
												  return; };

	private:
		// used sprites
		LPD3DXSPRITE			SpriteBackground;
		resManNS::RESOURCEID				ridImage;

		// methods
		bool					WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	} ;


} // end of namespace 