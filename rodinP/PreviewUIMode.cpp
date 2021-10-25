#include "stdafx.h"
#include "PreviewUIMode.h"

#include "PrinterBody.h"
#include "GCodePath.h"

PreviewUIMode::PreviewUIMode(ViewerModule* engine_)
	: UIMode(engine_)
	, planeHeight(100.0f)
	, printerBody(engine_->getPrinterBody())
	, gcodePath(nullptr)
{

}
PreviewUIMode::~PreviewUIMode()
{
	delete gcodePath;
}

void PreviewUIMode::initialize(int width_, int height_)
{
	gcodePath = new GCodePath();
}
void PreviewUIMode::updateGCodePath(std::vector<std::vector<GCodePathforDraw>> path_)
{
	gcodePath->updateGCodePath(path_);
}
void PreviewUIMode::setZHeightBase(float zHeightBase_)
{
	z_height_base = zHeightBase_;
	/*for (auto it : modelContainer->models)
		it->Translate(0, 0, z_height_base);*/
}

void PreviewUIMode::draw()
{
	QMatrix4x4 projMat = ProjectionMatrixGetter()(*camera);
	QMatrix4x4 viewMat = ViewMatrixGetter()(*camera);

	drawOnScreenCanvas(projMat, viewMat);
}

void PreviewUIMode::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	/// Initialize OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	QColor color = UIMode::previewUIColor;
	glFuncs->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);
	glFuncs->glEnable(GL_BLEND);
	glFuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/// Draw all models in the Model Container
	glFuncs->glDepthMask(GL_FALSE);
	for (int i = 0; i < modelContainer->models.size(); i++)
		modelContainer->models[i]->drawPreviewMode(proj_, view_, z_height_base);
	glFuncs->glDepthMask(GL_TRUE);
	gcodePath->drawOnScreenCanvas(proj_, view_);
	/// Draw Printer Body
	printerBody->drawOnScreenCanvas(proj_, view_);
	glFuncs->glDisable(GL_BLEND);
	glFuncs->glDisable(GL_DEPTH_TEST);
}

void PreviewUIMode::changeLayerIndex(int index_)
{
	if (gcodePath == nullptr)
		return;
	gcodePath->setCurrentLayer(index_ - 1);
	//todo
	//planeHeight += index_ * 0.05;// SliceProfile::layer_height.value;

	/*int numOfRafts = qFloor(SlicerOptions::platformHeight / SlicerOptions::platformLayerHeight + 0.5);
	if (index_ <= numOfRafts)
		planeHeight = (index_ - 0.5) * SlicerOptions::platformLayerHeight;
	else
	{
		planeHeight = (numOfRafts - 0.5) * SliceProfile::platform_layer_height.value;
		planeHeight += (index_ - numOfRafts) *  SliceProfile::layer_height.value;
	}*/
}
void PreviewUIMode::setPlaneHeight(float planeHeight_)
{
	planeHeight = planeHeight_;
}

void PreviewUIMode::toggleExtruderMode(bool flag_)
{
	if (gcodePath == nullptr)
		return;
	gcodePath->toggleExtruderMode(flag_);
}
void PreviewUIMode::toggleCurrentLayerOnly(bool flag_)
{
	if (gcodePath == nullptr)
		return;
	gcodePath->toggleCurrentLayerOnly(flag_);
}
void PreviewUIMode::toggleShowTravelPath(bool flag_)
{
	if (gcodePath == nullptr)
		return;
	gcodePath->toggleShowTravelPath(flag_);
}

void PreviewUIMode::clear()
{
	gcodePath->updateBufferData({});
}