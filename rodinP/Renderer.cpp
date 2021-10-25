#include "stdafx.h"
#include "Renderer.h"

// Interface Class ---------------------------------------------------------------------------------
Renderer::Renderer()
	: reference(nullptr)
	, vao(nullptr)
{
}

Renderer::~Renderer()
{
	delete vao;
	delete renderer;
}

void Renderer::create(Mesh* mesh_)
{
	if (mesh_ == nullptr)
		return;
	/*if (mesh_->n_vertices() == 0)
		return;*/

	vao->create(mesh_);
	reference = mesh_;
	n_vertices = mesh_->n_vertices();
	n_faces = mesh_->n_faces();
}
void Renderer::create(std::vector<Mesh*>& meshes_)
{
	/*if (meshes_.size() == 0)
		return;*/

	vao->create(meshes_);
	n_vertices = 0;
	n_faces = 0;
	for (auto it : meshes_)
	{
		n_vertices += it->n_vertices();
		n_faces += it->n_faces();
	}
}
void Renderer::create(std::vector<float> points_)
{
	if (points_.size() == 0)
		return;

	vao->createVertexBuffer(points_);
	n_vertices = points_.size() / 3;
}
void Renderer::draw(Mesh* mesh_, MVPMatrices mvp_)
{
	if (mesh_ == nullptr)
		return;
	if (mesh_->n_vertices() == 0)
		return;

	if (!IsSameMesh()(reference, mesh_))
	{
		create(mesh_);
		reference = mesh_;
	}
	renderer->draw(vao, n_faces * 3, mvp_, GL_TRIANGLES);
}
void Renderer::draw(std::vector<Mesh*>& meshes_, MVPMatrices mvp_)
{
	if (meshes_.size() == 0)
		return;

	if (!IsSameMesh()(reference, &meshes_))
	{
		create(meshes_);
		reference = &meshes_;
	}
	renderer->draw(vao, n_faces * 3, mvp_, GL_TRIANGLES);
}
void Renderer::draw(MVPMatrices mvp_, GLenum mode_)
{
	int n_buffer = 0;
	if (mode_ == GL_TRIANGLES ||
		mode_ == GL_TRIANGLE_STRIP ||
		mode_ == GL_TRIANGLE_FAN)
		n_buffer = n_faces * 3;
	else
		n_buffer = n_vertices;
	renderer->draw(vao, n_buffer, mvp_, mode_);
}
void Renderer::drawTriangles(MVPMatrices mvp_)
{
	renderer->draw(vao, n_faces * 3, mvp_, GL_TRIANGLES);
}
void Renderer::drawLines(MVPMatrices mvp_)
{
	renderer->draw(vao, n_vertices, mvp_, GL_LINES);
}
void Renderer::drawLineLoop(MVPMatrices mvp_)
{
	renderer->draw(vao, n_vertices, mvp_, GL_LINE_LOOP);
}
void Renderer::drawPolygon(MVPMatrices mvp_)
{
	renderer->draw(vao, n_vertices, mvp_, GL_POLYGON);
}
void Renderer::setColor(QColor color_)
{
	renderer->setColor(color_.redF(), color_.greenF(), color_.blueF(), color_.alphaF());
}
void Renderer::setColor(QColor color_, float a_)
{
	renderer->setColor(color_.redF(), color_.greenF(), color_.blueF(), a_);
}
void Renderer::setColor(float r_, float g_, float b_, float a_)
{
	renderer->setColor(r_, g_, b_, a_);
}
void Renderer::setLayerColors(std::vector<QVector3D> layerColors_)
{
	renderer->setLayerColors(layerColors_);
}
void Renderer::setLayerHeights(std::vector<float> layerHeigts_)
{
	renderer->setLayerHeights(layerHeigts_);
}
void Renderer::setClipping(float h_)
{
	renderer->setClipping(h_);
}
void Renderer::deleteReference()
{
	reference = nullptr;
}

// Non-Shaded Renderer Class -----------------------------------------------------------------------
NonShadedRenderer::NonShadedRenderer() 
{
	renderer = new SimpleBufferRenderer();
	program = renderer->getProgram();
	vao = new V3TriangleVAO(*program);
}

// Phong-Shaded Renderer Class ---------------------------------------------------------------
PhongShadedRenderer::PhongShadedRenderer()
{
	renderer = new PhongBufferRenderer();
	program = renderer->getProgram();
	vao = new V3N3TriangleVAO(*program);
}

// NormalMapRGBRenderer Renderer Class ---------------------------------------------------------------
NormalMapRGBRenderer::NormalMapRGBRenderer()
{
	renderer = new VertexNormalBufferRenderer();
	program = renderer->getProgram();
	vao = new V3N3TriangleVAO(*program);
}

// VertexColorRenderer Class ---------------------------------------------------------------------
VertexColorRenderer::VertexColorRenderer()
{
	renderer = new VertexColorBufferRenderer();
	program = renderer->getProgram();
	vao = new V3C3TriangleVAO(*program);
}

// LineRenderer Renderer Class -----------------------------------------------------------------------
LineRenderer::LineRenderer() : Renderer()
{
	renderer = new SimpleBufferRenderer();
	program = renderer->getProgram();
	vao = new LineVAO(*program);
}
void LineRenderer::draw(Mesh* mesh_, MVPMatrices mvp_)
{
	if (mesh_ == nullptr)
		return;
	if (mesh_->n_vertices() == 0)
		return;

	if (!IsSameMesh()(reference, mesh_))
	{
		create(mesh_);
		reference = mesh_;
	}
	renderer->draw(vao, n_vertices, mvp_, GL_LINES);
}
void LineRenderer::draw(std::vector<Mesh*>& meshes_, MVPMatrices mvp_)
{
	if (meshes_.size() == 0)
		return;

	if (!IsSameMesh()(reference, &meshes_))
	{
		create(meshes_);
		reference = &meshes_;
	}
	renderer->draw(vao, n_vertices, mvp_, GL_LINES);
}

// LineRenderer Renderer Class -----------------------------------------------------------------------
LineColorRenderer::LineColorRenderer()
{
	renderer = new VertexColorBufferRenderer();
	program = renderer->getProgram();
	vao = new LineColorVAO(*program);
}
void LineColorRenderer::draw(Mesh* mesh_, MVPMatrices mvp_)
{
	if (mesh_ == nullptr)
		return;
	if (mesh_->n_vertices() == 0)
		return;

	if (!IsSameMesh()(reference, mesh_))
	{
		create(mesh_);
		reference = mesh_;
	}
	renderer->draw(vao, n_vertices, mvp_, GL_LINES);
}
void LineColorRenderer::draw(std::vector<Mesh*>& meshes_, MVPMatrices mvp_)
{
	if (meshes_.size() == 0)
		return;

	if (!IsSameMesh()(reference, &meshes_))
	{
		create(meshes_);
		reference = &meshes_;
	}
	renderer->draw(vao, n_vertices, mvp_, GL_LINES);
}