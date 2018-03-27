#pragma once


#include "stdafx.h"
#include "CNetMsg.h"


#define WM_WSAASYNCS (WM_USER + 5)
#define NS_BUFFER_SIZE 32768 //vetsi nez 8 and datagram size
#define MAX_CONNECTIONS 8


namespace Network
{


	class CNetServer
	{
		
		public:
			CNetServer(HWND _hWnd) {hWnd=_hWnd; ZeroMemory(players,MAX_CONNECTIONS*sizeof(SOCKET)); };
			~CNetServer();
			HRESULT Init();
			HRESULT HostSession(u_short dwPort);
			HRESULT StopSession();
			HRESULT SendMsg(CNetMsg * pMsg,int IDPlayer = -1);

			int ReceiveMsg(SOCKET sock,CNetMsg * pMsg);
			int GetPlayerID(SOCKET socket);
			
			int AcceptPlayer(); //accepted ID return -1 when is rejected
			int ClosePlayer(int i); 
			int CloseAllPlayersSocket();

			HRESULT StartBroadcast(u_short port);
			HRESULT StopBroadcast();
			// Binds broadcast socket on specific IP
			HRESULT BindBroadcast(sockaddr_in * Address, u_short port);
			HRESULT SendBroadcastMsg(CNetMsg * pMsg,u_short port);
			int SendMsgToSocket(SOCKET sock,CNetMsg * pMsg);
			BOOL isConnected(int id){ return players[id] ? TRUE:FALSE;};

		private:
			
			SOCKET players[MAX_CONNECTIONS];
			
			SOCKET m_socket;
			SOCKET broadcast_socket;
			
			HWND hWnd;
	};

}; // end of namespace Network