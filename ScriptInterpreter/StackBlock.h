/*
	CStackBlock:		This class is used to store a block of variables. This represents the block
						of a script that has its own variable declaration.
						Variables declared in this block shadows formerly defined variables and dies
						with the end of a block , so when this block is being destructed.

	Creation Date:		22.4.2004
	Last Update:		22.4.2004
	Author:				Roman Margold

*/


#pragma once


#include "stdafx.h"
#include "ScriptInterpreterTools.h"
#include "variables.h"


//////////////////////////////////////////////////////////////////////////////////////////////
// class that represents one block of program in stack
//////////////////////////////////////////////////////////////////////////////////////////////
class CStackBlock
{
public:
					~CStackBlock(); // frees stored variables
	
	VARIABLES		Variables; // hash table of variables
	LINES_ITERATOR	Jump; // reference to the first line of a block (first inside)
	LINES_ITERATOR	End; // reference to the list line of a block (})
	UINT			Type; // type of block - defined by SBT macros
	BOOL			WasTrue; // specifies the result of a condition when this block is "if" clause

	BOOL			VariableExists( CString varName ); // returns true if this variable exists in this block
	CString			GetVariableString( CString varName ); // returns a value of a variable
	int				GetVariableInt( CString varName ); // returns a value of a variable
	FLOAT			GetVariableFloat( CString varName ); // returns a value of a variable

} ;


typedef std::vector<CStackBlock *>					BLOCKSSTACK;
typedef std::vector<CStackBlock *>::iterator		BLOCKSSTACK_ITERATOR;



