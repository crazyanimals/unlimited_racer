/*
Shadow mapping class - provides support and setting for shadow maps and light maps generation
By Pavel Celba
1. 12. 2004 - revised
*/

#pragma once
#include "..\Globals\GlobalsIncludes.h"
#include "..\GraphicObjects\Light.h"
#include "..\GraphicObjects\GrObjectTerrainPlate.h"
#include "..\GraphicObjects\GrObjectMesh.h"
#include "Camera.h"
#include <map>
#include "TSM.h"

namespace graphic 
{

// Shadow mapping rendering method
enum SMRenderingMethod
{
	SHADOW_MAPPING, // shadow mapping
	PROJECTIVE // projective shadows
};
	
// Structure for Shadow map class initialization
struct ShadowMapInit
{
	LPDIRECT3DDEVICE9 Device; // Device
	IDirect3DTexture9 * BlackLightMap; // The basic light map - black
	float Bias; // the amount of bias to use with shadows
	UINT LightMapSize; // Size of light maps
	UINT DynMapSize; // Size of dynamic shadow texture
	UINT DynMap2Size; // Size of dynamic shadow texture 2
	UINT SunMapSize; // Size used for sun trapezoidal shadow map
	UINT MaxLampLOD; // Maximum lamp LOD - cannot be greater then DynMapSize
	UINT MinLampLOD; // Minimum lamp LOD - >= 1
	UINT CarLOD; // Car shadow LOD
	D3DFORMAT DynTextureFormat; // Dynamic shadow texture format
	SMRenderingMethod Method; // Shadow rendering method
};


// Shadow map class
class CShadowMap
{
public:
	
	// Constructor
	CShadowMap();

	// Destructor
	~CShadowMap();

	// TODO - add light map size param
	// Init - needs Device, Scene class and current width and height of rendering area
	int Init(const ShadowMapInit Init);

	// Releases render targets, ... created for shadow mapping
	int ReleaseD3DResources();

	// Creates render targets, ... for shadow mapping
	int CreateD3DResources();
	
	// Computes light view and projection matrix
	// only for LT_LAMP or LT_CARREFLECTOR
	inline int ComputeLightViewProjMat(D3DXMATRIX * LightViewMat, D3DXMATRIX * LightProjMat, const CLight * Light)
	{
		// Compute light view matrix
		D3DXMatrixLookAtLH(LightViewMat, & Light->Position, & (Light->Position + Light->Direction), &D3DXVECTOR3(0,1,0) );
		
		// Projection matrix computation
		D3DXMatrixPerspectiveFovLH(LightProjMat, Light->Phi, 1, 1.0f, Light->Range);
		//D3DXMatrixOrthoLH(LightProjMat, 1000, 1000, 10.0f, 1200);
		
		return 0;
	}

	// Computes light view and projection matrix for concrete object
	inline int ComputeObjectViewProjMatrix(D3DXMATRIX * LightViewMat, D3DXMATRIX * LightProjMat, const CLight * Light, CGrObjectMesh * ObjMesh) 
	{
		// Compute light view matrix
		D3DXMatrixLookAtLH(LightViewMat, & Light->Position, & (Light->Position + Light->Direction), &D3DXVECTOR3(0,0,1) );
		
		// Projection matrix computation
		
		// BBox vertices
		LPD3DXMESH Mesh;
		D3DXVECTOR3 Min, Max;
		D3DXVECTOR3 Box[8];
		D3DXVECTOR3 * MeshVerts;
		D3DXMATRIX WorldViewLightMat;

		// Compute bounding box of mesh
		Mesh = ObjMesh->GetMesh();
		Mesh->LockVertexBuffer(D3DLOCK_READONLY, (void **) &MeshVerts);
		D3DXComputeBoundingBox(MeshVerts, Mesh->GetNumVertices(), Mesh->GetNumBytesPerVertex(), &Min, &Max);
		Mesh->UnlockVertexBuffer();

		// Transform bounding box of the mesh to Light view
		D3DXMatrixMultiply(&WorldViewLightMat, &(ObjMesh->GetWorldMat()), LightViewMat);

		Box[0] = D3DXVECTOR3(Min.x, Min.y, Min.z);
		Box[1] = D3DXVECTOR3(Min.x, Min.y, Max.z);
		Box[2] = D3DXVECTOR3(Min.x, Max.y, Min.z);
		Box[3] = D3DXVECTOR3(Min.x, Max.y, Max.z);
		Box[4] = D3DXVECTOR3(Max.x, Min.y, Min.z);
		Box[5] = D3DXVECTOR3(Max.x, Min.y, Max.z);
		Box[6] = D3DXVECTOR3(Max.x, Max.y, Min.z);
		Box[7] = D3DXVECTOR3(Max.x, Max.y, Max.z);
		
		D3DXVec3TransformCoordArray(Box, sizeof(D3DXVECTOR3), Box, sizeof (D3DXVECTOR3), &WorldViewLightMat, 8);

		// Compute min and max
		Max = D3DXVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		Min = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);

