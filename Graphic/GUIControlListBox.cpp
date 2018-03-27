#include "stdafx.h"
#include "GUIControlListBox.h"


using namespace graphic;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlListBox::CGUIControlListBox()
{
	// inherited constructor automatically called here
	
	// set other properties default for this control type
	ControlType = GUICT_LISTBOX;

	bStopable = true;
	bVisible = true;
	iTabStop = -1;
	uiTextFormat = GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER; // default value for text formating

	uiLineHeight = 20;

	Lines.clear();
	Value.SetValue( -1 );
	iActiveLine = -1;
	iFirstVisibleLine = 0;

	D3DXCreateSprite( pD3DDevice, &SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlListBox::~CGUIControlListBox()
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
HRESULT CGUIControlListBox::Init( GUICONTROLDECLARATION * decl )
{
	// call derived Init
	this->CGUIControlBase::Init( decl );

	bEnabled = true;

	uiLineHeight = decl->uiListLineHeight;
	SetLinesCount( decl->uiLinesCount );

	SetValue( 0 );


	return ERRNOERROR;	
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders this control
// must be called between BeginScene and EndScene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlListBox::Render()
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
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlListBox::Render()", "Can't obtain control's texture." )
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
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Control's surface initialization failed." );
	




	// DRAW ALL LISTBOX CORNERS
	
	// top left corner
	position.x = (float) actRect.left;
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = TopLeftCornerPos.iPosX;
	rect.top = TopLeftCornerPos.iPosY;
	rect.right = rect.left + TopLeftCornerPos.iWidth;
	rect.bottom = rect.top + TopLeftCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background2." );


	// top right corner
	position.x = (float) ( actRect.right - TopRightCornerPos.iWidth + 1 );
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = TopRightCornerPos.iPosX;
	rect.top = TopRightCornerPos.iPosY;
	rect.right = rect.left + TopRightCornerPos.iWidth;
	rect.bottom = rect.top + TopRightCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background3." );


	// bottom left corner
	position.x = (float) actRect.left;
	position.y = (float) ( actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) + 
		(uiLinesCount + 2) * uiLineHeight );
	position.z = 0.0f;

	rect.left = BottomLeftCornerPos.iPosX;
	rect.top = BottomLeftCornerPos.iPosY;
	rect.right = rect.left + BottomLeftCornerPos.iWidth;
	rect.bottom = rect.top + BottomLeftCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background4." );


	// bottom right corner
	position.x = (float) ( actRect.right - BottomRightCornerPos.iWidth + 1 );
	position.y = (float) ( actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) + 
							(uiLinesCount + 2) * uiLineHeight );
	position.z = 0.0f;

	rect.left = BottomRightCornerPos.iPosX;
	rect.top = BottomRightCornerPos.iPosY;
	rect.right = rect.left + BottomRightCornerPos.iWidth;
	rect.bottom = rect.top + BottomRightCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background5." );





	// DRAW THE LISTBOX FRAME LINES

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
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background6." );


	// bottom horizontal line
	scaleX = (float) ( iWidth - BottomLeftCornerPos.iWidth - BottomRightCornerPos.iWidth ) / (float) HorizontalLinePos.iWidth;
	D3DXMatrixScaling( &mat, scaleX, 1.0f, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.left + BottomLeftCornerPos.iWidth ) / scaleX;
	position.y = (float) ( actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) + 
							(uiLinesCount + 2) * uiLineHeight );
	position.z = 0.0f;

	rect.left = HorizontalLinePos.iPosX;
	rect.top = HorizontalLinePos.iPosY;
	rect.right = rect.left + HorizontalLinePos.iWidth;
	rect.bottom = rect.top + HorizontalLinePos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background7." );


	// left vertical line
	scaleY = (float) ( (uiLinesCount + 2) * uiLineHeight ) / (float) VerticalLinePos.iHeight;
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
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background8." );


	// right vertical line
	scaleY = (float) ( (uiLinesCount + 2) * uiLineHeight ) / (float) VerticalLinePos.iHeight;
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
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background9." );





	// DRAW "UP" AND "DOWN" BUTTONS

	// up button
	// fit the image to the line height (leave the X:Y aspect ratio on 1.0)
	scaleX = scaleY = (float) uiLineHeight / TexturePos3.iHeight;

	D3DXMatrixScaling( &mat, scaleX, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	rect.left = actRect.left + min( TopLeftCornerPos.iWidth, BottomLeftCornerPos.iWidth );
	rect.right = actRect.right - min( TopRightCornerPos.iWidth, BottomRightCornerPos.iWidth );
	position.x = (float) ( rect.left + (rect.right - rect.left + 1 - TexturePos3.iWidth) / 2 );
	position.x /= scaleX;
	position.y = (float) ( actRect.top + min( TopRightCornerPos.iHeight, TopLeftCornerPos.iHeight ) 
		+ ( (int) uiLineHeight - TexturePos3.iHeight ) / 2 );
	position.y /= scaleY;
	position.z = 0.0f;

	rect.left = TexturePos3.iPosX;
	rect.top = TexturePos3.iPosY;
	rect.right = rect.left + TexturePos3.iWidth;
	rect.bottom = rect.top + TexturePos3.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background1." );

	
	// down button
	// fit the image to the line height (leave the X:Y aspect ratio on 1.0)
	scaleX = scaleY = (float) uiLineHeight / TexturePos4.iHeight;

	D3DXMatrixScaling( &mat, scaleX, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	rect.left = actRect.left + min( TopLeftCornerPos.iWidth, BottomLeftCornerPos.iWidth );
	rect.right = actRect.right - min( TopRightCornerPos.iWidth, BottomRightCornerPos.iWidth );
	position.x = (float) ( rect.left + (rect.right - rect.left + 1 - TexturePos4.iWidth) / 2 );
	position.x /= scaleX;
	position.y = (float) ( actRect.top + min( TopRightCornerPos.iHeight, TopLeftCornerPos.iHeight )
		+ ( uiLinesCount + 1 ) * uiLineHeight + ( (int) uiLineHeight - TexturePos4.iHeight ) / 2 );
	position.y /= scaleY;    
	position.z = 0.0f;

	rect.left = TexturePos4.iPosX;
	rect.top = TexturePos4.iPosY;
	rect.right = rect.left + TexturePos4.iWidth;
	rect.bottom = rect.top + TexturePos4.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Unable to draw control object's background1." );


	D3DXMatrixIdentity( &mat );
	SpriteBackground->SetTransform( &mat );




	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Control's surface presentation failed." );





	// DRAW ALL LINES
	rect.left = actRect.left + min( TopLeftCornerPos.iWidth, BottomLeftCornerPos.iWidth );
	rect.right = actRect.right - min( TopRightCornerPos.iWidth, BottomRightCornerPos.iWidth );

	rect.top = actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight );
	rect.bottom = actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) + uiLineHeight - 1;
	DrawLineBackground( -1, &rect, texture, dwTransparency, format );

	rect.top += (uiLinesCount + 1) * uiLineHeight;
	rect.bottom += (uiLinesCount + 1) * uiLineHeight;
	DrawLineBackground( -1, &rect, texture, dwTransparency, format );

	for ( int i = 1, j = iFirstVisibleLine; i <= (int) uiLinesCount; i++, j++ )
	{
		rect.top = actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) + 
			i * uiLineHeight;
		rect.bottom = actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) +
			( i + 1 ) * uiLineHeight - 1;
		hr = DrawLineBackground( j, &rect, texture, dwTransparency, format );
		if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Control's item rendering failed." );
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
HRESULT CGUIControlListBox::DrawLineBackground(	int index,
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

	

	pTexPos = (index == iActiveLine && index != -1)? &TexturePos2 : &TexturePos1;


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
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Control's surface initialization failed." );

	hr = SpriteBackground->Draw( tex, &rect, NULL, &position, transparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::DrawLineBackground()", "Unable to draw control object's background." );
	
	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlListBox::Render()", "Control's surface presentation failed." );

	D3DXMatrixIdentity( &mat );
	SpriteBackground->SetTransform( &mat );



	if ( index >= (int) Lines.size() || index < 0 ) return ERRNOERROR;

	if ( index == iActiveLine )
	{
		if ( pBasicFont && iActiveLine >= 0 ) 
			pBasicFont->DrawText( NULL, (LPCTSTR) Lines[index].csText, -1, pRect, format, uiActiveTextColor & (transparency | 0x00ffffff) );
	}
	else 
	{
		if ( pBasicFont && iActiveLine >= 0 ) 
			pBasicFont->DrawText( NULL, (LPCTSTR) Lines[index].csText, -1, pRect, format, uiBasicTextColor & (transparency | 0x00ffffff) );
	}

	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the ID of an item (line) to be actually selected
// sets 0 if there is no line of such ID
// also call the function bound to onChange handler
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::SetValue( int ID )
{
	CString		str;
	
	SetValueSilent( ID );

	str = GetActualString();
	if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &str );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the ID of an item (line) to be actually selected
// sets 0 if there is no line of such ID
// !!! this DOES NOT call the onChange event !!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::SetValueSilent( int ID )
{
	if ( ID < 0 ) iActiveLine = 0;
	else if ( ID >= (int) Lines.size() ) iActiveLine = 0;
	else iActiveLine = ID;
	
	Value.SetValue( iActiveLine );
	
	if ( iActiveLine < iFirstVisibleLine ) ChangeFirstVisibleLine( iActiveLine );
	if ( iActiveLine >= iFirstVisibleLine + (int) uiLinesCount ) 
		ChangeFirstVisibleLine( iActiveLine - (int) uiLinesCount + 1 );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// WndProc for combobox
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIControlListBox::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
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
	actRect.bottom = actRect.top + ( uiLinesCount + 2 ) * uiLineHeight;
									   
	switch ( message ) 
	{
	case WM_LBUTTONDOWN:
		if ( xPos >= actRect.left && xPos <= actRect.right && yPos >= actRect.top && yPos <= actRect.bottom )
		{
			int index = ( yPos - actRect.top ) / uiLineHeight;
			if ( index == 0 ) ChangeFirstVisibleLine( iFirstVisibleLine - 1 ); 
			else if ( index == uiLinesCount + 1 ) ChangeFirstVisibleLine( iFirstVisibleLine + 1 ); 
			else SetValue( index + iFirstVisibleLine - 1 );
		}

		this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
		return true;

	case WM_KEYDOWN:
		// one step down
		if ( wParam == VK_DOWN ) if ( iActiveLine + 1 < (int) Lines.size() ) SetValue( iActiveLine + 1 );
		// one step up
		if ( wParam == VK_UP ) if ( iActiveLine > 0 ) SetValue( iActiveLine - 1 );
		// one page down
		if ( wParam == VK_NEXT ) ChangeFirstVisibleLine( iFirstVisibleLine + (int) uiLinesCount - 1 );
		// one page up
		if ( wParam == VK_PRIOR ) ChangeFirstVisibleLine( iFirstVisibleLine - (int) uiLinesCount + 1 );
		
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
CString CGUIControlListBox::GetActualString() const
{
	if ( iActiveLine < 0 ) return "";
	if ( iActiveLine >= (int) Lines.size() ) return "";

	return Lines[iActiveLine].csText;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns text associated with the specified item or empty string if the item doesn't exist
//
//////////////////////////////////////////////////////////////////////////////////////////////
CString CGUIControlListBox::GetItemText( int index ) const
{ 
	if ( index < (int) Lines.size() && index >= 0 ) return Lines[index].csText;
	else return ""; 
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns the structure associated with the specified list item (line) or empty structure if 
//  the item doesn't exist
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlListBox::SINGLEITEM CGUIControlListBox::GetItem( int index ) const
{ 
	if ( index < (int) Lines.size() && index >= 0 ) return Lines[index];
	else return SINGLEITEM(); 
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns the actual line string or empty string when no line is choosen
//
//////////////////////////////////////////////////////////////////////////////////////////////
CString CGUIControlListBox::GetActualItemText() const
{ 
	return GetActualString();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns the structure associated with the actually selected item (line) or empty structure
//  if the item doesn't exist
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlListBox::SINGLEITEM CGUIControlListBox::GetActualItem() const
{ 
	if ( iActiveLine < (int) Lines.size() && iActiveLine >= 0 ) return Lines[iActiveLine];
	else return SINGLEITEM(); 
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// removes the last item from the list;
// if the item being removed is active, the last but one item is set as active
// if there are no more items, active item is set to -1
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::PopBack()
{
	Lines.pop_back();
	if ( iActiveLine >= (int) Lines.size() ) iActiveLine = (int) Lines.size() - 1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a new item at the end of the list
// if there were no items before this action, active item index is set to 0
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::PushBack( SINGLEITEM item )
{
	Lines.push_back( item );
	if ( iActiveLine < 0 ) iActiveLine = (int) Lines.size() - 1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a new item at the end of the list
// the item being added is defined by its text
// if there were no items before this action, active item index is set to 0
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::PushBack( CString text )
{
	Lines.push_back( SINGLEITEM( text ) );
	if ( iActiveLine < 0 ) iActiveLine = (int) Lines.size() - 1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a new item at the end of the list
// the item being added is defined by its text and integer value
// if there were no items before this action, active item index is set to 0
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::PushBack( CString text, int value )
{
	Lines.push_back( SINGLEITEM( text, value ) );
	if ( iActiveLine < 0 ) iActiveLine = (int) Lines.size() - 1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a new item at the end of the list
// the item being added is defined by its text and custom data
// if there were no items before this action, active item index is set to 0
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::PushBack( CString text, void * data )
{
	Lines.push_back( SINGLEITEM( text, data ) );
	if ( iActiveLine < 0 ) iActiveLine = (int) Lines.size() - 1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// clears the list and sets the active item idex to -1
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::Clear()
{ 
	Lines.clear();
	iActiveLine = -1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the first visible line to the one specified if possible or change it as much as possible
// due to listbox size
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::ChangeFirstVisibleLine( int index )
{
	iFirstVisibleLine = index;
	if ( iFirstVisibleLine + (int) uiLinesCount > (int) Lines.size() ) 
		iFirstVisibleLine = (int) ( Lines.size() - uiLinesCount );
	if ( iFirstVisibleLine < 0 ) iFirstVisibleLine = 0;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the actual count of lines visible
// also recomputes the height of the control
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlListBox::SetLinesCount( UINT linesCount )
{ 
	uiLinesCount = linesCount;
	iHeight = (uiLinesCount + 2) * uiLineHeight + min( TopRightCornerPos.iHeight, TopLeftCornerPos.iHeight )
		+ min( BottomRightCornerPos.iHeight, BottomLeftCornerPos.iHeight );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
