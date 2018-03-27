/*
	GUIControlEdit:		Graphic User Interface edit class derives functionality directly from GUIControlBase and
						represents a control allowing user text input.
						This control can get focus and therefor is tab-stopable.
						NOTE: Control edit has predefined text format that can't be changed !!!!!!
	Creation Date:		4.12.2004
	Last Update:		30.10.2005
	Author:				Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GUIControlBase.h"



namespace graphic
{

	class CGUIControlEdit : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;

	public:
		// constructors & destructor
								CGUIControlEdit();
								~CGUIControlEdit();

		// public methods
		HRESULT					Init( GUICONTROLDECLARATION * decl );
		HRESULT					Render();

		virtual	void			SetValue( LPCTSTR str );
		virtual void			SetValueSilent( LPCTSTR str );
		virtual CString			GetValue() { return Value.GetString(); }; // returns input text

		virtual inline void		OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice();
												 if (pLineObject) pLineObject->OnLostDevice();
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual inline void		OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  if (pLineObject) pLineObject->OnResetDevice();
												  CGUIControlBase::OnResetDevice();
												  return; };

		void					SetCursorPosition( int pos );

	private:
		// used sprites
		LPD3DXSPRITE			SpriteBackground;
		LPD3DXLINE				pLineObject;
		int						iMaxTextLength; // maximal lenght of a string that could be inputted
		int						iCursorPosition;
		UINT					uiTextFormat; // GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER as default and can't be changed !!!

		// methods
		bool					WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	} ;


} // end of namespace 