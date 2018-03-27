#include "stdafx.h"
#include "CNetServer.h"


using namespace Network;


CNetServer::~CNetServer(){
       WSACleanup();
	   
}
////////////////////////////////////////////////////////////////////
//Network initialization
//return 0 
HRESULT CNetServer::Init(){
	
	HRESULT hr=ERRNOERROR;
	WSADATA wsaData;
    int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
    //if ( iResult != NO_ERROR );
        //ERROR printf("Error at WSAStartup()\n");
   
	return hr;
};

////////////////////////////////////////////////////////////////////
//Send message in CNetMsg format to socket
//Return 0 - succ, Return ERRSOCKET - error

int CNetServer::SendMsgToSocket(SOCKET sock,CNetMsg * pMsg){
	int itime=10;
	int neterr;
	neterr=send(sock,pMsg->getBuffer(),pMsg->getBufferSize(),0);
	while(neterr==SOCKET_ERROR  && WSAGetLastError() == WSAEWOULDBLOCK && itime < 1300){
		OUTS( "CNetServer::SendMsgToSocket() Warning:socket is blocked: ", 2 );
		OUTI( itime, 2 );OUTS("\n",2);
		Sleep(itime);itime*=5;
		neterr = send(sock,pMsg->getBuffer(),pMsg->getBufferSize(),0);	
	};
	if(neterr==SOCKET_ERROR){
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetServer::SendMsgToSocket()",err);
		ClosePlayer(GetPlayerID(sock));
	}
	return ERRNOERROR;

}

////////////////////////////////////////////////////////////////////
//Close all sockets
//Return ERRNOERROR - succ, Return ERRSOCKET - error
HRESULT CNetServer::StopSession(){
	
	CloseAllPlayersSocket();
	if(m_socket){
		closesocket(m_socket);
		m_socket= NULL;
	}
	if(broadcast_socket){
		closesocket(broadcast_socket);
		broadcast_socket = NULL;
	}

	return ERRNOERROR;
};

////////////////////////////////////////////////////////////////////
// Host session on port
//Return ERRNOERROR - succ, Return ERRSOCKET - error
HRESULT CNetServer::HostSession(u_short dwPort){
	

	
	sockaddr_in service;
	
    
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	

    if ( m_socket == INVALID_SOCKET ) {
        CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetServer::HostSession()",err);
    };

   
    
	
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = htonl(INADDR_ANY);
    service.sin_port = htons( dwPort );
	
	
	if(WSAAsyncSelect(m_socket, hWnd, WM_WSAASYNCS, FD_READ|FD_ACCEPT|FD_CLOSE) == SOCKET_ERROR){
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetServer::HostSession()",err);

	}

	if ( bind( m_socket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR ) {
        CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetServer::HostSession()",err);
    }
	if ( listen( m_socket, 1 ) == SOCKET_ERROR ){
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetServer::HostSession()",err);
	}
	
	return ERRNOERROR;
};
////////////////////////////////////////////////////////////////////
//Close broadcast socket 
//return ERRNOERROR - succ Return ERRSOCKET - error
HRESULT CNetServer::StartBroadcast(u_short port){
	
	///////////
	// broadcast
	HRESULT hr=ERRNOERROR;
	sockaddr_in service;

	broadcast_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(broadcast_socket == INVALID_SOCKET){
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetServer::StartBroadcast()",err);
	}

	BOOL on = true;
	if (  setsockopt(broadcast_socket, SOL_SOCKET ,SO_BROADCAST,(char *) &on,sizeof(on) ) == SOCKET_ERROR){
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetServer::StartBroadcast()",err);
	}
	

	if(WSAAsyncSelect(broadcast_socket, hWnd, WM_WSAASYNCS, FD_READ|FD_WRITE|FD_ACCEPT|FD_CLOSE) == SOCKET_ERROR){
		
		closesocket(broadcast_socket);
		broadcast_socket=NULL;
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetServer::StartBroadcast()",err);

	}
	
	// bind to any address - used in case IP list is not obtained
	// because in that case we will try to broadcast on all IPs
    service.sin_family = AF_INET;
    service.sin_addr.s_addr =  htonl(INADDR_ANY);
    service.sin_port = htons( port );

	if ( bind( broadcast_socket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR ) {
		
		closesocket(broadcast_socket);
		broadcast_socket=NULL;
        CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET,"CNetServer::StartBroadcast()",err);
    }




	return ERRNOERROR;
};

// Binds broadcast socket on specific IP
HRESULT CNetServer::BindBroadcast(sockaddr_in * Address, u_short port)
{
	sockaddr_in service;

	if (broadcast_socket)
	{
		service.sin_addr = Address->sin_addr;
		service.sin_family = AF_INET;
		service.sin_port = htons( port );

		bind(broadcast_socket, (SOCKADDR*) &service, sizeof(service) );
	}

	return ERRNOERROR;
}
////////////////////////////////////////////////////////////////////
//Close broadcast socket 
HRESULT CNetServer::StopBroadcast(){

	if(broadcast_socket) closesocket(broadcast_socket);
	broadcast_socket=NULL;
	return ERRNOERROR;
	
};
////////////////////////////////////////////////////////////////////
//Send Broadcast message 
//return ERRNOERROR - ok, return ERROSOCKET - error
HRESULT CNetServer::SendBroadcastMsg(CNetMsg *pMsg,u_short port){
	
	HRESULT hr=ERRNOERROR;
	
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
		ERRORMSG( ERRSOCKET," CNetServer::SendBroadcastMsg()",err);	
	};

	return hr;
};
/////////////////////////////////////////////
//Return -1 when is server full
//Return ERRSOCKET when accepted failed
//Return index to players
int CNetServer::AcceptPlayer(){
	
	
	SOCKADDR_IN clientInfo;
	SOCKET c_socket;
	CNetMsg msg;
	int t;

	int cl_size = sizeof(clientInfo);
	

	///
	// Find free socket
	t = -1;
	for(int i=0;i < MAX_CONNECTIONS;i++){
		if(players[i] == 0) {
			t = i;
			break;
		}
	}
	if (t == -1) return -1;
	
	c_socket = accept(m_socket, (SOCKADDR *)&clientInfo, &cl_size);
	if(c_socket == INVALID_SOCKET)
	{
		CString err;err.Format("%d",WSAGetLastError());
		ERRORMSG( ERRSOCKET," bind()",err);	
		
	}
	players[t] = c_socket;
	
	return t;
};

