/*
Dynamic sky system implematation
by Pavel Celba
1. 1. 2005
Last Update 14.12.2005 by RM
*/

#include "GrObjectSkySystem.h"

using namespace graphic;

#define DAY_END 0.75f
#define DAY_START 0.25f

// constructor 
CGrObjectSkySystem::CGrObjectSkySystem()
{
	// Set other values to their initial value
	SkySystemResourceID = -1;
	AerosolColorHandle = 0;
	MoleculeColorHandle = 0;
	Cloud = 0;
	Counter = 0;
	DayEnd = DAY_END;
	DayStart = DAY_START;
	for (int i = 0; i < 4; i++)	Mountains[i] = NULL;
	MountBuffer = NULL;
	SkyDome = 0;
	Stars = 0;
	StartSunNormal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	D3DXVec3Normalize(&StartSunNormal, &StartSunNormal);
	SunColor = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	SunColorHandle = 0;
	SunMatHandle = 0;
	SunNormal = StartSunNormal;
	SunNormalHandle = 0;
	D3DXMatrixIdentity( &SunProjTexMat);
	SunTexture = 0;
	TimeAcceleration = 100.0f;
	TimerLastTime = 0;
	UT = 0.0f;
};


// inits this object
HRESULT	CGrObjectSkySystem::Init(	BOOL Static,
									CString fileName, 
									UINT LODcount)
{
	UINT i;
	HRESULT Result;

	ID3DXBuffer * Adjacency;

	// Init base object
	CGrObjectBase::Init(Static);

	// Create sky dome
	SAFE_RELEASE(SkyDome);
	Result = D3DXCreateSphere(D3DDevice, 100, 33, 33, &SkyDome, &Adjacency);
	if (FAILED(Result) )
		ERRORMSG(Result, "CGrObjectSkySystem::Init()", "Unable to create sky dome.");

	// optimize
	SkyDome->OptimizeInplace(D3DXMESHOPT_COMPACT |  D3DXMESHOPT_VERTEXCACHE , (DWORD *) Adjacency->GetBufferPointer(),
							 0, 0, 0);

	SAFE_RELEASE(Adjacency);

	// Get shader handles
	Shader->BeginShaderConstantFetch();

	Shader->GetHandleByName("SunNormal", &SunNormalHandle);
	Shader->GetHandleByName("SunColor", &SunColorHandle);
	Shader->GetHandleByName("MoleculeColor", &MoleculeColorHandle);
	Shader->GetHandleByName("AerosolColor", &AerosolColorHandle);
	Shader->GetHandleByName("SunMat", &SunMatHandle);
	Shader->GetHandleByName("CloudMat", &CloudMatHandle);
	
	Shader->EndShaderConstantFetch();
	
	// Load sky system resource along with textures
	SkySystemResourceID = ResourceManager->LoadResource(fileName, RES_SkySystem, 0);
	if (SkySystemResourceID < 0)
		ERRORMSG(SkySystemResourceID, "CGRObjectSkySystem::Init()", CString("Error while loading sky system settings and textures from filename: ") + fileName);

	resManNS::SkySystem  * _SkySystem;
	_SkySystem = ResourceManager->GetSkySystem(SkySystemResourceID);

	// Save to sky system class
	AerosolColor = *(_SkySystem->AerosolColor);
	MoleculeColor = *(_SkySystem->MoleculeColor);
	for (i = 0; i < 24; i++)
		SunColors[i] = _SkySystem->SunColors[i];
	
	// Init params
	// Sun normal used to compute molecule and aerosol scattering
	StartSunNormal = D3DXVECTOR3(0, -1, 0);
	D3DXVec3Normalize(&StartSunNormal, &StartSunNormal);
	SunNormal = StartSunNormal;
	
	// Init sun texture
	SunTexture = ResourceManager->GetTexture(_SkySystem->SunTextureID)->texture;

	D3DSURFACE_DESC Desc;
	SunTexture->GetLevelDesc(0, &Desc);
	// Compute sun projection, texture matrix
	D3DXMATRIX TexMat = D3DXMATRIX( 0.5f, 0.0f, 0.0f, 0.0f,
						  0.0f, -0.5f, 0.0f, 0.0f,
						  0.0f, 0.0f, 1.0f, 0.0f,
						  0.5f + 1.0f / Desc.Width, 0.5f + 1.0f / Desc.Height, 0.0f, 1.0f );
	
	D3DXMatrixPerspectiveFovLH(&SunProjTexMat, D3DX_PI / 12, 1.0f, 1.0f, 1500.0f);
	SunProjTexMat *= TexMat;

	// Init stars
	StarVertexStruct * StarVertex;
	int StarColor;

	SAFE_RELEASE(Stars);
	Result = D3DDevice->CreateVertexBuffer( sizeof(StarVertexStruct)* 1000, 0, D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_PSIZE, D3DPOOL_MANAGED, &Stars, 0);
	if (FAILED(Result) )
		ERRORMSG(Result, "CGrObjectSkySystem::Init()", "Unable to create stars vertex buffer");

	// generate stars
	// seed random time
	srand( (unsigned) time(NULL) );

	Stars->Lock(0,0, (void **) &StarVertex, D3DLOCK_NOSYSLOCK);
		for (i = 0; i < 1000; i++)
		{
			StarVertex->x = (float) rand() / (float) RAND_MAX - 0.5f;
			StarVertex->y = (float) rand() / (float) RAND_MAX - 0.5f;
			StarVertex->z = (float) rand() / (float) RAND_MAX - 0.5f;
			StarColor = max( rand() % 255 + 30, 255);
			StarVertex->Color = D3DCOLOR_ARGB(255, StarColor, StarColor, StarColor);
			StarVertex->PointSize = 1.0f + ((float) rand() / (float) RAND_MAX) * 2;
			StarVertex++;
		}
	Stars->Unlock();

	// Init mountains 
	MountainsVertexStruct * Mountain;

	SAFE_RELEASE( MountBuffer );
	Result = D3DDevice->CreateVertexBuffer( 4*6*3*sizeof(MountainsVertexStruct), 0, D3DFVF_TEXCOORDSIZE1(1) | D3DFVF_XYZ | D3DFVF_TEX2, D3DPOOL_MANAGED, &MountBuffer, 0 );
	if (FAILED(Result) )
			ERRORMSG(Result, "CGrObjectSkySystem::Init()", "Unable to create mountain vertex buffer");

	MountBuffer->Lock( 0, 0, (void **) &Mountain, D3DLOCK_NOSYSLOCK );
	
	float		X = 200000;
	float		Z = 200000;
	float		Y = 100000;
	const UINT	uiCount = 4;
	float		Ys[uiCount];
	float		Alpha[uiCount];
	UINT		uiIndex;

	Ys[0] = 1;
	Ys[1] = 0.4f;
	Ys[2] = 0;
	Ys[3] = -1;

	Alpha[0] = 0.1f;
	Alpha[1] = 0.3f;
	Alpha[2] = 0.9f;
	Alpha[3] = 1;

	uiIndex = 0;
	
	for ( i = 0; i < uiCount - 1; i++ )
	{
		Mountain[6*i+uiIndex].x = -X;
		Mountain[6*i+uiIndex].y = Ys[i] * Y;
		Mountain[6*i+uiIndex].z = Z;
		Mountain[6*i+uiIndex].Alpha = Alpha[i];
		Mountain[6*i+uiIndex].tu = 0;
		Mountain[6*i+uiIndex].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+1].x = X;
		Mountain[6*i+uiIndex+1].y = Ys[i] * Y;
		Mountain[6*i+uiIndex+1].z = Z;
		Mountain[6*i+uiIndex+1].Alpha = Alpha[i];
		Mountain[6*i+uiIndex+1].tu = 1;
		Mountain[6*i+uiIndex+1].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+2].x = X;
		Mountain[6*i+uiIndex+2].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+2].z = Z;
		Mountain[6*i+uiIndex+2].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+2].tu = 1;
		Mountain[6*i+uiIndex+2].tv = (Ys[i+1]-1.0f) * (-0.5f);


		Mountain[6*i+uiIndex+3].x = -X;
		Mountain[6*i+uiIndex+3].y = Ys[i] * Y;
		Mountain[6*i+uiIndex+3].z = Z;
		Mountain[6*i+uiIndex+3].Alpha = Alpha[i];
		Mountain[6*i+uiIndex+3].tu = 0;
		Mountain[6*i+uiIndex+3].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+4].x = X;
		Mountain[6*i+uiIndex+4].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+4].z = Z;
		Mountain[6*i+uiIndex+4].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+4].tu = 1;
		Mountain[6*i+uiIndex+4].tv = (Ys[i+1]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+5].x = -X;
		Mountain[6*i+uiIndex+5].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+5].z = Z;
		Mountain[6*i+uiIndex+5].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+5].tu = 0;
		Mountain[6*i+uiIndex+5].tv = (Ys[i+1]-1.0f) * (-0.5f);
	}

	uiIndex += 6 * (uiCount - 1);

	for ( i = 0; i < uiCount - 1; i++ )
	{
		Mountain[6*i+uiIndex].x = X;
		Mountain[6*i+uiIndex].y = Ys[i] * Y;
		Mountain[6*i+uiIndex].z = Z;
		Mountain[6*i+uiIndex].Alpha = Alpha[i];
		Mountain[6*i+uiIndex].tu = 0;
		Mountain[6*i+uiIndex].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+1].x = X;
		Mountain[6*i+uiIndex+1].y = Ys[i] * Y;
		Mountain[6*i+uiIndex+1].z = -Z;
		Mountain[6*i+uiIndex+1].Alpha = Alpha[i];
		Mountain[6*i+uiIndex+1].tu = 1;
		Mountain[6*i+uiIndex+1].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+2].x = X;
		Mountain[6*i+uiIndex+2].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+2].z = -Z;
		Mountain[6*i+uiIndex+2].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+2].tu = 1;
		Mountain[6*i+uiIndex+2].tv = (Ys[i+1]-1.0f) * (-0.5f);


		Mountain[6*i+uiIndex+3].x = X;
		Mountain[6*i+uiIndex+3].y = Ys[i] * Y;
		Mountain[6*i+uiIndex+3].z = Z;
		Mountain[6*i+uiIndex+3].Alpha = Alpha[i];
		Mountain[6*i+uiIndex+3].tu = 0;
		Mountain[6*i+uiIndex+3].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+4].x = X;
		Mountain[6*i+uiIndex+4].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+4].z = -Z;
		Mountain[6*i+uiIndex+4].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+4].tu = 1;
		Mountain[6*i+uiIndex+4].tv = (Ys[i+1]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+5].x = X;
		Mountain[6*i+uiIndex+5].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+5].z = Z;
		Mountain[6*i+uiIndex+5].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+5].tu = 0;
		Mountain[6*i+uiIndex+5].tv = (Ys[i+1]-1.0f) * (-0.5f);
	}

	uiIndex += 6 * (uiCount - 1);

	for ( i = 0; i < uiCount - 1; i++ )
	{
		Mountain[6*i+uiIndex].x = X;
		Mountain[6*i+uiIndex].y = Ys[i] * Y;
		Mountain[6*i+uiIndex].z = -Z;
		Mountain[6*i+uiIndex].Alpha = Alpha[i];
		Mountain[6*i+uiIndex].tu = 0;
		Mountain[6*i+uiIndex].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+1].x = -X;
		Mountain[6*i+uiIndex+1].y = Ys[i] * Y;
		Mountain[6*i+uiIndex+1].z = -Z;
		Mountain[6*i+uiIndex+1].Alpha = Alpha[i];
		Mountain[6*i+uiIndex+1].tu = 1;
		Mountain[6*i+uiIndex+1].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+2].x = -X;
		Mountain[6*i+uiIndex+2].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+2].z = -Z;
		Mountain[6*i+uiIndex+2].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+2].tu = 1;
		Mountain[6*i+uiIndex+2].tv = (Ys[i+1]-1.0f) * (-0.5f);


		Mountain[6*i+uiIndex+3].x = X;
		Mountain[6*i+uiIndex+3].y = Ys[i] * Y;
		Mountain[6*i+uiIndex+3].z = -Z;
		Mountain[6*i+uiIndex+3].Alpha = Alpha[i];
		Mountain[6*i+uiIndex+3].tu = 0;
		Mountain[6*i+uiIndex+3].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+4].x = -X;
		Mountain[6*i+uiIndex+4].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+4].z = -Z;
		Mountain[6*i+uiIndex+4].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+4].tu = 1;
		Mountain[6*i+uiIndex+4].tv = (Ys[i+1]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+5].x = X;
		Mountain[6*i+uiIndex+5].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+5].z = -Z;
		Mountain[6*i+uiIndex+5].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+5].tu = 0;
		Mountain[6*i+uiIndex+5].tv = (Ys[i+1]-1.0f) * (-0.5f);
	}

	uiIndex += 6 * (uiCount - 1);

	for ( i = 0; i < uiCount - 1; i++ )
	{
		Mountain[6*i+uiIndex].x = -X;
		Mountain[6*i+uiIndex].y = Ys[i] * Y;
		Mountain[6*i+uiIndex].z = -Z;
		Mountain[6*i+uiIndex].Alpha = Alpha[i];
		Mountain[6*i+uiIndex].tu = 0;
		Mountain[6*i+uiIndex].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+1].x = -X;
		Mountain[6*i+uiIndex+1].y = Ys[i] * Y;
		Mountain[6*i+uiIndex+1].z = Z;
		Mountain[6*i+uiIndex+1].Alpha = Alpha[i];
		Mountain[6*i+uiIndex+1].tu = 1;
		Mountain[6*i+uiIndex+1].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+2].x = -X;
		Mountain[6*i+uiIndex+2].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+2].z = Z;
		Mountain[6*i+uiIndex+2].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+2].tu = 1;
		Mountain[6*i+uiIndex+2].tv = (Ys[i+1]-1.0f) * (-0.5f);


		Mountain[6*i+uiIndex+3].x = -X;
		Mountain[6*i+uiIndex+3].y = Ys[i] * Y;
		Mountain[6*i+uiIndex+3].z = -Z;
		Mountain[6*i+uiIndex+3].Alpha = Alpha[i];
		Mountain[6*i+uiIndex+3].tu = 0;
		Mountain[6*i+uiIndex+3].tv = (Ys[i]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+4].x = -X;
		Mountain[6*i+uiIndex+4].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+4].z = Z;
		Mountain[6*i+uiIndex+4].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+4].tu = 1;
		Mountain[6*i+uiIndex+4].tv = (Ys[i+1]-1.0f) * (-0.5f);

		Mountain[6*i+uiIndex+5].x = -X;
		Mountain[6*i+uiIndex+5].y = Ys[i+1] * Y;
		Mountain[6*i+uiIndex+5].z = -Z;
		Mountain[6*i+uiIndex+5].Alpha = Alpha[i+1];
		Mountain[6*i+uiIndex+5].tu = 0;
		Mountain[6*i+uiIndex+5].tv = (Ys[i+1]-1.0f) * (-0.5f);
	}

		
		MountBuffer->Unlock();


	// load mountain texture
	Mountains[0] = ResourceManager->GetTexture(_SkySystem->MountainNorthTextureID)->texture;
	Mountains[1] = ResourceManager->GetTexture(_SkySystem->MountainWestTextureID)->texture;
	Mountains[2] = ResourceManager->GetTexture(_SkySystem->MountainSouthTextureID)->texture;
	Mountains[3] = ResourceManager->GetTexture(_SkySystem->MountainEastTextureID)->texture;
	
	// Load clouds
	Cloud = ResourceManager->GetTexture(_SkySystem->CloudTextureID)->texture;

	// Init timing
	Counter = 0;
	
	// Start timer
	Timer.Start();

	// Get last time of timer
	TimerLastTime = Timer.GetRunTime();
	
	// Set initial time acceleration
	TimeAcceleration = 0.0f;
	UT = 0.5f;
	
	// Set day start and day end
	DayStart = DAY_START;
	DayEnd = DAY_END;

	return 0;
}

