#include "stdafx.h"
#include "GUIControlPopupMenu.h"


using namespace graphic;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlPopupMenu::CGUIControlPopupMenu()
{
	// inherited constructor automatically called here
	
	// set other properties default for this control type
	ControlType = GUICT_POPUPMENU;

	bStopable = true;
	bVisible = false;
	iTabStop = -1;
	uiTextFormat = GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER; // default value for text formating

	uiLineHeight = 20;

	Lines.clear();
	Value.SetValue( -1 );
	iActiveLine = -1;
	pInvokingObject = NULL;

	D3DXCreateSprite( pD3DDevice, &SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlPopupMenu::~CGUIControlPopupMenu()
{
	SAFE_RELEASE( SpriteBackground );
	Lines.clear();
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
HRESULT CGUIControlPopupMenu::Init( GUICONTROLDECLARATION * decl )
{
	// call derived Init
	this->CGUIControlBase::Init( decl );

	bEnabled = false;

	uiLineHeight = decl->uiListLineHeight;

	RecomputeMySize();

	return ERRNOERROR;	
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders this control
// must be called between BeginScene and EndScene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlPopupMenu::Render()
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
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlPopupMenu::Render()", "Can't obtain control's texture." )
	texture = texStruct->texture;


	// set text formatting
	if ( uiTextFormat & GUITEXT_HALIGN_LEFT ) format |= DT_LEFT;
	if ( uiTextFormat & GUITEXT_HALIGN_RIGHT ) format |= DT_RIGHT;
	if ( uiTextFormat & GUITEXT_HALIGN_CENTER ) format |= DT_CENTER;
	if ( uiTextFormat & GUITEXT_VALIGN_TOP ) format |= DT_TOP;
	if ( uiTextFormat & GUITEXT_VALIGN_BOTTOM ) format |= DT_BOTTOM;
	if ( uiTextFormat & GUITEXT_VALIGN_CENTER ) format |= DT_VCENTER;
	if ( uiTextFormat & GUITEXT_NOCLIP ) format |= DT_NOCLIP;



	// intitialize sprites rendering 
	hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Control's surface initialization failed." );
	




	// DRAW ALL POPUP MENU CORNERS
	
	// top left corner
	position.x = (float) actRect.left;
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = TopLeftCornerPos.iPosX;
	rect.top = TopLeftCornerPos.iPosY;
	rect.right = rect.left + TopLeftCornerPos.iWidth;
	rect.bottom = rect.top + TopLeftCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Unable to draw control object's background2." );


	// top right corner
	position.x = (float) ( actRect.right - TopRightCornerPos.iWidth + 1 );
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = TopRightCornerPos.iPosX;
	rect.top = TopRightCornerPos.iPosY;
	rect.right = rect.left + TopRightCornerPos.iWidth;
	rect.bottom = rect.top + TopRightCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Unable to draw control object's background3." );


	// bottom left corner
	position.x = (float) actRect.left;
	position.y = (float) ( actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) + 
							Lines.size() * uiLineHeight );
	position.z = 0.0f;

	rect.left = BottomLeftCornerPos.iPosX;
	rect.top = BottomLeftCornerPos.iPosY;
	rect.right = rect.left + BottomLeftCornerPos.iWidth;
	rect.bottom = rect.top + BottomLeftCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Unable to draw control object's background4." );


	// bottom right corner
	position.x = (float) ( actRect.right - BottomRightCornerPos.iWidth + 1 );
	position.y = (float) ( actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) + 
							Lines.size() * uiLineHeight );
	position.z = 0.0f;

	rect.left = BottomRightCornerPos.iPosX;
	rect.top = BottomRightCornerPos.iPosY;
	rect.right = rect.left + BottomRightCornerPos.iWidth;
	rect.bottom = rect.top + BottomRightCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Unable to draw control object's background5." );





	// DRAW THE POPUP MENU FRAME LINES

	// top horizontal line
	scaleX = (float) ( iWidth - TopLeftCornerPos.iWidth - TopRightCornerPos.iWidth ) / (float) HorizontalLinePos.iWidth;
	D3DXMatrixScaling( &mat, scaleX, 1.0f, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.left + TopLeftCornerPos.iWidth ) / scaleX;
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = HorizontalLinePos.iPosX;
	rect.top = HorizontalLinePos.iPosY;
	rect.right = rect.left + HorizontalLinePos.iWidth;
	rect.bottom = rect.top + HorizontalLinePos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Unable to draw control object's background6." );


	// bottom horizontal line
	scaleX = (float) ( iWidth - BottomLeftCornerPos.iWidth - BottomRightCornerPos.iWidth ) / (float) HorizontalLinePos.iWidth;
	D3DXMatrixScaling( &mat, scaleX, 1.0f, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.left + BottomLeftCornerPos.iWidth ) / scaleX;
	position.y = (float) ( actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) + 
							Lines.size() * uiLineHeight );
	position.z = 0.0f;

	rect.left = HorizontalLinePos.iPosX;
	rect.top = HorizontalLinePos.iPosY;
	rect.right = rect.left + HorizontalLinePos.iWidth;
	rect.bottom = rect.top + HorizontalLinePos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Unable to draw control object's background7." );


	// left vertical line
	scaleY = (float) ( Lines.size() * uiLineHeight ) / (float) VerticalLinePos.iHeight;
	D3DXMatrixScaling( &mat, 1.0f, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) actRect.left;
	position.y = (float) ( actRect.top + TopLeftCornerPos.iHeight ) / scaleY;
	position.z = 0.0f;

	rect.left = VerticalLinePos.iPosX;
	rect.top = VerticalLinePos.iPosY;
	rect.right = rect.left + VerticalLinePos.iWidth;
	rect.bottom = rect.top + VerticalLinePos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Unable to draw control object's background8." );


	// right vertical line
	scaleY = (float) ( Lines.size() * uiLineHeight ) / (float) VerticalLinePos.iHeight;
	D3DXMatrixScaling( &mat, 1.0f, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.right - VerticalLinePos.iWidth + 1 );
	position.y = (float) ( actRect.top + TopLeftCornerPos.iHeight ) / scaleY;
	position.z = 0.0f;

	rect.left = VerticalLinePos.iPosX;
	rect.top = VerticalLinePos.iPosY;
	rect.right = rect.left + VerticalLinePos.iWidth;
	rect.bottom = rect.top + VerticalLinePos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Unable to draw control object's background9." );



	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Control's surface presentation failed." );




		
	// DRAW ALL LINES
	rect.left = actRect.left + min( TopLeftCornerPos.iWidth, BottomLeftCornerPos.iWidth );
	rect.right = actRect.right - min( TopRightCornerPos.iWidth, BottomRightCornerPos.iWidth );
	for ( int i = 0; i < (int) Lines.size(); i++ )
	{
		rect.top = actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) + 
					i * uiLineHeight;
		rect.bottom = actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) +
						( i + 1 ) * uiLineHeight - 1;
		hr = DrawLineBackground( i, &rect, texture, dwTransparency, format );
		if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Control's item rendering failed." );
	}
	


	
	// call the binded event
	if ( lpOnRender ) lpOnRender( (CGUIControlBase *) this, 0 );

	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// draws single line background to specified location and writes the proper text on it
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlPopupMenu::DrawLineBackground(	int index,
													RECT * pRect,
													LPDIRECT3DTEXTURE9 tex,
													DWORD transparency,
													UINT format )
{
	HRESULT				hr;
	RECT				rect;
	D3DXVECTOR3			position;
	float				scaleX, scaleY;
	D3DXMATRIX			mat;
	GUITEXTUREFILEPOSITION	*	pTexPos;

	
	pTexPos = index == iActiveLine ? &TexturePos2 : &TexturePos1;


	// compute transformation
	scaleX = (float) ( pRect->right - pRect->left + 1 ) / pTexPos->iWidth;
	scaleY = (float) ( pRect->bottom - pRect->top + 1 ) / pTexPos->iHeight;
	
	D3DXMatrixScaling( &mat, scaleX, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );
	
	position.x = (float) pRect->left / scaleX;
	position.y = (float) pRect->top / scaleY;
	position.z = 0.0f;
	
	rect.left = pTexPos->iPosX;
	rect.top = pTexPos->iPosY;
	rect.right = rect.left + pTexPos->iWidth;
	rect.bottom = rect.top + pTexPos->iHeight;
	
	// intitialize sprites rendering 
	hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Control's surface initialization failed." );

	hr = SpriteBackground->Draw( tex, &rect, NULL, &position, transparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::DrawLineBackground()", "Unable to draw control object's background." );
	
	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlPopupMenu::Render()", "Control's surface presentation failed." );


	D3DXMatrixIdentity( &mat );
	SpriteBackground->SetTransform( &mat );

	
	if ( index == iActiveLine )
	{
		if ( pBasicFont ) pBasicFont->DrawText( NULL, (LPCTSTR) Lines[index], -1, pRect, format, uiActiveTextColor & (transparency | 0x00ffffff) );
	}
	else 
	{
		if ( pBasicFont ) pBasicFont->DrawText( NULL, (LPCTSTR) Lines[index], -1, pRect, format, uiBasicTextColor & (transparency | 0x00ffffff) );
	}

	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the ID of an item (line) to be actually selected
// sets -1 if there is no line of such ID
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlPopupMenu::SetValue( int ID )
{
	if ( ID < 0 ) iActiveLine = -1;
	else if ( ID >= (int) Lines.size() ) iActiveLine = -1;
	else iActiveLine = ID;
	Value.SetValue( iActiveLine );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// WndProc for combobox
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIControlPopupMenu::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	MESSAGEPARAMSSTRUCT	params;
	int					xPos, yPos;
	RECT				actRect;

	params.wParam = wParam;
	params.lParam = lParam;

	xPos = LOWORD( lParam ); 
	yPos = HIWORD( lParam );

	ComputeAbsoluteLocation( &actRect );
	actRect.left += min( TopLeftCornerPos.iWidth, BottomLeftCornerPos.iWidth );
	actRect.right -= min( TopRightCornerPos.iWidth, BottomRightCornerPos.iWidth );
	actRect.top += min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight );
	actRect.bottom = actRect.top + (int) Lines.size() * uiLineHeight;
									   
	switch ( message ) 
	{
	case WM_MOUSEMOVE:
		if ( xPos >= actRect.left && xPos <= actRect.right && yPos >= actRect.top && yPos <= actRect.bottom )
		{
			SetValue( (yPos - actRect.top) / uiLineHeight );
		}
		this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
		return true;
	
	case WM_LBUTTONDOWN:
		if ( xPos >= actRect.left && xPos <= actRect.right && yPos >= actRect.top && yPos <= actRect.bottom )
			SetValue( (yPos - actRect.top) / uiLineHeight );

		if ( lpOnChange )
		{
			// !!!!! If the menu is associated to some object and invoking object it therefor not NULL, the 
			// pointer to invoking object is used instead of a pointer to this menu
			if ( !pInvokingObject ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &(Value.GetString()) );
			else lpOnChange( pInvokingObject, (LPARAM) &(Value.GetString()) );
		}

		this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
		return true;

	case WM_KEYDOWN:
		// one step down
		if ( wParam == VK_DOWN ) if ( iActiveLine + 1 < (int) Lines.size() ) SetValue( iActiveLine + 1 );
		// one step up
		if ( wParam == VK_UP ) if ( iActiveLine > 0 ) SetValue( iActiveLine - 1 );
		// select item
		if ( wParam == VK_RETURN )
			if ( lpOnChange )
			{
				// !!!!! If the menu is associated to some object and invoking object it therefor not NULL, the 
				// pointer to invoking object is used instead of a pointer to this menu
				if ( !pInvokingObject ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &(Value.GetString()) );
				else lpOnChange( pInvokingObject, (LPARAM) &(Value.GetString()) );
			}
		
		this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
		return true;

	default:
		return this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
	}
	
	
	return false;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
//	returns the actual line string or empty string when no line is choosen
//
//////////////////////////////////////////////////////////////////////////////////////////////
CString CGUIControlPopupMenu::GetActualString()
{
	if ( iActiveLine < 0 ) return "";
	if ( iActiveLine >= (int) Lines.size() ) return "";

	return Lines[iActiveLine];
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// recomputes the size of popup menu dependingly on lines count
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlPopupMenu::RecomputeMySize()
{
	iHeight = (int) Lines.size() * uiLineHeight + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight )
		+ min( BottomRightCornerPos.iHeight, TopRightCornerPos.iHeight );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes and returns actual absolute position of a control object in pixels
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlPopupMenu::ComputeAbsoluteLocation( RECT * rect )
{
	RecomputeMySize();
	this->CGUIControlBase::ComputeAbsoluteLocation( rect );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
