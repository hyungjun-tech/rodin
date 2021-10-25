#include "stdafx.h"

// cura profile convert class cpp to config file by swyang
#include "ProfileToConfig.h"
#include <math.h>
#include <algorithm>

ProfileToConfig::ProfileToConfig() { }
ProfileToConfig::~ProfileToConfig() { }

void ProfileToConfig::convertToConfig()
{
	if (Profile::sliceProfile.size() != Profile::configSettings.size())
		return;

	for (int n = 0; n < Profile::sliceProfile.size(); ++n)
	{
		convertToConfig(&Profile::sliceProfile[n], &Profile::sliceProfileCommon, &Profile::configSettings[n], n);
	}
}
void ProfileToConfig::convertToConfig(std::vector<SliceProfile>* sliceProfile, SliceProfileForCommon *sliceProfile_common, std::vector<ConfigSettings> *config)
{
	if (sliceProfile->size() != config->size())
		return;

	for (int n = 0; n < sliceProfile->size(); ++n)
	{
		convertToConfig(&sliceProfile->at(n), sliceProfile_common, &config->at(n), n);
	}
}
void ProfileToConfig::convertToConfig(SliceProfile *sliceProfile, SliceProfileForCommon *sliceProfile_common, ConfigSettings *config, const int _extruder_index)
{
	//슬라이스 프로파일에 common으로 임시 대체하여 config로 변경
	//이거 사용해야 함.
	SliceProfile tempProfile = *sliceProfile;
	sliceProfile_common->setSliceProfileDataFromCommon(&tempProfile);

	//TODO//
	//tempProfile.nozzle_size.value = 0.4;

	convertToConfig(&tempProfile, config, _extruder_index);
}

