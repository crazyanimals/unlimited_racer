/*
	GUITimer:		GUI timer is covering standard TimeCounter advanced by event queue with timeouts.
					This queue is went thru during ProcessEvents() method call and all events that should
					be invoked on the basis of actual app run time are processed in proper order -
					that means the queue is ordered by time an event should be processed and 
					if more events have the same time, FIFO rule is applied.
					
					Processing of a single event means calling event handler method assigned to this event
					and passing to it actual time, 
					
					Any event consists of a pointer to an object this event is assigned to, handler to
					a method that is about to be called and (app run) time it should be processed.
	Creation Date:	19.12.2004
	Last Update:	19.12.2004
	Author:			Roman Margold
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"




namespace graphic
{

	typedef HRESULT CALLBACK		__ONTIMEREVENTCALLBACK( APPRUNTIME time, LPVOID pReserved );
	typedef __ONTIMEREVENTCALLBACK	* ONTIMEREVENTCALLBACKPTR;  // pointer to event callback function


	// event structure containing single timer event
	// described in this file header
	struct GUITIMEREVENT
	{
		ONTIMEREVENTCALLBACKPTR		lpEventHandler;
		APPRUNTIME					uiTimeOut; // app run time in which this event should be processed
		LPVOID						pObject; // could containt a pointer to a GUI control object
	} ;


	typedef std::list<GUITIMEREVENT>	GUITE_LIST;
	typedef GUITE_LIST::iterator		GUITE_ITER;



	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// GUI timer class
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CGUITimer
	{
	public:
		// constructor & destructor
								CGUITimer( CTimeCounter * pTimer );
								~CGUITimer();
		
		// methods
		HRESULT					AddEvent( ONTIMEREVENTCALLBACKPTR lpEvent, UINT timeOut, LPVOID pObject ); // adds a new event to the queue
		void					ProcessEvents(); // went thru the event queue and find all events that should be processed; then process them
		
		APPRUNTIME				GetTime() const { return pTimer->GetRunTime(); };
		
	protected:
		// properties
		GUITE_LIST				Events;
		CTimeCounter		*	pTimer;
	} ;


} // end of namespace 
