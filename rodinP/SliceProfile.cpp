#include "stdafx.h"

#include "SliceProfile.h"
#include "CustomProfileList.h"
#include "GCodeParser.h"
#define PROFILE_NUM 88

TemperatureLayerSetPoint::TemperatureLayerSetPoint(int pLayer, int pTemperature)
{
	this->layerNr = pLayer;
	this->temperature = pTemperature;
}
TemperatureLayerSetPoint::TemperatureLayerSetPoint() { }
TemperatureLayerSetPoint::~TemperatureLayerSetPoint() { }


ProfileMapValue::ProfileMapValue(SliceProfile* _slice_profile, ProfileDataD* _profile_data_D)
{
	this->profile_data_D = _profile_data_D;
	this->slice_profile = _slice_profile;
	this->profile_type = ProfileDataType::_double;
}
ProfileMapValue::ProfileMapValue(SliceProfile* _slice_profile, ProfileDataI* _profile_data_I)
{
	this->profile_data_I = _profile_data_I;
	this->slice_profile = _slice_profile;
	this->profile_type = ProfileDataType::_int;
}
ProfileMapValue::ProfileMapValue(SliceProfile* _slice_profile, ProfileDataB* _profile_data_B)
{
	this->profile_data_B = _profile_data_B;
	this->slice_profile = _slice_profile;
	this->profile_type = ProfileDataType::_bool;
}
ProfileMapValue::ProfileMapValue(SliceProfile* _slice_profile, ProfileDataS* _profile_data_S)
{
	this->profile_data_S = _profile_data_S;
	this->slice_profile = _slice_profile;
	this->profile_type = ProfileDataType::_QString;
}

void ProfileMapValue::setProfileDataForProfileMap(std::string _profile_value)
{
	switch (profile_type)
	{
	case ProfileDataType::_double:
		slice_profile->setProfileData(profile_data_D, _profile_value);
		break;
	case ProfileDataType::_int:
		slice_profile->setProfileData(profile_data_I, _profile_value);
		break;
	case ProfileDataType::_bool:
		slice_profile->setProfileData(profile_data_B, _profile_value);
		break;
	case ProfileDataType::_QString:
		slice_profile->setProfileData(profile_data_S, _profile_value);
		break;
	}
}

std::string ProfileMapValue::getProfileDataNameKey()
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

std::string ProfileMapValue::getProfileDataUnitKey()
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

std::string ProfileMapValue::getProfileDataValueStr()
{
	std::string str_value;

	switch (profile_type)
	{
	case ProfileDataType::_double:
		str_value = Generals::to_string_with_precision(profile_data_D->value, 2);
		break;
	case ProfileDataType::_int:
		str_value = std::to_string(profile_data_I->value);
		break;
	case ProfileDataType::_bool:
		str_value = std::to_string(profile_data_B->value);
		break;
	case ProfileDataType::_QString:
		str_value = profile_data_S->value.toStdString();
		break;
	}

	return str_value;
}


SliceProfile::SliceProfile()
{
	//slice profile key-value map generating..//
	//setSliceProfileMap();
	//setSliceProfileMap_old();
}

SliceProfile::~SliceProfile()
{ }

void SliceProfile::setProfileData(ProfileDataD* data, std::string value)
{
	getProfileValue(value, &data->value, &data->min, &data->max);
}

void SliceProfile::setProfileData(ProfileDataB* data, std::string value)
{
	getProfileValue(value, &data->value);
}

void SliceProfile::setProfileData(ProfileDataI* data, std::string value)
{
	getProfileValue(value, &data->value, &data->min, &data->max);
}

void SliceProfile::setProfileData(ProfileDataS* data, std::string value)
{
	data->value = QString::fromStdString(value).trimmed();
}

void SliceProfile::getProfileValue(std::string fromValue, double* value, double* min, double* max)
{
	QString l_value, l_min, l_max;
	getProfileValue(fromValue, &l_value, &l_min, &l_max);

	if (l_value != "") *value = QString(l_value).toDouble();
	if (l_min != "") *min = QString(l_min).toDouble();
	if (l_max != "") *max = QString(l_max).toDouble();
}

void SliceProfile::getProfileValue(std::string fromValue, bool* value)
{
	QString l_value;
	getProfileValue(fromValue, &l_value);

	if (l_value != "") *value = (bool)QString(l_value).toInt();
}

void SliceProfile::getProfileValue(std::string fromValue, int* value, int* min, int* max)
{
	QString l_value, l_min, l_max;
	getProfileValue(fromValue, &l_value, &l_min, &l_max);

	if (l_value != "") *value = QString(l_value).toInt();
	if (l_min != "") *min = QString(l_min).toInt();
	if (l_max != "") *max = QString(l_max).toInt();
}

void SliceProfile::getProfileValue(std::string fromValue, QString* value, QString* min, QString* max)
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
			QString tempMsg = "[Error] Profile min/max value error : min value - " + *min + "- current value - " + *value;
			Logger() << tempMsg;
			std::cout << tempMsg.toStdString() << "\n";
		}
		if ((*value).toDouble() > (*max).toDouble())
		{
			QString tempMsg = "[Error] Profile min/max value error : min value - " + *min + "- current value - " + *value;
			Logger() << tempMsg;
			std::cout << tempMsg.toStdString() << "\n";
		}
	}
	iss.clear();
}

