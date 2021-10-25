#include "stdafx.h"
#include "ProfileControl.h"
#include "ProfileToConfig.h"
#include "CustomProfileList.h"
#include "ProfileSettingMultiDlg.h"
#include "CartridgeInfo.h"

//ProfileControl::ProfileControl(QString* profilePath, MachineProfile* machineProfile, std::vector<SliceProfile>* sliceProfile_multi, std::vector<SliceProfileForCartridge>* sliceProfile_cartridge, SliceProfileForCommon* sliceProfile_common, std::vector<ConfigSettings>* config_multi)
ProfileControl::ProfileControl()
{
}

ProfileControl::~ProfileControl()
{
}

bool ProfileControl::loadMachineProfile()
{
	//MachineProfile machineProfile;
	if (!Profile::machineProfile.loadMachineProfile(Generals::qstringTowchar_t(Profile::profilePath + QString("machine_setting_value.ini"))))
		return false;

	Profile::machineProfile.loadStartEndDefaultCode(Generals::qstringTowchar_t(Profile::profilePath + QString("profile_start_end_code.ini")));
	return true;
}

bool ProfileControl::loadProfileByMachine(QString settingMode)
{
	return loadProfileByMachine(settingMode, &Profile::sliceProfile, &Profile::sliceProfileCommon, &Profile::configSettings);
}

bool ProfileControl::loadProfileByMachine(QString settingMode, std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common, std::vector<ConfigSettings>* config_multi)
{
	//QString profilePath = Logger::getAppPath() + "\\profile\\" + machineModel + "\\";
	//m_profilePath = profilePath;
	//m_machineModel = machineModel;

	//profile의 loading순서는 반드시 지켜야 함//
	//mahcine -> slice 순으로..//
	//machine profile value loading...//
	sliceProfile_multi->resize(Profile::machineProfile.extruder_count.value);
	config_multi->resize(Profile::machineProfile.extruder_count.value);

	CartridgeInfo::cartInit();
	CartridgeInfo::layerColorListInit();

	//start-end code loading...//

	//slice profile loading..//	
	//easy mode//
	if (settingMode == "easy")
	{
		//machine이 변경 되면, 무조건 normal, PLA로 resetting..//
		if (!resetEasyProfile())
			return false;
	}
	//advanced mode//
	else
	{
		if (!loadRecentProfile())
			return false;
	}

	return true;
}

//multi profile 에서 common profile 추출
void ProfileControl::generateCommonProfileFromSliceProfile(int raftType, int raftIdx, int supportType, int supportIdx)
{
	generateCommonProfileFromSliceProfile(&Profile::sliceProfile, &Profile::sliceProfileCommon, raftType, raftIdx, supportType, supportIdx);
}
void ProfileControl::generateCommonProfileFromSliceProfile(std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common, int raftType, int raftIdx, int supportType, int supportIdx)
{
	if (raftIdx == -1)
		raftIdx = 0;
	if (supportIdx == -1)
		supportIdx = 0;
	if (raftType == -1)
		raftType = sliceProfile_multi->at(raftIdx).platform_adhesion.value;
	if (supportType == -1)
		supportType = sliceProfile_multi->at(supportIdx).support_placement.value;

	sliceProfile_common->getCommonProfileFromSliceProfile(&sliceProfile_multi->at(0));
	setRaftProfile(sliceProfile_multi, sliceProfile_common, raftType, raftIdx);

	setSupportProfile(sliceProfile_multi, sliceProfile_common, supportType, supportIdx);

	setCommonProfile(sliceProfile_multi, sliceProfile_common);
}

