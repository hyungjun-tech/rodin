#include "stdafx.h"
#include "ConnectionControl.h"
#include "CartridgeInfo.h"
#include "ModelMatchList.h"
#define debug_mode


NetworkConnection::NetworkConnection()
{
}

QHostAddress NetworkConnection::getLocalIP()
{
	// Get network interfaces list
	QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
	//QTcpSocket socket;
	QHostAddress ip;

	// Interfaces iteration
	for (int i = 0; i < ifaces.size(); i++)
	{
		// Now get all IP addresses for the current interface
		QList<QNetworkAddressEntry> addrs = ifaces[i].addressEntries();
		// And for any IP address, if it is IPv4 and the interface is active, send the packet
		for (int j = 0; j < addrs.size(); j++)
			if ((addrs[j].ip().protocol() == QAbstractSocket::IPv4Protocol) && (addrs[j].broadcast().toString() != ""))
			{
				ip = addrs[j].ip();
				break;
			}
	}

	return ip;
}
bool NetworkConnection::checkNetwork(QWidget* target_)
{
	if (!checkNetwork())
	{
		//네트워크 연결이 되지 않았습니다.\n네트워크 상태를 확인해 주세요.
		CommonDialog comDlg(target_, MessageAlert::tr("no_network_connection"), CommonDialog::Warning);
		return false;
	}
	return true;
}
bool NetworkConnection::checkNetwork()
{
	if (getLocalIP().toString() == NULL) return false;
	else return true;
}
bool NetworkConnection::checkConnection(QWidget* target_, QString ip, int tmo)
{
	//해당 ip로 접속 안됨
	if (!checkConnection(PrinterInfo::currentIP, tmo))
	{
		//해당 프린터로 접속이 되지 않습니다.
		QString message = MessageAlert::tr("cannot_connect_printer")
			+ QString("\n%1 (%2)").arg(PrinterInfo::currentPrinterName, PrinterInfo::currentIP);
		CommonDialog comDlg(target_, message, CommonDialog::Warning);
		return false;
	}
	return true;
}
bool NetworkConnection::checkConnection(QString ip, int tmo)
{
	QTcpSocket socket;
	socket.setProxy(QNetworkProxy::NoProxy);
	socket.connectToHost(ip, 9100);
	if (socket.waitForConnected(tmo))
	{
		socket.disconnect();
		socket.close();
		return true;
	}
	else
	{
		socket.close();
		return false;
	}
}



NetworkControl::NetworkControl()
	: a(nullptr)
{
}

NetworkControl::~NetworkControl()
{
	if (a != nullptr)
		delete a;
}


QString NetworkControl::getSnmp(QString ip, QString community, QString oid, int tmo)
{
	if (a == nullptr)
		a = new snmpControl();
	//qDebug() << "getSnmp";
	QString rtn = a->getSnmpValue(ip, oid, community, tmo);

#ifdef debug_mode
	qDebug() << rtn;
#endif

	return rtn;
}
bool NetworkControl::checkSindohMachine(QString ip)
{
	if (getVendor(ip) == maker)
	{
		if (getCategory(ip) == category) return true;
		else return false;
	}
	else return false;
}
QString NetworkControl::getVendor(QString ip)
{
	return getSnmp(ip, "public", "1.3.6.1.2.1.1.1.0", m_tmo);
}
QString NetworkControl::getCategory(QString ip)
{
	QString rtn = getSnmp(ip, "public", "1.3.6.1.2.1.25.3.2.1.3.1", m_tmo);
	//임시 코드
	if (rtn == "" || rtn == NULL || rtn == "NA")
		rtn = getSnmp(ip, "public", "1.3.6.1.4.1.27278.1.1.8.0", m_tmo);
	return rtn;
}
QString NetworkControl::getPrinterName(QString ip)
{
	return getSnmp(ip, "public", "1.3.6.1.2.1.43.5.1.1.16.1", m_tmo);
}
QString NetworkControl::getUserPrinterName(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.2.1.1.5.0", m_tmo);
	if (values == "DEFAULT_PRODUCT_STRING" || values == NULL || values == "NA")
	{
		values = getSnmp(ip, "public", "1.3.6.1.2.1.43.5.1.1.16.1", m_tmo);
	}
	return values;
}

