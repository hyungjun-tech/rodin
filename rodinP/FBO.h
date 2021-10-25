#pragma once

#include "ViewerModule.h"

class FBO
{
public:
	FBO();
	~FBO();

	void bind();
	void release();

	void initialize(int cols_, int rows_, float ratio_ = 1.0);
	void resize(int cols_, int rows_, float ratio_ = 1.0);

	float getDepth(QPoint cursor_);
	qglviewer::Vec getColor(QPoint cursor_);
	//cv::Mat getColor(int x_, int y_, int width_, int height_);
	QImage toImage() { return fbo->toImage(); }
private:
	QOpenGLFramebufferObject *fbo;
	int w, h;
	float ratio;
};