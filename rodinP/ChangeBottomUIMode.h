#pragma once

#include "UIMode.h"

class FBO;
class PrinterBody;
class BottomMeshHandle;
class ChangeBottomUIMode : public UIMode
{
public:
	ChangeBottomUIMode(ViewerModule* engine_);
	virtual ~ChangeBottomUIMode();

	virtual void initialize(int width_, int height_);

	virtual void draw();

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent *e);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void resizeEvent(QResizeEvent *e = nullptr);

	virtual void refreshUI();
	virtual void detectCollision();

	std::vector<qglviewer::Vec> triVertices;

	BottomMeshHandle* dummy;
	bool isDummyVisible;		// to draw bottomMesh
	void rotateToNewBottom();

private:
	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);

	void drawColoredIndex(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawNormalVecMap(QMatrix4x4 proj_, QMatrix4x4 view_);
	qglviewer::Vec getWorldPositionFromNormalMap(QPoint cursor_);

	FBO* fboColoredIndex;
	FBO* fboNormalVecMap;

	PrinterBody* printerBody;

	QPoint pressed;
	qglviewer::Vec onNormal;
	qglviewer::Vec B_worldPos;

	int onID;
	bool onMesh;
	int latestModelNo;
	float x, y, z;
	bool modelChanged;
	void colorTriMesh(bool);

	Mesh *bTriMesh;

	void updateHandle();
	void adjustManipulatedFrame(AABB& aabb_);
};