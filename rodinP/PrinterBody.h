#pragma once
#include "ScaledFrame.h"

#include "Mesh.h"
#include "Renderer.h"

#define TEST_NEW_BED

class AABB;
class PrinterBody
{
public:
	PrinterBody();
	~PrinterBody();
	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	void init();
	void setSize();
	void setSize(AABB machineBox, float offsetx_ = 0, float offsety_ = 0);

private:
	void createBed();	
	void createPanel();
	void createFrontBar();
	void creatPrintLimitRange();
	void creatBedOffset();

	Mesh *panel;
	Mesh *bed1;
	Mesh *bed2;
	Mesh *frontBar;
	Mesh *printLimitRange_clip_L;
	Mesh *printLimitRange_clip_R;
	Mesh *printLimitRange_upper;
	Mesh *bedOffset;

	Renderer *panelRenderer;
	Renderer *bed1Renderer;
	Renderer *bed2Renderer;
	Renderer *frontBarRenderer;
	Renderer *printLimitRange_clip_Renderer;
	Renderer *printLimitRange_upper_Renderer;
	Renderer *bedOffsetRenderer;

	ScaledFrame scaledFrame;

	float width, length, height;
	float offsetx, offsety;
	float gridGap;
	float zSinkOffset;
};