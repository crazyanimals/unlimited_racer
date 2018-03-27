/*
ConvexHull.h - contains functions for 2D convex hull computation
by Pavel Celba
16. 3. 2005
*/

#pragma once
#include "..\Globals\GlobalsIncludes.h"
#include <vector>

class ConvexHull
{
public:
	// Computes 2D convex hull based on x and y coords. The result is pushed back to ConvexHull param.
	int ComputeConvexHull(std::vector<D3DXVECTOR2> * Points, std::vector<D3DXVECTOR2> * ConvexHull);

private:
	// returns minimal y coord point (x is minimal, when y coords are same)
	std::vector<D3DXVECTOR2>::iterator MinY(std::vector<D3DXVECTOR2> * Points);

};