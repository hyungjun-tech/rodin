#include "stdafx.h"
#include "AddMyPrinter.h"
#include "ConnectionControl.h"
#include "MyPrinters.h"
#include "SearchPrinter.h"

AddMyPrinter::AddMyPrinter()
	: myPrinters(nullptr)
{
	ui.setupUi(this);
	connect(ui.openAddIPAddress, SIGNAL(labelClicked()), this, SLOT(showAddIPAddress()));
	//addComp("1234", "TEST");

	setConnection();
	setUI();
}

AddMyPrinter::AddMyPrinter(QWidget *parent)
	: QDialog(parent)
	, myPrinters(nullptr)
{
	ui.setupUi(this);

	setConnection();
	setUI();

	connect(this, SIGNAL(sigShowAddIP()), parent, SLOT(showFindIPPrinter()));
}

AddMyPrinter::~AddMyPrinter()
{
}
void AddMyPrinter::setUI()
{
	ui.verticalLayout_delay->addWidget(&pi);

	pi.setAnimationDelay(100);

	this->setModal(true);
	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	//commonDlg = new CommonDialog(parent);
	//commonDlg->setModal(true);
	//commonDlg->setParent(this);
	//ctr = new control();

	ui.machines->setColumnWidth(nameColumn, 140); //PrinterName
	ui.machines->setColumnWidth(modelColumn, 100); //Model
	ui.machines->setColumnWidth(ipColumn, 110); //IP
}

void AddMyPrinter::setConnection()
{
	connect(ui.openAddIPAddress, SIGNAL(labelClicked()), this, SLOT(showAddIPAddress()));
	//addComp("1234", "TEST");

	connect(ui.machines, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(click_action(QTreeWidgetItem*, int)));
	connect(ui.buttonAdd, SIGNAL(clicked()), this, SLOT(clickAdd()));
	connect(ui.buttonCancel, SIGNAL(clicked()), this, SLOT(clickCancel()));
}


void AddMyPrinter::findMyPrinters()
{
	//thread = new searchIPThread();
	//connect(thread, SIGNAL(gotMachine(QString, QString)), this, SLOT(addComp(QString, QString)));
	ui.machines->clear();
	//thread->Stop = false;
	//QStringList ips = localIP.toString().split(".");

	trds.clear();
	pi.startAnimation();
	this->show();

	if (m_connectionType == "Network")
	{
		ui.openAddIPAddress->setVisible(true);

		NetworkConnection network;
		if (!network.checkNetwork())
		{
			CommonDialog comDlg(this, MessageAlert::tr("no_network_connection"), CommonDialog::Warning);//네트워크 연결이 되지 않았습니다.\n네트워크 상태를 확인해 주세요.
			this->hide();
			//QMessageBox::warning(this, tr("3Dwox"), tr("No network connection.\nPlease check your network connection and try again."));//네트워크 연결이 되지 않았습니다.\n네트워크 상태를 확인해 주세요.
			return;
		}

		localIP = network.getLocalIP();
		for (int i = 0; i < 8; i++)
		{
			SearchNetworkPrinter * temp = new SearchNetworkPrinter(i);
			connect(temp, SIGNAL(signal_finishSearch()), this, SLOT(stopAni()));
			trds.append(temp);
			trds.at(i)->ip = localIP;
			//trds.at(i)->setNum(i);
			connect(trds.at(i), SIGNAL(signal_findPrinter(QString, QString, QString)), this, SLOT(addComp(QString, QString, QString)));
			//connect(trds.at(i), SIGNAL(finished()), trds.at(i), SLOT(deleteLater()));
			trds.at(i)->Stop = false;
			trds.at(i)->start();
			//trds.at(i)->stopProgress();
		}
	}
	else if (m_connectionType == "USB")
	{
		ui.openAddIPAddress->setVisible(false);

		QStringList headerLabels;
		headerLabels.push_back(tr("Printer Name"));
		headerLabels.push_back(tr("Model"));
		headerLabels.push_back(tr("Serial"));
		ui.machines->setHeaderLabels(headerLabels);
		SearchUSBPrinter * usb = new SearchUSBPrinter();
		connect(usb, SIGNAL(signal_finishSearch()), this, SLOT(stopAni()));
		connect(usb, SIGNAL(signal_findPrinter(QString, QString, QString)), this, SLOT(addComp(QString, QString, QString)));
		usb->start();
	}

}

