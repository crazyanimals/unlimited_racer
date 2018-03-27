/*
CShadowMap class implepentation - shadow mapping
by Pavel Celba
16. 8. 2004
*/

#include "CShadowMap.h"

using namespace graphic;

// Constructor
CShadowMap::CShadowMap()
{
	DynShadow = NULL;
	DynShadow2 = NULL;
	DynShadowSurf = NULL;
	DynShadowSurf2 = NULL;
}

// Releases render targets, ... created for shadow mapping
int CShadowMap::ReleaseD3DResources()
{
	SAFE_RELEASE(DynShadow);
	SAFE_RELEASE(DynShadow2);
	SAFE_RELEASE(DynShadowSurf);
	SAFE_RELEASE(DynShadowSurf2);

	ClearLightMaps();

	return 0;
}

// Creates render targets, ... for shadow mapping
int CShadowMap::CreateD3DResources()
{
	HRESULT Result; 

	// Init shadow map texture for dynamic objects
	D3DFORMAT DynTextureFormat;
	if (DynShadowFormat == D3DFMT_D24S8) 
	{
		DynTextureFormat = D3DFMT_X8R8G8B8;
	}
	else
	{
		DynTextureFormat = DynShadowFormat;
	}

	SAFE_RELEASE(DynShadow);
	Result = Device->CreateTexture(DynMapSize, DynMapSize, 1, D3DUSAGE_RENDERTARGET,DynTextureFormat, D3DPOOL_DEFAULT, &DynShadow, 0);
	if (FAILED(Result) )
	{
		ERRORMSG(Result, "CShadowMap::CreateD3DResources()", "Unable to create shadow map texture for dynamic objects, lights");
	}

	/*SAFE_RELEASE(DynShadow2);
	Result = Device->CreateTexture(DynMap2Size, DynMap2Size, 1, D3DUSAGE_RENDERTARGET, DynTextureFormat, D3DPOOL_DEFAULT, &DynShadow2, 0);
	if (FAILED(Result) )
	{
		ERRORMSG(Result, "CShadowMap::CreateD3DResources()", "Unable to create second shadow map texture for dynamic objects, lights");
	}*/

	// get surfaces
	DynShadow->GetSurfaceLevel(0, &DynShadowSurf);
	//DynShadow2->GetSurfaceLevel(0, &DynShadowSurf2);

	return 0;
}