// free sky system resources
void CGrObjectSkySystem::Free()
{
	if (SkySystemResourceID >= 0) ResourceManager->ReleaseResource(SkySystemResourceID);
	
	// release sky dome
	SAFE_RELEASE(SkyDome);
	// release stars
	SAFE_RELEASE(Stars);
	SAFE_RELEASE( MountBuffer );
}

// destructor
CGrObjectSkySystem::~CGrObjectSkySystem()
{
	Free();
}


// Computes position of Sun in equatorial, rectangular, geocentric coords according to time of day
// Also counting with rotation of earth around its axis
// According to "How to compute planetary positions, by Paul Schlyter, Stockholm, Sweden"
D3DXVECTOR3 CGrObjectSkySystem::SunPosition(float UT)
{
	/*
	// Day chosen to be reference day
	double d; // time
	double UT; // day time - between 0 and 1
	double ecl = 0.40909296; // obliquity of the ecliptic - angle between Earth's axis and ecliptic in radians
	
	double N; // longitude of the ascending node
	double i; // inclination to the ecliptic plane of the Earth's orbit
	double w; // argument of perihelion
	double a; // semi-major axis, or mean distance from Sun
	double e; // eccentricity 
	double M; // mean anomaly
	double E; // eccentric anomaly

	double xv, yv;
	double v; // true anomaly
	double r; // sun distance
	double lonsun; // sun's true longitude

	double xs, ys; // sun's ecliptic rectangular geocentric coords
	D3DXVECTOR3 SunCoords; // sun's equatorial rectangular geocentric coords
	D3DXMATRIX EarthRotation; // Earth rotation in given time

	int Date = 367 * 2000 - 7 * (2000 + (6 + 9) / 12) /4 + 275 * 6/9 + 30 - 730530; // leto
	//int Date = 367 * 2000 - 7 * (2000 + (1 + 9) / 12) /4 + 275 * 1/9 + 0 - 730530; // zima
	//int Date = 367 * 2000 - 7 * (2000 + (10 + 9) / 12) /4 + 275 * 10/9 + 15 - 730530; // podzim

	// compute universal time 
	UT = ((double) Hour + (double) Minute / 60.0f + (double) Second / 3600.0f) / 24.0;

	d = (double) Date  + UT;

	// Init vars for Sun
	N = 0.0;
	i = 0.0;
	w = (282.9404 + 4.70935E-5 * d)  * (D3DX_PI / 180);
	a = 1.0; // in astronomical units
	e = (0.016709 - 1.151E-9 * d);
	M = (356.0470 + 0.9856002585 * d) * (D3DX_PI / 180);

	// compute sun's position
	E = M + e * sin(M) * (1.0 + e * cos(M) );
	xv = cos(E) - e;
	yv = sqrt(1.0 - e*e) * sin(E);
	v = atan2(yv, xv);
	r = sqrt(xv*xv + yv*yv);
	lonsun = v + w;

	// Ecliptic rectangular geocentric coordinates
	xs = r * cos(lonsun);
	ys = r * sin(lonsun);


	// Convert into equatorial rectangular geocentric coordinates
	SunCoords.x = (float) xs;
	SunCoords.y = (float) (ys * cos(ecl));
	SunCoords.z = (float) (ys * sin(ecl));
	
	
	// Rotate Sun acoording to rotation of Earth along it's axis so, that
	// at 12 o'clock sun is on it's heighest place
	// Rotating along Z and Y allowed. (When rotating about Y - we can simulate arctic day and night
	// depending on the date)
	// Momental setting - position on equator - sun rise about 6 o'clock, sun dawn about 18 o'clock
	//D3DXMatrixRotationZ(&EarthRotation,(float) ( (float) (Date % 365) / 365 + UT) * 2 * D3DX_PI);
	D3DXMatrixRotationZ(&EarthRotation,(float) UT * 2 * D3DX_PI);
	
	D3DXVec3TransformCoord(&SunCoords, &SunCoords, &EarthRotation);

	return SunCoords;
	*/

	D3DXMATRIX EarthRotation;

	D3DXMatrixRotationZ(&EarthRotation, -UT * 2 * D3DX_PI);

	D3DXVec3TransformCoord(&SunNormal, &StartSunNormal, &EarthRotation);

	return SunNormal;
}

