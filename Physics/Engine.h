//////////////////////////////////////////////////////////////////////////////////////////////
//
// CEngine
//
// TODO: complete this ...
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "InitStructures.h"
#include "Materials.h"

namespace physics
{
#define RPM_COUNT 10

#define MIN_RPM 800	// volnobeh
#define MAX_RPM 7000// omezovac

#define MAX_GEARS 8
	enum eGears {
		GEAR_BACKWRD,
		GEAR_NEUTRAL,
		GEAR_1,
		GEAR_2,
		GEAR_3,
		GEAR_4,
		GEAR_5,
		GEAR_6,
	};
	inline eGears operator++ (eGears &val, int) {
		val = (eGears)( (int) val + 1);
		return val;
	}
	inline eGears operator-- (eGears &val, int) {
		val = (eGears)( (int) val - 1);
		return val;
	}

	enum eBurnOut {
		BURN_NONE,
		BURN_AUTO,
		BURN_MANUAL
	};

	enum eClutchState {
		CLUTCH_PUSHING,
		CLUTCH_UNPUSHING,
		CLUTCH_NOACTION
	};

	class CEngine
	{
	// CEngine structures
	struct TorqueRpmCurve
	{
		float torque;
		float rpm;
	};

	// CEngine properties
	public:
		float m_engineRpm;
		float averageRpm;
		float averageWheelRpm;

		float m_hiRPM, m_lowRPM;
		enum eBurnOut m_burningOut;	// hrabacky z neutralu

		eClutchState clutchState;
	
	private:
		enum eGears m_currGear;
		float gears[MAX_GEARS];

		float diff_ratio;
		float efficiency;
		float m_throttle;

		float m_currTorque;

		CMaterials *m_materials;

		int numTrCurve;
		TorqueRpmCurve * trCurve;
		float * torqueDiff;

	// CEngine methods
	public:
		~CEngine();
		CEngine() {}

		inline float GetCurrRPM (void) { return m_engineRpm; }
		inline float GetCurrTorque (void) { return m_currTorque; }

		HRESULT Init(InitEngine * initEngine, CMaterials *materials);

		HRESULT Reset();

		void Release();

		void Update (float wheelRPM, float clutchRatio);

		enum eGears GetGear() { return m_currGear; };

		inline void SetThrottle (float throttle) { m_throttle = throttle; }
		inline float GetThrottle (void) { return m_throttle; }

		inline void ShiftUp() { m_currGear++; }
		inline void ShiftDown() { m_currGear--; }
		inline void ShiftGear (enum eGears g) { m_currGear = g; }

	private:
		float GetTorqueAccordingRPM(float rpm);
		
		void FindBestEfficientRPM (void);
	};
}; // namespace physics
