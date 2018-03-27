#include "Crc32Static.h"
#include <windows.h>
#include <windef.h>
#include <atlstr.h>
#include <d3d9.h>
#include <d3dx9math.h>
#include <xact.h> // sounds
#include <CString>

#include <vector>
#include <set>

#define PHYSICAL_TEXTURE_SIZE           10

// specifies the version of map (.urmap) file this module can read
// this should prevent version conflicts between editor and game
#define MAP_FILE_FORMAT_VERSION			0x00010001


#define RES_NoType              0
#define RES_Texture             1
#define RES_XFile               2//
#define RES_Terrain             3//
#define RES_GrSurface           4//
#define RES_VertexShader        5
#define RES_PixelShader         6
#define RES_Road                7
#define RES_GrPlate             8//
#define RES_PhysicalTexture     9
#define RES_LoDMap             10 
#define RES_DlgDefinition      11 
#define RES_DlgDeclaration     12
#define RES_FontDef            13
#define RES_FilePack           14//
#define RES_Model              15//
#define RES_MemoryXFile        16//
#define RES_MemoryTexture      17
#define RES_Materials          18
#define RES_TSMRemapTable      19
#define RES_GrassItem          20
#define RES_EngineObject       21
#define RES_CarObject          22//
#define RES_MainObject         23//
#define RES_GameObject         24//
#define RES_LightObject        25
#define RES_Map                26//
#define RES_SkySystem          27//
#define RES_Waypoints          28
#define RES_TextureSet         29
#define RES_WaveBank			30 // .xwb file
#define RES_SoundBank			31 // .xsb file
#define RESOURCE_CNT           32

const bool bCheckResourceCRC[RESOURCE_CNT] = \
{ \
    false, /* RES_NoType           */\
    false, /* RES_Texture          */\
    true , /* RES_XFile            */\
    true , /* RES_Terrain          */\
    true , /* RES_GrSurface        */\
    false, /* RES_VertexShader     */\
    false, /* RES_PixelShader      */\
    false, /* RES_Road             */\
    true , /* RES_GrPlate          */\
    false, /* RES_PhysicalTexture  */\
    false, /* RES_LoDMap           */\
    false, /* RES_DlgDefinition    */\
    false, /* RES_DlgDeclaration   */\
    false, /* RES_FontDef          */\
    true , /* RES_FilePack         */\
    true , /* RES_Model            */\
    true , /* RES_MemoryXFile      */\
    false, /* RES_MemoryTexture    */\
    false, /* RES_Materials        */\
    false, /* RES_TSMRemapTable    */\
    true , /* RES_GrassItem        */\
    false, /* RES_EngineObject     */\
    true , /* RES_CarObject        */\
    true , /* RES_MainObject       */\
    true , /* RES_GameObject       */\
    false, /* RES_LightObject      */\
    true , /* RES_Map              */\
    true , /* RES_SkySystem        */\
    false, /* RES_Waypoints        */\
    false, /* RES_TextureSet       */\
    false, /* RES_WaveBank        */\
    false, /* RES_SoundBank       */\
}; 

#define EXPAND(WHAT) \
    WHAT( Texture           )\
    WHAT( XFile             )\
    WHAT( Terrain           )\
    WHAT( GrSurface         )\
    WHAT( VertexShader      )\
    WHAT( PixelShader       )\
    WHAT( Road              )\
    WHAT( GrPlate           )\
    WHAT( PhysicalTexture   )\
    WHAT( LoDMap            )\
    WHAT( FontDef           )\
    WHAT( FilePack          )\
    WHAT( Model             )\
    WHAT( MemoryXFile       )\
    WHAT( MemoryTexture     )\
    WHAT( Materials         )\
    WHAT( TSMRemapTable     )\
    WHAT( GrassItem         )\
    WHAT( EngineObject      )\
    WHAT( CarObject         )\
    WHAT( GameObject        )\
    WHAT( MainObject        )\
    WHAT( LightObject       )\
    WHAT( Map               )\
    WHAT( SkySystem         )\
    WHAT( Waypoints         )\
    WHAT( TextureSet        )\
    WHAT( WaveBank          )\
    WHAT( SoundBank         )

