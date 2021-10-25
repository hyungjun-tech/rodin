#include "stdafx.h"
#include "Generals.h"


Generals::ViewerMode Generals::currentViewerMode = Generals::ViewerMode::TranslateUIMode;
bool Generals::isLayerColorMode = false;
QString Generals::appPath = QString();
QString Generals::appDataPath = QString();
QString Generals::tempPath = QString();
QString Generals::temperatureUnit = QString();

wchar_t* Generals::qstringTowchar_t(QString qStr)
{
	//wchar_t�� ��ȯ�Ͽ� ����
	wchar_t* wStr = new wchar_t[qStr.length() + 1];
	qStr.toWCharArray(wStr);
	wStr[qStr.length()] = 0;

	return wStr;
}

void Generals::fileCopyFuction(QString fromPath, QString toPath)
{
	FILE* s = _wfopen(Generals::qstringTowchar_t(fromPath), L"r");
	FILE* d = _wfopen(Generals::qstringTowchar_t(toPath), L"w");

	if (s == NULL || d == NULL) return;

	int c, err;
	while ((c = fgetc(s)) != EOF)
	{
		err = fputc(c, d);
		if (err == EOF || ferror(s) || ferror(d))
		{
			printf("File copying error!!\n");
		}
	}
	fclose(d);
	fclose(s);
}

double Generals::roundDouble(double value, int position)
{
	double temp;

	temp = value * pow(10, position);	// ���ϴ� �Ҽ��� �ڸ�����ŭ 10�� ������ ��
	temp = floor(temp + 0.5);			// 0.5�� ������ �����ϸ� �ݿø��� ��
	temp *= pow(10, -position);			// �ٽ� ���� �Ҽ��� �ڸ�����

	return temp;
}

std::string Generals::to_string_with_precision(double _value, int _precision)
{
	std::ostringstream out_stream;
	out_stream.precision(_precision);
	out_stream << std::fixed << _value;

	return out_stream.str();
}

QString Generals::getMaterialShortName(QString pMaterial)
{
	QString rtn;
	if (pMaterial == "FLEXIBLE") rtn = "FLEX";
	else rtn = pMaterial;
	return rtn;
}

int Generals::getWeightForMaterial(QString pMaterial)
{
	int rtn = 0;

	if (pMaterial == "PLA")	rtn = 70;
	else if (pMaterial == "ABS") rtn = 40;
	else if (pMaterial == "WOOD") rtn = 50;
	else if (pMaterial == "PETG") rtn = 40;
	else if (pMaterial == "HIPS") rtn = 30;
	else if (pMaterial == "FLEXIBLE") rtn = 20;
	else if (pMaterial == "PVA") rtn = 80;
	else if (pMaterial == "PVA+") rtn = 100;
	else if (pMaterial == "ASA") rtn = 80;
	else if (pMaterial == "RZCB") rtn = 70;
	else if (pMaterial == "RZGF") rtn = 70;
	else if (pMaterial == "RZSU") rtn = 100;
	return rtn;
}

int Generals::checkMaterialCombination(QString pMaterial1, QString pMaterial2)
{
	//NA�� ��쿡�� ������ OK..//
	if (pMaterial1 == "NA" || pMaterial2 == "NA")
		return CheckPrintable::Ok;

	//ETC all available and ���� �ϳ��� ETC�� ��� OK..//
	//single nozzle�� ���..data������ ���߱� ���� is_ETC_all_available == TRUE�� ����..//
	//���ܻ��� : 2XC, 7XC//
	if (Profile::machineProfile.is_ETC_all_available.value && (pMaterial1 == "ETC" || pMaterial2 == "ETC"))
		return CheckPrintable::Ok;


	std::vector<QString> materials_check_list;
	materials_check_list.push_back(pMaterial1);
	materials_check_list.push_back(pMaterial2);

	//printable���� check..�켱����..//
	if (check_printable_combination(materials_check_list))
		return CheckPrintable::Ok;

	if (check_warning_combination(materials_check_list))
		return CheckPrintable::Warning;

	return CheckPrintable::NG;
}

bool Generals::check_printable_combination(const std::vector<QString> materials_check_list_)
{
	for (int i = 0; i < Profile::machineProfile.printable_material_combination_list.size(); ++i)
	{
		//�����ϰ� QStringList�� contains�Լ��� ������� ����.. PVA+�� PVA�� ���� ����.. ���� �ϳ��� ���ϴ� ���ۿ� ����..//
		//������ ����� ���� matching�� �Ǿ�� ��..//
		//if (none_printable_list.contains(pMaterial1, Qt::CaseSensitive) && none_printable_list.contains(pMaterial2, Qt::CaseSensitive))
		//	return 9;
		std::vector<QString> printable_list_element;
		for each  (auto str in Profile::machineProfile.printable_material_combination_list.at(i))
			printable_list_element.push_back(str);

		std::sort(printable_list_element.begin(), printable_list_element.end());
		if (printable_list_element == materials_check_list_)
			return true;

		std::sort(printable_list_element.rbegin(), printable_list_element.rend());
		if (printable_list_element == materials_check_list_)
			return true;
	}

	return false;
}

bool Generals::check_warning_combination(const std::vector<QString> materials_check_list_)
{
	for (int i = 0; i < Profile::machineProfile.warning_material_combination_list.size(); ++i)
	{
		std::vector<QString> warning_list_element;
		for each  (auto str in Profile::machineProfile.warning_material_combination_list.at(i))
			warning_list_element.push_back(str);

		std::sort(warning_list_element.begin(), warning_list_element.end());
		if (warning_list_element == materials_check_list_)
			return true;

		std::sort(warning_list_element.rbegin(), warning_list_element.rend());
		if (warning_list_element == materials_check_list_)
			return true;
	}

	//�ش� ���� ��..//
	return false;
}

