#include "Camera.h"

Camera::Camera()
	: mMoveFriction(1.0), mRotationFriction(1.0)
{

}

void Camera::Reset()
{
	// set speed to zero
	mVelocity = DRVector3();
	mRotationSpeed = DRVector3();
}

void Camera::AddRotationRel(const DRVector3 relRotation)
{
	mRotationSpeed += relRotation;
}

void Camera::AddVelocityRel(const DRVector3 velocity)
{
	mVelocity += velocity;
}

DRReturn Camera::Move(DRReal fTimeSinceLastFrame)
{
	translateRel(mVelocity * fTimeSinceLastFrame);
	rotateRel(mRotationSpeed * fTimeSinceLastFrame);

	if(mMoveFriction != 1.0) {
		mVelocity *= pow(mMoveFriction, fTimeSinceLastFrame);
	}
	if (mRotationFriction != 1.0f) {
		mRotationSpeed *= pow(mRotationFriction, fTimeSinceLastFrame);
	}
	return DR_OK;
}