#include "stdafx.h"
#include "StorageClasses.h"
#include "..\globals\Configuration.h"
#include "EditorProperties.h"
#include <atlstr.h>

using namespace resManNS;

CMainObject::CMainObject(){
	pImage = NULL;
	aktObj = NULL;
	
	//centerX = 0;
	//centerY = 0;
	//centerSizeX=0; //size as size
	//centerSizeY=0; //size as size
	size = 0;sizeX=0;sizeY=0;

	memset(mask,0,MAX_OBJECT_SIZE*MAX_OBJECT_SIZE);
	//maskSize=0;

};

HRESULT CMainObject::loadImage(CStringA path){
	HRESULT hr=ERRNOERROR;
	if (pImage){
		hr = pImage->Load(path);
	}
	else{
		pImage = new CImage;
		hr = pImage->Load(path);
		

	}
	if(hr) SAFE_DELETE(pImage);

	return hr;
}

HRESULT CMainObject::loadFromFile(CStringA filePath,CResourceManager *pRM){
	
	
	HRESULT hr=ERRNOERROR;
	RESOURCEID				resObjID,resWP;
	MainObject				* pMObj;
	GameObject				* pGObj;
	CGameObject				pomObject;
	Waypoints				* pWP;
	WAYPOINTS				pomWPS;
	WAYPOINT				pomWP;
	CPoint					pomPoint;
	UINT					i,j;
	
	
	resObjID=pRM->LoadResource(filePath,RES_MainObject,FALSE,NULL);
	objectFile = filePath;
	if(resObjID < 0) return ERRFILENOTFOUND		; 
	pMObj = pRM->GetMainObject(resObjID);
	
	sName=*(pMObj->sName);
	sIconName=*(pMObj->sIconName);
	bStartingPosition = pMObj->bStartingPosition;
	
	for(i = 0; i < pMObj->pvObjectIDs->size();i++){
		pGObj = pRM->GetGameObject(pMObj->pvObjectIDs->at(i));
		pomObject.setData(pRM,pGObj);
		obj.push_back(pomObject);
	}
	for(i = 0; i < pMObj->pvRoadIDs->size();i++){
		pGObj = pRM->GetGameObject(pMObj->pvRoadIDs->at(i));
		pomObject.setData(pRM,pGObj);
		roads.push_back(pomObject);
	}
	
	if (setSizeAndMask()){
		pRM->ReleaseResource(resObjID);
		return ERRGENERIC;
	}

	vAllWayPoints.clear();
	for(i = 0;i < pMObj->pvWaypointsIDs->size();i++){
		resWP = pMObj->pvWaypointsIDs->at(i);
		if(resWP){
			pWP = pRM->GetWaypoints(pMObj->pvWaypointsIDs->at(i));
			pomWPS.clear();
			for(j = 0;j < pWP->x->size();j++){
				
				if(pWP->x->at(j) >= size*TERRAIN_PLATE_WIDTH)  pomWP.x=size*TERRAIN_PLATE_WIDTH-1;
				else if(pWP->x->at(j) < 0) pomWP.x=0;
				else pomWP.x= pWP->x->at(j);
				pomWP.y=  pWP->y->at(j);
				
				if(pWP->z->at(j) >= size*TERRAIN_PLATE_WIDTH)  pomWP.z=size*TERRAIN_PLATE_WIDTH-1;
				else if(pWP->z->at(j) < 0) pomWP.z=0;
				else pomWP.z= pWP->z->at(j);
				pomWPS.push_back(pomWP);
			}
			vAllWayPoints.push_back(pomWPS);
		}
	}
	pRM->ReleaseResource(resObjID);
	
	return hr;

};
//CMainObject::getWaypoint(
WAYPOINT CMainObject::GetWaypoint(UINT WayPointSetID,int WayPointID, ORIENTATION or){
	WAYPOINT ret,_ret;
	_ret.x=WAYPOINT_ERR;_ret.y=WAYPOINT_ERR;_ret.z=WAYPOINT_ERR;
	if(vAllWayPoints[WayPointSetID].size() == 0) return _ret;
	if (WayPointID < 0) WayPointID = (int) vAllWayPoints[WayPointSetID].size()-1;
	_ret = vAllWayPoints[WayPointSetID].at(WayPointID);
	switch(or){

		case 0:
			ret.x=_ret.x;
			ret.z=_ret.z;
			break;

		case 1:
			ret.x=_ret.z;
			ret.z= size*TERRAIN_PLATE_WIDTH-1 - _ret.x;
			break;
		case 2:
			ret.x=size*TERRAIN_PLATE_WIDTH-1 - _ret.x;
			ret.z=size*TERRAIN_PLATE_WIDTH-1 - _ret.z;
			break;
		case 3:
			ret.x=size*TERRAIN_PLATE_WIDTH-1 - _ret.z;
			ret.z=_ret.x;
			break;
	}
	ret.y = _ret.y;
	return ret;
}
UINT CMainObject::GetWaypointSetSize(){
	return (UINT)vAllWayPoints.size();
}
UINT CMainObject::GetWaypointSize(UINT WayPointSetID){

	return (UINT)vAllWayPoints[WayPointSetID].size();
}


void CMainObject::clear(){
	aktObj=NULL;
	sName="";
	sIconName="";
	objectFile="";
	size=0;
	aktObj=NULL;
	obj.clear();
	roads.clear();
	pomObj.clear();
	
};




//
// Vytvori masku, pak nastavi velikosti a vylouci objekty, ktere vypadavaj
//
HRESULT CMainObject::setSizeAndMask(){

	UINT i,j,k;
	HRESULT hr=ERRNOERROR;
	BYTE hasGrass;
	sizeX=1;sizeY=1;
	for ( i = 0; i< MAX_OBJECT_SIZE ;i++)
		for(j=0;j<MAX_OBJECT_SIZE;j++){
			mask[i][j]=0;
			grassMask[i][j]=1;
		}
	for(i = 0;i < obj.size(); i++){
		//TODO: vyloucit, ktere vypadavaj	
		if((obj[i].sizeX + obj[i].posX > MAX_OBJECT_SIZE)) return ERRGENERIC;
		if((obj[i].sizeY + obj[i].posY > MAX_OBJECT_SIZE)) return ERRGENERIC;

		if((obj[i].sizeX + obj[i].posX) > sizeX)  sizeX = (obj[i].sizeX + obj[i].posX) ;
		if((obj[i].sizeY + obj[i].posY) > sizeY)  sizeY = (obj[i].sizeY + obj[i].posY) ;
		
		hasGrass=1;
		if(!obj[i].bGrassAllowed) hasGrass = 0;

		for(j = 0;j < obj[i].sizeY;j++)
			for(k = 0; k < obj[i].sizeX;k++){
				mask[j+obj[i].posY][k+obj[i].posX]=1;
				if(grassMask[j+obj[i].posY][k+obj[i].posX])
					grassMask[j+obj[i].posY][k+obj[i].posX]=hasGrass;
			};
		
	};

	for(i = 0;i < roads.size(); i++){
		if((roads[i].sizeX + roads[i].posX > MAX_OBJECT_SIZE)) return ERRGENERIC;
		if((roads[i].sizeY + roads[i].posY > MAX_OBJECT_SIZE)) return ERRGENERIC;
		
		if((roads[i].sizeX + roads[i].posX) > sizeX)  sizeX = (roads[i].sizeX + roads[i].posX) ;
		if((roads[i].sizeY + roads[i].posY) > sizeY)  sizeY = (roads[i].sizeY + roads[i].posY) ;
		
		hasGrass=1;
		if(!roads[i].bGrassAllowed) hasGrass = 0;
		for(j = 0;j < roads[i].sizeY;j++)
			for(k = 0; k < roads[i].sizeX;k++){
				mask[j+roads[i].posY][k+roads[i].posX]=1;
				if(grassMask[j+roads[i].posY][k+roads[i].posX])
					grassMask[j+roads[i].posY][k+roads[i].posX]=hasGrass;
		};
	};
	size = sizeX > sizeY ? sizeX : sizeY;
	return hr;
};

void CMainObject::CreateCountedImage(int number){
	
	COLORREF colorRed = RGB(255,0,0);
	
	CFont font;
	CRect bigrect,srect;
	CBrush brush;
	CPen pen,transPen;
	CString num; 
	if(number >= 100) number=0;
	num.Format("%d",number);
	if(!pImage) 
		pImage = new CImage;
	pImage->Create(MAX_ZOOM*size,MAX_ZOOM*size,32);
	bigrect.SetRect(0,0,MAX_ZOOM*size,MAX_ZOOM*size);
	CDC* pDC = CDC::FromHandle(pImage->GetDC());
	

	
	font.CreateFont(MAX_ZOOM,0,0,0,FW_NORMAL,FALSE,TRUE,FALSE,
		DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,NULL);
		pDC->SetBkColor(TRANSPARENT_COLOR);	
	pen.CreatePen(PS_SOLID, 4,colorRed );
	transPen.CreatePen(PS_SOLID,1,TRANSPARENT_COLOR);
	brush.CreateSolidBrush(TRANSPARENT_COLOR);
	
	pDC->SetTextColor(colorRed);
	pDC->SelectObject(font);
	pDC->SelectObject(brush);
	pDC->SelectObject(transPen);
	pDC->Rectangle(bigrect);
	pDC->SelectObject(pen);
	
	for(UINT i=0;i<size;i++)
		for(UINT  j=0;j<size;j++){
			if(mask[size-1-i][j]){
				srect.SetRect(j*MAX_ZOOM,i*MAX_ZOOM,(j+1)*MAX_ZOOM,(i+1)*MAX_ZOOM);
				pDC->DrawText(num,num.GetLength(),&srect,DT_VCENTER | DT_CENTER |DT_SINGLELINE);
			}
		}
	
	pImage->ReleaseDC();
	
};


int CMainObject::getMaskValue(UINT x,UINT y,ORIENTATION or){
	
	if (x >= size) return 0;
	if (y >= size) return 0;
	

	switch(or){
		case ORIENTATION::up:
			return mask[y][x];
		case ORIENTATION::right:
			return mask[x][size - 1 - y];
		case ORIENTATION::down:
			return mask[size - 1 - y][size - 1 - x];
		case ORIENTATION::left:	
			return mask[size - 1 - x][y];
		
	}
	return 0;

}

