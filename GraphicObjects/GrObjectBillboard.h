/*
	GrObjectBillboard:	This class inherits from GrObjectBase class. It is used to render simple (possibly transparent)
						quad that is always turned to camera. This object is initialized only by texture filename,
						because it's always simple quad of specified size.
						SetRotation, SetWorlMatrix (and so on) calls are not allowed (the effect is not defined
						and can be undesirable).

	Creation Date:	2.5.2006
	Last Update:	16.5.2006
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GrObjectType.h"
#include "GrObjectBase.h"


namespace graphic
{

	
	class CGrObjectBillboard : public CGrObjectBase
	{

		// billboard vertex structure and FVF definition
		struct VERTEX
		{
			D3DXVECTOR3	pos;
			D3DXVECTOR2	texcoord;
		};

		static const DWORD VertexFVF = D3DFVF_XYZ | D3DFVF_TEX1;


	public:


		// constructors & destructors
							CGrObjectBillboard();
							~CGrObjectBillboard();
		
		// public methods
		inline virtual int	GetObjectType() { return GROBJECT_TYPE_BILLBOARD; }; // returns type of object

		HRESULT				Init(	BOOL Static, 
									CString texFileName,
									UINT LODcount );
		HRESULT				Init(	BOOL Static, 
									resManNS::RESOURCEID rid,
									UINT LODcount );
		void				Free();

		HRESULT				Render();


		// redefinition of standard methods working with object's world coordinates
		virtual void		InitialWorldPlacement( D3DXVECTOR3 & position, D3DXVECTOR3 & rotation, D3DXVECTOR3 & scale );
		virtual void		SetPosition( FLOAT x, FLOAT y, FLOAT z );
		virtual void		SetPosition( D3DXVECTOR3 & vec );
		virtual void		SetScaling( FLOAT x, FLOAT y );
		virtual void		SetScaling( FLOAT scale );
		virtual void		SetWidth( FLOAT width );
		virtual void		SetHeight( FLOAT height );

		static void			SetViewerPosition( D3DXVECTOR3 & eyePt );

		virtual void		CountWorldMat();


		virtual D3DXVECTOR3	GetPosition();
		virtual D3DXVECTOR3	GetScaling();
		virtual D3DXVECTOR2	GetScalingXY();
		virtual D3DXVECTOR3	GetRotation();
		virtual inline FLOAT GetWidth() { return Scaling.x; };
		virtual inline FLOAT GetHeight() { return Scaling.y; };

		virtual D3DXMATRIX	GetWorldMat();
		virtual const D3DXMATRIX *	GetWorldMatPtr();
	
	protected:
		// methods
		virtual void		PreInitThis();
		virtual void		PreInit();
		virtual HRESULT		InternalInit( BOOL Static, UINT LODcount );


	private:
		// private properties
		static IDirect3DVertexBuffer9 *	pVertexBuffer;
		static int						iReferences;
		static D3DXVECTOR3				ViewerPosition; // must be set to actual camera.eyePt vector
	
		resManNS::RESOURCEID			ridTexture;

		// private methods
		virtual void		SetRotation( FLOAT x, FLOAT y, FLOAT z ) {}; // can't be used on this object type
		virtual void		SetRotation( D3DXVECTOR3 & vec ) {}; // can't be used on this object type
		virtual void		SetScaling( FLOAT x, FLOAT y, FLOAT z ) {}; // can't be used on this object type
		virtual void		SetScaleMatrix( FLOAT x, FLOAT y, FLOAT z ) {}; // can't be used on this object type
		virtual void		SetScaleMatrix( D3DXVECTOR3 & vec ) {}; // can't be used on this object type
		virtual void		SetWorldMat( D3DMATRIX & matrix ) {}; // can't be used on this object type
		virtual void		SetScaling( D3DXVECTOR3 & vec ) {}; // can't be used on this object type

	} ;


	typedef CGrObjectBillboard	*LPGROBJECTBILLBOARD;


} // end of namespace