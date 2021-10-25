#pragma once

#include "Communication.h"
#include "SnmpControl.h"
#include "PrinterInfo.h"
using std::string;


class NetworkConnection
{
public:
	NetworkConnection();
	static QHostAddress getLocalIP();
	static bool checkNetwork(QWidget* target_);
	static bool checkNetwork();
	static bool checkConnection(QWidget* target_, QString ip, int tmo = 100);
	static bool checkConnection(QString ip, int tmo = 100);
};

class NetworkControl
{
	friend class SNMPSession;
	friend class snmpControl;
public:
	NetworkControl();
	~NetworkControl();
	QString getSnmp(QString ip, QString community, QString oids, int tmo = 1000);
	//QList<QString> getSnmpValue(QString ip, QString community, QList<QString> oids, SNMPSession * pSession = NULL);

	bool checkSindohMachine(QString ip);
	QString getVendor(QString ip);
	QString getCategory(QString ip);
	QString getPrinterName(QString ip);
	QString getUserPrinterName(QString ip);
	QString getModel(QString ip);
	QString getStatus(QString ip);
	int getFilaLength(QString ip);
	std::vector<int> getFilaLengthList(QString ip);
	int getFilaUsed(QString ip);
	std::vector<int> getFilaUsedList(QString ip);
	int getFilaRemains(QString ip);
	std::vector<int> getFilaRemainsList(QString ip);
	QString getFilaMaterial(QString ip);
	QStringList getFilaMaterialList(QString ip);
	QString getFilaColor(QString ip);
	QStringList getFilaColorList(QString ip);
	QStringList getZOffsetList(QString ip);
	int getMemory(QString ip);
	QString getSecurityPrint(QString ip);
	int getSSDRemain(QString ip);

	int m_tmo = 500;
private:
	snmpControl * a;
	QString maker = "SINDOH";
	QString category = "3D Machine";
};



class PJLControl
{
public:
	PJLControl();

	PrinterInterface getSelectPrinter(QString serial, bool writeFlag);
	std::vector<PrinterInterface> getAllSindohPrinter(bool writeFlag);
	static bool checkConnection(QWidget* target_, PrinterInterface selectedPrinter);
	static bool checkConnection(PrinterInterface selectedPrinter);
	QStringList getPJLValue(PrinterInterface printerInt, QStringList commendList);
	QString getPJLValue(PrinterInterface printerInt, QString inqCommend);
	QString ParsePJL(char *pbuf, int len, QString inqCommend);

	QString getPrinterName(PrinterInterface selectedPrinter);
	QString getUserPrinterName(PrinterInterface selectedPrinter);
	QString getModel(PrinterInterface selectedPrinter);
	QString getStatus(PrinterInterface selectedPrinter);
	QString getSerial(PrinterInterface selectedPrinter);
	int getFilaLength(PrinterInterface selectedPrinter);
	int getFilaUsed(PrinterInterface selectedPrinter);
	int getFilaRemains(PrinterInterface selectedPrinter);
	QString getFilaMaterial(PrinterInterface selectedPrinter);
	QStringList getFilaMaterialList(PrinterInterface selectedPrinter);
	QStringList getZOffsetList(PrinterInterface selectedPrinter);
	QString getFilaColor(PrinterInterface selectedPrinter);
	int getMemory(PrinterInterface selectedPrinter);
	QString getSecurityPrint(PrinterInterface selectedPrinter);
	std::vector<int> getFilaRemainsList(PrinterInterface selectedPrinter);
	bool setFilaInfoList(PrinterInterface selectedPrinter);
	QStringList getPrinterInfo(PrinterInterface selectedPrinter);
	int getSSDRemain(PrinterInterface selectedPrinter);
private:
	//const char* sindohPrinter = "vid_19f1&pid_9903";
	std::vector<char*> sindohPrinters;
	//const char* sindohPrinter = "vid_8564&pid_1000";
};