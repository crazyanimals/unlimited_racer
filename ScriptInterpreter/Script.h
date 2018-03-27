/*
	Script:				This class represents a script and provides through its methods the most important
						functionality like script loading and Interpreting.

	Creation Date:		26.2.2004
	Last Update:		16.5.2005
	Author:				Roman Margold

*/


#pragma once

#include "stdafx.h"
#include "ScriptInterpreterTools.h"
#include "variables.h"
#include "StackBlock.h"


//////////////////////////////////////////////////////////////////////////////////////////////
// class representing whole script source code
//////////////////////////////////////////////////////////////////////////////////////////////
class CScript
{
public:
					CScript();
					~CScript();

	HRESULT			Load( LPCTSTR fileName );
	HRESULT			Interpret( HRESULT (CALLBACK *EFC) ( CString &, int, CString [] ), CString &returnVal );

	HRESULT			AddLine( CString str, int LineNumber, bool &inComment );
	bool			InterpretVersionNeeded( int &majVer, int &minVer );

	HRESULT			InterpretBlock(	BLOCKSSTACK &stack, 
									HRESULT (CALLBACK *EFC) ( CString &, int, CString [] ),
									UINT &uiExitType,
									UINT uiDeepness,
									CString &returnVal );

	HRESULT			Evaluate(	TOKENS &inTokens,
								TOKENS_ITERATOR	i1,
								TOKENS_ITERATOR	i2,
								BLOCKSSTACK &stack,
								CString &retVal );

	HRESULT			EvaluateNonBracketed(	TOKENS &inTokens, 
											TOKENS_ITERATOR i1,
											TOKENS_ITERATOR i2,
											BLOCKSSTACK &stack, 
											CString &retVal );

	HRESULT			EvaluateFunction(		CString func, 
											CString &retVal,
											UINT	paramCount,
											CString param1, 
											CString param2 = "", 
											CString param3 = "" );

private:	
	int				DetermineLineType( CScriptLine *line );
	bool			RemoveComments( CString &str, bool inComment );
	bool			IsIdentifierChar( char znak, int pos );
	bool			IsSymbolChar( char znak );
	bool			IsOperatorChar( char znak );
	CVariable		*GetVariable( BLOCKSSTACK &stack, CString varName ); // returns a pointer to a variable or NULL when variable not found
	OPERATOR		GetOperator( CString str );
	HRESULT			ApplyBinaryOperator( LPCTSTR op, CString val1, CString val2, CString &retVal );
	CStackBlock		*CreateProgramBlock(	BLOCKSSTACK &stack, 
											LINES_ITERATOR firstLine,
											LINES_ITERATOR lastLine,
											UINT Type,
											BOOL WasTrue = TRUE );
	void			RemoveProgramBlock( BLOCKSSTACK &stack );
	LINES_ITERATOR	FindEndOfBlock( LINES_ITERATOR firstLine );

	// variables
	LINES			Lines; // lines of the script
	bool			CycleInOneBlock;	
							/*	CycleInOneBlock specifies whether the cycle (for or while) will be evaluated as one block
								if this is false, the cycle block is created and deleted in each iteration; 
								true is default value */
	bool			IgnoreEFCError; 
							/*	IgnoreEFCError specifies whether the EFC error should terminate the script interpreting
								true means that the script interpratation runs although there was an error in EFC;
								false is the default values; this should be changed by ignoreEFCerror directive */

} ;

