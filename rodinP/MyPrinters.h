#pragma once

#include "ui_MyPrinters.h"
#include "AddIPAddress.h"
#include "AddMyPrinter.h"
//#include "rodinP.h"

//class PrinterList
//{
//public:
//	QString type;
//	QString name;
//	QString model;
//	QString ip;
//	QString defautlYN;
//};
//

class MyPrinters : public QDialog
{
	Q_OBJECT

public:
	MyPrinters();
	MyPrinters(QWidget *parent);
	~MyPrinters();
	bool checkDup(QString ip);
	void setCurrentIP(QString ip_) { currentIP = ip_; }

public slots:
	void showMyPrinters();
	void showAddNetPrinter();
	void showAddUSBPrinter();
	void showFindIPPrinter();
	//void showAddMyPrinter(){ emit sigShowAddMyPrt(); };
	//void showAddUSBPrinter(){ emit sigShowAddUSBPrt(); };
	void addComp(QString connectionType, QString printerName, QString ip, QString model, QString defaultYN);
	void addCompWithCheck(QString connectionType, QString printerName, QString ip, QString model, QString defaultYN);
	void loadFile();
	void saveFile();
	QList<QString> getComp();
	void clickedOK();

private slots:
	void tableWidget_printerList_cellDoubleClicked(int _row, int _column);
	void pushButton_delete_clicked();
	void checkBox_default_clicked(bool _checked);

signals:
	//void labelClicked();
	//void sigShowAddMyPrt();
	//void sigShowAddUSBPrt();
	void sigSelectPrinter(QString connectionType, QString ip, QString printerName, QString model);
	void signal_printerSelected();
	//void sendDialogContents(QString p_message, int p_type, bool p_OkVisible = true, bool p_cancelVisible = false, bool p_yesVisible = false, bool p_noVisible = false, bool p_delay = false);
private:
	Ui::MyPrinters ui;
	int defaultIndex = -1;
	bool m_defaultFlag;

	int columnCnt = 5;
	int typeColumn = 0;
	int nameColumn = 1;
	int modelColumn = 2;
	int ipColumn = 3;
	int checkColumn = 4;
	int delColumn = 5;

	QString currentIP;

	void setUI();
	void setConnection();
	void selectPrinter();

	//AddIPAddress * addIP;
	//AddMyPrinter * addPrt;
};