void ProfileControl::setRaftProfile(std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common, int raftType, int raftIdx)
{
	if (raftIdx == -1)
		raftIdx = 0;
	if (raftType == -1)
		raftType = sliceProfile_multi->at(raftIdx).platform_adhesion.value;

	if (Profile::machineProfile.machine_expanded_print_mode.value)
		raftIdx = Profile::machineProfile.machine_expanded_print_cartridgeIndex.value;

	sliceProfile_common->platform_adhesion.value = raftType;
	sliceProfile_common->adhesion_cartridge_index.value = raftIdx;

	sliceProfile_common->raft_margin = sliceProfile_multi->at(raftIdx).raft_margin;
	sliceProfile_common->raft_line_spacing = sliceProfile_multi->at(raftIdx).raft_line_spacing;
	sliceProfile_common->raft_incline_enabled = sliceProfile_multi->at(raftIdx).raft_incline_enabled;
	sliceProfile_common->raft_temperature_control = sliceProfile_multi->at(raftIdx).raft_temperature_control;
	sliceProfile_common->raft_airgap_initial_layer = sliceProfile_multi->at(raftIdx).raft_airgap_initial_layer;
	sliceProfile_common->raft_airgap_all = sliceProfile_multi->at(raftIdx).raft_airgap_all;

	sliceProfile_common->raft_base_thickness = sliceProfile_multi->at(raftIdx).raft_base_thickness;
	sliceProfile_common->raft_base_line_width = sliceProfile_multi->at(raftIdx).raft_base_line_width;
	sliceProfile_common->raft_base_speed = sliceProfile_multi->at(raftIdx).raft_base_speed;
	sliceProfile_common->raft_base_temperature = sliceProfile_multi->at(raftIdx).raft_base_temperature;
	sliceProfile_common->raft_interface_thickness = sliceProfile_multi->at(raftIdx).raft_interface_thickness;
	sliceProfile_common->raft_interface_line_width = sliceProfile_multi->at(raftIdx).raft_interface_line_width;
	sliceProfile_common->raft_interface_speed = sliceProfile_multi->at(raftIdx).raft_interface_speed;
	sliceProfile_common->raft_interface_temperature = sliceProfile_multi->at(raftIdx).raft_interface_temperature;
	sliceProfile_common->raft_surface_layers = sliceProfile_multi->at(raftIdx).raft_surface_layers;
	sliceProfile_common->raft_surface_thickness = sliceProfile_multi->at(raftIdx).raft_surface_thickness;
	sliceProfile_common->raft_surface_line_width = sliceProfile_multi->at(raftIdx).raft_surface_line_width;
	sliceProfile_common->raft_surface_speed = sliceProfile_multi->at(raftIdx).raft_surface_speed;
	sliceProfile_common->raft_surface_initial_temperature = sliceProfile_multi->at(raftIdx).raft_surface_initial_temperature;
	sliceProfile_common->raft_surface_last_temperature = sliceProfile_multi->at(raftIdx).raft_surface_last_temperature;

	sliceProfile_common->raft_inset_enabled = sliceProfile_multi->at(raftIdx).raft_inset_enabled;
	sliceProfile_common->raft_inset_offset = sliceProfile_multi->at(raftIdx).raft_inset_offset;

	//카트리지에 따르지 않고 무조건 첫번째 카트리지 정보 사용
	sliceProfile_common->brim_line_count = sliceProfile_multi->front().brim_line_count;
	//sliceProfile_common->brim_width_factor = sliceProfile_multi->front().brim_width_factor;
	//sliceProfile_common->brim_speed = sliceProfile_multi->front().brim_speed;
	//sliceProfile_common->brim_flow = sliceProfile_multi->front().brim_flow;

	if (!sliceProfile_multi->at(0).flag_custom && !sliceProfile_multi->at(0).flag_imported_profile)
	{
		//machine : enabled to select bed type..//
		if (Profile::machineProfile.machine_bed_selected_enabled.value == 1)
		{
			sliceProfile_common->initial_layer_width_factor.value = 100; //PLATFORM_RAFT 무조건 100

			if (raftType == Generals::PlatformAdhesion::NoneAdhesion || raftType == Generals::PlatformAdhesion::Skirt)
			{
				sliceProfile_common->initial_layer_width_factor.value = 200;
			}
			else if (raftType == Generals::PlatformAdhesion::Brim)
			{
				if (sliceProfile_multi->at(raftIdx).bed_type.value == 2) //ABS Side B 일 때
					sliceProfile_common->initial_layer_width_factor.value = 200;
			}
		}
		//machine : disabled to select bed type..//
		else
		{
			if (raftType == Generals::PlatformAdhesion::Brim || raftType == Generals::PlatformAdhesion::Raft)
			{
				if (!Profile::machineProfile.has_heated_bed.value) sliceProfile_common->initial_layer_width_factor.value = 120;
				else sliceProfile_common->initial_layer_width_factor.value = 100;
			}
			else
			{
				sliceProfile_common->initial_layer_width_factor.value = 200;
			}

		}
	}

	sliceProfile_common->print_bed_temperature = sliceProfile_multi->at(raftIdx).print_bed_temperature;

	if (!Profile::machineProfile.has_heated_bed.value)
	{
		sliceProfile_common->print_bed_temperature.value = 0;
		sliceProfile_common->raft_temperature_control.value = 0;
	}
	else if (raftType != Generals::PlatformAdhesion::Raft)
		sliceProfile_common->print_bed_temperature = getMaxBedTemperature(*sliceProfile_multi);
}

void ProfileControl::setSupportProfile(std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common, int supportType, int supportIdx)
{
	if (supportIdx == -1)
		supportIdx = 0;
	if (supportType == -1)
		supportType = sliceProfile_multi->at(supportIdx).support_placement.value;

	if (Profile::machineProfile.machine_expanded_print_mode.value)
		supportIdx = Profile::machineProfile.machine_expanded_print_cartridgeIndex.value;

	sliceProfile_common->support_placement.value = supportType;
	sliceProfile_common->support_main_cartridge_index.value = supportIdx;
	sliceProfile_common->support_main_pattern = sliceProfile_multi->at(supportIdx).support_main_pattern;
	sliceProfile_common->support_xy_distance = sliceProfile_multi->at(supportIdx).support_xy_distance;
	sliceProfile_common->support_z_distance = sliceProfile_multi->at(supportIdx).support_z_distance;
	sliceProfile_common->support_infill_density = sliceProfile_multi->at(supportIdx).support_infill_density;
	sliceProfile_common->support_angle = sliceProfile_multi->at(supportIdx).support_angle;
	sliceProfile_common->support_horizontal_expansion = sliceProfile_multi->at(supportIdx).support_horizontal_expansion;
	sliceProfile_common->support_main_speed = sliceProfile_multi->at(supportIdx).support_main_speed;
	sliceProfile_common->support_main_flow = sliceProfile_multi->at(supportIdx).support_main_flow;
	sliceProfile_common->support_main_width_factor = sliceProfile_multi->at(supportIdx).support_main_width_factor;

	sliceProfile_common->support_interface_enabled = sliceProfile_multi->at(supportIdx).support_interface_enabled;
	sliceProfile_common->support_interface_pattern = sliceProfile_multi->at(supportIdx).support_interface_pattern;
	sliceProfile_common->support_interface_roof_layers_count = sliceProfile_multi->at(supportIdx).support_interface_roof_layers_count;
	sliceProfile_common->support_interface_roof_width_factor = sliceProfile_multi->at(supportIdx).support_interface_roof_width_factor;
	sliceProfile_common->support_interface_roof_flow = sliceProfile_multi->at(supportIdx).support_interface_roof_flow;
	sliceProfile_common->support_interface_roof_speed = sliceProfile_multi->at(supportIdx).support_interface_roof_speed;
	sliceProfile_common->support_interface_floor_layers_count = sliceProfile_multi->at(supportIdx).support_interface_floor_layers_count;
	sliceProfile_common->support_interface_floor_width_factor = sliceProfile_multi->at(supportIdx).support_interface_floor_width_factor;
	sliceProfile_common->support_interface_floor_flow = sliceProfile_multi->at(supportIdx).support_interface_floor_flow;
	sliceProfile_common->support_interface_floor_speed = sliceProfile_multi->at(supportIdx).support_interface_floor_speed;

}

