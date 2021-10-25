#include "stdafx.h"
#include "AnalysisUIMode.h"

#include "PrinterBody.h"
#include "VolumeAnalysis.h"

AnalysisUIMode::AnalysisUIMode(ViewerModule* engine_)
	: UIMode(engine_)
	, printerBody(engine_->getPrinterBody())
{
}

AnalysisUIMode::~AnalysisUIMode()
{
	delete volumeAnalysis;
}

/// QGLViewer Functions
void AnalysisUIMode::initialize(int width_, int height_)
{
	volumeAnalysis = new VolumeAnalysis();
}

void AnalysisUIMode::draw()
{
	QMatrix4x4 projMat = ProjectionMatrixGetter()(*camera);
	QMatrix4x4 viewMat = ViewMatrixGetter()(*camera);

	drawOnScreenCanvas(projMat, viewMat);
}

/// State Refresh Functions
void AnalysisUIMode::mouseReleaseEvent(QMouseEvent* e)
{
	bool clicked = false;
	QPoint released = e->pos() * engine->devicePixelRatioF();
	if ((pressed - released).manhattanLength() < dragThreshold)
		clicked = true;

	if (clicked)
		if (e->modifiers() == Qt::NoModifier && e->button() == Qt::MidButton)
			engine->fitToModel(AABBGetter()(modelContainer->getSelectedModels()));
}
void AnalysisUIMode::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		engine->fitToModel(AABBGetter()(modelContainer->getSelectedModels()));
}

// Private Functions -------------------------------------------------------------------------------
/// Rendering Functions
void AnalysisUIMode::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	QColor color = UIMode::analysisUIColor;
	glFuncs->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);

	glFuncs->glEnable(GL_BLEND);
	glFuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	///// Draw all models in the Model Container
	//for (int i = 0; i < modelContainer->models.size(); i++)
	//	modelContainer->models[i]->DrawLayoutEditMode(proj_, view_);

	/// Draw Manipulator Handle, if there is any selected model
	//if (modelContainer->HasAnySelectedModel())
	//	translationHandle->DrawLayoutEditMode(proj_, view_);

	///VolumeAnalysis Draw
	volumeAnalysis->drawOnScreenCanvas(proj_, view_);

	/// Draw Printer Body
	printerBody->drawOnScreenCanvas(proj_, view_);
	glFuncs->glDisable(GL_BLEND);
	glFuncs->glDisable(GL_DEPTH_TEST);
}

VolumeAnalysis* AnalysisUIMode::getVolumeAnalysis()
{
	return this->volumeAnalysis;
}