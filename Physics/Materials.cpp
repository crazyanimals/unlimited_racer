#include "Materials.h"
#include "Physics.h"

using namespace physics;

TEffect CMaterials::currentEffect;
MaterialPairMap CMaterials::matMap;
CPhysics *CMaterials::physics;

// DEFAULT contructor
CMaterials::CMaterials( )
{
	/*
	// default behavior initialization
	// colision
	m_defaultColisionCoefs.materialA = 0;	// default
	m_defaultColisionCoefs.materialB = 0;
	m_defaultColisionCoefs.softness = 1;
	m_defaultColisionCoefs.elasticity = 1;
	m_defaultColisionCoefs.staticFriction = 1;
	m_defaultColisionCoefs.kineticFriction = 1;
	m_defaultColisionCoefs.collidable = 1;

	// gravity
	m_defaultGravity = 10.0f;
	m_defaultBreaksEfficiency = 1.0;
	m_defaultEngineEfficiency = 1.0;
	*/
}

CMaterials::~CMaterials( )
{
}

HRESULT CMaterials::Init(NewtonWorld * world, CResources * resources, CPhysics * physics)
{	
	nWorld = world;
	this->resources = resources;
	this->physics = physics;

	UINT i;

	// nacteni materialu z materials souboru
	resManNS::RESOURCEID resMaterialsID = resources->rsm->LoadResource("DATA\\PHYSICS\\MATERIALS", RES_Materials);
	if(resMaterialsID < 1)
		ERRORMSG(ERRGENERIC, "CMaterials::Init()", "Could not load _Materials object.");
	resources->AddResource (resMaterialsID);

	// get materials from resource manager
	materials = resources->rsm->GetMaterials (resMaterialsID);
	if (!materials) 
		ERRORMSG(ERRNOTFOUND, "CMaterials::Init()", "Could not obtain _Materials object.");

	// alloc array for material IDs
	SAFE_NEW_ARRAY (materialIDs, UINT, materials->uiMaterialIDsCount, "Materials::Init()");

	// create newton materials
	// default as first
	materialIDs[0] = NewtonMaterialGetDefaultGroupID (nWorld);
	// then the others
	for (i=1; i < materials->uiMaterialIDsCount; i++)
		materialIDs[i] = NewtonMaterialCreateGroupID (nWorld);

	// now we have all build-in materials and need to specify the behavior
	// when any two of them collide. Not all pairs need to be specified
	// in case e pair is missing, the default is used
	
	// special behavior is defined through mutators. Through it we can define behaviour between 
	// folowing material pairs: car<->wood, car<->metal, car<->grass, car<->asphalt.
	// so now we create default mutator...
	TMutator *defMut = new TMutator();
	
	// now we go over all material pairs coefs. The first are defualt mutator changable items
	// copy them into default mutator
	i = 0;
	while (i < CHANGEBLEMATCOEFPAIRCOUNT)
	{
		// copy coefs into mutator
		defMut->materialCoefs[i] = materials->pMaterialPairs[i].coefs;

		// set the materialPairSlots
		materialPairSlots[i] = &(materials->pMaterialPairs[i]);

		// insert pair into matMap
		InsertMatPair (
			materials->pMaterialPairs[i].uiMaterialA,
			materials->pMaterialPairs[i].uiMaterialB,
			&(materials->pMaterialPairs[i])
			);

		//and set callback
		SetCallback (&materials->pMaterialPairs[i]);

		i++;
	}

	// finish setting up the defualt mutator
	defMut->gravity = 10.0f;
	defMut->breaksEfficiecy = 1.0f;
	defMut->engineEfficiecy = 1.0f;

	// add it into mutator vector
	mutatorMap.insert (MutatorMap::value_type ("default", defMut));

	// the rest are unchangable ones, so we make behaviour for aprorpiate pair
	while (i < materials->uiMaterialPairsSize)
	{
		SetBehaviour (materials->pMaterialPairs[i].uiMaterialA,
			materials->pMaterialPairs[i].uiMaterialB,
			materials->pMaterialPairs[i].coefs);

		// insert pair into matMap
		InsertMatPair (
			materials->pMaterialPairs[i].uiMaterialA,
			materials->pMaterialPairs[i].uiMaterialB,
			&(materials->pMaterialPairs[i])
			);

		//and set callback
		SetCallback (&materials->pMaterialPairs[i]);
		
		i++;
	}

	return ERRNOERROR;
};

