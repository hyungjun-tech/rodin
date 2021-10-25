#pragma once

#include "ui_RotationDialog.h"

class ViewerModule;
class RotationDialog : public QDialog
{
	Q_OBJECT

public:
	RotationDialog(QWidget* _parent);
	~RotationDialog();
	void setContents(ViewerModule* const _viewerModule);

private slots:
	void checkModelSelected();
	void pushButton_apply_rot_clicked();
	void pushButton_resetori_clicked();
	void pushButton_layFlat_clicked();
	void pushButton_axisXplus90_clicked();
	void pushButton_axisXminus90_clicked();
	void pushButton_axisYplus90_clicked();
	void pushButton_axisYminus90_clicked();
	void pushButton_axisZplus90_clicked();
	void pushButton_axisZminus90_clicked();
private:
	Ui::RotationDialog ui;
	ViewerModule* viewerModule;
	
	void rotate(double rad_x, double rad_y, double rad_z);
};