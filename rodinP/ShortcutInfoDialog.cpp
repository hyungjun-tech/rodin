#include "stdafx.h"
#include "ShortcutInfoDialog.h"

ShortcutInfoDialog::ShortcutInfoDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));

	//Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
	//this->setWindowFlags(windowFlags() & (~helpFlag));

	ui.tabWidget->setCurrentIndex(0);
	exec();
}


ShortcutInfoDialog::~ShortcutInfoDialog()
{

}

void ShortcutInfoDialog::dialogClose()
{
	this->close();
}