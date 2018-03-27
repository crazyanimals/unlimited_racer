/*
RectInclusion.cpp - rectangle inclusion class implementation
by Pavel Celba
18. 3. 2006
*/

#include "RectInclusion.h"

using namespace std;

// constructor
CRectangleInclusion::CRectangleInclusion()
{
	Area = NULL;
	ComputationRect.left = 0;
	ComputationRect.right = 0;
	ComputationRect.bottom = 0;
	ComputationRect.top = 0;
}

// destructor
CRectangleInclusion::~CRectangleInclusion()
{
	SAFE_DELETE_ARRAY(Area);
}

// Sets computation area
HRESULT CRectangleInclusion::SetComputationArea(RECT & ComputationArea)
{
	UINT Width, Height;

	// Test whether computation area >= 0
	if (ComputationArea.left < 0 || ComputationArea.right < 0 ||
		ComputationArea.top < 0 || ComputationArea.bottom < 0)
		ERRORMSG(-1, "CRectangleInclusion::SetComputationArea()", "Computation area must be >= 0");

	Width = ComputationArea.right - ComputationArea.left + 1;
	Height = ComputationArea.bottom - ComputationArea.top + 1;

	// Test, against computation area bound
	if (Width > COMPUTATION_AREA_BOUND ||
		Height > COMPUTATION_AREA_BOUND)
		ERRORMSG(-1, "CRectangleInclusion::SetComputationArea()", "Computation area is too big.");

	// Set area
	ComputationRect = ComputationArea;
	ComputationRect.right += 1;
	ComputationRect.bottom += 1;
	
	// create new computation area
	// If the area is same size, we will not recreate it
	if (Area)
		if (Width == ComputationWidth && Height == ComputationHeight) return ERRNOERROR;
	
	SAFE_DELETE_ARRAY(Area);
	Area = new UINT[Width * Height];
	if (!Area)
		ERRORMSG(ERROUTOFMEMORY, "CRectangleInclusions::SetComputationArea()", "Unable to allocate area for rectangle inclusion computation.");

	ComputationWidth = Width;
	ComputationHeight = Height;

	return ERRNOERROR;
}

// Compute rectangle inclusion
// Returns vectors of rectangle inclusions sorted by maximal number of rectangle inclusions
HRESULT CRectangleInclusion::ComputeInclusions(RECT * RectArray, UINT NumRects, RECTINCLUSIONS & RectInclusions)
{
	UINT i;
	UINT RectKey;
	UINT x, y;
	RECTINCLUSIONS::iterator It;
	vector< pair<UINT, UINT> > PairVector;
	pair<UINT, UINT> Pair;
	
	if (!Area)
		ERRORMSG(-1, "CRectangleInclusion::ComputeInclusions()", "Computation area not set.");

	// Test whether rects in rect array are >= 0
	for (i = 0; i < NumRects; i++)
		if (RectArray[i].left < 0 || RectArray[i].right < 0 ||
			RectArray[i].top < 0 || RectArray[i].bottom < 0)
			ERRORMSG(-1, "CRectangleInclusion::ComputeInclusions()", "Rectangles in RectArray must be >= 0");

	// clear computation area
	ZeroMemory(Area, ComputationWidth * ComputationHeight * sizeof(UINT) );

	// add rectangle areas into computation area
	for (i = 0, RectKey = 1; i < NumRects; i++, RectKey = 1 << i)
		for (x = max((UINT)RectArray[i].left,(UINT) ComputationRect.left); 
			 x < min( (UINT) RectArray[i].right + 1,  (UINT) ComputationRect.right); x++)
			for (y = max((UINT) RectArray[i].top, (UINT) ComputationRect.top); 
				 y < min((UINT) RectArray[i].bottom + 1, (UINT) ComputationRect.bottom); y++)
			{
					_ASSERT( (y - ComputationRect.top) * ComputationWidth + (x - ComputationRect.left) < ComputationWidth * ComputationHeight );
					Area[ (y - ComputationRect.top) * ComputationWidth + (x - ComputationRect.left) ] += RectKey;
			}

	// construct result RECTINCLUSION map
	for (y = 0; y < ComputationHeight; y++)
		for (x = 0; x < ComputationWidth; x++)
		{
			Pair = pair<UINT, UINT>( (UINT) ComputationRect.left + x, (UINT) ComputationRect.top + y);
			It = RectInclusions.find(Area[y * ComputationWidth + x] );
			if (It != RectInclusions.end() )
			{
				It->second.push_back( Pair );
			}
			else
			{
				PairVector.clear();
				PairVector.push_back( Pair );
				RectInclusions.insert( pair<UINT, vector< pair<UINT, UINT> > >( Area[y * ComputationWidth + x], PairVector ) );
			}	
		}
		
	return ERRNOERROR;
}

// Expands key - listing rectangle numbers in key
void CRectangleInclusion::ExpandKey(UINT Key, vector<UINT> & RectNumbers, UINT RectMaxNumber)
{
	UINT i;

	for (i = 0; i < RectMaxNumber; i++)
	{
		if (Key & 1) RectNumbers.push_back(i);
		Key = Key >> 1;
	}
}