///////////////////////////////////////////////////////////
// if 2nd mat in pair is {MAT_GRASS,MAT_CONCRETE} set SurfaceBegin callback
// else se DefautlBeginCallback
///////////////////////////////////////////////////////////
void
CMaterials::SetCallback (resManNS::TMaterialPairInfo *matPair)
{
	switch (matPair->uiMaterialB)
	{
	case MAT_GRASS:
	case MAT_CONCRETE:
		NewtonMaterialSetCollisionCallback(nWorld, matPair->uiMaterialA,
			matPair->uiMaterialB, (void *)matPair,
			DefaultContactBegin, SurfaceContactProcess, DefaultContactEnd);

		NewtonMaterialSetContinuousCollisionMode (nWorld, matPair->uiMaterialA,
			matPair->uiMaterialB, 1);
		break;

	default:
		NewtonMaterialSetCollisionCallback(nWorld, matPair->uiMaterialA,
			matPair->uiMaterialB, (void *)matPair,
			DefaultContactBegin, DefaultContactProcess, DefaultContactEnd);
		break;
	};	
}

void
CMaterials::SetBehaviour (UINT mat1, UINT mat2, resManNS::MaterialsCoef & coefs)
{
	// apply softness 
	NewtonMaterialSetDefaultSoftness(nWorld, mat1, mat2, coefs.fSoftness);
	// apply elasticity
	NewtonMaterialSetDefaultElasticity(nWorld, mat1, mat2, coefs.fElasticity);
	// apply colidable
	NewtonMaterialSetDefaultCollidable(nWorld, mat1, mat2, coefs.iCollidable);
	// apply static and kinetic friction
	NewtonMaterialSetDefaultFriction(nWorld, mat1, mat2, coefs.fStaticFriction, coefs.fKineticFriction);
}

//////////////////////////////VX////////////////////////////////////
////////////////////////////////////////////////////////////////////

// support macro ...
#define CHECK_BOUNDS(what,lower,upper) ((what < lower) || (what > upper))
// for lazy people :)
#define CHB(w,l,u) CHECK_BOUNDS (w,l,u)


// max lenght of line in file that can contain meaning full information
// the rest is omited hahaha :)
#define LINELEN 128

//////////////////////////////////
// Returns num of line chars and the line in line param,
// -1 on EOF
//////////////////////////////////
int CMaterials::GetLine (char *line, FILE *fr)
{
	int i=0;

	do {
		i=0;

		// check EOF
		if (feof (fr))
			return -1;

		fgets (line, LINELEN, fr);		// get the line

		// read until [;\t\n0]
		while ( (line[i]!=0) && (line[i]!=';') && (line[i]!='\n') && (line[i]!='\t') )
			i++;

		// terminate the end
		line[i]=0;
	} while ( i == 0);

	// return # of chars in line
	return i;
}

