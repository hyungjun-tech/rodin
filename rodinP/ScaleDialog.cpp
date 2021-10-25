#include "stdafx.h"
#include "ScaleDialog.h"
#include "ViewerModule.h"

ScaleDialog::ScaleDialog(QWidget* _parent)
	: QDialog(_parent)
	, scaledFactor{ 1, 1, 1 }
	, noChange(false)
	, hasMultiScale(false)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose);


	//updateScaleFromVolume();

	//setScaleMinMax();
}

ScaleDialog::~ScaleDialog()
{
	
}

void ScaleDialog::setContents(ViewerModule* const _viewerModule)
{
	viewerModule = _viewerModule;
	checkModelSelected();

	connect(viewerModule->modelContainer, SIGNAL(signal_modelSelect()), this, SLOT(checkModelSelected()));
	connect(viewerModule->modelContainer, SIGNAL(signal_modelDeselect()), this, SLOT(checkModelSelected()));
	connect(viewerModule->modelContainer, SIGNAL(signal_modelDeleted()), this, SLOT(checkModelSelected()));

	connect(ui.pushButton_maxsize, SIGNAL(clicked(bool)), this, SLOT(pushButton_maxsize_clicked()));
	//connect(ui.pushButton_inches_to_millimeters, SIGNAL(clicked(bool)), m_Viewer, SLOT(ScaleInchesToMM()));
	connect(ui.pushButton_inches_to_millimeters, SIGNAL(clicked(bool)), this, SLOT(pushButton_inches_to_millimeters_clicked()));
	connect(ui.pushButton_resetscale, SIGNAL(clicked(bool)), this, SLOT(pushButton_resetscale_clicked()));

	connect(ui.doubleSpinBox_scale_x, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBox_scale_x_valueChanged(double)));
	connect(ui.doubleSpinBox_scale_y, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBox_scale_y_valueChanged(double)));
	connect(ui.doubleSpinBox_scale_z, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBox_scale_z_valueChanged(double)));
	connect(ui.doubleSpinBox_scale_x_mm, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBox_scale_x_mm_valueChanged(double)));
	connect(ui.doubleSpinBox_scale_y_mm, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBox_scale_y_mm_valueChanged(double)));
	connect(ui.doubleSpinBox_scale_z_mm, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBox_scale_z_mm_valueChanged(double)));
	connect(ui.checkBox_uniform_scale, SIGNAL(stateChanged(int)), this, SLOT(checkBox_uniform_scale_stateChanged()));
}

void ScaleDialog::scale(double x, double y, double z)
{
	viewerModule->scaleSelectedModels(x, y, z);
	viewerModule->update();
}

void ScaleDialog::scalemm(double x, double y, double z)
{
	viewerModule->scalemmSelectedModels(x, y, z);
	viewerModule->update();
}

void ScaleDialog::resetScale()
{
	viewerModule->resetScaleSelectedModels();
	viewerModule->update();
}

void ScaleDialog::maxSize()
{
	viewerModule->maxSizeSelectedMeshes();
	viewerModule->update();
}

void ScaleDialog::checkModelSelected()
{
	int selectedCnt = viewerModule->modelContainer->getSelectedModels().size();
	if (selectedCnt == 0)
	{
		Q_FOREACH(QWidget *widget, ui.frame_body->findChildren<QWidget*>())
			widget->setEnabled(false);
		return;
	}

	Q_FOREACH(QWidget *widget, ui.frame_body->findChildren<QWidget*>())
		widget->setEnabled(true);
	if (selectedCnt == 1)
	{
		hasMultiScale = false;
		updateScaleFromVolume();
	}
	if (selectedCnt > 1)
	{
		ui.pushButton_maxsize->setEnabled(false);
		ui.doubleSpinBox_scale_x_mm->setEnabled(false);
		ui.doubleSpinBox_scale_y_mm->setEnabled(false);
		ui.doubleSpinBox_scale_z_mm->setEnabled(false);
		hasMultiScale = !viewerModule->checkScaleRatio();
		if (hasMultiScale)
		{
			scaledFactor = { 1, 1, 1 };
			ui.doubleSpinBox_scale_x->setValue(scaledFactor[0]);
			ui.doubleSpinBox_scale_y->setValue(scaledFactor[1]);
			ui.doubleSpinBox_scale_z->setValue(scaledFactor[2]);
		}
		updateScaleFromVolume();
	}
	setScaleMinMax();
}

