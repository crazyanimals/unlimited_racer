#include "stdafx.h"
#include "CNetClient.h"


using namespace Network;


////////////////////////////////////////////////////////////////////
//Network initialization
//return 0 
HRESULT CNetClient::Init(){
	HRESULT hr=ERRNOERROR;
	

    WSADATA wsaData;
    int er = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( er != 0 ){
    	CString err;err.Format("%d",er);
		ERRORMSG( ERRSOCKET,"WSAStartup()",err);
	}

   
	return hr;
  
};
CNetClient::~CNetClient(){
	WSACleanup();

};

////////////////////////////////////////////////////////////////////
// Send message in CNetMsg format to connected server.
// Return 0 - succ, Return ERRSOCKET - error
HRESULT CNetClient::SendMsg(CNetMsg * pMsg){
	
	int itime=10;
	int neterr;
	neterr =send(m_socket,pMsg->getBuffer(),pMsg->getBufferSize(),0);
	while ( neterr ==  SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK && itime < 1300){
		OUTS( "CNetClient::SendMsg() Warning:socket is blocked: ", 2 );
		OUTI( itime, 2 );OUTS("\n",2);
		Sleep(itime);itime*=5;
		neterr =send(m_socket,pMsg->getBuffer(),pMsg->getBufferSize(),0);	
	}
	if( neterr ==  SOCKET_ERROR ){
		CString err;err.Format("%d",WSAGetLastError());
		Disconnect();
		ERRORMSG( ERRSOCKET,"CNetClient::SendMsg()",err);
		
	};
	return ERRNOERROR;
};
////////////////////////////////////////////////////////////////////
//Send message in CNetMsg format to socket
//Return 0 - succ, Return ERRSOCKET - error
int CNetClient::SendMsgToSocket(CNetMsg * pMsg,SOCKET sock){
	int itime=2;
	int neterr;
	neterr = send(sock,pMsg->getBuffer(),pMsg->getBufferSize(),0);
	while (neterr ==  SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK && itime < 1300){
		OUTS( "CNetClient::SendMsgToSocket() Warning:socket is blocked: ", 2 );
		OUTI( itime, 2 );OUTS("\n",2);
		Sleep(itime);itime*=5;
		neterr = send(sock,pMsg->getBuffer(),pMsg->getBufferSize(),0);		
	}
	if(neterr == SOCKET_ERROR){
			CString err;err.Format("%d",WSAGetLastError());
			Disconnect();
			ERRORMSG( ERRSOCKET,"CNetClient::SendMsgToSocket()",err);
	}

	
	return ERRNOERROR;

}
////////////////////////////////////////////////////////////////////
//Receive message from socket and fill CNetMsg
//Return size of message in bytes, Return ERRSOCKET - error
int CNetClient::ReceiveMsg(SOCKET sock,CNetMsg * pMsg){
	
	
	BOOL broad;
	int boolsize = sizeof(BOOL);
	int size;
	char * buff = new char[NC_BUFFER_SIZE];

	if (  getsockopt(sock, SOL_SOCKET ,SO_BROADCAST,(char *) &broad,&boolsize ) == SOCKET_ERROR){
		broad=FALSE;
	}
		
	if(broad){// is it broadcast message? you have to receive whole message.
		
		size = recv(sock,buff,NC_BUFFER_SIZE,0);
		if( size <= 0) {
			SAFE_DELETE_ARRAY(buff);
			return 0;
		}
		pMsg->addNetMsg((BYTE *) buff,size);
		SAFE_DELETE_ARRAY(buff);
		return size;
	}
	else {

		int * _buff;
		int readed_data;
		int datasize;
		int sizeToRead;
		int itime;
		
		sizeToRead = sizeof(int) + sizeof(int);

		size = recv(sock,buff,sizeToRead,0);
		
		if( size <= 0) {
			SAFE_DELETE_ARRAY(buff);
			return 0;
		}

		if(size != sizeToRead){
			SAFE_DELETE_ARRAY(buff);	
			ERRORMSG(-1,"CNetClient::ReceiveMsg()","Strange message1");
		}
		
		_buff = (int *) buff;
		datasize = _buff[1];
		readed_data = 0;
		pMsg->addNetMsg((BYTE *) buff,size);
		while(readed_data != datasize){
			sizeToRead = datasize-readed_data > NC_BUFFER_SIZE ? NC_BUFFER_SIZE : datasize-readed_data;	
			size = recv(sock,buff,sizeToRead,0);
			
			itime=2;
			while(size ==  SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK && itime < 1300){
				OUTS( "CNetClient::ReceiveMsg Warning:socket is blocked: ", 2 );
				OUTI( itime, 2 );OUTS("\n",2);
				Sleep(itime);itime*=5;
				size = recv(sock,buff,sizeToRead,0);	
			}
			if(size <= 0){
				CString err;err.Format("%d",WSAGetLastError());
				ERRORMSG( ERRSOCKET,"CNetClient::ReceiveMsg(): Strange Message.",err);
			}
			pMsg->addNetMsg((BYTE *) buff,size);
			readed_data += size;
		}
		SAFE_DELETE_ARRAY(buff);
		return readed_data+sizeToRead;
	}	
};
////////////////////////////////////////////////////////////////////
//Connect to server IP in IPv4 format and port.
//return ERRNOERROR - succ Return ERRSOCKET - error
HRESULT CNetClient::ConnectToServer(CString IP,u_short usPort){
	
	sockaddr_in clientService;
	int buffersize = 2097152;
	
	m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( m_socket == INVALID_SOCKET ) {
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetClient::ConnectToServer()",err);
    }

	
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr( IP );
    clientService.sin_port = htons( usPort );
	if (setsockopt(m_socket, SOL_SOCKET,SO_RCVBUF,(char *)&buffersize,sizeof(int))!=0){
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetClient::ConnectToServer()-setsockopt",err);
		
	};
	if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetClient::ConnectToServer()",err);
    }
	if(WSAAsyncSelect(m_socket, hWnd, WM_WSAASYNCC, FD_READ|FD_CONNECT|FD_CLOSE) == SOCKET_ERROR)
	{
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetClient::ConnectToServer()",err);
	}
	
	return ERRNOERROR;
}
////////////////////////////////////////////////////////////////////
//Start broadcast server and client listening on port.
//return ERRNOERROR - succ Return ERRSOCKET - error
HRESULT CNetClient::StartBroadcast(u_short port){
	
	///////////
	// broadcast
	
	sockaddr_in service;

	// For sure, stop broadcast first
	StopBroadcast();

	broadcast_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(broadcast_socket == INVALID_SOCKET){
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetClient::StartBroadcast()",err);
	}

	BOOL on = true;
	if (  setsockopt(broadcast_socket, SOL_SOCKET ,SO_BROADCAST,(char *) &on,sizeof(on) ) == SOCKET_ERROR){
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetClient::StartBroadcast()",err);
	}
	

	if(WSAAsyncSelect(broadcast_socket, hWnd, WM_WSAASYNCC, FD_READ|FD_WRITE|FD_ACCEPT|FD_CLOSE) == SOCKET_ERROR){
	
		closesocket(broadcast_socket);
		broadcast_socket=NULL;
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetClient::StartBroadcast()",err);

	}
	
	

    service.sin_family = AF_INET;
    service.sin_addr.s_addr =  htonl(INADDR_ANY);
    service.sin_port = htons( port );

	if ( bind( broadcast_socket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR ) {
        		
		closesocket(broadcast_socket);
		broadcast_socket=NULL;
        CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET," CNetClient::StartBroadcast()",err);
    }

	return ERRNOERROR;
};
////////////////////////////////////////////////////////////////////
//Close broadcast socket 
HRESULT CNetClient::StopBroadcast(){
	
	if(broadcast_socket) closesocket(broadcast_socket);
	broadcast_socket=NULL;
	return ERRNOERROR;
	
};
////////////////////////////////////////////////////////////////////
//Close socket 
int CNetClient::Disconnect(){
	
	closesocket(m_socket);
	m_socket = NULL;
	return ERRNOERROR;
};
////////////////////////////////////////////////////////////////////
//Send Broadcast message 
//return ERRNOERROR - ok, return ERROSOCKET - error
HRESULT CNetClient::SendBroadcastMsg(CNetMsg *pMsg,u_short port){
	
	sockaddr_in broadcastadr;
	/*  u_long host_addr = inet_addr("10.9.2.214");   // local IP addr
    u_long net_mask = inet_addr("255.255.248.0");   // LAN netmask
    u_long net_addr = host_addr & net_mask;         // 172.16.64.0
    u_long dir_bcast_addr = net_addr | (~net_mask); // 172.16.95.255
	*/
	
	broadcastadr.sin_family =AF_INET;
	broadcastadr.sin_port = htons( port );
	broadcastadr.sin_addr.s_addr = INADDR_BROADCAST;
	
	if ( sendto(broadcast_socket,pMsg->getBuffer(),pMsg->getBufferSize(),NULL,(SOCKADDR*) &broadcastadr,sizeof(broadcastadr)) == SOCKET_ERROR){
		 CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET," CNetClient::SendBroadcastMsg()",err);
	};

	return ERRNOERROR;
};
