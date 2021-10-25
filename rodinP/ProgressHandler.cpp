#include "stdafx.h"
#include "ProgressHandler.h"

ProgressHandler::ProgressHandler(QWidget *parent_)
	: initValue(0)
	, targetValue(100)
{
	progressDialog = new QProgressDialog(parent_);
	Qt::WindowFlags wf = progressDialog->windowFlags() & (~Qt::WindowContextHelpButtonHint) & (~Qt::WindowCloseButtonHint);
	progressDialog->setWindowFlags(wf);
	progressDialog->setWindowModality(Qt::ApplicationModal);
	progressDialog->setMinimumDuration(100);
	progressDialog->setWindowTitle(AppInfo::getAppName());
	progressDialog->setFont(QFont("Malgun Gothic", 10));
	progressDialog->setCancelButtonText(CustomTranslate::tr("Cancel"));
	progressDialog->setValue(0);
	progressDialog->setMinimum(0);
	progressDialog->setMaximum(100);
	connect(progressDialog, SIGNAL(canceled()), this, SIGNAL(signal_canceled()));
}
ProgressHandler::~ProgressHandler()
{
	progressDialog->deleteLater();
}
void ProgressHandler::show()
{
	progressDialog->show();
}
bool ProgressHandler::wasCanceled()
{
	return progressDialog->wasCanceled();
}
void ProgressHandler::setMinimum(int minimum_)
{
	progressDialog->setMinimum(minimum_);
}
void ProgressHandler::setMaximum(int maximum_)
{
	progressDialog->setMaximum(maximum_);
}
void ProgressHandler::setValue(int value_)
{
	progressDialog->setValue(value_);
}
void ProgressHandler::setCustomValue(int _value)
{
	progressDialog->setValue(initValue + _value * (targetValue - initValue) / 100);
}
void ProgressHandler::setInitValue(int _value)
{
	initValue = _value;
}
void ProgressHandler::setTargetValue(int value)
{
	targetValue = value;
}
void ProgressHandler::setValue(float value_)
{
	progressDialog->setValue(value_ * targetValue);
}
void ProgressHandler::setLabelText(QString label_)
{
	progressDialog->setLabelText(label_);
}
void ProgressHandler::setWindowTitle(QString title_)
{
	progressDialog->setWindowTitle(title_);
}
void ProgressHandler::close()
{
	progressDialog->close();
}
void ProgressHandler::tempProgress(int from_, int to_, int msSec_)
{
	int value;
	int maximum = progressDialog->maximum();
	for (int i = from_; i <= to_; i++) {
		value = i * maximum / 100;
		progressDialog->setValue(value);
		QThread::msleep(msSec_);	// mili-seconds

		if (progressDialog->wasCanceled()) {
			return;	// Cancelled
		}
	}
}