#include "stdafx.h"
#include "ChangeBottomUIMode.h"

#include "SliceProfile.h"
#include "PickingIdxManager.h"
#include "CollisionManager.h"
#include "UserProperties.h"
#include "FBO.h"
#include "PrinterBody.h"
#include "ManipulatorHandle.h"

ChangeBottomUIMode::ChangeBottomUIMode(ViewerModule* engine_)
	: UIMode(engine_)
	, printerBody(engine_->getPrinterBody())
	, fboColoredIndex(new FBO())
	, fboNormalVecMap(new FBO())
	, isDummyVisible(false)
	, bTriMesh(nullptr)
	, modelChanged(false)
	, latestModelNo(0)
	, onMesh(false)
{
}

ChangeBottomUIMode::~ChangeBottomUIMode()
{
	delete fboColoredIndex;
	delete fboNormalVecMap;

	delete dummy;
}

/// QGLViewer Functions
void ChangeBottomUIMode::initialize(int width_, int height_)
{
	fboColoredIndex->initialize(width_, height_);
	fboNormalVecMap->initialize(width_, height_);

	/// Initialize BottomSurface ...
	dummy = new BottomMeshHandle();
}

void ChangeBottomUIMode::draw()
{
	QMatrix4x4 projMat = ProjectionMatrixGetter()(*camera);
	QMatrix4x4 viewMat = ViewMatrixGetter()(*camera);

	drawOnScreenCanvas(projMat, viewMat);
	drawOffScreenCanvas(projMat, viewMat);
}

void ChangeBottomUIMode::mousePressEvent(QMouseEvent* e)
{
	engine->setCursor(Qt::ArrowCursor);

	if (e->button() == Qt::LeftButton)
	{
		isDummyVisible = false;
		modelChanged = false;
		latestModelNo = modelContainer->getSelectedModelNumber(onID);

		if (!onMesh)
			return;
		else {
			if (e->modifiers() == Qt::NoModifier)
			{
				if (!modelContainer->isSelected(onID))
				{
					modelContainer->deselectAll();
					modelContainer->select(onID);
					modelChanged = true;
				}
			}
			else if (e->modifiers() == Qt::ShiftModifier)
			{
				modelContainer->deSelect(onID);
				modelChanged = true;
			}
			else if (e->modifiers() == Qt::ControlModifier)
			{
				//modelContainer->Select(onID);
				modelContainer->toggleModelSelection(onID);
				modelChanged = true;
			}
			if (modelContainer->hasAnySelectedModel())
				updateHandle();

			if (modelContainer->isSelected(onID))
			{
				qglviewer::Vec eyePt(pressed.x(), pressed.y(), fboColoredIndex->getDepth(pressed));
				qglviewer::Vec savedPosition = camera->unprojectedCoordinatesOf(qglviewer::Vec(pressed.x(), pressed.y(), fboColoredIndex->getDepth(pressed)));
				manipulatedFrame->restrictTranslationToOXYPlane(savedPosition);
			}

			pressed = e->pos()*engine->devicePixelRatioF();
			engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,
				QGLViewer::FRAME, QGLViewer::TRANSLATE);
		}
	}
	else if (e->button() == Qt::RightButton) {
		pressed = e->pos()*engine->devicePixelRatioF();
	}
	else
		return;
}

void ChangeBottomUIMode::mouseMoveEvent(QMouseEvent* e)
{
	onID = PickingIdxManager::decodeID(fboColoredIndex->getColor(e->pos()*engine->devicePixelRatioF()));
	onMesh = PickingIdxManager::isMeshID(onID);			// == onOuterMesh

	// mouse move 에서 선택면 보일 때 활성화		// 너무 늦다. release에서 실행.
/*	qglviewer::Vec normal = fboNormalVecMap->getColor(e->pos()*engine->devicePixelRatioF());
	onNormal = (onMesh) ? (normal * 2 + qglviewer::Vec(-1, -1, -1)) : (normal);
	B_worldPos = GetWorldPositionFromNormalMap(e->pos()*engine->devicePixelRatioF());*/

	/// When Draggin Mouse ...
	if (e->buttons() == Qt::LeftButton)
	{
		if(onMesh)
			engine->setCursor(Qt::OpenHandCursor);
		else
			engine->setCursor(Qt::ArrowCursor);
	}
	/// Else, when just Hanging Aroud ...
	else
	{
		if (onMesh)
		{
			engine->setCursor(Qt::ArrowCursor);

			// mouse move 에서 선택면 보일 때 활성화
			// from here
		/*	x = onNormal.x;
			y = onNormal.y;
			z = onNormal.z;

			if (modelContainer->isSelected(onID))
				isDummyVisible = true;

			ColorTriMesh(true);*/
			// to here
		}
		else
			engine->setCursor(Qt::ArrowCursor);
	}
}

