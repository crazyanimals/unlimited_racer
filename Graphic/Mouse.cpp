#include "stdafx.h"
#include "Mouse.h"


using namespace graphic;


////////////////////////////////////////////////////////////////////////////////////////////// 
//
//									  constructor
//
////////////////////////////////////////////////////////////////////////////////////////////// 
CMouse::CMouse()
{
	CursorsList.clear();
	pActualCursor = NULL;
	pResourceManager = NULL;
	pD3DDevice = NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////// 
//
//									initializing  constructor
//
////////////////////////////////////////////////////////////////////////////////////////////// 
CMouse::CMouse( resManNS::CResourceManager * pResMan, LPDIRECT3DDEVICE9 pD3DDev )
{
	CursorsList.clear();
	pActualCursor = NULL;
	pResourceManager = pResMan;
	pD3DDevice = pD3DDev;
}


////////////////////////////////////////////////////////////////////////////////////////////// 
//
//									  destructor
//
////////////////////////////////////////////////////////////////////////////////////////////// 
CMouse::~CMouse()
{
	CURSOR_ITER		iter;

	for ( iter = CursorsList.begin(); iter != CursorsList.end(); iter++ )
	{
		SAFE_DELETE( *iter );
	}

	CursorsList.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////// 
//
// initializes this object
//
////////////////////////////////////////////////////////////////////////////////////////////// 
HRESULT CMouse::Init( resManNS::CResourceManager * pResMan, LPDIRECT3DDEVICE9 pD3DDev )
{
	pResourceManager = pResMan;
	pD3DDevice = pD3DDev;

	return ERRNOERROR;
}


////////////////////////////////////////////////////////////////////////////////////////////// 
//
// adds a new cursor and returns its index thru parameter
// also sets this cursor as actual if not specified otherwise (set the bSetAsActual parameter)
//
// returns a nonzero error code when any error occurs	
//
////////////////////////////////////////////////////////////////////////////////////////////// 
HRESULT CMouse::AddCursor( LPCTSTR fileName, int * index, bool bSetAsActual )
{
	HRESULT			hr;
	CMouseCursor	*pCursor;

	*index = -1;

	pCursor = new CMouseCursor();
	if ( !pCursor ) ERRORMSG( ERROUTOFMEMORY, "CMouse::AddCursor()", "Unable to create cursor object." ); 
	
	hr = pCursor->Init( pD3DDevice, pResourceManager, fileName );
	if ( hr ) ERRORMSG( hr, "CMouse::AddCursor()", "Can't initialize mouse cursor." );

	CursorsList.push_back( pCursor );
	if ( bSetAsActual ) pActualCursor = pCursor;
	
	*index = (int) CursorsList.size();
	
	return ERRNOERROR;
}


////////////////////////////////////////////////////////////////////////////////////////////// 
//
//					draws an actual cursor sprite to actual coordinates
//
////////////////////////////////////////////////////////////////////////////////////////////// 
HRESULT CMouse::DrawCursor() const
{
	HRESULT		hr;
	
	if ( pActualCursor ) 
	{
		hr = pActualCursor->Draw( iPosX, iPosY );
		if ( hr ) ERRORMSG( hr, "CMouse::DrawCursor()", "Unable to draw mouse cursor." );
	}

	return ERRNOERROR;
}


////////////////////////////////////////////////////////////////////////////////////////////// 
//
//						selects cursor from a queue to be actual
//
////////////////////////////////////////////////////////////////////////////////////////////// 
void CMouse::SelectCursor( int index )
{
	if ( index < (int) CursorsList.size() ) pActualCursor = CursorsList[index];
}


////////////////////////////////////////////////////////////////////////////////////////////// 
//
// returns actual cursor index
//
////////////////////////////////////////////////////////////////////////////////////////////// 
int CMouse::GetActualCursorIndex()
{
	CURSOR_ITER		iter;
	int				i = 1;
	
	for ( iter = CursorsList.begin(); iter != CursorsList.end(); iter++, i++ )
	{
		if ( *iter == pActualCursor ) return i;
	}

	return -1;
}


////////////////////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////////////////////// 
//
//						C M o u s e C u r s o r       c l a s s
//
////////////////////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////////////////////// 

////////////////////////////////////////////////////////////////////////////////////////////// 
//
//										constructor
//
////////////////////////////////////////////////////////////////////////////////////////////// 
CMouseCursor::CMouseCursor()
{
	idTexture = -1;
	pDXSprite = NULL;
	iHotSpotX = 0;
	iHotSpotY = 0;
	pResourceManager = NULL;
	pD3DDevice = NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////// 
//
//										destructor
//
////////////////////////////////////////////////////////////////////////////////////////////// 
CMouseCursor::~CMouseCursor()
{
	SAFE_RELEASE( pDXSprite );
	if ( pResourceManager && idTexture > 0 ) pResourceManager->ReleaseResource( idTexture );
}


////////////////////////////////////////////////////////////////////////////////////////////// 
//
// loads a mouse sprite from a file 
// returns a nonzero value when any error occurs
//
////////////////////////////////////////////////////////////////////////////////////////////// 
HRESULT CMouseCursor::Init( LPDIRECT3DDEVICE9 pD3DDev, resManNS::CResourceManager *pResMan, LPCTSTR fileName )
{
	FILE	*	fil;
	HRESULT		hr;
	char		str[520];
	int			anim;

	
	// store crucial pointers
	pD3DDevice = pD3DDev;
	pResourceManager = pResMan;
	TextureFileName = fileName;

	// create new sprite object
	hr = D3DXCreateSprite( pD3DDevice, &pDXSprite );
	if ( hr ) ERRORMSG( hr, "CMouseCursor::Init()", "Can't create mouse sprite object." );

	
	// read the cursor definition from specified file
	if ( !( fil = fopen( fileName, "rt" ) ) ) ERRORMSG( ERRFILENOTFOUND, "CMouseCursor::Init()", fileName );
	
	hr = fscanf( fil, "%li\n%li %li\n", &anim, &iHotSpotX, &iHotSpotY );
	if ( hr != 3 ) ERRORMSG( ERRFILECORRUPTED, "CMouseCursor::Init()", fileName );
	if ( !fgets( str, 520, fil ) ) ERRORMSG( ERRFILECORRUPTED, "CMouseCursor::Init()", fileName );
	
	fclose( fil );

	bAnimated = anim ? true : false;

	
	// load the cursor's sprite
	idTexture = pResourceManager->LoadResource( str, RES_Texture );
	if ( idTexture <= 0 ) ERRORMSG( ERRGENERIC, "CMouseCursor::Init()", "An error occurs while loading mouse cursor sprite." );


	return ERRNOERROR;
}


////////////////////////////////////////////////////////////////////////////////////////////// 
//
//					Draws a cursor to specified screen coordinates
//
////////////////////////////////////////////////////////////////////////////////////////////// 
HRESULT CMouseCursor::Draw( int X, int Y ) 
{
	LPDIRECT3DTEXTURE9	texture;
	D3DXVECTOR3			position;
	HRESULT				hr;
	resManNS::__Texture		*	texStruct;


	// obtain texture pointer
	texStruct = pResourceManager->GetTexture( idTexture );
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CMouseCursor::Draw()", "Can't obtain cursor's texture." )
	texture = texStruct->texture;

	position.x = (float) ( X - iHotSpotX );
	position.y = (float) ( Y - iHotSpotY );
	position.z = 0.0f;

	// intitialize sprites rendering 
	hr = pDXSprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CMouseCursor::Draw()", "Mouse cursor surface initialization failed." );
	
	hr = pDXSprite->Draw( texture, NULL, NULL, &position, 0xffffffff );
	if ( hr ) ERRORMSG( hr, "CMouseCursor::Draw()", "Unable to draw mouse sprite." );
	
	// signalize finished rendering and draw all sprites to backbuffer
	hr = pDXSprite->End();
	if ( hr ) ERRORMSG( hr, "CMouseCursor::Draw()", "Mouse cursor surface presentation failed." );

	
	return ERRNOERROR;
}

////////////////////////////////////////////////////////////////////////////////////////////// 
//
// Must be called when D3D device is lost
//
////////////////////////////////////////////////////////////////////////////////////////////// 
void CMouse::OnLostDevice()
{
	CURSOR_ITER It;

	for (It = CursorsList.begin(); It != CursorsList.end(); It++)
	{
		(*It)->OnLostDevice();
	}
	
}

////////////////////////////////////////////////////////////////////////////////////////////// 
//
// Must be called when D3D device is reset
//
////////////////////////////////////////////////////////////////////////////////////////////// 
void CMouse::OnResetDevice()
{
	CURSOR_ITER It;

	for (It = CursorsList.begin(); It != CursorsList.end(); It++)
	{
		(*It)->OnResetDevice();
	}

}

////////////////////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////////////////////// 
