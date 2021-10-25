#pragma once

#include "MachineProfile.h"
#include "SliceProfile.h"
#include "SliceProfileForCommon.h"
#include "settings.h"
#include <QGLViewer/qglviewer.h>

class AABB;
class Profile
{
public:
	static MachineProfile machineProfile;
	static std::vector<SliceProfile> sliceProfile;
	static SliceProfileForCommon sliceProfileCommon;
	static std::vector<ConfigSettings> configSettings;
	static int getCartridgeTotalCount();
	/*static ModelDatas modelMatch;
	static ModelData modelData;
	static SliceProfileData sliceProfile;*/

	static QString profilePath;
	static QString recentProfilePath;
	static QString customProfilePath;

	static int getTemperatureByLayer(std::vector<TemperatureLayerSetPoint> *temperature_layer_list, int layerNr, int default_temperature);
	static bool isTemperatureSetPointAtLayer(std::vector<TemperatureLayerSetPoint> *temperature_layer_list, int layerNr);


	///machine..
	static qglviewer::Vec getMachineSize();
	static AABB machineBox;
	static void setMachineAABB();
	static AABB getMachineAABB();
	static qglviewer::Vec getMargin();
	static qglviewer::Vec getRearQuad();
	static qglviewer::Vec getUpperRestrictedBox();
	static qglviewer::Vec getCamCenterFactor();

	//아래 4개 함수는 getMachineAABB() 함수로 변경해야함. 차차 수정하는걸로.
	static qglviewer::Vec getMachineCenter_withOffset();
	static unsigned int getMachineWidth_calculated();
	static unsigned int getMachineDepth_calculated();
	static unsigned int getMachineHeight_calculated();
};