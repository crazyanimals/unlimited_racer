/*
Trapezoidal shadow mapping implementation
by Pavel Celba
18. 3. 2005
*/

#include "TSM.h"

using namespace std;

TSM::TSM()
{
	BoxVerts[0] = D3DXVECTOR3(-1,-1, 0);
	BoxVerts[1] = D3DXVECTOR3(1,-1, 0);
	BoxVerts[2] = D3DXVECTOR3(1,1, 0);
	BoxVerts[3] = D3DXVECTOR3(-1,1, 0);
	BoxVerts[4] = D3DXVECTOR3(-1,-1, 1);
	BoxVerts[5] = D3DXVECTOR3(1,-1, 1);
	BoxVerts[6] = D3DXVECTOR3(1,1, 1);
	BoxVerts[7] = D3DXVECTOR3(-1,1, 1);

	// set epsilon to 1 cm of 200000 cms
	SetEpsilon(0.00001f);

	// set RegionPercentage to 20 percent
	RegionPercentage = 0.35f;
	
	// set RemapDistance to 80 percent
	RemapDistance = -0.6f;
}

// Inits trapezoidal shadow mapping
int TSM::Init()
{
	// Init remap table
/*	FILE * Input;
	
	Input = fopen("TSMRemapTable.txt", "r");
	if (!Input)
		ERRORMSG(-1, "TSM::Init()", "Unable to open remap table setting file.");
	
	for (int i = 1; i < 180; i++)
	{
		fscanf(Input, "%f\n", &RemapTable[i]);
		if (ferror(Input) )
			ERRORMSG(-1, "TSM::Init()", "Error while reading remap table setting file.");
	}

	fclose(Input);*/

	for ( int i = 1; i < 180; i++ ) RemapTable[i] = TSMRemapTable[i-1];

	RemapTable[0] = RemapTable[1];
	RemapTable[180] = RemapTable[179];

	return 0;
}