//////////////////////////////////////////////////
// loads material pair coeficients to coefs param on n-th position from line in param
// checks their values, uses default values on bad input ones
//////////////////////////////////////////////////
void
CMaterials::LoadMaterialPairColisionCoefs (resManNS::MaterialsCoef *coefs,										   
										   char *line, TMutator *defMut, UINT n)
{
	// read 
	if (sscanf_s (line, "%f %f %f %f %i",
		&(coefs[n].fSoftness),
		&(coefs[n].fElasticity),
		&(coefs[n].fStaticFriction),
		&(coefs[n].fKineticFriction),
		&(coefs[n].iCollidable)
		) < 5)
	{
		// error, use defualt
		coefs[n] = defMut->materialCoefs[n];
	}		

	// check ranges of loaded vals. Use defaults on bad ones
	if (CHECK_BOUNDS(coefs[n].fSoftness, 0.1f, 5.0f))
		coefs[n].fSoftness = defMut->materialCoefs[n].fSoftness;

	if (CHECK_BOUNDS(coefs[n].fElasticity, 0.1f, 5.0f))
		coefs[n].fElasticity = defMut->materialCoefs[n].fElasticity;

	if (CHECK_BOUNDS(coefs[n].fStaticFriction, 0.1f, 5.0f))
		coefs[n].fStaticFriction = defMut->materialCoefs[n].fStaticFriction;

	if (CHECK_BOUNDS(coefs[n].fKineticFriction, 0.1f, 5.0f))
		coefs[n].fKineticFriction = defMut->materialCoefs[n].fKineticFriction;

	if ((coefs[n].iCollidable < 0))
		coefs[n].iCollidable = defMut->materialCoefs[n].iCollidable;
	// if it is car<->grass or car<->asphalt material pair, prevent setting 0 for collidable
	// to prevent falling car under the surface
#define CAR_GRASS 2
#define CAR_ASPHALT 3
	if ( (n==CAR_GRASS) || (n==CAR_ASPHALT) )
		coefs[n].iCollidable = 1;
}

////////////////////////////////////////////////////
// applies current selected mutator
////////////////////////////////////////////////////
void
CMaterials::ApplyCurrentMutator (void)
{
	// modify gravity
	// engine efficiency
	// breaks efficiency
	// it is not needed. Vals are directly used
	
	
	for (int i=0; i < CHANGEBLEMATCOEFPAIRCOUNT; i++)
	{
		// copy to materialPairSlots from currentlySelectedMutator
		(materialPairSlots[i]->coefs) = m_currentlyUsedMutator->materialCoefs[i];
		
		// and then apply behaviour
		SetBehaviour (materials->pMaterialPairs[i].uiMaterialA,
			materials->pMaterialPairs[i].uiMaterialB,
			materials->pMaterialPairs[i].coefs);
	}	
}

// function that loads all mutators. It is neccesary that default.mut is among them.
void
CMaterials::LoadMutators (std::vector<CString> & fileNames, CString path )
{
	TMutator *newMut;
	
	int unnamedNum = 0;
	FILE *fr;

	CString mutName;

	char line[LINELEN];

	// load all the files specifiing mutators. If some constants are not specified
	// or are bad, default behavior is used 
	while (fileNames.size ())
	{	
		fileNames.back () = path + fileNames.back ();

		// open the file
		if(fopen_s(&fr, fileNames.back (), "r"))
		{
			ErrorHandler.HandleError (
				ERRFILECORRUPTED,
				"Materials::LoadMutators()",
				fileNames.back ()
				);

			// pop this wrong filename from vect and skip to next one
			fileNames.pop_back ();
			continue;
		}

		// retrieve default mutator
		TMutator *defMut = mutatorMap.find ("default")->second;		

		newMut = new TMutator ();
	
		// load values from file
		// name
		GetLine (line, fr);
		mutName = line;
		if (mutName.IsEmpty () )
		{
			// create meaningfull default name
			mutName = "Unnamed";
			mutName.AppendFormat ("%i",unnamedNum++);
		}

		// gravity
		GetLine (line, fr);
		if ( (sscanf_s (line, "%f", &newMut->gravity) < 1) || 
			(CHECK_BOUNDS(newMut->gravity, 0.01f, 50.0f) ) )
			newMut->gravity = defMut->gravity;	// use default on wrong input

		// breakes efficiency
		GetLine (line, fr);
		if ( (sscanf_s (line, "%f", &newMut->breaksEfficiecy) < 1) || 
			(CHECK_BOUNDS(newMut->breaksEfficiecy, 0.1f, 10.0f) ) )
			newMut->breaksEfficiecy = defMut->breaksEfficiecy;	// use default on wrong input

		// engine effic...
		GetLine (line, fr);
		if ( (sscanf_s (line, "%f", &newMut->engineEfficiecy) < 1) || 
			(CHECK_BOUNDS(newMut->engineEfficiecy, 0.1f, 3.0f) ) )
			newMut->engineEfficiecy = defMut->engineEfficiecy;	// use default on wrong input

		// load default material coefs for pair interaction simulation
		GetLine (line, fr);
		LoadMaterialPairColisionCoefs (newMut->materialCoefs, line, defMut, 0); // car<->wood
		GetLine (line, fr);
		LoadMaterialPairColisionCoefs (newMut->materialCoefs, line, defMut, 1); // car<->metal
		GetLine (line, fr);
		LoadMaterialPairColisionCoefs (newMut->materialCoefs, line, defMut, 2); // car<->grass
		GetLine (line, fr);
		LoadMaterialPairColisionCoefs (newMut->materialCoefs, line, defMut, 3); // car<->asphalt
		
		// add it into mutator vector
		mutatorMap.insert (MutatorMap::value_type (mutName, newMut));

		// skip on next filename
		fileNames.pop_back ();
	}

	// and current will be default
	SelectMutator ("default");
}

