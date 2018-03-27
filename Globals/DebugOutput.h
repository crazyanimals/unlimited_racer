/*
	DebugOutput:	Used for output debug messages (warning or other info)
	Creation Date:	12.11.2003
	Last Update:	18.3.2004
	Author:			Roman Margold
*/	
	
#pragma once

#include "stdafx.h"
#include "Globals.h"
#include "ErrorHandler.h"


// macro definitions
#define DEBUGOUTPUTFILENAME		_T( "DebugOutput.log" )

// outputs a debug message or value
// n means the priority of debug message, when a 1 means "I really want to write this msg" while 4 means "I dont want to write it as much..."
#define OUTMSG(m,n)				{ if ( n <= DebugLevel ) DebugOutput.MsgOut( _T( m ) ); }
#define OUTS(m,n)				{ if ( n <= DebugLevel ) DebugOutput.MsgOutS( _T( m ) ); }
#define OUTSN(m,n)				{ if ( n <= DebugLevel ) DebugOutput.MsgOutSN( _T( m ) ); }
#define OUTI(m,n)				{ if ( n <= DebugLevel ) DebugOutput.MsgOutI( _T( m ) ); }
#define OUTIN(m,n)				{ if ( n <= DebugLevel ) DebugOutput.MsgOutIN( _T( m ) ); }
#define OUTF(m,n)				{ if ( n <= DebugLevel ) DebugOutput.MsgOutF( _T( m ) ); }
#define OUTFN(m,n)				{ if ( n <= DebugLevel ) DebugOutput.MsgOutFN( _T( m ) ); }
#define OUTTIME(n)				{ if ( n <= DebugLevel ) DebugOutput.MsgOutTime(); }
#define OUTDATETIME(n)			{ if ( n <= DebugLevel ) DebugOutput.MsgOutDateTime(); }
#define OUTTIMEN(n)				{ if ( n <= DebugLevel ) DebugOutput.MsgOutTimeN(); }
#define OUTDATETIMEN(n)			{ if ( n <= DebugLevel ) DebugOutput.MsgOutDateTimeN(); }


extern CErrorHandler	DebugOutput;
extern int				DebugLevel;


void		CALLBACK DebugOutputCallBack( CAtlString msg, int level ); // callback function that uses an DEBUGOUTPUTFILE to output debug msgs when level is equal or less than the app DebugLevel 