void ProfileToConfig::convertToConfig(SliceProfile *sliceProfile, ConfigSettings *config, const int _extruder_index)
{
	//config->material = sliceProfile->material.value;
	config->filament_material = sliceProfile->filament_material.value.toLatin1();
	config->print_temperature = sliceProfile->print_temperature.value;
	config->print_temperature_max = sliceProfile->print_temperature.max;
	config->print_bed_temperature = sliceProfile->print_bed_temperature.value;
	config->standby_temperature_enabled = sliceProfile->standby_temperature_enabled.value;
	config->operating_standby_temperature = sliceProfile->operating_standby_temperature.value;
	config->initial_standby_temperature = sliceProfile->initial_standby_temperature.value;
	config->temperature_layer_setting_enabled = sliceProfile->temperature_layer_setting_enabled.value;

	//colored layer시 preheat_enabled 강제종료는 slicing()에서 하도록 수정함// --> 여기서 다시 제어..//
	if (Generals::isLayerColorModeOn() || Generals::isReplicationUIMode()) config->preheat_enabled = 0;
	else config->preheat_enabled = sliceProfile->preheat_enabled.value;

	config->preheat_threshold_time = sliceProfile->preheat_threshold_time.value;
	config->preheat_temperature_falling_rate = 10 * 1000;
	config->preheat_temperature_rising_rate = 0.4 * 1000;

	config->zOffset_raft = sliceProfile->z_offset_raft.value * 1000;
	config->zOffset_except_raft = sliceProfile->z_offset_except_raft.value * 1000;
	config->bed_type = sliceProfile->bed_type.value;

	config->layer_height = sliceProfile->layer_height.value * 1000;

	if (sliceProfile->initial_layer_height.value > 0.0)	config->initial_layer_height = sliceProfile->initial_layer_height.value * 1000;
	else config->initial_layer_height = sliceProfile->layer_height.value * 1000;

	config->filament_diameter = sliceProfile->filament_diameter.value * 1000;
	

	//flow setting//
	config->overall_flow_control_enabled = sliceProfile->overall_flow_control_enabled.value;
	config->overall_flow = sliceProfile->overall_flow.value;
	if (sliceProfile->overall_flow_control_enabled.value == 1)
	{
		config->initial_layer_flow = sliceProfile->overall_flow.value;
		config->infill_flow = sliceProfile->overall_flow.value;
		config->outer_wall_flow = sliceProfile->overall_flow.value;
		config->inner_wall_flow = sliceProfile->overall_flow.value;
		config->top_bottom_flow = sliceProfile->overall_flow.value;
	}
	else
	{
		config->initial_layer_flow = sliceProfile->initial_layer_flow.value;
		config->infill_flow = sliceProfile->infill_flow.value;
		config->outer_wall_flow = sliceProfile->outer_wall_flow.value;
		config->inner_wall_flow = sliceProfile->inner_wall_flow.value;
		config->top_bottom_flow = sliceProfile->top_bottom_flow.value;
	}

	config->extrusion_width = calculateEdgeWidth(sliceProfile, _extruder_index) * 1000;
	config->initial_layer_extrusion_width = calculateEdgeWidth(sliceProfile, _extruder_index) * sliceProfile->initial_layer_width_factor.value / 100 * 1000;
	config->wall_extrusion_width = calculateEdgeWidth(sliceProfile, _extruder_index) * sliceProfile->wall_width_factor.value / 100 * 1000;
	config->infill_extrusion_width = calculateEdgeWidth(sliceProfile, _extruder_index) * sliceProfile->infill_width_factor.value / 100 * 1000;
	config->top_bottom_extrusion_width = calculateEdgeWidth(sliceProfile, _extruder_index) * sliceProfile->top_bottom_width_factor.value / 100 * 1000;
	//config->skirt_extrusion_width = calculateEdgeWidth(sliceProfile) * sliceProfile->skirt_width_factor.value / 100 * 1000;
	//config->brim_extrusion_width = calculateEdgeWidth(sliceProfile) * sliceProfile->brim_width_factor.value / 100 * 1000;
	config->support_main_extrusion_width = calculateEdgeWidth(sliceProfile, _extruder_index) * sliceProfile->support_main_width_factor.value / 100 * 1000;
	config->support_interface_roof_extrusion_width = calculateEdgeWidth(sliceProfile, _extruder_index) * sliceProfile->support_interface_roof_width_factor.value / 100 * 1000;
	config->support_interface_floor_extrusion_width = calculateEdgeWidth(sliceProfile, _extruder_index) * sliceProfile->support_interface_floor_width_factor.value / 100 * 1000;

	config->wall_print_direction = sliceProfile->wall_printing_direction.value;
	config->internal_moving_area = sliceProfile->internal_moving_area.value;

	if (sliceProfile->solid_bottom.value == 1) config->down_skin_count = calculateSolidLayerCount(sliceProfile);
	else config->down_skin_count = 0;

	if (sliceProfile->solid_top.value == 1) config->up_skin_count = calculateSolidLayerCount(sliceProfile);
	else config->up_skin_count = 0;

	config->skin_outline = sliceProfile->skin_outline.value;
	config->skin_type = sliceProfile->skin_type.value;
	config->skin_removal_width_top = sliceProfile->skin_removal_width_top.value * 1000;
	config->skin_removal_width_bottom = sliceProfile->skin_removal_width_bottom.value * 1000;
	config->skin_overlap = sliceProfile->skin_overlap.value;
	config->infill_before_wall = sliceProfile->infill_before_wall.value;
	config->infill_overlap = sliceProfile->infill_overlap.value;
	

	//
	config->print_speed = sliceProfile->print_speed.value;
	config->initial_layer_speed = sliceProfile->initial_layer_speed.value;
	if (sliceProfile->infill_speed.value > 0) config->infill_speed = sliceProfile->infill_speed.value; else config->infill_speed = sliceProfile->print_speed.value;
	if (sliceProfile->outer_wall_speed.value > 0) config->outer_wall_speed = sliceProfile->outer_wall_speed.value; else config->outer_wall_speed = sliceProfile->print_speed.value;
	if (sliceProfile->inner_wall_speed.value > 0) config->inner_wall_speed = sliceProfile->inner_wall_speed.value; else config->inner_wall_speed = sliceProfile->print_speed.value;
	if (sliceProfile->top_bottom_speed.value > 0) config->top_bottom_speed = sliceProfile->top_bottom_speed.value; else config->top_bottom_speed = sliceProfile->print_speed.value;
	config->travel_speed = sliceProfile->travel_speed.value;
	config->slower_layers_count = sliceProfile->slower_layers_count.value;


	if (sliceProfile->fan_enabled.value == 1)
		config->fan_speed_regular = sliceProfile->fan_speed_regular.value;
	else
		config->fan_speed_regular = 0;

	if (sliceProfile->fan_enabled.value == 1)
		config->fan_speed_max = sliceProfile->fan_speed_max.value;
	else
		config->fan_speed_max = 0;

	//support//
	if (minimalExtruderCount(sliceProfile) > 1)
	{
		if (sliceProfile->support_main_cartridge_index.value == 0) config->support_main_cartridge_index = 0;
		else if (sliceProfile->support_main_cartridge_index.value == 1) config->support_main_cartridge_index = 1;
	}
	else config->support_main_cartridge_index = 0;

	config->support_placement = sliceProfile->support_placement.value;
	if (sliceProfile->support_placement.value == 0)
	{
		config->support_angle = -1;
	}
	else
	{
		//support angle값이 0이면.. 엔진에서 cosAngle을 등호값이 있어야 함.. 나중에 수정.. 일단은 임시로..//
		if (sliceProfile->support_angle.value == 0)
		{
			config->support_angle = 1;
		}
		else
		{
			config->support_angle = sliceProfile->support_angle.value;
		}
	}

	if (sliceProfile->support_placement.value == 2) config->support_everywhere = 1; else config->support_everywhere = 0;			// Everywhere
	if (sliceProfile->support_infill_density.value > 0) config->support_line_distance = 100 * calculateEdgeWidth(sliceProfile, _extruder_index) * 1000 / sliceProfile->support_infill_density.value; else config->support_line_distance = -1;
	config->support_xy_distance = sliceProfile->support_xy_distance.value * 1000;
	config->support_z_distance = sliceProfile->support_z_distance.value * 1000;
	config->support_main_flow = sliceProfile->support_main_flow.value;
	config->support_horizontal_expansion = sliceProfile->support_horizontal_expansion.value * 1000;
	config->support_main_speed = sliceProfile->support_main_speed.value;
	config->support_interface_enabled = sliceProfile->support_interface_enabled.value;
	config->support_interface_pattern = sliceProfile->support_interface_pattern.value;
	config->support_interface_roof_layers_count = sliceProfile->support_interface_roof_layers_count.value;
	config->support_interface_roof_speed = sliceProfile->support_interface_roof_speed.value;
	config->support_interface_roof_flow = sliceProfile->support_interface_roof_flow.value;
	config->support_interface_floor_layers_count = sliceProfile->support_interface_floor_layers_count.value;
	config->support_interface_floor_speed = sliceProfile->support_interface_floor_speed.value;
	config->support_interface_floor_flow = sliceProfile->support_interface_floor_flow.value;
	config->support_infill_overlap = sliceProfile->infill_overlap.value;
	config->support_interface_cartridge_index = config->support_main_cartridge_index;
	config->support_inset_count = 1;


	//
	if (sliceProfile->retraction_enable.value == true) config->retraction_amount = sliceProfile->retraction_amount.value * 1000; else config->retraction_amount = 0;
	config->retraction_speed = sliceProfile->retraction_speed.value;
	config->retraction_minimal_distance = sliceProfile->retraction_min_travel.value * 1000;
	config->retraction_z_hop = sliceProfile->retraction_hop.value * 1000;
	config->retraction_amount_prime = 0.0;
	config->minimal_extrusion_before_retraction = sliceProfile->retraction_minimal_extrusion.value * 1000;
	config->multi_volume_overlap = sliceProfile->overlap_dual.value * 1000;
	config->object_sink = std::max(double(0.0), sliceProfile->object_sink.value * 1000);
	config->minimal_layer_time = sliceProfile->cool_min_layer_time.value;
	config->minimal_feedrate = sliceProfile->cool_min_feedrate.value;

	//tool_change
	config->toolchange_retraction_amount = sliceProfile->toolchange_retraction_amount.value * 1000;
	config->toolchange_retraction_speed = sliceProfile->toolchange_retraction_speed.value;
	config->toolchange_lowering_bed = sliceProfile->toolchange_lowering_bed.value * 1000;
	config->toolchange_extra_restart_amount = sliceProfile->toolchange_extra_restart_amount.value * 1000;
	config->toolchange_extra_restart_speed = sliceProfile->toolchange_extra_restart_speed.value;


	if (sliceProfile->cool_head_lift.value == 1) config->cool_head_lift = 1; else config->cool_head_lift = 0;

	//
	config->fan_full_on_layer_number = (sliceProfile->fan_full_height.value * 1000 - config->initial_layer_height - 1) / config->layer_height + 1;
	if (config->fan_full_on_layer_number < 0) config->fan_full_on_layer_number = 0;

	//
	if (sliceProfile->retraction_combing.value == 1) config->enable_combing = 1;
	else if (sliceProfile->retraction_combing.value == 2) config->enable_combing = 2;
	else config->enable_combing = 0;

	config->support_main_pattern = sliceProfile->support_main_pattern.value;

	//infill//
	config->infill_pattern = sliceProfile->infill_pattern.value;

	config->infill_density = sliceProfile->infill_density.value;

	if (sliceProfile->infill_density.value == 0) config->sparse_infill_line_distance = -1;
	else if (sliceProfile->infill_density.value == 100)
	{
		config->sparse_infill_line_distance = config->extrusion_width;
		config->down_skin_count = 10000;
		config->up_skin_count = 10000;
	}
	else
	{
		config->sparse_infill_line_distance = 100 * config->infill_extrusion_width / sliceProfile->infill_density.value;
	}

	//platform adhesion type setting//
	config->platform_adhesion = sliceProfile->platform_adhesion.value;
	switch (sliceProfile->platform_adhesion.value)
	{
	case Generals::PlatformAdhesion::NoneAdhesion:
		config->skirt_line_count = 0;
		config->adhesion_cartridge_index = 0;	//안전하게 0으로 초기화함.

		raftSettingReset(config);
		break;

	case Generals::PlatformAdhesion::Skirt:
		config->adhesion_cartridge_index = sliceProfile->adhesion_cartridge_index.value;
		config->skirt_distance = sliceProfile->skirt_gap.value * 1000;
		config->skirt_line_count = sliceProfile->skirt_line_count.value;
		config->skirt_min_length = sliceProfile->skirt_minimal_length.value * 1000;
		//config->skirt_speed = sliceProfile->skirt_speed.value;
		//config->skirt_flow = sliceProfile->skirt_flow.value;
	
		raftSettingReset(config);
		break;


	case Generals::PlatformAdhesion::Brim:
		config->adhesion_cartridge_index = sliceProfile->adhesion_cartridge_index.value;

		config->skirt_distance = 0;
		config->skirt_line_count = sliceProfile->brim_line_count.value;
		config->skirt_min_length = sliceProfile->skirt_minimal_length.value * 1000;
		//config->brim_speed = sliceProfile->brim_speed.value;
		//config->brim_flow = sliceProfile->brim_flow.value;

		raftSettingReset(config);
		break;


	case Generals::PlatformAdhesion::Raft:
		config->adhesion_cartridge_index = sliceProfile->adhesion_cartridge_index.value;

		config->skirt_distance = 0;
		config->skirt_line_count = 0;

		config->raft_margin = sliceProfile->raft_margin.value * 1000;

		//raft line spacing//
		if (sliceProfile->raft_line_spacing.value == 0.0) config->raft_line_spacing = 0.1 * 1000;
		else config->raft_line_spacing = sliceProfile->raft_line_spacing.value * 1000;

		config->raft_base_thickness = sliceProfile->raft_base_thickness.value * 1000;
		config->raft_base_line_width = sliceProfile->raft_base_line_width.value * 1000;
		config->raft_base_temperature = sliceProfile->raft_base_temperature.value;
		config->raft_interface_thickness = sliceProfile->raft_interface_thickness.value * 1000;

		//raft interface linewidth//
		if (sliceProfile->raft_interface_line_width.value == 0.0)
		{
			config->raft_interface_line_width = Profile::machineProfile.nozzle_size.at(_extruder_index).value * 1000;
			config->raft_interface_line_spacing = Profile::machineProfile.nozzle_size.at(_extruder_index).value * 1000 * 2.0;
		}
		else
		{
			config->raft_interface_line_width = sliceProfile->raft_interface_line_width.value * 1000;
			config->raft_interface_line_spacing = sliceProfile->raft_interface_line_width.value * 1000 * 2.0;
		}
		config->raft_interface_temperature = sliceProfile->raft_interface_temperature.value;

		config->raft_airgap_initial_layer = (sliceProfile->raft_airgap_initial_layer.value * 1000) + (sliceProfile->raft_airgap_all.value * 1000);
		config->raft_airgap_all = sliceProfile->raft_airgap_all.value * 1000;
		config->raft_base_speed = sliceProfile->raft_base_speed.value;
		config->raft_interface_speed = sliceProfile->raft_interface_speed.value;
		//config->raftFanSpeed = 50; 
		config->raft_fan_speed = 0;
		config->raft_surface_thickness = sliceProfile->raft_surface_thickness.value * 1000;

		//raft surface linewidth//
		if (sliceProfile->raft_surface_line_width.value == 0.0)
		{
			config->raft_surface_line_width = Profile::machineProfile.nozzle_size.at(_extruder_index).value * 1000;
			config->raft_surface_line_spacing = Profile::machineProfile.nozzle_size.at(_extruder_index).value * 1000;
		}
		else
		{
			config->raft_surface_line_width = sliceProfile->raft_surface_line_width.value * 1000;
			config->raft_surface_line_spacing = sliceProfile->raft_surface_line_width.value * 1000;
		}

		config->raft_surface_layers = sliceProfile->raft_surface_layers.value;
		config->raft_surface_speed = sliceProfile->raft_surface_speed.value;
		config->raft_surface_initial_temperature = sliceProfile->raft_surface_initial_temperature.value;
		config->raft_surface_last_temperature = sliceProfile->raft_surface_last_temperature.value;

		config->raft_inset_enabled = sliceProfile->raft_inset_enabled.value;
		config->raft_inset_offset = sliceProfile->raft_inset_offset.value * 1000;

		config->raft_temperature_control = sliceProfile->raft_temperature_control.value;
		break;
	}

	config->raft_incline_enabled = sliceProfile->raft_incline_enabled.value;

	//fixHorrible//
	config->fix_horrible = 0x00;

	if (sliceProfile->fix_horrible_union_all_type_a.value == 1) config->fix_horrible |= 0x01;
	if (sliceProfile->fix_horrible_union_all_type_b.value == 1) config->fix_horrible |= 0x02;
	if (sliceProfile->fix_horrible_use_open_bits.value == 1) config->fix_horrible |= 0x10;
	if (sliceProfile->fix_horrible_extensive_stitching.value == 1) config->fix_horrible |= 0x04;

	if (config->layer_height <= 0) config->layer_height = 1000;

	//gcode flavor
	//config->gcodeFlavor = machineProfile->gcode_flavor.value;// sliceProfile->gcode_flavor.value;

	//
	config->spiralize_mode = sliceProfile->spiralize.value;
	config->simple_mode = sliceProfile->simple_mode.value;

	if (config->simple_mode)	config->inset_count = 0;
	else config->inset_count = calculateLineCount(sliceProfile, _extruder_index);

	//////////////////////////// extrudercount ??

	//wipeTower//
	if (Profile::machineProfile.extruder_count.value < 2)
		config->wipe_tower_enable = 0;

	if (Generals::isLayerColorModeOn() || Generals::isReplicationUIMode()) config->wipe_tower_enable = 0;
	else config->wipe_tower_enable = sliceProfile->wipe_tower_enabled.value;

	if (sliceProfile->wipe_tower_infill_density.value == 0) config->wipe_tower_sparse_infill_line_distance = -1;
	else if (sliceProfile->wipe_tower_infill_density.value == 100) config->wipe_tower_sparse_infill_line_distance = config->extrusion_width;
	else config->wipe_tower_sparse_infill_line_distance = 100 * calculateEdgeWidth(sliceProfile, _extruder_index) * 1000 / sliceProfile->wipe_tower_infill_density.value;
	
	config->wipe_tower_position = sliceProfile->wipe_tower_position.value;
	config->wipe_tower_raft_margin = sliceProfile->wipe_tower_raft_margin.value * 1000;
	config->wipe_tower_flow = sliceProfile->wipe_tower_flow.value;
	config->wipe_tower_infill_density = sliceProfile->wipe_tower_infill_density.value;
	config->wipe_tower_base_size = sliceProfile->wipe_tower_base_size.value * 1000;
	config->wipe_tower_base_layer_count = sliceProfile->wipe_tower_base_layer_count.value;
	config->wipe_tower_outer_size = sliceProfile->wipe_tower_outer_size.value * 1000;
	config->wipe_tower_outer_wall_thickness = sliceProfile->wipe_tower_outer_wall_thickness.value * 1000;
	config->wipe_tower_outer_inner_gap = sliceProfile->wipe_tower_outer_inner_gap.value * 1000;
	config->wipe_tower_inner_size = sliceProfile->wipe_tower_inner_size.value * 1000;
	config->wipe_tower_outer_cartridge_index = sliceProfile->wipe_tower_outer_cartridge_index.value;
	config->wipe_tower_inner_cartridge_index = sliceProfile->wipe_tower_inner_cartridge_index.value;
	config->wipe_tower_speed = sliceProfile->wipe_tower_speed.value;

	//config->enable_ooze_shield = sliceProfile->ooze_shield.value;


	//machine center
	config->auto_center = Profile::machineProfile.auto_center.value;

	if (config->auto_center == 1)
	{
		if (Profile::machineProfile.machine_center_is_zero.value)
		{
			config->object_position.X = 0;
			config->object_position.Y = 0;
		}
		else
		{
			config->object_position.X = Profile::getMachineWidth_calculated() * 1000 * 0.5;
			config->object_position.Y = Profile::getMachineDepth_calculated() * 1000 * 0.5;
		}
	}
	else
	{
		if (!Profile::machineProfile.machine_center_is_zero.value)
		{
			config->object_position.X = 0;
			config->object_position.Y = 0;
		}
		else
		{
			config->object_position.X = Profile::getMachineWidth_calculated() * 1000 * 0.5;
			config->object_position.Y = Profile::getMachineDepth_calculated() * 1000 * 0.5;
		}
	}

	//machine//
	config->machine_width = Profile::getMachineWidth_calculated() * 1000;
	config->machine_depth = Profile::getMachineDepth_calculated() * 1000;
	config->machine_height = Profile::getMachineHeight_calculated() * 1000;
	config->machine_center_is_bedcenter = Profile::machineProfile.machine_center_is_zero.value;
		
	// new // pre-retraction and over-moving
	/*config->preretration0Length = sliceProfile->retraction_preretraction_0_distance.value * 1000;
	config->preretration0FilamentLength = sliceProfile->retraction_preretraction_0_amount.value * 1000;
	config->overmoving0Length = sliceProfile->retraction_overmoving_0_distance.value * 1000;

	config->preretrationXLength = sliceProfile->retraction_preretraction_X_distance.value * 1000;
	config->preretrationXFilamentLength = sliceProfile->retraction_preretraction_X_amount.value * 1000;
	config->overmovingXLength = sliceProfile->retraction_overmoving_X_distance.value * 1000;*/


	if (sliceProfile->path_optimization_enabled.value)
	{
		//new//optimize path direction
		config->path_optimization_parameter[0] = 1;
		config->path_optimization_parameter[1] = 1;
		config->path_optimization_parameter[2] = 100;
		config->path_optimization_filtering_angle = 60;
	}
	else
	{
		config->path_optimization_parameter[0] = 0;
		config->path_optimization_parameter[1] = 0;
		config->path_optimization_parameter[2] = 0;
		config->path_optimization_filtering_angle = 180;
	}
				
	config->retraction_with_combing = 0;

	config->pre_switch_extruder_code.clear();
	config->pre_switch_extruder_code = generatePreSwitchExtruderCode(sliceProfile);

	config->pre_switch_extruder_code.clear();
	config->pre_switch_extruder_code = generatePostSwitchExtruderCode(sliceProfile);

	//adjust Z Gap//temp//

	if (Generals::isReplicationUIMode())
	{
		config->adjust_z_gap_layers = 1;
		config->adjust_z_gap_thickness = 0.1 * 1000;
		config->adjust_postion = 1;
	}
	else
	{
		config->adjust_z_gap_layers = 0;
		config->adjust_z_gap_thickness = 0;
		config->adjust_postion = 0;
	}
}

