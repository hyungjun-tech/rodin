#pragma once

#include "SliceProfile.h"
#include "SliceProfileForCommon.h"
#include "settings.h"

class ProfileControl
{
public:
	ProfileControl();
	//ProfileControl(std::vector<SliceProfile>*, SliceProfileForCommon*, std::vector<ConfigSettings>*);
	~ProfileControl();

	static bool loadMachineProfile();
	static bool loadProfileByMachine(QString settingMode = "easy");
	static bool loadProfileByMachine(QString settingMode, std::vector<SliceProfile>*, SliceProfileForCommon*, std::vector<ConfigSettings>*);
	//void loadProfileByMaterial(SliceProfile *,int);

	//for multi-cartridge///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//void updateCartridgeCount();
	static void updateConfigForMultiCartridge();
	static void updateConfigForMultiCartridge(std::vector<SliceProfile>*, SliceProfileForCommon*, std::vector<ConfigSettings>*);
	static void generateCommonProfileFromSliceProfile(int = -1, int = -1, int = -1, int = -1);
	static void generateCommonProfileFromSliceProfile(std::vector<SliceProfile>*, SliceProfileForCommon*, int = -1, int = -1, int = -1, int = -1);
	static void setRaftProfile(std::vector<SliceProfile>*, SliceProfileForCommon*, int = -1, int = -1);
	static void setSupportProfile(std::vector<SliceProfile>*, SliceProfileForCommon*, int = -1, int = -1);
	static void setCommonProfile(std::vector<SliceProfile>*, SliceProfileForCommon*);

	//static void setSliceProfileFromCommon(std::vector<SliceProfile>*, SliceProfileForCommon*);

	//for multi-cartridge/////////////////////////////////////
	//std::vector<SliceProfile> *m_sliceProfile_multi;
	//std::vector<ConfigSettings> *m_config_multi;
	//std::vector<SliceProfileForCartridge> *m_sliceProfile_cartridge;
	//SliceProfileForCommon *m_sliceProfile_common;
	//////////////////////////////////////////////////////////
	//std::vector<int> m_materialAvailableIndex;
	//std::vector<const char*> m_materialList;

	static bool resetProfile();
	static bool resetProfile(std::vector<SliceProfile>*, SliceProfileForCommon*);
	static bool importProfile(QString, int);
	static bool importProfile(QString, int, std::vector<SliceProfile>*, SliceProfileForCommon*);
	static bool exportProfile(QString, int);
	static bool exportProfile(QString, int, std::vector<SliceProfile>, SliceProfileForCommon);
	static bool exportProfile(QString, SliceProfile, SliceProfileForCommon);

	static bool resetEasyProfile();
	static bool resetEasyProfile(std::vector<SliceProfile>*, SliceProfileForCommon*);
	static bool loadRecentProfile();
	static bool loadRecentProfile(std::vector<SliceProfile>* _sliceProfile_multi, SliceProfileForCommon* _sliceProfile_common, MachineProfile* _machine_profile);
	static void saveRecentProfile(QString _settingMode);
	static void saveRecentProfile(QString _settingMode, std::vector<SliceProfile> _sliceProfile_multi, SliceProfileForCommon _sliceProfile_common);
	static void saveRecentMachineProfile(MachineProfile _machineProfile);

	static ProfileDataI getMaxBedTemperature(std::vector<SliceProfile> sliceProfile);

	static bool checkSupportProfileChanged(ConfigSettings beforeConfig_, ConfigSettings currentConfig_);
private:
	static int getCartIndex_maxWeight(std::vector<SliceProfile>*);

};