// Init - needs Device, Scene class and current width and height of rendering area
int CShadowMap::Init(const ShadowMapInit Init)
{
	HRESULT Result;
	IDirect3DSurface9 * StandardSurface;
	IDirect3DSurface9 * StandardDepthStencil;
	D3DVIEWPORT9 StandardViewport;
	D3DVIEWPORT9 DynMapViewport;

	// Set necessary vars
	Device = Init.Device;
	DynMapSize = Init.DynMapSize;
	DynMap2Size = Init.DynMap2Size;
	SunMapSize = Init.SunMapSize;
	LightMapSize = Init.LightMapSize;
	// Car LOD
	CarLOD = Init.CarLOD;
	// Set method
	Method = Init.Method;
	// Set dyn texture format
	DynShadowFormat = Init.DynTextureFormat;

	// Stride
	Stride = 1.0f / (float) DynMapSize;

	// Lamp LOD
	MaxLampLOD = max(min(Init.MaxLampLOD, DynMapSize), 1);
	MinLampLOD = max(min(Init.MinLampLOD, DynMapSize), 1);

	// Set bias
	SetBias(Init.Bias);

	Result = CreateD3DResources();
	if (FAILED(Result) )
		ERRORMSG(Result, "CShadowMap::Init()", "Unable to create D3D render target, ... resource for shadow mapping.");
	
	// clear entire created texture
	Device->GetRenderTarget(0, &StandardSurface);
	Device->GetDepthStencilSurface(&StandardDepthStencil);
	Device->GetViewport(&StandardViewport);
		
	DynMapViewport.X = 0;
	DynMapViewport.Y = 0;
	DynMapViewport.Width = DynMapSize;
	DynMapViewport.Height = DynMapSize;
	DynMapViewport.MinZ = 0;
	DynMapViewport.MaxZ = 1;

	Device->SetRenderTarget(0, DynShadowSurf);
	Device->SetDepthStencilSurface(NULL);
	Device->SetViewport(&DynMapViewport);

	Device->Clear(0, 0, D3DCLEAR_TARGET, D3DXCOLOR(255,255,255,255), 1,0);

	Device->SetRenderTarget(0, StandardSurface);
	Device->SetDepthStencilSurface(StandardDepthStencil);
	Device->SetViewport(&StandardViewport);

	SAFE_RELEASE(StandardSurface);
	SAFE_RELEASE(StandardDepthStencil);
	
	// init correction matrix
	CorrectMat = D3DXMATRIX(0.5f, 0, 0, 0,
						    0, -0.5f, 0, 0,
							0, 0, 1, 0,
							0.5f + (0.5f / (float) DynMapSize), 0.5f + (0.5f / (float) DynMapSize), 0, 1);
	
	
	/*
	CorrectMat = D3DXMATRIX(0.5f, 0, 0, 0,
						    0, -0.5f, 0, 0,
							0, 0, 1, 0,
							0.5f, 0.5f, 0, 1);
	*/
	
	// Insert black light map for terrain plates without light map
	AddLightMap(Init.BlackLightMap);

	// init projection box points
	ProjBoxPoints[0] = D3DXVECTOR3(-1, -1, 0);
	ProjBoxPoints[1] = D3DXVECTOR3(1, -1, 0);
	ProjBoxPoints[2] = D3DXVECTOR3(1, 1, 0);
	ProjBoxPoints[3] = D3DXVECTOR3(-1, 1, 0);
	ProjBoxPoints[4] = D3DXVECTOR3(-1, -1, 1);
	ProjBoxPoints[5] = D3DXVECTOR3(1, -1, 1);
	ProjBoxPoints[6] = D3DXVECTOR3(1, 1, 1);
	ProjBoxPoints[7] = D3DXVECTOR3(-1, 1, 1);

	// init trapezoidal shadow mapping class
	Result = Tsm.Init();
	if (FAILED(Result) )
		ERRORMSG(Result, "CShadowMap::Init()", "Error while initializing trapezoidal shadow mapping");
	
	// all ok
	return 0;
}

