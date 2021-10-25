#pragma once
#include "Mesh.h"
#include "GLBufferCreator.h"

class VAO
{
public:
	VAO(QOpenGLShaderProgram &program_);
	virtual ~VAO();

	void VAOCreate();
	void bind();
	void release();
	void setVertexBuffer(QOpenGLBuffer* buffer_);
	void setColorBuffer(QOpenGLBuffer* buffer_);
	void setBuffers(QOpenGLBuffer* vertexBuffer_, QOpenGLBuffer* colorBuffer_ = nullptr);

	virtual void create(Mesh *mesh_) = 0;
	virtual void create( std::vector<Mesh*>& meshes_) = 0;
	virtual void createVertexBuffer(std::vector<float> points_) = 0;
	virtual void createColorBuffer(std::vector<float> points_) = 0;

	QOpenGLShaderProgram &program;
private:
	QOpenGLVertexArrayObject *vao;
	QOpenGLBuffer *vertexBuffer, *colorBuffer;
};

class LineVAO : public VAO
{
public:
	LineVAO(QOpenGLShaderProgram &program_);
	~LineVAO();

	virtual void create(Mesh *mesh_);
	virtual void create(std::vector<Mesh*>& meshes_);
	virtual void createVertexBuffer(std::vector<float> points_);
	virtual void createColorBuffer(std::vector<float> points_) {}

private:
	QOpenGLBuffer* buffer;
};

class LineColorVAO : public VAO
{
public:
	LineColorVAO(QOpenGLShaderProgram &program_);
	~LineColorVAO();

	virtual void create(Mesh *mesh_);
	virtual void create(std::vector<Mesh*>& meshes_);
	virtual void createVertexBuffer(std::vector<float> points_);
	virtual void createColorBuffer(std::vector<float> points_);
};

class V3TriangleVAO : public VAO
{
public:
	V3TriangleVAO(QOpenGLShaderProgram &program_);
	~V3TriangleVAO();

	virtual void create(Mesh *mesh_);
	virtual void create(std::vector<Mesh*>& meshes_);
	virtual void createVertexBuffer(std::vector<float> points_) {}
	virtual void createColorBuffer(std::vector<float> points_) {}
};

class V3N3TriangleVAO : public VAO
{
public:
	V3N3TriangleVAO(QOpenGLShaderProgram &program_);
	~V3N3TriangleVAO();

	virtual void create(Mesh *mesh_);
	virtual void create(std::vector<Mesh*>& meshes_);
	virtual void createVertexBuffer(std::vector<float> points_) {}
	virtual void createColorBuffer(std::vector<float> points_) {}
};

class V3C3TriangleVAO : public VAO
{
public:
	V3C3TriangleVAO(QOpenGLShaderProgram &program_);
	~V3C3TriangleVAO();

	virtual void create(Mesh *mesh_);
	virtual void create(std::vector<Mesh*>& meshes_);
	virtual void createVertexBuffer(std::vector<float> points_) {}
	virtual void createColorBuffer(std::vector<float> points_);
};