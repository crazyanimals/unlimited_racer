#include "controls.h"

using namespace graphic;

CGameInput::CGameInput(){
	numDevices = 0;
	int i=0;	

	
	////////////////////////////////////////////
	// VOLANT
	///////////////////////////////////////////
	actionsMap[i].uAppData= eA_STEER; 
	actionsMap[i].dwSemantic= DIAXIS_DRIVINGR_STEER;
	actionsMap[i].dwFlags= 0;
	actionsMap[i].lptszActionName="Steer";
	i++;
	actionsMap[i].uAppData=eA_ACCELERATE;
	actionsMap[i].dwSemantic=  DIAXIS_DRIVINGR_ACCELERATE;
	actionsMap[i].dwFlags=   0;
	actionsMap[i].lptszActionName= "Accelerate";
	i++;
	actionsMap[i].uAppData=eA_DECELERATE;
	actionsMap[i].dwSemantic=       DIAXIS_DRIVINGR_BRAKE;
	actionsMap[i].dwFlags=        0;
	actionsMap[i].lptszActionName= "Brake";
	i++;
	//Genre-defined virtual buttons 

	actionsMap[i].uAppData=eA_UPSHIFT;
	actionsMap[i].dwSemantic=     DIBUTTON_DRIVINGR_SHIFTUP;
	actionsMap[i].dwFlags=    0;
	actionsMap[i].lptszActionName= "Upshift";
	i++;
	actionsMap[i].uAppData=eA_DOWNSHIFT;
	actionsMap[i].dwSemantic=   DIBUTTON_DRIVINGR_SHIFTDOWN;
	actionsMap[i].dwFlags=  0;
	actionsMap[i].lptszActionName= "DownShift";
	i++;
	actionsMap[i].uAppData=eA_CYCLEVIEW;
	actionsMap[i].dwSemantic=   DIBUTTON_DRIVINGR_VIEW;
	actionsMap[i].dwFlags=       0;
	actionsMap[i].lptszActionName= "Change View";
	
	////////////////////////////////////////////
	// keyboard
	///////////////////////////////////////////
	// Actions mapped to keys as well as to virtual controls
	i++;
	actionsMap[i].uAppData=eB_DRIVERVIEW;
	actionsMap[i].dwSemantic=  DIKEYBOARD_B;
	actionsMap[i].dwFlags= 0;
	actionsMap[i].lptszActionName= "Driver View";
	i++;
	actionsMap[i].uAppData=eB_CYCLEVIEW;
	actionsMap[i].dwSemantic=  DIKEYBOARD_C;
	actionsMap[i].dwFlags= 0;
	actionsMap[i].lptszActionName= "Cycle View";
	i++;
	actionsMap[i].uAppData=eB_UPSHIFT;
	actionsMap[i].dwSemantic=     DIKEYBOARD_PRIOR;
	actionsMap[i].dwFlags=              0;
	actionsMap[i].lptszActionName= "Upshift";
	i++;
	actionsMap[i].uAppData=eB_DOWNSHIFT;
	actionsMap[i].dwSemantic=   DIKEYBOARD_NEXT;
	actionsMap[i].dwFlags=               0;
	actionsMap[i].lptszActionName= "Downshift";
	i++;
	actionsMap[i].uAppData=eB_STEER_LEFT;
	actionsMap[i].dwSemantic=  DIKEYBOARD_LEFT;
	actionsMap[i].dwFlags=               0;
	actionsMap[i].lptszActionName= "Steer Left";
	i++;
	actionsMap[i].uAppData=eB_STEER_RIGHT;
	actionsMap[i].dwSemantic= DIKEYBOARD_RIGHT;
	actionsMap[i].dwFlags=              0;
	actionsMap[i].lptszActionName= "Steer Right";
	i++;
	actionsMap[i].uAppData=eB_ACCELERATE;
	actionsMap[i].dwSemantic=  DIKEYBOARD_UP;
	actionsMap[i].dwFlags=                 0;
	actionsMap[i].lptszActionName= "Accelerate";
	i++;
	actionsMap[i].uAppData=eB_DECELERATE;
	actionsMap[i].dwSemantic=  DIKEYBOARD_DOWN;
	actionsMap[i].dwFlags=               0;
	actionsMap[i].lptszActionName= "Decelerate";
	i++;
	actionsMap[i].uAppData=eB_BRAKE;
	actionsMap[i].dwSemantic=       DIKEYBOARD_END;
	actionsMap[i].dwFlags=                0;
	actionsMap[i].lptszActionName= "Brake";
	i++;
	actionsMap[i].uAppData=eB_GAMEPAUSE;
	actionsMap[i].dwSemantic=       DIKEYBOARD_P;
	actionsMap[i].dwFlags=                0;
	actionsMap[i].lptszActionName= "Game pause";
	i++;
	actionsMap[i].uAppData=eB_RESTART;
	actionsMap[i].dwSemantic=       DIKEYBOARD_R;
	actionsMap[i].dwFlags=                0;
	actionsMap[i].lptszActionName= "Car position Restart";
	i++;


	/////////////////////////////////////////////
	// mouse
	/////////////////////////////////////////////

	actionsMap[i].uAppData=eM_CAMVERTICAL;
	actionsMap[i].dwSemantic=DIMOUSE_YAXIS;
	actionsMap[i].dwFlags=              0;
	actionsMap[i].lptszActionName= "Camera vertical move";
	i++;
	actionsMap[i].uAppData=eM_CAMHORIZONTAL;
	actionsMap[i].dwSemantic=DIMOUSE_XAXIS;
	actionsMap[i].dwFlags=              0;
	actionsMap[i].lptszActionName= "Camera horizontal move";
	i++;
	actionsMap[i].uAppData=eM_CAMFORW;
	actionsMap[i].dwSemantic=     DIMOUSE_BUTTON0;
	actionsMap[i].dwFlags=              0;
	actionsMap[i].lptszActionName= "Camera forward";
	i++;
	actionsMap[i].uAppData=eM_CAMBACK;
	actionsMap[i].dwSemantic=DIMOUSE_BUTTON1;
	actionsMap[i].dwFlags=              0;
	actionsMap[i].lptszActionName="Camera backward";
	i++;

	/////////////////////////////////////////////
	// custom MOD actions
	/////////////////////////////////////////////
	
	actionsMap[i].uAppData = eC_ACTION1;
	actionsMap[i].dwSemantic = DIKEYBOARD_1;
	actionsMap[i].dwFlags = 0;
	actionsMap[i].lptszActionName = "Custom MOD action 1";
	i++;
	actionsMap[i].uAppData = eC_ACTION2;
	actionsMap[i].dwSemantic = DIKEYBOARD_2;
	actionsMap[i].dwFlags = 0;
	actionsMap[i].lptszActionName = "Custom MOD action 2";
	i++;
	actionsMap[i].uAppData = eC_ACTION3;
	actionsMap[i].dwSemantic = DIKEYBOARD_3;
	actionsMap[i].dwFlags = 0;
	actionsMap[i].lptszActionName = "Custom MOD action 3";
	i++;
	actionsMap[i].uAppData = eC_ACTION4;
	actionsMap[i].dwSemantic = DIKEYBOARD_4;
	actionsMap[i].dwFlags = 0;
	actionsMap[i].lptszActionName = "Custom MOD action 4";
	i++;
	actionsMap[i].uAppData = eC_ACTION5;
	actionsMap[i].dwSemantic = DIKEYBOARD_5;
	actionsMap[i].dwFlags = 0;
	actionsMap[i].lptszActionName = "Custom MOD action 5";
	i++;
	actionsMap[i].uAppData = eC_ACTION6;
	actionsMap[i].dwSemantic = DIKEYBOARD_6;
	actionsMap[i].dwFlags = 0;
	actionsMap[i].lptszActionName = "Custom MOD action 6";
	i++;
	actionsMap[i].uAppData = eC_ACTION7;
	actionsMap[i].dwSemantic = DIKEYBOARD_7;
	actionsMap[i].dwFlags = 0;
	actionsMap[i].lptszActionName = "Custom MOD action 7";
	i++;
	actionsMap[i].uAppData = eC_ACTION8;
	actionsMap[i].dwSemantic = DIKEYBOARD_8;
	actionsMap[i].dwFlags = 0;
	actionsMap[i].lptszActionName = "Custom MOD action 8";
	i++;

};

