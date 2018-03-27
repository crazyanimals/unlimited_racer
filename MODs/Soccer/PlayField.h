//
// Header file for the PlayField class
//
// (c) 2007 Branislav Repcek
//

#ifndef PLAY_FIELD_INCLUDED
#define PLAY_FIELD_INCLUDED

#include "stdafx.h"
#include "../../Globals/Globals.h"

namespace soccer
{
    const float SQUARE_SIZE = TERRAIN_PLATE_WIDTH_M;

    // Stores map of the playgound.
    // NOTE: Y is up, ground plane is therefore XZ
    class PlayField
    {
    public:

        // Type of the square on the play field
        enum SquareType
        {
            SQUARE_PLAYGROUND, // Playground square
            SQUARE_GRASS       // Other square
        };

        /* Create new PlayField

           w: width of the map in squares
           h: height of the map in squares
         */
        explicit PlayField(size_t w = 0, size_t h = 0);

        /* Get type of the square which contains point with given world coordinates

           x: x-coordinate of the point (mapped to the width)
           z: z-coordinate of the point (mapped to the height)

           returns: square type

           note: play field has to be properly initialized before calling this method
                 (its BBox and dimensions have to be set correctly)
         */
        SquareType GetSquareType(float x, float z) const;

        /* Set type of the square which contains point with given world coordinates

           x: x-coordinate of the point (mapped to the width)
           z: z-coordinate of the point (mapped to the height)
           new_type: type to which square will be set

           note: play field has to be properly initialized before calling this method
                 (its BBox and dimensions have to be set correctly)
         */
        void SetSquareType(float x, float z, SquareType new_type);

        /* Get reference to the square at given coordinates.

           x: x coordinate of the square (in squares, not world units)
           y: y coordinate of the square (in squares, not world units)

           returns: reference to the ground type of the square
         */
        SquareType &operator ()(size_t x, size_t y);

        /* Test if given point lies inside the playground (its type is SQUARE_PLAYGROUND)

           x: x-coordinate of the point (mapped to the width)
           z: z-coordinate of the point (mapped to the height)

           returns: true if point is outside of the playground (out occured), false if it is inside

           note: play field has to be properly initialized before calling this method
                 (its BBox and dimensions have to be set correctly)
         */
        bool IsOut(float x, float z) const;

        // Get width of the map in squares
        size_t GetWidth(void) const
        {
            return width;
        }

        // Get height of the map in squares
        size_t GetHeight(void) const
        {
            return height;
        }

        // Get width of the map in world units
        float GetMapWidth(void) const
        {
            return map_width;
        }

        // Get height of the map in world units
        float GetMapHeight(void) const
        {
            return map_height;
        }

        // Set width and height of the map in world units
        void SetDimensions(float world_width, float world_height)
        {
            map_width = world_width;
            map_height = world_height;
        }

    private:
        float       map_width;        // width of the map (world units)
        float       map_height;       // height of the map (world units)
        size_t      width;            // map width in squares
        size_t      height;           // map height in squares
        std::vector< SquareType > playground; // map map :)
        SquareType  temp_data;        // returned in case of overflow

        /* Calculate index of the square which contains given point

           returns: index of the square which contains given point
         */
        size_t Index(float x, float z) const
        {
            size_t xx = static_cast< size_t >(floorf(x / SQUARE_SIZE));
            size_t zz = static_cast< size_t >(floorf(z / SQUARE_SIZE));

            return std::min< size_t >(zz * width + xx, playground.size() - 1);
        }
    };}

#endif
