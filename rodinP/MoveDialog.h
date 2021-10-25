#pragma once

#include "ui_MoveDialog.h"

class ViewerModule;
class MoveDialog : public QDialog
{
	Q_OBJECT

public:
	MoveDialog(QWidget* _parent);
	~MoveDialog();
	void setContents(ViewerModule* const _viewerModule);

private slots:
	void checkModelSelected();
	void pushButton_apply_trans_clicked();
	void pushButton_bedcenter_clicked();
	void pushButton_resetpos_clicked();
private:
	Ui::MoveDialog ui;
	ViewerModule* viewerModule;
};

