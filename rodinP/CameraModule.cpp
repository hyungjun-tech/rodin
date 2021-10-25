#include "stdafx.h"
#include "CameraModule.h"

float CameraModule::clipLength = 0.01;
CameraModule::CameraModule()
{
	setType(qglviewer::Camera::ORTHOGRAPHIC);
	setViewDirection(qglviewer::Vec(0, 0, -1));
	setUpVector(qglviewer::Vec(0, 1, 0));
	posX = 0;
	posY = 0;
	posZ = 0;
}
CameraModule::CameraModule(qglviewer::Vec view_direction_, qglviewer::Vec up_, qglviewer::Camera::Type type_)
{
	setType(type_);
	setViewDirection(view_direction_);
	setUpVector(up_);
	posX = 0;
	posY = 0;
	posZ = 0;
}
CameraModule::CameraModule(const CameraModule &cam_)
{
	setType(qglviewer::Camera::ORTHOGRAPHIC);
	setViewDirection(qglviewer::Vec(0, 0, -1));
	setUpVector(qglviewer::Vec(0, 1, 0));

	setPosition(cam_.position());
	setFrustrumSize(cam_.halfWidth*2.0, cam_.halfHeight*2.0);
	setScreenWidthAndHeight(cam_.screenWidth(), cam_.screenHeight());
	setClippingDistance(cam_.farClip, cam_.nearClip);
	posX = cam_.posX;
	posY = cam_.posY;
	posZ = cam_.posZ;
}

CameraModule& CameraModule::operator=(const CameraModule& cam_)
{
	setPosition(cam_.position());
	setFrustrumSize(cam_.halfWidth*2.0, cam_.halfHeight*2.0);
	setScreenWidthAndHeight(cam_.screenWidth(), cam_.screenHeight());
	setClippingDistance(cam_.farClip, cam_.nearClip);
	posX = cam_.posX;
	posY = cam_.posY;
	posZ = cam_.posZ;

	return *this;
}

qglviewer::Vec CameraModule::WorldToCamera(const qglviewer::Vec& wol_) const
{
	qglviewer::Vec wol = wol_;
	wol.x -= (currentX - posX);
	wol.y -= (currentY - posY);
	wol.z -= (currentZ - posZ);
	qglviewer::Vec ret = projectedCoordinatesOf(wol);
	ret[0] = floor(ret[0]);
	ret[1] = floor(ret[1]);

	return ret;
}
qglviewer::Vec CameraModule::CameraToWorld(const qglviewer::Vec& vol_) const
{
	qglviewer::Vec v(vol_);
	v[0] += 0.5;
	v[1] += 0.5;
	qglviewer::Vec wol = unprojectedCoordinatesOf(v);
	wol[0] += (currentX - posX);
	wol[1] += (currentY - posY);
	wol[2] += (currentZ - posZ);
	return wol;
}

qglviewer::Vec CameraModule::unprojectedCoordinatesOf(
	const qglviewer::Vec &src, const qglviewer::Frame *frame) const
{
	GLdouble modelViewMatrix_[16], projectionMatrix_[16];
	getModelViewMatrix(modelViewMatrix_);
	getProjectionMatrix(projectionMatrix_);

	GLdouble x, y, z;
	GLint viewport[4];
	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = screenWidth();
	viewport[3] = screenHeight();
	//viewport[2] = halfWidth * 2;
	//viewport[3] = halfHeight * 2;
	gluUnProject(src.x, src.y, src.z, modelViewMatrix_, projectionMatrix_,
		viewport, &x, &y, &z);
	if (frame)
		return frame->coordinatesOf(qglviewer::Vec(x, y, z));
	else
		return qglviewer::Vec(x, y, z);
}
qglviewer::Vec CameraModule::projectedCoordinatesOf(
	const qglviewer::Vec &src, const qglviewer::Frame *frame) const
{
	GLdouble modelViewMatrix_[16], projectionMatrix_[16];
	getModelViewMatrix(modelViewMatrix_);
	getProjectionMatrix(projectionMatrix_);

	GLdouble x, y, z;
	GLint viewport[4];
	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = screenWidth();
	viewport[3] = screenHeight();
	//viewport[2] = halfWidth * 2;
	//viewport[3] = halfHeight * 2;
	if (frame) {
		const qglviewer::Vec tmp = frame->inverseCoordinatesOf(src);
		gluProject(tmp.x, tmp.y, tmp.z, modelViewMatrix_, projectionMatrix_,
			viewport, &x, &y, &z);
	}
	else
		gluProject(src.x, src.y, src.z, modelViewMatrix_, projectionMatrix_,
			viewport, &x, &y, &z);

	return qglviewer::Vec(x, y, z);
}


Camera::Camera()
	: qglviewer::Camera()
	, devicePixelRatioF(1.0f)
{
}
Camera::Camera(float _devicePixelRatioF)
	: qglviewer::Camera()
	, devicePixelRatioF(_devicePixelRatioF)
{
}
qglviewer::Vec Camera::unprojectedCoordinatesOf(
	const qglviewer::Vec &src, const qglviewer::Frame *frame) const
{
	GLdouble modelViewMatrix_[16], projectionMatrix_[16];
	GLdouble x, y, z;
	GLint viewport[4];
	getModelViewMatrix(modelViewMatrix_);
	getProjectionMatrix(projectionMatrix_);
	getViewport(viewport);
	for (int i = 0; i < 4; ++i)
	{
		viewport[i] *= devicePixelRatioF;
	}

	gluUnProject(src.x, src.y, src.z, modelViewMatrix_, projectionMatrix_,
		viewport, &x, &y, &z);
	if (frame)
		return frame->coordinatesOf(qglviewer::Vec(x, y, z));
	else
		return qglviewer::Vec(x, y, z);
}
qglviewer::Vec Camera::projectedCoordinatesOf(
	const qglviewer::Vec &src, const qglviewer::Frame *frame) const
{
	GLdouble modelViewMatrix_[16], projectionMatrix_[16];
	GLdouble x, y, z;
	GLint viewport[4];
	getModelViewMatrix(modelViewMatrix_);
	getProjectionMatrix(projectionMatrix_);
	getViewport(viewport);
	for (int i = 0; i < 4; ++i)
	{
		viewport[i] *= devicePixelRatioF;
	}
	if (frame) {
		const qglviewer::Vec tmp = frame->inverseCoordinatesOf(src);
		gluProject(tmp.x, tmp.y, tmp.z, modelViewMatrix_, projectionMatrix_,
			viewport, &x, &y, &z);
	}
	else
		gluProject(src.x, src.y, src.z, modelViewMatrix_, projectionMatrix_,
			viewport, &x, &y, &z);

	return qglviewer::Vec(x, y, z);
}