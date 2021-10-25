#include "stdafx.h"
#include "MyPrinters.h"
#include "PrinterInfo.h"

MyPrinters::MyPrinters()
{
}

MyPrinters::MyPrinters(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setUI();
	setConnection();
	connect(this, SIGNAL(signal_printerSelected()), parent, SLOT(afterCurrentPrinterChanged()));
}

MyPrinters::~MyPrinters()
{
	//delete addIP;
	//delete addPrt;
}

void MyPrinters::setUI()
{
	ui.tableWidget_printerList->setColumnWidth(typeColumn, 70); //Type
	ui.tableWidget_printerList->setColumnWidth(nameColumn, 140); //PrinterName
	ui.tableWidget_printerList->setColumnWidth(modelColumn, 100); //Model
	ui.tableWidget_printerList->setColumnWidth(ipColumn, 110);  //IP
	ui.tableWidget_printerList->setColumnWidth(checkColumn, 80);  //Defualt
	ui.tableWidget_printerList->setColumnWidth(delColumn, 60);  //Del
	ui.tableWidget_printerList->setIconSize(QSize(10, 10));
	QHeaderView *verticalHeader = ui.tableWidget_printerList->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);

	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	QHeaderView* headerView = ui.tableWidget_printerList->horizontalHeader();
	headerView->setSectionsClickable(false);
}

void MyPrinters::setConnection()
{
	connect(ui.openAddMyPrinter, SIGNAL(labelClicked()), this, SLOT(showAddNetPrinter()));
	connect(ui.openAddUSBPrinter, SIGNAL(labelClicked()), this, SLOT(showAddUSBPrinter()));
	connect(ui.buttonOK, SIGNAL(clicked()), this, SLOT(clickedOK()));
	connect(ui.tableWidget_printerList, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(tableWidget_printerList_cellDoubleClicked(int, int)));
}

void MyPrinters::showMyPrinters()
{
	ui.tableWidget_printerList->clearContents();
	ui.tableWidget_printerList->setRowCount(0);
	loadFile();
	//auto-select row
	if (defaultIndex >= 0)
		ui.tableWidget_printerList->setCurrentCell(defaultIndex, 0);
	this->show();
	//ui.myPrinters->topLevelItem(defaultIndex)->setSelected(true);
}

void MyPrinters::showAddNetPrinter()
{
	AddMyPrinter *addPrt = new AddMyPrinter(this);
	addPrt->myPrinters = this;
	addPrt->showAddNetPrinter();
}

void MyPrinters::showAddUSBPrinter()
{
	AddMyPrinter *addPrt = new AddMyPrinter(this);
	addPrt->myPrinters = this;
	addPrt->showAddUSBPrinter();
}

void MyPrinters::showFindIPPrinter()
{
	AddIPAddress *addIP = new AddIPAddress(this);
	addIP->myPrinters = this;
	addIP->initForm();
}

