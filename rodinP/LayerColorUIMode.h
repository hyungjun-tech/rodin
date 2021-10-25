#pragma once
#include "UIMode.h"

class FBO;
class PrinterBody;
class LayerColorUIMode : public UIMode
{
public:
	LayerColorUIMode(ViewerModule* engine_);
	~LayerColorUIMode();
	virtual void initialize(int width_, int height_);
	virtual void draw();

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void resizeEvent(QResizeEvent *e = nullptr);

	virtual void refreshUI();
	virtual void detectCollision();

	void createPlane();
	void changeLayerIndex(int index_);
private:
	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);

	FBO* fbo;
	PrinterBody* printerBody;

	int onID;
	bool onMesh;
	bool onHandle;

	void updateHandle();
	void adjustManipulatedFrame(AABB& aabb_);
};