CGameInput::~CGameInput()
{
	int i;
	// Unaquire and release devices
	for (i = 0; i < numDevices; i++)
	{
		DIDevice[i].pDevice->Unacquire();
		SAFE_RELEASE(DIDevice[i].pDevice);
	}

	// release direct input
	SAFE_RELEASE(pDI);
}

HRESULT CGameInput::init(HINSTANCE _hInst){
   
	HRESULT hr=ERRNOERROR;

    if (DirectInput8Create(_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&pDI, NULL)) //Direct input initialization
		ERRORMSG(ERRGENERIC,"DirectInput8Create()","DirectInput initialization failed");
	
	
    ZeroMemory( &diaf, sizeof(DIACTIONFORMAT) );
    diaf.dwSize          = sizeof(DIACTIONFORMAT);
    diaf.dwActionSize    = sizeof(DIACTION);
    diaf.dwDataSize      = NUM_OF_ACTMAP * sizeof(DWORD);
    diaf.guidActionMap   = g_guidApp;
    diaf.dwGenre         = DIVIRTUAL_DRIVING_RACE; 
    diaf.dwNumActions    = NUM_OF_ACTMAP;
    diaf.rgoAction       = actionsMap;
    diaf.lAxisMin        = -CONTROL_MAX_VALUE;
    diaf.lAxisMax        = CONTROL_MAX_VALUE;
    diaf.dwBufferSize    = 16;
    _tcscpy( diaf.tszActionMap, _T("ActionMap") );
 
	
	if( pDI->EnumDevicesBySemantics( NULL, &diaf,StaticEnumDevicesCallback,this, DIEDBSFL_ATTACHEDONLY )){
			ERRORMSG(ERRGENERIC,"pDI->EnumDevicesBySemantics()","Enum devices failed");
	}

	return hr;
}