bool SliceProfile::loadSliceProfile(const wchar_t* _filePath)
{
	bool isReplaced = false;

	std::ifstream fs(_filePath);

	if (fs.is_open())
	{
		std::cout << "Default profile loading complete!!" << "\n" << QString::fromWCharArray(_filePath).toStdString() << "\n";
		Logger() << "Profile loading complete.";
	}
	else
	{
		std::cout << "Default profile loading failure!!" << "\n" << QString::fromWCharArray(_filePath).toStdString() << "\n";
		Logger() << "Profile loading failure";
		return false;
	}
	
	slice_profile_map = setSliceProfileMap();
	slice_profile_map_old = setSliceProfileMap_old();


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

		//////////////////////////////////////////////////////////////////////////////////////
		// key finding at map //
		std::map<std::string, ProfileMapValue*>::const_iterator it = slice_profile_map.find(name);
		if (it != slice_profile_map.end())
		{
			it->second->setProfileDataForProfileMap(profile_value);
		}
		else if (name.find("temperature_layer_") == 0)
			continue;
		else
		{
			QString tempMsg = "[Warning] Missing slice profile - " + QString::fromStdString(name) + ": " + QString::fromStdString(profile_value);
			Logger() << tempMsg;
			std::cout << tempMsg.toStdString() << "\n";
		}
		//////////////////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////////////////
		//for old slice profile..// --> 나중에는 importProfile 로 가야함..//
		std::map<std::string, ProfileMapValue*>::const_iterator it_old = slice_profile_map_old.find(name);
		if (it_old != slice_profile_map_old.end())
		{
			it_old->second->setProfileDataForProfileMap(profile_value);

			//matching old to new//
			//if (it_old->second->getProfileDataNameKey() == "solid_layer_thickness") isReplaced = replaceProfileDataValue(&solid_layer_thickness, &top_bottom_thickness);
			//else if (it_old->second->getProfileDataNameKey() == "fill_density") isReplaced = replaceProfileDataValue(&fill_density, &infill_density);
			//else if (it_old->second->getProfileDataNameKey() == "printer_speed") isReplaced = replaceProfileDataValue(&printer_speed, &print_speed);
			//else if (it_old->second->getProfileDataNameKey() == "support_cartridge_index") isReplaced = replaceProfileDataValue(&support_cartridge_index, &support_main_cartridge_index);
			//else if (it_old->second->getProfileDataNameKey() == "support_pattern") isReplaced = replaceProfileDataValue(&support_pattern, &support_main_pattern);
			//else if (it_old->second->getProfileDataNameKey() == "support_fill_rate") isReplaced = replaceProfileDataValue(&support_fill_rate, &support_infill_density);
			//else if (it_old->second->getProfileDataNameKey() == "support_interface_count") 
			//	isReplaced = (replaceProfileDataValue(&support_interface_count, &support_interface_roof_layers_count) && replaceProfileDataValue(&support_interface_count, &support_interface_floor_layers_count));
			//else if (it_old->second->getProfileDataNameKey() == "wipe_tower_fill_density") isReplaced = replaceProfileDataValue(&wipe_tower_fill_density, &wipe_tower_infill_density);
			//else if (it_old->second->getProfileDataNameKey() == "wipe_tower_filament_flow") isReplaced = replaceProfileDataValue(&wipe_tower_filament_flow, &wipe_tower_flow);
			//else if (it_old->second->getProfileDataNameKey() == "filament_flow") isReplaced = replaceProfileDataValue(&filament_flow, &overall_flow);
			//else if (it_old->second->getProfileDataNameKey() == "layer0_filament_flow") isReplaced = replaceProfileDataValue(&layer0_filament_flow, &initial_layer_flow);
			//else if (it_old->second->getProfileDataNameKey() == "layer0_thickness") isReplaced = replaceProfileDataValue(&layer0_thickness, &initial_layer_height);
			//else if (it_old->second->getProfileDataNameKey() == "layer0_width_factor") isReplaced = replaceProfileDataValue(&layer0_width_factor, &initial_layer_width_factor);
			//else if (it_old->second->getProfileDataNameKey() == "bottom_layer_speed") isReplaced = replaceProfileDataValue(&bottom_layer_speed, &initial_layer_speed);
			//else if (it_old->second->getProfileDataNameKey() == "solidarea_speed") isReplaced = replaceProfileDataValue(&solidarea_speed, &top_bottom_speed);
			//else if (it_old->second->getProfileDataNameKey() == "inset0_speed") isReplaced = replaceProfileDataValue(&inset0_speed, &outer_wall_speed);
			//else if (it_old->second->getProfileDataNameKey() == "insetx_speed") isReplaced = replaceProfileDataValue(&insetx_speed, &inner_wall_speed);
			//else if (it_old->second->getProfileDataNameKey() == "raft_airgap_layer0") isReplaced = replaceProfileDataValue(&raft_airgap_layer0, &raft_airgap_initial_layer);
			//else if (it_old->second->getProfileDataNameKey() == "raft_base_linewidth") isReplaced = replaceProfileDataValue(&raft_base_linewidth, &raft_base_line_width);
			//else if (it_old->second->getProfileDataNameKey() == "raft_interface_linewidth") isReplaced = replaceProfileDataValue(&raft_interface_linewidth, &raft_interface_line_width);
			//else if (it_old->second->getProfileDataNameKey() == "raft_surface_linewidth") isReplaced = replaceProfileDataValue(&raft_surface_linewidth, &raft_surface_line_width);

			if (name == "solid_layer_thickness") isReplaced = replaceProfileDataValue(&solid_layer_thickness, &top_bottom_thickness);
			else if (name == "fill_density") isReplaced = replaceProfileDataValue(&fill_density, &infill_density);
			else if (name == "printer_speed") isReplaced = replaceProfileDataValue(&printer_speed, &print_speed);
			else if (name == "support_cartridge_index") isReplaced = replaceProfileDataValue(&support_cartridge_index, &support_main_cartridge_index);
			else if (name == "support_pattern") isReplaced = replaceProfileDataValue(&support_pattern, &support_main_pattern);
			else if (name == "support_fill_rate") isReplaced = replaceProfileDataValue(&support_fill_rate, &support_infill_density);
			else if (name == "support_interface_count")
				isReplaced = (replaceProfileDataValue(&support_interface_count, &support_interface_roof_layers_count) && replaceProfileDataValue(&support_interface_count, &support_interface_floor_layers_count));
			else if (name == "wipe_tower_fill_density") isReplaced = replaceProfileDataValue(&wipe_tower_fill_density, &wipe_tower_infill_density);
			else if (name == "wipe_tower_filament_flow") isReplaced = replaceProfileDataValue(&wipe_tower_filament_flow, &wipe_tower_flow);
			else if (name == "filament_flow") isReplaced = replaceProfileDataValue(&filament_flow, &overall_flow);
			else if (name == "layer0_filament_flow") isReplaced = replaceProfileDataValue(&layer0_filament_flow, &initial_layer_flow);
			else if (name == "layer0_thickness") isReplaced = replaceProfileDataValue(&layer0_thickness, &initial_layer_height);
			else if (name == "layer0_width_factor") isReplaced = replaceProfileDataValue(&layer0_width_factor, &initial_layer_width_factor);
			else if (name == "bottom_layer_speed") isReplaced = replaceProfileDataValue(&bottom_layer_speed, &initial_layer_speed);
			else if (name == "solidarea_speed") isReplaced = replaceProfileDataValue(&solidarea_speed, &top_bottom_speed);
			else if (name == "inset0_speed") isReplaced = replaceProfileDataValue(&inset0_speed, &outer_wall_speed);
			else if (name == "insetx_speed") isReplaced = replaceProfileDataValue(&insetx_speed, &inner_wall_speed);
			else if (name == "raft_airgap_layer0") isReplaced = replaceProfileDataValue(&raft_airgap_layer0, &raft_airgap_initial_layer);
			else if (name == "raft_base_linewidth") isReplaced = replaceProfileDataValue(&raft_base_linewidth, &raft_base_line_width);
			else if (name == "raft_interface_linewidth") isReplaced = replaceProfileDataValue(&raft_interface_linewidth, &raft_interface_line_width);
			else if (name == "raft_surface_linewidth") isReplaced = replaceProfileDataValue(&raft_surface_linewidth, &raft_surface_line_width);

			if (isReplaced)
			{
				QString tempMsg = "Success converting old slice_profile to new slice_profile - " + QString::fromStdString(name) + ": " + QString::fromStdString(profile_value);
				Logger() << tempMsg;
				std::cout << tempMsg.toStdString() << "\n";
			}
			else
			{
				QString tempMsg = "[Warning] Fail converting old slice_profile to new slice_profile - " + QString::fromStdString(name) + ": " + QString::fromStdString(profile_value);
				Logger() << tempMsg;
				std::cout << tempMsg.toStdString() << "\n";

			}
		}
			   		 	  
		//////////////////////////////////////////////////////////////////////////////////////


	}

	//isReplaced --> new profile defualt setting..//
	if (isReplaced)
	{
		replaceProfileDataValue(&initial_layer_width_factor, &wall_width_factor);
		replaceProfileDataValue(&initial_layer_width_factor, &infill_width_factor);
		replaceProfileDataValue(&initial_layer_width_factor, &top_bottom_width_factor);
		replaceProfileDataValue(&initial_layer_width_factor, &support_main_width_factor);
		replaceProfileDataValue(&initial_layer_width_factor, &support_interface_roof_width_factor);
		replaceProfileDataValue(&initial_layer_width_factor, &support_interface_floor_width_factor);

		replaceProfileDataValue(&overall_flow, &initial_layer_flow);
		replaceProfileDataValue(&overall_flow, &infill_flow);
		replaceProfileDataValue(&overall_flow, &outer_wall_flow);
		replaceProfileDataValue(&overall_flow, &inner_wall_flow);
		replaceProfileDataValue(&overall_flow, &top_bottom_flow);
		replaceProfileDataValue(&overall_flow, &support_main_flow);
		replaceProfileDataValue(&overall_flow, &support_interface_roof_flow);
		replaceProfileDataValue(&overall_flow, &support_interface_floor_flow);
		replaceProfileDataValue(&overall_flow, &wipe_tower_flow);

		replaceProfileDataValue(&print_speed, &support_main_speed);
		replaceProfileDataValue(&print_speed, &support_interface_roof_speed);
		replaceProfileDataValue(&print_speed, &support_interface_floor_speed);
		replaceProfileDataValue(&print_speed, &wipe_tower_speed);

		//at old slice profile, default value 0.0 is needed to replaced..//
		if (layer0_thickness.value == 0.0)
			replaceProfileDataValue(&layer_height, &initial_layer_height);
		if (inset0_speed.value == 0.0)
			replaceProfileDataValue(&print_speed, &outer_wall_speed);
		if (insetx_speed.value == 0.0)
			replaceProfileDataValue(&print_speed, &inner_wall_speed);
		if (solidarea_speed.value == 0.0)
			replaceProfileDataValue(&print_speed, &top_bottom_speed);
		if (infill_speed.value == 0.0)
			replaceProfileDataValue(&print_speed, &infill_speed);

		slower_layers_count.value = 4;
		slower_layers_count.min = 0;
		slower_layers_count.max = 10;
	}


	if (profile_name.value != "")
	{
		CustomProfileList customProfile;
		std::vector<Generals::ProfileList> l_profileList = customProfile.getProfileList(true, true);
		foreach(Generals::ProfileList a, l_profileList)
		{
			if (profile_name.value == a.name)
			{
				flag_custom = a.custom;
				break;
			}
		}
	}

	//temperature_setpoint_layer_number setting.. --> related to machine parameter //
	//layer number는 0이상이어야 함.. layer 0에서의 온도설정은 nozzle temperature에서 함.//
	temperature_setpoint_layer_number.min = 1.0;
	temperature_setpoint_layer_number.max = Profile::getMachineHeight_calculated() / layer_height.min;

	fs.close();

	return true;

}

