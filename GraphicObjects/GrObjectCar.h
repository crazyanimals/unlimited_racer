/*
	GrObjectCar:	This class inherits from CGrObjectMesh. There's only small differences between these two
					classes.

					Read all comments in GrObjectBase.h file before using this!!!

	Creation Date:	13.5.2006
	Last Update:	13.5.2006
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "GrObjectType.h"
#include "BoundingBox.h"
#include "GrObjectBase.h"
#include "GrObjectMesh.h"
#include "ShadowVolume.h"
#include "Light.h"


namespace graphic
{

	
	class CGrObjectCar : public CGrObjectMesh
	{
	public:
		// public methods
		inline virtual int	GetObjectType() { return GROBJECT_TYPE_CAR; }; // returns type of object

		HRESULT				RenderSpecular();

	} ;


	typedef CGrObjectCar	*	LPGROBJECTCAR;


} // end of namespace