QString NetworkControl::getModel(QString ip)
{
	QString tli = getSnmp(ip, "public", "1.3.6.1.2.1.43.5.1.1.16.1", m_tmo);
	return ModelMatchList::getModel(tli);
}

QString NetworkControl::getStatus(QString ip)
{
	/*
	No Cartridge			1 : other
	Engine SC				2 : unknown
	OK						3 : idle -> printing available
	Printing				4 : printing
	Bed clean is pending	5 : warmup*/
	QString rtn = getSnmp(ip, "public", "1.3.6.1.2.1.25.3.5.1.1.1", m_tmo);
	//if (rtn == "3") return "Y";
	//else return "N";
	if (rtn == "" || rtn == "NA")
	{
		rtn = getSnmp(ip, "public", "1.3.6.1.4.1.27278.1.1.9.0", m_tmo);
		if (rtn == "" || rtn == "NA")
			rtn = "2";
	}
	return rtn;
	/*if (rtn == "") rtn = "2";
	return rtn;*/
}
int NetworkControl::getFilaLength(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.3.2.0", m_tmo);
	//qDebug() << values;
	//default설정 - 숫자는 0으로 설정
	if (values == "" || values == "NA") values = "0";
	return values.toInt();
}
std::vector<int> NetworkControl::getFilaLengthList(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.3.2.0", m_tmo);
	QStringList rtn = values.split(":");
	std::vector<int> rtnList;
	for (int i = 0; i < rtn.size(); i++)
	{
		if (rtn.at(i) == "" || values == "NA") rtnList.push_back(0);
		else rtnList.push_back(rtn.at(i).toInt());
	}
	return rtnList;
}
int NetworkControl::getFilaUsed(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.3.3.0", m_tmo);
	//default설정 - 숫자는 0으로 설정
	if (values == "" || values == "NA") values = "0";
	return values.toInt();
}

std::vector<int> NetworkControl::getFilaUsedList(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.3.3.0", m_tmo);
	QStringList rtn = values.split(":");
	std::vector<int> rtnList;
	for (int i = 0; i < rtn.size(); i++)
	{
		if (rtn.at(i) == "" || rtn.at(i) == "NA") rtnList.push_back(0);
		else rtnList.push_back(rtn.at(i).toInt());
	}
	return rtnList;
}
int NetworkControl::getFilaRemains(QString ip)
{
	return getFilaLength(ip) - getFilaUsed(ip);
}
std::vector<int> NetworkControl::getFilaRemainsList(QString ip)
{
	std::vector<int> tempFilaLength, tempFilaUsed, rtn;
	tempFilaLength = getFilaLengthList(ip);
	tempFilaUsed = getFilaUsedList(ip);
	for (int i = 0; i < tempFilaLength.size(); i++)
	{
		rtn.push_back(tempFilaLength.at(i) - tempFilaUsed.at(i));
	}
	return rtn;
	//return getFilaLength(ip) - getFilaUsed(ip);
}

QString NetworkControl::getFilaMaterial(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.3.4.0", m_tmo);
	if (values == "NA" || values == "") return "None";
	return values;
}

