#include "stdafx.h"
#include "TranslateUIMode.h"

#include "PickingIdxManager.h"
#include "CollisionManager.h"
#include "UserProperties.h"
#include "FBO.h"
#include "PrinterBody.h"

TranslateUIMode::TranslateUIMode(ViewerModule* engine_)
	: UIMode(engine_)
	, printerBody(engine_->getPrinterBody())
	, fbo(new FBO())
	, onMesh(false)
	, onHandle(false)
	, onSelectionHandle(false)
	, isMultiSelectionOn(false)
{
	connect(this, SIGNAL(signal_mouseOnModel(int, int)), engine_, SIGNAL(signal_mouseOnModel(int, int)));
	connect(this, SIGNAL(signal_mouseOutModel(int, int)), engine_, SIGNAL(signal_mouseOutModel(int, int)));
}

TranslateUIMode::~TranslateUIMode()
{
	delete fbo;
}

/// QGLViewer Functions
void TranslateUIMode::initialize(int width_, int height_)
{
	fbo->initialize(width_, height_, engine->devicePixelRatioF());
}

void TranslateUIMode::draw()
{
	QMatrix4x4 projMat = ProjectionMatrixGetter()(*camera);
	QMatrix4x4 viewMat = ViewMatrixGetter()(*camera);

	drawOffScreenCanvas(projMat, viewMat);
	drawOnScreenCanvas(projMat, viewMat);
}

void TranslateUIMode::mousePressEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton && e->button() != Qt::MiddleButton)
		return;

	pressed = e->pos() * engine->devicePixelRatioF();

	if (onMesh || onHandle || onSelectionHandle)
	{
		engine->setCursor(Qt::ClosedHandCursor);
		if (onMesh)
		{
			onID = PickingIdxManager::decodeID(fbo->getColor(pressed));
			if (e->modifiers() == Qt::NoModifier)
			{
				if (!modelContainer->isSelected(onID))
				{
					modelContainer->selectExceptOthers(onID);
				}
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
			if (modelContainer->isSelected(onID))
			{
				manipulatedFrame->restrictTranslationToOXYPlane(
					camera->unprojectedCoordinatesOf(qglviewer::Vec(pressed.x(), pressed.y(), 1.0)));
			}
			else
				onID = -1;

			if (modelContainer->hasAnySelectedModel())
				updateHandle();
		}
		else if (onHandle)
			manipulatedFrame->restrictTranslationToZAxis();
		else if (onSelectionHandle)
			manipulatedFrame->restrictTranslationToOXYPlane(
				camera->unprojectedCoordinatesOf(qglviewer::Vec(pressed.x(), pressed.y(), 1.0)));

		engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,
			QGLViewer::FRAME, QGLViewer::TRANSLATE);
	}
	else {
		engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,
			QGLViewer::FRAME, QGLViewer::NO_MOUSE_ACTION);
	}

}

void TranslateUIMode::mouseMoveEvent(QMouseEvent* e)
{
	QPoint currentPos = e->pos()*engine->devicePixelRatioF();

	if (e->buttons() != Qt::LeftButton)
	{
		bool beforeOnMesh = onMesh;
		int beforeOnID = onID;
		onID = PickingIdxManager::decodeID(fbo->getColor(currentPos));
		onMesh = PickingIdxManager::isMeshID(onID);
		onHandle = PickingIdxManager::isTransZID(onID);
		onSelectionHandle = PickingIdxManager::isTransXYID(onID);

		if (onMesh || onHandle || onSelectionHandle)
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
			if (onID != beforeOnID)
			{
				if (beforeModelId >= 0 && beforeModelId < modelContainer->models.size())
					emit signal_mouseOutModel(beforeModelId, beforeSubModelId);
				if (modelId >= 0 && modelId < modelContainer->models.size())
					emit signal_mouseOnModel(modelId, subModelId);
			}

			if (modelId != beforeModelId)
			{
				modelContainer->refreshModelColor(beforeModelId);
				modelContainer->changeModelColorDark(modelId);
				engine->updateGL();
			}
		}
	}
	else {
		if (onMesh || onHandle || onSelectionHandle) return;

		if (isMultiSelectionOn == false) 
		{
			isMultiSelectionOn = true;
			engine->setCursor(Qt::CrossCursor);
		}

		offScreenRectangleCapture(e);
		engine->updateGL();
	}
}

