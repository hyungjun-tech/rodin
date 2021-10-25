#include "stdafx.h"
#include "SelectCartridgePopDialog.h"
#include "CartridgeInfo.h"
#include "ProfileControl.h"

SelectCartridgePopDialog::SelectCartridgePopDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);
	
	QFont qfont;
	qfont.setFamily(QStringLiteral("Malgun Gothic"));
	qfont.setPointSize(9);
	ui.comboBox_cartridge_selection->setFont(qfont);
}

SelectCartridgePopDialog::~SelectCartridgePopDialog()
{

}

void SelectCartridgePopDialog::showImportDialog()
{
	setWindowTitle(tr("Import Profile"));
	for (int i = 0; i < CartridgeInfo::cartridges.size(); i++)
	{
		QString tempItem;
		tempItem.clear();
		tempItem.append(CustomTranslate::tr("Cartridge"));
		tempItem.append(QString(" (%1)").arg(i + 1));

		ui.comboBox_cartridge_selection->addItem(tempItem);
	}
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(pushButton_ok_clicked_import()));
	this->show();
}

void SelectCartridgePopDialog::showExportDialog()
{
	setWindowTitle(tr("Export Profile"));
	for (int i = 0; i < CartridgeInfo::cartridges.size(); i++)
	{
		QString tempItem;
		tempItem.clear();
		tempItem.append(CustomTranslate::tr("Cartridge"));
		tempItem.append(QString(" (%1)").arg(i + 1));

		ui.comboBox_cartridge_selection->addItem(tempItem);

	}
	ui.comboBox_cartridge_selection->addItem(tr("All Cartridge"));
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(pushButton_ok_clicked_export()));
	this->show();
}

void SelectCartridgePopDialog::pushButton_ok_clicked_import()
{
	hide();
	importProfile(ui.comboBox_cartridge_selection->currentIndex());
	close();
}

void SelectCartridgePopDialog::pushButton_ok_clicked_export()
{
	hide();
	exportProfile(ui.comboBox_cartridge_selection->currentIndex());
	close();
}

void SelectCartridgePopDialog::importProfile(int profileSelectedIndex)
{
	qDebug() << "profileSelectedIndex : " << profileSelectedIndex;

	char szFilter[] = "Profile Information File (*.ini)";
	QString importProfileName = QFileDialog::getOpenFileName(this->parentWidget(), tr("Import profile"), "", szFilter);

	if (!(importProfileName.size())) return;

	if (!ProfileControl::importProfile(importProfileName, profileSelectedIndex))
	{
		CommonDialog comDlg(this, MessageAlert::tr("profile_not_supported"), CommonDialog::Warning);
		return;
	}

	emit signal_profileChanged();

	CommonDialog comDlg(this, MessageAlert::tr("success_import_profile"), CommonDialog::Information);
	_wremove(Generals::qstringTowchar_t(Profile::profilePath + "temp_profile_setting_value.ini"));
}

void SelectCartridgePopDialog::exportProfile(int profileSelectedIndex)
{
	char szFilter[] = "Profile Information File (*.ini)";
	QString exportProfileName = QFileDialog::getSaveFileName(this->parentWidget(), tr("Export profile"), "", szFilter);

	if (!(exportProfileName.size())) return;

	ProfileControl::exportProfile(exportProfileName, profileSelectedIndex);

	CommonDialog comDlg(this, MessageAlert::tr("success_export_profile"), CommonDialog::Information);
}