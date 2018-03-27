#pragma once

#include "stdafx.h"
#include "..\Graphic\Graphic.h"
#include "..\GraphicObjects\GrObjectTerrain.h"
#include "..\Globals\Configuration.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\GraphicObjects\Scene.h"
#include "..\GraphicObjects\GrObjectMesh.h"
#include "..\Physics\Physics.h"
#include "..\GameMods\GameMod.h"


class CClient;


//HRESULT SetCarPosition(graphic::CGrObjectMesh * mesh, UINT CarNumber, CResourceManager * ResManager, graphic::CScene * Scene, RESOURCEID MapID);
//HRESULT MapInit(LPCTSTR , graphic::CScene * , CResourceManager * ResManager, CClient * pClient=NULL);
//HRESULT MapPhInit(CStringA mapName, physics::CPhysics * Physics, CResourceManager * ResManager);//,std::vector< CPhCar *> * vPhCars);
//HRESULT loadCar(LPCTSTR carName, UINT CarNumber, CResourceManager * ResManager, graphic::CScene * Scene, RESOURCEID MapID);
//HRESULT loadCar(LPCTSTR carName, UINT CarNumber, CResourceManager * ResManager, CClient * client, RESOURCEID MapID,graphic::CScene * Scene=NULL);
//HRESULT loadPhCar(LPCTSTR carName, UINT CarNumber, physics::CPhysics * Physics, CResourceManager * ResManager, RESOURCEID MapID);

HRESULT SetCarPosition(graphic::CGrObjectMesh * mesh, UINT CarNumber, resManNS::CResourceManager * ResManager, graphic::CScene * Scene, resManNS::RESOURCEID MapID);
HRESULT MapInit(LPCTSTR , 
				graphic::CScene * , 
				resManNS::CResourceManager * ResManager, 
				void (CALLBACK *ProgressCallback) (CString),
				CClient * pClient=NULL);
HRESULT MapPhInit(CStringA mapName, 
				  physics::CPhysics * Physics, 
				  resManNS::CResourceManager * ResManager,
				  void (CALLBACK *ProgressCallback) (CString),
				  CGameMod * pGameMod );//,std::vector< CPhCar *> * vPhCars);
//HRESULT loadCar(LPCTSTR carName, UINT CarNumber, CResourceManager * ResManager, graphic::CScene * Scene, resManNS::RESOURCEID MapID);
HRESULT loadCar(LPCTSTR carName, 
				UINT CarNumber, 
				int CarIndex, 
				resManNS::CResourceManager * ResManager, 
				CClient * client, 
				resManNS::RESOURCEID MapID, 
				void (CALLBACK *ProgressCallback) (CString),
				graphic::CScene * Scene=NULL);
HRESULT loadPhCar(LPCTSTR carName, 
				  UINT CarNumber, 
				  int CarIndex, 
				  physics::CPhysics * Physics, 
				  resManNS::CResourceManager * ResManager, 
				  resManNS::RESOURCEID MapID,
				  CGameMod * pGameMod,
				  void (CALLBACK *ProgressCallback) (CString));

HRESULT InitGrass( graphic::CScene * Scene );
				


void FillInitWheelStruct(resManNS::GameObject * wheelObject, physics::InitWheel * initWheel, physics::InitCar * initCar);

void FillInitEngineStruct(resManNS::EngineObject * engineObject, physics::InitEngine * initEngine);

void FillInitCarStruct(resManNS::CarObject * carObject, physics::InitCar * initCar, resManNS::CResourceManager * ResManager);

void FillInitObjectStruct(resManNS::GameObject *, physics::InitObject * initObject);