void SliceProfile::saveSliceProfile(const wchar_t* filepath)
{
	std::ofstream fs(filepath);

	if (!(fs.is_open())) return;
	
	//fs << ProfileData::outputProfileData(&profile_version);                                 // new ProfileMapValue(this, &profile_version)));
	fs << ProfileData::outputProfileData(&profile_name);                                 // new ProfileMapValue(this, &profile_name)));
	fs << ProfileData::outputProfileData(&filament_material);                                 // new ProfileMapValue(this, &filament_material)));
	fs << ProfileData::outputProfileData(&filament_diameter);                                 // new ProfileMapValue(this, &filament_diameter)));

	fs << ProfileData::outputProfileData(&layer_height);                                 // new ProfileMapValue(this, &layer_height)));
	fs << ProfileData::outputProfileData(&initial_layer_height);                                 // new ProfileMapValue(this, &initial_layer_height)));
	fs << ProfileData::outputProfileData(&initial_layer_width_factor);                                 // new ProfileMapValue(this, &initial_layer_width_factor)));
	fs << ProfileData::outputProfileData(&wall_width_factor);                                 // new ProfileMapValue(this, &outer_wall_width_factor)));
	fs << ProfileData::outputProfileData(&infill_width_factor);                                 // new ProfileMapValue(this, &infill_width_factor)));
	fs << ProfileData::outputProfileData(&top_bottom_width_factor);                                 // new ProfileMapValue(this, &top_bottom_width_factor)));
	fs << ProfileData::outputProfileData(&z_offset_raft);                                 // new ProfileMapValue(this, &z_offset_raft)));
	fs << ProfileData::outputProfileData(&z_offset_except_raft);                                 // new ProfileMapValue(this, &z_offset_except_raft)));

	fs << ProfileData::outputProfileData(&wall_thickness);                                 // new ProfileMapValue(this, &wall_thickness)));
	fs << ProfileData::outputProfileData(&top_bottom_thickness);                                 // new ProfileMapValue(this, &top_bottom_thickness)));
	fs << ProfileData::outputProfileData(&solid_top);                                 // new ProfileMapValue(this, &solid_top)));
	fs << ProfileData::outputProfileData(&solid_bottom);                                 // new ProfileMapValue(this, &solid_bottom)));
	fs << ProfileData::outputProfileData(&wall_printing_direction);                                 // new ProfileMapValue(this, &wall_printing_direction)));

	fs << ProfileData::outputProfileData(&print_temperature);                                 // new ProfileMapValue(this, &print_temperature)));
	fs << ProfileData::outputProfileData(&print_bed_temperature);                                 // new ProfileMapValue(this, &print_bed_temperature)));
	fs << ProfileData::outputProfileData(&temperature_layer_setting_enabled);                                 // new ProfileMapValue(this, &temperature_layer_setting_enabled)));

	fs << ProfileData::outputProfileData(&overall_flow_control_enabled);                                
	fs << ProfileData::outputProfileData(&overall_flow);                                 // new ProfileMapValue(this, &overall_flow)));
	fs << ProfileData::outputProfileData(&initial_layer_flow);                                 // new ProfileMapValue(this, &initial_layer_flow)));
	fs << ProfileData::outputProfileData(&infill_flow);                                 // new ProfileMapValue(this, &infill_flow)));
	fs << ProfileData::outputProfileData(&outer_wall_flow);                                 // new ProfileMapValue(this, &outer_wall_flow)));
	fs << ProfileData::outputProfileData(&inner_wall_flow);                                 // new ProfileMapValue(this, &inner_wall_flow)));
	fs << ProfileData::outputProfileData(&top_bottom_flow);                                 // new ProfileMapValue(this, &top_bottom_flow)));

	fs << ProfileData::outputProfileData(&support_main_cartridge_index);                                 // new ProfileMapValue(this, &support_main_cartridge_index)));
	fs << ProfileData::outputProfileData(&support_placement);                                 // new ProfileMapValue(this, &support_placement)));
	fs << ProfileData::outputProfileData(&support_main_pattern);                                 // new ProfileMapValue(this, &support_main_pattern)));
	fs << ProfileData::outputProfileData(&support_angle);                                 // new ProfileMapValue(this, &support_angle)));
	fs << ProfileData::outputProfileData(&support_infill_density);                                 // new ProfileMapValue(this, &support_infill_density)));
	fs << ProfileData::outputProfileData(&support_xy_distance);                                 // new ProfileMapValue(this, &support_xy_distance)));
	fs << ProfileData::outputProfileData(&support_z_distance);                                 // new ProfileMapValue(this, &support_z_distance)));
	fs << ProfileData::outputProfileData(&support_main_flow);                                 // new ProfileMapValue(this, &support_main_flow)));
	fs << ProfileData::outputProfileData(&support_horizontal_expansion);                                 // new ProfileMapValue(this, &support_horizontal_expansion)));
	fs << ProfileData::outputProfileData(&support_main_speed);                                 // new ProfileMapValue(this, &support_main_print_speed)));
	fs << ProfileData::outputProfileData(&support_main_width_factor);                                 // new ProfileMapValue(this, &support_main_width_factor)));
	fs << ProfileData::outputProfileData(&support_interface_enabled);                                 // new ProfileMapValue(this, &support_interface_enabled)));
	fs << ProfileData::outputProfileData(&support_interface_pattern);                                 // new ProfileMapValue(this, &support_interface_pattern)));
	fs << ProfileData::outputProfileData(&support_interface_roof_layers_count);
	fs << ProfileData::outputProfileData(&support_interface_roof_width_factor);                                 // new ProfileMapValue(this, &support_interface_roof_width_factor)));
	fs << ProfileData::outputProfileData(&support_interface_roof_flow);                                 // new ProfileMapValue(this, &support_interface_roof_flow)));
	fs << ProfileData::outputProfileData(&support_interface_roof_speed);                                 // new ProfileMapValue(this, &support_interface_roof_speed)));
	fs << ProfileData::outputProfileData(&support_interface_floor_layers_count);                                 // new ProfileMapValue(this, &support_interface_roof_width_factor)));
	fs << ProfileData::outputProfileData(&support_interface_floor_width_factor);                                 // new ProfileMapValue(this, &support_interface_roof_width_factor)));
	fs << ProfileData::outputProfileData(&support_interface_floor_flow);                                 // new ProfileMapValue(this, &support_interface_floor_flow)));
	fs << ProfileData::outputProfileData(&support_interface_floor_speed);                                 // new ProfileMapValue(this, &support_interface_floor_speed)));

	fs << ProfileData::outputProfileData(&platform_adhesion);                                 // new ProfileMapValue(this, &platform_adhesion)));
	fs << ProfileData::outputProfileData(&adhesion_cartridge_index);                                 // new ProfileMapValue(this, &adhesion_cartridge_index)));
	fs << ProfileData::outputProfileData(&skirt_line_count);                                 // new ProfileMapValue(this, &skirt_line_count)));
	//fs << ProfileData::outputProfileData(&skirt_width_factor);                                 // new ProfileMapValue(this, &skirt_width_factor)));
	fs << ProfileData::outputProfileData(&skirt_gap);                                 // new ProfileMapValue(this, &skirt_gap)));
	fs << ProfileData::outputProfileData(&skirt_minimal_length);                                 // new ProfileMapValue(this, &skirt_minimal_length)));
	//fs << ProfileData::outputProfileData(&skirt_speed);                                 // new ProfileMapValue(this, &skirt_speed)));
	//fs << ProfileData::outputProfileData(&skirt_flow);                                 // new ProfileMapValue(this, &skirt_flow)));
	fs << ProfileData::outputProfileData(&brim_line_count);                                 // new ProfileMapValue(this, &brim_line_count)));
	//fs << ProfileData::outputProfileData(&brim_width_factor);                                 // new ProfileMapValue(this, &brim_width_factor)));
	//fs << ProfileData::outputProfileData(&brim_speed);                                 // new ProfileMapValue(this, &brim_speed)));
	//fs << ProfileData::outputProfileData(&brim_flow);                                 // new ProfileMapValue(this, &brim_flow)));

	fs << ProfileData::outputProfileData(&raft_margin);                                 // new ProfileMapValue(this, &raft_margin)));
	fs << ProfileData::outputProfileData(&raft_line_spacing);                                 // new ProfileMapValue(this, &raft_line_spacing)));
	fs << ProfileData::outputProfileData(&raft_airgap_all);                                 // new ProfileMapValue(this, &raft_airgap_all)));
	fs << ProfileData::outputProfileData(&raft_airgap_initial_layer);                                 // new ProfileMapValue(this, &raft_airgap_initial_layer)));
	fs << ProfileData::outputProfileData(&raft_inset_enabled);                                 // new ProfileMapValue(this, &raft_inset_enabled)));
	fs << ProfileData::outputProfileData(&raft_inset_offset);                                 // new ProfileMapValue(this, &raft_inset_offset)));
	fs << ProfileData::outputProfileData(&raft_temperature_control);                                 // new ProfileMapValue(this, &raft_temperature_control)));
	//fs << ProfileData::outputProfileData(&raft_fan_speed);                                 // new ProfileMapValue(this, &raft_fan_speed)));
	fs << ProfileData::outputProfileData(&raft_incline_enabled);                                 // new ProfileMapValue(this, &raft_incline_enabled)));
	fs << ProfileData::outputProfileData(&raft_base_thickness);                                 // new ProfileMapValue(this, &raft_base_thickness)));
	fs << ProfileData::outputProfileData(&raft_base_line_width);                                 // new ProfileMapValue(this, &raft_base_line_width)));
	fs << ProfileData::outputProfileData(&raft_base_speed);                                 // new ProfileMapValue(this, &raft_base_speed)));
	fs << ProfileData::outputProfileData(&raft_base_temperature);                                 // new ProfileMapValue(this, &raft_base_temperature)));
	fs << ProfileData::outputProfileData(&raft_interface_thickness);                                 // new ProfileMapValue(this, &raft_interface_thickness)));
	fs << ProfileData::outputProfileData(&raft_interface_line_width);                                 // new ProfileMapValue(this, &raft_interface_line_width)));
	fs << ProfileData::outputProfileData(&raft_interface_speed);                                 // new ProfileMapValue(this, &raft_interface_speed)));
	fs << ProfileData::outputProfileData(&raft_interface_temperature);                                 // new ProfileMapValue(this, &raft_interface_temperature)));
	fs << ProfileData::outputProfileData(&raft_surface_layers);                                 // new ProfileMapValue(this, &raft_surface_layers)));
	fs << ProfileData::outputProfileData(&raft_surface_thickness);                                 // new ProfileMapValue(this, &raft_surface_thickness)));
	fs << ProfileData::outputProfileData(&raft_surface_line_width);                                 // new ProfileMapValue(this, &raft_surface_line_width)));
	fs << ProfileData::outputProfileData(&raft_surface_speed);                                 // new ProfileMapValue(this, &raft_surface_speed)));
	fs << ProfileData::outputProfileData(&raft_surface_initial_temperature);                                 // new ProfileMapValue(this, &raft_surface_initial_temperature)));
	fs << ProfileData::outputProfileData(&raft_surface_last_temperature);                                 // new ProfileMapValue(this, &raft_surface_last_temperature)));

	fs << ProfileData::outputProfileData(&infill_density);                                 // new ProfileMapValue(this, &infill_density)));
	fs << ProfileData::outputProfileData(&infill_overlap);                                 // new ProfileMapValue(this, &infill_overlap)));
	fs << ProfileData::outputProfileData(&infill_pattern);                                 // new ProfileMapValue(this, &infill_pattern)));
	fs << ProfileData::outputProfileData(&infill_before_wall);                                 // new ProfileMapValue(this, &infill_before_wall)));
	fs << ProfileData::outputProfileData(&skin_outline);                                 // new ProfileMapValue(this, &skin_outline)));
	fs << ProfileData::outputProfileData(&skin_type);                                 // new ProfileMapValue(this, &skin_type)));
	fs << ProfileData::outputProfileData(&skin_removal_width_top);                                 // new ProfileMapValue(this, &skin_removal_width_top)));
	fs << ProfileData::outputProfileData(&skin_removal_width_bottom);                                 // new ProfileMapValue(this, &skin_removal_width_bottom)));
	fs << ProfileData::outputProfileData(&skin_overlap);                                 // new ProfileMapValue(this, &skin_overlap)));


	fs << ProfileData::outputProfileData(&print_speed);                                 // new ProfileMapValue(this, &print_speed)));
	fs << ProfileData::outputProfileData(&initial_layer_speed);                                 // new ProfileMapValue(this, &initial_layer_speed)));
	fs << ProfileData::outputProfileData(&outer_wall_speed);                                 // new ProfileMapValue(this, &outer_wall_speed)));
	fs << ProfileData::outputProfileData(&inner_wall_speed);                                 // new ProfileMapValue(this, &inner_wall_speed)));
	fs << ProfileData::outputProfileData(&infill_speed);                                 // new ProfileMapValue(this, &infill_speed)));
	fs << ProfileData::outputProfileData(&top_bottom_speed);                                 // new ProfileMapValue(this, &top_bottom_speed)));
	fs << ProfileData::outputProfileData(&travel_speed);                                 // new ProfileMapValue(this, &travel_speed)));
	fs << ProfileData::outputProfileData(&slower_layers_count);                                 // new ProfileMapValue(this, &slower_layers_count)));

	fs << ProfileData::outputProfileData(&retraction_enable);                                 // new ProfileMapValue(this, &retraction_enable)));
	fs << ProfileData::outputProfileData(&retraction_speed);                                 // new ProfileMapValue(this, &retraction_speed)));
	fs << ProfileData::outputProfileData(&retraction_amount);                                 // new ProfileMapValue(this, &retraction_amount)));
	fs << ProfileData::outputProfileData(&retraction_min_travel);                                 // new ProfileMapValue(this, &retraction_min_travel)));
	fs << ProfileData::outputProfileData(&retraction_combing);                                 // new ProfileMapValue(this, &retraction_combing)));
	fs << ProfileData::outputProfileData(&retraction_minimal_extrusion);                                 // new ProfileMapValue(this, &retraction_minimal_extrusion)));
	fs << ProfileData::outputProfileData(&retraction_hop);                                 // new ProfileMapValue(this, &retraction_hop)));
	fs << ProfileData::outputProfileData(&internal_moving_area);                                 // new ProfileMapValue(this, &internal_moving_area)));

	fs << ProfileData::outputProfileData(&cool_min_layer_time);                                 // new ProfileMapValue(this, &cool_min_layer_time)));
	fs << ProfileData::outputProfileData(&fan_enabled);                                 // new ProfileMapValue(this, &fan_enabled)));
	fs << ProfileData::outputProfileData(&fan_full_height);                                 // new ProfileMapValue(this, &fan_full_height)));
	fs << ProfileData::outputProfileData(&fan_speed_regular);                                 // new ProfileMapValue(this, &fan_speed_regular)));
	fs << ProfileData::outputProfileData(&fan_speed_max);                                 // new ProfileMapValue(this, &fan_speed_max)));
	fs << ProfileData::outputProfileData(&cool_min_feedrate);                                 // new ProfileMapValue(this, &cool_min_feedrate)));
	fs << ProfileData::outputProfileData(&cool_head_lift);                                 // new ProfileMapValue(this, &cool_head_lift)));

	fs << ProfileData::outputProfileData(&toolchange_retraction_amount);                                 // new ProfileMapValue(this, &toolchange_retraction_amount)));
	fs << ProfileData::outputProfileData(&toolchange_retraction_speed);                                 // new ProfileMapValue(this, &toolchange_retraction_speed)));
	fs << ProfileData::outputProfileData(&toolchange_lowering_bed);                                 // new ProfileMapValue(this, &toolchange_lowering_bed)));
	fs << ProfileData::outputProfileData(&toolchange_extra_restart_amount);                                 // new ProfileMapValue(this, &toolchange_extra_restart_amount)));
	fs << ProfileData::outputProfileData(&toolchange_extra_restart_speed);                                 // new ProfileMapValue(this, &toolchange_extra_restart_speed)));

	fs << ProfileData::outputProfileData(&standby_temperature_enabled);                                 // new ProfileMapValue(this, &standby_temperature_enabled)));
	fs << ProfileData::outputProfileData(&operating_standby_temperature);                                 // new ProfileMapValue(this, &operating_standby_temperature)));
	fs << ProfileData::outputProfileData(&initial_standby_temperature);                                 // new ProfileMapValue(this, &initial_standby_temperature)));
	fs << ProfileData::outputProfileData(&preheat_enabled);                                 // new ProfileMapValue(this, &preheat_enabled)));
	fs << ProfileData::outputProfileData(&preheat_threshold_time);                                 // new ProfileMapValue(this, &preheat_threshold_time)));

	fs << ProfileData::outputProfileData(&wipe_tower_enabled);                                 // new ProfileMapValue(this, &wipe_tower_enabled)));
	fs << ProfileData::outputProfileData(&wipe_tower_position);                                 // new ProfileMapValue(this, &wipe_tower_position)));
	fs << ProfileData::outputProfileData(&wipe_tower_infill_density);                                 // new ProfileMapValue(this, &wipe_tower_infill_density)));
	fs << ProfileData::outputProfileData(&wipe_tower_raft_margin);                                 // new ProfileMapValue(this, &wipe_tower_raft_margin)));
	fs << ProfileData::outputProfileData(&wipe_tower_base_size);                                 // new ProfileMapValue(this, &wipe_tower_base_size)));
	fs << ProfileData::outputProfileData(&wipe_tower_base_layer_count);                                 // new ProfileMapValue(this, &wipe_tower_base_layer_count)));
	fs << ProfileData::outputProfileData(&wipe_tower_outer_size);                                 // new ProfileMapValue(this, &wipe_tower_outer_size)));
	fs << ProfileData::outputProfileData(&wipe_tower_outer_wall_thickness);                                 // new ProfileMapValue(this, &wipe_tower_outer_wall_thickness)));
	fs << ProfileData::outputProfileData(&wipe_tower_outer_inner_gap);                                 // new ProfileMapValue(this, &wipe_tower_outer_inner_gap)));
	fs << ProfileData::outputProfileData(&wipe_tower_flow);                                 // new ProfileMapValue(this, &wipe_tower_flow)));
	fs << ProfileData::outputProfileData(&wipe_tower_speed);                                 // new ProfileMapValue(this, &wipe_tower_speed)));

	fs << ProfileData::outputProfileData(&spiralize);                                 // new ProfileMapValue(this, &spiralize)));
	fs << ProfileData::outputProfileData(&simple_mode);                                 // new ProfileMapValue(this, &simple_mode)));
	fs << ProfileData::outputProfileData(&fix_horrible_union_all_type_a);                                 // new ProfileMapValue(this, &fix_horrible_union_all_type_a)));
	fs << ProfileData::outputProfileData(&fix_horrible_union_all_type_b);                                 // new ProfileMapValue(this, &fix_horrible_union_all_type_b)));
	fs << ProfileData::outputProfileData(&fix_horrible_use_open_bits);                                 // new ProfileMapValue(this, &fix_horrible_use_open_bits)));
	fs << ProfileData::outputProfileData(&fix_horrible_extensive_stitching);                                 // new ProfileMapValue(this, &fix_horrible_extensive_stitching)));

	//fs << ProfileData::outputProfileData(&nozzle_size);                                 // new ProfileMapValue(this, &nozzle_size)));
	fs << ProfileData::outputProfileData(&bed_type);                                 // new ProfileMapValue(this, &bed_type)));
	fs << ProfileData::outputProfileData(&object_sink);                                 // new ProfileMapValue(this, &object_sink)));
	fs << ProfileData::outputProfileData(&overlap_dual);                                 // new ProfileMapValue(this, &overlap_dual)));

	//////////////////////////////////

	

}
//
std::string SliceProfile::getCurrentProfileOutput()
{
	std::string currentProfile;

	//currentProfile.append(";").append(ProfileData::outputProfileData_value(&profile_version));                                 // new ProfileMapValue(this, &profile_version))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&profile_name));                                 // new ProfileMapValue(this, &profile_name))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&filament_material));                                 // new ProfileMapValue(this, &filament_material))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&filament_diameter));                                 // new ProfileMapValue(this, &filament_diameter))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&layer_height));                                 // new ProfileMapValue(this, &layer_height))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&initial_layer_height));                                 // new ProfileMapValue(this, &initial_layer_height))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&initial_layer_width_factor));                                 // new ProfileMapValue(this, &initial_layer_width_factor))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wall_width_factor));                                 // new ProfileMapValue(this, &outer_wall_width_factor))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&infill_width_factor));                                 // new ProfileMapValue(this, &infill_width_factor))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&top_bottom_width_factor));                                 // new ProfileMapValue(this, &top_bottom_width_factor))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&z_offset_raft));                                 // new ProfileMapValue(this, &z_offset_raft))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&z_offset_except_raft));                                 // new ProfileMapValue(this, &z_offset_except_raft))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wall_thickness));                                 // new ProfileMapValue(this, &wall_thickness))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&top_bottom_thickness));                                 // new ProfileMapValue(this, &top_bottom_thickness))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&solid_top));                                 // new ProfileMapValue(this, &solid_top))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&solid_bottom));                                 // new ProfileMapValue(this, &solid_bottom))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wall_printing_direction));                                 // new ProfileMapValue(this, &wall_printing_direction))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&print_temperature));                                 // new ProfileMapValue(this, &print_temperature))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&print_bed_temperature));                                 // new ProfileMapValue(this, &print_bed_temperature))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&temperature_layer_setting_enabled));                                 // new ProfileMapValue(this, &temperature_layer_setting_enabled))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&overall_flow_control_enabled));                                 // new ProfileMapValue(this, &overall_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&overall_flow));                                 // new ProfileMapValue(this, &overall_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&initial_layer_flow));                                 // new ProfileMapValue(this, &initial_layer_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&infill_flow));                                 // new ProfileMapValue(this, &infill_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&outer_wall_flow));                                 // new ProfileMapValue(this, &outer_wall_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&inner_wall_flow));                                 // new ProfileMapValue(this, &inner_wall_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&top_bottom_flow));                                 // new ProfileMapValue(this, &top_bottom_flow))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_main_cartridge_index));                                 // new ProfileMapValue(this, &support_main_cartridge_index))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_placement));                                 // new ProfileMapValue(this, &support_placement))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_main_pattern));                                 // new ProfileMapValue(this, &support_main_pattern))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_angle));                                 // new ProfileMapValue(this, &support_angle))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_infill_density));                                 // new ProfileMapValue(this, &support_infill_density))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_xy_distance));                                 // new ProfileMapValue(this, &support_xy_distance))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_z_distance));                                 // new ProfileMapValue(this, &support_z_distance))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_main_flow));                                 // new ProfileMapValue(this, &support_main_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_horizontal_expansion));                                 // new ProfileMapValue(this, &support_horizontal_expansion))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_main_speed));                                 // new ProfileMapValue(this, &support_main_print_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_main_width_factor));                                 // new ProfileMapValue(this, &support_main_width_factor))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_enabled));                                 // new ProfileMapValue(this, &support_interface_enabled))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_pattern));                                 // new ProfileMapValue(this, &support_interface_pattern))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_roof_layers_count));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_roof_width_factor));                                 // new ProfileMapValue(this, &support_interface_roof_width_factor))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_roof_flow));                                 // new ProfileMapValue(this, &support_interface_roof_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_roof_speed));                                 // new ProfileMapValue(this, &support_interface_roof_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_floor_layers_count));                                 // new ProfileMapValue(this, &support_interface_roof_width_factor))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_floor_width_factor));                                 // new ProfileMapValue(this, &support_interface_roof_width_factor))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_floor_flow));                                 // new ProfileMapValue(this, &support_interface_floor_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&support_interface_floor_speed));                                 // new ProfileMapValue(this, &support_interface_floor_speed))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&platform_adhesion));                                 // new ProfileMapValue(this, &platform_adhesion))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&adhesion_cartridge_index));                                 // new ProfileMapValue(this, &adhesion_cartridge_index))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&skirt_line_count));                                 // new ProfileMapValue(this, &skirt_line_count))));
	//currentProfile.append(";").append(ProfileData::outputProfileData_value(&skirt_width_factor));                                 // new ProfileMapValue(this, &skirt_width_factor))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&skirt_gap));                                 // new ProfileMapValue(this, &skirt_gap))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&skirt_minimal_length));                                 // new ProfileMapValue(this, &skirt_minimal_length))));
	//currentProfile.append(";").append(ProfileData::outputProfileData_value(&skirt_speed));                                 // new ProfileMapValue(this, &skirt_speed))));
	//currentProfile.append(";").append(ProfileData::outputProfileData_value(&skirt_flow));                                 // new ProfileMapValue(this, &skirt_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&brim_line_count));                                 // new ProfileMapValue(this, &brim_line_count))));
	//currentProfile.append(";").append(ProfileData::outputProfileData_value(&brim_width_factor));                                 // new ProfileMapValue(this, &brim_width_factor))));
	//currentProfile.append(";").append(ProfileData::outputProfileData_value(&brim_speed));                                 // new ProfileMapValue(this, &brim_speed))));
	//currentProfile.append(";").append(ProfileData::outputProfileData_value(&brim_flow));                                 // new ProfileMapValue(this, &brim_flow))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_margin));                                 // new ProfileMapValue(this, &raft_margin))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_line_spacing));                                 // new ProfileMapValue(this, &raft_line_spacing))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_airgap_all));                                 // new ProfileMapValue(this, &raft_airgap_all))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_airgap_initial_layer));                                 // new ProfileMapValue(this, &raft_airgap_initial_layer))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_inset_enabled));                                 // new ProfileMapValue(this, &raft_inset_enabled))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_inset_offset));                                 // new ProfileMapValue(this, &raft_inset_offset))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_temperature_control));                                 // new ProfileMapValue(this, &raft_temperature_control))));
	//currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_fan_speed));                                 // new ProfileMapValue(this, &raft_fan_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_incline_enabled));                                 // new ProfileMapValue(this, &raft_incline_enabled))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_base_thickness));                                 // new ProfileMapValue(this, &raft_base_thickness))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_base_line_width));                                 // new ProfileMapValue(this, &raft_base_line_width))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_base_speed));                                 // new ProfileMapValue(this, &raft_base_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_base_temperature));                                 // new ProfileMapValue(this, &raft_base_temperature))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_interface_thickness));                                 // new ProfileMapValue(this, &raft_interface_thickness))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_interface_line_width));                                 // new ProfileMapValue(this, &raft_interface_line_width))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_interface_speed));                                 // new ProfileMapValue(this, &raft_interface_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_interface_temperature));                                 // new ProfileMapValue(this, &raft_interface_temperature))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_surface_layers));                                 // new ProfileMapValue(this, &raft_surface_layers))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_surface_thickness));                                 // new ProfileMapValue(this, &raft_surface_thickness))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_surface_line_width));                                 // new ProfileMapValue(this, &raft_surface_line_width))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_surface_speed));                                 // new ProfileMapValue(this, &raft_surface_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_surface_initial_temperature));                                 // new ProfileMapValue(this, &raft_surface_initial_temperature))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&raft_surface_last_temperature));                                 // new ProfileMapValue(this, &raft_surface_last_temperature))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&infill_density));                                 // new ProfileMapValue(this, &infill_density))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&infill_overlap));                                 // new ProfileMapValue(this, &infill_overlap))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&infill_pattern));                                 // new ProfileMapValue(this, &infill_pattern))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&infill_before_wall));                                 // new ProfileMapValue(this, &infill_before_wall))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&skin_outline));                                 // new ProfileMapValue(this, &skin_outline))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&skin_type));                                 // new ProfileMapValue(this, &skin_type))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&skin_removal_width_top));                                 // new ProfileMapValue(this, &skin_removal_width_top))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&skin_removal_width_bottom));                                 // new ProfileMapValue(this, &skin_removal_width_bottom))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&skin_overlap));                                 // new ProfileMapValue(this, &skin_overlap))));


	currentProfile.append(";").append(ProfileData::outputProfileData_value(&print_speed));                                 // new ProfileMapValue(this, &print_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&initial_layer_speed));                                 // new ProfileMapValue(this, &initial_layer_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&outer_wall_speed));                                 // new ProfileMapValue(this, &outer_wall_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&inner_wall_speed));                                 // new ProfileMapValue(this, &inner_wall_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&infill_speed));                                 // new ProfileMapValue(this, &infill_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&top_bottom_speed));                                 // new ProfileMapValue(this, &top_bottom_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&travel_speed));                                 // new ProfileMapValue(this, &travel_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&slower_layers_count));                                 // new ProfileMapValue(this, &slower_layers_count))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&retraction_enable));                                 // new ProfileMapValue(this, &retraction_enable))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&retraction_speed));                                 // new ProfileMapValue(this, &retraction_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&retraction_amount));                                 // new ProfileMapValue(this, &retraction_amount))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&retraction_min_travel));                                 // new ProfileMapValue(this, &retraction_min_travel))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&retraction_combing));                                 // new ProfileMapValue(this, &retraction_combing))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&retraction_minimal_extrusion));                                 // new ProfileMapValue(this, &retraction_minimal_extrusion))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&retraction_hop));                                 // new ProfileMapValue(this, &retraction_hop))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&internal_moving_area));                                 // new ProfileMapValue(this, &internal_moving_area))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&cool_min_layer_time));                                 // new ProfileMapValue(this, &cool_min_layer_time))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&fan_enabled));                                 // new ProfileMapValue(this, &fan_enabled))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&fan_full_height));                                 // new ProfileMapValue(this, &fan_full_height))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&fan_speed_regular));                                 // new ProfileMapValue(this, &fan_speed_regular))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&fan_speed_max));                                 // new ProfileMapValue(this, &fan_speed_max))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&cool_min_feedrate));                                 // new ProfileMapValue(this, &cool_min_feedrate))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&cool_head_lift));                                 // new ProfileMapValue(this, &cool_head_lift))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&toolchange_retraction_amount));                                 // new ProfileMapValue(this, &toolchange_retraction_amount))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&toolchange_retraction_speed));                                 // new ProfileMapValue(this, &toolchange_retraction_speed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&toolchange_lowering_bed));                                 // new ProfileMapValue(this, &toolchange_lowering_bed))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&toolchange_extra_restart_amount));                                 // new ProfileMapValue(this, &toolchange_extra_restart_amount))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&toolchange_extra_restart_speed));                                 // new ProfileMapValue(this, &toolchange_extra_restart_speed))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&standby_temperature_enabled));                                 // new ProfileMapValue(this, &standby_temperature_enabled))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&operating_standby_temperature));                                 // new ProfileMapValue(this, &operating_standby_temperature))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&initial_standby_temperature));                                 // new ProfileMapValue(this, &initial_standby_temperature))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&preheat_enabled));                                 // new ProfileMapValue(this, &preheat_enabled))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&preheat_threshold_time));                                 // new ProfileMapValue(this, &preheat_threshold_time))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_enabled));                                 // new ProfileMapValue(this, &wipe_tower_enabled))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_position));                                 // new ProfileMapValue(this, &wipe_tower_position))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_infill_density));                                 // new ProfileMapValue(this, &wipe_tower_infill_density))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_raft_margin));                                 // new ProfileMapValue(this, &wipe_tower_raft_margin))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_base_size));                                 // new ProfileMapValue(this, &wipe_tower_base_size))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_base_layer_count));                                 // new ProfileMapValue(this, &wipe_tower_base_layer_count))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_outer_size));                                 // new ProfileMapValue(this, &wipe_tower_outer_size))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_outer_wall_thickness));                                 // new ProfileMapValue(this, &wipe_tower_outer_wall_thickness))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_outer_inner_gap));                                 // new ProfileMapValue(this, &wipe_tower_outer_inner_gap))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_flow));                                 // new ProfileMapValue(this, &wipe_tower_flow))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&wipe_tower_speed));                                 // new ProfileMapValue(this, &wipe_tower_speed))));

	currentProfile.append(";").append(ProfileData::outputProfileData_value(&spiralize));                                 // new ProfileMapValue(this, &spiralize))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&simple_mode));                                 // new ProfileMapValue(this, &simple_mode))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&fix_horrible_union_all_type_a));                                 // new ProfileMapValue(this, &fix_horrible_union_all_type_a))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&fix_horrible_union_all_type_b));                                 // new ProfileMapValue(this, &fix_horrible_union_all_type_b))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&fix_horrible_use_open_bits));                                 // new ProfileMapValue(this, &fix_horrible_use_open_bits))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&fix_horrible_extensive_stitching));                                 // new ProfileMapValue(this, &fix_horrible_extensive_stitching))));

	//currentProfile.append(";").append(ProfileData::outputProfileData_value(&nozzle_size));                                 // new ProfileMapValue(this, &nozzle_size))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&bed_type));                                 // new ProfileMapValue(this, &bed_type))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&object_sink));                                 // new ProfileMapValue(this, &object_sink))));
	currentProfile.append(";").append(ProfileData::outputProfileData_value(&overlap_dual));                                 // new ProfileMapValue(this, &overlap_dual))));

	//////////////////////////////////

	return currentProfile;
}

