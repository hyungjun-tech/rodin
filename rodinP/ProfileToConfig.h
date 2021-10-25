#pragma once

#include "settings.h"
#include "SliceProfile.h"
#include "SliceProfileForCommon.h"
#include "MachineProfile.h"

class ProfileToConfig
{
public:
	ProfileToConfig();
	~ProfileToConfig();

	static void convertToConfig();
	static double	calculateEdgeWidth(SliceProfile*, const int _extruder_index);

	//void	LoadingProfile(void);
	//void	convertToConfig(SliceProfile*, ConfigSettings*);
	//void	convertToConfig(MachineProfile*, ConfigSettings*);
	void convertToConfig(std::vector<SliceProfile> *sliceProfile, SliceProfileForCommon *sliceProfile_common, std::vector<ConfigSettings> *config);
	static void	convertToConfig(SliceProfile*, SliceProfileForCommon*, ConfigSettings*, const int _extruder_index);
	
private:
	static void convertToConfig(SliceProfile*, ConfigSettings*, const int _extruder_index);

	static int calculateLineCount(SliceProfile*, const int _extruder_index);
	static int calculateSolidLayerCount(SliceProfile*);
	//static int calculateObjectSizeOffsets(SliceProfile*);

	//static void	getMachineCenterCoords(SliceProfile*);
	//static void	getMachineSizePolygons(SliceProfile*);
	static int minimalExtruderCount(SliceProfile*);
	static void	raftSettingReset(ConfigSettings*);

	static std::string generatePreSwitchExtruderCode(SliceProfile*);
	static std::string generatePostSwitchExtruderCode(SliceProfile*);
};