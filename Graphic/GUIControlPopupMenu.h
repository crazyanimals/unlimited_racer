/*
	GUIControlPopupMenu:Graphic User Interface popup menu class derives functionality directly from GUIControlBase and
						represents a control allowing user to choose between several options.
						It could be controlled by mouse or keyboard (arrows) and represents standard
						menu control. It is mainly used by combobox. 
						This object is disabled and invisible by default. While enabled, it can get focus 
						and therefor is tab-stopable.
	Creation Date:		19.12.2004
	Last Update:		24.12.2004
	Author:				Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GUIControlBase.h"



namespace graphic
{

	class CGUIControlComboBox;


	class CGUIControlPopupMenu : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;
		// combo box is using the popup menu as a part of self so it should be a friend
		friend class CGUIControlComboBox;


	public:
		// constructors & destructor
								CGUIControlPopupMenu();
								~CGUIControlPopupMenu();

		// public methods
		HRESULT					Init( GUICONTROLDECLARATION * decl );
		HRESULT					Render();

		virtual inline void		SetVisible() { RecomputeMySize(); bVisible = true; }

		virtual void			SetValue( int ID );
		virtual HRESULT			GetValue() { return iActiveLine; }; // returns the ID of the actual line

		virtual inline void		OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice(); 
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual inline void		OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  CGUIControlBase::OnResetDevice();
												  return; };
		CString					GetActualString();

		// public properties
		std::vector<CString>	Lines; // list of lines


	private:
		// used sprites
		LPD3DXSPRITE			SpriteBackground;
		UINT					uiLineHeight; // height of a single line of an unwrapped box 
		UINT					uiTextFormat; // GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER as default
		int						iActiveLine;
		CGUIControlBase		*	pInvokingObject; // pointer to an object that invoked this popup menu

		// methods
		bool					WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		HRESULT					DrawLineBackground( int index, RECT * pRect, LPDIRECT3DTEXTURE9 tex, DWORD transparency, UINT format ); // draws the background of single line
		HRESULT					OnMenuLostFocus( LPARAM lParam );
		HRESULT					OnMenuChangeValue( LPARAM lParam );


	protected:
		virtual void			ComputeAbsoluteLocation( RECT * rect ); // computes and returns actual absolute position of a control object in pixels
		virtual void			RecomputeMySize(); // recomputes the size of popup menu dependingly on lines count

	} ;


} // end of namespace 