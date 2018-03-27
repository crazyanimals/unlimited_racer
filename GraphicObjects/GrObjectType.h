/*
	GrObjectType:	Contains declarations of macros defining graphic objects and retyping macro.
	Creation Date:	26.10.2003
	Last Update:	13.5.2006
	Author:			Roman Margold
*/


#pragma once


#include "stdafx.h"


// definitions of graphic object type macros
#define GROBJECT_TYPE_BASE			1
#define GROBJECT_TYPE_MESH			2
#define GROBJECT_TYPE_TERRAINPLATE	3
#define GROBJECT_TYPE_TREE			4
#define GROBJECT_TYPE_TERRAIN		5
#define GROBJECT_TYPE_SKYBOX		6
#define GROBJECT_TYPE_BOUNDINGBOX	7
#define GROBJECT_TYPE_GRASS			8
#define GROBJECT_TYPE_BILLBOARD		9
#define GROBJECT_TYPE_CAR			10


// retyping macro declares a new object of type specified within the old object autoindentifying information and sets a pointer to it
#define GROBJECT_RETYPE(old,nw) { switch ( (old)->GrObjectType() ) { \
									case GROBJECT_TYPE_BASE:			CGrObjectBase			* nw = (CGrObjectBase*) (old); break; \
									case GROBJECT_TYPE_MESH:			CGrObjectMesh			* nw = (CGrObjectBase*) (old); break; \
									case GROBJECT_TYPE_TERRAINPLATE:	CGrObjectTerrainPlate	* nw = (CGrObjectBase*) (old); break; \
									case GROBJECT_TYPE_TERRAIN:			CGrObjectTerrain		* nw = (CGrObjectBase*) (old); break; \
									case GROBJECT_TYPE_TREE:			CGrObjectTree			* nw = (CGrObjectBase*) (old); break; \
									case GROBJECT_TYPE_SKYBOX:			CGrObjectSkyBox			* nw = (CGrObjectBase*) (old); break; \
									case GROBJECT_TYPE_BOUNDINGBOX:		CGrObjectBoundingBox	* nw = (CGrObjectBase*) (old); break; \
									case GROBJECT_TYPE_GRASS:			CGrObjectGrass			* nw = (CGrObjectBase*) (old); break; \
									case GROBJECT_TYPE_BILLBOARD:		CGrObjectBillboard		* nw = (CGrObjectBase*) (old); break; \
									case GROBJECT_TYPE_CAR:				CGrObjectCar			* nw = (CGrObjectBase*) (old); break; \
								}}