#include "stdafx.h"


#ifndef SCRIPTINTERPRETER_EXPORTS
#define SCRIPTINTERPRETER_EXPORTS
#endif

#include "Variables.h"
#include "ScriptInterpreter.h"
#include "ScriptInterpreterTools.h"
#include "Script.h"


CInterpretingError	InterpretingError;
HRESULT				CInterpretingError::Error;
UINT				CInterpretingError::LineNumber;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// main DLL function used to initialize error class
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	switch( fdwReason ) 
	{ 
	case DLL_PROCESS_ATTACH:
		InterpretingError = CInterpretingError::CInterpretingError();
		break;
	}

	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// This function loads a script from specified file and calls its interpretation.
//
// Returns a zero value when no error occurs, otherwise, this returns a nonzero value,
// for further information, try to check InterpretingError object.
//
// Also returns a return value of a script by returnVal parameter.
//
//////////////////////////////////////////////////////////////////////////////////////////////
SI_EXPORT HRESULT InterpretScript( LPCTSTR fileName, EXTERNCALL ExternFunc, CString &returnVal )
{
	int				majVer, minVer;
	HRESULT			hr;
	CScript			Script;

	hr = Script.Load( fileName );
	if ( hr ) return hr;

	if ( Script.InterpretVersionNeeded( majVer, minVer ) ) 
	{
		if ( (SCRIPTINTERPRETER_MAJOR_VERSION < majVer) ||
			(SCRIPTINTERPRETER_MAJOR_VERSION == majVer && SCRIPTINTERPRETER_MINOR_VERSION < minVer) )
			return ERR_WRONGINTERPRETVERSION;
	}

	return Script.Interpret( ExternFunc, returnVal );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// fills parameters with this DLL's version information
//
//////////////////////////////////////////////////////////////////////////////////////////////
SI_EXPORT void GetDLLVersion( int &major, int &minor, CString &descrip )
{
	major = SCRIPTINTERPRETER_MAJOR_VERSION;
	minor = SCRIPTINTERPRETER_MINOR_VERSION;
	descrip = SCRIPTINTERPRETER_VERSION_DESCRIPTOR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor of CInterpretingError sets the error to unidentified
//
//////////////////////////////////////////////////////////////////////////////////////////////
CInterpretingError::CInterpretingError()
{
	Error = ERR_NOERROR;
	LineNumber = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this fills the structure with error information
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CInterpretingError::ThrowError( HRESULT err, UINT lineNo )
{
	Error = err;
	LineNumber = lineNo;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads last error information, says the code of an error, line number, when an error occurs
// and a description of that error; returns false if no error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CInterpretingError::PickError( HRESULT &err, CString &des, UINT &lineNo )
{
	err = Error;
	lineNo = LineNumber + 1;
	switch ( err )
	{
	case ERR_UNIDENTIFIEDERROR:
		des = "unidentified error";
		break;
	case ERR_UNKNOWNCHARACTER:
		des = "a character that is not supplied in this interpreter version found";
		break;
	case ERR_WRONGSYNTAX:
		des = "wrong syntax";
		break;
	case ERR_FILENOTFOUND:
		des = "file not found";
		break;
	case ERR_WRONGINTERPRETVERSION:
		des = "version of this interpret (" + CString( SCRIPTINTERPRETER_VERSION );
		des += ") doesnt satisfy script requirements";
		break;
	case ERR_IDENTIFIEREXPECTED:
		des = "unknown character found when identifier expected";
		break;
	case ERR_NOERROR:
		des = "actually, not an error :o]";
		break;
	case ERR_LINEENDEXPECTED:
		des = "end of line expected, but an unknown character found";
		break;
	case ERR_ALREADYDEFINED:
		des = "identifier already defined in this block (multiple definition not allowed)";
		break;
	case ERR_NOTENOUGHMEMORY:
		des = "out of global memory";
		break;
	case ERR_UNEXPECTEDENDOFLINE:
		des = "end of line found when 'something' expected";
		break;
	case ERR_LEFTBRACKETEXPECTED:
		des = "left bracket expected after a function name";
		break;
	case ERR_UNKNOWNIDENTIFIER:
		des = "unknown identifier found (possibly undefined variable)";
		break;
	case ERR_NOTENOUGHPARAMETERS:
		des = "this function takes more parameters";
		break;
	case ERR_TOOMANYPARAMETERS:
		des = "this function doesnt take so many parameters";
		break;
	case ERR_RETURNVALUEEXPECTED:
		des = "script must return a value";
		break;
	case ERR_LVALUEEXPECTED:
		des = "variable name expected";
		break;
	case ERR_DIVISIONBYZERO:
		des = "division by zero";
		break;
	case ERR_RANGECHECK:
		des = "value is outside allowed interval";
		break;
	case ERR_UNFINISHEDIFCLAUSE:
		des = "if clause must be followed either by then or else clause";
		break;
	case ERR_UNEXPECTEDENDOFFILE:
		des = "some code expected, but an end of file reached";
		break;
	case ERR_EFC:
		des = "an error occurs inside external function";
		break;
	case ERR_EXPRESSIONEXPECTED:
		des = "an expression expected";
		break;
	case ERR_INVALIDDIRECTIVEPARAMETER:
		des = "the directive doesn't take such parameter";
		break;
	case ERR_ILLEGALEXPONENT:
		des = "an exponent must be an integer greater then zero";
		break;
	default:
		des = "unrecognized error";
		break;
	}
	   	 
	Error = ERR_NOERROR;
	LineNumber = 0;

	return ( err != ERR_NOERROR );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

