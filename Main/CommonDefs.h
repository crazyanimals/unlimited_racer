/*
	CommonDefs:		This header will contain definition used throughout the game. No includes should be added here
					so it keeps the includes order defined by compile order.
	Creation Date:	7.1.2007
	Last Update:	7.1.2007
	Author:			Roman Margold
*/


#pragma once


// contains resources related to commonly used sounds
struct COMMON_SOUNDS
{
	resManNS::RESOURCEID	ridPrimarySoundBank;
	resManNS::RESOURCEID	ridPrimaryWaveBank;
	IXACTSoundBank		*	pPrimarySoundBank;
	IXACTWaveBank		*	pPrimaryWaveBank;
};


