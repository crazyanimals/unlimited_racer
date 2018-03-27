#include "MapInit.h"
#include "CClient.h"


///////////////////////////////////////////////f////
// Sets car position on the map
//////////////////////////////////////////////////
HRESULT SetCarPosition(graphic::CGrObjectMesh * mesh, UINT CarNumber, resManNS::CResourceManager * ResManager, graphic::CScene * Scene, resManNS::RESOURCEID MapID)
{
	resManNS::Map * MapObj;
	resManNS::MainObject * pMOStartPositions;
	resManNS::GameObject * pGOStartPos;
	float x, y, z, Rot;
	D3DXMATRIX WorldMat;
	D3DXMATRIX TempMat;
	D3DXMATRIX RotMat;
	UINT Height;

	// Set car position on the map
	MapObj = ResManager->GetMap(MapID);

	pMOStartPositions=ResManager->GetMainObject(MapObj->ridStartPosition);
	if (CarNumber < pMOStartPositions->pvObjectIDs->size())
	{
		pGOStartPos=ResManager->GetGameObject(pMOStartPositions->pvObjectIDs->at(CarNumber));

		switch(MapObj->iStartRotation){
			case DIRECTION_N:
				x = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionX + pGOStartPos->x;
				z = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionZ + pGOStartPos->z;
				break;
			case DIRECTION_E:
				x = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionX + pGOStartPos->z;
				z = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionZ + TERRAIN_PLATE_WIDTH - pGOStartPos->x;
				break;
			case DIRECTION_S:
				x = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionX + TERRAIN_PLATE_WIDTH - pGOStartPos->x;
				z = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionZ + TERRAIN_PLATE_WIDTH - pGOStartPos->z;
				break;
			case DIRECTION_W:
				x = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionX + TERRAIN_PLATE_WIDTH -  pGOStartPos->z;
				z = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionZ +  pGOStartPos->x; 
				break;

		}
		Rot = D3DX_PI/ 2 * MapObj->iStartRotation + pGOStartPos->rotationY;
		Height = Scene->Map.GetFieldHeight( (int) (x / TERRAIN_PLATE_WIDTH), (int) (z / TERRAIN_PLATE_WIDTH) ); 
		y = (float) Height * TERRAIN_PLATE_HEIGHT;
		
		D3DXMatrixTranslation(&TempMat,x, y, z);
		D3DXMatrixRotationY(&RotMat, Rot);	
		WorldMat = mesh->GetWorldMat();
		WorldMat = RotMat * WorldMat * TempMat;
		mesh->SetWorldMat(WorldMat);
	}
	else
	{
		ERRORMSG(-1, "MapInit::SetCarPosition()", "Car position too big, no available starting position on map.");
	}
	
	return 0;
}


//////////////////////////////////////////////////////
// Load car from *.car file and return its mesh
///////////////////////////////////////////////////////
HRESULT loadPhCar(	LPCTSTR carName, 
					UINT CarNumber, 
					int CarIndex, 
					physics::CPhysics * Physics, 
					resManNS::CResourceManager * ResManager, 
					resManNS::RESOURCEID MapID, 
					CGameMod * pGameMod, 
					void (CALLBACK * ProgressCallback) (CString) )
{
	HRESULT hr;
	
	resManNS::RESOURCEID CarID;
	resManNS::CarObject * CarObj;
	resManNS::GameObject * GameObj;
	resManNS::MainObject * pMOStartPositions;
	resManNS::GameObject * pGOStartPos;
	resManNS::Map * MapObj;
	physics::CCar *car;
	physics::InitCar initCar;
	float x, y, z, Rot;
	UINT Height;
	
	if (ProgressCallback) ProgressCallback( "Loading physics for car...");
	CarID = ResManager->LoadResource(carName, RES_CarObject, true, 0);
	if (FAILED(CarID) ) ERRORMSG(CarID, "MapInit::loadPhCar()", CString("Unable to load car object name: ") + carName);
	CarObj = ResManager->GetCarObject(CarID);

	car = new physics::CCar();
	
	FillInitCarStruct(CarObj, &initCar, ResManager);

	// assign car material...
	initCar.material = Physics->materials.GetMaterialID (physics::MAT_METAL);

	hr = car->Init(Physics->World(), Physics->Resources(), &initCar, Physics);
	if(hr){ delete car; ErrorHandler.HandleError(hr,"MapInit::loadPhCar()","["+initCar.model+"] Car load failed");return ERRGENERIC;}
	
	GameObj = ResManager->GetGameObject(CarObj->ridCarObject);
	MapObj = ResManager->GetMap(MapID);
	
	pMOStartPositions=ResManager->GetMainObject(MapObj->ridStartPosition);
	if (CarNumber < pMOStartPositions->pvObjectIDs->size())
	{
		pGOStartPos=ResManager->GetGameObject(pMOStartPositions->pvObjectIDs->at(CarNumber));

		switch(MapObj->iStartRotation){
			case DIRECTION_N:
				x = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionX + pGOStartPos->x;
				z = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionZ + pGOStartPos->z;
				break;
			case DIRECTION_E:
				x = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionX + pGOStartPos->z;
				z = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionZ + TERRAIN_PLATE_WIDTH - pGOStartPos->x;
				break;
			case DIRECTION_S:
				x = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionX + TERRAIN_PLATE_WIDTH - pGOStartPos->x;
				z = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionZ + TERRAIN_PLATE_WIDTH - pGOStartPos->z;
				break;
			case DIRECTION_W:
				x = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionX + TERRAIN_PLATE_WIDTH -  pGOStartPos->z;
				z = (float) TERRAIN_PLATE_WIDTH * MapObj->iStartPositionZ +  pGOStartPos->x; 
				break;

		}
		Rot = D3DX_PI/ 2 * MapObj->iStartRotation + pGOStartPos->rotationY;
		//Height = Scene->Map.GetFieldHeight( (int) (x / TERRAIN_PLATE_WIDTH), (int) (z / TERRAIN_PLATE_WIDTH) ); 
		Height = 0;
		y = (float) Height * TERRAIN_PLATE_HEIGHT;
	}
	else
	{
		delete car; ERRORMSG(ERRGENERIC, "MapInit::loadPhCar()", "Car number too big, unable to find starting position.");
	}


	// Set car position on the map - if the GameMod uses its own position, use it; otherwise apply the one stored in map
	D3DXVECTOR3	carPos, carRot;
	
	if ( pGameMod->GetCarPosition( CarIndex, carPos, carRot ) ) {
		car->Rotation( carRot.x, carRot.y, carRot.z );
		if ( carPos.y < 0.0f ) car->Position( carPos.x, carPos.z );
		else car->Position( carPos.x, carPos.y, carPos.z );
	} else {
		car->Rotation( GameObj->rotationX, Rot + GameObj->rotationY, GameObj->rotationZ );
		car->Position( (GameObj->x + x) / 100.0f, /*(GameObj->y + y + 100) / 100.0f,*/ (GameObj->z + z) / 100.0f );
	}
	
	// add car to physics
	Physics->cars[CarIndex] = car;
	
	return 0;
}

