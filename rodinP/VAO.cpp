#include "stdafx.h"
#include "VAO.h"
#include "GLBufferCreator.h"

/// Interface Class --------------------------------------------------------------------------------
VAO::VAO(QOpenGLShaderProgram &program_)
	: vao(new QOpenGLVertexArrayObject())
	, program(program_)
	, vertexBuffer(nullptr)
	, colorBuffer(nullptr)
{
	VAOCreate();
}
VAO::~VAO()
{
	delete vao;
	delete vertexBuffer;
	delete colorBuffer;
}

void VAO::VAOCreate()
{
	if (vao->isCreated())
		vao->destroy();

	vao->create();
}

void VAO::bind()
{
	vao->bind();
}

void VAO::release()
{
	vao->release();
}
void VAO::setVertexBuffer(QOpenGLBuffer* buffer_)
{
	if (buffer_ == nullptr)
		return;
	if (vertexBuffer != nullptr)
		delete vertexBuffer;
	vertexBuffer = buffer_;
}
void VAO::setColorBuffer(QOpenGLBuffer* buffer_)
{
	if (buffer_ == nullptr)
		return;
	if (colorBuffer != nullptr)
		delete colorBuffer;
	colorBuffer = buffer_;
}
void VAO::setBuffers(QOpenGLBuffer* vertexBuffer_, QOpenGLBuffer* colorBuffer_)
{
	setVertexBuffer(vertexBuffer_);
	setColorBuffer(colorBuffer_);
}

/// LineVAO Class ------------------------------------------------------------------------------------
LineVAO::LineVAO(QOpenGLShaderProgram &program_)
	: VAO(program_)
	, buffer(nullptr)
{
}

LineVAO::~LineVAO()
{
	delete buffer;
}

void LineVAO::create(Mesh *mesh_)
{
	bind();
	setVertexBuffer(LineVertexBufferCreator()(mesh_, program));
	release();
}
void LineVAO::create(std::vector<Mesh*>& meshes_)
{
	bind();
	setVertexBuffer(LineVertexBufferCreator()(meshes_, program));
	release();
}
void LineVAO::createVertexBuffer(std::vector<float> points_)
{
	bind();
	setVertexBuffer(LineVertexBufferCreator()(points_, program));
	release();
}

/// LineColorVAO Class ------------------------------------------------------------------------------------
LineColorVAO::LineColorVAO(QOpenGLShaderProgram &program_)
	: VAO(program_)
{
}

LineColorVAO::~LineColorVAO()
{
}

void LineColorVAO::create(Mesh *mesh_)
{
	bind();
	setVertexBuffer(LineVertexBufferCreator()(mesh_, program));
	setColorBuffer(LineColorBufferCreator()(mesh_, program));
	release();
}

void LineColorVAO::create(std::vector<Mesh*>& meshes_)
{
	bind();
	setVertexBuffer(LineVertexBufferCreator()(meshes_, program));
	setColorBuffer(LineColorBufferCreator()(meshes_, program));
	release();
}
void LineColorVAO::createVertexBuffer(std::vector<float> points_)
{
	bind();
	setVertexBuffer(LineColorBufferCreator()(points_, program));
	release();
}
void LineColorVAO::createColorBuffer(std::vector<float> points_)
{
	bind();
	setColorBuffer(LineColorBufferCreator()(points_, program));
	release();
}

/// V3TriangleVAO Class ------------------------------------------------------------------------------------
V3TriangleVAO::V3TriangleVAO(QOpenGLShaderProgram &program_)
	: VAO(program_)
{
}
V3TriangleVAO::~V3TriangleVAO()
{
}

void V3TriangleVAO::create(Mesh *mesh_)
{
	bind();
	setVertexBuffer(TriangleVertexBufferCreator()(mesh_, program));
	release();
}
void V3TriangleVAO::create(std::vector<Mesh*>& meshes_)
{
	bind();
	setVertexBuffer(TriangleVertexBufferCreator()(meshes_, program));
	release();
}

/// V3N3TriangleVAOClass ----------------------------------------------------------------------------------
V3N3TriangleVAO::V3N3TriangleVAO(QOpenGLShaderProgram &program_)
	: VAO(program_)
{
}

V3N3TriangleVAO::~V3N3TriangleVAO()
{
}

void V3N3TriangleVAO::create(Mesh *mesh_)
{
	bind();
	setVertexBuffer(TriangleVertexBufferCreator()(mesh_, program));
	setColorBuffer(TriangleNormalBufferCreator()(mesh_, program));
	release();
}
void V3N3TriangleVAO::create(std::vector<Mesh*>& meshes_)
{
	bind();
	setVertexBuffer(TriangleVertexBufferCreator()(meshes_, program));
	setColorBuffer(TriangleNormalBufferCreator()(meshes_, program));
	release();
}

/// V3C3TriangleVAO Class ----------------------------------------------------------------------------------
V3C3TriangleVAO::V3C3TriangleVAO(QOpenGLShaderProgram &program_)
	: VAO(program_)
{
}

V3C3TriangleVAO::~V3C3TriangleVAO()
{
}

void V3C3TriangleVAO::create(Mesh *mesh_)
{
	bind();
	setVertexBuffer(TriangleVertexBufferCreator()(mesh_, program));
	setColorBuffer(TriangleColorBufferCreator()(mesh_, program));
	release();
}
void V3C3TriangleVAO::create(std::vector<Mesh*>& meshes_)
{
	bind();
	setVertexBuffer(TriangleVertexBufferCreator()(meshes_, program));
	setColorBuffer(TriangleColorBufferCreator()(meshes_, program));
	release();
}
void V3C3TriangleVAO::createColorBuffer(std::vector<float> points_)
{
	bind();
	setColorBuffer(TriangleColorBufferCreator()(points_, program));
	release();
}