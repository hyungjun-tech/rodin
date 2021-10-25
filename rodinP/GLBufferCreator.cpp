#include "stdafx.h"
#include "GLBufferCreator.h"

GLBufferCreator::GLBufferCreator()
{

}
GLBufferCreator::~GLBufferCreator()
{

}

//gl_begin(GL_LINES)와 유사
//mesh vertex array의 1쌍마다 하나의 라인을 만듬
LineVertexBufferCreator::LineVertexBufferCreator()
{
	
}

LineVertexBufferCreator::~LineVertexBufferCreator()
{
	
}

QOpenGLBuffer *LineVertexBufferCreator::operator()(Mesh *mesh_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(mesh_), program_);
}

QOpenGLBuffer *LineVertexBufferCreator::operator()(std::vector<Mesh*>& meshes_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(meshes_), program_);
}
QOpenGLBuffer *LineVertexBufferCreator::operator()(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(points_, program_);
}
std::vector<float> LineVertexBufferCreator::getPoints(Mesh *mesh_)
{
	std::vector<float> points;
	for (Mesh::VertexIter vit = mesh_->vertices_begin();
		vit != mesh_->vertices_end();
		vit++)
	{
		Mesh::Point p = mesh_->point(*vit);

		for (int i = 0; i < 3; i++)
			points.push_back(p[i]);
	}
	return points;
}
std::vector<float> LineVertexBufferCreator::getPoints(std::vector<Mesh*>& meshes_)
{
	std::vector<float> points;
	for (auto it : meshes_)
	{
		std::vector<float> temp = getPoints(it);
		points.insert(points.end(), temp.begin(), temp.end());
	}
	return points;
}
QOpenGLBuffer* LineVertexBufferCreator::getBuffer(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	QOpenGLBuffer* buffer = new QOpenGLBuffer();
	buffer->create();
	buffer->bind();
	buffer->allocate(points_.data(), points_.size() * sizeof(float));
	program_.enableAttributeArray(0);
	program_.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
	buffer->release();
	return buffer;
}

TriangleVertexBufferCreator::TriangleVertexBufferCreator()
{
	
}

TriangleVertexBufferCreator::~TriangleVertexBufferCreator()
{
	
}

QOpenGLBuffer* TriangleVertexBufferCreator::operator()(Mesh *mesh_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(mesh_), program_);
}
QOpenGLBuffer *TriangleVertexBufferCreator::operator()(std::vector<Mesh*>& meshes_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(meshes_), program_);
}
QOpenGLBuffer *TriangleVertexBufferCreator::operator()(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(points_, program_);
}
std::vector<float> TriangleVertexBufferCreator::getPoints(Mesh *mesh_)
{
	std::vector<float> points;
	for (Mesh::FaceIter fit = mesh_->faces_begin();
		fit != mesh_->faces_end();
		fit++)
	{
		for (Mesh::FaceVertexIter fvit = mesh_->fv_begin(*fit);
			fvit != mesh_->fv_end(*fit);
			fvit++)
		{
			Mesh::Point p = mesh_->point(*fvit);
			for (int i = 0; i < 3; i++)
				points.push_back(p[i]);
		}
	}
	return points;
}
std::vector<float> TriangleVertexBufferCreator::getPoints(std::vector<Mesh*>& meshes_)
{
	std::vector<float> points;
	for (auto it : meshes_)
	{
		std::vector<float> temp = getPoints(it);
		points.insert(points.end(), temp.begin(), temp.end());
	}
	return points;
}
QOpenGLBuffer* TriangleVertexBufferCreator::getBuffer(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	QOpenGLBuffer* buffer = new QOpenGLBuffer();
	buffer->create();
	buffer->bind();
	buffer->allocate(points_.data(), points_.size() * sizeof(float));
	program_.enableAttributeArray(0);
	program_.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
	buffer->release();
	return buffer;
}

TriangleNormalBufferCreator::TriangleNormalBufferCreator()
{

}

TriangleNormalBufferCreator::~TriangleNormalBufferCreator()
{

}

QOpenGLBuffer* TriangleNormalBufferCreator::operator()(Mesh *mesh_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(mesh_), program_);
}
QOpenGLBuffer *TriangleNormalBufferCreator::operator()(std::vector<Mesh*>& meshes_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(meshes_), program_);
}
QOpenGLBuffer *TriangleNormalBufferCreator::operator()(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(points_, program_);
}
std::vector<float> TriangleNormalBufferCreator::getPoints(Mesh *mesh_)
{
	std::vector<float> normals;
	for (Mesh::FaceIter fit = mesh_->faces_begin();
		fit != mesh_->faces_end();
		fit++)
	{
		//for (Mesh::FaceVertexIter fvit = mesh_->fv_begin(*fit);
		//	fvit != mesh_->fv_end(*fit);
		//	fvit++)
		//{
		//	Mesh::Normal n = mesh_->normal(*fvit);
		//	for (int i = 0; i < 3; i++)
		//		normals.push_back(n[i]);
		//}
		Mesh::Normal n = mesh_->normal(*fit);
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				normals.push_back(n[j]);
	}
	return normals;
}
std::vector<float> TriangleNormalBufferCreator::getPoints(std::vector<Mesh*>& meshes_)
{
	std::vector<float> points;
	for (auto it : meshes_)
	{
		std::vector<float> temp = getPoints(it);
		points.insert(points.end(), temp.begin(), temp.end());
	}
	return points;
}
QOpenGLBuffer* TriangleNormalBufferCreator::getBuffer(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	QOpenGLBuffer* buffer = new QOpenGLBuffer();
	buffer->create();
	buffer->bind();
	buffer->allocate(points_.data(), points_.size() * sizeof(float));
	program_.enableAttributeArray(1);
	program_.setAttributeBuffer(1, GL_FLOAT, 0, 3, 0);
	buffer->release();
	return buffer;
}

