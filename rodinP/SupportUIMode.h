#pragma once
#include "UIMode.h"

class PrinterBody;
class FBO;
class SupportEditPlane;
class SupportUIMode : public UIMode
{
public:
	SupportUIMode(ViewerModule* engine_);
	~SupportUIMode();

	virtual void initialize(int width_, int height_);
	virtual void draw();

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void resizeEvent(QResizeEvent *e = nullptr);

	virtual void refreshUI();
	virtual void detectCollision() {}

	void setSupportData(SupportData* supportData_);
	void changeLayerIndex(int index_);
	void updateData();
	//Point3 getPoint(int x, int y);
	//int getType(int x, int y);
	//void Enable(double x, double y);
	//void Disable(double x, double y);
	//void deleteAllsupports();
	//void resetAllsupports(int mode);
private:
	PrinterBody* printerBody;
	SupportEditPlane* supportEditPlane;

	FBO* fbo;
	int onID;
	bool onPlane;

	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	void drawOffScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	qglviewer::Vec getWorldPositionFromFBO(QPoint cursor_);
	void updateData(QMouseEvent* e);
};

