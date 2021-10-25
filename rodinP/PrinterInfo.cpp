#include "stdafx.h"
#include "PrinterInfo.h"
#include "ConnectionControl.h"
//
//PrinterData::PrinterData()
//{
//	connectionType = "";
//	printerName = "";
//	ip = "";
//	model = "";
//}
//
//PrinterInfo::PrinterInfo()
//{
//
//	prtData = new PrinterData();
//}
//
//
//PrinterInfo::~PrinterInfo()
//{
//	delete prtData;
//}
//
//bool PrinterInfo::setFilamentData(PrinterData *prtData)
//{
//	if (prtData->connectionType == "Network")
//	{
//		NetworkConnection network;
//		if (network.checkConnection(prtData->ip, 7000)){
//			NetworkControl a;
//			a.m_tmo = 2000;
//			/*temp.color = a.getFilaColor(prtData->ip); //필라멘트 색상
//			temp.length = a.getFilaLength(prtData->ip); //필라멘트 총 길이
//			temp.usedLength = a.getFilaUsed(prtData->ip);	//필라멘트 사용길이
//			temp.material = a.getFilaMaterial(prtData->ip); //필라멘트 종류
//			cartridges.push_back(temp);*/
//
//			QStringList l_material = a.getFilaMaterialList(prtData->ip);
//			QStringList l_color = a.getFilaColorList(prtData->ip);
//			std::vector<int> l_length = a.getFilaLengthList(prtData->ip);
//			std::vector<int> l_usedLength = a.getFilaUsedList(prtData->ip);
//
//			int size;
//			if (l_material.size() < CartridgeInfo::cartridges.size())
//				size = l_material.size();
//			else size = CartridgeInfo::cartridges.size();
//
//			for (int i = 0; i < size; i++)
//			{
//				CartridgeInfo::cartridges.at(i).color = l_color.at(i);
//				CartridgeInfo::cartridges.at(i).length = l_length.at(i);
//				CartridgeInfo::cartridges.at(i).usedLength = l_usedLength.at(i);
//				if (l_material.at(i) == "None" || l_material.at(i) == "")
//					CartridgeInfo::cartridges.at(i).material = "No Info.";
//				else
//					CartridgeInfo::cartridges.at(i).material = l_material.at(i);
//				//cartridges.push_back(temp);
//			}
//			return true;
//		}
//	}
//	else if (prtData->connectionType == "USB")
//	{
//		PJLControl pjl;
//		PrinterInterface selectedPrinter = pjl.getSelectPrinter(prtData->ip, false);
//
//		if (selectedPrinter.getRawPointer() != NULL) {
//			return pjl.setFilaInfoList(selectedPrinter);
//		}
//	}
//	return false;
//}


QString PrinterInfo::currentConnectionType = QString();
QString PrinterInfo::currentPrinterName = QString();
QString PrinterInfo::currentIP = QString();
QString PrinterInfo::currentModel = QString();
void PrinterInfo::clear()
{
	currentConnectionType = "";
	currentPrinterName = "";
	currentIP = "";
	currentModel = "";
}
void PrinterInfo::clear(QString currentModel_)
{
	currentConnectionType = "";
	currentPrinterName = "";
	currentIP = "";
	currentModel = currentModel_;
}
void PrinterInfo::setPrinterInfo(QString currentConnectionType_, QString currentIP_, QString currentPrinterName_, QString currentModel_)
{
	currentConnectionType = currentConnectionType_;
	currentIP = currentIP_;
	currentPrinterName = currentPrinterName_;
	currentModel = currentModel_;
}

bool PrinterInfo::checkMachine(QWidget* _widget)
{
	QString printStatus, securityPrint;
	//prtConfirm.setParent(this);
	if (PrinterInfo::currentConnectionType == "Network")
	{
		//내 컴퓨터 네트워크 연결 안됨
		if (!NetworkConnection::checkNetwork(_widget))
			return false;
		//해당 ip로 접속 안됨
		if (!NetworkConnection::checkConnection(_widget, PrinterInfo::currentIP, 7000))
			return false;
		//선택된 Printer가 현재 사용 불가
		NetworkControl a;
		a.m_tmo = 2000;
		printStatus = a.getStatus(PrinterInfo::currentIP);
	}
	else if (PrinterInfo::currentConnectionType == "USB")
	{
		PJLControl a;
		PrinterInterface selectedPrinter = a.getSelectPrinter(PrinterInfo::currentIP, false);//IP = Serial
		//선택된 Printer를 찾을 수 없음.
		if (selectedPrinter.getRawPointer() == NULL) {
			//선택하신 프린터를 찾을 수 없습니다.
			CommonDialog comDlg(_widget, MessageAlert::tr("cannot_connect_printer"), CommonDialog::Warning);
			//QMessageBox::warning(this, tr("3Dwox"), tr("Can't find the selected printer."));
			return false;
		}
		//선택된 Printer가 현재 사용 불가
		printStatus = a.getStatus(selectedPrinter);
	}

	QString message;
	if (!checkPrinterStatus(printStatus, message))
	{
		CommonDialog comDlg(_widget, message, CommonDialog::Warning);
		return false;
	}
	return true;
}

bool PrinterInfo::checkPrinterStatus(QString status, QString& _message)
{

	//No Cartridge					1 : other
	//Engine SC						2 : unknown
	//OK							3 : idle -> printing available
	//Printing						4 : printing
	//Bed clean is pending			5 : warmup
	//Bed leveling Required			6 : 
	//Nozzle Calibration Required	7 : 
	//Not Ready(UI is not ready)	8 : 
	//Front door open				9 :
	//

	if (status != "3" && status != "4")
	{
		//카트리지 미검지 상태.
		if (status == "1")
		{
			_message = MessageAlert::tr("status_cartridge_not_detected");
		}
		//베드 레벨링 필요
		else if (status == "6")
		{
			_message = MessageAlert::tr("status_indequate_bed_leveling");
		}
		//노즐 캘리브레이션 필요
		else if (status == "7")
		{
			_message = MessageAlert::tr("status_out_of_nozzle_calibration");
		}
		//UI가 준비중이 아님
		else if (status == "8")
		{
			_message = MessageAlert::tr("status_printer_is_not_ready");
		}
		//전면 도어 열림 경고
		else if (status == "9")
		{
			_message = MessageAlert::tr("status_front_door_open");
		}
		//선택된 프린터가 사용할 수 없는 상태입니다.
		else
		{
			_message = MessageAlert::tr("status_already_in_use");
		}
		return false;
	}
	else
		return true;
}