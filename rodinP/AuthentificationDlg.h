#pragma once

#include "ui_AuthentificationDlg.h"

typedef unsigned char byte;

class AuthentificationDlg : public QDialog
{
	Q_OBJECT

public:
	AuthentificationDlg(int mode, QWidget *parent = 0);
	~AuthentificationDlg();

	void clear();

private slots:
	void pushButton_ok_clicked();

private:
	Ui::AuthentificationDlg ui;
	QString m_id;
	QString m_pwd;
protected:
	virtual void keyPressEvent(QKeyEvent *e);
};