///////////////////////////////////////////////////////////////////////
//return 0 - succed return INVALID_SOCKET - error
int CNetServer::ClosePlayer(int t){
	int err=0;
	if (players[t]) err=closesocket(players[t]);
	players[t]=0;
	return err;
};


int CNetServer::CloseAllPlayersSocket(){
	int err = 0;
	for(int i=0;i < MAX_CONNECTIONS;i++)
		err += ClosePlayer(i);
	return err;

};

////////////////////////////////////////////////////////////////////
//Receive message from socket and fill CNetMsg
//Return size of message in bytes, Return ERRSOCKET - error
int CNetServer::ReceiveMsg(SOCKET sock,CNetMsg * pMsg){
	
	
BOOL broad;int boolsize = sizeof(BOOL);
	

	int size;
	char * buff = new char[NS_BUFFER_SIZE];

	if (  getsockopt(sock, SOL_SOCKET ,SO_BROADCAST,(char *) &broad,&boolsize ) == SOCKET_ERROR){
		broad=false;
	}	
		
	if(broad){
		
		size = recv(sock,buff,NS_BUFFER_SIZE,0);
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

		if(size != sizeToRead) {
			SAFE_DELETE_ARRAY(buff);
			ERRORMSG(-1,"CNetServer::ReceiveMsg()","Strange message1");
		}
		
		_buff = (int *) buff;
		datasize = _buff[1];
		readed_data = 0;
		pMsg->addNetMsg((BYTE *) buff,size);
		while(readed_data != datasize){
			sizeToRead = datasize-readed_data > NS_BUFFER_SIZE ? NS_BUFFER_SIZE : datasize-readed_data;	
			size = recv(sock,buff,sizeToRead,0);
			
			itime=2;
			while(size ==  SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK && itime < 1300){
				OUTS( "CNetServer::ReceiveMsg() Warning:socket is blocked: ", 2 );
				OUTI( itime, 2 );OUTS("\n",2);
				Sleep(itime);itime*=5;
				size = recv(sock,buff,sizeToRead,0);	
			}
			if(size <= 0){
				SAFE_DELETE_ARRAY(buff);	
				ERRORMSG(-1,"CNetServer::ReceiveMsg()","Strange message2");
			}
			pMsg->addNetMsg((BYTE *) buff,size);
			readed_data += size;
		}
		SAFE_DELETE_ARRAY(buff);
		return readed_data+sizeToRead;
	}
}
////////////////////////////////////////////////////
// Send message to Player, If ID is -1 send to all
HRESULT CNetServer::SendMsg(CNetMsg * pMsg,int IDPlayer){
	
	int i;
	int itime=2;
	int neterr;
	CString err;

	if(IDPlayer == -1){
		for(i = 0; i < MAX_CONNECTIONS;i++){
			if(players[i]){ 
				itime=2;
				neterr = send(players[i],pMsg->getBuffer(),pMsg->getBufferSize(),0);
				while(neterr ==  SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK && itime<1300){		
					OUTS( "CNetServer::SendMsg() Warning:socket is blocked: ", 2 );
					OUTI( itime, 2 );OUTS("\n",2);
					Sleep(itime);itime*=5;			
					neterr =send(players[i],pMsg->getBuffer(),pMsg->getBufferSize(),0);		
						
				}
				if(neterr == SOCKET_ERROR){ 
					err.Format("%d",WSAGetLastError());
					ClosePlayer(i);
					ERRORMSG( ERRSOCKET,"CNetServer::SendMsg()",err);
				};
			}
		}
	}else
		if(players[IDPlayer]){
			itime=2;
			neterr=send(players[IDPlayer],pMsg->getBuffer(),pMsg->getBufferSize(),0);
			while(neterr == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK && itime < 1300){
						OUTS( "CNetServer::SendMsg() Warning:socket is blocked: ", 2);
						OUTI( itime, 2 );OUTS("\n",2);
						Sleep(itime);itime*=5;
						neterr=send(players[IDPlayer],pMsg->getBuffer(),pMsg->getBufferSize(),0);	
			}
			if(neterr == SOCKET_ERROR){
				err.Format("%d",WSAGetLastError());
				ClosePlayer(IDPlayer);
				ERRORMSG( ERRSOCKET,"CNetServer::SendMsg()",err);	
			};
		}
	return ERRNOERROR;
};
////////////////////////////////////////////////////
// return player ID
// return -1 when socket is unknown
int CNetServer::GetPlayerID(SOCKET _sock){
	
	int i;
	for (i = 0; i < MAX_CONNECTIONS;i++){
		if(players[i] == _sock){
			return i;

		}
	};
	return -1;

}