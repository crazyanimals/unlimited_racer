#include "stdafx.h"
#include "GUIControlComboBox.h"


using namespace graphic;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlComboBox::CGUIControlComboBox()
{
	// inherited constructor automatically called here
	
	// set other properties default for this control type
	ControlType = GUICT_COMBOBOX;

	bStopable = true;
	iTabStop = -1;
	uiTextFormat = GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER; // default value for text formating

	uiLineHeight = 20;

	Value.SetValue( -1 );
	pPopupMenu = NULL;

	D3DXCreateSprite( pD3DDevice, &SpriteBackground );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlComboBox::~CGUIControlComboBox()
{
	SAFE_RELEASE( SpriteBackground );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits this control object by values specified by decl parameter
//
// this must at first call derived Init of CGUIControlBase class
// all objects are disabled by default before calling this, so this method enables the object
// and allows it to get focus and to behave as active component
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlComboBox::Init( GUICONTROLDECLARATION * decl )
{
	// call derived Init
	this->CGUIControlBase::Init( decl );

	uiLineHeight = decl->uiListLineHeight;

	bActive = false;

	return ERRNOERROR;	
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders this control
// must be called between BeginScene and EndScene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlComboBox::Render()
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
	static bool			bLastHasFocus = false;
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
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlComboBox::Render()", "Can't obtain control's texture." )
	texture = texStruct->texture;


	// set text formatting
	if ( uiTextFormat & GUITEXT_HALIGN_LEFT ) format |= DT_LEFT;
	if ( uiTextFormat & GUITEXT_HALIGN_RIGHT ) format |= DT_RIGHT;
	if ( uiTextFormat & GUITEXT_HALIGN_CENTER ) format |= DT_CENTER;
	if ( uiTextFormat & GUITEXT_VALIGN_TOP ) format |= DT_TOP;
	if ( uiTextFormat & GUITEXT_VALIGN_BOTTOM ) format |= DT_BOTTOM;
	if ( uiTextFormat & GUITEXT_VALIGN_CENTER ) format |= DT_VCENTER;
	if ( uiTextFormat & GUITEXT_NOCLIP ) format |= DT_NOCLIP;
	if ( uiTextFormat & GUITEXT_WORDBREAK ) format |= DT_WORDBREAK;



	// intitialize sprites rendering 
	hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CGUIControlComboBox::Render()", "Control's surface initialization failed." );
	


	// DRAW THE COMBOBOX BUTTON
	scaleX = (float) iWidth / TexturePos3.iWidth;
	scaleY = (float) iHeight / TexturePos3.iHeight;

	D3DXMatrixScaling( &mat, scaleX, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) actRect.left / scaleX;
	position.y = (float) actRect.top / scaleY;
	position.z = 0.0f;

	rect.left = TexturePos3.iPosX;
	rect.top = TexturePos3.iPosY;
	rect.right = rect.left + TexturePos3.iWidth;
	rect.bottom = rect.top + TexturePos3.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlComboBox::Render()", "Unable to draw control object's background1." );

	D3DXMatrixIdentity( &mat );
	SpriteBackground->SetTransform( &mat );



	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlComboBox::Render()", "Control's surface presentation failed." );



	
	// DRAW THE TEXT
	if ( GetActualString().GetLength() && pBasicFont )
	{
		actRect.left += 4; // these 4 lines should not be here, but most combobox buttons looks ugly without it
		rect2.left += 4;
		actRect.right -= 4;
		rect2.right -= 4;
		pBasicFont->DrawText( NULL, (LPCTSTR) GetActualString(), -1, &rect2, format, dwShadowColor << 24 );
		pBasicFont->DrawText( NULL, (LPCTSTR) GetActualString(), -1, &actRect, format, uiBasicTextColor & (dwTransparency | 0x00ffffff) );
	}

	

	// call the binded event
	if ( lpOnRender ) lpOnRender( (CGUIControlBase *) this, 0 );

	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the ID of an item (line) to be actually selected
// sets -1 if there is no line of such ID
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlComboBox::SetValue( int ID )
{
	if ( pPopupMenu ) pPopupMenu->SetValue( ID );
	if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &(Value.GetString()) );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a single line to the lines list of the associated popup menu

//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlComboBox::AddLine( LPCTSTR str )
{
	if ( pPopupMenu ) pPopupMenu->Lines.push_back( str );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// removes all lines from the lines list of the associated popup menu
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlComboBox::ClearAllLines()
{
	if ( pPopupMenu ) pPopupMenu->Lines.clear();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// WndProc for combobox
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIControlComboBox::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	MESSAGEPARAMSSTRUCT	params;

	params.wParam = wParam;
	params.lParam = lParam;
									   
	switch ( message ) 
	{
	case WM_KEYDOWN:
	case GUIMSG_MOUSECLICK:
		PostMessage( hWnd, GUIMSG_COMBOBOXROLLDOWN, (WPARAM) 0, (LPARAM) this );
		return this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
	default:
		return this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
	}
	
	
	return false;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
//	returns the actual line string or empty string when no line is choosen
//
//////////////////////////////////////////////////////////////////////////////////////////////
CString CGUIControlComboBox::GetActualString()
{
	return pPopupMenu ? pPopupMenu->GetActualString() : "";
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function that is binded to associated popup menu object as a onLostFocus event
// handler; it closes the popup menu and disables it
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlComboBox::OnMenuLostFocus( LPARAM lParam )
{
	PostMessage( hWnd, GUIMSG_COMBOBOXROLLDOWN, (WPARAM) 2, (LPARAM) this );

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function that is binded to associated popup menu object as a onChange event
// handler; it closes the popup menu, disables it, returns focus to the combo box and also
// sets its value to the one choosen from menu
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlComboBox::OnMenuChangeValue( LPARAM lParam )
{		
	PostMessage( hWnd, GUIMSG_COMBOBOXROLLDOWN, (WPARAM) 1, (LPARAM) this );
	if ( lpOnChange ) lpOnChange( this, lParam );

	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
