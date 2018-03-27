#pragma once

#include <atlimage.h>
#include <vector>
#include  "..\globals\globals.h"
#include  "..\globals\NameValuePair.h"
#include "EditorProperties.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\GameMods\GameMod.h"
#include "..\Globals\Base64Coder.h"


//#define TEST_BORDER(i,j) i >=0 && i<MAX_MAP_SIZE && j >=0 && j<MAX_MAP_SIZE
// y x
#define TEST_BORDER(i,j) i >=0 && i<sizeY && j >=0 && j< sizeX

class CEditorTree;
class CMapMainObject;
enum ORIENTATION{
	up = 0,
	right = 1,
	down = 2,
	left = 3

};


typedef enum __SideType { top = 0, middle, bottom, curved} SideType;
typedef enum __PlaneType {outercorner = 0, innercorner, hillside } PlaneType;



struct DIRECTLIGHT{
	unsigned int color; 
	float x,y,z,dx,dy,dz;
};

struct MAPPOSITION{
	UINT x;
	UINT y;
	ORIENTATION or;
};
struct REALPOSITION{
	float x;
	float y;
	ORIENTATION or;
};

struct WAYPOINT{
	float x;
	float y;
	float z;
};


typedef std::vector<WAYPOINT> WAYPOINTS;
typedef std::vector<CMapMainObject *> PATH;

class CObjectLight{
	public:
		CObjectLight(){ clear();	}
		void clear(){	x=0;y=0;z=0;
						directionX=0;directionY=0;directionZ=0;
						Attenuation0=0;Attenuation1=1;Attenuation2=0;
						Phi=DEG90;Theta=DEG45;Falloff=1;Range=50000;
						Diffuse=0xffffffff;Specular=0;Ambient=0xffffffff;type=0;}

		float x,y,z;
		float directionX,directionY,directionZ;
		float Attenuation0, Attenuation1, Attenuation2;
		float Phi,Theta,Falloff,Range;
		unsigned int Diffuse,Specular,Ambient;
		unsigned int type;
		void setData(resManNS::LightObject *);
		//FFU HRESULT SaveToFile(CString FileName); // Saves light object into file
};

class CGameObject{
		
		
	public:

		CGameObject(){ clear();	}
		void setData(resManNS::CResourceManager *,resManNS::GameObject *);
		void clear(){x=0;y=0;z=0;posX=0;posY=0;sizeX=1;sizeY=1;bGrassAllowed=0;
					model="";/*vLight.clear();*/vTextures.clear();vPhTextures.clear();
					vLightNames.clear();
					rotationX=0;rotationY=0;rotationZ=0;
					scaleX=1;scaleY=1;scaleZ=1;
					moiX=0;moiY=0;moiZ=0;
					cogX=0;cogY=0;cogZ=0;
					radius=0;
					material=0;
					wheel_material=0;
					}

		//FFU HRESULT SaveToFile(CString FileName, int Type, BOOL Append = TRUE); // Saves game object into file name
		
		int posX,posY;
		UINT sizeX,sizeY;
		float x,y,z;
		float rotationX,rotationY,rotationZ;
		float scaleX,scaleY,scaleZ;
		float mass;
		float moiX,moiY,moiZ;
		float cogX,cogY,cogZ;
		float radius;
		BYTE material;
		BYTE wheel_material;

		BOOL bPowered,bRotated,isStatic,bGrassAllowed;
		
	
		CString model,lightFile,pomString;
		//std::vector<CObjectLight> vLight;
		std::vector<CString > vTextures;
		std::vector<CString > vPhTextures;
		std::vector<CString > vLightNames;
			
};


class CMainObject{
	

	public:
		CMainObject();
		~CMainObject() { if(pImage) SAFE_DELETE(pImage)};
		void clear();
		CString getFileName(){ return objectFile;};
		UINT getSize()			{ return size;};
		int getShift()			{return (size-1)/2;};
		int getSizeN()			{return (size-1)/2;};
		int getSizeP()			{return (size-1)-(size-1)/2;};
		void init(CString _objectFile){objectFile=_objectFile;};

	private:
		CString objectFile;
	//	CString lightPath,file;
		CGameObject * aktObj;
		CGameObject pomObj;
			

		BYTE mask[MAX_OBJECT_SIZE][MAX_OBJECT_SIZE];
		BYTE grassMask[MAX_OBJECT_SIZE][MAX_OBJECT_SIZE];
		UINT maskSize;

	public:
		CString sName;
		CString sIconName;
		BOOL bStartingPosition;
		UINT size,sizeX,sizeY; //size ctvercova velikost, 
		//UINT centerX,centerY, centerSizeX,centerSizeY;
		std::vector<CGameObject> obj;
		std::vector<CGameObject> roads;
		std::vector<WAYPOINTS> vAllWayPoints; 
		CImage * pImage;
		
	private:
		HRESULT setSizeAndMask();
	

