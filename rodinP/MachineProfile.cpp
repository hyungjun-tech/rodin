#include "stdafx.h"
#include "MachineProfile.h"
#include "GCodeParser.h"

#define PROFILE_NUM 17

MachineProfile::MachineProfile() { }

MachineProfile::~MachineProfile() { }

//프로파일 로드에 실패하였을 경우 사용하도록 일단 유지.
void MachineProfile::setMachineDataDefault()
{
	machine_model = "DP200";
	group_model = "DP200";
	machine_width_default.value = 210;
	machine_depth_default.value = 200;
	machine_height_default.value = 195;
	machine_width_offset.value = 0;
	machine_depth_offset.value = 0;
	machine_expanded_print_function_enabled.value = 0;
	machine_expanded_print_mode.value = 0;
	machine_expanded_print_cartridgeIndex.value = 0;
	machine_expanded_width_offset.value = 0;
	machine_expanded_depth_offset.value = 0;
	machine_expanded_height_offset.value = 0;
	machine_shape.value = 0;

	machine_bed_selected_enabled.value = 0;
	machine_bed_side_default.value = 0;
	machine_bed_selected_side.value = 0;

	machine_center_is_zero.value = 0;
	machine_max_feedrate_X.value = 600;
	machine_max_feedrate_Y.value = 600;
	machine_max_feedrate_Z.value = 40;
	machine_max_feedrate_E.value = 25;
	machine_min_feedrate.value = 0.01;
	machine_acceleration.value = 2500;
	machine_max_acceleration_X.value = 2500;
	machine_max_acceleration_Y.value = 2500;
	machine_max_acceleration_Z.value = 100;
	machine_max_acceleration_E.value = 10000;
	machine_max_bump_X.value = 25.0;
	machine_max_bump_Y.value = 25.0;
	machine_max_bump_Z.value = 0.4;
	machine_max_bump_E.value = 5.0;

	has_heated_bed.value = 1;
	auto_center.value = 0;
	extruder_count.value = 1;
	cooling_control_enabled.value = 1;
	fan_speed_low_limit_value.value = 3;
	material_selection_enabled.value = 1;
	available_materials = "PLA_ABS";
	printable_material_combinations = "NA";
	warning_material_combinations = "NA";
	is_ETC_all_available.value = 1;
	raft_base_fan_control_enabled.value = 0;
	raft_base_retraction_amount.value = -6;
	raft_base_pathLengthLimit_enabled.value = 0;
	restrict_frontUpperRegion_enabled.value = 1;
	restrict_rearQuadRegion_enabled.value = 1;
	startcode_Z_leveling.value = 3;
	view_cam_distance.value = 420;
	view_cam_center_factor_from_bedcenter_X.value = 1.0;
	view_cam_center_factor_from_bedcenter_Y.value = 1.0;
	view_cam_center_factor_from_bedcenter_Z.value = 1.0;
	thumbnail_image_width.value = 300;
	thumbnail_image_height.value = 360;
	company_code.value = 0;
	has_web_camera.value = 1;
	has_SSD_storage.value = 0;
	//gcode_flavor.value = 0;
	firmware_code.value = 0;
	is_bed_type_selectable.value = 0;

	available_material_list = available_materials.split("_");
	openMode = false;
}

