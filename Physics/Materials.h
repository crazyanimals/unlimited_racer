//////////////////////////////////////////////////////////////////////////////////////////////
//
// CMaterials
// TODO: - complete this ...
//	
//////////////////////////////////////////////////////////////////////////////////////////////
// materialy cislovane od 1, 0 je defaultni, -1 je pouziti defaultniho parametru
// materialu je 1 - 255, 0 se pouziva pro nic
// v resourcemanageru - dodelat chybova hlasky

#pragma once

#include "stdafx.h"

#include "UserData.h"
#include "PhysicsMacros.h"
#include "Resources.h"

#include <map>
#include <vector>

namespace physics
{
	//class CMaterials;
	class CPhysics;
	class CMaterials;

	typedef struct sEffectStruct
	{
		const NewtonBody * bodyA;
		const NewtonBody * bodyB;
		dVector m_position;
		resManNS::TMaterialPairInfo *materialPair;
		dFloat normalSpeed, tangentSpeed;
	} TEffect;

	enum eMatNames { 
		MAT_DEFAULT, 
		MAT_WOOD, 
		MAT_METAL, 
		MAT_GRASS,
		MAT_CAR,
		MAT_TIRE, 
		MAT_CONCRETE, 
		MAT_GHOST,
		MAT_WORLDBOX
	};
	
#define CHANGEBLEMATCOEFPAIRCOUNT 4	// car<->wood, car<->metal, car<->grass, car<->asphalt

	//////////////////////////
	// structure of mutator
	//////////////////////////
	typedef struct sMutator {
		float engineEfficiecy;	// Vykonost motoru. Cislo od 0.5 (hodne malo vykonne motory), po 3
		float breaksEfficiecy;	// Vykonost brzd. Cislo od 0.1 {skoro nebrzdi), po 10 (instant Handbrake:).
		float gravity;
		// coeficients for interaction between pairs of material
		resManNS::MaterialsCoef materialCoefs[CHANGEBLEMATCOEFPAIRCOUNT];
	} TMutator;

	// map of all mutators
	typedef std::map<CString, TMutator *> MutatorMap;

	// map of maps of materials. Map is used because of quick searching according <int,int>pair
	typedef std::map<int, resManNS::TMaterialPairInfo *> MaterialPairMapInner;
	typedef std::map<int, MaterialPairMapInner *> MaterialPairMap;

	//////////////////////////////////////////////////////////

	class CMaterials
	{

	private:
		NewtonWorld * nWorld;
		CResources * resources;
		static CPhysics * physics;

		// map of existing mutators
		MutatorMap mutatorMap;
		TMutator *m_currentlyUsedMutator;

		static MaterialPairMap matMap;

		// pointer to those material pair info in resManager that are changable
		resManNS::TMaterialPairInfo *materialPairSlots[CHANGEBLEMATCOEFPAIRCOUNT];
		
		static TEffect currentEffect;

		BYTE * materialXmaterial;
		int materialXmaterialSize;

		resManNS::Materials * materials;
		UINT *materialIDs;

	// CMaterial methods
	private:		

		// reads line from file and removes comments, which are behind ';' character
		int GetLine (char *line, FILE *file);

		static resManNS::TMaterialPairInfo *GetMatPair (int mat1, int mat2);
		static void InsertMatPair (int mat1, int mat2, resManNS::TMaterialPairInfo *matPair);

		void SetCallback (resManNS::TMaterialPairInfo *matPair);

		void SetBehaviour (UINT mat1, UINT mat2, resManNS::MaterialsCoef & coefs);

		// function that loads all the mutators
		void LoadMaterialPairColisionCoefs (resManNS::MaterialsCoef *coefs,									   
										   char *line, TMutator *defMut, UINT n);
		static void SaveContactSpeeds (const NewtonMaterial * material, const NewtonContact * contact);

		static int DefaultContactBegin(const NewtonMaterial * material, const NewtonBody * body0, const NewtonBody * body1);
		static void DefaultContactEnd (const NewtonMaterial* material);

		static int DefaultContactProcess (const NewtonMaterial * material, const NewtonContact* contact);
		static int SurfaceContactProcess (const NewtonMaterial * material, const NewtonContact* contact);

		static int Car_Terrain_ContactProcess(const NewtonMaterial * material, const NewtonContact* contact);

		static void ApplyMatCoefsNow (const NewtonMaterial * material, resManNS::MaterialsCoef * coefs);

	public:

		CMaterials( );	// default constructor
		~CMaterials( ); // destructor

		inline UINT GetMaterialID (UINT matNum) { return materialIDs[matNum]; }

		// CALL from dialog, Roman!!
		void SelectMutator (CString mutatorName);
		void GetMutatorsNamesList (std::vector<CString> & vectorWhereNamesToBeStored);

		// applies current selected mutator
		void ApplyCurrentMutator (void);
		
		// loads all the mutators 
		void LoadMutators (std::vector<CString> & fileNames, CString path);

		inline TMutator *GetCurrentMutator (void) { return m_currentlyUsedMutator; }

		HRESULT Init(NewtonWorld * world, CResources * resources, CPhysics * physics);
	};
}; // namespace physics