int CMainObject::getGrassMaskValue(UINT x,UINT y,ORIENTATION or){
	
	if (x >= size) return 0;
	if (y >= size) return 0;
	

	switch(or){
		case ORIENTATION::up:
			return grassMask[y][x];
		case ORIENTATION::right:
			return grassMask[x][size - 1 - y];
		case ORIENTATION::down:
			return grassMask[size - 1 - y][size - 1 - x];
		case ORIENTATION::left:	
			return grassMask[size - 1 - x][y];
		
	}
	return 0;

}


CMapObject::CMapObject(){
	clear();

};

void CMapObject::clear(){
	pMapMainObj=NULL;
	alt=1;
	terOrient = (ORIENTATION) 0;
	texOrient = (ORIENTATION) 0;
	textInd =  0;
	textBaseColor = 0x00aa00;
	physInd = 0;
}


CGameMap::CGameMap(CMapStringToPtr * ptr,CResourceManager * _pRM,CEditorTree * _pEditorTree){
	pObjectsMap=ptr;
	pRM=_pRM;
	pEditorTree = _pEditorTree;
	posStart.or=up;
	posStart.x=0;
	posStart.y=0;
	
//	int i,j,t;
	clearMap();
	
};
CGameMap::~CGameMap(){
	clearMap();
}

HRESULT CGameMap::clearMap(){
	
	int i,j;
	HRESULT hr=ERRNOERROR;
	
	vPhysTextures.clear();
	vPath.clear();
	vTextureSet.clear();
	sWallSet ="default.textset";
	sTextureSet="grass.textset";
	sGrassType="stripped_medium.grass";
	csDescription = "";
	csImageFileName = "";
	vGameMods.clear();
	for ( int i = 0; i < gProp.ModWorkshop.GetModCount(); i++ )
		vGameMods.push_back( gProp.ModWorkshop[i]->GetModulePrimaryID() );
	
	

	sName = "New map";
	sSkySystem="SkySystem.sk";
	uiAmbColor=0xff444444;
	uiFogColor = 0xff8080a0;
	
	uiHours = 10;
	uiMinutes = 0;
	pMOStartPosition =NULL;
	for(i = 0; i <MAX_MAP_SIZE;i++)
		for(j=0;j<MAX_MAP_SIZE;j++) {	
			deleteObjectFromMap(j,i);
			pMapObject[i][j].clear();

		}
	return hr;
};

HRESULT CGameMap::setSize(UINT x, UINT y){
	UINT i=0;
	if (x >= MAX_MAP_SIZE && y >= MAX_MAP_SIZE) return  ERROUTOFMEMORY;
	sizeX = x;
	sizeY = y;
//	pMapObject = (CMapObject[][]) realloc(pMapObject, x*y * (sizeof CMapObject));

	return 0;
};

HRESULT CGameMap::saveMap(CString fName){
	
	std::vector< CMainObject *> vPom;
	CMapMainObject * _pMMO;
	FILE *f;
	CString name;
	UINT c,poc;
	CString			cstr, cstr2;
	CBase64Coder	B64Coder;
	CString mapspath = CONFIG_MAPSPATH;
	float fDayTime;
	int err;
	CMapMainObject * pMMO;


	if(GetPathSize()==0)return -1;
	err = fopen_s(&f,mapspath +fName,"w");
	if (err) return err;
	fprintf(f,"%u %u\n",sizeX,sizeY); // Velikost mapy
	name=fName;
	name.Delete((int)(name.GetLength()-strlen(MAP_EXT)),(int)strlen(MAP_EXT));
	fprintf(f,"%s\n",name);
	fprintf(f,"%i.%i\n", (MAP_FILE_FORMAT_VERSION & 0xffff0000) >> 16, MAP_FILE_FORMAT_VERSION & 0xffff );
	fprintf(f,"%s\n",csDescription);
	for ( int i = 0; i < (int)vGameMods.size(); i++ ) {
		if ( i > 0 ) fprintf(f,",%s",vGameMods[i]);
		else fprintf(f,"%s",vGameMods[i]);
	}
	fprintf(f,"\n");
	fprintf(f,"%s\n",csImageFileName);
	fprintf(f,"%s\n",sTerrain);
	fprintf(f,"%s\n",sSkySystem);
	fDayTime = (float)(uiHours*60+uiMinutes)/24/60;
	fprintf(f,"%f\n",fDayTime);
	fprintf(f,"%#x\n",uiAmbColor);
	fprintf(f,"%#x\n",uiFogColor);
	
	
	fprintf(f,"%s\n",pMOStartPosition->getFileName().GetString());
	fprintf(f,"%d %d %d\n",posStart.x,posStart.y,posStart.or);

	vPom.clear();

	poc = 0;
	//look to map for same objects
	for( UINT i = 0; i < sizeY;i++) 
		for( UINT j = 0; j < sizeX; j++){
				
				UINT k = 0;
				_pMMO = getMainObjectInPos(j,i);
				if(_pMMO){
					if(!_pMMO->counted){ 
						_pMMO->counted=true;
					
						poc++;
					}

					while( k <vPom.size()){
						
						if(vPom[k] ==_pMMO->pMainObject) break;
						k ++;
					}
					if(k == vPom.size()){
						
						vPom.push_back(_pMMO->pMainObject);
					}
				}

			//}
		}

	//print file names
	fprintf(f,"%u\n",vPom.size());
	
	for(UINT i = 0; i < vPom.size();i++){
		fprintf(f,"%s\n",vPom[i]->getFileName());
	}

	fprintf(f,"%u\n",poc);
	poc = 0;
	for( UINT i = 0; i < sizeY; i++)
		for( UINT j = 0; j < sizeX; j++){
			if (pMapObject[i][j].pMapMainObj && !pMapObject[i][j].pMapMainObj->saved){
				pMMO = pMapObject[i][j].pMapMainObj;

				cstr = "";
				for ( int imod = 0; imod < (int)pMMO->SpecialProperties.size(); imod++ ) {
					int modIndex = -1;
					// look for the MOD in the vGameMods list (supported MODs)
					for ( int k = 0; k < (int)vGameMods.size(); k++ ) {
						if ( vGameMods[k] == CString(pMMO->SpecialProperties[imod].sModID) ) {
							modIndex = k;
							break;
						}
					}
					if ( modIndex < 0 ) continue; // not supported MOD, ignore properties
					if ( imod > 0 ) cstr += ";";
					cstr += vGameMods[modIndex]; // MOD ID
					cstr += ":";
					for ( int iprop = 0; iprop < (int)pMMO->SpecialProperties[imod].vProperties.size(); iprop++ ) {
						if ( iprop > 0 ) cstr += ",";
						cstr += pMMO->SpecialProperties[imod].vProperties[iprop].GetName(); // property name
						cstr += "=";
						cstr += pMMO->SpecialProperties[imod].vProperties[iprop].GetString(); // property value
					}
				}
				
				if ( cstr == "" ) cstr2 = "=";
				else {	
					// convert the string to base64
					B64Coder.InitBuffer( (BYTE *)cstr.GetBuffer(), cstr.GetLength() );
					B64Coder.Encode();
					cstr2 = "";
					for ( UINT k = 0; k < B64Coder.GetEncodedBufferSize(); k++ )
						cstr2 += B64Coder.GetEncodedBuffer()[k];
				}
				
				// write the object to file
				c=0;
				while ( pMMO->pMainObject != vPom[c] ) c++;
				fprintf(f,"%i %i %i %u %s\n", c, pMMO->posX, pMMO->posY, pMMO->objOrient, cstr2);
				
				pMMO->saved=TRUE;
				pMMO->index=poc;
				poc++;
			}
		}
	for( UINT i = 0; i < sizeY; i++)
		for( UINT j = 0; j < sizeX; j++){
			if (pMapObject[i][j].pMapMainObj){ 
				pMapObject[i][j].pMapMainObj->saved=FALSE;
				pMapObject[i][j].pMapMainObj->counted=FALSE;
		
			}
			
		}
	fprintf(f,"%u\n",GetPathSize());

	UINT ret = 0;
	for(UINT i=0; i < vPath.size();i++){
		for(UINT j=0;j<vPath[i].size();j++){
			fprintf(f,"%u\n",vPath[i][j]->index);
		}
	}
	

	
	fclose(f);
	return 0;
};


int CGameMap::rohvnitrek(UINT i, UINT j){
	
	int opak,ojeden,odva,alt = pMapObject[i][j].alt;
	int krajy,krajx,krajx2,krajy2;
	int y[4] = { 1,-1,-1,1};
	int x[4] = { 1,1,-1,-1};
	int y1,x1;
	UINT k;
	

	for(k = 0 ; k < 4;k++){
		
		if(TEST_BORDER(i+y[k],j+x[k])) ojeden = pMapObject[i+y[k]][j+x[k]].alt;
		else ojeden=alt;
		
		if(TEST_BORDER(i+y[k],j)) krajy=pMapObject[i+y[k]][j].alt;
		else krajy=alt;

		if(TEST_BORDER(i,j+x[k])) krajx=pMapObject[i][j+x[k]].alt;
		else krajx=alt;

		y1 = y[(k+2)%4]; x1 = x[(k+2)%4];
		if(TEST_BORDER(i+y1,j+x1)) opak = pMapObject[i+y1][j+x1].alt;//naproti
		else opak = alt;//alt-1 < 0 ? 0 : alt -1;
		
		y1 = 2*y[k]; x1 = 2*x[k];
		if(TEST_BORDER(i+y1,j+x1)) odva = pMapObject[i+y1][j+x1].alt; //pozice o 2 od stredu
		else odva=alt;
		
		if(TEST_BORDER(i+y[k],j+x1)) krajy2 = pMapObject[i+y[k]][j+x1].alt; //pozice o 2 od stredu
		else krajy2=alt;
		
		if(TEST_BORDER(i+y1,j+x[k])) krajx2 = pMapObject[i+y1][j+x[k]].alt; //pozice o 2 od stredu
		else krajx2=alt;
		
				
		if( (alt == krajx-1) && (alt == krajy-1) ){
		
			if(alt == opak+1){ // neni to pata
				
				if ( (ojeden == krajx2-1) && (ojeden == krajy2-1)) //mame a pokracuje nahore stred			
					return TERRAIN_TYPE_ROHVNITRNISTRED*4+k;
				else 
					return TERRAIN_TYPE_ROHVNITRNIVRCH*4+k;

			
			}
			else { //kopec bez paty vrch
				
				if ( ((ojeden == krajx2-1) && (ojeden == krajy2-1))) //mame a pokracuje nahore stred
					return TERRAIN_TYPE_ROHVNITRNIPATA*4+k;
				else return TERRAIN_TYPE_ROHVNITRNI1 *4 + k;
		
			}
	
		}


		
	}
	
	return -1;
};


