#include "stdafx.h"
#include "ProfileSettingMachineDlg.h"
#include "AuthentificationDlg.h"
#include "PrinterInfo.h"
#include "UserProperties.h"
#include "ProfileControl.h"

ProfileSettingMachineDlg::ProfileSettingMachineDlg(QWidget *parent)
	: QDialog(parent)
	, b_enabledApplyButton(false)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);

	m_machine_profile = Profile::machineProfile;

	setUI();
	setConnection();
	setInitValue();
	loadSettingValue();

	//loadSettingValue();

	//모든 체크박스 변경시 apply 활성화
	setConnectControlforApplyButton();
	setDisabledApplyButton();
	connect(this, SIGNAL(signal_componentChanged()), this, SLOT(setEnabledApplyButton()));
}

ProfileSettingMachineDlg::~ProfileSettingMachineDlg()
{

}

void ProfileSettingMachineDlg::pushButton_apply_clicked()
{
	saveSettingValue();
	setDisabledApplyButton();
}

void ProfileSettingMachineDlg::pushButton_ok_clicked()
{
	if (saveSettingValue())
		this->close();
}

void ProfileSettingMachineDlg::setUI()
{
	//machine list setting..//
	QDir d = QDir(Generals::appPath + "\\profile");
	QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < dirs.size(); i++)
	{
		ui.comboBox_machineList->addItem(dirs.at(i));
	}

	//현재 machine Profile의 bed side는 사용하지 않음.
	ui.label_bedSide->setVisible(false);
	ui.comboBox_BedSide->setVisible(false);
	//ui.label_bedSide->setEnabled(m_MachineProfile->machine_bed_selected_enabled.value);
	//ui.comboBox_BedSide->setEnabled(m_MachineProfile->machine_bed_selected_enabled.value);
	//무조건 disable
	ui.checkBox_has_heated_bed->setEnabled(false);
	ui.label_6->setEnabled(false);

	int idx = ui.comboBox_machineList->findText(Profile::machineProfile.machine_model);
	if (idx != -1)
		ui.comboBox_machineList->setCurrentIndex(idx);

	//not use original value for nozzle size//
	ui.doubleSpinBox_nozzle_size_T0->setUsingOriginalValue(false);
	ui.doubleSpinBox_nozzle_size_T1->setUsingOriginalValue(false);

	ui.doubleSpinBox_nozzle_size_T0->setReadOnly(true);
	ui.doubleSpinBox_nozzle_size_T1->setReadOnly(true);

	ui.doubleSpinBox_nozzle_size_T0->setButtonSymbols(QAbstractSpinBox::NoButtons);
	ui.doubleSpinBox_nozzle_size_T1->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

void ProfileSettingMachineDlg::setConnection()
{
	connect(ui.pushButton_apply, SIGNAL(clicked()), this, SLOT(pushButton_apply_clicked()));
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(pushButton_ok_clicked()));
	connect(ui.checkBox_machine_center_is_zero, SIGNAL(stateChanged(int)), this, SLOT(checkBox_machine_center_is_zero_stateChanged(int)));
	connect(ui.groupBox_expanded_print_mode, SIGNAL(toggled(bool)), this, SLOT(groupBox_expanded_print_mode_toggled(bool)));
	connect(ui.groupBox_authentification_mode, SIGNAL(toggled(bool)), this, SLOT(groupBox_authentification_mode_toggled(bool)));
	connect(ui.comboBox_machineList, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_machineList_currentIndexChanged(int)));
	connect(ui.comboBox_authentification_setting_method, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_authentificationMethod_currentIndexChanged(int)));
	connect(ui.pushButton_authentification_info, SIGNAL(clicked()), this, SLOT(pushButton_authentification_info_clicked()));
}
void ProfileSettingMachineDlg::setConnectControlforApplyButton()
{
	//checkBox//
	Q_FOREACH(QCheckBox *checkBoxs, findChildren<QCheckBox*>())
	{
		connect(checkBoxs, SIGNAL(stateChanged(int)), this, SIGNAL(signal_componentChanged()));
	}
	//comboBox//
	Q_FOREACH(QComboBox *comboBoxs, findChildren<QComboBox*>())
	{
		connect(comboBoxs, SIGNAL(currentIndexChanged(int)), this, SIGNAL(signal_componentChanged()));
	}
	//DoubleSpinBox//
	Q_FOREACH(QDoubleSpinBox *doubleSpinBoxs, findChildren<QDoubleSpinBox*>())
	{
		connect(doubleSpinBoxs, SIGNAL(valueChanged(double)), this, SIGNAL(signal_componentChanged()));
	}
	//SpinBox//
	Q_FOREACH(QSpinBox *spinBoxs, findChildren<QSpinBox*>())
	{
		connect(spinBoxs, SIGNAL(valueChanged(int)), this, SIGNAL(signal_componentChanged()));
	}
	//RadioButton//
	Q_FOREACH(QRadioButton *radioButton, findChildren<QRadioButton*>())
	{
		connect(radioButton, SIGNAL(clicked()), this, SIGNAL(signal_componentChanged()));
	}
	//LineEdit//
	Q_FOREACH(QLineEdit *lineEdits, findChildren<QLineEdit*>())
	{
		connect(lineEdits, SIGNAL(textChanged(QString)), this, SIGNAL(signal_componentChanged()));
	}
}

