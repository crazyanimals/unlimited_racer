//////////////////////////////////////////////////////////////////////////////////////////////
//
// CObject
// TODO: complete this ...
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "RigidBody.h"
#include "PhysicsMesh.h"

namespace physics
{
	class CObject : public CRigidBody
	{
	// CObject properties
	public:

	private:

    protected:

	// CObject methods
	public:
		virtual HRESULT Init(NewtonWorld * world, CResources * resources, InitObject * initObject, CMaterials *mat) = 0;
	
		virtual HRESULT Reset() = 0;
		
		virtual void Release() = 0;

	private:
	};
}; // namespace physics

//////////////////////////////////////////////////////////////////////////////////////////////
//
// CStaticObject
// TODO: complete this ...
//
//////////////////////////////////////////////////////////////////////////////////////////////
namespace physics
{
    class CStaticObject : public CObject
    {
    // CStaticObject properties
    public:
    private:
    // CStaticObject methods
    public:
		~CStaticObject();

        HRESULT Init(NewtonWorld * world, CResources * resources, InitObject * initObject, CMaterials *mat);

		HRESULT Reset();

		void Release();

    private:
        //void ApplyForceAndTorque_Callback();
    };
}; // namespace physics

//////////////////////////////////////////////////////////////////////////////////////////////
//
// CDynamicObject
// TODO: complete this ...
//
//////////////////////////////////////////////////////////////////////////////////////////////
namespace physics
{
    class CDynamicObject : public CObject
    {
    // CDynamicObject properties
    public:
    private:
    // CDynamicObject methods
    public:
		~CDynamicObject();

        HRESULT Init(NewtonWorld * world, CResources * resources, InitObject * initObject, CMaterials *mat);

		HRESULT Reset();

		void Release();

    private:
        //void ApplyForceAndTorque_Callback();
    };
}; // namespace physics