int CGameMap::roh(UINT i,UINT j){
	
	int opak,ojeden,odva,alt = pMapObject[i][j].alt;
	int krajy,krajx;
	int y[4] = { 1,-1,-1,1};
	int x[4] = { 1,1,-1,-1};
	int y1,x1;
	UINT k;

	for(k = 0 ; k < 4;k++){
		
		if(TEST_BORDER(i+y[k],j+x[k])) ojeden = pMapObject[i+y[k]][j+x[k]].alt;
		else ojeden=alt;
		
		if(TEST_BORDER(i+y[k],j)) krajy=pMapObject[i+y[k]][j].alt;
		else krajy=alt;

		if(TEST_BORDER(i,j+x[k])) krajx=pMapObject[i][j+x[k]].alt;
		else krajx=alt;

		y1 = y[(k+2)%4]; x1 = x[(k+2)%4];
		if(TEST_BORDER(i+y1,j+x1)) opak = pMapObject[i+y1][j+x1].alt;//naproti
		else opak = alt;//alt-1 < 0 ? 0 : alt -1;
			
		y1 = 2*y[k]; x1 = 2*x[k];
		if(TEST_BORDER(i+y1,j+x1)) odva = pMapObject[i+y1][j+x1].alt; //pozice o 2 od stredu
		else odva=alt;
		
		if(alt == (ojeden - 1) && krajy==alt && krajx==alt){
			if(alt == opak+1){ // test druhe strany 
					
					
					if ( ojeden == odva-1) //mame a pokracuje nahore stred		
						
							return TERRAIN_TYPE_ROHSTRED*4+k;
					else{ 
							return TERRAIN_TYPE_ROHVRCH*4+k; // mame stenu s patou
					}
				
			}
			else { //kopec bez paty vrch
				
				if ( ojeden == odva-1) //mame a pokracuje nahore stred
					return TERRAIN_TYPE_ROHPATA*4+k;
				else
					return TERRAIN_TYPE_ROH1*4+k;
				}

		}
	}
	
	return -1;
};



int CGameMap::hrana(UINT i,UINT j){

	int opak,ojeden,odva,odval,odvar,alt = pMapObject[i][j].alt;
	int krajy,krajx;
	int y[4] = { 1,0,-1,0};
	int x[4] = { 0,1,0,-1};
	
	int y1,x1;
	UINT k;

	for(k = 0 ; k < 4;k++){
		
		if(TEST_BORDER(i+y[k],j+x[k])) ojeden = pMapObject[i+y[k]][j+x[k]].alt;
		else ojeden=alt;
		
		
		if(TEST_BORDER(i+x[k],j+y[k])) krajy=pMapObject[i+x[k]][j+y[k]].alt;
		else krajy=alt;

		if(TEST_BORDER(i-x[k],j-y[k])) krajx=pMapObject[i-x[k]][j-y[k]].alt;
		else krajx=alt;


		y1 = y[(k+2)%4]; x1 = x[(k+2)%4];
		if(TEST_BORDER(i+y1,j+x1)) opak = pMapObject[i+y1][j+x1].alt;//naproti
		else opak=alt;//alt-1 < 0 ? 0 : alt -1;
			
		y1 = 2*y[k]; x1 = 2*x[k];
		if(TEST_BORDER(i+y1,j+x1)) odva = pMapObject[i+y1][j+x1].alt; //pozice o 2 od stredu
		else odva=alt;

		y1 = 2*y[k]-x[k]; x1 = 2*x[k]-y[k];
		if(TEST_BORDER(i+y1,j+x1)) odval = pMapObject[i+y1][j+x1].alt; //pozice o 2 od stredu
		else odval=alt;
		
		y1 = 2*y[k]+x[k]; x1 = 2*x[k]+y[k];

		if(TEST_BORDER(i+y1,j+x1)) odvar = pMapObject[i+y1][j+x1].alt; //pozice o 2 od stredu
		else odvar=alt;

		if( alt == (ojeden - 1) && alt==krajx && alt==krajy){ //kopec je 
					
			if(alt == opak+1){ // test druhe strany 
				
				if ( ojeden == odva-1 || ojeden == odval -1 || ojeden == odvar-1) //mame a pokracuje nahore stred
					return TERRAIN_TYPE_STENASTRED*4+k;
				else 
					return TERRAIN_TYPE_STENAVRCH*4+k; // mame stenu s patou

			
			}
			else { //kopec bez paty vrch
				
				if ( ojeden == odva-1 || ojeden == odval -1 || ojeden == odvar-1) //mame a pokracuje nahore stred
				
						return TERRAIN_TYPE_STENAPATA*4+k;
				else	return TERRAIN_TYPE_STENA1*4 + k;
			}
				
		}
	}
	
	

	return -1;
};

int CGameMap::GetTerrainType(PlaneType pt, SideType st1, SideType st2, SideType stDiagonal) {
    int offset = 1;
    if (stDiagonal == curved && pt != hillside)
        if (st1 == st2 && st1 == stDiagonal && (pt == outercorner || pt == innercorner)) return offset + pt;
        else return 0;
    offset += 2;
    if (pt == hillside) return 3 + (st1 * 4 + st2); offset += 16;
    offset += pt * 3 * 16;
    offset += stDiagonal * 16;
    return offset + st1 * 4 + st2;
}

