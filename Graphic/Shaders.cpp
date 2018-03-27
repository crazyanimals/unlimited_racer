#pragma once
/*
Shaders.cpp - Shader storage and setting implementation
by Pavel Celba
18. 10. 2004
*/

#include "Shaders.h"

using namespace graphic;
using namespace std;

// constructor
CShaders::CShaders(IDirect3DDevice9 * Device)
{
	_Device = Device;
	_HandleCount = 0;
	_CurVSTable = 0;
	_CurPSTable = 0;
	_CurVS = -1;
	_CurPS = -1;

	// Handle init
	_HWorldMat = 0;
	_HViewMat = 0;
	_HProjMat = 0;
	_HWorldViewMat = 0;
	_HAllMat = 0;

	// init matrixes to identity
	D3DXMatrixIdentity(&_WorldMat);
	D3DXMatrixIdentity(&_ViewMat);
	D3DXMatrixIdentity(&_ProjMat);
	D3DXMatrixIdentity(&_WorldViewMat);
	D3DXMatrixIdentity(&_AllMat);

	ShaderConstantFetch = false;
}

// Initializes CShaders class
HRESULT CShaders::Init()
{
	HRESULT hr;

	// Shader infos
	resManNS::__VertexShader VSInfo;
	resManNS::__PixelShader PSInfo;

	// Some handles are added preliminary in order to know their number
	hr = AddDefaultHandles();
	if (FAILED(hr) ) 
		ERRORMSG(hr, "CShaders::Init()", "Unable to add default handles.");

	// Add zero shader to first place (for being able to set fixed function pipeline)
	VSInfo.ConstTable = 0;
	VSInfo._VS = 0;

	PSInfo.ConstTable = 0;
	PSInfo._PS = 0;

	_VertexShaders.push_back(VSInfo);
	_PixelShaders.push_back(PSInfo);
	
	hr = AddNewBitMap(&_VertexHandles, _HandleCount);
	if (FAILED(hr) )
		ERRORMSG(hr, "CShaders::Init()", "Unable to add new bitmap.");
	hr = AddNewBitMap(&_PixelHandles, _HandleCount);
	if (FAILED(hr) )
		ERRORMSG(hr, "CShaders::Init()", "Unable to add new bitmap.");


	return ERRNOERROR;
}

// destructor
CShaders::~CShaders()
{
	UINT i;
	
	// Clear bit maps
	DeleteAllBitMaps(&_VertexHandles);
	DeleteAllBitMaps(&_PixelHandles);

	// delete Extended tables
	for (i = 0; i < _ExtendedTables.size(); i++)
		if (_ExtendedTables[i])
			delete _ExtendedTables[i];
}

// Vertex shader loading
HRESULT CShaders::LoadVertexShader(const resManNS::__VertexShader Shader)
{
	HRESULT Result;
	size_t CurrentMap;

	// iterator
	map<string, UINT>::iterator It;

	// Add vertex shader to vertex shaders and assign him handle (position in dyn. array is handle)
	_VertexShaders.push_back( Shader);
	
	// Add new bit map to vertex handles
	CurrentMap = _VertexHandles.size();
	Result = AddNewBitMap(&_VertexHandles);
	if (FAILED(Result) )
		ERRORMSG(Result, "CShaders::LoadVertexShader()", "Unable to add new bitmap.");

	// Insert handles
	Result = InsertHandles( Shader.ConstTable, _VertexHandles.at(CurrentMap) );
	if (FAILED(Result) )
	{
		_VertexShaders.resize(_VertexShaders.size() - 1);
		delete _VertexHandles.back();
		_VertexShaders.resize(_VertexHandles.size() - 1);
		ERRORMSG(-1, "CShaders::LoadPixelShader()", "Unable to load vertex shader.");
	}

	// Set defaults
	Shader.ConstTable->SetDefaults(_Device);

	return (HRESULT) _VertexShaders.size() - 1;
}

