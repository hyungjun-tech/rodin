#include "stdafx.h"
#include "PrintingInfo.h"
#include "profile.h"
#include "CartridgeInfo.h"


PrintingInfo::PrintingInfo()
	: slicerType(SINDOH)
	, gcodeFileName(QFileInfo())
	, gcodeTempFileName(QFileInfo())
	, isLoadedGcode(false)
	, isReplicationPrint(false)
	, encodedImage("")
	, machineModel("NONE")
	, operatingZone(qglviewer::Vec(0, 0, 0))
	, totalPrintTime(0)
	, cartridgeCount(1)
	, usedCartridgeCount(1)
	, filaAmount({ 0 })
	, filaMass({ 0 })
	, material({ "NONE" })
	, usedState({ true })
{
}

PrintingInfo::~PrintingInfo()
{
}

void PrintingInfo::clear()
{
	slicerType = SINDOH;
	gcodeFileName.setFile("");
	//gcodeTempFileName.setFile("");//굳이 재생성하지 않고 재사용하도록 temp는 초기화 하지 않음.
	isLoadedGcode = false;
	isReplicationPrint = false;
	encodedImage = "";
	machineModel = "NONE";
	operatingZone = qglviewer::Vec(0, 0, 0);
	totalPrintTime = 0;
	cartridgeCount = 1;
	usedCartridgeCount = 1;
	filaAmount = { 0 };
	filaMass = { 0 };
	material = { "NONE" };
	usedState = { true };
}

void PrintingInfo::init()
{
	clear();
	int cartCount = CartridgeInfo::getCartCount();
	setCartridgeCountWithInit(cartCount);

	//std::vector<bool> usedCart = CartridgeInfo::getUsedCartridge();
	for (int i = 0; i < cartCount; i++)
	{
		//setUsedState(i, usedCart[i]);
		setMaterial(i, Profile::sliceProfile[i].filament_material.value);
	}
	setMachineModel(Profile::machineProfile.group_model);
	//useState -> filaAmount가 0이 아닐때
	//totalPrintTime -> fprintf(out_f, "\n;ESTIMATION_TIME: [%04d:%02d:%02d]\n", int(gcode.getTotalPrintTime()) / 3600, int(gcode.getTotalPrintTime()) % 3600 / 60, int(gcode.getTotalPrintTime()) % 60);
	//filaAmount -> m_filamentAmount[i] = gcode.getTotalFilamentUsed(i);
	//filaMass -> m_filamentMass[i] = (3.14159265358979*1.75*1.75 / 4) * m_filamentAmount[i] * filamentDensity; //filament area * length * density;
	//슬라이싱 이후 설정해줘야 함.
}

void PrintingInfo::setSlicerType(SlicerType slicerType_)
{
	slicerType = slicerType_;
}
SlicerType PrintingInfo::getSlicerType()
{
	return slicerType;
}

