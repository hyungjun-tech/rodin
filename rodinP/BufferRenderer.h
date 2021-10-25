#pragma once
#include "MVPMatrices.h"
#include "VAO.h"

class BufferRenderer
{
public:
	BufferRenderer();
	~BufferRenderer();

	void draw(VAO* vao_, int n_vertices_, MVPMatrices mvp_);
	void draw(VAO* vao_, int n_vertices_, MVPMatrices mvp_, GLenum mode_);
	void setColor(float r_, float g_, float b_, float a_ = 1.0);
	void setClipping(float h_);
	void setLayerColors(std::vector<QVector3D> layerColors_);
	void setLayerHeights(std::vector<float> layerHeights_);
	void loadShader(QString vertFile_, QString fragFile_);
	void activateUniforms();
	void updateMatrices(MVPMatrices mvp_);

	void updateModelMatrix(QMatrix4x4 mat_);
	void updateViewMatrix(QMatrix4x4 mat_);
	void updateProjectionMatrix(QMatrix4x4 mat_);
	void updateLayerColors(QVector3D layerColors_[], int length_);
	void updateLayerHeights(float layerHeights_[], int length_);
	void updatePlaneEquation(QVector4D eq_);
	QOpenGLShaderProgram* getProgram() { return program; }

	QOpenGLShaderProgram *program;

private:
	int modelMatLoc;
	int viewMatLoc;
	int projMatLoc;
	int colorLoc;
	int layerCountLoc;
	int layerColorsLoc;
	int layerHeighsLoc;
	int planeLoc;

	void updateModelColor(QVector4D clr_);
};

class VertexColorBufferRenderer : public BufferRenderer
{
public:
	VertexColorBufferRenderer();
};

class SimpleBufferRenderer : public BufferRenderer
{
public:
	SimpleBufferRenderer();
};

class PhongBufferRenderer : public BufferRenderer
{
public:
	PhongBufferRenderer();
};

class VertexNormalBufferRenderer : public BufferRenderer
{
public:
	VertexNormalBufferRenderer();
};