// Pixel shader loading
HRESULT CShaders::LoadPixelShader(const resManNS::__PixelShader Shader)
{
	HRESULT Result;
	size_t CurrentMap;

	// iterator
	map<string, UINT>::iterator It;
	
	// Add vertex shader to vertex shaders and assign him handle (position in dyn. array is handle)
	_PixelShaders.push_back( Shader);

	// Add new bit map to pixel handles
	CurrentMap = _PixelHandles.size();
	Result = AddNewBitMap(&_PixelHandles);
	if (FAILED(Result) )
		ERRORMSG(Result, "CShaders::LoadPixelShader()", "Unable to add new bitmap.");

	// Insert handles
	Result = InsertHandles(Shader.ConstTable, _PixelHandles.at(CurrentMap) );
	if (FAILED(Result) )
	{
		_PixelShaders.resize(_PixelShaders.size() - 1);
		delete _PixelHandles.back();
		_PixelHandles.resize(_PixelHandles.size() - 1);
		ERRORMSG(-1, "CShaders::LoadPixelShader()", "Unable to load pixel shader.");
	}
	
	// Set defaults
	Shader.ConstTable->SetDefaults(_Device);
	
	return (HRESULT) _PixelShaders.size() - 1;
}

// Set vertex shader to graphic card
HRESULT CShaders::SetVertexShader(const UINT VertexShader)
{
	// Check for setting back fixed function pipeline
	if (! VertexShader )
	{
		_Device->SetVertexShader(0);
		_CurVS = 0;
		_CurVSTable = 0;
		_HWorldMat = 0;
		_HViewMat = 0;
		_HProjMat = 0;
		_HWorldViewMat = 0;
		_HAllMat = 0;
		return 0;
	}

	_CurVS = VertexShader;
	_CurVSTable = _VertexShaders[VertexShader].ConstTable;
	_Device->SetVertexShader( _VertexShaders[VertexShader]._VS );

	// Set defaults
	_CurVSTable->SetDefaults(_Device);

	// Look for all matrixes, which are important
	_HWorldMat = _VertexHandles[_CurVS]->at(WorldMat);
	_HViewMat = _VertexHandles[_CurVS]->at(ViewMat);
	_HProjMat = _VertexHandles[_CurVS]->at(ProjMat);
	_HWorldViewMat = _VertexHandles[_CurVS]->at(WorldViewMat);
	_HAllMat = _VertexHandles[_CurVS]->at(AllMat);
	
	// Update them to shader
	UpdateVSMatrixes();

	return 0;
}

// Set pixel shader to graphic card
HRESULT CShaders::SetPixelShader(const UINT PixelShader)
{
	// Check for setting back fixed function pipeline
	if (! PixelShader )
	{
		_Device->SetPixelShader(0);
		return 0;
	}

	_CurPS = PixelShader;
	_CurPSTable = _PixelShaders[PixelShader].ConstTable;
	_Device->SetPixelShader( _PixelShaders[PixelShader]._PS );

	_CurPSTable->SetDefaults(_Device);

	return 0;
}


// Set pixel shader constant value
HRESULT CShaders::SetPSValue(const UINT PSHandle, const UINT Handle, const void * Data, const UINT Size)
{
	D3DXHANDLE D3DXHandle;

	D3DXHandle = _PixelHandles[PSHandle]->at(Handle);

	if (!D3DXHandle)
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(Handle, "CShaders::SetPSValue()", "Handle not found.");
#else
		return 0; //thread silently as no error
#endif
	
	_PixelShaders[PSHandle].ConstTable->SetValue(_Device, D3DXHandle, Data, Size);
	
	return 0;
}

// Set pixel shader matrix value
HRESULT CShaders::SetPSMatrix(const UINT PSHandle, const UINT Handle, const D3DXMATRIX * Matrix)
{
	D3DXHANDLE D3DXHandle;
	
	// find handle in handle map for current pixel shader
	
	D3DXHandle = _PixelHandles[PSHandle]->at(Handle);

	if ( !D3DXHandle )
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(Handle, "CShaders::SetPSValue()", "Handle not found.");
#else
		return 0; // thread silently as no error
#endif
	
	_PixelShaders[PSHandle].ConstTable->SetMatrix(_Device, D3DXHandle, Matrix);

	return 0;
}

HRESULT CShaders::SetPSBoolean(const UINT PSHandle, const UINT Handle, const BOOL Boolean)
{
	D3DXHANDLE D3DXHandle;
	
	// find handle in handle map for current pixel shader
	
	D3DXHandle = _PixelHandles[PSHandle]->at(Handle);

	if ( !D3DXHandle )
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(Handle, "CShaders::SetPSValue()", "Handle not found.");
#else
		return 0; // thread silently as no error
#endif

	_PixelShaders[PSHandle].ConstTable->SetBool(_Device, D3DXHandle, Boolean);

	return 0;
}


