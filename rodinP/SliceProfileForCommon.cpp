#include "stdafx.h"
#include "SliceProfileForCommon.h"

#define PROFILE_NUM 88

SliceProfileForCommon::SliceProfileForCommon()
{
	slice_profile_common_map = setSliceProfileForCommonMap();
}

SliceProfileForCommon::~SliceProfileForCommon()
{

}

bool SliceProfileForCommon::loadSliceProfileCommon(const wchar_t* filepath)
{
	std::ifstream fs(filepath);

	if (fs.is_open())
	{
		std::cout << "Common profile loading complete!!" << "\n" << QString::fromWCharArray(filepath).toStdString() << "\n";
		Logger() << "Common Profile loading complete." << "\n" << QString::fromWCharArray(filepath);
	}
	else
	{
		std::cout << "Common profile loading failure!!" << "\n" << QString::fromWCharArray(filepath).toStdString() << "\n";
		Logger() << "Common Profile loading failure" << "\n" << QString::fromWCharArray(filepath);
		return false;
	}

	std::string strLine;
	while (std::getline(fs, strLine))
	{
		std::string name;
		std::string profile_value;

		const auto equals_idx = strLine.find_first_of(": ");
		if (std::string::npos != equals_idx)
		{
			name = strLine.substr(0, equals_idx);
			profile_value = strLine.substr(equals_idx + 2);
		}

		// profile name --> map key //
		const auto unit_idx = name.find_first_of("(");
		if (std::string::npos != unit_idx)
			name = name.substr(0, unit_idx);

		// key finding at map //
		std::map<std::string, ProfileCommonMapValue*>::const_iterator it = slice_profile_common_map.find(name);
		if (it != slice_profile_common_map.end())
		{
			it->second->setProfileDataForProfileCommonMap(profile_value);
		}
		else
		{
			QString tempMsg = "[Warning] Missing common profile - " + QString::fromStdString(name) + ": " + QString::fromStdString(profile_value);
			Logger() << tempMsg;
			std::cout << tempMsg.toStdString() << "\n";
		}
	}

	fs.close();

	return true;
}


void SliceProfileForCommon::setProfileData(QString* data, std::string value)
{
	*data = QString::fromStdString(value).trimmed();
}

void SliceProfileForCommon::setProfileData(ProfileDataD* data, std::string value)
{
	getProfileValue(value, &data->value, &data->min, &data->max);
}

void SliceProfileForCommon::setProfileData(ProfileDataB* data, std::string value)
{
	getProfileValue(value, &data->value);
}

void SliceProfileForCommon::setProfileData(ProfileDataI* data, std::string value)
{
	getProfileValue(value, &data->value, &data->min, &data->max);
}

void SliceProfileForCommon::setProfileData(ProfileDataS* data, std::string value)
{
	data->value = QString::fromStdString(value).trimmed();
}

void SliceProfileForCommon::getProfileValue(std::string fromValue, double* value, double* min, double* max)
{
	QString l_value, l_min, l_max;
	getProfileValue(fromValue, &l_value, &l_min, &l_max);

	if (l_value != "") *value = QString(l_value).toDouble();
	if (l_min != "") *min = QString(l_min).toDouble();
	if (l_max != "") *max = QString(l_max).toDouble();
}

void SliceProfileForCommon::getProfileValue(std::string fromValue, bool* value)
{
	QString l_value;
	getProfileValue(fromValue, &l_value);

	if (l_value != "") *value = (bool)QString(l_value).toInt();
}

void SliceProfileForCommon::getProfileValue(std::string fromValue, int* value, int* min, int* max)
{
	QString l_value, l_min, l_max;
	getProfileValue(fromValue, &l_value, &l_min, &l_max);

	if (l_value != "") *value = QString(l_value).toInt();
	if (l_min != "") *min = QString(l_min).toInt();
	if (l_max != "") *max = QString(l_max).toInt();
}

void SliceProfileForCommon::getProfileValue(std::string fromValue, QString* value, QString* min, QString* max)
{
	char valueTemp[30];
	char minValueTemp[30];
	char maxValueTemp[30];
	std::istringstream iss(fromValue);

	if (!(iss >> valueTemp >> minValueTemp >> maxValueTemp))
	{
		if (!valueTemp) {
			iss.clear();
			return;
		}
		else *value = QString(valueTemp);
	}
	else
	{
		if (value != nullptr) *value = QString(valueTemp);
		if (min != nullptr) *min = QString(minValueTemp);
		if (max != nullptr) *max = QString(maxValueTemp);

		if ((*value).toDouble() < (*min).toDouble())
		{
			std::cout << "Profile Error : " << "min value - " << (*min).toStdString() << "- current value - " << (*value).toStdString() << "\n";
			Logger() << "Profile Error : min value - " + *min + "- current value - " + *value;
		}
		if ((*value).toDouble() > (*max).toDouble())
		{
			std::cout << "Profile Error : " << "max value - " << (*max).toStdString() << "- current value - " << (*value).toStdString() << "\n";
			Logger() << "Profile Error : max value - " + *max + "- current value - " + *value;
		}
	}
	iss.clear();
}


void SliceProfileForCommon::setProfileData(ProfileDataD* data, ProfileDataD value)
{
	data->value = value.value;
	data->min = value.min;
	data->max = value.max;
}
void SliceProfileForCommon::setProfileData(ProfileDataB* data, ProfileDataB value)
{
	data->value = value.value;
}
void SliceProfileForCommon::setProfileData(ProfileDataI* data, ProfileDataI value)
{
	data->value = value.value;
	data->min = value.min;
	data->max = value.max;
}

