#pragma once
#include "LayerDatasSet.h"
#include "ModelDataStorage.h"
class ConfigSettings;

class SliceCalculator
{
public:
	SliceCalculator(std::vector<IMeshModel*> models_);
	~SliceCalculator();
	//inset
	void generateInsetsLayerPart(SliceLayerPart* part, int offset, int insetCount, int internalMovingArea);
	void generateInsets(SliceLayer* layer, int offset, int insetCount, int internalMovingArea, bool simplemode = false);
	void generateSupportInsetsLayerPart(SupportLayerPart* support_part, int offset, int insetCount);
	void generateSupportInsets(SupportLayer* support_layer, int offset, int insetCount);

	//skin
	void generateSkins(int layerNr, std::vector<SliceLayer> *layers, int extrusionWidth, int downSkinCount, int upSkinCount, int infillOverlap);
	void generateSparse(SliceLayer* layer, int extrusionWidth);
	//void generateSupportSkins(int layerNr, std::vector<SupportLayer>* supportLayers, int extrusionWidth, int supportDownSkinCount, int supportUpSkinCount, int supportInfillOverlap);
	void generateSupportInterface(int _layer_number, std::vector<SupportLayer>* _support_layers, int _interface_roof_extrusion_width, int _interface_floor_extrusion_width, int _interface_roof_count, int _interface_floor_count, int _support_infill_overlap);
	//void generateSupportSparse(int layerNr, std::vector<SupportLayer>* supportLayers, int extrusionWidth, int supportDownSkinCount, int supportUpSkinCount);
	void generateSupportSparse(int _layer_number, std::vector<SupportLayer>* _support_layers, int _interface_roof_extrusion_width, int _interface_floor_extrusion_width, int _interface_roof_count, int _interface_floor_count);

	//skirt
	void generateSkirt(ModelDataStorage* storage, ConfigSettings config);

	//raft
	void generateRaft(ModelDataStorage* storage, ConfigSettings config);

	//
	void generateAdjustZGap(ModelDataStorage* storage, ConfigSettings config);

	bool checkPolygonsPathRange(engine::Polygons polygons, const int margin);
private:
	//bridge
	std::vector<IMeshModel*> models;
	int bridgeAngle(engine::Polygons outline, SliceLayer* prevLayer);
	int bridgeAngle(engine::Polygons outline, SupportLayer* prevLayer);
};

