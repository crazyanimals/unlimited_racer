/*
	ScriptInterpreterTools:		Here are some global definitions and macros used in ScriptInterpreter
								functions and other included modules. Also some other staff used to analyze text.
								This is the base of SI project.

	Creation Date:		22.4.2004
	Last Update:		8.5.2004
	Author:				Roman Margold

*/


#pragma once


#include "stdafx.h"
#include "ScriptInterpreter.h"



// DLL identifying macros
#define SCRIPTINTERPRETER_VERSION				"1.0"
#define SCRIPTINTERPRETER_MINOR_VERSION			0
#define SCRIPTINTERPRETER_MAJOR_VERSION			1
#define SCRIPTINTERPRETER_VERSION_DESCRIPTOR	"beta"


// script token types
#define STT_UNDEFINED			0
#define STT_IDENTIFIER			1
#define STT_OPERATOR			2
#define STT_SYMBOL				4
#define STT_NUMBER				8
#define STT_STRING				16
#define STT_KEYWORD				32
#define STT_FUNCTION			64
#define STT_VALUE				128


// script line type
#define SLT_UNDEFINED			0
#define SLT_VAR					1
#define SLT_FOR					2
#define SLT_WHILE				3
#define SLT_IF					5
#define SLT_THEN				7
#define SLT_ELSE				8
#define SLT_COMMENT				9
#define SLT_ASSIGNMENT			10
#define SLT_RETURN				11
#define SLT_DIRECTIVE			12
#define SLT_BEGIN				13
#define SLT_END					14
#define SLT_CONTINUE			15
#define SLT_BREAK				16
#define SLT_CALL				17


// script block type
#define SBT_UNDEFINED			0
#define SBT_FOR					2
#define SBT_WHILE				3
#define SBT_THEN				4
#define SBT_ELSE				5


// exit types used by evaluating and interpreting functions
// defines the type of exit that was used
#define ET_NORMAL				0
#define ET_ERROR				1
#define ET_CONTINUE				2
#define ET_BREAK				3
#define ET_EXIT					4
#define ET_RETURN				5


// characters
#define OPERATOR_CHARS			"+-*/%.=<>|&!^"
#define SYMBOL_CHARS			"#,(){}:"
#define NUMBER_CHARS			"0123456789"
#define LETTER_CHARS			"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define IDENTIFIER_CHARS		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
#define KEYWORDS				" for var if then else return call while continue break "
#define BUILTINFUNCTIONS		" sin cos tg cotg arcsin arccos arctg arccotg min max length char find substr sqrt "
#define OPERATOR_CHAR_PAIRS		"++\n--\n**\n//\n<=\n>=\n||\n&&\n==\n!="


// macros
#define RETURN_ERROR(val,line)	{ InterpretingError.ThrowError( val, line ); return val; }
#define SET_ERROR(val,line)		{ InterpretingError.ThrowError( val, line ); }



//////////////////////////////////////////////////////////////////////////////////////////////
// this structure is used to store an operator, there will be some predefined...
//////////////////////////////////////////////////////////////////////////////////////////////
struct OPERATOR
{
public:
	char		str[3];
	bool		LeftToRight; // determines associativity
	int			Priority; // 0-1000, 1000 is the most important
} ;



//////////////////////////////////////////////////////////////////////////////////////////////
// class representing one script token (or a word) - identifier, symbol, key word, etc.
//////////////////////////////////////////////////////////////////////////////////////////////
class CScriptToken
{
public:
					CScriptToken();

	int				Type; // specifies a token type
	CString			Content; // text content of this token
};


typedef std::vector<CScriptToken>					TOKENS;
typedef std::vector<CScriptToken>::iterator			TOKENS_ITERATOR;



//////////////////////////////////////////////////////////////////////////////////////////////
// class specifying one script line, that is one command
//////////////////////////////////////////////////////////////////////////////////////////////
class CScriptLine
{
public:
					CScriptLine();
	void			Print( bool lineNumbers = true, bool lineTypes = true, bool tokenTypes = false );

	TOKENS			Tokens; // a list of script tokens
	int				Number; // line number
	bool			Directive; // true if this line starts with # character which is signalizing a directive
	int				Type; // script line type
};


typedef std::vector<CScriptLine>					LINES;
typedef std::vector<CScriptLine>::iterator			LINES_ITERATOR;
typedef LINES_ITERATOR							*	LINES_ITERATOR_PTR;



// extern global variables:
extern const	OPERATOR		opPowering;
extern const	OPERATOR		opUnaryPlus;
extern const	OPERATOR		opUnaryMinus;
extern const	OPERATOR		opLogicalNot;
extern const	OPERATOR		opMultiplication;
extern const	OPERATOR		opDivision;
extern const	OPERATOR		opMultiplicationF;
extern const	OPERATOR		opDivisionF;
extern const	OPERATOR		opModulus;
extern const	OPERATOR		opAddition;
extern const	OPERATOR		opSubtraction;
extern const	OPERATOR		opAdditionF;
extern const	OPERATOR		opSubtractionF;
extern const	OPERATOR		opConcatenation;
extern const	OPERATOR		opLess;
extern const	OPERATOR		opGreater;
extern const	OPERATOR		opLessEq;
extern const	OPERATOR		opGreaterEq;
extern const	OPERATOR		opEquality;
extern const	OPERATOR		opInequality;
extern const	OPERATOR		opLogicalAnd;
extern const	OPERATOR		opLogicalOr;
extern const	OPERATOR		opAssignment;
