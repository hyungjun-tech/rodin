#pragma once
#include "ProfileData.h"
#include <QString>
#include <vector>

class ProfileMapValue;

class TemperatureLayerSetPoint
{
public:
	int layerNr;
	int temperature;

public:
	TemperatureLayerSetPoint(int, int);
	TemperatureLayerSetPoint();
	~TemperatureLayerSetPoint();
};

class SliceProfile
{
public:
	SliceProfile();
	~SliceProfile();

	bool loadSliceProfile(const wchar_t* _filePath); //new
	void saveSliceProfile(const wchar_t* _filePath); //new

	std::string getCurrentProfileOutput();

	bool loadTemperatureLayerList(const wchar_t*);
	bool saveTemperatureLayerList(const wchar_t*);
	void clearTemperatureLayerListAll();
	void rearrangeTemperatureLayerList();

public:
	//void setProfileData(QString* data, std::string value);
	void setProfileData(ProfileDataD* data, std::string value);
	void setProfileData(ProfileDataB* data, std::string value);
	void setProfileData(ProfileDataI* data, std::string value);
	void setProfileData(ProfileDataS* data, std::string value);



	//template <typename T>
	//void setProfileData(ProfileData<T>* _data, std::string _value)
	//{
	//	if (std::is_same<T, QString>::value)
	//	{
	//		_data->value = QString::fromStdString(_value).trimmed();
	//	}
	//	else
	//	{
	//		getProfileValue(_value, &_data->value, &_data->min, &_data->max);
	//	}
	//}

	//void getProfileValue(std::string fromValue, bool* value);
	//void getProfileValue(std::string fromValue, int* value, int* min = nullptr, int* max = nullptr);
	//void getProfileValue(std::string fromValue, QString* value, QString* min = nullptr, QString* max = nullptr);

	//template <typename T>
	//void getProfileValue(std::string _fromValue, T* _value, T* _min = nullptr, T* _max = nullptr)
	//{
	//	if (std::is_same<T, double>::value)
	//	{
	//		QString l_value, l_min, l_max;
	//		getProfileValue(fromValue, &l_value, &l_min, &l_max);

	//		if (l_value != "") *value = QString(l_value).toDouble();
	//		if (l_min != "") *min = QString(l_min).toDouble();
	//		if (l_max != "") *max = QString(l_max).toDouble();
	//	}
	//	else if (std::is_same<T, int>::value)
	//	{
	//		QString l_value, l_min, l_max;
	//		getProfileValue(fromValue, &l_value, &l_min, &l_max);

	//		if (l_value != "") *value = QString(l_value).toInt();
	//		if (l_min != "") *min = QString(l_min).toInt();
	//		if (l_max != "") *max = QString(l_max).toInt();
	//	}
	//	else if (std::is_same<T, bool>::value)
	//	{
	//		QString l_value;
	//		getProfileValue(fromValue, &l_value);

	//		if (l_value != "") *value = (bool)QString(l_value).toInt();
	//	}

	//}
	   	  	
	void getProfileValue(std::string fromValue, double* value, double* min = nullptr, double* max = nullptr);
	void getProfileValue(std::string fromValue, bool* value);
	void getProfileValue(std::string fromValue, int* value, int* min = nullptr, int* max = nullptr);
	void getProfileValue(std::string fromValue, QString* value, QString* min = nullptr, QString* max = nullptr);
	//void SetSliceProfileDataFromCartridge(SliceProfileForCartridge*);
	//void SetSliceProfileDataFromCommon(SliceProfileForCommon*);

	std::map<std::string, ProfileMapValue*> setSliceProfileMap();
	std::map<std::string, ProfileMapValue*> setSliceProfileMap_old();

