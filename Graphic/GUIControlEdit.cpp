#include "stdafx.h"
#include "GUIControlEdit.h"


using namespace graphic;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlEdit::CGUIControlEdit()
{
	// inherited constructor automatically called here
	
	// set other properties default for this control type
	ControlType = GUICT_EDIT;

	bStopable = true;
	iTabStop = -1;
	uiTextFormat = GUITEXT_HALIGN_LEFT | GUITEXT_VALIGN_CENTER; // default value for text formating

	iMaxTextLength = 20;

	SetValue( "" );
	iCursorPosition = 0;

	D3DXCreateSprite( pD3DDevice, &SpriteBackground );
	D3DXCreateLine( pD3DDevice, &pLineObject );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlEdit::~CGUIControlEdit()
{
	SAFE_RELEASE( SpriteBackground );
	SAFE_RELEASE( pLineObject );
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
HRESULT CGUIControlEdit::Init( GUICONTROLDECLARATION * decl )
{
	// call derived Init
	this->CGUIControlBase::Init( decl );

	iMaxTextLength = decl->iMaxEditTextLen;

	return ERRNOERROR;	
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders this control
// must be called between BeginScene and EndScene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlEdit::Render()
{
	UINT				format = 0;
	DWORD				dwTransparency;
	HRESULT				hr;
	RECT				rect, actRect;
	LPDIRECT3DTEXTURE9	texture;
	D3DXVECTOR3			position;
	float				scaleX, scaleY;
	D3DXMATRIX			mat;
	D3DCOLOR		*	pColor;
	resManNS::__Texture		*	texStruct;


	// don't render if this object is set to be invisible
	if ( !bVisible ) return ERRNOERROR;
	

	// compute control's transparency
	dwTransparency = ( (DWORD) ( 255 * (1 - fTransparency) ) ) << 24;

	
	// compute actual rectangle
	ComputeAbsoluteLocation( &actRect );


	// obtain texture pointer
	texStruct = pResourceManager->GetTexture( idBackgroundTexture );
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlEdit::Render()", "Can't obtain control's texture." )
	texture = texStruct->texture;



	// intitialize sprites rendering 
	hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Control's surface initialization failed." );
	


	// DRAW THE EDIT BACKGROUND
	
	// compute transformation
	scaleX = (float) ( iWidth - min( TopLeftCornerPos.iWidth, BottomLeftCornerPos.iWidth ) 
				- min( TopRightCornerPos.iWidth, BottomRightCornerPos.iWidth ) );
	scaleX /= TexturePos1.iWidth;
	scaleY = (float) ( iHeight - min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight )
				- min( BottomLeftCornerPos.iHeight, BottomRightCornerPos.iHeight ) );
	scaleY /= TexturePos1.iHeight;

	D3DXMatrixScaling( &mat, scaleX, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.left + min( TopLeftCornerPos.iWidth, BottomLeftCornerPos.iWidth ) ) / scaleX;
	position.y = (float) ( actRect.top + min( TopLeftCornerPos.iHeight, TopRightCornerPos.iHeight ) ) / scaleY;
	position.z = 0.0f;

	rect.left = TexturePos1.iPosX;
	rect.top = TexturePos1.iPosY;
	rect.right = rect.left + TexturePos1.iWidth;
	rect.bottom = rect.top + TexturePos1.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Unable to draw control object's background1." );

	D3DXMatrixIdentity( &mat );
	SpriteBackground->SetTransform( &mat );



	// DRAW THE EDIT CORNERS

	// top left corner
	position.x = (float) actRect.left;
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = TopLeftCornerPos.iPosX;
	rect.top = TopLeftCornerPos.iPosY;
	rect.right = rect.left + TopLeftCornerPos.iWidth;
	rect.bottom = rect.top + TopLeftCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Unable to draw control object's background1." );


 	// top right corner
	position.x = (float) ( actRect.right - TopRightCornerPos.iWidth + 1 );
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = TopRightCornerPos.iPosX;
	rect.top = TopRightCornerPos.iPosY;
	rect.right = rect.left + TopRightCornerPos.iWidth;
	rect.bottom = rect.top + TopRightCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Unable to draw control object's background2." );


	// bottom left corner
	position.x = (float) actRect.left;
	position.y = (float) ( actRect.bottom - BottomLeftCornerPos.iWidth + 1 );
	position.z = 0.0f;

	rect.left = BottomLeftCornerPos.iPosX;
	rect.top = BottomLeftCornerPos.iPosY;
	rect.right = rect.left + BottomLeftCornerPos.iWidth;
	rect.bottom = rect.top + BottomLeftCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Unable to draw control object's background3." );


 	// bottom right corner
	position.x = (float) ( actRect.right - BottomRightCornerPos.iWidth + 1 );
	position.y = (float) ( actRect.bottom - BottomRightCornerPos.iWidth + 1 );
	position.z = 0.0f;

	rect.left = BottomRightCornerPos.iPosX;
	rect.top = BottomRightCornerPos.iPosY;
	rect.right = rect.left + BottomRightCornerPos.iWidth;
	rect.bottom = rect.top + BottomRightCornerPos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Unable to draw control object's background4." );



	// DRAW THE EDIT FRAME LINES

	// top horizontal line
	scaleX = (float) ( iWidth - TopLeftCornerPos.iWidth - TopRightCornerPos.iWidth ) / (float) HorizontalLinePos.iWidth;
	D3DXMatrixScaling( &mat, scaleX, 1.0f, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.left + TopLeftCornerPos.iWidth )/ scaleX;
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = HorizontalLinePos.iPosX;
	rect.top = HorizontalLinePos.iPosY;
	rect.right = rect.left + HorizontalLinePos.iWidth;
	rect.bottom = rect.top + HorizontalLinePos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Unable to draw control object's background1." );


	// bottom horizontal line
	scaleX = (float) ( iWidth - BottomLeftCornerPos.iWidth - BottomRightCornerPos.iWidth ) / (float) HorizontalLinePos.iWidth;
	D3DXMatrixScaling( &mat, scaleX, 1.0f, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.left + BottomLeftCornerPos.iWidth ) / scaleX;
	position.y = (float) ( actRect.bottom - HorizontalLinePos.iHeight + 1 );
	position.z = 0.0f;

	rect.left = HorizontalLinePos.iPosX;
	rect.top = HorizontalLinePos.iPosY;
	rect.right = rect.left + HorizontalLinePos.iWidth;
	rect.bottom = rect.top + HorizontalLinePos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Unable to draw control object's background1." );


	// left vertical line
	scaleY = (float) ( iHeight - TopLeftCornerPos.iHeight - BottomLeftCornerPos.iHeight ) / (float) VerticalLinePos.iHeight;
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
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Unable to draw control object's background1." );


	// right vertical line
	scaleY = (float) ( iHeight - TopRightCornerPos.iHeight - BottomRightCornerPos.iHeight ) / (float) VerticalLinePos.iHeight;
	D3DXMatrixScaling( &mat, 1.0f, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.right - VerticalLinePos.iWidth + 1 );
	position.y = (float) ( actRect.top + TopRightCornerPos.iHeight ) / scaleY;
	position.z = 0.0f;

	rect.left = VerticalLinePos.iPosX;
	rect.top = VerticalLinePos.iPosY;
	rect.right = rect.left + VerticalLinePos.iWidth;
	rect.bottom = rect.top + VerticalLinePos.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Unable to draw control object's background1." );




	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlEdit::Render()", "Control's surface presentation failed." );



	
	// DRAW TEXT
	if ( Value.GetString().GetLength() > iMaxTextLength ) 
		Value.SetValue( Value.GetString().Left( iMaxTextLength ) );
		
	rect.left = actRect.left + HorizontalLinePos.iWidth + 2;
	rect.right = actRect.right - HorizontalLinePos.iWidth - 1;
	rect.top = actRect.top + VerticalLinePos.iHeight + 2;
	rect.bottom = actRect.bottom - VerticalLinePos.iHeight - 3;

	// set text formatting
	if ( uiTextFormat & GUITEXT_HALIGN_LEFT ) format |= DT_LEFT;
	if ( uiTextFormat & GUITEXT_VALIGN_TOP ) format |= DT_TOP;
	if ( uiTextFormat & GUITEXT_VALIGN_BOTTOM ) format |= DT_BOTTOM;
	if ( uiTextFormat & GUITEXT_VALIGN_CENTER ) format |= DT_VCENTER;

	pColor = bHasFocus ? &uiActiveTextColor : &uiBasicTextColor;

	// draw input text
	if ( pBasicFont ) pBasicFont->DrawText( NULL, (LPCTSTR) Value.GetString(), -1, &rect, format, (*pColor) & (dwTransparency | 0x00ffffff) );


	
	// DRAW INDICATOR OF THE CURSOR POSITION
	if ( bHasFocus )
	{
		D3DCOLOR		color;
		D3DXVECTOR2		vecs[2];
		CString			str = Value.GetString().Left( iCursorPosition );
		int				spaces = 0;

		// calculate the rectangle of text drawn left from the cursor position
		if ( str.GetLength() && pBasicFont ) 
		{
			format |= DT_CALCRECT;
			pBasicFont->DrawText( NULL, (LPCTSTR) str, -1, &rect, format, (*pColor) & (dwTransparency | 0x00ffffff) );
			// compute count of trailing spaces
			for ( int i = str.GetLength() - 1; i >= 0; i-- )
			{
				if ( str[i] == ' ' ) spaces++;
				else break;
			}
			vecs[0].x = vecs[1].x = (float) ( rect.right + 4 * spaces );
			vecs[0].y = (float) rect.top;
			vecs[1].y = (float) rect.bottom;
		}
		else
		{
			vecs[0].x = vecs[1].x = (float) (rect.left - 1);
			vecs[0].y = (float) rect.top;
			vecs[1].y = (float) rect.bottom;
		}

		// make the cursor color inverse to the text color but with full opacity
		color = ~(*pColor & 0x00ffffff); 
		
		// draw the line
		pLineObject->Begin();
		if ( vecs[0].x <= (float) ( actRect.right - HorizontalLinePos.iWidth - 1 ) ) 
			pLineObject->Draw( vecs, 2, (*pColor) & (dwTransparency | 0x00ffffff) );
		pLineObject->End();
	}




	// call the binded event
	if ( lpOnRender ) lpOnRender( (CGUIControlBase *) this, 0 );

	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the new text value for the edit box
// calls the onChange event with the paramter set to a pointer to CString with the input text
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlEdit::SetValue( LPCTSTR str )
{
	SetValueSilent( str );

	if ( lpOnChange ) lpOnChange( (CGUIControlBase *) this, (LPARAM) &(Value.GetString()) );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the new text value for the edit box, but DOES NOT call the onChange event !
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlEdit::SetValueSilent( LPCTSTR str )
{
	Value.SetValue( str );
	
	if ( Value.GetString().GetLength() > iMaxTextLength ) 
		Value.SetValue( Value.GetString().Left( iMaxTextLength ) ); 
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the cursor position (makes position correction if needed)
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIControlEdit::SetCursorPosition( int pos )
{
	iCursorPosition = pos;
	if ( iCursorPosition < 0 ) iCursorPosition = 0;
	if ( iCursorPosition > Value.GetString().GetLength() ) 
		iCursorPosition = Value.GetString().GetLength();
}	


//////////////////////////////////////////////////////////////////////////////////////////////
//
// WndProc for track bar
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIControlEdit::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	MESSAGEPARAMSSTRUCT	params;
	CString				str;
	bool				res = false;

	params.wParam = wParam;
	params.lParam = lParam;
									   
	switch ( message ) 
	{
	case WM_CHAR:
		// process character keys
		if ( wParam >= 32 && wParam <= 255 )
		{
			str = Value.GetString().Left( iCursorPosition );
			str += (char) wParam;
			str += Value.GetString().Right( Value.GetString().GetLength() - iCursorPosition );
			SetValue( str );
			SetCursorPosition( iCursorPosition + 1 );
			res = true;
		}
		
		this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
		
		return res;

	case WM_KEYDOWN:
		// process control keys
		switch ( wParam )
		{
		case VK_RIGHT:
			SetCursorPosition( iCursorPosition + 1 );
			res = true;
			break;
		case VK_LEFT:
			SetCursorPosition( iCursorPosition - 1 );
			res = true;
			break;
		case VK_HOME:
			SetCursorPosition( 0 );
			res = true;
			break;
		case VK_END:
			SetCursorPosition( Value.GetString().GetLength() );
			res = true;
			break;
		case VK_DELETE:
			str = Value.GetString().Left( iCursorPosition );
			str += Value.GetString().Right( Value.GetString().GetLength() - iCursorPosition - 1 );
			SetValue( str );
			res = true;
			break;
		case VK_BACK:
			if ( iCursorPosition <= 0 ) { res = true; break; }
			str = Value.GetString().Left( iCursorPosition - 1 );
			str += Value.GetString().Right( Value.GetString().GetLength() - iCursorPosition );
			SetValue( str );
			SetCursorPosition( iCursorPosition - 1 );
			res = true;
			break;
		}
		
		this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
		
		return res;

	default:
		return this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
	}
	
	
	return false;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////