	public:
		void CreateCountedImage(int number);
		int getMaskValue(UINT x,UINT y,ORIENTATION or);
		int getGrassMaskValue(UINT x, UINT y, ORIENTATION or);
		WAYPOINT GetWaypoint(UINT WayPointSetID,int WayPointID, ORIENTATION=up);
		UINT GetWaypointSetSize();
		UINT GetWaypointSize(UINT WayPointSetID);
		HRESULT loadFromFile(CStringA,resManNS::CResourceManager *);
		//FFU HRESULT SaveToFile(CStringA FileName); // Saves main object and dependent objects into files
		HRESULT loadImage(CStringA);
};

class CMapMainObject{
	public:
		CMapMainObject(){posX = 11111;posY=11111;painted = false;counted=false;saved=false;pMainObject=NULL;pathset=false;};
		CMainObject * pMainObject;

		//UINT cPosX,cPosY; //position of left bottom conner 
		CPoint GetCorner();
		float GetNearestWaypointDistance(CPoint pointPos);
		int GetNearestWaypointID(CPoint waypoint);
		UINT posX,posY;	//position of left bottom conner 
		ORIENTATION objOrient;
		BOOL painted;
		BOOL saved;
		BOOL counted;
		BOOL pathset;
		int index;
		bool bSpecialProperties; // indicates special MOD properties defined for this instance
		resManNS::OBJECTMODDATALIST SpecialProperties; // 'MOD properties' associated with this object's instance
};

struct CMapObject {
	
	public:
		
		CMapObject();
		CMapMainObject * pMapMainObj;
		UINT alt; 
		ORIENTATION terOrient;
		ORIENTATION texOrient;
		UINT textInd;
		UINT textBaseColor;
		UINT physInd;
		int type;
		void clear();
		//ORIENTATION terrainOri
		
};

struct TEXTURESET{
	TEXTURESET(CString _str,UINT _prob){sTexture =_str;probability=_prob;}
	UINT probability;
	CString sTexture;
};
class CGameMap
{
	
public: //public variables
	UINT sizeX, sizeY;	
	CString sTerrain; //name of terrain
	CString sName; //name of map
	CString sFileName;
	CString sSkySystem;	//name of skybox
	CString sTextureSet;
	CString sGrassType;
	CString sWallSet;
	CString csImageFileName;
	CString csDescription;
	UINT uiFogColor;
	MAPPOSITION posStart;
	CMainObject *pMOStartPosition;
	UINT uiHours,uiMinutes;
	std::vector< PATH >vPath;
	std::vector<CString> vGameMods;

private: //private variables
	resManNS::CResourceManager * pRM;
	CMapStringToPtr * pObjectsMap;
	CEditorTree		* pEditorTree;

	UINT uiAmbColor;
	UINT TextSum;
	std::vector< DIRECTLIGHT> vDirectLight; 
	std::vector< CString> vPhysTextures;
	std::vector< TEXTURESET > vTextureSet;
	CMapObject pMapObject[MAX_MAP_SIZE][MAX_MAP_SIZE];	//Y X


private://Private functions
	
	int hrana(UINT,UINT);
	int roh(UINT, UINT);
	int rohvnitrek(UINT , UINT );
	int GetTerrainType(PlaneType pt, SideType st1, SideType st2, SideType stDiagonal = middle);
	void GetPlateType(int n,SideType &st,PlaneType &pt,int &or);
	void ClearObjectOnPosition(UINT x,UINT y);
	int GetRandomTexture();

public://Public functions
	
