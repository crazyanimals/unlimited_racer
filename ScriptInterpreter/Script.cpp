#include "stdafx.h"

#include "Script.h"


//////////////////////////////////////////////////////////////////////////////////////////////
//
// script constructor, initializing some structures
//
//////////////////////////////////////////////////////////////////////////////////////////////
CScript::CScript()
{
	Lines = LINES();

	CycleInOneBlock = true;
	IgnoreEFCError = false;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// script destructor, frees some data
//
//////////////////////////////////////////////////////////////////////////////////////////////
CScript::~CScript()
{
	LINES_ITERATOR	iter;

	for ( iter = Lines.begin(); iter != Lines.end(); iter++ )
	{
		iter->Tokens.clear();
	}
	Lines.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// This method interprets a script stored in a specified file, ExternFunc is a pointer
// to function that is called every time an "ExternalFunctionCall" is used in source code;
//
// Returns a zero value when no error occurs, otherwise, this returns a nonzero value,
// for further information, try to check InterpretingError object.
// Returns a return value of a script by returnVal parameter.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScript::Interpret( HRESULT (CALLBACK *EFC) ( CString &, int, CString [] ), CString &returnVal )
{
	BLOCKSSTACK				BlockStack;
	CStackBlock				*pBlock = NULL;
	UINT					uiExited;
	HRESULT					hr;
	CString					ret;


	returnVal = ""; // undefine return value


	// create new program Block
	pBlock = CreateProgramBlock( BlockStack, Lines.begin(), Lines.end() - 1, SBT_UNDEFINED, FALSE );
	if ( !pBlock ) RETURN_ERROR( ERR_NOTENOUGHMEMORY, 0 );
	

	hr = InterpretBlock( BlockStack, EFC, uiExited, 1, returnVal );
	if ( hr ) return hr;

	
	for ( BLOCKSSTACK_ITERATOR i = BlockStack.begin(); i != BlockStack.end(); i++ ) 
		if ( *i ) delete (*i);

	BlockStack.clear();


	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// FindEndOfBlock finds a line that contains right(end) bracket of a block.
//   Obtains a firstLine parameter, that is a line iterator that points to a line containing the
//   left(begin) bracket of a block.
// 
// This only counts the left and right brackets and ends when the count is same.
//
// Returns a Lines.end() value (that's invalid iterator) when the last line was reached but 
//   not enough right brackets found, or returns a valid iterator that points to the line 
//   with appropriate right bracket.
//
//////////////////////////////////////////////////////////////////////////////////////////////
LINES_ITERATOR CScript::FindEndOfBlock( LINES_ITERATOR firstLine )
{
	LINES_ITERATOR	lineIter = firstLine;
	UINT			bracketCount = 0;

	while ( lineIter != Lines.end() )
	{
		if ( lineIter->Type == SLT_BEGIN ) bracketCount++;
		if ( lineIter->Type == SLT_END ) bracketCount--;
		if ( !bracketCount ) return lineIter;

		lineIter++;
	}

	return lineIter;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// This interprets a block - that is a segment of a script separated by { }.
//
// Info about this block, e.g. first block line after { bracket, is stored in the stack
// parameter as the most top record.
// In returnVal parameter block returns possibly evaluated value.
// If the script is terminated by a command, bExited is set to true and returnVal contains
// possible exit code.
//
// uiDeepness parameter is specifying the recursion deepness, that is count of blocks that
// are actually nested. If this value is 1, the actual block is the main script block, the
// value 2 means a block inside a main script and so on...
//
// Method returns a zero value when no error occurs, otherwise, it returns a nonzero value.
// When an error occurs in external function, the returned value is the value returned by
// that function, although, the InterpretingError object is filled with ERR_EFC error code.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScript::InterpretBlock(	BLOCKSSTACK &stack, 
									HRESULT (CALLBACK *EFC) ( CString &, int, CString [] ),
									UINT &uiExitType,
									UINT uiDeepness,
									CString &returnVal )
{
	CStackBlock				*pBlock = NULL, *pActBlock;
	CVariable				*pVariable = NULL, var( "var" );
	TOKENS					*pTokens;
	LINES_ITERATOR			lineIter, lineIter2;
	TOKENS_ITERATOR			tokenIter, tokenIter2;
	LINES::size_type		Size;
	VARIABLES_ITERATOR		varIter;
	VARIABLES_PAIR			varPair;
	VARIABLES_RETURN_PAIR	varRetPair;
	int						lineNo;
	HRESULT					hr;
	CString					ret;
	UINT					uiNeedExit;



	returnVal = ""; // undefine return value
	uiExitType = ET_NORMAL;


	// obtain a pointer to this program Block
	pActBlock = *(stack.end() - 1);



	// main interpreting cycle through lines of block, starting the first one
	for ( lineIter = pActBlock->Jump; true; lineIter++ ) // this will end when the end of block is found
	{
		if ( lineIter == Lines.end() ) // the end of script
		{
			if ( uiDeepness > 1 ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, (Lines.end() - 1)->Number ); // the block is unfinished
			uiExitType = ET_RETURN;
			RETURN_ERROR( ERR_RETURNVALUEEXPECTED, (--Lines.end())->Number ); // end of file without a return command
		}

		lineNo = lineIter->Number;
		pTokens = &(lineIter->Tokens); // store reference to tokens list
		Size = pTokens->size(); // count of tokens on this line
		

		// ------------------------------------
		// line is a variable definition
		// ------------------------------------
		if ( lineIter->Type == SLT_VAR )
		{
			bool	newVar = true; // defines whether a new variable identifier is expected

			if ( Size < 2 ) RETURN_ERROR( ERR_IDENTIFIEREXPECTED, lineNo ); // this line must have at least 2 tokens
			
			// add all variables separated by a comma to stackblock
			for (tokenIter = pTokens->begin(), tokenIter++; tokenIter != pTokens->end(); tokenIter++)
			{
				if ( newVar && tokenIter->Type == STT_IDENTIFIER ) // new variable definition
				{
					pVariable = new CVariable( tokenIter->Content );
					if ( !pVariable )
						RETURN_ERROR( ERR_NOTENOUGHMEMORY, lineNo );

					varRetPair = pActBlock->Variables.insert( VARIABLES_PAIR( tokenIter->Content, pVariable ) );

					if ( !varRetPair.second ) // variable already exists in this block
					{
						delete pVariable;
						RETURN_ERROR( ERR_ALREADYDEFINED, lineNo );
					}

					newVar = false;
				}
				else if ( !newVar && tokenIter->Type == STT_SYMBOL && tokenIter->Content == "," )
				{
					newVar = true;
				}
				else
				{
					RETURN_ERROR( ERR_IDENTIFIEREXPECTED, lineNo );
				}
			}

			continue;
		} // end of VAR clause


		// ------------------------------------
		// line is a condition (an IF clause)
		// ------------------------------------
		if ( lineIter->Type == SLT_IF )
		{
			if ( Size < 2 ) RETURN_ERROR( ERR_UNEXPECTEDENDOFLINE, lineNo ); // this line must have at least 2 tokens

			// evaluate an expression
			hr = Evaluate( *pTokens, pTokens->begin() + 1, pTokens->end() - 1, stack, ret );
			if ( hr ) RETURN_ERROR( hr, lineNo );

			// look to the next line and finish the IF clause, if that line is not a THEN or ELSE clause
			lineIter++;
			if ( lineIter == Lines.end() ) continue;
			if ( lineIter->Type != SLT_THEN && lineIter->Type != SLT_ELSE )
			{
				lineIter--;
				continue;
			}

			var.SetValue( ret ); // var is containing a result of a condition 

			
			if ( var.GetInt() ) // the condition was evaluated as "TRUE"
			{
				// process the THEN clause
				if ( lineIter->Type == SLT_THEN )
				{
					if ( lineIter->Tokens.size() > 1 ) // THEN clause is a single command
					{
						hr = Evaluate(	lineIter->Tokens, 
										lineIter->Tokens.begin() + 1, 
										lineIter->Tokens.end() - 1, 
										stack, ret );

						if ( hr ) RETURN_ERROR( hr, lineIter->Number );

						lineIter++; // mark the line with possible ELSE clause
					}
					else // THEN clause is a block of commands
					{
						lineIter++; // move to the "left bracket" line
						if ( lineIter == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, (lineIter - 1)->Number );
						if ( lineIter->Type != SLT_BEGIN ) RETURN_ERROR( ERR_WRONGSYNTAX, lineIter->Number );

						// find the end of a block
						lineIter2 = FindEndOfBlock( lineIter );
						if ( lineIter2 == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, lineIter->Number );
						lineIter++; // move to a line just after the left bracket
						
						// create new program block
						pBlock = CreateProgramBlock( stack, lineIter, lineIter2, SBT_THEN, var.GetInt() );
						if ( !pBlock ) RETURN_ERROR( ERR_NOTENOUGHMEMORY, lineIter->Number );
						
						// interpret the THEN block
						hr = InterpretBlock( stack, EFC, uiNeedExit, uiDeepness + 1, ret );
						RemoveProgramBlock( stack );
						if ( hr ) return hr;
						if ( uiNeedExit ) // an exit command was called from inside of a block
						{
							returnVal = ret;
							uiExitType = uiNeedExit;
							return ERR_NOERROR;
						}

						lineIter = lineIter2 + 1; // mark the line with possible ELSE clause
					}
				}
				
				// skip the ELSE clause
				if ( lineIter->Type == SLT_ELSE )
				{
					// single-command ELSE clause doesn't need to be processed, the lineIterator
					// will stay on that line and will be incremented at the end of this cycle iteration
					if ( lineIter->Tokens.size() == 1 ) // ELSE clause contains a block of commands
					{
						lineIter++;
						if ( lineIter == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, (lineIter - 1)->Number );
						if ( lineIter->Type != SLT_BEGIN ) RETURN_ERROR( ERR_WRONGSYNTAX, lineIter->Number );

						// find end of block
						lineIter2 = FindEndOfBlock( lineIter );
						if ( lineIter2 == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, lineIter->Number );
						lineIter = lineIter2;
					}
				}
				else lineIter--; // the lineIter must point to the actual line at the end of this iteration
			}
			else // the condition was evaluated as "FALSE"
			{
				// skip the THEN clause
				if ( lineIter->Type == SLT_THEN )
				{
					if ( lineIter->Tokens.size() > 1 ) lineIter++; // move to the first line after THEN clause
					else // THEN clause is a block of commands
					{
						lineIter++;
						if ( lineIter == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, (lineIter - 1)->Number );
						if ( lineIter->Type != SLT_BEGIN ) RETURN_ERROR( ERR_WRONGSYNTAX, lineIter->Number );

						// find end of block
						lineIter2 = FindEndOfBlock( lineIter );
						if ( lineIter2 == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, lineIter->Number );
						lineIter = lineIter2 + 1; // move to the first line after THEN clause
					}
				}

				// process the ELSE clause
				if ( lineIter->Type == SLT_ELSE )
				{
					if ( lineIter->Tokens.size() > 1 ) // ELSE clause is a single command 
					{
						hr = Evaluate(	lineIter->Tokens, 
										lineIter->Tokens.begin() + 1, 
										lineIter->Tokens.end() - 1, 
										stack, ret );

						if ( hr ) RETURN_ERROR( hr, lineIter->Number );
					}
					else // ELSE clause is a block of commands
					{
						lineIter++; // move to the "left bracket" line
						if ( lineIter == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, (lineIter - 1)->Number );
						if ( lineIter->Type != SLT_BEGIN ) RETURN_ERROR( ERR_WRONGSYNTAX, lineIter->Number );

						// find the end of a block
						lineIter2 = FindEndOfBlock( lineIter );
						if ( lineIter2 == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, lineIter->Number );
						lineIter++; // move to a line just after the left bracket
						
						// create new program block
						pBlock = CreateProgramBlock( stack, lineIter, lineIter2, SBT_THEN, var.GetInt() );
						if ( !pBlock ) RETURN_ERROR( ERR_NOTENOUGHMEMORY, lineIter->Number );
						
						// interpret this ELSE block
						hr = InterpretBlock( stack, EFC, uiNeedExit, uiDeepness + 1, ret );
						RemoveProgramBlock( stack );
						if ( hr ) return hr;
						if ( uiNeedExit ) // an Exit command was called from inside of a block
						{
							returnVal = ret;
							uiExitType = uiNeedExit;
							return ERR_NOERROR;
						}

						lineIter = lineIter2;
					}
				}
				else lineIter--; // the lineIter must point to the actual line at the end of this iteration
			}
			
			continue;
		}  // end of IF clause


		// ------------------------------------
		// line is a FOR cycle
		// ------------------------------------
		if ( lineIter->Type == SLT_FOR )
		{
			if ( Size < 4 ) RETURN_ERROR( ERR_UNEXPECTEDENDOFLINE, lineNo ); // this line must have at least 4 tokens
			
			// create a new variable
			tokenIter = pTokens->begin() + 1;
			if ( !(tokenIter->Type & STT_IDENTIFIER) ) RETURN_ERROR( ERR_IDENTIFIEREXPECTED, lineNo );
			
			// allocate memory for a variable
			pVariable = new CVariable( tokenIter->Content );
			if ( !pVariable ) RETURN_ERROR( ERR_NOTENOUGHMEMORY, lineNo );
            
			// insert a variable to actual program block
			varRetPair = pActBlock->Variables.insert( VARIABLES_PAIR( tokenIter->Content, pVariable ) );
			if ( !varRetPair.second ) delete pVariable; // just use the existing one, if the variable already exists
			pVariable = varRetPair.first->second;

			tokenIter++;
			if ( !(tokenIter->Type & STT_SYMBOL) || (tokenIter->Content != ",") ) RETURN_ERROR( ERR_WRONGSYNTAX, lineNo );
			tokenIter++;
			
			
			// find next comma or end of line
			UINT		uiHelpCount = 0;
			bool		found = false;
			int			iterationCount;
			
			while ( tokenIter != pTokens->end() )
			{
				if ( tokenIter->Type & STT_SYMBOL && tokenIter->Content == "," ) 
				{
					found = true;
					break;
				}
				tokenIter++;
				uiHelpCount++;
			}

			// compute the count of iterations required
			if ( !uiHelpCount ) RETURN_ERROR( ERR_EXPRESSIONEXPECTED, lineNo );
			hr = Evaluate( *pTokens, pTokens->begin() + 3, tokenIter - 1, stack, ret );
			if ( hr ) RETURN_ERROR( hr, lineNo );
			var.SetValue( ret );
			iterationCount = var.GetInt();

			
			// execute the FOR cycle
			if ( found ) // the body of FOR cycle is single command
			{
				tokenIter++;
				if ( tokenIter == pTokens->end() ) continue; // this is an empy for cycle
				
				pVariable->SetValue( 1 );
				while ( pVariable->GetInt() <= iterationCount )
				{
					hr = Evaluate( *pTokens, tokenIter, pTokens->end() - 1, stack, ret );
					if ( hr ) RETURN_ERROR( hr, lineNo );
					pVariable->SetValue( pVariable->GetInt() + 1 );
				}
			}
			else // the body of FOR cycle is a block
			{
				lineIter++;
				if ( lineIter == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, (lineIter - 1)->Number );
				if ( lineIter->Type != SLT_BEGIN ) RETURN_ERROR( ERR_WRONGSYNTAX, lineIter->Number );

				// find the end of a block
				lineIter2 = FindEndOfBlock( lineIter );
				if ( lineIter2 == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, lineIter->Number );
				lineIter++; // move to a line just after the left bracket

				// create new program block if the for cycle is runned in one block
				if ( CycleInOneBlock )
				{
					pBlock = CreateProgramBlock( stack, lineIter, lineIter2, SBT_FOR );
					if ( !pBlock ) RETURN_ERROR( ERR_NOTENOUGHMEMORY, lineIter->Number );
				}

				
				// interpret whole FOR cycle iterating block
				pVariable->SetValue( 1 );
				while ( pVariable->GetInt() <= iterationCount )
				{
					// create new program block if the for cycle is runned separate blocks
					if ( !CycleInOneBlock )
					{
						pBlock = CreateProgramBlock( stack, lineIter, lineIter2, SBT_FOR );
						if ( !pBlock ) RETURN_ERROR( ERR_NOTENOUGHMEMORY, lineIter->Number );
					}

					hr = InterpretBlock( stack, EFC, uiNeedExit, uiDeepness + 1, ret );
					if ( hr )
					{
						RemoveProgramBlock( stack );
						return hr;
					}

					if ( uiNeedExit ) // an exit command was called from inside of a block
					{
						if ( uiNeedExit == ET_CONTINUE ) goto FOR_CONTINUE;
						if ( uiNeedExit == ET_BREAK )
						{
							if ( !CycleInOneBlock ) RemoveProgramBlock( stack );
							break;
						}
						returnVal = ret;
						uiExitType = uiNeedExit;
						return ERR_NOERROR;
					}

				FOR_CONTINUE:	
					pVariable->SetValue( pVariable->GetInt() + 1 );
					if ( !CycleInOneBlock ) RemoveProgramBlock( stack );
				}
				
				if ( CycleInOneBlock ) RemoveProgramBlock( stack );				
				lineIter = lineIter2; // the lineIter must point to the actual line at the end of this iteration 
			}
			continue;
		}  // end of FOR cycle


		// ------------------------------------
		// line is a WHILE cycle
		// ------------------------------------
		if ( lineIter->Type == SLT_WHILE )
		{
			if ( Size < 2 ) RETURN_ERROR( ERR_UNEXPECTEDENDOFLINE, lineNo ); // this line must have at least 2 tokens
			
			bool	dvojtecka = false;
			int		tokensCount = 0;
			
			tokenIter = pTokens->begin() + 1;
			
			// find the ":" character or the end of a line 
			while ( tokenIter != pTokens->end() )
			{
				if ( tokenIter->Type & STT_SYMBOL && tokenIter->Content == ":" )
				{
					dvojtecka = true;
					break;
				}
				tokenIter++;
				tokensCount++;
			}

			if ( !tokensCount ) RETURN_ERROR( ERR_WRONGSYNTAX, lineNo );

			tokenIter2 = tokenIter - 1;
			tokenIter++;
			
			
			// the body of the WHILE cycle is a single command
			if ( dvojtecka )
			{
				// evaluate the condition
				hr = Evaluate( *pTokens, pTokens->begin() + 1, tokenIter2, stack, ret );
				if ( hr ) RETURN_ERROR( hr, lineNo );
				var.SetValue( ret );
				
				while ( var.GetInt() )
				{
                    if ( tokenIter != pTokens->end() )
					{
						hr = Evaluate( *pTokens, tokenIter, pTokens->end() - 1, stack, ret );
						if ( hr ) RETURN_ERROR( hr, lineNo );
					}
				
					// evaluate the condition
					hr = Evaluate( *pTokens, pTokens->begin() + 1, tokenIter2, stack, ret );
					if ( hr ) RETURN_ERROR( hr, lineNo );
					var.SetValue( ret );
				}
				
				continue;
			}


			// the body of the WHILE cycle is a made up of a block
			lineIter++;
			if ( lineIter == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, lineNo );
			if ( lineIter->Type != SLT_BEGIN ) RETURN_ERROR( ERR_WRONGSYNTAX, lineIter->Number );

			// find the end of a block
			lineIter2 = FindEndOfBlock( lineIter );
			if ( lineIter2 == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, lineIter->Number );
			lineIter++; // move to a line just after the left bracket

			// create new program block if the while cycle is runned in one block
			if ( CycleInOneBlock )
			{
				pBlock = CreateProgramBlock( stack, lineIter, lineIter2, SBT_WHILE );
				if ( !pBlock ) RETURN_ERROR( ERR_NOTENOUGHMEMORY, lineIter->Number );
			}

			// evaluate the condition
			hr = Evaluate( *pTokens, pTokens->begin() + 1, tokenIter2, stack, ret );
			if ( hr ) RETURN_ERROR( hr, lineNo );
			var.SetValue( ret );
				
			while ( var.GetInt() )
			{
				// create new program block if the WHILE cycle is runned as separate blocks
				if ( !CycleInOneBlock )
				{
					pBlock = CreateProgramBlock( stack, lineIter, lineIter2, SBT_WHILE );
					if ( !pBlock ) RETURN_ERROR( ERR_NOTENOUGHMEMORY, lineIter->Number );
				}

				hr = InterpretBlock( stack, EFC, uiNeedExit, uiDeepness + 1, ret );
				if ( hr )
				{
					RemoveProgramBlock( stack );
					return hr;
				}

				if ( uiNeedExit ) // an exit command was called from inside of a block
				{
					if ( uiNeedExit == ET_CONTINUE ) goto WHILE_CONTINUE;
					if ( uiNeedExit == ET_BREAK )
					{
						if ( !CycleInOneBlock ) RemoveProgramBlock( stack );
						break;
					}
					returnVal = ret;
					uiExitType = uiNeedExit;
					return ERR_NOERROR;
				}

			WHILE_CONTINUE:	
				if ( !CycleInOneBlock ) RemoveProgramBlock( stack );

				// evaluate the condition
				hr = Evaluate( *pTokens, pTokens->begin() + 1, tokenIter2, stack, ret );
				if ( hr ) RETURN_ERROR( hr, lineNo );
				var.SetValue( ret );
			}
				
			if ( CycleInOneBlock ) RemoveProgramBlock( stack );				
			lineIter = lineIter2; // the lineIter must point to the actual line at the end of this iteration 
			continue;
		}  // end of WHILE cycle


		// ------------------------------------
		// line is an EFC
		// ------------------------------------
		if ( lineIter->Type == SLT_CALL )
		{
			if ( Size < 4 ) RETURN_ERROR( ERR_UNEXPECTEDENDOFLINE, lineNo ); // this line must have at least 4 tokens
			
			CString		parameters[SI_MAX_PARAM_COUNT + 1];
			UINT		paramCount = 0;
			UINT		tokensCount;

			// store the function name
			tokenIter = pTokens->begin() + 1;
			if ( !(tokenIter->Type & STT_IDENTIFIER) ) RETURN_ERROR( ERR_IDENTIFIEREXPECTED, lineNo ); 
			parameters[0] = tokenIter->Content;
			
			tokenIter++;
			if ( !(tokenIter->Type & STT_SYMBOL) || tokenIter->Content != ":" ) RETURN_ERROR( ERR_LEFTBRACKETEXPECTED, lineNo );
			
			// find the return variable
			tokenIter++;
			if ( !(tokenIter->Type & STT_IDENTIFIER) ) RETURN_ERROR( ERR_IDENTIFIEREXPECTED, lineNo ); 
			pVariable = GetVariable( stack, tokenIter->Content );
			if ( !pVariable ) RETURN_ERROR( ERR_UNKNOWNIDENTIFIER, lineNo );

			
			// evaluate all paramaters
			tokenIter++;
			if ( tokenIter == pTokens->end() ) goto GoToEFC; // there are no parameters, jump to the EFC
			if ( tokenIter->Type & STT_SYMBOL && tokenIter->Content == "," ) tokenIter++;
			tokenIter2 = tokenIter;
			tokensCount = 0;

			while ( tokenIter2 != pTokens->end() )
			{
				// comma symbol means next parameter
				if ( tokenIter2->Type & STT_SYMBOL && tokenIter2->Content == "," )
				{
					if ( !tokensCount ) RETURN_ERROR( ERR_EXPRESSIONEXPECTED, lineNo );
					hr = Evaluate( *pTokens, tokenIter, tokenIter2 - 1, stack, ret );
					if ( hr ) RETURN_ERROR( hr, lineNo );
					
					parameters[++paramCount] = ret;
					tokenIter = ++tokenIter2;
					tokensCount = 0;

					if ( paramCount > SI_MAX_PARAM_COUNT - 1 ) RETURN_ERROR( ERR_TOOMANYPARAMETERS, lineNo );
					continue;
				}

				tokenIter2++;
				tokensCount++;
			}
			
			// evaluate the last parameter
			if ( tokensCount )
			{
				hr = Evaluate( *pTokens, tokenIter, tokenIter2 - 1, stack, ret );
				if ( hr ) RETURN_ERROR( hr, lineNo );
				parameters[++paramCount] = ret;
			}

		
		GoToEFC:
            // call the externall function		
			hr = EFC( ret, paramCount, parameters );
			if ( !IgnoreEFCError && hr )
			{
				InterpretingError.ThrowError( ERR_EFC, lineNo );
				return hr;
			}

			// store the returned value
			pVariable->SetValue( ret );

			continue;
		} // end of EFC 


		// ------------------------------------
		// line is the beginning of another block
		// ------------------------------------
		if ( lineIter->Type == SLT_BEGIN )
		{
			// find the end of a block
			lineIter2 = FindEndOfBlock( lineIter );
			if ( lineIter2 == Lines.end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, lineIter->Number );
			lineIter++; // move to a line just after the left bracket

			// create new program block
			pBlock = CreateProgramBlock( stack, lineIter, lineIter2, SBT_FOR );
			if ( !pBlock ) RETURN_ERROR( ERR_NOTENOUGHMEMORY, lineIter->Number );
			
			// interpret block
			hr = InterpretBlock( stack, EFC, uiNeedExit, uiDeepness + 1, ret );
			RemoveProgramBlock( stack );
			if ( hr ) return hr;
			if ( uiNeedExit ) // an exit command was called from inside of a block
			{
				returnVal = ret;
				uiExitType = uiNeedExit;
				return ERR_NOERROR;
			}
			lineIter = lineIter2; // the lineIter must point to the actual line at the end of this iteration 
			continue;
		}

			
		// ------------------------------------
		// line is a directive
		// ------------------------------------
		if ( lineIter->Type == SLT_DIRECTIVE )
		{
			tokenIter = pTokens->begin() + 1;
			if ( tokenIter == pTokens->end() ) RETURN_ERROR( ERR_UNEXPECTEDENDOFFILE, lineNo );
			if ( tokenIter->Content == "ignoreEFCerror" )
			{
				tokenIter++;
				if ( tokenIter == pTokens->end() ) this->IgnoreEFCError = true;
				else if ( tokenIter->Content == "1" ) this->IgnoreEFCError = true;
				else if ( tokenIter->Content == "0" ) this->IgnoreEFCError = false;
				else RETURN_ERROR( ERR_INVALIDDIRECTIVEPARAMETER, lineNo );
			}
			if ( tokenIter->Content == "cycleinoneblock" )
			{
				tokenIter++;
				if ( tokenIter == pTokens->end() ) this->CycleInOneBlock = true;
				else if ( tokenIter->Content == "1" ) this->CycleInOneBlock = true;
				else if ( tokenIter->Content == "0" ) this->CycleInOneBlock = false;
				else RETURN_ERROR( ERR_INVALIDDIRECTIVEPARAMETER, lineNo );
			}

			continue;
		}
		
		
		// ------------------------------------
		// line is a return command
		// ------------------------------------
		if ( lineIter->Type == SLT_RETURN )
		{
			if ( Size < 2 ) RETURN_ERROR( ERR_RETURNVALUEEXPECTED, lineNo ); // this line must have at least 2 tokens

			hr = Evaluate( *pTokens, pTokens->begin() + 1, pTokens->end() - 1, stack, returnVal );
			if ( hr ) RETURN_ERROR( hr, lineNo );

			uiExitType = ET_RETURN;

			return ERR_NOERROR;
		}


		// ------------------------------------
		// line is a break command
		// ------------------------------------
		if ( lineIter->Type == SLT_BREAK )
		{
			uiExitType = ET_BREAK;
			return ERR_NOERROR;
		}


		// ------------------------------------
		// line is a continue command
		// ------------------------------------
		if ( lineIter->Type == SLT_CONTINUE )
		{
			uiExitType = ET_CONTINUE;
			return ERR_NOERROR;
		}


		// ------------------------------------
		// line is an end of block
		// ------------------------------------
		if ( lineIter->Type == SLT_END )
		{
			pActBlock->End = lineIter;
			return ERR_NOERROR;
		}


		// ------------------------------------
		// line type is not specified, it must be evaluated
		// ------------------------------------
		hr = Evaluate( *pTokens, pTokens->begin(), pTokens->end() - 1, stack, ret );
		if ( hr ) RETURN_ERROR( hr, lineNo );
	}


	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Removes a program block from the top of the stack. Then frees the memory used by it.
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CScript::RemoveProgramBlock( BLOCKSSTACK &stack )
{
	BLOCKSSTACK_ITERATOR	iter;
	CStackBlock		*		pBlock;
	
	if ( stack.size() < 1 ) return;

	iter = stack.end() - 1;

	pBlock = *iter;
    stack.pop_back();

	delete pBlock;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// allocates a memory for a new program block and fills it by parameters
// returns a valid pointer to this block or NULL when error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
CStackBlock * CScript::CreateProgramBlock(	BLOCKSSTACK &stack, 
											LINES_ITERATOR firstLine,
											LINES_ITERATOR lastLine,
											UINT Type,
											BOOL WasTrue )
{
	CStackBlock	*pBlock;

	pBlock = new CStackBlock;
	if ( !pBlock ) 
	{
		InterpretingError.ThrowError( ERR_NOTENOUGHMEMORY, firstLine->Number ); 
		return NULL;
	}

	stack.push_back( pBlock );
	pBlock->Jump = firstLine;
	pBlock->End = lastLine;
	pBlock->Type = Type;
	pBlock->WasTrue = WasTrue;

	return pBlock;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns a pointer to variable on the stack or NULL when variable of this name is not found
//
//////////////////////////////////////////////////////////////////////////////////////////////
CVariable * CScript::GetVariable( BLOCKSSTACK &stack, CString varName )
{
	BLOCKSSTACK_ITERATOR	iter;
	bool					found = false;
	BLOCKSSTACK::size_type	size, i;
	VARIABLES_ITERATOR		varIter;

	
	size = stack.size();
	for ( iter = stack.end(), iter--, i = 0; i < size; i++, iter-- )
	{
		if ( (*iter)->VariableExists( varName) )
		{
			found = true;
			break;
		}
	}

	if ( !found ) return NULL;

	return ((*iter)->Variables.find( varName ))->second;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns the operator structure
//
//////////////////////////////////////////////////////////////////////////////////////////////
OPERATOR CScript::GetOperator( CString str )
{
	OPERATOR	op = { "", true, 2000 };

	if ( !strcmp( (LPCTSTR) str, opPowering.str ) )				return opPowering;
//	if ( !strcmp( (LPCTSTR) str, opUnaryPlus.str ) )			return opUnaryPlus;
//	if ( !strcmp( (LPCTSTR) str, opUnaryMinus.str ) )			return opUnaryMinus;
	if ( !strcmp( (LPCTSTR) str, opLogicalNot.str ) )			return opLogicalNot;
	if ( !strcmp( (LPCTSTR) str, opMultiplication.str ) )		return opMultiplication;
	if ( !strcmp( (LPCTSTR) str, opDivision.str ) )				return opDivision;
	if ( !strcmp( (LPCTSTR) str, opMultiplicationF.str ) )		return opMultiplicationF;
	if ( !strcmp( (LPCTSTR) str, opDivisionF.str ) )			return opDivisionF;
	if ( !strcmp( (LPCTSTR) str, opModulus.str ) )				return opModulus;
	if ( !strcmp( (LPCTSTR) str, opAddition.str ) )				return opAddition;
	if ( !strcmp( (LPCTSTR) str, opSubtraction.str ) )			return opSubtraction;
	if ( !strcmp( (LPCTSTR) str, opAdditionF.str ) )			return opAdditionF;
	if ( !strcmp( (LPCTSTR) str, opSubtractionF.str ) )			return opSubtractionF;
	if ( !strcmp( (LPCTSTR) str, opConcatenation.str ) )		return opConcatenation;
	if ( !strcmp( (LPCTSTR) str, opLess.str ) )					return opLess;
	if ( !strcmp( (LPCTSTR) str, opGreater.str ) )				return opGreater;
	if ( !strcmp( (LPCTSTR) str, opLessEq.str ) )				return opLessEq;
	if ( !strcmp( (LPCTSTR) str, opGreaterEq.str ) )			return opGreaterEq;
	if ( !strcmp( (LPCTSTR) str, opInequality.str ) )			return opInequality;
	if ( !strcmp( (LPCTSTR) str, opEquality.str ) )				return opEquality;
	if ( !strcmp( (LPCTSTR) str, opLogicalAnd.str ) )			return opLogicalAnd;
	if ( !strcmp( (LPCTSTR) str, opLogicalOr.str ) )			return opLogicalOr;
	if ( !strcmp( (LPCTSTR) str, opAssignment.str ) )			return opAssignment;

	return op;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// evaluates specified list of tokens that are brackets-free - that means there are no
// brackets between them
//
// i1 and i2 iterators are specifying an interval of tokens that are evaluated ( = <i1,i2> )
// use stack of variables and returns a value in retVal parameter
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScript::EvaluateNonBracketed(	TOKENS &inTokens, 
										TOKENS_ITERATOR i1, 
										TOKENS_ITERATOR i2, 
										BLOCKSSTACK &stack, 
										CString &retVal )
{

	TOKENS				Tokens( i1, i2 + 1 ); // select evaluated tokens only
	TOKENS_ITERATOR		iter, lowestPriority;
	HRESULT				hr;
	CString				val1, val2;
	OPERATOR			op, myOp;
	CVariable			*pVar, var( "v" ), var1( "v2" );



	// check out count of tokens
	if ( Tokens.size() == 0 ) return ERR_WRONGSYNTAX;

	
	// single token - evaluate it
	if ( Tokens.size() == 1 ) 
	{
		if ( ( i1->Type & STT_NUMBER ) || ( i1->Type & STT_STRING ) ) retVal = i1->Content;
		else if ( i1->Type & STT_IDENTIFIER )
		{
			pVar = GetVariable( stack, i1->Content ); // find a variable on the stack
			if ( !pVar ) return ERR_UNKNOWNIDENTIFIER; // variable doesnt exist
			retVal = pVar->GetString();
		}
		else return ERR_WRONGSYNTAX;

		return ERR_NOERROR;
	}

	
	// two tokens - this was caused because of unary operator
	if ( Tokens.size() == 2 ) 
	{
		if ( !( i1->Type & STT_OPERATOR ) ) return ERR_WRONGSYNTAX; // first token should be an operator
		
		op = GetOperator( i1->Content );

		if ( !strcmp( op.str, "+" ) ) // unary plus operator
		{
			if ( hr = EvaluateNonBracketed( inTokens, i2, i2, stack, retVal ) ) return hr;
		}
		else if ( !strcmp( op.str, "-" ) ) // unary minus operator
		{
			if ( hr = EvaluateNonBracketed( inTokens, i2, i2, stack, retVal ) ) return hr;

			var.SetValue( retVal );
			var1.SetValue( -var.GetInt() );
			retVal = var1.GetString();
		}
		else if ( !strcmp( op.str, "!" ) ) // logical not operator
		{
			if ( hr = EvaluateNonBracketed( inTokens, i2, i2, stack, retVal ) ) return hr;

			var.SetValue( retVal );
			if ( var.GetInt() ) retVal = "1";
			else retVal = "0";
		}
		else return ERR_WRONGSYNTAX; // an UNARY operator was expected
		
		return ERR_NOERROR;
	}



	// find an operator with the lowest priority
	myOp.str[0] = 0;
	myOp.LeftToRight = true;
	myOp.Priority = 2000;

	for ( iter = Tokens.end() - 1; iter != Tokens.begin(); iter-- ) // dont check the first token, its possibly an unary operator
	{
		if ( iter->Type & STT_OPERATOR )
		{
			op = GetOperator( iter->Content );
			// new operator with lowest priority is not matched in the same manner for LtR and RtL association
			if ( ( op.Priority < myOp.Priority && op.LeftToRight ) || 
				( op.Priority <= myOp.Priority && !op.LeftToRight ) )
			{
				lowestPriority = iter;
				myOp = op;
			}
		}
	}

	if ( myOp.Priority == 2000 ) return ERR_WRONGSYNTAX; // no operator found


	
	// divide the tokens list to two parts by the operator
	if ( hr = EvaluateNonBracketed( inTokens, Tokens.begin(), lowestPriority - 1, stack, val1 ) ) return hr;
	if ( hr = EvaluateNonBracketed( inTokens, lowestPriority + 1, Tokens.end() - 1, stack, val2 ) ) return hr;
	
	
	
	// compute the result of "val1 op val2"
	if ( hr = ApplyBinaryOperator( myOp.str, val1, val2, retVal ) ) return hr;



	// special case when myOp is an assignment operator
	// in that case the value of right the side of assignment must be stored to l-value
	if ( !strcmp( myOp.str, "=" ) ) 
	{
		// check whether the left side is an l-value
		if ( ! ((lowestPriority - 1)->Type & STT_IDENTIFIER) ) return ERR_LVALUEEXPECTED;

		// fill the variable with computed value
		pVar = GetVariable( stack, (lowestPriority - 1)->Content ); // find a variable on the stack
		if ( !pVar ) return ERR_UNKNOWNIDENTIFIER; // variable doesnt exist
		pVar->SetValue( val2 );
	}


	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// applies a binary operator to string operands val1 and val2
// returns the result in retVal
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScript::ApplyBinaryOperator( LPCTSTR op, CString val1, CString val2, CString &retVal )
{
	CVariable	var1( "a" ), var2( "a1" ), var3( "a2" );
	
	
	var1.SetValue( val1 );
	var2.SetValue( val2 );


	// apply integer addition operator
	if ( !strcmp( op, "+" ) )
	{
		var3.SetValue( var1.GetInt() + var2.GetInt() );
		retVal = var3.GetString();
	}


	// apply integer subtraction operator
	if ( !strcmp( op, "-" ) )
	{
		var3.SetValue( var1.GetInt() - var2.GetInt() );
		retVal = var3.GetString();
	}


	// apply integer multiplication operator
	if ( !strcmp( op, "*" ) )
	{
		var3.SetValue( var1.GetInt() * var2.GetInt() );
		retVal = var3.GetString();
	}


	// apply integer division operator
	if ( !strcmp( op, "/" ) )
	{
		if ( !var2.GetInt() ) return ERR_DIVISIONBYZERO;

		var3.SetValue( var1.GetInt() / var2.GetInt() );
		retVal = var3.GetString();
	}


	// apply float addition operator
	if ( !strcmp( op, "++" ) )
	{
		var3.SetValue( var1.GetFloat() + var2.GetFloat() );
		retVal = var3.GetString();
	}


	// apply float subtraction operator
	if ( !strcmp( op, "--" ) )
	{
		var3.SetValue( var1.GetFloat() - var2.GetFloat() );
		retVal = var3.GetString();
	}


	// apply float multiplication operator
	if ( !strcmp( op, "**" ) )
	{
		var3.SetValue( var1.GetFloat() * var2.GetFloat() );
		retVal = var3.GetString();
	}


	// apply float division operator
	if ( !strcmp( op, "//" ) )
	{
		if ( var2.GetFloat() == 0.0f ) return ERR_DIVISIONBYZERO;

		var3.SetValue( var1.GetFloat() / var2.GetFloat() );
		retVal = var3.GetString();
	}


	// apply modulus operator
	if ( !strcmp( op, "%" ) )
	{
		var3.SetValue( var1.GetInt() % var2.GetInt() );
		retVal = var3.GetString();
	}


	// apply power operator
	if ( !strcmp( op, "^" ) )
	{
		int		liExp = var2.GetInt(), liSum = 1;

		if ( liExp < 0 ) return ERR_ILLEGALEXPONENT; // only natural exponent allowed
		for ( int i = 0; i < liExp; i++ ) liSum *= var1.GetInt();
		var3.SetValue( liSum );
		retVal = var3.GetString();
	}


	// apply assignment operator
	if ( !strcmp( op, "=" ) )
	{
		retVal = var2.GetString();
	}


	// apply string concatenation operator
	if ( !strcmp( op, "." ) ) retVal = var1.GetString() + var2.GetString();

	// apply equality operator
	if ( !strcmp( op, "==" ) ) retVal = var1.GetInt() == var2.GetInt() ? "1" : "0";

	// apply inequality operator
	if ( !strcmp( op, "!=" ) ) retVal = var1.GetInt() != var2.GetInt() ? "1" : "0";

	// apply less than operator
	if ( !strcmp( op, "<" ) ) retVal = var1.GetInt() < var2.GetInt() ? "1" : "0";

	// apply greater than operator
	if ( !strcmp( op, ">" ) ) retVal = var1.GetInt() > var2.GetInt() ? "1" : "0";

	// apply less than or equal operator
	if ( !strcmp( op, "<=" ) ) retVal = var1.GetInt() <= var2.GetInt() ? "1" : "0";

	// apply greater than or equal operator
	if ( !strcmp( op, ">=" ) ) retVal = var1.GetInt() >= var2.GetInt() ? "1" : "0";

	// apply logical OR operator
	if ( !strcmp( op, "||" ) ) retVal = (var1.GetInt() || var2.GetInt()) ? "1" : "0";

	// apply logical AND operator
	if ( !strcmp( op, "&&" ) ) retVal = (var1.GetInt() && var2.GetInt()) ? "1" : "0";


	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// evaluates a built-in function
//
// func is specifying function name, retVal is returned value of a function
// param1 - param3 are parameters given to this function where paramCount indicates their count
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScript::EvaluateFunction(	CString func, 
									CString &retVal,
									UINT	paramCount,
									CString param1, 
									CString param2, 
									CString param3 )
{

	CVariable	var1( "var1" ), var2( "var2" ), var3( "var3" );
	
	var1.SetValue( param1 );
	var2.SetValue( param2 );
	var3.SetValue( param3 );


	// returns the sinus of the given angle (in radians)
	if ( func == "sin" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var2.SetValue( sinf( var1.GetFloat() ) );
		retVal = var2.GetString();
		return ERR_NOERROR;
	}


	// returns the cosinus of the given angle (in radians)
	if ( func == "cos" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var2.SetValue( cosf( var1.GetFloat() ) );
		retVal = var2.GetString();
		return ERR_NOERROR;
	}


	// returns the tangens of the given angle (in radians)
	if ( func == "tg" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var2.SetValue( tanf( var1.GetFloat() ) );
		retVal = var2.GetString();
		return ERR_NOERROR;
	}


	// returns the cotangens of the given angle (in radians)
	if ( func == "cotg" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var2.SetValue( 1.0f / tanf( var1.GetFloat() ) );
		retVal = var2.GetString();
		return ERR_NOERROR;
	}


	// returns the arcsin of the given angle (in radians)
	if ( func == "arcsin" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var2.SetValue( asinf( var1.GetFloat() ) );
		retVal = var2.GetString();
		return ERR_NOERROR;
	}


	// returns the arccosinus of the given angle (in radians)
	if ( func == "arccos" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var2.SetValue( acosf( var1.GetFloat() ) );
		retVal = var2.GetString();
		return ERR_NOERROR;
	}


	// returns the arctg of the given angle (in radians)
	if ( func == "arctg" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var2.SetValue( atanf( var1.GetFloat() ) );
		retVal = var2.GetString();
		return ERR_NOERROR;
	}


	// returns the arccotg of the given angle (in radians)
	if ( func == "arccotg" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var2.SetValue( 1.0f / atanf( var1.GetFloat() ) );
		retVal = var2.GetString();
		return ERR_NOERROR;
	}


	// returns the square root of the parameter
	if ( func == "sqrt" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var2.SetValue( sqrtf( var1.GetFloat() ) );
		retVal = var2.GetString();
		return ERR_NOERROR;
	}


	// returns minimum of two integers
	if ( func == "min" )
	{
		if ( paramCount < 2 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 2 ) return ERR_TOOMANYPARAMETERS;

		if ( var1.GetInt() < var2.GetInt() )
		{
			retVal = var1.GetString();
			return ERR_NOERROR;
		}
		else
		{
			retVal = var2.GetString();
			return ERR_NOERROR;
		}
	}


	// returns maximum of two integers
	if ( func == "max" )
	{
		if ( paramCount < 2 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 2 ) return ERR_TOOMANYPARAMETERS;

		if ( var1.GetInt() > var2.GetInt() )
		{
			retVal = var1.GetString();
			return ERR_NOERROR;
		}
		else
		{
			retVal = var2.GetString();
			return ERR_NOERROR;
		}
	}


	// returns a^b
	if ( func == "power" )
	{
		if ( paramCount < 2 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 2 ) return ERR_TOOMANYPARAMETERS;

		int	val = var1.GetInt();

		for ( int i = 0; i < var2.GetInt(); i++ ) val *= var1.GetInt();
		
		var3.SetValue( val );

		retVal = var3.GetString();

		return ERR_NOERROR;
	}


	// returns length of a string
	if ( func == "length" )
	{
		if ( paramCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var3.SetValue( (var1.GetString()).GetLength() );
		retVal = var3.GetString();

		return ERR_NOERROR;
	}


	// returns a character from specified position
	if ( func == "char" )
	{
		if ( paramCount < 2 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 2 ) return ERR_TOOMANYPARAMETERS;

		if ( var2.GetInt() >= (var1.GetString()).GetLength() ) return ERR_RANGECHECK;
		retVal = (var1.GetString())[var2.GetInt()];

		return ERR_NOERROR;
	}


	// finds an occurance of first parameter in the second
	if ( func == "find" )
	{
		if ( paramCount < 2 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 2 ) return ERR_TOOMANYPARAMETERS;

		var3.SetValue( var2.GetString().Find( var1.GetString() ) );
		retVal = var3.GetString();

		return ERR_NOERROR;
	}


	// returns a substring of a specified length and position
	if ( func == "substr" )
	{
		if ( paramCount < 3 ) return ERR_NOTENOUGHPARAMETERS;
		if ( paramCount > 3 ) return ERR_TOOMANYPARAMETERS;

		retVal = var1.GetString().Mid( var2.GetInt(), var3.GetInt() );

		return ERR_NOERROR;
	}


	
	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// evaluates specified list of tokens and returns its resultant value in a retVal parameter
// evaluates only those tokens that are in interval <i1,i2>
//
// returns 0 when no error occurs or a nonzero error code
// stack is needed to find all variables
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScript::Evaluate(	TOKENS &inTokens,
							TOKENS_ITERATOR	i1,
							TOKENS_ITERATOR	i2,
							BLOCKSSTACK &stack,
							CString &retVal )
{

	TOKENS				Tokens( i1, i2 + 1 );
	TOKENS_ITERATOR		iter, iter2, lastLeftBracket, lastRightBracket, lastComma;
	int					bracketsCount, paramCount;
	HRESULT				hr;
	CString				values[10];
	BOOL				bracketsRemoved;


	retVal = "";

	// count brackets and check their parity
	bracketsCount = 0;

	for ( iter = Tokens.begin(); iter != Tokens.end(); iter++ )
	{
		if ( (iter->Type == STT_SYMBOL) && (iter->Content == "(") ) bracketsCount++;
		if ( (iter->Type == STT_SYMBOL) && (iter->Content == ")") )	bracketsCount--;
		if ( bracketsCount < 0 ) return ERR_WRONGSYNTAX; // wrong brackets order
	}

	if ( bracketsCount ) return ERR_WRONGSYNTAX; // wrong count of brackets



REMOVE_BRACKETS:	

	// evaluate the most inner brackets (or function call)
	// so, find the first right bracket
	bracketsRemoved = false;

	for ( iter = Tokens.begin(); iter != Tokens.end(); iter++ )
	{
		if ( (iter->Type == STT_SYMBOL) && (iter->Content == "(" ) ) lastLeftBracket = iter;
		if ( (iter->Type == STT_SYMBOL) && (iter->Content == ")" ) )
		{
			// this is not a pair of brackets but a function call
			if ( ( lastLeftBracket != Tokens.begin() ) && ( (lastLeftBracket - 1)->Type & STT_FUNCTION ) )
			{
				// compute count of parameters and evaluate them
				iter2 = lastLeftBracket + 1;
				lastComma = lastLeftBracket;
				paramCount = 0;

				while ( ! ((iter2->Type & STT_SYMBOL) && (iter2->Content == ")")) )
				{
					if ( (iter2->Type & STT_SYMBOL) && (iter2->Content == ",") )
					{
						if ( paramCount > 8 ) return ERR_TOOMANYPARAMETERS; // there is a maximum number of parameters in built-in functions
						
						if ( hr = EvaluateNonBracketed( Tokens, lastComma + 1, iter2 - 1, stack, values[paramCount] ) ) return hr;
						lastComma = iter2;
						paramCount++;
					}

					iter2++;
				}

				// evaluate the last parameter
				if ( hr = EvaluateNonBracketed( Tokens, lastComma + 1, iter2 - 1, stack, values[paramCount] ) ) return hr;
				paramCount++;

				// evaluate the found function
				hr = EvaluateFunction( (lastLeftBracket - 1)->Content, retVal, paramCount, values[0], values[1], values[2] );
				if ( hr ) return hr;

				// substitute tokens by a computed value
				// first, erase tokens - let the right bracket be in
				iter = Tokens.erase( lastLeftBracket - 1, iter );
				// substitute the right bracket we left here for computed value
				iter->Type = STT_STRING;
				iter->Content = retVal;

				bracketsRemoved = true;
				break;

			} // was a function call
			else // evaluate brackets
			{
				hr = EvaluateNonBracketed( Tokens, lastLeftBracket + 1, iter - 1, stack, retVal );
				if ( hr ) return hr;
				
				// substitute tokens by a computed value
				// first, erase tokens - let the right bracket be in
				iter = Tokens.erase( lastLeftBracket, iter );
				// substitute the right bracket we left here for computed value
				iter->Type = STT_STRING;
				iter->Content = retVal;

				bracketsRemoved = true;
				break;

			} // not a function call
		} // right bracket found
	}

	// repeat removing brackets until there is no change
	if ( bracketsRemoved ) goto REMOVE_BRACKETS;


	// all brackets and function calls are removed
	// evaluate the rest now
	if ( hr = EvaluateNonBracketed( Tokens, Tokens.begin(), Tokens.end() - 1, stack, retVal ) ) return hr;
	

	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this returns true, when the specified character is an identifier character
// you must specify the position of a character in a word (note, that only 1. is interesting)
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CScript::IsIdentifierChar( char znak, int pos )
{
	return ( (strchr( IDENTIFIER_CHARS, znak ) != NULL) || 
		( (strchr( NUMBER_CHARS, znak ) != NULL ) && (pos > 1) ) );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this returns true, when the specified character is a symbol character
// that is (),#{}@
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CScript::IsSymbolChar( char znak )
{
	return ( strchr( SYMBOL_CHARS, znak ) != NULL );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this returns true, when the specified character is an operator character
// that is +-*/%&^.=|<>
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CScript::IsOperatorChar( char znak )
{
	return ( strchr( OPERATOR_CHARS, znak ) != NULL );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// deletes comments from a string, returns true when this string ends as a block comment;
//
// remember to specify (via inComment) whether the line is starting as a comment
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CScript::RemoveComments( CString &str, bool inComment )
{
	char	lchr = '\n';
	int		len = str.GetLength();
	CString	newStr = "";
	bool	inString = false;

	for ( int i = 0; i < len; i++ )
	{
		if (inComment) // we are in comment
		{
			if ((lchr == '-') && (str[i] == ';')) // the block comment ends here
			{
				lchr = '\n';
				inComment = false;
			}
			else lchr = str[i];
		}
		else // not in comment
		{
			if (inString) // we are in a string, no comment are available
			{
				if ((str[i] == '"') && (lchr != '\\'))
				{
					inString = false;
					newStr += str[i];
				}
				else newStr += str[i];
				continue;
			}
			if (str[i] == '"') // new string begins here
			{
				inString = true;
				lchr = '"';
				newStr += str[i];
				continue;
			}
			if (lchr == ';') // last character signalized a comment, but we dont know which one
			{
				if ( str[i] == '-' ) // new block comment is starting here
				{
					lchr = '\n';
					inComment = true;
				}
				else break; // line comment started last character
			}
			else
			{
				if ( str[i] == ';' ) lchr = ';'; // here, a comment is starting, but whe are not sure which one
				else
				{
					lchr = str[i];
					newStr += lchr;
				}
			}
		}

	}

    str = newStr;
	return inComment;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// deletes comments from a string and divides it to separate tokens, then adds those tokens
// into the Tokens list;
//
// inComment returns true when the line ends as a block comment;
// remember to specify (via inComment) whether the line is starting as a comment;
//
// returns a non-zero value when an error occurs or a mistake in source code is found
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScript::AddLine( CString str, int LineNumber, bool &inComment )
{
	char			chr, lchr;
	int				len, begin;
	CString			word, newStr = str;
	bool			firstChar, identifier, inString, number, wasapoint;
	CScriptToken	Token;
    CScriptLine		ScriptLine;
	TOKENS_ITERATOR	tokenIter;


	
	// first, remove comments from this line
	inComment = RemoveComments( newStr, inComment );
	newStr.TrimLeft( " \t" );
	newStr.TrimRight( " \t" );
	if ( newStr == "" ) return ERR_NOERROR;
	newStr += " "; // later, we will need to be sure, the last character is a space
	
	if ( newStr[0] == '#' ) ScriptLine.Directive = true;
	else ScriptLine.Directive = false;
	ScriptLine.Number = LineNumber;


	
	// now, create a linear list of words on this line
	lchr = ' ';
	len = newStr.GetLength();
	begin = 0;
	inString = false;
	identifier = false;
	firstChar = true;
	number = false;

	
	for ( int i = 0; i < len; i++ )
	{
		chr = newStr[i];

		// ------------------------------------
		// we are in a string, no special character's meanings are allowed
		// ------------------------------------
		if ( inString ) 
		{
			if ( ( chr == '"') && (lchr != '\\') )
			{
				inString = false;
				firstChar = true;
				lchr = '"';
				Token.Type = STT_STRING;
				Token.Content = newStr.Mid( begin + 1, i - begin - 1 ); // copy string without ""
				ScriptLine.Tokens.push_back( Token );
			}
			else lchr = chr;
			continue;
		}

		// ------------------------------------
		// new string begins here
		// ------------------------------------
		if ( chr == '"' )
		{
			inString = true;
			firstChar = false;
			begin = i;
			lchr = '"';
			continue;
		}

		// ------------------------------------
		// character is an (part of) operator;
		// however, a period can be also in number!
		// ------------------------------------
		if ( strchr( OPERATOR_CHARS, chr ) != NULL && !( !firstChar && number && chr == '.' ) ) 
		{
			firstChar = true;
			word = lchr;
			word += chr;
			if ( strstr( OPERATOR_CHAR_PAIRS, (LPCTSTR) word ) ) // this is two-char operator
			{
				ScriptLine.Tokens.pop_back(); // first, remove the last operator, because it wasnt whole yet
				Token.Type = STT_OPERATOR;
				Token.Content = word;
				ScriptLine.Tokens.push_back( Token );
				lchr = ' ';
			}
			else // single-character operator
			{
				if ( identifier)
				{
					Token.Type = STT_IDENTIFIER;
					Token.Content = newStr.Mid( begin, i-begin );
					ScriptLine.Tokens.push_back( Token );
					identifier = false;
				}
				if ( number )
				{
					Token.Type = STT_NUMBER;
					Token.Content = newStr.Mid( begin, i-begin );
					ScriptLine.Tokens.push_back( Token );
					number = false;
				}
				Token.Type = STT_OPERATOR;
				Token.Content = chr;
				ScriptLine.Tokens.push_back( Token );
				lchr = chr;
			}
			continue;
		}

		// ------------------------------------
		// character is a symbol
		// ------------------------------------
		if ( strchr( SYMBOL_CHARS, chr ) != NULL ) 
		{
			if ( identifier)
			{
				Token.Type = STT_IDENTIFIER;
				Token.Content = newStr.Mid( begin, i-begin );
				ScriptLine.Tokens.push_back( Token );
				identifier = false;
			}
			if ( number )
			{
				Token.Type = STT_NUMBER;
				Token.Content = newStr.Mid( begin, i-begin );
				ScriptLine.Tokens.push_back( Token );
				number = false;
			}
			firstChar = true;
			Token.Type = STT_SYMBOL;
			Token.Content = chr;
			ScriptLine.Tokens.push_back( Token );
			lchr = chr;
			continue;
		}
		
		// ------------------------------------
		// a new token is starting
		// ------------------------------------
		if ( firstChar ) 
		{
			begin = i;
			if ( IsIdentifierChar( chr, 1 ) ) // new identifier starts here
			{
				lchr = chr;
				firstChar = false;
				identifier = true;
			}
			else if ( strchr( NUMBER_CHARS, chr ) != NULL ) // a number starts here
			{
				lchr = chr;
				firstChar = false;
				number = true;
				wasapoint = false;
            }
			else if ( (chr == ' ') || (chr == '\t') ) lchr = ' ';
			else return ERR_UNKNOWNCHARACTER;
		}
		else // continue in already started token
		{
			if ( (chr == ' ') || (chr == '\t') ) // end the last token
			{
				Token.Type = STT_UNDEFINED;
				if ( identifier ) Token.Type = STT_IDENTIFIER;
				if ( number ) Token.Type = STT_NUMBER;
				Token.Content = newStr.Mid( begin, i-begin );
				ScriptLine.Tokens.push_back( Token );
				identifier = number = false;
				firstChar = true;
				lchr = ' ';
				continue;
			}
			if ( identifier ) // the actual token is an identifier
			{
				if ( !IsIdentifierChar( chr, 2 ) ) return ERR_UNKNOWNCHARACTER;
				lchr = chr;
				continue;
			}
			if ( number ) // the actual token is a number
			{
				// if the character is not a digit, or a period, but there was already a period in this number
				if ( strchr( NUMBER_CHARS, chr ) == NULL && ( chr != '.' || wasapoint ) ) return ERR_WRONGSYNTAX;
				lchr = chr;
				continue;
			}
		}
	}


	
	// identify tokens that are identifiers (possible keywords or built-in functions)
	CString		keyWords( KEYWORDS ), builtinFun( BUILTINFUNCTIONS );
	for ( tokenIter = ScriptLine.Tokens.begin(); tokenIter != ScriptLine.Tokens.end(); tokenIter++ )
	{
		if ( tokenIter->Type != STT_IDENTIFIER ) continue;
		if ( keyWords.Find( " " + tokenIter->Content + " " ) > -1 )	tokenIter->Type = STT_KEYWORD;
		if ( builtinFun.Find( " " + tokenIter->Content + " " ) > -1 ) tokenIter->Type = STT_FUNCTION;
	}
	
	ScriptLine.Type = DetermineLineType( &ScriptLine );

	//ScriptLine.Print( true, true, true );
	Lines.push_back( ScriptLine );


	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads a script from file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScript::Load( LPCTSTR fileName )
{
	FILE			*f;
	CString			str;
	char			chr;
	bool			comment;
	HRESULT			hr;
	int				lineNumber;
	
	f = fopen( fileName, "rt" );
	if (!f)	return ERR_FILENOTFOUND;
	
	str = "";
	comment = false;
	lineNumber = 0;

	while ( !feof( f ) )
	{
		chr = fgetc( f );
		if ( feof( f ) ) chr = '\n';
		if ( chr != '\n' ) str += chr;
		else
		{
			hr = AddLine( str, lineNumber, comment );
			if ( hr )	InterpretingError.ThrowError( hr, lineNumber );
			str = "";
			lineNumber++;
		}
	}

	fclose( f );

	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// determines the type of a line by the first word or the line's content
//
//////////////////////////////////////////////////////////////////////////////////////////////
int CScript::DetermineLineType( CScriptLine *line )
{
	TOKENS_ITERATOR		iter;
	CScriptToken		*pToken;

	pToken = &(line->Tokens.front());
	if ( pToken->Type == STT_SYMBOL )
	{
		if ( pToken->Content == "#" ) return SLT_DIRECTIVE;
		if ( pToken->Content == "{" ) return SLT_BEGIN;
		if ( pToken->Content == "}" ) return SLT_END;
	}
	
	if ( pToken->Type == STT_KEYWORD )
	{
		if ( pToken->Content == "for" )			return SLT_FOR;
		if ( pToken->Content == "then" )		return SLT_THEN;
		if ( pToken->Content == "else" )		return SLT_ELSE;
		if ( pToken->Content == "while" )		return SLT_WHILE;
		if ( pToken->Content == "return" )		return SLT_RETURN;
		if ( pToken->Content == "if" )			return SLT_IF;
		if ( pToken->Content == "var" )			return SLT_VAR;
		if ( pToken->Content == "continue" )	return SLT_CONTINUE;
		if ( pToken->Content == "break" )		return SLT_BREAK;
		if ( pToken->Content == "call" )		return SLT_CALL;
	}
	
	if ( (line->Tokens.size() >= 2) && (line->Tokens[1].Type == STT_OPERATOR) && (line->Tokens[1].Content == "=") )
		return SLT_ASSIGNMENT;

	return SLT_UNDEFINED;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this fills the parameters with version numbers of the interpreter's DLL that is needed to 
// interpret this script
//
// returns true, if this information is present in script's source
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CScript::InterpretVersionNeeded( int &majVer, int &minVer )
{
	LINES_ITERATOR	linesIter;
	TOKENS_ITERATOR	tokensIter;
    
	for ( linesIter = Lines.begin(); linesIter != Lines.end(); linesIter++ ) 
		if ( linesIter->Directive )
		{
			tokensIter = linesIter->Tokens.begin();
			tokensIter++;
			if ( tokensIter->Content == "version" )
			{
				tokensIter++;
				HRESULT hr = sscanf( (LPCTSTR) tokensIter->Content, "%d", &majVer );
				tokensIter++;
				hr += sscanf( (LPCTSTR) tokensIter->Content, "%d", &minVer );
				if ( hr != 2 ) continue;
				return true;
			}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