void ChangeBottomUIMode::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		if ((pressed - e->pos()*engine->devicePixelRatioF()).manhattanLength() < dragThreshold)
		{
			onID = PickingIdxManager::decodeID(fboColoredIndex->getColor(e->pos()*engine->devicePixelRatioF()));
			onMesh = PickingIdxManager::isMeshID(onID);
			qglviewer::Vec normal = fboNormalVecMap->getColor(e->pos()*engine->devicePixelRatioF());
			onNormal = (onMesh) ? (normal * 2 + qglviewer::Vec(-1, -1, -1)) : (normal);
			B_worldPos = getWorldPositionFromNormalMap(e->pos()*engine->devicePixelRatioF());

			// After Clicking ...
			if (onMesh)
			{
				if (e->modifiers() == Qt::NoModifier)
				{
					if (modelChanged)
					{
						updateHandle();
						isDummyVisible = false;
					}
					else
					{
						x = onNormal.x;
						y = onNormal.y;
						z = onNormal.z;

						if (modelContainer->isSelected(onID))
						{
							isDummyVisible = true;
							colorTriMesh(true);
						}
						else
							isDummyVisible = false;
					}
				}
			}
			else
				isDummyVisible = false;
		}
		else
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
		engine->setMouseBinding(Qt::NoModifier, Qt::LeftButton,
			QGLViewer::FRAME, QGLViewer::NO_MOUSE_ACTION);
	}
	else if (e->button() == Qt::RightButton)
	{
		if ((pressed - e->pos()*engine->devicePixelRatioF()).manhattanLength() < dragThreshold)
		{
			if (isDummyVisible)
			{
				rotateToNewBottom();
				isDummyVisible = false;
			}
		}
	}

	else
		return;
}

void ChangeBottomUIMode::wheelEvent(QWheelEvent *e)
{
	// Do Nothing
}


void ChangeBottomUIMode::keyPressEvent(QKeyEvent *e)
{
	// Do Nothing
}

void ChangeBottomUIMode::resizeEvent(QResizeEvent *e)
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
	fboColoredIndex->resize(w, h);
	fboNormalVecMap->resize(w, h);
}

/// State Refresh Functions
void ChangeBottomUIMode::refreshUI()
{
	updateHandle();
	resizeEvent();
}

/// Rendering Functions
void ChangeBottomUIMode::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	QColor color = UIMode::changeBottomUIColor;
	glFuncs->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/// Draw all models in the Model Container.
	for (int i = 0; i < modelContainer->models.size(); i++)
		modelContainer->models[i]->drawLayoutEditMode(proj_, view_);

	if (isDummyVisible)
		dummy->drawLayoutEditMode(proj_, view_);

	/// Draw Printer Body
	printerBody->drawOnScreenCanvas(proj_, view_);
}

void ChangeBottomUIMode::drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	drawColoredIndex(proj_, view_);

	drawNormalVecMap(proj_, view_);
}

void ChangeBottomUIMode::drawColoredIndex(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	fboColoredIndex->bind();

	glFuncs->glClearColor(0.0, 0.0, 0.0, 1.0);
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/// Draw all models in the Model Container
	for (int i = 0; i < modelContainer->models.size(); i++)
		modelContainer->models[i]->drawLayoutColoredIndexMap(proj_, view_);

	fboColoredIndex->release();
}

void ChangeBottomUIMode::drawNormalVecMap(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	fboNormalVecMap->bind();

	glFuncs->glClearColor(0.0, 0.0, 0.0, 1.0);
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);

	glFuncs->glDisable(GL_BLEND);

	for (int i = 0; i < modelContainer->models.size(); i++)
	{
		modelContainer->models[i]->drawNormalVectorMap(proj_, view_, true);
	}

	glFuncs->glEnable(GL_BLEND);
	glFuncs->glDisable(GL_DEPTH_TEST);
	fboNormalVecMap->release();
}

qglviewer::Vec ChangeBottomUIMode::getWorldPositionFromNormalMap(QPoint cursor_)
{
	float d = fboNormalVecMap->getDepth(cursor_);
	qglviewer::Vec eyePt(cursor_.x(), cursor_.y(), d);

	qglviewer::Vec pos = camera->unprojectedCoordinatesOf(eyePt);
	return qglviewer::Vec(pos.x, pos.y, pos.z);
}

