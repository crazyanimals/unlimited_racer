//
// Header file for the BoundingBox class
//
// (c) 2007 Branislav Repcek
//

#ifndef BOUNDING_BOX_INCLUDED
#define BOUNDING_BOX_INCLUDED

#include "stdafx.h"
#include <d3dx9math.h>

namespace soccer
{
    // Stores bounding box.
    class BoundingBox
    {
    public:
        // Create empty bounding box
        BoundingBox(void) :
        is_set(false)
        {
            box[0] = box[1] = dimensions = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        }

        // Create bounding box with given corners
        BoundingBox(const D3DXVECTOR3 &lower_left, const D3DXVECTOR3 &upper_right) :
        is_set(true)
        {
            box[0] = lower_left;
            box[1] = upper_right;
            dimensions = upper_right - lower_left;
        }

        // Get lower-left corner of the bounding box
        const D3DXVECTOR3 &GetBoundingBoxLL(void) const
        {
            return box[0];
        }

        // Get upper-right corner of the bounding box
        const D3DXVECTOR3 &GetBoundingBoxUR(void) const
        {
            return box[1];
        }

        /* Set bounding box of the home cage.

           lower_left: lower-left corner of the new BB
           upper_right: upper-right corner of the new BB
         */
        void SetBoundingBox(const D3DXVECTOR3 &lower_left, const D3DXVECTOR3 &upper_right)
        {
            box[0] = lower_left;
            box[1] = upper_right;
            dimensions = upper_right - lower_left;
            is_set = true;
        }

        /* Reset bounding box to all zeroes.
         */
        void ResetBoundingBox()
        {
            box[0] = box[1] = dimensions = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            is_set = false;
        }

        /* Adds new point in space to the bounding box of the home cage. If this point is inside the BB, nothing happens.
           If point is outside, BB is expanded so that the point will be inside.

           point: point to add to the home cage.
         */
        void HomeAddPoint(const D3DXVECTOR3 &point);

        // Test is fiven point is inside the BB of the home cage.
        bool IsInside(const D3DXVECTOR3 &point) const;

        /* Test if given sphere is inside the bbox

           center: position of the sphere's center point
           radius: radius of the sphere

           returns: true if sphere is inside the bbox, false otherwise
         */
        bool IsInside(const D3DXVECTOR3 &center, float radius) const;

    private:
        bool is_set; // is bounding box set?
        D3DXVECTOR3 box[2]; // corners (LL and UR)
        D3DXVECTOR3 dimensions; // bbox size
    };
}

#endif
