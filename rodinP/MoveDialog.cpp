#include "stdafx.h"
#include "MoveDialog.h"
#include "ViewerModule.h"

MoveDialog::MoveDialog(QWidget* _parent)
	: QDialog(_parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose);
}

MoveDialog::~MoveDialog()
{

}
void MoveDialog::setContents(ViewerModule* const _viewerModule)
{
	viewerModule = _viewerModule;
	checkModelSelected();

	connect(viewerModule->modelContainer, SIGNAL(signal_modelSelect()), this, SLOT(checkModelSelected()));
	connect(viewerModule->modelContainer, SIGNAL(signal_modelDeselect()), this, SLOT(checkModelSelected()));
	connect(viewerModule->modelContainer, SIGNAL(signal_modelDeleted()), this, SLOT(checkModelSelected()));

	connect(ui.pushButton_apply_trans, SIGNAL(clicked(bool)), this, SLOT(pushButton_apply_trans_clicked()));
	connect(ui.pushButton_bedcenter, SIGNAL(clicked(bool)), this, SLOT(pushButton_bedcenter_clicked()));
	connect(ui.pushButton_resetpos, SIGNAL(clicked(bool)), this, SLOT(pushButton_resetpos_clicked()));
}


void MoveDialog::checkModelSelected()
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

		if (viewerModule->modelContainer->isMultiSelected())
		{
			ui.pushButton_bedcenter->setEnabled(false);
			ui.pushButton_resetpos->setEnabled(false);
		}
		else
		{
			ui.pushButton_bedcenter->setEnabled(true);
			ui.pushButton_resetpos->setEnabled(true);
		}
	}
}
void MoveDialog::pushButton_apply_trans_clicked()
{
	double x = ui.doubleSpinBox_trans_x->value();
	double y = ui.doubleSpinBox_trans_y->value();

	viewerModule->translateSelectedModels(x, y, 0);
	viewerModule->updateGL();

	ui.doubleSpinBox_trans_x->setValue(0);
	ui.doubleSpinBox_trans_y->setValue(0);
}
void MoveDialog::pushButton_bedcenter_clicked()
{
	viewerModule->placeSelectedModelsToBedCenter();
	viewerModule->updateGL();
}
void MoveDialog::pushButton_resetpos_clicked()
{
	viewerModule->resetTranslateSelectedModels();
	viewerModule->updateGL();
}