// set vertex shader constant value
HRESULT CShaders::SetVSValue(const UINT VSHandle, const UINT Handle, const void * Data, const UINT Size)
{
	D3DXHANDLE D3DXHandle;

	// find handle in handle map for current vertex shader
	
	D3DXHandle = _VertexHandles[VSHandle]->at(Handle);
	if ( !D3DXHandle )
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(Handle, "CShaders::SetVSValue()", "Handle not found.");
#else
		return 0; // thread silently as no error
#endif

	_VertexShaders[VSHandle].ConstTable->SetValue(_Device, D3DXHandle, Data, Size);

	return 0;
}

// Set vertex shader constant float array
// Size is number of float to set
HRESULT CShaders::SetVSFloatArray(const UINT VSHandle, const UINT Handle, const FLOAT * Data, const UINT Size)
{
	D3DXHANDLE D3DXHandle;
	
	// find handle in handle map for current vertex shader
	D3DXHandle = _VertexHandles[VSHandle]->at(Handle);
	if ( !D3DXHandle )
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(Handle, "CShaders::SetVSValue()", "Handle not found.");
#else
		return 0; // thread silently as no error
#endif

	_VertexShaders[VSHandle].ConstTable->SetFloatArray(_Device, D3DXHandle, Data, Size);

	return 0;
}

// Set vertex shader vector array
// Size is number of vectors in array
HRESULT CShaders::SetVSVectorArray(const UINT VSHandle, const UINT Handle, const D3DXVECTOR4 * Data, const UINT Size)
{
	D3DXHANDLE D3DXHandle;

	// find handle in handle map for current vertex shader
	D3DXHandle = _VertexHandles[VSHandle]->at(Handle);

	if ( !D3DXHandle )
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(Handle, "CShaders::SetVSValue()", "Handle not found.");
#else
		return 0; // thread silently as no error
#endif

	_VertexShaders[VSHandle].ConstTable->SetVectorArray(_Device, D3DXHandle, Data, Size);

	return 0;
}

// set vertex shader matrix constant value
HRESULT CShaders::SetVSMatrix(const UINT VSHandle, const UINT Handle, const D3DXMATRIX * Matrix)
{
	D3DXHANDLE D3DXHandle;

	// find handle in handle map for current vertex shader
	D3DXHandle = _VertexHandles[VSHandle]->at(Handle);

	if ( !D3DXHandle )
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(Handle, "CShaders::SetVSValue()", "Handle not found.");
#else
		return 0; // thread silently as no error
#endif

	_VertexShaders[VSHandle].ConstTable->SetMatrix(_Device, D3DXHandle, Matrix);

	return 0;
}

// Set vertex shader boolean constant value
HRESULT CShaders::SetVSBoolean(const UINT VSHandle, const UINT Handle, const BOOL Boolean)
{
	D3DXHANDLE D3DXHandle;

	// find handle in handle map for current vertex shader
	D3DXHandle = _VertexHandles[VSHandle]->at(Handle);

	if (!D3DXHandle)
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(Handle, "CShaders::SetVSValue()", "Handle not found.");
#else
		return 0; // thread silently as no error
#endif

	_VertexShaders[VSHandle].ConstTable->SetBool(_Device, D3DXHandle, Boolean);
	
	return 0;
}
		

// Gets handle 
HRESULT CShaders::GetHandleByName(const std::string Name, UINT * Handle)
{
	map<string, UINT>::iterator It;

	if (!ShaderConstantFetch)
	{
#ifdef _DEBUG
		ERRORMSG(-1, "CShaders::GetHandleByName()", "Handles can be retrieved by name only in handle fetch phase.");
#endif
		return 0;
	}

	It = _Handles.find(Name);
#ifdef _DEBUG
	if (It == _Handles.end() )
		ERRORMSG(-1, "CShaders::GetHandleByName()", "No handle of such name exist");
#endif
	*Handle = It->second;

	return 0;
}