// Renders sky system
HRESULT CGrObjectSkySystem::Render()
{
	D3DXMATRIX SunViewMat;
	D3DXMATRIX SunMat;
	D3DXVECTOR3 SunNormal2;
	UINT TimerTime;

	TimerTime = Timer.GetRunTime();
	UT += TimeAcceleration * ((float)(TimerTime - TimerLastTime) / 1000.0f) / 86400.0f;
	TimerLastTime = TimerTime;

	SunColor = SunColorByTime(UT);
	SunNormal = SunPosition(UT);
	D3DXVec3Normalize(&SunNormal, &SunNormal);

	// Compute sun view mat
	D3DXMatrixLookAtLH(&SunViewMat, &D3DXVECTOR3(0,0,0), &SunNormal, &D3DXVECTOR3(0.1f,1,0) );
	SunMat = SunViewMat * SunProjTexMat;

	// Set shader constants
	Shader->SetVSValue(SunNormalHandle, &SunNormal, sizeof(D3DXVECTOR3) );
	Shader->SetVSValue(SunColorHandle, &SunColor, sizeof(D3DXVECTOR3) );
	Shader->SetVSValue(MoleculeColorHandle, &MoleculeColor, sizeof(D3DXVECTOR3) );
	Shader->SetVSValue(AerosolColorHandle, &AerosolColor, sizeof(D3DXVECTOR3) );
	Shader->SetVSMatrix(SunMatHandle, &SunMat);

	Shader->SetPSValue(SunColorHandle, &SunColor, sizeof(D3DXVECTOR3) );

	D3DDevice->SetTexture(0, SunTexture);
// because ATI is not supporting D3DTADDRESS_BORDER in debug mode,
// we will use clamp instead when defined NO_BORDER_ADDRESS
#ifndef NO_BORDER_ADDRESS
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER );
#else
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
#endif
	//D3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	//D3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

	// Cloud
	/*
	D3DXMATRIX CloudViewMat, CloudMat;
	D3DXMATRIX TexMat;
	UINT CloudMatHandle;
	D3DXMatrixLookAtLH(&CloudViewMat, &D3DXVECTOR3(0, 0,0), &D3DXVECTOR3(0, 1,0), &D3DXVECTOR3(0, 0, 1) );
	
	D3DXMatrixPerspectiveFovLH(&CloudMat, D3DX_PI / 1.1f, 1.0f, 1.0f, 1000.0f);
	D3DXMatrixIdentity(&TexMat);
	TexMat(0,0) = 0.5f;
	TexMat(1,1) = -0.5f;
	TexMat(3,0) = 0.5f;
	TexMat(3,1) = 0.5f;
	CloudMat = CloudViewMat * CloudMat;
	CloudMat *= TexMat;
	
	Shader->GetHandleByName("CloudMat", &CloudMatHandle);
	Shader->SetVSMatrix(CloudMatHandle, &CloudMat);
	D3DDevice->SetTexture(1, Cloud);
	*/

	// Render sky dome
	SkyDome->DrawSubset(0);

	//D3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	//D3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	
	return 0;
}

