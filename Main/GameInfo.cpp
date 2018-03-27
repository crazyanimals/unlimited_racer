/*
GameInfo class implementation
by Pavel Celba
created: 28. 11. 2005
*/

#include "GameInfo.h"

using namespace std;

// Init
// Finds all cars and maps 
HRESULT CGameInfo::Init(CConfig * Config)
{
	HRESULT hr;

	vector<CString> CarFileNames;
	vector<CString> MapFileNames;

	NameToFileNameMapping TempMapping;

	CString AllPath;
	CHAR CurrentDirectory[MAX_PATH];

	// Set configuration
	if (!Config) 
		ERRORMSG(-1, "CGameInfo::Init()", "Configuration pointer must not be NULL.");
	Configuration = Config;

	hr = GetCurrentDirectory(MAX_PATH, CurrentDirectory);
	if (!hr)
		ERRORMSG(hr, "CGameInfo::Init()", "Unable to get current directory");

	// Find all music files
	AllPath = CString(CurrentDirectory) + CString("\\") + Configuration->DataPath + Configuration->MusicPath;
	hr = EnumerateDirectoriesForFiles(AllPath, CString("*.mp3"), MusicNames);
	if (FAILED(hr) )
		ERRORMSG(hr, "CGameInfo::Init()", "Unable to find music file names");
	
	// Find all map and car files
	AllPath = CString(CurrentDirectory) + CString("\\") + Configuration->DataPath + Configuration->CarsPath;

	hr = EnumerateDirectoriesForFiles(AllPath, CString("*.car"), CarFileNames);
	if (FAILED(hr) ) 
		ERRORMSG(hr, "CGameInfo::Init()", "Unable to find car file names.");

	AllPath = CString(CurrentDirectory) + CString("\\") + Configuration->DataPath + Configuration->MapsPath;
	hr = EnumerateDirectoriesForFiles(AllPath, CString("*.map"), MapFileNames);
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
	FILE * File;
	char TempOne[255], TempTwo[255];

	MapNames.clear();
	for (It = MapFileNames.begin(); It != MapFileNames.end(); It++)
	{
		File = fopen(Configuration->DataPath + Configuration->MapsPath + (*It), "r");
		if (!File) continue;

		// name is on the second line
		hr = fscanf(File, "%250s %250s\n", &TempOne, &TempTwo);
		if (fgets(TempTwo, 100, File))
		{
			// Add to mapping
			TempMapping.FileName = (*It);
			TempMapping.Name = TempTwo; // Limit to name maximum is 100 characters
			
			MapNames.push_back(TempMapping);
		}

		fclose(File);
	}

	// sort names
	sort(CarNames.begin(), CarNames.end(), Compare);
	sort(MapNames.begin(), MapNames.end(), Compare);
	
	return 0;
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

// Binary predicate for sorting
bool CALLBACK CGameInfo::Compare(NameToFileNameMapping & Mapping1, NameToFileNameMapping & Mapping2)
{
	return (strcmp(Mapping1.Name, Mapping2.Name) < 0);
}


