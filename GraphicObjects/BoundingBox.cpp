/*
BoundingBox.cpp
Implementation of bounding boxes hierarchy
Last changed: 24.11.2003
Written by: Pavel Celba
*/

#include "BoundingBox.h"

using namespace graphic;

namespace graphic
{
	// provides strict week ordering of D3DXVECTOR3
	struct LessD3DXVECTOR3
	{
		bool operator()(const D3DXVECTOR3 & Vector1, const D3DXVECTOR3 & Vector2) const
		{
			if (Vector1.x < Vector2.x)
				return true;
			if (Vector1.x > Vector2.x)
				return false;
			if (Vector1.y < Vector2.y)
				return true;
			if (Vector1.y > Vector2.y)
				return false;
			if (Vector1.z < Vector2.z)
				return true;
			
			return false;
		}
	};
}

// returns true, if two hierarchies of bounding boxes intersect, returns intersect vectors in HItVectors
// the testing is provided by FastTest, PreciseTesting is done only, when one box has ForcePrecise = TRUE
BOOL CBoundingBox::HitTest(CBoundingBox * Box, int PrecisionLevel, D3DXVECTOR3 * HitVector1, D3DXVECTOR3 * HitVector2)
{
	BOOL Inside;

	if (Box->GetForcePrecision() || GetForcePrecision() )
	{
		Inside = PreciseBoxHit(this, Box);
	}
	else
		Inside = BoxHit(this, Box);

	if (Inside && PrecisionLevel != 0)
	{	
		if (pNodes.size() > 0 && Box->pNodes.size() > 0)
		{
			for (unsigned int i = 0; i < pNodes.size(); i++)
				for (unsigned int j = 0; j < Box->pNodes.size(); j++)
					if (pNodes[i]->HitTest(Box->pNodes[j], --PrecisionLevel, HitVector1, HitVector2))
					{
						return true;
					}
			return false;
		}
		else if (pNodes.size() > 0)
		{
			for (unsigned int i = 0; i < pNodes.size(); i++)
				if (pNodes[i]->HitTest(Box, --PrecisionLevel, HitVector1, HitVector2))
					return true;
			return false;
		}
		else if (Box->pNodes.size() > 0)
		{
			for (unsigned int j = 0; j < Box->pNodes.size(); j++)
				if (HitTest(Box->pNodes[j], --PrecisionLevel, HitVector1, HitVector2))
					return true;
			return false;
		}
	}	
	
	// Computes precisely hit point
	if (Inside)
	{
		PreciseBoxHit(this, Box, HitVector1, HitVector2);
	}

	return Inside;
}

// Determines, whether the Point is in Bounding box
BOOL CBoundingBox::FastTest(D3DXVECTOR3 Point, int PrecisionLevel)
{
	D3DXVECTOR3 TempVec;
	bool Inside = false;

	// Test, if transformation InvMatrix is computed
	if (Changed) Recompute();

	// Transform the point into bounding box coordinates
	D3DXVec3TransformCoord(&TempVec, &Point, &InvMatrix);

	// Test, whether the point is within
	if (TempVec.x >= Corners[0].x && TempVec.x <= Corners[7].x &&
		TempVec.y >= Corners[0].y && TempVec.y <= Corners[7].y &&
		TempVec.z >= Corners[0].z && TempVec.z <= Corners[7].z)
			Inside = true;

	// We are inside and want more precision in test
	if (Inside && PrecisionLevel != 0 && pNodes.size() > 0)
	{
		for (unsigned int i = 0; i < pNodes.size(); i++)
			if (pNodes[i]->FastTest(TempVec, --PrecisionLevel) )
				return true;
		
		return false;
	}
			
	return Inside;
}

BOOL CBoundingBox::FastTest(FLOAT x, FLOAT y, FLOAT z, int PrecisionLevel)
{
	return FastTest(D3DXVECTOR3(x, y, z), PrecisionLevel);
}

// determines, whether another bounding box set is in collision with
// this set of bounding boxes
BOOL CBoundingBox::FastTest(CBoundingBox * Box, int PrecisionLevel,
						   D3DXVECTOR3 * HitVector1, D3DXVECTOR3 * HitVector2)
{
	BOOL Inside;

	Inside = BoxHit(this, Box);

	if (Inside && PrecisionLevel != 0)
	{	
		if (pNodes.size() > 0 && Box->pNodes.size() > 0)
		{
			for (unsigned int i = 0; i < pNodes.size(); i++)
				for (unsigned int j = 0; j < Box->pNodes.size(); j++)
					if (pNodes[i]->FastTest(Box->pNodes[j], --PrecisionLevel, HitVector1, HitVector2))
					{
						return true;
					}
			return false;
		}
		else if (pNodes.size() > 0)
		{
			for (unsigned int i = 0; i < pNodes.size(); i++)
				if (pNodes[i]->FastTest(Box, --PrecisionLevel, HitVector1, HitVector2))
					return true;
			return false;
		}
		else if (Box->pNodes.size() > 0)
		{
			for (unsigned int j = 0; j < Box->pNodes.size(); j++)
				if (FastTest(Box->pNodes[j], --PrecisionLevel, HitVector1, HitVector2))
					return true;
			return false;
		}
	}	
	
	// Computes precisely hit point
	if (Inside)
	{
		PreciseBoxHit(this, Box, HitVector1, HitVector2);
	}

	return Inside;
}

