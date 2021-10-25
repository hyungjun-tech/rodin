#include "stdafx.h"
#include "ManipulatorHandle.h"

#include "MeshLoader.h"
#include "PickingIdxManager.h"

// Class for Translation Handles --------------------------------------------------------------------
TranslationHandle::TranslationHandle(ManipulatedFrame *manipulatedFrame_)
	: mesh(nullptr),
	matteRenderer(new NonShadedRenderer()),
	phongRenderer(new PhongShadedRenderer())
{
	scaledFrame.frame.setReferenceFrame(manipulatedFrame_);

	MeshLoader loader;
	mesh = loader.loadFirstMesh(Generals::appPath + "/mesh/cone.stl");
	assert(mesh);
	phongRenderer->setColor(0.2, 0.2, 0.2, 0.75);

	qglviewer::Vec fboColor = PickingIdxManager::encodeID(PickingIdxManager::TRANS_Z_ID);
	matteRenderer->setColor(fboColor[0], fboColor[1], fboColor[2]);
}

TranslationHandle::~TranslationHandle()
{
	delete matteRenderer;
	delete phongRenderer;

	delete mesh;
}

void TranslationHandle::setRelativePosition(float dx_, float dy_, float dz_)
{
	scaledFrame.frame.setTranslation(dx_, dy_, dz_);
}

void TranslationHandle::setScale(float factor_)
{
	scaledFrame.scale = qglviewer::Vec(factor_, factor_, factor_);
}

void TranslationHandle::drawForColorPicking(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	matteRenderer->draw(mesh, mvp);
}

void TranslationHandle::drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	glFuncs->glEnable(GL_CULL_FACE);
	glFuncs->glCullFace(GL_BACK);
	phongRenderer->draw(mesh, mvp);
	glFuncs->glDisable(GL_CULL_FACE);
}

// Class for Rotation Handles -----------------------------------------------------------------------
RotationHandles::RotationHandles(ManipulatedFrame *manipulatedFrame_)
	: meshX(nullptr)
	, meshY(nullptr)
	, meshZ(nullptr)
	, matteRendererX(new NonShadedRenderer())
	, matteRendererY(new NonShadedRenderer())
	, matteRendererZ(new NonShadedRenderer())
	, phongRendererX(new PhongShadedRenderer())
	, phongRendererY(new PhongShadedRenderer())
	, phongRendererZ(new PhongShadedRenderer())
	, circleRendererX(new LineRenderer())
	, circleRendererY(new LineRenderer())
	, circleRendererZ(new LineRenderer())
	, drawX(true)
	, drawY(true)
	, drawZ(true)
{
	scaledFrame.frame.setReferenceFrame(manipulatedFrame_);

	MeshLoader loader;
	//meshX = loader.Load(appPath + "/mesh/xrot.stl")[0];
	//meshY = loader.Load(appPath + "/mesh/yrot.stl")[0];
	//meshZ = loader.Load(appPath + "/mesh/zrot.stl")[0];
	meshX = loader.loadFirstMesh(Generals::appPath + "/mesh/xrot.stl");
	meshY = loader.loadFirstMesh(Generals::appPath + "/mesh/yrot.stl");
	meshZ = loader.loadFirstMesh(Generals::appPath + "/mesh/zrot.stl");

	qglviewer::Vec fboColorX = PickingIdxManager::encodeID(PickingIdxManager::ROT_X_ID);
	qglviewer::Vec fboColorZ = PickingIdxManager::encodeID(PickingIdxManager::ROT_Z_ID);
	qglviewer::Vec fboColorY = PickingIdxManager::encodeID(PickingIdxManager::ROT_Y_ID);

	matteRendererX->setColor(fboColorX[0], fboColorX[1], fboColorX[2]);
	matteRendererY->setColor(fboColorY[0], fboColorY[1], fboColorY[2]);
	matteRendererZ->setColor(fboColorZ[0], fboColorZ[1], fboColorZ[2]);

	circleRendererX->create(getCirclePoints(1.0, 1, 0, 0));
	circleRendererY->create(getCirclePoints(1.0, 0, 1, 0));
	circleRendererZ->create(getCirclePoints(1.0, 0, 0, 1));

	phongRendererX->setColor(0.65, 0.0, 0.0, 0.75);
	phongRendererY->setColor(0.0, 0.65, 0.0, 0.75);
	phongRendererZ->setColor(0.0, 0.0, 0.65, 0.75);
	circleRendererX->setColor(0.65, 0.0, 0.0, 0.75);
	circleRendererY->setColor(0.0, 0.65, 0.0, 0.75);
	circleRendererZ->setColor(0.0, 0.0, 0.65, 0.75);
}

