#include "stdafx.h"
#include "PrintControl.h"
#include "ConnectionControl.h"

#include "ViewerModule.h"
#include "SliceProcessor.h"
#include "ModelDataStorage.h"
#include "GCodeGenerator.h"
#include "PrinterInfo.h"
#include "PrintingInfo.h"
#include "FileTransfer.h"

PrintControl::PrintControl(ViewerModule* viewerModule_, bool isSSDSave_)
	: modelContainer(viewerModule_->modelContainer)
	, printingInfo(viewerModule_->modelContainer->printingInfo)
	, fileTransfer(nullptr)
	, dataStorage(nullptr)
	, m_isSSDSave(isSSDSave_)
{
	CommonDialog *comDlg = new CommonDialog(viewerModule_, false);
	comDlg->setAttribute(Qt::WA_DeleteOnClose, true);
	connect(this, SIGNAL(signal_dialogContents(QString, int, bool, bool, bool, bool, bool)), comDlg, SLOT(setDialogContents(QString, int, bool, bool, bool, bool, bool)));
	connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
	connect(this, SIGNAL(signal_close_dialog()), comDlg, SLOT(close()));
	connect(this, SIGNAL(signal_progressValue(int)), comDlg, SLOT(setProgressValue(int)));
	connect(this, SIGNAL(signal_setLabelText(QString)), comDlg, SLOT(setMessage(QString)));
	//connect(this, SIGNAL(finished()), mEngine, SLOT(MakeCurrent()));
	//connect(this, SIGNAL(finished()), mEngine, SLOT(update()));
	connect(comDlg, SIGNAL(clickCancelSig()), this, SLOT(threadCancel()));
	connect(comDlg, SIGNAL(clickYesSig()), this, SLOT(dialog_yesClicked()));
	connect(comDlg, SIGNAL(clickNoSig()), this, SLOT(dialog_noClicked()));
	connect(this, SIGNAL(signal_setPreviewUIMode(ModelDataStorage*)), viewerModule_, SLOT(setPreviewUIMode(ModelDataStorage*)));

	m_pw = "";
	//m_isLayerViewer = false;
	m_cancelFlag = false;
	m_yesFlag = false;
	m_noFlag = false;
}


PrintControl::~PrintControl()
{
	if (dataStorage)
		delete dataStorage;
}

void PrintControl::init()
{
	m_cancelFlag = false;
	m_yesFlag = false;
	m_noFlag = false;
}

