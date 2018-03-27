/*
	GUIControlCheckBox:	Graphic User Interface checkbox class derives functionality directly from GUIControlBase and
						represents doublestated control that is commonly used for turning some features on and off
						(more globally for representing doublestated variables).
						This control can get focus and therefor is tab-stopable.
	Creation Date:		23.11.2004
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

	class CGUIControlCheckBox : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;

	public:
		// constructors & destructor
								CGUIControlCheckBox( LPCTSTR text );
								~CGUIControlCheckBox();

		// public methods
		HRESULT					Init( GUICONTROLDECLARATION * decl );
		HRESULT					Render();

		virtual	void			SetChecked( bool newState = true );
		virtual	void			SetCheckedSilent( bool newState = true );

		virtual bool			IsChecked() { return (Value.GetString() == "1"); }; // returns true if the value is "1"
		virtual void			SetText( LPCTSTR text ) { Text = text; } // sets the text assigned to checkbox

		virtual inline void		OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice(); 
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual inline void		OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  CGUIControlBase::OnResetDevice();
												  return; };

		// public properties
		UINT					uiTextFormat; // GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER | GUITEXT_NOCLIP as default

	private:
		// used sprites
		LPD3DXSPRITE			SpriteBackground;
		CString					Text; // text placed beside the button

		// methods
		bool					WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	} ;


} // end of namespace 