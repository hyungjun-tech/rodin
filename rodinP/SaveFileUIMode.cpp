#include "stdafx.h"
#include "SaveFileUIMode.h"

#include "FBO.h"
//#include "TopCamera.h"
#include "CameraModule.h"
#include "PrinterBody.h"
#include "CartridgeInfo.h"

SaveFileUIMode::SaveFileUIMode()
	:printerBody(nullptr)
{
	modelContainer = nullptr;
}

SaveFileUIMode::SaveFileUIMode(ViewerModule* engine_)
	: planeHeight(100.0f)
{
	modelContainer = new ModelContainer();
	for (int i = 0; i < engine_->modelContainer->models.size(); i++)
	{
		modelContainer->addNewModel(engine_->modelContainer->models[i]->clone());
	}

	printerBody = new PrinterBody();
}


SaveFileUIMode::~SaveFileUIMode()
{
	if (printerBody != nullptr)
		delete printerBody;
	if (modelContainer != nullptr)
		delete modelContainer;
}

bool SaveFileUIMode::saveThumbnail(QString filename_)
{
	QImage img = saveThumbnail(modelContainer->models);
	bool result = img.save(filename_, "PNG");
	return result;
}

QImage SaveFileUIMode::saveThumbnail(std::vector<IMeshModel*> models_)
{
	int width = Profile::machineProfile.thumbnail_image_width.value;
	int height = Profile::machineProfile.thumbnail_image_height.value;
	///////////////////////////////////////////
	QGLFramebufferObject fbo(width, height, QGLFramebufferObject::Depth);
	qglviewer::Camera *tempCamera = getFitToModelCamera(models_);
	///////////////////////////////////////////

	fbo.bind();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	drawOnlyModel(tempCamera, models_);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
	fbo.release();
	delete tempCamera;

	QImage img = fbo.toImage();
	return img;
}

qglviewer::Camera * SaveFileUIMode::getFitToModelCamera()
{
	return getFitToModelCamera(modelContainer->models);
}
qglviewer::Camera * SaveFileUIMode::getFitToModelCamera(std::vector<IMeshModel*> models_)
{
	AABB aabb = AABBGetter()(models_);
	qglviewer::Vec viewDir(0, 1, -0.5);
	qglviewer::Vec upVec(0, 0, 1);
	qglviewer::Vec center = (aabb.getMaximum() + aabb.getMinimum())*0.5;
	qglviewer::Vec vSize = (aabb.getMaximum() - aabb.getMinimum())*0.5;

	qglviewer::Camera *tempCamera = new qglviewer::Camera();

	float fov = tempCamera->fieldOfView();
	float dist = vSize.norm() / sin(fov*0.5);

	qglviewer::Vec newPos = center - viewDir * dist;

	tempCamera->setPosition(newPos);
	tempCamera->setUpVector(upVec);
	tempCamera->setViewDirection(viewDir);
	tempCamera->setSceneCenter(center);
	tempCamera->setZNearCoefficient(0.001);
	tempCamera->setZClippingCoefficient(300);
	int width = Profile::machineProfile.thumbnail_image_width.value;
	int height = Profile::machineProfile.thumbnail_image_height.value;
	float aspectR = width / float(height);
	tempCamera->setAspectRatio(aspectR);

	return tempCamera;
}