void CGameMap::GetPlateType(int n,SideType &st,PlaneType &pt,int &or){
	int type = n / 4;
	or = n % 4;
	
	switch( (type-1) % 3){
		case 0:
			pt=outercorner;
		break;
		case 1:
			pt=hillside;
		break;
		case 2:
			pt=innercorner;
		break;
	}
	switch( (type-1) / 3){
		case 0:
			st=middle;
		break;
		case 1:
			st=top;
		break;
		case 2:
			st=bottom;
		break;
		case 3:
			st=curved;
		break;
	}
	
};
HRESULT CGameMap::saveTerrain(CString fName){
	
	
	
	int i,j,v,i1;
	UINT jp1, ip1,ip2,jp2;
	FILE *f;
	CString name,_set;
	int outtype,_outtype;
	int or,_or;
	SideType st,st1,st2;
	PlaneType pt,_pt;
	errno_t fileerr;
	RESOURCEID				grassID;
	TextureSet				* ts;	
	CString mapspath = CONFIG_MAPSPATH;
	int pole[MAX_MAP_SIZE][MAX_MAP_SIZE];
	fileerr = fopen_s(&f,mapspath+fName,"w");
	if(fileerr) return fileerr;
	

	int sicx[4]={-1,0,1,0};
	int sicy[4]={0,1,0,-1};

	int ocx[4]={1,0,-1,0};
	int ocy[4]={0,-1,0,1};
	
	
	//////////////////////////////
	// vytvoreni hranatych kopcu
	for(i = 0 ; i < (int) sizeY;i++)
			for(j =0; j < (int)sizeX;j++) {
		
				outtype = roh(i,j);
				_outtype = hrana(i,j);
				outtype = _outtype != -1 ? _outtype: outtype;
				_outtype = rohvnitrek(i,j);
				outtype = _outtype != -1 ? _outtype: outtype;
				if(outtype!=-1) 
					pole[i][j]=outtype;
				else 
					pole[i][j]=-1;
				pMapObject[i][j].type=-1;
	}
	////////////////////////////////////////////////////////////////////
	// odstraneni der krivi se v poradi:  middle, top,bottom,curved
	
	for(v = 0; v < 4; v++){
		for(i = 0 ; i < (int) sizeY;i++)
			for(j =0; j < (int)sizeX;j++) {
				
				if( (v*3+1)*4 <= pole[i][j] && pole[i][j] < (v*3+4)*4 ){
					
					GetPlateType(pole[i][j],st,pt,or);
					st1 = st;st2=st;
					switch(pt){
						case outercorner:
							ip1=i+ocy[or]; jp1=j + ocx[or];
							ip2=i+ocy[(or+3)%4]; jp2=j+ocx[(or+3)%4];
						break;
						case innercorner:
							ip1=i+sicy[or]; jp1=j + sicx[or];
							ip2=i+sicy[(or+3)%4]; jp2=j+sicx[(or+3)%4];
						break;
						case hillside:
							ip1=i+sicy[or]; jp1=j + sicx[or];
							ip2=i+sicy[(or+2)%4]; jp2=j+sicx[(or+2)%4];
						break;
					
					}
					if (getType(jp1,ip1)==-1){
						if(TEST_BORDER(ip1,jp1) && pole[ip1][jp1]!=-1){
							GetPlateType(pole[ip1][jp1],st1,_pt,_or);
						}		
					}
					if (getType(jp2,ip2)==-1){
						if(TEST_BORDER(ip2,jp2) && pole[ip2][jp2]!=-1 ){
							GetPlateType(pole[ip2][jp2],st2,_pt,_or);
						}
					}
					pMapObject[i][j].type=GetTerrainType(pt,st1,st2,st);
   					pMapObject[i][j].terOrient=(ORIENTATION) or;
				
				}


			}
	
	}
	for(i = 0 ; i < (int) sizeY;i++)
		for(j =0; j < (int)sizeX;j++) {
				if(pMapObject[i][j].type==-1){
					pMapObject[i][j].type=0;
					//pMapObject[i][j].terOrient=(ORIENTATION) (rand()%4);
				}
		}

	


	fprintf(f,"%u %u\n",sizeX,sizeY);
	_set = TEXTURESET_TERRAIN_PATH;_set += sTextureSet;
	fprintf(f,"%s\n",_set);
	_set = TEXTURESET_WALL_PATH;_set += sWallSet;
	fprintf(f,"%s\n",_set);
	fprintf(f,"1\n");
	fprintf(f,"%s\n",sGrassType);
	/////////////////////////////////////////
	// load Textures
	
	vTextureSet.clear();
	vPhysTextures.clear();
	_set = TEXTURESET_TERRAIN_PATH;_set += sTextureSet;
	grassID = pRM->LoadResource(_set,RES_TextureSet,FALSE,NULL);
	if(grassID > 0){
		ts = pRM->GetTextureSet(grassID);
		vPhysTextures.push_back(*(ts->psPhTexture));
		for(i=0; (UINT)i < ts->uiTextureCount;i++){
			name = ts->pTextureFileNames[i];
			vTextureSet.push_back(TEXTURESET(name,ts->pTextureProbabilities[i]));
		}

	}else{
	
		vTextureSet.push_back(TEXTURESET("terrain\\mafiagrass.jpg",7));
		vTextureSet.push_back(TEXTURESET("terrain\\mafiagrass2.jpg",1));
		vTextureSet.push_back(TEXTURESET("terrain\\mafiagrass3.jpg",1));
		vTextureSet.push_back(TEXTURESET("terrain\\mafiagrass4.jpg",1));
		vPhysTextures.push_back("grass.phTexture");
	}

	pRM->ReleaseResource(grassID);
	
	
	fprintf(f,"%u\n", vTextureSet.size());
	for(i=0;i<(int)vTextureSet.size();i++){
		fprintf(f,"%s\n",vTextureSet[i].sTexture);

	}
	
	fprintf(f,"%u\n", vPhysTextures.size());
	for(i=0;i<(int)vPhysTextures.size();i++){
		fprintf(f,"%s\n",vPhysTextures[i]);

	}
	

	for(i1 = sizeY - 1 ; i1 >= 0;i1--){
		for(j =0; j < (int)sizeX;j++){
		
		
			fprintf(f,"%u %u %u %u %u %u %u ",
				pMapObject[i1][j].alt,pMapObject[i1][j].type,
				pMapObject[i1][j].terOrient,GetRandomTexture(),
				0,pMapObject[i1][j].texOrient,
				isGrass(j,i1) 
				);
		
		}
		fprintf(f,"\n");
	}
	fclose(f);
	return 0;
};
BOOL CGameMap::isGrass(UINT x,UINT y){
	UINT xr,yr;
	if(!pMapObject[y][x].pMapMainObj || !pMapObject[y][x].pMapMainObj->pMainObject) return TRUE;
	CMapMainObject * pMMO;
	pMMO = pMapObject[y][x].pMapMainObj;
	xr=x-pMMO->GetCorner().x;
	yr=y-pMMO->GetCorner().y;
	return pMMO->pMainObject->getGrassMaskValue(xr,yr,pMMO->objOrient);
};
int CGameMap::GetPathSize(){
	UINT i;
	UINT ret = 0;
	for(i=0; i < vPath.size();i++){
		ret+=(UINT)vPath[i].size();
		
	}
	return ret;

}
//
// delete object from all positions in the map and return deleted object.
CMainObject * CGameMap::deleteObjectFromMap(UINT x,UINT y){
	
	CMapMainObject * pPomMMObj;
	CMainObject * pPomMObj;
	UINT objSize,k,l, posX, posY;
	int shift;
	ORIENTATION or;

	pPomMMObj = getMainObjectInPos(x,y);
	
	
	if(!pPomMMObj) return NULL;

	posX = pPomMMObj->posX;
	posY = pPomMMObj->posY;
	or = pPomMMObj->objOrient;
	pPomMObj = pPomMMObj->pMainObject;
	objSize=pPomMObj->getSize();	
	shift = pPomMObj->getShift();
	for(l = 0; l < objSize;l++)
		for (k = 0; k < objSize; k++){
			if (pPomMObj->getMaskValue(k,l,or))
				if (pMapObject[posY+l-shift][posX+k-shift].pMapMainObj) {
					pMapObject[posY+l-shift][posX+k-shift].pMapMainObj=NULL;
				}
		}
	SAFE_DELETE(pPomMMObj);
	return pPomMObj;
}

	
// return true, if you can set object esle false
BOOL CGameMap::isPlaceInMap(UINT x, UINT y, ORIENTATION or,CMainObject * pPomMObj ){
	int objSize;
	int k,l,shift;
	BOOL free=true;
	shift = pPomMObj->getSizeN();
	objSize = pPomMObj->getSize();
	if ((int)y - pPomMObj->getSizeN() < 0 ) return FALSE; // test na vypadek z mapy
	if ((int)x - pPomMObj->getSizeN() < 0 ) return FALSE; // test na vypadek z mapy
	if ((int)y + pPomMObj->getSizeP() > (int)sizeY ) return FALSE; // test na vypadek z mapy
	if ((int)x + pPomMObj->getSizeP() > (int) sizeX ) return FALSE; // test na vypadek z mapy
	

	for(l = 0; l < objSize;l++)
		for (k = 0; k < objSize; k++){
			if (pPomMObj->getMaskValue(k,l,or))
				if (pMapObject[y+l-shift][x+k-shift].pMapMainObj) {
					free = false;
					break;
				}
		}
	
	return free;
}


//
//Set Object to Map and Try if can 
CMapMainObject * CGameMap::setObjectToMap(UINT x, UINT y, ORIENTATION or,CMainObject * pPomMObj ){

	/*UINT csx,csy,dx,dy,k,l;
	CMapMainObject * pPomMMObj;
	//TODO: test pro okraje
	HRESULT hr = ERRNOERROR;
	pPomMMObj = new CMapMainObject(); 
	//TODO: test nalezeni ve tree a priani do Tree jako neznami;
	
	pPomMMObj->pMainObject = pPomMObj ;
	pPomMMObj->objOrient = or;
	pPomMMObj->posX = x;
	pPomMMObj->posY = y;
	
	
	//Where is work area - full
	objSize=pPomMObj->getSize();
/*
	
	objSize= 
	csx = pPomMObj->centerSizeX ? pPomMObj->centerSizeX : pPomMObj->getSize();
	csy = pPomMObj->centerSizeY ? pPomMObj->centerSizeY : pPomMObj->getSize();
	
	for(l = 0; l < csy;l++)
		for (k = 0; k < csx; k++){
						
			dx = x + pPomMObj->centerX;
			dy = y + pPomMObj->centerY;
			
			pMapObject[dy + l][dx + k].pMapMainObj=pPomMMObj;
			if(!k && !l){
				pMapObject[dy][dx].pMapMainObj->cPosX=dx; //set position of left bottom corner
				pMapObject[dy][dx].pMapMainObj->cPosY=dy; 
			}

		}
*/	if(!isPlaceInMap(x,y,or,pPomMObj)) return NULL;
	CMapMainObject * pMMObject;
	int objSize,k,l;
	int shift;
	pMMObject = new CMapMainObject();
	if(!pMMObject) return NULL;
	pMMObject->objOrient=or;
	pMMObject->posX=x;
	pMMObject->posY=y;
	pMMObject->pMainObject = pPomMObj;
	objSize=pPomMObj->getSize();
	shift  =pPomMObj->getShift();
	/*
	if(!objSize) {
		SAFE_DELETE(pMMObject);
		return 0;
	}
	shift = (objSize-1)/2;
	if (y + objSize > sizeY ) {
		SAFE_DELETE(pMMObject);
		return 0; // test na vypadek z mapy
	}
	if (x + objSize > sizeX ){
		SAFE_DELETE(pMMObject);
		return 0; // test na vypadek z mapy
	}
*/
	for(l = 0; l < objSize;l++)
		for (k = 0; k < objSize; k++){
			if (pPomMObj->getMaskValue(k,l,or))
				pMapObject[y+l-shift][x+k-shift].pMapMainObj=pMMObject;
		}
	return pMMObject;

};

