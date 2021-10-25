#pragma once
#include "ProfileData.h"
#include "SliceProfile.h"

class ProfileCommonMapValue;

class SliceProfileForCommon
{
public:
	SliceProfileForCommon();
	~SliceProfileForCommon();
	
	void getCommonProfileFromSliceProfile(SliceProfile*);
	void setSliceProfileDataFromCommon(SliceProfile*);

	bool loadSliceProfileCommon(const wchar_t*);
	void saveSliceProfileCommon(const wchar_t* filepath);

	void setProfileData(QString* data, std::string value);
	void setProfileData(ProfileDataD* data, std::string value);
	void setProfileData(ProfileDataB* data, std::string value);
	void setProfileData(ProfileDataI* data, std::string value);
	void setProfileData(ProfileDataS* data, std::string value);

	void getProfileValue(std::string fromValue, double* value, double* min = nullptr, double* max = nullptr);
	void getProfileValue(std::string fromValue, bool* value);
	void getProfileValue(std::string fromValue, int* value, int* min = nullptr, int* max = nullptr);
	void getProfileValue(std::string fromValue, QString* value, QString* min = nullptr, QString* max = nullptr);

	void setProfileData(ProfileDataD* data, ProfileDataD value);
	void setProfileData(ProfileDataB* data, ProfileDataB value);
	void setProfileData(ProfileDataI* data, ProfileDataI value);

	std::map<std::string, ProfileCommonMapValue*> setSliceProfileForCommonMap();

private:
	std::map<std::string, ProfileCommonMapValue*> slice_profile_common_map;

	//config value
public:
	//MultiCartridge일때 첫번째 항목 사용	
	ProfileDataD	layer_height = ProfileDataD("layer_height", "mm");
	ProfileDataD	initial_layer_height = ProfileDataD("initial_layer_height", "mm");
	ProfileDataI	initial_layer_width_factor = ProfileDataI("initial_layer_width_factor", "%");
	ProfileDataD	z_offset_raft = ProfileDataD("z_offset_raft", "mm");
	ProfileDataD	z_offset_except_raft = ProfileDataD("z_offset_except_raft", "mm");

	//ProfileDataD	nozzle_size = ProfileDataD("nozzle_size", "mm");
	ProfileDataI	print_bed_temperature = ProfileDataI("print_bed_temperature", "C");

	//support//
	ProfileDataI	support_main_cartridge_index = ProfileDataI("support_main_cartridge_index", "");
	ProfileDataI	support_placement = ProfileDataI("support_placement", "0=None,1=Touching_buildplate,2=Everywhere");
	ProfileDataI	support_main_pattern = ProfileDataI("support_main_pattern", "0=Grid,1=Line,2=Zigzag");				// 0=Grid, 1=Line
	ProfileDataD	support_angle = ProfileDataD("support_angle", "deg");
	ProfileDataI	support_infill_density = ProfileDataI("support_infill_density", "%");
	ProfileDataD	support_xy_distance = ProfileDataD("support_xy_distance", "mm");
	ProfileDataD	support_z_distance = ProfileDataD("support_z_distance", "mm");
	ProfileDataI	support_main_flow = ProfileDataI("support_main_flow", "%");
	ProfileDataD	support_horizontal_expansion = ProfileDataD("support_horizontal_expansion", "mm");
	ProfileDataD	support_main_speed = ProfileDataD("support_main_speed", "mm/s");
	ProfileDataI	support_main_width_factor = ProfileDataI("support_main_width_factor", "%");

	ProfileDataB	support_interface_enabled = ProfileDataB("support_interface_enabled", "0=False,1=True");
	ProfileDataI	support_interface_pattern = ProfileDataI("support_interface_pattern", "0=Lines,1=Concentric");
	ProfileDataI	support_interface_roof_layers_count = ProfileDataI("support_interface_roof_layers_count", "");
	ProfileDataI	support_interface_roof_width_factor = ProfileDataI("support_interface_roof_width_factor", "%");
	ProfileDataI	support_interface_roof_flow = ProfileDataI("support_interface_roof_flow", "%");
	ProfileDataD	support_interface_roof_speed = ProfileDataD("support_interface_roof_speed", "mm/s");
	ProfileDataI	support_interface_floor_layers_count = ProfileDataI("support_interface_floor_layers_count", "");
	ProfileDataI	support_interface_floor_width_factor = ProfileDataI("support_interface_floor_width_factor", "%");
	ProfileDataI	support_interface_floor_flow = ProfileDataI("support_interface_floor_flow", "%");
	ProfileDataD	support_interface_floor_speed = ProfileDataD("support_interface_floor_speed", "mm/s");


