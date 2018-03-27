//********************************************************************
// Newton Game dynamics 
// copyright 2000-2004
// By Julio Jerez
// VC: 6.0
// simple demo list vector class with iterators
//********************************************************************

// CustomUpVector.cpp: implementation of the CustomUpVector class.
//
//////////////////////////////////////////////////////////////////////

#include "DryRollingFriction.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


DryRollingFriction::DryRollingFriction(NewtonBody* child, dFloat radius, dFloat coeficient)
	:NewtonCustomJoint(1, child, NULL)
{
	dFloat mass;
	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;

	NewtonBodyGetMassMatrix (child, &mass, &Ixx, &Iyy, &Izz);

	m_frictionCoef = coeficient;
	m_frictionTorque = Ixx * radius;
}


DryRollingFriction::~DryRollingFriction()
{
}


// rolling friction works as follow: the idealization of the contact of a spherical object 
// with a nother surface is a point that pass by the center of the sphere.
// in most cases this is enought to model the collision but in insuficent for modelling 
// the rolling friction. In reality contact with the sphere with the other surface is not 
// a point but a contact patch. A contact patch has the property the it generates a fix 
// constant rolling torque that oposest the movement of the sphere.
// we can model this torque by adding a clamped torque aligned to the instanstaneus axis 
// of rotation of the ball. and with a magtitud of the stopping angular accelration.
void DryRollingFriction::SubmitConstrainst ()
{
	dVector omega;
	dFloat time;
	dFloat omegaMag;
	dFloat torqueFriction;

	// get the omega vector
	NewtonBodyGetOmega(m_body0, &omega[0]);

	omegaMag = sqrtf (omega % omega);
	if (omegaMag > 0.1f) {
		// tell newton to used this the driction of the omega vector to apply the rolling friction
		dVector pin (omega.Scale (1.0f / omegaMag));
		NewtonUserJointAddAngularRow (m_joint, 0.0f, &pin[0]);

		// calculate the acceleration to stop the ball in one time step
		time = NewtonGetTimeStep (m_world);
		NewtonUserJointSetRowAcceleration (m_joint, -omegaMag / time);

		// set the friction limit proportinal the sphere Inertia
		torqueFriction = m_frictionTorque * m_frictionCoef;
		NewtonUserJointSetRowMinimunFriction (m_joint, -torqueFriction);
		NewtonUserJointSetRowMaximunFriction (m_joint, torqueFriction);

	} else {
		// when omega is too low sheat a litle bit and damp the omega directly
		omega = omega.Scale (0.2f);
		NewtonBodySetOmega(m_body0, &omega[0]);
	}
}




