#pragma once

#include "MeshModel.h"
#include "MatrixGetter.h"
#include "ViewerModule.h"
#include "ModelContainer.h"
#include "ManipulatedFrame.h"

class UIMode : public QObject
{
	Q_OBJECT
public:
	UIMode();
	UIMode(ViewerModule* engine_);
	virtual ~UIMode();

	virtual void initialize(int width_, int height_) = 0;
	virtual void draw() = 0;

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e) {}
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent *e) {}
	virtual void keyPressEvent(QKeyEvent *e) {}
	virtual void resizeEvent(QResizeEvent *e = nullptr) {}

	virtual void refreshUI() {}
	virtual void detectCollision() {}

	const static int dragThreshold = 3;

	static QColor translateUIColor;
	static QColor rotateUIColor;
	static QColor hollowUIColor;
	static QColor previewUIColor;
	static QColor supportUIColor;
	static QColor changeBottomUIColor;
	static QColor analysisUIColor;
	static QColor layerColorUIColor;
protected:
	ViewerModule* engine;
	ModelContainer* modelContainer;
	ManipulatedFrame *manipulatedFrame;
	Camera *camera;

	QPoint pressed;
signals:
	void signal_mouseOnModel(int, int);
	void signal_mouseOutModel(int, int);
};