//이거는 sliceProfile쪽에서 해야 할 것 같은데.. sliceProfile::SetSliceProfileDataFromCommon(SliceProfileForCommon profileCommon)
void SliceProfileForCommon::setSliceProfileDataFromCommon(SliceProfile *sliceProfile)
{
	this->setProfileData(&sliceProfile->layer_height, this->layer_height);
	this->setProfileData(&sliceProfile->initial_layer_height, this->initial_layer_height);
	this->setProfileData(&sliceProfile->initial_layer_width_factor, this->initial_layer_width_factor);
	this->setProfileData(&sliceProfile->z_offset_raft, this->z_offset_raft);
	this->setProfileData(&sliceProfile->z_offset_except_raft, this->z_offset_except_raft);
	//this->setProfileData(&sliceProfile->nozzle_size, this->nozzle_size);
	this->setProfileData(&sliceProfile->print_bed_temperature, this->print_bed_temperature);

	this->setProfileData(&sliceProfile->support_main_cartridge_index, this->support_main_cartridge_index);
	this->setProfileData(&sliceProfile->support_placement, this->support_placement);
	this->setProfileData(&sliceProfile->support_main_pattern, this->support_main_pattern);
	this->setProfileData(&sliceProfile->support_angle, this->support_angle);
	this->setProfileData(&sliceProfile->support_main_speed, this->support_main_speed);
	this->setProfileData(&sliceProfile->support_infill_density, this->support_infill_density);
	this->setProfileData(&sliceProfile->support_xy_distance, this->support_xy_distance);
	this->setProfileData(&sliceProfile->support_z_distance, this->support_z_distance);
	this->setProfileData(&sliceProfile->support_main_flow, this->support_main_flow);
	this->setProfileData(&sliceProfile->support_main_width_factor, this->support_main_width_factor);
	this->setProfileData(&sliceProfile->support_interface_enabled, this->support_interface_enabled);
	this->setProfileData(&sliceProfile->support_interface_pattern, this->support_interface_pattern);
	this->setProfileData(&sliceProfile->support_interface_roof_layers_count, this->support_interface_roof_layers_count);
	this->setProfileData(&sliceProfile->support_interface_roof_width_factor, this->support_interface_roof_width_factor);
	this->setProfileData(&sliceProfile->support_interface_roof_flow, this->support_interface_roof_flow);
	this->setProfileData(&sliceProfile->support_interface_roof_speed, this->support_interface_roof_speed);
	this->setProfileData(&sliceProfile->support_interface_floor_layers_count, this->support_interface_floor_layers_count);
	this->setProfileData(&sliceProfile->support_interface_floor_width_factor, this->support_interface_floor_width_factor);
	this->setProfileData(&sliceProfile->support_interface_floor_flow, this->support_interface_floor_flow);
	this->setProfileData(&sliceProfile->support_interface_floor_speed, this->support_interface_floor_speed);
	this->setProfileData(&sliceProfile->support_horizontal_expansion, this->support_horizontal_expansion);

	this->setProfileData(&sliceProfile->platform_adhesion, this->platform_adhesion);
	this->setProfileData(&sliceProfile->adhesion_cartridge_index, this->adhesion_cartridge_index);
	this->setProfileData(&sliceProfile->skirt_line_count, this->skirt_line_count);
	//this->setProfileData(&sliceProfile->skirt_width_factor, this->skirt_width_factor);
	this->setProfileData(&sliceProfile->skirt_gap, this->skirt_gap);
	this->setProfileData(&sliceProfile->skirt_minimal_length, this->skirt_minimal_length);
	//this->setProfileData(&sliceProfile->skirt_speed, this->skirt_speed);
	//this->setProfileData(&sliceProfile->skirt_flow, this->skirt_flow);
	this->setProfileData(&sliceProfile->brim_line_count, this->brim_line_count);
	//this->setProfileData(&sliceProfile->brim_width_factor, this->brim_width_factor);
	//this->setProfileData(&sliceProfile->brim_speed, this->brim_speed);
	//this->setProfileData(&sliceProfile->brim_flow, this->brim_flow);

	this->setProfileData(&sliceProfile->raft_margin, this->raft_margin);
	this->setProfileData(&sliceProfile->raft_line_spacing, this->raft_line_spacing);
	this->setProfileData(&sliceProfile->raft_airgap_all, this->raft_airgap_all);
	this->setProfileData(&sliceProfile->raft_airgap_initial_layer, this->raft_airgap_initial_layer);
	this->setProfileData(&sliceProfile->raft_inset_enabled, this->raft_inset_enabled);
	this->setProfileData(&sliceProfile->raft_inset_offset, this->raft_inset_offset);
	this->setProfileData(&sliceProfile->raft_temperature_control, this->raft_temperature_control);
	this->setProfileData(&sliceProfile->raft_incline_enabled, this->raft_incline_enabled);
	this->setProfileData(&sliceProfile->raft_base_thickness, this->raft_base_thickness);
	this->setProfileData(&sliceProfile->raft_base_line_width, this->raft_base_line_width);
	this->setProfileData(&sliceProfile->raft_base_speed, this->raft_base_speed);
	this->setProfileData(&sliceProfile->raft_base_temperature, this->raft_base_temperature);
	this->setProfileData(&sliceProfile->raft_interface_thickness, this->raft_interface_thickness);
	this->setProfileData(&sliceProfile->raft_interface_line_width, this->raft_interface_line_width);
	this->setProfileData(&sliceProfile->raft_interface_speed, this->raft_interface_speed);
	this->setProfileData(&sliceProfile->raft_interface_temperature, this->raft_interface_temperature);
	this->setProfileData(&sliceProfile->raft_surface_layers, this->raft_surface_layers);
	this->setProfileData(&sliceProfile->raft_surface_thickness, this->raft_surface_thickness);
	this->setProfileData(&sliceProfile->raft_surface_line_width, this->raft_surface_line_width);
	this->setProfileData(&sliceProfile->raft_surface_speed, this->raft_surface_speed);
	this->setProfileData(&sliceProfile->raft_surface_initial_temperature, this->raft_surface_initial_temperature);
	this->setProfileData(&sliceProfile->raft_surface_last_temperature, this->raft_surface_last_temperature);
	
	this->setProfileData(&sliceProfile->retraction_combing, this->retraction_combing);
	this->setProfileData(&sliceProfile->toolchange_lowering_bed, this->toolchange_lowering_bed);

	this->setProfileData(&sliceProfile->wipe_tower_enabled, this->wipe_tower_enabled);
	this->setProfileData(&sliceProfile->wipe_tower_position, this->wipe_tower_position);
	this->setProfileData(&sliceProfile->wipe_tower_infill_density, this->wipe_tower_infill_density);
	this->setProfileData(&sliceProfile->wipe_tower_raft_margin, this->wipe_tower_raft_margin);
	this->setProfileData(&sliceProfile->wipe_tower_base_size, this->wipe_tower_base_size);
	this->setProfileData(&sliceProfile->wipe_tower_base_layer_count, this->wipe_tower_base_layer_count);
	this->setProfileData(&sliceProfile->wipe_tower_outer_size, this->wipe_tower_outer_size);
	this->setProfileData(&sliceProfile->wipe_tower_outer_wall_thickness, this->wipe_tower_outer_wall_thickness);
	this->setProfileData(&sliceProfile->wipe_tower_outer_inner_gap, this->wipe_tower_outer_inner_gap);
	this->setProfileData(&sliceProfile->wipe_tower_inner_size, this->wipe_tower_inner_size);
	this->setProfileData(&sliceProfile->wipe_tower_outer_cartridge_index, this->wipe_tower_outer_cartridge_index);
	this->setProfileData(&sliceProfile->wipe_tower_inner_cartridge_index, this->wipe_tower_inner_cartridge_index);
	this->setProfileData(&sliceProfile->wipe_tower_flow, this->wipe_tower_flow);
	this->setProfileData(&sliceProfile->wipe_tower_speed, this->wipe_tower_speed);
	
	this->setProfileData(&sliceProfile->spiralize, this->spiralize);
	this->setProfileData(&sliceProfile->simple_mode, this->simple_mode);
	this->setProfileData(&sliceProfile->fix_horrible_union_all_type_a, this->fix_horrible_union_all_type_a);
	this->setProfileData(&sliceProfile->fix_horrible_union_all_type_b, this->fix_horrible_union_all_type_b);
	this->setProfileData(&sliceProfile->fix_horrible_use_open_bits, this->fix_horrible_use_open_bits);
	this->setProfileData(&sliceProfile->fix_horrible_extensive_stitching, this->fix_horrible_extensive_stitching);

	this->setProfileData(&sliceProfile->overlap_dual, this->overlap_dual);
	this->setProfileData(&sliceProfile->object_sink, this->object_sink);
}