void MyPrinters::addComp(QString connectionType, QString printerName, QString ip, QString model, QString defaultYN)
{
	int targetRow = ui.tableWidget_printerList->rowCount();
	ui.tableWidget_printerList->insertRow(targetRow);
	QTableWidgetItem *typeItem = new QTableWidgetItem(connectionType);
	typeItem->setFlags(typeItem->flags() ^ Qt::ItemIsEditable);
	ui.tableWidget_printerList->setItem(targetRow, typeColumn, typeItem);
	ui.tableWidget_printerList->setItem(targetRow, nameColumn, new QTableWidgetItem(printerName));
	QTableWidgetItem *modelItem = new QTableWidgetItem(model);
	modelItem->setFlags(modelItem->flags() ^ Qt::ItemIsEditable);
	ui.tableWidget_printerList->setItem(targetRow, modelColumn, modelItem);
	QTableWidgetItem *ipItem = new QTableWidgetItem(ip);
	ipItem->setFlags(ipItem->flags() ^ Qt::ItemIsEditable);
	ui.tableWidget_printerList->setItem(targetRow, ipColumn, ipItem);

	QFont font("Malgun Gothic", 9, QFont::Normal);
	ui.tableWidget_printerList->item(targetRow, typeColumn)->setFont(font);
	ui.tableWidget_printerList->item(targetRow, nameColumn)->setFont(font);
	ui.tableWidget_printerList->item(targetRow, modelColumn)->setFont(font);
	ui.tableWidget_printerList->item(targetRow, ipColumn)->setFont(font);
	ui.tableWidget_printerList->item(targetRow, modelColumn)->setTextAlignment(Qt::AlignCenter);

	QWidget *widget_default = new QWidget();
	QCheckBox *checkBox_default = new QCheckBox(widget_default);
	QHBoxLayout *hBoxLayout_default = new QHBoxLayout(widget_default);
	hBoxLayout_default->setContentsMargins(0, 0, 0, 0);
	hBoxLayout_default->addWidget(checkBox_default);
	hBoxLayout_default->setAlignment(Qt::AlignCenter);
	if (defaultYN == "Y")
		checkBox_default->setChecked(true);
	connect(checkBox_default, SIGNAL(clicked(bool)), this, SLOT(checkBox_default_clicked(bool)));
	ui.tableWidget_printerList->setCellWidget(targetRow, checkColumn, widget_default);

	QWidget *widget_delete = new QWidget();
	QPushButton* pushButton_delete = new QPushButton(widget_delete);
	QHBoxLayout *hBoxLayout_delete = new QHBoxLayout(widget_delete);
	pushButton_delete->setIcon(QIcon(QPixmap(":/rodinP/Resources/cancel.png")));
	pushButton_delete->setIconSize(QSize(10, 10));
	pushButton_delete->setFixedSize(14, 14);
	pushButton_delete->setStyleSheet("background-color: rgb(255,255,255); border-color: rgb(100,100,100); color: rgb(255, 255, 255);border-radius: 2px; ");
	hBoxLayout_delete->addWidget(pushButton_delete);
	hBoxLayout_delete->setAlignment(Qt::AlignCenter);
	hBoxLayout_delete->setContentsMargins(0, 0, 0, 0);
	connect(pushButton_delete, SIGNAL(clicked()), this, SLOT(pushButton_delete_clicked()));
	ui.tableWidget_printerList->setCellWidget(targetRow, delColumn, widget_delete);
}

void MyPrinters::addCompWithCheck(QString connectionType, QString printerName, QString ip, QString model, QString defaultYN)
{
	if (checkDup(ip)) {
		//QMessageBox::warning(this, tr("3Dwox"), tr("This printer has already been registered.\nPlease select another printer."));
		//commonDlg->setDialogContents(tr("This printer has already been registered.\nPlease select another printer."), CommonDialog::Warning);
		//emit sendDialogContents(tr("printer_already_registered"), 2);
		CommonDialog comDlg(this, MessageAlert::tr("printer_already_registered"), CommonDialog::Warning);
		//NULL;
	}
	else
	{
		if (!m_defaultFlag) defaultYN = "Y";
		int rowCount = ui.tableWidget_printerList->rowCount();
		addComp(connectionType, printerName, ip, model, defaultYN);
		ui.tableWidget_printerList->setCurrentCell(rowCount, 0);
	}
}
bool MyPrinters::checkDup(QString ip)
{
	m_defaultFlag = false;
	int rowCount = ui.tableWidget_printerList->rowCount();
	QString listIP;
	for (int row = 0; row < rowCount; row++)
	{
		listIP = ui.tableWidget_printerList->item(row, ipColumn)->text();
		if (listIP == ip)
		{
			ui.tableWidget_printerList->setCurrentCell(row, 0);
			return true;
		}

		Q_FOREACH(QCheckBox *widget, ui.tableWidget_printerList->cellWidget(row, checkColumn)->findChildren<QCheckBox*>())
		{
			if (widget->isChecked())
				m_defaultFlag = true;;
		}
	}
	return false;
}

void MyPrinters::loadFile()
{
	QFile inputFile;
	Logger() << "loadFile path : " << Generals::appDataPath;
	inputFile.setFileName(Generals::appDataPath + "/myPrinters.ini");

	QList<QString> txtList;
	defaultIndex = -1;

	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			txtList.append(in.readLine());
		}
		int listCount = txtList.count();
		int check = listCount % columnCnt;
		if (check == 0)
		{
			for (int j = 0; j < listCount / columnCnt; j++)
			{
				QString connectionType;
				QString printerName;
				QString model;
				QString ip;
				QString defaultYN;

				connectionType = txtList.at(j * columnCnt + typeColumn);
				printerName = txtList.at(j * columnCnt + nameColumn);
				model = txtList.at(j * columnCnt + modelColumn);
				ip = txtList.at(j * columnCnt + ipColumn);
				defaultYN = txtList.at(j * columnCnt + checkColumn);

				if (currentIP == NULL & defaultYN == "Y" & defaultIndex == -1)
					defaultIndex = j;
				else if (currentIP == ip & defaultIndex == -1)
					defaultIndex = j;
				//qDebug() << defualtIndex;
				//qDebug() << a;
				//qDebug() << b;
				//qDebug() << c;

				addComp(connectionType, printerName, ip, model, defaultYN);
			}
		}
		inputFile.close();
	}
}

