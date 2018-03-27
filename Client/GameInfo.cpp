/*
GameInfo class implementation
by Pavel Celba
created: 7.1.2007
*/

#include "GameInfo.h"
#include "../main/Globals.h"

// Init
// Finds all cars and maps 
HRESULT CGameInfo::Init( CConfig * Config, resManNS::CResourceManager * pResMan )
{
	HRESULT			hr;
	resManNS::RESOURCEID	rid;
	vector<CString> CarFileNames;
	vector<CString> MapFileNames;
	NameToFileNameMapping	TempMapping;
	MapShortInfo			MapTempMapping;
	CString			AllPath;
	CHAR			CurrentDirectory[MAX_PATH];

	pResourceManager = pResMan;
	if ( !pResMan ) ERRORMSG( ERRINVALIDPARAMETER, "CGameInfo::Init()", "Invalid ResourceManager pointer." );

	// Set configuration
	if (!Config) 
		ERRORMSG(-1, "CGameInfo::Init()", "Configuration pointer must not be NULL.");
	Configuration = Config;

	hr = GetCurrentDirectory(MAX_PATH, CurrentDirectory);
	if (!hr)
		ERRORMSG(hr, "CGameInfo::Init()", "Unable to get current directory");

	// Find all music files
	AllPath = Configuration->NoRelativePaths ? Configuration->MusicPath : Configuration->DataPath + Configuration->MusicPath;
	hr = EnumerateDirectoriesForFiles(AllPath, CString("*.mp3"), MusicNames);
	if (FAILED(hr) )
		ERRORMSG(hr, "CGameInfo::Init()", "Unable to find music file names");
	

	// Find all map and car files
	AllPath = Configuration->NoRelativePaths ? Configuration->ObjectsPath : Configuration->DataPath + Configuration->ObjectsPath;

	hr = EnumerateDirectoriesForFiles(AllPath, CString("*.car"), CarFileNames);
	if (FAILED(hr) ) 
		ERRORMSG(hr, "CGameInfo::Init()", "Unable to find car file names.");

	AllPath = Configuration->NoRelativePaths ? Configuration->MapsPath : Configuration->DataPath + Configuration->MapsPath;
	hr = EnumerateDirectoriesForFiles(AllPath, CString("*.urmap"), MapFileNames);
	if (FAILED(hr) )
		ERRORMSG(hr, "CGameInfo::Init()", "Unable to find map file names.");
	

	// find car name and create mapping for car name and car filename
	vector<CString>::iterator It;

	CarNames.clear();
	for (It = CarFileNames.begin(); It != CarFileNames.end(); It++)
	{
		FoundName = FALSE;
		NAMEVALUEPAIR::Load(Configuration->DataPath + Configuration->CarsPath + (*It), StaticGameInfoCarCallback, this);
		if (FoundName)
		{
			// Add to mapping
			TempMapping.FileName = (*It);
			TempMapping.Name = CarName;
			// Limit name to maximum of 100 characters
			if (TempMapping.Name.GetLength() > 100) TempMapping.Name = TempMapping.Name.Left(100);
			CarNames.push_back(TempMapping);
		}
	}


	// find map name and create mapping for map name and map filename
	MapNames.clear();
	for (It = MapFileNames.begin(); It != MapFileNames.end(); It++)
	{
		// load the map file as a resource
		resManNS::Map	*	MapStruc;
		rid = pResourceManager->LoadResource( *It, RES_Map, false, NULL );
		MapStruc = pResourceManager->GetMap( rid );
		if ( !MapStruc ) continue;

		// Add to mapping
		MapTempMapping.FileName = *It;
		MapTempMapping.Name = MapStruc->sMapName->Left( MAX_MAP_NAME_LENGTH );
		MapTempMapping.ImageFileName = *(MapStruc->sMapImageFile);
		MapTempMapping.ModsList = *(MapStruc->pvSupportedModules);

		MapNames.push_back( MapTempMapping );

		pResourceManager->ReleaseResource( rid );
	}


	// sort names
	sort(CarNames.begin(), CarNames.end(), Compare);
	sort(MapNames.begin(), MapNames.end(), Compare2);

	///////////////////////////////
	// solve MUTATORS
	///////////////////////////////
	vector<CString> mutatorFileNames;

	// Find all mutators files
	AllPath = Configuration->NoRelativePaths ? Configuration->MutatorsPath : Configuration->DataPath + Configuration->MutatorsPath;
	hr = EnumerateDirectoriesForFiles(AllPath, CString("*.mut"), mutatorFileNames);
	if (FAILED(hr) )
		ERRORMSG(hr, "CGameInfo::Init()", "Unable to list mutator files");

	// load them
	Physics->materials.LoadMutators (mutatorFileNames, AllPath);

	//////////////////////////////////////////////////

	// Load best results
	hr = LoadBestResults();
	if (FAILED(hr) )
		ERRORMSG(hr, "CGameInfo::Init()", "Unable to load best results.");
	

	return ERRNOERROR;
}

