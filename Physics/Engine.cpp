#include "Engine.h"

using namespace physics;

//////////////////////////////////////////////////////////////////////////////////////////////
//
// class destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CEngine::~CEngine()
{
	Release();
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// class initialization
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CEngine::Init(InitEngine *initEngine, CMaterials *materials)
{
	m_materials = materials;

	// call reset
	Reset();

	m_throttle = 0.0f;
	m_engineRpm = MIN_RPM;
	averageRpm = 0.0f;
	averageWheelRpm = 0.0f;

	m_currGear = physics::GEAR_NEUTRAL; // neutral
	m_burningOut = BURN_AUTO;	// automatic launch

	// init gears ratios
	gears[0] = initEngine->gears[0];		// rear gear
	gears[1] = 0.0f;						// neutral gear
	for(int i=2; i<MAX_GEARS; i++)
		gears[i] = initEngine->gears[i-1];	// the rest of gears

	// init other vals
	diff_ratio = initEngine->diff_ratio;
	efficiency = initEngine->efficiency;
	

	//#define NUM_CURVE 8
	//numTrCurve = NUM_CURVE;
	//float torques[NUM_CURVE] = {0.0f, 400.0f, 430.0f, 450.0f, 470.0f, 480.0f, 490.0f, 0.0f};
	//float torques[NUM_CURVE] = {0.0f, 430.0f, 460.0f, 480.0f, 500.0f, 490.0f, 420.0f, 0.0f}; // original
	//float torques[NUM_CURVE] = {0.0f, 500.0f, 510.0f, 530.0f, 550.0f, 540.0f, 530.0f, 0.0f};
	//float torques[NUM_CURVE] = {0.0f, 210.0f, 250.0f, 300.0f, 450.0f, 480.0f, 420.0f, 0.0f};
	//float torques[NUM_CURVE] = {0.0f, 215.0f, 230.0f, 240.0f, 250.0f, 245.0f, 210.0f, 0.0f};
	//float torques[NUM_CURVE] = {0.0f, 43.0f, 46.0f, 48.0f, 50.0f, 49.0f, 42.0f, 0.0f};
	//float torques[NUM_CURVE] = {0.0f, 86.0f, 92.0f, 96.0f, 100.0f, 98.0f, 84.0f, 0.0f};
	//float rpms[NUM_CURVE] = {0.0f, 1000.0f, 2000.0f, 3000.0f, 4000.0f, 5000.0f, 6000.0f, 6001.0f};

	numTrCurve = initEngine->numCurve;
	SAFE_NEW_ARRAY(trCurve, TorqueRpmCurve, numTrCurve, "CEngine::Init()");
	SAFE_NEW_ARRAY(torqueDiff, float, numTrCurve-1, "CEngine::Init()");
	for(int i=0; i<numTrCurve; i++)
	{
		trCurve[i].rpm = initEngine->rpms[i];//(i+1)*1000;
		trCurve[i].torque = initEngine->torques[i];
		if(i>0)
			torqueDiff[i-1] = (trCurve[i].torque - trCurve[i-1].torque)/(trCurve[i].rpm - trCurve[i-1].rpm);
	}

	// find optimal RPMs for utomatic transmission
	FindBestEfficientRPM ();

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// engine reset
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CEngine::Reset()
{
	m_throttle = 0.0f;
	m_engineRpm = MIN_RPM;;
	//averageRpm = 0.0f;
	//averageWheelRpm = 0.0f;
	
	m_currGear = physics::GEAR_NEUTRAL;
	m_burningOut = BURN_AUTO;	// automatic launch
	clutchState = CLUTCH_NOACTION;

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// release resources
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CEngine::Release()
{
	//SAFE_DELETE_ARRAY(gears);

	SAFE_DELETE_ARRAY(trCurve);

	SAFE_DELETE_ARRAY(torqueDiff);
};

void
CEngine::FindBestEfficientRPM (void)
{
	float torque = 0;

	m_hiRPM = 0;
	m_lowRPM = 0;

	// go across trCurve and find RPM with maximal torque
	for (int i=0; i < numTrCurve; i++)
	{
		if (trCurve[i].torque > torque)
			m_hiRPM = trCurve[i].rpm;
	}

#define OPTIMAL_RPM_WIDHT 2500
	// low limit will be OPTIMAL_RPM_WIDHT lower
	if (m_hiRPM > OPTIMAL_RPM_WIDHT + MIN_RPM)
		m_lowRPM = m_hiRPM - OPTIMAL_RPM_WIDHT;
	else
		m_lowRPM = m_hiRPM - MIN_RPM;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
// engine update method
// clutchRatio: 1=clutchFullyPushed, 0=clutchFullyOpened
//////////////////////////////////////////////////////////////////////////////////////////////
void CEngine::Update(float wheelRPM, float clutchRatio)
{
	// get the RPMs according throttle
	float throttleRPMs = MIN_RPM + (m_throttle * (MAX_RPM - MIN_RPM));

	// now get the RPM based on wheelRPMs & differetial & gear ratio - simultation of gearBox with clutch
	// setup engine RPM based on , and wheel omega speed
	float gearboxRPMs = wheelRPM * diff_ratio * gears[(int)m_currGear];

	// now interpolate (linear) between them using clutchRatio to get real RPMs
	m_engineRpm = (throttleRPMs*clutchRatio) + (gearboxRPMs*(1-clutchRatio));

	// check if engine's rpms lowered under MIN_RPM
	if (m_engineRpm < MIN_RPM)
	{
		m_engineRpm = MIN_RPM;
		// start pushing clutch
		clutchState = CLUTCH_PUSHING;
	}

	// check if engine's rpms increase above limit
	if (m_engineRpm > MAX_RPM)
	{
		m_engineRpm = MAX_RPM;
	}

	// count current torque based on current shifted gear
	switch (m_currGear)
	{
	case GEAR_BACKWRD:
		m_currTorque = -m_throttle * GetTorqueAccordingRPM (m_engineRpm);
		break;

	case GEAR_NEUTRAL:
		m_currTorque = 0.0f;
		break;

	default:		
		m_currTorque = m_throttle * GetTorqueAccordingRPM (m_engineRpm);
		break;
	}	
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// finds torque for current rpm - linear interpolation
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CEngine::GetTorqueAccordingRPM(float rpm)
{
	float tousandsOfRPM = m_engineRpm / 1000.0f;
	int index = (int) tousandsOfRPM;
	float diff = 1 - (tousandsOfRPM - index);

	float retval;

	switch (index)
	{
	case 6:	//6000->rpm
		retval = (trCurve[index].torque * diff);
		break;

	default:	// linear interpolate between index and index+1
		retval = (trCurve[index].torque*diff) + (trCurve[index+1].torque*(1-diff));
		break;
	}
#define MAGIC 3	// nevim proc a nechci to dohledavat, ale na tocak 300NM to proste nejede..:)
	// takze to pronasobuju MAGIC

	// incount engine efficiency from mutator
	return retval * m_materials->GetCurrentMutator()->engineEfficiecy * MAGIC;
};

// this code is no longer used
#if 0
float CEngine::Update(float _throttle, float _rpm, float speed, int direction)
{
#ifdef __COMMENTED__
	float lowRpm = trCurve[0].rpm;
	float highRpm = trCurve[numTrCurve-1].rpm;
	float torque = 0.0f;
	//float rpm1 = rpm;

	//if(gear == 1)
	//{
	//	if(throttle > 0)
	//		gear++;
	//}
	//else
	//{
	//	if(rpm <= lowRpm)
	//		if(gear == 2)
	//			gear--;
	//}

	//if(abs(rpm - averageRpm) > 0.3f  * averageRpm)
	//	rpm = averageRpm;

	float newRpm = vel * 30.5472672025f;

	//averageRpm = (averageRpm * RPM_COUNT - averageRpm + rpm ) / RPM_COUNT;
	averageRpm = (averageRpm * RPM_COUNT - averageRpm + newRpm ) / RPM_COUNT;

	float rpm1 = averageRpm;
	rpm = averageRpm * diff_ratio * gears[gear];

	//if(rpm > 9000.f)
	//	return 0.0f;
	if(gear == 1)
		if(throttle > 0)
			gear++;
	
	//if(gear == 1)
	//{
	//	if(throttle > 0)
	//		if(direction) // direction > 0 means forward
	//			gear++;
	//		else
	//			gear--;
	//}
	//else
	//{
	//	if(rpm <= lowRpm)
	//		if(gear == 2)
	//			gear--;
	//		else
	//			if(gear == 0)
	//				gear++;
	//			else
	//				gear--;
	//	if(rpm >= highRpm)
	//		if(gear != 0)
	//			if(gear+1 != numGears)
	//				gear++;
	//	//torque = throttle * GetTorqueAccordingRPM(rpm);
	//}
	int cE = 0;
	if(rpm > 0.0f && rpm < 1000.0f)
		cE = 0;
	if(rpm > 1000.0f && rpm < 2000.0f)
		cE = 1;
	if(rpm > 2000.0f && rpm < 3000.0f)
		cE = 2;
	if(rpm > 3000.0f && rpm < 4000.0f)
		cE = 3;
	if(rpm > 4000.0f && rpm < 5000.0f)
		cE = 4;
	if(rpm > 5000.0f && rpm < 6000.0f)
		cE = 5;
	if(rpm > 6000.0f && rpm < 8000.0f)
		cE = 6;
	if(gear < 3)
		if(rpm > 7000.0f)
		{
			gear++;
			return 0;
		}
	if(gear < 5)
		if(rpm > 6000.0f)
		{
			gear++;
			return 0;
		}
	if(gear < 6)
		if(rpm > 4300.0f)
		{
			gear++;
			return 0;
		}
	if(gear > 2)
		if(rpm < 1000.0f)
		{
			gear--;
			return 0;
		}
	//if(gear == 2 || gear == 1)
	//	if(rpm < 500.0f)
	//	{
	//		gear--;
	//		return 0;
	//	}

	torque = throttle * diff_ratio * gears[gear] * (trCurve[cE].torque + torqueDiff[cE] * (rpm - trCurve[cE].rpm));	

	//torque = throttle * diff_ratio * gears[gear] * GetTorqueAccordingRPM(rpm);

	

	//CAtlString msg;
	//msg.Format("Velocity: %5.1f SetThrottle: %5.1f Gear: %5d rpm: %10.2f WheelRpm: %10.2f NewRpm: %10.2f Torque: %10.2f\n", 3.6f*vel, throttle, gear, rpm, rpm1, newRpm, torque);
	//OUTMSG(msg, 1);
#endif

	float torque = 0.0f;

#define RADIUS 0.23445542f

	float __rpm = (speed * 60.0f) / (RADIUS * 2 * D3DX_PI);
	float rpm = __rpm * diff_ratio * gears[gear];
	_rpm *= -1 * 60.0f / (2 * D3DX_PI);// * diff_ratio * gears[gear];

	if(LIES_AROUND(speed, 0.0f, 0.007f))
		gear = 1;
	if(gear == 1)
		if(throttle > 0)
			gear=2;
	if(gear == 2)
		if(rpm > 3000.0f)
			gear=3;
	if(gear == 3)
		if(rpm < 500.0f)
			gear=2;
	if(gear == 1)
		if(throttle < 0)
			gear=0;

	torque = (throttle/CONTROL_MAX_VALUE)*diff_ratio*gears[gear]*GetTorqueAccordingRPM(rpm);
	
	CAtlString msg;
	//msg.Format("Rpm<%10.2f> _Rpm<%10.2f> Torque<%10.2f>", __rpm, _rpm, torque);
	OUTMSG(msg, 1);

	return torque;
};

void CEngine::Update(float _rpm0, float long0, float _rpm1, float long1)
{
	float rpm0, rpm1;

	_rpm0 = (_rpm0 * 30) / D3DX_PI; // 60 * (omega / (2 * PI))
	_rpm1 = (_rpm1 * 30) / D3DX_PI;

	rpm = _rpm0;

	engineRpm = rpm0 = _rpm0 * diff_ratio * gears[gear];
	rpm1 = _rpm1 * diff_ratio * gears[gear];

	if(engineRpm > trCurve[numTrCurve-1].rpm)
		engineRpm = trCurve[numTrCurve-1].rpm;
	averageRpm = (averageRpm * RPM_COUNT - averageRpm + engineRpm) / RPM_COUNT;
	UINT _t = (int)averageRpm / 1000;
	CAtlString msg;
	for(UINT i=0; i<_t; i++)
		msg += "#";
	OUTMSG(msg,1);


	if((_rpm0 > 1.0f) && (gear < 1))
		gear = 1;
	if((_rpm0 < -1.0f) && (gear > 1))
		gear = 1;
	if((throttle < 0.001f) && (_rpm0 < 0.5f) && (gear > 1))
		gear = 1;
	if((throttle < 0.001f) && (_rpm0 > -0.5f) && (gear < 1))
		gear = 1;
	//if((throttle == 0.0f) && LIES_AROUND(_rpm0, 0.0f, 0.5f) && gear != 1)
	//	gear = 1;

	
	torque0 = throttle*diff_ratio*gears[gear]*GetTorqueAccordingRPM(abs(rpm0));
	torque1 = throttle*diff_ratio*gears[gear]*GetTorqueAccordingRPM(abs(rpm1));

	msg.Format("SetThrottle[%5.2f]", throttle);
	//OUTMSG(msg,1);
};
#endif