BOOL  CGameInput::EnumDevicesCallback( LPCDIDEVICEINSTANCE lpddi,LPDIRECTINPUTDEVICE8 lpdid,
									  DWORD dwFlags, DWORD dwRemaining )
{
    HRESULT hr;
	
    if( numDevices < MAX_DEVICES )
    {
		hr = lpdid->Unacquire();
		//hr = lpdid->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		hr = lpdid->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		

		// Build the action map against the device
       //if( FAILED(hr=lpdid->BuildActionMap( &diaf, NULL, DIDBAM_DEFAULT )) )
		if( FAILED(hr=lpdid->BuildActionMap( &diaf, "UnlimitedRacer", 0)) )
            // There was an error while building the action map. Ignore this
            // device, and contine with the enumeration
            return DIENUM_CONTINUE;

        // Inspect the results
        for( UINT i=0; i < diaf.dwNumActions; i++ )
        {
            DIACTION * dia = &(diaf.rgoAction[i]);

            if( dia->dwHow != DIAH_ERROR && dia->dwHow != DIAH_UNMAPPED )
                
				DIDevice[numDevices].bMapped[dia->uAppData] = TRUE;
        }

        // Set the action map
        if(hr = lpdid->SetActionMap( &diaf, NULL, DIDSAM_DEFAULT ))
        {
            // An error occured while trying the set the action map for the 
            // current device. Clear the stored values, and continue to the
            // next device.
            ZeroMemory( DIDevice[numDevices].bMapped, 
                 sizeof(DIDevice[numDevices].bMapped) );
            return DIENUM_CONTINUE;
        }

		//DumpActions();

        // The current device has been successfully mapped. By storing the
        // pointer and informing COM that we've added a reference to the 
        // device, we can use this pointer later when gathering input.
        DIDevice[numDevices].pDevice = lpdid;
        lpdid->AddRef();
		
		//name
		DIDevice[numDevices].szName= lpddi->tszInstanceName;
    
        // Store axis absolute/relative flag
        DIPROPDWORD dipdw;  
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD); 
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        dipdw.diph.dwObj        = 0; 
        dipdw.diph.dwHow        = DIPH_DEVICE; 
 
        hr = lpdid->GetProperty( DIPROP_AXISMODE, &dipdw.diph ); 
        if (!hr) 
            DIDevice[numDevices].bAxisRelative = ( DIPROPAXISMODE_REL == dipdw.dwData );

         numDevices++;    // Increment the global device index   
    }
	
    // Ask for the next device
    return DIENUM_CONTINUE;
}

