#include "stdafx.h"

#include "ProfileSettingEasymodeDlg.h"
#include "ProfileControl.h"

ProfileSettingEasymodeDlg::ProfileSettingEasymodeDlg(QWidget *parent, QString& easyModeProp_)
	: QDialog(parent)
	, b_enabledApplyButton(false)
	, easyModeProp(easyModeProp_)
{
	ui.setupUi(this);

	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);

	m_sliceProfile_multi = &Profile::sliceProfile;
	m_sliceProfileForCommon = &Profile::sliceProfileCommon;

	setUI();
	loadSettingValue();
	setDisabledApplyButton();

	connect(ui.pushButton_apply, SIGNAL(clicked()), this, SLOT(pushButton_apply_clicked(void)));
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(pushButton_ok_clicked(void)));

	connect(ui.comboBox_support_cartridge_index, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_support_cartridge_index_currentIndexChanged(int)));
	//connect(ui.comboBox_material_cartridge_1, SIGNAL(currentIndexChanged(int)), this, SLOT(setMaterialIndex_cart(int)));
	//connect(ui.comboBox_material_cartridge_2, SIGNAL(currentIndexChanged(int)), this, SLOT(setMaterialIndex_cart(int)));
	connect(ui.comboBox_material_cartridge_1, SIGNAL(currentTextChanged(QString)), this, SLOT(comboBox_material_cartridge_1_currentTextChanged(QString)));
	connect(ui.comboBox_material_cartridge_2, SIGNAL(currentTextChanged(QString)), this, SLOT(comboBox_material_cartridge_2_currentTextChanged(QString)));

	connect(ui.checkBox_wipetower_enable, SIGNAL(toggled(bool)), this, SLOT(checkBox_wipetower_enable_toggled(bool)));
	connect(ui.comboBox_wipetower_outer_cartridge_index, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_wipeToewer_cartridgeIndex_currentIndexChanged(int)));
	connect(ui.comboBox_wipetower_inner_cartridge_index, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_wipeToewer_cartridgeIndex_currentIndexChanged(int)));

	connect(ui.comboBox_support, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_support_currentIndexChanged(int)));

	connect(this, SIGNAL(signal_componentChanged()), this, SLOT(setEnabledApplyButton()));
}

ProfileSettingEasymodeDlg::~ProfileSettingEasymodeDlg()
{

}

void ProfileSettingEasymodeDlg::setUI()
{
	qDebug() << "void ProfileSettingEasymodeDlg::setUI()";;
	this->resetCartridgeUI(Profile::machineProfile.extruder_count.value);
	this->resetMaterialUI(Profile::machineProfile.extruder_count.value);
	this->setUIcontrolByMachineModel();

	this->setConnectControlforApplyButton();

	if (Generals::isLayerColorModeOn())
	{
		ui.checkBox_wipetower_enable->setEnabled(false);
		ui.label_wipetower_enable->setEnabled(false);

		ui.comboBox_support_cartridge_index->setVisible(false);
		ui.label_support_cartridge_index->setVisible(false);
	}
	else if (Generals::isReplicationUIMode())
	{
		ui.comboBox_material_cartridge_2->setVisible(false);
		ui.label_material_cartridge_2->setVisible(false);

		ui.comboBox_support_cartridge_index->setEnabled(false);
		ui.label_support_cartridge_index->setEnabled(false);
		
		ui.comboBox_raft_cartridge_index->setEnabled(false);
		ui.label_raft_cartridge_index->setEnabled(false);

		ui.checkBox_wipetower_enable->setEnabled(false);
		ui.label_wipetower_enable->setEnabled(false);
	}
	else if (Profile::machineProfile.machine_expanded_print_mode.value)
	{
		switch (Profile::machineProfile.machine_expanded_print_cartridgeIndex.value)
		{
		case 0:
			ui.comboBox_material_cartridge_1->setEnabled(true);
			ui.label_material_cartridge_1->setEnabled(true);

			//ui.comboBox_material_cartridge_2->clear();
			ui.comboBox_material_cartridge_2->setEnabled(false);
			ui.label_material_cartridge_2->setEnabled(false);

			break;
		case 1:
			//ui.comboBox_material_cartridge_1->clear();
			ui.comboBox_material_cartridge_1->setEnabled(false);
			ui.label_material_cartridge_1->setEnabled(false);

			ui.comboBox_material_cartridge_2->setEnabled(true);
			ui.label_material_cartridge_2->setEnabled(true);

			break;
		}
	}
	
	//window size fixing//
	//window()->layout()->setSizeConstraint(QLayout::SetFixedSize);

	//radio button 동작
	m_modeStr = easyModeProp;
	Q_FOREACH(QRadioButton *radioButton, ui.groupBox_QuickPrintProfile->findChildren<QRadioButton*>())
	{
		connect(radioButton, SIGNAL(clicked()), this, SLOT(radioButtonChanged()));
		if (radioButton->property("EasyProp").toString() == m_modeStr)
			radioButton->setChecked(true);
		else
			radioButton->setChecked(false);
	}

	ui.groupBox_infill->setVisible(false);
	ui.label_infill_density->setVisible(false);
	ui.doubleSpinBox_infill_density->setVisible(false);

	ui.label_infill_pattern->setVisible(false);
	ui.comboBox_infill_pattern->setVisible(false);

	//support placement check//
	comboBox_support_currentIndexChanged(Profile::sliceProfileCommon.support_placement.value);
}
void ProfileSettingEasymodeDlg::setConnectControlforApplyButton()
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

