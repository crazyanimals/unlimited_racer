#include "stdafx.h"
#include "NameValuePair.h"



//////////////////////////////////////////////////////////////////////////////////////////////
//
// opens a specified file and calls CallBack function for every read item
//
// Any line can contain one item; at first, lines are truncated if they contains comments and
// then they are trimmed of whitecpaces from left and right.
// Any identifier or value can contain 256 characters at max, further characters are ignored.
// 
// possible items pollows:
// 1) [label name] - anything after parenthesis is ignored; define new block
// 2) <tag name> - starts a new block that could be hierarchically built as a tree
// 3) </tag name> - ends a block of specified name
// 4) name=value - name-value pair; a variable of specified name is set to a specified value;
//                 value can be int, float or a string
//
// The type of an item that was read is stored in a NAMEVALUEPAIR structure as like as its
// possible value.
//
// Called routine could also return an andication of an error by filling Flags property
// of NAMEVALUEPAIR by value NVPTYPE_ERROR. In that case, the loading process is immediatelly
// stopped and returns an error.
//
// returns a zero when successful, or a nonzero error code
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT NAMEVALUEPAIR::Load( CAtlStringA fileName, void (CALLBACK *CallBack) ( NAMEVALUEPAIR *pPair ), LPVOID Reserved )
{
	FILE	*		fil;
	BOOL			konec = false;
	CAtlString			str;
	_TCHAR			chr = 'a';
	NAMEVALUEPAIR	pair;
	int				index;
	char			ret[520];


	pair.ReservedPointer = Reserved;


	fil = fopen( fileName, "rt" );
	if ( !fil )
	{	
		fileName.OemToAnsi();
		ErrorHandler.HandleError( ERRFILENOTFOUND, _T( "NAMEVALUEPAIR::Load()" ), (TCHAR *) fileName.GetString() );
		return ERRFILENOTFOUND;
	}
	
	while ( !konec ) // to the end of file
	{
		// reset values
		pair.Flags = NVPTYPE_UNDEFINED;
		pair.fValue = 0.0f;
		pair.iValue = 0;
		pair.sValue = "";
		pair.Name = "";
		
		// read next line
		if ( !fgets( ret, 520, fil ) ) break;

		str = ret;
		pair.OriginalString = str;   // save the original line content

		// truncate a possible comment
		index = str.Find( ';' );
		if ( index >= 0 ) str.Truncate( index );

		// trim all whitespaces
		str.Trim();
		
		if ( !str.GetLength() ) // this line is empty
		{
			pair.Flags = NVPTYPE_EMPTYLINE;
			goto CallAndQuit;
		}
		
		// process line - recognize its content
		if ( str[0] == '[' ) // it's a label
		{
			index = str.Find( ']' );
			if ( index > 0 ) // properly enclosed label
			{
				pair.Name = str.Mid( 1, index - 1 );
				pair.Flags = NVPTYPE_LABEL;
			}
		}
		else if ( str[0] == '<' ) // it's a tag
		{
			if ( ( str.GetLength() > 1 ) && ( ( index = str.Find( '>', 1 ) ) > 0 ) ) // properly enclosed tag
			{
				if ( str[1] == '/' ) // ending tag
				{
					pair.Name = str.Mid( 2, index - 2 );
					pair.Flags = NVPTYPE_TAGEND;
				}
				else 
				{
					pair.Name = str.Mid( 1, index - 1 );
					pair.Flags = NVPTYPE_TAGBEGIN;
				}
			}
		}
		else if ( ( index = str.Find( '=' ) ) > 0 ) // it's a proper name-value-pair
		{
			int		itemRead;
			pair.Name = str.Left( index );
			pair.sValue = str.Right( str.GetLength() - index - 1 );
			pair.Flags = NVPTYPE_STRING;

			itemRead = sscanf( (LPCTSTR) pair.sValue, "%f", &pair.fValue );
			if ( itemRead == 1 ) pair.Flags = NVPTYPE_FLOAT;

			itemRead = sscanf( (LPCTSTR) pair.sValue, "%i", &pair.iValue );
			if ( itemRead == 1 ) pair.Flags = NVPTYPE_INTEGER;
		}
		
		// CALL the callback routine
		pair.Name.Trim();
		pair.sValue.Trim();
		
	CallAndQuit:	
		CallBack( &pair );
		
		// check for an error
		if ( pair.Flags == NVPTYPE_ERROR )
		{
			ErrorHandler.HandleError( ERRFILECORRUPTED, "NAMEVALUEPAIR::Load()", fileName );
			fclose( fil );
			return ERRFILECORRUPTED;
		}
	}

	fclose( fil );

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function for SaveValue operation
// this fills the given lines list by every pPair obtained and if the variable name
// is the same as the one that is looked for, it's value is replaced
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK LoadCallBackForSaveValue( NAMEVALUEPAIR * pPair )
{
	CString					name, newName, str;
	NAMEVALUEPAIR::POINTERS	*pointers;
	char					ret[1000];
	
	pointers = (NAMEVALUEPAIR::POINTERS *) pPair->ReservedPointer;

	name = pPair->GetName();
	newName = pointers->pPair->GetName();
	
	newName.MakeLower();
	name.MakeLower();

    // the same variable already exists in the file
	if ( name == newName )
	{
		// this is the first occurance, so write it to the file (other are thrown away)
		if ( !pointers->bFound )
		{
			pointers->bFound = TRUE;
			if ( pointers->pPair->GetFlags() == NVPTYPE_STRING )
				sprintf( ret, "%s = %s\n", pointers->pPair->GetName(), pointers->pPair->GetString() );
			else if ( pointers->pPair->GetFlags() == NVPTYPE_INTEGER ) 
				sprintf( ret, "%s = %i\n", pointers->pPair->GetName(), pointers->pPair->GetInt() );
			else if ( pointers->pPair->GetFlags() == NVPTYPE_FLOAT )
				sprintf( ret, "%s = %f\n", pointers->pPair->GetName(), pointers->pPair->GetFloat() );
			else // pair has improperly set its flags
			{
				pPair->ThrowError();
				return;
			}
				
			str = ret;
			pointers->pFileLines->push_back( str ); // the old variable is replaced by the new one
		}
	}
	else // this is a different variable; just write it to the file as it was read
	{
		str = pPair->GetOriginalString();
		pointers->pFileLines->push_back( str );
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this method opens a specified file and writes the name=value pair into it
// if the specified pair doesn't exist in the file this adds a single line with name=value pair
// to the end of the file
// if a variable with the same name already exists in the file, the first occurance is modified
// (that means its value is set to the new one) and every next occurance is removed completely
//
// the return value should be zero; if any error occurs a nonzero error code is returned
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	NAMEVALUEPAIR::SaveValue( CAtlString fileName )
{
	NAMEVALUEPAIR			pair;
	STRING_LIST				fileLines;
	HRESULT					hr;
	POINTERS				pointers;
	CAtlString				str;
	char					ret[1000];
	FILE				*	f;


	pointers.bFound = FALSE;
	pointers.pPair = this;
	pointers.pFileLines = &fileLines;

	
	// load the file
	hr = Load( fileName, LoadCallBackForSaveValue, &pointers );
	if ( hr ) ERRORMSG( ERRGENERIC, "NAMEVALUEPAIR::SaveValue()", "Configuration pair could not be saved." );

	
	// if the variable was not found in the file add a new line at the end
	if ( !pointers.bFound ) 
	{
		if ( Flags == NVPTYPE_STRING ) sprintf( ret, "%s = %s\n", Name, sValue );
		else if ( Flags == NVPTYPE_INTEGER ) sprintf( ret, "%s = %i\n", Name, iValue );
		else if ( Flags == NVPTYPE_FLOAT ) sprintf( ret, "%s = %f\n", Name, fValue );
		else ERRORMSG( ERRINVALIDPARAMETER, "NAMEVALUEPAIR::SaveValue()", "Configuration pair has invalid type flags." ); 

		str = ret;
		fileLines.push_back( str );
	}

	
	// write all lines back to the file
	f = fopen( fileName, "wt" );
	if ( !f ) ERRORMSG( ERRCANNOTOPENFORWRITE, "NAMEVALUEPAIR::SetValue()", fileName );

	for ( STRING_ITER i = fileLines.begin(); i != fileLines.end(); i++ )
	{
		fprintf( f, "%s", *i );
	}
	
	fclose( f );

	
	return ERRNOERROR;	
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function for SaveValue operation
// this fills the given lines list by every pPair obtained and if the variable name
// and value are the same as of the one that is looked for, it's value is replaced
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK LoadCallBackForSaveValue2( NAMEVALUEPAIR * pPair )
{
	CString					name, newName, str, val, oldVal;
	NAMEVALUEPAIR::POINTERS	*pointers;
	char					ret[1000];
	
	pointers = (NAMEVALUEPAIR::POINTERS *) pPair->ReservedPointer;

	name = pPair->GetName();
	newName = pointers->pPair->GetName();
	val = pPair->GetString();
	oldVal = pointers->csValue;
	
	newName.MakeLower();
	name.MakeLower();

    // the same variable of the same value already exists in the file
	if ( name == newName && val == oldVal )
	{
		pointers->bFound = TRUE;
		if ( pPair->GetFlags() == NVPTYPE_STRING )
			sprintf( ret, "%s = %s\n", pointers->pPair->GetName(), pointers->pPair->GetString() );
		else if ( pPair->GetFlags() == NVPTYPE_INTEGER )
			sprintf( ret, "%s = %i\n", pointers->pPair->GetName(), pointers->pPair->GetInt() );
		else if ( pPair->GetFlags() == NVPTYPE_FLOAT )
			sprintf( ret, "%s = %f\n", pointers->pPair->GetName(), pointers->pPair->GetFloat() );
		else // pair has improperly set its flags
		{
			pPair->ThrowError();
			return;
		}
				
		str = ret;
		pointers->pFileLines->push_back( str ); // the old variable is replaced by the new one
	}
	else // this is a different variable; just write it to the file as it was read
	{
		str = pPair->GetOriginalString();
		pointers->pFileLines->push_back( str );
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this method opens a specified configuration file and changes specified variable's value
// if the specified pair doesn't exist in the file this method doesn't change anything
// if a variable with the same name and value already exists in the file all it's occurances
// are changed to have the new value
//
// the return value should be zero; if any error occurs a nonzero error code is returned
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	NAMEVALUEPAIR::SaveValue( CAtlString fileName, CAtlString oldValue )
{
	NAMEVALUEPAIR			pair;
	STRING_LIST				fileLines;
	HRESULT					hr;
	POINTERS				pointers;
	CAtlString				str;
	char					ret[1000];
	FILE				*	f;


	pointers.csValue = oldValue;
	pointers.pPair = this;
	pointers.pFileLines = &fileLines;

	
	// load the file
	hr = Load( fileName, LoadCallBackForSaveValue2, &pointers );
	if ( hr ) ERRORMSG( ERRGENERIC, "NAMEVALUEPAIR::SaveValue()", "Configuration pair could not be saved." );

	
	// if the variable was not found in the file add a new line at the end
	if ( !pointers.bFound ) 
	{
		if ( Flags == NVPTYPE_STRING ) sprintf( ret, "%s = %s\n", Name, sValue );
		else if ( Flags == NVPTYPE_INTEGER ) sprintf( ret, "%s = %i\n", Name, iValue );
		else if ( Flags == NVPTYPE_FLOAT ) sprintf( ret, "%s = %f\n", Name, fValue );
		else ERRORMSG( ERRINVALIDPARAMETER, "NAMEVALUEPAIR::SaveValue()", "Configuration pair has invalid type flags." ); 

		str = ret;
		fileLines.push_back( str );
	}

	
	// write all lines back to the file
	f = fopen( fileName, "wt" );
	if ( !f ) ERRORMSG( ERRCANNOTOPENFORWRITE, "NAMEVALUEPAIR::SetValue()", fileName );

	for ( STRING_ITER i = fileLines.begin(); i != fileLines.end(); i++ )
	{
		fprintf( f, "%s", *i );
	}
	
	fclose( f );

	
	return ERRNOERROR;	
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
