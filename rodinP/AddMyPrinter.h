#pragma once

#include "ui_AddMyPrinter.h"
#include "Communication.h"

class MyPrinters;
class SearchNetworkPrinter;
class AddMyPrinter : public QDialog
{
	Q_OBJECT

public:
	AddMyPrinter();
	AddMyPrinter(QWidget *parent);
	~AddMyPrinter();

	//searchIPThread * thread;
	QList<SearchNetworkPrinter*> trds;
	//void stratThread();
	//bool fromAction = false;
	//QString getPJLValue(PrinterInterface, QString);
	//QString ParsePJL(char *pbuf, int len, QString inqCommend);

	void findMyPrinters();

	QString m_connectionType;
	MyPrinters *myPrinters;
public slots:
	void showAddIPAddress();
	void showMyPrinters();
	void click_action(QTreeWidgetItem* tree, int a);
	void addComp(QString a, QString b, QString c);
	void clickAdd();
	void clickCancel();
	void clickClose();
	bool threadIsRunning();
	void threadStop();
	void stopAni();

	void showAddNetPrinter() { showAddMyPrinter("Network"); };
	void showAddUSBPrinter() { showAddMyPrinter("USB"); };
	void showAddMyPrinter(QString connectionType);
signals:
	void labelClicked();
	void sigShowAddIP();
	void sigAddMyPrinter(QString connectionType, QString printerName, QString IP, QString model);
	//void sendDialogContents(QString p_message, int p_type, bool p_OkVisible = true, bool p_cancelVisible = false, bool p_yesVisible = false, bool p_noVisible = false, bool p_delay = false);
private:
	Ui::AddMyPrinter ui;
	QHostAddress localIP;
	//ProgressIndicator* pi;
	ProgressIndicator pi;

	void setUI();
	void setConnection();
	void keyPressEvent(QKeyEvent *e);

	int nameColumn = 0;
	int modelColumn = 1;
	int ipColumn = 2;
};