// Return true, if structure of boxes has collision one with another
// Also returns HitVectors for top level bounding boxes
// Is totally precise (except inclusion of one box in other - very unlikely to occur) )
BOOL CBoundingBox::PreciseTest(CBoundingBox * Box, int PrecisionLevel, D3DXVECTOR3 * HitVector1, D3DXVECTOR3 * HitVector2)
{
	BOOL Inside;

	Inside = PreciseBoxHit(this, Box);

	if (Inside && PrecisionLevel != 0)
	{	
		if (pNodes.size() > 0 && Box->pNodes.size() > 0)
		{
			for (unsigned int i = 0; i < pNodes.size(); i++)
				for (unsigned int j = 0; j < Box->pNodes.size(); j++)
					if (pNodes[i]->PreciseTest(Box->pNodes[j], --PrecisionLevel, HitVector1, HitVector2))
					{
						return true;
					}
			return false;
		}
		else if (pNodes.size() > 0)
		{
			for (unsigned int i = 0; i < pNodes.size(); i++)
				if (pNodes[i]->PreciseTest(Box, --PrecisionLevel, HitVector1, HitVector2) )
					return true;
			return false;
		}
		else if (Box->pNodes.size() > 0)
		{
			for (unsigned int j = 0; j < Box->pNodes.size(); j++)
				if (PreciseTest(Box->pNodes[j], --PrecisionLevel, HitVector1, HitVector2) )
					return true;
			return false;
		}
	}	
	
	// Computes precisely hit point
	if (Inside)
	{
		PreciseBoxHit(this, Box, HitVector1, HitVector2);
	}

	return Inside;
}


/* 
BoxHit
Return true, if bounding boxes intersect
Test if corners of bounding box are inside other bounding box and vice versa
===> Doesn't detect all collisions, but most!!!
*/
BOOL CBoundingBox::BoxHit(CBoundingBox * Box1, CBoundingBox * Box2)
{
	D3DXVECTOR3 TempCorners[8];
	int i;

	// Translates coordinates
	CornersFromBoxToBox(Box1, Box2, Box1->Corners, TempCorners);

	// Test, whether the point is within
	for (i = 0; i < 8; i++)
	{
		if (TempCorners[i].x >= Box2->Corners[0].x && TempCorners[i].y >= Box2->Corners[0].y &&
			TempCorners[i].z >= Box2->Corners[0].z && TempCorners[i].x <= Box2->Corners[7].x &&
			TempCorners[i].y <= Box2->Corners[7].y && TempCorners[i].z <= Box2->Corners[7].z)
			return true;
	}
	
	// Translates coordinates
	CornersFromBoxToBox(Box2, Box1, Box2->Corners, TempCorners);

	// Test, whether the point is within
	for (i = 0; i < 8; i++)
	{
		if (TempCorners[i].x >= Box1->Corners[0].x && TempCorners[i].y >= Box1->Corners[0].y &&
			TempCorners[i].z >= Box1->Corners[0].z && TempCorners[i].x <= Box1->Corners[7].x &&
			TempCorners[i].y <= Box1->Corners[7].y && TempCorners[i].z <= Box1->Corners[7].z)
			return true;
	}


	return false;
}


// Recomputes matrix for point transformation into bouding box space
void CBoundingBox::Recompute()
{
	Changed = false;
	D3DXMATRIX TempMatrix; // temporal matrix

	// Identity matrix
	D3DXMatrixIdentity(&Matrix);
	
	// scale point back
	D3DXMatrixScaling(&TempMatrix, Scaling.x, Scaling.y, Scaling.z);
	Matrix *= TempMatrix;

	// rotate point back
	D3DXMatrixRotationX(&TempMatrix, Rotation.x);
	Matrix *= TempMatrix;
	D3DXMatrixRotationY(&TempMatrix, Rotation.y);
	Matrix *= TempMatrix;
	D3DXMatrixRotationZ(&TempMatrix, Rotation.z);
	Matrix *= TempMatrix;

	// move point
	D3DXMatrixTranslation(&TempMatrix, Position.x, Position.y, Position.z);
	Matrix *= TempMatrix;

	// Compute inverse matrix
	D3DXMatrixInverse(&InvMatrix, NULL, &Matrix);
}

// Recompute corners, after one corner has been set
void CBoundingBox::RecomputeCorners()
{
	// Right, bottom, front 
	Corners[1].x = Corners[7].x; 
	Corners[1].y = Corners[0].y;
	Corners[1].z = Corners[0].z;

	// Right, upper, front
	Corners[2].x = Corners[7].x;
	Corners[2].y = Corners[7].y;
	Corners[2].z = Corners[0].z;

	// Left, upper, front
	Corners[3].x = Corners[0].x;
	Corners[3].y = Corners[7].y;
	Corners[3].z = Corners[0].z;

	// Right, bottom, back
	Corners[4].x = Corners[7].x;
	Corners[4].y = Corners[0].y;
	Corners[4].z = Corners[7].z;

	// Left, bottom, back
	Corners[5].x = Corners[0].x;
	Corners[5].y = Corners[0].y;
	Corners[5].z = Corners[7].z;

	// Left, upper, back
	Corners[6].x = Corners[0].x;
	Corners[6].y = Corners[7].y;
	Corners[6].z = Corners[7].z;
}