qglviewer::Camera* SaveFileUIMode::getTopCamera(int w_, int h_, qglviewer::Vec center_)
{
	//set top camera..//
	const qglviewer::Camera::Type type(qglviewer::Camera::ORTHOGRAPHIC);
	const qglviewer::Vec view_direction(qglviewer::Vec(0, 0, -1));
	const qglviewer::Vec up_vec(qglviewer::Vec(0, 1, 0));
	const qglviewer::Vec position(center_[0], center_[1], Profile::getMachineHeight_calculated());

	CameraModule *camera_ = new CameraModule(view_direction, up_vec, type);

	camera_->setScreenWidthAndHeight(w_, h_);
	camera_->setPosition(position);
	camera_->setFrustrumSize(w_, h_);
	camera_->setClippingDistance(Profile::getMachineHeight_calculated(), 0);

	return camera_;
	/*qglviewer::Camera *tempCamera = new qglviewer::Camera();

	tempCamera->setType(qglviewer::Camera::ORTHOGRAPHIC);
	tempCamera->setViewDirection(qglviewer::Vec(0, 0, -1));
	tempCamera->setUpVector(qglviewer::Vec(0, 1, 0));
	tempCamera->setScreenWidthAndHeight(w_, h_);
	//qglviewer::Vec center(Profile::getMachineWidth_calculated() / 2, Profile::getMachineDepth_calculated() / 2, Profile::getMachineHeight_calculated() / 2);
	qglviewer::Vec camPos(center_[0], center_[1], Profile::getMachineHeight_calculated());
	tempCamera->setPosition(camPos);
	tempCamera->setZNearCoefficient(0);
	tempCamera->setZClippingCoefficient(Profile::getMachineHeight_calculated());
	float fov = std::atan(Profile::getMachineHeight_calculated() / (w_ * 0.5)) * M_PI / 180;
	tempCamera->setHorizontalFieldOfView(fov);
	//tempCamera->setSceneCenter(qglviewer::Vec(center_[0], center_[1], 0));
	return tempCamera;*/

	/*LDNICamera cam;
	cam.SetScreenWidthAndHeight(w, h);
	//qglviewer::Vec center(Profile::getMachineWidth_calculated() / 2, Profile::getMachineDepth_calculated() / 2, Profile::getMachineHeight_calculated() / 2);
	qglviewer::Vec camPos(
		center_[0], center_[1], Profile::getMachineHeight_calculated());
	cam.SetPosition(camPos);
	cam.SetFrustrumSize(w, h);
	cam.SetClippingDistance(
		Profile::getMachineHeight_calculated(), 0);*/
}
qglviewer::Camera* SaveFileUIMode::getRightCamera(int w_, int h_, qglviewer::Vec center_)
{
	const qglviewer::Camera::Type camera_type(qglviewer::Camera::ORTHOGRAPHIC);
	const qglviewer::Vec camera_view_direction(qglviewer::Vec(-1, 0, 0));
	const qglviewer::Vec camera_up_vec(qglviewer::Vec(0, 0, 1));
	const qglviewer::Vec camera_position(Profile::getMachineWidth_calculated(), center_[1], center_[2]);
	
	CameraModule *camera_ = new CameraModule(camera_view_direction, camera_up_vec, camera_type);

	camera_->setScreenWidthAndHeight(w_, h_);
	camera_->setPosition(camera_position);
	camera_->setFrustrumSize(w_, h_);
	camera_->setClippingDistance(Profile::getMachineWidth_calculated(), 0);

	return camera_;
}
qglviewer::Camera * SaveFileUIMode::getIsometricCamera()
{
	qglviewer::Camera *tempCamera = new qglviewer::Camera();

	qglviewer::Vec viewDir(0, 1, -0.5);
	qglviewer::Vec cam(1, 0, 0);
	const double camDistance = Profile::machineProfile.view_cam_distance.value - 110;
	const int camDirection = 0;// SliceProfile::camera_direction.value;
	qglviewer::Vec center(Profile::getMachineWidth_calculated() / 2, Profile::getMachineDepth_calculated() / 2, Profile::getMachineHeight_calculated() / 2);

	double rotate_rad = 90 * camDirection * M_PI / 180;
	qglviewer::Vec rotatedViewDir(viewDir.x * cos(rotate_rad) - viewDir.y * sin(rotate_rad), viewDir.x * sin(rotate_rad) + viewDir.y * cos(rotate_rad), viewDir.z);
	rotatedViewDir.normalize();

	qglviewer::Vec camX(cam.x * cos(rotate_rad) - cam.y * sin(rotate_rad), cam.x * sin(rotate_rad) + cam.y * cos(rotate_rad), cam.z);
	qglviewer::Vec upVec = cross(camX, rotatedViewDir);
	qglviewer::Vec camPos = center - rotatedViewDir * camDistance;

	tempCamera->setPosition(camPos);
	tempCamera->setViewDirection(rotatedViewDir);
	tempCamera->setUpVector(upVec);
	tempCamera->setZNearCoefficient(0.001);
	tempCamera->setZClippingCoefficient(300);
	tempCamera->setAspectRatio(1);

	return tempCamera;
}

