#pragma once

class UserProperties
{
public:
	UserProperties();
	~UserProperties();

	//static ProfileDataS workspacePath;
	static bool usingDetectCollision;
	static bool showLoadedFileName;
	static bool usingFullFilePath;

	static bool thinMessageCheck;

	static bool	authentification_print_mode;
	static int	authentification_print_setting_method;

	static bool detailedLogMode;
	//profileVersion
	//temperatureUnit
	//settingMode
	//easyModeProperty
	//email
	//emailCount
	//language
	//usingCustomColor
	//customColor -> array


	//현재 사용안함.
	static bool showBedModel;
	static bool showBorderLine;
	static bool permitModelOut;
};