double ProfileToConfig::calculateEdgeWidth(SliceProfile *sliceProfile, const int _extruder_index)
{
	double wallThickness = sliceProfile->wall_thickness.value;
	double nozzleSize = Profile::machineProfile.nozzle_size.at(_extruder_index).value;

	if (sliceProfile->spiralize.value == 1 || sliceProfile->simple_mode.value == 1)
		return wallThickness;

	if (wallThickness < 0.01)			return nozzleSize;
	if (wallThickness < nozzleSize)		return wallThickness;

	double lineCount = int(wallThickness / (nozzleSize - 0.0001));

	if (lineCount == 0)		return nozzleSize;

	double	lineWidth = wallThickness / lineCount;
	double	lineWidthAlt = wallThickness / (lineCount + 1);

	if (lineWidth > nozzleSize * 1.5)	return lineWidthAlt;

	return lineWidth;
}

int ProfileToConfig::calculateLineCount(SliceProfile *sliceProfile, const int _extruder_index)
{
	double	wallThickness = sliceProfile->wall_thickness.value;
	double	nozzleSize = Profile::machineProfile.nozzle_size.at(_extruder_index).value;

	if (wallThickness < 0.01)			return 0;
	if (wallThickness < nozzleSize)		return 1;
	if (sliceProfile->spiralize.value == 1 || sliceProfile->simple_mode.value == 1)		return 1;

	int	lineCount = int(wallThickness / (nozzleSize - 0.0001));

	if (lineCount < 1)		lineCount = 1;

	double lineWidth = wallThickness / lineCount;
	double lineWidthAlt = wallThickness / (lineCount + 1);

	if (lineWidth > nozzleSize * 1.5)	return (lineCount + 1);

	return lineCount;
}

