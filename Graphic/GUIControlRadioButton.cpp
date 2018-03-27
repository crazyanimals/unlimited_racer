#include "stdafx.h"
#include "GUIControlRadioButton.h"


using namespace graphic;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlRadioButton::CGUIControlRadioButton( LPCTSTR text )
{
	// inherited constructor automatically called here
	
	// set other properties default for this control type
	ControlType = GUICT_RADIOBUTTON;

	bStopable = true;
	iTabStop = -1;
	uiTextFormat = GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER | GUITEXT_NOCLIP; // default value for text formating
	iRadioGroup = -1;

	Text = text;

	Value.SetValue( "0" );

	D3DXCreateSprite( pD3DDevice, &SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlRadioButton::~CGUIControlRadioButton()
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
HRESULT CGUIControlRadioButton::Init( GUICONTROLDECLARATION * decl )
{
	// call derived Init
	this->CGUIControlBase::Init( decl );

	iRadioGroup = decl->iRadioGroupIndex;
	if ( decl->bSelected ) Value.SetValue( "1" );
	else Value.SetValue ( "0" );

	return ERRNOERROR;	
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders this control
// must be called between BeginScene and EndScene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlRadioButton::Render()
{
	UINT				format = 0;
	DWORD				dwTransparency;
	HRESULT				hr;
	RECT				rect, actRect;
	LPDIRECT3DTEXTURE9	texture;
	D3DXVECTOR3			position;
	float				scaleX, scaleY;
	D3DXMATRIX			mat;
	GUITEXTUREFILEPOSITION	*	pTSPos;
	resManNS::__Texture		*	texStruct;



	// don't render if this object is set to be invisible
	if ( !bVisible ) return ERRNOERROR;
	

	// compute control's transparency
	dwTransparency = ( (DWORD) ( 255 * (1 - fTransparency) ) ) << 24;

	
	// compute actual rectangle
	ComputeAbsoluteLocation( &actRect );


	// obtain texture pointer
	texStruct = pResourceManager->GetTexture( idBackgroundTexture );
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlRadioButton::Render()", "Can't obtain control's texture." )
	texture = texStruct->texture;


	pTSPos = Value.GetString() == "1" ? &TexturePos2 : &TexturePos1;


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


	// DRAW THE RADIO BUTTON

	// intitialize sprites rendering 
	hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CGUIControlRadioButton::Render()", "Control's surface initialization failed." );
	
	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlRadioButton::Render()", "Unable to draw control object's background." );
	
	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlRadioButton::Render()", "Control's surface presentation failed." );

	
	
	// draw the assigned text
	if ( Text.GetLength() && pBasicFont )
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

		// place the text beside the checkbox
		actRect.left += 5 + pTSPos->iWidth;
		actRect.right += 5 + pTSPos->iWidth + 200;

		// draw label text
		if ( pBasicFont ) pBasicFont->DrawText( NULL, (LPCTSTR) Text, -1, &actRect, format, uiBasicTextColor & (dwTransparency | 0x00ffffff) );
	}

	

	// call the binded event
	if ( lpOnRender ) lpOnRender( (CGUIControlBase *) this, 0 );

	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// selects or unselects the radio button - setting the Value property
// calls the onChange event with the Value's content as the parameter
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlRadioButton::SetSelected( bool newState )
{
	CString		str;

	SetSelectedSilent( newState );
	
	str = Value.GetString();

    if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &str );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// selects or unselects the radio button - setting the Value property, but DOES NOT call the 
//   onChange event !
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlRadioButton::SetSelectedSilent( bool newState )
{
	if ( newState ) Value.SetValue( "1" ); else Value.SetValue( "0" );
	
	PostMessage( hWnd, GUIMSG_RADIOBUTTONCHOOSEN, 0, UID );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// WndProc for check box
// checks the box on MouseClick message
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIControlRadioButton::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	MESSAGEPARAMSSTRUCT	params;

	if ( !bEnabled ) return false;

	params.wParam = wParam;
	params.lParam = lParam;

	switch ( message ) 
	{
	case GUIMSG_MOUSECLICK:
		if ( Value.GetString() != "1" ) SetSelected();
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