void CGameInput::React(graphic::CGraphic * Graphic){

	int up,down;
	
	ZeroMemory( allKeys, sizeof(DWORD)*NUM_FOR_SEND );
	for(int i=0; i < numDevices;i++){
			up = (int) DIDevice[i].dwInput[eM_CAMVERTICAL];
			down = (int) DIDevice[i].dwInput[eM_CAMHORIZONTAL];
	
			if ( DIDevice[i].dwInput[eM_CAMFORW] )	Graphic->MoveCamera( CM_FORWARD );
			if ( DIDevice[i].dwInput[eM_CAMBACK] )	Graphic->MoveCamera( CM_BACKWARD );
			if ( DIDevice[i].dwInput[eM_CAMVERTICAL] )  	Graphic->RotateCameraM( CR_PITCHUP, (float)up /8 );
			if ( DIDevice[i].dwInput[eM_CAMHORIZONTAL] )	Graphic->RotateCameraM( CR_YAWRIGHT, (float)down /10 );
			
			
			
			if(DIDevice[i].dwInput[eA_STEER]) allKeys[n_STEER]=DIDevice[i].dwInput[eA_STEER];
			if(DIDevice[i].dwInput[eB_STEER_LEFT] || DIDevice[i].dwInput[eB_STEER_RIGHT]) 
				allKeys[n_STEER]=DIDevice[i].dwInput[eA_STEER]= (DWORD) ( (-1)*(int) DIDevice[i].dwInput[eB_STEER_LEFT]) + DIDevice[i].dwInput[eB_STEER_RIGHT];
			
			if(DIDevice[i].dwInput[eA_ACCELERATE]) allKeys[n_ACCELERATE]=DIDevice[i].dwInput[eA_ACCELERATE];
			if(DIDevice[i].dwInput[eB_ACCELERATE]) allKeys[n_ACCELERATE]=DIDevice[i].dwInput[eB_ACCELERATE];
			

			if(DIDevice[i].dwInput[eA_DECELERATE]) allKeys[n_DECELERATE]=DIDevice[i].dwInput[eA_DECELERATE];
			if(DIDevice[i].dwInput[eB_DECELERATE]) allKeys[n_DECELERATE]=DIDevice[i].dwInput[eB_DECELERATE];
			
			if(DIDevice[i].dwInput[eB_BRAKE]) allKeys[n_BRAKE]=DIDevice[i].dwInput[eB_BRAKE];
			if(DIDevice[i].dwInput[eB_UPSHIFT]) allKeys[n_UPSHIFT]=DIDevice[i].dwInput[eB_UPSHIFT];
			if(DIDevice[i].dwInput[eB_DOWNSHIFT]) allKeys[n_DOWNSHIFT]=DIDevice[i].dwInput[eB_DOWNSHIFT];
	}

}
void CGameInput::CheckInput()
{
    HRESULT hr;
	CString test;
	BOOL bTurn;

	// reset KeysToSend[3] - we only need to know change of the state, except handbrake
	KeysToSend[3] &= (char) KEY_HANDBRAKE;
	for ( int i = CUSTOM_KEY_INDEX; i < KEYS_TO_SEND_SIZE; i++ )
		KeysToSend[i] = 0;
    
    for( int iDevice=0; iDevice < numDevices; iDevice++ )
    {
		LPDIRECTINPUTDEVICE8 pdidDevice = DIDevice[iDevice].pDevice;
        DIDEVICEOBJECTDATA rgdod[NUM_OF_ACTIONS];
        DWORD   dwItems = NUM_OF_ACTIONS;

        hr = pdidDevice->Acquire();
        hr = pdidDevice->Poll();
        hr = pdidDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
                                        rgdod, &dwItems, 0 );

        if( hr)  continue;
		
		if( DIDevice[iDevice].bAxisRelative ){
			DIDevice[iDevice].dwInput[eM_CAMHORIZONTAL] = 0;
			DIDevice[iDevice].dwInput[eM_CAMVERTICAL] = 0;
		}
		bTurn = FALSE;
        for( DWORD j=0; j<dwItems; j++ )
        {
            UINT_PTR dwAction = rgdod[j].uAppData;
            DWORD dwData = 0;

			switch( dwAction )
            {
				case eA_STEER:
					KeysToSend[0] = (char) rgdod[j].dwData;
					break;
				case eB_STEER_LEFT:
					left = rgdod[j].dwData;
					/*if((char) rgdod[j].dwData){
						if(bTurn){
							if(KeysToSend[0]) KeysToSend[0] = 0;
							else KeysToSend[0] = -CONTROL_MAX_VALUE;
						}
						else KeysToSend[0] = -CONTROL_MAX_VALUE;
						bTurn=TRUE;
					}
					else{
						if(bTurn){
							if(!KeysToSend[0]) KeysToSend[0] = 0;
						}	
						else KeysToSend[0] = 0;
						bTurn=TRUE;
					}*/
//					
//						
					break;
				case eB_STEER_RIGHT:
					right = rgdod[j].dwData;
					/*if((char) rgdod[j].dwData){
						if(bTurn){
							if(KeysToSend[0]) KeysToSend[0] = 0;
							else KeysToSend[0] = CONTROL_MAX_VALUE;
						}
						else KeysToSend[0] = CONTROL_MAX_VALUE;
						bTurn=TRUE;
					}
					else{
						if(bTurn){
							if(!KeysToSend[0]) KeysToSend[0] = 0;
						}	
						else KeysToSend[0] = 0;
						bTurn=TRUE;
					}*/
					//test.Format("R: %d ",rgdod[j].dwData);
					//	OUTMSG(test,0);
					break;
					
				case eA_ACCELERATE:
					KeysToSend[1] = (char) rgdod[j].dwData;
					break;
				case eB_ACCELERATE:
					if((char) rgdod[j].dwData)
						KeysToSend[1] = CONTROL_MAX_VALUE;
					else
						KeysToSend[1] = 0;
					break;
				case eA_DECELERATE:
					KeysToSend[2] = (char) rgdod[j].dwData;
					break;
				case eB_DECELERATE:
					if((char) rgdod[j].dwData)
						KeysToSend[2] = CONTROL_MAX_VALUE;
					else
						KeysToSend[2] = 0;
					break;
				case eA_UPSHIFT:
				case eB_UPSHIFT:
					if((char) rgdod[j].dwData)
						KeysToSend[3] |= (char) KEY_UPSHIFT;
					break;
				case eA_DOWNSHIFT:
				case eB_DOWNSHIFT:
					if((char) rgdod[j].dwData)
						KeysToSend[3] |= (char) KEY_DOWNSHIFT;
					break;
				case eA_CYCLEVIEW:
				case eB_CYCLEVIEW:
					Client->ChangeCamera();
					//if((char) rgdod[j].dwData)
					//	KeysToSend[3] |= (char) KEY_CYCLEVIEW;
					break;
				case eB_DRIVERVIEW:
					if((char) rgdod[j].dwData)
						KeysToSend[3] |= (char) KEY_DRIVERVIEW;
					break;
				case eB_BRAKE:
					if((char) rgdod[j].dwData)
 						KeysToSend[3] |= (char) KEY_HANDBRAKE;
					else
						KeysToSend[3] &= (char) !KEY_HANDBRAKE;
					break;
				case eB_GAMEPAUSE:
					if((char) rgdod[j].dwData)
						KeysToSend[3] |= (char) KEY_GAMEPAUSE;
					break;
				case eB_RESTART:
					if((char) rgdod[j].dwData)
						KeysToSend[3] |= (char) KEY_RESTART;
					break;
				case eC_ACTION1:
					if ( (char) rgdod[j].dwData )
						KeysToSend[CUSTOM_KEY_INDEX] |= (char) KEY_MODCUSTOM1;
					break;
				case eC_ACTION2:
					if ( (char) rgdod[j].dwData )
						KeysToSend[CUSTOM_KEY_INDEX] |= (char) KEY_MODCUSTOM2;
					break;
				case eC_ACTION3:
					if ( (char) rgdod[j].dwData )
						KeysToSend[CUSTOM_KEY_INDEX] |= (char) KEY_MODCUSTOM3;
					break;
				case eC_ACTION4:
					if ( (char) rgdod[j].dwData )
						KeysToSend[CUSTOM_KEY_INDEX] |= (char) KEY_MODCUSTOM4;
					break;
				case eC_ACTION5:
					if ( (char) rgdod[j].dwData )
						KeysToSend[CUSTOM_KEY_INDEX] |= (char) KEY_MODCUSTOM5;
					break;
				case eC_ACTION6:
					if ( (char) rgdod[j].dwData )
						KeysToSend[CUSTOM_KEY_INDEX] |= (char) KEY_MODCUSTOM6;
					break;
				case eC_ACTION7:
					if ( (char) rgdod[j].dwData )
						KeysToSend[CUSTOM_KEY_INDEX] |= (char) KEY_MODCUSTOM7;
					break;
				case eC_ACTION8:
					if ( (char) rgdod[j].dwData )
						KeysToSend[CUSTOM_KEY_INDEX] |= (char) KEY_MODCUSTOM8;
					break;
				default:
					break;
			}

			switch( dwAction )
			{
				case eM_CAMHORIZONTAL:  
                    dwData = rgdod[j].dwData;   
                    DIDevice[iDevice].dwInput[dwAction] += dwData;
		            break;
				case eM_CAMVERTICAL:
					dwData = rgdod[j].dwData;   
                    DIDevice[iDevice].dwInput[dwAction] += dwData;
					break; 
				default:
					dwData = rgdod[j].dwData & BUTTON_DOWN ?    1 : 0;
				    DIDevice[iDevice].dwInput[dwAction] = dwData;
					break;
			}
			
         
        }
    }
	if(left && right)
		KeysToSend[0] = 0;
	else if(left)
		KeysToSend[0] = -CONTROL_MAX_VALUE;
	else if(right)
		KeysToSend[0]=CONTROL_MAX_VALUE;
	else
		KeysToSend[0]=0;
	
}

