/*
	ErrorHandler:	Class used to handle all arised errors. Any error, that'll occur will be handled by HandleError method, which
					will accept 3 parameters. First one is the error code. Error codes are listed below. Second one is the location
					string, that informs about place in code, that araised an error. The last parameter will be used only sometimes,
					where any other text informations will be needed, e.g.when a generic error occurs and you want to discribe it
					outside this class or when file operation failed and you wanna specify the file name.
	Creation Date:	15.10.2003
	Last Update:	13.1.2007
	Author:			Roman Margold
*/

#pragma once

#include "stdafx.h"

// macro definitions
#define ERROROUTPUTFILENAME		_T( "ErrOutput.log" )

#define __ERRORMSG(code, location, param, ret) { ErrorHandler.HandleError( code, _T( location ), _T( param ) ); return ret; }
#define ERRORMSG(code, location, param)	__ERRORMSG(code, location, param, code)



// error values
#define ERRGENERIC				-0x0FFFFFFF // generate this error whenever you think the cause of the error is already noted and this is just a message about error's result
#define ERRD3DGENERIC			-0x0FFFFFFE // unrecognized D3D error
#define ERRNOERROR				 0x00000000 // no error occurs
#define ERRFILENOTFOUND			-0x00010000
#define ERRCANNOTOPENFORWRITE	-0x00020000
#define ERRFILECORRUPTED		-0x00030000
#define ERRFILEWRITE			-0x00040000
#define ERRWRONGFILEHANDLE		-0x00050000
#define ERROUTOFMEMORY			-0x00110000
#define ERROUTOFVIDEOMEMORY		-0x00120000
#define ERRNOTFOUND				-0x00200000 // generate this error whenever something (whatever) was not found
#define ERRINVALIDRESOURCETYPE  -0x00210000 // ResourceManager was asked for a resource of an ID of another type that the metod was waiting for
#define ERRWRONGMESHFORMAT		-0x00220000
#define ERRNOTPREPARED			-0x00230000 // this error singalizes wrong state of any part of the application, module or device
#define ERRINVALIDCONFIGURATION	-0x00300000
#define ERRINVALIDPARAMETER		-0x00310000
#define ERRTOOMANYRESOURCES		-0x00320000
#define ERRSOCKET				-0x00330000
#define ERRINVALIDDATA			-0x00340000 // presented data are invalid
#define ERRINCOMPATIBLE			-0x00350000 // incompatible version of used components or client x server incompatibility


// error handling class
// first, you must call Init with a pointer to static function which realizes the error message output
// then, call HandleError with defined parameters
class CErrorHandler
{
private: 
	void		(CALLBACK *pOutputFunc) ( CAtlString msg, int level ); // output function pointer
	CString		TranslateSocketErr( int err );
	CString		TranslateXACTErr( HRESULT err );

public:
	void		Init( void (CALLBACK *pOutputCallBack) ( CAtlString msg, int level ) ); // inits ErrorHandler, sets output function
	void		HandleError( HRESULT code, CAtlString location, CAtlString parameter ); // calls Output function with specified CAtlString msg and explanation of an error code
	void		MsgOut( CAtlString msg ); // outputs a string via this handler, it don't take it as an error, but just comment (warning,...); ends a line with \n
	void		MsgOutS( CAtlString msg ); // outputs a string via this handler
	void		MsgOutSN( CAtlString msg ); // outputs a string via this handler, ends line with \n
	void		MsgOutI( int val ); // outputs an integer via this handler
	void		MsgOutIN( int val ); // outputs an integer via this handler, ends line with \n
	void		MsgOutF( float val ); // outputs a float via this handler
	void		MsgOutFN( float val ); // outputs a float via this handler, ends line with \n
	void		MsgOutDateTime(); // outputs date and time via this handler
	void		MsgOutDateTimeN(); // outputs date and time via this handler, ends line with \n
	void		MsgOutTime(); // outputs time via this handler
	void		MsgOutTimeN(); // outputs time via this handler, ends line with \n
} ;


// varibles
extern CErrorHandler	ErrorHandler;


// functions
void		CALLBACK ErrOutputCallBack( CAtlString msg, int level ); // standard callback function that uses an ERROROUTPUTFILE to output error msgs