QStringList NetworkControl::getFilaMaterialList(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.3.4.0", m_tmo);
	QStringList rtn = values.split(":");
	for (int i = 0; i < rtn.size(); i++)
	{
		if (rtn.at(i) == "NA") rtn.replace(i, "");
	}
	return rtn;
}
QString NetworkControl::getFilaColor(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.3.1.0", m_tmo);

	//default설정
	//if (values == "") values = "208,208,208";
	//default color변경
	if (values == "" || values == "NA") values = "230,240,50";
	else if (values == "0,0,0") values = "30,30,30";
	//qDebug() << values;
	return values;
}
QStringList NetworkControl::getFilaColorList(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.3.1.0", m_tmo);

	QStringList rtn = values.split(":");
	Profile::machineProfile.openMode = false;
	for (int i = 0; i < rtn.size(); i++)
	{
		if (rtn.at(i) == "NA" || rtn.at(i) == "" || rtn.at(i) == "999,999,999")
		{
			if (rtn.at(i) == "999,999,999")
				Profile::machineProfile.openMode = true;

			if (i == 0)
				rtn.replace(i, "230,240,50");
			else
				rtn.replace(i, "255,148,53");
		}
		else if (rtn.at(i) == "0,0,0") rtn.replace(i, "30,30,30");
	}
	return rtn;
}
QStringList NetworkControl::getZOffsetList(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.4.2.0", m_tmo);//미정
	QStringList rtn = values.split(":");
	for (int i = 0; i < rtn.size(); i++)
	{
		if (rtn.at(i) == "NA" || rtn.at(i) == "") rtn.replace(i, "0");
	}
	return rtn;
}
int NetworkControl::getMemory(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.1.1.7.0", m_tmo);
	if (values == "" || values == "NA") values = "99999999";
	return values.toInt();
}
QString NetworkControl::getSecurityPrint(QString ip)
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.10.1.0", m_tmo);
	if (values == "" || values == "NA") values = "0,0000";
	return values;
}
int NetworkControl::getSSDRemain(QString ip) //H 기종 Only 
{
	QString values = getSnmp(ip, "public", "1.3.6.1.4.1.27278.1.1.10.0", m_tmo);
	if (values == "" || values == "NA") values = "0";
	return values.toInt();
}




PJLControl::PJLControl()
{
	sindohPrinters.push_back("vid_19f1&pid_9903");
	sindohPrinters.push_back("vid_19f1&pid_9952");
}

PrinterInterface PJLControl::getSelectPrinter(QString serial, bool writeFlag)
{
	std::vector<PrinterInterface> printerList = getAllSindohPrinter(writeFlag);
	QString SerialNo;
	//qDebug() << printerList.size();

	if (printerList.size() > 0){
		for (int j = 0; j < printerList.size(); j++)
		{
			SerialNo = getSerial(printerList[j]);
			if (SerialNo == serial)
			{
				return printerList[j];
				break;
			}
		}
	}
	return NULL;
}

std::vector<PrinterInterface> PJLControl::getAllSindohPrinter(bool writeFlag)
{
	std::vector<PrinterInterface> printerList;
	for (auto it = sindohPrinters.begin(); it != sindohPrinters.end(); ++it)
	{
		std::vector<PrinterInterface> temp = getAllCommunicationInterface(*it, writeFlag);
		printerList.insert(printerList.end(), temp.begin(), temp.end());
	}
	return printerList;
}
bool PJLControl::checkConnection(QWidget* target_, PrinterInterface selectedPrinter)
{
	if (!checkConnection(selectedPrinter))
	{
		//선택하신 프린터를 찾을 수 없습니다.
		CommonDialog comDlg(target_, MessageAlert::tr("cannot_connect_printer"), CommonDialog::Warning);
		return false;
	}
	return true;
}
bool PJLControl::checkConnection(PrinterInterface selectedPrinter)
{
	if (selectedPrinter.getRawPointer() == NULL)
		return false;
	return true;
}

QStringList PJLControl::getPJLValue(PrinterInterface printerInt, QStringList commendList)
{
	QStringList rtn;
	for (int i = 0; i < commendList.size(); i++)
	{
		string commend = "\x1b%-12345X@PJL\n @PJL INQUIRE " + commendList.at(i).toStdString() + "\n \x1b%-12345X";
		char * pjlCommend = strdup(commend.c_str());
		const int maxLen = 256;
		bool r = false;// = printerInt->WriteDataToPrinterOL(pjlCommend, (DWORD)strlen(pjlCommend));

		int retry = 0;
		while (!r)
		{
			r = printerInt->writeDataToPrinterOL(pjlCommend, (DWORD)strlen(pjlCommend));
			QThread::usleep(10);
			retry++;
			if (retry == 100) return rtn;
		}
		//if (!r) return rtn;
	}


	const int maxLen = 256;
	char* buff[maxLen];
	//int retry = 0;

	for (int j = 0; j < commendList.size(); j++)
	{
		int retry = 0;
		while (true)
		{
			int len = 0;
			len = printerInt->readDataFromPrinter(buff, maxLen);
			if (len > 0)
			{
				//qDebug() << "buff : " << *buff << " : " << QTime::currentTime();
				QString vRtn = ParsePJL((char*)*buff, len, commendList.at(j));
				/*if (vRtn == "") continue;
				else
				{*/
				rtn.append(vRtn);
				break;
				//}
			}
			if (retry == 1000) break;
			retry++;
			Sleep(1);
		}
	}
	return rtn;
}