	bool replaceProfileDataValue(ProfileDataI*  _profile_data_old, ProfileDataI* _profile_data_new);
	bool replaceProfileDataValue(ProfileDataD*  _profile_data_old, ProfileDataD* _profile_data_new);


public:
	std::map<std::string, ProfileMapValue*> slice_profile_map;
	std::map<std::string, ProfileMapValue*> slice_profile_map_old;
	

public:
	//ProfileDataS	profile_version = ProfileDataS("profile_version", "");
	ProfileDataS	profile_name = ProfileDataS("profile_name", "string");
	ProfileDataS	filament_material = ProfileDataS("filament_material", "string");
	ProfileDataD	filament_diameter = ProfileDataD("filament_diameter", "mm");

	//quality//
	ProfileDataD	layer_height = ProfileDataD("layer_height", "mm");
	ProfileDataD	initial_layer_height = ProfileDataD("initial_layer_height", "mm");
	ProfileDataI	initial_layer_width_factor = ProfileDataI("initial_layer_width_factor", "%");
	ProfileDataI	wall_width_factor = ProfileDataI("wall_width_factor", "%");
	ProfileDataI	infill_width_factor = ProfileDataI("infill_width_factor", "%");
	ProfileDataI	top_bottom_width_factor = ProfileDataI("top_bottom_width_factor", "%");
	ProfileDataD	z_offset_raft = ProfileDataD("z_offset_raft", "mm");
	ProfileDataD	z_offset_except_raft = ProfileDataD("z_offset_except_raft", "mm");
	
	//shell//
	ProfileDataD	wall_thickness = ProfileDataD("wall_thickness", "mm");
	ProfileDataD	top_bottom_thickness = ProfileDataD("top_bottom_thickness", "mm");
	ProfileDataB	solid_top = ProfileDataB("solid_top", "0=False,1=True");
	ProfileDataB	solid_bottom = ProfileDataB("solid_bottom", "0=False,1=True");
	ProfileDataI	wall_printing_direction = ProfileDataI("wall_printing_direction", "0=OutsideToInside,1=InsideToOutside");
	
	//temperature//
	ProfileDataI	print_temperature = ProfileDataI("print_temperature", "C");
	ProfileDataI	print_bed_temperature = ProfileDataI("print_bed_temperature", "C");
	ProfileDataB	temperature_layer_setting_enabled = ProfileDataB("temperature_layer_setting_enabled", "0=Disable,1=Enable");

	//temperature layer//
	std::vector<TemperatureLayerSetPoint> temperature_layer_list;
	ProfileDataI	temperature_setpoint_layer_number;

	//flow//
	ProfileDataB	overall_flow_control_enabled = ProfileDataB("overall_flow_control_enabled", "0=False,1=True");
	ProfileDataI	overall_flow = ProfileDataI("overall_flow", "%");
	ProfileDataI	initial_layer_flow = ProfileDataI("initial_layer_flow", "%");
	ProfileDataI	infill_flow	= ProfileDataI("infill_flow", "%");
	ProfileDataI	outer_wall_flow = ProfileDataI("outer_wall_flow", "%");
	ProfileDataI	inner_wall_flow = ProfileDataI("inner_wall_flow", "%");
	ProfileDataI	top_bottom_flow = ProfileDataI("top_bottom_flow", "%");

	//support//
	ProfileDataI	support_main_cartridge_index = ProfileDataI("support_main_cartridge_index", "index");
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
	ProfileDataI	support_interface_roof_layers_count = ProfileDataI("support_interface_roof_layers_count", "EA");
	ProfileDataI	support_interface_roof_width_factor = ProfileDataI("support_interface_roof_width_factor", "%");
	ProfileDataI	support_interface_roof_flow = ProfileDataI("support_interface_roof_flow", "%");
	ProfileDataD	support_interface_roof_speed = ProfileDataD("support_interface_roof_speed", "mm/s");
	ProfileDataI	support_interface_floor_layers_count = ProfileDataI("support_interface_floor_layers_count", "EA");
	ProfileDataI	support_interface_floor_width_factor = ProfileDataI("support_interface_floor_width_factor", "%");
	ProfileDataI	support_interface_floor_flow = ProfileDataI("support_interface_floor_flow", "%");
	ProfileDataD	support_interface_floor_speed = ProfileDataD("support_interface_floor_speed", "mm/s");

