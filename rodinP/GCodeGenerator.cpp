#include "stdafx.h"
#include "GCodeGenerator.h"
#include "ModelContainer.h"
#include "infill.h"
//#include "support.h"
#include "PathOrderOptimizer.h"
#include "ModelDataStorage.h"
#include "ProgressHandler.h"
#include "ProfileToConfig.h"

GCodeGenerator::GCodeGenerator(QWidget *parent_)
	: progressHandler(new ProgressHandler(parent_))
	, abort(false)
{
	progressHandler->setWindowTitle(MessageProgress::tr("Slicing.."));
	connect(this, SIGNAL(signal_setLabelText(QString)), progressHandler, SLOT(setLabelText(QString)));
	connect(this, SIGNAL(signal_setValue(int)), progressHandler, SLOT(setValue(int)));
}
GCodeGenerator::GCodeGenerator()
	: progressHandler(nullptr)
	, abort(false)
{
}

GCodeGenerator::~GCodeGenerator()
{
	//gcodeGenerator가 끝나면 더이상 필요없는 데이타이므로 삭제
	for (std::vector<IMeshModel*>::iterator vit = models.begin(); vit != models.end(); ++vit)
	{
		(*vit)->sliceLayers.clear();
		(*vit)->polygonLayers.clear();
	}
	if (progressHandler)
		delete progressHandler;
}

void GCodeGenerator::init(ModelContainer* modelContainer_, ModelDataStorage* dataStorage_)
{
	models = modelContainer_->getBasicModels();
	printingInfo = modelContainer_->printingInfo;
	dataStorage = dataStorage_;
	//gcode = &dataStorage->gcode;

	clear();

	dataStorage->layerCount = 0;
	CartridgeInfo::setLayerColorList();
	setConfig(Profile::configSettings);
	
	printingInfo->init();
	printingInfo->setGcodeTempFileName();
}

void GCodeGenerator::setConfig(vector<ConfigSettings>& p_configs)
{
	//configs.at(0)은 common profile에만 해당함.. 나머지 cartridge profile에는 cartridge index를 넣어줘야 함..//
	//support는 support cartirdge를 따라 가야함..//
	int cartridgeCount = p_configs.size();
	if (cartridgeCount == 0)
		return;

	if (cartridgeCount != configs.size())
	{
		configs.clear();
		configs.resize(cartridgeCount);

		pathconfigs_outer_wall.clear();
		pathconfigs_inner_wall.clear();
		pathconfigs_infill.clear();
		pathconfigs_skin.clear();
		pathconfigs_skirt.clear();

		pathconfigs_outer_wall.resize(cartridgeCount);
		pathconfigs_inner_wall.resize(cartridgeCount);
		pathconfigs_infill.resize(cartridgeCount);
		pathconfigs_skin.resize(cartridgeCount);
		pathconfigs_skirt.resize(cartridgeCount);
	}

	gcode.b_usesCartridgeState_index = CartridgeInfo::getUseStateForProfile();

	gcode.temperature_layer_list.clear();
	for (int i = 0; i < Profile::sliceProfile.size(); ++i)
		gcode.temperature_layer_list.push_back(Profile::sliceProfile.at(i).temperature_layer_list);

	configs = p_configs;

	//gocdeExport configs setup
	gcode.setConfig(configs);

	//configsSet setup
	configs_set_flow.setConfigsSet(configs);
	configs_set_speed.setConfigsSet(configs);
	configs_set_retraction.setConfigsSet(configs);
	configs_set_cooling.setConfigsSet(configs);

}

bool GCodeGenerator::processing()
{
	gcode.setFilename(Generals::qstringTowchar_t(printingInfo->getGcodeTempFileName()));
	if (!writeGCode())
		return false;

	//process : writeGcode() --> calculatePrintingArea() --> writeStartEndCode() //
	calculatePrintingArea();

	gcode.finalize();
	setPrintingInfo();

	if (!writeStartEndCode(printingInfo->getGcodeTempFileName()))
		return false;

	return (!abort);
}

void GCodeGenerator::calculatePrintingArea()
{
	AABB aabb = AABBGetter()(models);
	Point min = Point(aabb.getMinimum().x, aabb.getMinimum().y) * 1000;
	Point max = Point(aabb.getMaximum().x, aabb.getMaximum().y) * 1000;

	Point min_target = min;
	Point max_target = max;

	int minX, minY, maxX, maxY;

	if (dataStorage->raftOutline.size() > 0)
	{
		min_target.X = std::min(dataStorage->raft_printed_line_min.X, dataStorage->raftOutline.min().X);
		min_target.Y = std::min(dataStorage->raft_printed_line_min.Y, dataStorage->raftOutline.min().Y);
		max_target.X = std::max(dataStorage->raft_printed_line_max.X, dataStorage->raftOutline.max().X);
		max_target.Y = std::max(dataStorage->raft_printed_line_max.Y, dataStorage->raftOutline.max().Y);
	}
	else if (dataStorage->skirt.size() > 0)
	{
		//skirt printing point 추출..
		min_target = dataStorage->skirt.min();
		max_target = dataStorage->skirt.max();
	}
	else if (dataStorage->supportData->generated)
	{
		min_target = dataStorage->support_printed_line_min;
		max_target = dataStorage->support_printed_line_max;
	}

	//target과 model AABB point 비교..
	minX = std::min(min.X, min_target.X);
	minY = std::min(min.Y, min_target.Y);
	maxX = std::max(max.X, max_target.X);
	maxY = std::max(max.Y, max_target.Y);

	dataStorage->min_printing = FPoint3(INT2MM(minX), INT2MM(minY), 0);
	dataStorage->max_printing = FPoint3(INT2MM(maxX), INT2MM(maxY), aabb.getMaximum().z);
}

QString GCodeGenerator::getWarningMessage()
{
	if (UserProperties::thinMessageCheck)
		return "";
	if (b_raftPathLengthLimit)
		return MessageAlert::tr("raft_small_area");
	return "";
}

void GCodeGenerator::clear()
{
	gcode.clear();
	gcode.resetTotalPrintTime();
	b_reverseExtruderForSupport = false;
	b_raftPathLengthLimit = false;
	b_firstMultipleWipetower = true;
	b_needWipetowerFirst = false;
	b_skirtPrinted = false;
	currentLayers_with_adhesion = 0;
}

