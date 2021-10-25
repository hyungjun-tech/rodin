#include "stdafx.h"
#include <cctype>
#include <fstream>
#include <stdio.h>
#include "settings.h"

ConfigSettings::ConfigSettings()
{ }

void ConfigsSetForFlow::setConfigsSet(std::vector<ConfigSettings> _configs)
{
	clearConfigsSet();

	for (auto config : _configs)
	{
		overall_flow.push_back(config.overall_flow);
		initial_layer_flow.push_back(config.initial_layer_flow);
		infill_flow.push_back(config.infill_flow);
		outer_wall_flow.push_back(config.outer_wall_flow);
		inner_wall_flow.push_back(config.inner_wall_flow);
		top_bottom_flow.push_back(config.top_bottom_flow);
	}

	support_main_flow = _configs.front().support_main_flow;
	support_interface_roof_flow = _configs.front().support_interface_roof_flow;
	support_interface_floor_flow = _configs.front().support_interface_floor_flow;
	//skirt_flow = _configs.front().skirt_flow;
	//brim_flow = _configs.front().brim_flow;
	wipe_tower_flow = _configs.front().wipe_tower_flow;
}

void ConfigsSetForFlow::clearConfigsSet()
{
	overall_flow.clear();
	initial_layer_flow.clear();
	infill_flow.clear();
	outer_wall_flow.clear();
	inner_wall_flow.clear();
	top_bottom_flow.clear();

	support_main_flow = 0;
	support_interface_roof_flow = 0;
	support_interface_floor_flow = 0;
	//skirt_flow = 0;
	//brim_flow = 0;
	wipe_tower_flow = 0;
}

void ConfigsSetForSpeed::setConfigsSet(std::vector<ConfigSettings> _configs)
{
	clearConfigsSet();

	for (auto config : _configs)
	{
		print_speed.push_back(config.print_speed);
		initial_layer_speed.push_back(config.initial_layer_speed);
		infill_speed.push_back(config.infill_speed);
		outer_wall_speed.push_back(config.outer_wall_speed);
		inner_wall_speed.push_back(config.inner_wall_speed);
		top_bottom_speed.push_back(config.top_bottom_speed);
		travel_speed.push_back(config.travel_speed);

	}

	support_main_speed = _configs.front().support_main_speed;
	support_interface_roof_speed = _configs.front().support_interface_roof_speed;
	support_interface_floor_speed = _configs.front().support_interface_floor_speed;
	//skirt_speed = _configs.front().skirt_speed;
	//brim_speed = _configs.front().brim_speed;
	wipe_tower_speed = _configs.front().wipe_tower_speed;
	raft_base_speed = _configs.front().raft_base_speed;
	raft_interface_speed = _configs.front().raft_interface_speed;
	raft_surface_speed = _configs.front().raft_surface_speed;
}

void ConfigsSetForSpeed::clearConfigsSet()
{
	print_speed.clear();
	initial_layer_speed.clear();
	infill_speed.clear();
	outer_wall_speed.clear();
	inner_wall_speed.clear();
	top_bottom_speed.clear();
	travel_speed.clear();

	support_main_speed = 0;
	support_interface_roof_speed = 0;
	support_interface_floor_speed = 0;
	//skirt_speed = 0;
	//brim_speed = 0;
	wipe_tower_speed = 0;
	raft_base_speed = 0;
	raft_interface_speed = 0;
	raft_surface_speed = 0;
}

void ConfigSetForCooling::setConfigsSet(std::vector<ConfigSettings> _configs)
{
	clearConfigsSet();

	for (auto config : _configs)
	{
		minimal_layer_time.push_back(config.minimal_layer_time);
		minimal_feedrate.push_back(config.minimal_feedrate);
		cool_head_lift.push_back(config.cool_head_lift);
		fan_speed_regular.push_back(config.fan_speed_regular);
		fan_speed_max.push_back(config.fan_speed_max);
		fan_full_on_layer_number.push_back(config.fan_full_on_layer_number);
	}
}

void ConfigSetForCooling::clearConfigsSet()
{
	minimal_layer_time.clear();
	minimal_feedrate.clear();
	cool_head_lift.clear();
	fan_speed_regular.clear();
	fan_speed_max.clear();
	fan_full_on_layer_number.clear();

}

void ConfigSetForRetraction::setConfigsSet(std::vector<ConfigSettings> _configs)
{
	clearConfigsSet();

	for (auto config : _configs)
	{
		retraction_amount.push_back(config.retraction_amount);
		retraction_amount_prime.push_back(config.retraction_amount_prime);
		retraction_speed.push_back(config.retraction_speed);
		retraction_minimal_distance.push_back(config.retraction_minimal_distance);
		minimal_extrusion_before_retraction.push_back(config.minimal_extrusion_before_retraction);
		retraction_z_hop.push_back(config.retraction_z_hop);
	}

	retraction_with_combing = _configs.front().retraction_with_combing;

}

void ConfigSetForRetraction::clearConfigsSet()
{
	retraction_amount.clear();
	retraction_amount_prime.clear();
	retraction_speed.clear();
	retraction_minimal_distance.clear();
	minimal_extrusion_before_retraction.clear();
	retraction_z_hop.clear();

	retraction_with_combing = 0;
}