void TranslateUIMode::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton && e->button() != Qt::MiddleButton)
		return;

	bool clicked = false;
	QPoint released = e->pos() * engine->devicePixelRatioF();
	if ((pressed - released).manhattanLength() < dragThreshold)
		clicked = true;

	if (clicked)
	{
		if (e->modifiers() == Qt::NoModifier && e->button() == Qt::MiddleButton)
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
			if (!onMesh && !onHandle && !onSelectionHandle)
				modelContainer->deselectAll();
	}
	else //drag
	{
		if (e->button() == Qt::LeftButton)
		{
			if (!isMultiSelectionOn)		// 기존 process 
			{
				if (onMesh || onHandle || onSelectionHandle)
					engine->setCursor(Qt::OpenHandCursor);
				else
					engine->setCursor(Qt::ArrowCursor);

				for (int i = 0; i < modelContainer->models.size(); i++)
				{
					if (!modelContainer->models[i]->isSelected())
						continue;

					modelContainer->models[i]->manipulated();
				}

				if (UserProperties::usingDetectCollision)
					detectCollision();
			}
			else // Multi-Select
			{
				engine->setCursor(Qt::ArrowCursor);
				isMultiSelectionOn = false;
				if (modelContainer->models.empty())
					return;

				std::vector<int> selectedIDs = collectModelIDs(pressed, released);
				if (selectedIDs.size() != 0)
				{
					if (e->modifiers() == Qt::NoModifier) {
						modelContainer->selectExceptOthers(selectedIDs);
					}
					else if (e->modifiers() == Qt::ShiftModifier) {
						for (auto id : selectedIDs)
							modelContainer->deSelect(id);
					}
					else if (e->modifiers() == Qt::ControlModifier) {
						for (auto id : selectedIDs)
							modelContainer->toggleModelSelection(id);
					}
				}
			}

			if (modelContainer->hasAnySelectedModel())			// (2019.09.05) model 이동 후에도 camera pivot이 이전 위치에 고정되어 있던 현상 개선
				updateHandle();
		}
	}

	engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,
		QGLViewer::FRAME, QGLViewer::NO_MOUSE_ACTION);
}

void TranslateUIMode::mouseDoubleClickEvent(QMouseEvent* e)
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

void TranslateUIMode::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Delete)
		engine->deleteSelectedModels();
	else if (e->key() == Qt::Key_S)
		engine->undoJoinModels();
	else if (e->key() == Qt::Key_M)
		engine->joinModels();
	//if (e->key() == Qt::Key_T) 
	//{
	//	std::vector<IMeshModel*> selections = modelContainer->GetSelectedModels();
	//	for (int i = 0; i < selections.size(); i++) 
	//	{
	//		selections[i]->ToggleTransparency();
	//		engine->updateGL();
	//	}
	//}
}

void TranslateUIMode::resizeEvent(QResizeEvent *e)
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

void TranslateUIMode::drawSelectionRectangle()
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, engine->width() * engine->devicePixelRatioF(), engine->height() * engine->devicePixelRatioF(), 0, 0.0, -1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glFuncs->glDisable(GL_LIGHTING);
	glFuncs->glEnable(GL_BLEND);

	// Draw Area
	glColor4f(0.0, 0.0, 0.3f, 0.3f);
	glBegin(GL_QUADS);
	glVertex2i(rectangle.left(), rectangle.top());
	glVertex2i(rectangle.right(), rectangle.top());
	glVertex2i(rectangle.right(), rectangle.bottom());
	glVertex2i(rectangle.left(), rectangle.bottom());
	glEnd();

	// Draw Edge
	glLineWidth(1.0);
	glColor4f(0.4f, 0.4f, 0.5f, 0.5f);
	glBegin(GL_LINE_LOOP);
	glVertex2i(rectangle.left(), rectangle.top());
	glVertex2i(rectangle.right(), rectangle.top());
	glVertex2i(rectangle.right(), rectangle.bottom());
	glVertex2i(rectangle.left(), rectangle.bottom());
	glEnd();

	glFuncs->glDisable(GL_BLEND);
	//glFuncs->glEnable(GL_LIGHTING);

	///////////stopScreenCoordinatesSystem///////////
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	/////////////////////////////////////////////////
}

void TranslateUIMode::offScreenRectangleCapture(QMouseEvent * e)
{
	rectangle = QRect(pressed, e->pos()*engine->devicePixelRatioF());
	QRect rectangle_ = rectangle.normalized();
}

