/*
	TimeCounter:	class used to count time the app is running; necessary when counting game time
					time counting can be paused and reseted;
					the Pause()-Continue() pair calls can be nested
	Creation Date:	20.10.2003
	Last Update:	13.7.2004
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"

typedef UINT	APPRUNTIME;	// run time specified in miliseconds

class CTimeCounter
{
public:
				CTimeCounter();
				~CTimeCounter();
	
	void		Start(); // sets the miliseconds count to 0 and begins counting
	void		Pause(); /* pauses the miliseconds counting (useful when app losts exclusive mode or pauses);
					if any count of Pause method was already called and no Continue afterwards,
					this only increases PauseCalls count and lets the Pause state unchanged */
	void		Continue(); // forces counter to continue in time counting
	APPRUNTIME	GetRunTime( BOOL recount = true ); // returns time passed from calling Start() counted in miliseconds;
												// set recount to false to prevent method from recounting time
	APPRUNTIME	GetStartTime(); // returns time of Start() call
	BOOL		IsActive(); // says whether the timer is active (== not paused)

private:
	APPRUNTIME	firstPauseTime; // stores the time of last used Pause() call, that's the first unsatisfied with Continue() call
	APPRUNTIME	cPausedTime; // counts a time the timer is paused; used to subtract a time the app is paused from the counted time
	APPRUNTIME	startTime; // stores the time when Start() was called
	APPRUNTIME	cMiliSec; // game time passed from Start() calling counted originaly in miliseconds, but can be chenged in future
	BOOL		Active; // true if a timer is active and is counting
	UINT		cPauseCalls; // count of Pause() calls not ended with Continue() call
	// methods
	void		Recount(); // recounts time passed from Start() call on the base of actual time, or firstPausedTime if the counter is just paused
} ;