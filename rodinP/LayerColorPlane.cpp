#include "stdafx.h"
#include "LayerColorPlane.h"

LayerColorPlane::LayerColorPlane()
	: plane(nullptr)
	, planeBoundary(nullptr)
	, planeRenderer(nullptr)
	, planeBoundaryRenderer(nullptr)
{
	planeRenderer = new LineRenderer();
	planeRenderer->setColor(0.5, 0.5, 0.5, 0.5);
	planeBoundaryRenderer = new LineColorRenderer();
	createMesh();
	moveFrame(-1, -1);
}

LayerColorPlane::LayerColorPlane(ScaledFrame scaledFrame_)
	: plane(nullptr)
	, planeBoundary(nullptr)
	, planeRenderer(nullptr)
	, planeBoundaryRenderer(nullptr)
	, scaledFrame(scaledFrame_)
{
	planeRenderer = new NonShadedRenderer();
	planeRenderer->setColor(0.5, 0.5, 0.5, 0.5);
	planeBoundaryRenderer = new LineColorRenderer();
	scaledFrame.frame.setReferenceFrame(&scaledFrame_.frame);
	createMesh();
	//SetZ(10);
}
LayerColorPlane::~LayerColorPlane()
{
	delete plane;
	delete planeBoundary;
	delete planeRenderer;
	delete planeBoundaryRenderer;
}
void LayerColorPlane::setZ(float z)
{
	qglviewer::Vec pos = scaledFrame.frame.position();
	scaledFrame.moveFrame(0, 0, z - pos.z);
}
void LayerColorPlane::setPosition(qglviewer::Vec position_)
{
	qglviewer::Vec pos = scaledFrame.frame.position();
	qglviewer::Vec move = position_ - pos;
	scaledFrame.moveFrame(move.x, move.y, 0);
}
void LayerColorPlane::setScale(qglviewer::Vec scale_)
{
	scaledFrame.setScale(scale_ + qglviewer::Vec(2, 2, 0));
}
void LayerColorPlane::moveFrame(float x_, float y_)
{
	scaledFrame.moveFrame(x_, y_, 0);
}
void LayerColorPlane::createMesh()
{
	delete plane;
	plane = new Mesh();
	delete planeBoundary;
	planeBoundary = new Mesh();
	float margin = 0.5;

	//AABB aabb = model_->getAABB();
	//qglviewer::Vec min = aabb.GetMinimum();
	//qglviewer::Vec max = aabb.GetMaximum();
	qglviewer::Vec min = qglviewer::Vec(0, 0, 0);
	qglviewer::Vec max = qglviewer::Vec(0, 0, 0);// aabb.GetMaximum() - aabb.GetMinimum();

	Mesh::Point p0(min.x - margin, min.y - margin, 0);
	Mesh::Point p1(max.x + margin, min.y - margin, 0);
	Mesh::Point p2(max.x + margin, max.y + margin, 0);
	Mesh::Point p3(min.x - margin, max.y + margin, 0);

	std::vector<Mesh::VertexHandle> vhs;
	vhs.push_back(plane->add_vertex(p0));
	vhs.push_back(plane->add_vertex(p1));
	vhs.push_back(plane->add_vertex(p2));
	vhs.push_back(plane->add_vertex(p3));

	planeRenderer->create(plane);

	planeBoundary->add_vertex(p0);
	planeBoundary->add_vertex(p1);
	planeBoundary->add_vertex(p2);
	planeBoundary->add_vertex(p3);
	Mesh::Color c(0.0, 0.0, 0.0);
	for (auto it : vhs)
	{
		planeBoundary->set_color(it, c);
	}
	planeBoundaryRenderer->create(planeBoundary);
}

void LayerColorPlane::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);
	glFuncs->glDepthMask(GL_FALSE);
	glFuncs->glLineWidth(1.0);
	planeRenderer->drawPolygon(mvp);
	planeBoundaryRenderer->drawLineLoop(mvp);
	glFuncs->glDepthMask(GL_TRUE);
}