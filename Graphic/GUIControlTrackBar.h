/*
	GUIControlTrackBar:	Graphic User Interface trackbar class derives functionality directly from GUIControlBase and
						represents a control allowing smooth changing of any variable in predefined <0,1> interval.
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

	class CGUIControlTrackBar : public CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;

	public:
		// constructors & destructor
								CGUIControlTrackBar( float defaultPosition = 0.5f );
								~CGUIControlTrackBar();

		// public methods
		HRESULT					Init( GUICONTROLDECLARATION * decl );
		HRESULT					Render();

		virtual	void			SetValue( float newValue );
		virtual	void			SetValueSilent( float newValue );

		virtual float			GetValue() { return Value.GetFloat(); }; // returns the slider position (interval <0,1>)

		virtual inline void		OnLostDevice() { if (SpriteBackground) SpriteBackground->OnLostDevice(); 
												 CGUIControlBase::OnLostDevice();
												 return; };
		virtual inline void		OnResetDevice() { if (SpriteBackground) SpriteBackground->OnResetDevice(); 
												  CGUIControlBase::OnResetDevice();
												  return; };

	private:
		// used sprites
		LPD3DXSPRITE			SpriteBackground;
		int						iSliderPosMin, iSliderPosMax; // minimal and maximal (relative) X coordinates of the slider on the trackbar

		// methods
		bool					WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

		float					GetSliderValueFromPosition( int x ); // returns the value of the track bar based on the x coordinate of the slider

	} ;


} // end of namespace 