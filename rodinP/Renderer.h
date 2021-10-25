#pragma once

#include "MVPMatrices.h"

#include "VAO.h"
#include "Mesh.h"
#include "BufferRenderer.h"

// Interface Class -------------------------------------------------------------
class Renderer
{
public:
	Renderer();
	~Renderer();

	void create(Mesh* mesh_);
	void create(std::vector<Mesh*>& meshes_);
	void create(std::vector<float> points_);
	virtual void draw(Mesh* mesh_, MVPMatrices mvp_);
	virtual void draw(std::vector<Mesh*>& meshes_, MVPMatrices mvp_);
	void draw(MVPMatrices mvp_, GLenum mode_ = GL_TRIANGLES);
	void drawTriangles(MVPMatrices mvp_);
	void drawLines(MVPMatrices mvp_);
	void drawLineLoop(MVPMatrices mvp_);
	void drawPolygon(MVPMatrices mvp_);
	void setColor(QColor color_);
	void setColor(QColor color_, float a_);
	void setColor(float r_, float g_, float b_, float a_ = 1.0);
	void setLayerColors(std::vector<QVector3D> layerColors_);
	void setLayerHeights(std::vector<float> layerHeigts_);
	void setClipping(float h_);
	void deleteReference();
	class IsSameMesh
	{
	public:
		bool operator()(void* a_, void* b_)
		{
			return a_ == b_;
		}
	};

	void* reference;
	VAO *vao;
	BufferRenderer *renderer;
	QOpenGLShaderProgram *program;

	int n_vertices;
	int n_faces;
};

// NonShadedRenderer Class -----------------------------------------------------
class NonShadedRenderer : public Renderer
{
public:
	NonShadedRenderer();
};

// PhongShadedRenderer Class ---------------------------------------------------
class PhongShadedRenderer : public Renderer
{
public:
	PhongShadedRenderer();
};

// NormalMapRGBRenderer Class --------------------------------------------------
class NormalMapRGBRenderer : public Renderer
{
public:
	NormalMapRGBRenderer();
};

// VertexColorRenderer Class ---------------------------------------------------
class VertexColorRenderer : public Renderer
{
public:
	VertexColorRenderer();
};

// LineRenderer Class ----------------------------------------------------------
class LineRenderer : public Renderer
{
public:
	LineRenderer();
	void draw(Mesh* mesh_, MVPMatrices mvp_);
	void draw(std::vector<Mesh*>& meshes_, MVPMatrices mvp_);
};

// LineColorRenderer Class ----------------------------------------------------------
class LineColorRenderer : public Renderer
{
public:
	LineColorRenderer();
	void draw(Mesh* mesh_, MVPMatrices mvp_);
	void draw(std::vector<Mesh*>& meshes_, MVPMatrices mvp_);
};