// add default handle value to name -> handle map
HRESULT CShaders::AddDefaultHandles()
{
	D3DXCONSTANT_DESC Desc;

	// Matrixes 
	_Handles.insert(pair<string, UINT>(string("AllMat"), AllMat ) );
	_Handles.insert(pair<string, UINT>(string("WorldViewMat"), WorldViewMat) );
	_Handles.insert(pair<string, UINT>(string("ViewMat"), ViewMat));
	_Handles.insert(pair<string, UINT>(string("WorldMat"), WorldMat));
	_Handles.insert(pair<string, UINT>(string("ProjMat"), ProjMat));
	
	// Materials
	_Handles.insert(pair<string, UINT>(string("Material"), Material) );
	_Handles.insert(pair<string, UINT>(string("MaterialPower"), MaterialPower) );

	// Lighting
	_Handles.insert(pair<string, UINT>(string("Ambient"), Ambient) );
	_Handles.insert(pair<string, UINT>(string("Specular"), Specular) );
	_Handles.insert(pair<string, UINT>(string("DirLight"), DirLight) );
	_Handles.insert(pair<string, UINT>(string("SpotLight"),SpotLight) );
	

	// !!! DO NOT FORGET TO UPDATE !!!
	_HandleCount = 25;

	// Description structures
	ZeroMemory(&Desc, sizeof(D3DXCONSTANT_DESC) );
	_ConstantDescriptions.push_back(Desc);
	Desc = MatrixDescription("AllMat");
	_ConstantDescriptions.push_back(Desc);
	Desc = MatrixDescription("WorldViewMat");
	_ConstantDescriptions.push_back(Desc);
	Desc = MatrixDescription("ViewMat");
	_ConstantDescriptions.push_back(Desc);
	Desc = MatrixDescription("WorldMat");
	_ConstantDescriptions.push_back(Desc);
	Desc = MatrixDescription("ProjMat");
	_ConstantDescriptions.push_back(Desc);
	

	Desc.Bytes = 48;
	Desc.Columns = 12;
	Desc.Elements = 1;
	Desc.Name = "Material";
	Desc.Rows = 1;
	Desc.StructMembers = 3;
	Desc.Type = D3DXPT_VOID;

	_ConstantDescriptions.push_back(Desc);
	
	Desc.Bytes = 4;
	Desc.Columns = 1;
	Desc.Elements = 1;
	Desc.Name = "MaterialPower";
	Desc.Rows = 1;
	Desc.StructMembers = 0;
	Desc.Type = D3DXPT_FLOAT;

	_ConstantDescriptions.push_back(Desc);

	Desc = VectorDescription("Ambient");
	_ConstantDescriptions.push_back(Desc);
	
	Desc.Bytes = 4;
	Desc.Columns = 1;
	Desc.Elements = 1;
	Desc.Name = "Specular";
	Desc.Rows = 1;
	Desc.StructMembers = 0;
	Desc.Type = D3DXPT_BOOL;
	_ConstantDescriptions.push_back(Desc);

	Desc.Bytes = 60;
	Desc.Columns = 15;
	Desc.Elements = 1;
	Desc.Name = "DirLight";
	Desc.Rows = 1;
	Desc.StructMembers = 4;
	Desc.Type = D3DXPT_VOID;
	_ConstantDescriptions.push_back(Desc);

	Desc.Bytes = 112;
	Desc.Columns = 28;
	Desc.Elements = 1;
	Desc.Name = "SpotLight";
	Desc.Rows = 1;
	Desc.StructMembers = 7;
	Desc.Type = D3DXPT_VOID;
	_ConstantDescriptions.push_back(Desc);

	// Material constant description
	Desc = VectorDescription("Diffuse");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Specular");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Ambient");
	_ConstantDescriptions.push_back(Desc);

	// Dir light constant description
	Desc = VectorDescription("Ambient");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Diffuse");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Specular");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Direction", 3);
	_ConstantDescriptions.push_back(Desc);

	// Spot light constant description
	Desc = VectorDescription("Ambient");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Diffuse");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Specular");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Position");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Direction");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Atten");
	_ConstantDescriptions.push_back(Desc);
	Desc = VectorDescription("Angles");
	_ConstantDescriptions.push_back(Desc);
	
	// Extended tables
	_ExtendedTables.resize(_HandleCount + 1, 0);
	// Add extended info to material structure
	_ExtendedTables[6] = new vector<UINT>();
	if (! _ExtendedTables[6] )
		ERRORMSG(ERROUTOFMEMORY, "CShaders::AddDefaultHandles()", "Unable to allocate place for default handles.");
	_ExtendedTables[6]->push_back(12);
	_ExtendedTables[6]->push_back(13);
	_ExtendedTables[6]->push_back(14);

	// add extended info for directional light
	_ExtendedTables[10] = new vector<UINT>();
	if (! _ExtendedTables[10] )
		ERRORMSG(ERROUTOFMEMORY, "CShaders::AddDefaultHandles()", "Unable to allocate place for default handles.");
	_ExtendedTables[10]->push_back(15);
	_ExtendedTables[10]->push_back(16);
	_ExtendedTables[10]->push_back(17);
	_ExtendedTables[10]->push_back(18);

	// add extended info for spot light
	_ExtendedTables[11] = new vector<UINT>();
	if (! _ExtendedTables[11] )
		ERRORMSG(ERROUTOFMEMORY, "CShaders::AddDefaultHandles()", "Unable to allocate place for defaut handles.");
	_ExtendedTables[11]->push_back(19);
	_ExtendedTables[11]->push_back(20);
	_ExtendedTables[11]->push_back(21);
	_ExtendedTables[11]->push_back(22);
	_ExtendedTables[11]->push_back(23);
	_ExtendedTables[11]->push_back(24);
	_ExtendedTables[11]->push_back(25);

	return ERRNOERROR;
}