int ProfileToConfig::calculateSolidLayerCount(SliceProfile *sliceProfile)
{
	double layer_height = sliceProfile->layer_height.value;
	double top_bottom_thickness = sliceProfile->top_bottom_thickness.value;

	if (layer_height == 0.0)		return 1;

	return int(ceil((top_bottom_thickness - 0.0001) / layer_height));
}

int ProfileToConfig::minimalExtruderCount(SliceProfile *sliceProfile)
{
	if (sliceProfile->support_placement.value == 0)	return 1;		// 0=None
	if (Profile::machineProfile.extruder_count.value > 1)	return 2;		// dual extrusion

	return 1;
}

void ProfileToConfig::raftSettingReset(ConfigSettings *config)
{
	config->raft_margin = 0;
	config->raft_line_spacing = 0;
	config->raft_base_thickness = 0;
	config->raft_base_line_width = 0;
	config->raft_interface_thickness = 0;
	config->raft_interface_line_width = 0;
	config->raft_interface_line_spacing = 0;
	config->raft_airgap_all = 0;
	config->raft_airgap_initial_layer = 0;
	config->raft_base_speed = 0;
	config->raft_interface_speed = 0;
	config->raft_fan_speed = 0;
	config->raft_surface_thickness = 0;
	config->raft_surface_line_width = 0;
	config->raft_surface_line_spacing = 0;
	config->raft_surface_layers = 0;
	config->raft_surface_speed = 0;
}

std::string ProfileToConfig::generatePreSwitchExtruderCode(SliceProfile *sliceProfile)
{
	char str[1000];

	//bed descending when tool-changing starting..
	sprintf_s(str, sizeof(str), "G0 Z%.3f\n", sliceProfile->layer_height.value);
	
	std::string resultStr(str);

	return resultStr;
}

std::string ProfileToConfig::generatePostSwitchExtruderCode(SliceProfile *sliceProfile)
{
	char str[1000];

	//bed lifting when tool-changing is completed..
	sprintf_s(str, sizeof(str), "G0 Z-%.3f\n", sliceProfile->layer_height.value);

	//sprintf_s(str, sizeof(str),
	//	"M109 S%.f\n"
	//	"M190 S%.f\n"
	//	, sliceProfile->print_temperature.value, sliceProfile->print_bed_temperature.value);

	std::string resultStr(str);

	return resultStr;
}