// Recomputes planes
void CBoundingBox::RecomputePlanes()
{
	D3DXVECTOR3 x1y2z1 = D3DXVECTOR3(Corners[0].x,
									Corners[1].y,
									Corners[0].z);
	D3DXVECTOR3 x2y1z1 = D3DXVECTOR3(Corners[1].x,
									Corners[0].y,
									Corners[0].z);
	D3DXVECTOR3 x1y1z2 = D3DXVECTOR3(Corners[0].x,
									 Corners[0].y,
									 Corners[1].z);
	D3DXVECTOR3 x2y1z2 = D3DXVECTOR3(Corners[1].x,
									 Corners[0].y,
									 Corners[1].z);
	D3DXVECTOR3 x2y2z1 = D3DXVECTOR3(Corners[1].x,
									 Corners[1].y,
									 Corners[0].z);

	// Front side plane
	D3DXPlaneFromPoints(&BoundingBoxPlanes[0],
						&Corners[0],
						&x1y2z1,
						&x2y1z1);
	// Left side plane
	D3DXPlaneFromPoints(&BoundingBoxPlanes[1],
		                &Corners[0],
						&x1y2z1,
						&x1y1z2);
	// bottom side plane
	D3DXPlaneFromPoints(&BoundingBoxPlanes[2],
						&Corners[0],
						&x2y1z1,
						&x1y1z2);
	// back side plane
	D3DXPlaneFromPoints(&BoundingBoxPlanes[3],
						&Corners[1],
						&x1y1z2,
						&x2y1z2);
	// right side plane
	D3DXPlaneFromPoints(&BoundingBoxPlanes[4],
						&Corners[1],
						&x2y1z2,
						&x2y1z1);
	// top side plane
	D3DXPlaneFromPoints(&BoundingBoxPlanes[5],
						&Corners[1],
						&x1y2z1,
						&x2y2z1);
}

/*
PreciseBoxHit
Return true, when two bounding boxes collide
This is function, which computes it correctly in all cases
(except inclusion of one box in other, which is not likely to occur)
*/
BOOL CBoundingBox::PreciseBoxHit(CBoundingBox * Box1, CBoundingBox* Box2)
{
	// Temporary Corners of bounding box in world coordinates
	D3DXVECTOR3 TempCorners[8];

	// Recompute box coordinates to other box coordinates
	CornersFromBoxToBox(Box1, Box2, Box1->Corners, TempCorners);
	
	// Checks, whether lines intersect sides
	if (LineBoxIntersect(TempCorners[0], TempCorners[1], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[1], TempCorners[2], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[2], TempCorners[3], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[3], TempCorners[0], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[4], TempCorners[5], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[5], TempCorners[6], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[6], TempCorners[7], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[7], TempCorners[4], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[0], TempCorners[5], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[1], TempCorners[4], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[3], TempCorners[6], Box2) ) return true;
	if (LineBoxIntersect(TempCorners[2], TempCorners[7], Box2) ) return true;

	// checks for inclusion to box2
	if (TempCorners[0].x >= Box2->Corners[0].x && TempCorners[0].x <= Box2->Corners[7].x &&
		TempCorners[0].y >= Box2->Corners[0].y && TempCorners[0].y <= Box2->Corners[7].y &&
		TempCorners[0].z >= Box2->Corners[0].z && TempCorners[0].z <= Box2->Corners[7].z)
		return true;
	

	// and vice versa
	CornersFromBoxToBox(Box2, Box1, Box2->Corners, TempCorners);

	// Checks, whether lines intersect sides
	if (LineBoxIntersect(TempCorners[0], TempCorners[1], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[1], TempCorners[2], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[2], TempCorners[3], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[3], TempCorners[0], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[4], TempCorners[5], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[5], TempCorners[6], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[6], TempCorners[7], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[7], TempCorners[4], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[0], TempCorners[5], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[1], TempCorners[4], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[3], TempCorners[6], Box1) ) return true;
	if (LineBoxIntersect(TempCorners[2], TempCorners[7], Box1) ) return true;

	// checks for inclusion to box1
	if (TempCorners[0].x >= Box1->Corners[0].x && TempCorners[0].x <= Box1->Corners[7].x &&
		TempCorners[0].y >= Box1->Corners[0].y && TempCorners[0].y <= Box1->Corners[7].y &&
		TempCorners[0].z >= Box1->Corners[0].z && TempCorners[0].z <= Box1->Corners[7].z)
		return true;

	return false;
}