void ProfileSettingEasymodeDlg::loadSettingValue()
{
	ui.comboBox_material_cartridge_1->setCurrentText(m_sliceProfile_multi->at(0).filament_material.value);
	if (m_sliceProfile_multi->size() > 1)
		ui.comboBox_material_cartridge_2->setCurrentText(m_sliceProfile_multi->at(1).filament_material.value);


	ui.comboBox_support->setCurrentIndex(m_sliceProfileForCommon->support_placement.value);
	ui.comboBox_support_main_pattern->setCurrentIndex(m_sliceProfileForCommon->support_main_pattern.value);

	//infill setting 항목 삭제..//
	//setSpinBoxValue(ui.doubleSpinBox_infill_density, m_SliceProfile_multi->at(0).fill_density);
	//ui.comboBox_infill_pattern->setCurrentIndex(m_SliceProfile_multi->at(0).fill_pattern.value);

	if (Generals::isLayerColorModeOn())
	{
		ui.comboBox_raft_cartridge_index->setCurrentIndex(m_sliceProfileForCommon->adhesion_cartridge_index.value);
		ui.comboBox_support_cartridge_index->setCurrentIndex(0);
		ui.checkBox_wipetower_enable->setChecked(false);
	}
	else if (Generals::isReplicationUIMode())
	{
		ui.comboBox_raft_cartridge_index->setCurrentIndex(0);
		ui.comboBox_support_cartridge_index->setCurrentIndex(0);
		ui.checkBox_wipetower_enable->setChecked(false);
	}
	else
	{
		ui.comboBox_raft_cartridge_index->setCurrentIndex(m_sliceProfileForCommon->adhesion_cartridge_index.value);
		ui.comboBox_support_cartridge_index->setCurrentIndex(m_sliceProfileForCommon->support_main_cartridge_index.value);
		ui.checkBox_wipetower_enable->setChecked(m_sliceProfileForCommon->wipe_tower_enabled.value);
		ui.comboBox_wipetower_outer_cartridge_index->setCurrentIndex(m_sliceProfileForCommon->wipe_tower_outer_cartridge_index.value);
		ui.comboBox_wipetower_inner_cartridge_index->setCurrentIndex(std::abs(m_sliceProfileForCommon->wipe_tower_outer_cartridge_index.value - 1));
	}

	checkBox_wipetower_enable_toggled(ui.checkBox_wipetower_enable->isChecked());

	//Rhea7 model temporary code..//
	if (AppInfo::getAppName() == "Rhea")
	{
		ui.radioButton_fast_mode->setEnabled(false);
		ui.radioButton_fine_mode->setEnabled(false);
		ui.radioButton_silent_mode->setEnabled(false);
		ui.radioButton_normal_mode->setChecked(true);
	}
	else
	{
		ui.radioButton_fast_mode->setEnabled(true);
		ui.radioButton_fine_mode->setEnabled(true);
		ui.radioButton_silent_mode->setEnabled(true);
	}

	if (AppInfo::getAppName() == "3DWOX Desktop")
	{
		bool b_cartridge_FLEX = false;
		if (ui.comboBox_material_cartridge_1->currentText() == "FLEXIBLE")
			b_cartridge_FLEX = true;

		if (ui.comboBox_material_cartridge_2->currentText() == "FLEXIBLE")
			b_cartridge_FLEX = true;

		//flexible 가지고 있는 기종들 check//
		const bool has_FLEXIBLE_machine = Generals::check_material_in_list(Profile::machineProfile.available_material_list, "FLEXIBLE");

		if (b_cartridge_FLEX && has_FLEXIBLE_machine)
			ui.radioButton_fine_mode->setEnabled(false);
		else
			ui.radioButton_fine_mode->setEnabled(true);
		///////////////////////////////////////////////////////////////////////////////////////////////////
		checkBedAdhesionVisible(ui.comboBox_material_cartridge_1->currentText(), 0);
	}

	this->setModeTooltipString();
}