void ProfileControl::setCommonProfile(std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common)
{
	sliceProfile_common->layer_height = sliceProfile_multi->at(0).layer_height;
	//sliceProfile_common->nozzle_size = sliceProfile_multi->at(0).nozzle_size;
	sliceProfile_common->z_offset_raft = sliceProfile_multi->at(0).z_offset_raft;

	if (Profile::machineProfile.machine_bed_selected_enabled.value)
	{
		// Side A B 관련 - z_offset_except_raft 낮은 게 우선순위
		int preferredBedSide = 0;
		double minZoffset = std::fmin(sliceProfile_multi->at(0).z_offset_except_raft.value, sliceProfile_multi->at(1).z_offset_except_raft.value);
		if (minZoffset == sliceProfile_multi->at(1).z_offset_except_raft.value) preferredBedSide = 1;

		sliceProfile_common->z_offset_except_raft = sliceProfile_multi->at(preferredBedSide).z_offset_except_raft;
	}
	else sliceProfile_common->z_offset_except_raft = sliceProfile_multi->at(0).z_offset_except_raft;

	sliceProfile_common->skirt_line_count = sliceProfile_multi->at(0).skirt_line_count;
	//sliceProfile_common->skirt_width_factor = sliceProfile_multi->at(0).skirt_width_factor;
	sliceProfile_common->skirt_gap = sliceProfile_multi->at(0).skirt_gap;
	sliceProfile_common->skirt_minimal_length = sliceProfile_multi->at(0).skirt_minimal_length;
	//sliceProfile_common->skirt_speed = sliceProfile_multi->at(0).skirt_speed;
	//sliceProfile_common->skirt_flow = sliceProfile_multi->at(0).skirt_flow;
	sliceProfile_common->brim_line_count = sliceProfile_multi->at(0).brim_line_count;
	//sliceProfile_common->brim_width_factor = sliceProfile_multi->at(0).brim_width_factor;
	//sliceProfile_common->brim_speed = sliceProfile_multi->at(0).brim_speed;
	//sliceProfile_common->brim_flow = sliceProfile_multi->at(0).brim_flow;
	
	sliceProfile_common->retraction_combing = sliceProfile_multi->at(0).retraction_combing;
	sliceProfile_common->toolchange_lowering_bed = sliceProfile_multi->at(0).toolchange_lowering_bed;
	
	sliceProfile_common->spiralize = sliceProfile_multi->at(0).spiralize;
	sliceProfile_common->simple_mode = sliceProfile_multi->at(0).simple_mode;
	sliceProfile_common->fix_horrible_union_all_type_a = sliceProfile_multi->at(0).fix_horrible_union_all_type_a;
	sliceProfile_common->fix_horrible_union_all_type_b = sliceProfile_multi->at(0).fix_horrible_union_all_type_b;
	sliceProfile_common->fix_horrible_use_open_bits = sliceProfile_multi->at(0).fix_horrible_use_open_bits;
	sliceProfile_common->fix_horrible_extensive_stitching = sliceProfile_multi->at(0).fix_horrible_extensive_stitching;
	sliceProfile_common->overlap_dual = sliceProfile_multi->at(0).overlap_dual;
	sliceProfile_common->object_sink = sliceProfile_multi->at(0).object_sink;


	//가중치로 계산
	int cartIdx = getCartIndex_maxWeight(sliceProfile_multi);
	sliceProfile_common->initial_layer_height = sliceProfile_multi->at(cartIdx).initial_layer_height;
	sliceProfile_common->raft_airgap_all = sliceProfile_multi->at(cartIdx).raft_airgap_all;


	if (sliceProfile_multi->size() > 1)
	{
		int wipetowerIdx;

		if (sliceProfile_multi->at(0).filament_material.value == "PVA" || sliceProfile_multi->at(0).filament_material.value == "PVA+" || sliceProfile_multi->at(0).filament_material.value == "RZSU") wipetowerIdx = 0;
		else if (sliceProfile_multi->at(1).filament_material.value == "PVA" || sliceProfile_multi->at(1).filament_material.value == "PVA+" || sliceProfile_multi->at(1).filament_material.value == "RZSU") wipetowerIdx = 1;
		else wipetowerIdx = cartIdx;

		if (Profile::machineProfile.machine_expanded_print_mode.value)
		{
			sliceProfile_common->wipe_tower_enabled.value = false;
		}
		else
		{
			sliceProfile_common->wipe_tower_enabled = sliceProfile_multi->at(wipetowerIdx).wipe_tower_enabled;
		}
		
		sliceProfile_common->wipe_tower_position = sliceProfile_multi->at(wipetowerIdx).wipe_tower_position;
		sliceProfile_common->wipe_tower_infill_density = sliceProfile_multi->at(wipetowerIdx).wipe_tower_infill_density;
		sliceProfile_common->wipe_tower_raft_margin = sliceProfile_multi->at(wipetowerIdx).wipe_tower_raft_margin;
		sliceProfile_common->wipe_tower_base_size = sliceProfile_multi->at(wipetowerIdx).wipe_tower_base_size;
		sliceProfile_common->wipe_tower_base_layer_count = sliceProfile_multi->at(wipetowerIdx).wipe_tower_base_layer_count;
		sliceProfile_common->wipe_tower_outer_size = sliceProfile_multi->at(wipetowerIdx).wipe_tower_outer_size;
		sliceProfile_common->wipe_tower_outer_wall_thickness = sliceProfile_multi->at(wipetowerIdx).wipe_tower_outer_wall_thickness;
		sliceProfile_common->wipe_tower_outer_inner_gap = sliceProfile_multi->at(wipetowerIdx).wipe_tower_outer_inner_gap;
		sliceProfile_common->wipe_tower_inner_size = sliceProfile_multi->at(wipetowerIdx).wipe_tower_inner_size;
		sliceProfile_common->wipe_tower_outer_cartridge_index = sliceProfile_multi->at(wipetowerIdx).wipe_tower_outer_cartridge_index;
		sliceProfile_common->wipe_tower_inner_cartridge_index = sliceProfile_multi->at(wipetowerIdx).wipe_tower_inner_cartridge_index;
		sliceProfile_common->wipe_tower_flow = sliceProfile_multi->at(wipetowerIdx).wipe_tower_flow;
		sliceProfile_common->wipe_tower_speed = sliceProfile_multi->at(wipetowerIdx).wipe_tower_speed;

	}
}

