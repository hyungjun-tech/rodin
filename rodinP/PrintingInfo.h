#pragma once
#include "UserProperties.h"
enum SlicerType { SINDOH, CURA, SIMPLIFY3D, SLIC3R, ETC };
class PrintingInfo
{
public:
	PrintingInfo();
	~PrintingInfo();
	void clear();
	void init();

	void setSlicerType(SlicerType slicerType_);
	SlicerType getSlicerType();
	void setGcodeFileName(QFileInfo gcodeFileName_);
	void setGcodeFileName(QString gcodeFileName_);
	QString getGcodeFileName();
	QString getGcodeFileName(bool usingFullFilePath_);
	QString getGcodeBaseFileName();
	QFileInfo getGcodeFileInfo();

	void setGcodeTempFileName();
	void setGcodeTempFileName(QFileInfo gcodeTempFileName_);
	void setGcodeTempFileName(QString gcodeTempFileName_);
	QString getGcodeTempFileName();
	QFileInfo getGcodeTempFileInfo();

	void setIsLoadedGcode(bool isLoadedGcode_);
	bool getIsLoadedGcode();

	void setIsReplicationPrint(bool isReplicationPrint_);
	bool getIsReplicationPrint();

	bool setEncodedImage(QString imagePath_);
	void deleteEncodedImage();
	void appendEncodedImage(QString encodedImage_);
	QString getEncodedImage();
	bool hasEncodedImage();

	void setMachineModel(QString machineModel_);
	QString getMachineModel();

	void setOperatingZone(qglviewer::Vec operatingZone_);
	void setOperatingZoneX(float x_);
	void setOperatingZoneY(float y_);
	void setOperatingZoneZ(float z_);
	qglviewer::Vec getOperatingZone();

	void setTotalPrintTime(int totalPrintTime_);
	int getTotalPrintTime();
	QString getTotalPrintTimeString();

	void setCartridgeCountWithInit(int cartridgeCount_);
	void setCartridgeCount(int cartridgeCount_);
	int getCartridgeCount();

	void setUsedCartridgeCount(int usedCartridgeCount_);
	int getUsedCartridgeCount();

	void setFilaAmount(int index_, float value_);
	float getFilaAmount(int index_);
	std::vector<float> getFilaAmount();
	float getFilaAmountTotal();

	void setFilaMass(int index_, float value_);
	float getFilaMass(int index_);
	std::vector<float> getFilaMass();
	float getfilaMassTotal();

	void setMaterial(int index_, QString value_);
	QString getFilaMaterial(int index_);
	std::vector<QString> getFilaMaterial();

	void setUsedState(int index_, bool value_);
	bool getUsedState(int index_);
	std::vector<bool> getUsedState();
	std::string getUseStateTFString();
	std::vector<int> getUsedCartridgeIndex();
private:
	SlicerType slicerType;
	QFileInfo gcodeFileName;
	QFileInfo gcodeTempFileName;
	bool isLoadedGcode;
	bool isReplicationPrint;
	QString encodedImage;
	QString machineModel;
	qglviewer::Vec operatingZone;

	int totalPrintTime;
	int cartridgeCount;
	int usedCartridgeCount;
	std::vector<float> filaAmount;
	std::vector<float> filaMass;
	std::vector<QString> material;
	std::vector<bool> usedState;
};

