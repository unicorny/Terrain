#ifndef __CAMERA__
#define __CAMERA__

#include "DRCore2/Foundation/DRObject.h"
#include "DRCore2/DRTypes.h"

class Camera : public DRObject
{
public:
	Camera();
	virtual ~Camera() {};

	inline void SetMovementFriction(DRReal moveFriction) { mMoveFriction = moveFriction; }
	inline void SetRotationFriction(DRReal rotationFriction) { mRotationFriction = rotationFriction; }
	inline void SetPosition(DRReal x, DRReal y, DRReal z) { setPosition(DRVector3(x, y, z)); }
	inline const DRVector3& GetPosition() const { return getPosition(); }
	inline const DRMatrix& GetMatrix() { return getMatrix(); }
	inline void RotateRel(const DRVector3& rotation) { rotateRel(rotation); }
	void AddRotationRel(const DRVector3 relRotation);
	void AddVelocityRel(const DRVector3 velocity);
	DRReturn Move(DRReal fTimeSinceLastFrame);
	void Reset();

protected:
	DRVector3 mVelocity;
	DRReal mMoveFriction;
	DRVector3 mRotationSpeed;
	DRReal mRotationFriction;

};

#endif //__CAMERA__