void ProfileSettingMachineDlg::setInitValue()
{
	m_emailAddress = Generals::getProps("emailAddress");
	m_emailCount = Generals::getProps("emailCount").toInt();
	m_cartridgeList.clear();

	ui.lineEdit_email_address->setText(m_emailAddress);
	ui.comboBox_email_count->setCurrentIndex(m_emailCount);

	ui.groupBox_authentification_mode->setChecked(UserProperties::authentification_print_mode);
	if (UserProperties::authentification_print_mode)
		ui.comboBox_authentification_setting_method->setCurrentIndex(UserProperties::authentification_print_setting_method);
	resetCartridgeList();
}

void ProfileSettingMachineDlg::loadSettingValue()
{
	//set web camera
	bool hasWebCamera = m_machine_profile.has_web_camera.value;
	if (!hasWebCamera)
	{
		ui.lineEdit_email_address->clear();
		ui.comboBox_email_count->setCurrentIndex(0);
	}
	ui.label_email_address->setEnabled(hasWebCamera);
	ui.lineEdit_email_address->setEnabled(hasWebCamera);
	ui.label_email_count->setEnabled(hasWebCamera);
	ui.comboBox_email_count->setEnabled(hasWebCamera);

	//set value from temp profile
	ui.lineEdit_extruder_count->setText(QString::number(m_machine_profile.extruder_count.value));

	ui.groupBox_expanded_print_mode->setEnabled(m_machine_profile.machine_expanded_print_function_enabled.value);
	ui.groupBox_expanded_print_mode->setChecked(m_machine_profile.machine_expanded_print_mode.value);
	if (m_machine_profile.machine_expanded_print_mode.value)
		ui.comboBox_expanded_print_cartridge_index->setCurrentIndex(m_machine_profile.machine_expanded_print_cartridgeIndex.value);
	else
		setBedSize();

	ui.checkBox_has_heated_bed->setChecked(m_machine_profile.has_heated_bed.value);
	ui.checkBox_machine_center_is_zero->setChecked(m_machine_profile.machine_center_is_zero.value);

	//nozzle size..//
	ui.doubleSpinBox_nozzle_size_T0->setValue(m_machine_profile.nozzle_size.at(0).value);
	if (m_machine_profile.extruder_count.value > 1)
	{
		ui.label_nozzle_size_T1->setEnabled(true);
		ui.doubleSpinBox_nozzle_size_T1->setEnabled(true);
		ui.doubleSpinBox_nozzle_size_T1->setValue(m_machine_profile.nozzle_size.at(1).value);
	}
	else
	{
		ui.label_nozzle_size_T1->setEnabled(false);
		ui.doubleSpinBox_nozzle_size_T1->setEnabled(false);
		ui.doubleSpinBox_nozzle_size_T1->clear();
	}
}

