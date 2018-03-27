#pragma once
#include "stdafx.h"
#include "..\Graphic\Graphic.h"
#include "..\Client\CClient.h"
#include "..\GameMods\GameMod.h"

#define MAX_DEVICES			8
#define NUM_OF_ACTIONS		(21+MAX_CUSTOM_KEYS)	// 21 initial action + actions supported by custom keys
#define NUM_OF_ACTMAP		(21+MAX_CUSTOM_KEYS)
#define NUM_FOR_SEND		6	
#define BUTTON_DOWN			0x80 // Mask for determining button state
#define KEYS_TO_SEND_SIZE	(CUSTOM_KEY_INDEX + ((MAX_CUSTOM_KEYS+7) >> 3))  // size in bytes of the array KeysToSend

// default keys are defined in Physics module, however game MOD key codes are defined here:
#define KEY_MODCUSTOM1	0x00000001
#define KEY_MODCUSTOM2	0x00000002
#define KEY_MODCUSTOM3	0x00000004
#define KEY_MODCUSTOM4	0x00000008
#define KEY_MODCUSTOM5	0x00000010
#define KEY_MODCUSTOM6	0x00000020
#define KEY_MODCUSTOM7	0x00000040
#define KEY_MODCUSTOM8	0x00000080


enum nGameActions
{
	n_STEER,
	n_ACCELERATE,
	n_DECELERATE,
	n_BRAKE,
	n_UPSHIFT,
	n_DOWNSHIFT,
	
};
enum eGameActions 
{
    eA_STEER,       // Steering 
    eA_DECELERATE,  // Brake 
	eA_ACCELERATE,  // Change speed
	eA_UPSHIFT,
	eA_DOWNSHIFT,
	eA_CYCLEVIEW,
	eB_STEER_LEFT,  // Steer left 
    eB_STEER_RIGHT, // Steer right
    eB_ACCELERATE,  // Speed up
    eB_DECELERATE,  // Slow down
    eB_BRAKE,       // Brake 
    eB_UPSHIFT,     // Shift to higher gear
    eB_DOWNSHIFT,   // Shift to lower gear
    // eB_BRAKEBIAS,   // Brake bias 
    eB_CYCLEVIEW,   // Cycle to next view
    eB_DRIVERVIEW,  // View from driver's seat 
	eB_GAMEPAUSE, // Game pause
	eB_RESTART, // Restores car position from unrecoverable situation
	eM_CAMHORIZONTAL, //camera move
	eM_CAMVERTICAL,	//camera move
	eM_CAMFORW, //camera move
	eM_CAMBACK,	//camera move
	
	// custom MOD actions, there HAS TO be as many action as the MAX_CUSTOM_KEYS macro says
	eC_ACTION1,
	eC_ACTION2,
	eC_ACTION3,
	eC_ACTION4,
	eC_ACTION5,
	eC_ACTION6,
	eC_ACTION7,
	eC_ACTION8

};


struct DeviceState
{
    LPDIRECTINPUTDEVICE8 pDevice;   // Pointer to the device 
    CString szName;					// Friendly name of the device
    bool  bAxisRelative;            // Relative x-axis data flag
    DWORD dwInput[NUM_OF_ACTIONS];  // Arrays of the current input values and
    bool  bMapped[NUM_OF_ACTIONS];  // Flags whether action was successfully
};                                  //   mapped to a device object



class CGameInput{
	

	private:
		
		int						numDevices;
		LPDIRECTINPUT8			pDI; 
		DIACTION				actionsMap[NUM_OF_ACTMAP];
		DeviceState				DIDevice[MAX_DEVICES];
		DIACTIONFORMAT			diaf;
		int						left,right;
		
		// Maps windows virtual key into DirectInput virtual key
		DWORD					MapKeyFromWindowsIntoDirectInput( int Key );
		DWORD					GetMapKeySafe( CGameMod::INT_LIST * pList, UINT keyIndex, UINT mapIndex );
		
	
	public:
		
		DWORD					allKeys[NUM_FOR_SEND];
		HWND					hWnd;
		// This is structure, which is send to server
		// the format is following:
		// Byte 1 - Steer - range -99 to 99	CONTROL_MAX_VALUE
		// Byte 2 - Accelerate - range 0 to 99 CONTROL_MAX_VALUE
		// Byte 3 - Decelerate - range 0 to 99 CONTROL_MAX_VALUE
		// Byte 4 is bit field:
		// Upshift
		// Downshift
		// Cycleview
		// Driver view
		// Brake
		// Pause
		// Restart
		// Byte 5 and above are used by custom keys defined by game MOD; by default, each bit represents
		//  one key, where the lowest bit represents the first one and so on (but can be probably different in 
		//  some game MOD implementation)
		char					KeysToSend[KEYS_TO_SEND_SIZE];

		CGameInput();
		~CGameInput();
		void CheckInput();
		void React(graphic::CGraphic *);

		// Dumps actions into file
		void DumpActions();

		// Reloads new action mapping from configuration
		HRESULT RefreshActionMapping( CConfig * Configuration, CGameMod::INT_LIST * pMODKeys ); 
		
		HRESULT init(HINSTANCE);
		
		
		BOOL EnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi,LPDIRECTINPUTDEVICE8 lpdid, 
								DWORD dwFlags, DWORD dwRemaining);

		static BOOL CALLBACK StaticEnumDevicesCallback(	LPCDIDEVICEINSTANCE lpddi,
														LPDIRECTINPUTDEVICE8 lpdid, 
														DWORD dwFlags,
														DWORD dwRemaining,
														LPVOID pvRef )
				{ return ((CGameInput *) pvRef)->EnumDevicesCallback(lpddi,lpdid,dwFlags,dwRemaining);}
		
	
};