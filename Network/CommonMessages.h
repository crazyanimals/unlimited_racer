/*
	CommonMessages:	This tiny module offers several functions which can be used for easier network message setup.
	Creation Date:	10.12.2006
	Last Update:	4.2.2007
	Author:			Roman Margold
*/


#pragma once

#include "stdafx.h"
#include "CNetMsg.h"


namespace Network
{

	HRESULT		SetMsgID( CNetMsg * Msg, int ID );
	HRESULT		SetMsgMap( CNetMsg * Msg, CString * MapName );
	HRESULT		SetMsgMapCRCs( CNetMsg * Msg, std::multiset<DWORD> * MapCRCs );
	HRESULT 	SetMsgAllCarsFileNames( CNetMsg * Msg, std::vector<CString> * AllCarsFileNames );
	HRESULT		SetMsgCarCRCs( CNetMsg * Msg, UINT CarNum, std::vector<std::multiset<DWORD>> * AllowedCarsCRCs );
	HRESULT 	SetMsgPlayerStatus( CNetMsg * Msg, UINT ID, CString * Name, CString * Car, int TeamID, BOOL Ready, BOOL Active );
	HRESULT		SetMsgGameName( CNetMsg * Msg, CString * GameName );
	HRESULT		SetMsgGameType( CNetMsg * Msg, CString * GameType );
	HRESULT		SetMsgChat( CNetMsg * Msg, CString * ChatMessage );
	HRESULT		SetMsgCRCs( CNetMsg * Msg, std::multiset<DWORD> * CRCs );
	HRESULT 	SetMsgGameSettings( CNetMsg * Msg, int TimeHours, int TimeMinutes/*RACER MOD ONLY:, int NumLaps*/ );
	HRESULT		SetMsgString( CNetMsg * Msg, WORD msgID, CString & text );
	//RACER MOD ONLY:
	//HRESULT		SetMsgCheckpointPosition( CNetMsg * Msg, D3DXVECTOR3 * Position );
	//HRESULT		SetMsgLapsGone( CNetMsg * Msg, UINT ID, int LapsGone );
	//HRESULT 		SetMsgRacePosition( CNetMsg * Msg, UINT ID, UINT RacePosition );

	HRESULT 	GetMsgName( CNetMsg * Msg, CString & Name );
	HRESULT 	GetMsgCRCs( CNetMsg * Msg, std::multiset<DWORD> & CRCs );
	HRESULT 	GetMsgAllCarsFileNames( CNetMsg * Msg, std::vector<CString> & Names );
	HRESULT 	GetMsgPlayerStatus(	CNetMsg * Msg,
									UINT & ID,
									CString & Name,
									CString & Car,
									int & TeamID,
									BOOL & Ready,
									BOOL & Active );
	HRESULT 	GetMsgGameSettings( CNetMsg * Msg, int & TimeHours, int & TimeMinutes /*RACER MOD ONLY:, int & NumberOfLaps */ );
	//RACER MOD ONLY:
	//HRESULT 	GetMsgCheckpointPosition( CNetMsg * Msg, D3DXVECTOR3 & Position );
	//HRESULT 	GetMsgLapsGone( CNetMsg * Msg, UINT & ID, int & LapsGone );
	//HRESULT 	GetMsgRacePosition( CNetMsg * Msg, UINT & ID, UINT & RacePosition );


}; // end of namespace