#define RES_NAME_NoType             "NoType"
#define RES_NAME_Texture            "Texture"
#define RES_NAME_XFile              "XFile"
#define RES_NAME_Terrain            "Terrain"
#define RES_NAME_GrSurface          "GrSurface"
#define RES_NAME_VertexShader       "VertexShader"
#define RES_NAME_PixelShader        "PixelShader"
#define RES_NAME_Road               "Road"
#define RES_NAME_GrPlate            "GrPlate"
#define RES_NAME_PhysicalTexture    "PhysicalTexture"
#define RES_NAME_LoDMap             "LoDMap"
#define RES_NAME_FontDef            "FontDef"
#define RES_NAME_FilePack           "FilePack"
#define RES_NAME_Model              "Model"
#define RES_NAME_MemoryXFile        "MemoryXFile"
#define RES_NAME_MemoryTexture      "MemoryTexture"
#define RES_NAME_Materials          "Materials"
#define RES_NAME_TSMRemapTable      "TSMRemapTable"
#define RES_NAME_GrassItem          "GrassItem"
#define RES_NAME_EngineObject       "EngineObject"
#define RES_NAME_CarObject          "CarObject"
#define RES_NAME_MainObject         "MainObject"
#define RES_NAME_GameObject         "GameObject"
#define RES_NAME_LightObject        "LightObject"
#define RES_NAME_Map                "Map"
#define RES_NAME_SkySystem          "SkySystem"
#define RES_NAME_Waypoints          "Waypoints"
#define RES_NAME_TextureSet         "TextureSet"
#define RES_NAME_WaveBank		    "WaveBank"
#define RES_NAME_SoundBank          "SoundBank"


namespace resManNS {

    typedef signed long int RESOURCEID;
    typedef std::vector<RESOURCEID> RVec; // generally a vector of RESOURCEIDs

    typedef struct __Texture {
        LPDIRECT3DTEXTURE9 texture;
    } Texture;

    typedef struct __ShaderLoadParams {
        CAtlString ShaderVersion; // version of shader to compile to
        CAtlString MainName; // Main routine name
        DWORD Flags; // Loading flags
    } ShaderLoadParams;

    typedef struct __VertexShader {
    	IDirect3DVertexShader9 * _VS;
    	ID3DXConstantTable * ConstTable;
    } VertexShader;
    
    typedef struct __PixelShader {
    	IDirect3DPixelShader9 * _PS;
    	ID3DXConstantTable * ConstTable;
    } PixelShader;

    typedef struct __XFile {
        DWORD             cMtrl;
        LPD3DXMESH        Mesh;
        D3DMATERIAL9 *    Material;
        RESOURCEID   *    pTextureIDs; // type: Texture
    } XFile;
    
    typedef struct __GrSurface {
        LPDIRECT3DVERTEXBUFFER9    pVertices;
        LPDIRECT3DINDEXBUFFER9     pIndices;
        UINT                       SegsCountX, SegsCountZ, VertsCount; // count of segments and vertices of plate 
    } GrSurface;
    
    
    typedef struct __TerrainPlateInfo {
        UINT uiHeight;
        UINT uiGrPlateIndex;            // index to pGrPlateIDs array
        UINT uiPlateRotation;
        UINT uiTextureIndex;            // index to pTextureIDs array
        UINT uiPhysicalTextureIndex;    // index to pPhysicalTextureIDs array
        UINT uiTextureRotation;
        UINT uiGrassIndex;              // grass index, no special array of RESOURCEIDs associated
    } TerrainPlateInfo;