// Static car callback for loading car name
void CALLBACK CGameInfo::StaticGameInfoCarCallback(NAMEVALUEPAIR * Pair)
{
	CGameInfo * GameInfo;
	GameInfo = (CGameInfo*) Pair->ReservedPointer;
	GameInfo->GameInfoCarCallback(Pair);
}

// Car callback for loading car name
void CGameInfo::GameInfoCarCallback(NAMEVALUEPAIR * Pair)
{
	CString Name;
	Name = Pair->GetName();

	Name.MakeUpper();

	if (Name == _T("NAME") ) 
	{
		// Remember found car name
		CarName = Pair->GetString();
		FoundName = TRUE;
	}
}

// Finds all files in specified directory and its subdirectories, which satisfy FileMask, and puts them into FileNames list
HRESULT CGameInfo::EnumerateDirectoriesForFiles(CString & Path, CString & FileMask, vector<CString> & FileNames)
{
	// Subdirectory list
	vector<CString> Subdirectories;

	// One directory found filenames
	vector<CString> DirectoryFileNames;
	
	CString FindPath;

	vector<CString>::iterator It, It2;

	BuildSubdirectoryList(Path, CString(""), Subdirectories);

	FindPath = Path;

	// Find all files in root directory
	EnumerateDirectoryForFiles(FindPath, FileMask, DirectoryFileNames);

	// Copy them all
	FileNames = DirectoryFileNames;
	

	for (It = Subdirectories.begin(); It != Subdirectories.end(); It++)
	{
		DirectoryFileNames.clear();
		FindPath = Path + (*It);
		EnumerateDirectoryForFiles(FindPath, FileMask, DirectoryFileNames);
		
		for (It2 = DirectoryFileNames.begin(); It2 != DirectoryFileNames.end(); It2++)
			FileNames.push_back( (*It) + (*It2) );
	
	}

	return 0;
}

