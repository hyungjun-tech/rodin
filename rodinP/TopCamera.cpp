#include "stdafx.h"
#include "TopCamera.h"

float TopCamera::clipLength = 0.01;
TopCamera::TopCamera()
{
	setType(qglviewer::Camera::ORTHOGRAPHIC);
	setViewDirection(qglviewer::Vec(0, 0, -1));
	setUpVector(qglviewer::Vec(0, 1, 0));
	posX = 0;
	posY = 0;
	posZ = 0;
}
TopCamera::TopCamera(const TopCamera &cam_)
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

TopCamera& TopCamera::operator=(const TopCamera& cam_)
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

qglviewer::Vec TopCamera::worldToCamera(const qglviewer::Vec& wol_) const
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
qglviewer::Vec TopCamera::cameraToWorld(const qglviewer::Vec& vol_) const
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

qglviewer::Vec TopCamera::unprojectedCoordinatesOf(
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
qglviewer::Vec TopCamera::projectedCoordinatesOf(
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