/*
	BoundingBox:	This class is used to store a tree structure of blocks that surronds any graphic object.
	Creation Date:	26.10.2003
	Last Update:	27.10.2003
	Author:			Pavel Celba
*/

#pragma once

#include "stdafx.h"
#include "d3dx9.h"
#include "d3d9.h"
#include "windows.h"

using namespace std;

namespace graphic
{

	// forward declaration
	struct LessD3DXVECTOR3;

	// for searching all boxes
	const int ALL_BOXES = 1000000;


#ifndef DEFINED_GROBJECTBOUNDINGBOX
	// forward declaration if it is not already included
	class CGrObjectBoundingBox;
#endif   // DEFINED_GROBJECTBOUNDINGBOX


	// class representing a block (or a system of blocks) that surrounds any graphic object
	class CBoundingBox
	{
		friend class CGrObjectBoundingBox;

	public:
		typedef std::vector<CBoundingBox*>					BBVECTOR;
		typedef std::vector<CBoundingBox*>::iterator		BBVECTOR_ITER;
		typedef std::vector<CBoundingBox*>::const_iterator	BBVECTOR_CONSTITER;
		
		
		// methods
		CBoundingBox();
		~CBoundingBox();
		void			Init();
		
		// INTERSECTION FINDING SECTION
		
		// returns true, if two hierarchies of bounding boxes intersect, returns intersect vectors in HItVectors
		// the testing is provided by FastTest, PreciseTesting is done only, when one box has ForcePrecise = TRUE
		BOOL			HitTest(CBoundingBox * Box, int PrecisionLevel, D3DXVECTOR3 * HitVector1, D3DXVECTOR3 * HitVector2);
		
		BOOL			FastTest(FLOAT x, FLOAT y, FLOAT z, int precisionLevel); // returns true if a [x,y,z] point is inside of bounding box;
														// set precisionLevel to control max depth of tree level which will be used to test collision
		BOOL			FastTest(D3DXVECTOR3 point, int precisionLevel); // returns true if a point is inside of bounding box;
														// set precisionLevel to control max depth of tree level which will be used to test collision
		BOOL			FastTest(CBoundingBox *box, int precisionLevel, D3DXVECTOR3 * HitVector1, D3DXVECTOR3 * HitVector2); // returns true if a structure of boxes has a collision with another one
														// set precisionLevel to control max depth of tree level which will be used to test collision
		// Return true, if structure of boxes has collision one with another
		// Also returns HitVectors for top level bounding boxes
		// Is totally precise (except inclusion of one box in other - very unlikely to occur) )
		BOOL			PreciseTest(CBoundingBox * Box, int PrecisionLevel, D3DXVECTOR3 * HitVector1, D3DXVECTOR3 * HitVector2);

		// SAVING AND LOADING SECTION
		
		// Saves hierarchy to filename
		void Save(const string FileName) const;
		// Loads hierarchy from filename and returns pointer to top box
		static CBoundingBox * Load(const string FileName);

		// CURRENT BOX SETTING SECTION

		// sets the lower left corner of the a box
		inline void		SetLowerLeftCorner(const FLOAT x, const FLOAT y, const FLOAT z)
		{
			SetLowerLeftCorner(D3DXVECTOR3(x, y, z) );
		}
		// sets the lower left corner of the a box
		inline void		SetLowerLeftCorner(const D3DXVECTOR3 vec)
		{	
			Corners[0] = vec;
			RecomputeCorners();
			//RecomputePlanes(); 
		}
		// sets the upper right corner of the box
		inline void		SetUpperRightCorner(const FLOAT x, const FLOAT y, const FLOAT z)
		{
			SetUpperRightCorner(D3DXVECTOR3(x, y, z));
		}
		// sets the upper right corner of the box
		inline void		SetUpperRightCorner(const D3DXVECTOR3 vec)
		{
			Corners[7] = vec;
			RecomputeCorners();
			//RecomputePlanes();
		}
		inline D3DXVECTOR3 GetLowerLeftCorner() const { return Corners[0]; }; // returns lower left corner of whole box structure
		inline D3DXVECTOR3 GetUpperRightCorner() const { return Corners[7]; }; // returns upper right corner of whole box structure
		
		// sets box's transformation (world) matrix 
		inline void		SetWorldMatrix( const D3DXMATRIX *mat )	{
			Matrix = *mat;
			D3DXMatrixInverse(&InvMatrix, NULL, &Matrix);
		}
		
		// sets box's position
		inline void		SetPosition(const FLOAT x, const FLOAT y, const FLOAT z)
		{
			SetPosition(D3DXVECTOR3(x, y, z));
		}
		// sets box's position
		inline void		SetPosition(const D3DXVECTOR3 vec)
		{
			Changed = true;
			Position = vec;
		}
		
		 // sets box's rotation
		inline void		SetRotation(const FLOAT x, const FLOAT y, const FLOAT z)
		{
			SetRotation(D3DXVECTOR3(x, y, z));
		}
		// sets box's rotation
		inline void		SetRotation(const D3DXVECTOR3 vec)
		{
			Changed = true;
			Rotation = vec;
		}

		// sets box's scale factor
		inline void		SetScaling(const FLOAT x, const FLOAT y, const FLOAT z)
		{
			SetScaling(D3DXVECTOR3(x, y, z));
		}

