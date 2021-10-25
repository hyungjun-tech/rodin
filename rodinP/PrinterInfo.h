#pragma once

/*class PrinterData
{
public:
	PrinterData();
	QString connectionType;
	QString printerName;
	QString ip;
	QString model;
};

class PrinterInfo
{

public:
	PrinterInfo();
	~PrinterInfo();
	bool setFilamentData(PrinterData *prtData);
	
	PrinterData *prtData;
};*/



class PrinterInfo
{
public:
	static void clear();
	static void clear(QString currentModel_);
	static void setPrinterInfo(QString currentConnectionType_, QString currentIP_, QString currentPrinterName_, QString currentModel_);

	static QString currentConnectionType;
	static QString currentPrinterName;
	static QString currentIP;
	static QString currentModel;

	static bool checkMachine(QWidget* _widget);
	static bool checkPrinterStatus(QString _status, QString& _message);
};