void SaveFileUIMode::drawOnlyModel(qglviewer::Camera *camera_)
{
	drawOnlyModel(camera_, modelContainer->models);
}
void SaveFileUIMode::drawOnlyModel(qglviewer::Camera *camera_, std::vector<IMeshModel*> models_)
{
	QMatrix4x4 proj_ = ProjectionMatrixGetter()(*camera_);
	QMatrix4x4 view_ = ViewMatrixGetter()(*camera_);

	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	Renderer *tempRenderer = new PhongShadedRenderer();

	glFuncs->glClearColor(0.0, 0.0, 0.0, 0.0);
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFuncs->glEnable(GL_DEPTH_TEST);
	
	/// Draw all models in the Model Container
	for (auto model : models_)
	{
		for (auto part : model->getModels())
		{
			MVPMatrices mvp((ScaledFrame()).getModelMatrix(), view_, proj_);
			QColor color = CartridgeInfo::getCartColor(part->getCartridgeIndexes().front());
			tempRenderer->setColor(color);
			tempRenderer->draw(part->getMesh(), mvp);
		}
	}
	glFuncs->glDisable(GL_DEPTH_TEST);
}
QImage SaveFileUIMode::saveFullUpperImage(std::vector<IMeshModel*> models_)
{
	//std::vector<IMeshModel*> models;
	//for (int i = 0; i < models_.size(); i++)
	//{
	//	models.push_back(models_[i]->Clone());
	//}

	int w = Profile::getMachineWidth_calculated();
	int h = Profile::getMachineDepth_calculated();
	qglviewer::Vec center(Profile::getMachineWidth_calculated() / 2, Profile::getMachineDepth_calculated() / 2, Profile::getMachineHeight_calculated() / 2);

	QImage rtn = saveUpperImage(w, h, center, models_);
	//for (int i = 0; i < models_.size(); i++)
	//	delete models[i];

	return rtn;
}

QImage SaveFileUIMode::saveFullRightImage(std::vector<IMeshModel*> models_)
{
	//IMeshModel* model;
	//model = model_->Clone();

	

	int w = Profile::getMachineDepth_calculated();
	int h = Profile::getMachineHeight_calculated();
	qglviewer::Vec center(Profile::getMachineWidth_calculated() / 2, Profile::getMachineDepth_calculated() / 2, Profile::getMachineHeight_calculated() / 2);

	//qglviewer::Camera *camera_right = GetRightCamera(w, h, center);

	QImage rtn = saveRightImage(w, h, center, models_);
	
	//delete model;

	return rtn;
}

QImage SaveFileUIMode::saveCroppedUpperImage(std::vector<IMeshModel*> models_)
{
	//std::vector<IMeshModel*> models;
	//for (int i = 0; i < models_.size(); i++)
	//{
	//	models.push_back(models_[i]->Clone());
	//}

	AABB aabb = AABBGetter()(models_);

	int w = aabb.getLengthX();
	int h = aabb.getLengthY();
	qglviewer::Vec center = aabb.getCenter();

	QImage rtn = saveUpperImage(w, h, center, models_);
	//for (int i = 0; i < models.size(); i++)
	//	delete models[i];

	return rtn;
}

