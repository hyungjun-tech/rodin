#pragma once

#include "ui_AddIPAddress.h"

class MyPrinters;
class AddIPAddress : public QDialog
{
	Q_OBJECT

public:
	AddIPAddress(QWidget *parent);
	~AddIPAddress();
	MyPrinters *myPrinters;
public slots:
	void initForm();
	void showMyPrinters();
	bool checkIPAddress();
	void clickAdd();
private:
	Ui::AddIPAddress ui;

	QString printerName;
	QString ipAddress;
	QString model;
};