		// sets box's scale factor
		inline void		SetScaling(const D3DXVECTOR3 vec)
		{
			Changed = true;
			Scaling = vec;
		}
		// sets box's scale factor same in all directions
		inline void		SetScaling(const FLOAT scale)
		{
			SetScaling(D3DXVECTOR3(scale, scale, scale));
		}
		// assigns a node as a child of this one
		inline void		AddNode(CBoundingBox *node)
		{
			pNodes.push_back(node);
			node->Parent = this; // Set parent
		}

		// Set force precision variable
		inline void SetForcePrecision(BOOL precision)
		{
			ForcePrecision = precision;
		}

		// Get position
		inline D3DXVECTOR3 GetPosition() const
		{
			return Position;
		}

		// Get rotation
		inline D3DXVECTOR3 GetRotation() const
		{
			return Rotation;
		}

		// Get scaling
		inline D3DXVECTOR3 GetScaling() const
		{
			return Scaling;
		}

		// Get number of children
		inline unsigned int NumChildren() const
		{
			return (unsigned int) pNodes.size();
		}

		// Get child number Num
		inline CBoundingBox * GetChild(int Num) const
		{
			if (Num >= (int) pNodes.size() )
				return 0; // error
			
			return pNodes[Num];
		}

		// returns count of nodes in whole hierarchy
		UINT GetNodesCount() const;
		
		// Get force precision variable
		inline BOOL GetForcePrecision() const
		{
			return ForcePrecision;
		}

	protected:
		// variables
		D3DXMATRIX		InvMatrix; // Inverse matrix
		D3DXMATRIX		Matrix; // Matrix
		D3DXVECTOR3		Position; // transforms
		D3DXVECTOR3		Rotation;
		D3DXVECTOR3		Scaling;
        D3DXVECTOR3		Corners[8]; // bounding box corners seted in initialisation part
		D3DXPLANE		BoundingBoxPlanes[6]; // bounding box planes
		CBoundingBox *	Parent; // pointer to parent box
		BBVECTOR		pNodes; // nodes
		BOOL Changed; // indicator, if data change and so matrix needs recomputing
		BOOL ForcePrecision; // Forces intersection computation for this box to be absolutely precise

		void Recompute(); // Recompute transformation matrix for point
		void RecomputeCorners(); // Recompute corners after corner has been set
		void RecomputePlanes(); // Recompute bounding box planes
		// Returns true, if bounding boxes intersect - fast, but inacurate
		BOOL BoxHit(CBoundingBox * Box1, CBoundingBox * Box2);
		// Return true, if bounding boxes intersect - precise, but slow
		BOOL PreciseBoxHit(CBoundingBox* Box1, CBoundingBox * Box2);
		// Returns true, if bounding boxes intersect - precise, return HitVector
		BOOL PreciseBoxHit(CBoundingBox * Box1, CBoundingBox * Box2, D3DXVECTOR3* HitVector1, D3DXVECTOR3 * HitVector2); 

		// Recomputes box corners coordinates representation 
		// into other box coordinate representation
		void CornersFromBoxToBox(CBoundingBox * FromBox, 
								 CBoundingBox * ToBox, 
								 D3DXVECTOR3 * InCorners, 
								 D3DXVECTOR3 * OutCorners);

		// Creates parametric line representation
		void ParamLine(D3DXVECTOR3 * Point1, D3DXVECTOR3 * Point2, D3DXVECTOR3 * Direction, D3DXVECTOR3 * Point);
		// Checks, if line intersect box in it's coordinates
		BOOL LineBoxIntersect(D3DXVECTOR3 & Point1, D3DXVECTOR3 & Point2, CBoundingBox * Box, set<D3DXVECTOR3, LessD3DXVECTOR3> & IntersectPoints);
		BOOL LineBoxIntersect(D3DXVECTOR3 & Point1, D3DXVECTOR3 & Point2, CBoundingBox * Box);
		
		// LineSideIntersectYZ - computes points of intersect for line and side parallel to YZ plane
		BOOL LineSideIntersectYZ(const D3DXVECTOR3 & LowerLeft, const D3DXVECTOR3 & UpperRight, const D3DXVECTOR3 & Direction,
								 const D3DXVECTOR3 & Point, set<D3DXVECTOR3, LessD3DXVECTOR3> & IntersectPoints);
		// LineSideIntersectXZ - computes points of intersect for line and side parallel to XZ plane
		BOOL LineSideIntersectXZ(const D3DXVECTOR3 & LowerLeft, const D3DXVECTOR3 & UpperRight, const D3DXVECTOR3 & Direction,
								 const D3DXVECTOR3 & Point, set<D3DXVECTOR3, LessD3DXVECTOR3> & IntersectPoints);
		// LineSideIntersectXY - computes points of intersect for line and side parallel to XY plane
		BOOL LineSideIntersectXY(const D3DXVECTOR3 & LowerLeft, const D3DXVECTOR3 & UpperRight, const D3DXVECTOR3 & Direction,
								 const D3DXVECTOR3 & Point, set<D3DXVECTOR3, LessD3DXVECTOR3> & IntersectPoints);

		// SAVE & LOAD SECTION
		// Saves current bounding box hierarchy to opened file stream
		void SaveBox(ofstream * Output) const;
		// Loads current bounding box hierarchy from opened file stream
		static CBoundingBox * LoadBox(ifstream * Input, CBoundingBox * Parent);

	} ;

} // end of namespace