	CGameMap(CMapStringToPtr * ptr,resManNS::CResourceManager * pRM,CEditorTree * pEditorTree);
	~CGameMap();
	HRESULT clearMap();
	HRESULT setSize(UINT,UINT); //set size of map and create array;
	HRESULT loadMap(CString fName);
	HRESULT saveMap(CString fName);
	HRESULT loadTerrain(CString fName);
	HRESULT saveTerrain(CString fName);
	HRESULT setStartToMap(UINT x, UINT y, ORIENTATION or, CMainObject *);
	CMapMainObject * setObjectToMap(UINT x, UINT y, ORIENTATION or,CMainObject *);
	void ChangeMainObject(CMainObject * pOld,CMainObject *pNew);
	UINT getAltitude(UINT x, UINT y){ return x < sizeX && y < sizeY ?pMapObject[y][x].alt : 0;}
	int getType(UINT x, UINT y){ return x < sizeX && y < sizeY ?pMapObject[y][x].type : -1;}
	void setAltitude(UINT x, UINT y,int alt){if(x < sizeX && y < sizeY) pMapObject[y][x].alt=alt;}
	CImage * getImage(UINT x,UINT y) { return  x < sizeX && y < sizeY && pMapObject[y][x].pMapMainObj ? pMapObject[y][x].pMapMainObj->pMainObject->pImage : NULL;}
	UINT getObjectSize(UINT x,UINT y) { return  x < sizeX && y < sizeY && pMapObject[y][x].pMapMainObj ? pMapObject[y][x].pMapMainObj->pMainObject->getSize(): 0;};	
	ORIENTATION getObjectOrientation(UINT x, UINT y) { return  x < sizeX && y < sizeY && pMapObject[y][x].pMapMainObj ? pMapObject[y][x].pMapMainObj->objOrient: up;};
	UINT getObjectPosX(UINT x,UINT y){  return  x < sizeX && y < sizeY && pMapObject[y][x].pMapMainObj ? pMapObject[y][x].pMapMainObj->posX: 0;};
	UINT getObjectPosY(UINT x,UINT y){  return  x < sizeX && y < sizeY && pMapObject[y][x].pMapMainObj ? pMapObject[y][x].pMapMainObj->posY: 0;};
	CMapMainObject * GetNextMainObject( CMapMainObject * pMapMainObject,CPoint endRelativeWP,MAPPOSITION * pStartPos =NULL); 
	CMapMainObject * GetObjectOnPath(int setID,int objID);
	HRESULT FindPath(CMapMainObject * pMapMainObject,CPoint absolutPoint);
	HRESULT FindPathFromStart();
	HRESULT FindPathFromPos(CPoint pos);
	HRESULT FindPathBack(CPoint pos);
	int GetPathSize();
	static CPoint RealToPos(CPoint real);
	static CPoint PosToReal(CPoint real);


	CMainObject * deleteObjectFromMap(UINT x,UINT y);
	BOOL isGrass(UINT x, UINT y);
	BOOL isPlaceInMap(UINT x, UINT y, ORIENTATION or,CMainObject * pPomMObj );
//	BOOL isObjectConnerInMap(UINT x,UINT y)	{ return (pMapObject[y][x].pMapMainObj && x == pMapObject[y][x].pMapMainObj->posX && y == pMapObject[y][x].pMapMainObj->posY); };
	BOOL isObjectPainted(UINT x,UINT y) { return  x < sizeX && y < sizeY &&pMapObject[y][x].pMapMainObj ? pMapObject[y][x].pMapMainObj->painted : true; };
	void setPaintedForObject(UINT x, UINT y) { if ( x < sizeX && y < sizeY && pMapObject[y][x].pMapMainObj) pMapObject[y][x].pMapMainObj->painted = true;};
	void unsetPaintedForObject(UINT x, UINT y) { if ( x < sizeX && y < sizeY && pMapObject[y][x].pMapMainObj) pMapObject[y][x].pMapMainObj->painted = false;};

	CMapMainObject * getMainObjectInPos(UINT x, UINT y) { return x < sizeX && y < sizeY ? pMapObject[y][x].pMapMainObj:NULL; };

};

class CMapTreeCtrl : public CTreeCtrl{
	
	public:
		CMapTreeCtrl(){};
		~CMapTreeCtrl(){};
		HTREEITEM CopyItem(HTREEITEM hItem, HTREEITEM htiNewParent, HTREEITEM htiAfter = TVI_LAST );
		HTREEITEM CopyBranch( HTREEITEM htiBranch, HTREEITEM htiNewParent,HTREEITEM htiAfter = TVI_LAST);
		void ChangePointer(HTREEITEM hItem, DWORD_PTR pOld,DWORD_PTR pNew );
};

class CEditorTree{

	public: //public variables
		CMapTreeCtrl   *pTreeCtrl;
		resManNS::CResourceManager * pResourceManager;	
		CMapStringToPtr * pObjectsMap;
		CMapStringToPtr mapIconCounter;
		int iIconCounter;
			
	private: //private variable
		HTREEITEM actItem;
		CPtrArray   hPa;
		


	public: //public functions
		CEditorTree() {pTreeCtrl =NULL;iIconCounter = 1;};
		void Init(resManNS::CResourceManager * pResourceManager,CMapStringToPtr * objectsMap);
		void setTreeCtrl(CMapTreeCtrl * _pTreeCtrl){pTreeCtrl = _pTreeCtrl;};
		CMainObject * LoadMainObject(CString name);
		HRESULT LoadTree(CString file);  // First load of objets Tree 
		HRESULT deleteItemFromTree(HTREEITEM);
		HRESULT AddItem(HTREEITEM hParent,CMainObject * pCMainObject=NULL);
		HRESULT AddNewItem(CMainObject * pMainObject=NULL);
		//HRESULT addNewItem(HTREEITEM hParent);
		//HRESULT addNewGroup();
		//HRESULT cutItem();
		//HRESULT pasteItem();
		HRESULT SaveTree(CString file);

	private: //private functions
		void SaveChild(HTREEITEM hRoot,FILE *f);
		void LoadCallBackTree(NAMEVALUEPAIR *);
		static void CALLBACK LoadCallBackStaticTree(NAMEVALUEPAIR *);

		

};