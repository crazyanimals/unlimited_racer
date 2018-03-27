#pragma once

#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\Physics\toolbox\dMatrix.h"


#define MAX_GAME_NAME_LENGTH	MAX_MAP_NAME_LENGTH	// is also used for game type length restriction
#define MAX_HOST_NAME_LENGTH	64
#define MAX_PLAYER_NAME_LENGTH	15
#define MAX_CAR_NAME_LENGTH		20


// message identifier constants
#define NETMSG_NULL					0


#define NETMSG_PING					9
#define NETMSG_PING_BACK			10
#define NETMSG_ACCEPTED				11
#define NETMSG_REJECTED				12


#define NETMSG_PLAYER_DATA			13
#define NETMSG_PLAYER_CAR_NAME		14
#define NETMSG_PLAYER_READY			15
#define NETMSG_PLAYER_MAP_NAME		16
#define NETMSG_PLAYER_ID			17
#define NETMSG_PLAYER_CRC			18
#define NETMSG_PLAYER_CAR_NAMES		19

#define NETMSG_PLAYER_STATUS		50
#define NETMSG_MAP_SETTINGS			51
#define NETMSG_GAME_NAME			52
#define NETMSG_CHAT					53
#define NETMSG_MULTIGAME_STARTLOAD	54
#define NETMSG_PLAYER_LOADED		55
#define NETMSG_STATIC_POSITION		56
#define NETMSG_CAR_SPEED			57
#define NETMSG_CAR_GEAR				58
#define NETMSG_ENGINE_RPM			59
//RACER MOD ONLY:
//#define NETMSG_CHECKPOINT_POSITION	59
//#define NETMSG_LAPS_GONE			60
//#define NETMSG_RACE_POSITION		61
#define NETMSG_RETURNTOLOBBY		62

//#define NETMSG_SHOW_DIALOG_3		63
//#define NETMSG_SHOW_DIALOG_2		64
//#define NETMSG_SHOW_DIALOG_1		65
//#define NETMSG_SHOW_DIALOG_GO		66
//#define NETMSG_HIDE_DIALOG		67
#define	NETMSG_GAME_TYPE			68
#define	NETMSG_HUD					0x0080	// flag specifying HUD msg
#define	NETMSG_HUD_SCORE			(NETMSG_HUD + 0x0001)
#define NETMSG_HUD_INFO_TEXT        (NETMSG_HUD + 0x0002)
#define NETMSG_HUD_TIME				(NETMSG_HUD + 0x0003)

#define NETMSG_CAR_POSITION			20
#define NETMSG_OBJECT_POSITION		21
#define NETMSG_CONTROLS				30
#define NETMSG_BROADCAST_DEMAND		40
#define NETMSG_BROADCAST_SERVER_INFO 41

#define NETMSG_GAMEMOD				0x8000 // flag specifying GAMEMOD custom message
#define NETMSG_SOCCER_GAME_EVENT    (NETMSG_GAMEMOD | 1) // some king of game event - event is specified in the message data).



namespace Network
{

	struct SERVER_INFO
	{
		char name[MAX_GAME_NAME_LENGTH];
		u_short port;
		char addr[MAX_HOST_NAME_LENGTH];
		BOOL Address;
		char type[MAX_GAME_NAME_LENGTH]; // specifies the game MOD ID
	};

	struct MSG_CAR{
		dMatrix mxCar;
		int iWheelNum;
		int idWheel[MAX_CAR_WHEELS];
		dMatrix mxWheel[MAX_CAR_WHEELS];
		dVector camPos;
	};



	///////////////////////////////////////////////////
	// PROTOCOL buffer = type(INT)|size(INT)|data
	///////////////////////////////////////////////////

	class CNetMsg
	{
		
		public:
						CNetMsg();
						~CNetMsg();
				
			
			HRESULT		setNetMsg(int type,BYTE * data,int dataSize); 
			HRESULT		setNetMsg( BYTE * data,int dataSize );
			HRESULT		addNetMsg( BYTE * data,int dataSize );
			HRESULT		setNetMsg(int type,DWORD index,BYTE * data,int dataSize);
			void		Release();
			char	*	getBuffer(){ return (char *)buffer;}
			int			getBufferSize(){ return buffSize;}
			void	*	getData();
			void	*	getIndexedData();
			DWORD		getIndex();
			int			getType();
		
		private:
			BYTE	*	buffer;
			int			buffSize;		
			
	};


}; // end of namespace Network