	//adhesion settings//
	ProfileDataI	platform_adhesion = ProfileDataI("platform_adhesion", "0=None,1=Skirt,2=Brim,3=Raft");			// 0=None, 1=Brim', 2=Raft
	ProfileDataI	adhesion_cartridge_index = ProfileDataI("adhesion_cartridge_index", "index");
	ProfileDataI	skirt_line_count = ProfileDataI("skirt_line_count", "EA");
	//ProfileDataI	skirt_width_factor = ProfileDataI("skirt_width_factor", "%");
	ProfileDataD	skirt_gap = ProfileDataD("skirt_gap", "mm");
	ProfileDataD	skirt_minimal_length = ProfileDataD("skirt_minimal_length", "mm");
	//ProfileDataD	skirt_speed = ProfileDataD("skirt_speed", "mm/s");
	//ProfileDataI	skirt_flow = ProfileDataI("skirt_flow", "%");
	ProfileDataI	brim_line_count = ProfileDataI("brim_line_count", "EA");
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

	//infill//
	ProfileDataI	infill_density = ProfileDataI("infill_density", "%");
	ProfileDataI	infill_overlap = ProfileDataI("infill_overlap", "%");
	ProfileDataI	infill_pattern = ProfileDataI("infill_pattern", "0=Automatic,1=Grid,2=Lines,3=Concentric,4=Crystal(I),5=Crystal(II)");				// 0=Automatic, 1=Grid, 2=Lines, 3=Concentric
	ProfileDataB	infill_before_wall = ProfileDataB("infill_before_wall", "0=False,1=True");
	ProfileDataB	skin_outline = ProfileDataB("skin_outline", "0=False,1=True");
	ProfileDataI	skin_type = ProfileDataI("skin_type", "0=Line,1=Concentric");
	ProfileDataD	skin_removal_width_top = ProfileDataD("skin_removal_width_top", "mm");
	ProfileDataD	skin_removal_width_bottom = ProfileDataD("skin_removal_width_bottom", "mm");
	ProfileDataI	skin_overlap = ProfileDataI("skin_overlap", "%");

	//speed//
	ProfileDataD	print_speed = ProfileDataD("print_speed", "mm/s");
	ProfileDataD	initial_layer_speed = ProfileDataD("initial_layer_speed", "mm/s");
	ProfileDataD	outer_wall_speed = ProfileDataD("outer_wall_speed", "mm/s");
	ProfileDataD	inner_wall_speed = ProfileDataD("inner_wall_speed", "mm/s");
	ProfileDataD	infill_speed = ProfileDataD("infill_speed", "mm/s");
	ProfileDataD	top_bottom_speed = ProfileDataD("top_bottom_speed", "mm/s");
	ProfileDataD	travel_speed = ProfileDataD("travel_speed", "mm/s");
	ProfileDataI	slower_layers_count = ProfileDataI("slower_layers_count", "EA");

	//retraction//
	ProfileDataB	retraction_enable = ProfileDataB("retraction_enable", "0=Disable,1=Enable");		// 0=Off, 1=On
	ProfileDataD	retraction_speed = ProfileDataD("retraction_speed", "mm/s");
	ProfileDataD	retraction_amount = ProfileDataD("retraction_amount", "mm");
	ProfileDataD	retraction_min_travel = ProfileDataD("retraction_min_travel", "mm");
	ProfileDataI	retraction_combing = ProfileDataI("retraction_combing", "0=Off,1=All,2=No_Skin");			// 0=off, 1=All, 2=No Skin
	ProfileDataD	retraction_minimal_extrusion = ProfileDataD("retraction_minimal_extrusion", "mm");
	ProfileDataD	retraction_hop = ProfileDataD("retraction_hop", "mm");
	//ProfileDataD	retraction_preretraction_0_distance;	// new
	//ProfileDataD	retraction_preretraction_0_amount;		// new
	//ProfileDataD	retraction_overmoving_0_distance;		// new
	//ProfileDataD	retraction_preretraction_X_distance;	// new
	//ProfileDataD	retraction_preretraction_X_amount;		// new
	//ProfileDataD	retraction_overmoving_X_distance;		// new
	ProfileDataI	internal_moving_area = ProfileDataI("internal_moving_area", "0=Narrow,1=Default,2=Wide");


