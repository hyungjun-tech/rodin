#include "stdafx.h"
#include "VolumeAnalysis.h"
#include "settings.h"
#include "LayerDatasSet.h"
//#include "DataStorage.h"
VolumeAnalysis::VolumeAnalysis()
	: overhangMode(false)
	, m_mesh_overhang(nullptr)
	, progressHandler(nullptr)
{
	overhangRenderer = new VertexColorRenderer();// PhongColorRenderer();
}

VolumeAnalysis::~VolumeAnalysis()
{
	delete overhangRenderer;
	if (m_mesh_overhang)
		delete m_mesh_overhang;
	clear();
}

void VolumeAnalysis::clear()
{
	polygon_thickness.clear();
	overhangMode = false;
	polygon_vnormals.clear();
	vertexHandles_thickness_layers.clear();

	for (auto i : m_mesh_thickness_layers)
		for (auto j : i)
			for (auto k : j)
				delete k;

	m_mesh_thickness_layers.clear();

	for (auto i : thicknessRenderers)
		for (auto j : i)
			for (auto k : j)
				delete k;

	thicknessRenderers.clear();
}
void VolumeAnalysis::setProgressHandler(ProgressHandler* handler_)
{
	progressHandler = handler_;
}

void VolumeAnalysis::setMeshModel(IMeshModel *meshModel_)
{
	m_model = meshModel_;
}

void VolumeAnalysis::setOverhangMode(bool toggle_)
{
	overhangMode = toggle_;
	if (overhangMode)
		m_model->setColor(QColor(0, 0, 255, 255));
	else
		m_model->setColor(QColor(0.8 * 255, 0.8 * 255, 0.8 * 255, 1.0 * 255));
}

bool VolumeAnalysis::isOverhangMode()
{
	return overhangMode;
}

bool VolumeAnalysis::calcThickness()
{
	clear();
	polygon_thickness.resize(m_model->sliceLayers.size());
	polygon_vnormals.resize(m_model->sliceLayers.size());
	vertexHandles_thickness_layers.resize(m_model->sliceLayers.size());
	m_mesh_thickness_layers.resize(m_model->sliceLayers.size());
	thicknessRenderers.resize(m_model->sliceLayers.size());

	for (uint i = 0; i < m_model->sliceLayers.size(); ++i)
	{
		SliceLayer& layer = m_model->sliceLayers[i];

		polygon_thickness[i].resize(layer.parts.size());
		polygon_vnormals[i].resize(layer.parts.size());
		vertexHandles_thickness_layers[i].resize(layer.parts.size());
		m_mesh_thickness_layers[i].resize(layer.parts.size());
		thicknessRenderers[i].resize(layer.parts.size());

		for (uint j = 0; j < layer.parts.size(); ++j)
		{
			engine::Polygons& polygons = layer.parts[j].outline;
			calcPartThickness(polygons, polygon_thickness[i][j], polygon_vnormals[i][j]);
			updateBufferData(polygons, vertexHandles_thickness_layers[i][j], m_mesh_thickness_layers[i][j], thicknessRenderers[i][j], layer.sliceZ);
		}

		if (progressHandler)
		{
			if (progressHandler->wasCanceled())
			{
				std::cout << "canceled!" << std::endl;
				return false;
			}
			int percent = i * 100 / m_model->sliceLayers.size();
			progressHandler->setValue(percent);
		}
	}
	//updateBufferData_thickness_path();
	return true;
}

void VolumeAnalysis::calcPartThickness(engine::Polygons& polygons, std::vector<std::vector<double>>& thicks, std::vector<std::vector<QVector2D>>& v_normals)
{
	thicks.clear();
	thicks.resize(polygons.size());

	v_normals.clear();
	v_normals.resize(polygons.size());

	std::vector<std::vector<QVector2D>> segment_normals;
	segment_normals.resize(polygons.size());

	for (uint i = 0; i < polygons.size(); ++i)
		calcPolygonNormals(polygons[i], v_normals[i], segment_normals[i]);

	for (uint i = 0; i < polygons.size(); ++i)
		calcPolygonThickness(polygons[i], thicks[i], polygons, v_normals[i], segment_normals);
}

void VolumeAnalysis::calcPolygonThickness(engine::PolygonRef& polygon, std::vector<double>& thick, engine::Polygons& refPolygons, std::vector<QVector2D>& refVNormals, std::vector<std::vector<QVector2D>>& refSNormals)
{
	thick.clear();
	thick.resize(polygon.size());

	for (uint i = 0; i < polygon.size(); ++i)
	{
		QVector2D projected;
		thick[i] = getThickness(polygon[i], -refVNormals[i], refPolygons, refSNormals, projected)*0.001;
	}
}

