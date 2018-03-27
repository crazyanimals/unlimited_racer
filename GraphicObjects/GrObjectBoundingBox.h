/*
	GrObjectBoundingBox:	This class inherits from GrObjectBase class. It is used to load and work with BoundingBox.
							It contains some extra methods to set up rendering options.
	Creation Date:	1.8.2004
	Last Update:	17.12.2005
	Author:			Roman Margold
*/

#pragma once

#define DEFINED_GROBJECTBOUNDINGBOX

#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GrObjectType.h"
#include "BoundingBox.h"
#include "GrObjectBase.h"


namespace graphic
{

	class CGrObjectBoundingBox : public CGrObjectBase
	{
	public:
		// TYPES
		
		// vertex declaration
		struct VERTEX
		{
			float		x, y, z;
		} ;

		static const DWORD	VertexFVF = ( D3DFVF_XYZ );

		// bounding box node declaration
		struct BBSINGLENODE
		{
			UINT				Level;
			D3DXMATRIX			Matrix;
			CBoundingBox	 *	pBox;
		} ;


		// constructors & destructors
							CGrObjectBoundingBox();
							~CGrObjectBoundingBox();
		
		// methods
		inline virtual int	GetObjectType() { return GROBJECT_TYPE_BOUNDINGBOX; }; // returns type of object
		HRESULT				Init(	BOOL Static, 
									CString fileName,
									UINT LODcount ); // loads a bounding box from a file
		HRESULT				ReInit( LPCTSTR fileName ); // loads a bounding box hierarchy from the file and creates buffers for rendering
		HRESULT				Render(); // sets all needed states and renders the object, then restores all saved state


	private:
		// variables
		LPDIRECT3DVERTEXBUFFER9		VertexBuffer;
		LPDIRECT3DINDEXBUFFER9 		IndexBuffer;
		std::vector<int>			NodesCountOnEachLevel; // list of counts of nodes on each tree-level
		UINT						uiNodeLevelsCount; // count of tree-levels
		UINT						uiNodesCount; // total count of nodes

		// methods
		HRESULT				LoadFromFile( LPCTSTR fileName );
		HRESULT				InitBuffers(); // creates vertex and index buffers and fills them with a proper bounding box vertices data
	} ;


	typedef CGrObjectBoundingBox	*LPGROBJECTBOUNDINGBOX;


} // end of namespace