// Renders clouds
HRESULT CGrObjectSkySystem::RenderClouds()
{
	
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

	// Cloud
	D3DXMATRIX CloudViewMat, CloudMat;
	D3DXMATRIX TexMat;
	D3DXMatrixLookAtLH(&CloudViewMat, &D3DXVECTOR3(0, 0,0), &D3DXVECTOR3(0, 1,0), &D3DXVECTOR3(0, 0, 1) );
	
	D3DXMatrixPerspectiveFovLH(&CloudMat, D3DX_PI / 1.1f, 1.0f, 1.0f, 1000.0f);
	D3DXMatrixIdentity(&TexMat);
	TexMat(0,0) = 0.5f;
	TexMat(1,1) = -0.5f;
	TexMat(3,0) = 0.5f;
	TexMat(3,1) = 0.5f;
	CloudMat = CloudViewMat * CloudMat;
	CloudMat *= TexMat;
	
	Shader->SetVSMatrix(CloudMatHandle, &CloudMat);
	D3DDevice->SetTexture(0, Cloud);

	// Render sky dome
	SkyDome->DrawSubset(0);
	
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	return 0;
};

// Render star field
HRESULT CGrObjectSkySystem::RenderStarField()
{
	D3DXMATRIX WorldStars;
	// render only if it's enough night
	if (SunColor.x < 0.2 && SunColor.y < 0.2 && SunColor.z < 0.2)
	{
		D3DXMatrixRotationZ(&WorldStars, -UT * 2 * D3DX_PI);
		Shader->SetWorldMatrix(&WorldStars);

		Shader->SetVSValue(SunColorHandle, &SunColor, sizeof(D3DXVECTOR3) );

		//D3DDevice->SetRenderState(D3DRS_POINTSIZE,(DWORD) 20.0f);

		D3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_PSIZE);
		D3DDevice->SetStreamSource(0, Stars, 0, sizeof(StarVertexStruct) );
		D3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, 1000);

		D3DXMatrixIdentity(&WorldStars);
		Shader->SetWorldMatrix(&WorldStars);

	}

	return 0;
}

