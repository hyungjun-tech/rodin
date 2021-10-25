#pragma once
#include "Coords.h"

class CameraModule : public qglviewer::Camera
{
public:
	CameraModule();
	CameraModule(qglviewer::Vec view_direction_, qglviewer::Vec up_, qglviewer::Camera::Type type_);
	CameraModule(const CameraModule &cam_);
	~CameraModule() {}

	CameraModule& operator=(const CameraModule& cam_);

	void setPosition(qglviewer::Vec position_)
	{
		qglviewer::Camera::setPosition(position_);
	}
	void setFrustrumSize(float width_, float height_)
	{
		halfWidth = width_ / 2.0;
		halfHeight = height_ / 2.0;
	}
	void setScreenWidthAndHeight(int cols_, int rows_)
	{
		qglviewer::Camera::setScreenWidthAndHeight(cols_, rows_);
	}
	void setClippingDistance(float far_, float near_)
	{
		farClip = far_;
		nearClip = near_;
	}
	void setX0Y0Z0(float posX_, float posY_, float posZ_)
	{
		posX = posX_;
		posY = posY_;
		posZ = posZ_;
	}

	qglviewer::Vec WorldToCamera(const qglviewer::Vec& wol_) const;
	qglviewer::Vec CameraToWorld(const qglviewer::Vec& vol_) const;

	static float clipLength;
	void setCurrentValue(float currentX_, float currentY_, float currentZ_)
	{
		currentZ = currentZ_;
		currentX = currentX_;
		currentY = currentY_;
	}
private:
	float nearClip, farClip;
	virtual qreal zNear() const { return nearClip; }
	virtual qreal zFar() const { return farClip; }

	float halfWidth, halfHeight;
	float posX, posY, posZ, currentX, currentY, currentZ;
	virtual void getOrthoWidthHeight(GLdouble &halfWidth_,
		GLdouble &halfHeight_) const
	{
		halfHeight_ = halfHeight;
		halfWidth_ = halfWidth;
	}

	qglviewer::Vec unprojectedCoordinatesOf(
		const qglviewer::Vec &src, const qglviewer::Frame *frame = NULL) const;
	qglviewer::Vec projectedCoordinatesOf(
		const qglviewer::Vec &src, const qglviewer::Frame *frame = NULL) const;

};

class Camera : public qglviewer::Camera
{
public:
	Camera();
	Camera(float _devicePixelRatio);
	qglviewer::Vec unprojectedCoordinatesOf(
		const qglviewer::Vec &src, const qglviewer::Frame *frame = NULL) const;
	qglviewer::Vec projectedCoordinatesOf(
		const qglviewer::Vec &src, const qglviewer::Frame *frame = NULL) const;
	void setDevicePixelRatioF(float _devicePixelRatioF) { devicePixelRatioF = _devicePixelRatioF; }
private:
	float devicePixelRatioF;
};