#include "GeneratingFunctions.h"
#include <math.h>

#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#define max(a, b)  (((a) > (b)) ? (a) : (b)) 

SideType stOpposite[] = { bottom, middle, top, curved };

int GetTerrainType(PlaneType pt, SideType st1, SideType st2, SideType stDiagonal) {
    int offset = 1;
    if (stDiagonal == curved && pt != hillside)
        if (st1 == st2 && st1 == stDiagonal && (pt == outercorner || pt == innercorner)) return offset + pt;
        else return 0;
    offset += 2;
    if (pt == hillside) return 3 + (st1 * 4 + st2); offset += 16;
    offset += pt * 3 * 16;
    offset += stDiagonal * 16;
    return offset + st1 * 4 + st2;
}

//portion=0 - returns v1, portion=1 returns v2. When 0<portion<1, value is interpolated
float Interpolate(float v1, float v2, float portion) {
    return (portion < 0 || portion > 1) ? -1 : v2*portion + v1*(1-portion);   
}

float SideFunction(SideType st, float x) {
    if (x < 0 || x > 1) return -1;
    switch (st) {
        case middle: return x;                break; // x
        case    top: return ((1-x)*x+1)*x;    break; // -x^3 + x^2 + x
        case bottom: return (2-x)*x*x;        break; // -x^3 + 2*x^2
        case curved: return (3-2*x)*x*x;      break; // -2*x^3 + 3*x^2
        default      : return 0;   
    }
}

float GetOuterCornerY(SideType stX, SideType stZ, SideType stDiagonal, int X, int Z, int segsx, int segsz, float LevelHeight) {
	float xf = 1 - (float) X / segsx;
	float zf = 1 - (float) Z / segsz;
    float xz = (float) segsx / segsz;
	float d = 1 - sqrtf(xf*xf + zf*zf)/sqrtf(1 + (zf*xf ? min(zf,xf)*min(zf,xf)/(max(zf,xf)*max(zf,xf)) : 0));
    float coef;

    if (X == Z    ) return LevelHeight * SideFunction(stDiagonal, d);
    if (X == segsx) return LevelHeight * SideFunction(stX       , d);
    if (Z == segsz) return LevelHeight * SideFunction(stZ       , d);

    coef = X < Z ? (Z - X/xz) / (segsz - X/xz) : (X - Z*xz) / (segsx - Z*xz);

    return LevelHeight * Interpolate(SideFunction(stDiagonal, d), X < Z ? SideFunction(stZ, d) : SideFunction(stX, d), coef);
}

float GetInnerCornerY(SideType stX, SideType stZ, SideType stDiagonal, int X, int Z, int segsx, int segsz, float LevelHeight) {
    return LevelHeight - GetOuterCornerY(stOpposite[stX], stOpposite[stZ], stOpposite[stDiagonal], segsx - X, segsz - Z, segsx, segsz, LevelHeight);
}

float GetHillSideY(SideType st1, SideType st2, int X, int Z, int segsx, int segsz, float LevelHeight) {
    float xf = (float) X / segsx;
    float zf = (float) Z / segsz;
    return LevelHeight * Interpolate(SideFunction(st1, zf), SideFunction(st2, zf), xf);
}

float GetTerrainY(PlaneType pt, SideType st1, SideType st2, int X, int Z, int segsx, int segsz, float LevelHeight, SideType stDiagonal) {
    switch (pt) {
        case hillside   : return GetHillSideY   (st1, st2,             X, Z, segsx, segsz, LevelHeight); break;
        case innercorner: return GetInnerCornerY(st1, st2, stDiagonal, X, Z, segsx, segsz, LevelHeight); break;
        case outercorner: return GetOuterCornerY(st1, st2, stDiagonal, X, Z, segsx, segsz, LevelHeight); break;
		default: return 0;
    }
}