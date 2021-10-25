#pragma once

#include "ui_CartridgeDialog.h"

class CartridgeDialog : public QDialog
{
	Q_OBJECT

public:
	CartridgeDialog(QWidget *parent = 0);
	~CartridgeDialog();

private:
	Ui::CartridgeDialog ui;

	void deleteFilaInfo();
	void addFilaInfo(int idx, int filaRemainCnt, QString material);
	void refreshContents();
private slots:
	void clickedColorSetting();
	void clickedOK();
	void stateChanged_usingCustomColor();
signals:
	void signal_customColorChanged();
};