HRESULT CGameMap::setStartToMap(UINT x, UINT y, ORIENTATION or, CMainObject * pSP){
	//TODO:TEST
	UINT i;
	float xr,zr,rot;
	CGameObject * pGOStartPos;

	if(pSP)
		for(i =0; i < pSP->obj.size(); i++){
			pGOStartPos=&pSP->obj[i];
			switch(or){
				case 0:
					xr = (float) TERRAIN_PLATE_WIDTH * x + pGOStartPos->x;
					zr = (float) TERRAIN_PLATE_WIDTH * y + pGOStartPos->z;
					break;
				case 1:
					xr = (float)TERRAIN_PLATE_WIDTH* x +  pGOStartPos->z;
					zr = (float)TERRAIN_PLATE_WIDTH* y + TERRAIN_PLATE_WIDTH - pGOStartPos->x;
					break;
				case 2:
					xr = (float)TERRAIN_PLATE_WIDTH* x +TERRAIN_PLATE_WIDTH - pGOStartPos->x;
					zr = (float)TERRAIN_PLATE_WIDTH* y +TERRAIN_PLATE_WIDTH - pGOStartPos->z;
					break;
				case 3:
					xr = (float)TERRAIN_PLATE_WIDTH* x + TERRAIN_PLATE_WIDTH -  pGOStartPos->z;
					zr = (float)TERRAIN_PLATE_WIDTH* y + pGOStartPos->x; 
					break;

			}
			rot = D3DX_PI/ 2 *posStart.or + pGOStartPos->rotationY;
			if(!(STARTPOS_R < xr && xr < sizeX*TERRAIN_PLATE_WIDTH && STARTPOS_R < zr && zr < sizeY*TERRAIN_PLATE_WIDTH))
				return ERRGENERIC;
	
		}
	posStart.x=x;
	posStart.y=y;
	posStart.or=or;
	pMOStartPosition=pSP;
	
	return ERRNOERROR;
	
};
///////////////////////////////////////////////////
//Load map from file and set 
///////////////////////////////////////////////////
HRESULT CGameMap::loadMap(CString fName){
	
	RESOURCEID	resMapID;
	Map			* pMapData;
	UINT		i,id;
	int num;
	void * point;
	ORIENTATION or;
	int			x,y;
	void		* pointer;
	float		fDayTime;
	BOOL		res;
	CMainObject * pMainObj=NULL;
	HRESULT hr =ERRNOERROR,hr2;
	CString path,sStartPos;
	std::vector<CMapMainObject *> pomVect;
	std::vector<CMapMainObject *> _vPath;
	CMapMainObject * pMMO;
	pMOStartPosition = NULL;

	resMapID = pRM->LoadResource(fName,RES_Map,FALSE,NULL);
	if ( resMapID < 1 ) ERRORMSG( ERRGENERIC, "MapInit()", "Could not load map object." );
	pMapData = pRM->GetMap(resMapID);
	
	if ( pMapData->uiSegsCountX >= MAX_MAP_SIZE || pMapData->uiSegsCountZ >= MAX_MAP_SIZE) return  ERROUTOFMEMORY;
	setSize(pMapData->uiSegsCountX,pMapData->uiSegsCountZ);
	sName=*pMapData->sMapName;
	sSkySystem = *pMapData->sSkyboxName;
	uiAmbColor = pMapData->uiColor;
	uiFogColor = pMapData->uiFogColor;
	csDescription = *(pMapData->sMapDescription);
	csImageFileName = *(pMapData->sMapImageFile);
	vGameMods.clear(); 
	for ( int i = 0; i < (int) pMapData->pvSupportedModules->size(); i++ ) {
		vGameMods.push_back( (*(pMapData->pvSupportedModules))[i] );
	}
	fDayTime = pMapData->fDayTime;
	uiHours = (int) (pMapData->fDayTime*24) ;
	uiMinutes = (int) (fmod(pMapData->fDayTime*24,1.0f)*60);
	sTerrain = *pMapData->sTerrainName;
	sStartPos = *pMapData->sStartPositionFileName;
	
			
	//TODO:STARTING POSITIONS
	//for(i=0;i<pMapData->uiStartingPositionsCount;i++){
		//pMapData->pfStartingPositions[i];
	//};

//	pMapObject = (CMapObject[][]) realloc(pMapObject, x*y * (sizeof CMapObject));
	
	////////////////////////////////////////////////////////////////////////
	// Start position
	///////////////////////////////////////////////////////////////////////
	res = pObjectsMap->Lookup(sStartPos,pointer );
	if (res && pointer){
		setStartToMap(pMapData->iStartPositionX,pMapData->iStartPositionZ,(ORIENTATION)pMapData->iStartRotation,(CMainObject *) pointer);
	}
	else{
		pMainObj = new CMainObject();
		if(!pMainObj){
			
			hr2 = pMainObj->loadFromFile(sStartPos,pRM);
			if (hr2){ SAFE_DELETE(pMainObj); }
			else {
				path=CONFIG_EDITORPATH;
				path+=pMainObj->sIconName;
				hr2 = pMainObj->loadImage(path);
				
				
				if(hr2){
					if( !pEditorTree->mapIconCounter.Lookup(pMainObj->getFileName(),point)){
						num=pEditorTree->iIconCounter++;
						pEditorTree->mapIconCounter[pMainObj->getFileName()]=IntToPtr(num);
					}else num = PtrToInt(point);
					pMainObj->CreateCountedImage(num);
				}
			}
				
			(* pObjectsMap)[sStartPos]=pMainObj;

			if(!pMainObj || pMainObj->sName=="") {
				pMainObj->sName=sStartPos;
				pMainObj->sName.Delete((int)(pMainObj->sName.GetLength()-strlen(OBJECT_EXT)),(int)strlen(OBJECT_EXT));
			} 			
			pEditorTree->AddNewItem(pMainObj);
			setStartToMap(pMapData->iStartPositionX,pMapData->iStartPositionZ,(ORIENTATION)pMapData->iStartRotation,pMainObj);
		}
	}
	///////////////////////////////////////////////////////////////////////
	// Objects
	//////////////////////////////////////////////////////////////////////
	for(i=0; i < pMapData->uiPlacedObjectsCount ;i++){
		
		id=pMapData->puiPlacedObjectIndexes[i]; 
		x=pMapData->piPlacedObjectX[i];
		y=pMapData->piPlacedObjectZ[i];
		or=(ORIENTATION) pMapData->puiPlacedObjectOrientations[i];
		pointer = NULL; 
		res = pObjectsMap->Lookup(pMapData->psObjectNames[id],pointer );
		
		if (res && pointer){
			pMMO = setObjectToMap(x,y,or,(CMainObject *) pointer);
			if ( pMMO ) {
				// copy MOD specific properties for this instance
				if ( pMapData->pbPlacedObjectSpecial[i] ) {
					pMMO->bSpecialProperties = true;
					pMMO->SpecialProperties.clear();
					for ( int j = 0; j < (int)pMapData->pPlacedObjectData[i].size(); j++ ) {
						pMMO->SpecialProperties.push_back( pMapData->pPlacedObjectData[i][j] );
					}
				} else {
					pMMO->bSpecialProperties = false;
					pMMO->SpecialProperties.clear();
				}
				pomVect.push_back(pMMO);
			}

		}
		else{
			pMainObj = new CMainObject();
			if(!pMainObj) break;
			
			hr2 = pMainObj->loadFromFile(pMapData->psObjectNames[id],pRM);
			if (hr2){ SAFE_DELETE(pMainObj); }
			else {
				path=CONFIG_EDITORPATH;
				path+=pMainObj->sIconName;
				hr2 = pMainObj->loadImage(path);

				if(hr2){
					if( !pEditorTree->mapIconCounter.Lookup(pMainObj->getFileName(),point)){
						num=pEditorTree->iIconCounter++;
						pEditorTree->mapIconCounter[pMainObj->getFileName()]=IntToPtr(num);
					}else num = PtrToInt(point);	
					pMainObj->CreateCountedImage(num);
				}
				
				(* pObjectsMap)[pMapData->psObjectNames[id]]=pMainObj;

				if( pMainObj && pMainObj->sName=="") {
					pMainObj->sName=pMapData->psObjectNames[id];
					pMainObj->sName.Delete((int)(pMainObj->sName.GetLength()-strlen(OBJECT_EXT)),(int)strlen(OBJECT_EXT));
				} 			
				pEditorTree->AddNewItem(pMainObj);
				pMMO = setObjectToMap(x,y,or,pMainObj);
				if ( pMMO ) {
					// copy MOD specific properties for this instance
					if ( pMapData->pbPlacedObjectSpecial[i] ) {
						pMMO->bSpecialProperties = true;
						pMMO->SpecialProperties.clear();
						for ( int j = 0; j < (int)pMapData->pPlacedObjectData[i].size(); j++ ) {
							pMMO->SpecialProperties.push_back( pMapData->pPlacedObjectData[i][j] );
						}
					} else {
						pMMO->bSpecialProperties = false;
						pMMO->SpecialProperties.clear();
					}
					pomVect.push_back(pMMO);
				}
			}
		}
	}

	for(i = 0 ; i < pMapData->uiPathPointsCount;i++){
		if(pomVect.size() > pMapData->puiPathPointIndexes[i])
		{	_vPath.push_back(pomVect[pMapData->puiPathPointIndexes[i]]);
			pomVect[pMapData->puiPathPointIndexes[i]]->pathset=true;
		}
	}
	if(_vPath.size())vPath.push_back(_vPath);
	pRM->ReleaseResource(resMapID);
	
	return hr;
	
};
///////////////////////////////////////////////////
//Load terrain from file and set 
///////////////////////////////////////////////////
HRESULT CGameMap::loadTerrain(CString fName){
	
	
	HRESULT hr=ERRNOERROR;
	RESOURCEID				resTerrainID;
	Terrain					* pTerrain;
	UINT					i,j;
	UINT					index;
	
	CString					name;
	resTerrainID = pRM->LoadResource(fName,RES_Terrain ,FALSE,NULL);
	if ( resTerrainID < 1 ) ERRORMSG( ERRGENERIC, "MapInit()", "Could not load map object." );
	pTerrain = pRM->GetTerrain(resTerrainID);
	sGrassType = pTerrain->pGrassFileNames[0];
	sWallSet = *(pTerrain->sWallTextureSet);
	if(sWallSet.Left((int)strlen(TEXTURESET_WALL_PATH))== TEXTURESET_WALL_PATH)
		sWallSet.Delete(0,(int)strlen(TEXTURESET_WALL_PATH));

	if(setSize(pTerrain->uiSizeX,pTerrain->uiSizeY)){ ERRORMSG(  ERROUTOFMEMORY,"loadMap()","bad size"); }
	sTextureSet = *(pTerrain->sTextureSet);
	
	if(sTextureSet.Left((int)strlen(TEXTURESET_TERRAIN_PATH))== TEXTURESET_TERRAIN_PATH)
		sTextureSet.Delete(0,(int)strlen(TEXTURESET_TERRAIN_PATH));
	
	for(j = sizeY-1; j < sizeY ;j--) 
		for(i = 0; i < sizeX;i++){
			index = sizeX * j + i;
			//pMapObject[j][i].clear();
			pMapObject[j][i].alt=pTerrain->pTerrainPlateInfo[index].uiHeight;
			pMapObject[j][i].terOrient=(ORIENTATION) pTerrain->pTerrainPlateInfo[index].uiPlateRotation;
			pMapObject[j][i].textInd=pTerrain->pTerrainPlateInfo[index].uiTextureIndex;
			pMapObject[j][i].physInd=pTerrain->pTerrainPlateInfo[index].uiGrPlateIndex;
			pMapObject[j][i].texOrient=(ORIENTATION) pTerrain->pTerrainPlateInfo[index].uiTextureRotation;
			
		}
	pRM->ReleaseResource(resTerrainID);
	return 0;

};
void CGameMap::ChangeMainObject(CMainObject *pOld, CMainObject *pNew){
	UINT j,i;
	ORIENTATION or;
	UINT posX,posY;
	
	if (pOld->bStartingPosition){
		if(pNew->bStartingPosition){
			if(setStartToMap(posStart.x,posStart.y,posStart.or,pNew)!=0)
				pMOStartPosition=NULL;
		}else{
			pMOStartPosition = NULL;
		}
	}
	else 
	for(j = sizeY-1; j < sizeY ;j--) 
		for(i = 0; i < sizeX;i++){
			if(pMapObject[i][j].pMapMainObj && pMapObject[i][j].pMapMainObj->pMainObject == pOld){
				
				or=pMapObject[i][j].pMapMainObj->objOrient;
				posX = pMapObject[i][j].pMapMainObj->posX;
				posY = pMapObject[i][j].pMapMainObj->posY;
				
				deleteObjectFromMap(j,i);
				if(pNew->bStartingPosition){
					if(setStartToMap(posStart.x,posStart.y,posStart.or,pNew)!=0)
						pMOStartPosition=NULL;;
				}
				else setObjectToMap(posX,posY,or,pNew);
					
			}


		}

}
/////////////////////////////////////////////////////
// Find object next to pMapMainObject 
/////////////////////////////////////////////////////
CMapMainObject * CGameMap::GetNextMainObject( CMapMainObject * pMapMainObject, CPoint endWP, MAPPOSITION * pStartPos){
	CPoint pos,real,posWP;
	UINT i;
	float dist,mindist;

	CMapMainObject * _pMMO,* pMMO;
	if(pStartPos){
		pos.SetPoint(pStartPos->x,pStartPos->y);
	}
	else{
		pos = pMapMainObject->GetCorner();
	}	
	real = PosToReal(pos) + endWP;
	posWP = RealToPos(real);
	
	pMMO = NULL;
	mindist = TERRAIN_PLATE_WIDTH;
	for(i = 0; i < 4; i++){
		switch(i){
			case 0:
				_pMMO = getMainObjectInPos(posWP.x, posWP.y+1);
				break;
			case 1:
				_pMMO = getMainObjectInPos(posWP.x+1, posWP.y);
				break;
			case 2:
				_pMMO = getMainObjectInPos(posWP.x, posWP.y-1);
				break;
			case 3:
				_pMMO = getMainObjectInPos(posWP.x-1, posWP.y);
				break;
		}
		if(_pMMO && _pMMO != pMapMainObject){
			dist=_pMMO->GetNearestWaypointDistance(real);
			if(dist>=0 && dist < (TERRAIN_PLATE_WIDTH/2)){
				if(dist < mindist){
					mindist = dist;	
					pMMO = _pMMO;
				}

			}
		}
	}
	return pMMO;

}
///////////////////////////////////////////////////////////
// Find Path 
// pMapMainObject selected object
// absolut position last waypoint;
HRESULT CGameMap::FindPath(CMapMainObject * pMapMainObject,CPoint waypoint ){
	PATH _vPath;
	CMapMainObject * _pMMO;
	CPoint awp,pos;
	int wsID;
	WAYPOINT wp;
	
	_pMMO = pMapMainObject;
	awp=  waypoint;

	while(_pMMO!=NULL && _pMMO->pMainObject!=NULL 
		&& (_vPath.size()==0 || !_pMMO->pathset)){
		if(GetObjectOnPath(-1,-1) == _pMMO) break;
		wsID = _pMMO->GetNearestWaypointID(awp);
		if(wsID ==-2){
			_pMMO->pathset=TRUE;
			_vPath.push_back(_pMMO);
			break;
		}
		_pMMO->pathset=TRUE;
		_vPath.push_back(_pMMO);
		wp=_pMMO->pMainObject->GetWaypoint(wsID,-1,_pMMO->objOrient);
		awp.x = (int)wp.x;
		awp.y = (int)wp.z;
		awp += PosToReal(_pMMO->GetCorner());
		_pMMO = GetNextMainObject(_pMMO,CPoint((int)wp.x,(int)wp.z));
		
	
	}
	if(_vPath.size())
		vPath.push_back(_vPath);
	return ERRNOERROR;
}

