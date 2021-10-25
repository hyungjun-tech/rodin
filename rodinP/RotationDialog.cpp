#include "stdafx.h"
#include "RotationDialog.h"
#include "ViewerModule.h"

RotationDialog::RotationDialog(QWidget* _parent)
	:QDialog(_parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	this->setAttribute(Qt::WA_DeleteOnClose);
}

RotationDialog::~RotationDialog()
{
	if (Generals::isRotateUIMode())
	{
		viewerModule->setTranslateUIMode();
		viewerModule->update();
	}
}
void RotationDialog::setContents(ViewerModule* const _viewerModule)
{
	viewerModule = _viewerModule;
	checkModelSelected();

	connect(viewerModule->modelContainer, SIGNAL(signal_modelSelect()), this, SLOT(checkModelSelected()));
	connect(viewerModule->modelContainer, SIGNAL(signal_modelDeselect()), this, SLOT(checkModelSelected()));
	connect(viewerModule->modelContainer, SIGNAL(signal_modelDeleted()), this, SLOT(checkModelSelected()));

	connect(ui.pushButton_apply_rot, SIGNAL(clicked()), this, SLOT(pushButton_apply_rot_clicked()));
	connect(ui.pushButton_resetori, SIGNAL(clicked()), this, SLOT(pushButton_resetori_clicked()));
	connect(ui.pushButton_layFlat, SIGNAL(clicked()), this, SLOT(pushButton_layFlat_clicked()));
	connect(ui.pushButton_axisXplus90, SIGNAL(clicked()), this, SLOT(pushButton_axisXplus90_clicked()));
	connect(ui.pushButton_axisXminus90, SIGNAL(clicked()), this, SLOT(pushButton_axisXminus90_clicked()));
	connect(ui.pushButton_axisYplus90, SIGNAL(clicked()), this, SLOT(pushButton_axisYplus90_clicked()));
	connect(ui.pushButton_axisYminus90, SIGNAL(clicked()), this, SLOT(pushButton_axisYminus90_clicked()));
	connect(ui.pushButton_axisZplus90, SIGNAL(clicked()), this, SLOT(pushButton_axisZplus90_clicked()));
	connect(ui.pushButton_axisZminus90, SIGNAL(clicked()), this, SLOT(pushButton_axisZminus90_clicked()));

	viewerModule->setRotateUIMode();
	viewerModule->update();
}

void RotationDialog::checkModelSelected()
{
	int selectedCnt = viewerModule->modelContainer->getSelectedModels().size();
	if (selectedCnt == 0)
	{
		Q_FOREACH(QWidget *widget, ui.body->findChildren<QWidget*>())
			widget->setEnabled(false);
	}
	else
	{
		Q_FOREACH(QWidget *widget, ui.body->findChildren<QWidget*>())
			widget->setEnabled(true);
	}
}

void RotationDialog::rotate(double rad_x, double rad_y, double rad_z)
{
	viewerModule->rotateSelectedModels(rad_x, rad_y, rad_z);
	viewerModule->update();
}
void RotationDialog::pushButton_apply_rot_clicked()
{
	float x_rad = ui.doubleSpinBox_rot_x->value() * M_PI / 180.0;		// radian으로 미리 변경		// ijeong 2019 08
	float y_rad = ui.doubleSpinBox_rot_y->value() * M_PI / 180.0;
	float z_rad = ui.doubleSpinBox_rot_z->value() * M_PI / 180.0;
	//rotate(mEngine, ui.doubleSpinBox_rot_x->value(), ui.doubleSpinBox_rot_y->value(), ui.doubleSpinBox_rot_z->value());
	rotate(x_rad, y_rad, z_rad);
	ui.doubleSpinBox_rot_x->setValue(0);
	ui.doubleSpinBox_rot_y->setValue(0);
	ui.doubleSpinBox_rot_z->setValue(0);
}
void RotationDialog::pushButton_resetori_clicked()
{
	viewerModule->resetRotateSelectedModels();
	viewerModule->update();
}
void RotationDialog::pushButton_layFlat_clicked()
{
	viewerModule->layFlat();
	viewerModule->update();

}
void RotationDialog::pushButton_axisXplus90_clicked()
{
	float x_rad = 90 * M_PI / 180.0;
	rotate(x_rad, 0, 0);
}
void RotationDialog::pushButton_axisXminus90_clicked()
{
	float x_rad = -90 * M_PI / 180.0;
	rotate(x_rad, 0, 0);
}
void RotationDialog::pushButton_axisYplus90_clicked()
{
	float y_rad = 90 * M_PI / 180.0;
	rotate(0, y_rad, 0);
}
void RotationDialog::pushButton_axisYminus90_clicked()
{
	float y_rad = -90 * M_PI / 180.0;
	rotate(0, y_rad, 0);
}
void RotationDialog::pushButton_axisZplus90_clicked()
{
	float z_rad = 90 * M_PI / 180.0;
	rotate(0, 0, z_rad);
}
void RotationDialog::pushButton_axisZminus90_clicked()
{
	float z_rad = -90 * M_PI / 180.0;
	rotate(0, 0, z_rad);
}