void MyPrinters::saveFile()
{
	/*QFile file("out.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	return;

	//QTextStream out(&file);
	file.write(data);
	file.close();*/

	QFile file;
	file.setFileName(Generals::appDataPath + "/myPrinters.ini");

	if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
	{
		QTextStream stream(&file);
		QList<QString> txtList = getComp();
		//qDebug() << txtList.count();
		for (int i = 0; i < txtList.size(); i++) {
			//qDebug() << txtList.at(i);
			stream << txtList.at(i);
			if (i != txtList.size()) stream << "\n";
		}
		file.close();
	}
}

QList<QString> MyPrinters::getComp()
{
	QList<QString> txtList;
	int rowCount = ui.tableWidget_printerList->rowCount();
	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < columnCnt; column++)
		{
			if (column == checkColumn)
			{
				Q_FOREACH(QCheckBox *widget, ui.tableWidget_printerList->cellWidget(row, checkColumn)->findChildren<QCheckBox*>())
				{
					if (widget->isChecked())
						txtList.append("Y");
					else
						txtList.append("N");
					break;
				}
			}
			else
				txtList.append(ui.tableWidget_printerList->item(row, column)->text());
		}
	}
	return txtList;
}

void MyPrinters::clickedOK()
{
	saveFile();
	selectPrinter();
	close();
}

void MyPrinters::tableWidget_printerList_cellDoubleClicked(int _row, int _column)
{
	//qDebug() << tree->text(0);
	if (_column == nameColumn)
	{
		//printerName을 DoubleClick할 경우 Edit
		NULL;
	}
	else
	{
		clickedOK();
	}
}

void MyPrinters::pushButton_delete_clicked()
{
	QPushButton* const pushButton_delete = qobject_cast<QPushButton*>(sender());
	int rowCount = ui.tableWidget_printerList->rowCount();
	for (int i = 0; i < rowCount; i++)
	{
		Q_FOREACH(QPushButton *widget, ui.tableWidget_printerList->cellWidget(i, delColumn)->findChildren<QPushButton*>())
		{
			if (widget == pushButton_delete)
				ui.tableWidget_printerList->removeRow(i);
		}
	}
}

void MyPrinters::checkBox_default_clicked(bool _checked)
{
	QCheckBox* const checkBox_default = qobject_cast<QCheckBox*>(sender());
	int rowCount = ui.tableWidget_printerList->rowCount();
	for (int i = 0; i < rowCount; i++)
	{
		Q_FOREACH(QCheckBox *widget, ui.tableWidget_printerList->cellWidget(i, checkColumn)->findChildren<QCheckBox*>())
		{
			if (widget == checkBox_default)
				ui.tableWidget_printerList->setCurrentCell(i, 0);
			else
				widget->setChecked(false);
		}
	}
}

void MyPrinters::selectPrinter()
{
	if (ui.tableWidget_printerList->rowCount() == 0)
	{
		CommonDialog comDlg(this, MessageAlert::tr("please_register_printer"), CommonDialog::Warning);
		//emit sigSelectPrinter("", "", "", "");
		PrinterInfo::clear(Profile::machineProfile.machine_model);
		emit signal_printerSelected();
	}
	else if (ui.tableWidget_printerList->selectedItems().size() == 0)
	{
		CommonDialog comDlg(this, MessageAlert::tr("please_select_printer"), CommonDialog::Warning);
		//emit sigSelectPrinter("", "", "", "");
		PrinterInfo::clear(Profile::machineProfile.machine_model);
		emit signal_printerSelected();
	}
	else
	{
		int	currentRow = ui.tableWidget_printerList->currentRow();
		QString connectionType = ui.tableWidget_printerList->item(currentRow, typeColumn)->text();
		QString printerName = ui.tableWidget_printerList->item(currentRow, nameColumn)->text();
		QString model = ui.tableWidget_printerList->item(currentRow, modelColumn)->text();
		QString ip = ui.tableWidget_printerList->item(currentRow, ipColumn)->text();
		//emit sigSelectPrinter(connectionType, ip, printerName, model);
		PrinterInfo::setPrinterInfo(connectionType, ip, printerName, model);
		emit signal_printerSelected();
	}
}