	//adhesion settings//
	ProfileDataI	platform_adhesion = ProfileDataI("platform_adhesion", "0=None,1=Skirt,2=Brim,3=Raft");			// 0=None, 1=Brim', 2=Raft
	ProfileDataI	adhesion_cartridge_index = ProfileDataI("adhesion_cartridge_index", "");
	ProfileDataI	skirt_line_count = ProfileDataI("skirt_line_count", "");
	//ProfileDataI	skirt_width_factor = ProfileDataI("skirt_width_factor", "%");
	ProfileDataD	skirt_gap = ProfileDataD("skirt_gap", "mm");
	ProfileDataD	skirt_minimal_length = ProfileDataD("skirt_minimal_length", "mm");
	//ProfileDataD	skirt_speed = ProfileDataD("skirt_speed", "mm/s");
	//ProfileDataI	skirt_flow = ProfileDataI("skirt_flow", "%");
	ProfileDataI	brim_line_count = ProfileDataI("brim_line_count", "");
	//ProfileDataI	brim_width_factor = ProfileDataI("brim_width_factor", "%");
	//ProfileDataD	brim_speed = ProfileDataD("brim_speed", "mm/s");
	//ProfileDataI	brim_flow = ProfileDataI("brim_flow", "%");

	ProfileDataD	raft_margin = ProfileDataD("raft_margin", "mm");
	ProfileDataD	raft_line_spacing = ProfileDataD("raft_line_spacing", "mm");
	ProfileDataD	raft_airgap_all = ProfileDataD("raft_airgap_all", "mm");
	ProfileDataD	raft_airgap_initial_layer = ProfileDataD("raft_airgap_initial_layer", "mm");
	ProfileDataB	raft_inset_enabled = ProfileDataB("raft_inset_enabled", "0=Disabled,1=Enabled");					// bool, 0=off, 1=on, new
	ProfileDataD	raft_inset_offset = ProfileDataD("raft_inset_offset", "0mm~1mm");					// double value, new
	ProfileDataB	raft_temperature_control = ProfileDataB("raft_temperature_control", "0=Disabled,1=Enabled");			// bool,  0=disable, 1=enable, new
	//ProfileDataD	raft_fan_speed = ProfileDataD("raft_fan_speed", "mm/s");
	ProfileDataB	raft_incline_enabled = ProfileDataB("raft_incline_enabled", "0=Disabled,1=Enabled");
	ProfileDataD	raft_base_thickness = ProfileDataD("raft_base_thickness", "mm");
	ProfileDataD	raft_base_line_width = ProfileDataD("raft_base_line_width", "mm");
	ProfileDataD	raft_base_speed = ProfileDataD("raft_base_speed", "mm/s");
	ProfileDataI	raft_base_temperature = ProfileDataI("raft_base_temperature", "C");
	ProfileDataD	raft_interface_thickness = ProfileDataD("raft_interface_thickness", "mm");
	ProfileDataD	raft_interface_line_width = ProfileDataD("raft_interface_line_width", "mm");
	ProfileDataD	raft_interface_speed = ProfileDataD("raft_interface_speed", "mm/s");
	ProfileDataI	raft_interface_temperature = ProfileDataI("raft_interface_temperature", "C");
	ProfileDataI	raft_surface_layers = ProfileDataI("raft_surface_layers", "mm");
	ProfileDataD	raft_surface_thickness = ProfileDataD("raft_surface_thickness", "mm");
	ProfileDataD	raft_surface_line_width = ProfileDataD("raft_surface_line_width", "mm");
	ProfileDataD	raft_surface_speed = ProfileDataD("raft_surface_speed", "mm/s");
	ProfileDataI	raft_surface_initial_temperature = ProfileDataI("raft_surface_initial_temperature", "C");
	ProfileDataI	raft_surface_last_temperature = ProfileDataI("raft_surface_last_temperature", "C");

	ProfileDataI	retraction_combing = ProfileDataI("retraction_combing", "0=Off,1=All,2=No_Skin");			// 0=off, 1=All, 2=No Skin

	ProfileDataD	toolchange_lowering_bed = ProfileDataD("toolchange_lowering_bed", "mm");


	//wipe tower//
	ProfileDataB	wipe_tower_enabled = ProfileDataB("wipe_tower_enabled", "0=False,1=True");
	ProfileDataI	wipe_tower_position = ProfileDataI("wipe_tower_position", "");
	ProfileDataI	wipe_tower_infill_density = ProfileDataI("wipe_tower_infill_density", "%");
	ProfileDataD	wipe_tower_raft_margin = ProfileDataD("wipe_tower_raft_margin", "mm");
	ProfileDataI	wipe_tower_outer_cartridge_index;
	ProfileDataI	wipe_tower_inner_cartridge_index;
	ProfileDataD	wipe_tower_base_size = ProfileDataD("wipe_tower_base_size", "mm");
	ProfileDataI	wipe_tower_base_layer_count = ProfileDataI("wipe_tower_base_layer_count", "");
	ProfileDataD	wipe_tower_outer_size = ProfileDataD("wipe_tower_outer_size", "mm");
	ProfileDataD	wipe_tower_outer_wall_thickness = ProfileDataD("wipe_tower_outer_wall_thickness", "mm");
	ProfileDataD	wipe_tower_outer_inner_gap = ProfileDataD("wipe_tower_outer_inner_gap", "mm");
	ProfileDataD	wipe_tower_inner_size = ProfileDataD("wipe_tower_inner_size", "mm");
	ProfileDataI	wipe_tower_flow = ProfileDataI("wipe_tower_flow", "%");
	ProfileDataD	wipe_tower_speed = ProfileDataD("wipe_tower_speed", "mm/s");


