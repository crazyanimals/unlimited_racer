/*
	CVariable:			This class is storing a variable used in script interpreter.
						A CVariable object can convert itself from float/int/string to float/int/string.

	Creation Date:		22.4.2004
	Last Update:		17.11.2004
	Author:				Roman Margold

*/


#pragma once

#include "stdafx.h"
#include "ScriptInterpreterTools.h"


#ifdef SI_EXPORT
#undef SI_EXPORT
#endif // SI_EXPORT

#ifdef SCRIPTINTERPRETER_EXPORTS
#define SI_EXPORT __declspec(dllexport)
#else
#define SI_EXPORT __declspec(dllimport)
#endif



class SI_EXPORT CVariable
{
public:
					CVariable( LPCTSTR varName ) { value = ""; name = varName; };
					CVariable() { value = ""; name = ""; };

	CString			GetString(); // returns the value of a variable as a CString
	FLOAT			GetFloat(); // returns the value of a variable as a float
	int				GetInt(); // returns the value of a variable as an integer

	inline void		SetValue( CAtlString val ) { value = val; }; // sets the value of a variable
	void			SetValue( FLOAT val ); // sets the value of a variable
	void			SetValue( int val ); // sets the value of a variable
	
private:
	CAtlString		value; // contained value
	CAtlString		name; // variable name

} ;


typedef std::map<CString, CVariable *>				VARIABLES;
typedef std::map<CString, CVariable *>::iterator	VARIABLES_ITERATOR;
typedef std::pair<CString, CVariable *>				VARIABLES_PAIR; // pair needed to inserting to a map
typedef std::pair<VARIABLES_ITERATOR, bool>			VARIABLES_RETURN_PAIR; // pair needed as a return value when inserting to a map
