/*
	GrObjectBase:	This class is a base class for all graphic objects. It provides methods to transform and render it
					and to count bounding box around that object and make tests to collisions with other ones.
					Also has classification to static or dynamic object group.		
					
					Important: Static object can't move more than once per its lifetime.
					
					There are two modes of positioning the object into scene. The old mode offers easy to use
					functions like SetPosition, SetRotation etc. This mode is supposed to be used for user
					input - testing purposes etc. To use this mode you have to define GROBJECT_USE_OLD_WORLDMAT_STYLE
					directive.
					Second mode offers only SetWorldMat and GetWorldMat functions that are supposed to be used
					for direct input (for example using physical engine that precomputes the matrix).
					The GROBJECT_USE_OLD_WORLDMAT_STYLE macro must not be defined so you can use this mode.
					You can even use SetScaleMatrix method to be able to have world matrix separated
					from scaling. InitialWorldPlacement can be used for initial specification of pos. vectors.

	Creation Date:	25.10.2003
	Last Update:	26.2.2007
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GrObjectType.h"
#include "BoundingBox.h"
#include "..\Graphic\Shaders.h"


#define MAX_TEXTURE_COUNT		8 // maximal count of texture stages used in single draw


namespace graphic
{

	struct MAPPOSITION
	{
		int			PosX, PosZ;
	} ;

	typedef std::vector<MAPPOSITION>			MAPPOSITIONLIST;
	typedef std::vector<MAPPOSITION>::iterator	MAPPOS_ITER;



	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// base class for all graphic objects
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CGrObjectBase
	{
	friend class CShadowVolume;
	
	public:
		enum COLORMODIFIER_ENUM
		{
			CME_NONE		= 0x00000000,
			CME_GREYSCALE	= 0x00000001,
			CME_REDONLY		= 0x00000002,
			CME_GREENONLY	= 0x00000003,
			CME_BLUEONLY	= 0x00000004,
			CME_NORED		= 0x00000005,
			CME_NOGREEN		= 0x00000006,
			CME_NOBLUE		= 0x00000007,
			CME_DARKER		= 0x00000008,
			CME_LIGHTER		= 0x00000009,
			CME_ROTATEGBR	= 0x0000000a,
			CME_ROTATEBRG	= 0x0000000b,
			CME_FORCEDWORD	= 0xffffffff
		};


		// public variables
		static UINT			MaxTextureStages; // count of texture stages that are available

		static bool			bVertexShader; // Rendering through vertex shader
		static bool			bPixelShader; // Rendering through pixel shader


		// constructors & destructors
							CGrObjectBase();
		virtual				~CGrObjectBase();
		
		// basic methods
		virtual int			GetObjectType() { return GROBJECT_TYPE_BASE; }; // returns type of object
		virtual HRESULT		Init(	BOOL Static,
									CString fileName = "", 
									UINT LODcount = 0 );
		
		static void			StaticInit(	LPDIRECT3DDEVICE9 D3DDevice,
										CShaders * Shader,
										resManNS::CResourceManager * ResManager,
										IDirect3DTexture9 * InvisibleTexture);
		
		virtual void		Free();


		virtual	HRESULT		Render() = 0; // sets all needed states and renders the object, then restores all saved state
		virtual BOOL		IsStatic() { return Static; }; // returns true if this object was initialized as static


		// transformation methods
		virtual void		InitialWorldPlacement( D3DXVECTOR3 & position, D3DXVECTOR3 & rotation, D3DXVECTOR3 & scale );
		virtual void		SetPosition( FLOAT x, FLOAT y, FLOAT z );
		virtual void		SetPosition( D3DXVECTOR3 & vec );
		virtual void		SetRotation( FLOAT x, FLOAT y, FLOAT z );
		virtual void		SetRotation( D3DXVECTOR3 & vec );
		virtual void		SetScaling( FLOAT x, FLOAT y, FLOAT z );
		virtual void		SetScaling( D3DXVECTOR3 & vec );
		virtual void		SetScaling( FLOAT scale );
		virtual void		SetScaleMatrix( FLOAT x, FLOAT y, FLOAT z );
		virtual void		SetScaleMatrix( D3DXVECTOR3 & vec );
		virtual void		SetWorldMat( D3DMATRIX & matrix );

		virtual void		CountWorldMat();


		virtual D3DXVECTOR3	GetPosition();
		virtual D3DXVECTOR3	GetScaling();
		virtual D3DXVECTOR3	GetRotation();

		virtual D3DXMATRIX	GetWorldMat();
		virtual const D3DXMATRIX *	GetWorldMatPtr() { return &WorldMat; }; // returns a pointer to world matrix

		// SetColorModifier method sets the color modifier matrix; default value is identity matrix
		virtual void		SetColorModifier( D3DXMATRIX * mat ) { ColorModifier = *mat; };
		virtual void		SetColorModifier( COLORMODIFIER_ENUM mod );

		virtual void		GetColorModifier( D3DXMATRIX * mat ) { *mat = ColorModifier; };
		virtual D3DXMATRIX * GetColorModifier() { return &ColorModifier; };
		
		virtual void		ResetColorModifier() { D3DXMatrixIdentity( &ColorModifier); };


		// map position methods
		virtual void		AddMapPosition( int x, int z ); // adds a map position to the list
		virtual void		AddMapPosition( MAPPOSITION *pos ); // adds a map position to the list
		virtual void		SetMapPositionAsWorldPosition(); // clears the MapPosList and adds a position to it computed from object's world position
		virtual void		ClearMapPosList() { MapPosList.clear(); }; // clears the list of map position coordinates
		virtual MAPPOSITIONLIST	*	GetMapPosList() { return &MapPosList; }; // returns a pointer to MAPPOSITIONLIST property

		virtual BOOL		InterferesToField( int x, int z ); // returns true if this object interferes to specified map position
		virtual BOOL		InterferesToField( MAPPOSITION *pos ); // returns true if this object interferes to specified map position
		virtual BOOL		InterferesToRectangularArea( int x1, int z1, int x2, int z2 ); // returns true if this object interferes to the specified rectangular location
		virtual BOOL 		InterferesToSquareArea( int x, int z, int range ); // returns true if this object interferes to the specified square location
		virtual BOOL		InterferesToSquareArea( MAPPOSITION *pos, int range ); // returns true if this object interferes to the specified square location
		virtual BOOL		InterferesToMapPositionList( MAPPOSITIONLIST *mapPosList ); // returns true if the object interferes to one of the fields in specified list


		// hit test methods
		virtual D3DXVECTOR3	GetLowerLeftCorner() { return BoundingBox->GetLowerLeftCorner(); }; // returns position of lower left corner of bounding box
		virtual D3DXVECTOR3	GetUpperRightCorner() { return BoundingBox->GetUpperRightCorner(); }; // returns position of upper right corner of bounding box

		// HitTest tests for a collision, returns true if specified object (3D point or another GrObject) collides with this one;
		// set precisionLevel to control max depth of tree level which will be used to test collision
		//virtual BOOL		HitTest( FLOAT x, FLOAT y, FLOAT z, int precisionLevel ); // returns true if a [x,y,z] point is inside of object's bounding box
		//virtual BOOL		HitTest( D3DXVECTOR3 point, int precisionLevel ); // returns true if a point is inside of object's bounding box
		// next HitTest returns true if an object's bounding box collides with specified one
		// also returns HitVectors for top level bounding boxes
		virtual BOOL		HitTest( CGrObjectBase *obj, int precisionLevel, D3DXVECTOR3 * HitVector1, D3DXVECTOR3 * HitVector2 );
		               
		
	protected:
		// variables
		static LPDIRECT3DDEVICE9	D3DDevice; // pointer to Direct3DDevice object
		static resManNS::CResourceManager	*	ResourceManager; // stores pointer to CResourceManager object, obtainde when calling Init method
 		static CShaders			*	Shader; // Shader setting class
		static IDirect3DTexture9 *	InvisibleTexture; // Invisible texture

		// shader handles
		static UINT			TextureRotationHandle;
		static UINT			RoadRotationHandle;
		static UINT			TextureRotationMatHandle;
		static UINT			RoadRotationMatHandle;

		D3DXMATRIX			WorldMat; // matrix that places object into world
		D3DXMATRIX			ScaleMatrix; // scaling matrix - its used when you enter world position without scale
		D3DXVECTOR3			Position;
		D3DXVECTOR3			Rotation;
		D3DXVECTOR3			Scaling;
		CBoundingBox	*	BoundingBox; // bounding box surrounding the object (world space)

		D3DXMATRIX			ColorModifier; /* defines the modifier for all color channels; can be even used for channel mixing;
											however, not all child objects have this object implemented */

		BOOL				WorldMatNeedRecount; // true if WorldMat variable needs recounting because of some applied transformations
		BOOL				WasMoved; // this is used to determine if this object can move one more time when it is static

		BOOL				Static; // specifies whether this object is static or dynamic (static object cant move)
		MAPPOSITIONLIST		MapPosList; // list of coordinates of map field's that are interfered by this object


		// methods
		virtual void		PreInitThis();
		virtual void		PreInit();

	} ;

	
	typedef CGrObjectBase	*LPGROBJECTBASE;

} // end of namespace