// Set material to VS
HRESULT CShaders::SetVSMaterial(const D3DMATERIAL9 * Mat)
{
	D3DXHANDLE MatHandle;
	vector<UINT> * ExtendedTable;
	
	// find handle in handle map for current vertex shader
	MatHandle = _VertexHandles[_CurVS]->at(Material);
	
	if ( !MatHandle )
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(_CurVS, "CShaders::SetVSValue()", "Handle not found.");
#else
		return 0; // threat silently as no error
#endif

	ExtendedTable = GetExtendedTable(Material);
	
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(0)), &Mat->Diffuse, 16);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(1)), &Mat->Specular, 16);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(2)), &Mat->Ambient, 16);

	SetVSValue(MaterialPower, &Mat->Power, sizeof(float) );

	return 0;
}

// Set directional light to VS
HRESULT CShaders::SetVSSingleDirLight(const D3DLIGHT9 * Light)
{
	D3DXHANDLE LightHandle;
	vector<UINT> * ExtendedTable;

	// find handle in handle map for current vertex shader
	LightHandle = _VertexHandles[_CurVS]->at(DirLight);
	
	if ( !LightHandle )
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(_CurVS, "CShaders::SetVSValue()", "Handle not found.");
#else
		return 0; // threat silently as no error;
#endif

	ExtendedTable = GetExtendedTable(DirLight);

	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(0)), &Light->Ambient, 16);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(1)), &Light->Diffuse, 16);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(2)), &Light->Specular, 16);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(3)), &Light->Direction, 12);

	return 0;
}

HRESULT CShaders::SetVSSingleSpotLight(const D3DLIGHT9 * Light)
{
	D3DXHANDLE LightHandle;
	vector<UINT> * ExtendedTable;
	D3DXVECTOR4 Temp;

	// find handle in handle map for current vertex shader
	LightHandle = _VertexHandles[_CurVS]->at(SpotLight);

	if ( !LightHandle )
#ifdef WRONG_CONSTANT_SETTING_AS_ERROR
		ERRORMSG(_CurVS, "CShaders::SetVSValue()", "Handle not found.");
#else
		return 0; // threat silently as no error
#endif
	
	ExtendedTable = GetExtendedTable(SpotLight);

	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(0)), &Light->Ambient, 16);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(1)), &Light->Diffuse, 16);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(2)), &Light->Specular, 16);
	Temp = D3DXVECTOR4(Light->Position.x, Light->Position.y, Light->Position.z, 1);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(3)), &Temp, 16);
	Temp = D3DXVECTOR4(Light->Direction.x, Light->Direction.y, Light->Direction.z, 0);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(4)), &Temp, 16);
	Temp = D3DXVECTOR4(Light->Attenuation0, Light->Attenuation1, Light->Attenuation2, Light->Range);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(5)), &Temp, 16);
	Temp = D3DXVECTOR4(cos(Light->Theta / 2), cos(Light->Phi / 2), Light->Falloff, 0.0f);
	_CurVSTable->SetValue(_Device, _VertexHandles[_CurVS]->at(ExtendedTable->at(6)), &Temp, 16);

	return 0;
}

// Update all matrixes, which are important to shader
void CShaders::UpdateVSMatrixes()
{
	if (_CurVSTable != 0)
	{
		if (_HWorldMat)
			_CurVSTable->SetMatrix(_Device, _HWorldMat, &_WorldMat);
		if (_HViewMat)
			_CurVSTable->SetMatrix(_Device, _HViewMat, &_ViewMat);
		if (_HProjMat)
			_CurVSTable->SetMatrix(_Device, _HProjMat, &_ProjMat);
		if (_HWorldViewMat)
		{	
			D3DXMatrixMultiply(&_WorldViewMat, &_WorldMat, &_ViewMat);
			_CurVSTable->SetMatrix(_Device, _HWorldViewMat, &_WorldViewMat);
		}
		if (_HAllMat)
		{
			_AllMat = _WorldMat * _ViewMat * _ProjMat;
			_CurVSTable->SetMatrix(_Device, _HAllMat, &_AllMat);
		}
	}
}