// Renders mountains
HRESULT CGrObjectSkySystem::RenderMountains()
{
	
	//D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER );
	//D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER );
	D3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	D3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );

	D3DDevice->SetFVF( D3DFVF_TEXCOORDSIZE1(1) | D3DFVF_XYZ | D3DFVF_TEX2 );
		//D3DDevice->SetTexture( 0, Mountains[0] );
		//D3DDevice->SetStreamSource( 0, MountBuffer, 0, sizeof(MountainsVertexStruct) );
		//D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 1 );
	for ( int i = 0; i < 4; i++ )
	{
		D3DDevice->SetTexture( 0, Mountains[i] );
		D3DDevice->SetStreamSource( 0, MountBuffer, 0, sizeof(MountainsVertexStruct) );
		HRESULT hr = D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, i*6*3, 2*3 );
	}

	//D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	//D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	D3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	return 0;
}

// Returns sun color by time of day
D3DXVECTOR3 CGrObjectSkySystem::SunColorByTime(float UT)
{
	float Inter = (UT * 24) - floor(UT * 24);

	int Index1 = ((int) floor(UT * 24)) % 24;
	int Index2 = (Index1 + 1) % 24;

	return (1 - Inter) * SunColors[Index1] + SunColors[Index2] * Inter;
}






