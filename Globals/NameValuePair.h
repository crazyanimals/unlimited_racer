/*
	NameValuePair:	NAMEVALUEPAIR structure represents standard Name=Value pair, that are often used in configuration files.
					Because the value type is commonly not specified at read time, there are three value members that
					covers main used types - string, float and int. Each of them are filled when read, if it is possible.
	Creation Date:	3.12.2003
	Last Update:	15.4.2005
	Author:			Roman Margold

*/

#pragma once

#include "Globals.h"
#include "ErrorHandler.h"
// this covers kinds of pairs that could be found in configuration file


// NAMEVALUETYPE enumeration type defines type of line that was read from a configuration file
// there are several types defined, some of them are flags, some are disjunctive - that means
// not all combinations are allowed, basically only one from following flags should be set:
// NVPTYPE_LABEL, NVPTYPE_TAG, NVPTYPE_VARIABLE 
enum NAMEVALUETYPE
{
	NVPTYPE_UNDEFINED	= 0x0000, // basically, this shouldn't occur; anyway, if it does, it likely implies an error

	NVPTYPE_EMPTYLINE	= 0x0001, // this line is either empty or contains only comment

	NVPTYPE_LABEL		= 0x0004, // file line (NAMEVALUE pair) is a label (e.g.[LABEL])

	NVPTYPE_TAG			= 0x0010, // flag - NAMEVALUE pair is any type of TAG
	  // if NVPTYPE_TAG bit is set, the return value is one of following:
	NVPTYPE_TAGBEGIN	= 0x0030, // opening tag (<TAG>)
	NVPTYPE_TAGEND		= 0x0050, // closing tag (</TAG>)
	  
	NVPTYPE_VARIABLE	= 0x0100, // flag - NAMEVALUE pair is any type of allowed variable
	  // if NVPTYPE_VARIABLE bit is set, the return value is one of the following  values:
	NVPTYPE_STRING		= 0x0300, // string variable (name=value)
	NVPTYPE_FLOAT		= 0x0500, // float variable (name=1.23456)
	NVPTYPE_INTEGER		= 0x0900, // integer variable (name=-456578456 or name=0xffaabbcc)
	
	NVPTYPE_ERROR		= 0x1000  // an error occurs - config file parsing process should be stopped
	  // this value is returned back from your callback function to the config loader function
} ;



struct NAMEVALUEPAIR;



// structure used to store Name=Value pair
struct NAMEVALUEPAIR
{
public:
	typedef std::vector<CAtlString>		STRING_LIST;
	typedef STRING_LIST::iterator		STRING_ITER;

	// this structure is used in SaveValue method for passing important pointers to LoadCallBack
	struct POINTERS
	{
		NAMEVALUEPAIR		*	pPair; // pointer to the calling NAMEVALUEPAIR object
		STRING_LIST			*	pFileLines; // contains file content (each line as one string)
		BOOL					bFound; // specifies whether the name=value pair was already found
		CAtlString				csValue; // value that is looked for
	}; 

private:
	// variables
	CAtlString				Name; // name of the config pair
	CAtlString				OriginalString; // stores the original file line

	NAMEVALUETYPE			Flags; // this is filled by Load method and says what kind of pair is the result (LABEL, TAG, INTEGER, FLLOAT, etc.)

	CAtlString				sValue; // Value part of the config pair understood as a string
	float					fValue; // Value part of the config pair understood as a float value
	int						iValue; // Value part of the config pair understood as a float value

public:
	// public variables
	LPVOID					ReservedPointer; // Pointer that could be used to post structures to the called calback function


	// data obtaining methods
	inline const CAtlString &	GetName() const { return Name; } // returns name part of name-value-pair
	inline const CAtlString &	GetOriginalString() const { return OriginalString; } // returns the original file line
	inline int					GetInt() const { return iValue; } // returns integer value associated with this structure
	inline float				GetFloat() const { return fValue; } // returns integer value associated with this structure
	inline const CAtlString &	GetString() const { return sValue; } // returns integer value associated with this structure
	inline NAMEVALUETYPE		GetFlags() const { return Flags; } // returns name-value-pair flags

	// executive methods
	static HRESULT			Load( CAtlStringA fileName, void (CALLBACK *CallBack)(NAMEVALUEPAIR *pPair), LPVOID Reserved = NULL ); // opens a file and calls CallBack for every read pair
	HRESULT					SaveValue( CAtlString fileName ); // this method writes the name=value pair to a file
	HRESULT					SaveValue( CAtlString fileName, CAtlString oldValue ); // this method changes the name=value pair in a file
	inline void				ThrowError() { Flags = NVPTYPE_ERROR; }; // sets the error indicating flag

	// property setting methods
	inline void				SetFlags( NAMEVALUETYPE flags ) { Flags = flags; }
	inline void				SetInt( int val ) { iValue = val; Flags = NVPTYPE_INTEGER; } // this changes the integer part of the pair and also sets Flags to a proper state
	inline void				SetFloat( float val ) { fValue = val; Flags = NVPTYPE_FLOAT; } // this changes the integer part of the pair and also sets Flags to a proper state
	inline void				SetString( CAtlString val ) { sValue = val; Flags = NVPTYPE_STRING; } // this changes the integer part of the pair and also sets Flags to a proper state
	inline void				SetName( CAtlString name ) { Name = name; } // sets the property name string

} ;


typedef NAMEVALUEPAIR		*LPNAMEVALUEPAIR;
