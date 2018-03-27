#include "stdafx.h"
#include "TimedTextOutput.h"


using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CTimedTextOutput::CTimedTextOutput()
{
	pFont = NULL;
	pTimer = NULL;
	Messages.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CTimedTextOutput::~CTimedTextOutput()
{
	Messages.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// initializing method
//
// init object pointers and set all default properties
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CTimedTextOutput::Init(	LPD3DXFONT font,
								CTimeCounter * timer,
								APPRUNTIME timeout,
								D3DCOLOR col,
								UINT maxMsgs )
{
	SetFont( font );
	SetTimer( timer );
	
	uiDefaultTimeOut = timeout;
	uiDefaultColor = col;
	uiMaxMessages = maxMsgs;

	RenderingRectangle.left = RenderingRectangle.right = RenderingRectangle.top = RenderingRectangle.bottom = 0;
	dwTextFormat = DT_LEFT | DT_TOP | DT_SINGLELINE;

	Messages.clear();

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the pointer to a font object that will be used for text output
// font object is never changed, used only as if it was "readonly"
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTimedTextOutput::SetFont( LPD3DXFONT font )
{
	pFont = font;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the pointer to a timer object that will be used for time counting
// timer object is never changed, used only as if it was "readonly"
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTimedTextOutput::SetTimer( CTimeCounter * timer )
{
	pTimer = timer;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a new timed message to the list
// if the timeout is set to zero (or unused), the default timeout is used instead
// (timeout is specified in miliseconds)
// if the msg color is set to zero (or unused), the default color is used instead
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTimedTextOutput::AddMessage( LPCTSTR text, D3DCOLOR col, APPRUNTIME timeout )
{
	CTimedMessage	msg;
	
	msg.csText = text;
	msg.uiDeathTime = pTimer->GetRunTime() + (timeout ? timeout : uiDefaultTimeOut);
	msg.uiColor = col ? col : uiDefaultColor;

	Messages.push_back( msg );
}


//////////////////////////////////////////////////////////////////////////////////////////////
// 
// this sets clipping rectangle, places the text to the screen and sets the text format
// for text formating these flags are used:
//	  DT_LEFT, DT_CENTER, DT_RIGHT
//    DT_TOP, DT_VCENTER, DT_BOTTOM
//    DT_NOCLIP, DT_WORDBREAK, DT_SINGLELINE
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTimedTextOutput::SetRenderParams( PRECT pRect, DWORD format )
{
	RenderingRectangle = *pRect;
	dwTextFormat = format;
}
	

//////////////////////////////////////////////////////////////////////////////////////////////
// 
// prints out the message to the specified rectangle and returns the height in pixels that
// the message occupies on the screen
//
//////////////////////////////////////////////////////////////////////////////////////////////
int CTimedTextOutput::OutputSingleMessage( CTimedMessage * msg, LPRECT pRect )
{
	char	ret[1000];
	RECT	rect;
	DWORD	format;

	if ( msg->csText.GetLength() < 995 ) sprintf( ret, "%s", msg->csText.GetBuffer() );
	else sprintf( ret, "%s", msg->csText.GetBufferSetLength( 995 ) );

	format = dwTextFormat | DT_CALCRECT;
	pFont->DrawText( NULL, ret, min( msg->csText.GetLength(), 995 ), &rect, format, msg->uiColor );
	pFont->DrawText( NULL, ret, min( msg->csText.GetLength(), 995 ), pRect, dwTextFormat, msg->uiColor );

	return rect.bottom - rect.top;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// 
// renders listed messages to the back buffer
// only first uiMaxMessages are rendered
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CTimedTextOutput::Render()
{
	RECT		rect;
	MSG_ITER	iter;
	UINT		ind, len;
	
	PurifyMsgList(); // clear all dead messages first
	
	rect = RenderingRectangle;

	len = (UINT) Messages.size();
	if ( len <= uiMaxMessages )
	{
		for ( iter = Messages.begin(); iter != Messages.end(); iter++ )
			rect.top += OutputSingleMessage( &(*iter), &rect );
	}
	else
	{
		for ( ind = 0, iter = Messages.begin(); ind < len - uiMaxMessages; ind++, iter++ ) ; // run thru the list to it's tail that should be written
		for ( ; iter != Messages.end(); iter++ )
		{
			rect.top += OutputSingleMessage( &(*iter), &rect );
		}
	}

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// 
// deletes from the list all messages that already exceeded their timeout
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CTimedTextOutput::PurifyMsgList()
{
	pTimer->GetRunTime();

	for ( MSG_ITER iter = Messages.begin(); iter != Messages.end(); )
		if ( iter->uiDeathTime <= pTimer->GetRunTime( false ) ) // this message should be already dead :)
			iter = Messages.erase( iter );
		else iter++;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////