HRESULT CShaders::SetTransforms(const D3DXMATRIX * World, const D3DXMATRIX * View, const D3DXMATRIX * Proj)
{
	_WorldMat = *World;
	_ViewMat = *View;
	_ProjMat = *Proj;

	UpdateVSMatrixes();

	return 0;
}

HRESULT CShaders::SetWorldMatrix(const D3DXMATRIX * Matrix)
{
	_WorldMat = *Matrix;
	
	UpdateVSMatrixes();

	return 0;
}

HRESULT CShaders::SetViewMatrix(const D3DXMATRIX * Matrix)
{
	_ViewMat = *Matrix;
	UpdateVSMatrixes();

	return 0;
}

HRESULT CShaders::SetProjMatrix(const D3DXMATRIX * Matrix)
{
	_ProjMat = *Matrix;
	UpdateVSMatrixes();

	return 0;
}

// Resizes all bit maps
// Precondition:
// Must be called with at least one bit map
HRESULT CShaders::ResizeAllBitMaps(std::vector< std::vector<D3DXHANDLE> * > * BitMaps, UINT NewSize)
{
	UINT i;
	
	// Resize all bit maps of master bit map
	for (i = 0; i < BitMaps->size(); i++)
	{
		BitMaps->at(i)->resize(NewSize + 1);
		BitMaps->at(i)->at(NewSize) = 0;
	}

	return ERRNOERROR;
}

// Adds new bit map to bit map field
HRESULT CShaders::AddNewBitMap(std::vector< std::vector<D3DXHANDLE> * > * BitMaps, UINT InitSize)
{
	vector<D3DXHANDLE> * NewBitMap = new vector<D3DXHANDLE>();
	size_t Size;

	if (! NewBitMap)
		ERRORMSG(ERROUTOFMEMORY, "CShaders::AddNewBitMap()", "Unable to allocate place for new bitmap.");

	Size = BitMaps->size();

	if (Size == 0)
	{
		// add bit map and into it add the number of standard handles
		BitMaps->push_back( NewBitMap );
		for (UINT i = 0; i <= InitSize; i++)
			BitMaps->at(0)->push_back(0);
	}
	else
	{
		// add bit map and add as much zeros as in other bit maps
		BitMaps->push_back( NewBitMap );
		for (UINT i = 0; i < BitMaps->at(0)->size(); i++)
			BitMaps->at(Size)->push_back(0);
	}

	return ERRNOERROR;
}

// Deletes all bit maps
HRESULT CShaders::DeleteAllBitMaps(std::vector< std::vector<D3DXHANDLE> * > * BitMaps)
{
	for (UINT i = 0; i < BitMaps->size(); i++)
		delete BitMaps->at(i);

	return ERRNOERROR;
}

// Insert handles into handle map
HRESULT CShaders::InsertHandles(ID3DXConstantTable * ConstTable, std::vector<D3DXHANDLE> * ShaderHandles)
{
	// table desc
	D3DXCONSTANTTABLE_DESC TableDesc;
	D3DXHANDLE Handle;
	D3DXCONSTANT_DESC HandleDesc;
	UINT i;
	UINT Count = 1;
	std::map<string, UINT>::iterator It;
	pair< map<string, UINT>::iterator, bool > Inserted;
	UINT OurHandle;
	UINT Order = 0;
	HRESULT Result;

	ConstTable->GetDesc(&TableDesc);

	for (i = 0; i < TableDesc.Constants; i++)
	{
		// Get handle description
		Handle = ConstTable->GetConstant(NULL, i);
		ConstTable->GetConstantDesc(Handle, &HandleDesc, &Count);

		_HandleCount++;
		Inserted = _Handles.insert( pair<std::string, UINT>( string(HandleDesc.Name), _HandleCount));
		
		// Get our handle - UINT
		OurHandle = Inserted.first->second;
		
		if (Inserted.second)
		{
			// Resize bit maps for mapping
			ResizeAllBitMaps(&_VertexHandles, _HandleCount);
			ResizeAllBitMaps(&_PixelHandles, _HandleCount);
			// Resize extended table
			_ExtendedTables.push_back(0);
			// add constant description
			_ConstantDescriptions.push_back(HandleDesc);
		}
		else
		{
			_HandleCount--;
			// Control
			Result = CheckHandle(OurHandle, HandleDesc);
			if (Result)
				ERRORMSG(-1, "CShaders::InsertHandles()", 
						CString("Constant shader variables with same name are having different structure: ") + CString(HandleDesc.Name) );
		}
		
		Order = 0;
		if (HandleDesc.Elements > 1)
		{
			Result = InsertHandlesElem(OurHandle, Handle, HandleDesc.Elements, ConstTable, ShaderHandles, !(Inserted.second), Order);
			if (Result)
				ERRORMSG(-1, "CShaders::InsertHandles()", 
						CString("Constant shader variables with same name are having different structure: ") + CString(HandleDesc.Name) );
		}
		else if (HandleDesc.StructMembers > 0)
		{
			Result = InsertHandlesStruct(OurHandle, Handle, HandleDesc.StructMembers, ConstTable, ShaderHandles, !(Inserted.second), Order);
			if (Result)
				ERRORMSG(-1, "CShaders::InsertHandles()", 
						CString("Constant shader variables with same name are having different structure: ") + CString(HandleDesc.Name) );
		}
		
		// Put handle into mapping
		ShaderHandles->at(OurHandle) = Handle;
	}

	return ERRNOERROR;
}

