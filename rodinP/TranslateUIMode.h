#pragma once
#include "UIMode.h"

class FBO;
class PrinterBo;
class TranslateUIMode : public UIMode
{
public:
	TranslateUIMode(ViewerModule* viewerModule_);
	virtual ~TranslateUIMode();

	virtual void initialize(int width_, int height_);

	virtual void draw();

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void resizeEvent(QResizeEvent *e = nullptr);

	virtual void refreshUI();
	virtual void resetBed();
	virtual void detectCollision();

private:
	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);

	FBO* fbo;
	PrinterBody* printerBody;

	int onID;
	bool onMesh;
	bool onHandle;
	bool onSelectionHandle;

	//Multi Selection Function
	bool isMultiSelectionOn;
	QRect rectangle;
	void drawSelectionRectangle();
	void offScreenRectangleCapture(QMouseEvent* e);
	std::vector<int> collectModelIDs(QPoint corner1st, QPoint corner2nd);

	void updateHandle();
	void adjustManipulatedFrame(AABB& aabb_);
};