HRESULT loadCar(LPCTSTR carName, UINT CarNumber, int CarIndex, resManNS::CResourceManager * ResManager, CClient * client, resManNS::RESOURCEID MapID, void (CALLBACK * ProgressCallback) (CString), graphic::CScene * _Scene)
{

	HRESULT hr;
	graphic::CGrObjectCar *mesh;
	UINT i;

	//CScene * Scene = client ? client->GetScene() : _Scene;

	graphic::CScene * Scene = client ? client->GetScene() : _Scene;
	
	resManNS::RESOURCEID CarID;
	resManNS::CarObject * CarObj;
	resManNS::GameObject * GameObj;
	resManNS::LightObject * LightObj;
	
	// Car light
	graphic::CLight * Light;
	// Car billboard light
	graphic::CGrObjectBillboard		* pBillboard;

	if (ProgressCallback) ProgressCallback( "Loading car...");
	CarID = ResManager->LoadResource(carName, RES_CarObject, true, 0);
	if (FAILED(CarID) ) ERRORMSG(CarID , "MapInit::loadCar()", CString("Unable to load car object name: ") + carName);
	CarObj = ResManager->GetCarObject(CarID);
	GameObj = ResManager->GetGameObject(CarObj->ridCarObject);

	mesh = new graphic::CGrObjectCar();
	hr = mesh->Init(FALSE, *(GameObj->psModel), Configuration.ModelLODCount);
	if(hr){delete mesh; ErrorHandler.HandleError(hr,"MapInit::loadCar()","Mesh init failed");return ERRGENERIC;}
	
	mesh->InitialWorldPlacement(	D3DXVECTOR3(	GameObj->x,
													GameObj->y,
													GameObj->z),
									D3DXVECTOR3(	GameObj->rotationX, 
													GameObj->rotationY, 
													GameObj->rotationZ),
									D3DXVECTOR3(	GameObj->scaleX, 
													GameObj->scaleY, 
													GameObj->scaleZ) );													

	// Init car light - only one light and must be car reflector
	for (i = 0; i < GameObj->pvLightObjectIDs->size(); i++)
	{
		LightObj = ResManager->GetLightObject(GameObj->pvLightObjectIDs->at(i) );
		if (LightObj->type == LT_CARREFLECTOR)
		{
			Light = new graphic::CLight();
			Light->Ambient = D3DXCOLOR(LightObj->Ambient);
			Light->Attenuation0 = LightObj->Attenuation0;
			Light->Attenuation1 = LightObj->Attenuation1;
			Light->Attenuation2 = LightObj->Attenuation2;
			Light->Diffuse = D3DXCOLOR(LightObj->Diffuse);
			Light->Direction = D3DXVECTOR3(LightObj->directionX, LightObj->directionY, LightObj->directionZ);
			Light->Enabled = true;
			Light->Falloff = LightObj->Falloff;
			Light->pEmitor = mesh;
			Light->Phi = LightObj->Phi;
			Light->Position = D3DXVECTOR3(LightObj->x, LightObj->y, LightObj->z);
			Light->Range = LightObj->Range;
			Light->Specular = D3DXCOLOR(LightObj->Specular);
			Light->Theta = LightObj->Theta;
			Light->Type = LightObj->type;

			mesh->SetLight(Light);
		}
		else if ( LightObj->type == LT_FOGLAMP ) // light represented by sprite object
		{
			if ( !Scene->SkySystem.IsDay() )
			{
				pBillboard = new graphic::CGrObjectBillboard;
				if ( !pBillboard ) ERRORMSG( ERROUTOFMEMORY, "MapInit::loadCar()", "" );
				hr = pBillboard->Init( false, LightObj->ridLightMap, 0 );
				if ( hr ) ERRORMSG( ERRGENERIC, "MapInit::loadCar()", "Light glow initialization failed." );

				pBillboard->SetPosition(LightObj->x, LightObj->y, LightObj->z);
				pBillboard->SetScaling( LightObj->Scale );
				Scene->TransparentObjects.push_back( pBillboard );
				mesh->Billboards.push_back(pBillboard);
				mesh->BillboardsObjects.push_back(LightObj);
			}
		}
	}

	// Set car position on the map - if the GameMod uses its own position, use it; otherwise apply the one stored in map
	D3DXVECTOR3	carPos, carRot;
	
	if ( Client->GetGameMOD()->GetCarPosition( CarIndex, carPos, carRot ) ) {
		D3DXMATRIX	WorldMat, TempMat, RotMat;
		D3DXMatrixTranslation( &TempMat, carPos.x, carPos.y, carPos.z );
		D3DXMatrixRotationY( &RotMat, carRot.y );	
		WorldMat = mesh->GetWorldMat();
		WorldMat = RotMat * WorldMat * TempMat;
		mesh->SetWorldMat( WorldMat );
	} else {
		hr = SetCarPosition(mesh, CarNumber, ResManager, Scene,  MapID);
		if (FAILED(hr) ) { delete mesh; delete Light; ERRORMSG(hr, "MapInit::LoadCar()", "Unable to set car position") };
	}

	graphic::CGrObjectCar * mesh1;
	
	// Add car object into dynamic scene objects
	Scene->DynamicObjects.push_back(mesh);
	Scene->Cars.push_back(mesh);
	if(client)
		client->GetvCarsMesh()[CarIndex] = mesh;
	Scene->Map.BoundObject(mesh);

	resManNS::GameObject * WheelObj;

	for(int i=0; i<(int)CarObj->pvWheelIDs->size(); i++)
	{
		mesh1 = new graphic::CGrObjectCar();

		WheelObj = ResManager->GetGameObject((*CarObj->pvWheelIDs)[i]);
		

		hr = mesh1->Init(FALSE, *(WheelObj->psModel), Configuration.ModelLODCount);
		if(hr){ErrorHandler.HandleError(ERRGENERIC,"MapInit::loadCar()","Mesh init failed"); return ERRGENERIC;}

		mesh1->InitialWorldPlacement(	D3DXVECTOR3(	WheelObj->x,
														WheelObj->y,
														WheelObj->z ),
										D3DXVECTOR3(	WheelObj->rotationX, 
														WheelObj->rotationY, 
														WheelObj->rotationZ ),
										D3DXVECTOR3(	GameObj->scaleX, 
														GameObj->scaleY, 
														GameObj->scaleZ ) );
	
		if (client) client->GetvObjectsMesh()->push_back(mesh1);
		Scene->DynamicObjects.push_back(mesh1);
		Scene->Cars.push_back(mesh1);
		Scene->Map.BoundObject(mesh1);
		mesh->SetWheel( (UINT) i, mesh1);
	}

	return 0;
};


//////////////////////////////////////////////////////////////////////////
// SERVER mapInit function:
// Load map from file mapName (LPTCTSTR)
// set all object to the physics (CPhysics)
// 
//////////////////////////////////////////////////////////////////////////

