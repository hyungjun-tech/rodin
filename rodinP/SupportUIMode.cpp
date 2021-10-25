#include "stdafx.h"
#include "SupportUIMode.h"

#include "PickingIdxManager.h"
#include "PrinterBody.h"
#include "SupportEditPlane.h"
#include "FBO.h"

SupportUIMode::SupportUIMode(ViewerModule* engine_)
	: UIMode(engine_)
	, supportEditPlane(nullptr)
	, printerBody(engine_->getPrinterBody())
	, fbo(new FBO())
{
}

SupportUIMode::~SupportUIMode()
{
	delete supportEditPlane;
	delete fbo;
}

void SupportUIMode::initialize(int width_, int height_)
{
	supportEditPlane = new SupportEditPlane();

	fbo->initialize(width_, height_, engine->devicePixelRatioF());
}

void SupportUIMode::draw()
{
	QMatrix4x4 projMat = ProjectionMatrixGetter()(*camera);
	QMatrix4x4 viewMat = ViewMatrixGetter()(*camera);

	drawOffScreenCanvas(projMat, viewMat);
	drawOnScreenCanvas(projMat, viewMat);
}

void SupportUIMode::mousePressEvent(QMouseEvent* e)
{
	UIMode::mousePressEvent(e);
	//fbo->toImage().save("aaaa.png");
	if (e->button() != Qt::LeftButton)
		return;

	if (onPlane)
		updateData(e);
}
void SupportUIMode::mouseMoveEvent(QMouseEvent* e)
{
	onID = PickingIdxManager::decodeID(fbo->getColor(e->pos()*engine->devicePixelRatioF()));
	onPlane = PickingIdxManager::isPlaneID(onID);

	if (e->buttons() != Qt::LeftButton)
		if (onPlane)
			engine->setCursor(Qt::PointingHandCursor);
		else
			engine->setCursor(Qt::ArrowCursor);
	else if (onPlane)
		updateData(e);
}
void SupportUIMode::resizeEvent(QResizeEvent *e)
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

void SupportUIMode::refreshUI()
{
	resizeEvent();
}

void SupportUIMode::setSupportData(SupportData* supportData_)
{
	supportEditPlane->setSupportData(supportData_);
	supportEditPlane->createMesh();
}

void SupportUIMode::changeLayerIndex(int index_)
{
	if (supportEditPlane == nullptr)
		return;
	int intZ = Profile::configSettings.front().layer_height * index_;
	double z = INT2MM(intZ);
	modelContainer->supportData->setZ(intZ);
	supportEditPlane->updateBufferData(z);
}

void SupportUIMode::updateData()
{
	supportEditPlane->updateColorData();
	supportEditPlane->createSupportLines();
	engine->updateGL();
}

void SupportUIMode::updateData(QMouseEvent* e)
{
	qglviewer::Vec worldPos = getWorldPositionFromFBO(e->pos()*engine->devicePixelRatioF());
	if (e->modifiers() == Qt::NoModifier)
	{
		modelContainer->supportData->enable(worldPos.x, worldPos.y);
		updateData();
	}
	else if (e->modifiers() == Qt::CTRL)
	{
		modelContainer->supportData->disable(worldPos.x, worldPos.y);
		updateData();
	}
}

void SupportUIMode::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	QColor color = UIMode::supportUIColor;
	glFuncs->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);

	glFuncs->glEnable(GL_BLEND);
	glFuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/// Draw all models in the Model Container
	for (int i = 0; i < modelContainer->models.size(); i++)
	{
		modelContainer->models[i]->drawLayoutEditMode(proj_, view_);
		if (engine->getToggle(1))
			modelContainer->models[i]->drawPolygon(proj_, view_);
	}

	supportEditPlane->drawOnScreenCanvas(proj_, view_);
	
	/// Draw Printer Body
	printerBody->drawOnScreenCanvas(proj_, view_);

	glFuncs->glDisable(GL_BLEND);
	glFuncs->glDisable(GL_DEPTH_TEST);
}

void SupportUIMode::drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	fbo->bind();

	glFuncs->glClearColor(0.0, 0.0, 0.0, 1.0);
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);

	glDisable(GL_BLEND);

	supportEditPlane->drawForColorPicking(proj_, view_);

	glEnable(GL_BLEND);
	glFuncs->glDisable(GL_DEPTH_TEST);

	fbo->release();
}

qglviewer::Vec SupportUIMode::getWorldPositionFromFBO(QPoint cursor_)
{
	float d = fbo->getDepth(cursor_);
	qglviewer::Vec eyePt(cursor_.x(), cursor_.y(), d);

	qglviewer::Vec pos = camera->unprojectedCoordinatesOf(eyePt);
	return qglviewer::Vec(pos.x, pos.y, pos.z);
}