bool SliceProfile::loadTemperatureLayerList(const wchar_t* filePath)
{
	temperature_layer_list.clear();

	QFile inputFile;
	inputFile.setFileName(QString::fromWCharArray(filePath));
	inputFile.open(QIODevice::ReadOnly);

	if (!inputFile.isOpen())
		return false;
	
	QTextStream stream(&inputFile);
	QString line;

	std::vector<QString> str_temperature_layer_setpoint;
	int temperature_layer_list_size = 0;
	int count_temperature_list_setpoint_count = 0;
	bool b_temperature_layer_list_size = false;

	while (!stream.atEnd())
	{
		line = stream.readLine();

		if (line == "")
			continue;
		else if (line.startsWith("temperature_layer_list_size:"))
		{
			temperature_layer_list_size = GCodeParser::getInnerText(line).toInt();
			b_temperature_layer_list_size = true;

			str_temperature_layer_setpoint.clear();

			for (int i = 0; i < temperature_layer_list_size; i++)
				str_temperature_layer_setpoint.push_back(QString("temperature_layer_setpoint_%1:").arg(i));

			continue;
		}

		if (b_temperature_layer_list_size && temperature_layer_list_size > 0 && count_temperature_list_setpoint_count < str_temperature_layer_setpoint.size())
		{
			this->temperature_layer_list.resize(temperature_layer_list_size);

			if (line.startsWith(str_temperature_layer_setpoint.at(count_temperature_list_setpoint_count)))
			{
				QString setPoint_string = GCodeParser::getInnerText(line);
				QStringList setPoint_stringList = setPoint_string.split(":");

				if (setPoint_stringList.size() != 2)
					continue;

				this->temperature_layer_list.at(count_temperature_list_setpoint_count).layerNr = setPoint_stringList[0].toInt();
				this->temperature_layer_list.at(count_temperature_list_setpoint_count).temperature = setPoint_stringList[1].toInt();

				count_temperature_list_setpoint_count++;
			}
		}
	}

	rearrangeTemperatureLayerList();

	inputFile.close();

	return true;
}

