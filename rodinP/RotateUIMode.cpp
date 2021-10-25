#include "stdafx.h"
#include "RotateUIMode.h"

#include "PickingIdxManager.h"
#include "CollisionManager.h"
#include "UserProperties.h"
#include "ManipulatorHandle.h"
#include "FBO.h"
#include "PrinterBody.h"

RotateUIMode::RotateUIMode(ViewerModule* engine_)
	: UIMode(engine_)
	, printerBody(engine_->getPrinterBody())
	, fbo(new FBO())
{
}

RotateUIMode::~RotateUIMode()
{
	delete rotationHandles;
	delete fbo;
}

/// QGLViewer Functions
void RotateUIMode::initialize(int width_, int height_)
{
	rotationHandles = new RotationHandles(manipulatedFrame);
	fbo->initialize(width_, height_, engine->devicePixelRatioF());
}

void RotateUIMode::draw()
{
	adjustHandleScale();

	QMatrix4x4 projMat = ProjectionMatrixGetter()(*camera);
	QMatrix4x4 viewMat = ViewMatrixGetter()(*camera);

	drawOffScreenCanvas(projMat, viewMat);
	drawOnScreenCanvas(projMat, viewMat);
}

void RotateUIMode::mousePressEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton && e->button() != Qt::MiddleButton)
		return;

	pressed = e->pos()*engine->devicePixelRatioF();
	if (!onMesh && !onHandleX && !onHandleY && !onHandleZ)
		return;

	engine->setCursor(Qt::ClosedHandCursor);
	if (onHandleX || onHandleY || onHandleZ)
	{
		if (onHandleX)
		{
			rotationHandles->setDrawAxis(0);
			manipulatedFrame->restrictRotationToLocalXAxis();
		}
		else if (onHandleY)
		{
			rotationHandles->setDrawAxis(1);
			manipulatedFrame->restrictRotationToLocalYAxis();
		}
		else if (onHandleZ)
		{
			rotationHandles->setDrawAxis(2);
			manipulatedFrame->restrictRotationToLocalZAxis();
		}

		engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,
			QGLViewer::FRAME, QGLViewer::ROTATE);
	}

	if (onMesh)
	{
		onID = PickingIdxManager::decodeID(fbo->getColor(pressed));
		if (e->modifiers() == Qt::NoModifier)
		{
			if (!modelContainer->isSelected(onID))
				modelContainer->selectExceptOthers(onID);
		}
		else if (e->modifiers() == Qt::ShiftModifier)
		{
			modelContainer->deSelect(onID);
		}
		else if (e->modifiers() == Qt::ControlModifier)
		{
			//modelContainer->Select(onID);
			modelContainer->toggleModelSelection(onID);
		}

		if (modelContainer->hasAnySelectedModel())
			updateHandle();

		if (modelContainer->isSelected(onID))
		{
			qglviewer::Vec eyePt(pressed.x(), pressed.y(), fbo->getDepth(pressed));
			qglviewer::Vec savedPosition = camera->unprojectedCoordinatesOf(qglviewer::Vec(pressed.x(), pressed.y(), fbo->getDepth(pressed)));
			manipulatedFrame->restrictTranslationToOXYPlane(savedPosition);
		}
		else
		{
			onID = -1;
		}

		engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,	QGLViewer::FRAME, QGLViewer::TRANSLATE);
	}
}

void RotateUIMode::mouseMoveEvent(QMouseEvent* e)
{
	bool beforeOnMesh = onMesh;
	int beforeOnID = onID;
	onID = PickingIdxManager::decodeID(fbo->getColor(e->pos()*engine->devicePixelRatioF()));

	onMesh = PickingIdxManager::isMeshID(onID);
	onHandleX = PickingIdxManager::isRotXID(onID);
	onHandleY = PickingIdxManager::isRotYID(onID);
	onHandleZ = PickingIdxManager::isRotZID(onID);

	if (e->buttons() != Qt::LeftButton)
	{
		if (onMesh || onHandleX || onHandleY || onHandleZ)
			engine->setCursor(Qt::OpenHandCursor);
		else
			engine->setCursor(Qt::ArrowCursor);

		if (beforeOnID != onID)
		{
			int modelId = -1;
			int subModelId = -1;
			int beforeModelId = -1;
			int beforeSubModelId = -1;
			if (beforeOnMesh)
				modelContainer->getModelIndex(beforeOnID, beforeModelId, beforeSubModelId);
			if (onMesh)
				modelContainer->getModelIndex(onID, modelId, subModelId);
			if (modelId != beforeModelId)
			{
				modelContainer->refreshModelColor(beforeModelId);
				modelContainer->changeModelColorDark(modelId);
				engine->updateGL();
			}
		}
	}
}