void VolumeAnalysis::updateBufferData(engine::Polygons& polygons_, std::vector<std::vector<Mesh::VertexHandle>>& vertexHandles_, std::vector<Mesh*>& meshes_, std::vector<Renderer*>& renderers_, float sliceZ_)
{
	vertexHandles_.resize(polygons_.size());
	int cnt = 0;
	for (auto polygon : polygons_)
	{
		meshes_.push_back(new Mesh());
		renderers_.push_back(new LineColorRenderer());
		for (auto vertex : polygon)
		{
			vertexHandles_[cnt].push_back(meshes_[cnt]->add_vertex(Mesh::Point(vertex.X*0.001, vertex.Y*0.001, sliceZ_*0.001)));
		}
		cnt++;
	}
}

void VolumeAnalysis::calcPolygonNormals(engine::PolygonRef& polygon, std::vector<QVector2D>& v_normals, std::vector<QVector2D>& s_normals)
{
	int numPts = polygon.size();

	v_normals.clear();
	v_normals.resize(numPts);

	s_normals.clear();
	s_normals.resize(numPts);

	for (int i = 0; i < numPts; ++i)
	{
		int nextIndex = i + 1 == numPts ? 0 : i + 1;
		int beforeIndex = i == 0 ? numPts - 1 : i - 1;

		Point curPt = polygon[i];
		Point nextPt = polygon[nextIndex];
		Point beforePt = polygon[beforeIndex];

		Point beforeSegment = curPt - beforePt;
		Point nextSegment = nextPt - curPt;

		Point beforeNormal = crossZ(beforeSegment);
		Point nextNormal = crossZ(nextSegment);

		QVector2D bn(beforeNormal.X, beforeNormal.Y);
		bn.normalize();

		QVector2D nn(nextNormal.X, nextNormal.Y);
		nn.normalize();

		v_normals[i] = -(bn + nn).normalized();
		s_normals[i] = -nn;
	}
}

double VolumeAnalysis::getThickness(const Point& pt, const QVector2D& dir, engine::Polygons& refPolygons, std::vector<std::vector<QVector2D>>& refSNormals, QVector2D& projected)
{
	double thick = 100000000;
	for (uint i = 0; i < refPolygons.size(); ++i)
	{
		for (uint j = 0; j < refPolygons[i].size(); ++j)
		{
			int nextIndex = j + 1 == refPolygons[i].size() ? 0 : j + 1;

			Point curPt = refPolygons[i][j];
			Point nextPt = refPolygons[i][nextIndex];

			int state;

			QVector2D temProjected;
			double tem = projectToSegment(pt, dir, curPt, nextPt, refSNormals[i][j], state, temProjected);

			if (state > 0 && tem < thick)
			{
				thick = tem;
				projected = temProjected;
			}

		}
	}
	return thick;
}

double VolumeAnalysis::projectToSegment(const Point& pt, const QVector2D& dir, const Point& ptFrom, const Point& ptTo, const QVector2D& snormal, int& state, QVector2D& projected)
{
	QVector2D p(pt.X, pt.Y);
	QVector2D p0(ptFrom.X, ptFrom.Y);
	QVector2D p1(ptTo.X, ptTo.Y);

	projected = p;

	if (p == p0)
	{
		state = 0;
		return 0.0;
	}
	else if (p == p1)
	{
		state = 0;
		return 0.0;
	}

	QVector2D v = p1 - p0;
	float den = QVector2D::dotProduct(dir, snormal);
	float k = QVector2D::dotProduct(p0 - p, snormal) / den;

	if (k < 0)
	{
		state = 0;
		return 0.0;
	}

	projected = p + k * dir;

	Point p_proj(projected.x(), projected.y());


	if (p_proj.X == ptFrom.X && p_proj.Y == ptFrom.Y)
		state = 1;
	else if (p_proj.X == ptTo.X && p_proj.Y == ptTo.Y)
		state = 2;
	else
	{
		float tem = QVector2D::dotProduct(v, projected - p0) * QVector2D::dotProduct(-v, projected - p1);
		if (tem < 0)
		{
			state = 0;
			return 0.0;
		}
		else
			state = 3;
	}
	return k;
}