bool ProfileSettingEasymodeDlg::checkBedAdhesionVisible(QString _check_material, int _cartridge_index)
{
	m_bed_type_material.clear();

	///////////////////////////////////////////////////////////////////////////////////////////////////
		//UI파트에 해당하나 material 값이 필요하므로..//
		//TODO//기종 사양 --> 코드가 마음에 들지 않지만 profile 사양으로 넣자니 애매함.. 차후 고민..//

	SliceProfile temp_slice_profile_material;
	temp_slice_profile_material.loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_normal_" + _check_material + ".ini"));

	bool is_bed_type_material = false;
	if (temp_slice_profile_material.bed_type.value != 0)
	{
		is_bed_type_material = true;
		m_bed_type_material.append(_check_material);
	}
	
	if (Profile::machineProfile.extruder_count.value == 2)
	{
		//counter cartridge material check..//
		int current_counter_index = std::abs(_cartridge_index - 1);
		QString current_counter_material;
		switch(current_counter_index)
		{
		case 0:
			current_counter_material = ui.comboBox_material_cartridge_1->currentText();
			break;
		case 1:
			current_counter_material = ui.comboBox_material_cartridge_2->currentText();
			break;
		}

		SliceProfile temp_slice_profile_material2;
		temp_slice_profile_material2.loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_normal_" + current_counter_material + ".ini"));

		if (temp_slice_profile_material2.bed_type.value != 0)
		{
			is_bed_type_material = true;
			
			if (m_bed_type_material.isEmpty())
				m_bed_type_material = current_counter_material;
			else if (m_bed_type_material == current_counter_material)
			{ }
			else
				m_bed_type_material.append(", ").append(current_counter_material);
		}
	}
		

	if (is_bed_type_material && Profile::machineProfile.is_bed_type_selectable.value)
	{
		ui.groupBox_bedAdhesion->setVisible(false);
		return false;
	}
	else
	{
		ui.groupBox_bedAdhesion->setVisible(true);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////
}

//경로 가져오는 방식.. 간단하게.. 반복문 처리??//
void ProfileSettingEasymodeDlg::saveSettingValue()
{
	QStringList materialList;
	materialList.push_back(ui.comboBox_material_cartridge_1->currentText());
	materialList.push_back(ui.comboBox_material_cartridge_2->currentText());

	/////////////////////////////////////////////////////////////////////////////////
	//cartridge profile generate//
	for (int i = 0; i < m_sliceProfile_multi->size(); i++)
	{
		QString tempStr = Profile::profilePath + "profile_setting_easy_" + m_modeStr + "_" + Generals::getMaterialShortName(materialList.at(i)) + ".ini";

		if (!(m_sliceProfile_multi->at(i).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_" + m_modeStr + "_" + Generals::getMaterialShortName(materialList.at(i)) + ".ini"))))
		{
			CommonDialog comDlg(this, MessageAlert::tr("profile_not_found"), CommonDialog::Warning);
		}

		//infill setting 항목 삭제..//
		//m_SliceProfile_multi->at(i).fill_density.value = ui.doubleSpinBox_infill_density->value();
		//m_SliceProfile_multi->at(i).fill_pattern.value = ui.comboBox_infill_pattern->currentIndex();
	}

	int raftIdx = ui.comboBox_raft_cartridge_index->currentIndex();
	int supportType = ui.comboBox_support->currentIndex();
	int supportIdx = ui.comboBox_support_cartridge_index->currentIndex();
	ProfileControl::generateCommonProfileFromSliceProfile(m_sliceProfile_multi, m_sliceProfileForCommon, -1, raftIdx, supportType, supportIdx);
	//profileControl.setRaftProfile(m_SliceProfile_multi, m_SliceProfileForCommon, -1, raftIdx);
	//profileControl.setSupportProfile(m_SliceProfile_multi, m_SliceProfileForCommon, supportType, supportIdx);
	//profileControl.setCommonProfile(m_SliceProfile_multi, m_SliceProfileForCommon, m_parent->getUsedCartIdx(*m_SliceProfileForCommon));

	m_sliceProfileForCommon->support_main_pattern.value = ui.comboBox_support_main_pattern->currentIndex();

	m_sliceProfileForCommon->wipe_tower_enabled.value = ui.checkBox_wipetower_enable->isChecked();

	m_sliceProfileForCommon->wipe_tower_outer_cartridge_index.value = ui.comboBox_wipetower_outer_cartridge_index->currentIndex();
	m_sliceProfileForCommon->wipe_tower_inner_cartridge_index.value = ui.comboBox_wipetower_inner_cartridge_index->currentIndex();

	/////////////////////////////////////////////////////////////////////////////////
	//commmon profile generate//
	
	//raft의 값을 tempCommon에 담아서 값을 추출하여 생성..//
	//int raftCartridgeIndex = ui.comboBox_raft_cartridge_index->currentIndex();
	//if (!(m_SliceProfileForCommon->SetProfileDataDefault2(Generals::qstringTowchar_t(m_parent->m_profilePath + "profile_setting_easy_" + m_modeStr + "_" + materialList.at(raftCartridgeIndex) + ".ini"))))
	//{
	//	CommonDialog comDlg(this, tr("profile_not_found"), CommonDialog::Warning);
	//}
	//
	//m_SliceProfileForCommon->raft_cartridge_index.value = raftCartridgeIndex;
	//


	////support 값을 추출하여 생성..//
	//int supportCartridgeIndex = ui.comboBox_support_cartridge_index->currentIndex();
	//	
	//SliceProfileForCommon tempSupportProfile;
	//tempSupportProfile.SetProfileDataDefault2(Generals::qstringTowchar_t(m_parent->m_profilePath + "profile_setting_easy_" + m_modeStr + "_" + materialList.at(supportCartridgeIndex) + ".ini"));

	////support UI값을 가져오고..//
	//m_SliceProfileForCommon->support_main_cartridge_index.value = supportCartridgeIndex;
	//m_SliceProfileForCommon->support.value = ui.comboBox_support->currentIndex();
	//m_SliceProfileForCommon->support_type.value = ui.comboBox_support_main_pattern->currentIndex();

	////나머지 support항목은 직접 입력..//
	//m_SliceProfileForCommon->support_angle.value = tempSupportProfile.support_angle.value;
	//m_SliceProfileForCommon->support_infill_density.value = tempSupportProfile.support_fill_rate.value;
	//m_SliceProfileForCommon->support_xy_distance.value = tempSupportProfile.support_xy_distance.value;
	//m_SliceProfileForCommon->support_z_distance.value = tempSupportProfile.support_z_distance.value;

	//m_SliceProfileForCommon->wipe_tower.value = ui.checkBox_wipetower_enable->isChecked();


	////common중에서 재질의 우선 순위에 따라 변경되는 항목 적용..//
	//setCommonProfileUIbyMaterialIndex();
	

	/////////////////////////////////////////////////////////////////////////////////
	//multi slice profile generate//
	//profileControl.generateSliceProfileFromMultiCartProfile();
	/////////////////////////////////////////////////////////////////////////////////

	m_sliceProfileForCommon->wipe_tower_inner_size.value = m_sliceProfileForCommon->wipe_tower_outer_size.value - 2 * m_sliceProfileForCommon->wipe_tower_outer_wall_thickness.value - 2 * m_sliceProfileForCommon->wipe_tower_outer_inner_gap.value;


	//bed side값을 적용하기 이전에 volume들의 material을 파악하여 관련있을 때만 적용하도록 함..//
	//std::vector<int> usedCartIdx;
	//for (int i = 0; i < m_parent->model.volumes.size(); ++i)
	//{
	//	for (int j = 0; j < m_parent->model.getVolume(i)->meshInfo.size(); j++)
	//	{
	//		usedCartIdx.push_back(m_parent->model.getVolume(i)->meshInfo[j].mesh_cartridgeIndex);
	//	}
	//}
	//std::sort(usedCartIdx.begin(), usedCartIdx.end());
	//usedCartIdx.erase(unique(usedCartIdx.begin(), usedCartIdx.end()), usedCartIdx.end());

	//bool bed_type_flag = false;
	//for (int i = 0; i < usedCartIdx.size(); ++i)
	//{
	//	if (m_SliceProfile_multi->at(usedCartIdx.at(i)).bed_type.value != 0)
	//		bed_type_flag = true;
	//}


	//if (Profile::machineProfile.machine_bed_selected_enabled.value && bed_type_flag)
	//{
	//	if (Profile::machineProfile.machine_bed_selected_side.value == Generals::BED_SIDE::SIDE_A)
	//	{
	//		m_SliceProfileForCommon->z_offset_except_raft.value = 0;  //side A
	//		m_SliceProfileForCommon->print_bed_temperature.value = 90;
	//		m_SliceProfileForCommon->platform_adhesion.value = 0;
	//		m_SliceProfileForCommon->layer0_width_factor.value = 100;
	//	}
	//	else
	//	{
	//		m_SliceProfileForCommon->z_offset_except_raft.value = -0.15;  //side B
	//		m_SliceProfileForCommon->print_bed_temperature.value = 100;
	//		m_SliceProfileForCommon->platform_adhesion.value = 2;
	//		m_SliceProfileForCommon->layer0_width_factor.value = 200;
	//	}
	//}

	easyModeProp = m_modeStr;
	Generals::setProps("EasyProp", easyModeProp);

	emit signal_profileChanged(b_enabledApplyButton);

	setDisabledApplyButton();
}

void ProfileSettingEasymodeDlg::setSupportIndex(int supportIndex)
{
	if (supportIndex == -1) return;
	QString materialStr;

	switch (supportIndex)
	{
	case 0:
		materialStr = ui.comboBox_material_cartridge_1->currentText();
		break;

	case 1:
		materialStr = ui.comboBox_material_cartridge_2->currentText();
		break;
	}
	
	SliceProfileForCommon tempSupportProfile;
	tempSupportProfile.loadSliceProfileCommon(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_" + m_modeStr + "_" + Generals::getMaterialShortName(materialStr) + ".ini"));
	
	if (Profile::machineProfile.machine_expanded_print_mode.value)
		tempSupportProfile.wipe_tower_enabled.value = false;

	//ui화면용..//
	//ui.comboBox_support->setCurrentIndex(tempSupportProfile.support.value);
	ui.comboBox_support_main_pattern->setCurrentIndex(tempSupportProfile.support_main_pattern.value);
	ui.checkBox_wipetower_enable->setChecked(tempSupportProfile.wipe_tower_enabled.value);
}

void ProfileSettingEasymodeDlg::setMaterialIndex_cart(int _cartridge_index, QString materialStr_, QString supportMaterialStr_)
{
	if (!checkBedAdhesionVisible(materialStr_, _cartridge_index))
	{
		CommonDialog comDlg(this, MessageAlert::tr("recommand_to_use_bed_side_b").arg(m_bed_type_material), CommonDialog::Warning);
	}


	bool b_cartridge_FLEX = false;
	if (ui.comboBox_material_cartridge_1->currentText() == "FLEXIBLE")
		b_cartridge_FLEX = true;

	if (ui.comboBox_material_cartridge_2->currentText() == "FLEXIBLE")
		b_cartridge_FLEX = true;

	//flexible 가지고 있는 기종들 check//
	const bool has_FLEXIBLE_machine = Generals::check_material_in_list(Profile::machineProfile.available_material_list, "FLEXIBLE");

	if (b_cartridge_FLEX && has_FLEXIBLE_machine)
	{
		ui.radioButton_fine_mode->setEnabled(false);

		if (ui.radioButton_fine_mode->isChecked())
		{
			ui.radioButton_normal_mode->setChecked(true);
			m_modeStr = ui.radioButton_normal_mode->property("EasyProp").toString();
		}
	}
	else
		ui.radioButton_fine_mode->setEnabled(true);

	if (supportMaterialStr_ != "")
	{
		SliceProfileForCommon tempSupportProfile;
		tempSupportProfile.loadSliceProfileCommon(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_" + m_modeStr + "_" + Generals::getMaterialShortName(supportMaterialStr_) + ".ini"));

		if (Profile::machineProfile.machine_expanded_print_mode.value)
			tempSupportProfile.wipe_tower_enabled.value = false;

		//ui화면용..//
		ui.comboBox_support->setCurrentIndex(tempSupportProfile.support_placement.value);
		ui.comboBox_support_main_pattern->setCurrentIndex(tempSupportProfile.support_main_pattern.value);
		ui.checkBox_wipetower_enable->setChecked(tempSupportProfile.wipe_tower_enabled.value);
	}

	this->setModeTooltipString();
}

void ProfileSettingEasymodeDlg::setSpinBoxValue(QDoubleSpinBox *targetUI, ProfileDataD profileValue)
{
	double minValue = profileValue.min;
	double maxValue = profileValue.max;
	double value = profileValue.value;

	targetUI->setMinimum(minValue);
	targetUI->setMaximum(maxValue);
	targetUI->setValue(value);
}

void ProfileSettingEasymodeDlg::setEnabledApplyButton()
{
	b_enabledApplyButton = true;
	ui.pushButton_apply->setEnabled(true);
}

void ProfileSettingEasymodeDlg::setDisabledApplyButton()
{
	b_enabledApplyButton = false;
	ui.pushButton_apply->setEnabled(false);
}

void ProfileSettingEasymodeDlg::setUIcontrolByMachineModel()
{
	int cartCount = Profile::machineProfile.extruder_count.value;

	switch (cartCount)
	{
	case 1:
		ui.comboBox_material_cartridge_2->clear();
		ui.comboBox_material_cartridge_2->setEnabled(false);
		ui.label_material_cartridge_2->setEnabled(false);

		ui.groupBox_wipetower->setEnabled(false);
		ui.label_wipetower_enable->setEnabled(false);
		ui.checkBox_wipetower_enable->setEnabled(false);
		ui.checkBox_wipetower_enable->setChecked(false);

		ui.comboBox_raft_cartridge_index->setEnabled(false);
		ui.comboBox_support_cartridge_index->setEnabled(false);
		break;
	end:
		ui.comboBox_material_cartridge_2->setEnabled(true);
		ui.label_material_cartridge_2->setEnabled(true);

		ui.groupBox_wipetower->setEnabled(true);
		ui.label_wipetower_enable->setEnabled(true);
		ui.checkBox_wipetower_enable->setEnabled(true);
		ui.checkBox_wipetower_enable->setChecked(m_sliceProfile_multi->at(0).wipe_tower_enabled.value);

		ui.comboBox_raft_cartridge_index->setEnabled(true);
		ui.comboBox_support_cartridge_index->setEnabled(true);
		break;
	}

	//expanded mode setting..//
	{
		ui.comboBox_raft_cartridge_index->setEnabled(!Profile::machineProfile.machine_expanded_print_mode.value);
		ui.comboBox_support_cartridge_index->setEnabled(!Profile::machineProfile.machine_expanded_print_mode.value);

		ui.label_wipetower_enable->setEnabled(!Profile::machineProfile.machine_expanded_print_mode.value);
		ui.checkBox_wipetower_enable->setEnabled(!Profile::machineProfile.machine_expanded_print_mode.value);
		ui.checkBox_wipetower_enable->setChecked(!Profile::machineProfile.machine_expanded_print_mode.value);
	}

}

void ProfileSettingEasymodeDlg::resetCartridgeUI(int cartridgeCount)
{
	QStringList strList;

	for (int i = 0; i < cartridgeCount; i++)
	{
		strList.push_back(CustomTranslate::tr("Cartridge") + QString("(%1)").arg(i + 1));
	}

	ui.comboBox_support_cartridge_index->clear();
	ui.comboBox_raft_cartridge_index->clear();

	ui.comboBox_support_cartridge_index->addItems(strList);
	ui.comboBox_raft_cartridge_index->addItems(strList);
}

void ProfileSettingEasymodeDlg::resetMaterialUI(int cartridgeCount)
{
	QStringList tempList = Profile::machineProfile.available_material_list;
	tempList.removeOne("ETC");

	ui.comboBox_material_cartridge_1->clear();
	ui.comboBox_material_cartridge_1->addItems(tempList);
	if (cartridgeCount == 2)
	{
		ui.comboBox_material_cartridge_2->clear();
		ui.comboBox_material_cartridge_2->addItems(tempList);
	}
}

void ProfileSettingEasymodeDlg::setModeTooltipString()
{
	//mode : fast, normal, silent, fine//
	//mode size 4 --> fixed..//
	std::vector<SliceProfile> sliceProfile_mode;
	sliceProfile_mode.resize(4);

	//mode tooltip values --> 4 mode size//
	std::vector<ModeTooltipValues> modeTooltipValues;

	QString materialStr;
	materialStr = ui.comboBox_material_cartridge_1->currentText();

	sliceProfile_mode.at(0).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_fast_" + Generals::getMaterialShortName(materialStr) + ".ini"));
	sliceProfile_mode.at(1).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_normal_" + Generals::getMaterialShortName(materialStr) + ".ini"));
	sliceProfile_mode.at(2).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_silent_" + Generals::getMaterialShortName(materialStr) + ".ini"));
	sliceProfile_mode.at(3).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_easy_fine_" + Generals::getMaterialShortName(materialStr) + ".ini"));
	
	//mode tooltip value : layer height, fill density, print speed setting..//
	for (auto profile : sliceProfile_mode)
	{
		modeTooltipValues.push_back({ profile.layer_height.value, profile.infill_density.value, profile.print_speed.value });
	}

	QString mode_tooltip;
	mode_tooltip = tr("fast_mode_tooltip").arg(modeTooltipValues.at(0).layer_height, 0, 'f', 2).arg(modeTooltipValues.at(0).infill_density).arg(int(modeTooltipValues.at(0).print_speed));
	ui.radioButton_fast_mode->setToolTip(mode_tooltip);
	mode_tooltip = tr("normal_mode_tooltip").arg(modeTooltipValues.at(1).layer_height, 0, 'f', 2).arg(modeTooltipValues.at(1).infill_density).arg(int(modeTooltipValues.at(1).print_speed));
	ui.radioButton_normal_mode->setToolTip(mode_tooltip);
	mode_tooltip = tr("silent_mode_tooltip").arg(modeTooltipValues.at(2).layer_height, 0, 'f', 2).arg(modeTooltipValues.at(2).infill_density).arg(int(modeTooltipValues.at(2).print_speed));
	ui.radioButton_silent_mode->setToolTip(mode_tooltip);
	mode_tooltip = tr("fine_mode_tooltip").arg(modeTooltipValues.at(3).layer_height, 0, 'f', 2).arg(modeTooltipValues.at(3).infill_density).arg(int(modeTooltipValues.at(3).print_speed));
	ui.radioButton_fine_mode->setToolTip(mode_tooltip);
}




#pragma region private slots
void ProfileSettingEasymodeDlg::pushButton_ok_clicked()	//accept() overloaded
{
	saveSettingValue();
	close();
}
void ProfileSettingEasymodeDlg::pushButton_apply_clicked()
{
	saveSettingValue();
}
void ProfileSettingEasymodeDlg::comboBox_support_cartridge_index_currentIndexChanged(int index_)
{
	setSupportIndex(index_);
}
void ProfileSettingEasymodeDlg::comboBox_material_cartridge_1_currentTextChanged(QString text_)
{
	QString supportMaterialStr = "";
	// hjkim add 09.24 
	// 카트리지 하나를 Rize Miaterial 로 세팅시에 나머지 하나를 자동으로 Rize support 로 변경 
	if (text_ == "RZCB" || text_ == "RZGF") {
		qDebug() << text_;
		ui.comboBox_material_cartridge_2->setCurrentText("RZSU");
		ui.comboBox_raft_cartridge_index->setCurrentIndex(1);
		ui.comboBox_support_cartridge_index->setCurrentIndex(1);
		ui.comboBox_support->setCurrentIndex(2);
	}

	if (ui.comboBox_support_cartridge_index->currentIndex() == 0)
	{
		supportMaterialStr = text_;
	}
	setMaterialIndex_cart(0, text_, supportMaterialStr);
}
void ProfileSettingEasymodeDlg::comboBox_material_cartridge_2_currentTextChanged(QString text_)
{
	QString supportMaterialStr = "";
	if (ui.comboBox_support_cartridge_index->currentIndex() == 1)
	{
		supportMaterialStr = text_;
		if (text_ == "RZCB" || text_ == "RZGF") {
			qDebug() << text_;
		}
	}
	setMaterialIndex_cart(1, text_, supportMaterialStr);
}
void ProfileSettingEasymodeDlg::comboBox_wipeToewer_cartridgeIndex_currentIndexChanged(int idx)
{
	QComboBox* ui_comboBox = dynamic_cast<QComboBox*>(sender());

	int counter_idx = std::abs(idx - 1);

	if (ui_comboBox == ui.comboBox_wipetower_outer_cartridge_index)
	{
		ui.comboBox_wipetower_inner_cartridge_index->setCurrentIndex(counter_idx);
	}
	else
	{
		ui.comboBox_wipetower_outer_cartridge_index->setCurrentIndex(counter_idx);
	}
}
void ProfileSettingEasymodeDlg::checkBox_wipetower_enable_toggled(bool check)
{
	ui.label_wipetower_outer_cartridge_index->setEnabled(check);
	ui.comboBox_wipetower_outer_cartridge_index->setEnabled(check);

	ui.label_wipetower_inner_cartridge_index->setEnabled(check);
	ui.comboBox_wipetower_inner_cartridge_index->setEnabled(check);

}
void ProfileSettingEasymodeDlg::radioButtonChanged()
{
	QRadioButton* ui_radio = dynamic_cast<QRadioButton*>(sender());
	QString easyMode = ui_radio->property("EasyProp").toString();
	if (easyMode != "")
		m_modeStr = easyMode;
}
void ProfileSettingEasymodeDlg::comboBox_support_currentIndexChanged(int _index)
{
	if (_index == -1)
		_index = ui.comboBox_support->currentIndex();

	if (_index == Generals::SupportPlacement::SupportNone)
	{
		ui.label_support_cartridge_index->setVisible(false);
		ui.comboBox_support_cartridge_index->setVisible(false);

		ui.label_support_main_pattern->setVisible(false);
		ui.comboBox_support_main_pattern->setVisible(false);
	}
	else
	{
		if (Generals::isLayerColorModeOn() || Generals::isReplicationUIMode())
		{
			ui.label_support_cartridge_index->setVisible(false);
			ui.comboBox_support_cartridge_index->setVisible(false);
		}
		else
		{
			ui.label_support_cartridge_index->setVisible(true);
			ui.comboBox_support_cartridge_index->setVisible(true);
		}

		ui.label_support_main_pattern->setVisible(true);
		ui.comboBox_support_main_pattern->setVisible(true);
	}
}
#pragma endregion