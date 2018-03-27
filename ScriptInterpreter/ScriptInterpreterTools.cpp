#include "stdafx.h"
#include "ScriptInterpreterTools.h"


// predefined operators
const	OPERATOR		opPowering = { "^", false, 900 };
const	OPERATOR		opUnaryPlus = { "+", false, 800 };
const	OPERATOR		opUnaryMinus = { "-", false, 800 };
const	OPERATOR		opLogicalNot = { "!", false, 800 };
const	OPERATOR		opMultiplication = { "*", true, 700 };
const	OPERATOR		opDivision = { "/", true, 700 };
const	OPERATOR		opMultiplicationF = { "**", true, 700 };
const	OPERATOR		opDivisionF = { "//", true, 700 };
const	OPERATOR		opModulus = { "%", true, 700 };
const	OPERATOR		opAddition = { "+", true, 600 };
const	OPERATOR		opSubtraction = { "-", true, 600 };
const	OPERATOR		opAdditionF = { "++", true, 600 };
const	OPERATOR		opSubtractionF = { "--", true, 600 };
const	OPERATOR		opConcatenation = { ".", true, 600 };
const	OPERATOR		opLess = { "<", true, 500 };
const	OPERATOR		opGreater = { ">", true, 500 };
const	OPERATOR		opLessEq = { "<=", true, 500 };
const	OPERATOR		opGreaterEq = { ">=", true, 500 };
const	OPERATOR		opEquality = { "==", true, 400 };
const	OPERATOR		opInequality = { "!=", true, 400 };
const	OPERATOR		opLogicalAnd = { "&&", true, 300 };
const	OPERATOR		opLogicalOr = { "||", true, 200 };
const	OPERATOR		opAssignment = { "=", false, 100 };



//////////////////////////////////////////////////////////////////////////////////////////////
//
// CScriptToken constructor undefines its type and creates assigned CString object
//
//////////////////////////////////////////////////////////////////////////////////////////////
CScriptToken::CScriptToken()
{
	Type = STT_UNDEFINED;
	Content = CString("");
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor sets some default values
//
//////////////////////////////////////////////////////////////////////////////////////////////
CScriptLine::CScriptLine()
{
	Directive = false;
	Number = 0;
	Tokens = TOKENS();
	Type = SLT_UNDEFINED;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// prints the line to standard output
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CScriptLine::Print( bool lineNumbers, bool lineTypes, bool tokenTypes )
{
	TOKENS_ITERATOR	iter;

	if ( lineNumbers ) printf( "%li\t", Number );

	if ( lineTypes )
	{
		switch ( this->Type )
		{
		case SLT_VAR:
			printf( "VAR\t\t" );
			break;
		case SLT_BEGIN:
			printf( "BEGIN\t\t" );
			break;
		case SLT_END:
			printf( "END\t\t" );
			break;
		case SLT_WHILE:
			printf( "WHILE\t\t" );
			break;
		case SLT_CONTINUE:
			printf( "CONTINUE\t\t" );
			break;
		case SLT_BREAK:
			printf( "BREAK\t\t" );
			break;
		case SLT_DIRECTIVE:
			printf( "DIRECTIVE\t" );
			break;
		case SLT_FOR:
			printf( "FOR\t\t" );
			break;
		case SLT_IF:
			printf( "IF\t\t" );
			break;
		case SLT_THEN:
			printf( "THEN\t\t" );
			break;
		case SLT_ELSE:
			printf( "ELSE\t\t" );
			break;
		case SLT_RETURN:
			printf( "RETURN\t\t" );
			break;
		case SLT_CALL:
			printf( "CALL\t\t" );
			break;
		case SLT_ASSIGNMENT:
			printf( "=\t\t" );
			break;
		case SLT_COMMENT:
			printf( "//\t\t" );
			break;
		default:
			printf( "UNDEFINED\t" );
			break;
		}
	}
	
	for ( iter = Tokens.begin(); iter != Tokens.end(); iter++ )	
	{
		printf( "%s", (LPCTSTR) (iter->Content) );
		if ( tokenTypes )
		{
			switch ( iter->Type )
			{
			case STT_FUNCTION:
				printf( "(func)  " );
				break;
			case STT_IDENTIFIER:
				printf( "(ide)  " );
				break;
			case STT_OPERATOR:
				printf( "(op)  " );
				break;
			case STT_SYMBOL:
				printf( "(symb)  " );
				break;
			case STT_NUMBER:
				printf( "(#)  " );
				break;
			case STT_STRING:
				printf( "(str)  " );
				break;
			case STT_KEYWORD:
				printf( "(key)  " );
				break;
			default:
				printf( "(undef)  " );
				break;
			}
		}
		else printf( " " );
	}
	printf( "\n" );
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
