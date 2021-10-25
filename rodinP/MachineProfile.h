#pragma once
#include <string>
#include <qstring.h>
#include <QStringList>
#include <vector>
#include "ProfileData.h"

class MachineProfile
{
public:
	MachineProfile();
	~MachineProfile();

	void setMachineDataDefault();
	bool loadMachineProfile(const wchar_t*);

	bool loadRecentMachineProfile(const wchar_t*);
	void saveRecentMachineProfile(const wchar_t*);

	void setProfileData(ProfileDataB*, bool);
	void setProfileData(ProfileDataD*, double, double, double);
	void setProfileData(ProfileDataD*, double);
	void setProfileData(ProfileDataI*, int, int, int);

	void loadStartEndDefaultCode(const wchar_t*);

public:
	ProfileDataI	machine_width_default;
	ProfileDataI	machine_depth_default;
	ProfileDataI	machine_height_default;
	ProfileDataI	machine_width_offset;
	ProfileDataI	machine_depth_offset;

	ProfileDataB	machine_expanded_print_function_enabled;
	ProfileDataB	machine_expanded_print_mode;
	ProfileDataI	machine_expanded_print_cartridgeIndex;
	ProfileDataI	machine_expanded_width_offset;
	ProfileDataI	machine_expanded_depth_offset;
	ProfileDataI	machine_expanded_height_offset;

	ProfileDataB	machine_bed_selected_enabled; //0=False,1=True
	ProfileDataI	machine_bed_side_default; //0=side_a,1=side_b
	ProfileDataI	machine_bed_selected_side; //0=side_a, 1=side_b

	ProfileDataI	machine_shape;
	ProfileDataB	machine_center_is_zero;

	ProfileDataD	machine_max_feedrate_X;
	ProfileDataD	machine_max_feedrate_Y;
	ProfileDataD	machine_max_feedrate_Z;
	ProfileDataD	machine_max_feedrate_E;
	ProfileDataD	machine_min_feedrate;
	ProfileDataD	machine_acceleration;
	ProfileDataD	machine_max_acceleration_X;
	ProfileDataD	machine_max_acceleration_Y;
	ProfileDataD	machine_max_acceleration_Z;
	ProfileDataD	machine_max_acceleration_E;
	ProfileDataD	machine_max_bump_X;
	ProfileDataD	machine_max_bump_Y;
	ProfileDataD	machine_max_bump_Z;
	ProfileDataD	machine_max_bump_E;

	ProfileDataB	has_heated_bed;
	ProfileDataB	auto_center;

	ProfileDataI	extruder_count;
	std::vector<ProfileDataD>	nozzle_size;

	QString			machine_model;
	QString			group_model;
	QString			available_materials;
	QStringList		available_material_list;
	QString							printable_material_combinations;
	std::vector<QStringList>		printable_material_combination_list;

	QString							warning_material_combinations;
	std::vector<QStringList>		warning_material_combination_list;
	ProfileDataB	is_ETC_all_available;

	ProfileDataB	cooling_control_enabled;
	ProfileDataI	fan_speed_low_limit_value;
	ProfileDataB	material_selection_enabled;
	ProfileDataB	raft_base_fan_control_enabled;
	ProfileDataD	raft_base_retraction_amount;
	ProfileDataB	raft_base_pathLengthLimit_enabled;
	ProfileDataB	restrict_frontUpperRegion_enabled;
	ProfileDataB	restrict_rearQuadRegion_enabled;
	ProfileDataD	startcode_Z_leveling;
	ProfileDataI	view_cam_distance;
	ProfileDataD	view_cam_center_factor_from_bedcenter_X;
	ProfileDataD	view_cam_center_factor_from_bedcenter_Y;
	ProfileDataD	view_cam_center_factor_from_bedcenter_Z;
	ProfileDataB	replication_print;
	ProfileDataI	thumbnail_image_width;
	ProfileDataI	thumbnail_image_height;

	ProfileDataI	company_code;
	ProfileDataB	has_web_camera;
	ProfileDataB	has_SSD_storage;
	//ProfileDataI	gcode_flavor;
	ProfileDataI	firmware_code;
	ProfileDataB	is_bed_type_selectable;

	std::string		start_default_code;
	std::string		end_default_code;

	//////////////////////////////////////////////////////////////////////////
	//for internal data//
	//////////////////////////////////////////////////////////////////////////
	bool openMode;

	//machine size calculated = machine default + expanded offset
	//ProfileDataI	machine_width_calculated;
	//ProfileDataI	machine_depth_calculated;
	//ProfileDataI	machine_height_calculated;
	//////////////////////////////////////////////////////////////////////////

};