void ScaleDialog::setScaleMinMax()
{
	AABB aabb = AABBGetter()(viewerModule->modelContainer->getSelectedModels());
	float length_x = aabb.getLengthX() / scaledFactor[0];
	float length_y = aabb.getLengthY() / scaledFactor[1];
	float length_z = aabb.getLengthZ() / scaledFactor[2];
	AABB machine_aabb = Profile::getMachineAABB();
	float max_ratio = 0;
	float ratio_x = machine_aabb.getLengthX() / length_x;
	float ratio_y = machine_aabb.getLengthY() / length_y;
	float ratio_z = machine_aabb.getLengthZ() / length_z;
	if (max_ratio < ratio_x)
		max_ratio = ratio_x;
	if (max_ratio < ratio_y)
		max_ratio = ratio_y;
	if (max_ratio < ratio_z)
		max_ratio = ratio_z;
	if (max_ratio > 100)
	{
		int temp_ratio = ceil(max_ratio);
		ui.doubleSpinBox_scale_x->setMaximum(temp_ratio);
		ui.doubleSpinBox_scale_y->setMaximum(temp_ratio);
		ui.doubleSpinBox_scale_z->setMaximum(temp_ratio);
	}
	else
	{
		ui.doubleSpinBox_scale_x->setMaximum(100);
		ui.doubleSpinBox_scale_y->setMaximum(100);
		ui.doubleSpinBox_scale_z->setMaximum(100);
	}

	ui.doubleSpinBox_scale_x_mm->setMaximum(length_x * ui.doubleSpinBox_scale_x->maximum());
	ui.doubleSpinBox_scale_x_mm->setMinimum(length_x * ui.doubleSpinBox_scale_x->minimum());
	ui.doubleSpinBox_scale_y_mm->setMaximum(length_y * ui.doubleSpinBox_scale_y->maximum());
	ui.doubleSpinBox_scale_y_mm->setMinimum(length_y * ui.doubleSpinBox_scale_y->minimum());
	ui.doubleSpinBox_scale_z_mm->setMaximum(length_z * ui.doubleSpinBox_scale_z->maximum());
	ui.doubleSpinBox_scale_z_mm->setMinimum(length_z * ui.doubleSpinBox_scale_z->minimum());
}

void ScaleDialog::pushButton_inches_to_millimeters_clicked()
{
	scale(25.4 / scaledFactor[0], 25.4 / scaledFactor[1], 25.4 / scaledFactor[2]);
	updateScaleFromVolume();
}

void ScaleDialog::pushButton_resetscale_clicked()
{
	resetScale();
	updateScaleFromVolume();
}

void ScaleDialog::scale(double value_, int dim_)
{
	bool uniform = ui.checkBox_uniform_scale->isChecked();
	double f = 1;
	double scaleF = value_ / scaledFactor[dim_];
	if (uniform)
		f = scaleF;

	qglviewer::Vec temp((dim_ == 0 ? scaleF : f), (dim_ == 1 ? scaleF : f), (dim_ == 2 ? scaleF : f));
	scale(temp[0], temp[1], temp[2]);
	scaledFactor = qglviewer::Vec(scaledFactor[0] * temp[0], scaledFactor[1] * temp[1], scaledFactor[2] * temp[2]);
	updateScaleFromVolume();
}
void ScaleDialog::doubleSpinBox_scale_x_valueChanged(double val)
{
	if (noChange) return;
	scale(val, 0);
}

void ScaleDialog::doubleSpinBox_scale_y_valueChanged(double val)
{
	if (noChange) return;
	scale(val, 1);
}

void ScaleDialog::doubleSpinBox_scale_z_valueChanged(double val)
{
	if (noChange) return;
	scale(val, 2);
}

