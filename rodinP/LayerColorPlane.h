#pragma once
#include "ScaledFrame.h"
#include "Mesh.h"
#include "Renderer.h"

class LayerColorPlane
{
public:
	LayerColorPlane();
	LayerColorPlane(ScaledFrame scaledFrame_);
	~LayerColorPlane();
	void setZ(float z);
	void setPosition(qglviewer::Vec position_);
	void setScale(qglviewer::Vec scale_);
	void moveFrame(float x_, float y_);
	void createMesh();
	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
private:
	Mesh *plane;
	Mesh *planeBoundary;
	Renderer *planeRenderer;
	Renderer *planeBoundaryRenderer;

	ScaledFrame scaledFrame;
};