// Returns true, if bounding boxes intersect - precise, return HitVector
BOOL CBoundingBox::PreciseBoxHit(CBoundingBox * Box1, CBoundingBox * Box2, 
								 D3DXVECTOR3 * HitVector1, D3DXVECTOR3 * HitVector2)
{
	// Temporary Corners of bounding box in world coordinates
	D3DXVECTOR3 TempCorners[8];
	set<D3DXVECTOR3, LessD3DXVECTOR3> IntersectPoints;
	D3DXVECTOR3 Result;
	Result.x = Result.y = Result.z = 0;
	unsigned int IntersectPointCount = 0;
	set<D3DXVECTOR3, LessD3DXVECTOR3>::iterator It;
	int i;
	
	// Matrix for recomputing points to world coordinates
	D3DXMATRIX WorldMatrix;
	D3DXVECTOR3 TempVector;
	CBoundingBox * Parents = Box2;

	// Recompute box coordinates to other box coordinates
	CornersFromBoxToBox(Box1, Box2, Box1->Corners, TempCorners);
	
	// Checks, whether lines intersect sides
	LineBoxIntersect(TempCorners[0], TempCorners[1], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[1], TempCorners[2], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[2], TempCorners[3], Box2, IntersectPoints); 
	LineBoxIntersect(TempCorners[3], TempCorners[0], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[4], TempCorners[5], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[5], TempCorners[6], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[6], TempCorners[7], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[7], TempCorners[4], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[0], TempCorners[5], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[1], TempCorners[4], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[3], TempCorners[6], Box2, IntersectPoints);
	LineBoxIntersect(TempCorners[2], TempCorners[7], Box2, IntersectPoints);
	
	// Check for point inclusion in Box2
	for (i = 0; i < 8; i++)
	{
		if (TempCorners[i].x >= Box2->Corners[0].x && TempCorners[i].x <= Box2->Corners[7].x &&
		TempCorners[i].y >= Box2->Corners[0].y && TempCorners[i].y <= Box2->Corners[7].y &&
		TempCorners[i].z >= Box2->Corners[0].z && TempCorners[i].z <= Box2->Corners[7].z)
			IntersectPoints.insert(TempCorners[i]);
	}

	if (IntersectPoints.size() != 0)
	{
		// Computes matrix for point transformation to world coords
		D3DXMatrixIdentity(&WorldMatrix);
		do
		{
			D3DXMatrixMultiply(&WorldMatrix, &WorldMatrix, &(Parents->Matrix));
			Parents = Parents->Parent;
		}
		while (Parents != NULL);

		// Recomputes all points to world coordinates
		for (It = IntersectPoints.begin(); It != IntersectPoints.end(); It++)
		{
			TempVector = *It;
			D3DXVec3TransformCoord(&TempVector, &TempVector, &WorldMatrix);
			Result += TempVector;
		}
		IntersectPointCount = (unsigned int) IntersectPoints.size();
		// erases all elements in set
		IntersectPoints.clear();
	}

	// and vice versa
	CornersFromBoxToBox(Box2, Box1, Box2->Corners, TempCorners);

	// Checks, whether lines intersect sides
	LineBoxIntersect(TempCorners[0], TempCorners[1], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[1], TempCorners[2], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[2], TempCorners[3], Box1, IntersectPoints); 
	LineBoxIntersect(TempCorners[3], TempCorners[0], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[4], TempCorners[5], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[5], TempCorners[6], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[6], TempCorners[7], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[7], TempCorners[4], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[0], TempCorners[5], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[1], TempCorners[4], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[3], TempCorners[6], Box1, IntersectPoints);
	LineBoxIntersect(TempCorners[2], TempCorners[7], Box1, IntersectPoints);

	// Check for point inclusion in Box1
	for (i = 0; i < 8; i++)
	{
		if (TempCorners[i].x >= Box1->Corners[0].x && TempCorners[i].x <= Box1->Corners[7].x &&
		TempCorners[i].y >= Box1->Corners[0].y && TempCorners[i].y <= Box1->Corners[7].y &&
		TempCorners[i].z >= Box1->Corners[0].z && TempCorners[i].z <= Box1->Corners[7].z)
			IntersectPoints.insert(TempCorners[i]);
	}

	if (IntersectPoints.size() != 0)
	{
		Parents = Box1;
		// Computes matrix for point transformation to world coords
		D3DXMatrixIdentity(&WorldMatrix);
		do
		{
			D3DXMatrixMultiply(&WorldMatrix, &WorldMatrix, &(Parents->Matrix));
			Parents = Parents->Parent;
		}
		while (Parents != NULL);

		// Recomputes all points to world coordinates
		for (It = IntersectPoints.begin(); It != IntersectPoints.end(); It++)
		{
			TempVector = *It;
			D3DXVec3TransformCoord(&TempVector, &TempVector, &WorldMatrix);
			Result += TempVector;
		}
		IntersectPointCount += (unsigned int) IntersectPoints.size();
		// erases all elements in set
		IntersectPoints.clear();
	}
	
	if (!IntersectPointCount)
		return false; // No intersection

	// Compute average of all intersect points
	Result.x /= IntersectPointCount;
	Result.y /= IntersectPointCount;
	Result.z /= IntersectPointCount;
	
	// Translates hit point into top level bounding box
	// So result is vector from coords center to hit point in top level
	// bounding box coordinates

	// find top level bounding box
	Parents = Box1;
	while (Parents->Parent != NULL)
	{
		Parents = Parents->Parent;
	}
	// Translate to top level bounding box coordinates
	D3DXVec3TransformCoord(HitVector1, &Result, &(Parents->InvMatrix));
	
	// find top level bounding box
	Parents = Box2;
	while (Parents->Parent != NULL)
	{
		Parents = Parents->Parent;
	}
	// Translate to top level bounding box coordinates
	D3DXVec3TransformCoord(HitVector2, &Result, &(Parents->InvMatrix));
	
	return true;
}