RotationHandles::~RotationHandles()
{
	delete matteRendererX;
	delete matteRendererY;
	delete matteRendererZ;

	delete phongRendererX;
	delete phongRendererY;
	delete phongRendererZ;

	delete circleRendererX;
	delete circleRendererY;
	delete circleRendererZ;

	delete meshX;
	delete meshY;
	delete meshZ;
}

void RotationHandles::setDrawAxis(int axis_)
{
	drawX = drawY = drawZ = false;
	if (axis_ == 0)
		drawX = true;
	else if (axis_ == 1)
		drawY = true;
	else if (axis_ == 2)
		drawZ = true;

}
void RotationHandles::resetDrawAxis()
{
	drawX = drawY = drawZ = true;
}

void RotationHandles::setRelativeOrientation(qglviewer::Quaternion quaternion_)
{
	qglviewer::Vec axis = quaternion_.axis();
	float angle = quaternion_.angle();

	scaledFrame.rotateFrame(axis, angle);
}

void RotationHandles::setScale(float factor_)
{
	scaledFrame.scale = qglviewer::Vec(factor_, factor_, factor_);
}

void RotationHandles::drawForColorPicking(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	matteRendererX->draw(meshX, mvp);
	matteRendererY->draw(meshY, mvp);
	matteRendererZ->draw(meshZ, mvp);
}

void RotationHandles::drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	glFuncs->glEnable(GL_CULL_FACE);
	glFuncs->glCullFace(GL_BACK);
	if (drawX && drawY && drawZ)
	{
		phongRendererX->draw(meshX, mvp);
		phongRendererY->draw(meshY, mvp);
		phongRendererZ->draw(meshZ, mvp);
	}
	glFuncs->glDisable(GL_CULL_FACE);
	glLineWidth(2.0);

	if (drawX)
		circleRendererX->drawLineLoop(mvp);
	if (drawY)
		circleRendererY->drawLineLoop(mvp);
	if (drawZ)
		circleRendererZ->drawLineLoop(mvp);
}

#ifndef DEG2RAD
#define DEG2RAD M_PI/180.0
#endif
std::vector<float> RotationHandles::getCirclePoints(float _radius, int _axis_x, int _axis_y, int _axis_z)
{
	std::vector<float> rtn;
	for (int i = 0; i < 180; i++)
	{
		float degInRad = i * 2 * DEG2RAD;
		rtn.push_back((cos(degInRad) * _axis_z + sin(degInRad) * _axis_y) * _radius);
		rtn.push_back((cos(degInRad) * _axis_x + sin(degInRad) * _axis_z) * _radius);
		rtn.push_back((cos(degInRad) * _axis_y + sin(degInRad) * _axis_x) * _radius);
	}
	return rtn;
}

// Class for Support Handles ---------------------------------------------------------------------
float SupportHandle::supportPointRadius = 1.0f;
SupportHandle::SupportHandle()
	:sphere(nullptr),
	phongRenderer(new PhongShadedRenderer())
{
	MeshLoader loader;
	sphere = loader.loadFirstMesh(Generals::appPath + "/mesh/sphere.stl");
	assert(sphere);
	scaledFrame.setScale(qglviewer::Vec(
		supportPointRadius,
		supportPointRadius,
		supportPointRadius));

	phongRenderer->setColor(0.6, 0.6, 0.2, 0.8); // Yellow
}

SupportHandle::~SupportHandle()
{
	delete sphere;
	delete phongRenderer;
}

void SupportHandle::setPosition(qglviewer::Vec pos_)
{
	scaledFrame.frame.setPosition(pos_);
}

void SupportHandle::setScale(float scale_)
{
	scaledFrame.scale = qglviewer::Vec(scale_, scale_, scale_);
}

void SupportHandle::drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	glFuncs->glEnable(GL_CULL_FACE);
	glFuncs->glCullFace(GL_BACK);
	phongRenderer->draw(sphere, mvp);
	glFuncs->glDisable(GL_CULL_FACE);
}

