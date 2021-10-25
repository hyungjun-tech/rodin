#pragma once

#include "floatpoint.h"

#define FIX_HORRIBLE_UNION_ALL_TYPE_A    0x01
#define FIX_HORRIBLE_UNION_ALL_TYPE_B    0x02
#define FIX_HORRIBLE_EXTENSIVE_STITCHING 0x04
#define FIX_HORRIBLE_UNION_ALL_TYPE_C    0x08
#define FIX_HORRIBLE_KEEP_NONE_CLOSED    0x10

//config에서 사용되는 extruder의 갯수//
//rodin기준 : 2 --> 나중에 더 확장 가능..//
#define MAX_EXTRUDERS 2

class ConfigSettings
{
public:
    ConfigSettings();

	int filament_diameter;
	std::string filament_material;


    //quality//
    int layer_height;
    int initial_layer_height;

	//extrudion width (mm) at config//
	//mm (config) - % (slice profile)//
	int extrusion_width;
	int initial_layer_extrusion_width;
	int wall_extrusion_width;
	int infill_extrusion_width;
	int top_bottom_extrusion_width;
    int zOffset_raft;
    int zOffset_except_raft;

    //shell//
    int inset_count;
    int down_skin_count;
    int up_skin_count;
    int wall_print_direction;
    int internal_moving_area;

    //temperature//
    int print_temperature;
    int print_temperature_max;
    int print_bed_temperature;
    int temperature_layer_setting_enabled;

    //flow//
	int overall_flow_control_enabled;
    int overall_flow;
    int initial_layer_flow;
	int infill_flow;
    int outer_wall_flow;
    int inner_wall_flow;
    int top_bottom_flow;


    //support//
    int support_main_cartridge_index;		// add swyang
    int support_interface_cartridge_index;	// add swyang
    int support_placement;
    int support_main_pattern;
    int support_angle;
	int support_infill_denstiy;
    int support_everywhere;
    int support_line_distance;
    int support_xy_distance;
    int support_z_distance;
    int support_horizontal_expansion;	// add swyang
	int support_infill_overlap;		// add swyang
	//int support_extruder;
	int support_inset_count;			// add swyang
	int support_main_speed;
	int support_main_flow;
	int support_main_extrusion_width;

	int support_interface_enabled;			// add swyang
    int support_interface_count;		// add swyang
	int support_interface_pattern;			// add swyang
	int support_interface_roof_layers_count;
	int support_interface_roof_extrusion_width;
	int support_interface_roof_speed;
	int support_interface_roof_flow;
	int support_interface_floor_layers_count;
	int support_interface_floor_extrusion_width;
	int support_interface_floor_speed;
	int support_interface_floor_flow;

    //adhesion settings//
    int platform_adhesion;
    int adhesion_cartridge_index;
    
    int skirt_line_count;
	//int skirt_extrusion_width;
	int skirt_distance;
	int skirt_min_length;
	//int skirt_speed;
	//int skirt_flow;
	int brim_line_count;
	//int brim_extrusion_width;
	//int brim_speed;
	//int brim_flow;

    int raft_margin;
    int raft_line_spacing;
	int raft_airgap_all;
	int raft_airgap_initial_layer;
	int raft_inset_enabled;			// add jusung
	int raft_inset_offset;			// add jusung
	int raft_temperature_control;		// add jusung
	int raft_incline_enabled;			// add jusung	0 : false, 1 : true
    int raft_base_thickness;
    int raft_base_line_width;
    int raft_base_speed;
    int raft_base_temperature;
    int raft_interface_thickness;
    int raft_interface_line_width;
    int raft_interface_line_spacing;
    int raft_interface_speed;
    int raft_interface_temperature;
    int raft_fan_speed;
    int raft_surface_thickness;
    int raft_surface_line_width;
    int raft_surface_line_spacing;
    int raft_surface_layers;
    int raft_surface_speed;
    int raft_surface_initial_temperature;
    int raft_surface_last_temperature;

    //infill//
    int infill_density;
	int sparse_infill_line_distance;
    int infill_overlap;
    int infill_pattern;
    int skin_outline;
    int skin_type;
    int infill_before_wall;
    int skin_removal_width_top;
    int skin_removal_width_bottom;
    int skin_overlap;


    //speed//
	int print_speed;
	int initial_layer_speed;
	int outer_wall_speed;
	int inner_wall_speed;
	int infill_speed;
	int top_bottom_speed;
	int travel_speed;
	int slower_layers_count;

    //retraction settings//
    int retraction_amount;
    int retraction_amount_prime;
    int retraction_speed;
    int retraction_minimal_distance;
    int minimal_extrusion_before_retraction;
    int retraction_z_hop;
    int retraction_with_combing;		// add jusung	0 : false, 1 : true
    //int preretration0Length;			// add jusung
    //int preretrationXLength;			// add jusung
    //int preretration0FilamentLength;	// add jusung
    //int preretrationXFilamentLength;	// add jusung
    //int overmoving0Length;				// add jusung
    //int overmovingXLength;				// add jusung