// returns post post projection aquire matrix
int CShadowMap::ComputePostProjMatrix(D3DXMATRIX * PostProjMat, const D3DXMATRIX LightViewMat, const D3DXMATRIX LightProjMat,
						  const D3DXMATRIX ViewMat, const D3DXMATRIX ProjMat, const D3DXVECTOR3 CameraPos)
{
	D3DXMATRIX ViewProjToLightProj; // transforms from camera projection space to light projection space
	D3DXVECTOR3 LightBoxPoints[8]; // camera view frustrum in light projection space
	D3DXVECTOR3 Max, Min;
	D3DXVECTOR3 Center;

	D3DXMATRIX Translation;
	D3DXMATRIX Scale;
	int i;
	bool Intersect = true;
	
	D3DXMATRIX InvViewProjMat; // Inverse view * projection matrix
	
	// compute transformation matrix
	D3DXMatrixInverse(&InvViewProjMat, 0, &(ViewMat * ProjMat) );

	ViewProjToLightProj = InvViewProjMat * LightViewMat * LightProjMat;

	// transform points to light projection space - near plane points
	D3DXVec3TransformCoordArray(LightBoxPoints, sizeof(D3DXVECTOR3), ProjBoxPoints, sizeof(D3DXVECTOR3), &ViewProjToLightProj, 8);

	// far plane points are not enough good, so seek intersection with y = -1 plane
	D3DXVECTOR3 NearPlaneWorld[4];
	D3DXVec3TransformCoordArray(NearPlaneWorld, sizeof(D3DXVECTOR3), ProjBoxPoints, sizeof(D3DXVECTOR3), &InvViewProjMat, 4);

	D3DXVECTOR3 Rays[4];
	float t;
	for (i = 0; i < 4; i++)
	{
		Rays[i].x = NearPlaneWorld[i].x - CameraPos.x;
		Rays[i].y = NearPlaneWorld[i].y - CameraPos.y;
		Rays[i].z = NearPlaneWorld[i].z - CameraPos.z;

		// perform intersection with plane y = -1
		t = -1;
		if (Rays[i].y != 0)
			t = (-1 - CameraPos.y) / Rays[i].y;
		if (t > 0) 
		{
			LightBoxPoints[i + 4].x = CameraPos.x + t * Rays[i].x;
			LightBoxPoints[i + 4].y = -1;
			LightBoxPoints[i + 4].z = CameraPos.z + t * Rays[i].z;
			D3DXVec3TransformCoord(&LightBoxPoints[i + 4], &LightBoxPoints[i + 4], & (LightViewMat * LightProjMat) );
		}

	}
	
	
	// compute AABB bounding box
	Max = D3DXVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	Min = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);

	for (i = 0; i < 8; i++)
	{
		if (LightBoxPoints[i].x < Min.x) Min.x = LightBoxPoints[i].x;
		if (LightBoxPoints[i].y < Min.y) Min.y = LightBoxPoints[i].y;
		if (LightBoxPoints[i].z < Min.z) Min.z = LightBoxPoints[i].z;
		if (LightBoxPoints[i].x > Max.x) Max.x = LightBoxPoints[i].x;
		if (LightBoxPoints[i].y > Max.y) Max.y = LightBoxPoints[i].y;
		if (LightBoxPoints[i].z > Max.z) Max.z = LightBoxPoints[i].z;
	}
	
	// compute intersection with [-1, -1, 0] - [1, 1, 1] box
	if (Min.x > -1 && Min.x < 1)
	{
		if (Max.x > 1) Max.x = 1;
	}
	else if (Max.x > -1 && Max.x < 1)
		Min.x = -1;
	else
		Intersect = false;

	if (Min.y > -1 && Min.y < 1)
	{
		if (Max.y > 1) Max.y = 1;
	}
	else if (Max.y > -1 && Max.y < 1)
		Min.y = -1;
	else
		Intersect = false;

	if (Min.z > 0 && Min.z < 1)
	{
		if (Max.z > 1) Max.z = 1;
	}
	else if (Max.z > 0 && Max.z < 1)
		Min.z = 0;
	else
		Intersect = false;

	if (!Intersect)
	{
		D3DXMatrixIdentity(PostProjMat);
		return 0;
	}

	// compute point to translate
	Center = D3DXVECTOR3( (Min.x + Max.x) / 2, (Min.y + Max.y) / 2, Min.z); 

	// Translate center to [0,0]
	D3DXMatrixTranslation(&Translation, -Center.x, -Center.y, -Center.z);
	
	// Scale sides of boxes so the box covers entire [-1, -1, 0] - [1, 1, 1]
	D3DXMatrixScaling(&Scale, 2.0f / (Max.x - Min.x), 2.0f / (Max.y - Min.y), 1.0f / (Max.z - Min.z) );

	// Compute resulting matrix
	D3DXMatrixMultiply(PostProjMat, &Translation, &Scale);

	return 0;
};

// Destructor
CShadowMap::~CShadowMap()
{
	// Release dynamic shadow maps
	SAFE_RELEASE(DynShadow);
	SAFE_RELEASE(DynShadowSurf);

	SAFE_RELEASE(DynShadow2);
	SAFE_RELEASE(DynShadowSurf2);

	// delete all light maps
	ClearLightMaps();
}

// Set Bias - needed for shadowing artifact avoidance
int CShadowMap::SetBias(float pBias)
{
	Bias = pBias;
	D3DXMatrixTranslation(&BiasMat, 0, 0, Bias);

	return 0;
}