void AddMyPrinter::showAddIPAddress()
{
	//thread->Stop = true;
	//thread->quit();
	threadStop();
	close();
	emit sigShowAddIP();
}

void AddMyPrinter::showMyPrinters()
{
	QString model = ui.machines->currentItem()->text(modelColumn);
	if (model == "No Model")
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
	}
	else
	{
		QString printerName = ui.machines->currentItem()->text(nameColumn);
		QString IP = ui.machines->currentItem()->text(ipColumn);
		if (myPrinters == nullptr) {
			myPrinters = new MyPrinters(parentWidget());
			myPrinters->showMyPrinters();
		}
		else
			myPrinters->show();
		myPrinters->addCompWithCheck(m_connectionType, printerName, IP, model, "N");
		close();
	}
}

void AddMyPrinter::showAddMyPrinter(QString connectionType)
{
	m_connectionType = connectionType;
	findMyPrinters();
}

void AddMyPrinter::addComp(QString pPrinterName, QString pIp, QString pModel)
{
	QTreeWidgetItem *machine = new QTreeWidgetItem();
	machine->setText(nameColumn, pPrinterName);
	machine->setText(ipColumn, pIp);
	machine->setText(modelColumn, pModel);
	ui.machines->addTopLevelItem(machine);
	ui.machines->repaint();
}

void AddMyPrinter::click_action(QTreeWidgetItem* tree, int a)
{
	//qDebug() << tree->text(0);
	//thread->Stop = true;
	//thread->quit();
	//threadStop();
	//showMyPrinters();
	clickAdd();
}


void AddMyPrinter::clickAdd()
{
	if (ui.machines->topLevelItemCount() == 1)
	{
		ui.machines->setCurrentItem(ui.machines->topLevelItem(0));
	}
	QTreeWidgetItem* a = ui.machines->currentItem();
	if (!a)
	{
		//QMessageBox::warning(this, tr("3Dwox"), tr("Selected printer does not exist."));
		//commonDlg->setDialogContents(tr("Selected printer does not exist."), CommonDialog::Warning);
		//emit sendDialogContents(tr("AddIPAddressMsgE"), 2);
		CommonDialog comDlg(this, MessageAlert::tr("please_select_printer"), CommonDialog::Warning);
	}
	else
	{
		//thread->Stop = true;
		//thread->quit();
		threadStop();
		showMyPrinters();
	}
}

void AddMyPrinter::clickCancel()
{
	//thread->Stop = true;
	//thread->quit();
	threadStop();
	close();
}

void AddMyPrinter::clickClose()
{
	//thread->Stop = true;
	//thread->quit();
	threadStop();
	close();
}

bool AddMyPrinter::threadIsRunning()
{
	for (int i = 0; i < trds.size(); i++)
	{
		if (trds.at(i)->isRunning()) return true;
	}
	return false;
}

void AddMyPrinter::threadStop()
{
	if (m_connectionType == "Network")
	{
		for (int i = 0; i < trds.size(); i++)
		{
			trds.at(i)->Stop = true;
			//trds.at(i)->terminate();
			//delete &trds.at(i);
			//if (trds.at(i)->isRunning()) trds.at(i)->terminate();
		}
	}
	stopAni();
}

void AddMyPrinter::stopAni()
{
	if (threadIsRunning()) NULL;
	else pi.stopAnimation();
}

void AddMyPrinter::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Escape)
		clickClose();
	//return;
}