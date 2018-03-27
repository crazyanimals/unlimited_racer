/*
	FPSCounter:		this class is using TimeCounter to count framerate (FPS == Frames Per Second);
					every rendered scene must increase counter, any other operations are done automatically
	Creation Date:	20.10.2003
	Last Update:	10.4.2005
	Author:			Roman Margold
*/

#pragma once

#include "stdafx.h"
#include "../Globals/TimeCounter.h"


class CFPSCounter
{
public:
	void				Start( APPRUNTIME recountInterval = 1000 );  // resets counter and starts counting frames, recountInterval is specifying how 
													// often is FPS value being recounted (in miliseconds)
	void				Pause(); // pauses counter
	void				Continue(); // activates paused counter
	float				GetFPS(); // returns actual FPS value
	float				GetFPSFromStart(); // returns an average FPS counted on whole counter runtime
	__int64				GetFramesFromStart(); // returns count of frames rendered from the Start() call
	BOOL				IsActive(); // says whether the counter is active (== not paused)
	UINT				operator++(); // overloaded preincrement and postincrement operators - increments count of rendered frames
	UINT				operator++( int ); // and returns count of frames rendered from the last Recount() call
	void				SetRecountInterval( APPRUNTIME recountInterval ); // sets the recountInterval
	inline APPRUNTIME	GetRunTime() { return TimeCounter.GetRunTime(); } // returns run time of this counter
	inline CTimeCounter * GetTimer() { return &TimeCounter; }; // returns a pointer to TimeCounter object

private:
	CTimeCounter		TimeCounter;
	APPRUNTIME			lastRecountTime; // time of last successful Recount() call
	__int64				cFrames; // count of frames already rendered from the Start() call
	UINT				cUncountedFrames; // count of frames rendered from last Recount() call
	APPRUNTIME			recountInterval; // value specifying minimal lastRecountTime needed to recount FPS when calling GetFPS
	float				FPS; // counted FPS value
	
	// methods
	void				Recount(); // recounts actual FPS, but only ones per recountInterval at max
	void				Increment(); // increments the cFrames and cUncountedFrames values

} ;