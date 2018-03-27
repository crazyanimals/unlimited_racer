/*
	GUIControlLabel:Graphic User Interface label class derives functionality directly from GUIControlBase and
					represents uneditable one-lined text control. This control can't get focus and therefor is not
					tab-stopable. This simplest control is commonly used as a label for other more complex controls.
	Creation Date:	17.11.2004
	Last Update:	15.11.2005
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GUIControlBase.h"



namespace graphic
{

	class CGUIControlLabel : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;

	public:
		// constructors & destructor
								CGUIControlLabel( CString caption = "" );
								~CGUIControlLabel();

		// public methods
		HRESULT					Render();

		virtual	void			SetValue( FLOAT val );
		virtual	void			SetValue( int val );
		virtual	void			SetValue( CString val );
		virtual void			SetValueSilent( FLOAT val );
		virtual void			SetValueSilent( int val );
		virtual void			SetValueSilent( CString val );

		virtual CString			GetValue(){ return Value.GetString(); }; // returns the text of the label

		virtual inline void		OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice(); 
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual inline void		OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  CGUIControlBase::OnResetDevice();
												  return; };
		// public properties
		UINT					uiTextFormat; // GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER as default

	private:
		// used sprites
		LPD3DXSPRITE			SpriteBackground;

	} ;


} // end of namespace 