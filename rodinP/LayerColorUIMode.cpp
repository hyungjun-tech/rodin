#include "stdafx.h"
#include "LayerColorUIMode.h"

#include "PickingIdxManager.h"
#include "CollisionManager.h"
#include "UserProperties.h"
#include "FBO.h"
#include "PrinterBody.h"

LayerColorUIMode::LayerColorUIMode(ViewerModule* engine_)
	: UIMode(engine_)
	, printerBody(engine_->getPrinterBody())
	, fbo(new FBO())
{
}

LayerColorUIMode::~LayerColorUIMode()
{
	delete fbo;
}
void LayerColorUIMode::initialize(int width_, int height_)
{
	fbo->initialize(width_, height_, engine->devicePixelRatioF());
}
void LayerColorUIMode::draw()
{
	QMatrix4x4 projMat = ProjectionMatrixGetter()(*camera);
	QMatrix4x4 viewMat = ViewMatrixGetter()(*camera);

	drawOffScreenCanvas(projMat, viewMat);
	drawOnScreenCanvas(projMat, viewMat);
}

void LayerColorUIMode::mousePressEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton && e->button() != Qt::MiddleButton)
		return;

	pressed = e->pos() * engine->devicePixelRatioF();

	if (!onMesh && !onHandle)
		return;

	engine->setCursor(Qt::ClosedHandCursor);
	if (onMesh)
	{
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
	}
	else if (onHandle)
		manipulatedFrame->restrictTranslationToZAxis();

	engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,
		QGLViewer::FRAME, QGLViewer::TRANSLATE);
}

void LayerColorUIMode::mouseMoveEvent(QMouseEvent* e)
{
	onID = PickingIdxManager::decodeID(fbo->getColor(e->pos()*engine->devicePixelRatioF()));
	onMesh = PickingIdxManager::isMeshID(onID);
	onHandle = PickingIdxManager::isTransZID(onID);

	if (e->buttons() != Qt::LeftButton)
		if (onMesh || onHandle)
			engine->setCursor(Qt::OpenHandCursor);
		else
			engine->setCursor(Qt::ArrowCursor);
}

void LayerColorUIMode::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton && e->button() != Qt::MiddleButton)
		return;

	bool clicked = false;
	if ((pressed - e->pos() * engine->devicePixelRatioF()).manhattanLength() < dragThreshold)
		clicked = true;

	if (onMesh || onHandle)
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
			if (!onMesh && !onHandle)
				modelContainer->deselectAll();
	}
	else
	{
		if (e->button() == Qt::LeftButton) {
			for (int i = 0; i < modelContainer->models.size(); i++)
			{
				if (!modelContainer->models[i]->isSelected())
					continue;

				modelContainer->models[i]->manipulated();
			}

			if (UserProperties::usingDetectCollision)
				detectCollision();

			if (modelContainer->hasAnySelectedModel())			// (2019.09.05) model 이동 후에도 camera pivot이 이전 위치에 고정되어 있던 현상 개선
				updateHandle();
		}
	}

	engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,
		QGLViewer::FRAME, QGLViewer::NO_MOUSE_ACTION);
}

void LayerColorUIMode::mouseDoubleClickEvent(QMouseEvent* e)
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

void LayerColorUIMode::keyPressEvent(QKeyEvent *e)
{
	//if (e->key() == Qt::Key_Delete)
	//	engine->deleteSelectedModels();
}

void LayerColorUIMode::resizeEvent(QResizeEvent *e)
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
void LayerColorUIMode::updateHandle()
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	if (selecteds.size() == 0)
	{
		camera->setPivotPoint(qglviewer::Vec(Profile::getMachineWidth_calculated() / 2, Profile::getMachineDepth_calculated() / 2, 0));
		return;
	}
	//AABB updatedBox = AABBGetter()(selecteds);
	AABB updatedBox = TotalBoxGetter()(selecteds);			// (2019.09.05) Raft/Support 있는 Model 이동 시 외곽 벗어나는 문제 대응
	camera->setPivotPoint(updatedBox.getCenter());

	adjustManipulatedFrame(updatedBox);
}

void LayerColorUIMode::adjustManipulatedFrame(AABB& aabb_)
{
	qglviewer::Vec center = aabb_.getCenter();
	manipulatedFrame->setPosition(qglviewer::Vec(center[0], center[1], center[2]));
	manipulatedFrame->setOrientation(qglviewer::Quaternion());
}

void LayerColorUIMode::detectCollision()
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

/// State Refresh Functions
void LayerColorUIMode::refreshUI()
{
	updateHandle();
	resizeEvent();
}
void LayerColorUIMode::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	QColor color = UIMode::layerColorUIColor;
	glFuncs->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);

	glFuncs->glEnable(GL_BLEND);
	glFuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/// Draw all models in the Model Container
	for (auto model : modelContainer->models)
	{
		model->drawLayerColorMode(proj_, view_);
		if (engine->getToggle(1))
			model->drawPolygon(proj_, view_);
	}
	for (auto model : modelContainer->models)
		model->drawLayerColorPlane(proj_, view_);

	printerBody->drawOnScreenCanvas(proj_, view_);

	glFuncs->glDisable(GL_BLEND);
	glFuncs->glDisable(GL_DEPTH_TEST);
}

void LayerColorUIMode::drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	fbo->bind();

	glFuncs->glClearColor(0.0, 0.0, 0.0, 1.0);
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/// Draw all models in the Model Container
	for (int i = 0; i < modelContainer->models.size(); i++)
		modelContainer->models[i]->drawLayoutColoredIndexMap(proj_, view_);

	fbo->release();
}

void LayerColorUIMode::createPlane()
{
}
void LayerColorUIMode::changeLayerIndex(int index_)
{
	float z = Profile::sliceProfileCommon.layer_height.value * index_;
	for (auto it : modelContainer->models)
	{
		it->setColorPlaneHeight(z);
	}
	//if (layerColorPlane == nullptr)
//		return;
	//float z = Profile::sliceProfileCommon.layer_height.value * index_;
	//layerColorPlane->SetZ(z);
}