void RotateUIMode::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton && e->button() != Qt::MiddleButton)
		return;

	bool clicked = false;
	if ((pressed - e->pos() * engine->devicePixelRatioF()).manhattanLength() < dragThreshold)
		clicked = true;

	if (onMesh || onHandleX || onHandleY || onHandleZ)
		engine->setCursor(Qt::OpenHandCursor);
	else
		engine->setCursor(Qt::ArrowCursor);

	if (clicked)
	{
		if (e->button() == Qt::MiddleButton && e->modifiers() == Qt::NoModifier)
		{
			if (onMesh)
			{
				IMeshModel* model = modelContainer->findModel(onID);
				if (model)
					engine->fitToModel(model);
			}
			else
				engine->fitToModel();
		}
		else if (e->button() == Qt::LeftButton)
			if (!onMesh && !onHandleX && !onHandleY && !onHandleZ)
				modelContainer->deselectAll();
	}
	else //drag
	{
		if (e->button() == Qt::LeftButton)
		{
			for (int i = 0; i < modelContainer->models.size(); i++)
			{
				if (!modelContainer->models[i]->isSelected())
					continue;

				modelContainer->models[i]->manipulated();
			}

			if (UserProperties::usingDetectCollision)
				detectCollision();
		}
		else if (e->button() == Qt::RightButton)
		{
			if (modelContainer->hasAnySelectedModel())
				updateHandle();
		}
	}

	if (modelContainer->hasAnySelectedModel())
	{
		rotationHandles->resetDrawAxis();
		updateHandle();
	}

	engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,
		QGLViewer::FRAME, QGLViewer::NO_MOUSE_ACTION);
}

void RotateUIMode::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		if (onMesh)
		{
			IMeshModel* model = modelContainer->findModel(onID);
			if (model)
				engine->fitToModel(model);
		}
		else
			engine->fitToModel();
	}
}

void RotateUIMode::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Delete)
		engine->deleteSelectedModels();
}

void RotateUIMode::resizeEvent(QResizeEvent *e)
{
	int w, h;
	if (e == nullptr)
	{
		/*int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		w = viewport[2];
		h = viewport[3];*/
		w = engine->width();
		h = engine->height();
	}
	else
	{
		w = e->size().width();
		h = e->size().height();
	}
	fbo->resize(w, h, engine->devicePixelRatioF());
}

/// Manipulator Handle Functions
void RotateUIMode::updateHandle()
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	if (selecteds.size() == 0)
	{
		camera->setPivotPoint(qglviewer::Vec(Profile::getMachineWidth_calculated() / 2, Profile::getMachineDepth_calculated() / 2, 0));
		return;
	}
	AABB updatedBox = AABBGetter()(selecteds);
	camera->setPivotPoint(updatedBox.getCenter());

	adjustManipulatedFrame(updatedBox);
	adjustHandlePosition(updatedBox);
	adjustHandleScale();
}

void RotateUIMode::adjustManipulatedFrame(AABB& aabb_)
{
	qglviewer::Vec center = aabb_.getCenter();
	manipulatedFrame->setPosition(qglviewer::Vec(center[0], center[1], center[2]));
	manipulatedFrame->setOrientation(qglviewer::Quaternion());
	// Multi-Selected Case ...
	//if (modelContainer->isMultiSelected()) 
	//{
	//	qglviewer::Vec center = aabb_.getCenter();
	//	manipulatedFrame->setPosition(qglviewer::Vec(center[0], center[1], center[2]));
	//	manipulatedFrame->setOrientation(qglviewer::Quaternion());
	//}
	//// Single Model Selected Case
	//else {
	//	for (int i = 0; i < modelContainer->models.size(); i++)
	//	{
	//		if (!modelContainer->models[i]->isSelected())
	//			continue;

	//		qglviewer::Frame modelFrame = modelContainer->models[i]->getFrame().frame;
	//		qglviewer::Vec center = aabb_.getCenter();
	//		manipulatedFrame->setPosition(qglviewer::Vec(center[0], center[1], center[2]));
	//		manipulatedFrame->setOrientation(modelFrame.orientation());

	//		return;
	//	}
	//}
}

void RotateUIMode::adjustHandlePosition(AABB& aabb_)
{
}

