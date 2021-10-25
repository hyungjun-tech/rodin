#pragma once

#include "ui_MultiplyDialog.h"

class ViewerModule;
class MultiplyDialog : public QDialog
{
	Q_OBJECT
public:
	MultiplyDialog(ViewerModule* viewerModule_);
	~MultiplyDialog();

private slots:
	void pushButton_ok_clicked();

private:
	Ui::MultiplyDialogClass ui;
	ViewerModule* viewerModule;
};