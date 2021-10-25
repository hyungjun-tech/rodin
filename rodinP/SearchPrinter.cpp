#include "stdafx.h"
#include "SearchPrinter.h"
#include "ConnectionControl.h"
#include "Communication.h"

SearchNetworkPrinter::SearchNetworkPrinter(int myNum)
	: num(myNum)
{
}

void SearchNetworkPrinter::run()
{
	NetworkControl *a = new NetworkControl();

	QStringList ips = ip.toString().split(".");
	int CNum = ips[2].toDouble();

	QString printerName;
	QString model;

	for (int j = 0; j < 256 / 2; j++)
	{
		int mNum = CNum - j;
		int pNum = CNum + 1 + j;
		if (mNum < 0) mNum = mNum + 256;
		if (pNum > 256) pNum = pNum - 256;
		CNums.append(mNum);
		CNums.append(pNum);
	}

	for (int k = 0; k < CNums.size(); k++)
	{
		for (int i = 0; i < 256 / 8; i++)
		{
			QString targetIP;
			targetIP = ips[0] + "." + ips[1] + "." + QString::number(CNums.at(k)) + "." + QString::number(i * 8 + num);
			if (a->checkSindohMachine(targetIP))
			{
				printerName = a->getUserPrinterName(targetIP);
				model = a->getModel(targetIP);
				emit signal_findPrinter(printerName, targetIP, model);
			}

			if (this->Stop)
			{
				break;
			}
		}
		if (this->Stop)
		{
			break;
		}
	}
	delete a;
	emit signal_finishSearch();
}




SearchUSBPrinter::SearchUSBPrinter()
{
}

void SearchUSBPrinter::run()
{
	PJLControl pjl;
	std::vector<PrinterInterface> printerList = pjl.getAllSindohPrinter(false);
	//qDebug() << printerList.size();
	QString printerName;
	QString serialNo;
	QString model;
	if (printerList.size() > 0) {
		for (int j = 0; j < printerList.size(); j++)
		{
			//일단 찾아지는 Printer는 전부 대상이라고 고려. -> 추후에 조건 추가
			//TLI, Serial 찾아옴.
			//qDebug() << "1";
			//QString printerName = pjl->getPJLValue(printerList[j], "TLI");
			//QString SerailNo = pjl.getPJLValue(printerList[j],"SERIAL");
			//현재 Serial 에 대한 PJL설정이 안되어 있음. 일단 TLI로 처리

			QStringList prtData = pjl.getPrinterInfo(printerList[j]);
			if (prtData.at(1) != "")
				emit signal_findPrinter(prtData.at(1), prtData.at(0), prtData.at(2));
			/*printerName = pjl.getUserPrinterName(printerList[j]);
			if (printerName != NULL)
			{
			serialNo = pjl.getSerial(printerList[j]);
			model = pjl.getModel(printerList[j], m_modelMatchList);
			emit signal_findPrinter(printerName, serialNo, model);
			}*/
		}
	}
	emit signal_finishSearch();
}