bool SliceProfile::saveTemperatureLayerList(const wchar_t* filePath)
{
	std::fstream fs;

	fs.open(filePath, std::ios_base::app | std::ios_base::out);

	if (!(fs.is_open()))
		return false;


	if (!temperature_layer_list.empty())
	{
		fs << ("temperature_layer_list_size:") << (" ") << temperature_layer_list.size() << ("\n");

		for (int i = 0; i < temperature_layer_list.size(); ++i)
		{
			fs << ("temperature_layer_setpoint_") << i << (": [") << temperature_layer_list.at(i).layerNr << (":") << temperature_layer_list.at(i).temperature << ("]") << ("\n");
		}
	}

	fs.close();

	return true;
}

void SliceProfile::clearTemperatureLayerListAll()
{
	temperature_layer_list.clear();
}

void SliceProfile::rearrangeTemperatureLayerList()
{
	//post processing..//

	//temperatureLayerList_bed//
	//오름차순 정렬..//
	std::sort(temperature_layer_list.begin(), temperature_layer_list.end(), [](const TemperatureLayerSetPoint a, const TemperatureLayerSetPoint b) { return (a.layerNr < b.layerNr); });
	//중복되는 부분의 iterator 추출..//
	auto it = std::unique(temperature_layer_list.begin(), temperature_layer_list.end(), [](const TemperatureLayerSetPoint a, const TemperatureLayerSetPoint b) { return (a.layerNr == b.layerNr); });
	//iterator기준으로 끝까지 삭제 -> layerNr 기준 중복 data 삭제..//
	temperature_layer_list.erase(it, temperature_layer_list.end());
}


