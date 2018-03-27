#include "stdafx.h"
#include "GUIControlLabel.h"


using namespace graphic;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlLabel::CGUIControlLabel( CString caption )
{
	// inherited constructor automatically called here
	
	// set other properties default for this control type
	ControlType = GUICT_LABEL;

	bStopable = false;
	iTabStop = -1;
	uiTextFormat = GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER; // default value for text formating

	Value.SetValue( caption );

	D3DXCreateSprite( pD3DDevice, &SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlLabel::~CGUIControlLabel()
{
	SAFE_RELEASE( SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders this control
// must be called between BeginScene and EndScene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlLabel::Render()
{
	UINT				format = 0;
	DWORD				dwTransparency;
	DWORD				dwShadowColor;
	RECT				rect, rect2;


	// don't render if this object is set to be invisible
	if ( !bVisible ) return ERRNOERROR;
	

	// compute actual rectangle
	ComputeAbsoluteLocation( &rect );
	rect2 = rect;
	rect2.left += 2;
	rect2.right += 2;
	rect2.bottom += 2;
	rect2.top += 2;


	// compute control's transparency
	dwTransparency = ( (DWORD) ( 255 * (1 - fTransparency) ) ) << 24;
	dwShadowColor = 3 * ((DWORD) ( 255 * (1 - fTransparency) )) / 4;

	
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
	if ( pBasicFont ) pBasicFont->DrawText( NULL, (LPCTSTR) Value.GetString(), -1, &rect2, format, dwShadowColor << 24 );
	if ( pBasicFont ) pBasicFont->DrawText( NULL, (LPCTSTR) Value.GetString(), -1, &rect, format, uiBasicTextColor & (dwTransparency | 0x00ffffff));

	
	// call the binded event
	if ( lpOnRender ) lpOnRender( (CGUIControlBase *) this, 0 );

	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the label text to specified float value
// also calls the onChange event
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlLabel::SetValue( FLOAT val )
{ 
	CString		str;

	SetValueSilent( val );
	str = Value.GetString();
	
	if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &str );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the label text to specified integer value
// also calls the onChange event
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlLabel::SetValue( int val )
{ 
	CString		str;

	SetValueSilent( val );
	str = Value.GetString();
	
	if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &str );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the text of the label
// also calls the onChange event
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlLabel::SetValue( CString val )
{ 
	CString		str;

	SetValueSilent( val );
	str = Value.GetString();
	
	if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &str );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the label text to specified float value
// !!! this DOES NOT call the onChange event !!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlLabel::SetValueSilent( FLOAT val )
{ 
	Value.SetValue( val );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the label text to specified integer value
// !!! this DOES NOT call the onChange event !!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlLabel::SetValueSilent( int val )
{ 
	Value.SetValue( val );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the text of the label
// !!! this DOES NOT call the onChange event !!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlLabel::SetValueSilent( CString val )
{ 
	Value.SetValue( val );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
