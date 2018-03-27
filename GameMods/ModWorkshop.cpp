#include "stdafx.h"
#include "ModWorkshop.h"



//////////////////////////////////////////////////////////////////////////////////////////////
//
// Simple destructor unloads all previously loaded libraries (MODs).
//
//////////////////////////////////////////////////////////////////////////////////////////////
CMODWorkshop::~CMODWorkshop()
{
	for ( int i = 0; i < (int) Mods.size(); i++ )
		FreeLibrary( Mods[i].hModule );

	Mods.clear();
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// GetModsList method performs a search for all available game modifications in specified path.
//   Each game MOD is a DLL, so this method loads single function from each DLL which is then
//   used to obtain pointer to MOD's implementation of CGameMod class.
// On each object a version compatibility test is performed and valid pointers are stored for
//   later use.
// The pInitStruc structure is used to initialize each MOD. If this parameter is set to NULL,
//	 MODs are not being initialized. This is useful in case you only want to enumerat them, but
//	 are not planning to work with them deeply further (editor for example).
//
// returns zero on success
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CMODWorkshop::GetModsList( IN LPCTSTR sPath, CGameMod::INITSTRUC * pInitStruc )
{
	_finddata_t			FData;
	intptr_t			FInfoHandle;
	CString				csFilter;
	CString				csPath = sPath;


	// debug info
	OUTS( "Obtaining list of available game modules... (loader interface v. ", 1 );
	OUTI( HIWORD(GAMEMOD_INTERFACE_VERSION), 1 );
	OUTS( ".", 1 );
	OUTI( LOWORD(GAMEMOD_INTERFACE_VERSION), 1 );
	OUTSN( ").", 1 );

	// find all .dll files, load each one; ignore return values since we doesn't care
	//   if some of them get wrong
	csPath += "\\";
	csFilter = csPath + "*.dll";

	if ( (FInfoHandle = _findfirst( csFilter, &FData )) != -1 )
	{
		if ( !(FData.attrib & _A_SUBDIR) ) LoadModule( csPath + FData.name, pInitStruc );

		while ( !_findnext( FInfoHandle, &FData ) )
		{
			if ( !(FData.attrib & _A_SUBDIR) ) LoadModule( csPath + FData.name, pInitStruc );
		}
	}
	_findclose( FInfoHandle );	
	

	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// LoadModule method loads specified DLL into memory and performs the version compatibility
//   check. If everything goes ok, this method adds a new GAMEMODULESTRUC item into list of
//   MODs and returns zero. Otherwise this returns nonzero error code.
// The pInitStruc structure is used to initialize the MOD. If this parameter is set to NULL,
//   this MOD is not being initialized. This is useful in case you only want to enumerat it,
//   but are not planning to work with it deeply further (editor for example).
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CMODWorkshop::LoadModule( IN LPCTSTR csFileName, CGameMod::INITSTRUC * pInitStruc )
{
	GAMEMODULESTRUC		struc;
	LPGAMEMODOBJPTR		hObjPtr;
	LPGAMEMODVERSION	hVersion;
	CString				csMODName;
	DWORD				dwVersion;
	HRESULT				hr;


	OUTS( "Loading module '", 1 );
	OUTS( csFileName, 1 );
	OUTS( "'... ", 1 );

	
	// load the DLL
	struc.hModule = LoadLibrary( csFileName );
	
	if ( !struc.hModule )
	{  // unable to load library
		ErrorHandler.HandleError( ERRGENERIC, "CMODWorkshop::LoadModule()", "Unable to load specified library." );
		OUTSN( "FAILED!", 1 );
		return ERRGENERIC;
	}



	// library loaded, now we must obtain a pointer to appropriate CGameMod object
	hVersion = (LPGAMEMODVERSION) GetProcAddress( struc.hModule, "GetInterfaceVersion" );
	hObjPtr = (LPGAMEMODOBJPTR) GetProcAddress( struc.hModule, "GetModulePointer" );
	
	if ( !hObjPtr || !hVersion )
	{ // unable to load mandatory functions, the library is not valid game MOD
		ErrorHandler.HandleError( ERRGENERIC, "CMODWorkshop::LoadModule()", "Specified library is not a valid game MOD." );
		OUTSN( "FAILED!", 1 );
		FreeLibrary( struc.hModule );
		return ERRGENERIC;
	}



	// check the interface version used by the module
	dwVersion = hVersion();

	if ( dwVersion != GAMEMOD_INTERFACE_VERSION )
	{ // module is of a wrong version (uses different version of loader interface than game does)
		ErrorHandler.HandleError( ERRGENERIC, "CMODWorkshop::LoadModule()", "Specified module is of a wrong version. See log file for details." );
		OUTSN( "FAILED!", 1 );
		OUTS( "Loader interface used by game module: ", 1 );
		OUTI( HIWORD(dwVersion), 1 );
		OUTS( ".", 1 );
		OUTIN( LOWORD(dwVersion), 1 );
		FreeLibrary( struc.hModule );
		return ERRGENERIC;
	}



	// obtain a pointer to the CGameMod object
	struc.pModule = hObjPtr();

	if ( !struc.pModule )
	{ // module initialization didn't end correctly .. or whatever; problem is inside of the module though
		ErrorHandler.HandleError( ERRGENERIC, "CMODWorkshop::LoadModule()", "Specified module is not responding correctly." );
		OUTSN( "FAILED!", 1 );
		FreeLibrary( struc.hModule );
		return ERRGENERIC;
	}



	// try to init the module if initialization required
	if ( pInitStruc )
	{
		hr = struc.pModule->Init( pInitStruc );
		if ( hr )
		{ // module initialization didn't end correctly
			ErrorHandler.HandleError( ERRINVALIDPARAMETER, "CMODWorkshop::LoadModule()", "Specified module initialization failed." );
			OUTSN( "FAILED!", 1 );
			FreeLibrary( struc.hModule );
			return ERRINVALIDPARAMETER;
		}
	}


	// everything went ok, lets add the module to the list
	Mods.push_back( struc );
	

	// debug info
	csMODName = struc.pModule->GetModuleName();
	OUTS( "OK. Module '", 1 );
	OUTS( csMODName, 1 );
	OUTSN( "' loaded.", 1 );


	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// Overloaded array access operator - used to access MOD pointers directly. So this[i] returns
//   i-th pointer to a CGameMod object.
// If there is not such an object (invalid index), this method returns zero and thus it should
//   be used wisely.
//
//////////////////////////////////////////////////////////////////////////////////////////////
LPGAMEMOD CMODWorkshop::operator[]( IN int index ) const
{
	// check for subscript out-of-range error
	if ( index >= (int) Mods.size() || index < 0 )
	{
		__ERRORMSG( ERRINVALIDPARAMETER, "CMODWorkshop::operator[]()", "Index out of range.", NULL );
		//return NULL;
	}

	return Mods[index].pModule;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// FindMOD method looks for specified game MOD and returns pointer to it if it exists (which 
//  means that it is listed by this MOD Workshop object). Particular MOD is not specified by 
//  it's name, rather by it's ID.
// If the specified MOD doesn't exist, this method returns NULL pointer.
//
//////////////////////////////////////////////////////////////////////////////////////////////
LPGAMEMOD CMODWorkshop::FindMOD( IN LPCTSTR csID ) const
{
	for ( UINT i = 0; i < Mods.size(); i++ )
		if ( Mods[i].pModule->GetModulePrimaryID() == csID ) return Mods[i].pModule;

	return NULL;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// FindMODIndex method looks for specified game MOD and returns its index in the list. 
// If the requested MOD doesn't exist, this returns a negative value.
//
//////////////////////////////////////////////////////////////////////////////////////////////
int CMODWorkshop::FindMODIndex( IN LPCTSTR csID ) const
{
	for ( int i = 0; i < (int)Mods.size(); i++ )
		if ( Mods[i].pModule->GetModulePrimaryID() == csID ) return i;

	return -1;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
