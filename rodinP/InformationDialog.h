#pragma once
#include "ui_InformationDialog.h"

class rodinP;
class InformationDialog : public QDialog
{
	Q_OBJECT

public:
	InformationDialog(QWidget *parent);
	~InformationDialog();
private:
	Ui::InformationDialog ui;
	//rodinP* parentClass;

	void setContents(QString platform);
};

