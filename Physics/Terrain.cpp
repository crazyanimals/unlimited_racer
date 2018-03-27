#include "Terrain.h"
#include "Physics.h"

using namespace physics;

/////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
CTerrain::~CTerrain()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTerrain::Release()
{
	SAFE_DELETE_ARRAY(physicsSurfaces);
	
	SAFE_DELETE_ARRAY(physicsPlates);

	if(worldBoundingBox)
	{
		NewtonDestroyBody(nWorld, worldBoundingBox); worldBoundingBox = NULL;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTerrain::Init(NewtonWorld * world, CResources * resources)
{
	this->resources = resources;
	nWorld = world;
	physicsSurfaces = NULL;
	physicsPlates = NULL;
	materialsMap = NULL;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CTerrain::Load(CAtlString terrainName, physics::CPhysics *physics, int bboxID)
{
	try
	{
		HRESULT hr;
		resManNS::RESOURCEID resTerrainID;
		resManNS::Terrain * resTerrain;

		// load terrain resource
		resTerrainID = resources->rsm->LoadResource(terrainName, RES_Terrain);
		if(resTerrainID < 1) ERRORMSG(ERRGENERIC, "CTerrain::Load()", "Could not load terrain object.");
		resources->AddResource(resTerrainID);

		resTerrain = resources->rsm->GetTerrain(resTerrainID);
		if(!resTerrain) ERRORMSG(ERRNOTFOUND, "CTerrain::Load()", "Could not obtain terrain object.");

		// load physics surfaces (certain lod) to physicsSurfaces array
		if((hr = LoadPhysicsSurfaces(resTerrain)) != ERRNOERROR) ERRORMSG(hr, "CTerrain::Load()", "");

		sizeX = resTerrain->uiSizeX;
		sizeY = resTerrain->uiSizeY;
		mmSizeX = PHYSICAL_TEXTURE_SIZE*sizeX;
		mmSizeY = PHYSICAL_TEXTURE_SIZE*sizeY;
		platesSizeX = ((sizeX-1) / PLATE_SIZE)+1;
		platesSizeY = ((sizeY-1) / PLATE_SIZE)+1;

		physicsPlatesCount = platesSizeX*platesSizeY;
		SAFE_NEW_ARRAY(physicsPlates, CPhysicsPlate, physicsPlatesCount, "CTerrain::Load()");

		materialsMapSize = sizeX*sizeY*PHYSICAL_TEXTURE_SIZE*PHYSICAL_TEXTURE_SIZE;
		SAFE_NEW_ARRAY(materialsMap, BYTE, materialsMapSize, "CTerrain::Load()");

		if((hr = LoadPhysicsPlates(resTerrain, physics)) != ERRNOERROR) ERRORMSG(hr, "CTerrain::Load()", "");
		


		// create world bounding box and define newton world size
		CreateWorldBoundingBox(physics->materials.GetMaterialID (physics::MAT_WORLDBOX));
	}
	catch(...)
	{
		ERRORMSG(ERRGENERIC, "CTerrain::Load()", "Terrain loading failed (Newton)");
	}

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTerrain::Debug_Print(CAtlString fileName)
{
	FILE * fw;
	fopen_s(&fw, "C:\\"+fileName, "w");

	for(UINT y=mmSizeY; y>0; y--)
	{
		for(UINT x=0; x<mmSizeX; x++)
			if(MaterialsMap(x, y-1) == 6)
				fprintf_s(fw, "#");
			else
				fprintf_s(fw, "%u", MaterialsMap(x, y-1));
		fprintf_s(fw, "\n");
	}

	fclose(fw);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CTerrain::LoadPhysicsSurfaces(resManNS::Terrain * resTerrain)
{
	HRESULT hr;
	resManNS::RESOURCEID resGrPlateID;

	physicsSurfacesCount = resTerrain->uiGrPlateCount;
	SAFE_NEW_ARRAY(physicsSurfaces, CPhysicsSurface, physicsSurfacesCount, "CTerrain::LoadPhysicsPlates");

	for(UINT i=0; i<physicsSurfacesCount; i++)
	{
		if((resGrPlateID = resTerrain->pGrPlateIDs[i]) != -1)
		{
			if((hr = physicsSurfaces[i].Load(resources, resGrPlateID)) != ERRNOERROR)
				ERRORMSG(hr, "CTerrain::LoadPhysicsSurfaces()", "");
		}
	}

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTerrain::SetMaterialsMap(resManNS::PhysicalTexture * resPT, UINT posX, UINT posY, UINT rotation)
{
	for(UINT y=0; y<PHYSICAL_TEXTURE_SIZE; y++)
		for(UINT x=0; x<PHYSICAL_TEXTURE_SIZE; x++)
			switch(rotation)
			{
			case DIRECTION_N:
				MaterialsMap(posX+x, posY+y) = resPT->pPhysicalTexture[x][y] > -1 ? resPT->pPhysicalTexture[x][y] : 0;
				break;
			case DIRECTION_E:
				MaterialsMap(posX+x, posY+y) = resPT->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-y][x] > -1 ? resPT->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-y][x] : 0;				
				break;
			case DIRECTION_S:
				MaterialsMap(posX+x, posY+y) = resPT->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-x][PHYSICAL_TEXTURE_SIZE-1-y] > -1 ? resPT->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-x][PHYSICAL_TEXTURE_SIZE-1-y] : 0;
				break;
			case DIRECTION_W:
				MaterialsMap(posX+x, posY+y) = resPT->pPhysicalTexture[y][PHYSICAL_TEXTURE_SIZE-1-x] > -1 ? resPT->pPhysicalTexture[y][PHYSICAL_TEXTURE_SIZE-1-x] : 0;
				break;
			}	
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CTerrain::LoadPhysicsPlates(resManNS::Terrain * resTerrain, CPhysics * physics)
{
	UINT index;
	UINT phPlateSizeX, phPlateSizeY;
	PhysicsPlateData phPlateData;

	for(UINT platesY=0; platesY<platesSizeY; platesY++)
		for(UINT platesX=0; platesX<platesSizeX; platesX++)
		{
			index = platesSizeX*platesY + platesX;
			phPlateSizeX = (platesX == platesSizeX-1 ? sizeX - PLATE_SIZE*platesX : PLATE_SIZE);
			phPlateSizeY = (platesY == platesSizeY-1 ? sizeY - PLATE_SIZE*platesY : PLATE_SIZE);

			// construction of physics_plate begins
			physicsPlates[index].Begin(physics, platesX, platesY, phPlateSizeX, phPlateSizeY);

			phPlateData.index = index;
			phPlateData.sizeX = phPlateSizeX;
			phPlateData.sizeY = phPlateSizeY;
			phPlateData.platesX = platesX;
			phPlateData.platesY = platesY;

			LoadPhysicsPlate(resTerrain, &phPlateData);

			// finish construction of physics_plate
			physicsPlates[index].End();
		}

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CTerrain::LoadPhysicsPlate(resManNS::Terrain * resTerrain, PhysicsPlateData * phPlateData)
{
	UINT posX, posY;
	UINT phTexturePosX, phTexturePosY;
	UINT index;
	resManNS::TerrainPlateInfo * resPlateInfo;
	resManNS::RESOURCEID resGrPlateID;
	resManNS::GrPlate * resGrPlate;
	resManNS::RESOURCEID resPhysicalTextureID;
	resManNS::PhysicalTexture * resPhysicalTexture;
	CPhysicsSurface * phSurface;
	UINT rotation;
	dVector position;

	for(UINT plateY=0; plateY<phPlateData->sizeY; plateY++)
		for(UINT plateX=0; plateX<phPlateData->sizeX; plateX++)
		{
			posY = phPlateData->platesY*PLATE_SIZE + plateY;
			posX = phPlateData->platesX*PLATE_SIZE + plateX;
			index = sizeX*(posY) + (posX);

			resPlateInfo = &resTerrain->pTerrainPlateInfo[index];

			resGrPlateID = resTerrain->pGrPlateIDs[resPlateInfo->uiGrPlateIndex];
			resGrPlate = resources->rsm->GetGrPlate(resGrPlateID);
			if(!resGrPlate)	ERRORMSG(ERRNOTFOUND, "CTerrrain::Load()", "Could not obtain grplate object.");

			resPhysicalTextureID = resTerrain->pPhysicalTextureIDs[resPlateInfo->uiPhysicalTextureIndex];
			resPhysicalTexture = resources->rsm->GetPhysicalTexture(resPhysicalTextureID);
			if(!resPhysicalTexture) ERRORMSG(ERRNOTFOUND, "CTerrain::Load()", "Could not obtain physicaltexture object.");

			phSurface = &physicsSurfaces[resPlateInfo->uiGrPlateIndex];
			
			rotation = (resPlateInfo->uiPlateRotation % 4);

			position = dVector(
				TERRAIN_PLATE_WIDTH / 2 + TERRAIN_PLATE_WIDTH * posX,
				resPlateInfo->uiHeight * TERRAIN_PLATE_HEIGHT,
				TERRAIN_PLATE_WIDTH / 2 + TERRAIN_PLATE_WIDTH * posY);

			if(resGrPlate->bPlanar)
			{
				// add flat physics_plate
				physicsPlates[phPlateData->index].AddPlanar(phSurface, rotation, position);
			}
			else
			{
				// add other physics_plates
				physicsPlates[phPlateData->index].AddNonPlanar(phSurface, rotation, position);
			}

			// set material map for plate at _posX, _posY (terrain coords)
			phTexturePosX = PHYSICAL_TEXTURE_SIZE*posX;
			phTexturePosY = PHYSICAL_TEXTURE_SIZE*posY;
			SetMaterialsMap(resPhysicalTexture, phTexturePosX, phTexturePosY, rotation);
		}

	//Debug_Print("map_empty");

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTerrain::CreateWorldBoundingBox(int bboxID)
{
	float left = WORLD_LEFT;
	float front = WORLD_FRONT;
	float bottom = WORLD_BOTTOM-1; // because of raycast function
	float right = sizeX*TERRAIN_PLATE_WIDTH_M;
	float back = sizeY*TERRAIN_PLATE_WIDTH_M;
	float top = WORLD_TOP;

	NewtonCollision * collision = NewtonCreateTreeCollision(nWorld, NULL);
	NewtonTreeCollisionBeginBuild(collision);

	// bottom
	CreateWorldBoundingBoxFace(collision, dVector(left, bottom, front), dVector(right, bottom, front), dVector(left, bottom, back));
	CreateWorldBoundingBoxFace(collision, dVector(right, bottom, front), dVector(right, bottom, back), dVector(left, bottom, back));
	// front
	CreateWorldBoundingBoxFace(collision, dVector(left, bottom, front), dVector(right, bottom, front), dVector(left, top, front));
	CreateWorldBoundingBoxFace(collision, dVector(right, bottom, front), dVector(right, top, front), dVector(left, top, front));
	// right
	CreateWorldBoundingBoxFace(collision, dVector(right, bottom, front), dVector(right, bottom, back), dVector(right, top, front));
	CreateWorldBoundingBoxFace(collision, dVector(right, bottom, back), dVector(right, top, back), dVector(right, top, front));
	// back
	CreateWorldBoundingBoxFace(collision, dVector(right, bottom, back), dVector(left, bottom, back), dVector(right, top, back));
	CreateWorldBoundingBoxFace(collision, dVector(left, bottom, back), dVector(left, top, back), dVector(right, top, back));
	// left
	CreateWorldBoundingBoxFace(collision, dVector(left, bottom, back), dVector(left, bottom, front), dVector(left, top, back));
	CreateWorldBoundingBoxFace(collision, dVector(left, bottom, front), dVector(left, top, front), dVector(left, top, back));
	// top
	CreateWorldBoundingBoxFace(collision, dVector(left, top, front), dVector(right, top, front), dVector(right, top, back));
	CreateWorldBoundingBoxFace(collision, dVector(right, top, back), dVector(left, top, back), dVector(left, top, front));

	NewtonTreeCollisionEndBuild(collision, 0);
	worldBoundingBox = NewtonCreateBody(nWorld, collision);
	NewtonReleaseCollision(nWorld, collision);

	userData.dataType = PHYSICS_STATIC_OBJECT;
	//userData.dataType = PHYSICS_WORLD_BOUNDING_BOX;
	userData.material0 = 8;
    userData.data = this;

    NewtonBodySetUserData(worldBoundingBox, &userData);

	NewtonBodySetMaterialGroupID(worldBoundingBox, bboxID);

	dVector minBox(left-10.0f, bottom-10.0f, front-10.0f);
	dVector maxBox(right+10.0f, top+10.0f, back+10.0f);
	NewtonSetWorldSize(nWorld, &minBox.m_x, &maxBox.m_x);
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTerrain::CreateWorldBoundingBoxFace(NewtonCollision * collision, dVector p0, dVector p1, dVector p2)
{
	dVector points[3];

	points[0] = p0;
	points[1] = p1;
	points[2] = p2;

	NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), 1);
};

// This code is no longer used
#if 0
//////////////////////////////////////////////////////////////////////////////////////////////
//
//for(UINT iy=0; iy<sizeY; iy++)
//	for(UINT ix=0; ix<sizeX; ix++)
//	{
//		UINT _x = PHYSICAL_TEXTURE_SIZE*ix;
//		UINT _y = PHYSICAL_TEXTURE_SIZE*iy;
//		
//		for(UINT iiy=0; iiy<PHYSICAL_TEXTURE_SIZE; iiy++)
//			for(UINT iix=0; iix<PHYSICAL_TEXTURE_SIZE; iix++)
//				switch(resPlateInfo->uiTextureRotation % 4)
//				{
//				case 0:
//					MaterialsMap(_y+iiy, _x+iix) = resPhysicalTexture->pPhysicalTexture[iix][iiy];
//					break;
//				case 1:
//					MaterialsMap(_y+iiy, _x+iix) = resPhysicalTexture->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-iiy][iix];
//					break;
//				case 2:
//					MaterialsMap(_y+iiy, _x+iix) = resPhysicalTexture->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-iix][PHYSICAL_TEXTURE_SIZE-1-iiy];
//					break;
//				case 3:
//					MaterialsMap(_y+iiy, _x+iix) = resPhysicalTexture->pPhysicalTexture[iiy][PHYSICAL_TEXTURE_SIZE-1-iix];
//					break;
//				}			
//	}
//
//FILE * fw;
//fopen_s(&fw, "C:\\map_empty", "w");
//// pouze kousek mapy !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//for(int y=mmSizeY-1; y>-1; y--)
//{
//	for(int x=0; x<mmSizeX; x++)
//		fprintf_s(fw, "%u", physics->terrain.materialsMap[mmSizeX*y+x]);
//	fprintf_s(fw, "\n");
//}
//fclose(fw);
//
//for(UINT platesY=0; platesY<platesSizeY; platesY++)
//	for(UINT platesX=0; platesX<platesSizeX; platesX++)
//	{
//		UINT index = platesSizeX*platesY + platesX;
//		UINT physicsPlateSizeX = (platesX == platesSizeX-1 ? sizeX - PLATE_SIZE*platesX : PLATE_SIZE);
//		UINT physicsPlateSizeY = (platesY == platesSizeY-1 ? sizeY - PLATE_SIZE*platesY : PLATE_SIZE);
//
//		// construction of physics_plate begins
//		physicsPlates[index].Begin(physics, platesX, platesY, physicsPlateSizeX, physicsPlateSizeY);
//
//		for(UINT plateY=0; plateY<physicsPlateSizeY; plateY++)
//			for(UINT plateX=0; plateX<physicsPlateSizeX; plateX++)
//			{
//				UINT posY = platesY*PLATE_SIZE + plateY;
//				UINT posX = platesX*PLATE_SIZE + plateX;
//				UINT index0 = sizeX*(posY) + (posX);
//
//				resPlateInfo = &resTerrain->pTerrainPlateInfo[index0];
//
//				resGrPlateID = resTerrain->pGrPlateIDs[resPlateInfo->uiGrPlateIndex];
//				resGrPlate = resources->rsm->GetGrPlate(resGrPlateID);
//				if(!resGrPlate)	ERRORMSG(ERRNOTFOUND, "CTerrrain::Load()", "Could not obtain grplate object.");
//
//				resPhysicalTextureID = resTerrain->pPhysicalTextureIDs[resPlateInfo->uiPhysicalTextureIndex];
//				resPhysicalTexture = resources->rsm->GetPhysicalTexture(resPhysicalTextureID);
//				if(!resPhysicalTexture) ERRORMSG(ERRNOTFOUND, "CTerrain::Load()", "Could not obtain physicaltexture object.");
//
//				CPhysicsSurface * phSurface = &physicsSurfaces[resPlateInfo->uiGrPlateIndex];
//				
//				UINT rotation = (resPlateInfo->uiPlateRotation % 4);
//				float height = resPlateInfo->uiHeight * TERRAIN_PLATE_HEIGHT;
//				float xposition = TERRAIN_PLATE_WIDTH / 2 + TERRAIN_PLATE_WIDTH * posX;
//				float yposition = TERRAIN_PLATE_WIDTH / 2 + TERRAIN_PLATE_WIDTH * posY;
//
//				if(resGrPlate->bPlanar)
//				{
//					// add flat physics_plate
//					phSurface->Copy(points, 0, 0, 0, rotation, dVector(xposition, height, yposition));
//					phSurface->Copy(points, 1, phSurface->segsCountX, 0, rotation, dVector(xposition, height, yposition));
//					phSurface->Copy(points, 2, phSurface->segsCountX, phSurface->segsCountZ, rotation, dVector(xposition, height, yposition));
//					phSurface->Copy(points, 3, 0, phSurface->segsCountZ, rotation, dVector(xposition, height, yposition));
//
//					physicsPlates[index].Add(4, points);
//				}
//				else
//				{
//					// add other physics_plates
//					for(UINT sz=0; sz<phSurface->segsCountZ; sz++)
//						for(UINT sx=0; sx<phSurface->segsCountX; sx++)
//						{
//							phSurface->Copy(points, 0, sx, sz, rotation, dVector(xposition, height, yposition)); 
//							phSurface->Copy(points, 1, sx+1, sz, rotation, dVector(xposition, height, yposition));
//							phSurface->Copy(points, 2, sx, sz+1, rotation, dVector(xposition, height, yposition));
//
//							physicsPlates[index].Add(3, points);
//
//							phSurface->Copy(points, 0, sx+1, sz, rotation, dVector(xposition, height, yposition));
//							phSurface->Copy(points, 1, sx+1, sz+1, rotation, dVector(xposition, height, yposition));
//							phSurface->Copy(points, 2, sx, sz+1, rotation, dVector(xposition, height, yposition));
//
//							physicsPlates[index].Add(3, points);
//						}
//				}
//
//				// set material map for plate at _posX, _posY (terrain coords)
//				UINT _posX = PHYSICAL_TEXTURE_SIZE*posX;
//				UINT _posY = PHYSICAL_TEXTURE_SIZE*posY;
//				SetMaterialsMap(resPhysicalTexture, _posX, _posY, rotation);
//			}
//
//			// finish construction of physics_plate
//			physicsPlates[index].End();
//	}
//
//////////////////////////////////////////////////////////////////////////////////////////////
//
//CPlateCollision::CPlateCollision()
//{
//	collision = NULL;
//};
//
//CPlateCollision::~CPlateCollision()
//{
//	Release();
//};
//
//void CPlateCollision::Create(NewtonWorld * world, int segsx, int segsz, _VERTEX * vert, UINT bPlanar)
//{
//	this->world = world;
//
//	dVector points[4];
//
//	collision = NewtonCreateTreeCollision(world, NULL);
//
//	NewtonTreeCollisionBeginBuild(collision);
//
//	if(bPlanar)
//	{
//		Copy(points, 0, vert, 0);
//		Copy(points, 1, vert, segsx);
//		Copy(points, 2, vert, (segsx+1)*(segsz+1)-1);
//		Copy(points, 3, vert, segsz*(segsx+1));
//
//		NewtonTreeCollisionAddFace(collision, 4, &points[0].m_x, sizeof(dVector), 1);
//		
//		NewtonTreeCollisionEndBuild(collision, 1);
//
//		//Copy(points, 0, vert, 0);
//		//Copy(points, 1, vert, segsx);
//		//Copy(points, 2, vert, segsz*(segsx+1));
//
//		//NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), 1);
//
//		//Copy(points, 0, vert, segsx);
//		//Copy(points, 1, vert, (segsx+1)*(segsz+1)-1);
//		//Copy(points, 2, vert, segsz*(segsx+1));
//
//		//NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), 1);
//
////#define BOX
//#ifdef BOX
//		Copy(points, 0, vert, 0);
//		Copy1(points, 1, vert, 0);
//		Copy1(points, 2, vert, segsx);
//		Copy(points, 3, vert, segsx);
//
//		NewtonTreeCollisionAddFace(collision, 4, &points[0].m_x, sizeof(dVector), 1);
//
//		Copy(points, 0, vert, segsx);
//		Copy1(points, 1, vert, segsx);
//		Copy1(points, 2, vert, (segsx+1)*(segsz+1)-1);
//		Copy(points, 3, vert, (segsx+1)*(segsz+1)-1);
//
//		NewtonTreeCollisionAddFace(collision, 4, &points[0].m_x, sizeof(dVector), 1);
//
//		Copy(points, 0, vert, (segsx+1)*(segsz+1)-1);
//		Copy1(points, 1, vert, (segsx+1)*(segsz+1)-1);
//		Copy1(points, 2, vert, segsz*(segsx+1));
//		Copy(points, 3, vert, segsz*(segsx+1));
//
//		NewtonTreeCollisionAddFace(collision, 4, &points[0].m_x, sizeof(dVector), 1);
//
//		Copy(points, 0, vert, segsz*(segsx+1));
//		Copy1(points, 1, vert, segsz*(segsx+1));
//		Copy1(points, 2, vert, 0);
//		Copy(points, 3, vert, 0);
//
//		NewtonTreeCollisionAddFace(collision, 4, &points[0].m_x, sizeof(dVector), 1);
//
//		Copy1(points, 0, vert, segsz*(segsx+1));
//		Copy1(points, 1, vert, (segsx+1)*(segsz+1)-1);
//		Copy1(points, 2, vert, segsx);
//		Copy1(points, 3, vert, 0);
//
//		NewtonTreeCollisionAddFace(collision, 4, &points[0].m_x, sizeof(dVector), 1);
//#endif
//		//NewtonTreeCollisionEndBuild(collision, 0);
//
//		//Copy(points, 0, vert, 0);
//		//Copy(points, 1, vert, segsx);
//		//Copy(points, 2, vert, (segsz/2)*(segsx+1));
//
//		//NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), 1);
//
//		//Copy(points, 0, vert, segsx);
//		//Copy(points, 1, vert, (segsz/2)*(segsz+1)+segsx);
//		//Copy(points, 2, vert, (segsz/2)*(segsx+1));
//
//		//NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), 1);
//
//
//		//int field[16] = {2, 2, 2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1};
//
//		//for(int sz=0; sz<segsz; sz++)
//		//	for(int sx=0; sx<segsx; sx++)
//		//	{
//		//		Copy(points, 0, vert, sz*(segsx+1)+sx);
//		//		Copy(points, 1, vert, sz*(segsx+1)+(sx+1));
//		//		Copy(points, 2, vert, (sz+1)*(segsx+1)+sx);
//
//		//		NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), field[segsx*sz+sx]);
//
//		//		Copy(points, 0, vert, sz*(segsx+1)+(sx+1));
//		//		Copy(points, 1, vert, (sz+1)*(segsx+1)+(sx+1));
//		//		Copy(points, 2, vert, (sz+1)*(segsx+1)+sx);
//
//		//		NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), field[segsx*sz+sx]);
//		//	}
//
//		//NewtonTreeCollisionEndBuild(collision, 1);
//	}
//	else
//	{
//		for(int sz=0; sz<segsz; sz++)
//			for(int sx=0; sx<segsx; sx++)
//			{
//				//Copy(points, 0, vert, sz*(segsx+1)+sx);
//				//Copy(points, 1, vert, sz*(segsx+1)+(sx+1));
//				//Copy(points, 2, vert, (sz+1)*(segsx+1)+sx);
//				Copy(points, 0, vert, sz*(segsx+1)+sx);
//				Copy(points, 1, vert, (sz+1)*(segsx+1)+(sx+1));
//				Copy(points, 2, vert, (sz+1)*(segsx+1)+sx);
//
//				NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), 3);
//
//				//Copy(points, 0, vert, sz*(segsx+1)+(sx+1));
//				//Copy(points, 1, vert, (sz+1)*(segsx+1)+(sx+1));
//				//Copy(points, 2, vert, (sz+1)*(segsx+1)+sx);
//				Copy(points, 0, vert, sz*(segsx+1)+sx);
//				Copy(points, 1, vert, sz*(segsx+1)+(sx+1));
//				Copy(points, 2, vert, (sz+1)*(segsx+1)+(sx+1));
//
//				NewtonTreeCollisionAddFace(collision, 3, &points[0].m_x, sizeof(dVector), 3);
//			}
//		NewtonTreeCollisionEndBuild(collision, 0);
//	}
//
//	//NewtonTreeCollisionEndBuild(collision, 0);
//};
//
//void CPlateCollision::Copy(dVector * points, int point, _VERTEX * vert, int index)
//{
//	points[point].m_x = vert[index].x/ONE_METER;
//	points[point].m_y = vert[index].y/ONE_METER;
//	points[point].m_z = vert[index].z/ONE_METER;
//};
//
//void CPlateCollision::Copy1(dVector * points, int point, _VERTEX * vert, int index)
//{
//	points[point].m_x = vert[index].x/ONE_METER;
//	points[point].m_y = vert[index].y/ONE_METER - 2.0f;
//	points[point].m_z = vert[index].z/ONE_METER;
//};
//
//
//void CPlateCollision::Release()
//{
//	if(collision) 
//	{
//		NewtonReleaseCollision(world, collision);
//		collision = NULL;
//		world = NULL;
//	}
//};
//
//CPlate::~CPlate()
//{
//	Release();
//};
//
//void CPlate::Create(NewtonWorld * world, NewtonCollision * collision, int rotation, dVector * position, CPhysics * physics)
//{
//	this->physics = physics;
//	nBody = NewtonCreateBody(world, collision);
//
//	userData.dataType = PHYSICS_TERRAIN;
//	userData.data = this;
//
//	NewtonBodySetUserData(nBody, &userData);
//
//	//NewtonBodySetMaterialGroupID(nBody, physics->levelID);
//	NewtonBodySetMaterialGroupID(nBody, physics->materials.terrainID);
//
//	dMatrix matrix (GetIdentityMatrix());
//
//	switch(rotation)
//	{
//	case 0:
//		break;
//	case 1:
//		matrix = matrix * dgYawMatrix(0.5f*D3DX_PI);
//		break;
//	case 2:
//		matrix = matrix * dgYawMatrix(D3DX_PI);
//		break;
//	case 3:
//		matrix = matrix * dgYawMatrix(1.5f*D3DX_PI);
//		break;
//	}
//
//	matrix.m_posit = *position;
//
//	NewtonBodySetMatrix(nBody, &matrix[0][0]);
//};
//
//void CPlate::Release()
//{
//	if(nBody)
//	{
//		NewtonDestroyBody(NewtonBodyGetWorld(nBody), nBody);
//		nBody = NULL;
//		physics = NULL;
//	}
//};
//
//HRESULT CTerrain::LoadPlateCollisions(resManNS::Terrain * resTerrain)
//{
//	_VERTEX * vert;
//	HRESULT hr;
//
//	plateCollisionsCount = resTerrain->uiGrPlateCount;
//	SAFE_NEW_ARRAY(plateCollisions, CPlateCollision, plateCollisionsCount, "CTerrain::LoadPlateCollisions()");
//
//	resManNS::RESOURCEID resGrPlateID;
//	resManNS::GrPlate * resGrPlate;
//	resManNS::RESOURCEID resGrSurfaceID;
//	resManNS::GrSurface * resGrSurface;
//
//	for(int i=0; i<plateCollisionsCount; i++)
//	{
//		if((resGrPlateID = resTerrain->pGrPlateIDs[i]) != -1)
//		{
//			resGrPlate = resources->rsm->GetGrPlate(resGrPlateID);
//			if(!resGrPlate) ERRORMSG(ERRNOTFOUND, "CTerrain::LoadPlateCollisions()", "Could not obtain grplate object.");
//	        
//			resGrSurfaceID = resGrPlate->LoD[(TERRAIN_LOD_PHYSICS >= resGrPlate->uiLoDCount ? resGrPlate->uiLoDCount : TERRAIN_LOD_COUNT)];	
//			//resGrSurfaceID = resGrPlate->LoD[resGrPlate->uiLoDCount-4];
//			resGrSurface = resources->rsm->GetGrSurface(resGrSurfaceID);
//			if(!resGrSurface) ERRORMSG(ERRNOTFOUND, "CTerrain::LoadPlateCollisions()", "Could not obtain terrain surface data.");
//
//			if(hr = resGrSurface->pVertices->Lock(0, 0, (LPVOID *) &vert, 0))
//				ERRORMSG(hr, "CTerrain::LoadPlateCollisions", "Locking of vertex buffer failed.");
//
//			plateCollisions[i].Create(nWorld, resGrSurface->SegsCountX, resGrSurface->SegsCountZ, vert, resGrPlate->bPlanar);
//
//			if(hr = resGrSurface->pVertices->Unlock())
//				ERRORMSG(hr, "CTerrain::LoadPlateCollisions", "Unlocking of vertex buffer failed.");
//		}
//	}
//
//	return ERRNOERROR;
//};
//
//HRESULT CTerrain::Load(CAtlString terrain, CPhysics * physics)
//{
//	resManNS::RESOURCEID resTerrainID = resources->rsm->LoadResource(terrain, RES_Terrain);
//	if(resTerrainID < 1) ERRORMSG(ERRGENERIC, "CTerrain::Load()", "Could not load terrain object.");
//	resources->AddResource(resTerrainID);
//
//	resManNS::Terrain * resTerrain = resources->rsm->GetTerrain(resTerrainID);
//	if(!resTerrain) ERRORMSG(ERRNOTFOUND, "CTerrain::Load()", "Could not obtain terrain object.");
//
//	if(HRESULT hr = LoadPlateCollisions(resTerrain)) ERRORMSG(hr, "CTerrain::Load()", "");
//
//	resManNS::TerrainPlateInfo * resPlateInfo;
//	resManNS::RESOURCEID resGrPlateID;
//	resManNS::GrPlate * resGrPlate;
//	resManNS::RESOURCEID resPhysicalTextureID;
//	resManNS::PhysicalTexture * resPhysicalTexture;
//
//	sizeX = resTerrain->uiSizeX;
//	sizeY = resTerrain->uiSizeY;
//	mmSizeX = PHYSICAL_TEXTURE_SIZE*sizeX;
//	mmSizeY = PHYSICAL_TEXTURE_SIZE*sizeY;
//
//	platesCount = sizeX*sizeY;
//	SAFE_NEW_ARRAY(plates, CPlate, platesCount, "CTerrain::Load()");
//
//	materialsMapSize = sizeX*sizeY*PHYSICAL_TEXTURE_SIZE*PHYSICAL_TEXTURE_SIZE;
//	SAFE_NEW_ARRAY(materialsMap, BYTE, materialsMapSize, "CTerrain::Load()");
//
//	for(UINT iy=0; iy<sizeY; iy++)
//		for(UINT ix=0; ix<sizeX; ix++)
//		{
//			int index = sizeX*iy + ix;
//
//			resPlateInfo = &resTerrain->pTerrainPlateInfo[index];
//			
//			resGrPlateID = resTerrain->pGrPlateIDs[resPlateInfo->uiGrPlateIndex];
//			resGrPlate = resources->rsm->GetGrPlate(resGrPlateID);
//			if(!resGrPlate)	ERRORMSG(ERRNOTFOUND, "CTerrrain::Load()", "Could not obtain grplate object.");
//
//			resPhysicalTextureID = resTerrain->pPhysicalTextureIDs[resPlateInfo->uiPhysicalTextureIndex];
//			resPhysicalTexture = resources->rsm->GetPhysicalTexture(resPhysicalTextureID);
//			if(!resPhysicalTexture) ERRORMSG(ERRNOTFOUND, "CTerrain::Load()", "Could not obtain physicaltexture object.");
//
//			dVector position(TERRAIN_PLATE_WIDTH_M / 2 + TERRAIN_PLATE_WIDTH_M * ix,
//				resPlateInfo->uiHeight * TERRAIN_PLATE_HEIGHT_M,
//				TERRAIN_PLATE_WIDTH_M / 2 + TERRAIN_PLATE_WIDTH_M * iy,
//				1.0f);
//
//			plates[index].Create(nWorld, plateCollisions[resPlateInfo->uiGrPlateIndex].Collision(),
//				resPlateInfo->uiPlateRotation, &position, physics);
//
//			int _x = PHYSICAL_TEXTURE_SIZE*ix;
//			int _y = PHYSICAL_TEXTURE_SIZE*iy;
//			
//			for(int iiy=0; iiy<PHYSICAL_TEXTURE_SIZE; iiy++)
//				for(int iix=0; iix<PHYSICAL_TEXTURE_SIZE; iix++)
//					switch(resPlateInfo->uiTextureRotation % 4)
//					{
//					case 0:
//						MaterialsMap(_y+iiy, _x+iix) = resPhysicalTexture->pPhysicalTexture[iix][iiy];
//						break;
//					case 1:
//						MaterialsMap(_y+iiy, _x+iix) = resPhysicalTexture->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-iiy][iix];
//						break;
//					case 2:
//						MaterialsMap(_y+iiy, _x+iix) = resPhysicalTexture->pPhysicalTexture[PHYSICAL_TEXTURE_SIZE-1-iix][PHYSICAL_TEXTURE_SIZE-1-iiy];
//						break;
//					case 3:
//						MaterialsMap(_y+iiy, _x+iix) = resPhysicalTexture->pPhysicalTexture[iiy][PHYSICAL_TEXTURE_SIZE-1-iix];
//						break;
//					}			
//		}
//
//	SAFE_DELETE_ARRAY(plateCollisions);
//
//	CreateWorldBoundingBox();
//
//	//FILE * fw;
//	//fopen_s(&fw, "C:\\map_empty", "w");
//	//// pouze kousek mapy !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	//for(int y=mmSizeY-1; y>-1; y--)
//	//{
//	//	for(int x=0; x<mmSizeX; x++)
//	//		fprintf_s(fw, "%u", physics->terrain.materialsMap[mmSizeX*y+x]);
//	//	fprintf_s(fw, "\n");
//	//}
//	//fclose(fw);
//
//	return ERRNOERROR;
//};
//
//////////////////////////////////////////////////////////////////////////////////////////////

#endif