bool ProfileSettingMachineDlg::saveSettingValue()
{
	if (!b_enabledApplyButton)
		return true;

	QString emailAddress = ui.lineEdit_email_address->text();
	m_emailCount = ui.comboBox_email_count->currentIndex();

	if (!validateEmailString(emailAddress))
		return false;

	m_emailAddress = emailAddress;

	ui.doubleSpinBox_nozzle_size_T0->getProfileValue(&(m_machine_profile.nozzle_size.at(0)));
	if (m_machine_profile.extruder_count.value > 1)
		ui.doubleSpinBox_nozzle_size_T1->getProfileValue(&(m_machine_profile.nozzle_size.at(1)));

	//////////////////////////////////////////////////////////////////////////////////
	//check expanded print mode//
	m_machine_profile.machine_expanded_print_cartridgeIndex.value = ui.comboBox_expanded_print_cartridge_index->currentIndex();

	bool isExpandedChanged = false;
	if (m_machine_profile.machine_expanded_print_mode.value != Profile::machineProfile.machine_expanded_print_mode.value)
		isExpandedChanged = true;
	else if (m_machine_profile.machine_expanded_print_mode.value &&	m_machine_profile.machine_expanded_print_cartridgeIndex.value != Profile::machineProfile.machine_expanded_print_cartridgeIndex.value)
		isExpandedChanged = true;

	//////////////////////////////////////////////////////////////////////////////////
	//authentification setting//
	UserProperties::authentification_print_mode = ui.groupBox_authentification_mode->isChecked();
	if (UserProperties::authentification_print_mode)
		UserProperties::authentification_print_setting_method = ui.comboBox_authentification_setting_method->currentIndex();
	if (!UserProperties::authentification_print_mode
		|| UserProperties::authentification_print_setting_method != Generals::AuthentificationMethod::SaveToPC)
	{
		Generals::setProps("id", "");
		Generals::setProps("password", "");
	}

	Generals::setProps("emailAddress", m_emailAddress);
	Generals::setProps("emailCount", QString::number(m_emailCount));


	//////////////////////////////////////////////////////////////////////////////////
	//backup : old machine profile//
	MachineProfile old_machine_profile = Profile::machineProfile;

	//sync : local machine profile -> global machine profile//
	Profile::machineProfile = m_machine_profile;
	//////////////////////////////////////////////////////////////////////////////////


	//sync 다음에 위치해야 함 -> Profile::machineProfile을 이용하기 때문..//
	if (isExpandedChanged)
		emit signal_expandedPrintModeChanged();


	//////////////////////////////////////////////////////////////////////////////////
	QString new_machine_model = ui.comboBox_machineList->currentText();

	if (PrinterInfo::currentModel != new_machine_model)
	{
		//saving old_machine_profile to recent machine profile//
		ProfileControl::saveRecentMachineProfile(old_machine_profile);

		PrinterInfo::clear(new_machine_model);
		emit signal_changeMachine(new_machine_model, true);
	}
	else
	{
		emit signal_profileChanged(b_enabledApplyButton);
	}

	return true;
}
//
void ProfileSettingMachineDlg::checkBox_machine_center_is_zero_stateChanged(int state)
{
	if (state)
	{
		CommonDialog comDlg(this, MessageAlert::tr("machine_center_is_zero_alert"), CommonDialog::Warning);
	}
}

bool ProfileSettingMachineDlg::validateEmailString(QString _email)
{
	bool b_validation = true;
	bool b_space = true;

	if (_email.isEmpty())
		return true;
	else
	{
		for (auto it = _email.begin(); it != _email.end(); it++)
		{
			if (*it != ' ')
			{
				b_space = false;
				break;
			}
		}
		if (b_space)
			return true;
	}

	//string에 문자가 있을 경우..
	QStringList emailList = _email.split(";");
	for (int i = 0; i < emailList.size(); i++)
	{
		if (!validateEmailSingleAddress(emailList.at(i)))
			b_validation = false;
	}

	if (!b_validation)
		CommonDialog comDlg(this, MessageAlert::tr("email_address_not_valid"), CommonDialog::Warning);
	else if (m_emailAddress != _email)
	{
		CommonDialog comDlg(this);
		//comDlg.setFixedSize(500, 180);
		comDlg.setDialogContents(MessageAlert::tr("email_setting_alert"), CommonDialog::Warning);
		comDlg.exec();
	}
	return b_validation;
}

bool ProfileSettingMachineDlg::validateEmailSingleAddress(QString email)
{
	QRegularExpression regex("^[0-9a-zA-Z]+([0-9a-zA-Z]*[-._+])*[0-9a-zA-Z]+@[0-9a-zA-Z]+([-.][0-9a-zA-Z]+)*([0-9a-zA-Z]*[.])[a-zA-Z]{2,6}$");

	if (!regex.match(email).hasMatch())
	{
		qDebug() << "single invalid";
		return false;
	}
	else
	{
		qDebug() << "single OK";
		return true;
	}
}

