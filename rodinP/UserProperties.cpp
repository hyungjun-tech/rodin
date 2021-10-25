#include "stdafx.h"
#include "UserProperties.h"

//ProfileDataS UserProperties::workspacePath = ProfileDataS();
bool UserProperties::usingDetectCollision = bool(true);
bool UserProperties::showLoadedFileName = bool(true);
bool UserProperties::usingFullFilePath = bool(false);
bool UserProperties::showBedModel = bool(false);
bool UserProperties::showBorderLine = bool(false);
bool UserProperties::permitModelOut = bool(false);
bool UserProperties::thinMessageCheck = bool(false);
bool UserProperties::authentification_print_mode = bool(false);
int UserProperties::authentification_print_setting_method = int(Generals::AuthentificationMethod::Everytime);
bool UserProperties::detailedLogMode = bool(false);

UserProperties::UserProperties()
{
}

UserProperties::~UserProperties()
{
}