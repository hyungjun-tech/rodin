#pragma once

#include "ui_SelectModelDialog.h"

class SelectModelDialog : public QDialog
{
	Q_OBJECT

public:
	SelectModelDialog(QWidget *parent = 0);
	~SelectModelDialog();
	bool isCancel() { return cancelFlag; };
public slots:
	void clickedOK();
	void clickedCancel();
	void machineChanged(int index);
private:
	Ui::SelectModelDialog ui;
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	int m_nMouseClick_X_Coordinate;
	int m_nMouseClick_Y_Coordinate;
	bool cancelFlag = false;
};