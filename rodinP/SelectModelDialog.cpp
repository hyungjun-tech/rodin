#include "stdafx.h"
#include "SelectModelDialog.h"

SelectModelDialog::SelectModelDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	this->setModal(true);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	connect(ui.btn_OK, SIGNAL(clicked()), this, SLOT(clickedOK()));
	connect(ui.btn_Cancel , SIGNAL(clicked()), this, SLOT(clickedCancel()));
	connect(ui.comboBox_machineList, SIGNAL(currentIndexChanged(int)), this, SLOT(machineChanged(int)));

	QDir d = QDir(Generals::appPath + "\\profile");
	QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	//하위에 폴더가 하나만 있는 경우 해당 모델로 지정하고 Skip
	if (dirs.size() == 1)
	{
		QSettings props(AppInfo::getCompanyName(), AppInfo::getAppName());
		props.setValue("model", dirs.at(0));
		props.sync();
		return;
	}

	ui.comboBox_machineList->addItem("");
	for (int i = 0; i < dirs.size(); i++)
	{
		ui.comboBox_machineList->addItem(dirs.at(i));
	}
	ui.comboBox_machineList->setCurrentIndex(-1);
	ui.btn_OK->setEnabled(false);
	exec();
}

SelectModelDialog::~SelectModelDialog()
{

}

void SelectModelDialog::clickedOK()
{
	QSettings props(AppInfo::getCompanyName(), AppInfo::getAppName());
	props.setValue("model", ui.comboBox_machineList->itemText(ui.comboBox_machineList->currentIndex()));
	props.sync();
	close();
}
void SelectModelDialog::clickedCancel()
{
	cancelFlag = true;
	close();
}

void SelectModelDialog::machineChanged(int index)
{
	QString machineModel = ui.comboBox_machineList->itemText(index);
	if (machineModel == "") ui.btn_OK->setEnabled(false);
	else ui.btn_OK->setEnabled(true);
}


void SelectModelDialog::mousePressEvent(QMouseEvent *event) {
	m_nMouseClick_X_Coordinate = event->x();
	m_nMouseClick_Y_Coordinate = event->y();
}

void SelectModelDialog::mouseMoveEvent(QMouseEvent *event) {
	int x = event->globalX() - m_nMouseClick_X_Coordinate;
	int y = event->globalY() - m_nMouseClick_Y_Coordinate;
	int checkX = event->x() - m_nMouseClick_X_Coordinate;
	int checkY = event->y() - m_nMouseClick_Y_Coordinate;

	if (checkX > 500 || checkX < -500) NULL;
	else if (checkY > 500 || checkY < -500) NULL;
	else move(x, y);
}
