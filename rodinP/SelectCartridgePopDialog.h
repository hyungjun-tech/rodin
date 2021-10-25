#pragma once

#include "ui_SelectCartridgePopDialog.h"

class SelectCartridgePopDialog : public QDialog
{
	Q_OBJECT

public:
	SelectCartridgePopDialog(QWidget *parent);
	~SelectCartridgePopDialog();

	void showExportDialog();
	void showImportDialog();
private slots:
	void pushButton_ok_clicked_import();
	void pushButton_ok_clicked_export();
private:
	Ui::SelectCartridgePopDialog ui;

	void importProfile(int profileSelectedIndex);
	void exportProfile(int profileSelectedIndex);
signals:
	void signal_profileChanged(bool flag_ = true);
};
