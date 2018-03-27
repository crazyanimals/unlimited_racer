#pragma once

#pragma pack(push, id, 4)

// structure used during saving and loading plate data
struct TG_FILEHEADER
{
	float	fSizeX, fSizeZ;
	int		iSegsX, iSegsZ;
	int		iOptions;
};

#pragma pack(pop, id)