bool GCodeGenerator::writeGCode()
{
	if (Generals::isReplicationUIMode())
	{
		writeAdjustZGapCode();

		char adjustZGapCode_temperature[100];
		for (int i = 0; i < configs.size(); i++)
		{
			sprintf(adjustZGapCode_temperature, "M104 T%i S%i		;Heat up the %ith nozzle - print temperature(replication print)\n", i, gcode.nozzle_print_temperature[i], i);
			gcode.writeCode(adjustZGapCode_temperature);
		}
		gcode.writeNewLine();
		gcode.writeNewLine();
		gcode.writeCode("G28 X ;Location Change\n");
		gcode.writeNewLine();
		gcode.writeCode("M605 S2 ;Replication Print\n");
	}

	gcode.gcodePaths.paths.clear();

	//write raft..//
	if(configs[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
		writeRaftGcode();

	emit signal_setLabelText(MessageProgress::tr("Generating GCode.."));

	qDebug() << "writeGcode 5";

	if (configs[0].infill_pattern > 3)
	{
		infill_tree.clear();
		infill_tree.resize(models.size());

		for (int i = 0; i < models.size(); ++i)
		{
			int cart_idx = models[i]->getCartridgeIndexes()[0];
			InfillVoxel* infill = &infill_tree[i];
			infill->infill_type = configs[cart_idx].infill_pattern - 4;
			infill->setMesh(models[i]->getMesh());

			if (infill->infill_type == 2)
			{
				infill->infill_type = 0;
				infill->run(double(configs[cart_idx].sparse_infill_line_distance)*0.001, 1);
			}
			else if (infill->infill_type == 3)
			{
				infill->infill_type = 1;
				infill->run(double(configs[cart_idx].sparse_infill_line_distance)*0.001, 1);
			}
			else
				infill->run(double(configs[cart_idx].sparse_infill_line_distance)*0.001, 0);
		}
	}


	//기본값들 미리 setting..//
	//firstToolChangeInfo.resize(parent->getCartridgeCountTotal() * 2); //push_back으로 대처..
	//gcode.T0_firstToolChanged = false;
	//gcode.T1_firstToolChanged = false;
	b_skirtPrinted = false;

	//support_printed_line min, max initialization..//
	dataStorage->support_printed_line_min = Point(99999999999, 99999999999);
	dataStorage->support_printed_line_max = Point(0, 0);

	//tool change order//
	bool ascendingFirst;

	////////////////////////////////////////////////////////////////////////
	//configs vector profile setting//
	//std::vector<int> configsFilamentFlow;
	//configsFilamentFlow.clear();

	//std::vector<int> configsMoveSpeed;
	//configsMoveSpeed.clear();

	//std::vector<int> configsRetractionMinimalDistance;
	//configsRetractionMinimalDistance.clear();

	//std::vector<double> configsMinimalLayerTime;
	//configsMinimalLayerTime.clear();

	//std::vector<int> configsMinimalSpeed;
	//configsMinimalSpeed.clear();

	//for (int i = 0; i < configs.size(); i++)
	//{
	//	configsFilamentFlow.push_back(configs[i].overall_flow);
	//	configsMoveSpeed.push_back(configs[i].travel_speed);
	//	configsRetractionMinimalDistance.push_back(configs[i].retraction_minimal_distance);
	//	configsMinimalLayerTime.push_back(configs[i].minimal_layer_time);
	//	configsMinimalSpeed.push_back(configs[i].minimal_feedrate);
	//}
	////////////////////////////////////////////////////////////////////////

	std::vector<PairIndexes> pairIndexInfo_cart_volume;
	std::vector<PairIndexes> ascendingCartInfo;
	std::vector<PairIndexes> descendingCartInfo;

	for (int i = 0; i < models.size(); i++)
	{
		int cartIdx = models[i]->getCartridgeIndexes()[0];
		pairIndexInfo_cart_volume.push_back(PairIndexes(cartIdx, i));
	}

	//ascending or descending
	ascendingCartInfo = pairIndexInfo_cart_volume;
	descendingCartInfo = pairIndexInfo_cart_volume;

	//sorting//
	std::sort(ascendingCartInfo.begin(), ascendingCartInfo.end(), [](const PairIndexes a, const PairIndexes b) { return (a.cartridgeIndex < b.cartridgeIndex); });
	std::sort(descendingCartInfo.begin(), descendingCartInfo.end(), [](const PairIndexes a, const PairIndexes b) { return (a.cartridgeIndex > b.cartridgeIndex); });

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//layer loops//
	for (unsigned int layerNr = 0; layerNr < dataStorage->totalLayer; layerNr++)
	{
		int supportCartIndex = configs[0].support_main_cartridge_index;

		//std::vector<int> extrusionWidth;
		//extrusionWidth.resize(configs.size());

		//for (int i = 0; i < configs.size(); i++)
		//{
		//	extrusionWidth.at(i) = configs[i].extrusion_width;

		//	if (layerNr == 0)
		//		extrusionWidth.at(i) = configs[i].initial_layer_extrusion_width;
		//}
		
		if (static_cast<int>(layerNr) < configs[0].slower_layers_count)
		{
			int n = configs[0].slower_layers_count; //common

			for (int i = 0; i < configs.size(); i++)
			{
#define MIN_TEMP(a,b) a < b ? a : b
#define SPEED_SMOOTH(speed) MIN_TEMP((speed), (((speed)*layerNr)/n + (configs[i].initial_layer_speed*(n-layerNr)/n)))
				pathconfigs_outer_wall.at(i).setData(SPEED_SMOOTH(configs[i].outer_wall_speed), getExtrusionWidthByLayer(layerNr, configs[i].wall_extrusion_width), "WALL-OUTER", configs.front().spiralize_mode);
				pathconfigs_inner_wall.at(i).setData(SPEED_SMOOTH(configs[i].inner_wall_speed), getExtrusionWidthByLayer(layerNr, configs[i].wall_extrusion_width), "WALL-INNER", configs.front().spiralize_mode);
				pathconfigs_infill.at(i).setData(SPEED_SMOOTH(configs[i].infill_speed), getExtrusionWidthByLayer(layerNr, configs[i].infill_extrusion_width), "FILL", configs.front().spiralize_mode);
				pathconfigs_skin.at(i).setData(SPEED_SMOOTH(configs[i].top_bottom_speed), getExtrusionWidthByLayer(layerNr, configs[i].top_bottom_extrusion_width), "SKIN", configs.front().spiralize_mode);
				pathconfigs_support_main.setData(SPEED_SMOOTH(configs[supportCartIndex].support_main_speed), getExtrusionWidthByLayer(layerNr, configs[supportCartIndex].support_main_extrusion_width), "SUPPORT_MAIN", configs.front().spiralize_mode);
				pathconfigs_support_interface_roof.setData(SPEED_SMOOTH(configs[supportCartIndex].support_interface_roof_speed), getExtrusionWidthByLayer(layerNr, configs[supportCartIndex].support_interface_roof_extrusion_width), "SUPPORT_INTERFACE_ROOF", configs.front().spiralize_mode);
				pathconfigs_support_interface_floor.setData(SPEED_SMOOTH(configs[supportCartIndex].support_interface_floor_speed), getExtrusionWidthByLayer(layerNr, configs[supportCartIndex].support_interface_floor_extrusion_width), "SUPPORT_INTERFACE_FLOOR", configs.front().spiralize_mode);
				pathconfigs_skirt.at(i).setData(SPEED_SMOOTH(configs[i].initial_layer_speed), getExtrusionWidthByLayer(layerNr, configs.front().initial_layer_extrusion_width), "SKIRT", configs.front().spiralize_mode);
				//pathconfigs_brim.setData(SPEED_SMOOTH(configs.front().brim_speed), getExtrusionWidthByLayer(layerNr, configs.front().brim_extrusion_width), "BRIM", configs.front().spiralize_mode);
				pathconfigs_wipe_tower.setData(SPEED_SMOOTH(configs.front().wipe_tower_speed), getExtrusionWidthByLayer(layerNr, configs.front().extrusion_width), "WIPE_TOWER", configs.front().spiralize_mode);

				//preretract0Config.setData(SPEED_SMOOTH(config.outer_wall_print_speed), extrusionWidth, "PRERETRACT0");
				//preretractXConfig.setData(SPEED_SMOOTH(config.inner_wall_print_speed), extrusionWidth, "PRERETRACTX");
				//overmove0Config.setData(SPEED_SMOOTH(config.outer_wall_print_speed), 0, "OVERMOVE0");
				//overmoveXConfig.setData(SPEED_SMOOTH(config.inner_wall_print_speed), 0, "OVERMOVEX");
				//if (config.preretration0Length != 0)
				// 	preretract0Config.lineWidth = -(extrusionWidth * config.preretration0FilamentLength / (double)config.preretration0Length);
				//if (config.preretrationXLength != 0)
				// 	preretractXConfig.lineWidth = -(extrusionWidth * config.preretrationXFilamentLength / (double)config.preretrationXLength);
#undef SPEED_SMOOTH
#undef MIN_TEMP
			}
		}
		else
		{
			for (int i = 0; i < configs.size(); i++)
			{
				pathconfigs_outer_wall.at(i).setData(configs[i].outer_wall_speed, getExtrusionWidthByLayer(layerNr, configs[i].wall_extrusion_width), "WALL-OUTER", configs.front().spiralize_mode);
				pathconfigs_inner_wall.at(i).setData(configs[i].inner_wall_speed, getExtrusionWidthByLayer(layerNr, configs[i].wall_extrusion_width), "WALL-INNER", configs.front().spiralize_mode);
				pathconfigs_infill.at(i).setData(configs[i].infill_speed, getExtrusionWidthByLayer(layerNr, configs[i].infill_extrusion_width), "FILL", configs.front().spiralize_mode);
				pathconfigs_skin.at(i).setData(configs[i].top_bottom_speed, getExtrusionWidthByLayer(layerNr, configs[i].top_bottom_extrusion_width), "SKIN", configs.front().spiralize_mode);
				pathconfigs_support_main.setData(configs[supportCartIndex].support_main_speed, getExtrusionWidthByLayer(layerNr, configs[supportCartIndex].support_main_extrusion_width), "SUPPORT_MAIN", configs.front().spiralize_mode);
				pathconfigs_support_interface_roof.setData(configs[supportCartIndex].support_interface_roof_speed, getExtrusionWidthByLayer(layerNr, configs[supportCartIndex].support_interface_roof_extrusion_width), "SUPPORT_INTERFACE_ROOF", configs.front().spiralize_mode);
				pathconfigs_support_interface_floor.setData(configs[supportCartIndex].support_interface_floor_speed, getExtrusionWidthByLayer(layerNr, configs[supportCartIndex].support_interface_floor_extrusion_width), "SUPPORT_INTERFACE_FLOOR", configs.front().spiralize_mode);
				pathconfigs_skirt.at(i).setData(configs[i].initial_layer_speed, getExtrusionWidthByLayer(layerNr, configs.front().initial_layer_extrusion_width), "SKIRT", configs.front().spiralize_mode);
				//pathconfigs_brim.setData(configs.front().brim_speed, getExtrusionWidthByLayer(layerNr, configs.front().brim_extrusion_width), "BRIM", configs.front().spiralize_mode);
				pathconfigs_wipe_tower.setData(configs.front().wipe_tower_speed, getExtrusionWidthByLayer(layerNr, configs.front().extrusion_width), "WIPE_TOWER", configs.front().spiralize_mode);

				//preretract0Config.setData(config.outer_wall_print_speed, extrusionWidth, "PRERETRACT0");
				//preretractXConfig.setData(config.inner_wall_print_speed, extrusionWidth, "PRERETRACTX");
				//overmove0Config.setData(config.outer_wall_print_speed, 0, "OVERMOVE0");
				//overmoveXConfig.setData(config.inner_wall_print_speed, 0, "OVERMOVEX");
				//if (config.preretration0Length != 0)
				// 	preretract0Config.lineWidth = -(extrusionWidth * config.preretration0FilamentLength / (double)config.preretration0Length);
				//if (config.preretrationXLength != 0)
				// 	preretractXConfig.lineWidth = -(extrusionWidth * config.preretrationXFilamentLength / (double)config.preretrationXLength);
			}
		}

		if (layerNr == 0)
			gcode.setExtrusion(configs[0].initial_layer_height, configs[0].filament_diameter, configs_set_flow);
		else
			gcode.setExtrusion(configs[0].layer_height, configs[0].filament_diameter, configs_set_flow);

		engine::GCodePlanner gcodeLayer(gcode, configs_set_speed.travel_speed, configs_set_retraction.retraction_minimal_distance, configs[0].spiralize_mode);
		//gcodeLayer.pathOptimizationParameter[0] = config.pathOptimizationParameter[0];
		//gcodeLayer.pathOptimizationParameter[1] = config.pathOptimizationParameter[1];
		//gcodeLayer.pathOptimizationParameter[2] = config.pathOptimizationParameter[2];
		//gcodeLayer.filteringAngle = config.pathOptimizationFilteringAngle;

		int32_t z = configs[0].initial_layer_height + layerNr * configs[0].layer_height;
		z += configs[0].adjust_z_gap_thickness * configs[0].adjust_z_gap_layers + configs[0].raft_base_thickness + configs[0].raft_interface_thickness + configs[0].raft_surface_layers*configs[0].raft_surface_thickness;

		if (configs[0].raft_base_thickness > 0 && configs[0].raft_interface_thickness > 0)
		{
			if (layerNr == 0)
			{
				z += configs[0].raft_airgap_initial_layer; //raft_airgap_layer0
				gcode.limitBedlifting = true;
			}
			else
			{
				z += configs[0].raft_airgap_all; //raft_airgap_all
				gcode.limitBedlifting = false;
			}
		}
		gcode.setZ(z);
		gcode.resetStartPosition();

		if (layerNr == (dataStorage->totalLayer - 1))
			dataStorage->z_height_base = z - layerNr * configs[0].layer_height;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//wipe tower pair index//
		std::vector<PairIndexes> pairIndexInfo_cart_wipetower;
		//pairIndexInfo_cart_wipetower.resize(configs.size());

		//setting PairInfo class//
		//wipeTower index : 0 -> main
		//wipeTower index : 1 -> sub
		if (configs.size() > 1 && configs[0].wipe_tower_enable)
		{
			for (int i = 0; i < configs.size(); i++)
			{
				int wipeTowerIdx;
				//i -> wipetower main
				if (configs[0].wipe_tower_outer_cartridge_index == i)
					wipeTowerIdx = 0;
				//i -> wipetower sub
				else
					wipeTowerIdx = 1;
				pairIndexInfo_cart_wipetower.push_back(PairIndexes(i, wipeTowerIdx));
			}
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//checking printing order between support and volume..//
		bool b_supportFirstPrint = false;

		if (dataStorage->supportData->generated && configs[0].support_main_cartridge_index >= 0)
			b_supportFirstPrint = true;
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//set pair index information and finding volumindex//
		if (layerNr == 0)
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//selection between ascending and descending..//

			//extruder 고려순위...//
			// 1) support cartridge index
			// 2) bed adhesion cartridge index
			// 3) 위 경우에 해당되지 않을 때는 ascending

			int currentExtruder = gcodeLayer.getExtruder();
			int supportExtruder = configs[0].support_main_cartridge_index;

			//support extruder에 맞추어 T순서 정렬..//
			if (b_supportFirstPrint)
			{
				if (supportExtruder == 0)
					ascendingFirst = true;
				else if (supportExtruder == 1)
					ascendingFirst = false;
				//support는 extruder change없이, wipe tower를 출력하므로 아래에서 강제로 넣어줄 필요가 없음..//
			}
			else
			{
				//support가 없고, adhesion이 있는 경우 (raft, brim, skirt) : T순서 정렬..//
				if (currentExtruder == 0)
					ascendingFirst = true;
				else if (currentExtruder == 1)
					ascendingFirst = false;
				//T100//
				//이 때는 어느것이나 상관없음 --> 일단 ascending으로 함..//
				else
					ascendingFirst = true;

				//support가 없으면 volume전에 set extruder를 하므로, change가 안일어남.. wipe tower가 강제로 필요..//
				b_needWipetowerFirst = true;
			}

			int cart_idx;
			int volume_idx;

			if (Generals::isLayerColorModeOn())
			{
				cart_idx = CartridgeInfo::getCartIndexFromCartridgeLayerList(layerNr);
			}
			else
			{
				if (ascendingFirst)
					volume_idx = ascendingCartInfo.at(0).index;
				else
					volume_idx = descendingCartInfo.at(0).index;

				//여기서 볼륨별로 카트리지 번호 가져와서 ..
				cart_idx = models[volume_idx]->getCartridgeIndexes()[0];
			}

			gcodeLayer.setExtruder(cart_idx);
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//add skirt//
		if (layerNr == 0 && !(configs[0].raft_base_thickness > 0 && configs[0].raft_interface_thickness > 0) && dataStorage->skirt.size() > 0 && !b_skirtPrinted)
		{
			gcodeLayer.addTravel(dataStorage->skirt.back().closestPointTo(gcode.getPositionXY()));
			gcodeLayer.addPolygonsByOptimizer(dataStorage->skirt, &pathconfigs_skirt.at(gcodeLayer.getExtruder()));

			b_skirtPrinted = true;
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//add support//
		//support switching 기능 삭제..//
		if (b_supportFirstPrint)
			addSupportToGCode(gcodeLayer, layerNr, pairIndexInfo_cart_wipetower);

		//tic2 = clock();
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//add volume// 
		for (int i = 0; i < pairIndexInfo_cart_volume.size(); i++)
		{
			if (ascendingFirst)
				addVolumeLayerToGCode(gcodeLayer, ascendingCartInfo.at(i).index, layerNr, pairIndexInfo_cart_wipetower);
			else
				addVolumeLayerToGCode(gcodeLayer, descendingCartInfo.at(i).index, layerNr, pairIndexInfo_cart_wipetower);
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//clock3 += (clock() - tic3) / (double)CLOCKS_PER_SEC;


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//fan control part//
		//Finish the layer by applying speed corrections for minimal layer times
		gcodeLayer.forceMinimalLayerTime(configs_set_cooling.minimal_layer_time, configs_set_cooling.minimal_feedrate);

		std::vector<int> fanSpeed;
		fanSpeed.resize(configs.size());

		for (int i = 0; i < fanSpeed.size(); i++)
		{
			fanSpeed.at(i) = configs[i].fan_speed_regular;

			if (gcodeLayer.getExtrudeSpeedFactor() <= 50)
			{
				fanSpeed.at(i) = configs[i].fan_speed_max;
			}
			else
			{
				int n = gcodeLayer.getExtrudeSpeedFactor() - 50;
				fanSpeed.at(i) = configs[i].fan_speed_regular * n / 50 + configs[i].fan_speed_max * (50 - n) / 50;
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		gcode.writeNewLine();
		gcode.writeComment("---------------------------------------------");
		gcode.writeComment("LAYER:%d", layerNr);

		currentLayers_with_adhesion++;

		char M532_code[100];
		sprintf_s(M532_code, "M532 L%i", currentLayers_with_adhesion);
		gcode.writeCode(M532_code);
		gcode.writeNewLine();

		int current_cartridge_index = gcode.getExtruderNr();


		if (layerNr == 0)
		{
			if (gcode.platformAdhesion == Generals::PlatformAdhesion::Raft)
			{
				int temp_temperature;
				char temp_temperature_code[100];

				if (gcode.raftTemperatureControl && gcode.raftSurfaceLastTemperature != gcode.nozzle_print_temperature[current_cartridge_index])
				{
					//raft temperature control--> print temperature check!..//
					temp_temperature = gcode.nozzle_print_temperature[current_cartridge_index];

					sprintf_s(temp_temperature_code, "M104 T%i S%d     ;print temperature", current_cartridge_index, temp_temperature);

					gcode.writeCode(temp_temperature_code);
					gcode.writeNewLine();
				}
			}
		}
		else if (gcode.temperature_layer_setting_enabled[current_cartridge_index])
		{
			int temp_temperature;
			char temp_temperature_code[100];

			//temperature profile bed..//
			if (Profile::isTemperatureSetPointAtLayer(&gcode.temperature_layer_list.at(current_cartridge_index), layerNr))
			{
				temp_temperature = Profile::getTemperatureByLayer(&gcode.temperature_layer_list.at(current_cartridge_index), layerNr, gcode.nozzle_print_temperature[current_cartridge_index]);

				sprintf_s(temp_temperature_code, "M104 T%i S%d     ;print temperature(table)", current_cartridge_index, temp_temperature);

				gcode.writeCode(temp_temperature_code);
				gcode.writeNewLine();
			}
		}

		//if (currentLayers_with_adhesion < 4)
		//{
		//	if (configs[0].filament_material == "ABS" || configs[1].filament_material == "ABS")
		//	{
		//		if (Profile::machineProfile.machine_model == "5X" || Profile::machineProfile.machine_model == "7X")
		//		{
		//			double M206_z_height = 0;

		//			if (currentLayers_with_adhesion == 1)
		//			{
		//				M206_z_height = 0.2;
		//			}
		//			else if (currentLayers_with_adhesion == 2 || currentLayers_with_adhesion == 3)
		//			{
		//				M206_z_height = 0.1;
		//			}

		//			char M206_code[100];
		//			sprintf_s(M206_code, "M206 Z%.1f", M206_z_height);
		//			gcode.writeCode(M206_code);
		//			gcode.writeNewLine();
		//		}
		//	}
		//}

		if (Generals::isLayerColorModeOn() && !CartridgeInfo::pauseList.empty() && !layerNr == 0)
		{
			if (CartridgeInfo::isPauseFromPauseLayerList(layerNr))
			{
				gcode.writeNewLine();
				gcode.writeCode("TX		;Pause");
				gcode.writeNewLine();
			}
		}


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//path print time calculation for preheat//
		if (gcode.preheat_OR_enabled)
		{
			gcodeLayer.pathPrintTimeInfo_accumulated_vector.clear();
			gcode.resetPathPrintTime_timeCalculate();
			gcode.resetExtrusionValue_timeCalculate();
			gcode.reset_pathPrintTime_pathEndPrintTime();

			gcode.clear_timeCalculate();

			if (gcode.b_multiExtruder)
			{
				gcodeLayer.getPathPrintTimeInfo(configs[0].cool_head_lift > 0, static_cast<int>(layerNr) > 0 ? configs[0].layer_height : configs[0].initial_layer_height, layerNr);

				gcodeLayer.getPreheatStartTime();
			}
			else
			{
				gcode.time_end.clear();
				gcode.time_standby.clear();
				gcode.time_preheat.clear();
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//temp test information by layer..//
		//if (gcode.b_multiExtruder && gcode.preheat_OR_enabled)
		//{
		//	gcode.writeNewLine();

		//	for (int i = 0; i < gcode.cartridgeCountTotal; i++)
		//	{
		//		QString timeStr;
		//		int counter_extruder = std::abs(i - 1);

		//		timeStr.clear();
		//		timeStr = QString("T%1 time_end (path print end time of T%2): %3 ").arg(i).arg(i).arg(gcode.time_end.at(i));
		//		gcode.writeComment(timeStr.toStdString().c_str());

		//		timeStr.clear();
		//		timeStr = QString("T%1 time_threshold (based of T%2)        : %3 ").arg(i).arg(counter_extruder).arg(gcode.preheat_threshold_time[i]);
		//		gcode.writeComment(timeStr.toStdString().c_str());

		//		timeStr.clear();
		//		timeStr = QString("T%1 time_standby (based of T%2)          : %3 ").arg(i).arg(counter_extruder).arg(gcode.time_standby.at(i));
		//		gcode.writeComment(timeStr.toStdString().c_str());

		//		timeStr.clear();
		//		timeStr = QString("T%1 time_preheat (preheat code for T%2)  : %3 ").arg(i).arg(counter_extruder).arg(gcode.time_preheat.at(i));
		//		gcode.writeComment(timeStr.toStdString().c_str());

		//	}

		//	gcode.writeNewLine();
		//}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////////////////////////////////
		//fan control 추가된 부분.by sewoongy
		//2016.04.06.
		//
		//DP200 : 기존의 fan 방식 그대로 적용..
		//
		//DP201 : layer0, layer1, layer2까지 강제 적용.
		//DP201 : raft의 유무에 따라 / raft surface 마지막 레이어 경과시간에 따라 
		/////////////////////////////////////////////////////////////////

		if (Profile::machineProfile.raft_base_fan_control_enabled.value)
		{
			int adhesionCartridgeIndex = configs[0].adhesion_cartridge_index;
			//raft가 있는 경우...
			if (configs[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
			{
				//raft surface print time이 300초 이상일 경우..
				//if (raftPrintTime.at(raftPrintTime.size() - 1) > 300)
				//{
				if (layerNr == 0)		gcode.writeFanCommand(adhesionCartridgeIndex, 12.5);
				else if (layerNr == 1)	gcode.writeFanCommand(adhesionCartridgeIndex, 25);
				else					gcode.writeFanCommand(adhesionCartridgeIndex, 100);
			}
			else //raft가 없는 경우 --> skirt or brim
			{
				//layer가 0일 때, writeMove에서 fan code 입력됨.
				if (layerNr == 0) { /*skip*/ }//gcode.writeFanCommand(12.5);
				else if (layerNr == 1)	gcode.writeFanCommand(adhesionCartridgeIndex, 25);
				else if (layerNr == 2)	gcode.writeFanCommand(adhesionCartridgeIndex, 50);
				else					gcode.writeFanCommand(adhesionCartridgeIndex, 100);
			}
		}
		else
		{
			int cartCountTotal = configs.size();
			if (Generals::isReplicationUIMode())
				cartCountTotal = 1;

			for (int i = 0; i < cartCountTotal; i++)
			{
				if (static_cast<int>(layerNr) < configs[i].fan_full_on_layer_number)
				{
					//Slow down the fan on the layers below the [fanFullOnLayerNr], where layer 0 is speed 0.
					fanSpeed.at(i) = fanSpeed.at(i) * layerNr / configs[i].fan_full_on_layer_number;
				}

				//fan speed가 machine profile의 speed low limit보다 작고 0보다 클 경우 제한을 함..//
				//disable cooling fan --> fan speed == 0 --> 0은 그대로 0d으로 출력함..//
				if (fanSpeed.at(i) < Profile::machineProfile.fan_speed_low_limit_value.value && fanSpeed.at(i) != 0)
					fanSpeed.at(i) = Profile::machineProfile.fan_speed_low_limit_value.value;

				gcode.writeFanCommand(i, fanSpeed.at(i));
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////////////////////////////////////////////////////////////
		//tic4 = clock();

		//이전 writeGcode2 bakcup..//
		//gcodeLayer.writeGCode2(configs[0].cool_head_lift > 0, static_cast<int>(layerNr) > 0 ? configs[0].layer_height : configs[0].initial_layer_height, layerNr, &firstToolChangeInfo);
		
		gcodeLayer.writeGCode2(
			configs[0].cool_head_lift > 0,
			static_cast<int>(layerNr) > 0 ? configs[0].layer_height : configs[0].initial_layer_height,
			layerNr,
			&firstToolChangeInfo,
			gcodeLayer.pathPrintTimeInfo_accumulated_vector,
			gcode.time_preheat);



		//airgap이 있을 때 true key..//
		gcode.limitBedlifting = false;

		//clock4 += (clock() - tic4) / (double)CLOCKS_PER_SEC;
		//tic5 = clock();

		//gcodePaths.paths.insert(gcodePaths.paths.end(), gcode.gcodePaths.paths.begin(), gcode.gcodePaths.paths.end());
		dataStorage->insertGcodePath(gcode.gcodePaths);
		//위의 update()를 통해 gcode.gcodePaths는 buffer에 입력하면서 line rendering 준비를 마침..//
		//rendering 준비 마친 후, clear() -> 메모리 확보..//

		//clock5 += (clock() - tic5) / (double)CLOCKS_PER_SEC;
		if (progressHandler)
		{
			if (progressHandler->wasCanceled())
			{
				//dataStorage->b_slicing_flag = false;
				fclose(gcode.f);
				std::cout << "canceled!" << std::endl;
				return false;
			}
		}
		if (abort)
			return false;
		emit signal_setValue((double)layerNr / (double)dataStorage->totalLayer * 100);

		//temp test information by layer..//
		//if (false)
		//{
		//	//////////////////////////////////////////////////////////////////////////////////////////
		//	//layer별 time check//
		//	char tempLayerTime[200];
		//	sprintf_s(tempLayerTime, "- print lap time : %f", gcode.getTotalPrintTime() - tempTime);
		//	gcode.writeComment(tempLayerTime);

		//	char tempLayerTimeAccumulated[200];
		//	sprintf_s(tempLayerTimeAccumulated, "- print accumulated time : %f", gcode.getTotalPrintTime());
		//	gcode.writeComment(tempLayerTimeAccumulated);

		//	tempTime = gcode.getTotalPrintTime();

		//	//////////////////////////////////////////////////////////////////////////////////////////
		//}

	}

	emit signal_setValue(100);
	//clock1 = (clock() - tic1) / (double)CLOCKS_PER_SEC;
	//std::printf("clock1 : %lf, clock2 : %lf, clock3 : %lf, clock4 : %lf, clock5 : %lf\n", clock1, clock2, clock3, clock4, clock5);
	//std::printf("support --> clock1 : %lf, clock2 : %lf, clock3 : %lf, clock4 : %lf, clock5 : %lf\n", global_clock[0], global_clock[1],
	//	global_clock[2], global_clock[3], global_clock[4]);

	qDebug() << "writeGcode 6";
	gcode.tellFileSize();
	//gcode.writeFanCommand(0); //swyang

	//z방향 최대 출력 크기 저장..
	dataStorage->min_printing.z = 0.0;
	dataStorage->max_printing.z = double(gcode.zPos) / 1000;
	return (!abort);
}

void GCodeGenerator::setPrintingInfo()
{
	printingInfo->setTotalPrintTime(gcode.getTotalPrintTime());
	int cartridgetCount = printingInfo->getCartridgeCount();
	int count = 0;
	for (int i = 0; i < cartridgetCount; i++)
	{
		float filaAmount = gcode.getTotalFilamentUsed(i);
		if (filaAmount != 0)
		{
			float filamentDensity = getFilamentDensity(printingInfo->getFilaMaterial(i));
			float diameter = INT2MM(Profile::configSettings[i].filament_diameter);
			float filaMass = (M_PI*pow(diameter, 2) / 4) * filaAmount * filamentDensity;

			printingInfo->setFilaAmount(i, filaAmount);
			//printingInfo->setUsedState(i, true);
			printingInfo->setFilaMass(i, filaMass);
			count++;
		}
	}
	printingInfo->setUsedCartridgeCount(count);
}

//Add a single layer from a single mesh-volume to the GCode
void GCodeGenerator::addVolumeLayerToGCode(engine::GCodePlanner& gcodeLayer, int volumeIdx, int layerNr, std::vector<PairIndexes> pairInfo_cart_wipetower)
{
	if (layerNr >= models[volumeIdx]->sliceLayers.size())
		return;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//cartridge index change part//
	//cartridge를 선택하는 데 있어, 두 가지 방법..//volume의 cartridge or coloredlayer//

	int cart_idx;
	if (Generals::isLayerColorModeOn())
	{
		cart_idx = CartridgeInfo::getCartIndexFromCartridgeLayerList(layerNr);
	}
	else
	{
		//여기서 볼륨별로 카트리지 번호 가져와서 ..
		cart_idx = models[volumeIdx]->getCartridgeIndexes()[0];
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//extrude를 바꾸는데.. 이 부분을 이용해서 Coloredlayer class에서 정보를 가져와 써줄 것..
	//extruder change 
	int prevExtruder = gcodeLayer.getExtruder();
	bool b_extruderChanged = gcodeLayer.setExtruder(cart_idx);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//filament flow vector setting//
	//std::vector<int> configsFilamentFlowLayer0;
	//std::vector<int> configsFilamentFlow;
	//configsFilamentFlowLayer0.clear();
	//configsFilamentFlow.clear();

	//for (int i = 0; i < configs.size(); i++)
	//{
	//	configsFilamentFlowLayer0.push_back(configs[i].initial_layer_flow);
	//	configsFilamentFlow.push_back(configs[i].overall_flow);
	//}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SliceLayer* layer = &models[volumeIdx]->sliceLayers[layerNr];

	//wipe tower가 있고, layer number가 0일 경우, raft일 경우 : wipe tower를 2번 출력 --> 접착력 증가 및 airgap 제거효과.//
	if (configs[0].wipe_tower_enable && layerNr == 0)
	{
		if (b_needWipetowerFirst)
		{
			if (b_reverseExtruderForSupport)
			{
				//addSupport에서 support의 cartridge를 변경한 경우 --> 여기서는 그대로 support cartridge index대로 출력함..//
				addWipeTower(gcodeLayer, layerNr, configs[0].support_main_cartridge_index, pairInfo_cart_wipetower);

				if (configs[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
					addWipeTower(gcodeLayer, layerNr, configs[0].support_main_cartridge_index, pairInfo_cart_wipetower);
			}
			else
			{
				//addSupport에서 cartridge를 변경하지 않은 경우 --> 그대로 volume에 해당하는 cartridge index대로 출력함..//
				addWipeTower(gcodeLayer, layerNr, cart_idx, pairInfo_cart_wipetower);

				if (configs[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
					addWipeTower(gcodeLayer, layerNr, cart_idx, pairInfo_cart_wipetower);
			}

			b_needWipetowerFirst = false;
		}
		else if (b_extruderChanged)
		{
			if (b_reverseExtruderForSupport)
			{
				//addSupport에서 support의 cartridge를 변경한 경우 --> 여기서는 그대로 support cartridge index대로 출력함..//
				addWipeTower(gcodeLayer, layerNr, configs[0].support_main_cartridge_index, pairInfo_cart_wipetower);

				if (configs[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
					addWipeTower(gcodeLayer, layerNr, configs[0].support_main_cartridge_index, pairInfo_cart_wipetower);
			}
			else
			{
				//addSupport에서 cartridge를 변경하지 않은 경우 --> 그대로 volume에 해당하는 cartridge index대로 출력함..//
				addWipeTower(gcodeLayer, layerNr, cart_idx, pairInfo_cart_wipetower);

				if (configs[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
					addWipeTower(gcodeLayer, layerNr, cart_idx, pairInfo_cart_wipetower);
			}
		}
	}

	//일반적인 wipe tower 출력 부분..//
	if (b_extruderChanged && configs[0].wipe_tower_enable && layerNr != 0)
	{
		addWipeTower(gcodeLayer, layerNr, cart_idx, pairInfo_cart_wipetower);
	}


	//if (model->oozeShield.size() > 0 && model->volumes.size() > 1)
	//{
	//	gcodeLayer.setAlwaysRetract(true);
	//	gcodeLayer.addPolygonsByOptimizer(model->oozeShield[layerNr], &skirtConfig);
	//	gcodeLayer.setAlwaysRetract(config.enableCombing == COMBING_OFF);
	//}

	if (layerNr == 0)
		gcode.setExtrusion(configs[0].initial_layer_height, configs[0].filament_diameter, configs_set_flow);
	else
		gcode.setExtrusion(configs[0].layer_height, configs[0].filament_diameter, configs_set_flow);

	gcodeLayer.pathconfigs_travel[cart_idx].speed = configs[cart_idx].travel_speed;
	gcodeLayer.retractionMinimalDistance[cart_idx] = configs[cart_idx].retraction_minimal_distance;


	if (configs[0].simple_mode)
	{
		engine::Polygons Polygons;
		for (unsigned int partNr = 0; partNr < layer->parts.size(); partNr++)
		{
			for (unsigned int n = 0; n < layer->parts[partNr].outline.size(); n++)	//?? n이 0이 아닌 case가 있는지?
			{
				for (unsigned int m = 1; m < layer->parts[partNr].outline[n].size(); m++)
				{
					engine::Polygon p;
					p.add(layer->parts[partNr].outline[n][m - 1]);
					p.add(layer->parts[partNr].outline[n][m]);
					Polygons.add(p);
				}
				if (layer->parts[partNr].outline[n].size() > 0)
				{
					engine::Polygon p;
					p.add(layer->parts[partNr].outline[n][layer->parts[partNr].outline[n].size() - 1]);
					p.add(layer->parts[partNr].outline[n][0]);
					Polygons.add(p);
				}
			}
		}
		for (unsigned int n = 0; n < layer->openLines.size(); n++)
		{
			for (unsigned int m = 1; m < layer->openLines[n].size(); m++)
			{
				engine::Polygon p;
				p.add(layer->openLines[n][m - 1]);
				p.add(layer->openLines[n][m]);
				Polygons.add(p);
			}
		}
		if (configs[0].spiralize_mode)
			pathconfigs_outer_wall[cart_idx].spiralize = true;

		gcodeLayer.addPolygonsByOptimizer(Polygons, &pathconfigs_outer_wall[cart_idx]);
		return;
	}



	engine::PathOrderOptimizer partOrderOptimizer(gcode.getStartPositionXY());
	partOrderOptimizer.pathOptimizationParameter[0] = gcodeLayer.pathOptimizationParameter[0];
	partOrderOptimizer.pathOptimizationParameter[1] = gcodeLayer.pathOptimizationParameter[1];
	partOrderOptimizer.pathOptimizationParameter[2] = gcodeLayer.pathOptimizationParameter[2];
	partOrderOptimizer.prevPoint = gcodeLayer.prevPosition;

	for (unsigned int partNr = 0; partNr < layer->parts.size(); partNr++)
		partOrderOptimizer.addPolygon(layer->parts[partNr].insets[0][0]);
	partOrderOptimizer.optimize();


	for (unsigned int partCounter = 0; partCounter < partOrderOptimizer.polyOrder.size(); partCounter++)
	{
		SliceLayerPart* part = &layer->parts[partOrderOptimizer.polyOrder[partCounter]];

		if (configs[cart_idx].enable_combing == Generals::Combing::Off)
		{
			gcodeLayer.setAlwaysRetract(true);
		}
		else
		{
			gcodeLayer.setCombBoundary(&part->combBoundery);
			gcodeLayer.setAlwaysRetract(false);
		}

		int fillAngle;
		int extrusion_width;

		if (configs[cart_idx].infill_before_wall)
		{

			/////////////////////////////////////////////////////////////////////////////////////////////////////
			//infill
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			if (configs[cart_idx].retraction_with_combing)
				gcodeLayer.retractionWithComb = false;

			engine::Polygons infillPolygons;

			fillAngle = 45;

			if (layerNr & 1)
				fillAngle += 90;

			extrusion_width = configs[cart_idx].infill_extrusion_width;

			if (layerNr == 0)
				extrusion_width = configs[cart_idx].initial_layer_extrusion_width;

			if (configs[cart_idx].sparse_infill_line_distance > 0)
			{
				switch (configs[cart_idx].infill_pattern)
				{
				case Generals::InfillPattern::Automatic:
					generateAutomaticInfill(
						part->sparseOutline, infillPolygons, extrusion_width,
						configs[cart_idx].sparse_infill_line_distance,
						configs[cart_idx].infill_overlap, fillAngle);
					break;

				case Generals::InfillPattern::Grid:
					generateGridInfill(part->sparseOutline, infillPolygons,
						extrusion_width,
						configs[cart_idx].sparse_infill_line_distance,
						configs[cart_idx].infill_overlap, fillAngle);
					break;

				case Generals::InfillPattern::Line:
					generateLineInfill(part->sparseOutline, infillPolygons,
						extrusion_width,
						configs[cart_idx].sparse_infill_line_distance,
						configs[cart_idx].infill_overlap, fillAngle);
					break;

				case Generals::InfillPattern::Concentric:
					generateConcentricInfill(
						part->sparseOutline,
						infillPolygons,
						configs[cart_idx].sparse_infill_line_distance);
					break;

				case Generals::InfillPattern::Crystal1:
					infill_tree[volumeIdx].generateInfill(part->sparseOutline, infillPolygons, extrusion_width,
						configs[cart_idx].infill_overlap, (configs[0].initial_layer_height + layerNr * configs[0].layer_height)*0.001);
					break;

				case Generals::InfillPattern::Crystal2:
					infill_tree[volumeIdx].generateInfill(part->sparseOutline, infillPolygons, extrusion_width,
						configs[cart_idx].infill_overlap, (configs[0].initial_layer_height + layerNr * configs[0].layer_height)*0.001);
					break;

	 			//case INFILL_VOXEL_M1_NEW:
	 			//	infill_tree[volumeIdx].generateInfill(part->sparseOutline, infillPolygons, extrusionWidth,
	 			//		config.infill_over_lap, (config.initial_layer_height + layerNr*config.layer_height)*0.001);
	 			//	break;
	 			//case INFILL_VOXEL_M2_NEW:
	 			//	infill_tree[volumeIdx].generateInfill(part->sparseOutline, infillPolygons, extrusionWidth,
	 			//		config.infill_over_lap, (config.initial_layer_height + layerNr*config.layer_height)*0.001);
	 			//	break;
				}
			}

			//overmoving은 다음에 생각하자.. 일단 가중치 파라메터로 꺾는 게 가능한지 확인 필요하니까!
			//gcodeLayer.addPolygonsByOptimizer_insetToinfill_filteredTheta(infillPolygons, &infillConfig);

			//이전에 사용하던 optimizer function..
			gcodeLayer.addPolygonsByOptimizer(infillPolygons, &pathconfigs_infill[cart_idx]);
			/////////////////////////////////////////////////////////////////////////////////////////////////////



			/////////////////////////////////////////////////////////////////////////////////////////////////////
			//inset
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			//addPolygonsByOptimizer()대신에 addPolygonsByOptimizerWithPreretractionAndOvermoving() 사용할 것.
			//parameter를 사용하기 위해서.. 가중치 3개 사용..
			//preretraction 및 overmoving은 0으로 설정하여 사용하지 않음.. 기능은 그대로 나둠..

			if (configs[cart_idx].retraction_with_combing)
				gcodeLayer.retractionWithComb = true;

			if (configs[cart_idx].inset_count > 0)
			{
				if (configs[0].spiralize_mode)
				{
					if (static_cast<int>(layerNr) >= configs[cart_idx].down_skin_count)
						pathconfigs_outer_wall[cart_idx].spiralize = true;
					if (static_cast<int>(layerNr) == configs[cart_idx].down_skin_count && part->insets.size() > 0)
						gcodeLayer.addPolygonsByOptimizer(part->insets[0], &pathconfigs_inner_wall[cart_idx]);
				}

				for (int iter = 0; iter < part->insets.size(); iter++)
				{
					int insetNr = 0;

					//wall printing direction : 0 = out -> in, 1 = in -> out//
					if (configs[cart_idx].wall_print_direction)
						insetNr = part->insets.size() - iter - 1;
					else
						insetNr = iter;


					if (insetNr == 0)
					{
						//가장 안쪽의 inset에서 바로 위의 inset으로 나갈 때.. 파라메터를 고려한 최적화 경로..
						//optimize_overmoving_filteringTheta 함수 사용..
						//gcodeLayer.addPolygonsByOptimizerWithPreretractionAndOvermoving(part->insets[insetNr],
						//	&inset0Config, &preretract0Config, &overmove0Config, config.preretration0Length, config.overmoving0Length);

						gcodeLayer.addPolygonsByOptimizer(part->insets[insetNr], &pathconfigs_outer_wall[cart_idx]);

						if (configs[cart_idx].skin_outline)
						{
							//skin outline을 따라 extrude path 생성//
							if (part->skinOutline.size() != 0)
								//gcodeLayer.addPolygonsByOptimizerWithPreretractionAndOvermoving(part->skinOutline.offset(-0.5*configs[0].extrusion_width),
								//&inset0Config, &preretract0Config, &overmove0Config, configs[0].preretration0Length, configs[0].overmoving0Length);
								gcodeLayer.addPolygonsByOptimizer(part->skinOutline.offset(-0.5*configs[cart_idx].top_bottom_extrusion_width), &pathconfigs_outer_wall[cart_idx]);
						}
					}
					else
					{
						//inset과 inset끼리 안에서는 최적화가 필요하지 않음..겉에서 품질에 영향을 주지 않으리라 판단..
						gcodeLayer.addPolygonsByOptimizer(part->insets[insetNr], &pathconfigs_inner_wall[cart_idx]);
					}
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////
		}
		else
		{

			/////////////////////////////////////////////////////////////////////////////////////////////////////
			//inset
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			//addPolygonsByOptimizer()대신에 addPolygonsByOptimizerWithPreretractionAndOvermoving() 사용할 것.
			//parameter를 사용하기 위해서.. 가중치 3개 사용..
			//preretraction 및 overmoving은 0으로 설정하여 사용하지 않음.. 기능은 그대로 나둠..

			if (configs[cart_idx].retraction_with_combing)
				gcodeLayer.retractionWithComb = true;

			if (configs[cart_idx].inset_count > 0)
			{
				if (configs[0].spiralize_mode)
				{
					if (static_cast<int>(layerNr) >= configs[cart_idx].down_skin_count)
						pathconfigs_outer_wall[cart_idx].spiralize = true;
					if (static_cast<int>(layerNr) == configs[cart_idx].down_skin_count && part->insets.size() > 0)
						gcodeLayer.addPolygonsByOptimizer(part->insets[0], &pathconfigs_inner_wall[cart_idx]);
				}
				for (int iter = 0; iter < part->insets.size(); iter++)
				{
					int insetNr = 0;

					//wall printing direction : 0 = out -> in, 1 = in -> out//
					if (configs[cart_idx].wall_print_direction)
						insetNr = part->insets.size() - iter - 1;
					else
						insetNr = iter;


					if (insetNr == 0)
					{
						//가장 안쪽의 inset에서 바로 위의 inset으로 나갈 때.. 파라메터를 고려한 최적화 경로..
						//optimize_overmoving_filteringTheta 함수 사용..
						//gcodeLayer.addPolygonsByOptimizerWithPreretractionAndOvermoving(part->insets[insetNr],
						//	&inset0Config, &preretract0Config, &overmove0Config, config.preretration0Length, config.overmoving0Length);

						gcodeLayer.addPolygonsByOptimizer(part->insets[insetNr], &pathconfigs_outer_wall[cart_idx]);

						if (configs[0].skin_outline)
						{
							//skin outline을 따라 extrude path 생성//
							if (part->skinOutline.size() != 0)
								//gcodeLayer.addPolygonsByOptimizerWithPreretractionAndOvermoving(part->skinOutline.offset(-0.5*configs[0].extrusion_width),
								//&inset0Config, &preretract0Config, &overmove0Config, configs[0].preretration0Length, configs[0].overmoving0Length);
								gcodeLayer.addPolygonsByOptimizer(part->skinOutline.offset(-0.5*configs[cart_idx].top_bottom_extrusion_width), &pathconfigs_outer_wall[cart_idx]);
						}
					}
					else
					{
						//inset과 inset끼리 안에서는 최적화가 필요하지 않음..겉에서 품질에 영향을 주지 않으리라 판단..
						gcodeLayer.addPolygonsByOptimizer(part->insets[insetNr], &pathconfigs_inner_wall[cart_idx]);
					}
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////


			/////////////////////////////////////////////////////////////////////////////////////////////////////
			//infill
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			if (configs[cart_idx].retraction_with_combing)
				gcodeLayer.retractionWithComb = false;

			engine::Polygons infillPolygons;

			fillAngle = 45;

			if (layerNr & 1)
				fillAngle += 90;

			extrusion_width = configs[cart_idx].infill_extrusion_width;

			if (layerNr == 0)
				extrusion_width = configs[cart_idx].initial_layer_extrusion_width;

			if (configs[cart_idx].sparse_infill_line_distance > 0)
			{
				switch (configs[cart_idx].infill_pattern)
				{
				case Generals::InfillPattern::Automatic:
					generateAutomaticInfill(
						part->sparseOutline, infillPolygons, extrusion_width,
						configs[cart_idx].sparse_infill_line_distance,
						configs[cart_idx].infill_overlap, fillAngle);
					break;

				case Generals::InfillPattern::Grid:
					generateGridInfill(part->sparseOutline, infillPolygons,
						extrusion_width,
						configs[cart_idx].sparse_infill_line_distance,
						configs[cart_idx].infill_overlap, fillAngle);
					break;

				case Generals::InfillPattern::Line:
					generateLineInfill(part->sparseOutline, infillPolygons,
						extrusion_width,
						configs[cart_idx].sparse_infill_line_distance,
						configs[cart_idx].infill_overlap, fillAngle);
					break;

				case Generals::InfillPattern::Concentric:
					generateConcentricInfill(
						part->sparseOutline, infillPolygons,
						configs[cart_idx].sparse_infill_line_distance);
					break;

				case Generals::InfillPattern::Crystal1:
					infill_tree[volumeIdx].generateInfill(part->sparseOutline, infillPolygons, extrusion_width,
						configs[cart_idx].infill_overlap, (configs[0].initial_layer_height + layerNr * configs[0].layer_height)*0.001);
					break;

				case Generals::InfillPattern::Crystal2:
					infill_tree[volumeIdx].generateInfill(part->sparseOutline, infillPolygons, extrusion_width,
						configs[cart_idx].infill_overlap, (configs[0].initial_layer_height + layerNr * configs[0].layer_height)*0.001);
					break;

					//case INFILL_VOXEL_M1_NEW:
					//	infill_tree[volumeIdx].generateInfill(part->sparseOutline, infillPolygons, extrusionWidth,
					//		config.infill_over_lap, (config.initial_layer_height + layerNr*config.layer_height)*0.001);
					//	break;
					//case INFILL_VOXEL_M2_NEW:
					//	infill_tree[volumeIdx].generateInfill(part->sparseOutline, infillPolygons, extrusionWidth,
					//		config.infill_over_lap, (config.initial_layer_height + layerNr*config.layer_height)*0.001);
					//	break;
				}
			}

			//overmoving은 다음에 생각하자.. 일단 가중치 파라메터로 꺾는 게 가능한지 확인 필요하니까!
			//gcodeLayer.addPolygonsByOptimizer_insetToinfill_filteredTheta(infillPolygons, &infillConfig);

			//이전에 사용하던 optimizer function..
			gcodeLayer.addPolygonsByOptimizer(infillPolygons, &pathconfigs_infill[cart_idx]);
			/////////////////////////////////////////////////////////////////////////////////////////////////////

		}



		/////////////////////////////////////////////////////////////////////////////////////////////////////
		//skin
		/////////////////////////////////////////////////////////////////////////////////////////////////////
		//for test..//
		int skinCartridgeIndex;
		skinCartridgeIndex = cart_idx;
		//skinCartridgeIndex = 1;

		int top_bottom_extrusion_width = configs[skinCartridgeIndex].top_bottom_extrusion_width;

		if (layerNr == 0)
			top_bottom_extrusion_width = configs[skinCartridgeIndex].initial_layer_extrusion_width;

		engine::Polygons skinPolygons;
		for (int i = 0; i < part->skinPartOutline.size(); ++i)
		{
			switch (configs[skinCartridgeIndex].skin_type)
			{
			case Generals::SkinType::Skin_Line:
				generateLineInfill(part->skinPartOutline[i],
					skinPolygons,
					top_bottom_extrusion_width,
					top_bottom_extrusion_width,
					configs[skinCartridgeIndex].skin_overlap,
					(part->bridges[i] > -1) ? part->bridges[i] : fillAngle);
				break;

			case Generals::SkinType::Skin_Concentric:
				generateConcentricInfill(
					part->skinPartOutline[i],
					skinPolygons,
					top_bottom_extrusion_width);
				break;
			}
		}

		if (configs[0].enable_combing == Generals::Combing::NoSkin)
		{
			gcodeLayer.setCombBoundary(nullptr);
			gcodeLayer.setAlwaysRetract(true);
		}

		gcodeLayer.addPolygonsByOptimizer(skinPolygons, &pathconfigs_skin[cart_idx]);
		//gcodeLayer.addPolygonsByOptimizerWithUserExtruder(skinPolygons, &skinConfigs[skinCartridgeIndex], skinCartridgeIndex);
		/////////////////////////////////////////////////////////////////////////////////////////////////////


		//After a layer part, make sure the nozzle is inside the comb boundary, so we do not retract on the perimeter.
		if (!configs[0].spiralize_mode || static_cast<int>(layerNr) < configs[cart_idx].down_skin_count)
			gcodeLayer.moveInsideCombBoundary(configs[cart_idx].extrusion_width * 2);
	}
	gcodeLayer.setCombBoundary(nullptr);
}

void GCodeGenerator::addSupportToGCode(engine::GCodePlanner& gcodeLayer, int layerNr, std::vector<PairIndexes> pairInfo_cart_wipetower)
{
	int tic[10];
	tic[0] = clock();
	if (!dataStorage->supportData->generated)
		return;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//cartridge를 선택하는 데 있어, 두 가지 방법..//support extruder 지정, coloredlayer//
	int cart_idx;

	if (Generals::isLayerColorModeOn())
	{
		cart_idx = CartridgeInfo::getCartIndexFromCartridgeLayerList(layerNr);
	}
	else
	{
		cart_idx = configs[0].support_main_cartridge_index;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (cart_idx < 0) return;

	int prevExtruder = gcodeLayer.getExtruder();

	////////////////////////////////////////////////////////////////////////////////
	//multi-cartridge 사양 : layer=0에서 wipe tower출력 시, support cartridge index가 PVA인 경우, 강제로 PVA 아닌 것으로 해야함..//
	if (layerNr == 0)
	{
		//support cartridge가 PVA일 경우, 반대편 cartridge로 전환..//
		if (configs[cart_idx].filament_material == "PVA" || configs[cart_idx].filament_material == "PVA+")
		{
			cart_idx = std::abs(cart_idx - 1);

			//reverse extruder flag//
			b_reverseExtruderForSupport = true;
		}
	}

	////////////////////////////////////////////////////////////////////////////////

	bool b_changedExtruder = gcodeLayer.setExtruder(cart_idx);

	if (configs[0].wipe_tower_enable && layerNr == 0 && (Profile::machineProfile.extruder_count.value > 1))
	{
		addWipeTower(gcodeLayer, layerNr, cart_idx, pairInfo_cart_wipetower);

		//wipe tower가 있고, layer number가 0일 경우, raft일 경우 : wipe tower를 2번 출력 --> 접착력 증가 및 airgap 제거효과.//
		if (configs[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
			addWipeTower(gcodeLayer, layerNr, cart_idx, pairInfo_cart_wipetower);
	}

	if (configs[0].wipe_tower_enable && b_changedExtruder && layerNr != 0 && (Profile::machineProfile.extruder_count.value > 1))
	{
		//layerNr이 0이 아닌 경우 --> 일반적으로 model과 support간에 nozzle 교체로 인한 wipe tower 발생..//
		addWipeTower(gcodeLayer, layerNr, cart_idx, pairInfo_cart_wipetower);
	}

	//if (model->oozeShield.size() > 0 && model->volumes.size() == 1)
	//{
	//	gcodeLayer.setAlwaysRetract(true);
	//	gcodeLayer.addPolygonsByOptimizer(model->oozeShield[layerNr], &skirtConfig);
	//	gcodeLayer.setAlwaysRetract(!configs[0].enable_combing);
	//}


	//tic[1] = clock();

	//global_clock[1] += (clock() - tic[1]) / (double)CLOCKS_PER_SEC;
	//tic[2] = clock();

	//global_clock[2] += (clock() - tic[2]) / (double)CLOCKS_PER_SEC;

	//tic[3] = clock();


	//global_clock[3] += (clock() - tic[3]) / (double)CLOCKS_PER_SEC;

	//tic[4] = clock();

	std::vector<SupportLayerPart> supportParts = dataStorage->support_layers.at(layerNr).support_parts;

	engine::PathOrderOptimizer support_partOrderOptimizer(gcode.getPositionXY());
	for (unsigned int n = 0; n < supportParts.size(); n++)
	{
		//여기서 island는 support part의 sparse outline에 해당함..//
		support_partOrderOptimizer.addPolygons(supportParts.at(n).support_outline);
	}

	//optimize() 실행..//
	support_partOrderOptimizer.optimize();

	for (unsigned int support_partCounter = 0; support_partCounter < support_partOrderOptimizer.polyOrder.size(); support_partCounter++)
	{
		if (supportParts.size() - 1 < support_partOrderOptimizer.polyOrder.at(support_partCounter))
		{
			qDebug() << "supportParts.size() - 1 < support_partOrderOptimizer.polyOrder.at(support_partCounter)";
			continue;
		}

		SupportLayerPart *support_part = &supportParts.at(support_partOrderOptimizer.polyOrder[support_partCounter]);


		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//support lines//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		engine::Polygons supportLines;
		engine::Polygons outlinePolygons;

		if (configs[cart_idx].support_interface_enabled) outlinePolygons = support_part->support_sparseOutline;
		else outlinePolygons = support_part->support_outline;

		if (configs[cart_idx].support_line_distance > 0)
		{
			switch (configs[cart_idx].support_main_pattern)
			{
			case Generals::SupportType::SupportGrid:
				if (configs[cart_idx].support_line_distance > configs[cart_idx].support_main_extrusion_width * 4)
				{
					generateLineInfill(outlinePolygons, supportLines, configs[cart_idx].support_main_extrusion_width, configs[cart_idx].support_line_distance * 2, configs[cart_idx].infill_overlap, 0);
					generateLineInfill(outlinePolygons, supportLines, configs[cart_idx].support_main_extrusion_width, configs[cart_idx].support_line_distance * 2, configs[cart_idx].infill_overlap, 90);
				}
				else
				{
					generateLineInfill(outlinePolygons, supportLines, configs[cart_idx].support_main_extrusion_width, configs[cart_idx].support_line_distance, configs[cart_idx].infill_overlap, (layerNr & 1) ? 0 : 90);
				}
				break;

			case Generals::SupportType::SupportLines:
				if (layerNr == 0)
				{
					generateLineInfill(outlinePolygons, supportLines, configs[cart_idx].initial_layer_extrusion_width, configs[cart_idx].support_line_distance, configs[cart_idx].infill_overlap + 150, 0);
					generateLineInfill(outlinePolygons, supportLines, configs[cart_idx].initial_layer_extrusion_width, configs[cart_idx].support_line_distance, configs[cart_idx].infill_overlap + 150, 90);
				}
				else
				{
					generateLineInfill(outlinePolygons, supportLines, configs[cart_idx].support_main_extrusion_width, configs[cart_idx].support_line_distance, configs[cart_idx].infill_overlap, 0);
				}
				break;

			case Generals::SupportType::SupportZigzag:
				NEWSupport support(&outlinePolygons, &supportLines, configs[cart_idx].support_line_distance, configs[cart_idx].support_main_extrusion_width, configs[cart_idx].infill_overlap, 0, layerNr);

				// static으로 minx,miny잡아놓으면, 다음 모델 왔을 때도 같이 minX, minY가 들어오기 때문에 따로 함수 만들어서 초기화해줌
				if (layerNr == 0)
					support.initParams(support_minX, support_minY, support_segmentLength, 20000, 600);

				support.generateNEWSupport(support_minX, support_minY, support_segmentLength, 20000, 600, true);

				break;
			}
		}

		gcodeLayer.forceRetract();

		if (configs[cart_idx].enable_combing)
			gcodeLayer.setCombBoundary(&support_part->support_outline);


		//이 부분은 나중에 생각해봐야 함..//
		if (configs[cart_idx].support_main_pattern == Generals::SupportType::SupportGrid)
		{
			gcodeLayer.addPolygonsByOptimizer(support_part->support_sparseOutline, &pathconfigs_support_main);
		}

		gcodeLayer.addPolygonsByOptimizer(supportLines, &pathconfigs_support_main);

		//support printed line check point..//
		if (layerNr == 0)
		{
			dataStorage->support_printed_line_min.X = std::min(dataStorage->support_printed_line_min.X, supportLines.min().X);
			dataStorage->support_printed_line_min.Y = std::min(dataStorage->support_printed_line_min.Y, supportLines.min().Y);

			dataStorage->support_printed_line_max.X = std::max(dataStorage->support_printed_line_max.X, supportLines.max().X);
			dataStorage->support_printed_line_max.Y = std::max(dataStorage->support_printed_line_max.Y, supportLines.max().Y);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////


		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//support interface//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (configs[cart_idx].support_interface_enabled)
		{
			int fillAngle;
			int extrusion_width;
			int interface_cartridge_index = cart_idx;

			engine::Polygons support_interface_polygons;
			engine::Polygons support_interface_roof_polygons;
			engine::Polygons support_interface_floor_polygons;


			fillAngle = 45;
			if (layerNr & 1)
				fillAngle += 90;

			//아랫면에는 support outline으로 출력하여 접착력이 좋게 하기 위함..//
			if (layerNr < configs[0].support_interface_floor_layers_count)
			{
				outlinePolygons.clear();
				outlinePolygons = support_part->support_outline;
				extrusion_width = configs[interface_cartridge_index].support_interface_floor_extrusion_width;

				switch (configs[interface_cartridge_index].support_interface_pattern)
				{
				case Generals::SkinType::Skin_Line:
					generateLineInfill(
						outlinePolygons,
						support_interface_polygons,
						extrusion_width,
						extrusion_width,
						configs[interface_cartridge_index].infill_overlap,
						(layerNr & 1) ? 0 : 90);
					break;

				case Generals::SkinType::Skin_Concentric:
					generateConcentricInfill(
						outlinePolygons,
						support_interface_polygons,
						extrusion_width);
					break;
				}

				gcodeLayer.addPolygonsByOptimizer(support_interface_polygons, &pathconfigs_support_interface_floor);

				//support printed line check point..//
				if (layerNr == 0)
				{
					dataStorage->support_printed_line_min.X = std::min(dataStorage->support_printed_line_min.X, support_interface_polygons.min().X);
					dataStorage->support_printed_line_min.Y = std::min(dataStorage->support_printed_line_min.Y, support_interface_polygons.min().Y);

					dataStorage->support_printed_line_max.X = std::max(dataStorage->support_printed_line_max.X, support_interface_polygons.max().X);
					dataStorage->support_printed_line_max.Y = std::max(dataStorage->support_printed_line_max.Y, support_interface_polygons.max().Y);
				}
			}
			else
			{
				//for (int i = 0; i < support_part->support_interfacePartOutline.size(); ++i)
				//{
				//	outlinePolygons.clear();
				//	outlinePolygons = support_part->support_interfacePartOutline[i];
				//	extrusion_width = configs[interface_cartridge_index].support_interface_roof_extrusion_width;

				//	switch (configs[interface_cartridge_index].support_interface_pattern)
				//	{
				//	case Generals::SkinType::Skin_Line:
				//		generateLineInfill(
				//			support_part->support_interfacePartOutline[i],
				//			support_interface_polygons,
				//			extrusion_width,
				//			extrusion_width,
				//			configs[interface_cartridge_index].infill_overlap,
				//			(support_part->support_bridges[i] > -1) ? support_part->support_bridges[i] : fillAngle);
				//		break;

				//	case Generals::SkinType::Skin_Concentric:
				//		generateConcentricInfill(
				//			support_part->support_interfacePartOutline[i],
				//			support_interface_polygons,
				//			extrusion_width);
				//		break;
				//	}
				//}

				//gcodeLayer.addPolygonsByOptimizer(support_interface_polygons, &pathconfigs_support_interface_roof);


				for (int i = 0; i < support_part->support_interface_part_roof.size(); ++i)
				{
					outlinePolygons.clear();
					outlinePolygons = support_part->support_interface_part_roof[i];
					extrusion_width = configs[interface_cartridge_index].support_interface_roof_extrusion_width;

					switch (configs[interface_cartridge_index].support_interface_pattern)
					{
					case Generals::SkinType::Skin_Line:
						generateLineInfill(
							support_part->support_interface_part_roof[i],
							support_interface_roof_polygons,
							extrusion_width,
							extrusion_width,
							configs[interface_cartridge_index].infill_overlap,
							(support_part->support_bridges_roof[i] > -1) ? support_part->support_bridges_roof[i] : fillAngle);
						break;

					case Generals::SkinType::Skin_Concentric:
						generateConcentricInfill(
							support_part->support_interface_part_roof[i],
							support_interface_roof_polygons,
							extrusion_width);
						break;
					}
				}
				gcodeLayer.addPolygonsByOptimizer(support_interface_roof_polygons, &pathconfigs_support_interface_roof);



				for (int i = 0; i < support_part->support_interface_part_floor.size(); ++i)
				{
					outlinePolygons.clear();
					outlinePolygons = support_part->support_interface_part_floor[i];
					extrusion_width = configs[interface_cartridge_index].support_interface_floor_extrusion_width;

					switch (configs[interface_cartridge_index].support_interface_pattern)
					{
					case Generals::SkinType::Skin_Line:
						generateLineInfill(
							support_part->support_interface_part_floor[i],
							support_interface_floor_polygons,
							extrusion_width,
							extrusion_width,
							configs[interface_cartridge_index].infill_overlap,
							(support_part->support_bridges_floor[i] > -1) ? support_part->support_bridges_floor[i] : fillAngle);
						break;

					case Generals::SkinType::Skin_Concentric:
						generateConcentricInfill(
							support_part->support_interface_part_floor[i],
							support_interface_floor_polygons,
							extrusion_width);
						break;
					}
				}
				gcodeLayer.addPolygonsByOptimizer(support_interface_floor_polygons, &pathconfigs_support_interface_floor);
			}

			//After a layer part, make sure the nozzle is inside the comb boundary, so we do not retract on the perimeter.
			if (!configs[0].spiralize_mode || static_cast<int>(layerNr) < configs[cart_idx].support_interface_floor_layers_count)
				gcodeLayer.moveInsideCombBoundary(configs[cart_idx].support_interface_floor_extrusion_width * 2);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////

		gcodeLayer.setCombBoundary(nullptr);
	}
	//global_clock[4] += (clock() - tic[4]) / (double)CLOCKS_PER_SEC;
	//global_clock[0] += (clock() - tic[0]) / (double)CLOCKS_PER_SEC;
}

void GCodeGenerator::addWipeTower(engine::GCodePlanner& gcodeLayer, int layerNr, int setExtruder, std::vector<PairIndexes> pairInfo_cart_wipetower)
{
	if (configs[0].wipe_tower_enable == 0 || configs[0].wipe_tower_outer_size < 1)
		return;

	if (configs[0].raft_fan_speed)
		gcode.writeFanCommand(configs[0].adhesion_cartridge_index, configs[0].raft_fan_speed);

	//cartridge index번호를 받아와야 함..//
	//int cart_idx = configs[0].support_main_cartridge_index;
	int cartIndex = setExtruder;
	if (setExtruder == 100) cartIndex = 0;


	//pairInfo vector는 중복 데이터가 없어야함..//
	//cart 0 - poly 0 :: cart 1 - poly 1
	//cart 0 - poly 1 :: cart 1 - poly 0
	engine::Polygons wipetower_polygons;

	int wipetower_polygonKey = pairInfo_cart_wipetower.at(cartIndex).index;

	switch (wipetower_polygonKey)
	{
	case 0:
		wipetower_polygons = dataStorage->wipeTower_layers.at(layerNr).wipeTower_mainOutline;
		break;

	case 1:
		wipetower_polygons = dataStorage->wipeTower_layers.at(layerNr).wipeTower_subOutline;
		break;
	}

	//restart position rotating around wipe-tower inner line..//
	//int wipePoint_index = layerNr % 4;
	//gcodeLayer.addTravel(model->wipePoint.at(wipePoint_index));

	//wipe tower outline optimizing..//
	gcodeLayer.addPolygonsByOptimizer(wipetower_polygons, &pathconfigs_wipe_tower);


	engine::Polygons fillPolygons;

	int extrusionWidth = configs[cartIndex].extrusion_width;

	if (layerNr == 0)
		extrusionWidth = configs[cartIndex].initial_layer_extrusion_width;

	if (layerNr < 15)
	{
		generateLineInfill(wipetower_polygons, fillPolygons, extrusionWidth, extrusionWidth, configs[cartIndex].infill_overlap, 45 + 90 * (layerNr % 2));
	}
	else
	{
		//switch (wipetower_polygonKey)
		//{
		//case 0:
		//	generateConcentricInfill(wipetower_polygons, fillPolygons, configs[cartIndex].extrusion_width);
		//	break;

		//case 1:
		//	int wipeTowerFillDensity = 170 - 10 * layerNr;

		//	if (wipeTowerFillDensity >= parent->m_SliceProfile_multi[0].wipe_tower_fill_density.value)
		//	{
		//		//profile to cofnig class//
		//		ProfileToConfig converter;
		//		int wipeTowerSparseInfillLineDistance = 100 * converter.CalculateEdgeWidth(&(parent->m_SliceProfile_multi[0])) * 1000 / wipeTowerFillDensity;
		//		
		//		generateLineInfill(wipetower_polygons, fillPolygons, configs[cartIndex].extrusion_width, wipeTowerSparseInfillLineDistance, configs[cartIndex].infill_overlap, 45 + 90 * (layerNr % 2));
		//	}
		//	else
		//	{
		//		//generateLineInfill(model->wipeTower, fillPolygons, configs[cart_idx].extrusion_width, configs[0].wipe_tower_sparse_infill_line_distance, configs[cartIndex].infill_overlap, 45 + 90 * (layerNr % 2));
		//		generateLineInfill(wipetower_polygons, fillPolygons, configs[cartIndex].extrusion_width, configs[0].wipe_tower_sparse_infill_line_distance, configs[cartIndex].infill_overlap, 45 + 90 * (layerNr % 2));

		//	}
		//	break;
		//}


		//wipetower_polygonkey에 따라, infill 종류가 변경되었던 방식 삭제.//
		//모든 wipetower는 line type infill로 함..//
		int temp_wipeTowerFillDensity = 250 - 10 * layerNr;

		if (temp_wipeTowerFillDensity >= Profile::configSettings[0].wipe_tower_infill_density)
		{
			int temp_wipeTowerSparseInfillLineDistance = 100 * ProfileToConfig::calculateEdgeWidth(&Profile::sliceProfile[cartIndex], cartIndex) * 1000 / temp_wipeTowerFillDensity;
			generateLineInfill(wipetower_polygons, fillPolygons, extrusionWidth, temp_wipeTowerSparseInfillLineDistance, configs[cartIndex].infill_overlap, 45 + 90 * (layerNr % 2));
		}
		else
		{
			//generateLineInfill(model->wipeTower, fillPolygons, configs[cart_idx].extrusion_width, configs[0].wipe_tower_sparse_infill_line_distance, configs[cartIndex].infill_overlap, 45 + 90 * (layerNr % 2));
			generateLineInfill(wipetower_polygons, fillPolygons, extrusionWidth, configs[0].wipe_tower_sparse_infill_line_distance, configs[cartIndex].infill_overlap, 45 + 90 * (layerNr % 2));
		}

	}

	//wipe tower infill optimizing..//
	gcodeLayer.addPolygonsByOptimizer(fillPolygons, &pathconfigs_wipe_tower);


	//before moving to new extruder model, move to wipe point on the tower..//
	//if ((wipePoint_index + 1) == 4) wipePoint_index = 0;
	//else wipePoint_index += 1;

	//gcodeLayer.addTravel(model->wipePoint.at(wipePoint_index));
	//gcodeLayer.addTravel(model->wipePoint - configs[0].extruderOffset[prevExtruder].p() + configs[0].extruderOffset[gcodeLayer.getExtruder()].p());

}


void GCodeGenerator::writeAdjustZGapCode()
{
	//보정해야 할 cartridgeIndex..//
	int cartridgeIndex = 0;
	int adjustZGapThickness = configs[0].adjust_z_gap_thickness;
	int adjustZGapLayers = configs[0].adjust_z_gap_layers; //보정해야 할 레이어 층수..//

	gcode.gcodePaths.paths.clear();

	engine::GCodePathConfig adjustZGapConfig(configs[0].initial_layer_speed, configs[0].initial_layer_extrusion_width, "ADJUST_Z_GAP", false);

	gcode.writeNewLine();
	gcode.writeComment("ADJUST_Z_GAP_START");
	gcode.writeNewLine();

	////////////////////////////////////////////////////////////////////////
	//configs vector profile setting//
	//std::vector<int> configsFilamentFlow;
	//configsFilamentFlow.clear();

	//std::vector<int> configsMoveSpeed;
	//configsMoveSpeed.clear();

	//std::vector<int> configsRetractionMinimalDistance;
	//configsRetractionMinimalDistance.clear();

	//for (int i = 0; i < configs.size(); i++)
	//{
	//	configsFilamentFlow.push_back(configs[i].overall_flow);
	//	configsMoveSpeed.push_back(configs[i].travel_speed);
	//	configsRetractionMinimalDistance.push_back(configs[i].retraction_minimal_distance);
	//}
	////////////////////////////////////////////////////////////////////////

	

	//layers iteration..//
	for (int layerNr = 0; layerNr < adjustZGapLayers; ++layerNr)
	{
		engine::GCodePlanner gcodeLayer(gcode, configs_set_speed.travel_speed, configs_set_retraction.retraction_minimal_distance, configs[cartridgeIndex].spiralize_mode);

		gcode.setZ(adjustZGapThickness + layerNr * adjustZGapThickness);

		gcode.writeMove(Point(gcode.currentPosition.x, gcode.currentPosition.y), configs[cartridgeIndex].travel_speed, 0, gcode.LAYER_NUMBER_NA, gcode.b_spiralize);

		gcode.setExtrusion(configs[cartridgeIndex].raft_interface_thickness, configs[cartridgeIndex].filament_diameter, configs_set_flow);
		gcodeLayer.addPolygonsByOptimizer(dataStorage->adjustZGapOutline_moved, &adjustZGapConfig);

		engine::Polygons adjust_ZGap_lines;
		//generateLineInfill(storage.adjustZGapOutline, adjust_Zgap_lines, configs[cartridgeIndex].initial_layer_extrusion_width, configs[cartridgeIndex].raftInterfaceLineSpacing, configs[cartridgeIndex].infill_overlap, configs[cartridgeIndex].raft_surface_layers > 0 ? 45 : 90);
		generateLineInfill_vertical_offset(dataStorage->adjustZGapOutline_moved, adjust_ZGap_lines, configs[cartridgeIndex].initial_layer_extrusion_width, configs[cartridgeIndex].raft_line_spacing, configs[cartridgeIndex].infill_overlap, 0, adjustZGapConfig.lineWidth);
		gcodeLayer.addPolygonsByOptimizer(adjust_ZGap_lines, &adjustZGapConfig);


		//adjust z gap code는 항상 처음으로 출력하므로, initRaft = true..//
		gcodeLayer.writeGCode(false, configs.front().initial_layer_height, gcode.LAYER_NUMBER_NA);

		dataStorage->insertGcodePath(gcode.gcodePaths);
	}

	gcode.writeComment("ADJUST_Z_GAP_END");
}

void GCodeGenerator::writeRaftGcode()
{
	//gcode.writeComment("Layer count: %d", totalLayers);

	double tempTime = 0.0;
	double tempLength = 0.0;

	qDebug() << "writeRaftGcode()";

	const int cartridgeIndex = configs[0].adhesion_cartridge_index;
	const int raftCount = configs[cartridgeIndex].raft_surface_layers + 2;
	//raftPrintTime.resize(raftCount);

	qDebug() << "writeGcode 2";
	//if (gcode.getFlavor() == GCODE_FLAVOR_ULTIGCODE)
	//{
	//	gcode.writeComment("FLAVOR:UltiGCode");
	//	gcode.writeComment("TIME:<__TIME__>");
	//	gcode.writeComment("MATERIAL:<FILAMENT>");
	//	gcode.writeComment("MATERIAL2:<FILAMEN2>");
	//}
	//if (gcode.getFlavor() == GCODE_FLAVOR_BFB)
	//{
	//	gcode.writeComment("enable auto-retraction");
	//	gcode.writeLine("M227 S%d P%d", configs[cartridgeIndex].retraction_amount * 2560 / 1000, configs[cartridgeIndex].retraction_amount * 2560 / 1000);
	//}

	qDebug() << "writeGcode 3";

	int raft_offset;

	if (configs[cartridgeIndex].raft_incline_enabled)
		raft_offset = configs[cartridgeIndex].raft_base_line_width / 3;
	else
		raft_offset = 0;

	int raft_layer_cnt = 1 + 1 + configs[cartridgeIndex].raft_surface_layers;
	//const int raft_layer_total = raft_layer_cnt;
	//totalLayers_with_adhesion += raft_layer_total;

	//raft를 support로 type 설정?? --> 이 부분을 RAFT로 하면 나중에 나눌 수 있는 건가??
	engine::GCodePathConfig raftBaseConfig((configs[cartridgeIndex].raft_base_speed <= 0) ? configs[cartridgeIndex].initial_layer_speed : configs[cartridgeIndex].raft_base_speed, configs[cartridgeIndex].raft_base_line_width, "RAFT", false);
	engine::GCodePathConfig raftMiddleConfig((configs[cartridgeIndex].raft_base_speed <= 0) ? configs[cartridgeIndex].initial_layer_speed : configs[cartridgeIndex].raft_base_speed, configs[cartridgeIndex].raft_interface_line_width, "RAFT", false);
	engine::GCodePathConfig raftInterfaceConfig((configs[cartridgeIndex].raft_interface_speed <= 0) ? configs[cartridgeIndex].initial_layer_speed : configs[cartridgeIndex].raft_interface_speed, configs[cartridgeIndex].raft_interface_line_width, "RAFT", false);
	engine::GCodePathConfig raftSurfaceConfig((configs[cartridgeIndex].raft_surface_speed > 0) ? configs[cartridgeIndex].raft_surface_speed : configs[cartridgeIndex].print_speed, configs[cartridgeIndex].raft_surface_line_width, "RAFT", false);

	////////////////////////////////////////////////////////////////////////
	//configs vector profile setting//
	std::vector<int> configsFilamentFlow;
	configsFilamentFlow.clear();

	std::vector<int> configsMoveSpeed;
	configsMoveSpeed.clear();

	std::vector<int> configsRetractionMinimalDistance;
	configsRetractionMinimalDistance.clear();

	for (int i = 0; i < configs.size(); i++)
	{
		configsFilamentFlow.push_back(configs[i].overall_flow);
		configsMoveSpeed.push_back(configs[i].travel_speed);
		configsRetractionMinimalDistance.push_back(configs[i].retraction_minimal_distance);
	}
	////////////////////////////////////////////////////////////////////////


	if (configs[cartridgeIndex].raft_fan_speed)
		gcode.writeFanCommand(cartridgeIndex, configs[cartridgeIndex].raft_fan_speed);

	//========================================================================================================================
	//RAFT BASE//
	{
		gcode.writeNewLine();
		gcode.writeComment("LAYER:-3");
		gcode.writeComment("RAFT BASE");

		currentLayers_with_adhesion++;

		char M532_code[100];
		sprintf_s(M532_code, "M532 L%i", currentLayers_with_adhesion);
		gcode.writeCode(M532_code);
		gcode.writeNewLine();

		//if (currentLayers_with_adhesion < 4)
		//{
		//	if (configs[0].filament_material == "ABS" || configs[1].filament_material == "ABS")
		//	{
		//		if (Profile::machineProfile.machine_model == "5X" || Profile::machineProfile.machine_model == "7X")
		//		{
		//			double M206_z_height = 0;

		//			if (currentLayers_with_adhesion == 1)
		//			{
		//				M206_z_height = 0.2;
		//			}
		//			else if (currentLayers_with_adhesion == 2 || currentLayers_with_adhesion == 3)
		//			{
		//				M206_z_height = 0.1;
		//			}

		//			char M206_code[100];
		//			sprintf_s(M206_code, "M206 Z%.1f", M206_z_height);
		//			gcode.writeCode(M206_code);
		//			gcode.writeNewLine();
		//		}
		//	}
		//}


		raft_layer_cnt--;


		///////////////////////////////////////////////////////////////////////////////////
		//fan control//
		//DP201용 raft fan control
		//if (parent->m_machineModel == "DP201")//fan control이 on일 경우에만 fan code
		if (Profile::machineProfile.raft_base_fan_control_enabled.value)
			gcode.writeFanCommand(cartridgeIndex, 12.5);
		///////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////
		//raft temperature control//
		if (configs[cartridgeIndex].raft_temperature_control)
		{
			char temp[100];

			if (configs[cartridgeIndex].raft_base_temperature > configs[cartridgeIndex].print_temperature_max)
				sprintf_s(temp, "M104 T%i S%d", cartridgeIndex, int(configs[cartridgeIndex].print_temperature_max));
			else
				sprintf_s(temp, "M104 T%i S%d", cartridgeIndex, int(configs[cartridgeIndex].raft_base_temperature));

			gcode.writeCode(temp);
			gcode.writeNewLine();
		}
		///////////////////////////////////////////////////////////////////////////////////

		engine::GCodePlanner gcodeLayer(gcode, configsMoveSpeed, configsRetractionMinimalDistance, configs[cartridgeIndex].spiralize_mode);

		int prevExtruder = gcodeLayer.getExtruder();
		bool extruderChanged = gcodeLayer.setExtruder(cartridgeIndex);


		if (configs[cartridgeIndex].support_main_cartridge_index > 0)		// TO DO : change setting
			gcodeLayer.setExtruder(configs[cartridgeIndex].adhesion_cartridge_index);

		gcode.setZ(configs[0].adjust_z_gap_layers * configs[0].adjust_z_gap_thickness + configs[cartridgeIndex].raft_base_thickness);
		gcode.writeMove(Point(gcode.currentPosition.x, gcode.currentPosition.y), configs[cartridgeIndex].travel_speed, 0, gcode.LAYER_NUMBER_NA, gcode.b_spiralize);
		gcode.setExtrusion(configs[cartridgeIndex].raft_base_thickness, configs[cartridgeIndex].filament_diameter, configs_set_flow);

		gcodeLayer.addPolygonsByOptimizer(dataStorage->skirt, &raftBaseConfig);
		gcodeLayer.addPolygonsByOptimizer(dataStorage->raftOutline.offset(-raft_layer_cnt * raft_offset), &raftBaseConfig);
		if (configs[cartridgeIndex].raft_inset_enabled)
			gcodeLayer.addPolygonsByOptimizer(dataStorage->raftOutline.offset(-configs[cartridgeIndex].raft_inset_offset - raft_layer_cnt * raft_offset), &raftBaseConfig);


		engine::Polygons raftLines;
		if (configs[cartridgeIndex].raft_inset_enabled)
			generateLineInfill_vertical_offset(dataStorage->raftOutline.offset(-configs[cartridgeIndex].raft_inset_offset - raft_layer_cnt * raft_offset), raftLines, configs[cartridgeIndex].raft_base_line_width, configs[cartridgeIndex].raft_line_spacing, configs[cartridgeIndex].infill_overlap, 0, raftBaseConfig.lineWidth);
		else
			generateLineInfill_vertical_offset(dataStorage->raftOutline.offset(-raft_layer_cnt * raft_offset), raftLines, configs[cartridgeIndex].raft_base_line_width, configs[cartridgeIndex].raft_line_spacing, configs[cartridgeIndex].infill_overlap, 0, raftBaseConfig.lineWidth);

		gcodeLayer.addPolygonsByOptimizer(raftLines, &raftBaseConfig);

		dataStorage->raft_printed_line_min = raftLines.min();
		dataStorage->raft_printed_line_max = raftLines.max();

		//////////////////////////////////////////////////////////////////////////////////////////
		//raft시 extrusion부분 path 길이 계산, DP201일 때.. //swyang 

		if (Profile::machineProfile.raft_base_pathLengthLimit_enabled.value && configs[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
		{
			double raftPathLength = gcodeLayer.writeGcode_extrusionPathLength(false, configs[cartridgeIndex].raft_base_thickness, -1, true);
			std::printf("raftPathLength = %f\n", raftPathLength);

			if (raftPathLength < 780)
				b_raftPathLengthLimit = true;
			else
				b_raftPathLengthLimit = false;

			if (b_raftPathLengthLimit) std::printf("raftPathLength limit (<780)\n");
			else std::printf("raftPathLength not limit (>=780)\n");
		}
		else
		{
			gcodeLayer.writeGCode(false, configs[cartridgeIndex].raft_base_thickness, true);

			b_raftPathLengthLimit = false;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		//raft time check//
		//printf("LAYER:-3 original time : %f\n", gcode.getTotalPrintTime() - tempTime);
		//printf("LAYER:-3 accumulated time : %f\n", gcode.getTotalPrintTime());

		//tempTime = gcode.getTotalPrintTime();
		//raftPrintTime.at(0) = gcode.getTotalPrintTime();
		//////////////////////////////////////////////////////////////////////////////////////////

		dataStorage->insertGcodePath(gcode.gcodePaths);
		gcode.gcodePaths.paths.clear();
	}
	//========================================================================================================================

	if (configs[cartridgeIndex].raft_fan_speed)
		gcode.writeFanCommand(cartridgeIndex, configs[cartridgeIndex].raft_fan_speed);


	//========================================================================================================================
	//RAFT INTERFACE//
	{
		gcode.writeNewLine();
		gcode.writeComment("LAYER:-2");
		gcode.writeComment("RAFT INTERFACE");
		raft_layer_cnt--;

		currentLayers_with_adhesion++;

		char M532_code[100];
		sprintf_s(M532_code, "M532 L%i", currentLayers_with_adhesion);
		gcode.writeCode(M532_code);
		gcode.writeNewLine();

		//if (currentLayers_with_adhesion < 4)
		//{
		//	if (configs[0].filament_material == "ABS" || configs[1].filament_material == "ABS")
		//	{
		//		if (Profile::machineProfile.machine_model == "5X" || Profile::machineProfile.machine_model == "7X")
		//		{
		//			double M206_z_height = 0;

		//			if (currentLayers_with_adhesion == 1)
		//			{
		//				M206_z_height = 0.2;
		//			}
		//			else if (currentLayers_with_adhesion == 2 || currentLayers_with_adhesion == 3)
		//			{
		//				M206_z_height = 0.1;
		//			}

		//			char M206_code[100];
		//			sprintf_s(M206_code, "M206 Z%.1f", M206_z_height);
		//			gcode.writeCode(M206_code);
		//			gcode.writeNewLine();
		//		}
		//	}
		//}

		///////////////////////////////////////////////////////////////////////////////////
		//fan control//
		//DP201용 raft fan control
		//if (parent->m_machineModel == "DP201")//fan control이 on일 경우에만 fan code
		if (Profile::machineProfile.raft_base_fan_control_enabled.value)
			gcode.writeFanCommand(cartridgeIndex, 25);
		///////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////
		//raft temperature control//
		if (configs[cartridgeIndex].raft_temperature_control)
		{
			char temp[100];
			//int temp_control = 0;
			//if (configs[cartridgeIndex].filament_material == "PLA" || configs[cartridgeIndex].filament_material == "ETC")
			//{
			//	if (configs[cartridgeIndex].raft_surface_layers == 1)
			//		temp_control = 10;
			//	else if (configs[cartridgeIndex].raft_surface_layers >= 2)
			//		temp_control = 13;
			//}
			//else
			//{
			//	if (configs[cartridgeIndex].raft_surface_layers == 1)
			//		temp_control = 10;
			//	else if (configs[cartridgeIndex].raft_surface_layers >= 2)
			//		temp_control = 10;
			//}

			//if (configs[cartridgeIndex].print_temperature + temp_control > parent->m_SliceProfile_multi[cartridgeIndex].print_temperature.max)
			//	sprintf_s(temp, "M104 T%i S%d", cartridgeIndex, int(parent->m_SliceProfile_multi[cartridgeIndex].print_temperature.max));
			//else
			//	sprintf_s(temp, "M104 T%i S%d", cartridgeIndex, int(configs[cartridgeIndex].print_temperature + temp_control));

			if (configs[cartridgeIndex].raft_interface_temperature > configs[cartridgeIndex].print_temperature_max)
				sprintf_s(temp, "M104 T%i S%d", cartridgeIndex, int(configs[cartridgeIndex].print_temperature_max));
			else
				sprintf_s(temp, "M104 T%i S%d", cartridgeIndex, int(configs[cartridgeIndex].raft_interface_temperature));

			gcode.writeCode(temp);
			gcode.writeNewLine();
		}
		///////////////////////////////////////////////////////////////////////////////////

		engine::GCodePlanner gcodeLayer(gcode, configsMoveSpeed, configsRetractionMinimalDistance, configs[cartridgeIndex].spiralize_mode);
		gcode.setZ(configs[0].adjust_z_gap_layers*configs[0].adjust_z_gap_thickness + configs[cartridgeIndex].raft_base_thickness + configs[cartridgeIndex].raft_interface_thickness);
		gcode.setExtrusion(configs[cartridgeIndex].raft_interface_thickness, configs[cartridgeIndex].filament_diameter, configs_set_flow);

		gcodeLayer.addPolygonsByOptimizer(dataStorage->raftOutline.offset(-raft_layer_cnt * raft_offset), &raftInterfaceConfig);

		engine::Polygons raftLines;
		generateLineInfill(dataStorage->raftOutline.offset(-raft_layer_cnt * raft_offset), raftLines, configs[cartridgeIndex].raft_interface_line_width, configs[cartridgeIndex].raft_interface_line_spacing, configs[cartridgeIndex].infill_overlap, configs[cartridgeIndex].raft_surface_layers > 0 ? 45 : 90);
		gcodeLayer.addPolygonsByOptimizer(raftLines, &raftInterfaceConfig);


		dataStorage->raft_printed_line_min.X = std::min(dataStorage->raft_printed_line_min.X, raftLines.min().X);
		dataStorage->raft_printed_line_min.Y = std::min(dataStorage->raft_printed_line_min.Y, raftLines.min().Y);
		dataStorage->raft_printed_line_max.X = std::max(dataStorage->raft_printed_line_max.X, raftLines.max().X);
		dataStorage->raft_printed_line_max.Y = std::max(dataStorage->raft_printed_line_max.Y, raftLines.max().Y);

		gcodeLayer.writeGCode(false, configs[cartridgeIndex].raft_interface_thickness, gcode.LAYER_NUMBER_NA);

		//////////////////////////////////////////////////////////////////////////////////////////
		//raft time check//
		//printf("LAYER:-2 original time : %f\n", gcode.getTotalPrintTime() - tempTime);
		//printf("LAYER:-2 accumulated time : %f\n", gcode.getTotalPrintTime());

		//raftPrintTime.at(1) = gcode.getTotalPrintTime() - tempTime;
		//tempTime = gcode.getTotalPrintTime();

		//raft length check//
		//printf("LAYER:-2 filament length : %f\n", gcode.getTotalFilamentUsed(0) - tempLength);
		//printf("LAYER:-2 accumulated length : %f\n", gcode.getTotalFilamentUsed(0));
		//tempLength = gcode.getTotalFilamentUsed(0);
		//////////////////////////////////////////////////////////////////////////////////////////

		dataStorage->insertGcodePath(gcode.gcodePaths);
	}
	//========================================================================================================================


	//========================================================================================================================
	//RAFT SURFACES//
	for (int raftSurfaceLayer = 1; raftSurfaceLayer <= configs[cartridgeIndex].raft_surface_layers; raftSurfaceLayer++)
	{
		char temp[100];

		gcode.writeNewLine();
		sprintf_s(temp, "LAYER:-1_%d", raftSurfaceLayer);
		gcode.writeComment(temp);

		char temp2[100];
		sprintf_s(temp2, "RAFT SURFACE_%d", raftSurfaceLayer);
		gcode.writeComment(temp2);


		currentLayers_with_adhesion++;

		char M532_code[100];
		sprintf_s(M532_code, "M532 L%i", currentLayers_with_adhesion);
		gcode.writeCode(M532_code);
		gcode.writeNewLine();

		//if (currentLayers_with_adhesion < 4)
		//{
		//	if (configs[0].filament_material == "ABS" || configs[1].filament_material == "ABS")
		//	{
		//		if (Profile::machineProfile.machine_model == "5X" || Profile::machineProfile.machine_model == "7X")
		//		{
		//			double M206_z_height = 0;

		//			if (currentLayers_with_adhesion == 1)
		//			{
		//				M206_z_height = 0.2;
		//			}
		//			else if (currentLayers_with_adhesion == 2 || currentLayers_with_adhesion == 3)
		//			{
		//				M206_z_height = 0.1;
		//			}

		//			char M206_code[100];
		//			sprintf_s(M206_code, "M206 Z%.1f", M206_z_height);
		//			gcode.writeCode(M206_code);
		//			gcode.writeNewLine();
		//		}
		//	}
		//}

		raft_layer_cnt--;

		///////////////////////////////////////////////////////////////////////////////////
		//fan control//
		//DP201용 raft fan control
		//if (parent->m_machineModel == "DP201")//fan control이 on일 경우에만 fan code
		if (Profile::machineProfile.raft_base_fan_control_enabled.value)
		{
			gcode.writeFanCommand(cartridgeIndex, 50);
		}
		///////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////
		//raft temperature control//
		if (configs[cartridgeIndex].raft_temperature_control)
		{
			char temp[100];
			int raft_temperature = 0;

			if (configs[cartridgeIndex].raft_surface_layers >= 2)
			{
				if (raftSurfaceLayer == 1)
					raft_temperature = configs[cartridgeIndex].raft_surface_initial_temperature;
				else if (raftSurfaceLayer == configs[cartridgeIndex].raft_surface_layers)
					raft_temperature = configs[cartridgeIndex].raft_surface_last_temperature;
				else
				{
					//linear interpolation//
					int t_i = configs[cartridgeIndex].raft_surface_initial_temperature;
					int t_l = configs[cartridgeIndex].raft_surface_last_temperature;
					int layers_size = configs[cartridgeIndex].raft_surface_layers;
					int layers = raftSurfaceLayer;

					raft_temperature = int(((t_l - t_i) / (layers_size - 1))*layers + (t_i - ((t_l - t_i) / (layers_size - 1))));
				}
			}
			else
			{
				//raftSurfaceLayers == 1//
				raft_temperature = configs[cartridgeIndex].raft_surface_last_temperature;
			}

			if (raft_temperature > configs[cartridgeIndex].print_temperature_max)
				sprintf_s(temp, "M104 T%i S%d", cartridgeIndex, int(configs[cartridgeIndex].print_temperature_max));
			else
				sprintf_s(temp, "M104 T%i S%d", cartridgeIndex, raft_temperature);

			gcode.writeCode(temp);
			gcode.writeNewLine();
		}

		///////////////////////////////////////////////////////////////////////////////////

		engine::GCodePlanner gcodeLayer(gcode, configsMoveSpeed, configsRetractionMinimalDistance, configs[cartridgeIndex].spiralize_mode);
		gcode.setZ(configs[cartridgeIndex].raft_base_thickness + configs[cartridgeIndex].raft_interface_thickness + configs[cartridgeIndex].raft_surface_thickness*raftSurfaceLayer);
		gcode.setExtrusion(configs[cartridgeIndex].raft_surface_thickness, configs[cartridgeIndex].filament_diameter, configs_set_flow);

		gcodeLayer.addPolygonsByOptimizer(dataStorage->raftOutline.offset(-raft_layer_cnt * raft_offset), &raftSurfaceConfig);

		engine::Polygons raftLines;
		generateLineInfill(dataStorage->raftOutline.offset(-raft_layer_cnt * raft_offset), raftLines, configs[cartridgeIndex].raft_surface_line_width, configs[cartridgeIndex].raft_surface_line_spacing, configs[cartridgeIndex].infill_overlap, 90 * raftSurfaceLayer);
		gcodeLayer.addPolygonsByOptimizer(raftLines, &raftSurfaceConfig);

		dataStorage->raft_printed_line_min.X = std::min(dataStorage->raft_printed_line_min.X, raftLines.min().X);
		dataStorage->raft_printed_line_min.Y = std::min(dataStorage->raft_printed_line_min.Y, raftLines.min().Y);
		dataStorage->raft_printed_line_max.X = std::max(dataStorage->raft_printed_line_max.X, raftLines.max().X);
		dataStorage->raft_printed_line_max.Y = std::max(dataStorage->raft_printed_line_max.Y, raftLines.max().Y);

		gcodeLayer.writeGCode(false, configs[cartridgeIndex].raft_interface_thickness, gcode.LAYER_NUMBER_NA);

		//////////////////////////////////////////////////////////////////////////////////////////
		//raft time check//
		//printf("LAYER:-1 original time : %f\n", gcode.getTotalPrintTime() - tempTime);
		//printf("LAYER:-1 accumulated time : %f\n", gcode.getTotalPrintTime());
		//raftPrintTime.at(raftSurfaceLayer + 1) = gcode.getTotalPrintTime() - tempTime;
		tempTime = gcode.getTotalPrintTime();

		//raft length check//
		//printf("LAYER:-1_%d filament length : %f\n", raftSurfaceLayer, gcode.getTotalFilamentUsed(0) - tempLength);
		//printf("LAYER:-1_%d accumulated length : %f\n", raftSurfaceLayer, gcode.getTotalFilamentUsed(0));
		//tempLength = gcode.getTotalFilamentUsed(0);
		//////////////////////////////////////////////////////////////////////////////////////////

		dataStorage->insertGcodePath(gcode.gcodePaths);
	}
	//========================================================================================================================
	

	qDebug() << "writeGcode 4";
}

bool GCodeGenerator::writeStartEndCode(QString _filePath)
{
	//temp only path code generate//
	QString temp_path_only = _filePath;
	temp_path_only.append("_path");
	Generals::fileCopyFuction(_filePath, temp_path_only);

	
	//set gcode output filestream//
	gcode.setFilename(Generals::qstringTowchar_t(_filePath));
	
	//writing start code..//
	writeStartCode();

	//writing temp path code..//
	writeTempPathCodeToGcode(temp_path_only, gcode.f);
	_wremove(Generals::qstringTowchar_t(temp_path_only));

	//writing end code..//
	gcode.writeNewLine();
	gcode.writeNewLine();
	//gcode.writeCode(Profile::machineProfile.end_default_code.c_str());
	gcode.writeCode(regenerateCodeByProfileKey(Profile::machineProfile.end_default_code).toStdString().c_str());

	gcode.finalize();

	return true;
}

void GCodeGenerator::writeTempPathCodeToGcode(QString _pathCode, FILE* _out)
{
	FILE* in_f = _wfopen(Generals::qstringTowchar_t(_pathCode), L"r");

	//main gcode loop//
	char buf[1000];
	int len;

	while (!feof(in_f))
	{
		len = fread(buf, sizeof(buf[0]), 1000, in_f);
		fwrite(buf, sizeof(buf[0]), len, _out);
	}

	std::fclose(in_f);
}

void GCodeGenerator::writeStartCode()
{
	char temp[1000];
	gcode.writeNewLine();
	sprintf(temp, ";%s Version : %s\n", AppInfo::getAppName().toStdString().c_str(), AppInfo::getCurrentVersion().toStdString().c_str());
	gcode.writeCode(temp);
	gcode.writeNewLine();
	writeCurrentProfile();
	gcode.writeNewLine();
	//image code
	writeImageText(Generals::getTempFolderPath() + "\\thumbnail_image.png");
	gcode.writeNewLine();

	if (Profile::machineProfile.machine_expanded_print_mode.value
		&& Profile::machineProfile.machine_expanded_print_function_enabled.value)
		gcode.writeCode(";RODIN_HH_X_WIDE\n\n");	//expanded print mode//

	writeFileName(models.back()->getOnlyFileName());
	gcode.writeNewLine();
	sprintf(temp, ";PRINTER_MODEL: [%s]", Profile::machineProfile.group_model.toStdString().c_str());
	gcode.writeCode(temp);
	gcode.writeNewLine();

	//email address, count
	writeEmailCode();
	sprintf(temp, ";ESTIMATION_TIME: [%s]\n", printingInfo->getTotalPrintTimeString().toStdString().c_str());
	gcode.writeCode(temp);
	
	//authentification print mode
	writeAuthentificationCode();
	
	gcode.writeNewLine();
	
	//dimension, location, operating zone
	writeModelInfo();
	
	gcode.writeNewLine();
	
	//cartridge count, used state, estimation, mass ...
	writeCartridgeInfo();
	
	gcode.writeNewLine();
	
	//T0, T1 for nozzle setting..
	writeRaftCartridgeCode();

	gcode.writeNewLine();

	//from start code
	if (Profile::machineProfile.firmware_code.value == Generals::FirwareCode::MARLIN)
		writeDefaultStartCode_New();
	else
		writeDefaultStartCode();
	
	writeZOffsetCode();

	//nozzle retraction code for ready to print..//
	writeRetractionCodeForStart();


	//기존 gcode 추가.
}
void GCodeGenerator::writeCurrentProfile()
{
	if (configs.size() != Profile::sliceProfile.size())
		return;

	for (int i = 0; i < configs.size(); i++)
	{
		SliceProfile temp_slice_profile = Profile::sliceProfile.at(i);
		SliceProfileForCommon temp_slice_profile_common = Profile::sliceProfileCommon;
		temp_slice_profile_common.setSliceProfileDataFromCommon(&temp_slice_profile);

		if (printingInfo->getUsedState(i) == true)
		{
			char temp_string[100];
			gcode.writeNewLine();
			sprintf(temp_string, ";[Cartridge_Number] : <%i>\n", i);
			gcode.writeCode(temp_string);
			gcode.writeCode(";-----------------------------------------\n");
			gcode.writeCode(temp_slice_profile.getCurrentProfileOutput().c_str());
		}
	}
}
void GCodeGenerator::writeAuthentificationCode()
{
	if (!UserProperties::authentification_print_mode)
		return;
	//if (UserProperties::authentification_print_setting_method.value != Generals::AuthentificationMethod::SaveToPC)
	//	return;

	gcode.writeNewLine();
	gcode.writeCode(generateAuthentificationCode().toStdString().c_str());
}

void GCodeGenerator::writeModelInfo()
{
	char temp[1000];
	AABB aabb = AABBGetter()(models);
	sprintf(temp, ";DIMENSION: [%.1f:%.1f:%.1f]\n", Generals::roundDouble(aabb.getLengthX(), 2), Generals::roundDouble(aabb.getLengthY(), 2), Generals::roundDouble(aabb.getLengthZ(), 2));
	gcode.writeCode(temp);
	sprintf(temp, ";LOCATION: [%d:%d]\n", int(floor(dataStorage->min_printing.x)), int(floor(dataStorage->min_printing.y)));
	//sprintf(temp, ";LOCATION: [%.3f:%.3f]\n", Generals::roundDouble(dataStorage->min_printing.x, 3), Generals::roundDouble(dataStorage->min_printing.y, 3));
	gcode.writeCode(temp);
	FPoint3 tempP = dataStorage->max_printing - dataStorage->min_printing;
	sprintf(temp, ";OPERATINGZONE: [%d:%d:%d]\n", int(ceil(tempP.x)), int(ceil(tempP.y)), int(ceil(tempP.z)));
	gcode.writeCode(temp);
}

void GCodeGenerator::writeCartridgeInfo()
{
	char temp[1000];
	sprintf(temp, ";CARTRIDGE_COUNT_TOTAL: [%d]\n", Profile::machineProfile.extruder_count.value);
	gcode.writeCode(temp);
	sprintf(temp, ";USEDNOZZLE: [%d]\n", printingInfo->getUsedCartridgeCount());
	gcode.writeCode(temp);
	sprintf(temp, ";CARTRIDGE_USED_STATE: [%s]\n", printingInfo->getUseStateTFString().c_str());
	gcode.writeCode(temp);
	gcode.writeNewLine();

	for (int i = 0; i < printingInfo->getCartridgeCount(); i++)
	{
		if (printingInfo->getUsedState(i))
		{
			sprintf(temp, ";ESTIMATION_FILAMENT_CARTRIDGE_%d: [%d]\n", i, int(printingInfo->getFilaAmount(i)));
			gcode.writeCode(temp);
			sprintf(temp, ";MASS_CARTRIDGE_%d: [%.1f]\n", i, printingInfo->getFilaMass(i));
			gcode.writeCode(temp);
			sprintf(temp, ";MATERIAL_CARTRIDGE_%d: [%s]\n", i, printingInfo->getFilaMaterial(i).toLatin1().data());
			gcode.writeCode(temp);
		}
	}

	if (printingInfo->getCartridgeCount() == 1)
	{
		sprintf(temp, ";ESTIMATION_FILAMENT: [%d]\n", int(printingInfo->getFilaAmount(0)));
		gcode.writeCode(temp);
		sprintf(temp, ";MASS: [%.1f]\n", printingInfo->getFilaMass(0));
		gcode.writeCode(temp);
		sprintf(temp, ";MATERIAL: [%s]\n", printingInfo->getFilaMaterial(0).toLatin1().data());
		gcode.writeCode(temp);
	}

	gcode.writeNewLine();

	sprintf(temp, ";TOTAL_LAYER: [%d]\n", dataStorage->layerCount);
	gcode.writeCode(temp);
	sprintf(temp, ";TOTAL_RAFTLAYER: [%d]\n", dataStorage->layerCount - dataStorage->totalLayer);
	gcode.writeCode(temp);
	sprintf(temp, ";TOTAL_MASS: [%.1f]\n", printingInfo->getfilaMassTotal());
	gcode.writeCode(temp);
}
void GCodeGenerator::writeRaftCartridgeCode()
{
	char temp[1000];
	int index;
	////////////////////////////////////////////////////////////////////////////////////////
	//first cartridge 준비//
	switch (configs[0].platform_adhesion)
	{
	case Generals::PlatformAdhesion::Raft:
		index = configs[0].adhesion_cartridge_index;
		sprintf(temp, "T%i	;Raft Cartridge No: [%i]\n", index, index);
		break;
	default:
		if (Profile::machineProfile.extruder_count.value < 2 || firstToolChangeInfo.empty())
		{
			firstToolChangeInfo.push_back(0); //tool number
			firstToolChangeInfo.push_back(0); //layer number
		}
		index = firstToolChangeInfo.at(0);
		sprintf(temp, "T%i	;First Cartridge No: [%i]\n", index, index);
		break;
	}
	gcode.writeCode(temp);
}

void GCodeGenerator::writeDefaultStartCode()
{
	char temp[1000];
	
	//M104
	writeTemperatureCode();

	gcode.writeNewLine();

	gcode.writeCode(regenerateCodeByProfileKey(Profile::machineProfile.start_default_code).toStdString().c_str());

	//M109
	writeWaitTemperatureCode();
	
	gcode.writeNewLine();
	
	gcode.writeCode("G28	    ;Home\n");
	gcode.writeCode("M298	;Move Nozzles on Bed plate\n");
	sprintf(temp, "G0 F9000 Z%.2f	;Z-leveling", Profile::machineProfile.startcode_Z_leveling.value);
	gcode.writeCode(temp);

	gcode.writeNewLine();
}

void GCodeGenerator::writeDefaultStartCode_New()
{
	char temp[100];

	gcode.writeCode(regenerateCodeByProfileKey(Profile::machineProfile.start_default_code).toStdString().c_str());
	//gcode.writeCode(Profile::machineProfile.start_default_code.c_str());
	
	sprintf(temp, "G0 F9000 Z%.2f	;Z-leveling", Profile::machineProfile.startcode_Z_leveling.value);
	gcode.writeCode(temp);

	gcode.writeNewLine();
}

QString GCodeGenerator::regenerateCodeByProfileKey(std::string _default_code)
{
	QString regenerated_code; 

	QStringList default_str_list = QString::fromStdString(_default_code).split("\n");
	QString text_line;

	bool isCorrespondCode = false;
	bool isWarning;

	if (Profile::sliceProfile.empty())
		return QString("");


	for (int i = 0; i < default_str_list.size(); ++i)
	{
		text_line = default_str_list.value(i);

		text_line.append("\n");

		QStringList profile_key_byline;
		QStringList remain_string_part;
		QStringList profile_value_byline;
		isWarning = false;

		//split by profile value prekye "{"//
		QStringList strlist_preKey_split = text_line.split("{");
		
		if (strlist_preKey_split.size() > 1)
		{
			for (int i = 1; i < strlist_preKey_split.size(); ++i)
			{
				QStringList strlist_postKey_split_sub = strlist_preKey_split.value(i).split("}");

				if (strlist_postKey_split_sub.size() == 2)
				{
					//pre, post key pairing..//

					//profile key//
					profile_key_byline.push_back(strlist_postKey_split_sub.value(0));

					//remain string part//
					remain_string_part.push_back(strlist_postKey_split_sub.value(1));
				
				}
				else
				{
					//pre, post key not pairing..//

					//profile key//
					profile_key_byline.push_back(QString(" ; --> [Warning] : Not pairing { }\n"));
					isWarning = true;

					break;
				}
			}

			for (auto profile_key : profile_key_byline)
			{
				//find profile-key//
				std::map<std::string, ProfileMapValue*>::const_iterator it = Profile::sliceProfile.front().slice_profile_map.find(profile_key.toStdString());

				if (it == Profile::sliceProfile.front().slice_profile_map.end())
				{
					//not found..//
					profile_value_byline.push_back(QString("0 ; ------> [Warning] : Not found in profile or Not pairing { }\n"));
					isWarning = true;

					break;
				}
				else
				{
					//found..//
					profile_value_byline.push_back(QString::fromStdString(it->second->getProfileDataValueStr()));
				}
			}

			//append to text_line//
			QString split_append_str;
			for (int i = 0; i < profile_value_byline.size(); ++i)
			{
				QString temp_str;
				//remain_string_part가 profile_value_byline보다 작을 수 있으므로..//
				//ex) xxxx{xxx}xxx{xxx} 와 같이 '}'으로 string이 끝날때..//
				if (remain_string_part.size() > i && !isWarning)
				{
					temp_str = profile_value_byline.value(i).append(remain_string_part.value(i));
				}
				else
				{
					temp_str = profile_value_byline.value(i);
				}
				split_append_str.append(temp_str);
			}
			text_line = strlist_preKey_split.value(0).append(split_append_str);
		}


		//temperature code replaced..//
		if (text_line.startsWith(";[M104_CODE]"))
		{
			text_line.clear();
			text_line.append("\n").append(generateM104Code());
		}
		else if (text_line.startsWith(";[M140_CODE]"))
		{
			text_line.clear();
			text_line.append("\n").append(generateM140Code());
		}
		else if (text_line.startsWith(";[M109_CODE]"))
		{
			text_line.clear();
			text_line.append("\n").append(generateM109Code());
		}
		else if (text_line.startsWith(";[M190_CODE]"))
		{
			text_line.clear();
			text_line.append("\n").append(generateM190Code());
		}

		regenerated_code.append(text_line);
	}
	
	return regenerated_code;
}

//
//QString GCodeGenerator::regenerateCodeByProfileKey(std::string _default_code)
//{
//	QString regenerated_code;
//
//	QStringList _default_str_list = QString::fromStdString(_default_code).split("\n");
//	QString text_line;
//	bool isCorrespondCode = false;
//
//	if (Profile::sliceProfile.empty())
//		return QString("");
//
//
//	for (int i = 0; i < _default_str_list.size(); ++i)
//	{
//		text_line = _default_str_list.value(i);
//
//		text_line.append("\n");
//
//		QStringList temp_strlist_ = text_line.split("{");
//		if (temp_strlist_.size() == 2)
//		{
//			//M140 S{temp} ;set//
//
//			QString temp_str_ = temp_strlist_.value(1);
//			QString profile_key = temp_str_.split("}").value(0);
//
//			//find profile-key//
//			std::map<std::string, ProfileMapValue*>::const_iterator it = Profile::sliceProfile.front().slice_profile_map.find(profile_key.toStdString());
//			std::string profile_value_str;
//			if (it == Profile::sliceProfile.front().slice_profile_map.end())
//			{
//				//not found..//
//				profile_value_str = std::string("0");
//
//				//warning message inserted..//
//				text_line = temp_strlist_.value(0).append(QString::fromStdString(profile_value_str)).append(QString(" ; ----------> [Warning] : Not found in profile\n"));
//			}
//			else
//			{
//				//found..//
//				profile_value_str = it->second->getProfileDataValueStr();
//
//				//regenerate -> insert profile value//
//				text_line = temp_strlist_.value(0).append(QString::fromStdString(profile_value_str)).append(temp_str_.split("}").value(1));
//			}
//
//
//		}
//
//		regenerated_code.append(text_line);
//	}
//
//	return regenerated_code;
//}



void GCodeGenerator::writeZOffsetCode()
{
	char temp[1000];
	int firstCartIndex = 0;
	switch (configs[0].platform_adhesion)
	{
	case Generals::PlatformAdhesion::Raft:
		firstCartIndex = configs[0].adhesion_cartridge_index;
		break;
	default:
		firstCartIndex = firstToolChangeInfo.at(0);
		break;
	}

	if (configs[firstCartIndex].platform_adhesion == Generals::PlatformAdhesion::Raft)
	{
		if (configs[firstCartIndex].zOffset_raft != 0.0)
		{
			if (Profile::machineProfile.firmware_code.value == Generals::FirwareCode::MARVELL)
				sprintf(temp, "M206 X0.0 Y0.0 Z%.2f		;Offset z direction\n\n", INT2MM(configs[firstCartIndex].zOffset_raft));
			else //MARLIN
				sprintf(temp, "M290 Z%.2f		;Offset z direction\n\n", INT2MM(configs[firstCartIndex].zOffset_raft));
		}
		else
		{
			sprintf(temp, ";Not applied Z offset function\n\n");
		}
	}
	else
	{
		if (configs[firstCartIndex].zOffset_except_raft != 0.0)
		{
			if (Profile::machineProfile.firmware_code.value == Generals::FirwareCode::MARVELL)
				sprintf(temp, "M206 X0.0 Y0.0 Z%.2f		;Offset z direction\n\n", INT2MM(configs[firstCartIndex].zOffset_except_raft));
			else //MARLIN
			 	sprintf(temp, "M290 Z%.2f		;Offset z direction\n\n", INT2MM(configs[firstCartIndex].zOffset_except_raft));
		}
		else
		{
			sprintf(temp, ";Not applied Z offset function\n\n");
		}
	}
	gcode.writeCode(temp);
}

void GCodeGenerator::writeRetractionCodeForStart()
{
	int cartridge_index = 0;
	if (configs[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
		cartridge_index = configs[0].adhesion_cartridge_index;

	char temp[1000];
	sprintf_s(temp, "G1 F%d E%lf		;Retraction to prepare for printing", configs[cartridge_index].retraction_speed * 60, Profile::machineProfile.raft_base_retraction_amount.value);
	gcode.writeCode(temp);
	gcode.writeNewLine();
}

void GCodeGenerator::writeTemperatureCode()
{
	char temp[1000];
	double bedTemperature = Profile::configSettings[0].print_bed_temperature;
	//M140//
	if (!(Profile::machineProfile.has_heated_bed.value))
		bedTemperature = 0;

	sprintf(temp, "M140 S%.f		;Heat up the bed without waiting\n", bedTemperature);
	gcode.writeCode(temp);

	int i = 0;
	for (auto it : printingInfo->getUsedState())
	{
		if (Generals::isReplicationUIMode())
			writeM104Code(i);
		else if (it)
			writeM104Code(i);
		i++;
	}
}

void GCodeGenerator::writeWaitTemperatureCode()
{
	char temp[1000];
	double bedTemperature = Profile::configSettings[0].print_bed_temperature;

	if (!(Profile::machineProfile.has_heated_bed.value)) 
		bedTemperature = 0;

	sprintf(temp, "M190 S%.f		;Heat up the bed\n", bedTemperature);
	gcode.writeCode(temp);

	int firstTool = 0;
	// hjkim modify 
	// 첫번째 사용하는 노즐만 M109로 온도 체크하고, 다른 노즐은 M104로 온도를 80도로 낮춤
	if (Profile::machineProfile.extruder_count.value >= 2)
	{
		if (Profile::configSettings[0].platform_adhesion != Generals::PlatformAdhesion::Raft)
			firstTool = firstToolChangeInfo.at(0);
		else
			firstTool = Profile::configSettings[0].adhesion_cartridge_index;
	}

	int i = 0;
	for (auto it : printingInfo->getUsedState())
	{
		if (i == firstTool)
		{
			if (it)
				writeM109Code(i);
		}
		else
		{
			sprintf(temp, "M104 T%i S%i		;Heat up the %ith nozzle without waiting\n", i, Profile::configSettings[i].initial_standby_temperature, i);
			gcode.writeCode(temp);
		}
		i++;
	}
}

void GCodeGenerator::writeEmailCode()
{
	char temp[100];
	////////////////////////////////////////////////////
	QString email = Generals::getProps("emailAddress");
	int count = 0;
	if (email.isEmpty())
		return;
	else if (email == " ")
	{
		sprintf(temp, ";EMAIL_CHECKCOUNT: [%i]\n", count);
	}
	else
	{
		count = Generals::getProps("emailCount").toInt();
		writeEmailAddress(email);
		sprintf(temp, ";EMAIL_CHECKCOUNT: [%i]\n", count);
	}
	gcode.writeCode(temp);
}

void GCodeGenerator::writeImageText(QString filePath)
{
	//encoding : image -> text
	QFile file(filePath);
	file.open(QIODevice::ReadOnly);
	splitEncodedCode(file.readAll(), ";IMAGE");
	file.close();
}

void GCodeGenerator::writeFileName(QString fileName_)
{
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QByteArray tempEncodedStr = codec->fromUnicode(fileName_);
	splitEncodedCode(tempEncodedStr, ";FILENAME");
}

void GCodeGenerator::writeEmailAddress(QString address_)
{
	QByteArray tempByteArray;
	tempByteArray.append(address_);
	splitEncodedCode(tempByteArray, ";EMAIL_RECIPIENTS");
}

void GCodeGenerator::splitEncodedCode(QByteArray inputCode_, std::string preCode_)
{
	char temp[1000];
	QByteArray encodedCode = inputCode_.toBase64();
	std::vector<char> encodedData(encodedCode.constData(), encodedCode.constData() + encodedCode.size());

	int totalLine = (encodedCode.size() / 80) + (encodedCode.size() % 80 == 0 ? 0 : 1);
	for (int i = 0; i < totalLine; i++)
	{
		int lastPos = ((i + 1) * 80);
		if (totalLine == i + 1)
			lastPos = encodedData.size();
		std::string tempcutStr(encodedData.begin() + (i * 80), encodedData.begin() + lastPos);
		sprintf(temp, "%s[%i/%i] %s\n", preCode_.c_str(), i + 1, totalLine, tempcutStr.c_str());
		gcode.writeCode(temp);
	}
}


QString GCodeGenerator::generateAuthentificationCode()
{
	if (!UserProperties::authentification_print_mode)
		return "";

	QString id = Generals::getProps("id");

	if (id.isEmpty())
		return "";

	QString pwd = Generals::getProps("password");
	QString code;
	code.append(QString(";SERVER_ID: %1\n").arg(id.toStdString().c_str()));
	code.append(QString(";SERVER_PW: %1\n").arg(pwd.toStdString().c_str()));
	code.append(QString(";PC_NAME: %1\n").arg(AppInfo::getSystemHostName()));
	code.append(QString(";PC_IP: %1\n").arg(AppInfo::getSystemIpAddress()));

	//after writing code -> clear the id & pw..//
	if (UserProperties::authentification_print_setting_method == Generals::AuthentificationMethod::Everytime)
	{
		Generals::setProps("id", "");
		Generals::setProps("password", "");
	}

	return code;
}

QString GCodeGenerator::generateImageTextCode(QString filePath)
{
	//encoding : image -> text
	QFile file(filePath);
	file.open(QIODevice::ReadOnly);
	QString rtn = generateSplitEncodedCode(file.readAll().toBase64(), ";IMAGE");
	file.close();
	return rtn;
}

QString GCodeGenerator::generateImageTextCode(QByteArray encodedCode_)
{
	return generateSplitEncodedCode(encodedCode_, ";IMAGE");
}

QString GCodeGenerator::generateFileNameCode(QString fileName_)
{
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QByteArray tempEncodedStr = codec->fromUnicode(fileName_);
	return generateSplitEncodedCode(tempEncodedStr.toBase64(), ";FILENAME");
}

QString GCodeGenerator::generateEmailAddressCode(QString address_)
{
	QByteArray tempByteArray;
	tempByteArray.append(address_);
	return generateSplitEncodedCode(tempByteArray.toBase64(), ";EMAIL_RECIPIENTS");
}
QString GCodeGenerator::generateCodeForOtherSlicer(PrintingInfo* printingInfo_)
{
	QString rtn;
	QString encodedFileName = GCodeGenerator::generateFileNameCode(printingInfo_->getGcodeBaseFileName());
	rtn.append(encodedFileName);
	if (printingInfo_->getTotalPrintTime() != 0)
	{
		QString estimationTimeString = QString(";ESTIMATION_TIME: [%1]\n").arg(printingInfo_->getTotalPrintTimeString());
		rtn.append(estimationTimeString);
	}
	qglviewer::Vec opZone = printingInfo_->getOperatingZone();
	if (opZone != qglviewer::Vec(0, 0, 0))
	{
		QString operatingZone = QString(";OPERATINGZONE: [%1:%2:%3]\n").arg(opZone[0]).arg(opZone[1]).arg(opZone[2]);
		rtn.append(operatingZone);
	}
	if (printingInfo_->getFilaAmountTotal() != 0)
	{
		QString filaAmount;
		for (int i = 0; i < printingInfo_->getCartridgeCount(); i++)
		{
			if (printingInfo_->getUsedState(i))
			{
				filaAmount = QString(";ESTIMATION_FILAMENT_CARTRIDGE_%1: [%2]\n").arg(i).arg(int(printingInfo_->getFilaAmount(i)));
				rtn.append(filaAmount);
			}
		}
	}

	return rtn;
}

QString GCodeGenerator::generateSplitEncodedCode(QByteArray encodedCode_, QString preCode_)
{
	QString rtn;
	//QByteArray encodedCode = inputCode_.toBase64();
	QString tempLine;
	QString tempMid;
	int splitSize = 80;

	int totalLine = (encodedCode_.size() / splitSize) + (encodedCode_.size() % splitSize == 0 ? 0 : 1);
	for (int i = 0; i < totalLine; i++)
	{
		int startPos = i * splitSize;
		tempMid = encodedCode_.mid(startPos, splitSize);
		tempLine = QString("%1[%2/%3] %4\n").arg(preCode_).arg(i + 1).arg(totalLine).arg(tempMid);
		rtn.append(tempLine);
	}

	return rtn;
}

void GCodeGenerator::writeM104Code(int toolIndex_)
{
	if (toolIndex_ >= Profile::configSettings.size())
		return;

	double nozzleTemperature = Profile::configSettings[toolIndex_].print_temperature;
	double maxTemp = Profile::configSettings[toolIndex_].print_temperature_max;
	if (nozzleTemperature > maxTemp)
		nozzleTemperature = maxTemp;

	char temp[1000];
	sprintf(temp, "M104 T%i S%.f		;Heat up the %ith nozzle without waiting\n", toolIndex_, nozzleTemperature, toolIndex_);
	gcode.writeCode(temp);
}

void GCodeGenerator::writeM109Code(int toolIndex_)
{
	if (toolIndex_ >= Profile::configSettings.size())
		return;

	//layer0 temperature는 사용안함 --> temperature layer list 이용//
	double nozzleTemperature = Profile::configSettings[toolIndex_].print_temperature;
	if (nozzleTemperature > Profile::configSettings[toolIndex_].print_temperature_max)
		nozzleTemperature = Profile::configSettings[toolIndex_].print_temperature_max;

	char temp_string[1000];
	if (Profile::configSettings[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
	{
		if (Profile::configSettings[toolIndex_].raft_temperature_control)
			sprintf(temp_string, ";Not applied M109 code to T%i by raft-temperature-control function\n", toolIndex_);
		else
			sprintf(temp_string, "M109 T%i S%.f		;Heat up %ith nozzle nozzle\n", toolIndex_, nozzleTemperature, toolIndex_);
	}
	else
	{
			sprintf(temp_string, "M109 T%i S%.f		;Heat up %ith nozzle nozzle\n", toolIndex_, nozzleTemperature, toolIndex_);
	}

	gcode.writeCode(temp_string);
}

int GCodeGenerator::getExtrusionWidthByLayer(int _layer_number, int _config_extusion_width)
{
	if (_layer_number == 0)
		return configs.front().initial_layer_extrusion_width;
	else
		return _config_extusion_width;
}

QString GCodeGenerator::generateM104Code()
{
	QString out_str;

	int tool_index = 0;
	for (auto it : printingInfo->getUsedState())
	{
		if (it)
		{
			double nozzle_temperature = Profile::configSettings[tool_index].print_temperature;
			double max_temperature = Profile::configSettings[tool_index].print_temperature_max;
			if (nozzle_temperature > max_temperature)
				nozzle_temperature = max_temperature;

			out_str.append(QString("M104 T%1 S%2	;Heat up the %1th nozzle without waiting").arg(tool_index).arg(nozzle_temperature));
		}

		out_str.append("\n");

		tool_index++;

		if (tool_index >= Profile::configSettings.size())
			break;
	}

	return out_str;
}

QString GCodeGenerator::generateM140Code()
{
	QString out_str;

	double bed_temperature = Profile::configSettings[0].print_bed_temperature;
	//M140//
	if (!(Profile::machineProfile.has_heated_bed.value))
		bed_temperature = 0;

	out_str.append(QString("M140 S%1			;Heat up the bed without waiting").arg(bed_temperature));
	
	return out_str;
}

QString GCodeGenerator::generateM109Code()
{
	QString out_str;

	int firstTool = 0;
	// hjkim modify 
	// 첫번째 사용하는 노즐만 M109로 온도 체크하고, 다른 노즐은 M104로 온도를 80도로 낮춤
	if (Profile::machineProfile.extruder_count.value >= 2)
	{
		if (Profile::configSettings[0].platform_adhesion == Generals::PlatformAdhesion::NoneAdhesion)
			firstTool = firstToolChangeInfo.at(0);
		else
			firstTool = Profile::configSettings[0].adhesion_cartridge_index;
	}

	int tool_index = 0;
	for (auto it : printingInfo->getUsedState())
	{
		if (tool_index == firstTool)
		{
			if (it)
			{
				//layer0 temperature는 사용안함 --> temperature layer list 이용//
				double nozzle_temperature = Profile::configSettings[tool_index].print_temperature;
				if (nozzle_temperature > Profile::configSettings[tool_index].print_temperature_max)
					nozzle_temperature = Profile::configSettings[tool_index].print_temperature_max;

				if (Profile::configSettings[0].platform_adhesion == Generals::PlatformAdhesion::Raft)
				{
					if (Profile::configSettings[tool_index].raft_temperature_control)
						out_str.append(QString(";Not applied M109 code to T%1 by raft-temperature-control function").arg(tool_index));
					else
						out_str.append(QString("M109 T%1 S%2	;Heat up %1th nozzle nozzle").arg(tool_index).arg(nozzle_temperature));
				}
				else
				{
					out_str.append(QString("M109 T%1 S%2	;Heat up %1th nozzle nozzle").arg(tool_index).arg(nozzle_temperature));
				}
			}
		}
		else
		{
			out_str.append(QString("M104 T%1 S%2	;Heat up the %1th nozzle without waiting").arg(tool_index).arg(Profile::configSettings[tool_index].initial_standby_temperature));
		}

		out_str.append("\n");

		tool_index++;

		if (tool_index >= Profile::configSettings.size())
			break;
	}

	return out_str;
}

QString GCodeGenerator::generateM190Code()
{
	QString out_str;

	double bed_temperature = Profile::configSettings[0].print_bed_temperature;

	if (!(Profile::machineProfile.has_heated_bed.value))
		bed_temperature = 0;

	out_str.append(QString("M190 S%1			;Heat up the bed").arg(bed_temperature));
	
	return out_str;
}