HRESULT CShaders::InsertHandlesElem(UINT OurHandle, D3DXHANDLE Handle, UINT NumElems, ID3DXConstantTable * ConstTable, std::vector<D3DXHANDLE> * ShaderHandles, BOOL Inserted, UINT & Order)
{
	D3DXCONSTANT_DESC HandleDesc;
	D3DXHANDLE ElemHandle;
	UINT i;
	UINT Count = 1;
	HRESULT Result;

	for (i = 0; i < NumElems; i++)
	{
		ElemHandle = ConstTable->GetConstantElement(Handle, i);
		ConstTable->GetConstantDesc(ElemHandle, &HandleDesc, &Count);

		if (HandleDesc.Elements > 1)
			InsertHandlesElem(OurHandle, ElemHandle, HandleDesc.Elements, ConstTable, ShaderHandles, Inserted, Order);
		else if (HandleDesc.StructMembers > 0)
			InsertHandlesStruct(OurHandle, ElemHandle, HandleDesc.StructMembers, ConstTable, ShaderHandles, Inserted, Order);
		else if (!Inserted)
		{
			if (_ExtendedTables[OurHandle] == 0)
			{
				_ExtendedTables[OurHandle] = new vector<UINT>();
				if (! _ExtendedTables[OurHandle] )
					ERRORMSG(ERROUTOFMEMORY, "CShaders::InsertHandlesElem()", "Unable to allocate place for extended table.");
			}
			
			_HandleCount++;
			_ExtendedTables[OurHandle]->push_back(_HandleCount);
			
			
			ResizeAllBitMaps(&_VertexHandles, _HandleCount);
			ResizeAllBitMaps(&_PixelHandles, _HandleCount);
			// Resize extended table
			_ExtendedTables.push_back(0);
			// add constant description
			_ConstantDescriptions.push_back(HandleDesc);

			ShaderHandles->at(_HandleCount) = ElemHandle;
		}
		else
		{
			// Control
			Result = CheckHandle(_ExtendedTables[OurHandle]->at(Order), HandleDesc);
			if (Result)
				ERRORMSG(-1, "CShaders::InsertHandlesElem()", CString("Constant shader variables of same name are having different structure." ) );
			
			ShaderHandles->at(_ExtendedTables[OurHandle]->at(Order)) = ElemHandle;
			Order++;
		}

	}

	return ERRNOERROR;
}

