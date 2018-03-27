#include "stdafx.h"
#include "CNetMsg.h"



using namespace Network;


CNetMsg::CNetMsg(){
	
	buffer=NULL;
	buffSize = 0;
};

CNetMsg::~CNetMsg(){
	buffSize = 0;
	SAFE_DELETE_ARRAY(buffer); 

};	
void CNetMsg::Release(){
	buffSize = 0;
	SAFE_DELETE_ARRAY(buffer); 
};



//////////////////////////////////////////////////////////////
// Set type of message,pointer to data, datasize to msg buffer
HRESULT CNetMsg::setNetMsg(int type,BYTE * data,int dataSize){

	int *_buff;
	SAFE_DELETE_ARRAY(buffer);
	buffer= new BYTE[dataSize + sizeof(int) + sizeof(int)];
	if (!buffer) ERRORMSG(ERROUTOFMEMORY,"CNetMsg::setNetMsg()","");

	_buff=(int *) buffer;
	
	buffSize=dataSize + sizeof(int) + sizeof(int);
	_buff[0]=type;
	_buff[1]=dataSize;
	if(data && dataSize) memcpy(buffer+sizeof(int) + sizeof(int),data,dataSize);
	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////
// Set only data, datasize to msg buffer
HRESULT CNetMsg::setNetMsg(BYTE * data,int dataSize){
	
	SAFE_DELETE_ARRAY(buffer);
	buffer= new BYTE[dataSize];
	
	if (!buffer) ERRORMSG(ERROUTOFMEMORY,"CNetMsg::setNetMsg()","");
	buffSize=dataSize;
	memcpy(buffer,data,dataSize);
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////
// Adding data to message buffer
HRESULT CNetMsg::addNetMsg( BYTE * data,int dataSize ){
	
	BYTE * _buff;
	_buff = buffer;
	buffer = new BYTE[buffSize + dataSize];
	if (!buffer){
		SAFE_DELETE_ARRAY(_buff);	
		ERRORMSG(ERROUTOFMEMORY,"CNetMsg::addNetMsg()","");
	};
	
	memcpy(buffer,_buff,buffSize);
	memcpy(buffer + buffSize,data,dataSize);
	buffSize += dataSize;
	
	SAFE_DELETE_ARRAY(_buff);
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////
// Set message: TYPE |any index | data |datasize
HRESULT CNetMsg::setNetMsg(int type,DWORD index,BYTE * data,int dataSize){
	
	SAFE_DELETE_ARRAY(buffer);
	DWORD *_buffer;
	int * _buff;
	buffer= new BYTE[dataSize + sizeof(int) + sizeof(int) + sizeof(DWORD)];
	
	if (!buffer) ERRORMSG(ERROUTOFMEMORY,"CNetMsg::setNetMsg()","");
	
	buffSize=dataSize + sizeof(int) + sizeof(int)+ sizeof(DWORD);
	
	_buff = (int *) buffer;
	_buff[0]=type;
	_buff[1]=dataSize + sizeof(DWORD);
	_buffer = (DWORD *)(buffer + sizeof(int) + sizeof(int));
	_buffer[0]=index;

	memcpy(buffer+ sizeof(int) + sizeof(int) + sizeof(DWORD),data,dataSize);
	
	return ERRNOERROR;


};


//////////////////////////////////////////////////////////////
// getType of buffered data
int CNetMsg::getType(){
	int * _buff = (int *) buffer;
	if(buffer)
		return _buff[0];
	else return 0;
};


//////////////////////////////////////////////////////////////
// Get only data not type
void * CNetMsg::getData(){
	int *_buff = (int *) buffer;
	if(buffer && _buff[1])
	return (buffer+ sizeof(int) + sizeof(int));
	else return 0;
};

//////////////////////////////////////////////////////////////
// Get only data not type not index from buffer
void * CNetMsg::getIndexedData(){
	if(buffer){
		return buffer+ sizeof(int) + sizeof(int)+sizeof(DWORD);
	}
	else return 0;
	
};
		

//////////////////////////////////////////////////////////////
// Get only index from buffer
DWORD CNetMsg::getIndex(){
	int *_buff = (int *) buffer;
	if(buffer && _buff[1])
		return *((DWORD *)(buffer+ sizeof(int) + sizeof(int)));
	else return 0;
};
	