//sliceprofile을 common에서 모든 항목을 생성하지 않도록 함..//
//common요소만 따로 생성하도록 함.. --> sliceProfileForCommon::setSliceProfileDataFromCommon() 사용//swyang
//void ProfileControl::setSliceProfileFromCommon(std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common)
//{
//	for (int i = 0; i < sliceProfile_multi->size(); i++)
//	{
//		sliceProfile_multi->at(i).layer_height = sliceProfile_common->layer_height;
//		sliceProfile_multi->at(i).nozzle_size = sliceProfile_common->nozzle_size;
//		sliceProfile_multi->at(i).z_offset_raft = sliceProfile_common->z_offset_raft;
//		sliceProfile_multi->at(i).print_bed_temperature = sliceProfile_common->print_bed_temperature;
//
//		if (Profile::machineProfile.group_model == "5X" || Profile::machineProfile.group_model == "7X")
//		{
//
//			// Side A B 관련 - z_offset_except_raft 낮은 게 우선순위
//			int preferredBedSide = 0;
//			int minZoffset = std::min(sliceProfile_multi->at(0).z_offset_except_raft.value, sliceProfile_multi->at(1).z_offset_except_raft.value);
//			if (minZoffset == sliceProfile_multi->at(1).z_offset_except_raft.value) preferredBedSide = 1;
//
//			sliceProfile_common->z_offset_except_raft = sliceProfile_multi->at(preferredBedSide).z_offset_except_raft;
//		}
//		else sliceProfile_multi->at(i).z_offset_except_raft = sliceProfile_common->z_offset_except_raft;
//
//		sliceProfile_multi->at(i).support_placement = sliceProfile_common->support_placement;
//		sliceProfile_multi->at(i).platform_adhesion = sliceProfile_common->platform_adhesion;
//		sliceProfile_multi->at(i).wipe_tower_enabled = sliceProfile_common->wipe_tower_enabled;
//		sliceProfile_multi->at(i).wipe_tower_position = sliceProfile_common->wipe_tower_position;
//		sliceProfile_multi->at(i).wipe_tower_fill_density = sliceProfile_common->wipe_tower_infill_density;
//		sliceProfile_multi->at(i).wipe_tower_raft_margin = sliceProfile_common->wipe_tower_raft_margin;
//		sliceProfile_multi->at(i).wipe_tower_filament_flow = sliceProfile_common->wipe_tower_filament_flow;
//		sliceProfile_multi->at(i).wipe_tower_base_size = sliceProfile_common->wipe_tower_base_size;
//		sliceProfile_multi->at(i).wipe_tower_base_layer_count = sliceProfile_common->wipe_tower_base_layer_count;
//		sliceProfile_multi->at(i).wipe_tower_outer_size = sliceProfile_common->wipe_tower_outer_size;
//		sliceProfile_multi->at(i).wipe_tower_outer_wall_thickness = sliceProfile_common->wipe_tower_outer_wall_thickness;
//		sliceProfile_multi->at(i).wipe_tower_outer_inner_gap = sliceProfile_common->wipe_tower_outer_inner_gap;
//		sliceProfile_multi->at(i).wipe_tower_inner_size = sliceProfile_common->wipe_tower_inner_size;
//		sliceProfile_multi->at(i).wipe_tower_outer_cartridge_index = sliceProfile_common->wipe_tower_outer_cartridge_index;
//		sliceProfile_multi->at(i).wipe_tower_inner_cartridge_index = sliceProfile_common->wipe_tower_inner_cartridge_index;
//
//		sliceProfile_multi->at(i).ooze_shield = sliceProfile_common->ooze_shield;
//		sliceProfile_multi->at(i).retraction_combing = sliceProfile_common->retraction_combing;
//		/*sliceProfile_multi->at(i).retraction_preretraction_0_distance = sliceProfile_common->retraction_preretraction_0_distance;
//		sliceProfile_multi->at(i).retraction_preretraction_0_amount = sliceProfile_common->retraction_preretraction_0_amount;
//		sliceProfile_multi->at(i).retraction_overmoving_0_distance = sliceProfile_common->retraction_overmoving_0_distance;
//		sliceProfile_multi->at(i).retraction_preretraction_X_distance = sliceProfile_common->retraction_preretraction_X_distance;
//		sliceProfile_multi->at(i).retraction_preretraction_X_amount = sliceProfile_common->retraction_preretraction_X_amount;
//		sliceProfile_multi->at(i).retraction_overmoving_X_distance = sliceProfile_common->retraction_overmoving_X_distance;*/
//		sliceProfile_multi->at(i).toolchange_lowering_bed = sliceProfile_common->toolchange_lowering_bed;
//		sliceProfile_multi->at(i).object_sink = sliceProfile_common->object_sink;
//		sliceProfile_multi->at(i).overlap_dual = sliceProfile_common->overlap_dual;
//		sliceProfile_multi->at(i).pathOptimization_enabled = sliceProfile_common->pathOptimization_enabled;
//		sliceProfile_multi->at(i).support_main_cartridge_index = sliceProfile_common->support_main_cartridge_index;
//		sliceProfile_multi->at(i).support_main_pattern = sliceProfile_common->support_main_pattern;
//		sliceProfile_multi->at(i).support_angle = sliceProfile_common->support_angle;
//		sliceProfile_multi->at(i).support_fill_rate = sliceProfile_common->support_infill_density;
//		sliceProfile_multi->at(i).support_xy_distance = sliceProfile_common->support_xy_distance;
//		sliceProfile_multi->at(i).support_z_distance = sliceProfile_common->support_z_distance;
//		sliceProfile_multi->at(i).support_horizontal_expansion = sliceProfile_common->support_horizontal_expansion;
//		sliceProfile_multi->at(i).support_speed = sliceProfile_common->support_speed;
//		sliceProfile_multi->at(i).spiralize = sliceProfile_common->spiralize;
//		sliceProfile_multi->at(i).simple_mode = sliceProfile_common->simple_mode;
//		sliceProfile_multi->at(i).brim_line_count = sliceProfile_common->brim_line_count;
//		sliceProfile_multi->at(i).skirt_line_count = sliceProfile_common->skirt_line_count;
//		sliceProfile_multi->at(i).skirt_gap = sliceProfile_common->skirt_gap;
//		sliceProfile_multi->at(i).skirt_minimal_length = sliceProfile_common->skirt_minimal_length;
//		sliceProfile_multi->at(i).adhesion_cartridge_index = sliceProfile_common->adhesion_cartridge_index;
//		sliceProfile_multi->at(i).raft_margin = sliceProfile_common->raft_margin;
//		sliceProfile_multi->at(i).raft_line_spacing = sliceProfile_common->raft_line_spacing;
//		sliceProfile_multi->at(i).raft_airgap_initial_layer = sliceProfile_common->raft_airgap_initial_layer;
//		sliceProfile_multi->at(i).raft_base_thickness = sliceProfile_common->raft_base_thickness;
//		sliceProfile_multi->at(i).raft_base_line_width = sliceProfile_common->raft_base_line_width;
//		sliceProfile_multi->at(i).raft_base_speed = sliceProfile_common->raft_base_speed;
//		sliceProfile_multi->at(i).raft_base_temperature = sliceProfile_common->raft_base_temperature;
//		sliceProfile_multi->at(i).raft_interface_thickness = sliceProfile_common->raft_interface_thickness;
//		sliceProfile_multi->at(i).raft_interface_line_width = sliceProfile_common->raft_interface_line_width;
//		sliceProfile_multi->at(i).raft_interface_speed = sliceProfile_common->raft_interface_speed;
//		sliceProfile_multi->at(i).raft_interface_temperature = sliceProfile_common->raft_interface_temperature;
//		sliceProfile_multi->at(i).raft_surface_layers = sliceProfile_common->raft_surface_layers;
//		sliceProfile_multi->at(i).raft_surface_thickness = sliceProfile_common->raft_surface_thickness;
//		sliceProfile_multi->at(i).raft_surface_linewidth = sliceProfile_common->raft_surface_line_width;
//		sliceProfile_multi->at(i).raft_surface_initial_temperature = sliceProfile_common->raft_surface_initial_temperature;
//		sliceProfile_multi->at(i).raft_surface_last_temperature = sliceProfile_common->raft_surface_last_temperature;
//		sliceProfile_multi->at(i).raft_surface_speed = sliceProfile_common->raft_surface_speed;
//		sliceProfile_multi->at(i).raft_inset_enabled = sliceProfile_common->raft_inset_enabled;
//		sliceProfile_multi->at(i).raft_inset_offset = sliceProfile_common->raft_inset_offset;
//		sliceProfile_multi->at(i).raft_temperature_control = sliceProfile_common->raft_temperature_control;
//		sliceProfile_multi->at(i).raft_fan_speed = sliceProfile_common->raft_fan_speed;
//		sliceProfile_multi->at(i).raft_incline_enabled = sliceProfile_common->raft_incline_enabled;
//		sliceProfile_multi->at(i).fix_horrible_union_all_type_a = sliceProfile_common->fix_horrible_union_all_type_a;
//		sliceProfile_multi->at(i).fix_horrible_union_all_type_b = sliceProfile_common->fix_horrible_union_all_type_b;
//		sliceProfile_multi->at(i).fix_horrible_use_open_bits = sliceProfile_common->fix_horrible_use_open_bits;
//		sliceProfile_multi->at(i).fix_horrible_extensive_stitching = sliceProfile_common->fix_horrible_extensive_stitching;
//
//	}
//}

