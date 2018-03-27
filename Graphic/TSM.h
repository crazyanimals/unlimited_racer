/*
TSM.h - Trapezoidal shadow mapping implementation
by Pavel Celba
18. 3. 2005
*/

#pragma once
#include "..\Globals\GlobalsIncludes.h"
#include <vector>
#include "ConvexHull.h"
#include "stdio.h"


class TSM
{
public:
	TSM();

	// Inits trapezoidal shadow mapping
	int Init();

	// Computes TSM post-projection matrix
	int ComputeTSMMatrix(D3DXMATRIX * ProjMat, D3DXMATRIX * ViewMat, 
						 D3DXMATRIX * LightProjMat, D3DXMATRIX * LightViewMat, 
						 D3DXVECTOR3 * ViewDirection, D3DXVECTOR3 * LightDirection,
						 D3DXMATRIX * TSMMatrix);

	// Computes TSM post-projection matrix with iteration on remap point
	int ComputeTSMMatrixIterate(D3DXMATRIX * ProjMat, D3DXMATRIX * ViewMat, 
						  D3DXMATRIX * LightProjMat, D3DXMATRIX * LightViewMat, 
						  D3DXMATRIX * TSMMatrix, float * Remap, float ItStep);

	// Transforms projective box coords by matrix Transform and returns eight transformed
    // vertices into TransVerts
	inline int TransformProjBox(D3DXMATRIX * Transform, D3DXVECTOR3 * TransVerts)
	{
		D3DXVec3TransformCoordArray(TransVerts, sizeof(D3DXVECTOR3), 
									BoxVerts, sizeof(D3DXVECTOR3),
									Transform, 8);

		return 0;
	}

	// Computes light matrixes for directional light, which includes entire view frustrum
	// in AABB style
	int ComputeDirectionalLightMatrixesFromFrustrum(D3DXMATRIX * ProjMat, D3DXMATRIX * ViewMat, D3DXVECTOR3 * LightDirection,
													D3DXMATRIX * LightProjMat, D3DXMATRIX * LightViewMat, float * ZRange);

	// Computes matrix for transforming 2D rotated rectangle into [-1, -1] - [1, 1] box
	int ComputeRotatedRectangleToBoxMatrix(D3DXVECTOR2 * Rectangle, D3DXMATRIX * RectToBoxMat);

	// Computes matrix for transforming 2D trapezoid (base left, base right, top right, top left) 
	// into [-1, -1] - [1, 1] box
	int ComputeTrapezoidToBoxMatrix(D3DXVECTOR2 * Trapezoid, D3DXMATRIX * TrapezoidToBoxMat);

	// Computes matrix from [0,0] - [1,1] box to trapezoid
	int ComputeUnitBoxToTrapezoidMatrix(D3DXVECTOR2 * Trapezoid, D3DXMATRIX * BoxToTrapezoidMat);

	// 2D Line intersection
	int IntersectLines(D3DXVECTOR3 * Line1, D3DXVECTOR3 * Line2, D3DXVECTOR2 * Intersection);

	// Computes trapezoid area
	// trapezoid points go in order - lower left, lower right, upper right, upper left
	float ComputeTrapezoidArea(D3DXVECTOR2 * Trapezoid);

	// Computes 2D analytic line equation 
	inline void ComputeLineEq(D3DXVECTOR2 * Point1, D3DXVECTOR2 * Point2, D3DXVECTOR3 * LineEq)
	{
		LineEq->x = Point2->y - Point1->y;
		LineEq->y = Point1->x - Point2->x;
		LineEq->z = - LineEq->x * Point1->x - LineEq->y * Point1->y;
	}

	// set epsilon
	inline void SetEpsilon(float _Epsilon)
	{
		Epsilon = _Epsilon;
	}

	// Get epsilon
	inline float GetEpsilon()
	{
		return Epsilon;
	}

	// Set focus region - sets the RegionPercentage from 0 to 1
	// focus region is from near plane to near plane + RegionPercentage * (Far - Near)
	// focus region makes better shadows for nearer objects - solves over-sampling wastage
	// RegionPercentage number must be smaller than RemapDistance number
	inline void SetFocusRegion(float _RegionPercentage)
	{
		RegionPercentage = _RegionPercentage;
	}

	// Get focus region
	inline float GetFocusRegion()
	{
		return RegionPercentage;
	}

	// set remap distance - number from -1 to 1
	// must be greater than RegionPercentage
	// defines, where (near plane + RegionPercentage * (Far - Near)) point will be remaped
	inline void SetRemapDistance(float _RemapDistance)
	{
		RemapDistance = _RemapDistance;
	}

	// Get remap distance
	inline float GetRemapDistance()
	{
		return RemapDistance;
	}
	
	// Generates remap distance for various angles between direction of light and camera for constant up vector
	// helper function
	int GenerateRemapDistances(char * FileName);
	

private:
	// Frustrum box vertices
    D3DXVECTOR3 BoxVerts[8];
	// Convex hull algorithm
	ConvexHull HullAlg;
	// Epsilon - used for testing coincidence of points
	float Epsilon;

	// Region percentage number used for computing projection point for trapezoid
	// defines focus region, which is from 
	//		near plane to near plane + RegionPercentage * (Far - Near)
	float RegionPercentage;

	// Remap distance - number from -1 to 1
	// defines, where (near plane + RegionPercentage * (Far - Near)) point will be remaped
	float RemapDistance;

	// Remap table - remap distances according to angle between light and camera direction
	float RemapTable[181];

	// provides debug information according to unsuccesful line intersection 
	void DumpLines(int TestNumber, D3DXVECTOR3 LeftLine, D3DXVECTOR3 BaseLine, D3DXVECTOR3 RightLine, D3DXVECTOR3 TopLine);


};



// better than having it in external text file
const float TSMRemapTable[179] = { 
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.343164f, -0.353906f, -0.365625f,
		-0.372949f, -0.380273f, -0.391016f, -0.407129f,
		-0.420313f, -0.429590f, -0.439844f, -0.454492f,
		-0.477930f, -0.482813f, -0.502344f, -0.523340f,
		-0.534570f, -0.569238f, -0.578516f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.600000f, -0.600000f, -0.600000f, -0.600000f,
		-0.569238f, -0.569238f, -0.534570f, -0.523340f,
		-0.499902f, -0.482813f, -0.473535f, -0.457910f,
		-0.443750f, -0.436426f, -0.417871f, -0.405176f,
		-0.392480f, -0.379297f, -0.379297f, -0.364648f,
		-0.354395f, -0.343164f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f, -0.332910f,
		-0.332910f, -0.332910f, -0.332910f
	}; 