void SliceProfileForCommon::getCommonProfileFromSliceProfile(SliceProfile *sliceProfile)
{
	this->setProfileData(&layer_height, sliceProfile->layer_height);
	this->setProfileData(&initial_layer_height, sliceProfile->initial_layer_height);
	this->setProfileData(&initial_layer_width_factor, sliceProfile->initial_layer_width_factor);
	this->setProfileData(&z_offset_raft, sliceProfile->z_offset_raft);
	this->setProfileData(&z_offset_except_raft, sliceProfile->z_offset_except_raft);
	//this->setProfileData(&nozzle_size, sliceProfile->nozzle_size);
	this->setProfileData(&print_bed_temperature, sliceProfile->print_bed_temperature);

	this->setProfileData(&support_main_cartridge_index, sliceProfile->support_main_cartridge_index);
	this->setProfileData(&support_placement, sliceProfile->support_placement);
	this->setProfileData(&support_main_pattern, sliceProfile->support_main_pattern);
	this->setProfileData(&support_angle, sliceProfile->support_angle);
	this->setProfileData(&support_main_speed, sliceProfile->support_main_speed);
	this->setProfileData(&support_infill_density, sliceProfile->support_infill_density);
	this->setProfileData(&support_xy_distance, sliceProfile->support_xy_distance);
	this->setProfileData(&support_z_distance, sliceProfile->support_z_distance);
	this->setProfileData(&support_main_flow, sliceProfile->support_main_flow);
	this->setProfileData(&support_main_width_factor, sliceProfile->support_main_width_factor);
	this->setProfileData(&support_interface_enabled, sliceProfile->support_interface_enabled);
	this->setProfileData(&support_interface_pattern, sliceProfile->support_interface_pattern);
	this->setProfileData(&support_interface_roof_layers_count, sliceProfile->support_interface_roof_layers_count);
	this->setProfileData(&support_interface_roof_width_factor, sliceProfile->support_interface_roof_width_factor);
	this->setProfileData(&support_interface_roof_flow, sliceProfile->support_interface_roof_flow);
	this->setProfileData(&support_interface_roof_speed, sliceProfile->support_interface_roof_speed);
	this->setProfileData(&support_interface_floor_layers_count, sliceProfile->support_interface_floor_layers_count);
	this->setProfileData(&support_interface_floor_width_factor, sliceProfile->support_interface_floor_width_factor);
	this->setProfileData(&support_interface_floor_flow, sliceProfile->support_interface_floor_flow);
	this->setProfileData(&support_interface_floor_speed, sliceProfile->support_interface_floor_speed);
	this->setProfileData(&support_horizontal_expansion, sliceProfile->support_horizontal_expansion);

	this->setProfileData(&platform_adhesion, sliceProfile->platform_adhesion);
	this->setProfileData(&adhesion_cartridge_index, sliceProfile->adhesion_cartridge_index);
	this->setProfileData(&skirt_line_count, sliceProfile->skirt_line_count);
	//this->setProfileData(&skirt_width_factor, sliceProfile->skirt_width_factor);
	this->setProfileData(&skirt_gap, sliceProfile->skirt_gap);
	this->setProfileData(&skirt_minimal_length, sliceProfile->skirt_minimal_length);
	//this->setProfileData(&skirt_speed, sliceProfile->skirt_speed);
	//this->setProfileData(&skirt_flow, sliceProfile->skirt_flow);
	this->setProfileData(&brim_line_count, sliceProfile->brim_line_count);
	//this->setProfileData(&brim_width_factor, sliceProfile->brim_width_factor);
	//this->setProfileData(&brim_speed, sliceProfile->brim_speed);
	//this->setProfileData(&brim_flow, sliceProfile->brim_flow);

	this->setProfileData(&raft_margin, sliceProfile->raft_margin);
	this->setProfileData(&raft_line_spacing, sliceProfile->raft_line_spacing);
	this->setProfileData(&raft_airgap_all, sliceProfile->raft_airgap_all);
	this->setProfileData(&raft_airgap_initial_layer, sliceProfile->raft_airgap_initial_layer);
	this->setProfileData(&raft_inset_enabled, sliceProfile->raft_inset_enabled);
	this->setProfileData(&raft_inset_offset, sliceProfile->raft_inset_offset);
	this->setProfileData(&raft_temperature_control, sliceProfile->raft_temperature_control);
	this->setProfileData(&raft_incline_enabled, sliceProfile->raft_incline_enabled);
	this->setProfileData(&raft_base_thickness, sliceProfile->raft_base_thickness);
	this->setProfileData(&raft_base_line_width, sliceProfile->raft_base_line_width);
	this->setProfileData(&raft_base_speed, sliceProfile->raft_base_speed);
	this->setProfileData(&raft_base_temperature, sliceProfile->raft_base_temperature);
	this->setProfileData(&raft_interface_thickness, sliceProfile->raft_interface_thickness);
	this->setProfileData(&raft_interface_line_width, sliceProfile->raft_interface_line_width);
	this->setProfileData(&raft_interface_speed, sliceProfile->raft_interface_speed);
	this->setProfileData(&raft_interface_temperature, sliceProfile->raft_interface_temperature);
	this->setProfileData(&raft_surface_layers, sliceProfile->raft_surface_layers);
	this->setProfileData(&raft_surface_thickness, sliceProfile->raft_surface_thickness);
	this->setProfileData(&raft_surface_line_width, sliceProfile->raft_surface_line_width);
	this->setProfileData(&raft_surface_speed, sliceProfile->raft_surface_speed);
	this->setProfileData(&raft_surface_initial_temperature, sliceProfile->raft_surface_initial_temperature);
	this->setProfileData(&raft_surface_last_temperature, sliceProfile->raft_surface_last_temperature);

	this->setProfileData(&retraction_combing, sliceProfile->retraction_combing);
	this->setProfileData(&toolchange_lowering_bed, sliceProfile->toolchange_lowering_bed);

	this->setProfileData(&wipe_tower_enabled, sliceProfile->wipe_tower_enabled);
	this->setProfileData(&wipe_tower_position, sliceProfile->wipe_tower_position);
	this->setProfileData(&wipe_tower_infill_density, sliceProfile->wipe_tower_infill_density);
	this->setProfileData(&wipe_tower_raft_margin, sliceProfile->wipe_tower_raft_margin);
	this->setProfileData(&wipe_tower_base_size, sliceProfile->wipe_tower_base_size);
	this->setProfileData(&wipe_tower_base_layer_count, sliceProfile->wipe_tower_base_layer_count);
	this->setProfileData(&wipe_tower_outer_size, sliceProfile->wipe_tower_outer_size);
	this->setProfileData(&wipe_tower_outer_wall_thickness, sliceProfile->wipe_tower_outer_wall_thickness);
	this->setProfileData(&wipe_tower_outer_inner_gap, sliceProfile->wipe_tower_outer_inner_gap);
	this->setProfileData(&wipe_tower_inner_size, sliceProfile->wipe_tower_inner_size);
	this->setProfileData(&wipe_tower_outer_cartridge_index, sliceProfile->wipe_tower_outer_cartridge_index);
	this->setProfileData(&wipe_tower_inner_cartridge_index, sliceProfile->wipe_tower_inner_cartridge_index);
	this->setProfileData(&wipe_tower_flow, sliceProfile->wipe_tower_flow);
	this->setProfileData(&wipe_tower_speed, sliceProfile->wipe_tower_speed);

	this->setProfileData(&spiralize, sliceProfile->spiralize);
	this->setProfileData(&simple_mode, sliceProfile->simple_mode);
	this->setProfileData(&fix_horrible_union_all_type_a, sliceProfile->fix_horrible_union_all_type_a);
	this->setProfileData(&fix_horrible_union_all_type_b, sliceProfile->fix_horrible_union_all_type_b);
	this->setProfileData(&fix_horrible_use_open_bits, sliceProfile->fix_horrible_use_open_bits);
	this->setProfileData(&fix_horrible_extensive_stitching, sliceProfile->fix_horrible_extensive_stitching);

	this->setProfileData(&overlap_dual, sliceProfile->overlap_dual);
	this->setProfileData(&object_sink, sliceProfile->object_sink);
}

