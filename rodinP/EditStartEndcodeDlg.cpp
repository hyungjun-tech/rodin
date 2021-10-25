#include "stdafx.h"
#include "EditStartEndcodeDlg.h"

EditStartEndcodeDlg::EditStartEndcodeDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));

	connect(ui.pushButton_apply, SIGNAL(clicked()), this, SLOT(pushButton_apply_clicked()));
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(pushButton_ok_clicked()));
	connect(ui.textEdit_startGcode, SIGNAL(textChanged()), this, SLOT(setEnabledApplyButton()));
	connect(ui.textEdit_endGcode, SIGNAL(textChanged()), this, SLOT(setEnabledApplyButton()));

	loadSettingValue();

	ui.pushButton_apply->setEnabled(false);

	exec();
}


EditStartEndcodeDlg::~EditStartEndcodeDlg()
{

}

void EditStartEndcodeDlg::pushButton_apply_clicked()
{
	//
	saveSettingValue();
}

void EditStartEndcodeDlg::pushButton_ok_clicked()
{
	if (ui.pushButton_apply->isEnabled())
		this->saveSettingValue();

	this->close();
}

void EditStartEndcodeDlg::setEnabledApplyButton()
{
	ui.pushButton_apply->setEnabled(true);
}

void EditStartEndcodeDlg::loadSettingValue()
{
	ui.textEdit_startGcode->setText(QString::fromStdString(Profile::machineProfile.start_default_code));
	ui.textEdit_endGcode->setText(QString::fromStdString(Profile::machineProfile.end_default_code));
}

void EditStartEndcodeDlg::saveSettingValue()
{
	QString strTmp = ui.textEdit_startGcode->toPlainText();
	Profile::machineProfile.start_default_code = strTmp.toStdString();

	QString strTmp2 = ui.textEdit_endGcode->toPlainText();
	Profile::machineProfile.end_default_code = strTmp2.toStdString();

	ui.pushButton_apply->setEnabled(false);
}