// Creates parametric line representation
void CBoundingBox::ParamLine(D3DXVECTOR3 * Point1, D3DXVECTOR3 * Point2, D3DXVECTOR3 * Direction, D3DXVECTOR3 * Point)
{
	Direction->x = Point2->x - Point1->x;
	Direction->y = Point2->y - Point1->y;
	Direction->z = Point2->z - Point1->z;

	*Point = *Point1;
}


// destructor
CBoundingBox::~CBoundingBox()
{
	// Deletes all child objects
	for (unsigned int i = 0; i < pNodes.size();i++)
		delete pNodes[i];
}

// Constructor
CBoundingBox::CBoundingBox()
: Changed(true), Position(D3DXVECTOR3(0,0,0)), Rotation(D3DXVECTOR3(0,0,0)),
  Scaling(D3DXVECTOR3(1,1,1)), Parent(NULL), ForcePrecision(false)
{
 for (int i = 0; i < 8; i++)
	Corners[i] = D3DXVECTOR3(0, 0, 0);
}

// Recomputes box corners coordinates representation 
// into other box coordinate representation
void CBoundingBox::CornersFromBoxToBox(CBoundingBox * FromBox, 
						 CBoundingBox * ToBox, 
						 D3DXVECTOR3 * InCorners, 
						 D3DXVECTOR3 * OutCorners)
{
	CBoundingBox * Parents = FromBox;
	int i,e;
	vector<CBoundingBox *> WayToParent;

	// Translates all box coordinates to world coordinates
	memcpy(OutCorners, InCorners, 8 * sizeof(D3DXVECTOR3));
	
	do
	{
		if (Parents->Changed) Parents->Recompute();

		for (i = 0; i < 8; i++)
			D3DXVec3TransformCoord(&OutCorners[i], &OutCorners[i], &(Parents->Matrix));
		
		Parents = Parents->Parent;
	}
	while (Parents != NULL);
	
	// Tracks the way to parent
	Parents = ToBox;
	
	do
	{
		if (Parents->Changed) Parents->Recompute();
		WayToParent.push_back(Parents);
		Parents = Parents->Parent;
	}
	while (Parents != NULL);
	
	// Translates all box coordinates to other box coordinates
	for (e = (int) WayToParent.size() - 1; e > -1; e--)
	  for (i = 0; i < 8; i++)
			D3DXVec3TransformCoord(&OutCorners[i], &OutCorners[i], &(WayToParent[e]->InvMatrix));
}

// Checks, if line intersect box in it's coordinates
// Intersect points are pushed back into array
BOOL CBoundingBox::LineBoxIntersect(D3DXVECTOR3 & Point1, D3DXVECTOR3 & Point2, CBoundingBox * Box, set<D3DXVECTOR3, LessD3DXVECTOR3> & IntersectPoints)
{
	BOOL Result = false;
	D3DXVECTOR3 Direction;
	D3DXVECTOR3 Point;
	
	// Create parametric line representation
	ParamLine(&Point1, &Point2, &Direction, &Point);
	
	// Computes intersection points with sides
	Result = LineSideIntersectXY(Box->Corners[0], Box->Corners[2], Direction, Point, IntersectPoints);
	Result |= LineSideIntersectXY(Box->Corners[5], Box->Corners[7], Direction, Point, IntersectPoints);
	Result |= LineSideIntersectXZ(Box->Corners[0], Box->Corners[4], Direction, Point, IntersectPoints);
	Result |= LineSideIntersectXZ(Box->Corners[3], Box->Corners[7], Direction, Point, IntersectPoints);
	Result |= LineSideIntersectYZ(Box->Corners[0], Box->Corners[6], Direction, Point, IntersectPoints);
	Result |= LineSideIntersectYZ(Box->Corners[1], Box->Corners[7], Direction, Point, IntersectPoints);

	return Result;
}

BOOL CBoundingBox::LineBoxIntersect(D3DXVECTOR3 & Point1, D3DXVECTOR3 & Point2, CBoundingBox * Box)
{

	set <D3DXVECTOR3, LessD3DXVECTOR3> IntersectPoints;
	D3DXVECTOR3 Direction;
	D3DXVECTOR3 Point;
	
	// Create parametric line representation
	ParamLine(&Point1, &Point2, &Direction, &Point);
	
	if (LineSideIntersectXY(Box->Corners[0], Box->Corners[2], Direction, Point, IntersectPoints)) return true;
	if (LineSideIntersectXY(Box->Corners[5], Box->Corners[7], Direction, Point, IntersectPoints)) return true;
	if (LineSideIntersectXZ(Box->Corners[0], Box->Corners[4], Direction, Point, IntersectPoints)) return true;
	if (LineSideIntersectXZ(Box->Corners[3], Box->Corners[7], Direction, Point, IntersectPoints)) return true;
	if (LineSideIntersectYZ(Box->Corners[0], Box->Corners[6], Direction, Point, IntersectPoints)) return true;
	if (LineSideIntersectYZ(Box->Corners[1], Box->Corners[7], Direction, Point, IntersectPoints)) return true;

	return false;
}

