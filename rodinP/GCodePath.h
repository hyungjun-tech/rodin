#pragma once
#include "ScaledFrame.h"
#include "Mesh.h"
#include "Renderer.h"
#include "GCodeGenerator.h"

class GCodePathforDraw;
class GCodePath
{
public:
	GCodePath();
	~GCodePath();
	void updateGCodePath(std::vector<std::vector<GCodePathforDraw>> path_);
	void updateBufferData(std::vector<GCodePathforDraw> gcodePath_);
	void updateBufferDataForEx(std::vector<GCodePathforDraw> gcodePath_);

	void drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_);
	void setCurrentLayer(int layerNo_);

	void toggleExtruderMode(bool flag_);
	void toggleCurrentLayerOnly(bool flag_);
	void toggleShowTravelPath(bool flag_);

	//convert rgb to OpenMesh::Vec3f//
	Mesh::Point convertRGBtoMeshPoint(QColor _color);
private:
	ScaledFrame scaledFrame;
	//std::vector<std::vector<GCodePathforDraw>> gcodePath;
	//std::vector<GCodeLayer> gcodeLayers;
	Mesh::Point print_line_colors[20];
	Mesh::Point print_rect_colors[20];
	Mesh::Point print_ex_colors[2];
	std::vector<Mesh*> paths;
	std::vector<Mesh*> pathBolds;
	std::vector<Mesh*> pathTravels;
	std::vector<Mesh*> pathsForDraw;
	std::vector<Mesh*> pathBoldsForDraw;
	Mesh* pathTavelForDraw;
	Renderer* pathRenderer;
	Renderer* pathBoldRenderer;
	Renderer* pathTravelRenderer;

	bool isExtruderMode, isCurrentLayerOnly, showTravelPath;
	int currentIndex;
	void updateMeshForDraw();
};
