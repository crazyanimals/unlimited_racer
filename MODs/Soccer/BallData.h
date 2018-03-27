//
// Header file for the BallData class
//
// (c) 2007 Branislav Repcek
//

#ifndef BALL_DATA_INCLUDED
#define BALL_DATA_INCLUDED

#include "stdafx.h"
#include <d3dx9math.h>
#include "../../Physics/Physics.h"

namespace soccer
{
    // Stores details about the ball.
    class BallData
    {
    public:
        /* Create new ball.
         
           obj_index: index of the ball object in the list of dynamic objects
           start_pos: starting position of the ball
           r: radius of the ball
         */
        BallData(int obj_index, dVector start_pos, float r) :
        index(obj_index),
        radius(r),
        position(0.0f, 0.0f, 0.0f),
        start_position(D3DXVECTOR3(start_pos.m_x, start_pos.m_y, start_pos.m_z))
        {
        }

        // Get index of the ball in the list of dynamic objects in the game
        int GetIndex(void) const
        {
            return index;
        }

        // Get radius of the ball
        float GetRadius(void) const
        {
            return radius;
        }

        // Set position of the ball in the world
        void SetPosition(const D3DXVECTOR3 &pos)
        {
            position = pos;
        }

        // Get position of the ball
        D3DXVECTOR3 GetPosition(void) const
        {
            return position;
        }

        // Update position of the ball from the given physics manipulator
        void Update(physics::CPhysics *physics)
        {
            physics::CDynamicObject *ball_object = physics->dynamicObjects[index];
            position = D3DXVECTOR3(ball_object->Position().m_x, ball_object->Position().m_y, ball_object->Position().m_z);
        }

        // Get starting position of the ball
        D3DXVECTOR3 GetStartPos(void) const
        {
            return start_position;
        }

    private:
        int         index;          // index of the ball in the list of dynamic objects
        float       radius;         // radius of the ball
        D3DXVECTOR3 position;       // position of the ball in the world
        D3DXVECTOR3 start_position; // starting position of the ball
    };
}

#endif
