/*
	GUIControlComboBox:	Graphic User Interface combobox class derives functionality directly from GUIControlBase and
						represents a control allowing user to choose between several options. 
						This control can get focus and therefor is tab-stopable.
	Creation Date:		7.12.2004
	Last Update:		23.11.2006
	Author:				Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GUIControlBase.h"
#include "GUIControlPopupMenu.h"



namespace graphic
{



	class CGUIControlComboBox : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;

		// friend functions
		friend	HRESULT CALLBACK GUIComboOnMenuLostFocus( CGUIControlBase * pObject, LPARAM lParam )
				{
					return ((CGUIControlComboBox *) pObject)->OnMenuLostFocus( lParam );
				}
		friend	HRESULT CALLBACK GUIComboOnMenuChangeValue( CGUIControlBase * pObject, LPARAM lParam )
				{		
					return ((CGUIControlComboBox *) pObject)->OnMenuChangeValue( lParam );
				}

	public:
		// constructors & destructor
								CGUIControlComboBox();
								~CGUIControlComboBox();

		// public methods
		HRESULT					Init( GUICONTROLDECLARATION * decl );
		HRESULT					Render();

		virtual void			SetValue( int ID );
		virtual HRESULT			GetValue() { return pPopupMenu ? pPopupMenu->GetValue() : -1; }; // returns the ID of the actual line

		CString					GetActualString();
		void					AddLine( LPCTSTR str );
		void					ClearAllLines();

		virtual inline void		OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice(); 
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual inline void		OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  CGUIControlBase::OnResetDevice();
												  return; };

	private:
		// used sprites
		LPD3DXSPRITE			SpriteBackground;
		UINT					uiLineHeight; // height of a single line of an unwrapped box 
		UINT					uiTextFormat; // GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER as default
		CGUIControlPopupMenu  *	pPopupMenu;
		bool					bActive;
		

		// methods
		bool					WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		HRESULT					OnMenuLostFocus( LPARAM lParam );
		HRESULT					OnMenuChangeValue( LPARAM lParam );

	} ;



	// event handler routines associated to the popup menu
//	HRESULT CALLBACK GUIComboOnMenuLostFocus( CGUIControlBase * pObject, LPARAM lParam );
//	HRESULT CALLBACK GUIComboOnMenuChangeValue( CGUIControlBase * pObject, LPARAM lParam );




} // end of namespace 