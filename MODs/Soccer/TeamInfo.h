//
// Header file for the TeamInfo class
//
// (c) 2007 Branislav Repcek
//

#ifndef TEAM_INFO_INCLUDED
#define TEAM_INFO_INCLUDED

#include "stdafx.h"
#include <string>
#include <algorithm>
#include "BoundingBox.h"
#include "BallData.h"

namespace soccer
{

    // Stores details about the geometry of the home cage object
    class BoxObject
    {
    public:
        /* Create new BoxObject

           bbox: bounding box of the home cage
           bonus: number of bonus points that will be awarded to the team that scores to this home cage (may be negative)
         */
        explicit BoxObject(const BoundingBox &bbox = BoundingBox(), int bonus = 1) :
        bounding_box(bbox),
        score_bonus(bonus)
        {
        }

        // Get reference to the bounding box for this home cage
        BoundingBox &GetBoundingBox(void)
        {
            return bounding_box;
        }

        // Get reference to the bounding box for this home cage
        const BoundingBox &GetBoundingBox(void) const
        {
            return bounding_box;
        }

        // Get number of bonus points awared to the team that scores to this cage
        int GetScoreBonus(void) const
        {
            return score_bonus;
        }

        // Set number of bonus points awared to the team that scores to this cage
        void SetScoreBonus(int bonus)
        {
            score_bonus = bonus;
        }

        // Get list of all IDs of static objects that compose this home cage
        std::vector< int > &GetObjectIDs(void)
        {
            return object_ids;
        }

    private:
        BoundingBox bounding_box;       // bounding box
        int score_bonus;                // score bonus points
        std::vector< int > object_ids;  // ids of all static objects in this box object

    };

    class StartPosition
    {
    public:
        StartPosition(int x, int z, size_t ind = 0, float rot = 0.0f) :
        map_x(x),
        map_z(z),
        index(ind),
        rotation(rot)
        {
        }

        int GetMapX(void) const
        {
            return map_x;
        }

        int GetMapZ(void) const
        {
            return map_z;
        }

        size_t GetIndex(void) const
        {
            return index;
        }

        float GetRotation(void) const
        {
            return rotation;
        }

        static void SortByIndex(std::vector< StartPosition > &starts)
        {
            std::sort(starts.begin(), starts.end(), IndexComparer);
        }

    private:
        int    map_x;    // map position (in squares) on X axis
        int    map_z;    // map position (in squares) on Z axis
        size_t index;    // index of the starting position
        float  rotation; // rotation of the player in degrees.

        static bool IndexComparer(const StartPosition &pos1, const StartPosition &pos2)
        {
            return pos1.index < pos2.index;
        }
    };

    /* Stores details about one team.
     */
    class TeamInfo
    {
    public:

        // Create new team.
        explicit TeamInfo(const CString &team_name);

        // Get name of the team.
        const CString &GetName(void) const 
        {
            return name; 
        }

        // Set new name of the team.
        void SetName(const CString &team_name)
        {
            name = team_name;
        }

        // Get number of points scored by this team.
        int GetScore(void) const
        {
            return score;
        }

        // Set number of points scored by this team.
        void SetScore(int new_score)
        {
            score = new_score; 
        }

        // Increase score of this team.
        int AddScore(int award = 0) 
        {
            return (score += award); 
        }

        // Reset score to zero.
        void ResetScore(void)
        {
            score = 0; 
        }

        /* Test if given point is inside one of the objects designed as the home cage object

           point: coordinates of the point to test
           award: reference to variable which will receive number of points team should be awarded if the goal has been scored

           returns: true if point is inside one of the home cage objects, false otherwise.
         */
        bool InsideHome(const D3DXVECTOR3 &point, int &award) const;

        /* Test if given sphere is completely inside one of the home cage objects.

           center: position of the center of the sphere
           radius: radius of the sphere
           award: reference to variable which will receive number of points team should be awarded if the goal has been scored

           returns: true if sphere is inside one of the home cage objects, false otherwise.
        */
        bool InsideHome(const D3DXVECTOR3 &center, float radius, int &award) const;

        /* Test if at least one of the balls from given list is completely inside one of the home cage objects

           ball_data: list with balls to test
           award: reference to variable which will receive number of points team should be awarded if the goal has been scored

           return: true if at least one ball is completely inside of some home cage object of this team
         */
        bool InsideHomeFull(const std::vector< BallData > &ball_data, int &award) const;

        /* Test if the center point of at least one of the balls from given list is inside one of the home cage objects

           ball_data: list with balls to test
           award: reference to variable which will receive number of points team should be awarded if the goal has been scored

           return: true if center point of at least one ball is inside of some home cage object of this team
         */
        bool InsideHomeCenter(const std::vector< BallData > &ball_data, int &award) const;

        /* Add new object to the list of home cage objects.

           score_bonus: score bonus for the home cage object

           returns: reference to the newly created object so you can modify its properties.
         */
        BoxObject &AddHomeCageObject(int score_bonus = 1)
        {
            objects.push_back(BoxObject(BoundingBox(), score_bonus));

            return objects.back();
        }

        // Get list of all home cge objects
        std::vector< BoxObject > &GetHomeCageObjects(void)
        {
            return objects;
        }

        // Reset this team's data.
        void Reset(void)
        {
            ResetScore();
            starts.clear();
            objects.clear();
        }

        // Get list of all start position.
        std::vector< StartPosition > &GetStartPositions(void)
        {
            return starts;
        }

        // Sort starting positions
        void SortStarts(void)
        {
            StartPosition::SortByIndex(starts);
        }

    private:
        std::vector< StartPosition > starts; // starting positions allocated for this team
        std::vector< BoxObject > objects;   // home cage objects
        CString     name;                   // Name of the team.
        int         score;                  // Score of the team (may be negative if they are really lame :)).
    };
}

#endif