QString PJLControl::getPJLValue(PrinterInterface printerInt, QString inqCommend)
{
	QString vRtn = "";

	string commend = "\x1b%-12345X@PJL\n @PJL INQUIRE " + inqCommend.toStdString() + "\n \x1b%-12345X";
	//pjlCommend = a;

	//strcpy(pjlCommend, a);

	char * pjlCommend = strdup(commend.c_str());

	const int maxLen = 256;
	//qDebug() << "2";

	char* buff[maxLen];
	int len = 0;

	//Send
	//기계가 시작중일때 대기
	//if (printerInt->WriteDataToPrinter(pjlCommend, (DWORD)strlen(pjlCommend)))
	if (printerInt->writeDataToPrinterOL(pjlCommend, (DWORD)strlen(pjlCommend)))
	{
		//qDebug() << "pjlCommend : " << *pjlCommend << " : " << QTime::currentTime();
		//qDebug() << "3";
		int retry = 0;
		while (true)
		{
			len = 0;
			Sleep(1);
			len = printerInt->readDataFromPrinter(buff, maxLen);
			if (len > 0)
			{
				//qDebug() << "buff : " << *buff << " : " << QTime::currentTime();
				vRtn = ParsePJL((char*)*buff, len, inqCommend);
				//qDebug() << vRtn << " : " << QTime::currentTime();
				if (vRtn != "") break;
			}
			//else break;
			retry++;
			if (retry == 50) break;

		}
	}
#ifdef debug_mode
	qDebug() << vRtn;
#endif
	return vRtn;
}

QString PJLControl::ParsePJL(char *pbuf, int len, QString inqCommend)
{


	string pjlCommend = "@PJL INQUIRE " + inqCommend.toStdString();

	char *pjlCom = strdup(pjlCommend.c_str());



	//for (int i = 0; i < inqCommend.length(); i++)
	const int compareLen = pjlCommend.length();// sizeof(pjlCom);
	//char temp[compareLen];
	QString vReturn = "";

	for (int i = 0; i < len - 1; i++)
	{
		bool check = false;
		for (int j = 0; j < compareLen - 1; j++)
		{
			if (pbuf[i + j] == pjlCom[j])
			{
				check = true;
				break;
			}
		}
		if (check)
		{
			bool findFeed = false;
			for (int k = compareLen; k < len - 1; k++)
			{
				if (findFeed & pbuf[k] == '\r') findFeed = false;
				if (findFeed) vReturn.append((QString)pbuf[k]);
				if (!findFeed & pbuf[k] == '\n') findFeed = true;
			}
		}
		else return "";
		//qDebug() << vReturn;
		if (vReturn == "/r/n") vReturn = "";
		return vReturn;
	}
	return vReturn;
}