// Finds all files in specified directory, which satisfy FileMask, and puts them into FileNames list
HRESULT CGameInfo::EnumerateDirectoryForFiles(CString & Path, CString & FileMask, vector<CString> & FileNames)
{
	WIN32_FIND_DATA FileData;
	HANDLE hFind;
	BOOL Ret;

	hFind = FindFirstFile(Path + FileMask, &FileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	while (true)
	{
		FileNames.push_back(CString(FileData.cFileName));

		Ret = FindNextFile(hFind, &FileData);
		if (!Ret) break;
	}

	FindClose(hFind);

	return 0;
}



// Builds subdirectory list - recursive
// without common . and .. directories
HRESULT CGameInfo::BuildSubdirectoryList(CString & Path, CString Subdirectory, vector<CString> & Subdirectories)
{
	WIN32_FIND_DATA FileData;
	HANDLE hFind;
	BOOL Ret;

	hFind = FindFirstFileEx(Path + Subdirectory + CString("*"), FindExInfoStandard, &FileData, FindExSearchLimitToDirectories, NULL, 0);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;


	while( true)
	{
		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //Just for sure
		{
			if (strcmp(FileData.cFileName, ".") && strcmp(FileData.cFileName, "..") )
			{
				Subdirectories.push_back(Subdirectory + CString(FileData.cFileName) + CString("\\"));
				BuildSubdirectoryList(Path, Subdirectory + CString(FileData.cFileName) + CString("\\"), Subdirectories);
			}
		}

		Ret = FindNextFile(hFind, &FileData);

		if (!Ret) break;
		
	}

	FindClose(hFind);

	return 0;
}

// Binary predicate for sorting car names
bool CALLBACK CGameInfo::Compare(NameToFileNameMapping & Mapping1, NameToFileNameMapping & Mapping2)
{
	return (strcmp(Mapping1.Name, Mapping2.Name) < 0);
}

// Binary predicate for sorting map names
bool CALLBACK CGameInfo::Compare2(MapShortInfo & Mapping1, MapShortInfo & Mapping2)
{
	return (strcmp(Mapping1.Name, Mapping2.Name) < 0);
}

// Load best results for maps
HRESULT CGameInfo::LoadBestResults()
{
	FILE * File;
	UINT NumMapEntries, NumLapEntries;
	UINT i, e;
	HRESULT hr;
	char FileName[256];
	char PlayerName[256];
	UINT BestRaceTime;
	UINT BestLapTime;
	UINT NumOfLaps;
	CMapEntry MapEntry;
	CLapEntry LapEntry;

	// Clear best results
	BestResults.clear();

	// Open best results file
	File = fopen("BestResults.dat", "r");
	if (!File) return ERRNOERROR;

	// Load number of map entries
	hr = fscanf(File, "%u\n", &NumMapEntries);
	if (!hr)
	{
		fclose(File);
		ERRORMSG(hr, "CGameInfo::LoadBestResults()", "Best results file is corrupted. Unable to load number of map entries.");
	}

	// Load map entries consisting of:
	// 1) map filename (relative to map path)
	// 2) Player name
	// 3) Best lap time in hundreths of second
	// 4) Number of races on laps
	// 5) Entries consisting of:
	// 5) a) Number of laps
	// 5) b) Player name
	// 5) c) Best race time
	for (i = 0; i < NumMapEntries; i++)
	{
		// map file name
		if (! fgets(FileName, 255, File) )
		{
			fclose(File);
			ERRORMSG(-1, "CGameInfo::LoadBestResults()", "Best results file is corrupted. Unable to load map filename.");
		}

		// Player name
		if (! fgets(PlayerName, 255, File) )
		{
			fclose(File);
			ERRORMSG(-1, "CGameInfo::LoadBestResults()", "Best results file is corrupted. Unable to load map filename.");
		}

		// best lap time
		hr = fscanf(File, "%u\n", &BestLapTime);
		if (!hr)
		{
			fclose(File);
			ERRORMSG(hr, "CGameInfo::LoadBestResults()", "Best results file is corrupted. Unable to load best lap time.");
		}

		// Number of lap entries
		hr = fscanf(File, "%u\n", &NumLapEntries);
		if (!hr)
		{
			fclose(File);
			ERRORMSG(hr, "CGameInfo::LoadBestResults()", "Best results file is corrupted. Unable to load number of lap entries.");
		}

		MapEntry.MapFileName = CString(FileName);
		MapEntry.PlayerName = CString(PlayerName);
		MapEntry.BestLapTime = BestLapTime;

		MapEntry.LapEntries.clear();

		for (e = 0; e < NumLapEntries; e++)
		{
			// Number of laps, Player name, Best race time
			hr = fscanf(File, "%u\n", &NumOfLaps);
			if (!hr)
			{
				fclose(File);
				ERRORMSG(hr, "CGameInfo::LoadBestResults()", "Best results file is corrupted. Unable to load number of laps.");
			}

			if (! fgets(PlayerName, 255, File) )
			{
				fclose(File);
				ERRORMSG(-1, "CGameInfo::LoadBestResults()", "Best results file is corrupted. Unable to load player name.");
			}

			hr = fscanf(File, "%u\n", &BestRaceTime);
			if (!hr)
			{
				fclose(File);
				ERRORMSG(hr, "CGameInfo::LoadBestResults()", "Best results file is corrupted. Unable to load best race time.");
			}

			LapEntry.NumOfLaps = NumOfLaps;
			LapEntry.PlayerName = CString(PlayerName);
			LapEntry.BestRaceTime = BestRaceTime;

			MapEntry.LapEntries.push_back(LapEntry);
		}

		BestResults.push_back(MapEntry);
	}

	fclose(File);

	return ERRNOERROR;
}


// Saves best results for maps
HRESULT CGameInfo::SaveBestResults()
{
	HRESULT hr;
	FILE * File;
	UINT i, e;

	if (BestResults.size() == 0) return ERRNOERROR;

	File = fopen("BestResults.dat", "w");
	if (!File)
		ERRORMSG(-1, "CGameInfo::SaveBestResults()", "Unable to open BestResults.dat for writing.");

	// write number of map entries
	hr = fprintf(File, "%u\n", (UINT) BestResults.size());
	if (!hr)
	{
		fclose(File);
		ERRORMSG(hr, "CGameInfo::SaveBestResults()", "Unable to write to BestResults file.");
	}

	// write down map entries
	for (i = 0; i < (UINT) BestResults.size(); i++)
	{
		hr = fprintf(File, "%s\n", BestResults[i].MapFileName);
		if (!hr)
		{
			fclose(File);
			ERRORMSG(hr, "CGameInfo::SaveBestResults()", "Unable to write to BestResults file.");
		}

		hr = fprintf(File, "%s\n", BestResults[i].PlayerName);
		if (!hr)
		{
			fclose(File);
			ERRORMSG(hr, "CGameInfo::SaveBestResults()", "Unable to write to BestResults.file.");
		}

		hr = fprintf(File, "%u\n", BestResults[i].BestLapTime);
		if (!hr)
		{
			fclose(File);
			ERRORMSG(hr, "CGameInfo::SaveBestResults()", "Unable to write to BestResults.file.");
		}

		hr = fprintf(File, "%u\n", BestResults[i].LapEntries.size());
		if (!hr)
		{
			fclose(File);
			ERRORMSG(hr, "CGameInfo::SaveBestResults()", "Unable to write to BestResults.file.");
		}

		for (e = 0; e < (UINT) BestResults[i].LapEntries.size(); e++)
		{
			hr = fprintf(File, "%u\n", BestResults[i].LapEntries[e].NumOfLaps);
			if (!hr)
			{
				fclose(File);
				ERRORMSG(hr, "CGameInfo::SaveBestResults()", "Unable to write to BestResults.file.");
			}
			hr = fprintf(File, "%s\n", BestResults[i].LapEntries[e].PlayerName);
			if (!hr)
			{	
				fclose(File);
				ERRORMSG(hr, "CGameInfo::SaveBestResults()", "Unable to write to BestResults.file.");
			}
			hr = fprintf(File, "%u\n", BestResults[i].LapEntries[e].BestRaceTime);
			if (!hr)
			{	
				fclose(File);
				ERRORMSG(hr, "CGameInfo::SaveBestResults()", "Unable to write to BestResults.file.");
			}
		}
	}

	fclose(File);

	return ERRNOERROR;
}

