#include "stdafx.h"
#include "ManipulatedFrame.h"

ManipulatedFrame::ManipulatedFrame(ModelContainer *modelContainer_, qglviewer::Camera * camera_)
	: worldXYTranslation(new MFSetWorldConstraint(modelContainer_, camera_)),
	worldZTranslation(new MFSetWorldConstraint(modelContainer_)),
	localXRotation(new MFSetLocalConstraint(modelContainer_)),
	localYRotation(new MFSetLocalConstraint(modelContainer_)),
	localZRotation(new MFSetLocalConstraint(modelContainer_))
{
	// Restricted Translation --------------------------------------------------
	worldXYTranslation->setTranslationConstraint(
		qglviewer::AxisPlaneConstraint::FREE, qglviewer::Vec(0, 0, 1));
	worldXYTranslation->setRotationConstraint(
		qglviewer::AxisPlaneConstraint::FORBIDDEN, qglviewer::Vec());

	worldZTranslation->setTranslationConstraint(
		qglviewer::AxisPlaneConstraint::AXIS, qglviewer::Vec(0, 0, 1));
	worldZTranslation->setRotationConstraint(
		qglviewer::AxisPlaneConstraint::FORBIDDEN, qglviewer::Vec());

	// Restricted Rotation -----------------------------------------------------
	localXRotation->setTranslationConstraint(
		qglviewer::AxisPlaneConstraint::FORBIDDEN, qglviewer::Vec());
	localXRotation->setRotationConstraint(
		qglviewer::AxisPlaneConstraint::AXIS, qglviewer::Vec(1, 0, 0));

	localYRotation->setTranslationConstraint(
		qglviewer::AxisPlaneConstraint::FORBIDDEN, qglviewer::Vec());
	localYRotation->setRotationConstraint(
		qglviewer::AxisPlaneConstraint::AXIS, qglviewer::Vec(0, 1, 0));

	localZRotation->setTranslationConstraint(
		qglviewer::AxisPlaneConstraint::FORBIDDEN, qglviewer::Vec());
	localZRotation->setRotationConstraint(
		qglviewer::AxisPlaneConstraint::AXIS, qglviewer::Vec(0, 0, 1));
}

ManipulatedFrame::~ManipulatedFrame()
{
	delete worldXYTranslation;
	delete worldZTranslation;

	delete localXRotation;
	delete localYRotation;
	delete localZRotation;
}

// World Trans Rotation Constraint -------------------------------------------
void ManipulatedFrame::restrictTranslationToOXYPlane(qglviewer::Vec position_)
{
	worldXYTranslation->savedPosition = position_;
	setConstraint(worldXYTranslation);
}

void ManipulatedFrame::restrictTranslationToZAxis()
{
	setConstraint(worldZTranslation);
}

// Local Rotation Constraint -------------------------------------------
void ManipulatedFrame::restrictRotationToLocalXAxis()
{
	setConstraint(localXRotation);
}

void ManipulatedFrame::restrictRotationToLocalYAxis()
{
	setConstraint(localYRotation);
}

void ManipulatedFrame::restrictRotationToLocalZAxis()
{
	setConstraint(localZRotation);
}