bool MachineProfile::loadMachineProfile(const wchar_t* filepath)
{
	setMachineDataDefault();
	std::ifstream fs(filepath);

	if (fs.is_open())
	{
		std::cout << "Default Machine profile loading complete!!" << ("\n\n");
	}
	else
	{
		std::cout << "Default Machine profile loading failure!!" << ("\n\n");
		return false;
	}

	std::string strTemp;
	std::string valueTemp;
	std::string minValueTemp;
	std::string maxValueTemp;
	bool is_set_extruder_count = false;

	std::string strLine;
	QStringList strlist_nozzle_size;

	printable_material_combination_list.clear();
	warning_material_combination_list.clear();

	QString profile_str;
	QString value_str;
	QString min_value_str;
	QString max_value_str;

	while (std::getline(fs, strLine))
	{
		double value = 0.0;
		double min = 0.0;
		double max = 0.0;

		std::istringstream iss(strLine);
		if (strLine.find("printable_material_combination_list:") != std::string::npos)
		{
			std::string tempStr = "printable_material_combination_list: ";
			std::string valueStr = strLine.substr(strLine.find_first_of(tempStr) + tempStr.length(), strLine.length());
			valueStr.erase(std::find_if(valueStr.rbegin(), valueStr.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), valueStr.end());
			//std::cout << "valueSTr:" << valueStr << "." << std::endl;
			QStringList temp_list = QString(valueStr.c_str()).split("_");
			for (int i = 0; i < temp_list.size(); ++i)
				printable_material_combination_list.push_back(temp_list.at(i).split(":"));
		}
		else if (strLine.find("warning_material_combination_list:") != std::string::npos)
		{
			std::string tempStr = "warning_material_combination_list: ";
			std::string valueStr = strLine.substr(strLine.find_first_of(tempStr) + tempStr.length(), strLine.length());
			valueStr.erase(std::find_if(valueStr.rbegin(), valueStr.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), valueStr.end());
			//std::cout << "valueSTr:" << valueStr << "." << std::endl;
			QStringList temp_list = QString(valueStr.c_str()).split("_");
			for (int i = 0; i < temp_list.size(); ++i)
				warning_material_combination_list.push_back(temp_list.at(i).split(":"));
		}
		else
		{
			strTemp.clear();
			valueTemp.clear();
			minValueTemp.clear();
			maxValueTemp.clear();

			bool result_stream_operator(iss >> strTemp >> valueTemp >> minValueTemp >> maxValueTemp);
			profile_str = QString::fromStdString(strTemp);
			value_str = QString::fromStdString(valueTemp);
			min_value_str = QString::fromStdString(minValueTemp);
			max_value_str = QString::fromStdString(maxValueTemp);

			if (!result_stream_operator)
			{
				if (profile_str.isEmpty()) break;
				else value = QString::fromStdString(valueTemp).toDouble();

				if (profile_str.startsWith("machine_model"))
				{
					machine_model = value_str;
					if (!min_value_str.isEmpty())
						machine_model = machine_model.append(" ").append(min_value_str);
				}
				else if (profile_str.startsWith("group_model"))
				{
					group_model = value_str;
					if (!min_value_str.isEmpty())
						group_model = group_model.append(" ").append(min_value_str);
				}
				else if (profile_str.startsWith("available_materials"))
				{
					available_materials = value_str;
					available_material_list = value_str.split("_");
				}
				iss.clear();
			}
			else
			{
				value = value_str.toDouble();
				min = min_value_str.toDouble();
				max = max_value_str.toDouble();
			}
		}
		//////////////////fs >> strTemp >> valueTemp >> minValueTemp >> maxValueTemp

		if (profile_str.startsWith("machine_width(mm)"))						setProfileData(&machine_width_default, value, min, max);
		else if (profile_str.startsWith("machine_depth(mm)"))					setProfileData(&machine_depth_default, value, min, max);
		else if (profile_str.startsWith("machine_height"))						setProfileData(&machine_height_default, value, min, max);
		else if (profile_str.startsWith("machine_width_offset"))				setProfileData(&machine_width_offset, value, min, max);
		else if (profile_str.startsWith("machine_depth_offset"))				setProfileData(&machine_depth_offset, value, min, max);
		else if (profile_str.startsWith("machine_bed_selected_enabled"))			setProfileData(&machine_bed_selected_enabled, value);
		else if (profile_str.startsWith("machine_bed_side_default"))                setProfileData(&machine_bed_side_default, value, min, max);
		else if (profile_str.startsWith("machine_expanded_print_function_enabled"))	setProfileData(&machine_expanded_print_function_enabled, value);
		else if (profile_str.startsWith("machine_expanded_width_offset"))			setProfileData(&machine_expanded_width_offset, value, min, max);
		else if (profile_str.startsWith("machine_expanded_depth_offset"))			setProfileData(&machine_expanded_depth_offset, value, min, max);
		else if (profile_str.startsWith("machine_expanded_height_offset"))			setProfileData(&machine_expanded_height_offset, value, min, max);
		else if (profile_str.startsWith("machine_shape"))					setProfileData(&machine_shape, value, min, max);
		else if (profile_str.startsWith("machine_center_is_zero"))			setProfileData(&machine_center_is_zero, value);
		else if (profile_str.startsWith("machine_max_feedrate_X"))			setProfileData(&machine_max_feedrate_X, value);
		else if (profile_str.startsWith("machine_max_feedrate_Y"))			setProfileData(&machine_max_feedrate_Y, value);
		else if (profile_str.startsWith("machine_max_feedrate_Z"))			setProfileData(&machine_max_feedrate_Z, value);
		else if (profile_str.startsWith("machine_max_feedrate_E"))			setProfileData(&machine_max_feedrate_E, value);
		else if (profile_str.startsWith("machine_min_feedrate"))			setProfileData(&machine_min_feedrate, value);
		else if (profile_str.startsWith("machine_acceleration"))			setProfileData(&machine_acceleration, value);
		else if (profile_str.startsWith("machine_max_acceleration_X"))		setProfileData(&machine_max_acceleration_X, value);
		else if (profile_str.startsWith("machine_max_acceleration_Y"))		setProfileData(&machine_max_acceleration_Y, value);
		else if (profile_str.startsWith("machine_max_acceleration_Z"))		setProfileData(&machine_max_acceleration_Z, value);
		else if (profile_str.startsWith("machine_max_acceleration_E"))		setProfileData(&machine_max_acceleration_E, value);
		else if (profile_str.startsWith("machine_max_bump_X"))				setProfileData(&machine_max_bump_X, value);
		else if (profile_str.startsWith("machine_max_bump_Y"))				setProfileData(&machine_max_bump_Y, value);
		else if (profile_str.startsWith("machine_max_bump_Z"))				setProfileData(&machine_max_bump_Z, value);
		else if (profile_str.startsWith("machine_max_bump_E"))				setProfileData(&machine_max_bump_E, value);
		else if (profile_str.startsWith("is_ETC_all_available"))            setProfileData(&is_ETC_all_available, value);

		else if (profile_str.startsWith("has_heated_bed"))					setProfileData(&has_heated_bed, value);
		else if (profile_str.startsWith("auto_center"))						setProfileData(&auto_center, value);
		else if (profile_str.startsWith("extruder_count"))
		{
			setProfileData(&extruder_count, value, min, max);

			if (value != 0)
			{
				for (int i = 0; i < value; ++i)
					strlist_nozzle_size.push_back(QString("nozzle_size_T%1").arg(i));

				is_set_extruder_count = true;
			}
		}
		else if (profile_str.startsWith("cooling_control_enabled"))				setProfileData(&cooling_control_enabled, value);
		else if (profile_str.startsWith("fan_speed_low_limit_value"))			setProfileData(&fan_speed_low_limit_value, value, min, max);
		else if (profile_str.startsWith("material_selection_enabled"))			setProfileData(&material_selection_enabled, value);
		else if (profile_str.startsWith("raft_base_fan_control_enabled"))		setProfileData(&raft_base_fan_control_enabled, value);
		else if (profile_str.startsWith("raft_base_retraction_amount"))			setProfileData(&raft_base_retraction_amount, value, min, max);
		else if (profile_str.startsWith("raft_base_pathLengthLimit_enabled"))	setProfileData(&raft_base_pathLengthLimit_enabled, value);
		else if (profile_str.startsWith("restrict_frontUpperRegion_enabled"))	setProfileData(&restrict_frontUpperRegion_enabled, value);
		else if (profile_str.startsWith("restrict_rearQuadRegion_enabled"))		setProfileData(&restrict_rearQuadRegion_enabled, value);
		else if (profile_str.startsWith("startcode_Z_leveling"))				setProfileData(&startcode_Z_leveling, value, min, max);
		else if (profile_str.startsWith("view_cam_distance"))					setProfileData(&view_cam_distance, value, min, max);
		else if (profile_str.startsWith("view_cam_center_factor_from_bedcenter_X"))					setProfileData(&view_cam_center_factor_from_bedcenter_X, value, min, max);
		else if (profile_str.startsWith("view_cam_center_factor_from_bedcenter_Y"))					setProfileData(&view_cam_center_factor_from_bedcenter_Y, value, min, max);
		else if (profile_str.startsWith("view_cam_center_factor_from_bedcenter_Z"))					setProfileData(&view_cam_center_factor_from_bedcenter_Z, value, min, max);
		else if (profile_str.startsWith("thumbnail_image_width"))				setProfileData(&thumbnail_image_width, value, min, max);
		else if (profile_str.startsWith("thumbnail_image_height"))				setProfileData(&thumbnail_image_height, value, min, max);
		else if (profile_str.startsWith("company"))								setProfileData(&company_code, value, min, max);
		else if (profile_str.startsWith("has_web_camera"))						setProfileData(&has_web_camera, value);
		else if (profile_str.startsWith("has_SSD_storage"))						setProfileData(&has_SSD_storage, value);
		//else if (profile_str.startsWith("gcode_flavor"))				setProfileData(&gcode_flavor, value, min, max);
		else if (profile_str.startsWith("firmware_code"))						setProfileData(&firmware_code, value, min, max);
		else if (profile_str.startsWith("is_bed_type_selectable"))				setProfileData(&is_bed_type_selectable, value);
		else
		{
			//temporary code..//
			firmware_code.value = 0;
		}

		minValueTemp.clear();
		maxValueTemp.clear();
	}

	if (extruder_count.value != 0 && is_set_extruder_count)
	{
		nozzle_size.resize(extruder_count.value);
		
		//reset//
		fs.clear();
		fs.seekg(0, std::ios::beg);
		strLine.clear();
		int count_extruder_index = 0;

		while (std::getline(fs, strLine))
		{
			std::string name;
			std::string profile_value_stringlist;

			const auto equals_idx = strLine.find_first_of(": ");
			if (std::string::npos != equals_idx)
			{
				name = strLine.substr(0, equals_idx);
				profile_value_stringlist = strLine.substr(equals_idx + 2);
			}

			if (QString::fromStdString(strLine).startsWith(strlist_nozzle_size.at(count_extruder_index)))
			{
				std::string value;
				std::string min_value;
				std::string max_value;
				std::istringstream iss(profile_value_stringlist);

				if (!(iss >> value >> min_value >> max_value))
				{
					iss.clear();
					return false;
				}
				else
				{
					setProfileData(
						&nozzle_size.at(count_extruder_index),
						QString::fromStdString(value).toDouble(),
						QString::fromStdString(min_value).toDouble(),
						QString::fromStdString(max_value).toDouble());
				}

				count_extruder_index++;

				if (count_extruder_index >= strlist_nozzle_size.size())
					break;
			}
		}

	}


	fs.close();
	
	return true;
}