/*
LineSideIntersectYZ - computes points of intersect for line and side parallel to YZ plane
*/
BOOL CBoundingBox::LineSideIntersectYZ(const D3DXVECTOR3 & LowerLeft, const D3DXVECTOR3 & UpperRight, const D3DXVECTOR3 & Direction,
						 const D3DXVECTOR3 & Point, set<D3DXVECTOR3, LessD3DXVECTOR3> & IntersectPoints)
{
	// Intersect point
	D3DXVECTOR3 Intersect;
	
	// Compute param (first part)
	float Param = LowerLeft.x - Point.x;

	// Line is parallel to side, but is not in same coord
	if (Param != 0 && Direction.x == 0)
		return false;
	
	// Line is not parallel to side
	if (Direction.x != 0)
	{
		// Compute param (second part)
		Param /= Direction.x;
		Intersect.y = Direction.y * Param + Point.y;
		Intersect.z = Direction.z * Param + Point.z;

		// Test, if point is inside side and line
		if (Intersect.y >= LowerLeft.y && Intersect.z >= LowerLeft.z &&
			Intersect.y <= UpperRight.y && Intersect.z <= UpperRight.z &&
			Param >= 0 && Param <= 1)
		{
			Intersect.x = LowerLeft.x;
			IntersectPoints.insert(Intersect);
			return true;
		}
		else 
			return false;
	}

	// Line is parallel to side
	// The line can intersect side maximal at two points
	int NumPushed = 2;

	Intersect.x = LowerLeft.x;

	// check for endpoints inside rectangle
	if (Point.y >= LowerLeft.y && Point.z >= LowerLeft.z &&
		Point.y <= UpperRight.y && Point.z <= UpperRight.z)
	{
		Intersect.y = Point.y;
		Intersect.z = Point.z;
		if (IntersectPoints.insert(Intersect).second)
			NumPushed--;
	}

	if ((Point.y + Direction.y) >= LowerLeft.y && (Point.z + Direction.z) >= LowerLeft.z &&
		(Point.y + Direction.y) <= UpperRight.y && (Point.z + Direction.z) <= UpperRight.z)
	{
		Intersect.y = Point.y + Direction.y;
		Intersect.z = Point.z + Direction.z;
		if (IntersectPoints.insert(Intersect).second)
			NumPushed--;
		if (!NumPushed) return true;
	}

	// check for sideline and line intersection
	
	// Y parallel
	// Compute another param (first part)
	Param = LowerLeft.z - Point.z;
	Intersect.z = LowerLeft.z;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.z == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.z != 0)
		{
			// compute another param (second part)
			Param /= Direction.z;
			Intersect.y = Direction.y * Param + Point.y;

			// Test if points are on line
			if (Intersect.y >= LowerLeft.y && Intersect.y <= UpperRight.y &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
		
	}

	
	// Compute another param (first part)
	Param = UpperRight.z - Point.z;
	Intersect.z = UpperRight.z;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.z == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.z != 0)
		{
			// compute another param (second part)
			Param /= Direction.z;
			Intersect.y = Direction.y * Param + Point.y;

			// Test if points are on line
			if (Intersect.y >= LowerLeft.y && Intersect.y <= UpperRight.y &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
	}

	// Z Parallel sides
	// Compute another param (first part)
	Param = LowerLeft.y - Point.y;
	Intersect.y = LowerLeft.y;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.y == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.y != 0)
		{
			// compute another param (second part)
			Param /= Direction.y;
			Intersect.z = Direction.z * Param + Point.z;

			// Test if points are on line
			if (Intersect.z >= LowerLeft.z && Intersect.z <= UpperRight.z &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
		
	}

	// Compute another param (first part)
	Param = UpperRight.y - Point.y;
	Intersect.y = UpperRight.y;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.y == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.y != 0)
		{
			// compute another param (second part)
			Param /= Direction.y;
			Intersect.z = Direction.z * Param + Point.z;

			// Test if points are on line
			if (Intersect.z >= LowerLeft.z && Intersect.z <= UpperRight.z &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
	}

	return false;
}


