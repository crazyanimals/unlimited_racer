/*
	GUIDialog:		Graphic User Interface dialog forms the GUI controls into a single object and covers all methods
					for managing groups of controls.
					
		!!!!!	READ THIS BEFORE USING !!!!!
					
					Using is simple: after creating this object, it must be initialized by Init() call performing
					all user settings. Then the dialog is filled by control objects using methods AddXXX (AddLabel, AddButton, etc.).
					When using any object directly, you have to pass object's UID (obtained when adding object to dialog)
					to the GetControl() method. You can work with that object as you wish, BUT you shouldn't change
					TabStop or ZOrder properties. If you do that, you have to call RebuildStructures() method 
					of the Dialog so that all preordered seraching structures can work properly.

					Dialog class also defines four custom messages with numbers: 
						WM_USER + 163 (==GUIMSG_MOUSECLICK)
						WM_USER + 164 (==GUIMSG_RADIOBUTTONCHOOSEN)
						WM_USER + 165 (==GUIMSG_COMBOBOXROLLDOWN)
						WM_USER + 166 (==GUIMSG_TIMER)
					The first one is used only internally, but the second and third are used also externally and
					HAVE TO BE passed to the dialog WndProc to proper processing of radio button groups.
					Message call is using lParam to define UID of the GUIControl that ivokes this message
					(that is the radio button that became selected).
					The last user message should be called here and there (10-100 times per second) for proper
					functionality of tooltips and other time dependent activities
					
					Among standard windows messages, there are some that should also be passed to Dialog's WndProc
					for proper working of all components. They are: 
					WM_KEYDOWN, WM_CHAR, WM_MOUSEMOVE, WM_LBUTTONUP, WM_LBUTTONDOWN, WM_MOUSEWHEEL, WM_RBUTTONDOWN

	Creation Date:	16.11.2004
	Last Update:	11.12.2005
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GUITimer.h"
#include "GUIControlBase.h"
#include "GUIControlLabel.h"
#include "GUIControlButton.h"
#include "GUIControlCheckBox.h"
#include "GUIControlTrackBar.h"
#include "GUIControlRadioButton.h"
#include "GUIControlEdit.h"
#include "GUIControlPopupMenu.h"
#include "GUIControlComboBox.h"
#include "GUIControlListBox.h"
#include "GUIControlIconedList.h"
#include "GUIControlImage.h"



#define GUIDIALOG_BUFFERGRANULARITY		20 // this sets how long is the sorting dynamic array created
											// and also how much it is beeing resized
											// must be native positive number !!!



namespace graphic
{

	typedef std::map<GUICONTROLID, CGUIControlBase *>	CONTROLSTREE;
	typedef CONTROLSTREE::iterator						CONTROLS_ITER;
	typedef std::pair<GUICONTROLID, CGUIControlBase *>	GUICONTROL_PAIR;


	// structure used for declaring dialog properties at Init call
	struct GUIDIALOGDECLARATION
	{
		// mandatory fields, must be properly set when initializing dialog
		LPDIRECT3DDEVICE9		D3DDevice;
		HWND					hWnd;
		resManNS::CResourceManager	*	pResourceManager;
		CTimeCounter		*	pTimer;

		// dialog setting
		int						iPosX, iPosY; // x and y coordinates of the top-left corner	in pixels
		int						iWidth, iHeight; // width and height of the control in pixels
		int						iFilePosX, iFilePosY; // x and y coordinates of the top-left corner	in the source bitmap
		int						iFileWidth, iFileHeight; // width and height of the control in scource bitmap
		float					fTransparency; // dialog backfround transparecny in range <0.0, 1.0>
		float					fToolTipTransparency; // trqansparency of the tooltip box
		float					fControlsTransparency; // control's transparecny in range <0.0, 1.0>
		resManNS::RESOURCEID				idBackgroundTexture; // ID of the background texture as a handler for ResourceManager
		resManNS::RESOURCEID				ridBasicFont; // font's id if already created
		D3DCOLOR				uiBasicTextColor; // color of standard text
		D3DCOLOR				uiActiveTextColor; // color of highlighted text
		D3DCOLOR				uiColor; // control's color
		APPRUNTIME				uiToolTipDelay; // delay in milisecond before tooltip pops up

		int						iSliderPosMin, iSliderPosMax; // minimal and maximal (relative) X coordinates of the slider on the trackbar

		// texture positions info:
		GUITEXTUREFILEPOSITION	tpToolTipTopLeft, tpToolTipTopRight, tpToolTipBottomLeft, tpToolTipBottomRight;
		GUITEXTUREFILEPOSITION	tpToolTipHorizLine, tpToolTipVertLine;
		GUITEXTUREFILEPOSITION	tpToolTipBackground;
		GUITEXTUREFILEPOSITION	tpButtonUp, tpButtonDown;
		GUITEXTUREFILEPOSITION	tpCheckBoxChecked, tpCheckBoxUnchecked;
		GUITEXTUREFILEPOSITION	tpRadioButtonSelected, tpRadioButtonUnselected;
		GUITEXTUREFILEPOSITION	tpTrackBarTrack, tpTrackBarSlider;
		GUITEXTUREFILEPOSITION	tpEditTopLeft, tpEditTopRight, tpEditBottomLeft, tpEditBottomRight;
		GUITEXTUREFILEPOSITION	tpEditHorizLine, tpEditVertLine;
		GUITEXTUREFILEPOSITION	tpEditBackground;
		GUITEXTUREFILEPOSITION	tpListBoxTopLeft, tpListBoxTopRight, tpListBoxBottomLeft, tpListBoxBottomRight;
		GUITEXTUREFILEPOSITION	tpListBoxHorizLine, tpListBoxVertLine;
		GUITEXTUREFILEPOSITION	tpListBoxBackground, tpListBoxActiveBackground;
		GUITEXTUREFILEPOSITION	tpListBoxButtonUp, tpListBoxButtonDown;
		GUITEXTUREFILEPOSITION	tpComboTopLeft, tpComboTopRight, tpComboBottomLeft, tpComboBottomRight;
		GUITEXTUREFILEPOSITION	tpComboHorizLine, tpComboVertLine;
		GUITEXTUREFILEPOSITION	tpComboButton;
		GUITEXTUREFILEPOSITION	tpComboBackground, tpComboActiveBackground;
		GUITEXTUREFILEPOSITION	tpPopupTopLeft, tpPopupTopRight, tpPopupBottomLeft, tpPopupBottomRight;
		GUITEXTUREFILEPOSITION	tpPopupHorizLine, tpPopupVertLine;
		GUITEXTUREFILEPOSITION	tpPopupBackground, tpPopupActiveBackground;
		GUITEXTUREFILEPOSITION	tpScrollBarUp, tpScrollBarDown, tpScrollBarSlider, tpScrollBarBackground;
		GUITEXTUREFILEPOSITION	tpImageBackground;
	} ;
	
	typedef GUIDIALOGDECLARATION	*	LPGUIDIALOGDECLARATION;


	
	// helpful forward declaration
	class CGUIDialog;
	
	// structure used when declaring dialog thru definition file (InitFromFile() call)
	struct GUIDLGFILEDEFINITION
	{
		GUIDIALOGDECLARATION	dlgDecl;
		GUIDIALOGDECLARATION	dlgDeclTemp;
		GUICONTROLDECLARATION	ctrlDecl;
		GUITEXTUREFILEPOSITION *pPosDecl;
		CGUIDialog			*	pDialog;
		bool					bInDlg;
		bool					bInControl;
		bool					bInPosDecl;
		bool					bDialogInitialized;
		bool					bEnabled, bVisible;
		int						iDeepness;
		UINT					uiTextFormat;
		CVariable				cvValue;
		CString					str, csCtrlName, csCtrlCaption;
		CString					csFont, csTex, csDecl;
		UINT					uiControlType;
	} ;




	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// CGUIDialog - main GUI class coveringa and controling all control staff
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CGUIDialog
	{
	public:
		// STRUCTURES

		// structure used for ordered buffers
		struct SORTINGSTRUCTURE
		{
			int					iSortingKey;
			GUICONTROLID		UID;
			CGUIControlBase	*	pControl; // direct pointer to the object
		} ;

		typedef SORTINGSTRUCTURE *	LPSORTINGSTRUCTURE;

		
		// enumeration type used to define sorting options for EnumerateControls() method
		// use these as flags !
		enum SORTINGOPTIONS 
		{
			SO_Unsorted		= 0x0000, // unsorted
			SO_Sorted		= 0x0001, // sorted in any way
			SO_ZSort		= 0x0002, // ZSorted - if not set, then TabStop sorting used
			SO_Ascending	= 0x0004  // if set, ascending ordering used, elsewhere descending ordering used
		} ;



		// PUBLIC METHODS

		static HRESULT			LoadTextureDeclaration( LPCTSTR fileName, GUIDIALOGDECLARATION * decl ); // loads dialog texture declaration from a file


		// constructors & destructors								
								CGUIDialog();
								~CGUIDialog();

		// setting standard shared control properties
		inline void				SetD3DDevice( LPDIRECT3DDEVICE9 newDevice ) { CGUIControlBase::pD3DDevice = newDevice; }
		inline void				SetHWND( HWND newHWind ) { CGUIControlBase::hWnd = newHWind; }
		inline void				SetResourceManager( resManNS::CResourceManager * resMan ) { CGUIControlBase::pResourceManager = resMan; }

		// basic functionality
		virtual HRESULT			Init( GUIDIALOGDECLARATION * decl ); // inits Dialog object
		virtual HRESULT			InitFromFile(	LPCTSTR				fileName, 
												HWND				hWnd, 
												LPDIRECT3DDEVICE9	D3DDevice,
												resManNS::CResourceManager *	pResourceManager,
												CTimeCounter	 *	pTimer ); // inits the Dialog by configuration set in file

		virtual void			Reset();
		void					OnLostDevice();
		void					OnResetDevice();

		virtual bool			WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ); // default WndProc for dialog

		CGUIControlBase		*	GetControl( GUICONTROLID UID );
		CGUIControlBase		*	GetControl( LPCTSTR csCtrlName );
		void					EnumerateControls(	void (CALLBACK *CallBack)(CGUIControlBase * pControl, LPVOID pReserved),
													DWORD dwOptions = 0, LPVOID pReserved = NULL );
		CGUIControlRadioButton *GetSelectedRadioButton( int radioGroupIndex ); // returns the pointer to selected radio button
		virtual void			RebuildStructures(); // must be called after changing ZOrder or TabStop values of any control object
		virtual HRESULT			Render(); // renders the dialog and all of its control objects
		void					SetFocusTo( GUICONTROLID UID ); // sets the focus to specified control object
		
		HRESULT					AddLabel( int x, int y, int width, int height, LPCTSTR caption, GUICONTROLID * newID ); // adds a label to the dialog with standard dialog setting
		HRESULT					AddLabelEx( GUICONTROLDECLARATION * decl, LPCTSTR caption, GUICONTROLID * newID ); // adds a label to the dialog with full customized setting
		HRESULT					AddButton( int x, int y, int width, int height, LPCTSTR caption, ONEVENTCALLBACKPTR lpOnUse, GUICONTROLID * newID ); // adds a button to the dialog with standard dialog setting
		HRESULT					AddButtonEx( GUICONTROLDECLARATION * decl, LPCTSTR caption, GUICONTROLID * newID ); // adds a button to the dialog with full customized setting
		HRESULT					AddCheckBox( int x, int y, int width, int height, LPCTSTR text, bool checked, GUICONTROLID * newID ); // adds a checkbox to the dialog with standard dialog setting
		HRESULT					AddCheckBoxEx( GUICONTROLDECLARATION * decl, LPCTSTR text, GUICONTROLID * newID ); // adds a checkbox to the dialog with full customized setting
		HRESULT					AddTrackBar( int x, int y, int width, int height, float value, GUICONTROLID * newID ); // adds a trackbar to the dialog with standard dialog setting
		HRESULT					AddTrackBarEx( GUICONTROLDECLARATION * decl, float value, GUICONTROLID * newID ); // adds a trackbar to the dialog with full customized setting
		HRESULT					AddRadioButton( int x, int y, int width, int height, LPCTSTR text, bool selected, int groupIndex, GUICONTROLID * newID ); // adds a radio button to the dialog with standard dialog setting
		HRESULT					AddRadioButtonEx( GUICONTROLDECLARATION * decl, LPCTSTR text, GUICONTROLID * newID ); // adds a radio button to the dialog with full customized setting
		HRESULT					AddEditBox( int x, int y, int width, int height, int maxTextLen, GUICONTROLID * newID ); // adds an edit box to the dialog with standard dialog setting
		HRESULT					AddEditBoxEx( GUICONTROLDECLARATION * decl, GUICONTROLID * newID ); // adds an edit box to the dialog with full customized setting
		HRESULT					AddComboBox( int x, int y, int width, int height, int lineHeight, GUICONTROLID * newID ); // adds a combobox to the dialog with standard dialog setting
		HRESULT					AddComboBoxEx( GUICONTROLDECLARATION * decl, GUICONTROLID * newID ); // adds a combobox to the dialog with full customized setting
		HRESULT					AddPopupMenu( int x, int y, int width, int lineHeight, GUICONTROLID * newID ); // adds a popup menu to the dialog with standard dialog setting
		HRESULT					AddPopupMenuEx( GUICONTROLDECLARATION * decl, GUICONTROLID * newID ); // adds a popup menu to the dialog with full customized setting
		HRESULT					AddListBox( int x, int y, int width, int linesCount, int lineHeight, GUICONTROLID * newID ); // adds a listbox to the dialog with standard dialog setting
		HRESULT					AddListBoxEx( GUICONTROLDECLARATION * decl, GUICONTROLID * newID ); // adds a listbox to the dialog with full customized setting
		HRESULT					AddIconedList( int x, int y, int width, int iconWidth, int linesCount, int lineHeight, GUICONTROLID * newID ); // adds an iconed listbox to the dialog with standard dialog setting
		HRESULT					AddIconedListEx( GUICONTROLDECLARATION * decl, GUICONTROLID * newID ); // adds an iconed listbox to the dialog with full customized setting
		HRESULT					AddImage( int x, int y, int width, int height, LPCTSTR fileName, GUICONTROLID * newID ); // adds an image to the dialog with standard dialog setting
		HRESULT					AddImageEx( GUICONTROLDECLARATION * decl, LPCTSTR fileName, GUICONTROLID * newID ); // adds an image to the dialog with full customized setting

		
		// property READ methods
		inline CGUISharedDialogData *	GetSharedDialogData() { return &pSharedDialogData; }; // returns a pointer to data shared by all controls this dialog holds
		virtual inline float			GetTransparency() const { return fTransparency; }; // returns the current dialog transparency
		virtual inline float			GetToolTipTransparency() const { return fToolTipTransparency; }; // returns the current transparency set for tooltips
		virtual inline float			GetControlsTransparency() const { return fControlsTransparency; }; // returns the current control's transparency
		virtual inline resManNS::RESOURCEID		GetBackgroundTexture() const { return idBackgroundTexture; }; // returns the current background texture resource ID
		virtual inline resManNS::RESOURCEID		GetBasicFont() const { return ridBasicFont; }; // returns an ID of the current basis font
		virtual inline D3DCOLOR			GetBasicTextColor() const { return uiBasicTextColor; }
		virtual inline D3DCOLOR			GetActiveTextColor() const { return uiActiveTextColor; }
		virtual inline D3DCOLOR			GetColor() const { return uiColor; }

		virtual inline int				GetPositionX() const { return pSharedDialogData.GetPosX(); }
		virtual inline int				GetPositionY() const { return pSharedDialogData.GetPosY(); }
		virtual inline int				GetWidth() const { return pSharedDialogData.GetWidth(); }
		virtual inline int				GetHeight() const { return pSharedDialogData.GetHeight(); }
		virtual inline CGUITimer	*	GetEventTimer() const { return pSharedDialogData.GetTimer(); }
		virtual inline APPRUNTIME		GetToolTipDelay() const { return uiToolTipDelay; }

        		
		// property WRITE methods
		virtual void			SetTransparency( float f );	// sets the transparecny value; value outside of range <0,1> is clamped
		virtual void			SetToolTipTransparency( float f ); // sets the transparecny value for tooltips; value outside of range <0,1> is clamped
		virtual void			SetControlsTransparency( float f );	// sets the transparecny for controls; value outside of range <0,1> is clamped
		virtual void			SetBackgroundTexture( resManNS::RESOURCEID resID );
		virtual HRESULT			LoadBackgroundTexture( LPCTSTR fileName ); // releases actually attached texture and loads new one
		virtual void			SetBasicFont( resManNS::RESOURCEID ridFont );
		virtual HRESULT			LoadBasicFont( LPCTSTR fileName ); // loads font declaration and initializes the font object
		
		virtual inline void		SetPosition( int x, int y ) { pSharedDialogData.iPosX = x; pSharedDialogData.iPosY = y; };
		virtual inline void		SetPositionX( int x ) { pSharedDialogData.iPosX = x; };
		virtual inline void		SetPositionY( int y ) { pSharedDialogData.iPosY = y; };
		virtual inline void		SetWidth( int width ) { pSharedDialogData.iWidth = width; };
		virtual inline void		SetHeight( int height ) { pSharedDialogData.iHeight = height; };
		virtual inline void		SetBasicTextColor( D3DCOLOR col ) { uiBasicTextColor = col; }
		virtual inline void		SetActiveTextColor( D3DCOLOR col ) { uiActiveTextColor = col; }
		virtual inline void		SetColor( D3DCOLOR col ) { uiColor = col; }
		virtual inline void		SetToolTipDelay( APPRUNTIME delay ) { uiToolTipDelay = delay; }

		inline void				SetSliderPosMin( int i ) { iSliderPosMin = i; };
		inline void				SetSliderPosMax( int i ) { iSliderPosMax = i; };

		
		// PUBLIC PROPERTIES
		ONEVENTCALLBACKPTR		lpOnRButtonDown; // handler to a routine that should be called when right mouse button is pressed in the dialog


	private:
		// PRIVATE PROPERTIES

		// basic structures
		CONTROLSTREE			Controls; // binary searching tree of all controls
		int						iBufferLenght; // actual size of the sorting buffers (ZOrder and TabStops)
		LPSORTINGSTRUCTURE		pZOrderList; // dynamic array used for sorting controls by their ZOrder
 		LPSORTINGSTRUCTURE		pTabStopList; // dynamic array used for sorting controls by their TabStop index

		// some important data are stored in this class !!!! this prevents from data redundancy and problems with synchronization
		CGUISharedDialogData 	pSharedDialogData; // contains data that are shared among all controls this dialog holds
													

		// flow & optimization properties
		int						iLastUID; // controls which UID to assign to the next object that will be created
		int						iMaxTabStop; // actual highest tabstop index
		int						iMaxZOrder; // actual highest ZOrder index
		
		GUICONTROLID			idFocusedObject; // UID of the object that has focus
		GUICONTROLID			idMouseCapturingObject; // UID of the object that captured the mouse
		
		APPRUNTIME				uiLastMouseMoveTime; // time of the last mouse movement
		int						iLastMouseDownX, iLastMouseDownY; // coordinates of the last point where the LBUTTONDOWN message was invoked
		int						iLastMouseX, iLastMouseY; // last known coordinates of mouse cursor
		

		// appearance settings
		float					fTransparency; // dialog backfround transparecny in range <0.0, 1.0>
		float					fToolTipTransparency; // trqansparency of the tooltip box
		float					fControlsTransparency; // control's transparecny in range <0.0, 1.0>
		resManNS::RESOURCEID				idBackgroundTexture; // ID of the background texture as a handler for ResourceManager
		LPD3DXFONT				pBasicFont; // all objects get the font object from app. they are not creating fonts themself
		resManNS::RESOURCEID				ridBasicFont; // ID of the basic font
		D3DCOLOR				uiBasicTextColor; // color of standard text
		D3DCOLOR				uiActiveTextColor; // color of highlighted text
		D3DCOLOR				uiColor; // control's color
		LPD3DXSPRITE			SpriteBackground;
		int						iSliderPosMin, iSliderPosMax; // minimal and maximal (relative) X coordinates of the slider on the trackbar


		// tooltip properties
		GUICONTROLID			idToolTipObject; // UID of the object whose tooltip should be drawn in Render call
		APPRUNTIME				uiToolTipDelay; // delay in milisecond before tooltip pops up
		APPRUNTIME				uiToolTipDuration; // count of miliseconds before tooltip vanishes
		APPRUNTIME				uiToolTipStartTime; // time when tooltip showed up
		int						iToolTipPosX, iToolTipPosY; // position of the tooltip on the screen
		int						iToolTipRelX, iToolTipRelY; // relative position of the tooltip to the mouse cursor
		BOOL					bToolTipShowed; // is set to true when the tooltip is actually visible
		BOOL					bToolTipMouseMoved; // is set to true if the mouse cursor moved to another object
		


		// texture mapping properties
		int						iFilePosX, iFilePosY; // x and y coordinates of the top-left corner	in the source bitmap
		int						iFileWidth, iFileHeight; // width and height of the control in scource bitmap

		// texture positions info:
		GUITEXTUREFILEPOSITION	tpToolTipTopLeft, tpToolTipTopRight, tpToolTipBottomLeft, tpToolTipBottomRight;
		GUITEXTUREFILEPOSITION	tpToolTipHorizLine, tpToolTipVertLine;
		GUITEXTUREFILEPOSITION	tpToolTipBackground;
		GUITEXTUREFILEPOSITION	tpButtonUp, tpButtonDown;
		GUITEXTUREFILEPOSITION	tpCheckBoxChecked, tpCheckBoxUnchecked;
		GUITEXTUREFILEPOSITION	tpRadioButtonSelected, tpRadioButtonUnselected;
		GUITEXTUREFILEPOSITION	tpTrackBarTrack, tpTrackBarSlider;
		GUITEXTUREFILEPOSITION	tpEditTopLeft, tpEditTopRight, tpEditBottomLeft, tpEditBottomRight;
		GUITEXTUREFILEPOSITION	tpEditHorizLine, tpEditVertLine;
		GUITEXTUREFILEPOSITION	tpEditBackground;
		GUITEXTUREFILEPOSITION	tpListBoxTopLeft, tpListBoxTopRight, tpListBoxBottomLeft, tpListBoxBottomRight;
		GUITEXTUREFILEPOSITION	tpListBoxHorizLine, tpListBoxVertLine;
		GUITEXTUREFILEPOSITION	tpListBoxBackground, tpListBoxActiveBackground;
		GUITEXTUREFILEPOSITION	tpListBoxButtonUp, tpListBoxButtonDown;
		GUITEXTUREFILEPOSITION	tpComboTopLeft, tpComboTopRight, tpComboBottomLeft, tpComboBottomRight;
		GUITEXTUREFILEPOSITION	tpComboHorizLine, tpComboVertLine;
		GUITEXTUREFILEPOSITION	tpComboButton;
		GUITEXTUREFILEPOSITION	tpComboBackground, tpComboActiveBackground;
		GUITEXTUREFILEPOSITION	tpPopupTopLeft, tpPopupTopRight, tpPopupBottomLeft, tpPopupBottomRight;
		GUITEXTUREFILEPOSITION	tpPopupHorizLine, tpPopupVertLine;
		GUITEXTUREFILEPOSITION	tpPopupBackground, tpPopupActiveBackground;
		GUITEXTUREFILEPOSITION	tpScrollBarUp, tpScrollBarDown, tpScrollBarSlider, tpScrollBarBackground;
		GUITEXTUREFILEPOSITION	tpImageBackground;



		// PRIVATE METHODS
		HRESULT					AddItemToOrderedLists( CONTROLS_ITER iter );

		virtual bool			HandleKeyDownMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		virtual bool			HandleKeyUpMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		virtual bool			HandleCharMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		virtual bool			HandleLButtonDownMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		virtual bool			HandleLButtonUpMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		virtual bool			HandleMouseMoveMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		virtual bool			HandleMouseWheelMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		virtual bool			HandleRButtonDownMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		virtual bool			HandleRadioButtonSelectedMsg( HWND, UINT, WPARAM, LPARAM );
		virtual bool			HandleComboBoxRollDown( HWND, UINT, WPARAM, LPARAM );
		virtual bool			HandleTimerMsg( HWND, UINT, WPARAM, LPARAM );

		CGUIControlBase *		GetControlFromPoint( int x, int y );

		virtual HRESULT			RenderToolTip( CGUIControlBase * pObject, int x, int y );

		void static CALLBACK	LoadTextureDeclCallBack( LPNAMEVALUEPAIR pPair );
		void static CALLBACK	LoadDeclarationCallBack( LPNAMEVALUEPAIR pPair );
 		static HRESULT			LoadDeclarationDialog( LPNAMEVALUEPAIR pPair );
		static HRESULT			LoadDeclarationControl( LPNAMEVALUEPAIR pPair );

	} ;



} // end of namespace graphic