//////////////////////////////////////////////////////////////////////////////
//
// Reloads new action mapping from configuration
// pMODKeys parameter CAN point to a list of key codes defined by game MOD,
//   but can be NULL if these codes are not specified.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CGameInput::RefreshActionMapping( CConfig * Configuration, CGameMod::INT_LIST * pMODKeys )
{
	HRESULT hr;
	int i;
	DWORD Key;
	for (i = 0; i < NUM_OF_ACTIONS; i++)
	{
		Key = actionsMap[i].dwSemantic;
		if (actionsMap[i].uAppData == eB_STEER_LEFT) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeySteerLeft);
		if (actionsMap[i].uAppData == eB_STEER_RIGHT) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeySteerRight);
		if (actionsMap[i].uAppData == eB_ACCELERATE) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeyAccelerate);
		if (actionsMap[i].uAppData == eB_DECELERATE) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeyDecelerate);
		if (actionsMap[i].uAppData == eB_BRAKE) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeyBrake);
		if (actionsMap[i].uAppData == eB_UPSHIFT) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeyShiftUp);
		if (actionsMap[i].uAppData == eB_DOWNSHIFT) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeyShiftDown);
		if (actionsMap[i].uAppData == eB_CYCLEVIEW) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeyChangeView);
		if (actionsMap[i].uAppData == eB_DRIVERVIEW) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeyDriverView);
		if (actionsMap[i].uAppData == eB_GAMEPAUSE) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeyPause);
		if (actionsMap[i].uAppData == eB_RESTART) Key = MapKeyFromWindowsIntoDirectInput(Configuration->KeyRestart);
		
		if ( actionsMap[i].uAppData == eC_ACTION1 ) Key = GetMapKeySafe( pMODKeys, 0, i );
		if ( actionsMap[i].uAppData == eC_ACTION2 ) Key = GetMapKeySafe( pMODKeys, 1, i );
		if ( actionsMap[i].uAppData == eC_ACTION3 ) Key = GetMapKeySafe( pMODKeys, 2, i );
		if ( actionsMap[i].uAppData == eC_ACTION4 ) Key = GetMapKeySafe( pMODKeys, 3, i );
		if ( actionsMap[i].uAppData == eC_ACTION5 ) Key = GetMapKeySafe( pMODKeys, 4, i );
		if ( actionsMap[i].uAppData == eC_ACTION6 ) Key = GetMapKeySafe( pMODKeys, 5, i );
		if ( actionsMap[i].uAppData == eC_ACTION7 ) Key = GetMapKeySafe( pMODKeys, 6, i );
		if ( actionsMap[i].uAppData == eC_ACTION8 ) Key = GetMapKeySafe( pMODKeys, 7, i );

		actionsMap[i].dwSemantic = Key;
	}

	for (i = 0; i < numDevices; i++)
	{
		hr = DIDevice[i].pDevice->Unacquire();
		SAFE_RELEASE(DIDevice[i].pDevice);
	}

	numDevices = 0;

	if( pDI->EnumDevicesBySemantics( NULL, &diaf,StaticEnumDevicesCallback,this, DIEDBSFL_ATTACHEDONLY )){
		ERRORMSG(ERRGENERIC,"CGameInput::RefreshActionMapping()","Enum devices failed");
	}


	/*
	for (i = 0; i < numDevices; i++)
	{
		hr = DIDevice[i].pDevice->Unacquire();
		hr = DIDevice[i].pDevice->BuildActionMap(&diaf, NULL, 0);
		hr = DIDevice[i].pDevice->SetActionMap(&diaf, NULL, DIDSAM_DEFAULT);
			if (FAILED(hr) ) ERRORMSG(hr, "CGameInput::RefreshActionMapping()", "Cannot set action map for some input device.");
	
		hr = DIDevice[i].pDevice->Acquire();
	}
	*/
	

	return 0;
};

