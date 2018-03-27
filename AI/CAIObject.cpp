#include "CAIObject.h"

using namespace AI_NS;

CAIObject::CAIObject() {
    fRadius = fMass = 1.0f;
    vPosition = vVelocity = V3::Zero();
}

CAIObject::CAIObject(const CAIObject &o) {
    fRadius   = o.fRadius;
    fMass     = o.fMass;
    vPosition = o.vPosition;
    vVelocity = o.vVelocity;
}

V3      CAIObject::PredictFuturePosition(float fTime) const {
    return vPosition + (vVelocity * fTime);
}

float   CAIObject::Speed() const {
     return vVelocity.Length();
}

float   CAIObject::PredictNearestApproachTime(const CAIObject * object) const {
    const V3 vRelativeVelocity = object->vVelocity - this->vVelocity;
    const float fRelativeSpeed = vRelativeVelocity.Length();

    // for parallel paths, the vehicles will always be at the same distance,
    // so return 0 (aka "now") since "there is no time like the present"
    if (!fRelativeSpeed) return 0;

    // Now consider the path of the other vehicle in this relative
    // space, a line defined by the relative position and velocity.
    // The distance from the origin (our vehicle) to that line is
    // the nearest approach.

    // find distance from its path to origin (compute offset from
    // other to us, find length of projection onto path)
    return (vRelativeVelocity.Normalize()).DotProduct(vPosition - object->vPosition) / fRelativeSpeed;
}

float   CAIObject::ComputeNearestApproachPositions(const CAIObject *object, float fTime) const {
    return (vPosition + vVelocity * fTime).Distance(object->vPosition + object->vVelocity * fTime);
}

float   CAIObject::Mass() const {
    return fMass;
}

float   CAIObject::Radius() const {
    return fRadius;
}

V3      CAIObject::Forward() const {
    return vVelocity.Normalize();
}

V3      CAIObject::Side() const {
    return V3(Forward().z, Forward().y, -Forward().x);
}

V3      CAIObject::Velocity() const {
    return vVelocity;
}

V3      CAIObject::Position() const {
    return vPosition;
}

void    CAIObject::SetMass(float mass) {
    fMass = mass;
}

void    CAIObject::SetRadius(float radius) {
    fRadius = radius;
}

void    CAIObject::SetPosition(const V3 & position) {
    vPosition = position;
}

void    CAIObject::SetVelocity(const V3 & velocity) {
    vVelocity = velocity;
}