CPoint CGameMap::RealToPos(CPoint real){
	CPoint out;
	out.x = (int)(real.x /(TERRAIN_PLATE_WIDTH));
	out.y = (int)(real.y /(TERRAIN_PLATE_WIDTH));
	return out;
}
CPoint CGameMap::PosToReal(CPoint pos){
	CPoint out;
	out.x = (int)(pos.x * TERRAIN_PLATE_WIDTH);
	out.y = (int)(pos.y * TERRAIN_PLATE_WIDTH);
	return out;
}

CMapMainObject * CGameMap::GetObjectOnPath(int setID,int objID){
		if(setID==-1) setID = (int)vPath.size()-1;
		if((int)vPath.size() <= setID || setID < 0) return NULL;
		if(objID==-1) objID = (int)vPath[setID].size()-1;
		if((int)vPath[setID].size() <= objID || objID < 0) return NULL;
		else return vPath[setID][objID];
}
HRESULT CGameMap::FindPathFromStart(){

	CPoint wp;
	CMapMainObject* _pMMO;
	UINT i,j;
	if(! pMOStartPosition) return ERR_NOSTART;

	vPath.clear();
	for(i=0;i<sizeY;i++)
		for(j=0;j<sizeX;j++){
			_pMMO = getMainObjectInPos(j,i);
			if(_pMMO) _pMMO->pathset=0;

		}

	wp.y=0;
	switch(posStart.or){
		case 0:
			wp.x=(int)TERRAIN_PLATE_WIDTH/2-1;
			wp.y=(int)TERRAIN_PLATE_WIDTH-1;
			break;
		case 1:
			wp.x=(int)TERRAIN_PLATE_WIDTH-1;
			wp.y=(int)TERRAIN_PLATE_WIDTH/2-1;
			break;
		case 2:
			wp.x=(int)TERRAIN_PLATE_WIDTH/2-1;
			wp.y=0;
			break;
		case 3:
			wp.x=0;
			wp.y=(int)TERRAIN_PLATE_WIDTH/2-1;
			break;

	}

	_pMMO = GetNextMainObject(getMainObjectInPos(posStart.x,posStart.y),wp,&posStart);
	if(!_pMMO) return  ERR_NOWAYPOINT;
	wp += PosToReal(CPoint(posStart.x,posStart.y));
	FindPath(_pMMO,wp);
	return ERRNOERROR;
}