// Class for Find Bottom Mesh Handles ----------------------------------------------------------------------
BottomMeshHandle::BottomMeshHandle()
//BottomMeshHandle::BottomMeshHandle(ManipulatedFrame *manipulatedFrame_)		// 2019 08 28
	:triangle(nullptr),
	phongRenderer(new PhongShadedRenderer())
{
	//	scaledFrame.frame.setReferenceFrame(manipulatedFrame_);		// 2019 08 28
	phongRenderer->setColor(0.6, 0.0, 1.0, 0.75);
}

BottomMeshHandle::~BottomMeshHandle()
{
	delete phongRenderer;
}

void BottomMeshHandle::drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	glFuncs->glDepthFunc(GL_ALWAYS);
	//	glFuncs->glEnable(GL_CULL_FACE);
	//	glFuncs->glCullFace(GL_BACK);
	phongRenderer->draw(triangle, mvp);
	//	glFuncs->glDisable(GL_CULL_FACE);
	glFuncs->glDepthFunc(GL_LESS);
}

void BottomMeshHandle::findTriangle(std::vector<qglviewer::Vec> _vertices)
{
	triangle = makeTriangle(_vertices);
}

Mesh* BottomMeshHandle::makeTriangle(std::vector<qglviewer::Vec> _vertices)
{
	// make a mesh
	Mesh *surface = new Mesh();

	Mesh::Point p0(_vertices[0][0], _vertices[0][1], _vertices[0][2]);
	Mesh::Point p1(_vertices[1][0], _vertices[1][1], _vertices[1][2]);
	Mesh::Point p2(_vertices[2][0], _vertices[2][1], _vertices[2][2]);

	std::vector<Mesh::VertexHandle> vhs;
	Mesh::VertexHandle vhs0 = surface->add_vertex(p0);
	Mesh::VertexHandle vhs1 = surface->add_vertex(p1);
	Mesh::VertexHandle vhs2 = surface->add_vertex(p2);

	Mesh::FaceHandle face_vhandle;
	//	assert(surface->add_face(vhs0, vhs1, vhs2) != Mesh::InvalidFaceHandle);
	surface->add_face(vhs0, vhs1, vhs2);

	surface->update_normals();

#ifdef _DEBUG
	int n = surface->n_faces();
	int m = surface->n_vertices();
#endif

	return surface;
}

// Class for Selection Handles --------------------------------------------------------------------
SelectionHandle::SelectionHandle()
	: mesh(nullptr),
	matteRenderer(new NonShadedRenderer()),
	phongRenderer(new PhongShadedRenderer())
{
	MeshLoader loader;
	mesh = loader.loadFirstMesh(Generals::appPath + "/mesh/selection.stl");
	assert(mesh);
	//phongRenderer->setColor(0.6, 0.0, 0.0, 1.0);
	phongRenderer->setColor(0.6, 0.2, 0.0, 1.0);

	qglviewer::Vec fboColor = PickingIdxManager::encodeID(PickingIdxManager::TRANS_XY_ID);
	matteRenderer->setColor(fboColor[0], fboColor[1], fboColor[2]);
	//SetScale(0.7);
}

SelectionHandle::~SelectionHandle()
{
	delete matteRenderer;
	delete phongRenderer;

	delete mesh;
}
void SelectionHandle::setRelativePosition(float dx_, float dy_, float dz_)
{
	scaledFrame.frame.setPosition(dx_, dy_, dz_);
}
void SelectionHandle::moveFrame(float dx_, float dy_, float dz_)
{
	scaledFrame.frame.translate(qglviewer::Vec(dx_, dy_, dz_));
}

void SelectionHandle::setScale(float factor_)
{
	scaledFrame.scale = qglviewer::Vec(factor_, factor_, factor_);
}

void SelectionHandle::refreshHandle(AABB aabb_)
{
	float minValue = std::fmin(aabb_.getLengthX(), aabb_.getLengthY());
	float scale = minValue / 80;
	if (scale < 0.5) scale = 0.5;
	else if (scale > 1.5) scale = 1.5;

	qglviewer::Vec center = aabb_.getCenter();
	setScale(scale);
	setRelativePosition(center.x - 7.5 * scale, center.y - 7.5 * scale, center.z * 2 + 3);
}

void SelectionHandle::drawForColorPicking(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	matteRenderer->draw(mesh, mvp);
}
void SelectionHandle::drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	glFuncs->glEnable(GL_CULL_FACE);
	glFuncs->glCullFace(GL_BACK);
	phongRenderer->draw(mesh, mvp);
	glFuncs->glDisable(GL_CULL_FACE);
}