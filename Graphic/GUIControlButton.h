/*
	GUIControlButton:	Graphic User Interface button class derives functionality directly from GUIControlBase and
						represents clickable singlestated control that is commonly used for performing other actions. 
						This control can get focus and therefor is tab-stopable. Also has text on self.
	Creation Date:		22.11.2004
	Last Update:		15.11.2005
	Author:				Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GUIControlBase.h"



namespace graphic
{

	class CGUIControlButton : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;

	public:
		// constructors & destructor
								CGUIControlButton( CString caption = "" );
								~CGUIControlButton();

		// public methods
		HRESULT					Render();

		virtual void			Activate();
		virtual void			ActivateSilent();
		virtual void			Deactivate();
		virtual void			DeactivateSilent();

		virtual	void			SetCaption( CString val ) { Value.SetValue( val ); }; // sets the button's caption
		virtual CString			GetCaption() { return Value.GetString(); }; // returns the text written on the button

		virtual inline void		OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice(); 
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual inline void		OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  CGUIControlBase::OnResetDevice();
												  return; };

		// public properties
		UINT					uiTextFormat; // GUITEXT_HALIGN_CENTER | GUITEXT_VALIGN_CENTER as default

	private:
		// used sprites
		LPD3DXSPRITE			SpriteBackground;
		bool					bActivated;	// if true, button is activated (just render "clicked" version of the button texture)

		// methods
		bool					WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	} ;


} // end of namespace 