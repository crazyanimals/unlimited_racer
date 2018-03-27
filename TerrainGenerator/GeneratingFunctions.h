
typedef enum __SideType { top = 0, middle, bottom, curved} SideType;
typedef enum __PlaneType {outercorner = 0, innercorner, hillside } PlaneType;

extern SideType Opposite[];


float GetTerrainY(PlaneType pt, SideType st1, SideType st2, int X, int Z, int segsx, int segsz, float LevelHeight, SideType stDiagonal = middle);
int GetTerrainType(PlaneType pt, SideType st1, SideType st2, SideType stDiagonal = middle);