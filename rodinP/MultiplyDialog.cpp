#include "stdafx.h"
#include "MultiplyDialog.h"
#include "ViewerModule.h"

MultiplyDialog::MultiplyDialog(ViewerModule* viewerModule_)
	: QDialog(viewerModule_)
	, viewerModule(viewerModule_)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	//setAttribute(Qt::WA_DeleteOnClose, true);
	connect(ui.pushButton_ok, SIGNAL(clicked(bool)), this, SLOT(pushButton_ok_clicked()));
}

MultiplyDialog::~MultiplyDialog()
{

}

void MultiplyDialog::pushButton_ok_clicked()
{
	int numofCopies = ui.spinBox_num->value();
	viewerModule->multiplySelectedModels(numofCopies);

	viewerModule->update();
	this->close();
}