int ProfileControl::getCartIndex_maxWeight(std::vector<SliceProfile>* sliceProfile_multi)
{
	int maxWeight = 0;
	int rtn = 0;
	std::vector<CartridgeData> cartridges = CartridgeInfo::cartridges;
	for (int i = 0; i < cartridges.size(); i++)
	{
		if (cartridges[i].useStateForProfile || cartridges[i].useStateForModel)
		{
			int weight = Generals::getWeightForMaterial(sliceProfile_multi->at(i).filament_material.value);
			if (maxWeight < weight)
			{
				maxWeight = weight;
				rtn = i;
			}
		}
	}
	return rtn;
}

//config[i] converted
void ProfileControl::updateConfigForMultiCartridge()
{
	updateConfigForMultiCartridge(&Profile::sliceProfile, &Profile::sliceProfileCommon, &Profile::configSettings);
}
void ProfileControl::updateConfigForMultiCartridge(std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common, std::vector<ConfigSettings>* config_multi)
{
	ProfileToConfig conv;
	for (int i = 0; i < sliceProfile_multi->size(); i++)
	{
		//sliceProfile[] --> config[]
		conv.convertToConfig(&sliceProfile_multi->at(i), sliceProfile_common, &config_multi->at(i), i);
	}
}

bool ProfileControl::resetProfile()
{
	return resetProfile(&Profile::sliceProfile, &Profile::sliceProfileCommon);
}