/// Multi Selection Funcs

void ChangeBottomUIMode::rotateToNewBottom()
{
	// 특별 케이스 먼저			// 'cos - x, y, z 한 성분이 1 이여도 다른 성분값이 0이 아님(-0.00392150879..). 1/255..
	if (abs(x) < 0.004)
		x = 0.0;
	if (abs(y) < 0.004)
		y = 0.0;
	if (abs(z) < 0.004)
		z = 0.0;
	// normalize x,y,z
	x = x / sqrt(x*x + y*y + z*z);
	y = y / sqrt(x*x + y*y + z*z);
	z = z / sqrt(x*x + y*y + z*z);

	// 한 방향만 갖게 되었을 때
	if (z == 1)
		engine->rotateSelectedModels(M_PI, 0, 0);
	else if (z == -1){}
	else if (x == 1)
		engine->rotateSelectedModels(0, M_PI / 2, 0);
	else if (x == -1)
		engine->rotateSelectedModels(0, -M_PI / 2, 0);
	else if (y == 1)
		engine->rotateSelectedModels(-M_PI / 2, 0, 0);
	else if (y == -1)
		engine->rotateSelectedModels(M_PI / 2, 0, 0);
	else			// 일반적인 경우. 한 방향으로 +/-1의 값을 가지지 않을 때
	{
		float lxy = sqrtf(x*x + y*y);
		float Angle2Xaxis = acos(x / lxy);
		if (y < 0)
			Angle2Xaxis = -Angle2Xaxis;
		float Angle2RotateZ = -Angle2Xaxis;
		float Angle2Zaxis = acos(z);
		float Angle2RotateY = -Angle2Zaxis + M_PI;
		engine->rotateSelectedModels(0, 0, Angle2RotateZ);
		engine->rotateSelectedModels(0, Angle2RotateY, 0);
		engine->rotateSelectedModels(0, 0, -Angle2RotateZ);		// 위치 보상
	}
}

void ChangeBottomUIMode::colorTriMesh(bool multiFaces_)
{
	std::vector<qglviewer::Vec> triVertices;
	if (multiFaces_)
	{		// 삼각형 그려 넣기
		triVertices = engine->findBottomMesh(latestModelNo, B_worldPos, onNormal);  // qglviewer vec 3 points
#ifdef _DEBUG
		qglviewer::Vec temp;
		temp = triVertices[0];
		temp = triVertices[1];
		temp = triVertices[2];
		temp;
#endif 
		// make a mesh
		dummy->findTriangle(triVertices);
		//isDummyVisible = true;
	}
	else {		// 삼각형 삭제
//		isDummyVisible = false;
	}
}

void ChangeBottomUIMode::detectCollision()
{
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
	std::vector<qglviewer::Vec> response =
		cm.resolveCollision(updatedBox, movable);

	for (int i = 0; i < response.size(); i++)
	{
		qglviewer::Vec t = response[i];
		movableModels[i]->translate(t.x, t.y, t.z);
		movableModels[i]->manipulated();
	}
}

/// Manipulator Handle Functions
void ChangeBottomUIMode::updateHandle()
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	if (selecteds.size() == 0)
	{
		camera->setPivotPoint(Profile::getMachineCenter_withOffset());
		return;
	}
	AABB updatedBox = AABBGetter()(selecteds);
	camera->setPivotPoint(updatedBox.getCenter());

	adjustManipulatedFrame(updatedBox);
//	AdjustHandlePosition(updatedBox);
//	AdjustHandleScale();
}

void ChangeBottomUIMode::adjustManipulatedFrame(AABB& aabb_)
{
//	qglviewer::Vec center = aabb_.getCenter();
//	manipulatedFrame->setPosition(qglviewer::Vec(center[0], center[1], center[2]));
//	manipulatedFrame->setOrientation(qglviewer::Quaternion());
	for (int i = 0; i < modelContainer->models.size(); i++) 
	{
		if (!modelContainer->models[i]->isSelected())
			continue;

		qglviewer::Frame modelFrame = modelContainer->models[i]->getFrame().frame;
		qglviewer::Vec center = aabb_.getCenter();
		manipulatedFrame->setPosition(qglviewer::Vec(center[0], center[1], center[2]));
		manipulatedFrame->setOrientation(modelFrame.orientation());

		return;
	}


}