QString PJLControl::getPrinterName(PrinterInterface selectedPrinter)
{
	return getPJLValue(selectedPrinter, "TLI");
}
QString PJLControl::getUserPrinterName(PrinterInterface selectedPrinter)
{
	QString userPrinterName = getPJLValue(selectedPrinter, "MECHNAME");
	//qDebug() << "userPrinterName : " << userPrinterName;
	if (userPrinterName == "" || userPrinterName == NULL || userPrinterName == "NA")
	{
		userPrinterName = getPJLValue(selectedPrinter, "TLI");
	}
	//qDebug() << "printerName : " << userPrinterName;
	return userPrinterName;
}
QString PJLControl::getModel(PrinterInterface selectedPrinter)
{
	QString tli = getPJLValue(selectedPrinter, "TLI");
	return ModelMatchList::getModel(tli);
}
QString PJLControl::getStatus(PrinterInterface selectedPrinter)
{
	/*
	No Cartridge			1 : other
	Engine SC				2 : unknown
	OK						3 : idle -> printing available
	Printing				4 : printing
	Bed clean is pending	5 : warmup*/

	//return "Y";
	//return getPJLValue(selectedPrinter, "PRINTONLINE");
	QString rtn = getPJLValue(selectedPrinter, "PRINTONLINE");
	//for previous version
	if (rtn == "Y") rtn = "3";
	else if (rtn == "N") rtn = "2";
	else if (rtn == "" || rtn == "NA") rtn = "2";
	return rtn;
}
QString PJLControl::getSerial(PrinterInterface selectedPrinter)
{
	return getPJLValue(selectedPrinter, "SERIAL");
}
int PJLControl::getFilaLength(PrinterInterface selectedPrinter)
{
	QString rtn = getPJLValue(selectedPrinter, "FILALENGTH");
	//default설정 - 숫자는 0으로 설정
	if (rtn == "" || rtn == "NA") rtn = "0";
	return rtn.toInt();
}
int PJLControl::getFilaUsed(PrinterInterface selectedPrinter)
{
	QString rtn = getPJLValue(selectedPrinter, "FILAUSED");
	//default설정 - 숫자는 0으로 설정
	if (rtn == "" || rtn == "NA") rtn = "0";
	return rtn.toInt();
}
int PJLControl::getFilaRemains(PrinterInterface selectedPrinter)
{
	return getFilaLength(selectedPrinter) - getFilaUsed(selectedPrinter);
}
QString PJLControl::getFilaMaterial(PrinterInterface selectedPrinter)
{
	QString rtn = getPJLValue(selectedPrinter, "FILAMATERIAL");
	//default설정
	if (rtn == "" || rtn == "NA") return "None";
	return rtn;
	//return getPJLValue(selectedPrinter, "FILAMATERIAL");
}
QStringList PJLControl::getFilaMaterialList(PrinterInterface selectedPrinter)
{
	QString values = getPJLValue(selectedPrinter, "FILAMATERIAL");
	QStringList rtn = values.split(":");
	for (int i = 0; i < rtn.size(); i++)
	{
		if (rtn.at(i) == "NA" || rtn.at(i) == "") rtn.replace(i, "None");
	}
	return rtn;
}

QStringList PJLControl::getZOffsetList(PrinterInterface selectedPrinter)
{
	QString values = getPJLValue(selectedPrinter, "NZOFFSET");
	QStringList rtn = values.split(":");
	for (int i = 0; i < rtn.size(); i++)
	{
		if (rtn.at(i) == "NA" || rtn.at(i) == "") rtn.replace(i, "0");
	}
	return rtn;
}

QString PJLControl::getFilaColor(PrinterInterface selectedPrinter)
{
	QString rtn = getPJLValue(selectedPrinter, "FILACOLOR");
	//default설정
	//if (rtn == "") return "208,208,208";
	//default color변경
	if (rtn == "" || rtn == "NA") return "230,240,50";
	else if (rtn == "0,0,0") rtn = "30,30,30";
	return rtn;
}
int PJLControl::getMemory(PrinterInterface selectedPrinter)
{
	QString rtn = getPJLValue(selectedPrinter, "MEMSTATUS");
	if (rtn == "" || rtn == "NA") rtn = "99999999";
	return rtn.toInt();
}
QString PJLControl::getSecurityPrint(PrinterInterface selectedPrinter)
{
	QString rtn = getPJLValue(selectedPrinter, "SECURITYPRINT");
	if (rtn == "" || rtn == "NA") rtn = "0,0000";
	return rtn;
}

std::vector<int> PJLControl::getFilaRemainsList(PrinterInterface selectedPrinter)
{
	QStringList commendList = { "FILALENGTH", "FILAUSED" };
	QStringList rtn = getPJLValue(selectedPrinter, commendList);

	QStringList tempList;
	std::vector<int> tempFilaLength, tempFilaUsed, rtnList;
	for (int i = 0; i < rtn.size(); i++)
	{
		tempList.clear();
		if (i == 0)
		{
			tempList = rtn.at(i).split(":");
			for (int j = 0; j < tempList.size(); j++)
			{
				if (tempList.at(j) == "" || tempList.at(j) == "NA")
					tempFilaLength.push_back(0);
				else
					tempFilaLength.push_back(tempList.at(j).toInt());
			}
		}
		else if (i == 1)
		{
			tempList = rtn.at(i).split(":");
			for (int j = 0; j < tempList.size(); j++)
			{
				if (tempList.at(j) == "" || tempList.at(j) == "NA")
					tempFilaUsed.push_back(0);
				else
					tempFilaUsed.push_back(tempList.at(j).toInt());

				rtnList.push_back(tempFilaLength.at(j) - tempFilaUsed.at(j));
			}
		}
	}
	return rtnList;
}

