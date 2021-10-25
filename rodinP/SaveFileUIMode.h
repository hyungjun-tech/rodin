#pragma once
#include "UIMode.h"

class PrinterBody;
class SaveFileUIMode : public UIMode
{
public:
	SaveFileUIMode();
	SaveFileUIMode(ViewerModule* engine_);
	~SaveFileUIMode();

	virtual void initialize(int width_, int height_) {};

	virtual void draw() {};

	virtual void mousePressEvent(QMouseEvent* e) {};
	virtual void mouseMoveEvent(QMouseEvent* e) {};
	virtual void mouseReleaseEvent(QMouseEvent* e) {};
	virtual void wheelEvent(QWheelEvent *e) {};
	virtual void keyPressEvent(QKeyEvent *e) {};
	virtual void resizeEvent(QResizeEvent *e) {};

	virtual void refreshUI() {};
	virtual void resetBed() {};
	virtual void detectCollision() {};

	bool saveThumbnail(QString filename_);
	static QImage saveThumbnail(std::vector<IMeshModel*> models_);
	
	static QImage saveCroppedUpperImage(std::vector<IMeshModel*> models_);
	static QImage saveFullUpperImage(std::vector<IMeshModel*> models_);
	static QImage saveFullRightImage(std::vector<IMeshModel*> models_);

	static QImage saveUpperImage(int w_, int h_, qglviewer::Vec center_, std::vector<IMeshModel*> models_);
	static QImage saveRightImage(int w_, int h_, qglviewer::Vec center_, std::vector<IMeshModel*> models_);
	static QImage saveColoredIndexMapImage(qglviewer::Camera *camera_, int w_, int h_, std::vector<IMeshModel*> models_);

	static QImage dilationImage(const QImage& input_);
private:
	PrinterBody* printerBody;
	float planeHeight;
	void drawOnlyModel(qglviewer::Camera *camera_);
	static void drawOnlyModel(qglviewer::Camera *camera_, std::vector<IMeshModel*> models_);
	qglviewer::Camera* getFitToModelCamera();
	static qglviewer::Camera* getFitToModelCamera(std::vector<IMeshModel*> models_);
	static qglviewer::Camera* getTopCamera(int w_, int h_, qglviewer::Vec center_);
	static qglviewer::Camera* getRightCamera(int w_, int h_, qglviewer::Vec center_);
	static qglviewer::Camera* getIsometricCamera();
		
	static void drawColoredIndexMapImage(qglviewer::Camera *camera_, std::vector<IMeshModel*> models_);

};

