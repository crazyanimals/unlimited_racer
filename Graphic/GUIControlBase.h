/*
	GUIControlBase:	Graphic User Interface main class covers the standard common properties derived to its child classes.
					It is focusable, has a tabstop value, name - handler, position data, some user data etc.
	Creation Date:	16.11.2004
	Last Update:	9.12.2006
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#ifdef SCRIPTINTERPRETER_EXPORTS
#undef SCRIPTINTERPRETER_EXPORTS
#endif
#include "..\ScriptInterpreter\variables.h"
#include "GUITimer.h"


// text formating macros
#define GUITEXT_HALIGN_LEFT			0x00001 // aligns horitontally to the left
#define GUITEXT_HALIGN_RIGHT		0x00002
#define GUITEXT_HALIGN_CENTER		0x00004 // centers the text horizontally
#define GUITEXT_VALIGN_TOP			0x00010 // aligns the text vertically to top
#define GUITEXT_VALIGN_BOTTOM		0x00020 // FOR SINGLE LINED ONLY !!!
#define GUITEXT_VALIGN_CENTER		0x00040 // centers the text vertically ; FOR SINGLE LINED TEXT ONLY !!!
#define GUITEXT_NOCLIP				0x00100 // doesn't perform clipping outside of the rectangle - faster !
#define GUITEXT_WORDBREAK			0x00200 // performs line breaking when the text is wider then output rectangle



#define GUIMSG_MOUSECLICK			WM_USER + 163 // message informing about mouse click
#define GUIMSG_RADIOBUTTONCHOOSEN	WM_USER + 164 // message that some radio button was clicked 
#define GUIMSG_COMBOBOXROLLDOWN		WM_USER + 165 // message that some combo box have just been popped up
#define GUIMSG_TIMER				WM_USER + 166 // message activating timer




namespace graphic
{

	// forward declarations
	class CGUIControlBase;
	class CGUISharedDialogData;


	// GUI control object types enumeration object
	enum GUICONTROLTYPE
	{
		GUICT_BASE = 0,
		GUICT_LABEL = 1,
		GUICT_EDIT = 2,
		GUICT_BUTTON = 3,
		GUICT_CHECKBOX = 4,
		GUICT_RADIOBUTTON = 5,
		GUICT_LISTBOX = 6,
		GUICT_COMBOBOX = 7,
		GUICT_TRACKBAR = 8,
		GUICT_POPUPMENU = 9,
		GUICT_ICONEDLIST = 10,
		GUICT_IMAGE = 11
	} ;



	typedef HRESULT CALLBACK	__ONEVENTCALLBACK( CGUIControlBase * pObject, LPARAM lParam );
	typedef __ONEVENTCALLBACK	* ONEVENTCALLBACKPTR;  // pointer to event callback function


	typedef int			GUICONTROLID; // UID of a GUI control object



	// structure defining position of a sprite in a bitmap
	struct GUITEXTUREFILEPOSITION
	{
		int						iPosX;
		int						iPosY;
		int						iWidth;
		int						iHeight;
	} ;


	// structure used for declaring control properties at Init call
	struct GUICONTROLDECLARATION
	{
		CGUISharedDialogData *	pSharedDialogData;

		int						iTabStop; // index that is used to order controls by tab stop (changing focus by tabulator key)
		int						iPosX, iPosY; // x and y coordinates of the top-left corner	in pixels
		int						iWidth, iHeight; // width and height of the control in pixels
		bool					bAbsolutePosition; // if set to true, the position is set absolute on screen; otherwise it is set relatively from the dialog origin
		int						iZOrder; // z-coordinate for visibility and overlaying sorting
		
		float					fTransparency; // transparecny in range <0.0, 1.0>
		resManNS::RESOURCEID				idBackgroundTexture; // ID of the background texture as a handler for ResourceManager
		resManNS::RESOURCEID				ridBasicFont; // font's id if already created
		D3DCOLOR				uiBasicTextColor; // color of standard text
		D3DCOLOR				uiActiveTextColor; // color of highlighted text
		D3DCOLOR				uiColor; // control's color

		char					cToolTip[512]; // null terminated tooltip text - have to be 511 chars long at max

		// control-type dependent values
		int						iSliderPosMin, iSliderPosMax; // minimal and maximal (relative) X coordinates of the slider on the trackbar
		int						iMaxEditTextLen; // maximal length of text in text box
		BOOL					bSelected; // initially "selected" or "checked" flag
		int						iRadioGroupIndex; // index of radio group for radio button
		ONEVENTCALLBACKPTR		lpOnUse; // handle to onMouseClick event for button
		UINT					uiListLineHeight; // height of a single line of list box or combo box
		UINT					uiLinesCount; // count of line of a list box
		UINT					uiListIconWidth; // width of the icon area of a single line of IconedList
		
		// texture positions:
		GUITEXTUREFILEPOSITION	TexturePos1, TexturePos2; // positions of main two textures in bitmap 
														  // first one used for untouched button, unchecked checkbox, unselected radio button, track in trackbar, background for image; background for edit, list box, combo box and popup menu
														  // second one for touched button, checked checkbox, selected radio button, slider in trackbar; active background for list box, combo box and popup menu
		GUITEXTUREFILEPOSITION	TexturePos3; // texture of a combobox's button, "up" button for listbox
		GUITEXTUREFILEPOSITION	TexturePos4; // "down" button for listbox
		GUITEXTUREFILEPOSITION	TopLeftCornerPos, BottomLeftCornerPos, TopRightCornerPos, BottomRightCornerPos; // positions of textures in bitmap - corners for edit, list box, combo box and popup menu
		GUITEXTUREFILEPOSITION	ScrollBarUpPos, ScrollBarDownPos, ScrollBarSliderPos, ScrollBarBackgroundPos; // positions of textures in bitmap - parts of scroll bar; also the ScrollBarDownPos is a setting for combobox rollingup button
		GUITEXTUREFILEPOSITION	HorizontalLinePos, VerticalLinePos; // horizontal and vertical lines as borders for Edit, List box, Combo box and popup menu
	} ;



	// structure contains parameters got by invoked message and sometimes passed to event handler
	struct MESSAGEPARAMSSTRUCT
	{
		WPARAM			wParam;
		LPARAM			lParam;
	} ;

	typedef MESSAGEPARAMSSTRUCT		*	LPMESSAGEPARAMS;



	// CGUISharedDialogData class contains data that are shared among all controls of one particular dialog
	// so it is an internal part of a dialog class, but every dialog child control can read its data
	class CGUISharedDialogData
	{
	friend class CGUIDialog;
	
	private:
		int						iWidth; // parent object width
		int						iHeight; // parent object height
		int						iPosX; // parent object X coordinate 
		int						iPosY; // parent object Y coordinate
		CGUITimer			*	pEventTimer; // shared timer object
		
	public:
		inline int				GetWidth() const { return iWidth; };
		inline int				GetHeight() const { return iHeight; };
		inline int				GetPosX() const { return iPosX; };
		inline int				GetPosY() const { return iPosY; };
		inline int				GetMaxPosX() const { return iPosX + iWidth - 1; };
		inline int				GetMaxPosY() const { return iPosY + iHeight - 1; };
		inline CGUITimer	*	GetTimer() const { return pEventTimer; };

		inline CGUISharedDialogData()
		{
			pEventTimer = 0;
		}

		inline ~CGUISharedDialogData()
		{
			SAFE_DELETE(pEventTimer);
		}
	} ;





	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Base class for all GUI control objects
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CGUIControlBase
	{
		// define CGUIDialog as a friend of all control objects so it can handle their properties
		friend class CGUIDialog;

	public:
		// PUBLIC METHODS

		// constructors & destructor
								CGUIControlBase();
		virtual					~CGUIControlBase();


		// setting shared static properties
		static void				SetD3DDevice( LPDIRECT3DDEVICE9 newDevice ) { pD3DDevice = newDevice; }
		static void				SetHWND( HWND newHWind ) { hWnd = newHWind; }
		static void				SetResourceManager( resManNS::CResourceManager * resMan ) { pResourceManager = resMan; }
	
		
		// basic staff
		virtual	GUICONTROLTYPE	GetControlType() const { return ControlType; }
		virtual HRESULT			Init( GUICONTROLDECLARATION * decl );

		virtual HRESULT			Render() = 0; // renders this control
		virtual void			OnLostDevice() { if (pBasicFont) pBasicFont->OnLostDevice(); return; };
		virtual void			OnResetDevice() { if (pBasicFont) pBasicFont->OnResetDevice(); return; };
		

		
		// property READ methods
		virtual GUICONTROLID	GetUID() const { return UID; }
		virtual CString			GetName() const { return Name; }

		virtual FLOAT			GetValueFloat() { return Value.GetFloat(); }; // returns the value stored in this object as a float
		virtual int				GetValueInt() { return Value.GetInt(); }; // returns the value stored in this object as an integer
		virtual CString			GetValueString() { return Value.GetString(); }; // returns the value stored in this object as a CString

		virtual bool			IsEnabled() const { return bEnabled; }
		virtual bool			IsVisible() const { return bVisible; }
		virtual bool			IsAbsolutelyPositioned() const { return bAbsolutePosition; }
		virtual bool			IsStopable() const { return bStopable; }
		virtual bool			HasFocus() const { return bHasFocus; }
        	
		virtual float			GetTransparency() const { return fTransparency; }; // returns the current control transparency
		virtual float			GetOrigTransparency() const { return fOrigTransparency; }; // returns the original control transparency
		virtual resManNS::RESOURCEID	GetBackgroundTexture() const { return idBackgroundTexture; }; // returns the current background texture resource ID
		virtual resManNS::RESOURCEID	GetBasicFont() const { return ridBasicFont; }; // returns an ID of the current basis font

		virtual int				GetPositionX() const { return iPosX; }
		virtual int				GetPositionY() const { return iPosY; }
		virtual int				GetWidth() const { return iWidth; }
		virtual int				GetHeight() const { return iHeight; }
		virtual int				GetZOrder() const { return iZOrder; }
		virtual int				GetTabStop() const { return iTabStop; }
		virtual D3DCOLOR		GetBasicTextColor() const { return uiBasicTextColor; }
		virtual D3DCOLOR		GetActiveTextColor() const { return uiActiveTextColor; }
		virtual D3DCOLOR		GetColor() const { return uiColor; }
  		virtual CString			GetToolTip() const { return csToolTip; } // returns this object's tooltip text


		
		// property WRITE methods
		virtual void			Enable() { bEnabled = true; }
		virtual void			Disable() { bEnabled = false; }
		virtual void			SetVisible() { bVisible = true; }
		virtual void			SetInvisible() { bVisible = false; }
		virtual void			SetAbsolutePositioning() { bAbsolutePosition = true; };
		virtual void			SetRelativePositioning() { bAbsolutePosition = false; };

		virtual void			SetTransparency( float f );	// sets the transparecny value; value outside of range <0,1> is clamped
		virtual void			ChangeOrigTransparency( float f );	// changes the original transparecny value; value outside of range <0,1> is clamped
		virtual void			SetOrigTransparency(); // reloads the original transparecny value
		virtual void			SetBackgroundTexture( resManNS::RESOURCEID resID );
		virtual HRESULT			LoadBackgroundTexture( LPCTSTR fileName ); // releases actually attached texture and loads new one
		virtual void			SetBasicFont( resManNS::RESOURCEID ridFont );
		virtual HRESULT			LoadBasicFont( LPCTSTR fileName ); // loads font declaration and initializes the font object
		
		virtual void			SetPosition( int x, int y ) { iPosX = x; iPosY = y; };
		virtual void			SetPositionX( int x ) { iPosX = x; };
		virtual void			SetPositionY( int y ) { iPosY = y; };
		virtual void			SetWidth( int width ) { iWidth = width; };
		virtual void			SetHeight( int height ) { iHeight = height; };
		virtual void			SetZOrder( int z ) { iZOrder = z; }; // if using thru CDialog class, RebuildStructures() method of that Dialog must be called!!!
		virtual void			SetTabStop( int index ) { iTabStop = index; }; // if using thru CDialog class, RebuildStructures() method of that Dialog must be called!!!
		virtual void			SetBasicTextColor( D3DCOLOR col ) { uiBasicTextColor = col; }
		virtual void			SetActiveTextColor( D3DCOLOR col ) { uiActiveTextColor = col; }
		virtual void			SetColor( D3DCOLOR col ) { uiColor = col; }
		virtual void			SetToolTip( LPCTSTR str ) { csToolTip = str; } // sets the tooltip for this object



		
		// PUBLIC PROPERTIES

		// event handlers
		ONEVENTCALLBACKPTR		lpOnGetFocus; // is called whenever the control object gets focus; lParam contains UID of the newly focused item
		ONEVENTCALLBACKPTR		lpOnLostFocus; // is called whenever the control object loses focus; lParam contains the UID of the	last focused item
		ONEVENTCALLBACKPTR		lpOnRender;	// is called any time this object is rendered; without parameter
		ONEVENTCALLBACKPTR		lpOnChange;	// is called whenever the control's value is changed; lParam is a pointer to CString contains new value
		ONEVENTCALLBACKPTR		lpOnKeyDown; // all keyboard and mouse messages is passing a pointer to MESSAGEPARAMSSTRUCT structure in lParam parameter
		ONEVENTCALLBACKPTR		lpOnKeyUp;
		ONEVENTCALLBACKPTR		lpOnMouseMove;
		ONEVENTCALLBACKPTR		lpOnMouseDown;
		ONEVENTCALLBACKPTR		lpOnMouseUp;
		ONEVENTCALLBACKPTR		lpOnMouseClick;
		ONEVENTCALLBACKPTR		lpOnMouseWheel;
		ONEVENTCALLBACKPTR		lpOnRMouseDown;


		
	protected:
		// PROTECTED PROPERTIES

		CGUISharedDialogData *	pSharedDialogData;

		// basic control properties
		GUICONTROLTYPE			ControlType; // identifies the control object's type among all basic types defined
		CString					Name; // object name
		GUICONTROLID			UID; // object UID - basic access handler
		CVariable				Value; // this represents any single float/int/string value of this control; can't be accessed directly

		bool					bEnabled; // says whther the control object is enabled (can be used)
		bool					bVisible; // tells whether this object is visible on screen
		int						iTabStop; // index that is used to order controls by tab stop (changing focus by tabulator key)
											// only positive tab stop are used for tabing
		bool					bHasFocus; // specifies whether this object has a focus
		CString					csToolTip; // tooltip text; no tooltip pops up if the string is empty


		// fixed control type based properties
		bool					bStopable; // specifies whether this object is focusable by tabulator
											// this can't be turned on when using unfocusable control, however it can be turned on by specifying iTabStop negative

		// position properties
		int						iPosX, iPosY; // x and y coordinates of the top-left corner	in pixels
		int						iWidth, iHeight; // width and height of the control in pixels
		bool					bAbsolutePosition; // if set to true, the position is set absolute on screen; otherwise it is set relatively from the dialog origin
		int						iZOrder; // z-coordinate for visibility and overlaying sorting

		// appearance properties
		float					fTransparency; // transparecny in range <0.0, 1.0>
		float					fOrigTransparency; // saved original transparency
		resManNS::RESOURCEID	idBackgroundTexture; // ID of the background texture as a handler for ResourceManager
		LPD3DXFONT				pBasicFont; // all objects get the font object from app. they are not creating fonts themself
		resManNS::RESOURCEID	ridBasicFont; // ID of the basic font
		D3DCOLOR				uiBasicTextColor; // color of standard text
		D3DCOLOR				uiActiveTextColor; // color of highlighted text
		D3DCOLOR				uiColor; // control's color
		
		// textures info
		GUITEXTUREFILEPOSITION	TexturePos1, TexturePos2; // positions of main two textures in bitmap 
														  // first one used for untouched button, unchecked checkbox, unselected radio button, track in trackbar
														  // second one for touched button, checked checkbox, selected radio button, slider in trackbar
		GUITEXTUREFILEPOSITION	TexturePos3; // texture of a combobox's button, "up" button for listbox
		GUITEXTUREFILEPOSITION	TexturePos4; // "down" button for listbox
		GUITEXTUREFILEPOSITION	TopLeftCornerPos, BottomLeftCornerPos, TopRightCornerPos, BottomRightCornerPos; // positions of textures in bitmap - corners for edit, list and combo box
		GUITEXTUREFILEPOSITION	ScrollBarUpPos, ScrollBarDownPos, ScrollBarSliderPos, ScrollBarBackgroundPos; // positions of textures in bitmap - parts of scroll bar; also the ScrollBarDownPos is a setting for combobox rollingup button
		GUITEXTUREFILEPOSITION	HorizontalLinePos, VerticalLinePos; // horizontal and vertical lines as borders for Edit, List and Combo box

		// shared properties (among all controls)
		static LPDIRECT3DDEVICE9	pD3DDevice; // pointer to a D3D device object
		static HWND				hWnd; // window handle
		static resManNS::CResourceManager	* pResourceManager;

		
		
		// PROTECTED METHODS
		virtual void			ComputeAbsoluteLocation( RECT * rect ); // computes and returns actual absolute position of a control object in pixels
		virtual bool			WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ); // default WndProc for all control objects

	} ;


} // end of namespace 