QImage SaveFileUIMode::saveUpperImage(int w_, int h_, qglviewer::Vec center_, std::vector<IMeshModel*> models_)
{
	//w = Profile::getMachineWidth_calculated();
	//h = Profile::getMachineDepth_calculated();
	FBO fbo;
	fbo.initialize(w_, h_);
	//QGLFramebufferObject fbo(Profile::getMachineWidth_calculated(), Profile::getMachineDepth_calculated());
	qglviewer::Camera *topCamera = getTopCamera(w_, h_, center_);

	fbo.bind();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glViewport(0, 0, w_, h_);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);

	drawColoredIndexMapImage(topCamera, models_);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
	fbo.release();

	delete topCamera;
	//fbo.toImage().save("fbo_all.png", "PNG");
	return fbo.toImage();
}
QImage SaveFileUIMode::saveRightImage(int w_, int h_, qglviewer::Vec center_, std::vector<IMeshModel*> models_)
{
	//w = Profile::getMachineWidth_calculated();
	//h = Profile::getMachineDepth_calculated();
	FBO fbo;
	fbo.initialize(w_, h_);
	//QGLFramebufferObject fbo(Profile::getMachineWidth_calculated(), Profile::getMachineDepth_calculated());
	qglviewer::Camera *rightCamera = getRightCamera(w_, h_, center_);

	fbo.bind();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glViewport(0, 0, w_, h_);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);

	drawColoredIndexMapImage(rightCamera, models_);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
	fbo.release();

	delete rightCamera;
	//fbo.toImage().save("fbo_all.png", "PNG");
	return fbo.toImage();
}

QImage SaveFileUIMode::saveColoredIndexMapImage(qglviewer::Camera *camera_, int w_, int h_, std::vector<IMeshModel*> models_)
{
	//w = Profile::getMachineWidth_calculated();
	//h = Profile::getMachineDepth_calculated();
	FBO fbo;
	fbo.initialize(w_, h_);
	//QGLFramebufferObject fbo(Profile::getMachineWidth_calculated(), Profile::getMachineDepth_calculated());

	fbo.bind();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glViewport(0, 0, w_, h_);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);

	drawColoredIndexMapImage(camera_, models_);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
	fbo.release();

	//fbo.toImage().save("fbo_all.png", "PNG");
	return fbo.toImage();
}

void SaveFileUIMode::drawColoredIndexMapImage(qglviewer::Camera *camera_, std::vector<IMeshModel*> models_)
{
	QMatrix4x4 proj_ = ProjectionMatrixGetter()(*camera_);
	QMatrix4x4 view_ = ViewMatrixGetter()(*camera_);

	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	glFuncs->glClearColor(0.0, 0.0, 0.0, 0.0);
	glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/// Draw all models in the Model Container
	for (int i = 0; i < models_.size(); i++)
	{
		models_[i]->drawLayoutColoredIndexMap(proj_, view_);
	}
}

QImage SaveFileUIMode::dilationImage(const QImage& input_)
{
	QImage output(input_);

	for (int x = 0; x < input_.width(); ++x)
	{
		for (int y = 0; y < input_.height(); ++y)
		{
			// up
			if (y - 1 >= 0)
			{
				QRgb temPixel = input_.pixel(x, y - 1);
				if (qBlue(temPixel) > 0)
				{
					output.setPixel(x, y, temPixel);
					continue;
				}
			}

			// left
			if (x - 1 >= 0)
			{
				QRgb temPixel = input_.pixel(x - 1, y);
				if (qBlue(temPixel) > 0)
				{
					output.setPixel(x, y, temPixel);
					continue;
				}
			}
			// right
			if (x + 1 < input_.width())
			{
				QRgb temPixel = input_.pixel(x + 1, y);
				if (qBlue(temPixel) > 0)
				{
					output.setPixel(x, y, temPixel);
					continue;
				}
			}

			// down
			if (y + 1 < input_.height())
			{
				QRgb temPixel = input_.pixel(x, y + 1);
				if (qBlue(temPixel) > 0)
				{
					output.setPixel(x, y, temPixel);
					continue;
				}
			}
		}
	}
	return output;
}