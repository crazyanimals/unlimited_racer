
#pragma once
#include "V3.h"
#include "AI_Common.h"

namespace AI_NS {

    class CAIObject;

    typedef std::vector<CAIObject *> ObjectVec;    


    class CAIObject {
    private:
        float   fMass, fRadius;
        V3      vPosition, vVelocity;
      
    public:
                CAIObject();
                CAIObject(const CAIObject & o);

        virtual float   Speed   () const;
        virtual float   Mass    () const;
        virtual float   Radius  () const;
        virtual V3      Forward () const;
        virtual V3      Side    () const;
        virtual V3      Velocity() const;
        virtual V3      Position() const;
        virtual void    SetMass    (float mass);
        virtual void    SetRadius  (float radius);
        virtual void    SetPosition(const V3 & position);
        virtual void    SetVelocity(const V3 & velocity);
        virtual V3      PredictFuturePosition          (float fTime)              const;
        virtual float   PredictNearestApproachTime     (const CAIObject * object) const;
        virtual float   ComputeNearestApproachPositions(const CAIObject * object, float fTime) const;

    };

};