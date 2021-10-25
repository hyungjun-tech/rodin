#include "stdafx.h"
#include "PrintConfirm.h"
#include "PrinterInfo.h"
#include "ViewerModule.h"
#include "PrintingInfo.h"
#include "ConnectionControl.h"
#include "AuthentificationDlg.h"
#include "FileCopier.h"
#include "ProfileToConfig.h"
#include "SaveFileUIMode.h"
#include "PrintControl.h"

PrintConfirm::PrintConfirm(ViewerModule *viewerModule_)
	: QDialog(viewerModule_)
	, parent(viewerModule_)
	, viewerModule(viewerModule_)
	, printingInfo(viewerModule_->modelContainer->printingInfo)
	, isSecurityPrint(false)
{
	ui.setupUi(this);
	connect(ui.pushButton_yes, SIGNAL(clicked()), this, SLOT(pushButton_yes_clicked()));
	connect(ui.pushButton_no, SIGNAL(clicked()), this, SLOT(pushButton_no_clicked()));
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint) & (~Qt::WindowCloseButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);
}

PrintConfirm::~PrintConfirm()
{

}
bool PrintConfirm::setContents()
{
	if (!getSecurityPrint())
		return false;
	setAlertMessage();
	setUI();
	return true;
}

bool PrintConfirm::getSecurityPrint()
{
	QString securityPrint;
	//prtConfirm.setParent(this);
	if (PrinterInfo::currentConnectionType == "Network")
	{
		//선택된 Printer가 현재 사용 불가
		NetworkControl a;
		a.m_tmo = 2000;
		securityPrint = a.getSecurityPrint(PrinterInfo::currentIP);
	}
	else if (PrinterInfo::currentConnectionType == "USB")
	{
		PJLControl a;
		PrinterInterface selectedPrinter = a.getSelectPrinter(PrinterInfo::currentIP, false);//IP = Serial
		//선택된 Printer를 찾을 수 없음.
		if (selectedPrinter.getRawPointer() == NULL) {
			return false;
		}
		//선택된 Printer가 현재 사용 불가
		securityPrint = a.getSecurityPrint(selectedPrinter);
	}
	QString isSecurity = securityPrint.mid(0, 1);
	if (isSecurity == "1")
	{
		Logger() << "securityPrint on";
		isSecurityPrint = true;
		securityPW = securityPrint.mid(2, securityPrint.size()).trimmed();
	}
	return true;
}

void PrintConfirm::setAlertMessage()
{
	QStringList alertMsgs;
	alertMsgs.push_back(MessageAlert::tr("remove_object_before_printing"));
	//ABS 출력시 베드시트 B면 사용을 권장합니다.
	if (!printingInfo->getIsLoadedGcode()
		&& Profile::machineProfile.machine_bed_selected_enabled.value == 1
		&& ((Profile::sliceProfile.front().bed_type.value || Profile::sliceProfile.back().bed_type.value) != 0))
		alertMsgs.push_back(MessageAlert::tr("check_bed_sheet_match"));

	if (alertMsgs.size() < 0) return;
	QString alertMsg;
	ui.msgHeader->setText(MessageAlert::tr("please_check_info_below"));
	for (auto msg : alertMsgs)
	{
		alertMsg.append("\n" + QString::fromLocal8Bit("√ ") + msg + "\n");
	}
	ui.Message->setText(alertMsg);
}

void PrintConfirm::setUI()
{
	setWindowTitle(AppInfo::getAppName());
	ui.frame_securityPrint->setVisible(isSecurityPrint);
	if (PrinterInfo::currentConnectionType == "Network" && Profile::machineProfile.has_web_camera.value)
	{
		ui.webEngineView_preview->load(QUrl("http://" + PrinterInfo::currentIP + "/index_slicer.html"));
		ui.webEngineView_preview->setVisible(true);
		this->resize(QSize(528, size().height()));
		//setGeometry(geometry().x(), geometry().y(), 528, size().height());
	}
	else
	{
		ui.webEngineView_preview->setVisible(false);
		this->resize(QSize(450, size().height()));
		//setGeometry(geometry().x(), geometry().y(), 322, size().height());
	}
}

void PrintConfirm::pushButton_yes_clicked()
{
	if (isSecurityPrint)
	{
		QString pw = (QString)QCryptographicHash::hash("Y3BL67OKB8EPA9F" + ui.lineEdit_pw->text().toLocal8Bit(), QCryptographicHash::Md5).toHex();

		if (ui.lineEdit_pw->text() == "")
		{
			CommonDialog comDlg(this, MessageAlert::tr("please_enter_password"), CommonDialog::Warning);
			//비밀번호를 입력해 주세요.
			return;
		}
		else if (pw != securityPW)
		//else if (ui.lineEdit_pw->text() != m_securityPW)
		{
			CommonDialog comDlg(this, MessageAlert::tr("password_not_match"), CommonDialog::Warning);
			//비밀번호가 일치하지 않습니다. 다시 한번 확인해 주세요.
			ui.lineEdit_pw->setText("");

			return;
		}
	}
	hide();

	if (!Generals::isPreviewUIMode())
	{
		ProfileToConfig::convertToConfig();
		SaveFileUIMode::saveThumbnail(viewerModule->modelContainer->models).save(Generals::getTempFolderPath() + "\\thumbnail_image.png");
	}

	PrintControl * printControl = new PrintControl(viewerModule);
	printControl->m_pw = securityPW;
	printControl->start();
	close();

}
void PrintConfirm::pushButton_no_clicked()
{
	noFlag = true;
	close();
}