void ScaleDialog::doubleSpinBox_scale_x_mm_valueChanged(double val)
{
	if (noChange) return;
	bool uniform = ui.checkBox_uniform_scale->isChecked();
	double f = 1;
	if (uniform)
		f = val / aabbLength[0];

	qglviewer::Vec temp(val / aabbLength[0], f, f);
	scale(temp[0], temp[1], temp[2]);
	updateScaleFromVolume();
}

void ScaleDialog::doubleSpinBox_scale_y_mm_valueChanged(double val)
{
	if (noChange) return;
	bool uniform = ui.checkBox_uniform_scale->isChecked();
	double f = 1;
	if (uniform)
		f = val / aabbLength[1];

	qglviewer::Vec temp(f, val / aabbLength[1], f);
	scale(temp[0], temp[1], temp[2]);
	updateScaleFromVolume();
}

void ScaleDialog::doubleSpinBox_scale_z_mm_valueChanged(double val)
{
	if (noChange) return;
	bool uniform = ui.checkBox_uniform_scale->isChecked();
	double f = 1;
	if (uniform)
		f = val / aabbLength[2];

	qglviewer::Vec temp(f, f, val / aabbLength[2]);
	scale(temp[0], temp[1], temp[2]);
	updateScaleFromVolume();
}

void ScaleDialog::checkBox_uniform_scale_stateChanged()
{
	return;
	if (ui.checkBox_uniform_scale->isChecked())
	{
		noChange = true;
		double temp = 0;
		if (ui.doubleSpinBox_scale_x->value() > temp)
			temp = ui.doubleSpinBox_scale_x->value();
		if (ui.doubleSpinBox_scale_y->value() > temp)
			temp = ui.doubleSpinBox_scale_y->value();
		if (ui.doubleSpinBox_scale_z->value() > temp)
			temp = ui.doubleSpinBox_scale_z->value();

		ui.doubleSpinBox_scale_x->setValue(temp);
		ui.doubleSpinBox_scale_y->setValue(temp);
		ui.doubleSpinBox_scale_z->setValue(temp);

		scale(temp,
			temp,
			temp);
		noChange = false;
	}
}

void ScaleDialog::pushButton_maxsize_clicked()
{
	maxSize();
	updateScaleFromVolume();
}





void ScaleDialog::updateScaleFromVolume()
{
	noChange = true;
	AABB aabb = AABBGetter()(viewerModule->modelContainer->getSelectedModels());
	aabbLength = qglviewer::Vec(aabb.getLengthX(), aabb.getLengthY(), aabb.getLengthZ());
	//계산 중 소수자리가 달라져서 서로 다른 값으로 인식되는 것을 맞기위해 반올림하여 비교
	if (round(aabbLength[0] * 100) != round(ui.doubleSpinBox_scale_x_mm->value() * 100))
		ui.doubleSpinBox_scale_x_mm->setValue(aabbLength[0]);
	if (round(aabbLength[1] * 100) != round(ui.doubleSpinBox_scale_y_mm->value() * 100))
		ui.doubleSpinBox_scale_y_mm->setValue(aabbLength[1]);
	if (round(aabbLength[2] * 100) != round(ui.doubleSpinBox_scale_z_mm->value() * 100))
		ui.doubleSpinBox_scale_z_mm->setValue(aabbLength[2]);
	
	if (!hasMultiScale)
		scaledFactor = viewerModule->getScaleSelectedModels();

	if (round(scaledFactor[0] * 100) != round(ui.doubleSpinBox_scale_x->value() * 100))
		ui.doubleSpinBox_scale_x->setValue(scaledFactor[0]);
	if (round(scaledFactor[1] * 100) != round(ui.doubleSpinBox_scale_y->value() * 100))
		ui.doubleSpinBox_scale_y->setValue(scaledFactor[1]);
	if (round(scaledFactor[2] * 100) != round(ui.doubleSpinBox_scale_z->value() * 100))
		ui.doubleSpinBox_scale_z->setValue(scaledFactor[2]);

	noChange = false;
}