	//shape error correction//
	ProfileDataB	spiralize = ProfileDataB("spiralize", "0=False,1=True");
	ProfileDataB	simple_mode = ProfileDataB("simple_mode", "0=False,1=True");
	ProfileDataB	fix_horrible_union_all_type_a = ProfileDataB("fix_horrible_union_all_type_a", "0=False,1=True");
	ProfileDataB	fix_horrible_union_all_type_b = ProfileDataB("fix_horrible_union_all_type_b", "0=False,1=True");
	ProfileDataB	fix_horrible_use_open_bits = ProfileDataB("fix_horrible_use_open_bits", "0=False,1=True");
	ProfileDataB	fix_horrible_extensive_stitching = ProfileDataB("fix_horrible_extensive_stitching", "0=False,1=True");

	
	ProfileDataD	overlap_dual = ProfileDataD("overlap_dual", "mm");
	ProfileDataD	object_sink = ProfileDataD("object_sink", "mm");
	//ProfileDataB	ooze_shield;
	//ProfileDataB	pathOptimization_enabled = ProfileDataB("pathOptimization_enabled", "0=False,1=True");
};



class ProfileCommonMapValue
{
private:
	ProfileDataD* profile_data_D;
	ProfileDataB* profile_data_B;
	ProfileDataI* profile_data_I;
	ProfileDataS* profile_data_S;
	int profile_type;

	SliceProfileForCommon* slice_profile_common;

public:
	ProfileCommonMapValue(SliceProfileForCommon* _slice_profile_common, ProfileDataD* _profile_data_D)
		: profile_data_D(_profile_data_D),
		slice_profile_common(_slice_profile_common),
		profile_type(ProfileDataType::_double) { };

	ProfileCommonMapValue(SliceProfileForCommon* _slice_profile_common, ProfileDataI* _profile_data_I)
		: profile_data_I(_profile_data_I),
		slice_profile_common(_slice_profile_common),
		profile_type(ProfileDataType::_int) { };

	ProfileCommonMapValue(SliceProfileForCommon* _slice_profile_common, ProfileDataB* _profile_data_B)
		: profile_data_B(_profile_data_B),
		slice_profile_common(_slice_profile_common),
		profile_type(ProfileDataType::_bool) { };

	ProfileCommonMapValue(SliceProfileForCommon* _slice_profile_common, ProfileDataS* _profile_data_S)
		: profile_data_S(_profile_data_S),
		slice_profile_common(_slice_profile_common),
		profile_type(ProfileDataType::_QString) { };

	void setProfileDataForProfileCommonMap(std::string _profile_value)
	{
		switch (profile_type)
		{
		case ProfileDataType::_double:
			slice_profile_common->setProfileData(profile_data_D, _profile_value);
			break;
		case ProfileDataType::_int:
			slice_profile_common->setProfileData(profile_data_I, _profile_value);
			break;
		case ProfileDataType::_bool:
			slice_profile_common->setProfileData(profile_data_B, _profile_value);
			break;
		case ProfileDataType::_QString:
			slice_profile_common->setProfileData(profile_data_S, _profile_value);
			break;
		}
	}

	std::string getProfileDataNameKey()
	{
		std::string name_key;

		switch (profile_type)
		{
		case ProfileDataType::_double:
			name_key = profile_data_D->name_key;
			break;
		case ProfileDataType::_int:
			name_key = profile_data_I->name_key;
			break;
		case ProfileDataType::_bool:
			name_key = profile_data_B->name_key;
			break;
		case ProfileDataType::_QString:
			name_key = profile_data_S->name_key;
			break;
		}

		return name_key;
	}

	std::string getProfileDataUnitKey()
	{
		std::string unit_key;

		switch (profile_type)
		{
		case ProfileDataType::_double:
			unit_key = profile_data_D->unit_key;
			break;
		case ProfileDataType::_int:
			unit_key = profile_data_I->unit_key;
			break;
		case ProfileDataType::_bool:
			unit_key = profile_data_B->unit_key;
			break;
		case ProfileDataType::_QString:
			unit_key = profile_data_S->unit_key;
			break;
		}

		return unit_key;
	}
};

