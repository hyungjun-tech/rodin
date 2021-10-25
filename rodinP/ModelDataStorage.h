#pragma once
#include "SupportData.h"
#include "LayerDatasSet.h"
#include "settings.h"
#include "GCodeGenerator.h"

class ModelDataStorage
{
public:
	ModelDataStorage();
	~ModelDataStorage();
	void wipeTowerClear();
	void insertGcodePath(engine::LayerPathG gcodePaths_);
	bool supportGenerated() { return supportData->generated; }

	//engine::GCodeExport gcode;
	SupportData* supportData;
	std::vector<SupportLayer> support_layers;

	engine::Polygons skirt;
	engine::Polygons raftOutline;
	engine::Polygons adjustZGapOutline_origin;
	engine::Polygons adjustZGapOutline_moved;

	//실제 출력 polygon line for boundary check..//
	Point raft_printed_line_min;
	Point raft_printed_line_max;
	Point support_printed_line_min;
	Point support_printed_line_max;


	std::vector<WipeTowerLayer> wipeTower_layers;
	engine::Polygons wipeTower_mainOutline_for_skirt;
	std::vector<Point> wipeTopwer_points;

	int totalLayer;
	int layerCount; //실제 슬라이싱 후에 나온 layer의 갯수
	//bool b_slicing_flag;
	int z_height_base;

	FPoint3 min_printing, max_printing; //mm
	std::vector<std::vector<GCodePathforDraw>> gcodePath;
};