void MachineProfile::setProfileData(ProfileDataD* data, double value, double min, double max)
{
	data->value = value;
	data->min = min;
	data->max = max;
}

void MachineProfile::setProfileData(ProfileDataD* data, double value)
{
	data->value = value;
}

void MachineProfile::setProfileData(ProfileDataB* data, bool value)
{
	data->value = value;
}

void MachineProfile::setProfileData(ProfileDataI* data, int value, int min, int max)
{
	data->value = value;
	data->min = min;
	data->max = max;
}

bool MachineProfile::loadRecentMachineProfile(const wchar_t* filepath)
{
	std::ifstream fs(filepath);

	if (fs.is_open())
	{
		std::cout << "success to load recent machine profile.." << ("\n\n");
	}
	else
	{
		std::cout << "fail to load recent machine profile.." << ("\n\n");
		return false;
	}

	if (Profile::machineProfile.extruder_count.value == 0)
	{
		std::cout << "fail to load recent machine profile -> Profile::machineProfile.extruder_count value = 0 " << ("\n\n");
		return false;
	}

	std::string strLine;
	QStringList strlist_nozzle_size;
	int count_extruder_index = 0;

	for (int i = 0; i < Profile::machineProfile.extruder_count.value; ++i)
		strlist_nozzle_size.push_back(QString("nozzle_size_T%1").arg(i));

	nozzle_size.resize(Profile::machineProfile.extruder_count.value);

	while (std::getline(fs, strLine))
	{
		//std::string name;
		std::string profile_value_stringlist;

		const auto equals_idx = strLine.find_first_of(": ");
		if (std::string::npos != equals_idx)
		{
			//name = strLine.substr(0, equals_idx);
			profile_value_stringlist = strLine.substr(equals_idx + 2);
		}

		if (QString::fromStdString(strLine).startsWith(strlist_nozzle_size.at(count_extruder_index)))
		{
			std::string value;
			std::string min_value;
			std::string max_value;
			std::istringstream iss(profile_value_stringlist);

			if (!(iss >> value >> min_value >> max_value))
			{
				std::cout << "fail to load recent machine profile -> nozzle size data type mismatch" << ("\n\n");
				return false;
			}
			else
			{
				setProfileData(
					&nozzle_size.at(count_extruder_index),
					QString::fromStdString(value).toDouble(),
					QString::fromStdString(min_value).toDouble(),
					QString::fromStdString(max_value).toDouble());
			}

			count_extruder_index++;

			if (count_extruder_index >= strlist_nozzle_size.size())
				break;
		}
	}

	if (count_extruder_index == 0 || (count_extruder_index =! Profile::machineProfile.extruder_count.value))
	{
		std::cout << "fail to load recent machine profile -> not found nozzle_size or mismatch extruder count" << ("\n\n");
		return false;
	}


	fs.close();

	return true;
}