HRESULT MapPhInit(	CStringA mapName,
					physics::CPhysics * Physics,
					resManNS::CResourceManager * ResManager,
					void (CALLBACK * ProgressCallback) (CString),
					CGameMod * pGameMod )
{
	
	HRESULT						hr = ERRNOERROR;
	
	float						dx, dy, dz;
	int							px,pz,dpx,dpy;
	int							t,i,j,rot,shift,_x,_y;
	UINT						j1, i1,id;

	CAtlStringA					pomString;
	CAtlStringA					objectPath,string,lightPath;

	// TODO: add map loading CPhPlate					* pTP;
	
	dVector						pomVect;

	resManNS::Map			*	resMap=NULL;
	resManNS::MainObject	*	pMO=NULL;
	resManNS::GameObject	*	pGO=NULL;
	resManNS::PhysicalTexture *	PT=NULL;
	resManNS::RESOURCEID		resMapID;
	resManNS::RESOURCEID		pomObjID;
	
	physics::InitObject			initObject;
	physics::CObject		*	pPhObj = NULL;
	UINT						size;
	CGameMod::OBJECT_LIST		vModObjects;



	lightPath = CONFIG_LIGHTSPATH;
	pomString = CONFIG_MAPSPATH;


	resMapID = ResManager->LoadResource(mapName,RES_Map,TRUE,NULL);
	
	if ( resMapID < 1 ) ERRORMSG( ERRGENERIC, "MapInit::MapPhInit()", "Could not load map object." );
	resMap = ResManager->GetMap(resMapID);

	/////////////////////////////
	//LOAD TERRAIN             
	/////////////////////////////
	if (ProgressCallback) ProgressCallback( "Loading physics for terrain...");

	hr = Physics->terrain.Load(*(resMap->sTerrainName), Physics, 
		Physics->materials.GetMaterialID (physics::MAT_DEFAULT) );
	if (hr)
	{
		ERRORMSG(hr,"MapInit::MapPhInit()","Terrain wasn't loaded");
	}
	
	/////////////////////////////////
	// LOADING OBJECTS
	/////////////////////////////////
	////////////////////////////////////////
	// LOADING OBJETS AND ROADS POSITIONS 
	////////////////////////////////////////
	if (ProgressCallback) ProgressCallback( "Loading physics for objects...");

	t = (int) resMap->uiPlacedObjectsCount;
	for(i=0; i < t ;i++)
	{

		/* Some objects have special meanings in particular game MOD(s), some doesn't have to exist in another
			MOD at all. These objects are being processed with special care. */
		
		// if this object is a special one, find out if it is supported by actual GameMod
		if ( resMap->pbPlacedObjectSpecial[i] )
		{
			bool bFound = false;

			for ( UINT j = 0; j < resMap->pPlacedObjectData[i].size(); j++ )
				if ( resMap->pPlacedObjectData[i][j].sModID == pGameMod->GetModulePrimaryID().MakeUpper() )
				{ 
					bFound = true;
					break;
				}

			if ( !bFound ) continue; // this object is not designed for actual game MOD
		}

		
		// clear the index list of meshes inside this object
		vModObjects.clear();


		//////////////////////////////////
		// LOADING OBJETS 
		/////////////////////////////////
		pomObjID = resMap->puiPlacedObjectIndexes[i];
		pMO=ResManager->GetMainObject(resMap->pridObjects[pomObjID]);
		if (!pMO) ERRORMSG(-1, "MapInit::MapPhInit()", CString("Unable to get main object name: ") + resMap->psObjectNames[i]);
		px = resMap->piPlacedObjectX[i];
		pz = resMap->piPlacedObjectZ[i];
		rot = resMap->puiPlacedObjectOrientations[i];
		
		//Get size
		size = 1;
		for(j=0; j < (int) pMO->pvObjectIDs->size(); j++)
		{
			pomObjID = pMO->pvObjectIDs->at(j);
			pGO = ResManager->GetGameObject(pomObjID);
			if((pGO->sizeX + pGO->posX) > size)  size = (pGO->sizeX + pGO->posX) ;
			if((pGO->sizeY + pGO->posY) > size)  size =(pGO->sizeY + pGO->posY) ;
		}

		for(j = 0;j < (int) pMO->pvRoadIDs->size(); j++)
		{
			pomObjID = pMO->pvRoadIDs->at(j);
			pGO = ResManager->GetGameObject(pomObjID);
			if((pGO->sizeX + pGO->posX) > size)  size = (pGO->sizeX + pGO->posX);
			if((pGO->sizeY + pGO->posY) > size)  size =(pGO->sizeY + pGO->posY);
		}
		shift = (size-1)/2;

		for(j=0; j < (int) pMO->pvObjectIDs->size();j++)
		{ 
			pomObjID = pMO->pvObjectIDs->at(j);
			pGO = ResManager->GetGameObject(pomObjID);

			FillInitObjectStruct(pGO, &initObject);

			initObject.objectID = Physics->materials.GetMaterialID (physics::MAT_DEFAULT);

			if(initObject.isStatic)
				pPhObj = new physics::CStaticObject();
			else
				pPhObj = new physics::CDynamicObject();

			hr = pPhObj->Init(Physics->World(), Physics->Resources(), &initObject, &Physics->materials);

			if(hr) { ErrorHandler.HandleError(ERRGENERIC, "MapInit::MapPhInit()", "["+initObject.model+"] Object load failed."); return ERRGENERIC;}
									
			switch( rot % 4)
			{
				case DIRECTION_N:					
					dx = (float) TERRAIN_PLATE_WIDTH *px + pGO->x + TERRAIN_PLATE_WIDTH * pGO->posX;
					dy = (float) pGO->y;
					dz = (float) TERRAIN_PLATE_WIDTH *pz + pGO->z + TERRAIN_PLATE_WIDTH * pGO->posY;
					break;
				
				case DIRECTION_E:
					dx = (float) TERRAIN_PLATE_WIDTH *px +  pGO->z + TERRAIN_PLATE_WIDTH * pGO->posY;
					dy = (float) pGO->y;
					dz = (float) TERRAIN_PLATE_WIDTH *pz +  TERRAIN_PLATE_WIDTH*(size) - pGO->x - TERRAIN_PLATE_WIDTH * pGO->posX;
					break;
				
				case DIRECTION_S:
					dx = (float) TERRAIN_PLATE_WIDTH *px +  TERRAIN_PLATE_WIDTH*(size)  - pGO->x - TERRAIN_PLATE_WIDTH * pGO->posX;
					dy = (float) pGO->y;
					dz = (float) TERRAIN_PLATE_WIDTH *pz +  TERRAIN_PLATE_WIDTH*(size)  - pGO->z - TERRAIN_PLATE_WIDTH * pGO->posY;
					break;

				case DIRECTION_W:
					dx = (float) TERRAIN_PLATE_WIDTH *px + TERRAIN_PLATE_WIDTH*(size)  - pGO->z - TERRAIN_PLATE_WIDTH * pGO->posY;
					dy = (float) pGO->y;
					dz = (float) TERRAIN_PLATE_WIDTH *pz + pGO->x + TERRAIN_PLATE_WIDTH * pGO->posX;
					break;
			
			}

			/////////////////////////////////
			// SET POSITION TO PHYSICS
			////////////////////////////////
			
			dVector rotation (pGO->rotationX,D3DX_PI/2*rot + pGO->rotationY,pGO->rotationZ);
			dVector position ((dx-shift*TERRAIN_PLATE_WIDTH)/ONE_METER,dy/ONE_METER,(dz-shift*TERRAIN_PLATE_WIDTH)/ONE_METER);
			pPhObj->PlaceInWorld(position, rotation);
			if( initObject.isStatic )
			{
				vModObjects.push_back( CGameMod::OBJECT(false,(int)Physics->staticObjects.size()) );
				Physics->staticObjects.push_back((physics::CStaticObject *)pPhObj);
			}
			else
			{
				vModObjects.push_back( CGameMod::OBJECT(true,(int)Physics->dynamicObjects.size()) );
				Physics->dynamicObjects.push_back((physics::CDynamicObject *)pPhObj);
			}
		}
		
		////////////////////////////////////////////////////
		// LOADING ROADS POSITION
		////////////////////////////////////////////////////
		for(j=0; j < (int) pMO->pvRoadIDs->size(); j++)
		{
			pomObjID = pMO->pvRoadIDs->at(j);
			pGO = ResManager->GetGameObject(pomObjID);
	
			for(j1=0;j1<pGO->sizeY;j1++)	
				for(i1=0;i1<pGO->sizeX;i1++){
					id = j1*pGO->sizeX+i1;
					if( !(pGO->pvPhTextureFileNames->size() > 0) ){
						ERRORMSG(ERRGENERIC,"["+*(pMO->sName)+"] MapPhInit()","No physical texture");
					}
					if(pGO->pvPhTextureFileNames->size() <= id) {
						id=(UINT) pGO->pvPhTextureFileNames->size()-1;
					}
					if(pGO->pvPhTextureFileNames->at(id) !="")
					{
						PT = ResManager->GetPhysicalTexture(pGO->pvPhTextureIDs->at(id));
						switch(rot % 4){	//rotation and move
							case DIRECTION_N:
								dpx = (px+pGO->posX + i1)-shift;	 
								dpy = (pz+pGO->posY + j1)-shift;
								_x = PHYSICAL_TEXTURE_SIZE*dpx;
								_y = PHYSICAL_TEXTURE_SIZE*dpy;
								for(int iiy=0; iiy<PHYSICAL_TEXTURE_SIZE; iiy++)
									for(int iix=0; iix<PHYSICAL_TEXTURE_SIZE; iix++)
										if(PT->pPhysicalTexture[iix][iiy] > -1)
											Physics->terrain.MaterialsMap(_x+iix, _y+iiy) = PT->pPhysicalTexture[iix][iiy];
								// add this "surface" object into the list being sent to GameMod
								vModObjects.push_back( CGameMod::OBJECT( dpx, dpy ) );
								break;
							case DIRECTION_E:
								dpx = (px + pGO->posY + j1)-shift;
								dpy = (pz + size-1 - pGO->posX - i1)-shift;
								_x = PHYSICAL_TEXTURE_SIZE*dpx;
								_y = PHYSICAL_TEXTURE_SIZE*dpy;
								for(int iiy=0; iiy<PHYSICAL_TEXTURE_SIZE; iiy++)
									for(int iix=0; iix<PHYSICAL_TEXTURE_SIZE; iix++)
										if(PT->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-iiy][iix] > -1)
											Physics->terrain.MaterialsMap(_x+iix, _y+iiy) = PT->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-iiy][iix];
								// add this "surface" object into the list being sent to GameMod
								vModObjects.push_back( CGameMod::OBJECT( dpx, dpy ) );
								break;

							case DIRECTION_S:
								dpx = (px + size-1 - pGO->posX - i1)-shift;
								dpy = (pz + size-1 - pGO->posY - j1)-shift;
								_x = PHYSICAL_TEXTURE_SIZE*dpx;
								_y = PHYSICAL_TEXTURE_SIZE*dpy;
								for(int iiy=0; iiy<PHYSICAL_TEXTURE_SIZE; iiy++)
									for(int iix=0; iix<PHYSICAL_TEXTURE_SIZE; iix++)
										if(PT->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-iix][PHYSICAL_TEXTURE_SIZE-1-iiy] > -1)
											Physics->terrain.MaterialsMap(_x+iix, _y+iiy) = PT->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-iix][PHYSICAL_TEXTURE_SIZE-1-iiy];
								// add this "surface" object into the list being sent to GameMod
								vModObjects.push_back( CGameMod::OBJECT( dpx, dpy ) );
								break;
							
							case DIRECTION_W:	
								dpx = (px + size-1 - pGO->posY - j1)-shift;
								dpy = (pz + pGO->posX + i1)-shift;
								_x = PHYSICAL_TEXTURE_SIZE*dpx;
								_y = PHYSICAL_TEXTURE_SIZE*dpy;
								for(int iiy=0; iiy<PHYSICAL_TEXTURE_SIZE; iiy++)
									for(int iix=0; iix<PHYSICAL_TEXTURE_SIZE; iix++)
										if(PT->pPhysicalTexture[iiy][PHYSICAL_TEXTURE_SIZE-1-iix] > -1)
											Physics->terrain.MaterialsMap(_x+iix, _y+iiy) = PT->pPhysicalTexture[iiy][PHYSICAL_TEXTURE_SIZE-1-iix];
								// add this "surface" object into the list being sent to GameMod
								vModObjects.push_back( CGameMod::OBJECT( dpx, dpy ) );
								break;
						}
					}
				}
		} // loading roads


		////////////////////////////////////////////////////
		// PROCESSING CUSTOM PROPERTIES
		////////////////////////////////////////////////////
		
		// if this object is a special one, process it by actual game MOD
		if ( resMap->pbPlacedObjectSpecial[i] )
		{
			int		iModInd = 0;

			for ( UINT j = 0; j < resMap->pPlacedObjectData[i].size(); j++ )
				if ( resMap->pPlacedObjectData[i][j].sModID == pGameMod->GetModulePrimaryID().MakeUpper() )
					iModInd = j;
			
			pGameMod->ServerInitObject( &vModObjects, &(resMap->pPlacedObjectData[i][iModInd].vProperties) );
		}


	} // loading main objects
	
	//Physics->terrain.Debug_Print("map_roads");

	return 0;//hr
};



