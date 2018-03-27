/*
Game information class
provides information about available cars, maps and music for the game
by Pavel Celba
created:		28.11.2005
last update:	6.1.2007
*/


#pragma once

#include "stdafx.h"
#include "..\globals\Globals.h"
#include "..\globals\Configuration.h"
#include "..\globals\NameValuePair.h"
#include "..\ResourceManager\ResourceManager.h"


#include <vector>

using namespace std;

class CGameInfo
{
public:
	// Init
	// Finds all cars and maps even mutators
	HRESULT Init( CConfig * Config, resManNS::CResourceManager * pResMan );

	// Provides structure to map names to filenames
	struct NameToFileNameMapping
	{
		CString		Name;
		CString		FileName;
	};

	struct MapShortInfo
	{
		CString		Name;
		CString		FileName;
		std::vector<CString>	ModsList;
		CString		ImageFileName;
	};

	

	// Contain car names mapped to their definition filenames
	std::vector<NameToFileNameMapping> CarNames;
	// Contain map names mapped to their definition filenames
	std::vector<MapShortInfo> MapNames;
	// Contain music filenames
	std::vector<CString> MusicNames;

	// BEST RESULTS
	struct CLapEntry
	{
		UINT NumOfLaps;
		CString PlayerName;
		UINT BestRaceTime;
	};

	struct CMapEntry
	{
		CString MapFileName;
		CString PlayerName;
		UINT BestLapTime;
		std::vector<CLapEntry> LapEntries;
	};

	// Best results info
	std::vector<CMapEntry> BestResults;

	// Loads best results for maps
	HRESULT LoadBestResults();
	// Saves best results for maps
	HRESULT SaveBestResults();

private:
	resManNS::CResourceManager	* pResourceManager;
	// This is set to true by car name load callback function, when name is found
	BOOL			FoundName;
	// To remember car name
	CString			CarName;

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
	static bool CALLBACK Compare2(MapShortInfo & Mapping1, MapShortInfo & Mapping2);

	CConfig * Configuration; // Configuration class

	
};