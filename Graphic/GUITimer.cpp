#include "GUITimer.h"


using namespace graphic;



//////////////////////////////////////////////////////////////////////////////////////////////
//
// standard constructor and destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUITimer::CGUITimer( CTimeCounter * pTimer )
{
	this->pTimer = pTimer;
	Events.clear();
}


CGUITimer::~CGUITimer()
{
	Events.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a new event to the queue
// pObject parameter could point to some GUI control object but also could be null
// actually, it could be used in any way, this parameter is just passed to the event call
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUITimer::AddEvent( ONTIMEREVENTCALLBACKPTR lpEvent, UINT timeOut, LPVOID pObject )
{
	GUITE_ITER		iter;
	GUITIMEREVENT	event;

	event.lpEventHandler = lpEvent;
	event.pObject = pObject;
	event.uiTimeOut = pTimer->GetRunTime() + (APPRUNTIME) timeOut;
	
	if ( !event.lpEventHandler ) ERRORMSG( ERRINVALIDPARAMETER, "CGUITimer::AddEvent()", "Event handler can't be NULL." )

	if ( Events.begin() == Events.end() ) Events.push_back( event );
	else
	{
		iter = Events.begin();
		while ( iter != Events.end() && event.uiTimeOut >= iter->uiTimeOut ) iter++;

		Events.insert( iter, event );
	}

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// went thru the event queue and find all events that should be processed; then process them
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUITimer::ProcessEvents()
{
	GUITE_ITER		iter;
	APPRUNTIME		time;

	time = pTimer->GetRunTime();
	// events are sorted by time so we don't have to go thru whole queue
	for ( iter = Events.begin(); iter != Events.end() && iter->uiTimeOut < time; iter++ )
	{
		if ( iter->lpEventHandler ) iter->lpEventHandler( time, iter->pObject );
		Events.pop_front();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
