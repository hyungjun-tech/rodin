#pragma once

#include "ModelContainer.h"
#include "VolumeAnalysis.h"

#include "ui_VolumAnalysisDialog.h"

class VolumeAnalysis;
class ViewerModule;
class ConfigSettings;
class VolumeAnalysisDialog : public QDialog
{
	Q_OBJECT

public:
	VolumeAnalysisDialog(QWidget* _parent);
	~VolumeAnalysisDialog();
	bool init(ViewerModule* const _viewerModule);
	bool isOverhang() const { return ui.checkBox_overhang->isChecked(); }
	//VolumeAnalysis m_VolumeAnalysis;

private slots:
	void doubleSpinBox_minthick_valueChanged(double value);
	void horizontalSlider_minthick_valueChanged(int value);
	void horizontalSlider_overhang_valueChanged(int value);
	void checkBox_overhang_toggled(bool state);

private:
	Ui::VolumAnalysisDialogClass ui;
	VolumeAnalysis* m_VolumeAnalysis;
	ViewerModule* viewerModule;
signals:
	void signal_updateGL();
};