    typedef struct __Terrain {
        UINT uiSizeX, uiSizeY;

        CAtlString * sTextureSet;
        
        CAtlString * sWallTextureSet;
        RESOURCEID ridWallTextureSet;

        UINT uiGrassCount;
        CAtlString * pGrassFileNames;           

        UINT uiTextureCount;
        CAtlString * pTextureFileNames;
        RESOURCEID * pTextureIDs;               // type: Texture

        UINT uiPhysicalTextureCount;
        CAtlString * pPhysicalTextureFileNames;
        RESOURCEID * pPhysicalTextureIDs;       // type: PhysicalTexture

        UINT uiGrPlateCount;
        RESOURCEID * pGrPlateIDs;               // type: GrPlate

        TerrainPlateInfo * pTerrainPlateInfo;   // array of size = uiSizeX * uiSizeY
    } Terrain;
    
    typedef struct __RoadPlateInfo {
        UINT uiTextureIndex;
        UINT uiPhysicalTextureIndex;
        UINT uiRotation;
    } RoadPlateInfo;
    
    typedef struct __Road {
        UINT uiSizeX, uiSizeY;

        UINT uiTextureCount;
        RESOURCEID * pTextureIDs;               //type: Texture

        UINT uiPhysicalTextureCount;
        RESOURCEID * pPhysicalTextureIDs;       // type: PhysicalTexture

        RoadPlateInfo * pRoadPlateInfo;         // array of size = uiSizeX * uiSizeY
    } Road;

    typedef struct __GrPlate {
        UINT uiLoDCount;
        RESOURCEID LoD[TERRAIN_LOD_COUNT];      //type: GrSurface
        UINT bPlanar;
		UINT bConcave;
    } GrPlate;

    //typedef struct __PhysicalTexturePoint {
    //    int iCoefIndex;
    //    //bool bTransparent;
    //} PhysicalTexturePoint;
    
    typedef struct __PhysicalTexture {                  /* X */               /* Y */
        //PhysicalTexturePoint pPhysicalTexturePoint[PHYSICAL_TEXTURE_SIZE][PHYSICAL_TEXTURE_SIZE];
		int pPhysicalTexture[PHYSICAL_TEXTURE_SIZE][PHYSICAL_TEXTURE_SIZE];
    } PhysicalTexture;

    typedef struct __LoDMap {
        int iWidth, iHeight;
        char * pData;
    } LoDMap;

    typedef struct __FontDef {
        LPD3DXFONT pFont;
        int iColor;
    } FontDef;

    typedef struct __FilePackInfo {
        CAtlString sFileName;
        DWORD dwFileSize;
        DWORD dwFileOffset;
    } FilePackInfo;

    typedef struct __FilePack {
        DWORD dwFileCount;
        BYTE * lpBasePtr;
        FilePackInfo * lpFPI;
    } FilePack;

    typedef struct __ModelLoadParams { 
        UINT uiOptions;
        UINT uiLODCount;
    } ModelLoadParams;

    typedef struct __Model {
        DWORD dwXFileCount;
        RESOURCEID * pXFileIDs;
    } Model;

    typedef struct __MemoryXFileLoadParams {
        FilePack * pFP;
        UINT uiIndex;        
        UINT uiOptions;
    } MemoryXFileLoadParams;
        
    typedef struct __MemoryXFile {
        int iMeaningless;
    } MemoryXFile;

    typedef struct __MemoryTextureLoadParams {
        LPVOID pSrcData;
        UINT uiSrcDataSize;
    } MemoryTextureLoadParams;

    typedef struct __MemoryTexture {
        int iMeaningless;
    } MemoryTexture;

	typedef struct __MaterialsCoef {		
		float fSoftness;
		float fElasticity;
		float fStaticFriction;
		float fKineticFriction;
		int iCollidable;

		// operator =
		/*
		resManNS::__MaterialsCoef & resManNS::__MaterialsCoef::operator= (
			const resManNS::__MaterialsCoef & right)
		{
			fSoftness = right.fSoftness;
			fElasticity = right.fElasticity;
			fStaticFriction = right.fStaticFriction;
			fKineticFriction = right.fKineticFriction;
			iCollidable = right.iCollidable;
		}
		*/
		
    } MaterialsCoef;

	typedef struct __MaterialPairInfo
	{
		UINT uiMaterialA;
		UINT uiMaterialB;
		MaterialsCoef coefs;
		CString soundEffect;
		CString scratchSoundEffect;
	} TMaterialPairInfo;

