#pragma once

#include "MeshModel.h"
#include "ProgressHandler.h"

namespace engine
{
	class Polygons;
	class PolygonRef;	
}
class ConfigSettings;
class VolumeAnalysis
{
public:
	VolumeAnalysis();
	~VolumeAnalysis();

	void clear();
	void setProgressHandler(ProgressHandler* handler_);
	void setOverhangMode(bool toggle_);// { overhangMode = toggle_; }
	bool isOverhangMode();// { return overhangMode; }

	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	void setMeshModel(IMeshModel *meshModel_);


private:
	IMeshModel* m_model;
	Mesh* m_mesh_overhang;


	bool overhangMode;
	ProgressHandler* progressHandler;
	Renderer *overhangRenderer;

	//////////////////////////////////////////////////////////////////////////
	// thickness
	//////////////////////////////////////////////////////////////////////////
public:
	bool calcThickness();
	void assignPolygonColor(const double minThick);

private:
	void calcPartThickness(engine::Polygons& polygons, std::vector<std::vector<double>>& thicks, std::vector<std::vector<QVector2D>>& v_normals);
	void calcPolygonThickness(engine::PolygonRef& polygon, std::vector<double>& thick, engine::Polygons& refPolygons, std::vector<QVector2D>& refVNormals, std::vector<std::vector<QVector2D>>& refSNormals);
	double getThickness(const Point& pt, const QVector2D& dir, engine::Polygons& refPolygons, std::vector<std::vector<QVector2D>>& refSNormals, QVector2D& projected);
	double projectToSegment(const Point& pt, const QVector2D& dir, const Point& ptFrom, const Point& ptTo, const QVector2D& snormal, int& state, QVector2D& projected);
	void calcPolygonNormals(engine::PolygonRef& polygon, std::vector<QVector2D>& v_normals, std::vector<QVector2D>& s_normals);
	void updateBufferData(engine::Polygons& polygons_, std::vector<std::vector<Mesh::VertexHandle>>& vertexHandles_, std::vector<Mesh*>& meshes_, std::vector<Renderer*>& renderers_, float sliceZ_);
	

	std::vector<std::vector<std::vector<std::vector<double>>>> polygon_thickness;
	std::vector<std::vector<std::vector<std::vector<QVector2D>>>> polygon_vnormals;

	std::vector<std::vector<std::vector<std::vector<Mesh::VertexHandle>>>> vertexHandles_thickness_layers;

	std::vector<std::vector<std::vector<Mesh*>>> m_mesh_thickness_layers;
	std::vector<std::vector<std::vector<Renderer*>>> thicknessRenderers;




	//////////////////////////////////////////////////////////////////////////
	// overhang
	//////////////////////////////////////////////////////////////////////////
public:
	void initializeOverhang();
	void calcOverhangeRegion(const double degree = 90);	

};

