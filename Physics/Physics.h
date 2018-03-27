//////////////////////////////////////////////////////////////////////////////////////////////
//
// class CPhysics
// TODO: - complete this ...
//	
// Main class of physical engine.
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "..\GraphicObjects\BoundingEnvelopes.h"

#include "Car.h"
#include "Object.h"
#include "Terrain.h"
#include "Materials.h"

namespace physics
{
	// vector type for static objects
	typedef std::vector<CStaticObject *> CStaticObjectVector;
	// vector type for dynamic objects
	typedef std::vector<CDynamicObject *> CDynamicObjectVector;
	
	class CPhysics
	{
	// CPhysics properties
	public:
		// cars
		CCar * cars[MAX_PLAYERS];
		// dynamic objects
		CDynamicObjectVector dynamicObjects;
		// static objects
		CStaticObjectVector staticObjects;
		// terrain representation
		CTerrain terrain;
		// materials used in simulations
		CMaterials materials;

	private:
		// reference to resources (resource manager)
		CResources resources;
		// newton library world object
		NewtonWorld * nWorld;
		// user information inserted into NewtonWorld object
		UserData userData;
		// here is stored value that found by NewtonWorldRayCast function
		float getHeightParam;
		// time accumulator, used for time slicing algorithm
		float accumulator;

		// CPhysics methods
	public:
		//// class constructor
		//CPhysics()

		//// class destructor
		//~CPhysics();

		// call after class object created - class initialization
		//HRESULT Create();
		// call before game start
		HRESULT Init(resManNS::CResourceManager * rsm);

		HRESULT Debug_Init(graphic::CBoundingEnvelopes * _bEnvelope, HRESULT (CALLBACK * start)(graphic::CBoundingEnvelopes * bEnvelope), HRESULT (CALLBACK * finish)(graphic::CBoundingEnvelopes * bEnvelope), HRESULT (CALLBACK * processface)(graphic::CBoundingEnvelopes * bEnvelope, UINT uiVertCount, float * pBuf));

		HRESULT Reset();

		HRESULT Clean();

		HRESULT Destroy();

		HRESULT Update(int dt);

		NewtonWorld * World();

		CResources * Resources();

		//HRESULT LoadMap(CAtlString terrainName);//, std::vector<CMainObject > * objects);

		void Debug_ShowCollisions();

		static void Debug_ShowBodyCollision(const NewtonBody * body);

		static void Debug_CollisionPolygon(const NewtonBody * body, int vertexCount, const float * FaceArray, int faceId);

		float GetHeight(float x, float z);

		dVector GetWorldSize() { return terrain.WorldSize(); };

		void ReleaseStartUpData();

	private:
		static graphic::CBoundingEnvelopes * bEnvelope;

		static HRESULT (CALLBACK * Start)(graphic::CBoundingEnvelopes * bEnvelope);

		static HRESULT (CALLBACK * Finish)(graphic::CBoundingEnvelopes * bEnvelope);

		static HRESULT (CALLBACK * ProcessFace)(graphic::CBoundingEnvelopes * bEnvelope, UINT uiVertCount, float * pBuf);

		static float RayCast_CallbackStatic(const NewtonBody * nBody, const float * normal, int collisionID, void * userData, float intersetParam);

		float RayCast_Callback(/*const float * normal, int collisionID, void * userData,*/ float intersetParam);
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// get newton world object
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	inline NewtonWorld * CPhysics::World()
	{
		return nWorld;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// get resource manager object
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	inline CResources * CPhysics::Resources()
	{
		return &resources;
	};
}; // namespace physics

// This code is no longer used
#if 0
	//typedef std::vector<CCar *> CCarVector;

	//struct SpecialEffectStruct
	//{
	//	void * soundHandle;
	//	void * particeHandle;

	//	void Init(char * effectName)
	//	{
	//	};

	//	void Destroy()
	//	{
	//	};

	//	void PlayImpactSound(float volume)
	//	{
	//	};

	//	void PlayScratchSound(float volume)
	//	{
	//	};

	//	NewtonBody * m_body0;
	//	NewtonBody * m_body1;
	//	dVector m_position;
	//	float m_contactMaxNormalSpeed;
	//	float m_contactMaxTangentSpeed;
	//};

	// collection of material IDs
	//int * materialID;
	//int materialSize;

	//int defaultID;
	//int woodID;
	//int metalID;
	//int levelID;
	//int vehicleID;
	//int characterID;

	//SpecialEffectStruct wood_wood;
	//SpecialEffectStruct wood_metal;
	//SpecialEffectStruct wood_level;
	//SpecialEffectStruct metal_metal;
	//SpecialEffectStruct metal_level;
	//SpecialEffectStruct vehicle_level;

	//static SpecialEffectStruct * g_currentEffect;

	//NewtonBody * levelBody;
	// map
	// CPhMap map;

	// initializes materials
	//HRESULT InitMaterials();
	//HRESULT CleanUpMaterials();

	//static int GenericContactBegin(const NewtonMaterial * material, const NewtonBody * body0, const NewtonBody * body1);
	//static int GenericContactProcess (const NewtonMaterial* material, const NewtonContact* contact);
	//static void GenericContactEnd (const NewtonMaterial* material);
	//static int CharacterContactProcess (const NewtonMaterial* material, const NewtonContact* contact);
	//static int VehicleContactProcess (const NewtonMaterial* material, const NewtonContact* contact);

#endif