bool Generals::check_material_in_list(const QStringList _available_materials, const QString _check_material)
{
	bool has_material = false;

	for each (auto material in _available_materials)
	{
		if (material == _check_material)
			has_material = true;
	}

	return has_material;
}


bool Generals::checkMaterialCombination_wipetower(QString pMaterial1, QString pMaterial2)
{
	if (pMaterial1 == "PLA" && pMaterial2 == "PVA") return true;
	else if (pMaterial1 == "PLA" && pMaterial2 == "PVA+") return true;
	else if (pMaterial1 == "PVA" && pMaterial2 == "PLA") return true;
	else if (pMaterial1 == "PVA+" && pMaterial2 == "PLA") return true;
	else return false;
}

double Generals::convertTemperatureUnitCtoF(double tempC)
{
	double tempF = 0.0;
	tempF = tempC * 9 / 5 + 32;
	return tempF;
}
double Generals::convertTemperatureUnitFtoC(double tempF)
{
	double tempC = 0.0;
	tempC = (tempF - 32) * 5 / 9;
	return tempC;
}

int Generals::convertTemperatureUnitCtoF(int tempC)
{
	int tempF = round((double)tempC * 9 / 5) + 32;
	return tempF;
}
int Generals::convertTemperatureUnitFtoC(int tempF)
{
	double tempC = round((double)(tempF - 32) * 5 / 9);
	return tempC;
}
const QString Generals::unknownProfileName = "Unknown Profile";

void Generals::checkPath(QString pPath)
{
	QDir path = pPath;
	if (!path.exists())
		path.mkpath(".");
}

QString Generals::getAppPath()
{
	QDir appPath = QCoreApplication::applicationDirPath();
	return appPath.absolutePath();
}

QString Generals::getAppDataPath()
{
	if (AppInfo::getAppCode() == AppInfo::AppCode::ADDIN_APP)
	{
		QString datapath = getFolderPath(QStandardPaths::GenericDataLocation) + "/" + AppInfo::getAppName();
		return datapath;
	}
	else
		return getFolderPath(QStandardPaths::DataLocation);
}

QString Generals::getTempFolderPath()
{
	QString tempPath = getFolderPath(QStandardPaths::TempLocation) + "/" + AppInfo::getAppName();
	checkPath(tempPath);

	return tempPath;
}

QString Generals::getFolderPath(QStandardPaths::StandardLocation pathType)
{
	QDir path = QStandardPaths::writableLocation(pathType);
	checkPath(path.absolutePath());

	return path.absolutePath();
}

int Generals::getFileSize(FILE* fp)
{
	int fpos;
	int fsize;

	fpos = ftell(fp); // ���� ���������Ͱ� ���ۺ��� �󸶳� �������ִ��� ����Ʈ ������ ���ؼ� fpos������ ���� 
	fseek(fp, 0, SEEK_END);  // ���������͸� ������ ����ġ�� �ű� 
	fsize = ftell(fp); // ���� ���������Ͱ� ���ۺ��� �󸶳� �������ִ��� ����Ʈ ������ ���ؼ� fsize������ ���� 
	fseek(fp, fpos, SEEK_SET); // ���������͸� ������ ������ġ���� fpos��ŭ ������ ��ġ�� �ű� 

	return fsize;
}

int Generals::getFileSize(const wchar_t* filePath)
{
	FILE* s = _wfopen(filePath, L"r");
	fclose(s);
	return getFileSize(s);
}

int Generals::getFileCurrentReadSize(FILE* fp)
{
	return ftell(fp);
}

void Generals::setProps(QString org, QString app, QString propName, QString propValue)
{
	QSettings props(org, app);
	props.setValue(propName, propValue);
	props.sync();
}

void Generals::setProps(QString propName, QString propValue)
{
	setProps(AppInfo::getCompanyName(), AppInfo::getAppName(), propName, propValue);
}

QString Generals::getProps(QString org, QString app, QString propName)
{
	QString rtn;
	QSettings props(org, app);
	rtn = props.value(propName).toString();
	return rtn;
}

QString Generals::getProps(QString propName)
{
	return getProps(AppInfo::getCompanyName(), AppInfo::getAppName(), propName);
}

QString Generals::removeSpecialChar(QString from_)
{
	QString rtn = from_;
	//QRegExp RegularEx("[^A-Za-z0-9`~!@$%\\^&()_+={},. \\[\\]\\-\\xAC00-\\xD7AF\\x3130-\\x318F\\x1100-\\x11FF]");
	QRegExp RegularEx("[,;#/\:*?\"\'<>|]");
	rtn.remove(RegularEx);
	return rtn;
}

void Generals::setGeometry(QWidget* w_, QPoint center_)
{
	QRect geo = w_->geometry();
	geo.moveCenter(center_);
	w_->setGeometry(geo);
}
void Generals::setGeometry(QWidget* w_)
{
	if (center.x() == 0 & center.y() == 0)
		center = w_->parentWidget()->geometry().center();
	setGeometry(w_, center);
}
void Generals::setCenter(int centerX_, int centerY_)
{
	center.setX(centerX_);
	center.setY(centerY_);
}

QPoint Generals::center = QPoint();

bool Generals::qstringToBool(QString _str)
{
	if (_str == "true")
		return true;
	else
		return false;
}

QString Generals::boolToQString(bool _flag)
{
	if (_flag)
		return "true";
	else
		return "false";
}