		for (int i = 0; i < 8; i++)
		{
			if (Box[i].x > Max.x) Max.x = Box[i].x;
			if (Box[i].y > Max.y) Max.y = Box[i].y;
			if (Box[i].z > Max.z) Max.z = Box[i].z;
			if (Box[i].x < Min.x) Min.x = Box[i].x;
			if (Box[i].y < Min.y) Min.y = Box[i].y;
			if (Box[i].z < Min.z) Min.z = Box[i].z;
		}


		D3DXMatrixOrthoOffCenterLH(LightProjMat, Min.x, Max.x,
												 Min.y, Max.y,
												 Min.z, Max.z);

		return 0;
	}

	// returns post post projection aquire matrix
	int ComputePostProjMatrix(D3DXMATRIX * PostProjMat, const D3DXMATRIX LightViewMat, const D3DXMATRIX LightProjMat,
							  const D3DXMATRIX ViewMat, const D3DXMATRIX ProjMat, const D3DXVECTOR3 CameraPos);

	// Set lamp LOD
	inline int SetLampLOD(UINT _MinLampLOD, UINT _MaxLampLOD)
	{
		MaxLampLOD = max(min(_MaxLampLOD, DynMapSize), 1);
		MinLampLOD = max(min(_MinLampLOD, DynMapSize), 1);
		
		return 0;
	};

	// Return viewport for lamp based on shadow map progresive LOD
	inline D3DVIEWPORT9 GetLampViewport(CGrObjectTerrainPlate * Plate, CCamera * Camera)
	{
		float Dist;
		D3DVIEWPORT9 ShadowMapView;
		// Compute distance between center of plate to camera position
		D3DXVECTOR3 PlateCenter;
		D3DXVec3TransformCoord(&PlateCenter, & D3DXVECTOR3(0,0,0) , & Plate->GetWorldMat() );
		Dist = D3DXVec3Length( &(Camera->GetEyePtVector() - PlateCenter) );
		if (Dist == 0) Dist = 1;

		ShadowMapView.X = 0;
		ShadowMapView.Y = 0;
		ShadowMapView.MinZ = 0;
		ShadowMapView.MaxZ = 1;
		ShadowMapView.Width = (DWORD) max(min(DynMapSize * 350 / Dist, MaxLampLOD), MinLampLOD);
		ShadowMapView.Height = (DWORD) max(min(DynMapSize * 350 / Dist, MaxLampLOD), MinLampLOD);
		
		return ShadowMapView;
	}

	// Set Bias - needed for shadowing artifact avoidance
	int SetBias(float pBias);

	// Get bias - return value of bias used for shadow mapping
	inline float GetBias() const
	{
		return Bias;	
	};

	// Get bias matrix
	inline D3DXMATRIX & GetBiasMat()
	{
		return BiasMat;
	}

	// Return correction matrix in dependence with size
	inline D3DXMATRIX & GetCorrectMat(const UINT TextureSize)
	{
		static D3DXMATRIX Result;
		// Because not all texture is always used
		D3DXMatrixScaling(&Result, TextureSize / (float) DynMapSize, TextureSize / (float) DynMapSize, 1);
		Result = CorrectMat * Result;
		return Result;
	}

	// return correction matrix of some texture rectangle
	inline D3DXMATRIX & GetCorrectMat(const RECT Rect)
	{
		static D3DXMATRIX Result;

		D3DXMATRIX Translation;

		float TextureSizeX, TextureSizeY;
		TextureSizeX = (float) Rect.right - Rect.left;
		TextureSizeY = (float) Rect.bottom - Rect.top;

		D3DXMatrixScaling(&Result, TextureSizeX / (float) DynMapSize, TextureSizeY / (float) DynMapSize, 1);
		D3DXMatrixTranslation(&Translation, (float) Rect.left / (float) DynMapSize, (float) Rect.top / (float) DynMapSize, 0.0f);
		
		Result = CorrectMat * Result * Translation;

		return Result;
	}

