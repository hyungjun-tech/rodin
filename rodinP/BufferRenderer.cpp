#include "stdafx.h"
#include "BufferRenderer.h"

BufferRenderer::BufferRenderer()
	: program(nullptr)
	, modelMatLoc(-1)
	, viewMatLoc(-1)
	, projMatLoc(-1)
	, colorLoc(-1)
	, layerCountLoc(-1)
	, layerColorsLoc(-1)
	, layerHeighsLoc(-1)
	, planeLoc(-1)
{
	program = new QOpenGLShaderProgram();
}

BufferRenderer::~BufferRenderer()
{
	delete program;
}
void BufferRenderer::draw(VAO* vao_, int n_vertices_, MVPMatrices mvp_)
{
	draw(vao_, n_vertices_, mvp_, GL_LINES);
}
void BufferRenderer::draw(VAO* vao_, int n_vertices_, MVPMatrices mvp_, GLenum mode_)
{
	if (vao_ == nullptr)
		return;

	// Prepare to use OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	updateMatrices(mvp_);

	// Draw VAO
	program->bind();
	vao_->bind();

	glFuncs->glDrawArrays(mode_, 0, n_vertices_);

	vao_->release();
	program->release();
}
void BufferRenderer::setColor(float r_, float g_, float b_, float a_)
{
	updateModelColor(QVector4D(r_, g_, b_, a_));
}
void BufferRenderer::setClipping(float h_)
{
	updatePlaneEquation(QVector4D(0, 0, -1, h_));
}
void BufferRenderer::setLayerColors(std::vector<QVector3D> layerColors_)
{
	updateLayerColors(layerColors_.data(), layerColors_.size());
}
void BufferRenderer::setLayerHeights(std::vector<float> layerHeights_)
{
	updateLayerHeights(layerHeights_.data(), layerHeights_.size());
}
void BufferRenderer::loadShader(QString vertFile_, QString fragFile_)
{
	program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertFile_);
	program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragFile_);
	program->link();
}
void BufferRenderer::activateUniforms()
{
	program->bind();

	// Set Locations for Uniforms
	modelMatLoc = program->uniformLocation("model");
	viewMatLoc = program->uniformLocation("view");
	projMatLoc = program->uniformLocation("proj");
	colorLoc = program->uniformLocation("model_color");
	layerCountLoc = program->uniformLocation("layerCount");
	layerColorsLoc = program->uniformLocation("layerColors");
	layerHeighsLoc = program->uniformLocation("layerHeights");
	planeLoc = program->uniformLocation("clipPlane");

	//2020.06.03 opengl version이 낮은 경우 layout(locaion=x) in 문법을 사용하지 못해 location 지정을 위해 아래 코드 추가함.
	program->bindAttributeLocation("vertex_position", 0);
	program->bindAttributeLocation("vertex_normal", 1);
	program->bindAttributeLocation("vertex_color", 2);

	program->release();
}
void BufferRenderer::updateMatrices(MVPMatrices mvp_)
{
	// Prepare to use OpenGL Functions ...
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	// Update MVP Matrices
	updateProjectionMatrix(mvp_.projectionMatrix);
	updateViewMatrix(mvp_.viewMatrix);
	updateModelMatrix(mvp_.modelMatrix);
}
void BufferRenderer::updateModelMatrix(QMatrix4x4 mat_)
{
	program->bind();
	program->setUniformValue(modelMatLoc, mat_);
	program->release();
}
void BufferRenderer::updateViewMatrix(QMatrix4x4 mat_)
{
	program->bind();
	program->setUniformValue(viewMatLoc, mat_);
	program->release();
}
void BufferRenderer::updateProjectionMatrix(QMatrix4x4 mat_)
{
	program->bind();
	program->setUniformValue(projMatLoc, mat_);
	program->release();
}
void BufferRenderer::updateModelColor(QVector4D clr_)
{
	program->bind();
	program->setUniformValue(colorLoc, clr_);
	program->release();
}
void BufferRenderer::updateLayerColors(QVector3D layerColors_[], int length_)
{
	program->bind();
	program->setUniformValueArray(layerColorsLoc, layerColors_, 2);
	program->release();
}
void BufferRenderer::updateLayerHeights(float layerHeights_[], int length_)
{
	program->bind();
	program->setUniformValue(layerCountLoc, length_);
	program->setUniformValueArray(layerHeighsLoc, layerHeights_, length_, 1);
	program->release();
}
void BufferRenderer::updatePlaneEquation(QVector4D eq_)
{
	program->bind();
	program->setUniformValue(planeLoc, eq_);
	program->release();
}



// VertexColorBufferRenderer Renderer Class -----------------------------------------------------------------------
VertexColorBufferRenderer::VertexColorBufferRenderer()
{
	loadShader(Generals::appPath + "/shader/vertexColor.vert", Generals::appPath + "/shader/vertexColor.frag");
	activateUniforms();
}

//SimpleBufferRenderer
SimpleBufferRenderer::SimpleBufferRenderer()
{
	loadShader(Generals::appPath + "/shader/simple.vert", Generals::appPath + "/shader/simple.frag");
	activateUniforms();
}

//PhongBufferRenderer
PhongBufferRenderer::PhongBufferRenderer()
{
	loadShader(Generals::appPath + "/shader/phong.vert", Generals::appPath + "/shader/phong.frag");
	activateUniforms();
}

//VertexNormalBufferRenderer
VertexNormalBufferRenderer::VertexNormalBufferRenderer()
{
	loadShader(Generals::appPath + "/shader/vertexNormal.vert", Generals::appPath + "/shader/vertexNormal.frag");
	activateUniforms();
}