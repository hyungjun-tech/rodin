#pragma once

#include "ui_PrintConfirm.h"

class ViewerModule;
class PrintingInfo;
class PrintConfirm : public QDialog
{
	Q_OBJECT

public:
	PrintConfirm(ViewerModule *viewerModule_);
	~PrintConfirm();
	bool setContents();
	bool isYes() { return yesFlag; };
	bool isNo() { return noFlag; };
	QString getPW() { return securityPW; };
	//void reUI();
private slots:
	void pushButton_yes_clicked();
	void pushButton_no_clicked();
private:
	Ui::PrintConfirm ui;
	QWidget* parent;
	ViewerModule* viewerModule;
	PrintingInfo* printingInfo;

	bool yesFlag = false;
	bool noFlag = false;





	bool isSecurityPrint;
	QString securityPW;

	bool getSecurityPrint();
	void setAlertMessage();
	void setUI();
};