DWORD CGameInput::GetMapKeySafe( CGameMod::INT_LIST * pList, UINT keyIndex, UINT mapIndex )
{
	if ( pList && pList->size() > keyIndex ) 
		return MapKeyFromWindowsIntoDirectInput( (*pList)[keyIndex] );
	else return actionsMap[mapIndex].dwSemantic;
};



// Maps windows virtual key into DirectInput virtual key
DWORD CGameInput::MapKeyFromWindowsIntoDirectInput(int Key)
{
	switch (Key)
	{
	case VK_BACK: return DIKEYBOARD_BACK;
	case VK_TAB: return DIKEYBOARD_TAB;
	case VK_RETURN: return DIKEYBOARD_RETURN;
	case VK_CAPITAL: return DIKEYBOARD_CAPITAL;
	case VK_SPACE: return DIKEYBOARD_SPACE;		
	case VK_PRIOR: return DIKEYBOARD_PRIOR;
	case VK_NEXT: return DIKEYBOARD_NEXT;
	case VK_END: return DIKEYBOARD_END;
	case VK_HOME: return DIKEYBOARD_HOME;
	case VK_LEFT: return DIKEYBOARD_LEFT;
	case VK_UP: return DIKEYBOARD_UP;
	case VK_RIGHT: return DIKEYBOARD_RIGHT;
	case VK_DOWN: return DIKEYBOARD_DOWN;
	case VK_INSERT: return DIKEYBOARD_INSERT;
	case VK_DELETE: return DIKEYBOARD_DELETE;
	case VK_MULTIPLY: return DIKEYBOARD_MULTIPLY;
	case VK_ADD: return DIKEYBOARD_ADD;
	case VK_SEPARATOR: return DIKEYBOARD_NUMPADENTER; // To by me zajimalo, co ten separator je za klavesu??? - zatim zkousim numpad enter
	case VK_SUBTRACT: return DIKEYBOARD_SUBTRACT;
	case VK_DECIMAL: return DIKEYBOARD_NUMPADCOMMA;
	case VK_DIVIDE: return DIKEYBOARD_DIVIDE;
	case VK_NUMLOCK: return DIKEYBOARD_NUMLOCK;
	case VK_SCROLL: return DIKEYBOARD_SCROLL;
	case VK_OEM_1: return DIKEYBOARD_SEMICOLON;
	case VK_OEM_PLUS: return DIKEYBOARD_EQUALS;
	case VK_OEM_COMMA: return DIKEYBOARD_COMMA;
	case VK_OEM_MINUS: return DIKEYBOARD_MINUS;
	case VK_OEM_PERIOD: return DIKEYBOARD_PERIOD;
	case VK_OEM_2: return DIKEYBOARD_SLASH;
	case VK_OEM_3: return DIKEYBOARD_GRAVE;
	case VK_OEM_4: return DIKEYBOARD_LBRACKET;
	case VK_OEM_5: return DIKEYBOARD_BACKSLASH;
	case VK_OEM_6: return DIKEYBOARD_RBRACKET;
	case VK_OEM_7: return DIKEYBOARD_APOSTROPHE;
	// numbers
	case 0x30: return DIKEYBOARD_0;
	case 0x31: return DIKEYBOARD_1;
	case 0x32: return DIKEYBOARD_2;
	case 0x33: return DIKEYBOARD_3;
	case 0x34: return DIKEYBOARD_4;
	case 0x35: return DIKEYBOARD_5;
	case 0x36: return DIKEYBOARD_6;
	case 0x37: return DIKEYBOARD_7;
	case 0x38: return DIKEYBOARD_8;
	case 0x39: return DIKEYBOARD_9;
	// alphabet
	case 0x41: return DIKEYBOARD_A;
	case 0x42: return DIKEYBOARD_B;
	case 0x43: return DIKEYBOARD_C;
	case 0x44: return DIKEYBOARD_D;
	case 0x45: return DIKEYBOARD_E;
	case 0x46: return DIKEYBOARD_F;
	case 0x47: return DIKEYBOARD_G;
	case 0x48: return DIKEYBOARD_H;
	case 0x49: return DIKEYBOARD_I;
	case 0x4A: return DIKEYBOARD_J;
	case 0x4B: return DIKEYBOARD_K;
	case 0x4C: return DIKEYBOARD_L;
	case 0x4D: return DIKEYBOARD_M;
	case 0x4E: return DIKEYBOARD_N;
	case 0x4F: return DIKEYBOARD_O;
	case 0x50: return DIKEYBOARD_P;
	case 0x51: return DIKEYBOARD_Q;
	case 0x52: return DIKEYBOARD_R;
	case 0x53: return DIKEYBOARD_S;
	case 0x54: return DIKEYBOARD_T;
	case 0x55: return DIKEYBOARD_U;
	case 0x56: return DIKEYBOARD_V;
	case 0x57: return DIKEYBOARD_W;
	case 0x58: return DIKEYBOARD_X;
	case 0x59: return DIKEYBOARD_Y;
	case 0x5A: return DIKEYBOARD_Z;
	// Numpad keys
	case VK_NUMPAD0: return DIKEYBOARD_NUMPAD0;
	case VK_NUMPAD1: return DIKEYBOARD_NUMPAD1;
	case VK_NUMPAD2: return DIKEYBOARD_NUMPAD2;
	case VK_NUMPAD3: return DIKEYBOARD_NUMPAD3;
	case VK_NUMPAD4: return DIKEYBOARD_NUMPAD4;
	case VK_NUMPAD5: return DIKEYBOARD_NUMPAD5;
	case VK_NUMPAD6: return DIKEYBOARD_NUMPAD6;
	case VK_NUMPAD7: return DIKEYBOARD_NUMPAD7;
	case VK_NUMPAD8: return DIKEYBOARD_NUMPAD8;
	case VK_NUMPAD9: return DIKEYBOARD_NUMPAD9;
	// F1 - F24 keys
	case VK_F1: return DIKEYBOARD_F1;
	case VK_F2: return DIKEYBOARD_F2;
	case VK_F3: return DIKEYBOARD_F3;
	case VK_F4: return DIKEYBOARD_F4;
	case VK_F5: return DIKEYBOARD_F5;
	case VK_F6: return DIKEYBOARD_F6;
	case VK_F7: return DIKEYBOARD_F7;
	case VK_F8: return DIKEYBOARD_F8;
	case VK_F9: return DIKEYBOARD_F9;
	case VK_F10: return DIKEYBOARD_F10;
	case VK_F11: return DIKEYBOARD_F11;
	case VK_F12: return DIKEYBOARD_F12;
	case VK_F13: return DIKEYBOARD_F13;
	case VK_F14: return DIKEYBOARD_F14;
	case VK_F15:
	case VK_F16:
	case VK_F17:
	case VK_F18:
	case VK_F19:
	case VK_F20:
	case VK_F21:
	case VK_F22:
	case VK_F23:
	case VK_F24: return DIKEYBOARD_F15; // must be enough
	default: // we hope, that it does not reach this point, but if it reaches interpret key as enter
		return DIKEYBOARD_RETURN;
	}

}

