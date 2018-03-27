/*
ConvexHull.cpp - implementation of ConvexHull class. Computes 2D convex hull.
by Pavel Celba
16. 3. 2005
*/

#include "ConvexHull.h"

using namespace std;

// Computes 2D convex hull based on x and y coords. The result is pushed back to ConvexHull param.
// Precondition:
// At least distinct 3 points present
int ConvexHull::ComputeConvexHull(std::vector<D3DXVECTOR2> * Points, std::vector<D3DXVECTOR2> * ConvexHull)
{
	vector<D3DXVECTOR2>::iterator It;
	vector<D3DXVECTOR2>::iterator MaxIt;
	
	// Direction vector 
	D3DXVECTOR2 Direction = D3DXVECTOR2(1,0);
	D3DXVECTOR2 RefVec = D3DXVECTOR2(1,0);
	
	float CosAngle;
	float MaxCosAngle;
	
	// Find minimal y coord point 
	vector<D3DXVECTOR2>::iterator Cur = MinY(Points);

	// Add mininal y coord point to convex hull
	ConvexHull->push_back(*Cur);

	do
	{
		MaxCosAngle = -2;
		for (It = Points->begin(); It != Points->end(); It++)
		{
			// calculate Direction and normalize
			Direction = (*It) - (*Cur);
			if (Direction.x == 0 && Direction.y == 0)
			{
				CosAngle = -1;
			}
			else
			{
				D3DXVec2Normalize(&Direction, &Direction);
				// compute cos(angle) - is decreasing from 1 to -1
				CosAngle = D3DXVec2Dot(&Direction, &RefVec);
			}
			if ( CosAngle > MaxCosAngle)
			{
				MaxCosAngle = CosAngle;
				MaxIt = It;
			}
			else if (CosAngle == MaxCosAngle)
			{
				// Take the point with bigger distance
				if (D3DXVec2Length(& ((*It) - (*Cur)) ) > D3DXVec2Length(& ((*MaxIt) - (*Cur) ) ) )
				{
					MaxCosAngle = CosAngle;
					MaxIt = It;
				}
			}
		}

		// add point to convex hull
		ConvexHull->push_back(*MaxIt);
		// recompute reference vector
		RefVec = (*MaxIt) - (*Cur);
		D3DXVec2Normalize(&RefVec, &RefVec);
		Cur = MaxIt;
	}
	while (  *(ConvexHull->begin()) !=  *(ConvexHull->end() - 1) );

	// Erase last since it's same as first
	ConvexHull->erase(ConvexHull->end() - 1);

	return 0;
}

// returns minimal y coord point (x is minimal, when y coords are same)
vector<D3DXVECTOR2>::iterator ConvexHull::MinY(std::vector<D3DXVECTOR2> * Points)
{
	vector<D3DXVECTOR2>::iterator Min = Points->begin();
	vector<D3DXVECTOR2>::iterator It;

	for (It = Points->begin(); It != Points->end(); It++)
	{
		if (It->y < Min->y)
			Min = It;
		else if (It->y == Min->y && It->x < Min->x)
			Min = It;
	}
	
	return Min;
}