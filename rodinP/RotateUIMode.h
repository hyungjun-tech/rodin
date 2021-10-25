#pragma once

#include "UIMode.h"

class RotationHandles;
class FBO;
class PrinterBody;
class RotateUIMode : public UIMode
{
public:
	RotateUIMode(ViewerModule* engine_);
	virtual ~RotateUIMode();

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

private:
	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);

	FBO* fbo;
	RotationHandles *rotationHandles;
	PrinterBody* printerBody;

	int onID;
	bool onMesh;
	bool onHandleX;
	bool onHandleY;
	bool onHandleZ;

	void updateHandle();
	void adjustManipulatedFrame(AABB& aabb_);
	void adjustHandlePosition(AABB& aabb_);
	void adjustHandleScale();

	//void DetectCollision();
	//void PushModels(AABB aabb_, std::vector<IMeshModel*> models_);
};