void RotateUIMode::adjustHandleScale()
{
	qglviewer::Vec origin;
	manipulatedFrame->getPosition(origin[0], origin[1], origin[2]);

	qglviewer::Vec camPosition = camera->position();
	float dist = (camPosition - origin).norm();

	AABB updatedBox = AABBGetter()(modelContainer->getSelectedModels());
	float sqr = qglviewer::Vec(updatedBox.getLengthX(), updatedBox.getLengthY(), updatedBox.getLengthZ()).norm();

	float coeff = 0.3;
	float scaleFactor = dist * coeff;
	if (scaleFactor < (sqr * 0.5 + 2))
		scaleFactor = (sqr * 0.5 + 2);
	else if (scaleFactor > (sqr * 0.5 + 50))
		scaleFactor = (sqr * 0.5 + 50);
	rotationHandles->setScale(scaleFactor);
}

/// State Refresh Functions
void RotateUIMode::refreshUI()
{
	//DetectCollision();
	updateHandle();
	resizeEvent();
}

void RotateUIMode::detectCollision()
{
	/*std::vector<IMeshModel*> selectedModels = modelContainer->GetSelectedModels();
	CollisionManager cm;
	cm.DetectCollision(selectedModels, modelContainer->GetModelsExceptOne(selectedModels));*/
	AABB updatedBox = AABBGetter()(modelContainer->getSelectedModels());
	std::vector<IMeshModel*> movableModels;
	std::vector<AABB> movable;
	for (int i = 0; i < modelContainer->models.size(); i++)
	{
		if (modelContainer->models[i]->isSelected())
			continue;

		movableModels.push_back(modelContainer->models[i]);
		movable.push_back(modelContainer->models[i]->getAABB());
	}

	CollisionManager cm;
	std::vector<qglviewer::Vec> response = cm.resolveCollision(updatedBox, movable);

	for (int i = 0; i < response.size(); i++)
	{
		qglviewer::Vec t = response[i];
		movableModels[i]->translate(t.x, t.y, t.z);
		movableModels[i]->manipulated();
	}
}

// Private Functions -------------------------------------------------------------------------------
/// Rendering Functions
void RotateUIMode::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	QColor color = UIMode::rotateUIColor;
	glFuncs->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);

	glFuncs->glEnable(GL_BLEND);
	glFuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/// Draw all models in the Model Container
	for (auto model : modelContainer->models)
	{
		model->drawLayoutEditMode(proj_, view_);
		if (engine->getToggle(1))
			model->drawPolygon(proj_, view_);
	}

	/// Draw Manipulator Handle, if there is any selected model
	if (modelContainer->hasAnySelectedModel())
		rotationHandles->drawLayoutEditMode(proj_, view_);

	/// Draw Printer Body
	printerBody->drawOnScreenCanvas(proj_, view_);
	glFuncs->glDisable(GL_BLEND);
	glFuncs->glDisable(GL_DEPTH_TEST);
}

void RotateUIMode::drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	fbo->bind();

	glFuncs->glClearColor(0.0, 0.0, 0.0, 1.0);
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);
	glFuncs->glDisable(GL_BLEND);

	/// Draw all models in the Model Container
	for (int i = 0; i < modelContainer->models.size(); i++)
		modelContainer->models[i]->drawLayoutColoredIndexMap(proj_, view_);

	/// Draw Manipulator Handle, if there is any selected model
	if (modelContainer->hasAnySelectedModel())
		rotationHandles->drawForColorPicking(proj_, view_);

	glFuncs->glDisable(GL_DEPTH_TEST);
	fbo->release();
}

/// AABB Functions
//void RotateUIMode::DetectCollision()
//{
//	// Collect Unselected Models
//	std::vector<IMeshModel*> unselecteds;
//	for (int i = 0; i < modelContainer->models.size(); i++) {
//		if (!modelContainer->models[i]->isSelected()) {
//			unselecteds.push_back(modelContainer->models[i]);
//		}
//	}
//
//	// Calc AABB of Selections
//	AABB targetAABB = CreateAABBforSelections(modelContainer->models);
//	PushModels(targetAABB, unselecteds);
//}
//
//void RotateUIMode::PushModels(AABB aabb_, std::vector<IMeshModel*> models_)
//{
//	if (models_.size() == 0)
//		return;
//
//	std::vector<IMeshModel*> moved;
//	for (int i = 0; i < models_.size(); i++)
//		if (models_[i]->Pushed(aabb_))
//			moved.push_back(models_[i]);
//
//	std::vector<IMeshModel*> untouched;
//	for (int i = 0; i < models_.size(); i++)
//		if (std::find(moved.begin(), moved.end(), models_[i]) == moved.end())
//			untouched.push_back(models_[i]);
//
//	for (int i = 0; i < moved.size(); i++)
//		PushModels(moved[i]->getAABB(), untouched);
//}