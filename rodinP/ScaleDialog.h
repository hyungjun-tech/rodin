#pragma once

#include "ui_ScaleDialog.h"

class ViewerModule;
class ScaleDialog : public QDialog
{
	Q_OBJECT

public:
	ScaleDialog(QWidget* _parent);
	~ScaleDialog();
	void setContents(ViewerModule* const _viewerModule);

	void scale(double x, double y, double z);
	void scalemm(double x, double y, double z);
	void resetScale();
	void maxSize();

private:
	Ui::ScaleDialog ui;
	ViewerModule* viewerModule;

	qglviewer::Vec scaledFactor;
	bool hasMultiScale;
	qglviewer::Vec aabbLength;
	bool noChange;
	void updateScaleFromVolume();

	void scale(double value_, int dim_);
	void setScaleMinMax();
private slots:
	void checkModelSelected();
	void pushButton_inches_to_millimeters_clicked();
	void pushButton_resetscale_clicked();
	void doubleSpinBox_scale_x_valueChanged(double);
	void doubleSpinBox_scale_y_valueChanged(double);
	void doubleSpinBox_scale_z_valueChanged(double);
	void doubleSpinBox_scale_x_mm_valueChanged(double);
	void doubleSpinBox_scale_y_mm_valueChanged(double);
	void doubleSpinBox_scale_z_mm_valueChanged(double);
	void checkBox_uniform_scale_stateChanged();
	void pushButton_maxsize_clicked();
};