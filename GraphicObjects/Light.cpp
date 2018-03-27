#include "stdafx.h"
#include "Light.h"


using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all colors at once
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetColor( D3DXVECTOR4 *col )
{
	Diffuse.r = col->x;
	Diffuse.g = col->y;
	Diffuse.b = col->z;
	Diffuse.a = col->w;

	Specular.r = col->x;
	Specular.g = col->y;
	Specular.b = col->z;
	Specular.a = col->w;

	Ambient.r = col->x;
	Ambient.g = col->y;
	Ambient.b = col->z;
	Ambient.a = col->w;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all colors at once
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetColor( D3DCOLORVALUE *col )
{
	Diffuse.r = col->r;
	Diffuse.g = col->g;
	Diffuse.b = col->b;
	Diffuse.a = col->a;

	Specular.r = col->r;
	Specular.g = col->g;
	Specular.b = col->b;
	Specular.a = col->a;

	Ambient.r = col->r;
	Ambient.g = col->g;
	Ambient.b = col->b;
	Ambient.a = col->a;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all colors at once
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetColor( D3DCOLOR col )
{
	Diffuse.r = (float) ( ( col & 0x00ff0000 ) >> 16 ) / 255.0f;
	Diffuse.g = (float) ( ( col & 0x0000ff00 ) >> 8 ) / 255.0f;
	Diffuse.b = (float) ( ( col & 0x000000ff ) ) / 255.0f;
	Diffuse.a = (float) ( ( col & 0xff000000 ) >> 24 ) / 255.0f;

	Specular = Ambient = Diffuse;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all colors at once
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetColor( FLOAT r, FLOAT g, FLOAT b, FLOAT a )
{
	Diffuse.r = r;
	Diffuse.g = g;
	Diffuse.b = b;
	Diffuse.a = a;

	Specular.r = r;
	Specular.g = g;
	Specular.b = b;
	Specular.a = a;

	Ambient.r = 0;
	Ambient.g = 0;
	Ambient.b = 0;
	Ambient.a = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all light's colors with own values
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetColor( D3DCOLOR ambient, D3DCOLOR diffuse, D3DCOLOR specular )
{
	Ambient.r = (float) ( ( ambient & 0x00ff0000 ) >> 16 ) / 255.0f;
	Ambient.g = (float) ( ( ambient & 0x0000ff00 ) >> 8 ) / 255.0f;
	Ambient.b = (float) ( ( ambient & 0x000000ff ) ) / 255.0f;
	Ambient.a = (float) ( ( ambient & 0xff000000 ) >> 24 ) / 255.0f;

	Diffuse.r = (float) ( ( diffuse & 0x00ff0000 ) >> 16 ) / 255.0f;
	Diffuse.g = (float) ( ( diffuse & 0x0000ff00 ) >> 8 ) / 255.0f;
	Diffuse.b = (float) ( ( diffuse & 0x000000ff ) ) / 255.0f;
	Diffuse.a = (float) ( ( diffuse & 0xff000000 ) >> 24 ) / 255.0f;

	Specular.r = (float) ( ( specular & 0x00ff0000 ) >> 16 ) / 255.0f;
	Specular.g = (float) ( ( specular & 0x0000ff00 ) >> 8 ) / 255.0f;
	Specular.b = (float) ( ( specular & 0x000000ff ) ) / 255.0f;
	Specular.a = (float) ( ( specular & 0xff000000 ) >> 24 ) / 255.0f;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// use this if you want to set Theta in degrees
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetTheta( FLOAT angle )
{
	Theta = (angle / 180.0f) * D3DX_PI;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// use this if you want to set Phi in degrees
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetPhi( FLOAT angle )
{
	Phi = (angle / 180.0f) * D3DX_PI;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all three relative parameters at once
// use NULL for parameters that shouldn't change
// after settings these parameters, PrecomputePosition() method is called that computes
//  position and direction vectors on the base of relative params
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetRelativeSystem( D3DXMATRIX * mat, D3DXVECTOR3 * pos, D3DXVECTOR3 * dir )
{
	if ( mat ) RelPointWorldMatrix = *mat;
	if ( pos ) RelativePosition = *pos;
	if ( dir ) RelativeDirection = *dir;
	
	PrecomputePosition();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes position and direction vectors on the base of relative params
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::PrecomputePosition()
{
	D3DXVECTOR3		vec;

	D3DXVec3TransformCoord( &Position, &RelativePosition, &RelPointWorldMatrix );
	vec = RelativeDirection + RelativePosition;
	D3DXVec3TransformCoord( &Direction, &vec, &RelPointWorldMatrix );
	Direction -= Position;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets standard light settings, type is specifying the light type
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetStandardLight( UINT type )
{
	ZeroMemory( this, sizeof( CLight ) );

	Enabled = TRUE;
	SetColor( 1.5f, 1.5f, 1.5f, 1.0f );
	Attenuation1 = 0.002f;
	Falloff = 1.0f;
	Theta = D3DX_PI / 6;
	Phi = D3DX_PI / 2.75f;
	Type = type;

	switch ( type )
	{
	case LT_DIRECTIONAL:
	case LT_FOGLAMP:
	case LT_AMBIENT:
		Range = 2000000.0f; // unranged
		break;
	case LT_LAMP:
	case LT_POINT:
		Range = 1200.0f;
		break;
	case LT_CARREFLECTOR:
		Range = 2000.0f;
		break;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// fills the standard D3DLIGHT9 structure with values appropriate to this light object
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::GetD3DLight( D3DLIGHT9 *D3DLight )
{
	D3DLight->Ambient = this->Ambient;
	D3DLight->Attenuation0 = this->Attenuation0;
	D3DLight->Attenuation1 = this->Attenuation1;
	D3DLight->Attenuation2 = this->Attenuation2;
	D3DLight->Diffuse = this->Diffuse;
	D3DLight->Direction = this->Direction;
	D3DLight->Falloff = this->Falloff;
	D3DLight->Phi = this->Phi;
	D3DLight->Position = this->Position;
	D3DLight->Range = this->Range;
	D3DLight->Specular = this->Specular;
	D3DLight->Theta = this->Theta;

	switch ( this->Type )
	{
	case LT_DIRECTIONAL:
		D3DLight->Type = D3DLIGHT_DIRECTIONAL;
		break;
	case LT_CARREFLECTOR:
	case LT_LAMP:
		D3DLight->Type = D3DLIGHT_SPOT;
		break;
	case LT_POINT:
		D3DLight->Type = D3DLIGHT_POINT;
		break;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes the map-position coordinates from the light's real world position
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::SetMapPositionAsWorldPosition()
{
	MapPosition.PosX = (int) ( Position.x / TERRAIN_PLATE_WIDTH );
	MapPosition.PosZ = (int) ( Position.z / TERRAIN_PLATE_WIDTH );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns all four color components computed from UINT input color
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CLight::ComputeD3DColorValue( D3DCOLORVALUE * newColorValue, D3DCOLOR col )
{
	newColorValue->r = (float) ( ( col & 0x00ff0000 ) >> 16 ) / 255.0f;
	newColorValue->g = (float) ( ( col & 0x0000ff00 ) >> 8 ) / 255.0f;
	newColorValue->b = (float) ( ( col & 0x000000ff ) ) / 255.0f;
	newColorValue->a = (float) ( ( col & 0xff000000 ) >> 24 ) / 255.0f;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
