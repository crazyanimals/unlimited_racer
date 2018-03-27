//////////////////////////////////////////////////////////////////////////////////////////////
//
// InitStructures
// TODO: - complete this ...
//	
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

namespace physics
{
    struct InitWheel
    {
        CAtlString model;
        dVector position;
        dVector scale;
		bool rotated;
		bool powered;
        // and many more
		float mass;
		float suspensionShock;
		float suspensionSpring;
		float suspensionLength;
		float carMassOnWheel;
		float suspensionCoef;

		// this is CCar
		void * car;
		int num;
    };

    typedef std::vector<InitWheel> InitWheelVector;

    struct InitEngine
    {
        // this is only temporary structure
		int numGears;
		float * gears;
	    float diff_ratio;
	    float efficiency;
		int numCurve;
		float * torques;
		float * rpms;
    };
    
    struct InitCar
    {
        CAtlString model;
        dVector cog;
        dVector scale;
        dVector rotation;
        float mass;
        dVector moi;
        BYTE material;
		BYTE wheel_material;
        InitWheelVector wheels;
        InitEngine engine;
		int carID;
		float shift_up;
		float shift_down;
    };

    struct InitObject
    {
        CAtlString model;
		bool isStatic;
        dVector cog;
        dVector scale;
        dVector rotation;
        float mass;
        dVector moi;
        int material;
		int objectID;
    };

}; // namespace physics