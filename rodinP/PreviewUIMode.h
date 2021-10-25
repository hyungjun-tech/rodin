#pragma once

#include "UIMode.h"

class GCodePath;
class PrinterBody;
class GCodePathforDraw;
class PreviewUIMode : public UIMode
{
public:
	PreviewUIMode(ViewerModule* engine_);
	virtual ~PreviewUIMode();

	virtual void initialize(int width_, int height_);
	virtual void draw();

	virtual void detectCollision() {}

	void changeLayerIndex(int index_);
	void updateGCodePath(std::vector<std::vector<GCodePathforDraw>> path_);
	void setZHeightBase(float zHeightBase_);
	void setPlaneHeight(float planeHeight_);
	void toggleExtruderMode(bool flag_);
	void toggleCurrentLayerOnly(bool flag_);
	void toggleShowTravelPath(bool flag_);
	void clear();

private:
	GCodePath *gcodePath;

	float planeHeight;

	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);

	PrinterBody* printerBody;
	float z_height_base;
};