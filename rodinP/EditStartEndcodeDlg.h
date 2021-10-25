#pragma once

#include "ui_EditStartEndcodeDlg.h"

class EditStartEndcodeDlg : public QDialog
{
	Q_OBJECT

public:
	EditStartEndcodeDlg(QWidget *parent);
	~EditStartEndcodeDlg();

private slots:
	void pushButton_apply_clicked();
	void pushButton_ok_clicked();
	void setEnabledApplyButton();

private:
	Ui::EditStartEndcodeDlg ui;

	void loadSettingValue();
	void saveSettingValue();
};