std::vector<int> TranslateUIMode::collectModelIDs(QPoint corner1st, QPoint corner2nd)
{
	int upperX = std::min(corner1st.x(), corner2nd.x());
	int upperY = std::min(corner1st.y(), corner2nd.y());
	int lowerX = std::max(corner1st.x(), corner2nd.x());
	int lowerY = std::max(corner1st.y(), corner2nd.y());

	std::set<int> exceptIDs;
	for (int i = upperX; i <= lowerX; i++)
	{
		int gottenID = PickingIdxManager::decodeID(fbo->getColor(QPoint(i, upperY)));
		if (gottenID != 0) exceptIDs.insert(gottenID);
		gottenID = PickingIdxManager::decodeID(fbo->getColor(QPoint(i, lowerY)));
		if (gottenID != 0) exceptIDs.insert(gottenID);
	}
	for (int j = upperY + 1; j < lowerY; j++)
	{
		int gottenID = PickingIdxManager::decodeID(fbo->getColor(QPoint(lowerX, j)));
		if (gottenID != 0) exceptIDs.insert(gottenID);
		gottenID = PickingIdxManager::decodeID(fbo->getColor(QPoint(upperX, j)));
		if (gottenID != 0) exceptIDs.insert(gottenID);
	}

	std::vector<int> includeIDs;
	for (auto it = modelContainer->models.cbegin(); it != modelContainer->models.cend(); it++)
	{
		qglviewer::Vec projCenter = camera->projectedCoordinatesOf((*it)->getTotalBox().getCenter());
		int center_x = (int)projCenter.x;
		int center_y = (int)projCenter.y;
		if (upperX < center_x && center_x < lowerX && upperY < center_y && center_y < lowerY)
			includeIDs.push_back((*it)->getID());
	}

	for (auto it = exceptIDs.cbegin(); it != exceptIDs.cend(); it++)
	{
		auto found = std::find(includeIDs.begin(), includeIDs.end(), (*it));
		if (found == includeIDs.end()) continue;
		includeIDs.erase(found);
	}
	return includeIDs;
}


/// Manipulator Handle Functions
void TranslateUIMode::updateHandle()
{
	if (!camera)
		return;

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

void TranslateUIMode::adjustManipulatedFrame(AABB& aabb_)
{
	qglviewer::Vec center = aabb_.getCenter();
	manipulatedFrame->setPosition(qglviewer::Vec(center[0], center[1], center[2]));
	manipulatedFrame->setOrientation(qglviewer::Quaternion());
}

void TranslateUIMode::detectCollision()
{
	/*std::vector<IMeshModel*> selectedModels = modelContainer->GetSelectedModels();
	CollisionManager cm;
	cm.detectCollision(selectedModels, modelContainer->GetModelsExceptOne(selectedModels));*/
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
void TranslateUIMode::refreshUI()
{
	updateHandle();
	resizeEvent();
}

void TranslateUIMode::resetBed()
{
}

// Private Functions -------------------------------------------------------------------------------
/// Rendering Functions
void TranslateUIMode::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	QColor color = UIMode::translateUIColor;
	glFuncs->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);

	glFuncs->glEnable(GL_BLEND);
	glFuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/// Draw all models in the Model Container
	for (auto model : modelContainer->models)
	{
		model->drawSelectionHandle(proj_, view_);
		model->drawLayoutEditMode(proj_, view_);
		if (engine->getToggle(1))
			model->drawPolygon(proj_, view_);
	}

		/// Draw Printer Body
	if (printerBody)
		printerBody->drawOnScreenCanvas(proj_, view_);

	glFuncs->glDisable(GL_BLEND);
	glFuncs->glDisable(GL_DEPTH_TEST);

	if (isMultiSelectionOn)	drawSelectionRectangle();
}

void TranslateUIMode::drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	fbo->bind();

	glFuncs->glClearColor(0.0, 0.0, 0.0, 1.0);
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);

	glDisable(GL_BLEND);


	/// Draw all models in the Model Container
	for (auto model : modelContainer->models)
	{
		model->drawSelectionHandleForPicking(proj_, view_);
		model->drawLayoutColoredIndexMap(proj_, view_);
	}

	glEnable(GL_BLEND);
	glFuncs->glDisable(GL_DEPTH_TEST);

	fbo->release();
}