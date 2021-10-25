#include "stdafx.h"
#include "AddIPAddress.h"
#include "ConnectionControl.h"
#include "MyPrinters.h"

AddIPAddress::AddIPAddress(QWidget *parent)
	: QDialog(parent)
	, myPrinters(nullptr)
{
	ui.setupUi(this);
	connect(ui.buttonAdd, SIGNAL(clicked()), this, SLOT(clickAdd()));
	//ui.ip0->setInputMask("999");
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);
	//setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
}

AddIPAddress::~AddIPAddress()
{
}

void AddIPAddress::initForm()
{
	//control ctr;
	//if (!ctr.checkNetwork())
	//{
	//	//commonDlg->setDialogContents(tr("AddIPAddressMsgE"), CommonDialog::Warning);//네트워크 연결이 되지 않았습니다.\n네트워크 상태를 확인해 주세요.
	//	CommonDialog comDlg(this, tr("AddIPAddressMsgE"), CommonDialog::Warning);
	//	this->hide();
	//	//QMessageBox::warning(this, tr("3Dwox"), tr("No network connection.\nPlease check your network connection and try again."));//네트워크 연결이 되지 않았습니다.\n네트워크 상태를 확인해 주세요.
	//	return;
	//}
	ui.ip0->setText("");
	ui.ip1->setText("");
	ui.ip2->setText("");
	ui.ip3->setText("");
	ui.ip0->setFocus();
	this->show();
}

void AddIPAddress::showMyPrinters()
{
	if (myPrinters == nullptr) {
		myPrinters = new MyPrinters(parentWidget());
		myPrinters->showMyPrinters();
	}
	else
		myPrinters->show();
	myPrinters->addCompWithCheck("Network", printerName, ipAddress, model, "N");
	close();
}
bool AddIPAddress::checkIPAddress()
{
	if (!NetworkConnection::checkNetwork(this))
	{
		this->hide();
		return false;
	}
	QList<QLineEdit *> ips;
	//ips.clear();
	ips.append(ui.ip0);
	ips.append(ui.ip1);
	ips.append(ui.ip2);
	ips.append(ui.ip3);

	for (int i = 0; i < ips.size(); i++)
	{
		QString ip = ips.at(i)->text();
		if (ip == "")
		{
			//QMessageBox::warning(this, tr("3Dwox"), tr("Enter your IP address."));
			//commonDlg->setDialogContents(tr("Enter your IP address."), CommonDialog::Warning);
			//emit sendDialogContents(tr("enter_user_ip_address"), 2);
			CommonDialog comDlg(this, MessageAlert::tr("enter_user_ip_address"), CommonDialog::Warning);
			return false;
		}
		if (0 <= ip.toInt() & ip.toInt() <= 255) NULL;
		else
		{
			//QMessageBox::warning(this, tr("3Dwox"), tr("Invalid IP address."));
			//commonDlg->setDialogContents(tr("Invalid IP address."), CommonDialog::Warning);
			//emit sendDialogContents(tr("invalid_ip_address"), 2);
			CommonDialog comDlg(this, MessageAlert::tr("invalid_ip_address"), CommonDialog::Warning);
			return false;
		}
	}
	//snmp check
	//014->14와 같이 변경하기 위해 숫자로 변환한 후 문자로 재변환
	QString targetIP = QString::number(ui.ip0->text().toInt()) + "." + QString::number(ui.ip1->text().toInt()) + "." + QString::number(ui.ip2->text().toInt()) + "." + QString::number(ui.ip3->text().toInt());
	//qDebug() << targetIP;
	if (!NetworkConnection::checkConnection(targetIP, 1000))
	{
		CommonDialog comDlg(this, MessageAlert::tr("cannot_connect_ip_address"), CommonDialog::Warning);
		return false;
	}
	NetworkControl ctr;
	//control *ctr = new control();
	ctr.m_tmo = 3000;
	if (ctr.checkSindohMachine(targetIP))
	{
		ipAddress = targetIP;
		model = ctr.getModel(targetIP);
		if (model != "No Model")
		{
			printerName = ctr.getUserPrinterName(ipAddress);
		}
		else
		{
			CommonDialog comDlg(this);
			//comDlg.setFixedSize(400, 180);
			QString message = MessageAlert::tr("not_supported_printer").arg(AppInfo::getAppName());
			comDlg.setDialogContents(message, CommonDialog::Question, false, false, true, true);
			comDlg.exec();

			if (comDlg.isYes())
			{
				QString link = AppInfo::getDownloadURL();
				QDesktopServices::openUrl(QUrl(link));
			}

			return false;
		}
	}
	else
	{
		//QMessageBox::warning(this, tr("3Dwox"), tr("Unsupported Printer system."));//지원가능한 프린터가 아닙니다.
		//commonDlg->setDialogContents(tr("Unsupported Printer system."), CommonDialog::Warning);
		//emit sendDialogContents(tr("unsupported_printer"), 2);
		CommonDialog comDlg(this, MessageAlert::tr("unsupported_printer"), CommonDialog::Warning);
		return false;
	}
	//delete ctr;
	/*QString _vendor;
	QString _category;
	//QString printerName;
	//QString userPrinterName;
	QString maker = "SINDOH";
	QString category = "3D Machine";

	_vendor = a.getVendor(targetIP);
	//QList<QString> values = a.getSnmpValue(targetIP, "public", oids, pSession);
	if (_vendor != "")
	{
		_category = a.getCategory(targetIP);
		if (_vendor == maker & _category == category) {
			printerName = a.getUserPrinterName(targetIP);
			ipAddress = targetIP;
			//printerName = userPrinterName;
			return true;
		}
		else
		{
			QMessageBox::information(this, tr("Alert"), tr("Connection to IP failed."));
			return false;
		}
	}
	else
	{
		QMessageBox::information(this, tr("Alert"), tr("Connection to IP failed."));
		return false;
	}*/


	/*QList<QString> oids;
	oids.append("1.3.6.1.2.1.1.1.0");
	oids.append("1.3.6.1.2.1.25.3.2.1.3.1");

	SNMPSession* pSession = new SNMPSession();
	QList<QString> values = a->getSnmpValue(fullIP, "public", oids, pSession);
	if (pSession != NULL)
	{
		SnmpClose(pSession);
	}

	if (values.size() <= 1)
	{
		QMessageBox::information(this, tr("alert"), tr("Connection to IP failed."));
		return false;
	}
	else
	{
		ipAddress = fullIP;
		printerName = values.at(0);
		return true;
	}*/

	return true;
}

void AddIPAddress::clickAdd()
{
	if (checkIPAddress()) showMyPrinters();
	else ui.ip0->setFocus();
}