void VolumeAnalysis::assignPolygonColor(const double minThick)
{
	for (uint i = 0; i < m_model->sliceLayers.size(); ++i)
	{
		SliceLayer& layer = m_model->sliceLayers[i];

		for (uint j = 0; j < layer.parts.size(); ++j)
		{
			std::vector<QColor> polygon_color_part;

			engine::Polygons& polygons = layer.parts[j].outline;

			int count = 0;
			for (uint k = 0; k < polygons.size(); ++k)
			{
				engine::PolygonRef polygon = polygons[k];

				for (uint l = 0; l < polygon.size(); ++l)
				{
					double thick = polygon_thickness[i][j][k][l];

					QColor c;
					if (thick >= minThick)
						c = QColor::fromHsv(240, 255, 255).toRgb();
					else
						c = QColor::fromRgb(255, 0, 0);

					Mesh::VertexHandle vh = vertexHandles_thickness_layers[i][j][k][l];
					Mesh::Point color_f = Mesh::Point(c.redF(), c.greenF(), c.blueF());
					m_mesh_thickness_layers[i][j][k]->set_color(vh, color_f);
					count++;
				}
				// creat Renderer//
				thicknessRenderers[i][j][k]->create(m_mesh_thickness_layers[i][j][k]);
			}

		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////

void VolumeAnalysis::initializeOverhang()
{
	//////////////////////////////////////////////////////////////
	//mesh reconstruction for overhang..//
	delete m_mesh_overhang;
	m_mesh_overhang = new Mesh();

	std::vector<Mesh::VertexHandle> vertex_handles;
	for (auto model : m_model->getModels())
	{
		Mesh* modelMesh = model->getOuter();
		for (Mesh::FaceIter fit = modelMesh->faces_begin();
			fit != modelMesh->faces_end();
			fit++)
		{
			for (Mesh::FaceVertexIter fvit = modelMesh->fv_begin(*fit);
				fvit != modelMesh->fv_end(*fit);
				fvit++)
			{
				Mesh::Point lp = modelMesh->point(*fvit);
				qglviewer::Vec wp = model->getFrame().toWorldCoords(qglviewer::Vec(lp[0], lp[1], lp[2]));
				vertex_handles.push_back(m_mesh_overhang->add_vertex(Mesh::Point(wp[0], wp[1], wp[2])));
			}

			if (vertex_handles.size() != 3)
				continue;
			m_mesh_overhang->add_face(vertex_handles);
			vertex_handles.clear();
		}
	}
	m_mesh_overhang->update_normals();
}

void VolumeAnalysis::calcOverhangeRegion(const double degree /*= 90*/)
{
	double cosAngle = sin(degree*M_PI / 180.0);

	OpenMesh::VPropHandleT<float> alpha;
	if (!m_mesh_overhang->get_property_handle(alpha, "alpha"))
		m_mesh_overhang->add_property(alpha, "alpha");

	for (Mesh::VertexIter vit = m_mesh_overhang->vertices_begin();
		vit != m_mesh_overhang->vertices_end();
		vit++)
	{
		m_mesh_overhang->set_color(vit, Mesh::Color(0, 0, 1));
		m_mesh_overhang->property(alpha, vit) = 0.0f;
	}


	int cnt = 0;
	for (Mesh::FaceIter fit = m_mesh_overhang->faces_begin();
		fit != m_mesh_overhang->faces_end();
		fit++)
	{
		Mesh::Normal n = m_mesh_overhang->calc_face_normal(fit).normalize();
		if (-n[2] > cosAngle)
		{
			for (Mesh::FaceVertexIter fvit = m_mesh_overhang->fv_begin(*fit);
				fvit != m_mesh_overhang->fv_end(*fit);
				fvit++)
			{
				m_mesh_overhang->set_color(fvit, Mesh::Color(1.f, 0, 0));
				m_mesh_overhang->property(alpha, fvit) = 1.0f;
			}
		}
	}

	overhangRenderer->create(m_mesh_overhang);
}

void VolumeAnalysis::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	MVPMatrices mvp(QMatrix4x4(), view_, proj_);

	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	if (isOverhangMode())
	{
		m_model->drawLayoutEditMode(proj_, view_);
		overhangRenderer->draw(mvp);
	}
	else
	{
		glFuncs->glDepthFunc(GL_LEQUAL);
		m_model->drawLayoutEditMode(proj_, view_);

		for (auto i : thicknessRenderers)
		{
			for (auto j : i)
			{
				for (auto k : j)
					k->drawLineLoop(mvp);
			}
		}
	}
}