void SliceProfileForCommon::saveSliceProfileCommon(const wchar_t* filepath)
{
	std::ofstream fs(filepath);

	if (!(fs.is_open())) return;

	fs << ProfileData::outputProfileData(&layer_height);
	fs << ProfileData::outputProfileData(&initial_layer_height);
	fs << ProfileData::outputProfileData(&initial_layer_width_factor);
	fs << ProfileData::outputProfileData(&z_offset_raft);
	fs << ProfileData::outputProfileData(&z_offset_except_raft);
	//fs << ProfileData::outputProfileData(&nozzle_size);
	fs << ProfileData::outputProfileData(&print_bed_temperature);

	fs << ProfileData::outputProfileData(&support_main_cartridge_index);
	fs << ProfileData::outputProfileData(&support_placement);
	fs << ProfileData::outputProfileData(&support_main_pattern);
	fs << ProfileData::outputProfileData(&support_angle);
	fs << ProfileData::outputProfileData(&support_infill_density);
	fs << ProfileData::outputProfileData(&support_xy_distance);
	fs << ProfileData::outputProfileData(&support_z_distance);
	fs << ProfileData::outputProfileData(&support_main_flow);
	fs << ProfileData::outputProfileData(&support_horizontal_expansion);
	fs << ProfileData::outputProfileData(&support_main_speed);
	fs << ProfileData::outputProfileData(&support_main_width_factor);
	fs << ProfileData::outputProfileData(&support_interface_enabled);
	fs << ProfileData::outputProfileData(&support_interface_pattern);
	fs << ProfileData::outputProfileData(&support_interface_roof_layers_count);
	fs << ProfileData::outputProfileData(&support_interface_roof_width_factor);
	fs << ProfileData::outputProfileData(&support_interface_roof_flow);
	fs << ProfileData::outputProfileData(&support_interface_roof_speed);
	fs << ProfileData::outputProfileData(&support_interface_floor_layers_count);
	fs << ProfileData::outputProfileData(&support_interface_floor_width_factor);
	fs << ProfileData::outputProfileData(&support_interface_floor_flow);
	fs << ProfileData::outputProfileData(&support_interface_floor_speed);

	fs << ProfileData::outputProfileData(&platform_adhesion);
	fs << ProfileData::outputProfileData(&adhesion_cartridge_index);
	fs << ProfileData::outputProfileData(&skirt_line_count);
	//fs << ProfileData::outputProfileData(&skirt_width_factor);
	fs << ProfileData::outputProfileData(&skirt_gap);
	fs << ProfileData::outputProfileData(&skirt_minimal_length);
	//fs << ProfileData::outputProfileData(&skirt_speed);
	//fs << ProfileData::outputProfileData(&skirt_flow);
	fs << ProfileData::outputProfileData(&brim_line_count);
	//fs << ProfileData::outputProfileData(&brim_width_factor);
	//fs << ProfileData::outputProfileData(&brim_speed);
	//fs << ProfileData::outputProfileData(&brim_flow);
	fs << ProfileData::outputProfileData(&raft_margin);
	fs << ProfileData::outputProfileData(&raft_line_spacing);
	fs << ProfileData::outputProfileData(&raft_airgap_all);
	fs << ProfileData::outputProfileData(&raft_airgap_initial_layer);
	fs << ProfileData::outputProfileData(&raft_inset_enabled);
	fs << ProfileData::outputProfileData(&raft_inset_offset);
	fs << ProfileData::outputProfileData(&raft_temperature_control);
	//fs << ProfileData::outputProfileData(&raft_fan_speed);
	fs << ProfileData::outputProfileData(&raft_incline_enabled);
	fs << ProfileData::outputProfileData(&raft_base_thickness);
	fs << ProfileData::outputProfileData(&raft_base_line_width);
	fs << ProfileData::outputProfileData(&raft_base_speed);
	fs << ProfileData::outputProfileData(&raft_base_temperature);
	fs << ProfileData::outputProfileData(&raft_interface_thickness);
	fs << ProfileData::outputProfileData(&raft_interface_line_width);
	fs << ProfileData::outputProfileData(&raft_interface_speed);
	fs << ProfileData::outputProfileData(&raft_interface_temperature);
	fs << ProfileData::outputProfileData(&raft_surface_layers);
	fs << ProfileData::outputProfileData(&raft_surface_thickness);
	fs << ProfileData::outputProfileData(&raft_surface_line_width);
	fs << ProfileData::outputProfileData(&raft_surface_speed);
	fs << ProfileData::outputProfileData(&raft_surface_initial_temperature);
	fs << ProfileData::outputProfileData(&raft_surface_last_temperature);

	fs << ProfileData::outputProfileData(&retraction_combing);
	fs << ProfileData::outputProfileData(&toolchange_lowering_bed);

	fs << ProfileData::outputProfileData(&wipe_tower_enabled);
	fs << ProfileData::outputProfileData(&wipe_tower_position);
	fs << ProfileData::outputProfileData(&wipe_tower_infill_density);
	fs << ProfileData::outputProfileData(&wipe_tower_raft_margin);
	fs << ProfileData::outputProfileData(&wipe_tower_base_size);
	fs << ProfileData::outputProfileData(&wipe_tower_base_layer_count);
	fs << ProfileData::outputProfileData(&wipe_tower_outer_size);
	fs << ProfileData::outputProfileData(&wipe_tower_outer_wall_thickness);
	fs << ProfileData::outputProfileData(&wipe_tower_outer_inner_gap);
	//fs << ProfileData::outputProfileData(&wipe_tower_inner_size);
	fs << ProfileData::outputProfileData(&wipe_tower_flow);
	fs << ProfileData::outputProfileData(&wipe_tower_speed);

	fs << ProfileData::outputProfileData(&spiralize);
	fs << ProfileData::outputProfileData(&simple_mode);
	fs << ProfileData::outputProfileData(&fix_horrible_union_all_type_a);
	fs << ProfileData::outputProfileData(&fix_horrible_union_all_type_b);
	fs << ProfileData::outputProfileData(&fix_horrible_use_open_bits);
	fs << ProfileData::outputProfileData(&fix_horrible_extensive_stitching);

	fs << ProfileData::outputProfileData(&overlap_dual);
	fs << ProfileData::outputProfileData(&object_sink);


	////////////////////////////////////////////////////////////////////////////////////////////////


	//fs << ("layer_height(mm):") << (" ") << layer_height.value << (" ") << layer_height.min << (" ") << layer_height.max << ("\n");
	//fs << ("nozzle_size(mm):") << (" ") << nozzle_size.value << (" ") << nozzle_size.min << (" ") << nozzle_size.max << ("\n");
	//fs << ("print_bed_temperature(C):") << (" ") << print_bed_temperature.value << (" ") << print_bed_temperature.min << (" ") << print_bed_temperature.max << ("\n");
	//fs << ("z_offset_raft(mm):") << (" ") << z_offset_raft.value << (" ") << z_offset_raft.min << (" ") << z_offset_raft.max << ("\n");
	//fs << ("z_offset_except_raft(mm):") << (" ") << z_offset_except_raft.value << (" ") << z_offset_except_raft.min << (" ") << z_offset_except_raft.max << ("\n");
	//
	//fs << ("support_placement(0=None,1=Touching_buildplate,2=Everywhere):") << (" ") << support_placement.value << ("\n");
	//fs << ("support_main_cartridge_index:") << (" ") << support_main_cartridge_index.value << ("\n");
	//fs << ("support_main_pattern(0=Grid,1=Line,2=Zigzag):") << (" ") << support_main_pattern.value << ("\n");
	//fs << ("support_angle(deg):") << (" ") << support_angle.value << (" ") << support_angle.min << (" ") << support_angle.max << ("\n");
	//fs << ("support_infill_density(%):") << (" ") << support_infill_density.value << (" ") << support_infill_density.min << (" ") << support_infill_density.max << ("\n");
	//fs << ("support_xy_distance(mm):") << (" ") << support_xy_distance.value << (" ") << support_xy_distance.min << (" ") << support_xy_distance.max << ("\n");
	//fs << ("support_z_distance(mm):") << (" ") << support_z_distance.value << (" ") << support_z_distance.min << (" ") << support_z_distance.max << ("\n");
	//fs << ("support_horizontal_expansion(mm):") << (" ") << support_horizontal_expansion.value << (" ") << support_horizontal_expansion.min << (" ") << support_horizontal_expansion.max << ("\n");
	//fs << ("support_interface_enabled(0=False,1=True):") << (" ") << support_interface_enabled.value << ("\n");
	//fs << ("support_interface_count:") << (" ") << support_interface_count.value << (" ") << support_interface_count.min << (" ") << support_interface_count.max << ("\n");
	//fs << ("support_interface_pattern(0=Lines,1=Concentric):") << (" ") << support_interface_pattern.value << ("\n");
	//
	//fs << ("platform_adhesion(0=None,1=Skirt,2=Brim,3=Raft):") << (" ") << platform_adhesion.value << ("\n");
	//fs << ("wipe_tower_enabled(0=False,1=True):") << (" ") << wipe_tower_enabled.value << ("\n");
	//fs << ("wipe_tower_position:") << (" ") << wipe_tower_position.value << ("\n");
	//fs << ("wipe_tower_infill_density(%):") << (" ") << wipe_tower_infill_density.value << (" ") << wipe_tower_infill_density.min << (" ") << wipe_tower_infill_density.max << ("\n");
	//fs << ("wipe_tower_raft_margin(mm):") << (" ") << wipe_tower_raft_margin.value << (" ") << wipe_tower_raft_margin.min << (" ") << wipe_tower_raft_margin.max << ("\n");
	//fs << ("wipe_tower_filament_flow(%):") << (" ") << wipe_tower_filament_flow.value << (" ") << wipe_tower_filament_flow.min << (" ") << wipe_tower_filament_flow.max << ("\n");
	//fs << ("wipe_tower_base_size(mm):") << (" ") << wipe_tower_base_size.value << (" ") << wipe_tower_base_size.min << (" ") << wipe_tower_base_size.max << ("\n");
	//fs << ("wipe_tower_base_layer_count:") << (" ") << wipe_tower_base_layer_count.value << (" ") << wipe_tower_base_layer_count.min << (" ") << wipe_tower_base_layer_count.max << ("\n");
	//fs << ("wipe_tower_outer_size(mm):") << (" ") << wipe_tower_outer_size.value << (" ") << wipe_tower_outer_size.min << (" ") << wipe_tower_outer_size.max << ("\n");
	//fs << ("wipe_tower_outer_wall_thickness(mm):") << (" ") << wipe_tower_outer_wall_thickness.value << (" ") << wipe_tower_outer_wall_thickness.min << (" ") << wipe_tower_outer_wall_thickness.max << ("\n");
	//fs << ("wipe_tower_outer_inner_gap(mm):") << (" ") << wipe_tower_outer_inner_gap.value << (" ") << wipe_tower_outer_inner_gap.min << (" ") << wipe_tower_outer_inner_gap.max << ("\n");
	////fs << ("wipe_tower_inner_size(mm):") << (" ") << wipe_tower_inner_size.value << (" ") << wipe_tower_inner_size.min << (" ") << wipe_tower_inner_size.max << ("\n");
	////fs << ("ooze_shield(0=False,1=True):") << (" ") << ooze_shield.value << ("\n");

	//fs << ("retraction_combing(0=Off,1=All,2=No_Skin):") << (" ") << retraction_combing.value << ("\n");
	//fs << ("toolchange_lowering_bed(mm):") << (" ") << toolchange_lowering_bed.value << (" ") << toolchange_lowering_bed.min << (" ") << toolchange_lowering_bed.max << ("\n");
	//fs << ("layer0_thickness(mm):") << (" ") << initial_layer_height.value << (" ") << initial_layer_height.min << (" ") << initial_layer_height.max << ("\n");
	//fs << ("layer0_width_factor(%):") << (" ") << initial_layer_width_factor.value << (" ") << initial_layer_width_factor.min << (" ") << initial_layer_width_factor.max << ("\n");
	//fs << ("object_sink(mm):") << (" ") << object_sink.value << ("\n");
	//fs << ("overlap_dual(mm):") << (" ") << overlap_dual.value << ("\n");
	//fs << ("skirt_line_count:") << (" ") << skirt_line_count.value << (" ") << skirt_line_count.min << (" ") << skirt_line_count.max << ("\n");
	//fs << ("skirt_gap(mm):") << (" ") << skirt_gap.value << (" ") << skirt_gap.min << (" ") << skirt_gap.max << ("\n");
	//fs << ("skirt_minimal_length(mm):") << (" ") << skirt_minimal_length.value << (" ") << skirt_minimal_length.min << (" ") << skirt_minimal_length.max << ("\n");
	//fs << ("spiralize(0=False,1=True):") << (" ") << spiralize.value << ("\n");
	//fs << ("simple_mode(0=False,1=True):") << (" ") << simple_mode.value << ("\n");
	//
	//fs << ("brim_line_count:") << (" ") << brim_line_count.value << (" ") << brim_line_count.min << (" ") << brim_line_count.max << ("\n");
	//fs << ("adhesion_cartridge_index:") << (" ") << adhesion_cartridge_index.value << ("\n");
	//fs << ("raft_margin(mm):") << (" ") << raft_margin.value << (" ") << raft_margin.min << (" ") << raft_margin.max << ("\n");
	//fs << ("raft_line_spacing(mm):") << (" ") << raft_line_spacing.value << (" ") << raft_line_spacing.min << (" ") << raft_line_spacing.max << ("\n");
	//fs << ("raft_airgap_all(mm):") << (" ") << raft_airgap_all.value << (" ") << raft_airgap_all.min << (" ") << raft_airgap_all.max << ("\n");
	//fs << ("raft_airgap_initial_layer(mm):") << (" ") << raft_airgap_initial_layer.value << (" ") << raft_airgap_initial_layer.min << (" ") << raft_airgap_initial_layer.max << ("\n");
	//fs << ("raft_inset_enabled(0=Disabled,1=Enabled):") << (" ") << raft_inset_enabled.value << ("\n");
	//fs << ("raft_inset_offset(0mm~1mm):") << (" ") << raft_inset_offset.value << (" ") << raft_inset_offset.min << (" ") << raft_inset_offset.max << ("\n");
	//fs << ("raft_temperature_control(0=Disabled,1=Enabled):") << (" ") << raft_temperature_control.value << ("\n");
	//fs << ("raft_incline_enabled(0=Disabled,1=Enabled):") << (" ") << raft_incline_enabled.value << ("\n");
	//fs << ("raft_base_thickness(mm):") << (" ") << raft_base_thickness.value << (" ") << raft_base_thickness.min << (" ") << raft_base_thickness.max << ("\n");
	//fs << ("raft_base_line_width(mm):") << (" ") << raft_base_line_width.value << (" ") << raft_base_line_width.min << (" ") << raft_base_line_width.max << ("\n");
	//fs << ("raft_base_speed(mm/s):") << (" ") << raft_base_speed.value << (" ") << raft_base_speed.min << (" ") << raft_base_speed.max << ("\n");
	//fs << ("raft_base_temperature(C):") << (" ") << raft_base_temperature.value << (" ") << raft_base_temperature.min << (" ") << raft_base_temperature.max << ("\n");
	//fs << ("raft_interface_thickness(mm):") << (" ") << raft_interface_thickness.value << (" ") << raft_interface_thickness.min << (" ") << raft_interface_thickness.max << ("\n");
	//fs << ("raft_interface_line_width(mm):") << (" ") << raft_interface_line_width.value << (" ") << raft_interface_line_width.min << (" ") << raft_interface_line_width.max << ("\n");
	//fs << ("raft_interface_speed(mm/s):") << (" ") << raft_interface_speed.value << (" ") << raft_interface_speed.min << (" ") << raft_interface_speed.max << ("\n");
	//fs << ("raft_interface_temperature(C):") << (" ") << raft_interface_temperature.value << (" ") << raft_interface_temperature.min << (" ") << raft_interface_temperature.max << ("\n");
	//fs << ("raft_surface_layers:") << (" ") << raft_surface_layers.value << (" ") << raft_surface_layers.min << (" ") << raft_surface_layers.max << ("\n");
	//fs << ("raft_surface_thickness(mm):") << (" ") << raft_surface_thickness.value << (" ") << raft_surface_thickness.min << (" ") << raft_surface_thickness.max << ("\n");
	//fs << ("raft_surface_line_width(mm):") << (" ") << raft_surface_line_width.value << (" ") << raft_surface_line_width.min << (" ") << raft_surface_line_width.max << ("\n");
	//fs << ("raft_surface_speed(mm):") << (" ") << raft_surface_speed.value << (" ") << raft_surface_speed.min << (" ") << raft_surface_speed.max << ("\n");
	//fs << ("raft_surface_initial_temperature(C):") << (" ") << raft_surface_initial_temperature.value << (" ") << raft_surface_initial_temperature.min << (" ") << raft_surface_initial_temperature.max << ("\n");
	//fs << ("raft_surface_last_temperature(C):") << (" ") << raft_surface_last_temperature.value << (" ") << raft_surface_last_temperature.min << (" ") << raft_surface_last_temperature.max << ("\n");
	//
	//fs << ("fix_horrible_union_all_type_a(0=False,1=True):") << (" ") << fix_horrible_union_all_type_a.value << ("\n");
	//fs << ("fix_horrible_union_all_type_b(0=False,1=True):") << (" ") << fix_horrible_union_all_type_b.value << ("\n");
	//fs << ("fix_horrible_use_open_bits(0=False,1=True):") << (" ") << fix_horrible_use_open_bits.value << ("\n");
	//fs << ("fix_horrible_extensive_stitching(0=False,1=True):") << (" ") << fix_horrible_extensive_stitching.value << ("\n");

	fs.close();
}

