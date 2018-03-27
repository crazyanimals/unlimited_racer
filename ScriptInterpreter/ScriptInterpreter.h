/*
	ScriptInterpreter:	DLL that interprets special unnamed scripting language. Its syntax will be written in the end of this file.
						Using this is very simple. Just load this dll to your program and call InterpretScript function to interpret your
						source code. Passed call back function will be used for every call.

						This is only exported header file.
	Creation Date:		25.2.2004
	Last Update:		7.11.2004
	Author:				Roman Margold

*/

//TODO: Zcela prepracovat princip EFC: nebude se to volat pomoci call, ale jako standardni built-in funkce. 
//		Pribyde prikaz define, ktery se bude definovat jmeno externi funkce, tak se utvori seznam nevestavenych funkci
//		a oproti tomuto seznamu se bude kontrolovat existence pouzite funkce. Pokud je nadefinovana, zavola se
//		EFC s touto uzivatelskou funkci. Tim se bude moci volat uzivatelska funkce stejne jako built-in a bude moci vracet
//		hodnotu primo do programu, pricemz pujde zavolat i jako procedure, tady opomenutim parametru.

#pragma once


#include "stdafx.h"


#ifdef SCRIPTINTERPRETER_EXPORTS
#define SI_EXPORT __declspec(dllexport)
#else
#define SI_EXPORT __declspec(dllimport)
#endif


// global macro settings
#define SI_MAX_PARAM_COUNT			16
#define DEFAULT_HASH_SIZE			100

// error codes
#define ERR_NOERROR					0
#define ERR_FILENOTFOUND			-1
#define ERR_WRONGINTERPRETVERSION	-2
#define ERR_UNIDENTIFIEDERROR		-32
#define ERR_UNKNOWNCHARACTER		-33
#define ERR_WRONGSYNTAX				-34
#define ERR_IDENTIFIEREXPECTED		-64
#define ERR_LINEENDEXPECTED			-65
#define ERR_UNEXPECTEDENDOFLINE		-66
#define ERR_LEFTBRACKETEXPECTED		-67
#define ERR_UNKNOWNIDENTIFIER		-68
#define ERR_NOTENOUGHPARAMETERS		-69
#define ERR_TOOMANYPARAMETERS		-70
#define ERR_RETURNVALUEEXPECTED		-71
#define ERR_LVALUEEXPECTED			-72
#define ERR_UNFINISHEDIFCLAUSE		-73
#define ERR_UNEXPECTEDENDOFFILE		-74
#define ERR_EXPRESSIONEXPECTED		-75
#define ERR_INVALIDDIRECTIVEPARAMETER	-76
#define ERR_DIVISIONBYZERO			-128
#define ERR_ALREADYDEFINED			-129
#define ERR_RANGECHECK				-130
#define ERR_ILLEGALEXPONENT			-131
#define ERR_NOTENOUGHMEMORY			-256
#define ERR_EFC						-257


//////////////////////////////////////////////////////////////////////////////////////////////
//
// ExternalFunctionCall prototype
//
// returnVal parameters is used to return a value from calling function
//
// paramCount must be filled with proper count of parameters, that is limited
// the maximum count of parameters is SI_MAX_PARAM_COUNT
//
// params filed contains all of the parameters
// first item in this field contains function name, so the field must be one item larger
// then the count of parameters
//
//////////////////////////////////////////////////////////////////////////////////////////////
typedef HRESULT ( CALLBACK *EXTERNCALL ) ( CString &returnVal, int paramCount, CString params[] );



// this class is used to send errors to your program, use PickError to get the last one occured
class SI_EXPORT CInterpretingError
{
public:
						CInterpretingError();
	static void			ThrowError( HRESULT err, UINT lineNo );
	static bool			PickError( HRESULT &err, CString &des, UINT &lineNo );

private:
	static HRESULT		Error;
	static UINT			LineNumber;
} ;



// export functions
SI_EXPORT	HRESULT				InterpretScript( LPCTSTR fileName, EXTERNCALL ExternFunc, CString &returnVal );
SI_EXPORT	void				GetDLLVersion( int &major, int &minor, CString &descrip );
extern		CInterpretingError	InterpretingError;


/*

ARITHMETIC OPERATORS
	Binary
		^	power operator defined above natural numbers
		+	addition
        -	subtraction
		*	multiplication
		/	division
		++	floating point addition
		--	floating point subtraction
		**	floating point multiplication
        //	floating point division
		%	modulus
		.	string concatenation
		==	equality
		!=	inequality
		<	less than
		>	greater than
		<=	less than or equal
		>=	greater than or equal
		||	logical OR
		&&	logical AND
	Unary
		+	unary plus
		-	unary minus
		!	logical NOT
	Assignment
		=

FUNCTIONS
	Goniometric functions
		sin(x), cos(x), tg, cotg, arcsin, arccos, arctg, arccotg - in radians
	Integer functions
		min(a,b)	returns a value of greater number from a, b
		max(a,b)	returns a value of greater number from a, b
		power(a,b)	returns a^b; a and b are read as an integers
	Math functions
		sqrt(a)		returns the square root of a - returns float value
	String functions
		length(s)		returns length of a string
		char(s,a)		returns a character on specified position (zero based)
		find(s1,s2)		returns a position of first occurance s1 in s2 (or -1)
		substr(s,a,b)	returns a substring of s of length b starting at position a

STRINGS
	String characters
		"
	Escape sequences
		\\
		\"

COMMENTS
	Line comments
		;
	Block comments
		;-	begin
		-;	end
		Block comments are not nested.

OTHER SYMBOLS
	#	if first on line, this specifies an interpreting directive
	()	just a brackets
	,	separator - separates multiple variable declaration or function parameters
	{}	block brackets
	:	another type of separator

DIRECTIVES
	version	a b		specifies major and minor version needed to interpret this script 

SYNTAX

IF_CLAUSE:
if expression
[THEN_CLAUSE]
[ELSE_CLAUSE]

THEN_CLAUSE:
then expression
|
then
{
commands
}

ELSE_CLAUSE:
else expression
|
else
{
commands
}


FOR_CLAUSE:
for newvariable,count,expression // this doesn't evaluate to a new block
|
for newvariable,count
{
commands
}

FOR_CLAUSE is expanded to the following commands:
var newvariable	// if the variable already exists, this will use the existing one
{
newvariable = 1
if newvariable>count
then goto JUMP
command(s)
newvariable = newvariable + 1
@JUMP
}


WHILE_CLAUSE:
while expression: expression
|
while expression
{
commands
}


CALL_COMMAND:
call function_name:retVar[,param1,param2,...]
retVar is a name of variable which will be filled with return value
param1 .. paramn are input parameters, the maximal count of parameters is 16


*/