void PrintControl::run()
{
	Logger() << PrinterInfo::currentConnectionType << " PrintControl Start.";

	if (!slice())
		return;

	if (m_isSSDSave) emit signal_dialogContents(MessageInfo::tr("Save G-code Start."), CommonDialog::NoIcon, false, false, false, false, true);
	else emit signal_dialogContents(MessageInfo::tr("ready_to_print"), CommonDialog::NoIcon, false, false, false, false, true);

	if (!createConnection())
		return;

	if (m_isSSDSave)
	{
		if (!checkBeforeFileTransferToSSD())
			return;
		if (!fileTransfer->sendSaveToSSD())
			return;
	}
	else
	{
		if (!checkBeforeFileTransfer())
			return;
	}

	if (!fileTransfer->sendPassword(m_pw))
		return;
	QString gcodeFileName = printingInfo->getIsLoadedGcode() ? printingInfo->getGcodeFileName() : printingInfo->getGcodeTempFileName();
	if (!fileTransfer->transferFile(gcodeFileName))
	{
		if (fileTransfer != nullptr)
			delete fileTransfer;
		return;
	}

	if (fileTransfer != nullptr)
		delete fileTransfer;

	emit signal_dialogContents(MessageInfo::tr("print_file_sent"), CommonDialog::Information);
	Logger() << PrinterInfo::currentConnectionType << " PrintControl End.";
}
bool PrintControl::slice()
{
	if (m_cancelFlag)
	{
		emit signal_close_dialog();
		return false;
	}
	//레이어 뷰어 모드를 제외하고 다시 슬라이싱해야 함.
	if (!Generals::isPreviewUIMode())
	{
		if (m_isSSDSave) emit signal_dialogContents(MessageInfo::tr("Save G-code Start."), CommonDialog::Progress, false, true);
		else emit signal_dialogContents(MessageInfo::tr("ready_to_print"), CommonDialog::Progress, false, true);

		SliceProcessor processor;
		connect(&processor, SIGNAL(signal_setLabelText(QString)), this, SIGNAL(signal_setLabelText(QString)));
		connect(&processor, SIGNAL(signal_setValue(int)), this, SIGNAL(signal_progressValue(int)));
		connect(this, SIGNAL(signal_cancel()), &processor, SLOT(cancel_processor()), Qt::DirectConnection);
		processor.init(modelContainer);
		//p.setConfig(m_Config_multi);
		if (!processor.processing() || m_cancelFlag)
		{
			QString message = processor.getErrorMessage();
			if (message.isEmpty())
				emit signal_dialogContents(MessageError::tr("temporary_error_and_retry"), CommonDialog::Critical);
			else
				emit signal_dialogContents(message, CommonDialog::Critical);
			return false;
		}
		dataStorage = new ModelDataStorage();
		*dataStorage = *processor.getDataStorage();

		GCodeGenerator generator;
		connect(&generator, SIGNAL(signal_setLabelText(QString)), this, SIGNAL(signal_setLabelText(QString)));
		connect(&generator, SIGNAL(signal_setValue(int)), this, SIGNAL(signal_progressValue(int)));
		connect(this, SIGNAL(signal_cancel()), &generator, SLOT(cancel_processor()), Qt::DirectConnection);
		generator.init(modelContainer, dataStorage);
		//g.setConfig(m_Config_multi);
		if (!generator.processing() || m_cancelFlag)
		{
			emit signal_dialogContents(MessageError::tr("temporary_error_and_retry"), CommonDialog::Critical);
			return false;
		}

		emit signal_setPreviewUIMode(dataStorage);
	}
	return true;
}
bool PrintControl::createConnection()
{
	if (PrinterInfo::currentConnectionType == "USB")
		fileTransfer = new USBTransfer();
	else
		fileTransfer = new NetworkTransfer();

	connect(fileTransfer, SIGNAL(signal_dialogContents(QString, int, bool, bool, bool, bool, bool)), this, SIGNAL(signal_dialogContents(QString, int, bool, bool, bool, bool, bool)));
	connect(fileTransfer, SIGNAL(signal_progressValue(int)), this, SIGNAL(signal_progressValue(int)));
	connect(this, SIGNAL(signal_cancel()), fileTransfer, SLOT(cancel_transfer()), Qt::DirectConnection);

	if (fileTransfer->createConnection(PrinterInfo::currentIP))
		return true;
	else
		return false;
}
bool PrintControl::checkBeforeFileTransferToSSD()
{
	if (m_cancelFlag)
	{
		emit signal_close_dialog();
		return false;
	}
	if (PrinterInfo::currentIP == NULL || PrinterInfo::currentIP == "")			// 연결된 기기가 없을 경우에 false 반환
	{
		emit signal_close_dialog();
		return false;
	}

	//file size check..//
	QFileInfo gcodeFile = printingInfo->getGcodeFileInfo();
	double fileSize_MB = gcodeFile.size() / (double)(1024 * 1024);
	qDebug() << "print_gcode_file_size : " << fileSize_MB << "MB";

	QString machineId = PrinterInfo::currentIP;
	int ssdRemain;//SSD사이즈
	if (PrinterInfo::currentConnectionType == "Network")
	{
		NetworkControl con;
		ssdRemain = con.getSSDRemain(machineId);
	}
	else
	{
		PJLControl con;
		PrinterInterface selectedPrinter = con.getSelectPrinter(machineId, false);
		if (selectedPrinter.getRawPointer() == NULL)
			return false;
		ssdRemain = con.getSSDRemain(selectedPrinter);
	}

	if (m_isSSDSave)
	{
		if (ssdRemain <= fileSize_MB)
		{
			Logger() << "Transmission failed due to insufficient storage space on the device's SSD. / Remaining SSD capacity : " << ssdRemain;
			emit signal_dialogContents(MessageAlert::tr("insufficient_SSD_storage_space_with_remains")
				+ ssdRemain, CommonDialog::Warning);
			return false;
		}
	}
	return true;
}