void ProfileSettingMachineDlg::comboBox_machineList_currentIndexChanged(int idx)
{
	QString machineModel = ui.comboBox_machineList->itemText(idx);

	QString l_profilePath = Generals::appPath + "\\profile\\" + machineModel + "\\";
	//machine profile만 temp에 로드하여 화면에 set
	if (!m_machine_profile.loadMachineProfile(Generals::qstringTowchar_t(l_profilePath + QString("machine_setting_value.ini"))))
	{
		CommonDialog comDlg(this, MessageAlert::tr("fail_to_load_machine_profile"), CommonDialog::Warning);
	}
	//expanded ui reset//
	resetCartridgeList();
	loadSettingValue();
}

void ProfileSettingMachineDlg::resetCartridgeList()
{
	m_cartridgeList.clear();
	for (int i = 0; i < m_machine_profile.extruder_count.value; i++)
	{
		m_cartridgeList.push_back(CustomTranslate::tr("Cartridge") + QString("(%1)").arg(i + 1));
	}
}

void ProfileSettingMachineDlg::comboBox_authentificationMethod_currentIndexChanged(int index)
{
	//index == 0 --> every time..//
	//index == 1 --> save to PC..//
	if (index == 1)
	{
		ui.label_authentification_info->setEnabled(true);
		ui.pushButton_authentification_info->setEnabled(true);
	}
	else
	{
		ui.label_authentification_info->setEnabled(false);
		ui.pushButton_authentification_info->setEnabled(false);
	}
}

void ProfileSettingMachineDlg::groupBox_authentification_mode_toggled(bool state)
{
	ui.comboBox_authentification_setting_method->clear();

	ui.label_authentification_setting_method->setEnabled(state);
	ui.comboBox_authentification_setting_method->setEnabled(state);

	if (state)
	{
		ui.comboBox_authentification_setting_method->addItem(tr("Every Time You Print"));
		ui.comboBox_authentification_setting_method->addItem(tr("Save To This PC"));
		ui.comboBox_authentification_setting_method->setCurrentIndex(0);
	}
	else
	{
		ui.comboBox_authentification_setting_method->clear();
		//comboBox_authentificationMethod_changed(state);
	}
}


void ProfileSettingMachineDlg::groupBox_expanded_print_mode_toggled(bool state)
{
	ui.label_expanded_print_cartridge_index->setEnabled(state);
	ui.comboBox_expanded_print_cartridge_index->setEnabled(state);

	//expandedPrintMode에 따라 combobox에 넣는 것을 구분하여 settting..//
	ui.comboBox_expanded_print_cartridge_index->clear();
	if (state)
	{
		ui.comboBox_expanded_print_cartridge_index->addItems(m_cartridgeList);
		ui.comboBox_expanded_print_cartridge_index->setCurrentIndex(0);
	}

	m_machine_profile.machine_expanded_print_mode.value = state;
	setBedSize();
}

void ProfileSettingMachineDlg::setModelEnabled(bool a)
{
	ui.comboBox_machineList->setEnabled(a);
}

void ProfileSettingMachineDlg::setEnabledApplyButton()
{
	b_enabledApplyButton = true;

	ui.pushButton_apply->setEnabled(true);
}

void ProfileSettingMachineDlg::setDisabledApplyButton()
{
	b_enabledApplyButton = false;

	ui.pushButton_apply->setEnabled(false);
}

void ProfileSettingMachineDlg::pushButton_authentification_info_clicked()
{
	AuthentificationDlg *authDlg = new AuthentificationDlg(Generals::AuthentificationMethod::SaveToPC, this);
	authDlg->show();
}

void ProfileSettingMachineDlg::setBedSize()
{
	//reset UI machine dialog//
	unsigned int machine_width_temp = m_machine_profile.machine_width_default.value;
	unsigned int machine_depth_temp = m_machine_profile.machine_depth_default.value;
	unsigned int machine_height_temp = m_machine_profile.machine_height_default.value;
	ui.lineEdit_machine_width->setStyleSheet("");
	if (m_machine_profile.machine_expanded_print_mode.value)
	{
		machine_width_temp += m_machine_profile.machine_expanded_width_offset.value;
		machine_depth_temp += m_machine_profile.machine_expanded_depth_offset.value;
		machine_height_temp += m_machine_profile.machine_expanded_height_offset.value;

		ui.lineEdit_machine_width->setStyleSheet("background-color: rgb(255, 170, 80);");
	}

	ui.lineEdit_machine_width->setText(QString::number(machine_width_temp));
	ui.lineEdit_machine_depth->setText(QString::number(machine_depth_temp));
	ui.lineEdit_machine_height->setText(QString::number(machine_height_temp));

}