bool PJLControl::setFilaInfoList(PrinterInterface selectedPrinter)
{
	QStringList commendList = { "FILAMATERIAL", "FILACOLOR", "FILAUSED", "FILALENGTH" };
	QStringList value = getPJLValue(selectedPrinter, commendList);
	//QList<FilamentInfo> rtn;
	//FilamentInfo temp;

	QStringList l_material;
	QStringList l_color;
	QStringList l_usedLength;
	QStringList l_length;
	if (commendList.size() == value.size())
	{
		for (int i = 0; i < value.size(); i++)
		{
			if (i == 0)
			{
				l_material = value.at(0).split(":");
				for (int j = 0; j < l_material.size(); j++)
				{
					if (l_material.at(j) == "NA" || l_material.at(j) == "") l_material.replace(j, "No Info.");
				}
			}
			else if (i == 1)
			{
				l_color = value.at(1).split(":");
				Profile::machineProfile.openMode = false;
				for (int j = 0; j < l_color.size(); j++)
				{
					if (l_color.at(j) == "" || l_color.at(j) == "NA" || l_color.at(j) == "999,999,999")
					{
						if (l_color.at(j) == "999,999,999")
							Profile::machineProfile.openMode = true;

						if (j == 0)
							l_color.replace(j, "230,240,50");
						else
							l_color.replace(j, "255,148,53");
					}
					else if (l_color.at(j) == "0,0,0") l_color.replace(j, "30,30,30");
				}
			}
			else if (i == 2)
			{
				l_usedLength = value.at(2).split(":");
				for (int j = 0; j < l_usedLength.size(); j++)
				{
					if (l_usedLength.at(j) == "" || l_usedLength.at(j) == "NA")
						l_usedLength.replace(j, "0");
				}
			}
			else if (i == 3)
			{
				l_length = value.at(3).split(":");
				for (int j = 0; j < l_length.size(); j++)
				{
					if (l_length.at(j) == "" || l_length.at(j) == "NA")
						l_length.replace(j, "0");
				}
			}


		}
		if (l_material.size() > CartridgeInfo::cartridges.size())
			return false;

		for (int i = 0; i < l_material.size(); i++)
		{
			CartridgeInfo::cartridges.at(i).color = l_color.at(i);
			CartridgeInfo::cartridges.at(i).length = l_length.at(i).toInt();
			CartridgeInfo::cartridges.at(i).usedLength = l_usedLength.at(i).toInt();
			CartridgeInfo::cartridges.at(i).material = l_material.at(i);
		}
		return true;
	}
	return false;
}

QStringList PJLControl::getPrinterInfo(PrinterInterface selectedPrinter)
{
	QStringList commendList = { "SERIAL", "MECHNAME", "TLI" };
	QStringList rtn = getPJLValue(selectedPrinter, commendList);
	if (rtn.size() < 3)
		return (QStringList() << "" << "" << ""); //정상적인 값이 나오지 않은것으로 판단 -> 전부 null로 리턴
	
	rtn[2] = ModelMatchList::getModel(rtn.at(2));
	if (rtn.at(1) == "" || rtn.at(1) == NULL || rtn.at(1) == "NA")
		rtn[1] = rtn.at(2);
	return rtn;
}

int PJLControl::getSSDRemain(PrinterInterface selectedPrinter) //H 기종 Only 
{
	QString rtn = getPJLValue(selectedPrinter, "SSDMEMSTATUS");
	if (rtn == "" || rtn == NULL || rtn == "NA") rtn = "0";
	return rtn.toInt();
}