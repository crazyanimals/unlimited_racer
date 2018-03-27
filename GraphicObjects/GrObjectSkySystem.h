/*
Sky system - dynamic sky system with sky color change according to Sun. Also with Moon.
by Pavel Celba
1. 1. 2005
Last Update 18.1.2006 by RM
*/
#pragma once

#include "GrObjectBase.h"
#include "..\Globals\TimeCounter.h"


namespace graphic
{
	class CGrObjectSkySystem: public CGrObjectBase
	{
	public:
		virtual HRESULT		Init(	BOOL Static,
									CString fileName = "", 
									UINT LODcount = 0 ); // inits this object
		inline virtual int	GetObjectType() { return GROBJECT_TYPE_SKYBOX; }; // returns type of object

		virtual	HRESULT		Render();
		
		// Renders star field
		HRESULT RenderStarField();

		// Renders mountains
		HRESULT RenderMountains();

		// Renders clouds
		HRESULT RenderClouds();

	// constructor 
	CGrObjectSkySystem();

	// destructor
	~CGrObjectSkySystem();

	// free sky system resources
	void Free();

	// Computes position of Sun in equatorial, rectangular, geocentric coords according to time of day
	// Also counting with rotation of earth around its axis
	D3DXVECTOR3 SunPosition(float UT);

	// Returns sun color by time of day
	D3DXVECTOR3 SunColorByTime(float UT);

	// Sets molecule color - effective for setting sky color around sun
	inline void SetMoleculeColor(D3DXVECTOR3 & NewColor)
	{
		MoleculeColor = NewColor;
	}

	// Sets aerosol color - effective for setting general sky color
	inline void SetAerosolColor(D3DXVECTOR3 & NewColor)
	{
		AerosolColor = NewColor;
	}

	// Sets sun color in hour - used for interpolating sun color during day
	// Hour between 0 and 23
	inline HRESULT SetSunColorInHour(int Hour, D3DXVECTOR3 & NewColor)
	{
		if (Hour < 0 || Hour > 23) return -1;
		SunColors[Hour] = NewColor;

		return 0;
	}

	// Set time
	// format is: 1.0 is one day
	inline void SetSkyTime(float NewTime)
	{
		UT = NewTime;	
	}

	// Set time acceleration - acceleration of time flow
	// Setting time acceleration to 0 will fix current sky time
	inline void SetTimeAcceleration(float NewAcceleration)
	{
		TimeAcceleration = NewAcceleration;
	}	

	// Returns time and date
	// Format is floating point, where 1.0 is one day
	inline float GetSkyTime()
	{
		return UT;
	}

	// Returns day start
	inline float GetDayStart()
	{
		return DayStart;
	};

	// Returns day end
	inline float GetDayEnd()
	{
		return DayEnd;
	};

	// Sets day start
	inline float SetDayStart(float NewStart)
	{
		DayStart = NewStart;
	};

	// Sets day end
	inline float SetDayEnd(float NewEnd)
	{
		DayEnd = NewEnd;
	};


	// Returns molecule color
	inline D3DXVECTOR3 GetMoleculeColor()
	{
		return MoleculeColor;
	}

	// Returns aerosol color
	inline D3DXVECTOR3 GetAerosolColor()
	{
		return AerosolColor;
	}

	// Returns sun color - current color
	inline D3DXVECTOR3 GetSunColor()
	{
		SunColor = SunColorByTime(UT);
		return SunColor;
	}

	// Returns sun color in particular hour
	inline HRESULT GetSunColorInHour(int Hour, D3DXVECTOR3 * Color)
	{
		if (Hour < 0 || Hour > 23) return -1;
		(*Color) = SunColors[Hour];

		return 0;
	}
	
	// Returns sun normal - the direction of sun shining
	inline D3DXVECTOR3 GetSunNormal()
	{
		return -SunNormal;
	}

	// Get time acceleration
	inline float GetTimeAcceleration()
	{
		return TimeAcceleration;
	}

	// returns true during DayTime
	inline bool IsDay()
	{
		float frac, n;
		frac = modff(GetSkyTime(), &n);
		return frac > GetDayStart() && frac < GetDayEnd();
	}


	private:
		// Sky system resource
		resManNS::RESOURCEID SkySystemResourceID;

		// Sky dome mesh
		ID3DXMesh * SkyDome;

		// Sky dome FVF
		UINT SkySystemFVF;

		// Sky dome vertex data structure
		struct SkySystemVertex
		{
			float x, y, z; // position
			float nx, ny, nz; // normal
		};

		// Sun normal used to compute molecule and aerosol scattering
		D3DXVECTOR3 SunNormal;
		D3DXVECTOR3 StartSunNormal;

		UINT SunNormalHandle;
		// Sun color
		D3DXVECTOR3 SunColor;
		UINT SunColorHandle;
		// Molecule color
		D3DXVECTOR3 MoleculeColor;
		UINT MoleculeColorHandle;
		// Aerosol color
		D3DXVECTOR3 AerosolColor;
		UINT AerosolColorHandle;

		// Handle to cloud matrix
		UINT CloudMatHandle;

		// Sun texture
		IDirect3DTexture9 * SunTexture;
	
		// Sun matrix handle
		UINT SunMatHandle;

		// Sun projection - texture matrix
		D3DXMATRIX SunProjTexMat;

		// Sun colors during day by hour
		D3DXVECTOR3 SunColors[24];

		// Stars
		IDirect3DVertexBuffer9 * Stars;

		struct StarVertexStruct
		{
			float x,y,z;
			float PointSize;
			DWORD Color;
		};

		// Mountains

		// Mountain textures - north, west, south, east
		IDirect3DTexture9 * Mountains[4];
		// vertex buffers for mountain rendering
		IDirect3DVertexBuffer9 * MountBuffer;

		struct MountainsVertexStruct
		{
			float x, y, z;
			float tu, tv;
			float Alpha;
		};

		// Clouds
		IDirect3DTexture9 * Cloud;
		
		// Date and time
		float UT;

		// Start and end of day
		// this is used to determine, when to cast shadows from sun
		// values are between 0 and 1, where 0 is 0:00 and 1 is 24:00
		float DayStart, DayEnd;

		// Timer
		CTimeCounter Timer;
		// Timer last time
		UINT TimerLastTime;
		// the amount by which normal flow of time is accelerated 
		float TimeAcceleration;

		int Counter;
	};
};