/*
LineSideIntersectXZ - computes points of intersect for line and side parallel to YZ plane
*/
BOOL CBoundingBox::LineSideIntersectXZ(const D3DXVECTOR3 & LowerLeft, const D3DXVECTOR3 & UpperRight, const D3DXVECTOR3 & Direction,
						 const D3DXVECTOR3 & Point, set<D3DXVECTOR3, LessD3DXVECTOR3> & IntersectPoints)
{
	// Intersect point
	D3DXVECTOR3 Intersect;
	
	// Compute param (first part)
	float Param = LowerLeft.y - Point.y;

	// Line is parallel to side, but is not in same coord
	if (Param != 0 && Direction.y == 0)
		return false;
	
	// Line is not parallel to side
	if (Direction.y != 0)
	{
		// Compute param (second part)
		Param /= Direction.y;
		Intersect.x = Direction.x * Param + Point.x;
		Intersect.z = Direction.z * Param + Point.z;

		// Test, if point is inside side and line
		if (Intersect.x >= LowerLeft.x && Intersect.z >= LowerLeft.z &&
			Intersect.x <= UpperRight.x && Intersect.z <= UpperRight.z &&
			Param >= 0 && Param <= 1)
		{
			Intersect.y = LowerLeft.y;
			IntersectPoints.insert(Intersect);
			return true;
		}
		else 
			return false;
	}

	// Line is parallel to side
	// The line can intersect side maximal at two points
	int NumPushed = 2;

	Intersect.y = LowerLeft.y;

	// check for endpoints inside rectangle
	if (Point.x >= LowerLeft.x && Point.z >= LowerLeft.z &&
		Point.x <= UpperRight.x && Point.z <= UpperRight.z)
	{
		Intersect.x = Point.x;
		Intersect.z = Point.z;
		if (IntersectPoints.insert(Intersect).second)
			NumPushed--;
	}

	if ((Point.x + Direction.x) >= LowerLeft.x && (Point.z + Direction.z) >= LowerLeft.z &&
		(Point.x + Direction.x) <= UpperRight.x && (Point.z + Direction.z) <= UpperRight.z)
	{
		Intersect.x = Point.x + Direction.x;
		Intersect.z = Point.z + Direction.z;
		if (IntersectPoints.insert(Intersect).second)
			NumPushed--;
		if (!NumPushed) return true;
	}

	// check for sideline and line intersection
	
	// X parallel
	// Compute another param (first part)
	Param = LowerLeft.z - Point.z;
	Intersect.z = LowerLeft.z;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.z == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.z != 0)
		{
			// compute another param (second part)
			Param /= Direction.z;
			Intersect.x = Direction.x * Param + Point.x;

			// Test if points are on line
			if (Intersect.x >= LowerLeft.x && Intersect.x <= UpperRight.x &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
		
	}

	
	// Compute another param (first part)
	Param = UpperRight.z - Point.z;
	Intersect.z = UpperRight.z;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.z == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.z != 0)
		{
			// compute another param (second part)
			Param /= Direction.z;
			Intersect.x = Direction.x * Param + Point.x;

			// Test if points are on line
			if (Intersect.x >= LowerLeft.x && Intersect.x <= UpperRight.x &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
	}

	// Z Parallel sides
	// Compute another param (first part)
	Param = LowerLeft.x - Point.x;
	Intersect.x = LowerLeft.x;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.x == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.x != 0)
		{
			// compute another param (second part)
			Param /= Direction.x;
			Intersect.z = Direction.z * Param + Point.z;

			// Test if points are on line
			if (Intersect.z >= LowerLeft.z && Intersect.z <= UpperRight.z &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
		
	}

	// Compute another param (first part)
	Param = UpperRight.x - Point.x;
	Intersect.x = UpperRight.x;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.x == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.x != 0)
		{
			// compute another param (second part)
			Param /= Direction.x;
			Intersect.z = Direction.z * Param + Point.z;

			// Test if points are on line
			if (Intersect.z >= LowerLeft.z && Intersect.z <= UpperRight.z &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
	}

	return false;
}

/*
LineSideIntersectXY - computes points of intersect for line and side parallel to XY plane
*/
BOOL CBoundingBox::LineSideIntersectXY(const D3DXVECTOR3 & LowerLeft, const D3DXVECTOR3 & UpperRight, const D3DXVECTOR3 & Direction,
						 const D3DXVECTOR3 & Point, set<D3DXVECTOR3, LessD3DXVECTOR3> & IntersectPoints)
{
	// Intersect point
	D3DXVECTOR3 Intersect;
	
	// Compute param (first part)
	float Param = LowerLeft.z - Point.z;

	// Line is parallel to side, but is not in same coord
	if (Param != 0 && Direction.z == 0)
		return false;
	
	// Line is not parallel to side
	if (Direction.z != 0)
	{
		// Compute param (second part)
		Param /= Direction.z;
		Intersect.x = Direction.x * Param + Point.x;
		Intersect.y = Direction.y * Param + Point.y;

		// Test, if point is inside side and line
		if (Intersect.x >= LowerLeft.x && Intersect.y >= LowerLeft.y &&
			Intersect.x <= UpperRight.x && Intersect.y <= UpperRight.y &&
			Param >= 0 && Param <= 1)
		{
			Intersect.z = LowerLeft.z;
			IntersectPoints.insert(Intersect);
			return true;
		}
		else 
			return false;
	}

	// Line is parallel to side
	// The line can intersect side maximal at two points
	int NumPushed = 2;

	Intersect.z = LowerLeft.z;

	// check for endpoints inside rectangle
	if (Point.x >= LowerLeft.x && Point.y >= LowerLeft.y &&
		Point.x <= UpperRight.x && Point.y <= UpperRight.y)
	{
		Intersect.x = Point.x;
		Intersect.y = Point.y;
		if (IntersectPoints.insert(Intersect).second)
			NumPushed--;
	}

	if ((Point.x + Direction.x) >= LowerLeft.x && (Point.y + Direction.y) >= LowerLeft.y &&
		(Point.x + Direction.x) <= UpperRight.x && (Point.y + Direction.y) <= UpperRight.y)
	{
		Intersect.x = Point.x + Direction.x;
		Intersect.y = Point.y + Direction.y;
		if (IntersectPoints.insert(Intersect).second)
			NumPushed--;
		if (!NumPushed) return true;
	}

	// check for sideline and line intersection
	
	// X parallel
	// Compute another param (first part)
	Param = LowerLeft.y - Point.y;
	Intersect.y = LowerLeft.y;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.y == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.y != 0)
		{
			// compute another param (second part)
			Param /= Direction.y;
			Intersect.x = Direction.x * Param + Point.x;

			// Test if points are on line
			if (Intersect.x >= LowerLeft.x && Intersect.x <= UpperRight.x &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
		
	}

	
	// Compute another param (first part)
	Param = UpperRight.y - Point.y;
	Intersect.y = UpperRight.y;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.y == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.y != 0)
		{
			// compute another param (second part)
			Param /= Direction.y;
			Intersect.x = Direction.x * Param + Point.x;

			// Test if points are on line
			if (Intersect.x >= LowerLeft.x && Intersect.x <= UpperRight.x &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
	}

	// Y Parallel sides
	// Compute another param (first part)
	Param = LowerLeft.x - Point.x;
	Intersect.x = LowerLeft.x;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.x == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.x != 0)
		{
			// compute another param (second part)
			Param /= Direction.x;
			Intersect.y = Direction.y * Param + Point.y;

			// Test if points are on line
			if (Intersect.y >= LowerLeft.y && Intersect.y <= UpperRight.y &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
		
	}

	// Compute another param (first part)
	Param = UpperRight.x - Point.x;
	Intersect.x = UpperRight.x;

	// Lines are parallel, but no intersection
	if (Param != 0 && Direction.x == 0)
	{
		;
	}
	else
	{
		// Lines are not parallel
		if (Direction.x != 0)
		{
			// compute another param (second part)
			Param /= Direction.x;
			Intersect.y = Direction.y * Param + Point.y;

			// Test if points are on line
			if (Intersect.y >= LowerLeft.y && Intersect.y <= UpperRight.y &&
				Param >= 0 && Param <= 1)
			{
				if (IntersectPoints.insert(Intersect).second)
					NumPushed--;
				if (!NumPushed) return true;
			}
		}
		// Lines are parallel - solved by computation of end point for other bounding box line
	}

	return false;
}

