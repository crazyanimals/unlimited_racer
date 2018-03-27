/*
	Scene:			This class is representing a light.
					It contains light's position and direction vectors and information about its type.
	Creation Date:	21.4.2004
	Last Update:	27.5.2004
	Author:			Roman Margold
*/

#pragma once

#define DEFINED_LIGHT

#include "stdafx.h"
#include "GrObjectBase.h"
#include "..\Globals\GlobalsIncludes.h"


// light-type macros
#define LT_UNDEFINED			0
#define LT_CARREFLECTOR			1 // proper reflector (=spot light)	with flare
#define LT_LAMP					2 // simpled reflector with attenuation based on map fields
#define LT_DIRECTIONAL			3 // any directional light
#define LT_FOGLAMP				4 // is rendered only as a sprite
#define LT_AMBIENT				5 // ambient light used to luminate current scene
#define LT_POINT				6 // classic point light - emits light to all directions


namespace graphic
{

	class CLight
	{
	public:
		// properties
		UINT				Type; // one of light-type macros
		BOOL				Enabled; // must by true so the light would be used by rendering

		LPGROBJECTBASE		pEmitor; // set this to a proper pointer to a object that emmits this light or to NULL
		MAPPOSITION			MapPosition; // coordinates in the map (used for attenuated lights)
		
		D3DXVECTOR3			Position; // final precomputed position vector in world space coordinates
		D3DXVECTOR3			Direction; // final precomputed direction vector in world space coordinates
		D3DXVECTOR3			Scale; // scaling factor used mainly by LT_FOGLAMP lights (sprites)
		
		D3DCOLORVALUE		Diffuse;
		D3DCOLORVALUE		Specular;
		D3DCOLORVALUE		Ambient;
		
		FLOAT				Range; // distance beyond which the light has no effect; this does not affect directional lights
		FLOAT				Falloff; // decrease in illumination between a spotlight's inner cone - best to set this to 1.0
		FLOAT				Theta; // angle, in radians, of a spotlight's inner cone
		FLOAT				Phi; // angle, in radians, defining the outer edge of the spotlight's outer cone

		FLOAT				Attenuation0; // value specifying how the light intensity changes over distance
		FLOAT				Attenuation1; // attenuation values are ignored for directional lights
		FLOAT				Attenuation2; // the attenuation equation is  Atten = 1 / ( att0 + att1 * d + att2 * d^2 )
		LPDIRECT3DTEXTURE9  LightMap; // Light light map
		
		// methods
		void				SetColor( D3DXVECTOR4 *col ); // sets all colors at once
		void				SetColor( D3DCOLORVALUE *col); // sets all colors at once
		void				SetColor( FLOAT r, FLOAT g, FLOAT b, FLOAT a ); // sets all colors at once
		void				SetColor( D3DCOLOR col ); // sets all colors at once using UINT as the input color - not recommended!
		void				SetColor( D3DCOLOR ambient, D3DCOLOR diffuse, D3DCOLOR specular ); // sets all light's colors with own values

		void				SetTheta( FLOAT angle ); // use this if you want to set Theta in degrees
		void				SetPhi( FLOAT angle ); // use this if you want to set Phi in degrees

		void				SetRelativeSystem( D3DXMATRIX * mat, D3DXVECTOR3 * pos, D3DXVECTOR3 * dir ); // sets all three relative parameters at once

		void				SetStandardLight( UINT type ); // sets standard light settings, type is specifying the light type
		void				GetD3DLight( D3DLIGHT9 *D3DLight ); // fills the standard D3DLIGHT9 structure with values appropriate to this light object

		void				SetMapPositionAsWorldPosition(); // computes a map position coordinates from light's world position
		
		static void			ComputeD3DColorValue( D3DCOLORVALUE * newColorValue, D3DCOLOR col ); // returns all four color components computed from UINT input color

	private:
		D3DXVECTOR3			RelativePosition; // position related to some RelativePoint
		D3DXVECTOR3			RelativeDirection; // direction related to some RelativePoint
		D3DXMATRIX			RelPointWorldMatrix; // RelativePoint world matrix
		
		void				PrecomputePosition(); // precomputes position based on relative parameters

	} ;


	typedef CLight			*LPLIGHT;


} // end of namespace graphic

