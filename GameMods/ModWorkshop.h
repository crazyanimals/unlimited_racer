/*
	ModWorkshop:	CMODWorkshop class is working interface covering basic work with game modifications =
					CGameMod class objects. This object contains list of available MODs and provides 
					method for their enumeration. Also offers easy access to every stored MOD.
	Creation Date:	15.11.2006
	Last Update:	7.1.2007
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "../Globals/GlobalsIncludes.h"
#include "GameMod.h"



// GAMEMODULESTRUC structure contains description of a single loaded module
struct GAMEMODULESTRUC
{
	CGameMod	*	pModule;
	HMODULE			hModule;
};



/////////////////////////////////////////////////
// main class declaration
/////////////////////////////////////////////////
class CMODWorkshop
{
	// data types
	typedef std::vector<GAMEMODULESTRUC>	GAMEMOD_LIST;
	typedef GAMEMOD_LIST::iterator			GAMEMOD_ITER;

public:
						~CMODWorkshop();

	HRESULT				GetModsList( IN LPCTSTR csPath, CGameMod::INITSTRUC * pInitStruc );
	int					GetModCount() const { return (int) Mods.size(); };
	LPGAMEMOD			operator[]( IN int index ) const;
	LPGAMEMOD			FindMOD( IN LPCTSTR csID ) const;
	int					FindMODIndex( IN LPCTSTR csID ) const;
	
private:
	HRESULT				LoadModule( IN LPCTSTR csFileName, CGameMod::INITSTRUC * pInitStruc );
	
	GAMEMOD_LIST		Mods; // list of available mods

};
