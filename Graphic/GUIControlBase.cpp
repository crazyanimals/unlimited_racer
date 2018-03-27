#include "stdafx.h"
#include "GUIControlBase.h"


using namespace graphic;


// static control properties
LPDIRECT3DDEVICE9	CGUIControlBase::pD3DDevice;
HWND				CGUIControlBase::hWnd;
resManNS::CResourceManager *	CGUIControlBase::pResourceManager;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
// note that control object is disabled by default until it is enabled by calling Init method
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlBase::CGUIControlBase()
{
	ControlType = GUICT_BASE;
	UID = 0;
	Name = "";
	pSharedDialogData = NULL;
	csToolTip = "";
	
	// default values for a common control; any derived class can overwrite this
	bHasFocus = false;
	bStopable = true;
	bEnabled = false; // the control object is enabled by calling Init method
	bVisible = true;
	iTabStop = 0;

	iPosX = iPosY = iWidth = iHeight = 0;
	bAbsolutePosition = false;
	iZOrder = 0;
 	uiBasicTextColor = 0xffffffff;   // 0xargb format

	fTransparency = 0.0f;
	idBackgroundTexture = -1;
	ridBasicFont = -1;
	pBasicFont = NULL;

	// unset all event handlers
	lpOnChange = NULL;
	lpOnGetFocus = NULL;
	lpOnKeyDown = NULL;
	lpOnKeyUp = NULL;
	lpOnLostFocus = NULL;
	lpOnMouseClick = NULL;
	lpOnMouseDown = NULL;
	lpOnMouseMove = NULL;
	lpOnMouseUp = NULL;
	lpOnMouseWheel = NULL;
	lpOnRMouseDown = NULL;
	lpOnRender = NULL;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlBase::~CGUIControlBase()
{
	if ( idBackgroundTexture >= 0 ) CGUIControlBase::pResourceManager->ReleaseResource( idBackgroundTexture );
	if ( ridBasicFont >= 0 ) CGUIControlBase::pResourceManager->ReleaseResource( ridBasicFont );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the actual background texture to the one specified by its handler to
// ResourceManager
//
// also performs releasing of the old texture and increasing reference count of the new one 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlBase::SetBackgroundTexture( resManNS::RESOURCEID resID )
{
	// increase the reference count to the new texture
	pResourceManager->DuplicateResource( resID );

	// release the old background texture if one is set
	if ( idBackgroundTexture >= 0 ) pResourceManager->ReleaseResource( idBackgroundTexture );

	// set the new actual background texture
	idBackgroundTexture = resID;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases actually attached texture and loads a new one
//
// returns a nonzero error value when unsuccessful
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlBase::LoadBackgroundTexture( LPCTSTR fileName )
{
	resManNS::RESOURCEID		resID;
	
	resID = idBackgroundTexture; // save the ID

	// load new resource (if it is same, it is duplicated)
	idBackgroundTexture = pResourceManager->LoadResource( fileName, RES_Texture );
	if ( idBackgroundTexture < 0 ) ERRORMSG( ERRGENERIC, "CGUIControlBase::LoadBackgroundTexture()", "Can't obtain a texture for this control." )

	// release the old background texture if one is set
	if ( resID >= 0 ) pResourceManager->ReleaseResource( resID );

	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases actually attached font and sets new one for standard text
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlBase::SetBasicFont( resManNS::RESOURCEID ridFont )
{
	// duplicate the resource
	pResourceManager->DuplicateResource( ridFont );

	// release the old one
	if ( ridBasicFont >= 0 ) pResourceManager->ReleaseResource( ridBasicFont );

	ridBasicFont = ridFont;

	// set font pointer
	resManNS::__FontDef * font = pResourceManager->GetFontDef( ridBasicFont );
	if ( font ) pBasicFont = font->pFont;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads font declaration and initializes the font object
// also releases previously attached font
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlBase::LoadBasicFont( LPCTSTR fileName )
{
	resManNS::RESOURCEID		resID;
	
	resID = ridBasicFont; // save the ID

	// load new resource (if it is same, it is duplicated)
	ridBasicFont = pResourceManager->LoadResource( fileName, RES_FontDef );
	if ( ridBasicFont < 0 ) ERRORMSG( ERRNOTFOUND, "CGUIControlBase::LoadBasicFont()", "Font object could not be created." );

	// release old resource - this way it won't release the resource if it is not necessary
	if ( resID >= 0 ) pResourceManager->ReleaseResource( resID );

	// set font pointer
	resManNS::__FontDef * font = pResourceManager->GetFontDef( ridBasicFont );
	if ( font ) pBasicFont = font->pFont;
	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the transparecny value; value outside of range <0,1> is clamped
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlBase::SetTransparency( float f )
{
	if ( f < 0.0f ) f = 0.0f;
	if ( f > 1.0f ) f = 1.0f;

	fTransparency = f;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// reloads the original transparecny value
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlBase::SetOrigTransparency()
{
	fTransparency = fOrigTransparency;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Changes the original transparecny value; value outside of range <0,1> is clamped.
// If you are not sure of what you are doing, use SetTransparency method instead!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlBase::ChangeOrigTransparency( float f )
{
	if ( f < 0.0f ) f = 0.0f;
	if ( f > 1.0f ) f = 1.0f;

	fOrigTransparency = f;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits this control object by values specified by decl parameter
//
// all objects are disabled by default before calling this, so this method enables the object
// and allows it to get focus and to behave as active component
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlBase::Init( GUICONTROLDECLARATION * decl )
{
	if ( !decl->pSharedDialogData ) ERRORMSG( ERRINVALIDPARAMETER, "CGUIControlBase::Init()", "Valid CGUISharedDialogData object has to be initialized." );
	pSharedDialogData = decl->pSharedDialogData;

	iTabStop = decl->iTabStop;
	iPosX = decl->iPosX;
	iPosY = decl->iPosY;
	iWidth = decl->iWidth;
	iHeight = decl->iHeight;
	bAbsolutePosition = decl->bAbsolutePosition;
	iZOrder = decl->iZOrder;
	fTransparency = decl->fTransparency;
	fOrigTransparency = decl->fTransparency;
	uiBasicTextColor = decl->uiBasicTextColor;
	uiActiveTextColor = decl->uiActiveTextColor;
	uiColor = decl->uiColor;
	csToolTip = decl->cToolTip;

	TexturePos1 = decl->TexturePos1;
	TexturePos2 = decl->TexturePos2;
	TexturePos3 = decl->TexturePos3;
	TexturePos4 = decl->TexturePos4;
	TopLeftCornerPos = decl->TopLeftCornerPos;
	BottomLeftCornerPos = decl->BottomLeftCornerPos;
	TopRightCornerPos = decl->TopRightCornerPos;
	BottomRightCornerPos = decl->BottomRightCornerPos;
	ScrollBarUpPos = decl->ScrollBarUpPos;
	ScrollBarDownPos = decl->ScrollBarDownPos;
	ScrollBarSliderPos = decl->ScrollBarSliderPos;
	ScrollBarBackgroundPos = decl->ScrollBarBackgroundPos;
	HorizontalLinePos = decl->HorizontalLinePos;
	VerticalLinePos = decl->VerticalLinePos;


	SetBackgroundTexture( decl->idBackgroundTexture );
	SetBasicFont( decl->ridBasicFont );

	// enable this object
	bEnabled = true;
		
	return ERRNOERROR;	
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes and returns actual absolute position of a control object in pixels
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlBase::ComputeAbsoluteLocation( RECT * rect )
{
	rect->left = iPosX;
	rect->right = iPosX + iWidth - 1;
	rect->top = iPosY;
	rect->bottom = iPosY + iHeight - 1;

	if ( !bAbsolutePosition )
	{
		rect->left += pSharedDialogData->GetPosX();
		rect->right += pSharedDialogData->GetPosX();
		rect->top += pSharedDialogData->GetPosY();
		rect->bottom += pSharedDialogData->GetPosY();

		if ( rect->right > pSharedDialogData->GetMaxPosX() ) rect->right = pSharedDialogData->GetMaxPosX();
		if ( rect->bottom > pSharedDialogData->GetMaxPosY() ) rect->bottom = pSharedDialogData->GetMaxPosY();
	}
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// default WndProc for all conotrol objects
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIControlBase::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	MESSAGEPARAMSSTRUCT	params;

	if ( !bEnabled ) return false;

	params.wParam = wParam;
	params.lParam = lParam;

	switch ( message ) 
	{
	case WM_KEYDOWN:
		if ( lpOnKeyDown ) lpOnKeyDown( (CGUIControlBase *) this, (LPARAM) &params );
		return true;
	case WM_KEYUP:
		if ( lpOnKeyUp ) lpOnKeyUp( (CGUIControlBase *) this, (LPARAM) &params );
		return true;
	case WM_LBUTTONDOWN:
		if ( lpOnMouseDown ) lpOnMouseDown( (CGUIControlBase *) this, (LPARAM) &params );
		return true;
	case WM_LBUTTONUP:
		if ( lpOnMouseUp ) lpOnMouseUp( (CGUIControlBase *) this, (LPARAM) &params );
		return true;
	case WM_MOUSEMOVE:
		if ( lpOnMouseMove ) lpOnMouseMove( (CGUIControlBase *) this, (LPARAM) &params );
		return true;
	case WM_RBUTTONDOWN:
		if ( lpOnRMouseDown ) lpOnRMouseDown( (CGUIControlBase *) this, (LPARAM) &params );
		return true;
	case WM_MOUSEWHEEL:
		if ( lpOnMouseWheel ) lpOnMouseWheel( (CGUIControlBase *) this, (LPARAM) &params );
		return true;
	case GUIMSG_MOUSECLICK:
		if ( lpOnMouseClick ) lpOnMouseClick( (CGUIControlBase *) this, (LPARAM) &params );
		return true;
	}
	
	
	return false;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