std::map<std::string, ProfileCommonMapValue*> SliceProfileForCommon::setSliceProfileForCommonMap()
{
	std::map<std::string, ProfileCommonMapValue*> slice_profile_common_map;

	slice_profile_common_map.insert(std::make_pair(layer_height.name_key.c_str(), new ProfileCommonMapValue(this, &layer_height)));
	slice_profile_common_map.insert(std::make_pair(initial_layer_height.name_key.c_str(), new ProfileCommonMapValue(this, &initial_layer_height)));
	slice_profile_common_map.insert(std::make_pair(initial_layer_width_factor.name_key.c_str(), new ProfileCommonMapValue(this, &initial_layer_width_factor)));
	slice_profile_common_map.insert(std::make_pair(z_offset_raft.name_key.c_str(), new ProfileCommonMapValue(this, &z_offset_raft)));
	slice_profile_common_map.insert(std::make_pair(z_offset_except_raft.name_key.c_str(), new ProfileCommonMapValue(this, &z_offset_except_raft)));

	//slice_profile_common_map.insert(std::make_pair(nozzle_size.name_key.c_str(), new ProfileCommonMapValue(this, &nozzle_size)));
	slice_profile_common_map.insert(std::make_pair(print_bed_temperature.name_key.c_str(), new ProfileCommonMapValue(this, &print_bed_temperature)));
	
	slice_profile_common_map.insert(std::make_pair(support_main_cartridge_index.name_key.c_str(), new ProfileCommonMapValue(this, &support_main_cartridge_index)));
	slice_profile_common_map.insert(std::make_pair(support_placement.name_key.c_str(), new ProfileCommonMapValue(this, &support_placement)));
	slice_profile_common_map.insert(std::make_pair(support_main_pattern.name_key.c_str(), new ProfileCommonMapValue(this, &support_main_pattern)));
	slice_profile_common_map.insert(std::make_pair(support_angle.name_key.c_str(), new ProfileCommonMapValue(this, &support_angle)));
	slice_profile_common_map.insert(std::make_pair(support_infill_density.name_key.c_str(), new ProfileCommonMapValue(this, &support_infill_density)));
	slice_profile_common_map.insert(std::make_pair(support_xy_distance.name_key.c_str(), new ProfileCommonMapValue(this, &support_xy_distance)));
	slice_profile_common_map.insert(std::make_pair(support_z_distance.name_key.c_str(), new ProfileCommonMapValue(this, &support_z_distance)));
	slice_profile_common_map.insert(std::make_pair(support_main_flow.name_key.c_str(), new ProfileCommonMapValue(this, &support_main_flow)));
	slice_profile_common_map.insert(std::make_pair(support_horizontal_expansion.name_key.c_str(), new ProfileCommonMapValue(this, &support_horizontal_expansion)));
	slice_profile_common_map.insert(std::make_pair(support_main_speed.name_key.c_str(), new ProfileCommonMapValue(this, &support_main_speed)));
	slice_profile_common_map.insert(std::make_pair(support_main_width_factor.name_key.c_str(), new ProfileCommonMapValue(this, &support_main_width_factor)));
	slice_profile_common_map.insert(std::make_pair(support_interface_enabled.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_enabled)));
	slice_profile_common_map.insert(std::make_pair(support_interface_pattern.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_pattern)));
	slice_profile_common_map.insert(std::make_pair(support_interface_roof_layers_count.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_roof_layers_count)));
	slice_profile_common_map.insert(std::make_pair(support_interface_roof_width_factor.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_roof_width_factor)));
	slice_profile_common_map.insert(std::make_pair(support_interface_roof_flow.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_roof_flow)));
	slice_profile_common_map.insert(std::make_pair(support_interface_roof_speed.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_roof_speed)));
	slice_profile_common_map.insert(std::make_pair(support_interface_floor_layers_count.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_floor_layers_count)));
	slice_profile_common_map.insert(std::make_pair(support_interface_floor_width_factor.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_floor_width_factor)));
	slice_profile_common_map.insert(std::make_pair(support_interface_floor_flow.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_floor_flow)));
	slice_profile_common_map.insert(std::make_pair(support_interface_floor_speed.name_key.c_str(), new ProfileCommonMapValue(this, &support_interface_floor_speed)));

	slice_profile_common_map.insert(std::make_pair(platform_adhesion.name_key.c_str(), new ProfileCommonMapValue(this, &platform_adhesion)));
	slice_profile_common_map.insert(std::make_pair(adhesion_cartridge_index.name_key.c_str(), new ProfileCommonMapValue(this, &adhesion_cartridge_index)));
	slice_profile_common_map.insert(std::make_pair(skirt_line_count.name_key.c_str(), new ProfileCommonMapValue(this, &skirt_line_count)));
	//slice_profile_common_map.insert(std::make_pair(skirt_width_factor.name_key.c_str(), new ProfileCommonMapValue(this, &skirt_width_factor)));
	slice_profile_common_map.insert(std::make_pair(skirt_gap.name_key.c_str(), new ProfileCommonMapValue(this, &skirt_gap)));
	slice_profile_common_map.insert(std::make_pair(skirt_minimal_length.name_key.c_str(), new ProfileCommonMapValue(this, &skirt_minimal_length)));
	//slice_profile_common_map.insert(std::make_pair(skirt_speed.name_key.c_str(), new ProfileCommonMapValue(this, &skirt_speed)));
	//slice_profile_common_map.insert(std::make_pair(skirt_flow.name_key.c_str(), new ProfileCommonMapValue(this, &skirt_flow)));
	slice_profile_common_map.insert(std::make_pair(brim_line_count.name_key.c_str(), new ProfileCommonMapValue(this, &brim_line_count)));
	//slice_profile_common_map.insert(std::make_pair(brim_width_factor.name_key.c_str(), new ProfileCommonMapValue(this, &brim_width_factor)));
	//slice_profile_common_map.insert(std::make_pair(brim_speed.name_key.c_str(), new ProfileCommonMapValue(this, &brim_speed)));
	//slice_profile_common_map.insert(std::make_pair(brim_flow.name_key.c_str(), new ProfileCommonMapValue(this, &brim_flow)));

	slice_profile_common_map.insert(std::make_pair(raft_margin.name_key.c_str(), new ProfileCommonMapValue(this, &raft_margin)));
	slice_profile_common_map.insert(std::make_pair(raft_line_spacing.name_key.c_str(), new ProfileCommonMapValue(this, &raft_line_spacing)));
	slice_profile_common_map.insert(std::make_pair(raft_airgap_all.name_key.c_str(), new ProfileCommonMapValue(this, &raft_airgap_all)));
	slice_profile_common_map.insert(std::make_pair(raft_airgap_initial_layer.name_key.c_str(), new ProfileCommonMapValue(this, &raft_airgap_initial_layer)));
	slice_profile_common_map.insert(std::make_pair(raft_inset_offset.name_key.c_str(), new ProfileCommonMapValue(this, &raft_inset_offset)));
	slice_profile_common_map.insert(std::make_pair(raft_inset_enabled.name_key.c_str(), new ProfileCommonMapValue(this, &raft_inset_enabled)));
	slice_profile_common_map.insert(std::make_pair(raft_temperature_control.name_key.c_str(), new ProfileCommonMapValue(this, &raft_temperature_control)));
	//slice_profile_common_map.insert(std::make_pair(raft_fan_speed.name_key.c_str(), new ProfileCommonMapValue(this, &raft_fan_speed)));
	slice_profile_common_map.insert(std::make_pair(raft_incline_enabled.name_key.c_str(), new ProfileCommonMapValue(this, &raft_incline_enabled)));
	slice_profile_common_map.insert(std::make_pair(raft_base_thickness.name_key.c_str(), new ProfileCommonMapValue(this, &raft_base_thickness)));
	slice_profile_common_map.insert(std::make_pair(raft_base_line_width.name_key.c_str(), new ProfileCommonMapValue(this, &raft_base_line_width)));
	slice_profile_common_map.insert(std::make_pair(raft_base_speed.name_key.c_str(), new ProfileCommonMapValue(this, &raft_base_speed)));
	slice_profile_common_map.insert(std::make_pair(raft_base_temperature.name_key.c_str(), new ProfileCommonMapValue(this, &raft_base_temperature)));
	slice_profile_common_map.insert(std::make_pair(raft_interface_thickness.name_key.c_str(), new ProfileCommonMapValue(this, &raft_interface_thickness)));
	slice_profile_common_map.insert(std::make_pair(raft_interface_line_width.name_key.c_str(), new ProfileCommonMapValue(this, &raft_interface_line_width)));
	slice_profile_common_map.insert(std::make_pair(raft_interface_speed.name_key.c_str(), new ProfileCommonMapValue(this, &raft_interface_speed)));
	slice_profile_common_map.insert(std::make_pair(raft_interface_temperature.name_key.c_str(), new ProfileCommonMapValue(this, &raft_interface_temperature)));
	slice_profile_common_map.insert(std::make_pair(raft_surface_layers.name_key.c_str(), new ProfileCommonMapValue(this, &raft_surface_layers)));
	slice_profile_common_map.insert(std::make_pair(raft_surface_thickness.name_key.c_str(), new ProfileCommonMapValue(this, &raft_surface_thickness)));
	slice_profile_common_map.insert(std::make_pair(raft_surface_line_width.name_key.c_str(), new ProfileCommonMapValue(this, &raft_surface_line_width)));
	slice_profile_common_map.insert(std::make_pair(raft_surface_speed.name_key.c_str(), new ProfileCommonMapValue(this, &raft_surface_speed)));
	slice_profile_common_map.insert(std::make_pair(raft_surface_initial_temperature.name_key.c_str(), new ProfileCommonMapValue(this, &raft_surface_initial_temperature)));
	slice_profile_common_map.insert(std::make_pair(raft_surface_last_temperature.name_key.c_str(), new ProfileCommonMapValue(this, &raft_surface_last_temperature)));

	slice_profile_common_map.insert(std::make_pair(retraction_combing.name_key.c_str(), new ProfileCommonMapValue(this, &retraction_combing)));

	slice_profile_common_map.insert(std::make_pair(toolchange_lowering_bed.name_key.c_str(), new ProfileCommonMapValue(this, &toolchange_lowering_bed)));

	slice_profile_common_map.insert(std::make_pair(wipe_tower_enabled.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_enabled)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_position.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_position)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_infill_density.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_infill_density)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_raft_margin.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_raft_margin)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_base_size.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_base_size)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_base_layer_count.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_base_layer_count)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_outer_size.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_outer_size)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_outer_wall_thickness.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_outer_wall_thickness)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_outer_inner_gap.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_outer_inner_gap)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_flow.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_flow)));
	slice_profile_common_map.insert(std::make_pair(wipe_tower_speed.name_key.c_str(), new ProfileCommonMapValue(this, &wipe_tower_speed)));

	slice_profile_common_map.insert(std::make_pair(spiralize.name_key.c_str(), new ProfileCommonMapValue(this, &spiralize)));
	slice_profile_common_map.insert(std::make_pair(simple_mode.name_key.c_str(), new ProfileCommonMapValue(this, &simple_mode)));
	slice_profile_common_map.insert(std::make_pair(fix_horrible_union_all_type_a.name_key.c_str(), new ProfileCommonMapValue(this, &fix_horrible_union_all_type_a)));
	slice_profile_common_map.insert(std::make_pair(fix_horrible_union_all_type_b.name_key.c_str(), new ProfileCommonMapValue(this, &fix_horrible_union_all_type_b)));
	slice_profile_common_map.insert(std::make_pair(fix_horrible_use_open_bits.name_key.c_str(), new ProfileCommonMapValue(this, &fix_horrible_use_open_bits)));
	slice_profile_common_map.insert(std::make_pair(fix_horrible_extensive_stitching.name_key.c_str(), new ProfileCommonMapValue(this, &fix_horrible_extensive_stitching)));

	return slice_profile_common_map;
}
