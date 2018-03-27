#include "stdafx.h"
#include "GUIControlImage.h"


using namespace graphic;




//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlImage::CGUIControlImage()
{
	// inherited constructor automatically called here
	
	// set other properties default for this control type
	ControlType = GUICT_IMAGE;

	bStopable = false;
	iTabStop = -1;
	ridImage = -1;

	D3DXCreateSprite( pD3DDevice, &SpriteBackground );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlImage::~CGUIControlImage()
{
	SAFE_RELEASE( SpriteBackground );
	if ( ridImage > 0 ) pResourceManager->ReleaseResource( ridImage );
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads an image from file; set image position in the texture in pTexPos paramter, or set
// this paramter to NULL if the entire image is used
// this also releases texture assigned to it before
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlImage::LoadImage( LPCTSTR fileName, GUITEXTUREFILEPOSITION *pTexPos )
{
	HRESULT				hr;
 	resManNS::__Texture		*	texStruct;
	LPDIRECT3DTEXTURE9	texture;
	D3DSURFACE_DESC		SurfDesc;
	resManNS::RESOURCEID			resID;
	

	resID = ridImage;

	// load new texture resource
	ridImage = pResourceManager->LoadResource( fileName, RES_Texture );

	// release possibly allocated texture resource first
	if ( resID > 0 ) pResourceManager->ReleaseResource( resID );

	if ( ridImage <= 0 ) ERRORMSG( ERRNOTFOUND, "CGUIControlImage::LoadImage()", "Can't load image to control object." );


	// if the texture position in file is not defined, use texture size instead
	if ( pTexPos ) TexturePos2 = *pTexPos;
	else
	{
		texStruct = pResourceManager->GetTexture( ridImage );
		if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlImage::LoadImage()", "Can't obtain control's associated image." )
		texture = texStruct->texture;
		
		hr = texture->GetLevelDesc( 0, &SurfDesc );
		if ( hr ) ERRORMSG( hr, "CGUIControlImage::LoadImage()", "" );

		TexturePos2.iPosX = TexturePos2.iPosY = 0;
		TexturePos2.iWidth = SurfDesc.Width;
		TexturePos2.iHeight = SurfDesc.Height;
	}
	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// duplicates and assignes already loaded texture resource
// this also releases texture assigned to it before
//
// set image position in the texture in pTexPos paramter, or set this paramter to NULL 
// if the entire image is used
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlImage::SetImage(	resManNS::RESOURCEID ridTexture, GUITEXTUREFILEPOSITION *pTexPos )
{
	HRESULT				hr;
 	resManNS::__Texture		*	texStruct;
	LPDIRECT3DTEXTURE9	texture;
	D3DSURFACE_DESC		SurfDesc;


	// release possibly allocated texture resource first
	if ( ridImage > 0 ) pResourceManager->ReleaseResource( ridImage );

	ridImage = ridTexture;
	
	if ( ridImage <= 0 ) return ERRNOERROR;

	// duplicate texture resource
	pResourceManager->DuplicateResource( ridImage );

	// if the texture position in file is not defined, use texture size instead
	if ( pTexPos ) TexturePos2 = *pTexPos;
	else
	{
		texStruct = pResourceManager->GetTexture( ridImage );
		if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlImage::SetImage()", "Can't obtain control's associated image." )
		texture = texStruct->texture;
		
		hr = texture->GetLevelDesc( 0, &SurfDesc );
		if ( hr ) ERRORMSG( hr, "CGUIControlImage::SetImage()", "" );

		TexturePos2.iPosX = TexturePos2.iPosY = 0;
		TexturePos2.iWidth = SurfDesc.Width;
		TexturePos2.iHeight = SurfDesc.Height;
	}

	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders this control
// must be called between BeginScene and EndScene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIControlImage::Render()
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



	// intitialize sprites rendering 
	hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CGUIControlImage::Render()", "Control's surface initialization failed." );
	

	
	// obtain texture pointer
	texStruct = pResourceManager->GetTexture( idBackgroundTexture );
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlImage::Render()", "Can't obtain control's texture." )
	texture = texStruct->texture;

	pTSPos = &TexturePos1;


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


	// draw control's background
	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIControlImage::Render()", "Unable to draw control object's background." );


	if ( ridImage > 0 )
	{
		// obtain texture pointer
		texStruct = pResourceManager->GetTexture( ridImage );
		if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIControlImage::Render()", "Can't obtain control's assigned image." )
		texture = texStruct->texture;

		pTSPos = &TexturePos2;


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


		// draw control's background
		hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
		if ( hr ) ERRORMSG( hr, "CGUIControlImage::Render()", "Unable to draw control object's background." );
	}



	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIControlImage::Render()", "Control's surface presentation failed." );
	

	
	// call the binded event
	if ( lpOnRender ) lpOnRender( (CGUIControlBase *) this, 0 );

	
	return ERRNOERROR;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// WndProc for image
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIControlImage::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	MESSAGEPARAMSSTRUCT	params;

	if ( !bEnabled ) return false;

	params.wParam = wParam;
	params.lParam = lParam;

	return this->CGUIControlBase::WndProc( hWnd, message, wParam, lParam );
	
	
	return false;
} ;


//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
