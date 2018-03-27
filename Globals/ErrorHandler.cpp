#include "stdafx.h"
#include "ErrorHandler.h"


//global variables
CErrorHandler	ErrorHandler;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits ErrorHandler, sets output function and 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::Init( void (CALLBACK *pOutputCallBack)( CAtlString msg, int level ) )
{
	pOutputFunc = pOutputCallBack;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs a string via this handler, it don't take it as an error, but just comment (warning,...)
// ends a line with \n
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOut( CAtlString msg )
{
	pOutputFunc( msg + _T( "\n" ), 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs a string via this handler
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutS( CAtlString msg )
{
	pOutputFunc( msg, 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs a string via this handler
// ends line with \n
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutSN( CAtlString msg )
{
	pOutputFunc( msg + _T( "\n" ), 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs an integer via this handler
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutI( int val )
{
	TCHAR str[20];
	_stprintf( str, _T( "%li" ), val );
	pOutputFunc( str, 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs an integer via this handler
// ends line with \n
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutIN( int val )
{
	TCHAR str[20];
	_stprintf( str, _T( "%li\n" ), val );
	pOutputFunc( str, 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs a float via this handler
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutF( float val )
{
	TCHAR str[20];
	_stprintf( str, _T( "%f" ), val );
	pOutputFunc( str, 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs a float via this handler
// ends line with \n
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutFN( float val )
{
	TCHAR str[20];
	_stprintf( str, _T( "%f\n" ), val );
	pOutputFunc( str, 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs date and time via this handler
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutDateTime()
{
	TCHAR str[50];
	SYSTEMTIME tim;
	
	GetLocalTime( &tim );
	_stprintf( str, _T( "%i-%i-%i %i:%i:%i.%i" ), tim.wYear, tim.wMonth, tim.wDay, tim.wHour, tim.wMinute, tim.wSecond, tim.wMilliseconds );
	pOutputFunc( str, 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs date and time via this handler, ends line with \n
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutDateTimeN()
{
	TCHAR str[50];
	SYSTEMTIME tim;
	
	GetLocalTime( &tim );
	_stprintf( str, _T( "%i-%i-%i %i:%i:%i.%i\n" ), tim.wYear, tim.wMonth, tim.wDay, tim.wHour, tim.wMinute, tim.wSecond, tim.wMilliseconds );
	pOutputFunc( str, 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs time via this handler
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutTime()
{
	TCHAR str[30];
	SYSTEMTIME tim;
	
	GetLocalTime( &tim );
	_stprintf( str, _T( "%i:%i:%i.%i" ), tim.wHour, tim.wMinute, tim.wSecond, tim.wMilliseconds );
	pOutputFunc( str, 0 );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// outputs time via this handler, ends line with \n
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::MsgOutTimeN()
{
	TCHAR str[30];
	SYSTEMTIME tim;
	
	GetLocalTime( &tim );
	_stprintf( str, _T( "%i:%i:%i.%i\n" ), tim.wHour, tim.wMinute, tim.wSecond, tim.wMilliseconds );
	pOutputFunc( str, 0 );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// translates the ERRSOCKET error to a description string
//
//////////////////////////////////////////////////////////////////////////////////////////////
CString CErrorHandler::TranslateSocketErr( int err )
{
	switch ( err )
	{
	case WSAEACCES: return "Permission denied.";
	case WSAEINVAL: return "Invalid argument or socket in a wrong state.";
	case WSAEMFILE: return "Too many open files.";
	case WSAEWOULDBLOCK: return "Resource temporarily unavailable.";
	case WSAEINPROGRESS: return "Another blocking operation already in progress.";
	case WSAEALREADY: return "Such an operation already in progress.";
	case WSAENOTSOCK: return "Not a valid socket.";
	case WSAEDESTADDRREQ: return "Destination address required.";
	case WSAEMSGSIZE: return "Message too long.";
	case WSAEPROTOTYPE: return "Protocol wrong type for socket.";
	case WSAENOPROTOOPT: return "Ivalid protocol option selected.";
	case WSAEPROTONOSUPPORT: return "Protocol not supported.";
	case WSAEADDRINUSE: return "Specified address (IP:port) already in use.";
	case WSAENETDOWN: return "Network is down.";
	case WSAENETUNREACH: return "Network is unreachable.";
	case WSAENETRESET: return "Network dropped connection on reset.";
	case WSAECONNABORTED: return "Some other software caused connection abort.";
	case WSAECONNRESET: return "An existing connection was forcibly closed by the remote host.";
	case WSAETIMEDOUT: return "Connection timed out.";
	case WSAENOTCONN: return "Socket is not connected.";
	case WSAECONNREFUSED: return "No connection could be made because the target computer actively refused it.";
	case WSAEHOSTDOWN: return "Host is down.";
	case WSAHOST_NOT_FOUND: return "Host not found.";
	default: return "Unknown network problem.";
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// translates the error code returned from an XACT engine to a short description string
//
//////////////////////////////////////////////////////////////////////////////////////////////
CString CErrorHandler::TranslateXACTErr( HRESULT err )
{
	switch (err)
	{
	case XACTENGINE_E_OUTOFMEMORY: return "The system has run out of memory. ";
	case XACTENGINE_E_INVALIDARG: return "One or more arguments to a function or method are invalid. ";
	case XACTENGINE_E_NOTIMPL: return "The called function or method has not been implemented. ";
	case XACTENGINE_E_FAIL: return "A failure of an unspecified type has occured. ";
	case XACTENGINE_E_ALREADYINITIALIZED: return "The engine has already been initialized. ";
	case XACTENGINE_E_NOTINITIALIZED: return "The engine has not yet been initialized. ";
	case XACTENGINE_E_EXPIRED: return "The engine is a pre-release version and has expired. ";
	case XACTENGINE_E_NONOTIFICATIONCALLBACK: return "No notification callback has been registered with the engine. ";
	case XACTENGINE_E_NOTIFICATIONREGISTERED: return "A notification callback has already been registered. ";
	case XACTENGINE_E_INVALIDUSAGE: return "The method or function called cannot be used in the manner requested. ";
	case XACTENGINE_E_INVALIDDATA: return "The data used by the method or function is invalid. ";
	case XACTENGINE_E_INSTANCELIMITFAILTOPLAY: return "A sound or cue has reached an instance limit and cannot be played. ";
	case XACTENGINE_E_NOGLOBALSETTINGS: return "Global settings have not been loaded. ";
	case XACTENGINE_E_INVALIDVARIABLEINDEX: return "The specified variable index is invalid. ";
	case XACTENGINE_E_INVALIDCATEGORY: return "The specified category is invalid. ";
	case XACTENGINE_E_INVALIDCUEINDEX: return "The specified cue index is invalid. ";
	case XACTENGINE_E_INVALIDWAVEINDEX: return "The specified wave index is invalid. ";
	case XACTENGINE_E_INVALIDTRACKINDEX: return "The specified track index is invalid. ";
	case XACTENGINE_E_INVALIDSOUNDOFFSETORINDEX: return "The specified sound offset or index is invalid. ";
	case XACTENGINE_E_READFILE: return "An error has occured while attempting to read a file. ";
	case XACTENGINE_E_UNKNOWNEVENT: return "The event specified could not be found. ";
	case XACTENGINE_E_INCALLBACK: return "An invalid method or function was called during a callback function. ";
	case XACTENGINE_E_NOWAVEBANK: return "No wavebank exists for the requested operation. ";
	case XACTENGINE_E_SELECTVARIATION: return "A variation could not be selected. ";
	case XACTENGINE_E_AUDITION_WRITEFILE: return "An error has occured while writing a file during auditioning. ";
	case XACTENGINE_E_AUDITION_NOSOUNDBANK: return "A required sound bank is missing. ";
	case XACTENGINE_E_AUDITION_INVALIDRPCINDEX: return "A required wave bank is missing. ";
	case XACTENGINE_E_AUDITION_MISSINGDATA: return "A required set of data is missing for the requested audition command. ";
	case XACTENGINE_E_AUDITION_UNKNOWNCOMMAND: return "The audition command requested is not known. ";
	default: return "Unknown error. ";
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// calls Output function with specified CAtlString msg and explanation of an error code
//
// parameter is used to store some additional information, e.g. name of file that was not found
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CErrorHandler::HandleError( HRESULT code, CAtlString loc, CAtlString parameter )
{
	CAtlString msg( loc );

	switch ( code )
	{
	case ERRNOERROR:
		msg += _T( " : No error occurs.\n" );
		break;
	case ERRGENERIC:
		msg += _T(" : ") + parameter;
		break;
	case ERRFILENOTFOUND:
		msg += _T( " : Can not find file '" ) + parameter + _T( "'. " );
		break;
	case ERRCANNOTOPENFORWRITE:
		msg += _T( " : Can not open file '" ) + parameter + _T( "' for writing. " );
		break;
	case ERRFILECORRUPTED:
		msg += _T( " : File '" ) + parameter + _T( "' is corrupted. " );
		break;
	case ERROUTOFMEMORY:
		msg += _T( " : Not enough memory." );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case ERRNOTFOUND:
		msg += _T( " : Object or resource, that was looked for, was not found." );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case ERRINVALIDRESOURCETYPE:
		msg += _T( " : Resource, that was looked for, is of another type that was expected." );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case ERROUTOFVIDEOMEMORY:
		msg += _T( " : Not enough video memory." );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case ERRD3DGENERIC:
		msg += _T( " : " ) + parameter;
		break;
	case ERRINVALIDCONFIGURATION:
		msg += _T( " : Invalid configuration." );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case ERRINVALIDPARAMETER:
		msg += _T( " : Invalid parameter." );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case ERRTOOMANYRESOURCES:
		msg += _T( " : Too many objects and/or resources." );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case ERRNOTPREPARED:
		msg += _T( " : Device/module is not prepared. " );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case ERRSOCKET:
		int val;
		sscanf_s( parameter, "%i", &val );
		msg += _T( " : Winsock error. " );
		msg += TranslateSocketErr( val );
		if ( parameter != "" ) msg += _T( " (#" ) + parameter + _T( ")" );
		break;
	case ERRINCOMPATIBLE:
		msg += _T( " : Some modules are not compatible or the server is not compatible with your client." );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case D3DERR_OUTOFVIDEOMEMORY:
	case D3DERR_NOTAVAILABLE:
	case D3DERR_INVALIDCALL:
	case D3DXERR_INVALIDDATA:
	case D3DXFERR_BADFILETYPE:
	case E_OUTOFMEMORY:
	case DXFILEERR_FILENOTFOUND:
	case D3DERR_INVALIDDEVICE:
	case D3DERR_NOTFOUND:
	case E_FAIL:
	case E_NOTIMPL:
	case E_INVALIDARG:
		msg += _T( " - D3D error '" );
		msg += DXGetErrorString9( code );
		msg += _T( "' == ");
		msg += DXGetErrorDescription9( code );
		msg += _T( ".");
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	case XACTENGINE_E_ALREADYINITIALIZED:
	case XACTENGINE_E_NOTINITIALIZED:
	case XACTENGINE_E_EXPIRED:
	case XACTENGINE_E_NONOTIFICATIONCALLBACK:
	case XACTENGINE_E_NOTIFICATIONREGISTERED:
	case XACTENGINE_E_INVALIDUSAGE:
	case XACTENGINE_E_INVALIDDATA:
	case XACTENGINE_E_INSTANCELIMITFAILTOPLAY:
	case XACTENGINE_E_NOGLOBALSETTINGS:
	case XACTENGINE_E_INVALIDVARIABLEINDEX:
	case XACTENGINE_E_INVALIDCATEGORY:
	case XACTENGINE_E_INVALIDCUEINDEX:
	case XACTENGINE_E_INVALIDWAVEINDEX:
	case XACTENGINE_E_INVALIDTRACKINDEX:
	case XACTENGINE_E_INVALIDSOUNDOFFSETORINDEX:
	case XACTENGINE_E_READFILE:
	case XACTENGINE_E_UNKNOWNEVENT:
	case XACTENGINE_E_INCALLBACK:
	case XACTENGINE_E_NOWAVEBANK:
	case XACTENGINE_E_SELECTVARIATION:
	case XACTENGINE_E_MULTIPLEAUDITIONENGINES:
	case XACTENGINE_E_WAVEBANKNOTPREPARED:
	case XACTENGINE_E_NORENDERER:
	case XACTENGINE_E_INVALIDENTRYCOUNT:
	case XACTENGINE_E_SEEKTIMEBEYONDCUEEND:
	case XACTENGINE_E_AUDITION_WRITEFILE:
	case XACTENGINE_E_AUDITION_NOSOUNDBANK:
	case XACTENGINE_E_AUDITION_INVALIDRPCINDEX:
	case XACTENGINE_E_AUDITION_MISSINGDATA:
	case XACTENGINE_E_AUDITION_UNKNOWNCOMMAND:
	case XACTENGINE_E_AUDITION_INVALIDDSPINDEX:
		msg += _T( " - XACT error: " );
		msg += TranslateXACTErr( code );
		if ( parameter != "" ) msg += _T( "\n  (Reason/result: " ) + parameter + _T( ")" );
		break;
	default: 
		TCHAR	sVal[20];
		_stprintf( sVal, _T("%li"), code );
		msg += _T( " : Unrecognized error #" );
		msg += sVal;
		if ( parameter.GetLength() )	msg += _T( " with caption: " ) + parameter;
		break;
	}

	pOutputFunc( msg + _T( "\n" ), 0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// standard callback function that is using an ERROROUTPUTFILE to output error msgs
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK ErrOutputCallBack( CAtlString msg, int level )
{
	FILE	*fil;
	
	fil = fopen( ERROROUTPUTFILENAME, _T( "at" ) );
	fprintf( fil, _T( "%s" ), (LPCTSTR) msg );
	fclose( fil );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
