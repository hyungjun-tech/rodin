#pragma once

#include "MFSetConstraint.h"
#include "ModelContainer.h"

class ManipulatedFrame : public qglviewer::ManipulatedFrame
{
public:
	ManipulatedFrame(ModelContainer *modelContainer_, qglviewer::Camera * camera_ = nullptr);
	virtual ~ManipulatedFrame();

	void restrictTranslationToOXYPlane(qglviewer::Vec position_);
	void restrictTranslationToZAxis();

	void restrictRotationToLocalXAxis();
	void restrictRotationToLocalYAxis();
	void restrictRotationToLocalZAxis();

private:
	MFSetConstraint *worldXYTranslation;
	MFSetConstraint *worldZTranslation;

	MFSetConstraint *localXRotation;
	MFSetConstraint *localYRotation;
	MFSetConstraint *localZRotation;
};