// returns count of nodes in whole hierarchy
UINT CBoundingBox::GetNodesCount() const
{
	UINT				sum = 0;
	BBVECTOR_CONSTITER	iter; 
	
	for ( iter = pNodes.begin(); iter != pNodes.end(); iter++ )
		sum += (*iter)->GetNodesCount();

	return ++sum;
}

// Saves hierarchy to filename
void CBoundingBox::Save(const string FileName) const
{
	ofstream Output;

	Output.open(FileName.c_str(), ios_base::binary);
	
	if (! Output)
		return; // Error - typically file not found

	// Save bounding box hierarchy
	SaveBox(&Output);
}

// Saves current bounding box hierarchy to opened file stream
void CBoundingBox::SaveBox(ofstream * Output) const
{
	// BB Block
	Output->write((char *)&Corners[0], sizeof(D3DXVECTOR3));
	Output->write((char *)&Corners[7], sizeof(D3DXVECTOR3));
	Output->write((char *)&Scaling, sizeof(D3DXVECTOR3));
	Output->write((char *)&Rotation, sizeof(D3DXVECTOR3));
	Output->write((char *)&Position, sizeof(D3DXVECTOR3));
	Output->write((char *)&ForcePrecision, sizeof(BOOL));
	unsigned int Size = (unsigned int) pNodes.size();
	Output->write((char *) & Size, sizeof(unsigned int));

	for (unsigned int i = 0; i < pNodes.size(); i++)
		pNodes[i]->SaveBox(Output);
}

// Loads hierarchy from filename and returns pointer to top box
CBoundingBox * CBoundingBox::Load(const string FileName)
{
	ifstream Input;

	Input.open(FileName.c_str(), ios_base::binary);

	if (! Input)
		return 0; // Error - typically file not found

	// Loads bounding box hierarchy
	return LoadBox(&Input, 0);
}

// Loads current bounding box hierarchy from opened file stream
CBoundingBox * CBoundingBox::LoadBox(ifstream * Input, CBoundingBox * Parent)
{
	CBoundingBox * TempBox = new CBoundingBox();
	D3DXVECTOR3 TempVec;
	BOOL TempBool;
	unsigned int Size;

	// Upper right corner
	Input->read( (char *) &TempVec, sizeof(D3DXVECTOR3));
	TempBox->SetLowerLeftCorner(TempVec);
	
	// Lower left corner
	Input->read((char *) &TempVec, sizeof(D3DXVECTOR3));
	TempBox->SetUpperRightCorner(TempVec);

	// Compute other corners
	TempBox->RecomputeCorners();

	// Scaling
	Input->read((char *) &TempVec, sizeof(D3DXVECTOR3));
	TempBox->SetScaling(TempVec);

	// Rotation
	Input->read((char *) &TempVec, sizeof(D3DXVECTOR3));
	TempBox->SetRotation(TempVec);

	// Position
	Input->read((char *) &TempVec, sizeof(D3DXVECTOR3));
	TempBox->SetPosition(TempVec);

	// Recompute matrixes
	TempBox->Recompute();

	// Force precision
	Input->read((char *) &TempBool, sizeof(BOOL) );
	TempBox->SetForcePrecision(TempBool);

	// Number of children
	Input->read((char *) &Size, sizeof(unsigned int) );
	
	// Load and add children
	for (unsigned int i = 0; i < Size; i++)
		TempBox->AddNode(LoadBox(Input, TempBox) );

	// Set parent
	TempBox->Parent = Parent;
	
	return TempBox;	
}