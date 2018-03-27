#include "stdafx.h"
#include "GrObjectBase.h"

using namespace graphic;


// Init static vars
LPDIRECT3DDEVICE9	CGrObjectBase::D3DDevice = 0;
resManNS::CResourceManager *	CGrObjectBase::ResourceManager = 0;
CShaders		*	CGrObjectBase::Shader = 0;
IDirect3DTexture9 * CGrObjectBase::InvisibleTexture = 0;
bool				CGrObjectBase::bVertexShader = false;
bool				CGrObjectBase::bPixelShader = false;
UINT				CGrObjectBase::MaxTextureStages; // count of texture stages that are available
UINT CGrObjectBase::TextureRotationHandle = 0;
UINT CGrObjectBase::RoadRotationHandle = 0;
UINT CGrObjectBase::TextureRotationMatHandle = 0;
UINT CGrObjectBase::RoadRotationMatHandle = 0;



//////////////////////////////////////////////////////////////////////////////////////////////
//
// static variables setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::StaticInit( LPDIRECT3DDEVICE9 D3DDevice,
								CShaders * Shader,
								resManNS::CResourceManager * ResManager,
								IDirect3DTexture9 * InvisibleTexture)
{
	CGrObjectBase::D3DDevice = D3DDevice;
	CGrObjectBase::Shader = Shader;
	CGrObjectBase::ResourceManager = ResManager;
	CGrObjectBase::InvisibleTexture = InvisibleTexture;

	Shader->BeginShaderConstantFetch();
	Shader->GetHandleByName("RotateTerrainTexture", &TextureRotationHandle);
	Shader->GetHandleByName("RotateRoadTexture", &RoadRotationHandle);
	Shader->GetHandleByName("TerrainTextureMat", &TextureRotationMatHandle);
	Shader->GetHandleByName("RoadTextureMat", &RoadRotationMatHandle);
	Shader->EndShaderConstantFetch();

};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectBase::CGrObjectBase()
{
	// sets all properties to default values
	PreInitThis();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectBase::~CGrObjectBase()
{
	Free();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// private method called by constructor
// this initializes all internal structures to default values
// !!! this doesn't call inherited PreInit() method
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::PreInitThis()
{
	BoundingBox = NULL;
	MapPosList.clear();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// private method called by initializator
// this initializes all internal structures to default values
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::PreInit()
{
	PreInitThis();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits some basic data that are used among all GrObject types
// also specifies whether this object is or is not static
// optional parameter fileName is specifying a name of file from which to load an object
// next optional parameter LODcount is specifying count of LOD's to load (user setting)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectBase::Init(	BOOL Static,
								CString fileName,
								UINT LODcount )
{
	PreInitThis();

	this->Static = Static;

	BoundingBox = new CBoundingBox;
	if ( !BoundingBox ) ERRORMSG( ERROUTOFMEMORY, "CGrObjectBase::Init()", "Unable to create bounding box object." );

	D3DXMatrixIdentity( &WorldMat );

	WorldMatNeedRecount = FALSE;
	WasMoved = FALSE;

	Position = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	Rotation = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	Scaling = D3DXVECTOR3( 1.0f, 1.0f, 1.0f );
	D3DXMatrixIdentity( &ScaleMatrix );

	ResetColorModifier();

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases all structures created by Init() or other calls
// this is called by destructor, but could be used separately for this object's reuse
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::Free()
{
	SAFE_DELETE( BoundingBox );
};


/*
//////////////////////////////////////////////////////////////////////////////////////////////
//
// tests for a collision, returns true if specified point collides with this object
// set precisionLevel to control max depth of tree level which will be used to test collision
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectBase::HitTest( FLOAT x, FLOAT y, FLOAT z, int precisionLevel )
{
	if ( !BoundingBox ) return FALSE;
	return BoundingBox->HitTest( x, y, z, precisionLevel );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// tests for a collision, returns true if specified point collides with this object
// set precisionLevel to control max depth of tree level which will be used to test collision
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectBase::HitTest( D3DXVECTOR3 point, int precisionLevel )
{
	if ( !BoundingBox ) return FALSE;
	return BoundingBox->HitTest( point, precisionLevel );
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////
//
// tests for a collision, returns true if specified object collides with this one
// set precisionLevel to control max depth of tree level which will be used to test collision
// also returns HitVectors for top level bounding boxes
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectBase::HitTest( CGrObjectBase *obj, int precisionLevel, D3DXVECTOR3 * HitVector1, D3DXVECTOR3 * HitVector2 )
{
	if ( !BoundingBox ) return FALSE;
	if ( !obj->BoundingBox ) return FALSE;
	return BoundingBox->HitTest( obj->BoundingBox, precisionLevel, HitVector1, HitVector2 );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets object's position
// SetPosition() method is considered deprecated. If possible, use SetWorldMat() instead,
//   but to do so, you have to undefine GROBJECT_USE_OLD_WORLDMAT_STYLE directive
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetPosition( FLOAT x, FLOAT y, FLOAT z )
{
#ifndef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetPosition() method could be used only in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetWorldMat() instead.") )
#endif
	this->Position = D3DXVECTOR3( x, y, z );
	WorldMatNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets object's position
// SetPosition() method is considered deprecated. If possible, use SetWorldMat() instead,
//   but to do so, you have to undefine GROBJECT_USE_OLD_WORLDMAT_STYLE directive
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetPosition( D3DXVECTOR3 & vec )
{
#ifndef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetPosition() method could be used only in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetWorldMat() instead.") )
#endif
	this->Position = vec;
	WorldMatNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets object's rotation
// SetRotation() method is considered deprecated. If possible, use SetWorldMat() instead,
//   but to do so, you have to undefine GROBJECT_USE_OLD_WORLDMAT_STYLE directive
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetRotation( FLOAT x, FLOAT y, FLOAT z )
{
#ifndef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetRotation() method could be used only in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetWorldMat() instead.") )
#endif
	this->Rotation = D3DXVECTOR3( x, y, z );
	WorldMatNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets object's rotation
// SetRotation() method is considered deprecated. If possible, use SetWorldMat() instead,
//   but to do so, you have to undefine GROBJECT_USE_OLD_WORLDMAT_STYLE directive
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetRotation( D3DXVECTOR3 & vec )
{
#ifndef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetRotation() method could be used only in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetWorldMat() instead.") )
#endif
	this->Rotation = vec;
	WorldMatNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets object's scale factor
// SetScaling() method is considered deprecated. If possible, use SetScaleMatrix() instead,
//   but to do so, you have to undefine GROBJECT_USE_OLD_WORLDMAT_STYLE directive
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetScaling( FLOAT x, FLOAT y, FLOAT z )
{
#ifndef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetScaling() method could be used only in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetWorldMat() instead.") )
#endif
	this->Scaling = D3DXVECTOR3( x, y, z );
	WorldMatNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets object's scale factor
// SetScaling() method is considered deprecated. If possible, use SetScaleMatrix() instead,
//   but to do so, you have to undefine GROBJECT_USE_OLD_WORLDMAT_STYLE directive
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetScaling( D3DXVECTOR3 & vec )
{
#ifndef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetScaling() method could be used only in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetWorldMat() instead.") )
#endif
	this->Scaling = vec;
	WorldMatNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets object's scale factor same in all directions
// SetScaling() method is considered deprecated. If possible, use SetScaleMatrix() instead,
//   but to do so, you have to undefine GROBJECT_USE_OLD_WORLDMAT_STYLE directive
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetScaling( FLOAT scale )
{
#ifndef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetScaling() method could be used only in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetWorldMat() instead.") )
#endif
	this->Scaling = D3DXVECTOR3( scale, scale, scale );
	WorldMatNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// recomputes object's scaling matrix to match newly set scale factors
//
// !!!!!IMPORTANT!!!!! this doesn't affect WorldMatrix and that means that any change made by 
//   this method won't be applied until you call SetWorldMat() method! (optimization)
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetScaleMatrix( FLOAT x, FLOAT y, FLOAT z )
{
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetScaleMatrix() method should not be used in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetScaling() instead.") )
#endif
	D3DXMatrixScaling( &ScaleMatrix, x, y, z );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// recomputes object's scaling matrix to match newly set scale factors
//
// !!!!!IMPORTANT!!!!! this doesn't affect WorldMatrix and that means that any change made by 
//   this method won't be applied until you call SetWorldMat() method! (optimization)
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetScaleMatrix( D3DXVECTOR3 & vec )
{
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetScaleMatrix() method should not be used in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetScaling() instead.") )
#endif
	D3DXMatrixScaling( &ScaleMatrix, vec.x, vec.y, vec.z );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes and sets the WorldMat using Position, Rotation and Scaling vector
//
// CountWorldMat() method is considered deprecated. If possible, use SetWorldMat() instead,
//   but to do so, you have to undefine GROBJECT_USE_OLD_WORLDMAT_STYLE directive
//
// Pay attention that this method can be called only once if the object is set to be static!
//   (This is for optimization.)
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::CountWorldMat()
{
#ifndef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("CountWorldMat() method could be used only in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use SetWorldMat() instead.") )
#endif
	D3DXMATRIX		mat;

	WorldMatNeedRecount = FALSE;

	if ( Static && WasMoved ) return;
	WasMoved = TRUE;

	D3DXMatrixScaling( &WorldMat, Scaling.x, Scaling.y, Scaling.z );
	D3DXMatrixRotationYawPitchRoll( &mat, Rotation.y, Rotation.x, Rotation.z );
	D3DXMatrixMultiply( &WorldMat, &WorldMat, &mat );
	D3DXMatrixTranslation( &mat, Position.x, Position.y, Position.z );
	D3DXMatrixMultiply( &WorldMat, &WorldMat, &mat );

	SetMapPositionAsWorldPosition();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the World Matrix and multiplies it with the Scaling Matrix
// Thus you can include the scaling matrix to the world matrix passed to this function
//   or you can use SetScaleMatrix to define scaling separately; anyway, this method computes
//   the resultant matrix and save it for future use
//
// to use this method the GROBJECT_USE_OLD_WORLDMAT_STYLE macro must not be defined
//
// Pay attention that this method can be called only once if the object is set to be static!
//   (This is for optimization.)
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetWorldMat( D3DMATRIX & matrix )
{	
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("SetWorldMat() method should not be used in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use Set[vector]() instead.") )
#endif
	if ( Static && WasMoved ) return;
	WasMoved = TRUE;

	WorldMat = ScaleMatrix * matrix;
	SetMapPositionAsWorldPosition();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns position of an object in worldspace coordinates
//
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 CGrObjectBase::GetPosition()
{ 
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	return D3DXVECTOR3( Position.x, Position.y, Position.z );
#else
	return D3DXVECTOR3( WorldMat._41, WorldMat._42, WorldMat._43 );
#endif
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns rotation of an object in worldspace coordinates
//
// GetRotation() method is considered deprecated. If possible, use GetWorldMat() instead,
//   but to do so, you have to undefine GROBJECT_USE_OLD_WORLDMAT_STYLE directive
//
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 CGrObjectBase::GetRotation()
{
#ifndef GROBJECT_USE_OLD_WORLDMAT_STYLE
#pragma message ( COMPILER_WARN("GetRotation() method could be used only in GROBJECT_USE_OLD_WORLDMAT_STYLE mode. Use GetWorldMat() instead.") )
#endif
	return Rotation;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns scaling vector of an object in worldspace coordinates
//
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 CGrObjectBase::GetScaling()
{ 
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	return D3DXVECTOR3( Scaling.x, Scaling.y, Scaling.z );
#else
	return D3DXVECTOR3( ScaleMatrix._11, ScaleMatrix._22, ScaleMatrix._33 );
#endif
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns object's world matrix, the matrix returned DOES include object scale!
//
//////////////////////////////////////////////////////////////////////////////////////////////
D3DXMATRIX CGrObjectBase::GetWorldMat()
{ 
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	if ( WorldMatNeedRecount ) CountWorldMat();
#endif

	return WorldMat;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// InitialWorldPlacement method can be used for intuitive specification of position and
//   rotation of the object without need of computing the world matrix manually.
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::InitialWorldPlacement(	D3DXVECTOR3 & position,
											D3DXVECTOR3 & rotation,
											D3DXVECTOR3 & scale )
{
	D3DXMATRIX		mat;

	if ( Static && WasMoved ) return;

	D3DXMatrixScaling( &WorldMat, scale.x, scale.y, scale.z );
	D3DXMatrixRotationYawPitchRoll( &mat, rotation.y, rotation.x, rotation.z );
	D3DXMatrixMultiply( &WorldMat, &WorldMat, &mat );
	D3DXMatrixTranslation( &mat, position.x, position.y, position.z );
	D3DXMatrixMultiply( &WorldMat, &WorldMat, &mat );

	SetScaleMatrix(scale);
	SetMapPositionAsWorldPosition();

	Position = position;
	Rotation = rotation;
	Scaling = scale;

	WasMoved = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a map position to the list
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::AddMapPosition( int x, int z )
{
	MAPPOSITION		pos;

	pos.PosX = x;
	pos.PosZ = z;

	MapPosList.push_back( pos );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a map position to the list
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::AddMapPosition( MAPPOSITION *pos )
{
	MapPosList.push_back( *pos );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// clears the MapPosList and adds a single map-position record to it that is computed on the
//   base of object's real world position
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetMapPositionAsWorldPosition()
{
	MAPPOSITION		pos;
	
	if (! IsStatic() )
	{
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	pos.PosX = (int) ( Position.x / TERRAIN_PLATE_WIDTH );
	pos.PosZ = (int) ( Position.z / TERRAIN_PLATE_WIDTH );
#else
	pos.PosX = (int) ( WorldMat._41 / TERRAIN_PLATE_WIDTH );
	pos.PosZ = (int) ( WorldMat._43 / TERRAIN_PLATE_WIDTH );
#endif

	MapPosList.clear();
	MapPosList.push_back( pos );
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if this object interferes to specified map position
// that is when one of the object's map positions are same as the specified one
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectBase::InterferesToField( int x, int z )
{
	MAPPOS_ITER		pIt;
	
	for ( pIt = MapPosList.begin(); pIt != MapPosList.end(); pIt ++ )
		if ( pIt->PosX == x && pIt->PosZ == z ) return TRUE;

	return FALSE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if this object interferes to specified map position
// that is when one of the object's map positions are same as the specified one
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectBase::InterferesToField( MAPPOSITION *pos )
{
	MAPPOS_ITER		pIt;
	
	for ( pIt = MapPosList.begin(); pIt != MapPosList.end(); pIt ++ )
		if ( pIt->PosX == pos->PosX && pIt->PosZ == pos->PosZ ) return TRUE;

	return FALSE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if this object interferes to the specified rectangular location
// that is when at least one of the object's map positions are inside the specified
// rectangular location
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectBase::InterferesToRectangularArea( int x1, int z1, int x2, int z2 )
{
	MAPPOS_ITER		pIt;
	int				_x1, _x2, _z1, _z2;
	
	_x1 = min( x1, x2 );
	_x2 = max( x1, x2 );
	_z1 = min( z1, z2 );
	_z2 = max( z1, z2 );

	for ( pIt = MapPosList.begin(); pIt != MapPosList.end(); pIt ++ )
		if ( pIt->PosX >= _x1 && pIt->PosX <= _x2 && pIt->PosZ >= _z1 && pIt->PosZ <= _z2 ) return TRUE;

	return FALSE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if this object interferes to the specified rectangular location
// that is when at least one of the object's map positions are inside the specified
// square location
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectBase::InterferesToSquareArea( int x, int z, int range )
{
	MAPPOS_ITER		pIt;

	for ( pIt = MapPosList.begin(); pIt != MapPosList.end(); pIt ++ )
		if ( ( pIt->PosX >= x - range ) && ( pIt->PosX <= x + range ) 
			&& ( pIt->PosZ >= z - range ) && ( pIt->PosZ <= z + range ) ) return TRUE;

	return FALSE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if this object interferes to the specified rectangular location
// that is when at least one of the object's map positions are inside the specified
// square location
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectBase::InterferesToSquareArea( MAPPOSITION *pos, int range )
{
	MAPPOS_ITER		pIt;

	for ( pIt = MapPosList.begin(); pIt != MapPosList.end(); pIt ++ )
		if ( ( pIt->PosX >= pos->PosX - range ) && ( pIt->PosX <= pos->PosX + range ) 
			&& ( pIt->PosZ >= pos->PosZ - range ) && ( pIt->PosZ <= pos->PosZ + range ) ) return TRUE;

	return FALSE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if the object interferes to one of the fields in specified list
// that is when at least one of the object's map positions is same as one of the
// map positions stored in specified MAPPOSITIONLIST
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectBase::InterferesToMapPositionList( MAPPOSITIONLIST *mapPosList )
{
	MAPPOS_ITER		pIt1, pIt2;

	for ( pIt1 = MapPosList.begin(); pIt1 != MapPosList.end(); pIt1++ )
		for ( pIt2 = mapPosList->begin(); pIt2 != mapPosList->end(); pIt2++ )
			if ( pIt1->PosX == pIt2->PosX && pIt1->PosZ == pIt2->PosZ ) return TRUE;

	return FALSE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the color modifier for this particular object (for example swap input color channels
//   or make the object brighter)
// be aware of that child objects does not have to implement color modifications (depending 
//   on which shader do they use)
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectBase::SetColorModifier( COLORMODIFIER_ENUM mod )
{
	D3DXMatrixIdentity( &ColorModifier );

	switch ( mod ) {
		case CME_NONE:
			break;
		case CME_REDONLY:
			ColorModifier._22 = 0;
			ColorModifier._33 = 0;
			break;
		case CME_GREENONLY:
			ColorModifier._11 = 0;
			ColorModifier._33 = 0;
			break;
		case CME_BLUEONLY:
			ColorModifier._11 = 0;
			ColorModifier._22 = 0;
			break;
		case CME_NORED:
			ColorModifier._11 = 0;
			break;
		case CME_NOGREEN:
			ColorModifier._22 = 0;
			break;
		case CME_NOBLUE:
			ColorModifier._33 = 0;
			break;
		case CME_DARKER:
			ColorModifier._11 = 0.5f;
			ColorModifier._22 = 0.5f;
			ColorModifier._33 = 0.5f;
			break;
		case CME_LIGHTER:
			ColorModifier._11 = 2;
			ColorModifier._22 = 2;
			ColorModifier._33 = 2;
			break;
		case CME_ROTATEGBR:
			ColorModifier._11 = 0;
			ColorModifier._22 = 0;
			ColorModifier._33 = 0;
			ColorModifier._21 = 1;
			ColorModifier._32 = 1;
			ColorModifier._13 = 1;
			break;
		case CME_ROTATEBRG:
			ColorModifier._11 = 0;
			ColorModifier._22 = 0;
			ColorModifier._33 = 0;
			ColorModifier._31 = 1;
			ColorModifier._12 = 1;
			ColorModifier._23 = 1;
			break;
		case CME_GREYSCALE:
			ColorModifier._11 = 0.299f;
			ColorModifier._21 = 0.587f;
			ColorModifier._31 = 0.114f;
			ColorModifier._12 = 0.299f;
			ColorModifier._22 = 0.587f;
			ColorModifier._32 = 0.114f;
			ColorModifier._13 = 0.299f;
			ColorModifier._23 = 0.587f;
			ColorModifier._33 = 0.114f;
			break;
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
