/*
	VertexNormals:		Class used to store normals of faces that shares any vertex.
						It also computes the resultant normal as an average of all adjacent faces (squares).
	Creation Date:		27.12.2003
	Last Update:		28.4.2006
	Author:				Roman Margold
*/

#pragma once

#include "stdafx.h"


#define MAX_COUNT_OF_NORMALS	8



class CVertexNormals
{
public:
				CVertexNormals() { Count = 0; };
	D3DXVECTOR3	GetNormal(); // returns the resultant average normal
	void		AddNormal( D3DXVECTOR3 * vec ); // adds a normal, it does nothing, when count of normals reaches the maximal count
	void		AddNormal( D3DXVECTOR3 * v1, D3DXVECTOR3 * v2, D3DXVECTOR3 * v3 ); // adds a normal, but computes it from vertices of triangle given in clockwise order
	void		SetNormal( D3DXVECTOR3 * vec ) { Normals[0] = *vec; Count = 1; };

private:
	D3DXVECTOR3	Normals[MAX_COUNT_OF_NORMALS];
	DWORD		Count; // count of normals
} ;

