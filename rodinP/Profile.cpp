#include "stdafx.h"
#include "profile.h"
#include "AABB.h"

MachineProfile Profile::machineProfile = MachineProfile();
std::vector<SliceProfile> Profile::sliceProfile = std::vector<SliceProfile>{};
SliceProfileForCommon Profile::sliceProfileCommon = SliceProfileForCommon();
std::vector<ConfigSettings> Profile::configSettings = std::vector<ConfigSettings>{};

QString Profile::profilePath = QString();
QString Profile::recentProfilePath = QString();
QString Profile::customProfilePath = QString();
AABB Profile::machineBox = AABB();

//ModelDatas Profile::modelMatch = ModelDatas();
//ModelData Profile::modelData = ModelData();
//SliceProfileData Profile::sliceProfile = SliceProfileData();

int Profile::getCartridgeTotalCount()
{
	return machineProfile.extruder_count.value;
}

int Profile::getTemperatureByLayer(std::vector<TemperatureLayerSetPoint> *temperature_layer_list, int layerNr, int temperature_default)
{
	int vec_size_temperatureList = temperature_layer_list->size();

	if (vec_size_temperatureList == 1)
	{
		if (temperature_layer_list->at(0).layerNr <= layerNr)
			return temperature_layer_list->at(0).temperature;
		else
			return temperature_default;
	}

	if (temperature_layer_list->begin()->layerNr > layerNr)
	{
		return temperature_default;
	}
	else if (temperature_layer_list->begin()->layerNr <= layerNr && temperature_layer_list->at(vec_size_temperatureList - 1).layerNr > layerNr)
	{
		for (int i = 0; i < temperature_layer_list->size() - 1; i++)
		{
			if (temperature_layer_list->at(i).layerNr <= layerNr && layerNr < temperature_layer_list->at(i + 1).layerNr)
			{
				return temperature_layer_list->at(i).temperature;
			}
		}
	}
	else //pTemperatureProfile.temperatureLayerList_cartridge.at(cartridgeIndex).end()->layerNr <= layerNr
	{
		return temperature_layer_list->at(vec_size_temperatureList - 1).temperature;
	}

	return temperature_default;
}
bool Profile::isTemperatureSetPointAtLayer(std::vector<TemperatureLayerSetPoint> *temperature_layer_list, int layerNr)
{
	for (int i = 0; i < temperature_layer_list->size(); i++)
	{
		if (temperature_layer_list->at(i).layerNr == layerNr)
			return true;
	}

	return false;
}

qglviewer::Vec Profile::getMachineCenter_withOffset()
{
	return qglviewer::Vec
	(
		Profile::getMachineWidth_calculated() * 0.5 + Profile::machineProfile.machine_width_offset.value,
		Profile::getMachineDepth_calculated() * 0.5 + Profile::machineProfile.machine_depth_offset.value,
		Profile::getMachineHeight_calculated() * 0.5
	);
}

qglviewer::Vec Profile::getMachineSize()
{
	qglviewer::Vec rtn(machineProfile.machine_width_default.value,
		machineProfile.machine_depth_default.value,
		machineProfile.machine_height_default.value);

	if (machineProfile.machine_expanded_print_mode.value)
	{
		qglviewer::Vec offset(machineProfile.machine_expanded_width_offset.value,
			machineProfile.machine_expanded_depth_offset.value,
			machineProfile.machine_expanded_height_offset.value);
		rtn += offset;
	}
	return rtn;
}
void Profile::setMachineAABB()
{
	machineBox.clear();
	machineBox.expand(qglviewer::Vec(0, 0, 0));
	machineBox.expand(getMachineSize());
	qglviewer::Vec offset(machineProfile.machine_width_offset.value, machineProfile.machine_depth_offset.value, 0);
	machineBox.translate(offset);
}
AABB Profile::getMachineAABB()
{
	return machineBox;
}
qglviewer::Vec Profile::getMargin()
{
	ConfigSettings config = configSettings[0];

	float xy_margin = 0;
	float z_margin = 0;
	if (config.raft_base_thickness > 0 && config.raft_interface_thickness > 0) // raft가 있을 때..
	{
		xy_margin = (float)config.raft_margin
			+ (float)0.5*sqrt(2)*config.raft_base_line_width
			+ (float)config.extrusion_width;

		z_margin = config.raft_base_thickness
			+ config.raft_interface_thickness
			+ config.raft_surface_thickness * config.raft_surface_layers
			+ config.raft_airgap_all + 100; //0.1 -> z margin
	}
	else if (config.skirt_line_count != 0) // brim or skirt
		xy_margin = (float)config.skirt_line_count*config.initial_layer_extrusion_width
			+ (float)config.skirt_distance
			+ (float)0.5*sqrt(2)*config.initial_layer_extrusion_width
			+ (float)config.initial_layer_extrusion_width;

	//support가 있을 때, horizontal expansion을 고려..//
	//if (Profile::configSettings[config.support_main_cartridge_index].support_placement != Generals::SupportPlacement::SupportNone)// profileToConfig할때 common의 값을 모든 config에 반영하므로 동일함.
	if (config.support_placement != Generals::SupportPlacement::SupportNone)
		xy_margin += (float)config.support_horizontal_expansion;

	qglviewer::Vec rtn(xy_margin, xy_margin, z_margin);
	rtn *= 0.001;

	return rtn;
}
qglviewer::Vec Profile::getRearQuad()
{
	if (!machineProfile.restrict_rearQuadRegion_enabled.value)
		return qglviewer::Vec();
	return qglviewer::Vec(4, 14, 0);
}
qglviewer::Vec Profile::getUpperRestrictedBox()
{
	if (!machineProfile.restrict_frontUpperRegion_enabled.value)
		return qglviewer::Vec();
	return qglviewer::Vec(machineProfile.machine_width_default.value, 12, 10);
}
qglviewer::Vec Profile::getCamCenterFactor()
{
	return qglviewer::Vec(machineProfile.view_cam_center_factor_from_bedcenter_X.value,
		machineProfile.view_cam_center_factor_from_bedcenter_Y.value,
		machineProfile.view_cam_center_factor_from_bedcenter_Z.value);
}








unsigned int Profile::getMachineWidth_calculated()
{
	if (machineProfile.machine_expanded_print_mode.value)
		return machineProfile.machine_width_default.value + Profile::machineProfile.machine_expanded_width_offset.value;
	else
		return machineProfile.machine_width_default.value;
}
unsigned int Profile::getMachineDepth_calculated()
{
	if (machineProfile.machine_expanded_print_mode.value)
		return machineProfile.machine_depth_default.value + Profile::machineProfile.machine_expanded_depth_offset.value;
	else
		return machineProfile.machine_depth_default.value;
}
unsigned int Profile::getMachineHeight_calculated()
{
	if (machineProfile.machine_expanded_print_mode.value)
		return machineProfile.machine_height_default.value + Profile::machineProfile.machine_expanded_height_offset.value;
	else
		return machineProfile.machine_height_default.value;
}