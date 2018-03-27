#pragma once

#include <math.h>

namespace AI_NS {
    class V3 {
    public:
        float x, y, z;

        // constructors
        V3() { x = y = z = 0;   };
        V3(const AI_NS::V3 &_Vec) { x = _Vec.x; y = _Vec.y; z = _Vec.z; };
        V3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; } ;

        // basic operations
        V3  operator +  (const V3 & _Vec) const { return V3( x + _Vec.x, y + _Vec.y, z + _Vec.z); };
        V3  operator -  (const V3 & _Vec) const { return V3( x - _Vec.x, y - _Vec.y, z - _Vec.z); };
        V3  operator -  (               ) const { return V3( - x       , - y       , -z        ); };
        V3  operator *  (const float mul) const { return V3( x * mul   , y * mul   , z * mul   ); };
        V3  operator /  (const float div) const { return V3( x / div   , y / div   , z / div   ); };
        
        // assignment  operations
        V3  operator =  (const V3 & _Vec)   { x = _Vec.x; y = _Vec.y; z = _Vec.z; return *this;};
        V3  operator += (const V3 & _Vec)   { return *this = (*this + _Vec); };
        V3  operator -= (const V3 & _Vec)   { return *this = (*this - _Vec); }; 
        V3  operator *= (const float mul)   { return *this = (*this * mul ); };
        V3  operator /= (const float div)   { return *this = (*this / div ); };
    
        // comparison

        bool    operator == (const V3 & _Vec) const { return x == _Vec.x && y == _Vec.y && z == _Vec.z; };
        bool    operator != (const V3 & _Vec) const { return !(*this == _Vec); };
        
        
        // advanced operations that do not alter vector
        float   DotProduct(const V3 & _Vec) const; 
        float   Distance  (const V3 & _Vec) const; 
        float   Length    ()                const; 
        V3      Normalize ()                const;
        V3      SetYtoZero()                const  { return V3(this->x, 0, this->z); };
        V3      ParallelComponent       (const V3 & _ReferenceV3) const;
        V3      PerpendicularComponent  (const V3 & _ReferenceV3) const; 
		V3		Interpolate(const V3 & _Vec, float Factor) const;
        
        // advanced operations that alter vector
        V3      SetCrossProduct         (const V3 & v1, const V3 & v2); 
        V3      RotateY(float fAngle);
    
        static V3   Zero();


    };
};