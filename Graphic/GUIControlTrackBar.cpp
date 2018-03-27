#include "stdafx.h"
#include "GUIControlTrackBar.h"


using namespace graphic;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlTrackBar::CGUIControlTrackBar( float defaultPosition )
{
	// inherited constructor automatically called here
	
	// set other properties default for this control type
	ControlType = GUICT_TRACKBAR;

	bStopable = true;
	iTabStop = -1;

	iSliderPosMin = 0;
	iSliderPosMax = 0;

	SetValue( defaultPosition );

	D3DXCreateSprite( pD3DDevice, &SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlTrackBar::~CGUIControlTrackBar()
{
	SAFE_RELEASE( SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits this control object by values specified by decl parameter
//
// this must at first call derived Init of CGUIControlBase class
// all objects are disabled by default before calling this, so this method enables the object
// and allows it to get focus and to behave as active component
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlTrackBar::Init( GUICONTROLDECLARATION * decl )
{
	// call derived Init
	this->CGUIControlBase::Init( decl );

	iSliderPosMin = decl->iSliderPosMin;
	iSliderPosMax = decl->iSliderPosMax;

	return ERRNOERROR;	
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders this control
// must be called between BeginScene and EndScene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlTrackBar::Render()
{
	UINT				format = 0;
	DWORD				dwTransparency;
	HRESULT				hr;
	RECT				rect, actRect;
	LPDIRECT3DTEXTURE9	texture;
	D3DXVECTOR3			position;
	float				scaleX, scaleY;
	D3DXMATRIX			mat;
	resManNS::__Texture		*	texStruct;


	// don't render if this object is set to be invisible
	if ( !bVisible ) return ERRNOERROR;
	

	// compute control's transparency
	dwTransparency = ( (DWORD) ( 255 * (1 - fTransparency) ) ) << 24;

	
	// compute actual rectangle
	ComputeAbsoluteLocation( &actRect );


	// obtain texture pointer
	texStruct = pResourceManager->GetTexture( idBackgroundTexture );
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlTrackBar::Render()", "Can't obtain control's texture." )
	texture = texStruct->texture;



	// intitialize sprites rendering 
	hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CGUIControlTrackBar::Render()", "Control's surface initialization failed." );
	

	// compute transofrmaitons of both sprites based on the trackbar's texture size
	scaleX = (float) iWidth / TexturePos1.iWidth;
	scaleY = (float) iHeight / TexturePos1.iHeight;

	D3DXMatrixScaling( &mat, scaleX, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );
	

	// DRAW THE TRACKBAR
	position.x = (float) actRect.left / scaleX;
	position.y = (float) actRect.top / scaleY;
	position.z = 0.0f;

	rect.left = TexturePos1.iPosX;
	rect.top = TexturePos1.iPosY;
	rect.right = TexturePos1.iPosX + TexturePos1.iWidth;
	rect.bottom = TexturePos1.iPosY + TexturePos1.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlTrackBar::Render()", "Unable to draw control object's background." );

	// DRAW THE SLIDEBAR
	position.x += Value.GetFloat() * (float) (iSliderPosMax - iSliderPosMin + 1) + (float) (iSliderPosMin - TexturePos2.iWidth / 2);
	//position.y = (float) actRect.top / scaleY;
	position.z = 0.1f;

	rect.left = TexturePos2.iPosX;
	rect.top = TexturePos2.iPosY;
	rect.right = TexturePos2.iPosX + TexturePos2.iWidth;
	rect.bottom = TexturePos2.iPosY + TexturePos2.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlTrackBar::Render()", "Unable to draw control object's active item." );

	
	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlTrackBar::Render()", "Control's surface presentation failed." );



	// call the binded event
	if ( lpOnRender ) lpOnRender( (CGUIControlBase *) this, 0 );

	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the new value for the track bar, therefore changing the slider position
// calls the onChange event with the paramter set to a pointer to CString containing the float value
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlTrackBar::SetValue( float newValue )
{
	SetValueSilent( newValue );
	
	if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &(Value.GetString()) );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the new value for the track bar, therefore changing the slider position
// !!! this DOES NOT call the onChange event !!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlTrackBar::SetValueSilent( float newValue )
{
	if ( newValue < 0.0f ) newValue = 0.0f;
	if ( newValue > 1.0f ) newValue = 1.0f;

	Value.SetValue( newValue );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// WndProc for track bar
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIControlTrackBar::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	MESSAGEPARAMSSTRUCT	params;
	static bool			bTracking = false;

	if ( !bEnabled ) return false;

	params.wParam = wParam;
	params.lParam = lParam;

	switch ( message ) 
	{
	case WM_LBUTTONDOWN:
		bTracking = true;
		SetValue( GetSliderValueFromPosition( LOWORD( lParam ) ) );
		this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
		return true;
	case WM_LBUTTONUP:
		bTracking = false;
		SetValue( GetSliderValueFromPosition( LOWORD( lParam ) ) );
		this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
		return true;
	case WM_MOUSEMOVE:
		if ( bTracking ) SetValue( GetSliderValueFromPosition( LOWORD( lParam ) ) );
		this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
		return true;
	default:
		return this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
	}
	
	
	return false;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns the value of the track bar based on the x coordinate of the slider
// slider position could be out of range, it is clamped in such case
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CGUIControlTrackBar::GetSliderValueFromPosition( int x )
{
	RECT		rect;

	if ( iSliderPosMax < iSliderPosMin ) return 0;

	ComputeAbsoluteLocation( &rect );

	return (float) ( x - rect.left - iSliderPosMin ) / (float) ( iSliderPosMax - iSliderPosMin + 1 );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////