HRESULT CGameMap::FindPathFromPos(CPoint pos){
	CMapMainObject *pMMO,*pLMMO;
	CPoint rp;
	pMMO = getMainObjectInPos(pos.x,pos.y);
	if(!pMMO || !pMMO->pMainObject || !pMMO->pMainObject->vAllWayPoints.size()) return 1;
	pLMMO = GetObjectOnPath(-1,-1);
	if(!pLMMO || !pLMMO->pMainObject){
		if(pMOStartPosition){
			rp.x=posStart.x;rp.y=posStart.y;
			rp = PosToReal(rp)+ CPoint((int)(TERRAIN_PLATE_WIDTH/2),(int)(TERRAIN_PLATE_WIDTH/2));
		}
	}else{
		rp = PosToReal(pLMMO->GetCorner())+ CPoint((int)(pLMMO->pMainObject->size*TERRAIN_PLATE_WIDTH/2),(int)(pLMMO->pMainObject->size*TERRAIN_PLATE_WIDTH/2));
	}
	FindPath(pMMO,rp);

	return ERRNOERROR;
}
HRESULT CGameMap::FindPathBack(CPoint pos){
	CMapMainObject *pMMO;
	int setID,objID,i,j;
	CPoint rp;
	pMMO = getMainObjectInPos(pos.x,pos.y);
	setID = -1;
	objID = -1;
	if(!pMMO || !pMMO->pathset) return 1;
	for(i = 0; i < (int)vPath.size();i++)
		for(j=0;j< (int)vPath[i].size();j++){
			if(GetObjectOnPath(i,j))GetObjectOnPath(i,j)->pathset=0;
			if(GetObjectOnPath(i,j)==pMMO) {
				setID=i;
				objID=j;
			}
		}
	
	if(setID!=-1 && objID !=-1){
		for(i = (int)vPath.size()-1;i > setID ;i--){
			vPath.pop_back();
		}
		for(j = (int)vPath[setID].size(); j > objID;j --){
			vPath[setID].pop_back();
			if(vPath[setID].size()==0)vPath.pop_back();
		}
	}
	for(i = 0; i < (int)vPath.size();i++)
		for(j=0;j< (int)vPath[i].size();j++){
			if(GetObjectOnPath(i,j))GetObjectOnPath(i,j)->pathset=1;
		}
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////
// CMapMainObject
/////////////////////////////////////////////////////////////////////
CPoint CMapMainObject::GetCorner(){
	CPoint out;
	if(pMainObject)
		out.SetPoint(posX - pMainObject->getShift(),posY - pMainObject->getShift());
	else out.SetPoint(posX,posY);
	return out;
}
float CMapMainObject::GetNearestWaypointDistance(CPoint pointPos){
	WAYPOINT wp;
	CPoint realwp,pos;
	UINT i;
	float dist, mindist;
	int ret=-1;
	if(!pMainObject) return -1;
	mindist = -1;
	for(i = 0; i < pMainObject->GetWaypointSetSize();i++){
		wp = pMainObject->GetWaypoint(i,0,objOrient);
		if(wp.x!=WAYPOINT_ERR){
			realwp.SetPoint((int)wp.x,(int)wp.z);
			pos = GetCorner();
			realwp += CGameMap::PosToReal(pos);
			dist = (float)((realwp.x-pointPos.x)*(realwp.x-pointPos.x) + (realwp.y-pointPos.y)*(realwp.y-pointPos.y));
			if(mindist == -1) mindist = dist;
			else if (mindist > dist ) mindist = dist;
		}
	}


	return  sqrt(mindist); 
}
int CMapMainObject::GetNearestWaypointID(CPoint pointPos){
	
	WAYPOINT wp;
	CPoint realwp,pos;
	UINT i,count,id;
	float dist, mindist;
	if(!pMainObject)return -1;
	mindist = -1;
	id = -1;
	count=0;
	for(i = 0; i < pMainObject->GetWaypointSetSize();i++){
		wp = pMainObject->GetWaypoint(i,0,objOrient);
		if(wp.x!=WAYPOINT_ERR){
			realwp.SetPoint((int)wp.x,(int)wp.z);
			pos = GetCorner();
			realwp += CGameMap::PosToReal(pos);
			dist = (float)((realwp.x-pointPos.x)*(realwp.x-pointPos.x) + (realwp.y-pointPos.y)*(realwp.y-pointPos.y));
			if(mindist == -1) {id=i;mindist = dist;count=1;}
			else if (mindist > dist ){ id=i;mindist = dist;count=1;}
			else if(mindist == dist) count++;
		}
	}

	return  count==1 ? id : -2; 
}

//////////////////////////////////////////////////////////////////////
// CGameObject
/////////////////////////////////////////////////////////////////////

void CGameObject::setData(CResourceManager *pRM,GameObject * GO){
	
	 UINT i;
	 //LightObject			* pLObj;
	 CObjectLight			pomObj;
	 posX=GO->posX;
	 posY=GO->posY;
	 x=GO->x;
	 y=GO->y;
	 z=GO->z;
	 bPowered=GO->bPowered;
	 bRotated=GO->bRotated;
	 cogX=GO->cogX;
	 cogY=GO->cogY;
	 cogZ=GO->cogZ;
	 isStatic=GO->isStatic;
	 mass=GO->mass;
	 material=GO->material;
	 wheel_material=GO->wheel_material;
	 model=*(GO->psModel);
	 moiX = GO->moiX;
	 moiY = GO->moiY;
	 moiZ = GO->moiZ;
	 rotationX = GO->rotationX;
	 rotationY = GO->rotationY;
	 rotationZ = GO->rotationZ;
	 sizeX = GO->sizeX;
	 sizeY = GO->sizeY;
	 vTextures.clear();
	 vPhTextures.clear();
	 vLightNames.clear();
	 bGrassAllowed=GO->bGrassAllowed;
	 for(i = 0;i < GO->pvLightObjectFileNames->size();i++)
		 vTextures.push_back(GO->pvLightObjectFileNames->at(i));
	 for(i = 0;i < GO->pvPhTextureFileNames->size();i++)
		vPhTextures.push_back(GO->pvPhTextureFileNames->at(i));
	 for(i = 0;i < GO->pvTextureFileNames->size();i++)
		 vLightNames.push_back(GO->pvTextureFileNames->at(i));		
/*	 for(i=0; i < GO->pvLightObjectIDs->size(); i++){
		  pLObj=pRM->GetLightObject(GO->pvLightObjectIDs->at(i));
		  
		  pomObj.setData(pLObj);
		  vLight.push_back(pomObj);	
	 }*/
	
}

// Saves game object into file name
// Type:
// 0 - Object
// 1 - Road
// 2 - Car
// 3 - Wheel
//HRESULT CGameObject::SaveToFile(CString FileName, int Type, BOOL Append)
//{
//	UINT i;
//	FILE * File;
//	errno_t Result;
//	HRESULT hr=ERRNOERROR;
//	CString objectspath=CONFIG_OBJECTSPATH;
//	CString lightspath=CONFIG_LIGHTSPATH;
//	if (Append)
//		Result = fopen_s(&File, objectspath+FileName, "a");
//	else
//		Result = fopen_s(&File, objectspath+FileName, "w");
//
//	if (!Result)
//		ERRORMSG(ERRFILENOTFOUND, "CGameObject::SaveToFile()", CString("Unable to save game object filename:") + objectspath + FileName);
//
//	// Object
//	if (Type == 0)
//	{
//		fprintf(File, "[OBJECT]\n");
//		fprintf(File, "SizeX=%i\n", sizeX);
//		fprintf(File, "SizeY=%i\n", sizeY);
//		fprintf(File, "Model=%s\n", model);
//		fprintf(File, "PosX=%i\n", posX);
//		fprintf(File, "PosY=%i\n", posY);
//		fprintf(File, "x=%f\n", x);
//		fprintf(File, "y=%f\n", y);
//		fprintf(File, "z=%f\n", z);
//		fprintf(File, "Rotation.x=%f\n", rotationX);
//		fprintf(File, "Rotation.y=%f\n", rotationY);
//		fprintf(File, "Rotation.z=%f\n", rotationZ);
//		fprintf(File, "Scale.x=%f\n", scaleX);
//		fprintf(File, "Scale.y=%f\n", scaleY);
//		fprintf(File, "Scale.z=%f\n", scaleZ);
//
///*		for (i = 0; i < min(vLightNames.size(), vLight.size() ); i++)
//		{
//			// Save light into file
//			hr = vLight[i].SaveToFile(vLightNames[i]);
//			if (!hr)
//			{
//				fclose(File);
//				ERRORMSG(hr, "CGameObject::SaveToFile()", CString("Unable to save light filename: ") + lightspath + vLightNames[i]);
//			}
//			fprintf(File, "Light=%s\n", vLightNames[i]);
//		}*/
//
//		fprintf(File, "\n");
//	}
//	// road
//	else if (Type == 1)
//	{
//		fprintf(File, "[ROAD]\n");
//		fprintf(File, "SizeX=%i\n", sizeX);
//		fprintf(File, "SizeY=%i\n", sizeY);
//
//		for (i = 0; i < vTextures.size(); i++)
//			fprintf(File, "Texture=%s\n", vTextures[i]);
//
//		fprintf(File, "x=%f\n", x);
//		fprintf(File, "y=%f\n\n", y);
//	}
//	// car
//	else if (Type == 2)
//	{
//		fprintf(File, "[CAR]\n");
//		fprintf(File, "Model=%s\n", model);
//		fprintf(File, "x=%f\n", x);
//		fprintf(File, "y=%f\n", y);
//		fprintf(File, "z=%f\n", z);
//		fprintf(File, "Scale.x=%f\n", scaleX);
//		fprintf(File, "Scale.y=%f\n", scaleY);
//		fprintf(File, "Scale.z=%f\n", scaleZ);
//		fprintf(File, "Rotation.x=%f\n", rotationX);
//		fprintf(File, "Rotation.y=%f\n", rotationY);
//		fprintf(File, "Rotation.z=%f\n", rotationZ);
//		fprintf(File, "Cog.x=%f\n", cogX);
//		fprintf(File, "Cog.y=%f\n", cogY);
//		fprintf(File, "Cog.z=%f\n", cogZ);
//		fprintf(File, "Mass=%f\n", mass);
//		fprintf(File, "Moi.x=%f\n", moiX);
//		fprintf(File, "Moi.y=%f\n", moiY);
//		fprintf(File, "Moi.z=%f\n", moiZ);
//		fprintf(File, "Material=%i\n", material);
//		fprintf(File, "Wheel_Material=%i\n", wheel_material);
//	/*	if (vLightNames.size() >= 1 && vLight.size() >= 1)
//		{
//			hr = vLight[0].SaveToFile(vLightNames[0]);
//			if (!hr)
//			{
//				fclose(File);
//				ERRORMSG(hr, "CGameObject::SaveToFile()", CString("Unable to save light filename: ") + lightspath + vLightNames[0]);
//			}
//			fprintf(File, "Light=%s\n\n", vLightNames[0]);
//		}*/
//	}
//	// wheel
//	else if (Type == 3)
//	{
//		fprintf(File, "[WHEEL]\n");
//		fprintf(File, "Model=%s\n", model);
//		fprintf(File, "x=%f\n", x);
//		fprintf(File, "y=%f\n", y);
//		fprintf(File, "z=%f\n", z);
//		fprintf(File, "Scale.x=%f\n", scaleX);
//		fprintf(File, "Scale.y=%f\n", scaleY);
//		fprintf(File, "Scale.z=%f\n", scaleZ);
//		fprintf(File, "Rotation.x=%f\n", rotationX);
//		fprintf(File, "Rotation.y=%f\n", rotationY);
//		fprintf(File, "Rotation.z=%f\n", rotationZ);
//		fprintf(File, "Powered=%i\n", bPowered);
//		fprintf(File, "Rotated=%i\n", bRotated);
//	}
//
//	fclose(File);
//
//	return hr;
//}

void CObjectLight::setData(LightObject * LO){
	Ambient=LO->Ambient;
	Diffuse=LO->Diffuse;
	directionX=LO->directionX;
	directionY=LO->directionY;
	directionZ=LO->directionZ;
	x=LO->x;
	y=LO->y;
	z=LO->z;
	Falloff=LO->Falloff;
	Phi=LO->Phi;
	Range=LO->Range;
	Specular=LO->Specular;
	type=LO->type;
	Theta=LO->Theta;
	Attenuation0=LO->Attenuation0;
	Attenuation1=LO->Attenuation1;
	Attenuation2=LO->Attenuation2;

}

//// Saves light object into file
//HRESULT CObjectLight::SaveToFile(CString FileName)
//{
//	FILE * File;
//	errno_t Result;
//	CString lightspath=CONFIG_LIGHTSPATH;
//	Result = fopen_s(&File, lightspath + FileName, "w");
//	
//	if (!Result)
//		ERRORMSG(ERRFILENOTFOUND, "CObjectLight::SaveToFile()", CString("Unable to create (or open) light file for writing, filename: ") + lightspath + FileName);
//
//	fprintf(File, "Direction.x=%f\n", directionX);
//	fprintf(File, "Direction.y=%f\n", directionY);
//	fprintf(File, "Direction.z=%f\n", directionZ);
//	fprintf(File, "x=%f\n", x);
//	fprintf(File, "y=%f\n", y);
//	fprintf(File, "z=%f\n", z);
//	fprintf(File, "AttenuationConstant=%f\n", Attenuation0);
//	fprintf(File, "AttenuationLinear=%f\n", Attenuation1);
//	fprintf(File, "AttenuationQuadratic=%f\n", Attenuation2);
//	fprintf(File, "OuterConeAngle=%f\n", Phi);
//	fprintf(File, "InnerConeAngle=%f\n", Theta);
//	fprintf(File, "Falloff=%f\n", Falloff);
//	fprintf(File, "Range=%f\n", Range);
//	fprintf(File, "Type=%u\n", type);
//	fprintf(File, "Diffuse=%x\n", Diffuse);
//	fprintf(File, "Specular=%x\n", Specular);
//	fprintf(File, "Ambient=%x\n", Ambient);
//
//	fclose(File);
//
//	return ERRNOERROR;
//}

// Saves main object and dependent objects into files
//HRESULT CMainObject::SaveToFile(CStringA FileName)
//{
//	UINT i;
//	FILE * File;
//	errno_t Result;
//	HRESULT hr;
//	CString objectspath=CONFIG_OBJECTSPATH;
//	Result = fopen_s(&File, objectspath + FileName, "w");
//
//	if (!Result)
//		ERRORMSG(ERRFILENOTFOUND, "CMainObject::SaveToFile()", CString("Unable to create (or open) main object file for writing, filename: ") + objectspath+ FileName);
//
//	fclose(File);
//
//	for (i = 0; i < obj.size(); i++)
//	{
//		hr = obj[i].SaveToFile(FileName, 0);
//		if (!hr) ERRORMSG(hr, "CMainObject::SaveToFile()", CString("Unable to save [object] section into main object filename: ") + objectspath+ FileName);
//	}
//
//	for (i = 0; i < roads.size(); i++)
//	{
//		hr = roads[i].SaveToFile(FileName, 1);
//		if (!hr) ERRORMSG(hr, "CMainObject::SaveToFile()", CString("Unable to save [road] section into main object filename: ") + objectspath + FileName);
//	}
//
//	return ERRNOERROR;
//}

int CGameMap::GetRandomTexture(){
	int sum,i,res;
	int last,next;
	sum=0;
	for(i = 0; i < (int)vTextureSet.size();i++){
		sum+=vTextureSet[i].probability;
	}
	res = rand() % sum;
	
	last = 0;next =0;
	for(i = 0; i < (int)vTextureSet.size();i++){
		next+=vTextureSet[i].probability;
		if ( last <= res &&   res<next)
			return i;
		last = next;
	}

	return 0;
}
//////////////////////////////////////////////////////////
// EDITOR TREE
/////////////////////////////////////////////////////////


void CALLBACK CEditorTree::LoadCallBackStaticTree( NAMEVALUEPAIR * nameValue){
	CEditorTree *res; 
	res = (CEditorTree *)nameValue->ReservedPointer;
	res->LoadCallBackTree(nameValue);
};
CMainObject * CEditorTree::LoadMainObject(CString name){
	CMainObject * pMainObj;
	HRESULT hr;
	CString path;
	int num;
	void * point;
	pMainObj = new CMainObject();
	if(!pMainObj) return NULL;
	hr = pMainObj->loadFromFile(name,pResourceManager);
	if (hr){
		SAFE_DELETE(pMainObj);
	}
	else {
		path=CONFIG_EDITORPATH;
		path+=pMainObj->sIconName;
		hr = pMainObj->loadImage(path);
		
		pMainObj->getFileName();
		
		if(hr){
			if( !mapIconCounter.Lookup(pMainObj->getFileName(),point)){
				num=iIconCounter++;
				mapIconCounter[pMainObj->getFileName()]=IntToPtr(num);
			}else num = PtrToInt(point);
			pMainObj->CreateCountedImage(num);
		}
	
			if(!pMainObj || pMainObj->sName=="") {
				pMainObj->sName=name;
				pMainObj->sName.Delete((int)(pMainObj->sName.GetLength()-strlen(OBJECT_EXT)),(int)strlen(OBJECT_EXT));
			} 

	};
	return pMainObj;
};

void CEditorTree::LoadCallBackTree(NAMEVALUEPAIR * nameValue){
	

	CMainObject * pMainObj=NULL;
	CString path,value,nm;
	void * pointer;
	value=nameValue->GetName();
//	FILE *f;
	if((nameValue->GetFlags() == NVPTYPE_LABEL)){
				
		if(!pObjectsMap->Lookup(value,pointer)){
				pMainObj=LoadMainObject(value);

				//pTreeCtrl->InsertItem(&(nameValue->GetName())
				if(pMainObj){
					
					(* pObjectsMap)[value]=pMainObj;
					//if(pMainObj->
					
					actItem = pTreeCtrl->InsertItem(pMainObj->sName,
						(HTREEITEM )hPa.GetAt(hPa.GetUpperBound())
						,TVI_SORT);
			
					pTreeCtrl->SetItemData(actItem,PtrToUlong(pMainObj));		
				}
		}
			
	};

	if(nameValue->GetFlags() == NVPTYPE_TAGBEGIN){
	
		hPa.Add( pTreeCtrl->InsertItem(nameValue->GetName(),0,0,
								(HTREEITEM) hPa.GetAt(hPa.GetUpperBound())
								,NULL));
	
	}
	if(nameValue->GetFlags() == NVPTYPE_TAGEND){
		HTREEITEM po;
		po = (HTREEITEM) hPa.GetAt(hPa.GetUpperBound());
		hPa.RemoveAt(hPa.GetUpperBound());
	};
		
		

}
void CEditorTree::Init(CResourceManager * _pResourceManager,CMapStringToPtr * _pObjectsMap){
	pResourceManager = _pResourceManager;
	pObjectsMap = _pObjectsMap;
}

HRESULT CEditorTree::deleteItemFromTree(HTREEITEM hItem){
	HRESULT hr = ERRNOERROR;
	pTreeCtrl->DeleteItem(hItem);
	//pTreeCtrl->get
	//objectsMap.RemoveKey(
	return hr;
};

HRESULT CEditorTree::LoadTree(CString file){
	
	hPa.Add(NULL);

	NAMEVALUEPAIR vlpair;
	
	return vlpair.Load(file,LoadCallBackStaticTree,this);
}

HRESULT CEditorTree::SaveTree(CString file){
	
	FILE *f;
	HTREEITEM item;
	errno_t err = fopen_s(&f,file,"w");
	if(err) return err;
	item = pTreeCtrl->GetRootItem();
	SaveChild(item,f);
	fclose(f);
	return ERRNOERROR;
}
void CEditorTree::SaveChild(HTREEITEM hRoot, FILE *f){
	
	CString sRootName;
	CMainObject * pObj;
	CString sFileName;
	HTREEITEM hItem;
	
	sRootName=pTreeCtrl->GetItemText(hRoot);
	fprintf(f,"<%s>\n",sRootName.GetString());
	
	hItem=pTreeCtrl->GetNextItem(hRoot,TVGN_CHILD);
	
	do{
		
		if(pTreeCtrl->ItemHasChildren(hItem)) {
			SaveChild(hItem,f);
			//pTreeCtrl->Is
		}else{
			pObj = (CMainObject *) pTreeCtrl->GetItemData(hItem);
			if(pObj){
				sFileName = pObj->getFileName();
				fprintf(f,"[%s]\n",sFileName.GetString());
			}
		}

	}
	while(hItem=pTreeCtrl->GetNextItem(hItem,TVGN_NEXT));
	fprintf(f,"</%s>\n",sRootName.GetString());
};

/////////////////////////////////////////////////////////
//CMapTree
/////////////////////////////////////////////////////////
void CMapTreeCtrl::ChangePointer(HTREEITEM hItem,DWORD_PTR pOld, DWORD_PTR pNew){
	
        HTREEITEM hChild;
        hChild = GetChildItem(hItem);
		if (GetItemData(hItem) && GetItemData(hItem) == pOld)
			SetItemData(hItem,pNew);
        while( hChild != NULL)
        {
                // recursively transfer all the items
                ChangePointer( hChild,pOld,pNew );
				hChild = GetNextSiblingItem( hChild );
		}
  

}
HTREEITEM CMapTreeCtrl::CopyItem( HTREEITEM hItem, HTREEITEM htiNewParent, 
                                        HTREEITEM htiAfter /*= TVI_LAST*/ )
{
        TV_INSERTSTRUCT         tvstruct;
        HTREEITEM                       hNewItem;
        CString                         sText;

        // get information of the source item
        tvstruct.item.hItem = hItem;
        tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE | 
                                TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        GetItem(&tvstruct.item);  
        sText = GetItemText( hItem );
        
        tvstruct.item.cchTextMax = sText.GetLength();
        tvstruct.item.pszText = sText.LockBuffer();

        // Insert the item at proper location
        tvstruct.hParent = htiNewParent;
        tvstruct.hInsertAfter = htiAfter;
        tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
        hNewItem = InsertItem(&tvstruct);
        sText.ReleaseBuffer();

        // Now copy item data and item state.
        SetItemData( hNewItem, GetItemData( hItem ));
        SetItemState( hNewItem, GetItemState( hItem, TVIS_STATEIMAGEMASK ), 
                                                        TVIS_STATEIMAGEMASK );

        // Call virtual function to allow further processing in derived class
     //   OnItemCopied( hItem, hNewItem );

        return hNewItem;
}

HTREEITEM CMapTreeCtrl::CopyBranch( HTREEITEM htiBranch, HTREEITEM htiNewParent, 
                                                HTREEITEM htiAfter /*= TVI_LAST*/ )
{
        HTREEITEM hChild;

        HTREEITEM hNewItem = CopyItem( htiBranch, htiNewParent, htiAfter );
        hChild = GetChildItem(htiBranch);
        while( hChild != NULL)
        {
                // recursively transfer all the items
                CopyBranch(hChild, hNewItem);  
                hChild = GetNextSiblingItem( hChild );
        }
        return hNewItem;
}

HRESULT CEditorTree::AddItem(HTREEITEM _hParent, CMainObject * _pMainObj){
	HRESULT hr=ERRNOERROR;
	CString fname;	
	HTREEITEM actual=NULL;
	if(_pMainObj==NULL)
		actual = pTreeCtrl->InsertItem("New",_hParent,TVI_SORT);
		if (actual) pTreeCtrl->SetItemData(actual,NULL);
		
	else{
		actual = pTreeCtrl->InsertItem(_pMainObj->sName,_hParent,TVI_SORT);	
		if(actual) pTreeCtrl->SetItemData(actual,PtrToUlong( _pMainObj));
	};
	return hr;


}

HRESULT CEditorTree::AddNewItem(CMainObject *pMainObject){
	HRESULT hr=ERRNOERROR;
	HTREEITEM treeItem=pTreeCtrl->GetRootItem();
	AddItem(treeItem,pMainObject);
//	pMainObject->CreateCountedImage(iIconCounter++);
	return hr;
}


