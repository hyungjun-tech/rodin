#pragma once

class ProjectionMatrixGetter
{
public:
	QMatrix4x4 operator()(const qglviewer::Camera& camera_)
	{
		GLfloat projArr[16];
		camera_.getProjectionMatrix(projArr);

		return QMatrix4x4(projArr).transposed();
	}
};
class ViewMatrixGetter
{
public:
	QMatrix4x4 operator()(const qglviewer::Camera& camera_)
	{
		GLfloat viewArr[16];
		camera_.getModelViewMatrix(viewArr);

		return QMatrix4x4(viewArr).transposed();
	}
};