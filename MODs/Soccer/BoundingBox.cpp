//
// Implementations of some methdos from the BoundingBox class
//
// (c) 2007 Branislav Repcek
//

#include "stdafx.h"
#include "BoundingBox.h"

namespace soccer
{
    bool BoundingBox::IsInside(const D3DXVECTOR3 &point) const
    {
        return (point.x >= box[0].x) && (point.y >= box[0].y) && (point.z >= box[0].z) &&
               (point.x <= box[1].x) && (point.y <= box[1].y) && (point.z <= box[1].z);
    }

    bool BoundingBox::IsInside(const D3DXVECTOR3 &center, float radius) const
    {
        if (!IsInside(center))
        {
            return false;
        }

        D3DXVECTOR3 d_pos = center - box[0];

        return (d_pos.x - radius >= 0.0f) && (d_pos.y - radius >= 0.0f) && (d_pos.z - radius >= 0.0f) &&
               (d_pos.x + radius <= dimensions.x) && (d_pos.y + radius <= dimensions.y) && (d_pos.z + radius <= dimensions.z);
    }

    void BoundingBox::HomeAddPoint(const D3DXVECTOR3 &point)
    {
        if (!is_set)
        {
            box[0] = box[1] = point;
            dimensions = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            is_set = true;
            return;
        }

        if (!IsInside(point))
        {
            D3DXVECTOR3 new_dim = point - box[0];

            dimensions.x = std::max< float >(new_dim.x, dimensions.x);
            dimensions.y = std::max< float >(new_dim.y, dimensions.y);
            dimensions.z = std::max< float >(new_dim.z, dimensions.z);

            box[1] = box[0] + dimensions;
            box[0].x += std::min< float >(new_dim.x, 0);
            box[0].y += std::min< float >(new_dim.y, 0);
            box[0].z += std::min< float >(new_dim.z, 0);
        }
    }
}
