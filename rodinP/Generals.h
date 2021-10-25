#pragma once

#include <QString>
#include <QStringList>
#include <QDir>
#include <vector>
#include <QStandardPaths>


class Generals
{
public:
	//enum
	enum ViewerMode { TranslateUIMode, LayerColorUIMode, ReplicationUIMode, RotateUIMode, SupportUIMode, PreviewUIMode, ChangeBottomUIMode, AnalysisUIMode, None };
	enum PlatformAdhesion { NoneAdhesion, Skirt, Brim, Raft };
	enum SupportPlacement { SupportNone, SupportTouching, SupportEverywhere };
	enum SupportType { SupportGrid, SupportLines, SupportZigzag };
	enum Combing { Off, All, NoSkin };
	enum InternalMoving_Area { Narrow, Defualt, Wide };
	enum WipetowerPosition { RearLeft, RearCenter, RearRight, MiddleLeft, MiddleRight, FrontLeft, FrontCenter, FrontRight };
	enum SkinType { Skin_Line, Skin_Concentric };
	enum InfillPattern { Automatic, Grid, Line, Concentric, Crystal1, Crystal2 };
	enum AuthentificationMethod { Everytime, SaveToPC };
	enum ModelControl { Move, Scale, Rotate };
	enum CheckPrintable { Ok = 0, Warning = 1, NG = 9 };
	enum CheckModelSize { InvadeFrontUpperRegion = -2, InvadeBackCornerRegion, ExceedPrintableRegion, Good };
	enum FirwareCode { MARVELL, MARLIN };
	
	struct ProfileList
	{
		QString name;
		QString description;
		QString fileLocation;
		bool custom;
		QString visible;
	};
	//QString을 wchar_t로 변환하여 전달//
	static wchar_t* qstringTowchar_t(QString qStr);

	//text file copy function//
	static void fileCopyFuction(QString fromPath, QString toPath);

	//get file size function
	static int getFileSize(FILE* fp);
	static int getFileSize(const wchar_t* filePath);

	//
	static int getFileCurrentReadSize(FILE *fp);

	//round function, 자릿수 지정//
	static double roundDouble(double value, int position);

	//std::to_string precision//
	static std::string to_string_with_precision(double _value, int _precision);
	
	static QString getMaterialShortName(QString pMaterial);
	static int getWeightForMaterial(QString pMaterial);

	static int checkMaterialCombination(QString pMaterial1, QString pMaterial2);
	static bool checkMaterialCombination_wipetower(QString pMaterial1, QString pMaterial2);
	static bool check_printable_combination(const std::vector<QString> strList_);
	static bool check_warning_combination(const std::vector<QString> strList_);
	static bool check_material_in_list(const QStringList _str_list, QString _check_material);

	static double convertTemperatureUnitCtoF(double tempC);
	static double convertTemperatureUnitFtoC(double tempF);
	static int convertTemperatureUnitCtoF(int tempC);
	static int convertTemperatureUnitFtoC(int tempF);

	static const QString unknownProfileName;

	static void checkPath(QString pPath);
	static QString getAppPath();
	static QString getAppDataPath();
	static QString getTempFolderPath();
	static QString getFolderPath(QStandardPaths::StandardLocation pathType);

	static void setProps(QString org, QString app, QString propName, QString propValue);
	static void setProps(QString propName, QString propValue);
	static QString getProps(QString org, QString app, QString propName);
	static QString getProps(QString propName);

	static bool stop;
	static QString removeSpecialChar(QString from_);

	static ViewerMode currentViewerMode;
	static bool isLayerColorMode;
	static QString appPath;
	static QString appDataPath;
	static QString tempPath;
	static QString temperatureUnit;
	static bool isTranslateUIMode() { return currentViewerMode == TranslateUIMode; }
	static bool isLayerColorUIMode() { return currentViewerMode == LayerColorUIMode; }
	static bool isReplicationUIMode() { return currentViewerMode == ReplicationUIMode; }
	static bool isRotateUIMode() { return currentViewerMode == RotateUIMode; }
	static bool isSupportUIMode() { return currentViewerMode == SupportUIMode; }
	static bool isPreviewUIMode() { return currentViewerMode == PreviewUIMode; }
	static bool isChangeBottomUIMode() { return currentViewerMode == ChangeBottomUIMode; }
	static bool isAnalysisUIMode() { return currentViewerMode == AnalysisUIMode; }

	//
	static bool isLayerColorModeOn() { return isLayerColorMode; }

	static void setGeometry(QWidget* w_, QPoint center_);
	static void setGeometry(QWidget* w_);
	static void setCenter(int centerX_, int centerY_);
	static QPoint center;

	static bool qstringToBool(QString _str);
	static QString boolToQString(bool _flag);
};