std::map<std::string, ProfileMapValue*> SliceProfile::setSliceProfileMap()
{
	std::map<std::string, ProfileMapValue*> _slice_profile_map;


	//_slice_profile_map.clear();
	
	//_slice_profile_map.insert(std::make_pair(profile_version.name_key.c_str(), new ProfileMapValue(this, &profile_version)));
	_slice_profile_map.insert(std::make_pair(profile_name.name_key.c_str(), new ProfileMapValue(this, &profile_name)));
	_slice_profile_map.insert(std::make_pair(filament_material.name_key.c_str(), new ProfileMapValue(this, &filament_material)));
	_slice_profile_map.insert(std::make_pair(filament_diameter.name_key.c_str(), new ProfileMapValue(this, &filament_diameter)));

	_slice_profile_map.insert(std::make_pair(layer_height.name_key.c_str(), new ProfileMapValue(this, &layer_height)));
	_slice_profile_map.insert(std::make_pair(initial_layer_height.name_key.c_str(), new ProfileMapValue(this, &initial_layer_height)));
	_slice_profile_map.insert(std::make_pair(initial_layer_width_factor.name_key.c_str(), new ProfileMapValue(this, &initial_layer_width_factor)));
	_slice_profile_map.insert(std::make_pair(wall_width_factor.name_key.c_str(), new ProfileMapValue(this, &wall_width_factor)));
	_slice_profile_map.insert(std::make_pair(infill_width_factor.name_key.c_str(), new ProfileMapValue(this, &infill_width_factor)));
	_slice_profile_map.insert(std::make_pair(top_bottom_width_factor.name_key.c_str(), new ProfileMapValue(this, &top_bottom_width_factor)));
	_slice_profile_map.insert(std::make_pair(z_offset_raft.name_key.c_str(), new ProfileMapValue(this, &z_offset_raft)));
	_slice_profile_map.insert(std::make_pair(z_offset_except_raft.name_key.c_str(), new ProfileMapValue(this, &z_offset_except_raft)));

	_slice_profile_map.insert(std::make_pair(wall_thickness.name_key.c_str(), new ProfileMapValue(this, &wall_thickness)));
	_slice_profile_map.insert(std::make_pair(top_bottom_thickness.name_key.c_str(), new ProfileMapValue(this, &top_bottom_thickness)));
	_slice_profile_map.insert(std::make_pair(solid_top.name_key.c_str(), new ProfileMapValue(this, &solid_top)));
	_slice_profile_map.insert(std::make_pair(solid_bottom.name_key.c_str(), new ProfileMapValue(this, &solid_bottom)));
	_slice_profile_map.insert(std::make_pair(wall_printing_direction.name_key.c_str(), new ProfileMapValue(this, &wall_printing_direction)));
	
	_slice_profile_map.insert(std::make_pair(print_temperature.name_key.c_str(), new ProfileMapValue(this, &print_temperature)));
	_slice_profile_map.insert(std::make_pair(print_bed_temperature.name_key.c_str(), new ProfileMapValue(this, &print_bed_temperature)));
	_slice_profile_map.insert(std::make_pair(temperature_layer_setting_enabled.name_key.c_str(), new ProfileMapValue(this, &temperature_layer_setting_enabled)));

	_slice_profile_map.insert(std::make_pair(overall_flow_control_enabled.name_key.c_str(), new ProfileMapValue(this, &overall_flow_control_enabled)));
	_slice_profile_map.insert(std::make_pair(overall_flow.name_key.c_str(), new ProfileMapValue(this, &overall_flow)));
	_slice_profile_map.insert(std::make_pair(initial_layer_flow.name_key.c_str(), new ProfileMapValue(this, &initial_layer_flow)));
	_slice_profile_map.insert(std::make_pair(infill_flow.name_key.c_str(), new ProfileMapValue(this, &infill_flow)));
	_slice_profile_map.insert(std::make_pair(outer_wall_flow.name_key.c_str(), new ProfileMapValue(this, &outer_wall_flow)));
	_slice_profile_map.insert(std::make_pair(inner_wall_flow.name_key.c_str(), new ProfileMapValue(this, &inner_wall_flow)));
	_slice_profile_map.insert(std::make_pair(top_bottom_flow.name_key.c_str(), new ProfileMapValue(this, &top_bottom_flow)));
	
	_slice_profile_map.insert(std::make_pair(support_main_cartridge_index.name_key.c_str(), new ProfileMapValue(this, &support_main_cartridge_index)));
	_slice_profile_map.insert(std::make_pair(support_placement.name_key.c_str(), new ProfileMapValue(this, &support_placement)));
	_slice_profile_map.insert(std::make_pair(support_main_pattern.name_key.c_str(), new ProfileMapValue(this, &support_main_pattern)));
	_slice_profile_map.insert(std::make_pair(support_angle.name_key.c_str(), new ProfileMapValue(this, &support_angle)));
	_slice_profile_map.insert(std::make_pair(support_infill_density.name_key.c_str(), new ProfileMapValue(this, &support_infill_density)));
	_slice_profile_map.insert(std::make_pair(support_xy_distance.name_key.c_str(), new ProfileMapValue(this, &support_xy_distance)));
	_slice_profile_map.insert(std::make_pair(support_z_distance.name_key.c_str(), new ProfileMapValue(this, &support_z_distance)));
	_slice_profile_map.insert(std::make_pair(support_main_flow.name_key.c_str(), new ProfileMapValue(this, &support_main_flow)));
	_slice_profile_map.insert(std::make_pair(support_horizontal_expansion.name_key.c_str(), new ProfileMapValue(this, &support_horizontal_expansion)));
	_slice_profile_map.insert(std::make_pair(support_main_speed.name_key.c_str(), new ProfileMapValue(this, &support_main_speed)));
	_slice_profile_map.insert(std::make_pair(support_main_width_factor.name_key.c_str(), new ProfileMapValue(this, &support_main_width_factor)));
	_slice_profile_map.insert(std::make_pair(support_interface_enabled.name_key.c_str(), new ProfileMapValue(this, &support_interface_enabled)));
	_slice_profile_map.insert(std::make_pair(support_interface_roof_layers_count.name_key.c_str(), new ProfileMapValue(this, &support_interface_roof_layers_count)));
	_slice_profile_map.insert(std::make_pair(support_interface_pattern.name_key.c_str(), new ProfileMapValue(this, &support_interface_pattern)));
	_slice_profile_map.insert(std::make_pair(support_interface_roof_width_factor.name_key.c_str(), new ProfileMapValue(this, &support_interface_roof_width_factor)));
	_slice_profile_map.insert(std::make_pair(support_interface_roof_flow.name_key.c_str(), new ProfileMapValue(this, &support_interface_roof_flow)));
	_slice_profile_map.insert(std::make_pair(support_interface_roof_speed.name_key.c_str(), new ProfileMapValue(this, &support_interface_roof_speed)));
	_slice_profile_map.insert(std::make_pair(support_interface_floor_layers_count.name_key.c_str(), new ProfileMapValue(this, &support_interface_floor_layers_count)));
	_slice_profile_map.insert(std::make_pair(support_interface_floor_width_factor.name_key.c_str(), new ProfileMapValue(this, &support_interface_floor_width_factor)));
	_slice_profile_map.insert(std::make_pair(support_interface_floor_flow.name_key.c_str(), new ProfileMapValue(this, &support_interface_floor_flow)));
	_slice_profile_map.insert(std::make_pair(support_interface_floor_speed.name_key.c_str(), new ProfileMapValue(this, &support_interface_floor_speed)));
	
	_slice_profile_map.insert(std::make_pair(platform_adhesion.name_key.c_str(), new ProfileMapValue(this, &platform_adhesion)));
	_slice_profile_map.insert(std::make_pair(adhesion_cartridge_index.name_key.c_str(), new ProfileMapValue(this, &adhesion_cartridge_index)));
	_slice_profile_map.insert(std::make_pair(skirt_line_count.name_key.c_str(), new ProfileMapValue(this, &skirt_line_count)));
	//_slice_profile_map.insert(std::make_pair(skirt_width_factor.name_key.c_str(), new ProfileMapValue(this, &skirt_width_factor)));
	_slice_profile_map.insert(std::make_pair(skirt_gap.name_key.c_str(), new ProfileMapValue(this, &skirt_gap)));
	_slice_profile_map.insert(std::make_pair(skirt_minimal_length.name_key.c_str(), new ProfileMapValue(this, &skirt_minimal_length)));
	//_slice_profile_map.insert(std::make_pair(skirt_speed.name_key.c_str(), new ProfileMapValue(this, &skirt_speed)));
	//_slice_profile_map.insert(std::make_pair(skirt_flow.name_key.c_str(), new ProfileMapValue(this, &skirt_flow)));
	_slice_profile_map.insert(std::make_pair(brim_line_count.name_key.c_str(), new ProfileMapValue(this, &brim_line_count)));
	//_slice_profile_map.insert(std::make_pair(brim_width_factor.name_key.c_str(), new ProfileMapValue(this, &brim_width_factor)));
	//_slice_profile_map.insert(std::make_pair(brim_speed.name_key.c_str(), new ProfileMapValue(this, &brim_speed)));
	//_slice_profile_map.insert(std::make_pair(brim_flow.name_key.c_str(), new ProfileMapValue(this, &brim_flow)));

	_slice_profile_map.insert(std::make_pair(raft_margin.name_key.c_str(), new ProfileMapValue(this, &raft_margin)));
	_slice_profile_map.insert(std::make_pair(raft_line_spacing.name_key.c_str(), new ProfileMapValue(this, &raft_line_spacing)));
	_slice_profile_map.insert(std::make_pair(raft_airgap_all.name_key.c_str(), new ProfileMapValue(this, &raft_airgap_all)));
	_slice_profile_map.insert(std::make_pair(raft_airgap_initial_layer.name_key.c_str(), new ProfileMapValue(this, &raft_airgap_initial_layer)));
	_slice_profile_map.insert(std::make_pair(raft_inset_enabled.name_key.c_str(), new ProfileMapValue(this, &raft_inset_enabled)));
	_slice_profile_map.insert(std::make_pair(raft_inset_offset.name_key.c_str(), new ProfileMapValue(this, &raft_inset_offset)));
	_slice_profile_map.insert(std::make_pair(raft_temperature_control.name_key.c_str(), new ProfileMapValue(this, &raft_temperature_control)));
	//_slice_profile_map.insert(std::make_pair(raft_fan_speed.name_key.c_str(), new ProfileMapValue(this, &raft_fan_speed)));
	_slice_profile_map.insert(std::make_pair(raft_incline_enabled.name_key.c_str(), new ProfileMapValue(this, &raft_incline_enabled)));
	_slice_profile_map.insert(std::make_pair(raft_base_thickness.name_key.c_str(), new ProfileMapValue(this, &raft_base_thickness)));
	_slice_profile_map.insert(std::make_pair(raft_base_line_width.name_key.c_str(), new ProfileMapValue(this, &raft_base_line_width)));
	_slice_profile_map.insert(std::make_pair(raft_base_speed.name_key.c_str(), new ProfileMapValue(this, &raft_base_speed)));
	_slice_profile_map.insert(std::make_pair(raft_base_temperature.name_key.c_str(), new ProfileMapValue(this, &raft_base_temperature)));
	_slice_profile_map.insert(std::make_pair(raft_interface_thickness.name_key.c_str(), new ProfileMapValue(this, &raft_interface_thickness)));
	_slice_profile_map.insert(std::make_pair(raft_interface_line_width.name_key.c_str(), new ProfileMapValue(this, &raft_interface_line_width)));
	_slice_profile_map.insert(std::make_pair(raft_interface_speed.name_key.c_str(), new ProfileMapValue(this, &raft_interface_speed)));
	_slice_profile_map.insert(std::make_pair(raft_interface_temperature.name_key.c_str(), new ProfileMapValue(this, &raft_interface_temperature)));
	_slice_profile_map.insert(std::make_pair(raft_surface_layers.name_key.c_str(), new ProfileMapValue(this, &raft_surface_layers)));
	_slice_profile_map.insert(std::make_pair(raft_surface_thickness.name_key.c_str(), new ProfileMapValue(this, &raft_surface_thickness)));
	_slice_profile_map.insert(std::make_pair(raft_surface_line_width.name_key.c_str(), new ProfileMapValue(this, &raft_surface_line_width)));
	_slice_profile_map.insert(std::make_pair(raft_surface_speed.name_key.c_str(), new ProfileMapValue(this, &raft_surface_speed)));
	_slice_profile_map.insert(std::make_pair(raft_surface_initial_temperature.name_key.c_str(), new ProfileMapValue(this, &raft_surface_initial_temperature)));
	_slice_profile_map.insert(std::make_pair(raft_surface_last_temperature.name_key.c_str(), new ProfileMapValue(this, &raft_surface_last_temperature)));

	_slice_profile_map.insert(std::make_pair(infill_density.name_key.c_str(), new ProfileMapValue(this, &infill_density)));
	_slice_profile_map.insert(std::make_pair(infill_overlap.name_key.c_str(), new ProfileMapValue(this, &infill_overlap)));
	_slice_profile_map.insert(std::make_pair(infill_pattern.name_key.c_str(), new ProfileMapValue(this, &infill_pattern)));
	_slice_profile_map.insert(std::make_pair(infill_before_wall.name_key.c_str(), new ProfileMapValue(this, &infill_before_wall)));
	_slice_profile_map.insert(std::make_pair(skin_outline.name_key.c_str(), new ProfileMapValue(this, &skin_outline)));
	_slice_profile_map.insert(std::make_pair(skin_type.name_key.c_str(), new ProfileMapValue(this, &skin_type)));
	_slice_profile_map.insert(std::make_pair(skin_removal_width_top.name_key.c_str(), new ProfileMapValue(this, &skin_removal_width_top)));
	_slice_profile_map.insert(std::make_pair(skin_removal_width_bottom.name_key.c_str(), new ProfileMapValue(this, &skin_removal_width_bottom)));
	_slice_profile_map.insert(std::make_pair(skin_overlap.name_key.c_str(), new ProfileMapValue(this, &skin_overlap)));


	_slice_profile_map.insert(std::make_pair(print_speed.name_key.c_str(), new ProfileMapValue(this, &print_speed)));
	_slice_profile_map.insert(std::make_pair(initial_layer_speed.name_key.c_str(), new ProfileMapValue(this, &initial_layer_speed)));
	_slice_profile_map.insert(std::make_pair(outer_wall_speed.name_key.c_str(), new ProfileMapValue(this, &outer_wall_speed)));
	_slice_profile_map.insert(std::make_pair(inner_wall_speed.name_key.c_str(), new ProfileMapValue(this, &inner_wall_speed)));
	_slice_profile_map.insert(std::make_pair(infill_speed.name_key.c_str(), new ProfileMapValue(this, &infill_speed)));
	_slice_profile_map.insert(std::make_pair(top_bottom_speed.name_key.c_str(), new ProfileMapValue(this, &top_bottom_speed)));
	_slice_profile_map.insert(std::make_pair(travel_speed.name_key.c_str(), new ProfileMapValue(this, &travel_speed)));
	_slice_profile_map.insert(std::make_pair(slower_layers_count.name_key.c_str(), new ProfileMapValue(this, &slower_layers_count)));

	_slice_profile_map.insert(std::make_pair(retraction_enable.name_key.c_str(), new ProfileMapValue(this, &retraction_enable)));
	_slice_profile_map.insert(std::make_pair(retraction_speed.name_key.c_str(), new ProfileMapValue(this, &retraction_speed)));
	_slice_profile_map.insert(std::make_pair(retraction_amount.name_key.c_str(), new ProfileMapValue(this, &retraction_amount)));
	_slice_profile_map.insert(std::make_pair(retraction_min_travel.name_key.c_str(), new ProfileMapValue(this, &retraction_min_travel)));
	_slice_profile_map.insert(std::make_pair(retraction_combing.name_key.c_str(), new ProfileMapValue(this, &retraction_combing)));
	_slice_profile_map.insert(std::make_pair(retraction_minimal_extrusion.name_key.c_str(), new ProfileMapValue(this, &retraction_minimal_extrusion)));
	_slice_profile_map.insert(std::make_pair(retraction_hop.name_key.c_str(), new ProfileMapValue(this, &retraction_hop)));
	_slice_profile_map.insert(std::make_pair(internal_moving_area.name_key.c_str(), new ProfileMapValue(this, &internal_moving_area)));
	
	_slice_profile_map.insert(std::make_pair(cool_min_layer_time.name_key.c_str(), new ProfileMapValue(this, &cool_min_layer_time)));
	_slice_profile_map.insert(std::make_pair(fan_enabled.name_key.c_str(), new ProfileMapValue(this, &fan_enabled)));
	_slice_profile_map.insert(std::make_pair(fan_full_height.name_key.c_str(), new ProfileMapValue(this, &fan_full_height)));
	_slice_profile_map.insert(std::make_pair(fan_speed_regular.name_key.c_str(), new ProfileMapValue(this, &fan_speed_regular)));
	_slice_profile_map.insert(std::make_pair(fan_speed_max.name_key.c_str(), new ProfileMapValue(this, &fan_speed_max)));
	_slice_profile_map.insert(std::make_pair(cool_min_feedrate.name_key.c_str(), new ProfileMapValue(this, &cool_min_feedrate)));
	_slice_profile_map.insert(std::make_pair(cool_head_lift.name_key.c_str(), new ProfileMapValue(this, &cool_head_lift)));

	_slice_profile_map.insert(std::make_pair(toolchange_retraction_amount.name_key.c_str(), new ProfileMapValue(this, &toolchange_retraction_amount)));
	_slice_profile_map.insert(std::make_pair(toolchange_retraction_speed.name_key.c_str(), new ProfileMapValue(this, &toolchange_retraction_speed)));
	_slice_profile_map.insert(std::make_pair(toolchange_lowering_bed.name_key.c_str(), new ProfileMapValue(this, &toolchange_lowering_bed)));
	_slice_profile_map.insert(std::make_pair(toolchange_extra_restart_amount.name_key.c_str(), new ProfileMapValue(this, &toolchange_extra_restart_amount)));
	_slice_profile_map.insert(std::make_pair(toolchange_extra_restart_speed.name_key.c_str(), new ProfileMapValue(this, &toolchange_extra_restart_speed)));

	_slice_profile_map.insert(std::make_pair(standby_temperature_enabled.name_key.c_str(), new ProfileMapValue(this, &standby_temperature_enabled)));
	_slice_profile_map.insert(std::make_pair(operating_standby_temperature.name_key.c_str(), new ProfileMapValue(this, &operating_standby_temperature)));
	_slice_profile_map.insert(std::make_pair(initial_standby_temperature.name_key.c_str(), new ProfileMapValue(this, &initial_standby_temperature)));
	_slice_profile_map.insert(std::make_pair(preheat_enabled.name_key.c_str(), new ProfileMapValue(this, &preheat_enabled)));
	_slice_profile_map.insert(std::make_pair(preheat_threshold_time.name_key.c_str(), new ProfileMapValue(this, &preheat_threshold_time)));
	
	_slice_profile_map.insert(std::make_pair(wipe_tower_enabled.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_enabled)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_position.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_position)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_infill_density.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_infill_density)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_raft_margin.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_raft_margin)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_base_size.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_base_size)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_base_layer_count.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_base_layer_count)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_outer_size.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_outer_size)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_outer_wall_thickness.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_outer_wall_thickness)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_outer_inner_gap.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_outer_inner_gap)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_flow.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_flow)));
	_slice_profile_map.insert(std::make_pair(wipe_tower_speed.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_speed)));

	_slice_profile_map.insert(std::make_pair(spiralize.name_key.c_str(), new ProfileMapValue(this, &spiralize)));
	_slice_profile_map.insert(std::make_pair(simple_mode.name_key.c_str(), new ProfileMapValue(this, &simple_mode)));
	_slice_profile_map.insert(std::make_pair(fix_horrible_union_all_type_a.name_key.c_str(), new ProfileMapValue(this, &fix_horrible_union_all_type_a)));
	_slice_profile_map.insert(std::make_pair(fix_horrible_union_all_type_b.name_key.c_str(), new ProfileMapValue(this, &fix_horrible_union_all_type_b)));
	_slice_profile_map.insert(std::make_pair(fix_horrible_use_open_bits.name_key.c_str(), new ProfileMapValue(this, &fix_horrible_use_open_bits)));
	_slice_profile_map.insert(std::make_pair(fix_horrible_extensive_stitching.name_key.c_str(), new ProfileMapValue(this, &fix_horrible_extensive_stitching)));

	//_slice_profile_map.insert(std::make_pair(nozzle_size.name_key.c_str(), new ProfileMapValue(this, &nozzle_size)));
	_slice_profile_map.insert(std::make_pair(bed_type.name_key.c_str(), new ProfileMapValue(this, &bed_type)));
	_slice_profile_map.insert(std::make_pair(object_sink.name_key.c_str(), new ProfileMapValue(this, &object_sink)));
	_slice_profile_map.insert(std::make_pair(overlap_dual.name_key.c_str(), new ProfileMapValue(this, &overlap_dual)));
	//_slice_profile_map.insert(std::make_pair(profile_name.name_key.c_str(), new ProfileMapValue(this, &profile_name)));
	//_slice_profile_map.insert(std::make_pair(filament_material.name_key.c_str(), new ProfileMapValue(this, &filament_material)));
	

	return _slice_profile_map;
}