    typedef struct __Materials {		
		UINT uiMaterialIDsCount;		// #materials, first is default

        UINT uiMaterialPairsSize;				// size of pMaterialPairs array
        TMaterialPairInfo * pMaterialPairs;		// array containing material details
    } Materials;

    typedef struct __TSMRemapTable {
        float * pfTable;
    } TSMRemapTable;

	typedef struct __GrassVertex {
		float		x, y; // 2D coordinates (Z is always zero)
		float		u, v; // texture coordinates
	} GrassVertex;
	
	typedef struct __GrassItemLOD {
		UINT			uiSegmentsCount; // count of segments
		UINT			uiStripesCount; // count of stripes
		GrassVertex	*	pVertices; // all vertex data; (#vertices == (#segments+1) * (#stripes+1))
	} GrassItemLOD;

	typedef struct __GrassItem {
		UINT			uiLODCount; // count of LODs loaded for this prototype
		GrassItemLOD	pLODs[MODEL_LOD_COUNT];
		RESOURCEID		ridTexture; // RID of associated texture
	} GrassItem;

    typedef struct __EngineObject {
		int gear_count;
        float gear_1;
        float gear_2;
        float gear_3;
        float gear_4;
        float gear_5;
        float gear_6;
        float gear_r;
        float diff_ratio;
        float efficiency;

		int curve_points_count;
		float curve_point_1_rpm;
		float curve_point_1_torque;
		float curve_point_2_rpm;
		float curve_point_2_torque;
		float curve_point_3_rpm;
		float curve_point_3_torque;
		float curve_point_4_rpm;
		float curve_point_4_torque;
		float curve_point_5_rpm;
		float curve_point_5_torque;
		float curve_point_6_rpm;
		float curve_point_6_torque;
		float curve_point_7_rpm;
		float curve_point_7_torque;
		float curve_point_8_rpm;
		float curve_point_8_torque;

		float shift_up;
		float shift_down;
    } EngineObject;

    typedef struct __CarObject {
		RESOURCEID ridEngine;           //type: EngineObject
        RESOURCEID ridCarObject;        //type: GameObject
		RVec * pvWheelIDs;              //type: GameObject
    } CarObject; 

    typedef struct __MainObject {
        CAtlString * sName;
        CAtlString * sIconName;
		RVec * pvObjectIDs;             //type: GameObject
        RVec * pvRoadIDs;               //type: GameObject
        RVec * pvWaypointsIDs;          //type: Waypoints
        bool   bHasCheckpoint, bStartingPosition;
    } MainObject;

    typedef struct __GameObjectLoadParams {
        CAtlString sLabelName;
        int iOrder;
    } GameObjectLoadParams;

    typedef struct __GameObject {
		int posX,posY;
		UINT sizeX,sizeY;
		float x,y,z;
		float rotationX,rotationY,rotationZ;
		float scaleX,scaleY,scaleZ;
		float mass;
		float moiX,moiY,moiZ;
		float cogX,cogY,cogZ;
		bool bRotated;
		bool bPowered;
        bool bGrassAllowed;
		bool isStatic;
		BYTE material;
		BYTE wheel_material;
		float suspensionShock;
		float suspensionSpring;
		float suspensionLength;
		float suspensionCoef;

		CAtlString * psModel;
		RVec * pvLightObjectIDs;            //type: LightObject
		RVec * pvTextureIDs;                //type: Texture
		RVec * pvPhTextureIDs;              //type: PhysicalTexture
		std::vector<CAtlString> * pvTextureFileNames;
		std::vector<CAtlString> * pvPhTextureFileNames;
        std::vector<CAtlString> * pvLightObjectFileNames;
    } GameObject; // (GameObjectLoadParams *) as pParam in LoadResource
    
    typedef struct __LightObject {
        float x,y,z;
		float directionX,directionY,directionZ;
		float Scale;
		float Attenuation0, Attenuation1, Attenuation2;
		float Phi, Theta, Falloff, Range;
		unsigned int Diffuse, Specular, Ambient;
		unsigned int type;
        RESOURCEID ridLightMap;
    } LightObject;

