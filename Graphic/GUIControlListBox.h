/*
	GUIControlListBox:	Graphic User Interface list box class derives functionality directly from GUIControlBase and
						represents a control allowing user to choose between many (possibly huge ammount) options.
						It could be controlled by mouse or keyboard (arrows) and represents standard listing control.
						Can get focus and therefor is tab-stopable.
	Creation Date:		19.12.2004
	Last Update:		7.1.2007
	Author:				Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GUIControlBase.h"



namespace graphic
{


	class CGUIControlListBox : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;


	public:
		
		// defines one item (=line) of the list
		struct SINGLEITEM
		{
			CString			csText; // text to display
			int				iValue; // private integer value for any use
			void		*	pPrivate; // private value for any use
			void		*	pReserved; // not used now

			// constructors
			SINGLEITEM() { 
				csText = ""; iValue = 0; pPrivate = pReserved = NULL;
			};
			SINGLEITEM( CString text ) {
				csText = text; iValue = 0; pPrivate = pReserved = NULL;
			};
			SINGLEITEM( CString text, int value ) {
				csText = text; iValue = value; pPrivate = pReserved = NULL;
			};
			SINGLEITEM( CString text, void * data ) {
				csText = text; iValue = 0; pPrivate = data; pReserved = NULL;
			};
		};


		// constructors & destructor
								CGUIControlListBox();
								~CGUIControlListBox();

		// public methods
		HRESULT					Init( GUICONTROLDECLARATION * decl );
		HRESULT					Render();

		virtual void			SetLinesCount( UINT linesCount );

		virtual void			SetValue( int ID );
		virtual void			SetValueSilent( int ID );

		virtual HRESULT			GetValue() const { return iActiveLine; }; // returns the ID of the actual line
		virtual CString			GetActualString() const;
		virtual CString			GetActualItemText() const;
		virtual SINGLEITEM		GetActualItem() const;
		virtual CString			GetItemText( int index ) const;
		virtual SINGLEITEM		GetItem( int index ) const;
		virtual UINT			GetLinesCount() const { return (UINT) Lines.size(); }; // returns count of items (lines) in the list

		virtual void			PopBack();
		virtual void			PushBack( SINGLEITEM item );
		virtual void			PushBack( CString text );
		virtual void			PushBack( CString text, int value );
		virtual void			PushBack( CString text, void * data );
		virtual void			Clear();

		virtual void			OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice(); 
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual void			OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  CGUIControlBase::OnResetDevice();
												  return; };


	private:
		// private properties
		LPD3DXSPRITE			SpriteBackground;
		UINT					uiLineHeight; // height of a single line of an unwrapped box 
		UINT					uiLinesCount; // count of lines visible at once - defines listbox height 
		UINT					uiTextFormat; // GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER as default

		std::vector<SINGLEITEM>	Lines; // list of lines
		int						iActiveLine;
		int						iFirstVisibleLine; // index of a line that is actually the first visible in the list
		

		// methods
		bool					WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		HRESULT					DrawLineBackground( int index, RECT * pRect, LPDIRECT3DTEXTURE9 tex, DWORD transparency, UINT format ); // draws the background of single line
		void					ChangeFirstVisibleLine( int index ); // sets the actual first line visible


	} ;


} // end of namespace 