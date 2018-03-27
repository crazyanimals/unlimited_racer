/*
	GUIControlIconedList:	Graphic User Interface iconed list box class derives functionality directly from GUIControlBase and
						represents a control allowing user to choose between many (possibly huge ammount) options.
						Even if it doesn't derive from GUI list box, it has similar (but wider) functionality.
						It works like standard list, but a single line is not just a text but could be advanced by a bitmap.
						It could be controlled by mouse or keyboard (arrows) and represents standard listing control.
						Can get focus and therefor is tab-stopable.
	Creation Date:		29.12.2004
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


	class CGUIControlIconedList : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;


	public:

		// class defining single listbox line
		class CLine
		{
		public:
			CString					csText;
			GUITEXTUREFILEPOSITION	tpIcon; // position of the icon in texture file
			resManNS::RESOURCEID				idIconTexture; // texture's resource ID	- set to -1 to disable icon for this line
		} ;


		// constructors & destructor
								CGUIControlIconedList();
								~CGUIControlIconedList();

		// public methods
		HRESULT					Init( GUICONTROLDECLARATION * decl );
		HRESULT					Render();

		virtual void			SetLinesCount( UINT linesCount );

		virtual void			SetValue( int ID );
		virtual void			SetValueSilent( int ID );

		virtual HRESULT			GetValue() { return iActiveLine; }; // returns the ID of the actual line

		virtual inline void		OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice(); 
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual inline void		OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  CGUIControlBase::OnResetDevice();
												  return; };

		CString					GetActualString();

		// public properties
		std::vector<CLine>		Lines; // list of lines


	private:
		// used sprites
		LPD3DXSPRITE			SpriteBackground;
		UINT					uiLineHeight; // height of a single line of an unwrapped box 
		UINT					uiLinesCount; // count of lines visible at once - defines listbox height 
		UINT					uiTextFormat; // GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER as default
		int						iActiveLine;
		int						iFirstVisibleLine; // index of a line that is actually the first visible in the list
		UINT					uiIconWidth; // width of the icon area of a single line
		

		// methods
		bool					WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		HRESULT					DrawLineBackground( int index, RECT * pRect, LPDIRECT3DTEXTURE9 tex, DWORD transparency, UINT format ); // draws the background of single line
		void					ChangeFirstVisibleLine( int index ); // sets the actual first line visible


	} ;


} // end of namespace 