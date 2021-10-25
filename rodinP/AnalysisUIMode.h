#pragma once
#include "UIMode.h"

class PrinterBody;
class VolumeAnalysis;
class AnalysisUIMode : public UIMode
{
public:
	AnalysisUIMode(ViewerModule* engine_);
	virtual ~AnalysisUIMode();

	virtual void initialize(int width_, int height_);
	virtual void draw();
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);

	VolumeAnalysis* getVolumeAnalysis();

private:
	VolumeAnalysis *volumeAnalysis;

	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);

	PrinterBody* printerBody;
};