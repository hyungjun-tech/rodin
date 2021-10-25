#include "stdafx.h"
#include "MFSetConstraint.h"

MFSetConstraint::MFSetConstraint(ModelContainer *modelContainer_, qglviewer::Camera * camera_)
	: modelContainer(modelContainer_)
	, camera(camera_)
{

}

MFSetLocalConstraint::MFSetLocalConstraint(ModelContainer *modelContainer_, qglviewer::Camera * camera_)
	: MFSetConstraint(modelContainer_, camera_)
{
	constraint = new qglviewer::LocalConstraint();
}

MFSetWorldConstraint::MFSetWorldConstraint(ModelContainer *modelContainer_, qglviewer::Camera * camera_)
	: MFSetConstraint(modelContainer_, camera_)
{
	constraint = new qglviewer::WorldConstraint();
}

MFSetConstraint::~MFSetConstraint()
{
}

MFSetLocalConstraint::~MFSetLocalConstraint()
{
	delete constraint;
}

MFSetWorldConstraint::~MFSetWorldConstraint()
{
	delete constraint;
}

void MFSetConstraint::moveModels(std::vector<IMeshModel*> models_, qglviewer::Vec translation_)
{
	for (int i = 0; i < models_.size(); i++)
	{
		if (models_[i]->isSelected())
			models_[i]->translate(translation_[0], translation_[1], translation_[2]);
	}
}

void MFSetConstraint::rotateModels(std::vector<IMeshModel*> models_,
	qglviewer::Vec worldAxis_, qglviewer::Vec pos_, float angle_)
{
	for (int i = 0; i < models_.size(); i++) {
		if (!models_[i]->isSelected())
			continue;

		AABB aabb = models_[i]->getAABB();
		qglviewer::Vec center = aabb.getCenter();

		models_[i]->rotateAroundAPoint(
			worldAxis_[0], worldAxis_[1], worldAxis_[2],
			center[0], center[1], center[2],
			angle_);
	}
}

void MFSetConstraint::constrainTranslation(qglviewer::Vec &translation,
	qglviewer::Frame *const frame)
{
	if (camera != nullptr)
	{
		qglviewer::Vec point = camera->projectedCoordinatesOf(savedPosition + translation);
		qglviewer::Vec eyePt(point[0], point[1], 0);
		qglviewer::Vec temp = camera->unprojectedCoordinatesOf(eyePt);
		qglviewer::Vec temp2 = camera->unprojectedCoordinatesOf(qglviewer::Vec(eyePt[0], eyePt[1], 1));
		qglviewer::Vec dir = temp2 - temp;
		dir.normalize();
		qglviewer::Vec pos = temp + (savedPosition[2] - temp[2]) / dir[2] * dir;
		qglviewer::Vec diff = pos - savedPosition;
		diff[2] = 0;
		translation = diff;
	}
	constraint->constrainTranslation(translation, frame);

	float machineWidth = Profile::getMachineWidth_calculated();
	float machineDepth = Profile::getMachineDepth_calculated();
	float bedMinX = Profile::machineProfile.machine_width_offset.value;
	float bedMinY = Profile::machineProfile.machine_depth_offset.value;
	float bedMaxX = machineWidth + bedMinX;
	float bedMaxY = machineDepth + bedMinY;
	AABB aabb = TotalBoxGetter()(modelContainer->getSelectedModels());
	float minX = frame->position().x - aabb.getLengthX() / 2 + translation.x;
	float maxX = frame->position().x + aabb.getLengthX() / 2 + translation.x;
	float minY = frame->position().y - aabb.getLengthY() / 2 + translation.y;
	float maxY = frame->position().y + aabb.getLengthY() / 2 + translation.y;
	if (aabb.getLengthX() <= machineWidth && aabb.getLengthY() <= machineDepth)
	{
		if (minX < bedMinX)
			translation.x = translation.x - (minX - bedMinX);
		if (maxX > bedMaxX)
			translation.x = translation.x - (maxX - bedMaxX);
		if (minY < bedMinY)
			translation.y = translation.y - (minY - bedMinY);
		if (maxY > bedMaxY)
			translation.y = translation.y - (maxY - bedMaxY);
	}
	moveModels(modelContainer->models, translation);
	savedPosition += translation;

	// +Update AABBs of models
}

void MFSetConstraint::constrainRotation(qglviewer::Quaternion &rotation,
	qglviewer::Frame *const frame)
{
	constraint->constrainRotation(rotation, frame);

	// A little bit of math. Easy to understand, hard to guess (tm).
	// rotation is expressed in the frame local coordinates system. Convert it
	// back to world coordinates.
	const qglviewer::Vec worldAxis = frame->inverseTransformOf(rotation.axis());
	const qglviewer::Vec pos = frame->position();
	const float angle = rotation.angle();

	rotateModels(modelContainer->models, worldAxis, pos, angle);
}

void MFSetConstraint::setTranslationConstraint(
	qglviewer::AxisPlaneConstraint::Type type_,
	const qglviewer::Vec& direction)
{
	constraint->setTranslationConstraint(type_, direction);
}
void MFSetConstraint::setRotationConstraint(
	qglviewer::AxisPlaneConstraint::Type type_,
	const qglviewer::Vec& direction)
{
	constraint->setRotationConstraint(type_, direction);
}