void
CMaterials::SelectMutator (CString mutatorName)
{
	MutatorMap::iterator it = mutatorMap.find (mutatorName);
	if (it != mutatorMap.end ())
	{
		m_currentlyUsedMutator = it->second;
	}
	// else error -> mutator not present
}

void
CMaterials::GetMutatorsNamesList (std::vector<CString> &v)
{
	MutatorMap::iterator it;

	// put defualt at the beginning
	it = mutatorMap.find ("default");
	v.push_back (it->first);
	
	// insert the rest
	for (it=mutatorMap.begin (); it!=mutatorMap.end (); it++)
	{
		if (it->first != "default")
			v.push_back (it->first);
	}
}

resManNS::TMaterialPairInfo *
CMaterials::GetMatPair (int mat1, int mat2)
{
	MaterialPairMap::iterator inner;
	MaterialPairMapInner::iterator matPair;
	MaterialPairMapInner *innerMap;

	// find according 1st material index
	inner = matMap.find (mat1);
	if (inner == matMap.end())
		return NULL;	// mat1 not present

	innerMap = inner->second;
	// find according 2nd material index
	matPair = innerMap->find (mat2);
	if (matPair == innerMap->end())
		return NULL;

	return matPair->second;
}

void
CMaterials::InsertMatPair (int mat1, int mat2, resManNS::TMaterialPairInfo *matPair)
{
	MaterialPairMap::iterator inner;
	MaterialPairMapInner *innerCont;

	// check if 1st material is already present
	inner = matMap.find (mat1);
	if (inner == matMap.end())
	{
		// not present so lets make new inner container
		innerCont = new MaterialPairMapInner();
		
		// and add it into matMap
		matMap.insert (MaterialPairMap::value_type (mat1, innerCont) );
	}
	else
	{
		innerCont = inner->second;
	}

	// and add the mat2 into innerContainer
	innerCont->insert (MaterialPairMapInner::value_type (mat2, matPair) );
}

/*
// creates new material in newton and inserts it into newton
void
CMaterials::CreateMaterial (CString materialName)
{
	// check if there already exists
	if (matMap.find (materialName) != matMap.end () )
		return;

	// insert if not
	matMap.insert( 
		TMaterialMap::value_type ( 
			materialName, 
			NewtonMaterialCreateGroupID (nWorld) ) );
}
*/
////////////////////////////////////////////////////////////////////

int CMaterials::DefaultContactBegin(const NewtonMaterial * material, const NewtonBody * body0, const NewtonBody * body1)
{
	// fill current effect
	currentEffect.materialPair = (resManNS::TMaterialPairInfo *) NewtonMaterialGetMaterialPairUserData (material);
	currentEffect.bodyA = body0;
	currentEffect.bodyB = body1;

	// continue resolving contact by returning 1
	return 1;
}

