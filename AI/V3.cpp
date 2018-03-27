#include "V3.h"

using namespace AI_NS;

float   V3::DotProduct(const V3 &_Vec) const {
     return x * _Vec.x + 
            y * _Vec.y + 
            z * _Vec.z ;
}

float   V3::Distance(const V3 & _Vec) const {
    return (*this - _Vec).Length();
}

float   V3::Length() const {
    return sqrtf(DotProduct(*this));
}


V3  V3::Normalize() const {
    float L = Length();
    return L ? *this / L : *this;
}

V3  V3::SetCrossProduct(const V3 &v1, const V3 &v2) { 
    x = v1.y * v2.z - v1.z * v2.y;
    y = v1.z * v2.x - v1.x * v2.z;
    z = v1.x * v2.y - v1.y * v2.x;
    return *this; 
}

V3  V3::ParallelComponent(const V3 &_ReferenceV3) const {
    V3 vUnitBasis = _ReferenceV3;
    return vUnitBasis * DotProduct(vUnitBasis.Normalize());
}

V3  V3::PerpendicularComponent(const V3 &_ReferenceV3) const {
    return *this - ParallelComponent(_ReferenceV3);
}

V3  V3::RotateY(float fAngle) {    
    float s = sinf(fAngle);
    float c = cosf(fAngle);
    return *this = V3(x * c + z * s, y, z * c - x * s);
}

V3  V3::Zero() {
    return V3(0.0f, 0.0f, 0.0f);
}

// Factor == 0 - this vector
// Factor == 1 - parametr vector
V3	V3::Interpolate(const V3 & _Vec, float Factor) const
{
	float InvFactor = 1 - Factor;
	return V3( InvFactor * x + Factor * _Vec.x, 
			   InvFactor * y + Factor * _Vec.y,
			   InvFactor * z + Factor * _Vec.z);
}