#include "stdafx.h"
#include "GUIControlButton.h"


using namespace graphic;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlButton::CGUIControlButton( CString caption )
{
	// inherited constructor automatically called here
	
	// set other properties default for this control type
	ControlType = GUICT_BUTTON;

	bStopable = true;
	iTabStop = -1;
	bActivated = false; // this button is not "clicked"
	uiTextFormat = GUITEXT_HALIGN_CENTER | GUITEXT_VALIGN_CENTER; // default value for text formating

	Value.SetValue( caption );

	D3DXCreateSprite( pD3DDevice, &SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlButton::~CGUIControlButton()
{
	SAFE_RELEASE( SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders this control
// must be called between BeginScene and EndScene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlButton::Render()
{
	UINT				format = 0;
	DWORD				dwTransparency;
	DWORD				dwShadowColor;
	HRESULT				hr;
	RECT				rect, actRect, rect2;
	LPDIRECT3DTEXTURE9	texture;
	D3DXVECTOR3			position;
	float				scaleX, scaleY;
	D3DXMATRIX			mat;
	GUITEXTUREFILEPOSITION	*	pTSPos;
	D3DCOLOR		*	pColor;	
 	resManNS::__Texture		*	texStruct;


	// don't render if this object is set to be invisible
	if ( !bVisible ) return ERRNOERROR;
	

	// compute control's transparency
	dwTransparency = ( (DWORD) ( 255 * (1 - fTransparency) ) ) << 24;
	dwShadowColor = 3 * ((DWORD) ( 255 * (1 - fTransparency) )) / 4;

	
	// compute actual rectangle
	ComputeAbsoluteLocation( &actRect );
	rect2 = actRect;
	rect2.left += 1;
	rect2.right += 1;
	rect2.bottom += 1;
	rect2.top += 1;


	// obtain texture pointer
	texStruct = pResourceManager->GetTexture( idBackgroundTexture );
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlButton::Render()", "Can't obtain control's texture." )
	texture = texStruct->texture;

	if ( bActivated )
	{
        pTSPos = &TexturePos2;
        pColor = &uiActiveTextColor;
	}
	else
	{
		pTSPos = &TexturePos1;
		pColor = &uiBasicTextColor;
	}


	// compute transofrmaitons and position of the sprite	
	scaleX = (float) iWidth / pTSPos->iWidth;
	scaleY = (float) iHeight / pTSPos->iHeight;

	D3DXMatrixScaling( &mat, scaleX, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );
		
	position.x = (float) actRect.left / scaleX;
	position.y = (float) actRect.top / scaleY;
	position.z = 0.0f;

	rect.left = pTSPos->iPosX;
	rect.top = pTSPos->iPosY;
	rect.right = pTSPos->iPosX + pTSPos->iWidth;
	rect.bottom = pTSPos->iPosY + pTSPos->iHeight;



	// intitialize sprites rendering 
	hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CGUIControlButton::Render()", "Control's surface initialization failed." );
	
	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlButton::Render()", "Unable to draw control object's background." );
	
	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlButton::Render()", "Control's surface presentation failed." );

	

	// draw the button caption
	if ( Value.GetString().GetLength() && pBasicFont )
	{	
		// set text formatting
		if ( uiTextFormat & GUITEXT_HALIGN_LEFT ) format |= DT_LEFT;
		if ( uiTextFormat & GUITEXT_HALIGN_RIGHT ) format |= DT_RIGHT;
		if ( uiTextFormat & GUITEXT_HALIGN_CENTER ) format |= DT_CENTER;
		if ( uiTextFormat & GUITEXT_VALIGN_TOP ) format |= DT_TOP;
		if ( uiTextFormat & GUITEXT_VALIGN_BOTTOM ) format |= DT_BOTTOM;
		if ( uiTextFormat & GUITEXT_VALIGN_CENTER ) format |= DT_VCENTER;
		if ( uiTextFormat & GUITEXT_NOCLIP ) format |= DT_NOCLIP;
		if ( uiTextFormat & GUITEXT_WORDBREAK ) format |= DT_WORDBREAK;

		// draw label text
		pBasicFont->DrawText( NULL, (LPCTSTR) Value.GetString(), -1, &rect2, format, dwShadowColor << 24 );
		pBasicFont->DrawText( NULL, (LPCTSTR) Value.GetString(), -1, &actRect, format, (*pColor) & (dwTransparency | 0x00ffffff) );
	}

	
	// call the binded event
	if ( lpOnRender ) lpOnRender( (CGUIControlBase *) this, 0 );

	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// performs activating action - its called when the button is clicked
// calls the onChange event with the "1" string as a value
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlButton::Activate()
{
	CString		str = "1";

	ActivateSilent();

	if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &str );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// performs activating action - its called when the button is clicked
// !!! this DOES NOT call the onChange event !!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlButton::ActivateSilent()
{
	bActivated = true;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// performs deactivating action - its called when the button is clicked
// calls the onChange event with the "0" string as a value
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlButton::Deactivate()
{
	CString		str = "0";

	DeactivateSilent();

	if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &str );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// performs deactivating action - its called when the button is clicked
// !!! this DOES NOT call the onChange event !!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlButton::DeactivateSilent()
{
	bActivated = false;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// WndProc for button
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIControlButton::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	MESSAGEPARAMSSTRUCT	params;

	if ( !bEnabled ) return false;

	params.wParam = wParam;
	params.lParam = lParam;

	switch ( message ) 
	{
	case WM_LBUTTONDOWN:
		Activate();
		return this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
	case WM_LBUTTONUP:
		Deactivate();
		return this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
	default:
		return this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
	}
	
	
	return false;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