// Computes TSM post-projection matrix
int TSM::ComputeTSMMatrix(D3DXMATRIX * ProjMat, D3DXMATRIX * ViewMat, 
						  D3DXMATRIX * LightProjMat, D3DXMATRIX * LightViewMat, 
						  D3DXVECTOR3 * ViewDirection, D3DXVECTOR3 * LightDirection,
						  D3DXMATRIX * TSMMatrix)
{
  UINT i;
  float Temp;
  D3DXMATRIX InvViewProjMat;
  D3DXMATRIX ProjToLightProjMat;

  D3DXVECTOR3 TransFrustrum[8];
  D3DXVECTOR3 CenterLine[2] = { D3DXVECTOR3(0,0,0), D3DXVECTOR3(0,0,1) };

  vector<D3DXVECTOR2> Points;
  vector<D3DXVECTOR2> Hull;

  D3DXVECTOR3 TopLine, BaseLine;
  D3DXVECTOR2 NormalVector;

  D3DXVECTOR2 ProjectionPoint;
  D3DXVECTOR2 TopLineCenterPoint;
  D3DXVECTOR3 CenterLineEq;

  float L, D, E;
  float Near;

  D3DXVECTOR3 LeftLine, RightLine;
  float MaxAngleLeft = FLT_MAX;
  float MaxAngleRight = FLT_MAX;
  float CosAngle;
  D3DXVECTOR2 LineDirection;
  D3DXVECTOR2 CenterLineNormal;

  D3DXVECTOR2 Trapezoid[4];
  int Result;

  // Step 1 - Transform the eye's frustrum into the perspective space of the light
  D3DXMatrixMultiply(&InvViewProjMat, ViewMat, ProjMat);
  D3DXMatrixInverse(&InvViewProjMat, 0, &InvViewProjMat);
  D3DXMatrixMultiply(&ProjToLightProjMat, LightViewMat, LightProjMat);
  D3DXMatrixMultiply(&ProjToLightProjMat, &InvViewProjMat, &ProjToLightProjMat);

  TransformProjBox(&ProjToLightProjMat, TransFrustrum);

  // Step 2 - compute the center line, which passes through the centers of the near and
  //		  far plane
  D3DXVec3TransformCoordArray(CenterLine, sizeof(D3DXVECTOR3),
							  CenterLine, sizeof(D3DXVECTOR3),
							  &ProjToLightProjMat, 2);

  // Step 3 - calculate the 2D convex hull of transformed frustrum
  for (i = 0; i < 8; i++)
	  Points.push_back(D3DXVECTOR2(TransFrustrum[i].x, TransFrustrum[i].y));
  HullAlg.ComputeConvexHull(&Points, &Hull);

  // Step 4 - Calculate the top line, that is ortogonal to center line and
  //		  touches the boundary of the convex hull. Top line intersects
  //		  center line closer to the center of near plane than that of the
  //		  far plane. Calculate the base line - also orthogonal to center line and
  //          touches boundary of convex hull closer to the center of far plane.
  
  // Test for degenerate case - near and far plane are (almost) coincident in 2D,
  // so center line is (almost) point, also means, that bounding box has only 4 verts

 if ( sqrt( (CenterLine[0].x - CenterLine[1].x) * (CenterLine[0].x - CenterLine[1].x) +
			 (CenterLine[0].y - CenterLine[1].y) * (CenterLine[0].y - CenterLine[1].y) ) < Epsilon)
  {
#if _DEBUG
  
	  // test whether hull has only 4 points
	  if (Hull.size() != 4)
	  {
		  OUTMSG("WARNING: Convex hull in degenerate doesn't have 4 vertices", 1);
		  OUTMSG("WARNING: Dump of TSM setting and state of computation begins", 1);
		  OUTMSG("Hull points:", 1);
		  for (i = 0; i < Hull.size(); i++)
		  {
			OUTF(Hull[i].x, 1);
			OUTS(", ", 1);
			OUTF(Hull[i].y, 1);
			OUTSN("", 1);
		  }
		  OUTMSG("Center line:", 1);
		  for (i = 0; i < 2; i++)
		  {
			 OUTF(CenterLine[i].x ,1);
			 OUTS(", ", 1);
			 OUTF(CenterLine[i].y, 1);
			 OUTSN("", 1);
		  }
		  OUTMSG("Dump of TSM setting and state of computation ends", 1);

		  return -1;
	  }
#endif
	  

 //if (Hull.size() == 4)
 //{

     // Smallest bounding box is our choice
	  ComputeRotatedRectangleToBoxMatrix(&Hull[0], TSMMatrix);
	  return 0;
// }

}


  // find maximum and minimum in direction of center line
  NormalVector = D3DXVECTOR2(CenterLine[0].x - CenterLine[1].x,
							 CenterLine[0].y - CenterLine[1].y);

  D3DXVec2Normalize(&NormalVector, &NormalVector);

  float Max = -FLT_MAX;
  float Min = FLT_MAX;
  UINT MaxIndex, MinIndex;

  for (i = 0; i < Hull.size(); i++)
  {
	Temp = D3DXVec2Dot(&NormalVector, &Hull[i]);
	if (Temp > Max)
	{
		Max = Temp;
		MaxIndex = i;
	}
	if (Temp < Min)
	{
		Min = Temp;
		MinIndex = i;
	}
  }

  TopLine.x = BaseLine.x = NormalVector.x;
  TopLine.y = BaseLine.y = NormalVector.y;

  TopLine.z = - TopLine.x * Hull[MaxIndex].x - TopLine.y * Hull[MaxIndex].y;
  BaseLine.z = - BaseLine.x * Hull[MinIndex].x - BaseLine.y * Hull[MinIndex].y;

  // step 5 - calculate center of projection point q and then sidelines of trapezoid
  
 /*
  // calculate projection point by computing distance of near plane from that point
  // L = distance between top and base line
  // E = Remap point
  // D = RegionPercentage * L
*/

  D3DXVec3Normalize(ViewDirection, ViewDirection);
  D3DXVec3Normalize(LightDirection, LightDirection);

  L = abs(Max - Min);
  E = RemapTable[ (int) floor(acos(D3DXVec3Dot(LightDirection,  ViewDirection)) *180 / D3DX_PI)];
  //E = RemapDistance;
  D = RegionPercentage * L;

  Near = (L * D + L * D * E) / (L - 2 * D - L * E);

  ComputeLineEq(&D3DXVECTOR2(CenterLine[0].x, CenterLine[0].y) , 
				&D3DXVECTOR2(CenterLine[1].x, CenterLine[1].y), &CenterLineEq);

  Result = IntersectLines(&CenterLineEq, &TopLine, &TopLineCenterPoint);
  if (Result)
  {
	 OUTMSG("WARNING: TSM cannot intersect center line and top line.", 1);
	 return Result;
  }


  ProjectionPoint = D3DXVECTOR2(TopLineCenterPoint.x + NormalVector.x * Near, 
								TopLineCenterPoint.y + NormalVector.y * Near);

  // calculate side lines 
  // go through all points of hull and compute maximal angle to left (right) from the center line
  // and according to that compute side lines equations
  
  // Normal is pointing to the right when looking along center line from base to top
  CenterLineNormal = D3DXVECTOR2(NormalVector.y, - NormalVector.x);

  for (i = 0; i < Hull.size(); i++)
  {
    // LineDirection is pointing from Hull point to Projection point
    LineDirection = ProjectionPoint - Hull[i];
	D3DXVec2Normalize(&LineDirection, &LineDirection);
	CosAngle = D3DXVec2Dot( &LineDirection, &NormalVector);
	if (D3DXVec2Dot(&LineDirection, &CenterLineNormal) > 0)
	{
		// Hull[i] point is to the left when looking along center line from base to top
		if (MaxAngleLeft > CosAngle)
		{
			MaxAngleLeft = CosAngle;
			LeftLine.x = -LineDirection.y;
			LeftLine.y = LineDirection.x;
			LeftLine.z = -LeftLine.x * Hull[i].x - LeftLine.y * Hull[i].y;
		}

	}
	else
	{
		// Hull[i] point is to the right when looking along center line from base to top
		if (MaxAngleRight > CosAngle)
		{
			MaxAngleRight = CosAngle;
			RightLine.x = LineDirection.y;
			RightLine.y = -LineDirection.x;
			RightLine.z = -RightLine.x * Hull[i].x - RightLine.y * Hull[i].y;
		}
	}
  }

    // Step 6 - Compute trapezoid points (from left base, right base, right top to left top)
	
	ZeroMemory(Trapezoid, sizeof(D3DXVECTOR2)* 4); // trying to avoid the worst when unsuccesful intersection occurs
	Result = IntersectLines(&LeftLine, &BaseLine, &(Trapezoid[0]));
	if (Result)
	{
		DumpLines(1, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}
	Result = IntersectLines(&BaseLine, &RightLine, &(Trapezoid[1]));
	if (Result)
	{
		DumpLines(2, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}
	Result = IntersectLines(&RightLine, &TopLine, &(Trapezoid[2]));
	if (Result)
	{
		DumpLines(3, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}
	Result = IntersectLines(&TopLine, &LeftLine, &(Trapezoid[3]));
	if (Result)
	{
		DumpLines(4, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}

	// Step 7 - Compute transformation from trapezoid to [-1, -1] - [1, 1] box
	Result = ComputeTrapezoidToBoxMatrix(Trapezoid, TSMMatrix);
	if (Result != 0)
	{
		OUTMSG("WARNING: Unable to compute TSM matrix transformation.", 1);
		return -1;
	}

  return 0;
}

// Computes matrix for transforming 2D rotated rectangle into [-1, -1] - [1, 1] box
int TSM::ComputeRotatedRectangleToBoxMatrix(D3DXVECTOR2 * Rectangle, D3DXMATRIX * RectToBoxMat)
{
	D3DXMATRIX TransMat;
	D3DXMATRIX Temp;

	float Angle;

	// Translate center of rectangle to [0,0]
	D3DXVECTOR2 Center = (Rectangle[0] + Rectangle[2]) / 2;
	D3DXMatrixTranslation(&TransMat, -Center.x, - Center.y, 0);

	// Rotate rectangle to be aligned with x and y axes
	D3DXVECTOR2 u = D3DXVECTOR2(1, 0);
	D3DXVECTOR2 v = Rectangle[1] - Rectangle[0];
	D3DXVec2Normalize(&v, &v);
	Angle = - acos( D3DXVec2Dot(&u, &v) );

	D3DXMatrixRotationZ(&Temp, Angle);
	TransMat *= Temp;

	// scale rectangle to fit into [-1, -1] - [1, 1]
	u.x = D3DXVec2Length( &(Rectangle[0] - Rectangle[1]) );
	u.y = D3DXVec2Length( &(Rectangle[1] - Rectangle[2]) );

	if (u.x != 0 && u.y != 0)
	{
		D3DXMatrixScaling(&Temp, 2 / u.x, 2 / u.y, 1);
		TransMat *= Temp;
	}

	// assign result
	(*RectToBoxMat) = TransMat;

	return 0;
}

// 2D Line intersection
int TSM::IntersectLines(D3DXVECTOR3 * Line1, D3DXVECTOR3 * Line2, D3DXVECTOR2 * Intersection)
{
  float Temp;

  if (Line1->x == 0 && Line1->y == 0) return -2; // no line;
  if (Line2->x == 0 && Line2->y == 0) return -2; // no line;

  if (Line1->x != 0)
  {
	Temp = Line2->y * Line1->x - Line2->x * Line1->y;
	if (Temp != 0)
	{
		Intersection->y = (Line2->x * Line1->z - Line1->x * Line2->z) / Temp;
		Intersection->x = (-Line1->y * Intersection->y - Line1->z) / Line1->x;
		return 0;
	}
	// lines are parallel
	else
	{
		Temp = Line2->x * Line1->z - Line1->x * Line2->z;
		if (Temp == 0)
			return -1; // Coincident;
		else
			return 1; // No intersection;
	}

  }
  else
  {
	// first line is parallel with x-axes

	Temp = Line1->z * Line2->y - Line1->y * Line2->z;
    if (Line2->x == 0)
	{
		// Lines are parallel
		if (Temp == 0)
			return -1; // Coincident;
		else
			return 1; // No intersection;
	}
	else
	{
		Intersection->x = Temp / (Line2->x * Line1->y);
		Intersection->y = -Line1->z / Line1->y;
		return 0;	
	}

  }

}

// Computes matrix for transforming 2D trapezoid (base left, base right, top right, top left) 
// into [-1, -1] - [1, 1] box
// Transcripted from Fundamentals of texture mapping by Paul S. Heckbert with minor changes
int TSM::ComputeTrapezoidToBoxMatrix(D3DXVECTOR2 * Trapezoid, D3DXMATRIX * TrapezoidToBoxMat)
{
	D3DXMATRIX BoxTrapezoid;
	int Result;
	float Det;

	float u0 = -1;
	float v0 = -1;
	float u1 = 1;
	float v1 = 1;

	float du = u1-u0;
	float dv = v1-v0;
	
	// find mapping from unit square to trapezoid
	Result = ComputeUnitBoxToTrapezoidMatrix(Trapezoid, &BoxTrapezoid);
	if (Result != 0)
		return -1;

	// concatenate transform from uv rectangle to unit square
	BoxTrapezoid(0, 0) /= du;
	BoxTrapezoid(1, 0) /= dv;
	BoxTrapezoid(3, 0) -= BoxTrapezoid(0,0)*u0 + BoxTrapezoid(1, 0)*v0;
	BoxTrapezoid(0, 1) /= du;
	BoxTrapezoid(1, 1) /= dv;
	BoxTrapezoid(3, 1) -= BoxTrapezoid(0,1)*u0 + BoxTrapezoid(1, 1)*v0;
	BoxTrapezoid(0, 3) /= du;
	BoxTrapezoid(1, 3) /= dv;
	BoxTrapezoid(3, 3) -= BoxTrapezoid(0, 3)*u0 + BoxTrapezoid(1, 3)* v0;

	// Inverse matrix
	D3DXMatrixInverse(TrapezoidToBoxMat, &Det, &BoxTrapezoid);
	if (Det == 0)
		return -1;

	return 0;
}

// Computes matrix from [0,0] - [1,1] box to trapezoid
// Transcripted from Fundamentals of texture mapping by Paul S. Heckbert with minor changes
int TSM::ComputeUnitBoxToTrapezoidMatrix(D3DXVECTOR2 * Trapezoid, D3DXMATRIX * BoxToTrapezoidMat)
{
	float px, py;
	D3DXMATRIX Mat;
	

	D3DXMatrixIdentity(&Mat);

	px = Trapezoid[0].x - Trapezoid[1].x + Trapezoid[2].x - Trapezoid[3].x;
	py = Trapezoid[0].y - Trapezoid[1].y + Trapezoid[2].y - Trapezoid[3].y;

	
	if ( abs(px) < 1e-13 && abs(py) < 1e-13)
	{
		// affine mapping
		
		Mat(0,0) = Trapezoid[1].x - Trapezoid[0].x;
		Mat(1,0) = Trapezoid[2].x - Trapezoid[1].x;
		Mat(3,0) = Trapezoid[0].x;
		Mat(0,1) = Trapezoid[1].y - Trapezoid[0].y;
		Mat(1,1) = Trapezoid[2].y - Trapezoid[1].y;
		Mat(3,1) = Trapezoid[0].y;
		Mat(0,3) = 0.0f;
		Mat(1,3) = 0.0f;
		Mat(3,3) = 1.0f;

		(*BoxToTrapezoidMat) = Mat;
		return 0;
	}
	else
	{
		// projective mapping
		float dx1, dx2, dy1, dy2, del;

		dx1 = Trapezoid[1].x - Trapezoid[2].x;
		dx2 = Trapezoid[3].x - Trapezoid[2].x;
		dy1 = Trapezoid[1].y - Trapezoid[2].y;
		dy2 = Trapezoid[3].y - Trapezoid[2].y;
		del = dx1 * dy2 - dx2 * dy1;
		if (del == 0)
			return -1; // bad mapping;

		Mat(0,3) = (px * dy2 - dx2 * py) / del;
		Mat(1,3) = (dx1 * py - px * dy1) / del;
		Mat(3,3) = 1.0f;
		Mat(0,0) = Trapezoid[1].x - Trapezoid[0].x + Mat(0, 3) * Trapezoid[1].x;
		Mat(1,0) = Trapezoid[3].x - Trapezoid[0].x + Mat(1, 3) * Trapezoid[3].x;
		Mat(3,0) = Trapezoid[0].x;
		Mat(0,1) = Trapezoid[1].y - Trapezoid[0].y + Mat(0, 3) * Trapezoid[1].y;
		Mat(1,1) = Trapezoid[3].y - Trapezoid[0].y + Mat(1, 3) * Trapezoid[3].y;
		Mat(3,1) = Trapezoid[0].y;

		(*BoxToTrapezoidMat) = Mat;
		return 0;
	}
	
}

// Computes light matrixes for directional light, which includes entire view frustrum
// in AABB style
int TSM::ComputeDirectionalLightMatrixesFromFrustrum(D3DXMATRIX * ProjMat, D3DXMATRIX * ViewMat, D3DXVECTOR3 * LightDirection,
													D3DXMATRIX * LightProjMat, D3DXMATRIX * LightViewMat, float * ZRange)
{
	D3DXMATRIX TempMat;
	D3DXVECTOR3 TransFrustrum[8];
	D3DXVECTOR3 Min, Max;

	D3DXVECTOR3 UpVector;
	
	// Compute inverse of view-proj matrix
	D3DXMatrixMultiply(&TempMat, ViewMat, ProjMat);
	D3DXMatrixInverse(&TempMat, 0, &TempMat);

	// Compute directional light view matrix
	
	D3DXVec3Cross(&UpVector, LightDirection, &D3DXVECTOR3(0, 1, 0) );
	if (D3DXVec3Length(&UpVector) < 0.01)
	{
		UpVector = D3DXVECTOR3(1, 0, 0);
	}
	else
	{
		UpVector = D3DXVECTOR3(0,1, 0);
	}
	
	D3DXMatrixLookAtLH(LightViewMat, &D3DXVECTOR3(0,0,0), LightDirection, &UpVector);

	// Transform frustrum into view space of light
	D3DXMatrixMultiply(&TempMat, &TempMat, LightViewMat);
	TransformProjBox(&TempMat, TransFrustrum);

	// Compute bounding box
	D3DXComputeBoundingBox(TransFrustrum, 8, sizeof(D3DXVECTOR3), &Min, &Max);

	// Compute light projection matrix
	// The offset added to Min.z and Max.z ensures, that objects not seen by player camera, but still
	// casting shadows to camera frustrum will cast shadows correctly
	D3DXMatrixOrthoOffCenterLH(LightProjMat, Min.x, Max.x, Min.y, Max.y, Min.z - 30000, Max.z + 30000);

	// compute ZRange - needed to set bias correctly in shader or by matrix
	(*ZRange) = abs(Max.z - Min.z) + 60000;

	return 0;
}

// Computes trapezoid area
// trapezoid points go in order - lower left, lower right, upper right, upper left
float TSM::ComputeTrapezoidArea(D3DXVECTOR2 * Trapezoid)
{
	float Height;
		
	D3DXVECTOR3 BottomLine;
	D3DXVECTOR2 BottomLineDirection;
	float BottomLength;
	float TopLength;


	// Bottom line direction
	BottomLineDirection = Trapezoid[1] - Trapezoid[0];
	D3DXVec2Normalize(&BottomLineDirection, &BottomLineDirection);

	// Compute bottom line eq
	BottomLine.x = -BottomLineDirection.y;
	BottomLine.y = BottomLineDirection.x;
	BottomLine.z = -BottomLine.x * Trapezoid[0].x - BottomLine.y * Trapezoid[0].y;

	// Bottom line length
	BottomLength = D3DXVec2Length( &(Trapezoid[1] - Trapezoid[0] ) );

	// Compute height of trapezoid
	Height = BottomLine.x * Trapezoid[2].x + BottomLine.y * Trapezoid[2].y + BottomLine.z;

	// Top line Length
	TopLength = D3DXVec2Length( &(Trapezoid[2] - Trapezoid[3]) );

	return (BottomLength - TopLength) * Height / 2 + TopLength * Height;
}



// Computes TSM post-projection matrix with iteration on RemapPoint
int TSM::ComputeTSMMatrixIterate(D3DXMATRIX * ProjMat, D3DXMATRIX * ViewMat, 
						  D3DXMATRIX * LightProjMat, D3DXMATRIX * LightViewMat, 
						  D3DXMATRIX * TSMMatrix, float * Remap, float ItStep)
{
  UINT i;
  float Temp;
  D3DXMATRIX InvViewProjMat;
  D3DXMATRIX ProjToLightProjMat;

  D3DXVECTOR3 TransFrustrum[8];
  D3DXVECTOR3 CenterLine[2] = { D3DXVECTOR3(0,0,0), D3DXVECTOR3(0,0,1) };

  vector<D3DXVECTOR2> Points;
  vector<D3DXVECTOR2> Hull;

  D3DXVECTOR3 TopLine, BaseLine;
  D3DXVECTOR2 NormalVector;

  D3DXVECTOR2 ProjectionPoint;
  D3DXVECTOR2 TopLineCenterPoint;
  D3DXVECTOR3 CenterLineEq;

  float L, D, E;
  float Near;

  D3DXVECTOR3 LeftLine, RightLine;
  float MaxAngleLeft;
  float MaxAngleRight;
  float CosAngle;
  D3DXVECTOR2 LineDirection;
  D3DXVECTOR2 CenterLineNormal;

  D3DXVECTOR2 Trapezoid[4];
  int Result;

  // Step 1 - Transform the eye's frustrum into the perspective space of the light
  D3DXMatrixMultiply(&InvViewProjMat, ViewMat, ProjMat);
  D3DXMatrixInverse(&InvViewProjMat, 0, &InvViewProjMat);
  D3DXMatrixMultiply(&ProjToLightProjMat, LightViewMat, LightProjMat);
  D3DXMatrixMultiply(&ProjToLightProjMat, &InvViewProjMat, &ProjToLightProjMat);

  TransformProjBox(&ProjToLightProjMat, TransFrustrum);

  // Step 2 - compute the center line, which passes through the centers of the near and
  //		  far plane
  D3DXVec3TransformCoordArray(CenterLine, sizeof(D3DXVECTOR3),
							  CenterLine, sizeof(D3DXVECTOR3),
							  &ProjToLightProjMat, 2);

  // Step 3 - calculate the 2D convex hull of transformed frustrum
  for (i = 0; i < 8; i++)
	  Points.push_back(D3DXVECTOR2(TransFrustrum[i].x, TransFrustrum[i].y));
  HullAlg.ComputeConvexHull(&Points, &Hull);

  // Step 4 - Calculate the top line, that is ortogonal to center line and
  //		  touches the boundary of the convex hull. Top line intersects
  //		  center line closer to the center of near plane than that of the
  //		  far plane. Calculate the base line - also orthogonal to center line and
  //          touches boundary of convex hull closer to the center of far plane.
  
  // Test for degenerate case - near and far plane are (almost) coincident in 2D,
  // so center line is (almost) point, also means, that bounding box has only 4 verts

 if ( sqrt( (CenterLine[0].x - CenterLine[1].x) * (CenterLine[0].x - CenterLine[1].x) +
			 (CenterLine[0].y - CenterLine[1].y) * (CenterLine[0].y - CenterLine[1].y) ) < Epsilon)
  {
#if _DEBUG
  
	  // test whether hull has only 4 points
	  if (Hull.size() != 4)
	  {
		  OUTMSG("WARNING: Convex hull in degenerate doesn't have 4 vertices", 1);
		  OUTMSG("WARNING: Dump of TSM setting and state of computation begins", 1);
		  OUTMSG("Hull points:", 1);
		  for (i = 0; i < Hull.size(); i++)
		  {
			OUTF(Hull[i].x, 1);
			OUTS(", ", 1);
			OUTF(Hull[i].y, 1);
			OUTSN("", 1);
		  }
		  OUTMSG("Center line:", 1);
		  for (i = 0; i < 2; i++)
		  {
			 OUTF(CenterLine[i].x ,1);
			 OUTS(", ", 1);
			 OUTF(CenterLine[i].y, 1);
			 OUTSN("", 1);
		  }
		  OUTMSG("Dump of TSM setting and state of computation ends", 1);

		  return -1;
	  }
#endif
	  
	  // Smallest bounding box is our choice
	  ComputeRotatedRectangleToBoxMatrix(&Hull[0], TSMMatrix);
	  return 0;
}


  // find maximum and minimum in direction of center line
  NormalVector = D3DXVECTOR2(CenterLine[0].x - CenterLine[1].x,
							 CenterLine[0].y - CenterLine[1].y);

  D3DXVec2Normalize(&NormalVector, &NormalVector);

  float Max = -FLT_MAX;
  float Min = FLT_MAX;
  UINT MaxIndex, MinIndex;

  for (i = 0; i < Hull.size(); i++)
  {
	Temp = D3DXVec2Dot(&NormalVector, &Hull[i]);
	if (Temp > Max)
	{
		Max = Temp;
		MaxIndex = i;
	}
    if (Temp < Min)
	{
		Min = Temp;
		MinIndex = i;
	}
  }

  TopLine.x = BaseLine.x = NormalVector.x;
  TopLine.y = BaseLine.y = NormalVector.y;

  TopLine.z = - TopLine.x * Hull[MaxIndex].x - TopLine.y * Hull[MaxIndex].y;
  BaseLine.z = - BaseLine.x * Hull[MinIndex].x - BaseLine.y * Hull[MinIndex].y;

// REMAP POINT ITERATION PROCESS

int Iter = 0;
float MaxAreaRatio = -FLT_MAX;
D3DXMATRIX MaxAreaTSMMatrix;

D3DXMATRIX InvTSMMatrix;

D3DXVECTOR2 EPoint;
D3DXVECTOR3 ELine;


while (true)
{
  // step 5 - calculate center of projection point q and then sidelines of trapezoid
  
/* 
  // calculate projection point by computing distance of near plane from that point
  // L = distance between top and base line
  // E = Remap point
  // D = RegionPercentage * L
  //
  // Near = (1 - E) * L * D / (E*L - D)
  //

  L = abs(Max - Min);
  //L = 1;
  E = RemapDistance - Iter * ItStep;
  D = RegionPercentage * L;

  if (E*L - D <= 0) break;
  Iter++;

  Near = (1 - E) * L * D / (E*L - D);
*/

  L = abs(Max - Min);
  E = RemapDistance + Iter * ItStep;
  D = RegionPercentage * L;
	
  (*Remap) = E;
  
  if (E >= 1 - 2 * RegionPercentage) 
  {
	  break;
  }
  Iter++;

  Near = (L * D + L * D * E) / (L - 2 * D - L * E);

 ComputeLineEq(&D3DXVECTOR2(CenterLine[0].x, CenterLine[0].y) , 
				&D3DXVECTOR2(CenterLine[1].x, CenterLine[1].y), &CenterLineEq);

  Result = IntersectLines(&CenterLineEq, &TopLine, &TopLineCenterPoint);
  if (Result)
  {
	 OUTMSG("WARNING: TSM cannot intersect center line and top line.", 1);
	 return Result;
  }


  ProjectionPoint = D3DXVECTOR2(TopLineCenterPoint.x + NormalVector.x * Near, 
								TopLineCenterPoint.y + NormalVector.y * Near);
  // calculate side lines 
  // go through all points of hull and compute maximal angle to left (right) from the center line
  // and according to that compute side lines equations
  
  // Normal is pointing to the right when looking along center line from base to top
  CenterLineNormal = D3DXVECTOR2(NormalVector.y, - NormalVector.x);

 MaxAngleLeft = MaxAngleRight = FLT_MAX;

  for (i = 0; i < Hull.size(); i++)
  {
    // LineDirection is pointing from Hull point to Projection point
    LineDirection = ProjectionPoint - Hull[i];
	D3DXVec2Normalize(&LineDirection, &LineDirection);
	CosAngle = D3DXVec2Dot( &LineDirection, &NormalVector);
	if (D3DXVec2Dot(&LineDirection, &CenterLineNormal) > 0)
	{
		// Hull[i] point is to the left when looking along center line from base to top
		if (MaxAngleLeft > CosAngle)
		{
			MaxAngleLeft = CosAngle;
			LeftLine.x = -LineDirection.y;
			LeftLine.y = LineDirection.x;
			LeftLine.z = -LeftLine.x * Hull[i].x - LeftLine.y * Hull[i].y;
		}

	}
	else
	{
		// Hull[i] point is to the right when looking along center line from base to top
		if (MaxAngleRight > CosAngle)
		{
			MaxAngleRight = CosAngle;
			RightLine.x = LineDirection.y;
			RightLine.y = -LineDirection.x;
			RightLine.z = -RightLine.x * Hull[i].x - RightLine.y * Hull[i].y;
		}
	}
  }

    // Step 6 - Compute trapezoid points (from left base, right base, right top to left top)
	
	ZeroMemory(Trapezoid, sizeof(D3DXVECTOR2)* 4); // trying to avoid the worst when unsuccesful intersection occurs
	Result = IntersectLines(&LeftLine, &BaseLine, &(Trapezoid[0]));
	if (Result)
	{
		DumpLines(1, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}
	Result = IntersectLines(&BaseLine, &RightLine, &(Trapezoid[1]));
	if (Result)
	{
		DumpLines(2, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}
	Result = IntersectLines(&RightLine, &TopLine, &(Trapezoid[2]));
	if (Result)
	{
		DumpLines(3, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}
	Result = IntersectLines(&TopLine, &LeftLine, &(Trapezoid[3]));
	if (Result)
	{
		DumpLines(4, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}

	// Step 7 - Compute transformation from trapezoid to [-1, -1] - [1, 1] box
	Result = ComputeTrapezoidToBoxMatrix(Trapezoid, TSMMatrix);
	if (Result != 0)
	{
		OUTMSG("WARNING: Unable to compute TSM matrix transformation.", 1);
		return -1;
	}

	// Step 8 - find maximal focus region area to trapezoid area ratio
	//TrapezoidArea = ComputeTrapezoidArea(Trapezoid);

	/*
	D3DXMatrixInverse(&InvTSMMatrix, 0, TSMMatrix);

	D3DXVec2TransformCoord(&Trapezoid[0], &D3DXVECTOR2(-1.0f, (E - 0.5) * 2), &InvTSMMatrix);
	D3DXVec2TransformCoord(&Trapezoid[1], &D3DXVECTOR2(1.0f, (E - 0.5) * 2), &InvTSMMatrix);
	*/

	
	// compute E point
	/*
	EPoint = D3DXVECTOR2(TopLineCenterPoint.x - NormalVector.x * D, 
						 TopLineCenterPoint.y - NormalVector.y * D);

	// compute E line
	ELine.x = NormalVector.x;
	ELine.y = NormalVector.y;
	ELine.z = -ELine.x * EPoint.x - ELine.y * EPoint.y;

	Result = IntersectLines(&LeftLine, &ELine, &(Trapezoid[0]));
	if (Result)
	{
		DumpLines(1, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}
	Result = IntersectLines(&RightLine, &ELine, &(Trapezoid[1]));
	if (Result)
	{
		DumpLines(2, LeftLine, BaseLine, RightLine, TopLine);
		return -1;
	}
	
	
	D3DXVec2TransformCoordArray(Trapezoid, sizeof(D3DXVECTOR2), Trapezoid, sizeof(D3DXVECTOR2),
								TSMMatrix, 4);

	FocusRegionArea = ComputeTrapezoidArea(Trapezoid);

	if (FocusRegionArea  > MaxAreaRatio)
	{
		MaxAreaRatio = FocusRegionArea;
		MaxAreaRemapDistance = E;
		MaxAreaTSMMatrix = *TSMMatrix;
	}
	*/

	if ( D3DXVec2Length(&(Trapezoid[0] - Trapezoid[1])) < 4)  break;

}

  //(*TSMMatrix) = MaxAreaTSMMatrix;

  return 0;
}

// Generates remap distance for various angles between direction of light and camera for constant up vector
int TSM::GenerateRemapDistances(char * FileName)
{
	FILE * Output = 0;

	Output = fopen(FileName, "w");

	float Angle;
	float Remap;
	float Range;
	D3DXVECTOR3 LookAt;
	D3DXMATRIX ViewMat;
	D3DXMATRIX ProjMat;
	D3DXMATRIX LightViewMat;
	D3DXMATRIX LightProjMat;
	D3DXMATRIX Identity;
	D3DXMATRIX TSMMat;
	
	D3DXMatrixIdentity(&Identity);

	D3DXMatrixPerspectiveFovLH(&ProjMat,FIELD_OF_VIEW_Y, 1.33f,  NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE );
	
	// generate remap distance for all angles except singularity angles 0 and 180
	for (Angle = 1; Angle < 180; Angle++)
	{
		LookAt = D3DXVECTOR3(sin(Angle * D3DX_PI / 180),0, cos(Angle * D3DX_PI / 180) );
		D3DXMatrixLookAtLH(&ViewMat, &D3DXVECTOR3(0, 0, 0), &LookAt, &D3DXVECTOR3(0, 1, 0) );

		ComputeDirectionalLightMatrixesFromFrustrum(&ProjMat, &ViewMat, &D3DXVECTOR3(0,0,1), &LightProjMat, &LightViewMat, &Range);

		ComputeTSMMatrixIterate(&ProjMat, &ViewMat, &LightProjMat, &LightViewMat, &TSMMat, &Remap, 1.0f / 2048.0f);

		//fprintf(Output, "Angle: %f, Remap: %f\n", Angle, Remap);
		fprintf(Output, "%f\n", Remap);
	}

	fclose(Output);
	
	return 0;
}




// provides debug information according to unsuccesful line intersection 
void TSM::DumpLines(int TestNumber, D3DXVECTOR3 LeftLine, D3DXVECTOR3 BaseLine, D3DXVECTOR3 RightLine, D3DXVECTOR3 TopLine)
{
	OUTMSG("WARNING: Lines of trapezoid in TSM giving no one point intersection", 1);
	OUTMSG("Dump of line params begin:",1);
	OUTS("Test number -> what lines has no intersection: ", 1); OUTI(TestNumber, 1); OUTSN("", 1);
	OUTS("Left line: ", 1);
	OUTF(LeftLine.x, 1); OUTS(", ", 1); OUTF(LeftLine.y, 1); OUTS(", ", 1); OUTF(LeftLine.z, 1);
	OUTSN("", 1);
	OUTS("Base line: ", 1);
	OUTF(BaseLine.x, 1); OUTS(", ", 1); OUTF(BaseLine.y, 1); OUTS(", ", 1); OUTF(BaseLine.z, 1);
	OUTSN("", 1);
	OUTS("Right line: ", 1);
	OUTF(RightLine.x, 1); OUTS(", ", 1); OUTF(RightLine.y, 1); OUTS(", ", 1); OUTF(RightLine.z, 1);
	OUTSN("", 1);
	OUTS("Top line: ", 1);
	OUTF(TopLine.x, 1); OUTS(", ", 1); OUTF(TopLine.y, 1); OUTS(", ", 1); OUTF(TopLine.z, 1);
	OUTSN("", 1);
	OUTMSG("End of line params dump.", 1);
}