	//cooling//
	ProfileDataD	cool_min_layer_time = ProfileDataD("cool_min_layer_time", "sec");
	ProfileDataB	fan_enabled = ProfileDataB("fan_enabled", "0=False,1=True");				// 0=disable, 1=e`nable
	ProfileDataD	fan_full_height = ProfileDataD("fan_full_height", "mm");
	ProfileDataI	fan_speed_regular = ProfileDataI("fan_speed_regular", "%");
	ProfileDataI	fan_speed_max = ProfileDataI("fan_speed_max", "%");
	ProfileDataD	cool_min_feedrate = ProfileDataD("cool_min_feedrate", "%");
	ProfileDataB	cool_head_lift = ProfileDataB("cool_head_lift", "0=False,1=True");				// 0=false, 1=true


	//tool_change//
	ProfileDataD	toolchange_retraction_amount = ProfileDataD("toolchange_retraction_amount", "mm");			// @cura --> retraction_dual_amount
	ProfileDataD	toolchange_retraction_speed = ProfileDataD("toolchange_retraction_speed", "mm/s");			// new
	ProfileDataD	toolchange_lowering_bed = ProfileDataD("toolchange_lowering_bed", "mm");
	ProfileDataD	toolchange_extra_restart_amount = ProfileDataD("toolchange_extra_restart_amount", "mm");		// cartridge profile
	ProfileDataD	toolchange_extra_restart_speed = ProfileDataD("toolchange_extra_restart_speed", "mm/s");			// cartridge profile

	//standby & preheat//
	ProfileDataB	standby_temperature_enabled = ProfileDataB("standby_temperature_enabled", "0=False,1=True");
	ProfileDataI	operating_standby_temperature = ProfileDataI("operating_standby_temperature", "C");
	ProfileDataI	initial_standby_temperature = ProfileDataI("initial_standby_temperature", "C");
	ProfileDataB	preheat_enabled = ProfileDataB("preheat_enabled", "0=Disable,1=Enable");
	ProfileDataD	preheat_threshold_time = ProfileDataD("preheat_threshold_time", "sec");

	//wipe tower//
	ProfileDataB	wipe_tower_enabled = ProfileDataB("wipe_tower_enabled", "0=False,1=True");
	ProfileDataI	wipe_tower_position = ProfileDataI("wipe_tower_position", "index");
	ProfileDataI	wipe_tower_infill_density = ProfileDataI("wipe_tower_infill_density", "%");
	ProfileDataD	wipe_tower_raft_margin = ProfileDataD("wipe_tower_raft_margin", "mm");
	ProfileDataI	wipe_tower_outer_cartridge_index;
	ProfileDataI	wipe_tower_inner_cartridge_index;
	ProfileDataD	wipe_tower_base_size = ProfileDataD("wipe_tower_base_size", "mm");
	ProfileDataI	wipe_tower_base_layer_count = ProfileDataI("wipe_tower_base_layer_count", "EA");
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
	
	//ProfileDataD	nozzle_size = ProfileDataD("nozzle_size", "mm");
	ProfileDataI	bed_type = ProfileDataI("bed_type", "0=N/A,1=SIDE_A,2=SIDE_B");
	ProfileDataB	path_optimization_enabled = ProfileDataB("path_optimization_enabled", "0=False,1=True");
	ProfileDataD	object_sink = ProfileDataD("object_sink", "mm");
	ProfileDataD	overlap_dual = ProfileDataD("overlap_dual", "mm");
	bool			flag_custom = false;
	bool			flag_imported_profile = false;