bool PrintControl::checkBeforeFileTransfer()
{
	if (m_cancelFlag)
	{
		emit signal_close_dialog();
		return false;
	}
	if (PrinterInfo::currentIP == NULL || PrinterInfo::currentIP == "")			// 연결된 기기가 없을 경우에 false 반환
	{
		emit signal_close_dialog();
		return false;
	}

	//file size check..//
	QFileInfo gcodeFile = printingInfo->getGcodeFileInfo();
	double fileSize_MB = gcodeFile.size() / (double)(1024 * 1024);
	qDebug() << "print_gcode_file_size : " << fileSize_MB << "MB";
	if (!Profile::machineProfile.has_SSD_storage.value)
	{
		//gcode의 크기가 150MB이상일 경우, 경고 메세지 출력.. by swyang//
		if (fileSize_MB >= 150.0)
		{
			init();
			emit signal_dialogContents(MessageAlert::tr("PrintMsg_WARNING_gcodeLarge"), CommonDialog::Warning, false, false, true, true);//to do sj 메세지 변경 필요. 계속할건지 물어봐야 함.

			//끝날때까지 대기
			while (!(m_yesFlag || m_noFlag))
				msleep(10);
			if (m_noFlag)
			{
				emit signal_close_dialog();
				return false;
			}
		}
	}

	QString machineId = PrinterInfo::currentIP;
	QString status;//상태
	std::vector<int> filaRemains;//필라멘트 잔량
	int memorySize;//메모리 사이즈
	int ssdRemain;//SSD사이즈
	if (PrinterInfo::currentConnectionType == "Network")
	{
		NetworkControl con;
		status = con.getStatus(machineId);
		filaRemains = con.getFilaRemainsList(machineId);
		memorySize = con.getMemory(machineId);
	}
	else
	{
		PJLControl con;
		PrinterInterface selectedPrinter = con.getSelectPrinter(machineId, false);
		if (selectedPrinter.getRawPointer() == NULL)
			return false;
		status = con.getStatus(selectedPrinter);
		filaRemains = con.getFilaRemainsList(selectedPrinter);
		memorySize = con.getMemory(selectedPrinter);
	}

	QString message;
	if (!PrinterInfo::checkPrinterStatus(status, message))
	{
		emit signal_dialogContents(message, CommonDialog::Warning);
		return false;
	}

	bool checkLength = false;
	for (auto idx : printingInfo->getUsedCartridgeIndex())
	{
		if (filaRemains[idx] * 1000 < printingInfo->getFilaAmount(idx))
		{
			checkLength = true;
			break;
		}
	}
	if (checkLength)
	{
		init();
		emit signal_dialogContents(MessageQuestion::tr("not_enough_filament"), CommonDialog::Question, false, false, true, true);
		//끝날때까지 대기
		while (!(m_yesFlag || m_noFlag))
			msleep(10);
		if (m_noFlag)
		{
			emit signal_close_dialog();
			return false;
		}
	}

	Logger() << "memory size = " << memorySize << "MB";
	if (fileSize_MB > memorySize)
	{
		emit signal_dialogContents(MessageAlert::tr("not_enough_storage"), CommonDialog::Warning);
		return false;
	}

	return true;
}

void PrintControl::threadCancel()
{
	m_cancelFlag = true;
	emit signal_cancel();
}