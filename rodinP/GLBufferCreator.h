#pragma once
#include "Mesh.h"

class GLBufferCreator
{
public:
	GLBufferCreator();
	virtual ~GLBufferCreator();

	virtual QOpenGLBuffer* operator()(Mesh *mesh_, 
		QOpenGLShaderProgram& program_) = 0;
	virtual QOpenGLBuffer* operator()(std::vector<Mesh*>& meshes_,
		QOpenGLShaderProgram& program_) = 0;
	virtual QOpenGLBuffer* operator()(std::vector<float> points_,
		QOpenGLShaderProgram& program_) = 0;
	virtual std::vector<float> getPoints(Mesh *mesh_) = 0;
	virtual std::vector<float> getPoints(std::vector<Mesh*>& meshes_) = 0;
	virtual QOpenGLBuffer* getBuffer(std::vector<float> points_,
		QOpenGLShaderProgram& program_) = 0;
};

class LineVertexBufferCreator : public GLBufferCreator
{
public:
	LineVertexBufferCreator();
	virtual ~LineVertexBufferCreator();

	virtual QOpenGLBuffer* operator()(Mesh *mesh_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<Mesh*>& meshes_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
	virtual std::vector<float> getPoints(Mesh *mesh_);
	virtual std::vector<float> getPoints(std::vector<Mesh*>& meshes_);
	virtual QOpenGLBuffer* getBuffer(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
};

class TriangleVertexBufferCreator : public GLBufferCreator
{
public:
	TriangleVertexBufferCreator();
	virtual ~TriangleVertexBufferCreator();

	virtual QOpenGLBuffer* operator()(Mesh *mesh_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<Mesh*>& meshes_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
	virtual std::vector<float> getPoints(Mesh *mesh_);
	virtual std::vector<float> getPoints(std::vector<Mesh*>& meshes_);
	virtual QOpenGLBuffer* getBuffer(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
};

class TriangleNormalBufferCreator : public GLBufferCreator
{
public:
	TriangleNormalBufferCreator();
	virtual ~TriangleNormalBufferCreator();

	virtual QOpenGLBuffer* operator()(Mesh *mesh_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<Mesh*>& meshes_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
	virtual std::vector<float> getPoints(Mesh *mesh_);
	virtual std::vector<float> getPoints(std::vector<Mesh*>& meshes_);
	virtual QOpenGLBuffer* getBuffer(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
};

class TriangleColorBufferCreator : public GLBufferCreator
{
public:
	TriangleColorBufferCreator();
	virtual ~TriangleColorBufferCreator();

	virtual QOpenGLBuffer* operator()(Mesh *mesh_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<Mesh*>& meshes_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
	virtual std::vector<float> getPoints(Mesh *mesh_);
	virtual std::vector<float> getPoints(std::vector<Mesh*>& meshes_);
	virtual QOpenGLBuffer* getBuffer(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
};

class LineColorBufferCreator : public GLBufferCreator
{
public:
	LineColorBufferCreator();
	virtual ~LineColorBufferCreator();

	virtual QOpenGLBuffer* operator()(Mesh *mesh_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<Mesh*>& meshes_,
		QOpenGLShaderProgram& program_);
	virtual QOpenGLBuffer* operator()(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
	virtual std::vector<float> getPoints(Mesh *mesh_);
	virtual std::vector<float> getPoints(std::vector<Mesh*>& meshes_);
	virtual QOpenGLBuffer* getBuffer(std::vector<float> points_,
		QOpenGLShaderProgram& program_);
};