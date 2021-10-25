#pragma once
#include "Mesh.h"
#include "Renderer.h"
#include "SupportData.h"

class SupportEditPlane
{
public:
	SupportEditPlane();
	~SupportEditPlane();
	void setSupportData(SupportData* supportData_);
	void createMesh();
	void updateBufferData(double z);
	void updateColorData();

	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawForColorPicking(QMatrix4x4 proj_, QMatrix4x4 view_);
	void createPlaneBoundary();
	void createEditPlane();
	void createSupportLines();
private:
	Mesh *editPlane;
	Mesh *planeBoundary;
	Mesh *supportLines;
	Renderer *editPlaneRenderer;
	Renderer *planeBoundaryRenderer;
	Renderer *supportLinesRenderer;
	Renderer *fboRenderer;

	SupportData* supportData;

	ScaledFrame scaledFrame;
	ScaledFrame bottomFrame;

	//std::vector<QPointF> base_outlines;

	std::vector<Mesh::VertexHandle> vhs;
	std::vector<int> vertex_idx;
};