//////////////////////////////////////////////////////////////////////////
// CLIENT mapInit function:
// Load map from file mapName (LPTCTSTR)
// set all object to the Scene (CScene)
// set all cars Mesh in vector to the scene (vector< CGrObjectMesh *>)
//////////////////////////////////////////////////////////////////////////
HRESULT MapInit(LPCTSTR mapName, graphic::CScene * Scene, resManNS::CResourceManager * ResManager, void (CALLBACK * ProgressCallback) (CString), CClient * pClient)
{
	
	HRESULT hr = ERRNOERROR;
	
	resManNS::Map					*	resMap=NULL;
	resManNS::RESOURCEID				resMapID;
	resManNS::RESOURCEID				pomObjID;
	resManNS::MainObject			*	pMO=NULL;
	resManNS::LightObject			*	pLO=NULL;
	resManNS::GameObject			*	pGO=NULL;
	resManNS::Texture				*	pTO=NULL;					
	graphic::CGrObjectTerrainPlate	*	pTP=NULL;

	graphic::CLight					*	pSunLight;
	graphic::CLight			    	*	pLight;
	graphic::CGrObjectBillboard		*	pBillboard;
		
	int						px,pz,rot,dpx,dpy,shift;
	UINT					size;
	float					dx,dy,dz;
	float					lx,ly,lz,rx,ry,rz;
	UINT					i,is,j1,i1,j,k,xn,yn,id;

	D3DXVECTOR3				pomVect;
	D3DXVECTOR3				vecPos;
	D3DXVECTOR3				vecDir;
	D3DXMATRIX				dxMat;
	D3DXMATRIX				TempMat;

	graphic::CGrObjectMesh			*	pMesh;
	CGameMod::OBJECT_LIST				vModObjects;


	

	if (ProgressCallback) ProgressCallback( "Loading map...");
	resMapID = ResManager->LoadResource(mapName,RES_Map,TRUE,NULL);
	
	if ( resMapID < 1 ) ERRORMSG( ERRGENERIC, "MapInit::MapInit()", "Could not load map object." );
	resMap = ResManager->GetMap(resMapID);

	/////////////////////////////
	// INITIALIZE TERRAIN 
	/////////////////////////////
	if (ProgressCallback) ProgressCallback( "Loading terrain...");
	hr = Scene->Map.Init( TRUE, *(resMap->sTerrainName), Configuration.TerrainLODCount );
	if (hr){ ERRORMSG(hr,"MapInit::MapInit()","Terrain wasn't loaded");}

	if (ProgressCallback) ProgressCallback( "Generating terrain borders...");
	hr = Scene->Map.InitBorders();
	if (hr) ERRORMSG(hr,"MapInit::MapInit()","Unable to generate terrain borders.");

	if (ProgressCallback) ProgressCallback( "Planting grass...");
	// Grass initialization 
	if ( Configuration.RenderGrass )
	{
		hr = InitGrass( Scene );
		if ( hr ) ERRORMSG( ERRGENERIC, "MapInit::MapInit()", "Unsuccessful grass initialization." ); 
	}
	

	if (ProgressCallback) ProgressCallback( "Loading sky...");
	/////////////////////////////
	//LOAD SKYBOX             
	/////////////////////////////
	hr = Scene->SkySystem.Init( true, *(resMap->sSkyboxName) );
	if (hr){ERRORMSG(hr,"MapInit::MapInit()","Sky system wasn't loaded");}
	
	Scene->SkySystem.SetTimeAcceleration( 0 );
	if (!pClient)
	{
		Scene->SkySystem.SetSkyTime( (float) Configuration.HoursOfDay / 24 + (float) Configuration.MinutesOfDay / (24 * 60));
	}
	else
	{
		Scene->SkySystem.SetSkyTime( (float) Client->MultiPlayerInfo.GetHours() / 24 + (float) Client->MultiPlayerInfo.GetMinutes() / (24 * 60) );
	}

	pSunLight = new graphic::CLight();
	if ( !pSunLight ) { ERRORMSG(ERROUTOFMEMORY, "MapInit::MapInit()", "Sun light can't be created."); }
	
	Scene->SunLight = pSunLight;

	pSunLight->SetStandardLight( LT_DIRECTIONAL );
	Scene->SunLight->Diffuse.r = Scene->SkySystem.SunColorByTime(Scene->SkySystem.GetSkyTime() ).x;
	Scene->SunLight->Diffuse.g = Scene->SkySystem.SunColorByTime(Scene->SkySystem.GetSkyTime() ).y;
	Scene->SunLight->Diffuse.b = Scene->SkySystem.SunColorByTime(Scene->SkySystem.GetSkyTime() ).z;
	Scene->SunLight->Direction = -Scene->SkySystem.SunPosition(Scene->SkySystem.GetSkyTime() );
	D3DXVec3Normalize(&(Scene->SunLight->Direction), &(Scene->SunLight->Direction));
	Scene->SunLight->Position = 10 * ONE_KILOMETER * (-Scene->SunLight->Direction);
	

	/////////////////////////////////////
	//LOADING AND SETTING AMBIENT LIGHT & FOG COLOR
	/////////////////////////////////////
	pLight = new graphic::CLight();
	pLight->SetStandardLight( LT_AMBIENT );
	pLight->SetColor(resMap->uiColor,0,0);
	Scene->AmbientLight = pLight;
	Scene->FogColor = D3DXVECTOR3( ((float)((resMap->uiFogColor & 0x00ff0000) >> 16)) / 255.0f,
									((float)((resMap->uiFogColor & 0x0000ff00) >> 8)) / 255.0f,
									((float)(resMap->uiFogColor & 0x000000ff)) / 255.0f );
	

	if (ProgressCallback) ProgressCallback( "Loading map objects...");
	////////////////////////////////////////
	// LOADING OBJETS AND ROADS POSITIONS 
	////////////////////////////////////////
	// Clear client objects mesh
	if (pClient) pClient->GetvObjectsMesh()->clear();
	for(i=0; i < resMap->uiPlacedObjectsCount ;i++){

		/* Some objects have special meanings in particular game MOD(s), some doesn't have to exist in another
			MOD at all. These objects are being processed with special care. */
		
		// if this object is a special one, find out if it is supported by actual GameMod
		if ( resMap->pbPlacedObjectSpecial[i] )
		{
			bool bFound = false;

			for ( UINT j = 0; j < resMap->pPlacedObjectData[i].size(); j++ )
				if ( resMap->pPlacedObjectData[i][j].sModID == Client->GetGameMOD()->GetModulePrimaryID().MakeUpper() )
				{ 
					bFound = true;
					break;
				}

			if ( !bFound ) continue; // this object is not designed for actual game MOD
		}

		
		// clear the index list of meshes inside this object
		vModObjects.clear();


		///////////////////
		// LOADING OBJETS 
		/////////////////////
		pomObjID = resMap->puiPlacedObjectIndexes[i];
		pMO=ResManager->GetMainObject(resMap->pridObjects[pomObjID]);
		px = resMap->piPlacedObjectX[i];
		pz = resMap->piPlacedObjectZ[i];
		rot = resMap->puiPlacedObjectOrientations[i];
		
		//GET SIZE
		size = 1;
		for(is = 0;is < pMO->pvObjectIDs->size(); is++){
			pomObjID = pMO->pvObjectIDs->at(is);
			pGO = ResManager->GetGameObject(pomObjID);
			if((pGO->sizeX + pGO->posX) > size)  size = (pGO->sizeX + pGO->posX) ;
			if((pGO->sizeY + pGO->posY) > size)  size =(pGO->sizeY + pGO->posY) ;
		};
		for(is = 0;is < pMO->pvRoadIDs->size(); is++){
			pomObjID = pMO->pvRoadIDs->at(is);
			pGO = ResManager->GetGameObject(pomObjID);
			if((pGO->sizeX + pGO->posX) > size)  size = (pGO->sizeX + pGO->posX) ;
			if((pGO->sizeY + pGO->posY) > size)  size =(pGO->sizeY + pGO->posY) ;
		};
		shift = (size-1)/2;

		for(j=0; j < pMO->pvObjectIDs->size();j++){ 
			pomObjID = pMO->pvObjectIDs->at(j);
			pGO = ResManager->GetGameObject(pomObjID);
			
			pMesh = new graphic::CGrObjectMesh;
			hr = pMesh->Init(pGO->isStatic,*pGO->psModel,Configuration.ModelLODCount);
			if(hr) { SAFE_DELETE(pMesh); ERRORMSG(ERRGENERIC,"MapInit::MapInit()","Mesh wasn't initialized");} 
			/*
			pGO->sizeX;
			pGO->sizeY;
			pGO->posX;
			pGO->posY;*/
			switch( rot % 4){
				case DIRECTION_N:					
					dx = (float) TERRAIN_PLATE_WIDTH *px + pGO->x + TERRAIN_PLATE_WIDTH * pGO->posX;
					dy = (float) pGO->y;
					dz = (float) TERRAIN_PLATE_WIDTH *pz + pGO->z + TERRAIN_PLATE_WIDTH * pGO->posY;
					break;
				
				case DIRECTION_E:
					dx = (float) TERRAIN_PLATE_WIDTH *px +  pGO->z + TERRAIN_PLATE_WIDTH * pGO->posY;
					dy = (float) pGO->y;
					dz = (float) TERRAIN_PLATE_WIDTH *pz +  TERRAIN_PLATE_WIDTH*(size) - pGO->x - TERRAIN_PLATE_WIDTH * pGO->posX;
					break;
				
				case DIRECTION_S:
					dx = (float) TERRAIN_PLATE_WIDTH *px +  TERRAIN_PLATE_WIDTH*(size)  - pGO->x - TERRAIN_PLATE_WIDTH * pGO->posX;
					dy = (float) pGO->y;
					dz = (float) TERRAIN_PLATE_WIDTH *pz +  TERRAIN_PLATE_WIDTH*(size)  - pGO->z - TERRAIN_PLATE_WIDTH * pGO->posY;
					break;

				case DIRECTION_W:
					dx = (float) TERRAIN_PLATE_WIDTH *px + TERRAIN_PLATE_WIDTH*(size)  - pGO->z - TERRAIN_PLATE_WIDTH * pGO->posY;
					dy = (float) pGO->y;
					dz = (float) TERRAIN_PLATE_WIDTH *pz + pGO->x + TERRAIN_PLATE_WIDTH * pGO->posX;
					break;
			
			}
			
			/////////////////////////////////
			// SET POSITION TO GRAPHIC
			////////////////////////////////
			if(!pClient)
			{

				pMesh->InitialWorldPlacement(	D3DXVECTOR3( dx-shift*TERRAIN_PLATE_WIDTH, dy, dz-shift*TERRAIN_PLATE_WIDTH),
												D3DXVECTOR3( pGO->rotationX, D3DX_PI/2*rot + pGO->rotationY, pGO->rotationZ ),
												D3DXVECTOR3( pGO->scaleX, pGO->scaleY, pGO->scaleZ ) );
				pMesh->SetMapPositionAsWorldPosition();
				dxMat = pMesh->GetWorldMat();
			}
			else{
				pMesh->SetScaleMatrix( pGO->scaleX, pGO->scaleY, pGO->scaleZ);

				D3DXMatrixScaling( &dxMat, pGO->scaleX, pGO->scaleY, pGO->scaleZ);
				D3DXMatrixRotationYawPitchRoll( &TempMat, D3DX_PI/2*rot + pGO->rotationY, pGO->rotationX, pGO->rotationZ  );
				D3DXMatrixMultiply( &dxMat, &dxMat, &TempMat );
				D3DXMatrixTranslation( &TempMat, dx-shift*TERRAIN_PLATE_WIDTH, dy, dz-shift*TERRAIN_PLATE_WIDTH);
				D3DXMatrixMultiply( &dxMat, &dxMat, &TempMat );
			}
			// Bound object to certain terrain plates
			for(yn=pGO->posY;yn<(pGO->posY+pGO->sizeY);yn++)
				for(xn=pGO->posX;xn<(pGO->posX+pGO->sizeX);xn++){
					switch(rot % 4){
						case DIRECTION_N:
							pMesh->AddMapPosition(px+xn-shift,pz+yn-shift);
							break;
						case DIRECTION_E:
							pMesh->AddMapPosition(px+yn-shift,pz+(size-xn-1)-shift);
							break;
						case DIRECTION_S:
							pMesh->AddMapPosition(px+(size-xn-1)-shift,pz+(size-yn-1)-shift);
							break;
						case DIRECTION_W:
							pMesh->AddMapPosition(px+(size-yn-1)-shift,pz+xn-shift);
							break;
					}

				}

			//Scene->Map.BoundObject(pMesh);
			if(pGO->isStatic)
			{
				vModObjects.push_back( CGameMod::OBJECT(false,(int)Scene->StaticObjects.size()) );
				Scene->StaticObjects.push_back(pMesh);
				if ( Configuration.ShadowComplexity == SC_COMPLEX ) pMesh->InitShadowVolumes( &pSunLight->Position );
			}
			else
			{
				vModObjects.push_back( CGameMod::OBJECT(true,(int)Scene->DynamicObjects.size()) );
				Scene->DynamicObjects.push_back(pMesh);
				if (pClient) pClient->GetvObjectsMesh()->push_back(pMesh);				
			}
			
			//TODO: SHADOW VOLUMES ?
			//pMesh->InitShadowVolumes( &pSunLight->Position );
			
			// SET LIGHTS
			for(k = 0 ; k < pGO->pvLightObjectIDs->size(); k++)
			{
				pomObjID = pGO->pvLightObjectIDs->at(k);
				pLO = ResManager->GetLightObject(pomObjID);

				if ( pLO->type == LT_LAMP )
				{
					pLight = new graphic::CLight;
					if ( !pLight ) ERRORMSG( ERROUTOFMEMORY, "MapInit::MapInit()", "" );

					lx = pGO->scaleX*pLO->x;
					ly = pGO->scaleY*pLO->y,
					lz = pGO->scaleZ*pLO->z;
					rx = pLO->directionX;
					ry = pLO->directionY;
					rz = pLO->directionZ;
					
					vecDir= D3DXVECTOR3(rx,ry,rz);
					vecPos= D3DXVECTOR3(lx,ly,lz);

					pLight->Attenuation0 =pLO->Attenuation0;
					pLight->Attenuation1 =pLO->Attenuation1;
					pLight->Attenuation2 =pLO->Attenuation2;
					graphic::CLight::ComputeD3DColorValue(&(pLight->Diffuse),pLO->Diffuse);
					graphic::CLight::ComputeD3DColorValue(&(pLight->Ambient),pLO->Ambient);
					graphic::CLight::ComputeD3DColorValue(&(pLight->Specular),pLO->Specular);
					pLight->Falloff =pLO->Falloff;
					pLight->Phi =pLO->Phi;
					pLight->Range =pLO->Range;
					pLight->Theta =pLO->Theta;
					pLight->Type =pLO->type;
					pLight->SetRelativeSystem(&dxMat,&vecPos,&vecDir);
					pLight->SetMapPositionAsWorldPosition();
					pLight->pEmitor=pMesh;
					pTP = Scene->Map.GetPlate( pLight->MapPosition.PosX, pLight->MapPosition.PosZ );
					pTP -> SetLamp( pLight );
					// Set light map to certain plate
					if (pLO->ridLightMap)
					{
						pTO = ResManager->GetTexture(pLO->ridLightMap);
						pLight->LightMap = pTO->texture;
						pTP->SetLightMap( pTO->texture);
					}
					else
						pLight->LightMap = 0;
					// Set light to mesh
					pMesh->SetLight(pLight);

					Scene->Lights.push_back( pLight );
				}
				else if ( pLO->type == LT_FOGLAMP ) // light represented by sprite object
				{
					if ( !Scene->SkySystem.IsDay() )
					{
						pBillboard = new graphic::CGrObjectBillboard;
						if ( !pBillboard ) ERRORMSG( ERROUTOFMEMORY, "MapInit::MapInit()", "" );
						hr = pBillboard->Init( false, pLO->ridLightMap, 0 );
						if ( hr ) ERRORMSG( ERRGENERIC, "MapInit::MapInit()", "Light glow initialization failed." );

						//pBillboard->SetPosition( pLight->Position );
						pBillboard->SetPosition(pLO->x, pLO->y, pLO->z);
						pBillboard->SetScaling( pLO->Scale );
						Scene->TransparentObjects.push_back( pBillboard );
						pMesh->Billboards.push_back(pBillboard);
						pMesh->BillboardsObjects.push_back(pLO);
					}
				}

				//graphic::CGrObjectMesh * pLightCone = new graphic::CGrObjectMesh;
				//if ( !pLightCone ) ERRORMSG( ERROUTOFMEMORY, "Client::MapInit()", "" );
				//pLightCone->Init( true, "light_cone.modelDef", 0 ); 
				//if ( hr ) ERRORMSG( ERRGENERIC, "Client::MapInit()", "Light glow initialization failed." );
			
				//if ( !pClient ) 
				//{
				//	pLightCone->InitialWorldPlacement( pLight->Position, D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 3, 3, 5 ) );
				//	pLightCone->SetMapPositionAsWorldPosition();
				//}
				
				//Scene->TransparentObjects.push_back( pLightCone );
			}

		}	
		////////////////////////////////////////////////////
		// LOADING ROADS POSITION
		////////////////////////////////////////////////////


		for(j=0; j < pMO->pvRoadIDs->size();j++){ //LOADING ROADS
			pomObjID = pMO->pvRoadIDs->at(j);
			pGO = ResManager->GetGameObject(pomObjID);
			
			for(j1=0;j1<pGO->sizeY;j1++)	
				for(i1=0;i1<pGO->sizeX;i1++){
					id = j1*pGO->sizeX+i1;
					if( !(pGO->pvTextureFileNames->size() > 0) ){
						ERRORMSG(ERRGENERIC,"["+*(pMO->sName)+"] MapInit()","No graphic texture");
					}
					if(pGO->pvTextureFileNames->size() <= id) {
						id=(UINT) pGO->pvTextureFileNames->size()-1;
					}
				
					if(pGO->pvTextureFileNames->at(id) && pGO->pvTextureFileNames->at(id)!="")
					
					switch(rot % 4){	//rotation and move
						case DIRECTION_N:
							dpx = px+ pGO->posX  + i1;	 
							dpy = pz+pGO->posY + j1;			
							pTP = Scene->Map.GetPlate(dpx-shift,dpy-shift );
							hr=pTP->LoadRoadTexture(pGO->pvTextureFileNames->at(id));
							if(hr){  ERRORMSG(ERRFILECORRUPTED,"MapInit::MapInit()","Road Texture wasn't load");} 
							pTP->SetRoadTextureRotation(pGO->rotationY+0);
							// add this "surface" object into the list being sent to GameMod
							vModObjects.push_back( CGameMod::OBJECT( dpx-shift, dpy-shift ) );
							break;

						case DIRECTION_E:
							dpx = px + pGO->posY  + j1;
							dpy = pz + size-1 - pGO->posX - i1;
							pTP = Scene->Map.GetPlate(dpx-shift,dpy-shift);
							hr=pTP->LoadRoadTexture(pGO->pvTextureFileNames->at(id));
							if(hr){  ERRORMSG(ERRFILECORRUPTED,"MapInit::MapInit()","Road Texture wasn't load");} 
							pTP->SetRoadTextureRotation(pGO->rotationY+DEG90);														
							// add this "surface" object into the list being sent to GameMod
							vModObjects.push_back( CGameMod::OBJECT( dpx-shift, dpy-shift ) );
							break;

						case DIRECTION_S:
							dpx = px + size-1 - pGO->posX - i1;
							dpy = pz + size-1 - pGO->posY - j1;
							pTP = Scene->Map.GetPlate(dpx-shift,dpy-shift);					
							hr=pTP->LoadRoadTexture(pGO->pvTextureFileNames->at(id));
							if(hr){ ERRORMSG(ERRFILECORRUPTED,"MapInit::MapInit()","Road Texture wasn't load");} 
							pTP->SetRoadTextureRotation(pGO->rotationY+DEG180);
							// add this "surface" object into the list being sent to GameMod
							vModObjects.push_back( CGameMod::OBJECT( dpx-shift, dpy-shift ) );
							break;
						
						case DIRECTION_W:	
							dpx = px + size-1 - pGO->posY - j1;
							dpy = pz + pGO->posX + i1;
							pTP = Scene->Map.GetPlate(dpx-shift,dpy-shift);
							hr=pTP->LoadRoadTexture(pGO->pvTextureFileNames->at(id));
							if(hr){  ERRORMSG(ERRFILECORRUPTED,"MapInit::MapInit()","Road Texture wasn't load");} 
							pTP->SetRoadTextureRotation(pGO->rotationY-DEG90);
							// add this "surface" object into the list being sent to GameMod
							vModObjects.push_back( CGameMod::OBJECT( dpx-shift, dpy-shift ) );
							break;
					}
				}
		}  // loading roads

		
		////////////////////////////////////////////////////
		// PROCESSING CUSTOM PROPERTIES
		////////////////////////////////////////////////////
		
		// if this object is a special one, process it by actual game MOD
		if ( resMap->pbPlacedObjectSpecial[i] )
		{
			int		iModInd = 0;
			for ( UINT j = 0; j < resMap->pPlacedObjectData[i].size(); j++ )
				if ( resMap->pPlacedObjectData[i][j].sModID == Client->GetGameMOD()->GetModulePrimaryID().MakeUpper() )
					iModInd = j;
			
			Client->GetGameMOD()->ClientInitObject( &vModObjects, &(resMap->pPlacedObjectData[i][iModInd].vProperties) );
		}

	}

	Scene->Exists = TRUE;

	return hr;
};


