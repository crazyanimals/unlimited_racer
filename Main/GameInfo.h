#pragma once

#include "..\globals\Globals.h"
#include "..\globals\Configuration.h"
#include "..\globals\NameValuePair.h"
#include <vector>
#include <list>
#include <algorithm>


/*
Game information class
provides information about available cars, maps and music for the game
by Pavel Celba
created: 28. 11. 2005
*/

class CGameInfo
{
public:
	// Init
	// Finds all cars and maps 
	HRESULT Init(CConfig * Config);

	// Provides structure to map names to filenames
	struct NameToFileNameMapping
	{
		CString Name;
		CString FileName;
	};
	
	// Contain car names mapped to their definition filenames
	std::vector<NameToFileNameMapping> CarNames;
	// Contain map names mapped to their definition filenames
	std::vector<NameToFileNameMapping> MapNames;
	// Contain music filenames
	std::vector<CString> MusicNames;

	
private:
	// This is set to true by car name load callback function, when name is found
	BOOL FoundName;
	// To remember car name
	CString CarName;

	// Static car callback for loading car name
	static void CALLBACK StaticGameInfoCarCallback(NAMEVALUEPAIR * Pair);

	// Car callback for loading car name
	void GameInfoCarCallback(NAMEVALUEPAIR * Pair);

	// Finds all files in specified directory and its subdirectories, which satisfy FileMask, and puts them into FileNames list
	HRESULT EnumerateDirectoriesForFiles(CString & Path, CString & FileMask, std::vector<CString> & FileNames);

	// Finds all files in specified directory, which satisfy FileMask, and puts them into FileNames list
	HRESULT EnumerateDirectoryForFiles(CString & Path, CString & FileMask, std::vector<CString> & FileNames);

	// Builds subdirectory list - recursive
	// without common . and .. directories
	HRESULT BuildSubdirectoryList(CString & Path, CString Subdirectory, std::vector<CString> & Subdirectories);

	// Binary predicate for sorting
	static bool CALLBACK Compare(NameToFileNameMapping & Mapping1, NameToFileNameMapping & Mapping2);

	CConfig * Configuration; // Configuration class
};