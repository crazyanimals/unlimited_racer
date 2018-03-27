#pragma once
/*
Shader storage and setting implementation
by Pavel Celba
18. 10. 2004
*/

#include <map>
#include <vector>
#include <string>
#include <fstream>

#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"

// Undefine not to check, whether shader constant exist in particular shader
// and output it's non-existence as error
// #define WRONG_CONSTANT_SETTING_AS_ERROR


namespace graphic
{
	// Spot light shader structure
	struct SpotLightShaderStruct
	{
		D3DXVECTOR4 Ambient;
		D3DXVECTOR4 Diffuse;
		D3DXVECTOR4 Specular;
		D3DXVECTOR3 Position;
		D3DXVECTOR3 Direction;
		D3DXVECTOR3 Atten; // 1, D, D^2
		D3DXVECTOR3 Angles; // cos(theta/2), cos(phi/2), falloff
		float Range; // Range
	};

	// Spot light shader structure
	struct SpotLightShaderStructVector
	{
		// constructor
		SpotLightShaderStructVector();

		D3DXVECTOR4 Ambient;
		D3DXVECTOR4 Diffuse;
		D3DXVECTOR4 Specular;
		D3DXVECTOR4 Position;
		D3DXVECTOR4 Direction;
		D3DXVECTOR4 Atten; // 1, D, D^2, Range
		D3DXVECTOR4 Angles; // cos(theta/2), cos(phi/2), falloff
	};

	
	// Shader loading and setting class
	class CShaders
	{
	public:
		CShaders(IDirect3DDevice9 * Device);
		~CShaders();

		// Initializes CShaders class
		HRESULT Init();

		HRESULT LoadVertexShader(const resManNS::__VertexShader);
		HRESULT LoadPixelShader(const resManNS::__PixelShader);
		
		HRESULT SetVertexShader(const UINT VertexShader);
		HRESULT SetPixelShader(const UINT PixelShader);

		HRESULT GetHandleByName(const std::string Name, UINT * Handle);
		
		HRESULT SetTransforms(const D3DXMATRIX * World, const D3DXMATRIX * View, const D3DXMATRIX * Proj);
		HRESULT SetWorldMatrix(const D3DXMATRIX * Matrix);
		HRESULT SetViewMatrix(const D3DXMATRIX * Matrix);
		HRESULT SetProjMatrix(const D3DXMATRIX * Matrix);
		

		inline void BeginShaderConstantFetch()
		{
			ShaderConstantFetch = true;
		}

		inline void EndShaderConstantFetch()
		{
			ShaderConstantFetch = false;	
		}

		inline HRESULT SetPSValue(const UINT Handle, const void * Data, const UINT Size)
		{
			return SetPSValue(_CurPS, Handle, Data, Size);
		}
		inline HRESULT SetVSValue(const UINT Handle, const void * Data, const UINT Size)
		{
			return SetVSValue(_CurVS, Handle, Data, Size);
		}

		inline HRESULT SetVSMatrix(const UINT Handle, const D3DXMATRIX * Matrix)
		{
			return SetVSMatrix(_CurVS, Handle, Matrix);
		}

		inline HRESULT SetPSMatrix(const UINT Handle, const D3DXMATRIX * Matrix)
		{
			return SetPSMatrix(_CurPS, Handle, Matrix);
		}

		inline HRESULT SetVSBoolean(const UINT Handle, const BOOL Boolean)
		{
			return SetVSBoolean(_CurVS, Handle, Boolean);
		}	

		inline HRESULT SetPSBoolean(const UINT Handle, const BOOL Boolean)
		{
			return SetPSBoolean(_CurPS, Handle, Boolean);
		}

		inline std::vector<UINT> * GetExtendedTable(const UINT Handle)
		{
			return _ExtendedTables[Handle];
		}

		inline HRESULT SetVSFloatArray(const UINT Handle, const FLOAT * Data, const UINT Size)
		{
			return SetVSFloatArray(_CurVS, Handle, Data, Size);
		}

		inline HRESULT	SetVSVectorArray(const UINT Handle, const D3DXVECTOR4 * Data, const UINT Size)
		{
			return SetVSVectorArray(_CurVS, Handle, Data, Size);
		}

		
		HRESULT SetVSVectorArray(const UINT ShaderHandle, const UINT Handle, const D3DXVECTOR4 * Data, const UINT Size);

		HRESULT SetVSFloatArray(const UINT ShaderHandle, const UINT Handle, const FLOAT * Data, const UINT Size);

		HRESULT SetVSValue(const UINT ShaderHandle, const UINT Handle, const void * Data, const UINT Size);
		HRESULT SetPSValue(const UINT ShaderHandle, const UINT Handle, const void * Data, const UINT Size);

		HRESULT SetVSMatrix(const UINT ShaderHandle, const UINT Handle, const D3DXMATRIX * Matrix);
		HRESULT SetPSMatrix(const UINT ShaderHandle, const UINT Handle, const D3DXMATRIX * Matrix);

