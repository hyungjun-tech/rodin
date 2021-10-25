#pragma once

#include "ScaledFrame.h"

#include "Mesh.h"
#include "Renderer.h"
#include "ManipulatedFrame.h"

// Class for Translation Handles -------------------------------------------------------------------
class TranslationHandle
{
public:
	TranslationHandle(ManipulatedFrame *manipulatedFrame_);
	~TranslationHandle();

	void setRelativePosition(float dx_, float dy_, float dz_);
	void setScale(float factor_);

	void drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawForColorPicking(QMatrix4x4 proj_, QMatrix4x4 view_);

	Mesh* mesh;

	ScaledFrame scaledFrame;

	Renderer* matteRenderer;
	Renderer* phongRenderer;
};

// Class for Rotation Handles ----------------------------------------------------------------------
class RotationHandles
{
public:
	RotationHandles(ManipulatedFrame *manipulatedFrame_);
	~RotationHandles();
	void setDrawAxis(int axis_);
	void resetDrawAxis();
	void setRelativeOrientation(qglviewer::Quaternion quaternion_);
	void setScale(float factor_);

	void drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawForColorPicking(QMatrix4x4 proj_, QMatrix4x4 view_);
	std::vector<float> getCirclePoints(float _radius, int _axis_x, int _axis_y, int _axis_z);
	Mesh* meshX;
	Mesh* meshY;
	Mesh* meshZ;
	bool drawX, drawY, drawZ;

	ScaledFrame scaledFrame;

	Renderer* matteRendererX;
	Renderer* matteRendererY;
	Renderer* matteRendererZ;

	Renderer* phongRendererX;
	Renderer* phongRendererY;
	Renderer* phongRendererZ;

	Renderer* circleRendererX;
	Renderer* circleRendererY;
	Renderer* circleRendererZ;
};

// Class for Support Handles ----------------------------------------------------------------------
class SupportHandle
{
public:
	SupportHandle();
	~SupportHandle();

	void setPosition(qglviewer::Vec pos_);
	void setScale(float scale_);

	void drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_);

	Mesh* sphere;

	ScaledFrame scaledFrame;

	Renderer* phongRenderer;
	static float supportPointRadius;
};

// Class for Find Bottom Mesh Handles ----------------------------------------------------------------------
class BottomMeshHandle
{
public:
	BottomMeshHandle();
//	BottomMeshHandle(ManipulatedFrame *manipulatedFrame_);		// 2019 08 28
	~BottomMeshHandle();

	void drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_);

	Mesh* makeTriangle(std::vector<qglviewer::Vec> _triVertices);
	void findTriangle(std::vector<qglviewer::Vec> _triVertices);
	Mesh* triangle;

	ScaledFrame scaledFrame;

	Renderer* phongRenderer;
};

// Class for Selection Handles ----------------------------------------------------------------------
class SelectionHandle
{
public:
	SelectionHandle();
	~SelectionHandle();

	void setRelativePosition(float dx_, float dy_, float dz_);
	void moveFrame(float dx_, float dy_, float dz_);
	void setScale(float factor_);
	void refreshHandle(AABB aabb_);

	void drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawForColorPicking(QMatrix4x4 proj_, QMatrix4x4 view_);

	Mesh* mesh;

	ScaledFrame scaledFrame;

	Renderer* matteRenderer;
	Renderer* phongRenderer;
};