	struct PlacedObjectMODData { // extended data stored for a sigle map object and a particular game MOD
		CAtlString		sModID; // game MOD primary ID as specified in Map.pvSupportedModules
		std::vector<NAMEVALUEPAIR>	vProperties; // list of name-value pairs specifying all properties of this object for/in particular MOD
	};

	typedef std::vector<PlacedObjectMODData>	OBJECTMODDATALIST;

    typedef struct __Map {
        UINT uiSegsCountX, uiSegsCountZ;
		UINT uiColor, uiFogColor;
        CAtlString *	sMapName; // map name - the maximal length of this string is defined by MAX_MAP_NAME_LENGTH macro
		CAtlString *	sTerrainName;
		CAtlString *	sSkyboxName;
		CAtlString *	sMapDescription;
		CAtlString *	sMapImageFile; // name of the file containing small logo or image of the map
		DWORD		dwVersion; // two words specifying major and minor version number of the map file format (=editor)
        float fDayTime;

		std::vector<CAtlString> * pvSupportedModules; // MODs supported by this map (their primary IDs)

		UINT uiObjectsCount;                // # of unique objects used on map
        RESOURCEID * pridObjects;           //type: MainObject  
        CAtlString * psObjectNames;         //their names

        
        UINT		uiPlacedObjectsCount;          // # of actual objects used on map
        UINT	*	puiPlacedObjectIndexes;      //indexes to pridObjects     
        UINT	*	puiPlacedObjectOrientations; //objects' orientation = 0-3, clockwise, starting at north
        int		*	piPlacedObjectX; //position of objects on map
		int		*	piPlacedObjectZ; //position of objects on map
		OBJECTMODDATALIST * pPlacedObjectData; //data stored along with the placed objects - originally base64 encoded, here stored in plain text
		bool	*	pbPlacedObjectSpecial; //indication of a special object (the one with special properties for some MODs)
        
        RESOURCEID ridStartPosition;
        int iStartRotation, iStartPositionX, iStartPositionZ;
        CAtlString * sStartPositionFileName;

        UINT uiPathPointsCount;      // length of designed track 
        UINT * puiPathPointIndexes;  // indexes to puiPlacedObejctIndexes (objects through which the track leads)
    } Map; 

    typedef struct __SkySystem {
        D3DXVECTOR3 * MoleculeColor; 
		D3DXVECTOR3 * AerosolColor;
		CAtlString * SunTextureName;
		D3DXVECTOR3 * SunColors;
		CAtlString * MountainNorth;
		CAtlString * MountainWest;
		CAtlString * MountainSouth;
		CAtlString * MountainEast;
		CAtlString * CloudTextureName;
		UINT SunTextureID;
		UINT CloudTextureID;
		UINT MountainNorthTextureID;
		UINT MountainWestTextureID;
		UINT MountainSouthTextureID;
		UINT MountainEastTextureID;
    } SkySystem;

    typedef struct __Waypoints {
        std::vector<float> *x, *y, *z, *min_speed, *max_speed;
    } Waypoints;

    typedef struct __TextureSet {
        UINT uiTextureCount;
        CAtlString * pTextureFileNames;
		CAtlString * psPhTexture;
        RESOURCEID * pTextureIDs;
        int * pTextureProbabilities;
    } TextureSet;

    typedef struct __WaveBank {
		IXACTWaveBank	*	pWaveBank;
		VOID			*	pRawBuffer; // Handle to wave bank data.  Its memory mapped so call UnmapViewOfFile() upon cleanup to release file
    } WaveBank;

    typedef struct __SoundBank {
	    IXACTSoundBank	*	pSoundBank;
		VOID			*	pRawBuffer; // Pointer to sound bank data.  Call delete on it when the sound bank is destroyed
    } SoundBank;

    #define RESOURCE_DATA_DECLARATOR(TYPE)   TYPE _##TYPE;

    typedef union __ResourceData {
        EXPAND(RESOURCE_DATA_DECLARATOR)
    } ResourceData;

};