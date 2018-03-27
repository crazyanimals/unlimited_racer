//
// Implementation of some methods from the PlayField class
//
// (c) 2007 Branislav Repcek
//

#include "stdafx.h"
#include "PlayField.h"

namespace soccer
{
    PlayField::PlayField(size_t w, size_t h) :
    width(w),
    height(h)
    {
        // Everything is out
        playground.resize(std::max< size_t >(1, width * height), SQUARE_GRASS);
    }

    PlayField::SquareType PlayField::GetSquareType(float x, float z) const
    {
        return playground[Index(x, z)];
    }

    void PlayField::SetSquareType(float x, float z, PlayField::SquareType new_type)
    {
        playground[Index(x, z)] = new_type;
    }

    bool PlayField::IsOut(float x, float z) const
    {
        return playground[Index(x, z)] == PlayField::SQUARE_GRASS;
    }

    PlayField::SquareType &PlayField::operator ()(size_t x, size_t y)
    {
        size_t index = y * width + x;

        if (index < playground.size())
        {
            return playground[index];
        }
        else
        {
            return temp_data; // ok, not that nice...
        }
    }
}
