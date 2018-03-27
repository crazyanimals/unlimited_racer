#pragma once


#include "stdafx.h"
#include "CNetMsg.h"


#define NC_BUFFER_SIZE 32768 // have to be bigger then 8 and datagram size
#define WM_WSAASYNCC (WM_USER + 6)


namespace Network
{

	class CNetClient{
		
		public:
			
			CNetClient(HWND _hWnd){ hWnd=_hWnd; };
			~CNetClient();
			HRESULT Init();
			HRESULT ConnectToServer(CString IP,u_short usPort);
			int Disconnect();
			HRESULT SendMsg(CNetMsg * pMsg);
			int ReceiveMsg(SOCKET sock,CNetMsg * pMsg);
			HRESULT StartBroadcast(u_short port);
			HRESULT StopBroadcast();
			HRESULT SendBroadcastMsg(CNetMsg * pMsg,u_short port);
			int SendMsgToSocket(CNetMsg * pMsg,SOCKET sock);
			bool isConnected(){ return m_socket ? true:false;};
		private:
			SOCKET broadcast_socket;
			HWND hWnd;
			SOCKET m_socket;
			
	};

}; // end of namespace Network