/*
Rectangle inclusion class - computes rectangle inclusions on some restricted small rectangular area
by Pavel Celba
18. 3. 2006
*/

#pragma once

#include "..\\Globals\\GlobalsIncludes.h"
#include <map>
#include <vector>

// Computation area bound - helps keeping computation area reasonably small
#define COMPUTATION_AREA_BOUND 100

class CRectangleInclusion
{
public:
	// Returning intersection type definition
	// pair<UINT, UINT> - represents square of computation area
	// vector< ... > - represents all squares of computation area with certain number of inclusions
	// map< UINT, ...> - UINT is key saying which rectangles inclusion it is 
	// coding of key is sum of twos powered by index of inputed rectangle - SUM( 2 ^ IndexOfInputRect) 
	typedef std::map<UINT, std::vector< std::pair<UINT, UINT> > > RECTINCLUSIONS;

	// constructor
	CRectangleInclusion();

	// destructor
	~CRectangleInclusion();

	// Sets computation area
	HRESULT SetComputationArea(RECT & ComputationArea);

	// Compute rectangle inclusion
	// Returns vectors of rectangle inclusions sorted by maximal number of rectangle inclusions
	HRESULT ComputeInclusions(RECT * RectArray, UINT NumRects, RECTINCLUSIONS & RectInclusions);

	// Expands key - listing rectangle numbers in key
	void ExpandKey(UINT Key, std::vector<UINT> & RectNumbers, UINT RectMaxNumber = 9);

private:
	// Pointer to computation area
	UINT * Area;

	// Computation area rect
	RECT ComputationRect;

	// Width and height
	UINT ComputationWidth;
	UINT ComputationHeight;
};