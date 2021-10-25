#include "stdafx.h"
#include "CommonDialog.h"

CommonDialog::CommonDialog(QWidget *parent)
	: QDialog(parent)
{
	init();
}
CommonDialog::CommonDialog(QWidget *parent, bool closeFlag_)
	: QDialog(parent)
	, closeFlag(closeFlag_)
{
	init();
}

CommonDialog::CommonDialog(QWidget *parent, QString p_message, int p_type, bool p_OkVisible, bool p_cancelVisible, bool p_yesVisible, bool p_noVisible, bool p_delay, bool p_continueVisible)
	: QDialog(parent)
{
	init();
	if (p_message != "") setDialogContents(p_message, p_type, p_OkVisible, p_cancelVisible, p_yesVisible, p_noVisible, p_delay, p_continueVisible);
	exec();
}

CommonDialog::~CommonDialog()
{
}

void CommonDialog::init()
{
	ui.setupUi(this);
	Qt::WindowFlags wf = windowFlags() & (~Qt::WindowContextHelpButtonHint) & (~Qt::WindowCloseButtonHint);
	setWindowFlags(wf);
	setWindowTitle(AppInfo::getAppName());

	ProgressIndicator pi;
	QHBoxLayout* hbl = new QHBoxLayout(ui.delay);
	hbl->addWidget(&pi);
	pi.setAnimationDelay(100);
	ui.checkBox->setVisible(false);

	connect(ui.buttonOK, SIGNAL(clicked()), this, SLOT(clickOk()));
	connect(ui.buttonCancel, SIGNAL(clicked()), this, SLOT(clickCancel()));
	connect(ui.buttonYes, SIGNAL(clicked()), this, SLOT(clickYes()));
	connect(ui.buttonNo, SIGNAL(clicked()), this, SLOT(clickNo()));
	connect(ui.buttonContinue, SIGNAL(clicked()), this, SLOT(clickContinue()));
	connect(ui.checkBox, SIGNAL(clicked()), this, SLOT(clickCheckbox()));
}

void CommonDialog::setDialogContents(QString p_message, int p_type, bool p_OkVisible, bool p_cancelVisible, bool p_yesVisible, bool p_noVisible, bool p_delay, bool p_continueVisible)
{
	Logger() << "Message Box : " << p_message;
	/*this->size().setWidth(340);
	this->size().setHeight(140);*/
	ui.progressBar->setVisible(false);
	ui.delay->setVisible(false);
	ui.img_confirm->setVisible(false);
	ui.img_alert->setVisible(false);
	ui.img_error->setVisible(false);
	ui.img_info->setVisible(false);
	okFlag = false;
	cancelFlag = false;
	yesFlag = false;
	noFlag = false;

	if (!this->isVisible()) this->show();
	ui.Message->setText(p_message);
	ui.buttonOK->setVisible(p_OkVisible);
	ui.buttonCancel->setVisible(p_cancelVisible);
	ui.buttonYes->setVisible(p_yesVisible);
	ui.buttonNo->setVisible(p_noVisible);
	ui.buttonContinue->setVisible(p_continueVisible);
	if (p_delay)
	{
		ui.delay->setVisible(p_delay);
		pi.startAnimation();
	}
	else
	{
		ui.delay->setVisible(p_delay);
		pi.stopAnimation();
	}

	if (p_type == Information)
	{
		ui.img_info->setVisible(true);
	}
	else if (p_type == Warning)
	{
		ui.img_alert->setVisible(true);
	}
	else if (p_type == Critical)
	{
		ui.img_error->setVisible(true);
	}
	else if (p_type == Question)
	{
		ui.img_confirm->setVisible(true);
	}
	else if (p_type == Progress)
	{
		ui.progressBar->setValue(0);
		ui.progressBar->setVisible(true);
	}
	this->adjustSize();
	show();
	/*if (!this->isVisible()) this->show();
	ui.Message->setText(p_message);
	ui.buttonOK->setVisible(p_buttonOK);
	ui.buttonCancel->setVisible(p_buttonCancel);
	ui.progressBar->setVisible(p_progressBar);
	if (p_delay)
	{
		ui.delay->setVisible(p_delay);
		pi->startAnimation();
	}
	else
	{
		ui.delay->setVisible(p_delay);
		pi->stopAnimation();
	}*/
}
void CommonDialog::setCheckbox(bool p_checkboxVisible, QString p_message, bool p_check)
{
	if (p_checkboxVisible) checked = p_check;
	ui.checkBox->setVisible(p_checkboxVisible);
	ui.checkBox->setText(p_message);
	ui.checkBox->setChecked(p_check);
	//checked = ui.checkBox->isChecked();
}
void CommonDialog::setProgressValue(int p_value)
{
	ui.progressBar->setValue(p_value);
}
void CommonDialog::setMessage(QString p_message)
{
	ui.Message->setText(p_message);
}

void CommonDialog::clickCancel()
{
	Logger() << "Message Box : Cancel Clicked";
	cancelFlag = true;
	emit clickCancelSig();
	if (closeFlag) close();
}

void CommonDialog::clickOk()
{
	Logger() << "Message Box : OK Clicked";
	okFlag = true;
	emit clickOkSig();
	close();
}
void CommonDialog::clickYes()
{
	Logger() << "Message Box : Yes Clicked";
	yesFlag = true;
	emit clickYesSig();
	if (closeFlag) close();
}
void CommonDialog::clickNo()
{
	Logger() << "Message Box : No Clicked";
	noFlag = true;
	emit clickNoSig();
	if (closeFlag) close();
}
void CommonDialog::clickContinue()
{
	Logger() << "Message Box : Continue Clicked";
	continueFlag = true;
	emit clickContinueSig();
	if (closeFlag) close();
}
void CommonDialog::clickCheckbox()
{
	Logger() << "Message Box : CheckBox Clicked : " + ui.checkBox->isChecked();
	checked = ui.checkBox->isChecked();
}

void CommonDialog::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Escape)
		return;
	else if (e->key() == Qt::Key_Enter || Qt::Key_Return)
	{
		if (ui.buttonOK->isVisible()) clickOk();
		if (ui.buttonYes->isVisible()) clickYes();
	}
}