#include "stdafx.h"
#include "DebugOutput.h"

CErrorHandler	DebugOutput;
int				DebugLevel = 0;

//////////////////////////////////////////////////////////////////////////////////////////////
// callback function that uses an DEBUGOUTPUTFILE to output debug msgs when level is equal or less than the app DebugLevel 
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK DebugOutputCallBack(CAtlString msg, int level)
{
	if ( level > DebugLevel ) return;
	
	FILE	*fil;
	
	fil = fopen( DEBUGOUTPUTFILENAME, _T("at") );
	fprintf( fil, _T("%s"), (LPCTSTR) msg );
	fclose( fil );
}