void CMaterials::DefaultContactEnd(const NewtonMaterial * material)
{
	// here we check the speeds and according them we play appropriate sounds...

	#define MIN_CONTACT_SPEED 15
	#define MIN_SCRATCH_SPEED 5
	
	if (currentEffect.normalSpeed > MIN_CONTACT_SPEED)
	{
		// PLAY the impact sound!!!!!!!!!!!!!
		// currentEffect.materialPair->soundEffect;
	}

	if (currentEffect.tangentSpeed > MIN_SCRATCH_SPEED)
	{
		// PLAY scratch sound!!!!!!!!!!!!!!!
		//currentEffect.materialPair->scratchSoundEffect;
	}
}

void
CMaterials::SaveContactSpeeds (const NewtonMaterial * material, const NewtonContact * contact)
{
	// Save contact speeds for decision if we play contact sounds
	// Get the normal speed of this impact to find out if impact sound is played
	float normalSpeed = NewtonMaterialGetContactNormalSpeed (material, contact);
	currentEffect.normalSpeed = normalSpeed;

	// get the tangent speed to find out if scratch sound is played
	float tangentSpeed1 = NewtonMaterialGetContactTangentSpeed (material, contact, 0);
	float tangentSpeed2 = NewtonMaterialGetContactTangentSpeed (material, contact, 1);
	if (tangentSpeed1 > tangentSpeed2) {
		tangentSpeed1 = tangentSpeed2;
	}
	currentEffect.tangentSpeed = tangentSpeed1;
}

int
CMaterials::SurfaceContactProcess (const NewtonMaterial * material, const NewtonContact* contact)
{
	dFloat contactPos[3];
	dFloat contactNorm[3];
	int mat1, mat2;
	CWheel *wheel;
	physics::UserData *udata;

	// here we need to solve special behaviour for metal<->grass and metal<->concrete.
	// Because map surface is not made of
	// plates of grass and asphalt material but 2dimensional array grass or asphalt is
	// so we need to check if current colision is tire<->somthng and set propreties of behaviour
	// according to material we get from that 2D array and set it in the tire

	// first we have to check which of the pair of bodies is terrain
	
	udata = (physics::UserData *) NewtonBodyGetUserData (currentEffect.bodyB);
	if (udata->dataType != PHYSICS_TERRAIN)
	{
		// bodyB is not terrain, check bodyA
		udata = (physics::UserData *) NewtonBodyGetUserData (currentEffect.bodyA);
		if (udata->dataType == PHYSICS_TERRAIN)
		{
			// switch bodyA & bodyB to have terrain in bodyB
			const NewtonBody * tmp = currentEffect.bodyB;
			currentEffect.bodyB = currentEffect.bodyA;
			currentEffect.bodyA = tmp;
		}
	}
	

	// get the 1st material
	mat1 = currentEffect.materialPair->uiMaterialA;

	// now we check if bodyB is terrain
	udata = (physics::UserData *) NewtonBodyGetUserData (currentEffect.bodyB);
	if (udata->dataType == PHYSICS_TERRAIN)
	{
		// retrieve material from 2dimensional array
		// get position of the contact
		NewtonMaterialGetContactPositionAndNormal (material, contactPos, contactNorm);

		// get the material index from this position on the map
		mat2 = physics->terrain.MaterialsMapf(
			contactPos[0],	// x-coord
			contactPos[2]	// z-coord
			);
	}
	else
	{
		// NOR the bodyA is terrain so don't solve collision with terrain
		// so we needn't to retrieve material from 2dimensional array
		mat2 = currentEffect.materialPair->uiMaterialB;
	}

	// now set the colision behaviour according got material and the other material	
	currentEffect.materialPair = GetMatPair (mat1, mat2);

	ApplyMatCoefsNow (material, & currentEffect.materialPair->coefs);

	// if coliding part is wheel, set the material in it
	UINT colID = NewtonMaterialGetBodyCollisionID (material, currentEffect.bodyA);
	if (colID >= WHEEL_COLLITION_ID)
	{ 
		// now set the material in the colliding wheel
		// we obtain poiter to wheel which is in UserData pointer associeated with body
		udata = (physics::UserData *) NewtonBodyGetUserData (currentEffect.bodyA);
		wheel = ((CCar *)udata->data)->GetWheel (colID - WHEEL_COLLITION_ID);

		// and set the material
		wheel->SetMaterial (& currentEffect.materialPair->coefs);
	}

	// continue resolving contact
	return 1;
}

