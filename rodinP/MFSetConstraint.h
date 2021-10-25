#pragma once

#include "ModelContainer.h"
#include "MeshModel.h"

class MFSetConstraint : public qglviewer::AxisPlaneConstraint
{
public:
	MFSetConstraint(ModelContainer *modelContainer_, qglviewer::Camera * camera_ = nullptr);
	~MFSetConstraint();

	virtual void constrainTranslation(qglviewer::Vec &translation,
		qglviewer::Frame *const frame);
	virtual void constrainRotation(qglviewer::Quaternion &rotation,
		qglviewer::Frame *const frame);

	void setTranslationConstraint(qglviewer::AxisPlaneConstraint::Type type_,
		const qglviewer::Vec& direction_);
	void setRotationConstraint(qglviewer::AxisPlaneConstraint::Type type_,
		const qglviewer::Vec& direction_);

	qglviewer::Vec savedPosition;
protected:
	qglviewer::AxisPlaneConstraint *constraint;
	ModelContainer *modelContainer;
	qglviewer::Camera* camera;

	void moveModels(std::vector<IMeshModel*> models_, qglviewer::Vec translation_);

	void rotateModels(std::vector<IMeshModel*> models_,
		qglviewer::Vec worldAxis_, qglviewer::Vec pos_, float angle_);
};

class MFSetLocalConstraint : public MFSetConstraint
{
public:
	MFSetLocalConstraint(ModelContainer *modelContainer_, qglviewer::Camera * camera_ = nullptr);
	~MFSetLocalConstraint();
};

class MFSetWorldConstraint : public MFSetConstraint
{
public:
	MFSetWorldConstraint(ModelContainer *modelContainer_, qglviewer::Camera * camera_ = nullptr);
	~MFSetWorldConstraint();
};