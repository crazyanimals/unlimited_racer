/*
	TimedTextOutput:The main class represents a buffer of messages that should be written on the screen using
					actual graphical output. Font object used for this purpose is passed as parameter from outside
					as like as the timer object.
					Every message added to this front could has its own timeout or use default one.
	Creation Date:	10.4.2005
	Last Update:	10.4.2005
	Author:			Roman Margold

*/


#pragma once

#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"


namespace graphic
{

	class CTimedTextOutput
	{
		
		class CTimedMessage
		{
		public:
			CAtlString				csText;
			APPRUNTIME				uiDeathTime;
			D3DCOLOR				uiColor;
		} ;
		
		typedef std::list<CTimedMessage>	MSG_LIST;
		typedef MSG_LIST::iterator			MSG_ITER;
	
	
	public:
		// PUBLIC PART
								CTimedTextOutput();
								~CTimedTextOutput();

		HRESULT					Init( LPD3DXFONT font, CTimeCounter * timer, APPRUNTIME timeout, D3DCOLOR col, UINT maxMsgs );
		void					SetFont( LPD3DXFONT font );
		void					SetTimer( CTimeCounter * timer );
		inline void				SetTimeout( APPRUNTIME timeout ) { uiDefaultTimeOut = timeout; };
		inline void				SetColor( D3DCOLOR col ) { uiDefaultColor = col; } ;
		inline void				SetMaxMessagesCount( UINT count ) { uiMaxMessages = count; };

		void					AddMessage( LPCTSTR text, D3DCOLOR col = 0, APPRUNTIME timeout = 0 );

		void					SetRenderParams( LPRECT pRect, DWORD format );

		HRESULT					Render();
	
	private:
		// PRIVATE PART
		int						OutputSingleMessage( CTimedMessage * msg, LPRECT pRect );
		void					PurifyMsgList();


		MSG_LIST				Messages;
		LPD3DXFONT				pFont; // font used for text output, this object never changes font's properties
		CTimeCounter		*	pTimer; // timer object used for time counting, this object never changes timer's status or properties
		APPRUNTIME				uiDefaultTimeOut; // specifies the timeout that will be used for new messages if no timeout is specified
												// timeout specified in miliseconds
		D3DCOLOR				uiDefaultColor; // default color used for all messages without it's own color value
		UINT					uiMaxMessages; // maximal count of messages displayed simultaneously
		RECT					RenderingRectangle; // rectangle used for text aligning
		DWORD					dwTextFormat; // format of the text used when beeing rendered		

	} ;

} 