	///////////////////////////////////////////////////////////////////////////////////////////////
	//old profile//
	///////////////////////////////////////////////////////////////////////////////////////////////
	ProfileDataD	solid_layer_thickness = ProfileDataD("solid_layer_thickness", "mm");
	ProfileDataI	fill_density = ProfileDataI("fill_density", "%");
	ProfileDataD	printer_speed = ProfileDataD("printer_speed", "mm/s");
	ProfileDataI	support_cartridge_index = ProfileDataI("support_cartridge_index", "index");
	ProfileDataI	support_pattern = ProfileDataI("support_pattern", "0=Grid,1=Line,2=Zigzag");
	ProfileDataI	support_fill_rate = ProfileDataI("support_fill_rate", "%");
	ProfileDataI	support_interface_count = ProfileDataI("support_interface_count", "EA");
	ProfileDataI	wipe_tower_fill_density = ProfileDataI("wipe_tower_fill_density", "%");
	ProfileDataI	wipe_tower_filament_flow = ProfileDataI("wipe_tower_filament_flow", "%");
	ProfileDataI	filament_flow = ProfileDataI("filament_flow", "%");
	ProfileDataI	layer0_filament_flow = ProfileDataI("layer0_filament_flow", "%");
	ProfileDataD	layer0_thickness = ProfileDataD("layer0_thickness", "mm");
	ProfileDataI	layer0_width_factor = ProfileDataI("layer0_width_factor", "%");
	ProfileDataD	bottom_layer_speed = ProfileDataD("bottom_layer_speed", "mm/s");
	ProfileDataD	solidarea_speed = ProfileDataD("solidarea_speed", "mm/s");
	ProfileDataD	inset0_speed = ProfileDataD("inset0_speed", "mm/s");
	ProfileDataD	insetx_speed = ProfileDataD("insetx_speed", "mm/s");
	ProfileDataD	raft_airgap_layer0 = ProfileDataD("raft_airgap_layer0", "mm");
	ProfileDataD	raft_base_linewidth = ProfileDataD("raft_base_linewidth", "mm");
	ProfileDataD	raft_interface_linewidth = ProfileDataD("raft_interface_linewidth", "mm");
	ProfileDataD	raft_surface_linewidth = ProfileDataD("raft_surface_linewidth", "mm");
	///////////////////////////////////////////////////////////////////////////////////////////////
	
} ;


class ProfileMapValue
{
public:
	ProfileDataD* profile_data_D;
	ProfileDataB* profile_data_B;
	ProfileDataI* profile_data_I;
	ProfileDataS* profile_data_S;
	int profile_type;

	SliceProfile* slice_profile;

public:
	ProfileMapValue(SliceProfile* _slice_profile, ProfileDataD* _profile_data_D);
	ProfileMapValue(SliceProfile* _slice_profile, ProfileDataI* _profile_data_I);
	ProfileMapValue(SliceProfile* _slice_profile, ProfileDataB* _profile_data_B);
	ProfileMapValue(SliceProfile* _slice_profile, ProfileDataS* _profile_data_S);

	void setProfileDataForProfileMap(std::string _profile_value);
	std::string getProfileDataNameKey();
	std::string getProfileDataUnitKey();
	std::string getProfileDataValueStr();

};

//////////////////////////////////////////////
//template <typename T>
//class ProfileDataMapValue
//{
//private:
//	ProfileData<T>* profile_data;
//	SliceProfile* slice_profile;
//
//public:
//	ProfileDataMapValue(SliceProfile* _slice_profile, ProfileData<T>* _profile_data)
//		: profile_data(_profile_data),
//		slice_profile(_slice_profile) { };
//
//	void setProfileDataForMap(std::string _profile_value)
//	{
//		slice_profile->setProfileData(profile_data, _profile_value);
//	}
//};



