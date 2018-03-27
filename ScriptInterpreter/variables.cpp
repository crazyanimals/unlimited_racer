#include "stdafx.h"
#include "variables.h"


//////////////////////////////////////////////////////////////////////////////////////////////
// returns the value of a variable as a CString
//////////////////////////////////////////////////////////////////////////////////////////////
CAtlString CVariable::GetString()
{
	return value;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// returns the value of a variable as a float
//////////////////////////////////////////////////////////////////////////////////////////////
FLOAT CVariable::GetFloat()
{
	float	f;

	if ( !value.GetLength() ) return 0.0f;
	if ( sscanf( (LPCTSTR) value, "%f", &f ) ) return f;
	else return 0.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// returns the value of a variable as an integer
//////////////////////////////////////////////////////////////////////////////////////////////
int CVariable::GetInt()
{
	int		i;

	if ( !value.GetLength() ) return 0;
	if ( sscanf( (LPCTSTR) value, "%li", &i ) ) return i;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// sets the value of a variable
//////////////////////////////////////////////////////////////////////////////////////////////
void CVariable::SetValue( FLOAT val )
{
	char	str[520];

	sprintf( str, "%f", val );
	
	value = str;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// sets the value of a variable
//////////////////////////////////////////////////////////////////////////////////////////////
void CVariable::SetValue( int val )
{
	char	str[520];

	sprintf( str, "%li", val );
	
	value = str;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////