		HRESULT SetVSBoolean(const UINT ShaderHandle, const UINT Handle, const BOOL Boolean);
		HRESULT SetPSBoolean(const UINT ShaderHandle, const UINT Handle, const BOOL Boolean);

		HRESULT SetVSMaterial( const D3DMATERIAL9 * Material);
		HRESULT SetVSSingleDirLight(const D3DLIGHT9 * Light);
		HRESULT SetVSSingleSpotLight(const D3DLIGHT9 * Light);

		// Material structure as used in shaders
		struct Material 
		{
			// constructor
			Material(const D3DCOLORVALUE _Diffuse, const D3DCOLORVALUE _Specular, const D3DCOLORVALUE _Ambient)
			{
				Diffuse = _Diffuse;
				Specular = _Specular;
				Ambient = _Ambient;
			};
			
			// constructor
			Material(const D3DMATERIAL9 Material)
			{
				Diffuse = Material.Diffuse;
				Specular = Material.Specular;
				Ambient = Material.Ambient;
			};

			D3DCOLORVALUE Diffuse;
			D3DCOLORVALUE Specular;
			D3DCOLORVALUE Ambient;
		};

		// Directional light structure as used in shaders
		struct DirectionalLight
		{
			// constructor
			DirectionalLight(const D3DVECTOR _Direction, const D3DCOLORVALUE _Diffuse, const D3DCOLORVALUE _Specular)
			{
				Direction = _Direction;
				Diffuse = _Diffuse;
				Specular = _Specular;
			};

			// constructor
			DirectionalLight(const D3DLIGHT9 Light)
			{
				// Reverse direction
				Direction = Light.Direction;
				Diffuse = Light.Diffuse;
				Specular = Light.Specular;
			};

			D3DXVECTOR3 Direction;
			D3DCOLORVALUE Diffuse;
			D3DCOLORVALUE Specular;
		};

		
		// Preliminary added names and their handle values
		enum Handles
		{
			AllMat = 1,
			WorldViewMat,
			ViewMat,
			WorldMat,
			ProjMat,
			Material,
			MaterialPower,
			Ambient,
			Specular,
			DirLight,
			SpotLight
		};

// For debugging purposes
#ifdef _DEBUG
		void ListHandles(std::string FileName);
#endif

	private:
		// data
		IDirect3DDevice9 * _Device;
		std::vector< resManNS::__VertexShader > _VertexShaders;
		std::vector< resManNS::__PixelShader > _PixelShaders;
		std::vector< std::vector<D3DXHANDLE> * > _PixelHandles;
		std::vector< std::vector<D3DXHANDLE> * > _VertexHandles;
		
		// constant name mapping to unique handles
		std::map<std::string , UINT> _Handles;
		UINT _HandleCount;

		// Current pixel and vertex shader
		UINT _CurVS, _CurPS;
		ID3DXConstantTable * _CurPSTable;
		ID3DXConstantTable * _CurVSTable;

		// World, View, Proj matrixes
		D3DXMATRIX _WorldMat, _ViewMat, _ProjMat, _WorldViewMat, _AllMat;
		// Handle
		D3DXHANDLE _HWorldMat, _HViewMat, _HProjMat, _HWorldViewMat, _HAllMat;

		// Extended tables
		std::vector< std::vector<UINT> * > _ExtendedTables;

		// Constant descriptions - contains data for constant member validation control
		std::vector< D3DXCONSTANT_DESC > _ConstantDescriptions;

		// Only when this is true, constant handles can be returned when asking by name
		BOOL ShaderConstantFetch;

		// functions
		HRESULT InsertHandles(ID3DXConstantTable * ConstTable, std::vector<D3DXHANDLE> * ShaderHandles);
		HRESULT InsertHandlesStruct(UINT OurHandle, D3DXHANDLE Handle, UINT NumStructElems,  ID3DXConstantTable * ConstTable, 
			std::vector<D3DXHANDLE> * ShaderHandles, BOOL Inserted, UINT & Order);
		HRESULT InsertHandlesElem(UINT OurHandle, D3DXHANDLE Handle, UINT NumElems, ID3DXConstantTable * ConstTable, 
			std::vector<D3DXHANDLE> * ShaderHandles, BOOL Inserted, UINT & Order);

		HRESULT CheckHandle(UINT Handle, D3DXCONSTANT_DESC & Desc);

		D3DXCONSTANT_DESC MatrixDescription(LPCSTR Name);
		D3DXCONSTANT_DESC VectorDescription(LPCSTR Name, int Size = 4);

		HRESULT AddDefaultHandles();
		void UpdateVSMatrixes();

		// Bit map manipulation functions
		HRESULT ResizeAllBitMaps(std::vector< std::vector<D3DXHANDLE> * > * BitMaps, UINT NewSize);
		HRESULT AddNewBitMap(std::vector< std::vector<D3DXHANDLE> * > * BitMaps, UINT InitSize = 1);
		HRESULT DeleteAllBitMaps(std::vector< std::vector<D3DXHANDLE> * > * BitMaps);
	};

};