	//cool settings//
	double minimal_layer_time;
	int minimal_feedrate;
	int cool_head_lift;
	int fan_speed_regular;
	int fan_speed_max;
	int fan_full_on_layer_number;

	//standby & preheat//
	int standby_temperature_enabled;
	int operating_standby_temperature;
	int initial_standby_temperature;
	int preheat_enabled;
	double preheat_threshold_time;
	int preheat_temperature_falling_rate;
	int preheat_temperature_rising_rate;

    //toolChange settings//
    int toolchange_retraction_amount;		// add swyang
    int toolchange_retraction_speed;		// add swyang
    int toolchange_lowering_bed;			// add swyang
    int toolchange_extra_restart_amount;	// add swyang
    int toolchange_extra_restart_speed;		// add swyang
    
    //path optimization//
    int path_optimization_parameter[3];
    int path_optimization_filtering_angle;
    int enable_combing;
    //int enable_ooze_shield;

    //wipe tower//
    int wipe_tower_enable;				// add swyang
    int wipe_tower_outer_size;
    int wipe_tower_base_size;
    int wipe_tower_inner_size;
    int wipe_tower_infill_density;			// add swyang
    int wipe_tower_sparse_infill_line_distance;
    int wipe_tower_position;
    int wipe_tower_raft_margin;
    int wipe_tower_outer_inner_gap;
    int wipe_tower_outer_wall_thickness;
    int wipe_tower_base_layer_count;
    int wipe_tower_outer_cartridge_index;
    int wipe_tower_inner_cartridge_index;
	int wipe_tower_flow;
	int wipe_tower_speed;

	//shape error correction//
	int fix_horrible;
	int spiralize_mode;
	int simple_mode;

    int multi_volume_overlap;
	   	 
	int bed_type;
	
    //adjust Z Gap//
    int adjust_z_gap_thickness;
    int adjust_z_gap_layers;
    int adjust_postion;				// 0 : left, 1 : right
	
    FMatrix3x3 matrix;
    IntPoint object_position;
    int object_sink;
    int auto_center;


    //int gcodeFlavor;

    IntPoint extruder_offset[MAX_EXTRUDERS];
    std::string start_code_image;
    std::string start_code_email_info;
    std::string start_code;
    std::string end_code;
    std::string pre_switch_extruder_code;
    std::string post_switch_extruder_code;

    //Machine Specification
    int machine_width;
    int machine_depth;
    int machine_height;
    int machine_center_is_bedcenter;

};

class ConfigsSetForFlow
{
public:
	ConfigsSetForFlow()	{ };

	void setConfigsSet(std::vector<ConfigSettings> _configs);
	void clearConfigsSet();

public:
	std::vector<int> overall_flow;
	std::vector<int> initial_layer_flow;
	std::vector<int> infill_flow;
	std::vector<int> outer_wall_flow;
	std::vector<int> inner_wall_flow;
	std::vector<int> top_bottom_flow;

	int support_main_flow;
	int support_interface_roof_flow;
	int support_interface_floor_flow;
	//int skirt_flow;
	//int brim_flow;
	int wipe_tower_flow;


};

class ConfigsSetForSpeed
{
public:
	ConfigsSetForSpeed() { };

	void setConfigsSet(std::vector<ConfigSettings> _configs);
	void clearConfigsSet();

public:
	std::vector<int> print_speed;
	std::vector<int> initial_layer_speed;
	std::vector<int> infill_speed;
	std::vector<int> outer_wall_speed;
	std::vector<int> inner_wall_speed;
	std::vector<int> top_bottom_speed;
	std::vector<int> travel_speed;

	int support_main_speed;
	int support_interface_roof_speed;
	int support_interface_floor_speed;
	//int skirt_speed;
	//int brim_speed;
	int wipe_tower_speed;
	int raft_base_speed;
	int raft_interface_speed;
	int raft_surface_speed;
};

class ConfigSetForCooling
{
public:
	ConfigSetForCooling() { };

	void setConfigsSet(std::vector<ConfigSettings> _configs);
	void clearConfigsSet();

public:
	std::vector<double> minimal_layer_time;
	std::vector<int> minimal_feedrate;
	std::vector<int> cool_head_lift;
	std::vector<int> fan_speed_regular;
	std::vector<int> fan_speed_max;
	std::vector<int> fan_full_on_layer_number;

};

class ConfigSetForRetraction
{
public:
	ConfigSetForRetraction() { };

	void setConfigsSet(std::vector<ConfigSettings> _configs);
	void clearConfigsSet();

public:
	std::vector<int> retraction_amount;
	std::vector<int> retraction_amount_prime;
	std::vector<int> retraction_speed;
	std::vector<int> retraction_minimal_distance;
	std::vector<int> minimal_extrusion_before_retraction;
	std::vector<int> retraction_z_hop;

	int retraction_with_combing;
};