//////////////////////////////////////////////////////////////////
//
// loads all grass items and sets it to appropriate position in
//   a scene
//
//////////////////////////////////////////////////////////////////
HRESULT InitGrass( graphic::CScene * Scene )
{
	resManNS::__Terrain *	resTerrain;
	HRESULT					hr;


	hr = Scene->InitGrassMap( Configuration.GrassLODCount );
	if ( hr ) ERRORMSG( ERRGENERIC, "MapInit::InitGrass()", "Unsuccessful grass initialization." );

	resTerrain = Scene->Map.GetTerrainResource();

	for ( int iz = (int) resTerrain->uiSizeY - 1; iz >= 0; iz-- )
	{
		for ( UINT ix = 0; ix < resTerrain->uiSizeX; ix++ )
		{
			int index = resTerrain->uiSizeX * iz + ix;

			if ( resTerrain->pTerrainPlateInfo[index].uiGrassIndex ) // non-zero indicates field with grass
			{
				int grIndex = resTerrain->pTerrainPlateInfo[index].uiGrassIndex - 1;
				
				hr = Scene->AddGrassToField( CONFIG_MODELSPATH + CAtlString( resTerrain->pGrassFileNames[grIndex] ), ix, iz );
				if ( hr ) ERRORMSG( ERRGENERIC, "MapInit::InitGrass()", "Unsuccessful grass initialization." );
			}
		}
	}

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void FillInitWheelStruct(resManNS::GameObject * wheelObject, physics::InitWheel * initWheel, physics::InitCar * initCar)
{
	initWheel->model = *(wheelObject->psModel);
	initWheel->position = dVector(wheelObject->x, wheelObject->y, wheelObject->z);
	initWheel->scale = initCar->scale;
	initWheel->rotated = wheelObject->bRotated;
	initWheel->powered = wheelObject->bPowered;
	initWheel->mass = wheelObject->mass;
	initWheel->suspensionShock = wheelObject->suspensionShock;
	initWheel->suspensionSpring = wheelObject->suspensionSpring;
	initWheel->suspensionLength = wheelObject->suspensionLength;
	initWheel->carMassOnWheel = initCar->mass / initCar->wheels.size();
	initWheel->suspensionCoef = wheelObject->suspensionCoef;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void FillInitEngineStruct(resManNS::EngineObject * engineObject, physics::InitEngine * initEngine)
{
	initEngine->numGears = engineObject->gear_count;
	initEngine->gears = new float[MAX_GEARS_COUNT];
	initEngine->gears[0] = engineObject->gear_r;
	initEngine->gears[1] = engineObject->gear_1;
	initEngine->gears[2] = engineObject->gear_2;
	initEngine->gears[3] = engineObject->gear_3;
	initEngine->gears[4] = engineObject->gear_4;
	initEngine->gears[5] = engineObject->gear_5;
	initEngine->gears[6] = engineObject->gear_6;
	initEngine->diff_ratio = engineObject->diff_ratio;
	initEngine->efficiency = engineObject->efficiency;

	initEngine->numCurve = engineObject->curve_points_count;
	initEngine->torques = new float[MAX_CURVE_POINTS_COUNT];
	initEngine->rpms = new float[MAX_CURVE_POINTS_COUNT];
	initEngine->rpms[0] = engineObject->curve_point_1_rpm;
	initEngine->torques[0] = engineObject->curve_point_1_torque;
	initEngine->rpms[1] = engineObject->curve_point_2_rpm;
	initEngine->torques[1] = engineObject->curve_point_2_torque;
	initEngine->rpms[2] = engineObject->curve_point_3_rpm;
	initEngine->torques[2] = engineObject->curve_point_3_torque;
	initEngine->rpms[3] = engineObject->curve_point_4_rpm;
	initEngine->torques[3] = engineObject->curve_point_4_torque;
	initEngine->rpms[4] = engineObject->curve_point_5_rpm;
	initEngine->torques[4] = engineObject->curve_point_5_torque;
	initEngine->rpms[5] = engineObject->curve_point_6_rpm;
	initEngine->torques[5] = engineObject->curve_point_6_torque;
	initEngine->rpms[6] = engineObject->curve_point_7_rpm;
	initEngine->torques[6] = engineObject->curve_point_7_torque;
	initEngine->rpms[7] = engineObject->curve_point_8_rpm;
	initEngine->torques[7] = engineObject->curve_point_8_torque;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void FillInitCarStruct(resManNS::CarObject * CarObj, physics::InitCar * initCar, resManNS::CResourceManager * ResManager)
{
	resManNS::GameObject * GameObj;
	resManNS::EngineObject * EngineObj;
	physics::InitWheel initWheel;
	UINT i;

	GameObj = ResManager->GetGameObject(CarObj->ridCarObject);

	initCar->model = *(GameObj->psModel);
	initCar->cog = dVector(GameObj->cogX, GameObj->cogY, GameObj->cogZ);
	initCar->scale = dVector(GameObj->scaleX, GameObj->scaleY, GameObj->scaleZ);
	initCar->rotation = dVector(GameObj->rotationX, GameObj->rotationY, GameObj->rotationZ);
	initCar->mass = GameObj->mass;
	initCar->moi = dVector(GameObj->moiX, GameObj->moiY, GameObj->moiZ);
	initCar->material = GameObj->material;
	initCar->wheel_material = GameObj->wheel_material;

	for (i = 0; i < CarObj->pvWheelIDs->size(); i++)
	{
		GameObj = ResManager->GetGameObject(CarObj->pvWheelIDs->at(i) );
		FillInitWheelStruct( GameObj, &initWheel, initCar); 
		initCar->wheels.push_back(initWheel);
	}
	
	EngineObj = ResManager->GetEngineObject(CarObj->ridEngine);
	FillInitEngineStruct(EngineObj, &initCar->engine);
	initCar->shift_up = EngineObj->shift_up;
	initCar->shift_down = EngineObj->shift_down;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void FillInitObjectStruct(resManNS::GameObject * gameObject, physics::InitObject * initObject)
{
	initObject->model = *(gameObject->psModel);
	initObject->isStatic = gameObject->isStatic;
	initObject->cog = dVector(gameObject->cogX, gameObject->cogY, gameObject->cogZ);
	initObject->scale = dVector(gameObject->scaleX, gameObject->scaleY, gameObject->scaleZ);
	initObject->rotation = dVector(gameObject->rotationX, gameObject->rotationY, gameObject->rotationZ);
	initObject->mass = gameObject->mass;
	initObject->moi = dVector(gameObject->moiX, gameObject->moiY, gameObject->moiZ);
	initObject->material = gameObject->material;
};