void PrintingInfo::setGcodeFileName(QFileInfo gcodeFileName_)
{
	gcodeFileName = gcodeFileName_;
}
void PrintingInfo::setGcodeFileName(QString gcodeFileName_)
{
	gcodeFileName.setFile(gcodeFileName_);
}
QString PrintingInfo::getGcodeFileName()
{
	return gcodeFileName.absoluteFilePath();
}
QString PrintingInfo::getGcodeFileName(bool usingFullFilePath_)
{
	QString name;
	if (usingFullFilePath_)
		name = gcodeFileName.absoluteFilePath();
	else
		name = gcodeFileName.fileName();
	return name;
}
QString PrintingInfo::getGcodeBaseFileName()
{
	return gcodeFileName.completeBaseName();
}
QFileInfo PrintingInfo::getGcodeFileInfo()
{
	return gcodeFileName;
}
void PrintingInfo::setGcodeTempFileName()
{
	if (gcodeTempFileName.isFile())
		return;
	QString currentTime = QTime::currentTime().toString();
	currentTime = currentTime.replace(":", "_");
	QString tempFileName = Generals::getTempFolderPath().append("\\") + currentTime + ".tempG";
	gcodeTempFileName.setFile(tempFileName);
	if (!gcodeFileName.isFile())
		gcodeFileName.setFile(tempFileName);
}
void PrintingInfo::setGcodeTempFileName(QFileInfo gcodeTempFileName_)
{
	gcodeTempFileName = gcodeTempFileName_;
}
void PrintingInfo::setGcodeTempFileName(QString gcodeTempFileName_)
{
	gcodeTempFileName.setFile(gcodeTempFileName_);
}
QString PrintingInfo::getGcodeTempFileName()
{
	return gcodeTempFileName.absoluteFilePath();
}
QFileInfo PrintingInfo::getGcodeTempFileInfo()
{
	return gcodeTempFileName;
}
void PrintingInfo::setIsLoadedGcode(bool isLoadedGcode_)
{
	isLoadedGcode = isLoadedGcode_;
}
bool PrintingInfo::getIsLoadedGcode()
{
	return isLoadedGcode;
}
void PrintingInfo::setIsReplicationPrint(bool isReplicationPrint_)
{
	isReplicationPrint = isReplicationPrint_;
}
bool PrintingInfo::getIsReplicationPrint()
{
	return isReplicationPrint;
}
bool PrintingInfo::setEncodedImage(QString imagePath_)
{
	QFile imageFile(imagePath_);
	if (!imageFile.open(QIODevice::ReadOnly))
		return false;
	encodedImage = imageFile.readAll().toBase64();
	imageFile.close();
	return true;
}
void PrintingInfo::deleteEncodedImage()
{
	encodedImage = "";
}
void PrintingInfo::appendEncodedImage(QString encodedImage_)
{
	encodedImage.append(encodedImage_);
}
QString PrintingInfo::getEncodedImage()
{
	return encodedImage;
}
bool PrintingInfo::hasEncodedImage()
{
	return !encodedImage.isEmpty();
}
void PrintingInfo::setMachineModel(QString machineModel_)
{
	machineModel = machineModel_;
}
QString PrintingInfo::getMachineModel()
{
	return machineModel;
}
void PrintingInfo::setOperatingZone(qglviewer::Vec operatingZone_)
{
	operatingZone = operatingZone_;
}
void PrintingInfo::setOperatingZoneX(float x_)
{

}
void PrintingInfo::setOperatingZoneY(float y_)
{

}
void PrintingInfo::setOperatingZoneZ(float z_)
{

}
qglviewer::Vec PrintingInfo::getOperatingZone()
{
	return operatingZone;
}
void PrintingInfo::setTotalPrintTime(int totalPrintTime_)
{
	totalPrintTime = totalPrintTime_;
}
int PrintingInfo::getTotalPrintTime()
{
	return totalPrintTime;
}
QString PrintingInfo::getTotalPrintTimeString()
{
	QString hr = QString::number(int(totalPrintTime) / 3600).rightJustified(4, '0');
	QString min = QString::number(int(totalPrintTime) % 3600 / 60).rightJustified(2, '0');
	QString sec = QString::number(int(totalPrintTime) % 60).rightJustified(2, '0');
	QString rtn = QString("%1:%2:%3").arg(hr, min, sec);
	return rtn;
}
void PrintingInfo::setCartridgeCountWithInit(int cartridgeCount_)
{
	setCartridgeCount(cartridgeCount_);
	//initialize
	for (int i = 0; i < cartridgeCount; i++)
	{
		filaAmount[i] = 0;
		filaMass[i] = 0;
		material[i] = "NONE";
		usedState[i] = false;
	}
}
void PrintingInfo::setCartridgeCount(int cartridgeCount_)
{
	cartridgeCount = cartridgeCount_;

	filaAmount.resize(cartridgeCount);
	filaMass.resize(cartridgeCount);
	material.resize(cartridgeCount);
	usedState.resize(cartridgeCount);
}
int PrintingInfo::getCartridgeCount()
{
	return cartridgeCount;
}
void PrintingInfo::setUsedCartridgeCount(int usedCartridgeCount_)
{
	usedCartridgeCount = usedCartridgeCount_;
}
int PrintingInfo::getUsedCartridgeCount()
{
	//return usedCartridgeCount;
	int rtn = 0;
	for (auto amount : filaAmount)
		if (amount != 0)
			rtn++;
	return rtn;
}
void PrintingInfo::setFilaAmount(int index_, float value_)
{
	if (filaAmount.size() <= index_)
		setCartridgeCount(index_ + 1);
	filaAmount[index_] = value_;
	if (value_ != 0)
		setUsedState(index_, true);
}
float PrintingInfo::getFilaAmount(int index_)
{
	if (filaAmount.size() <= index_)
		return 0;
	return filaAmount[index_];
}
std::vector<float> PrintingInfo::getFilaAmount()
{
	return filaAmount;
}
float PrintingInfo::getFilaAmountTotal()
{
	float rtn = 0;
	for (auto it : filaAmount)
		rtn += it;
	return rtn;
}
void PrintingInfo::setFilaMass(int index_, float value_)
{
	if (filaMass.size() <= index_)
		return;
	filaMass[index_] = value_;
}
float PrintingInfo::getFilaMass(int index_)
{
	if (filaMass.size() <= index_)
		return 0;
	return filaMass[index_];
}
std::vector<float> PrintingInfo::getFilaMass()
{
	return filaMass;
}
float PrintingInfo::getfilaMassTotal()
{
	float rtn = 0;
	for (auto it : filaMass)
		rtn = rtn + it;
	return rtn;
}
void PrintingInfo::setMaterial(int index_, QString value_)
{
	if (material.size() <= index_)
		return;
	material[index_] = value_;
}
QString PrintingInfo::getFilaMaterial(int index_)
{
	if (material.size() <= index_)
		return "NONE";
	return material[index_];
}
std::vector<QString> PrintingInfo::getFilaMaterial()
{
	return material;
}
void PrintingInfo::setUsedState(int index_, bool value_)
{
	if (usedState.size() <= index_)
		return;
	usedState[index_] = value_;
}
bool PrintingInfo::getUsedState(int index_)
{
	if (usedState.size() <= index_)
		return false;
	return usedState[index_];
}
std::vector<bool> PrintingInfo::getUsedState()
{
	return usedState;
}

std::string PrintingInfo::getUseStateTFString()
{
	std::string resultStr;
	resultStr.clear();
	for (int i = 0; i < usedState.size(); i++)
	{
		if (i != 0)
			resultStr.append(":");
		resultStr.append((usedState[i] == true ? "T" : "F"));
	}
	return resultStr;
}

std::vector<int> PrintingInfo::getUsedCartridgeIndex()
{
	std::vector<int> rtn;
	for (int i = 0; i < usedState.size(); i++)
	{
		if (usedState[i])
			rtn.push_back(i);
	}
	return rtn;
}