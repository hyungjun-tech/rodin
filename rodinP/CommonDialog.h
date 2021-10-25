#pragma once

#include "ui_CommonDialog.h"
#include "CommonWidgets.h"

class CommonDialog : public QDialog
{
	Q_OBJECT

public:
	CommonDialog(QWidget *parent = 0);
	CommonDialog(QWidget *parent, bool closeFlag_);
	CommonDialog(QWidget *parent, QString p_message, int p_type = Warning, bool p_OkVisible = true, bool p_cancelVisible = false, bool p_yesVisible = false, bool p_noVisible = false, bool p_delay = false, bool p_continueVisible = false);
	~CommonDialog();

	enum msgType {
		// keep this in sync with QMessageDialogOptions::Icon
		NoIcon = 0,
		Information = 1,
		Warning = 2,
		Critical = 3,
		Question = 4,
		Progress = 5
	};
	bool isCancel() { return cancelFlag; };
	bool isOk() { return okFlag; };
	bool isYes() { return yesFlag; };
	bool isNo() { return noFlag; };
	bool isCheck() { return checked; };
	bool isContinue() { return continueFlag; };
public slots:
	void setProgressValue(int);
	void setDialogContents(QString p_message, int p_type, bool p_OkVisible = true, bool p_cancelVisible = false, bool p_yesVisible = false, bool p_noVisible = false, bool p_delay = false, bool p_continueVisible = false);
	void setCheckbox(bool p_checkboxVisible, QString p_message = "", bool p_check = false);
	void setMessage(QString p_message);
	void clickCancel();
	void clickOk();
	void clickYes();
	void clickNo();
	void clickCheckbox();
	void clickContinue();
signals:
	void clickCancelSig();
	void clickOkSig();
	void clickYesSig();
	void clickNoSig();
	void clickContinueSig();
protected:
	virtual void keyPressEvent(QKeyEvent *e);
private:
	Ui::CommonDialog ui;
	ProgressIndicator pi;

	void init();
	bool cancelFlag = false;
	bool okFlag = false;
	bool yesFlag = false;
	bool noFlag = false;
	bool checked = false;
	bool continueFlag = false;
	bool closeFlag = true;
};