TriangleColorBufferCreator::TriangleColorBufferCreator()
{

}

TriangleColorBufferCreator::~TriangleColorBufferCreator()
{

}

QOpenGLBuffer* TriangleColorBufferCreator::operator()(Mesh *mesh_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(mesh_), program_);
}
QOpenGLBuffer *TriangleColorBufferCreator::operator()(std::vector<Mesh*>& meshes_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(meshes_), program_);
}
QOpenGLBuffer *TriangleColorBufferCreator::operator()(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(points_, program_);
}
std::vector<float> TriangleColorBufferCreator::getPoints(Mesh *mesh_)
{
	std::vector<float> colors;
	OpenMesh::VPropHandleT<float> alpha;
	bool check = mesh_->get_property_handle(alpha, "alpha");

	for (Mesh::FaceIter fit = mesh_->faces_begin();
		fit != mesh_->faces_end();
		fit++)
	{
		for (Mesh::FaceVertexIter fvit = mesh_->fv_begin(*fit);
			fvit != mesh_->fv_end(*fit);
			fvit++)
		{
			Mesh::Point n = mesh_->color(*fvit);
			for (int i = 0; i < 3; i++)
				colors.push_back(n[i]);

			if (check) colors.push_back(mesh_->property(alpha, *fvit));
			else colors.push_back(1.0f);
		}
	}
	return colors;
}
std::vector<float> TriangleColorBufferCreator::getPoints(std::vector<Mesh*>& meshes_)
{
	std::vector<float> points;
	for (auto it : meshes_)
	{
		std::vector<float> temp = getPoints(it);
		points.insert(points.end(), temp.begin(), temp.end());
	}
	return points;
}
QOpenGLBuffer* TriangleColorBufferCreator::getBuffer(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	QOpenGLBuffer* buffer = new QOpenGLBuffer();
	buffer->create();
	buffer->bind();
	buffer->allocate(points_.data(), points_.size() * sizeof(float));
	program_.enableAttributeArray(2);
	program_.setAttributeBuffer(2, GL_FLOAT, 0, 4, 0);
	buffer->release();
	return buffer;
}

LineColorBufferCreator::LineColorBufferCreator()
{

}

LineColorBufferCreator::~LineColorBufferCreator()
{

}

QOpenGLBuffer* LineColorBufferCreator::operator()(Mesh *mesh_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(mesh_), program_);
}

QOpenGLBuffer* LineColorBufferCreator::operator()(std::vector<Mesh*>& meshes_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(getPoints(meshes_), program_);
}
QOpenGLBuffer *LineColorBufferCreator::operator()(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	return getBuffer(points_, program_);
}
std::vector<float> LineColorBufferCreator::getPoints(Mesh *mesh_)
{
	std::vector<float> colors;
	OpenMesh::VPropHandleT<float> alpha;
	bool check = mesh_->get_property_handle(alpha, "alpha");
	for (Mesh::VertexIter vit = mesh_->vertices_begin();
		vit != mesh_->vertices_end();
		vit++)
	{
		Mesh::Point n = mesh_->color(*vit);
		for (int i = 0; i < 3; i++)
		{
			colors.push_back(n[i]);
		}
		if (check) colors.push_back(mesh_->property(alpha, *vit));
		else colors.push_back(1.0f);
	}

	return colors;
}
std::vector<float> LineColorBufferCreator::getPoints(std::vector<Mesh*>& meshes_)
{
	std::vector<float> points;
	for (auto it : meshes_)
	{
		std::vector<float> temp = getPoints(it);
		points.insert(points.end(), temp.begin(), temp.end());
	}
	return points;
}
QOpenGLBuffer* LineColorBufferCreator::getBuffer(std::vector<float> points_,
	QOpenGLShaderProgram& program_)
{
	QOpenGLBuffer* buffer = new QOpenGLBuffer();
	buffer->create();
	buffer->bind();
	buffer->allocate(points_.data(), points_.size() * sizeof(float));
	program_.enableAttributeArray(2);
	program_.setAttributeBuffer(2, GL_FLOAT, 0, 4, 0);
	buffer->release();
	return buffer;
}