HRESULT CShaders::InsertHandlesStruct(UINT OurHandle, D3DXHANDLE Handle, UINT NumStructElems,  ID3DXConstantTable * ConstTable, std::vector<D3DXHANDLE> * ShaderHandles, BOOL Inserted, UINT & Order)
{
	D3DXCONSTANT_DESC HandleDesc;
	D3DXHANDLE ElemHandle;
	UINT i;
	UINT Count = 1;
	HRESULT Result;

	for (i = 0; i < NumStructElems; i++)
	{
		ElemHandle = ConstTable->GetConstant(Handle, i);
		ConstTable->GetConstantDesc(ElemHandle, &HandleDesc, &Count);

		if (HandleDesc.Elements > 1)
			InsertHandlesElem(OurHandle, ElemHandle, HandleDesc.Elements, ConstTable, ShaderHandles, Inserted, Order);
		else if (HandleDesc.StructMembers > 0)
			InsertHandlesStruct(OurHandle, ElemHandle, HandleDesc.StructMembers, ConstTable, ShaderHandles, Inserted, Order);
		else if (!Inserted)
		{
			if (_ExtendedTables[OurHandle] == 0)
			{
				_ExtendedTables[OurHandle] = new vector<UINT>();
				if (! _ExtendedTables[OurHandle] )
					ERRORMSG(ERROUTOFMEMORY, "CShaders::InsertHandlesStruct()", "Unable to allocate place for extended table");
			}
			
			_HandleCount++;
			_ExtendedTables[OurHandle]->push_back(_HandleCount);
			
			
			ResizeAllBitMaps(&_VertexHandles, _HandleCount);
			ResizeAllBitMaps(&_PixelHandles, _HandleCount);
			// Resize extended table
			_ExtendedTables.push_back(0);
			// add constant description
			_ConstantDescriptions.push_back(HandleDesc);

			ShaderHandles->at(_HandleCount) = ElemHandle;
		}
		else
		{
			// Control
			Result = CheckHandle(_ExtendedTables[OurHandle]->at(Order), HandleDesc);
			if (Result)
				ERRORMSG(-1, "CShaders::InsertHandlesStruct()", CString("Constant shader variables of same name are having different structure." ) );
		
			ShaderHandles->at(_ExtendedTables[OurHandle]->at(Order)) = ElemHandle;
			Order++;
		}
	}

	return ERRNOERROR;
}

HRESULT CShaders::CheckHandle(UINT Handle, D3DXCONSTANT_DESC & Desc)
{
	D3DXCONSTANT_DESC CheckDesc;
	BOOL Failed = false;

	CheckDesc = _ConstantDescriptions[Handle];

	if (CheckDesc.Bytes != Desc.Bytes) Failed = true;
	if (CheckDesc.Columns != Desc.Columns) Failed = true;
	if (CheckDesc.Elements != Desc.Elements) Failed = true;
	if (strcmp(CheckDesc.Name, Desc.Name) ) Failed = true;
	if (CheckDesc.Rows != Desc.Rows) Failed = true;
	if (CheckDesc.StructMembers != Desc.StructMembers) Failed = true;
	if (CheckDesc.Type != Desc.Type) Failed = true;

	if (Failed)
		ERRORMSG(-1, "CShaders::CheckHandle()", CString("Constant shader variables of same name are having different structure.") );
		
	return ERRNOERROR;
}

D3DXCONSTANT_DESC CShaders::MatrixDescription(LPCSTR Name)
{
	D3DXCONSTANT_DESC Desc;

	Desc.Bytes = 64;
	Desc.Columns = 4;
	Desc.Rows = 4;
	Desc.Elements = 1;
	Desc.Name = Name;
	Desc.Type = D3DXPT_FLOAT;
	Desc.StructMembers = 0;

	return Desc;
}

D3DXCONSTANT_DESC CShaders::VectorDescription(LPCSTR Name, int Size)
{
	D3DXCONSTANT_DESC Desc;

	Desc.Bytes = 4 * Size;
	Desc.Columns = Size;
	Desc.Rows = 1;
	Desc.Elements = 1;
	Desc.Name = Name;
	Desc.Type = D3DXPT_FLOAT;
	Desc.StructMembers = 0;

	return Desc;
}

// For debugging purposes
#ifdef _DEBUG
/*
	void CShaders::ListHandles(string FileName)
	{
		ofstream File;
		map<string, UINT>::iterator it;

		File.open(FileName.c_str(), ios_base::out);
		
		for (it = _Handles.begin(); it != _Handles.end(); it++)
			File << it->first << endl;

		File.close();
	}
*/
#endif

// constructor
SpotLightShaderStructVector::SpotLightShaderStructVector()
{
	Ambient = D3DXVECTOR4(0, 0, 0, 1);
	Diffuse = D3DXVECTOR4(0, 0, 0, 1);
	Specular = D3DXVECTOR4(0, 0, 0, 1);

	Direction = D3DXVECTOR4(1, 0, 0, 0);
	Position = D3DXVECTOR4(0, 0, 0, 1);
	Atten = D3DXVECTOR4(1, 0, 0, 0);
	Angles = D3DXVECTOR4(1, 0.5, 1, 0);
}

