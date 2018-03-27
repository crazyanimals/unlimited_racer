//////////////////////////////////////////////////////////////////////////////////////////////
//
// InitStructures
// TODO: - complete this ...
//	
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#define PHYSICS_TERRAIN				1234
#define PHYSICS_CAR					1
#define PHYSICS_WHEEL				2
#define PHYSICS_STATIC_OBJECT		3
#define PHYSICS_DYNAMIC_OBJECT		4

#define PHYSICS_WORLD_BOUNDING_BOX	5

namespace physics
{
    struct UserData
    {
        int dataType;
		int material0;
		int material1;
        void * data;
    };
}; // namespace physics