bool ProfileControl::resetProfile(std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common)
{
	CustomProfileList customProfile;
	std::vector<Generals::ProfileList> l_profileList;
	l_profileList = customProfile.getProfileList();

	for (int cartIdx = 0; cartIdx < sliceProfile_multi->size(); cartIdx++)
	{
		QString l_profile_name = sliceProfile_multi->at(cartIdx).profile_name.value;
		QString l_material = sliceProfile_multi->at(cartIdx).filament_material.value;
		if (l_profile_name == "" || l_profile_name == Generals::unknownProfileName)//프로파일 네임이 없는 경우
			if (!sliceProfile_multi->at(cartIdx).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_value_" + Generals::getMaterialShortName(l_material) + ".ini")))
				return false;

		int l_profileNameIdx = -1;
		for (int j = 0; j < l_profileList.size(); j++)
		{
			if (l_profileList.at(j).name == sliceProfile_multi->at(cartIdx).profile_name.value)
			{
				l_profileNameIdx = j;
				break;
			}
		}

		if (l_profileNameIdx == -1) //프로파일을 찾지 못한 경우
		{
			if (!sliceProfile_multi->at(cartIdx).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_value_" + Generals::getMaterialShortName(l_material) + ".ini")))
				return false;
		}
		else
		{
			QString l_fileName = l_profileList.at(l_profileNameIdx).fileLocation;
			QString l_profileDir;
			if (l_profileList.at(l_profileNameIdx).custom)
				l_profileDir = Profile::customProfilePath;
			else
				l_profileDir = Profile::profilePath;

			if (!sliceProfile_multi->at(cartIdx).loadSliceProfile(Generals::qstringTowchar_t(l_profileDir + l_fileName)))
			{
				if (!sliceProfile_multi->at(cartIdx).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_value_" + Generals::getMaterialShortName(l_material) + ".ini")))
					return false;
			}
		}

		/*if (Profile::machineProfile.machine_bed_selected_enabled.value == 1 && sliceProfile_multi->at(cartIdx).bed_type.value != 0)
		{

			if (Profile::machineProfile.machine_bed_selected_side.value == 0)
			{
				sliceProfile_multi->at(cartIdx).z_offset_except_raft.value = 0; //side A
				sliceProfile_multi->at(cartIdx).print_bed_temperature.value = 90;
				sliceProfile_multi->at(cartIdx).platform_adhesion.value = 0;
				sliceProfile_multi->at(cartIdx).layer0_width_factor.value = 100;
			}
			else
			{
				sliceProfile_multi->at(cartIdx).z_offset_except_raft.value = -0.15;
				sliceProfile_multi->at(cartIdx).print_bed_temperature.value = 100;
				sliceProfile_multi->at(cartIdx).platform_adhesion.value = 2;
				sliceProfile_multi->at(cartIdx).layer0_width_factor.value = 200;
			}
		}*/
	}
	generateCommonProfileFromSliceProfile(sliceProfile_multi, sliceProfile_common, -1, 0, -1, 0);

	//config도 같이 업데이트 해줘야 checkvolume range가 update 됨..//
	//afterProfileChanged에서 동작하도록 함.
	//updateConfigForMultiCartridge(sliceProfile_multi, sliceProfile_common, config_multi);
	return true;
}

bool ProfileControl::importProfile(QString p_importProfileName, int p_profileSelectedIndex)
{
	return importProfile(p_importProfileName, p_profileSelectedIndex, &Profile::sliceProfile, &Profile::sliceProfileCommon);
}
bool ProfileControl::importProfile(QString p_importProfileName, int p_profileSelectedIndex, std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common)
{
	int pastSupportCartIndex, pastAdhesionCartIndex;
	if (sliceProfile_multi->size() != 1)
	{
		pastSupportCartIndex = sliceProfile_common->support_main_cartridge_index.value;
		pastAdhesionCartIndex = sliceProfile_common->adhesion_cartridge_index.value;
	}
	else
	{
		pastSupportCartIndex = 0;
		pastAdhesionCartIndex = 0;
	}

	//finding material in imported profile//
	SliceProfile temp_imported_slice_profile;
	QString imported_material;
	if (temp_imported_slice_profile.loadSliceProfile(Generals::qstringTowchar_t(p_importProfileName)))
	{
		imported_material = temp_imported_slice_profile.filament_material.value;
	}
	else
		return false;

	//if material information is not in imported profile, return false;..//
	if (imported_material.isEmpty())
		return false;


	SliceProfile imported_slice_profile;
	imported_slice_profile.flag_imported_profile = true;
	//load default slice profile by imported material//
	if (!imported_slice_profile.loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_value_" + Generals::getMaterialShortName(imported_material) + ".ini")))
		return false;

	//load imported path profile//
	if (!imported_slice_profile.loadSliceProfile(Generals::qstringTowchar_t(p_importProfileName)))
		return false;

	if (!imported_slice_profile.loadTemperatureLayerList(Generals::qstringTowchar_t(p_importProfileName)))
		return false;


	//////////////////////////////////////////////////////////////////
	//validation imported profile compared with current machine..//

	//if material of imported profile is not applied in a current machine, return false;..//
	//importedSliceProfile.filament_material
	bool check_appliedMaterial = false;
	for (int i = 0; i < Profile::machineProfile.available_material_list.size(); ++i)
	{
		if (Profile::machineProfile.available_material_list.at(i) == imported_material)
			check_appliedMaterial = true;
	}

	if (!check_appliedMaterial)
		return false;
	

	//////////////////////////////////////////////////////////////////

	sliceProfile_multi->at(p_profileSelectedIndex) = imported_slice_profile;

	//common 추출
	//1nozzle인 경우 첫번째 sliceProfile에서 common을 추출해서 사용
	//multi nozzle인 경우 여러개의 프로파일 값을 조합하여 추출
	if (sliceProfile_multi->size() == 1)
		sliceProfile_common->getCommonProfileFromSliceProfile(&sliceProfile_multi->at(0));
	else
		generateCommonProfileFromSliceProfile(sliceProfile_multi, sliceProfile_common);

	sliceProfile_common->support_main_cartridge_index.value = pastSupportCartIndex;
	sliceProfile_common->adhesion_cartridge_index.value = pastAdhesionCartIndex;

	//afterProfileChanged에서 동작하도록 함.
	//updateConfigForMultiCartridge(sliceProfile_multi, sliceProfile_common, config_multi);

	//reset flag//
	sliceProfile_multi->at(p_profileSelectedIndex).flag_imported_profile = false;

	return true;
}

bool ProfileControl::exportProfile(QString p_exportProfileName, int p_profileSelectedIndex)
{
	return exportProfile(p_exportProfileName, p_profileSelectedIndex, Profile::sliceProfile, Profile::sliceProfileCommon);
}

bool ProfileControl::exportProfile(QString p_exportProfileName, int p_profileSelectedIndex, std::vector<SliceProfile> sliceProfile_multi, SliceProfileForCommon sliceProfile_common)
{
	QFileInfo fileInfo = QFileInfo(p_exportProfileName);
	if (sliceProfile_multi.size() == 1)
		return exportProfile(p_exportProfileName, sliceProfile_multi.front(), sliceProfile_common);
	else if (p_profileSelectedIndex == sliceProfile_multi.size())//'ALL' case//
	{
		for (int i = 0; i < sliceProfile_multi.size(); i++)
		{
			QString filename = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + "_" + QString::number(i + 1) + "." + fileInfo.suffix();
			if (!exportProfile(filename, sliceProfile_multi.at(i), sliceProfile_common))
				return false;
		}
		return true;
	}
	else//profile을 하나만 선택했을 때..//
		return exportProfile(p_exportProfileName, sliceProfile_multi.at(p_profileSelectedIndex), sliceProfile_common);
}

bool ProfileControl::exportProfile(QString p_exportProfileName, SliceProfile sliceProfile_multi, SliceProfileForCommon sliceProfile_common)
{
	bool rtn = true;
	sliceProfile_common.setSliceProfileDataFromCommon(&sliceProfile_multi);
	sliceProfile_multi.saveSliceProfile(Generals::qstringTowchar_t(p_exportProfileName));

	if (sliceProfile_multi.temperature_layer_setting_enabled.value)
		rtn = sliceProfile_multi.saveTemperatureLayerList(Generals::qstringTowchar_t(p_exportProfileName));
	return rtn;
}

bool ProfileControl::resetEasyProfile()
{
	return resetEasyProfile(&Profile::sliceProfile, &Profile::sliceProfileCommon);
}

bool ProfileControl::resetEasyProfile(std::vector<SliceProfile>* sliceProfile_multi, SliceProfileForCommon* sliceProfile_common)
{
	for (int i = 0; i < sliceProfile_multi->size(); i++)
	{
		//profile안에서 material string을 가져옴..//
		QString l_material = sliceProfile_multi->at(i).filament_material.value;

		if (l_material == "ETC" || l_material == "") 
			l_material = "PLA"; //ETC에서 간편모드일 때 gram 표시

		//normal mode에 material을 맞추어서 resetting함..//
		if (!sliceProfile_multi->at(i).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_normal_" + Generals::getMaterialShortName(l_material) + ".ini")))
		{
			sliceProfile_multi->at(i).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_normal_PLA.ini"));
			
			//기본 PLA로 불러와도 common profile은 만들어져야 함..//
			generateCommonProfileFromSliceProfile(sliceProfile_multi, sliceProfile_common);

			return false;
		}
	}

	generateCommonProfileFromSliceProfile(sliceProfile_multi, sliceProfile_common);

	//afterProfileChanged에서 동작하도록 함.
	//updateConfigForMultiCartridge(sliceProfile_multi, sliceProfile_common, config_multi);
	return true;
}

bool ProfileControl::loadRecentProfile()
{
	return loadRecentProfile(&Profile::sliceProfile, &Profile::sliceProfileCommon, &Profile::machineProfile);
}
bool ProfileControl::loadRecentProfile(std::vector<SliceProfile>* _sliceProfile_multi, SliceProfileForCommon* _sliceProfile_common, MachineProfile* _machine_profile)
{
	for (int i = 0; i < _sliceProfile_multi->size(); i++)
	{

		//loading recent slice profile..//
		if (!_sliceProfile_multi->at(i).loadSliceProfile(Generals::qstringTowchar_t(Profile::recentProfilePath + QString("recent_profile_setting_value_%1.ini").arg(i + 1))))
		{
			if (!_sliceProfile_multi->at(i).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + QString("profile_setting_value_PLA.ini"))))
				return false;
		}


		//loading temperature layer ..//
		if (!_sliceProfile_multi->at(i).loadTemperatureLayerList(Generals::qstringTowchar_t(Profile::recentProfilePath + QString("recent_profile_setting_value_%1.ini").arg(i + 1))))
		{
			if (!_sliceProfile_multi->at(i).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + QString("profile_setting_value_PLA.ini"))))
				return false;
		}


	}


	//loading recent common profile..//
	if (!_sliceProfile_common->loadSliceProfileCommon(Generals::qstringTowchar_t(Profile::recentProfilePath + QString("recent_profile_setting_common_value.ini"))))
	{
		generateCommonProfileFromSliceProfile(_sliceProfile_multi, 
			_sliceProfile_common, 
			_sliceProfile_multi->at(0).platform_adhesion.value, 
			_sliceProfile_multi->at(0).adhesion_cartridge_index.value, 
			_sliceProfile_multi->at(0).support_placement.value, 
			_sliceProfile_multi->at(0).support_main_cartridge_index.value);
	}
	else
	{
		//expanded print mode에 따른 추가 처리
		if (Profile::machineProfile.machine_expanded_print_mode.value)
		{
			//support, adhesion cartridge index를 모두 expanded cartridge index로 변환..//
			_sliceProfile_common->support_main_cartridge_index.value = Profile::machineProfile.machine_expanded_print_cartridgeIndex.value;
			_sliceProfile_common->adhesion_cartridge_index.value = Profile::machineProfile.machine_expanded_print_cartridgeIndex.value;

			//wipe tower disable//
			_sliceProfile_common->wipe_tower_enabled.value = false;
		}
	}


	//loading recent machine profile..for nozzle size//
	MachineProfile temp_machine_profile;
	if (!temp_machine_profile.loadRecentMachineProfile(Generals::qstringTowchar_t(Profile::recentProfilePath + QString("recent_machine_setting_value.ini"))))
	{
		if (!temp_machine_profile.loadMachineProfile(Generals::qstringTowchar_t(Profile::profilePath + QString("machine_setting_value.ini"))))
		{
			return false;
		}
	}
	_machine_profile->nozzle_size = temp_machine_profile.nozzle_size;

	//recent_profile_setting_common_value 에 cartridge 값이 -1로 들어갈 경우 대비 수정
	if (_sliceProfile_common->adhesion_cartridge_index.value < 0 ||
		_sliceProfile_common->adhesion_cartridge_index.value >= _sliceProfile_multi->size())
		_sliceProfile_common->adhesion_cartridge_index.value = 0;
	if (_sliceProfile_common->support_main_cartridge_index.value < 0 ||
		_sliceProfile_common->support_main_cartridge_index.value >= _sliceProfile_multi->size())
		_sliceProfile_common->support_main_cartridge_index.value = 0;
	//afterProfileChanged에서 동작하도록 함.
	//updateConfigForMultiCartridge(sliceProfile_multi, sliceProfile_common, config_multi);
	return true;
}

void ProfileControl::saveRecentProfile(QString settingMode)
{
	saveRecentProfile(settingMode, Profile::sliceProfile, Profile::sliceProfileCommon);
}
void ProfileControl::saveRecentProfile(QString _settingMode, std::vector<SliceProfile> _sliceProfile_multi, SliceProfileForCommon _sliceProfile_common)
{
	if (_settingMode == "easy") //recent파일은 advanced 모드일때만 저장
		return;

	Generals::checkPath(Profile::recentProfilePath);

	//saving recent profile//
	for (int i = 0; i < _sliceProfile_multi.size(); i++)
	{
		QString recentPath = QString("recent_profile_setting_value_%1.ini").arg(i + 1);
		_sliceProfile_multi.at(i).saveSliceProfile(Generals::qstringTowchar_t(Profile::recentProfilePath + recentPath));
		_sliceProfile_multi.at(i).saveTemperatureLayerList(Generals::qstringTowchar_t(Profile::recentProfilePath + recentPath));
	}

	//saving recent common profile//
	QString recentCommonPath = "recent_profile_setting_common_value.ini";
	_sliceProfile_common.saveSliceProfileCommon(Generals::qstringTowchar_t(Profile::recentProfilePath + recentCommonPath));

	
	//recent machine profile은 따로 분리함 --> model changing 되는 시점 고려..//
}
void ProfileControl::saveRecentMachineProfile(MachineProfile _machineProfile)
{
	Generals::checkPath(Profile::recentProfilePath);

	_machineProfile.saveRecentMachineProfile(Generals::qstringTowchar_t(Profile::recentProfilePath + QString("recent_machine_setting_value.ini")));
}

ProfileDataI ProfileControl::getMaxBedTemperature(std::vector<SliceProfile> tempSliceProfile)
{
	ProfileDataI v_bed_temp = ProfileDataI("print_bed_temperature", "C");

	for (int i = 0; i < tempSliceProfile.size(); i++)
	{
		if (v_bed_temp.value < tempSliceProfile.at(i).print_bed_temperature.value)
		{
			v_bed_temp.value = tempSliceProfile.at(i).print_bed_temperature.value;
			v_bed_temp.min = tempSliceProfile.at(i).print_bed_temperature.min;
			v_bed_temp.max = tempSliceProfile.at(i).print_bed_temperature.max;
		}
	}
	return v_bed_temp;
}

bool ProfileControl::checkSupportProfileChanged(ConfigSettings beforeConfig_, ConfigSettings currentConfig_)
{
	if (currentConfig_.support_main_pattern != beforeConfig_.support_main_pattern
		|| currentConfig_.support_everywhere != beforeConfig_.support_everywhere
		|| currentConfig_.support_angle != beforeConfig_.support_angle
		|| currentConfig_.support_main_cartridge_index != beforeConfig_.support_main_cartridge_index
		|| currentConfig_.support_line_distance != beforeConfig_.support_line_distance
		|| currentConfig_.support_main_pattern != beforeConfig_.support_main_pattern
		|| currentConfig_.support_xy_distance != beforeConfig_.support_xy_distance
		|| currentConfig_.support_z_distance != beforeConfig_.support_z_distance
		|| currentConfig_.support_horizontal_expansion != beforeConfig_.support_horizontal_expansion)
		return true;
	else
		return false;
}