	// Return correction matrix with z scaling
	/*
	inline D3DXMATRIX & GetCorrectMatScale(const UINT TextureSize, const float ZScale)
	{
		static D3DXMATRIX Result;
		// Because not all texture is always used
		D3DXMatrixScaling(&Result, TextureSize / (float) DynMapSize, TextureSize / (float) DynMapSize, ZScale);
		Result = CorrectMat * Result;
		return Result;
	}
	*/

	// Get dynamic shadow map texture
	inline IDirect3DTexture9 * GetDynShadowTex()
	{
		return DynShadow;
	}
	
	// Get dynamic shadow surface
	inline IDirect3DSurface9 * GetDynShadowSurf()
	{
		return DynShadowSurf;
	}

	// Get dynamic shadow map texture 2
	inline IDirect3DTexture9 * GetDynShadowTex2()
	{
		return DynShadow2;
	}

	// Get dynamic shadow surface 2
	inline IDirect3DSurface9 * GetDynShadowSurf2()
	{
		return DynShadowSurf2;
	}

	// Add light map to container
	inline UINT AddLightMap(IDirect3DTexture9 * LightMap)
	{
		LightMaps.push_back(LightMap);
		return (UINT) LightMaps.size() - 1;
	}

	// return light map number Number
	// Light map must exist in container
	inline IDirect3DTexture9 * GetLightMap(const UINT Number)
	{
		return LightMaps[Number];
	}

	// Free shadow mapping gameplay resources 
	inline void Free()
	{
		ClearLightMaps();
	}

	// clears light maps container
	inline void ClearLightMaps()
	{
		for (unsigned int i = 1; i < LightMaps.size(); i++)
			SAFE_RELEASE(LightMaps[i]);
		LightMaps.resize(1);
	}

	
	// Returns light map size
	inline UINT GetLightMapSize() const 
	{
		return LightMapSize;
	}

	// Returns dynamic map size
	inline UINT GetDynMapSize() const 
	{
		return DynMapSize;
	}

	inline UINT GetDynMap2Size() const
	{
		return DynMap2Size;
	}

	// return max lamp LOD
	inline UINT GetMaxLampLOD() const 
	{
		return MaxLampLOD;
	}	

	// return min lamp LOD
	inline UINT GetMinLampLOD() const 
	{
		return MinLampLOD;
	}

	// return stride
	inline float GetStride() const
	{
		return Stride;
	}

	// returns shadow mapping method
	inline SMRenderingMethod GetSMMethod() const
	{
		return Method;
	}

	// returns dynamic texture format
	inline D3DFORMAT GetDynMapFormat() const
	{
		return DynShadowFormat;
	}

	// returns sun map size
	inline UINT GetSunMapSize() const
	{
		return SunMapSize;
	}

	// sets sun map size
	inline void SetSunMapSize(UINT _SunMapSize)
	{
		SunMapSize = _SunMapSize;	
	}

	// Sets car LOD
	inline void SetCarLOD(UINT NewLOD)
	{
		if (NewLOD <= 1022)
			CarLOD = NewLOD;
	}

	// gets car lod
	inline UINT GetCarLOD()
	{
		return CarLOD;
	}

	// Trapezoidal shadow mapping class - provides functions for computing trapezoidal shadow map
	TSM Tsm;

private:
	// Shadow mapping rendering method
	SMRenderingMethod Method;

	// Light map storage
	std::vector<IDirect3DTexture9 *> LightMaps;
	
	// Device
	LPDIRECT3DDEVICE9 Device;
	
	// Size of static lamp shadow light map
	UINT LightMapSize;
	// Size used for sun trapezoidal shadow map
	UINT SunMapSize; 
	// Size of dynamic lamp shadow map
	UINT DynMapSize, DynMap2Size;
	// Maximum lamp LOD
	UINT MaxLampLOD;
	// Minimum lamp LOD
	UINT MinLampLOD;

	// Car shadow LOD
	UINT CarLOD;

	// Texture for dynamic shadow object rendering
	IDirect3DTexture9 * DynShadow;
	IDirect3DSurface9 * DynShadowSurf;

	// Second texture for dynamic shadow rendering
	IDirect3DTexture9 * DynShadow2;
	IDirect3DSurface9 * DynShadowSurf2;

	D3DFORMAT DynShadowFormat; // dynamic shadow texture format

	// Bias matrix
	D3DXMATRIX BiasMat;
	// Bias
	float Bias; 

	// Dynamic map size texcoord stride
	float Stride;

	// Correction matrix
	D3DXMATRIX CorrectMat;

	// Projection box points
	D3DXVECTOR3 ProjBoxPoints[8];
};


};