#include "stdafx.h"
#include "FBO.h"


FBO::FBO()
	: fbo(nullptr),
	w(0),
	h(0),
	ratio(1.0)
{

}
FBO::~FBO()
{
	delete fbo;
}

void FBO::bind()
{
	fbo->bind();
}
void FBO::release()
{
	fbo->release();
}

void FBO::initialize(int cols_, int rows_, float ratio_)
{
	w = cols_ * ratio_;
	h = rows_ * ratio_;
	ratio = ratio_;

	fbo = new QOpenGLFramebufferObject(w, h, QOpenGLFramebufferObject::CombinedDepthStencil);
	release();
}

void FBO::resize(int cols_, int rows_, float ratio_)
{
	if (fbo == nullptr) return;

	w = cols_ * ratio_;
	h = rows_ * ratio_;
	ratio = ratio_;

	delete fbo;
	fbo = new QOpenGLFramebufferObject(w, h, QOpenGLFramebufferObject::Depth);
	release();
}

float FBO::getDepth(QPoint cursor_)
{
	bind();

	float depth;
	glReadPixels(cursor_.x(), h - ratio - cursor_.y(),
		1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	release();

	return depth;
}

qglviewer::Vec FBO::getColor(QPoint cursor_)
{
	bind();

	float color[3];
	glReadPixels(cursor_.x(), h - ratio - cursor_.y(),
		1, 1, GL_RGB, GL_FLOAT, color);

	release();

	return qglviewer::Vec(color[0], color[1], color[2]);
}

/*cv::Mat FBO::getColor(int x_, int y_, int width_, int height_)
{
	cv::Mat color = cv::Mat(height_, width_, CV_32FC3);

	bind();
	glReadPixels(x_, h - 1 - y_,
		width_, height_, GL_RGB, GL_FLOAT, color.data);
	release();

	return color;
}*/