void MachineProfile::saveRecentMachineProfile(const wchar_t* filepath)
{
	std::ofstream fs(filepath);

	if (!(fs.is_open())) return;

	//fs << ("machine_width(mm):") << (" ") << machine_width_default.value << ("\n");
	//fs << ("machine_depth(mm):") << (" ") << machine_depth_default.value << ("\n");
	//fs << ("machine_height(mm):") << (" ") << machine_height_default.value << ("\n");
	//fs << ("machine_width_offset(mm):") << (" ") << machine_width_offset.value << ("\n");
	//fs << ("machine_depth_offset(mm):") << (" ") << machine_depth_offset.value << ("\n");
	//fs << ("machine_expanded_print_function_enabled(0=False,1=True):") << (" ") << machine_expanded_print_function_enabled.value << ("\n");
	//fs << ("machine_expanded_width_offset(mm):") << (" ") << machine_expanded_width_offset.value << ("\n");
	//fs << ("machine_expanded_depth_offset(mm):") << (" ") << machine_expanded_depth_offset.value << ("\n");
	//fs << ("machine_expanded_height_offset(mm):") << (" ") << machine_expanded_height_offset.value << ("\n");
	//fs << ("machine_bed_selected_enabled(0=False,1=True):") << (" ") << machine_bed_selected_enabled.value << ("\n");
	//fs << ("machine_bed_side_default(0=side_a,1=side_b):") << (" ") << machine_bed_side_default.value << ("\n");
	//fs << ("machine_bed_selected_side:") << (" ") << machine_bed_selected_side.value << ("\n");
	//fs << ("machine_shape:") << (" ") << machine_shape.value << ("\n");
	//fs << ("machine_center_is_zero(0=False,1=True):") << (" ") << machine_center_is_zero.value << ("\n");
	//fs << ("machine_max_feedrate_X(mm/s):") << (" ") << machine_max_feedrate_X.value << ("\n");
	//fs << ("machine_max_feedrate_Y(mm/s):") << (" ") << machine_max_feedrate_Y.value << ("\n");
	//fs << ("machine_max_feedrate_Z(mm/s):") << (" ") << machine_max_feedrate_Z.value << ("\n");
	//fs << ("machine_max_feedrate_E(mm/s):") << (" ") << machine_max_feedrate_E.value << ("\n");
	//fs << ("machine_min_feedrate(mm/s):") << (" ") << machine_min_feedrate.value << ("\n");
	//fs << ("machine_acceleration(mm/s^2):") << (" ") << machine_acceleration.value << ("\n");
	//fs << ("machine_max_acceleration_X(mm/s^2):") << (" ") << machine_max_acceleration_X.value << ("\n");
	//fs << ("machine_max_acceleration_Y(mm/s^2):") << (" ") << machine_max_acceleration_Y.value << ("\n");
	//fs << ("machine_max_acceleration_Z(mm/s^2):") << (" ") << machine_max_acceleration_Z.value << ("\n");
	//fs << ("machine_max_acceleration_E(mm/s^2):") << (" ") << machine_max_acceleration_E.value << ("\n");
	//fs << ("machine_max_bump_X(mm/s):") << (" ") << machine_max_bump_X.value << ("\n");
	//fs << ("machine_max_bump_Y(mm/s):") << (" ") << machine_max_bump_Y.value << ("\n");
	//fs << ("machine_max_bump_Z(mm/s):") << (" ") << machine_max_bump_Z.value << ("\n");
	//fs << ("machine_max_bump_E(mm/s):") << (" ") << machine_max_bump_E.value << ("\n");
	//fs << ("has_heated_bed(0=False,1=True):") << (" ") << has_heated_bed.value << ("\n");
	//fs << ("auto_center(0=False,1=True):") << (" ") << auto_center.value << ("\n");
	//fs << ("extruder_count:") << (" ") << extruder_count.value << ("\n");
	for (int i = 0; i < extruder_count.value; ++i)
	{
		fs << QString("nozzle_size_T%1:").arg(i).toStdString().c_str() << (" ") << nozzle_size.at(i).value << (" ") << nozzle_size.at(i).min << (" ") << nozzle_size.at(i).max << ("\n");
	}
	//fs << ("cooling_control_enabled(0=False,1=True):") << (" ") << cooling_control_enabled.value << ("\n");
	//fs << ("fan_speed_low_limit_value(%):") << (" ") << fan_speed_low_limit_value.value << ("\n");
	//fs << ("material_selection_enabled(0=False,1=True):") << (" ") << material_selection_enabled.value << ("\n");
	//fs << ("available_materials:") << (" ") << available_materials.toStdString() << ("\n");
	//fs << ("printable_material_combination_list:") << (" ") << printable_material_combinations.toStdString() << ("\n");
	//fs << ("warning_material_combination_list:") << (" ") << warning_material_combinations.toStdString() << ("\n");
	//fs << ("is_ETC_all_available(0=False,1=True):") << (" ") << is_ETC_all_available.value << ("\n");
	//fs << ("raft_base_fan_control_enabled(0=False,1=True):") << (" ") << raft_base_fan_control_enabled.value << ("\n");
	//fs << ("raft_base_retraction_amount(mm):") << (" ") << raft_base_retraction_amount.value << (" ") << raft_base_retraction_amount.min << (" ") << raft_base_retraction_amount.max << ("\n");
	//fs << ("raft_base_pathLengthLimit_enabled(0=False,1=True):") << (" ") << raft_base_pathLengthLimit_enabled.value << ("\n");
	//fs << ("restrict_frontUpperRegion_enabled(0=False,1=True):") << (" ") << restrict_frontUpperRegion_enabled.value << ("\n");
	//fs << ("restrict_rearQuadRegion_enabled(0=False,1=True):") << (" ") << restrict_rearQuadRegion_enabled.value << ("\n");
	//fs << ("startcode_Z_leveling(mm):") << (" ") << startcode_Z_leveling.value << (" ") << startcode_Z_leveling.min << (" ") << startcode_Z_leveling.max << ("\n");
	//fs << ("view_cam_distance:") << (" ") << view_cam_distance.value << ("\n");
	//fs << ("view_cam_center_factor_from_bedcenter_X:") << (" ") << view_cam_center_factor_from_bedcenter_X.value << ("\n");
	//fs << ("view_cam_center_factor_from_bedcenter_Y:") << (" ") << view_cam_center_factor_from_bedcenter_Y.value << ("\n");
	//fs << ("view_cam_center_factor_from_bedcenter_Z:") << (" ") << view_cam_center_factor_from_bedcenter_Z.value << ("\n");
	//fs << ("thumbnail_image_width(pixel):") << (" ") << thumbnail_image_width.value << ("\n");
	//fs << ("thumbnail_image_height(pixel):") << (" ") << thumbnail_image_height.value << ("\n");
	//fs << ("company(0=Sindoh, 1=Stanley, 2=Leon3D):") << (" ") << company_code.value << ("\n");
	//fs << ("has_web_camera(0=False,1=True):") << (" ") << has_web_camera.value << ("\n");
	//fs << ("has_SSD_storage(0=False,1=True):") << (" ") << has_SSD_storage.value << ("\n");
	////fs << ("gcode_flavor(0=REPRAP,1=ULTIGCODE,2=MAKERBOT,3=BFB,4=MACH3,5=VOLUMATRIC):") << (" ") << gcode_flavor.value << ("\n");
	//fs << ("firmware_code(0=MARVELL,1=MARLIN):") << (" ") << firmware_code.value << ("\n");


	fs.close();

}

void MachineProfile::loadStartEndDefaultCode(const wchar_t* filepath)
{
	Profile::machineProfile.start_default_code.clear();
	Profile::machineProfile.end_default_code.clear();

	std::ifstream fs(filepath);

	if (fs.is_open()) std::cout << "Start_End file loading complete!!" << ("\n\n");
	else
	{
		std::cout << "Start_End file loading failure!!" << ("\n\n");
		return;
	}

	std::string strTmp;

	std::getline(fs, strTmp);

	if (strTmp == ";[START CODE]")
	{
		std::cout << "ok" << std::endl;

		while (std::getline(fs, strTmp))
		{
			if (strTmp == ";[END CODE]") break;

			strTmp.append("\n");
			Profile::machineProfile.start_default_code.append(strTmp);

		};

		while (std::getline(fs, strTmp))
		{
			strTmp.append("\n");
			Profile::machineProfile.end_default_code.append(strTmp);
		}
	}
	else
	{
		std::cout << "check the ;[START CODE] tag!!" << std::endl;
		fs.close();

		return;
	}

	fs.close();

}