void
CMaterials::ApplyMatCoefsNow (const NewtonMaterial * material, resManNS::MaterialsCoef *coefs)
{
	NewtonMaterialSetContactSoftness(material, coefs->fSoftness);
	NewtonMaterialSetContactElasticity(material, coefs->fElasticity);
	NewtonMaterialSetContactStaticFrictionCoef(material, coefs->fStaticFriction, 0);
	//NewtonMaterialSetContactStaticFrictionCoef(material, coefs->fStaticFriction, 1);
	NewtonMaterialSetContactKineticFrictionCoef(material, coefs->fKineticFriction, 0);
	//NewtonMaterialSetContactKineticFrictionCoef(material, coefs->fKineticFriction, 1);
}

int
CMaterials::DefaultContactProcess (const NewtonMaterial * material, const NewtonContact * contact)
{	
	SaveContactSpeeds (material, contact);

	/*

	int i;
	switch (currentEffect.materialPair->uiMaterialA)
	{
	case physics::MAT_METAL:
		// find out if it is part of a car
		switch (NewtonMaterialGetBodyCollisionID (material, currentEffect.bodyA) )
		{
		case CHASIS_COLLITION_ID:	// chasis
			i = 0;
			break;

		case WHEEL_COLLITION_ID:	// wheel
			i = 0;
			break;

		default:					// other metal material
			
			break;
		}
		break;

	case physics::MAT_WORLDBOX:

		break;

	default:
		if (currentEffect.materialPair->uiMaterialB == physics::MAT_METAL)
			i = 12;
		break;
	}
	*/
	/*
	if(((UserData *)NewtonBodyGetUserData(currentEffect->m_body0))->dataType == PHYSICS_CAR)
	{
		car = currentEffect->m_body0;
		object = currentEffect->m_body1;
	}
	else
	{
		car = currentEffect->m_body1;
		object = currentEffect->m_body0;
	}
	*/

	/*
	UserData * uDatacar = (UserData *)NewtonBodyGetUserData(car);
	UserData * uDataobject = (UserData *)NewtonBodyGetUserData(object);

	resManNS::MaterialsCoef * coef = NULL;
	int index;

	int collID = NewtonMaterialGetBodyCollisionID(material, car);

	switch(collID)
	{
	case CHASIS_COLLITION_ID:
		if(!(uDataobject->material0 && uDatacar->material0))
			return 1;
		index = currentEffect->materials->MaterialXMaterial(uDatacar->material0, uDataobject->material0);
		if(!index)
			return 1;

		coef = currentEffect->materials->MaterialsCoef(index);
		//coef = currentEffect->materials->MaterialsCoef(uDatacar->material0, uDataobject->material0);
		break;
	case WHEEL_COLLITION_ID:
		if(!(uDataobject->material0 && uDatacar->material1))
			return 1;
		index = currentEffect->materials->MaterialXMaterial(uDatacar->material1, uDataobject->material0);
		if(!index)
			return 1;

		coef = currentEffect->materials->MaterialsCoef(index);
		//coef = currentEffect->materials->MaterialsCoef(uDatacar->material1, uDataobject->material0);
		break;
	}

	if(!coef->iCollidable)
		NewtonMaterialDisableContact(material);

	NewtonMaterialSetContactSoftness(material, coef->fSoftness);
	NewtonMaterialSetContactElasticity(material, coef->fElasticity);
	NewtonMaterialSetContactStaticFrictionCoef(material, coef->fStaticFriction, 0);
	NewtonMaterialSetContactStaticFrictionCoef(material, coef->fStaticFriction, 1);
	NewtonMaterialSetContactKineticFrictionCoef(material, coef->fKineticFriction, 0);
	NewtonMaterialSetContactKineticFrictionCoef(material, coef->fKineticFriction, 1);
	*/

	return 1;
};