// Dumps actions into file
void CGameInput::DumpActions()
{
	FILE * AcDump;

	AcDump = fopen("AcDump.txt", "w");

	fprintf(AcDump, "DIAH_APPREQUESTED: %i\n", DIAH_APPREQUESTED);
	fprintf(AcDump, "DIAH_DEFAULT: %i\n", DIAH_DEFAULT);
	fprintf(AcDump, "DIAH_ERROR: %i\n", DIAH_ERROR);
	fprintf(AcDump, "DIAH_HWAPP: %i\n", DIAH_HWAPP);
	fprintf(AcDump, "DIAH_HWDEFAULT: %i\n", DIAH_HWDEFAULT);
	fprintf(AcDump, "DIAH_UNMAPPED: %i\n", DIAH_UNMAPPED);
	fprintf(AcDump, "DIAH_USERCONFIG: %i\n", DIAH_USERCONFIG);

	for(int i = 0; i < NUM_OF_ACTIONS; i++)
	{
			fprintf(AcDump, "App data: %i\n", actionsMap[i].uAppData);
			fprintf(AcDump, "Semantic: %i\n", actionsMap[i].dwSemantic);
			fprintf(AcDump, "How: %i\n", actionsMap[i].dwHow);
			fprintf(AcDump, "\n");
	}

	fclose(AcDump);

};