std::map<std::string, ProfileMapValue*> SliceProfile::setSliceProfileMap_old()
{
	std::map<std::string, ProfileMapValue*> _slice_profile_map_old;


	//slice_profile_map_old.clear();


	_slice_profile_map_old.insert(std::make_pair(solid_layer_thickness.name_key.c_str(), new ProfileMapValue(this, &solid_layer_thickness)));
	_slice_profile_map_old.insert(std::make_pair(fill_density.name_key.c_str(), new ProfileMapValue(this, &fill_density)));
	_slice_profile_map_old.insert(std::make_pair(printer_speed.name_key.c_str(), new ProfileMapValue(this, &printer_speed)));
	_slice_profile_map_old.insert(std::make_pair(support_cartridge_index.name_key.c_str(), new ProfileMapValue(this, &support_cartridge_index)));
	_slice_profile_map_old.insert(std::make_pair(support_pattern.name_key.c_str(), new ProfileMapValue(this, &support_pattern)));
	_slice_profile_map_old.insert(std::make_pair(support_fill_rate.name_key.c_str(), new ProfileMapValue(this, &support_fill_rate)));
	_slice_profile_map_old.insert(std::make_pair(support_interface_count.name_key.c_str(), new ProfileMapValue(this, &support_interface_count)));
	_slice_profile_map_old.insert(std::make_pair(wipe_tower_fill_density.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_fill_density)));
	_slice_profile_map_old.insert(std::make_pair(wipe_tower_filament_flow.name_key.c_str(), new ProfileMapValue(this, &wipe_tower_filament_flow)));
	_slice_profile_map_old.insert(std::make_pair(filament_flow.name_key.c_str(), new ProfileMapValue(this, &filament_flow)));
	_slice_profile_map_old.insert(std::make_pair(layer0_filament_flow.name_key.c_str(), new ProfileMapValue(this, &layer0_filament_flow)));
	_slice_profile_map_old.insert(std::make_pair(layer0_thickness.name_key.c_str(), new ProfileMapValue(this, &layer0_thickness)));
	_slice_profile_map_old.insert(std::make_pair(layer0_width_factor.name_key.c_str(), new ProfileMapValue(this, &layer0_width_factor)));
	_slice_profile_map_old.insert(std::make_pair(bottom_layer_speed.name_key.c_str(), new ProfileMapValue(this, &bottom_layer_speed)));
	_slice_profile_map_old.insert(std::make_pair(solidarea_speed.name_key.c_str(), new ProfileMapValue(this, &solidarea_speed)));
	_slice_profile_map_old.insert(std::make_pair(inset0_speed.name_key.c_str(), new ProfileMapValue(this, &inset0_speed)));
	_slice_profile_map_old.insert(std::make_pair(insetx_speed.name_key.c_str(), new ProfileMapValue(this, &insetx_speed)));
	_slice_profile_map_old.insert(std::make_pair(raft_airgap_layer0.name_key.c_str(), new ProfileMapValue(this, &raft_airgap_layer0)));
	_slice_profile_map_old.insert(std::make_pair(raft_base_linewidth.name_key.c_str(), new ProfileMapValue(this, &raft_base_linewidth)));
	_slice_profile_map_old.insert(std::make_pair(raft_interface_linewidth.name_key.c_str(), new ProfileMapValue(this, &raft_interface_linewidth)));
	_slice_profile_map_old.insert(std::make_pair(raft_surface_linewidth.name_key.c_str(), new ProfileMapValue(this, &raft_surface_linewidth)));

	return _slice_profile_map_old;
}

bool SliceProfile::replaceProfileDataValue(ProfileDataI*  _profile_data_old, ProfileDataI* _profile_data_new)
{
	_profile_data_new->value = _profile_data_old->value;
	_profile_data_new->min = _profile_data_old->min;
	_profile_data_new->max = _profile_data_old->max;

	return true;
}

bool SliceProfile::replaceProfileDataValue(ProfileDataD*  _profile_data_old, ProfileDataD* _profile_data_new)
{
	_profile_data_new->value = _profile_data_old->value;
	_profile_data_new->min = _profile_data_old->min;
	_profile_data_new->max = _profile_data_old->max;

	return true;
}
