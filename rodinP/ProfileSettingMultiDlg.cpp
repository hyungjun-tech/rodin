#include "stdafx.h"
#include "ProfileSettingMultiDlg.h"

#include "CartridgeInfo.h"
#include "ProfileControl.h"
#include "CustomProfileList.h"
#include "CustomProfileEditor.h"

enum tabWidget { tab_quality, tab_material, tab_support, tab_bed_adhesion, tab_speed, tab_retraction, tab_multi_nozzle, tab_wipe_tower, tab_other };


ProfileSettingMultiDlg::ProfileSettingMultiDlg(QWidget *parent)
	: QDialog(parent)
	, pre_selectedCartridgeIndex(-1)
	, selectedCartridgeIndex(0)
	, b_enabledApplyButton(false)
	, m_do_not_change(true)
	, m_color_common("120, 240, 0")
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);

	this->setUI();
	this->setConnection();

	CustomProfileList customProfile;
	m_profileList = customProfile.getProfileList(true, true);
	ui.widget->resetProfileNameList(m_profileList);

	//�������� �ʱ�ȭ �� ����
	initProfile();

	this->loadSettingValue();

	setUIcontrolByMachineModel();

	connect(ui.widget, SIGNAL(enableApplyButton(bool)), this, SLOT(setEnabledApplyButton(bool)));
	//connect(ui.widget_common, SIGNAL(enableApplyButton(bool)), this, SLOT(SetEnabledApplyButton(bool)));

	ui.widget->setMaterialListDisable();
	comboBox_cartridge_list_currentIndexChanged(0);

	setEnabledApplyButton(false);
}


ProfileSettingMultiDlg::~ProfileSettingMultiDlg()
{

}

void ProfileSettingMultiDlg::setUI()
{
	//īƮ������ ������ ���� ���� ����Ʈ
	m_colorList.append("255, 148, 53");
	m_colorList.append("53, 158, 255");
	m_colorList.append("255, 0, 126");
	m_colorList.append("3, 156, 140");

	//ȭ�� ������ ���̱� ���� unvisible�ϰ�...
	//����� ���۾ȵ�......�ٽ� Ȯ��
	/*ui.frame_support->setVisible(false);
	ui.frame_raft->setVisible(false);
	ui.frame_brim->setVisible(false);
	ui.frame_skirt->setVisible(false);*/
	//ui.comboBox_support_main_cartridge_index->setEnabled(false);
	//ui.comboBox_adhesion_cart_index->setEnabled(false);

	//īƮ���� ������ŭ List����
	resetCartridgeUI(CartridgeInfo::cartridges.size());

	//combobox_cartridge_list�� ������ �߰��ؾ� �ϹǷ�, nonfocus ���� �ʿ�..//swyang
	//ui.comboBox_cartridge_list->setFocusPolicy(Qt::FocusPolicy::NoFocus);

	//ui.tabWidget->setCurrentIndex(0);

	//filament diameter�� �ϴ� disable�� �Ͽ� ����ڰ� �������� ���ϰ� ��.//
	//extrusionPerMM�� ���� flow�� ����..//
	//���� open material type���� �� ���, ������ ����..//
	//ui.doubleSpinBox_filament_diameter->setEnabled(false);

	//window size fixing//
	//int sizeHint = ui.listWidget_cartridge_list->sizeHintForColumn(0);
	//if (ui.frame_left->maximumWidth() < (sizeHint + 50))
	//	ui.frame_left->setMaximumWidth(sizeHint + 50);
	//window()->layout()->setSizeConstraint(QLayout::SetFixedSize);



}

void ProfileSettingMultiDlg::setConnection()
{
	////////////////////////////////////////////////////////
	//connect(ui.listWidget_cartridge_list, SIGNAL(currentRowChanged(int)), this, SLOT(listWidget_cartridge_list_currentRowChanged(int)));
	connect(ui.comboBox_cartridge_list, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_cartridge_list_currentIndexChanged(int)));

	///////////////////////////////////////////////////////
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui.pushButton_cancel, SIGNAL(clicked()), this, SLOT(cancel()));
	connect(ui.pushButton_apply, SIGNAL(clicked()), this, SLOT(apply()));

	//connect(ui.widget_common, SIGNAL(raftValueChanged()), this, SLOT(setOriginalValue()));
	//connect(ui.widget_common, SIGNAL(raftValueChanged()), ui.widget, SLOT(setRaftValue()));
	//connect(ui.widget_common, SIGNAL(commonValueChanged()), ui.widget, SLOT(setCommonValue()));
	connect(ui.widget, SIGNAL(raftValueChanged()), this, SLOT(setOriginalValue()));
	connect(ui.widget, SIGNAL(raftValueChanged()), ui.widget, SLOT(setRaftValue()));
	connect(ui.widget, SIGNAL(commonValueChanged()), ui.widget, SLOT(setCommonValue()));

	connect(ui.widget, SIGNAL(warningMessage(QString)), this, SLOT(showMessage(QString)));
	//connect(ui.widget_common, SIGNAL(warningMessage(QString)), this, SLOT(showMessage(QString)));


	connect(ui.widget, SIGNAL(comboBox_profileNameList_change_sig(int)), this, SLOT(comboBox_profileNameList_currentIndexChanged(int)));
	connect(ui.widget, SIGNAL(comboBox_profileNameList_change_forUI(int)), this, SLOT(comboBox_profileNameList_currentIndexChanged_forUI(int)));
	connect(ui.widget, SIGNAL(pushButton_edit_clicked_sig()), this, SLOT(showCustomProfileEditor()));
	//connect(ui.widget, SIGNAL(pushButton_reset_clicked_sig()), ui.widget_common, SIGNAL(pushButton_reset_clicked_sig()));
	connect(ui.widget, SIGNAL(tabWidget_profile_current_change_signal(int)), this, SLOT(tabWidget_profile_currentChanged(int)));
}

void ProfileSettingMultiDlg::initProfile()
{
	m_tempSliceProfile = new std::vector<SliceProfile>;
	m_tempSliceProfile_common = new SliceProfileForCommon;
	m_originalProfile = new std::vector<SliceProfile>;
	m_originalProfile_common = new SliceProfileForCommon;

	m_sliceProfile_multi = &Profile::sliceProfile;
	m_sliceProfile_common = &Profile::sliceProfileCommon;

	m_tempSliceProfile->resize(m_sliceProfile_multi->size());
	m_originalProfile->resize(m_sliceProfile_multi->size());

	for (int i = 0; i < m_tempSliceProfile->size(); i++)
	{
		m_tempSliceProfile->at(i) = m_sliceProfile_multi->at(i);
		//m_tempSliceProfile->at(i).setSliceProfileMap();
	}
	*m_tempSliceProfile_common = *m_sliceProfile_common;

	setOriginalValue();
}

void ProfileSettingMultiDlg::accept()	//accept() overloaded
{
	if (!ui.widget->validateWipeTowerSize())
		return;

	this->saveSettingValue();

	this->close();
}

void ProfileSettingMultiDlg::cancel()	//cancel() overloaded
{
	this->close();
}

void ProfileSettingMultiDlg::apply()	//apply() overloaded
{
	if (!ui.widget->validateWipeTowerSize())
		return;

	this->saveSettingValue();
}

void ProfileSettingMultiDlg::loadSettingValue()
{
	ui.widget->setParameter(
		m_tempSliceProfile,
		m_tempSliceProfile_common,
		m_originalProfile,
		m_originalProfile_common,
		CartridgeInfo::getUsedCartIdx(),
		selectedCartridgeIndex);
	//ui.widget->setParameter(&m_tempSliceProfile->at(selectedCartridgeIndex), m_tempSliceProfile_common, m_parent->isColoredLayerOn());
	//ui.widget_common->setParameter(m_tempSliceProfile, m_tempSliceProfile_common, m_originalProfile, m_originalProfile_common, m_parent->isColoredLayerOn(), m_parent->getUsedCartIdx(*m_tempSliceProfile_common));

	//original setting�� �׻� loading �� �� �ؾ���..// 190705 �����Ʈ��
	//if (Profile::machineProfile.machine_bed_selected_enabled.value == 1 && m_tempSliceProfile->at(selectedCartridgeIndex).bed_type.value != 0)
	//{
	//	changeValueByBedSide_originalValue(Profile::machineProfile.machine_bed_selected_side.value);

	//	m_originalProfile_common->getCommonProfileFromSliceProfile(&(m_originalProfile->at(selectedCartridgeIndex)));
	//}

	//if (m_parent->b_machineBedSideChange_flag && m_tempSliceProfile->at(selectedCartridgeIndex).bed_type.value != 0)
	//{
	//		//CommonDialog comDlg(this, tr("SettingMsgJ"), CommonDialog::Warning);
	//		changeValueByBedSide_All(Profile::machineProfile.machine_bed_selected_side.value);

	//		//TODO//profilecontorl ����ġ�� ����Ұ���? �ƴ���..?
	//		m_tempSliceProfile_common->getCommonProfileFromSliceProfile(&(m_tempSliceProfile->at(selectedCartridgeIndex)));
	//		m_originalProfile_common->getCommonProfileFromSliceProfile(&(m_originalProfile->at(selectedCartridgeIndex)));

	//	m_parent->b_machineBedSideChange_flag = false;
	//}

	ui.widget->loadOriginalValue(m_originalProfile->at(0));
	ui.widget->loadOriginalCommonValue(*m_originalProfile_common);
	
	// 2021.10.14 hjkim delete . ���� setParameter���� �̹� profile�� load�� �Ͽ� �ߺ� �����ϴ� �� ��.
	//ui.widget->loadProfileValue(m_tempSliceProfile->at(selectedCartridgeIndex));
	//ui.widget->loadProfileCommonValue(*m_tempSliceProfile_common);

}

void ProfileSettingMultiDlg::resetOriginalValue(SliceProfile* sliceProfile, SliceProfileForCommon* sliceProfileCommon)
{
	m_originalProfile->at(selectedCartridgeIndex) = *sliceProfile;
	//*m_originalProfile_common = *sliceProfileCommon;

	//ui.widget_common->resetOriginalAdhesionValue(m_tempSliceProfile_common->platform_adhesion.value, m_tempSliceProfile_common->adhesion_cartridge_index.value);
	//ui.widget_common->resetOriginalSupportValue(m_tempSliceProfile_common->support_placement.value, m_tempSliceProfile_common->support_main_cartridge_index.value);
	//ui.widget_common->resetOriginalCommonValue();
	ui.widget->resetOriginalAdhesionValue(m_tempSliceProfile_common->platform_adhesion.value, m_tempSliceProfile_common->adhesion_cartridge_index.value);
	ui.widget->resetOriginalSupportValue(m_tempSliceProfile_common->support_placement.value, m_tempSliceProfile_common->support_main_cartridge_index.value);
	ui.widget->resetOriginalCommonValue();

	ui.widget->loadOriginalValue(m_originalProfile->at(selectedCartridgeIndex));
	ui.widget->loadOriginalCommonValue(*m_originalProfile_common);
	//ui.widget_common->loadOriginalValue(*m_originalProfile_common);
}

void ProfileSettingMultiDlg::resetUIValue(SliceProfile* sliceProfile, SliceProfileForCommon* sliceProfileCommon)
{
	m_sliceProfile_multi->at(selectedCartridgeIndex) = *sliceProfile;
	//*m_originalProfile_common = *sliceProfileCommon;

	//ui.widget_common->setAdhesionCurrentIdx(m_tempSliceProfile->at(selectedCartridgeIndex).platform_adhesion.value);
	//ui.widget_common->setSupportCurrentIdx(m_tempSliceProfile->at(selectedCartridgeIndex).support_placement.value);
	//ui.widget_common->resetProfileAdhesionValue(m_tempSliceProfile_common->platform_adhesion.value, m_tempSliceProfile_common->adhesion_cartridge_index.value);
	//ui.widget_common->resetProfileSupportValue(m_tempSliceProfile_common->support_placement.value, m_tempSliceProfile_common->support_main_cartridge_index.value);
	//ui.widget_common->resetProfileCommonValue();
	ui.widget->setAdhesionCurrentIdx(m_tempSliceProfile->at(selectedCartridgeIndex).platform_adhesion.value);
	ui.widget->setSupportCurrentIdx(m_tempSliceProfile->at(selectedCartridgeIndex).support_placement.value);
	ui.widget->resetProfileAdhesionValue(m_tempSliceProfile_common->platform_adhesion.value, m_tempSliceProfile_common->adhesion_cartridge_index.value);
	ui.widget->resetProfileSupportValue(m_tempSliceProfile_common->support_placement.value, m_tempSliceProfile_common->support_main_cartridge_index.value);
	ui.widget->resetProfileCommonValue();

	ui.widget->loadProfileValue(*sliceProfile);
	ui.widget->loadProfileCommonValue(*sliceProfileCommon);
}
//
void ProfileSettingMultiDlg::saveSettingValue()
{
	//UI�� ���� temp profile�� ����
	ui.widget->saveProfileValue(&m_tempSliceProfile->at(selectedCartridgeIndex));//SliceProfile_multi - UI���� ���� ���õ� īƮ���� �������Ͽ� ����
	//ui.widget_common->saveProfileCommonValue();//SliceProfile_common - ��� �����׸� ����
	ui.widget->saveProfileCommonValue();//SliceProfile_common - �ϴ� �����׸� ����
	//ui.widget_common->saveProfileValue();//SliceProfile_multi - adhesion / support ����

	//temp�� cartridge�� ��� ī��
	this->saveTempProfileToCartridgeProfile();

	//profileControl.generateSliceProfileFromMultiCartProfile();
	//ProfileControl::updateConfigForMultiCartridge(m_sliceProfile_multi, m_sliceProfile_common, &m_parent->m_Config_multi);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//m_parent->afterProfileChanged(b_enabledApplyButton);
	emit signal_profileChanged(b_enabledApplyButton);
	setEnabledApplyButton(false);
}

void ProfileSettingMultiDlg::updateCartridgeUIvalue(std::vector<SliceProfile> *sliceProfile, int index)
{
	ui.widget->loadProfileValue(sliceProfile->at(index));

}
void ProfileSettingMultiDlg::setEnabledApplyButton(bool enabledApplyButton)
{
	b_enabledApplyButton = enabledApplyButton;

	ui.pushButton_apply->setEnabled(enabledApplyButton);
}

//�𵨺��� UI���� ���� ��Ʈ���� �͵� ����.. bed adhesion type�� ���� ���� SetBedAdhesionType()���� setting�� ��.
void ProfileSettingMultiDlg::setUIcontrolByMachineModel()
{
	//UI control by cartridge count//
	if (Profile::machineProfile.extruder_count.value > 1)
	{
		//	if (Profile::machineProfile.replication_print.value)
		//	{
		//		for (int i = 1; i < ui.listWidget_cartridge_list->count(); i++)
		//		{
		//			QListWidgetItem *item = ui.listWidget_cartridge_list->item(i);
		//			item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
		//		}
		//	}
	}
}


//SJ
//Material�� �����ϸ� ���ο� ���������� �ҷ������� ��Ŀ���
//���������� �и��ϴ� ���·� ���������̹Ƿ� �ش� ���� �ϴ� ����
/*
//cartridge�� material�� ���� �� �� ȣ��..
void ProfileSettingMultiDlg::comboBox_Material_currentIndexChanged(int UImaterialIndex)
{
	if (m_do_not_change)
	{
		m_do_not_change = false;
		return;
	}
	//cartridge profile//
	QString l_material = ui.comboBox_Material->currentText();
	//ui�� ������ ���ÿ� temp�� �־���.
	bool rtn = tempSliceProfile->at(selectedCartridgeIndex).SetProfileDataDefault2(Generals::qstringTowchar_t(m_parent->m_profilePath + "profile_setting_value_" + Generals::getMaterialShortName(l_material) + ".ini"));

	this->updateCartridgeUIvalue(&tempSliceProfile->at(selectedCartridgeIndex));

	setRaftValueUI(selectedCartridgeIndex);
	setSupportValueUI(selectedCartridgeIndex);
	setCommonValueUI();
}*/
void ProfileSettingMultiDlg::comboBox_profileNameList_currentIndexChanged(int idx)
{
	if (idx == -1) idx = m_selectedProfileIdx;
	if (idx >= m_profileList.size()) return;
	QString l_fileName = m_profileList.at(idx).fileLocation;
	QString l_profileDir;
	if (m_profileList.at(idx).custom)
		l_profileDir = Profile::customProfilePath;
	else
		l_profileDir = Profile::profilePath;

	if (!m_tempSliceProfile->at(selectedCartridgeIndex).loadSliceProfile(Generals::qstringTowchar_t(l_profileDir + l_fileName)))
	{
		CommonDialog comDlg(this, MessageError::tr("fail_to_load_profile"), CommonDialog::Critical);
		return;
	}

	if (!m_tempSliceProfile->at(selectedCartridgeIndex).loadTemperatureLayerList(Generals::qstringTowchar_t(l_profileDir + l_fileName)))
	{
		CommonDialog comDlg(this, MessageError::tr("fail_to_load_profile"), CommonDialog::Critical);
		return;
	}


	// 2021.10.13 hjkim add 
    // temp profile�� common value setting 
	ProfileControl::generateCommonProfileFromSliceProfile(
		m_tempSliceProfile,
		m_tempSliceProfile_common,
		m_tempSliceProfile->at(selectedCartridgeIndex).platform_adhesion.value,
		selectedCartridgeIndex,
		m_tempSliceProfile->at(selectedCartridgeIndex).support_placement.value,
		selectedCartridgeIndex);

	setOriginalValue();


	/////////////////////////////////////////////////////////////////////
	//when profile list changed, add a code to check material combination between two profiles..//
	//only for multi-nozzle machine..//
	if (Profile::machineProfile.extruder_count.value >= 2)
	{
		int counter_selectedCartridgeIndex = std::abs(selectedCartridgeIndex - 1);

		QString counter_material = m_tempSliceProfile->at(counter_selectedCartridgeIndex).filament_material.value;
		QString selected_material = m_tempSliceProfile->at(selectedCartridgeIndex).filament_material.value;

		int check_result = Generals::checkMaterialCombination(counter_material, selected_material);

		if (check_result == Generals::CheckPrintable::Warning)
		{
			CartridgeInfo::setUseStateForProfile(*m_tempSliceProfile_common);
			std::vector<int> usedCartIndexResult = CartridgeInfo::getUsedCartIdx();
			if (usedCartIndexResult.size() > 1)
				CommonDialog comDlg(this, MessageAlert::tr("difficult_material_combination").arg(counter_material, selected_material), CommonDialog::Warning);
		}

		//for wipe tower enabled check..//
		if (Generals::checkMaterialCombination_wipetower(counter_material, selected_material) && check_result == Generals::CheckPrintable::Ok)
			m_tempSliceProfile_common->wipe_tower_enabled.value = true;
	}
	/////////////////////////////////////////////////////////////////////


	ui.widget->setProfilesLink(m_tempSliceProfile, m_tempSliceProfile_common, selectedCartridgeIndex);

	ui.widget->loadProfileValue(m_tempSliceProfile->at(selectedCartridgeIndex));
	ui.widget->loadProfileCommonValue(*m_tempSliceProfile_common);
	ui.widget->loadOriginalValue(m_originalProfile->at(selectedCartridgeIndex));
	ui.widget->loadOriginalCommonValue(*m_originalProfile_common);

	// 2021.10.13 hjkim delete �տ��� temp profile common�� ������ load �Ͽ����Ƿ� ui���� ���� �ٽ� �������� ����.
	//ui.widget->setAdhesionCurrentIdx(m_tempSliceProfile->at(selectedCartridgeIndex).platform_adhesion.value);
	//ui.widget->setSupportCurrentIdx(m_tempSliceProfile->at(selectedCartridgeIndex).support_placement.value);
	//ui.widget->setRaftValueUI(selectedCartridgeIndex);
	//ui.widget->setSupportValueUI(selectedCartridgeIndex);
	//ui.widget->setCommonValueUI();

	if (m_tempSliceProfile->at(selectedCartridgeIndex).temperature_layer_setting_enabled.value)
		ui.widget->updateTemperatureLayerListWidget(&m_tempSliceProfile->at(selectedCartridgeIndex));

	/////////////////////////////////////////////////////////////////////
	//for exception layer0_width_factor by UI//
	//set exception only when profile list changed signal emitted ProfileControl::setRaftProfile() could not change layer0_width_factor..// swyang 20200318
	// 2021.10.14 hjkim delete �տ��� common profile�� �����Ͽ����Ƿ�, ���� �ٽ� ������ �ʿ� ���� ���� 
	//m_tempSliceProfile_common->initial_layer_width_factor.value = m_tempSliceProfile->at(selectedCartridgeIndex).initial_layer_width_factor.value;
	//ui.widget->setProfileValue_initial_layer_width_factor(m_tempSliceProfile_common->initial_layer_width_factor);
}


void ProfileSettingMultiDlg::comboBox_profileNameList_currentIndexChanged_forUI(int idx)
{
	m_selectedProfileIdx = idx;
	//unknown�� ��� - idx�� m_profileList�� ������ ����� ��.
	if (m_profileList.size() <= idx) //unknown - edit �� reset �Ұ���
	{
		ui.widget->setEditVisible(false);
		ui.widget->setResetVisible(false);
		return;
	}
	if (m_profileList.at(idx).custom) //custom - edit �� reset ����
	{
		ui.widget->setEditVisible(true);
		ui.widget->setResetVisible(true);
	}
	else //standard - edit �Ұ� , reset ����
	{
		ui.widget->setEditVisible(false);
		ui.widget->setResetVisible(true);
	}
}

//raft�� cartridge�� �����Ͽ��� �� ȣ��..

void ProfileSettingMultiDlg::comboBox_cartridge_list_currentIndexChanged(int index)
{
	if (index < 0) return;
	//SetDisConnectControlforMaterialIndex();

	QFont font_arial_bold_underline = QFont("Malgun Gothic", 9, QFont::Bold);
	font_arial_bold_underline.setUnderline(true);

	cout << "index2: " << index << "\n";
	this->selectedCartridgeIndex = index;
	//ui.listWidget_cartridge_list->item(selectedCartridgeIndex)->setBackgroundColor(m_colorList.at(selectedCartridgeIndex));
	//ui.listWidget_cartridge_list->item(selectedCartridgeIndex)->setFont(font_arial_bold_underline);
	//ui.label_cartridge_profile_setting->setText(tr("Cartridge") + " " +  QString::number(selectedCartridgeIndex + 1) + " : " + tr("Profile Settings"));

	//tab �׵θ� ���� ����
	QString current_cartridge_color = m_colorList.at(selectedCartridgeIndex);
	ui.widget->setTabStyle(current_cartridge_color, m_color_common);
	ui.comboBox_cartridge_list->setStyleSheet("QComboBox { background-color: rgb(" + current_cartridge_color + "); selection-background-color: rgb(" + current_cartridge_color + "); selection-color: black;}");

	if (pre_selectedCartridgeIndex == selectedCartridgeIndex) return;
	if (pre_selectedCartridgeIndex >= 0)
	{
		//QFont font_arial_normal = QFont("Malgun Gothic", 9, QFont::Normal);
		//ui.listWidget_cartridge_list->item(pre_selectedCartridgeIndex)->setBackgroundColor(QColor(255, 255, 255));
		//ui.listWidget_cartridge_list->item(pre_selectedCartridgeIndex)->setFont(font_arial_normal);
		ui.widget->saveProfileValue(&m_tempSliceProfile->at(pre_selectedCartridgeIndex));
	}

	//m_tempSliceProfile �� side A �� side B �Ķ���� ����
	//if (Profile::machineProfile.machine_bed_selected_enabled.value == 1 && m_tempSliceProfile->at(selectedCartridgeIndex).bed_type.value != 0)
	//	setProfileByBedSide(true);

	ui.widget->loadOriginalValue(m_originalProfile->at(selectedCartridgeIndex));
	ui.widget->loadOriginalCommonValue(*m_originalProfile_common);
	this->updateCartridgeUIvalue(m_tempSliceProfile, selectedCartridgeIndex);
	//this->LoadSettingValue();

	pre_selectedCartridgeIndex = selectedCartridgeIndex;
	ui.widget->m_selectedCartridgeIndex = selectedCartridgeIndex;

	if (m_tempSliceProfile->at(selectedCartridgeIndex).temperature_layer_setting_enabled.value)
		ui.widget->updateTemperatureLayerListWidget(&m_tempSliceProfile->at(selectedCartridgeIndex));
	//SetConnectControlforMaterialIndex();
}

void ProfileSettingMultiDlg::tabWidget_profile_currentChanged(int _index)
{
	if (_index == tabWidget::tab_support || _index == tabWidget::tab_bed_adhesion || _index == tabWidget::tab_wipe_tower)
	{
		//ui.groupBox_cartridge_list->setEnabled(false);
		ui.label_cartridge_list->setEnabled(false);
		ui.comboBox_cartridge_list->setEnabled(false);
		ui.comboBox_cartridge_list->setStyleSheet("");
	}
	else
	{
		//ui.groupBox_cartridge_list->setEnabled(true);
		ui.label_cartridge_list->setEnabled(true);
		ui.comboBox_cartridge_list->setEnabled(true);
		ui.comboBox_cartridge_list->styleSheet().clear();
		
		//tab �׵θ� ���� ����
		QString current_cartridge_color = m_colorList.at(selectedCartridgeIndex);
		ui.comboBox_cartridge_list->setStyleSheet("QComboBox { background-color: rgb(" + current_cartridge_color + "); selection-background-color: rgb(" + current_cartridge_color + "); selection-color: black;}");
	}
}


void ProfileSettingMultiDlg::saveTempProfileToCartridgeProfile()
{
	*m_sliceProfile_common = *m_tempSliceProfile_common;

	if (!(m_tempSliceProfile->size() == m_sliceProfile_multi->size()))
		return;

	for (int i = 0; i < m_tempSliceProfile->size(); i++)
	{
		m_sliceProfile_multi->at(i) = m_tempSliceProfile->at(i);
		m_sliceProfile_multi->at(i).rearrangeTemperatureLayerList();
	}
}

void ProfileSettingMultiDlg::saveCartirdgeProfileToTempProfile()
{
	if (!(m_tempSliceProfile->size() == m_sliceProfile_multi->size()))
		return;

	for (int i = 0; i < m_sliceProfile_multi->size(); i++)
	{
		m_tempSliceProfile->at(i) = m_sliceProfile_multi->at(i);
	}
}

void ProfileSettingMultiDlg::resetCartridgeUI(int cartridgeCount)
{
	QStringList strList;

	for (int i = 0; i < cartridgeCount; i++)
	{
		strList.push_back(CustomTranslate::tr("Cartridge") + QString("(%1)").arg(i + 1));
	}
	//ui.listWidget_cartridge_list->clear();
	//ui.listWidget_cartridge_list->addItems(strList);
	ui.comboBox_cartridge_list->clear();
	ui.comboBox_cartridge_list->addItems(strList);

	//expanded mode�� ��, expanded cartridge �ܿ� ������ cartridge index�� disabled..//
	if (Profile::machineProfile.machine_expanded_print_mode.value)
	{
		for (int i = 0; i < cartridgeCount; ++i)
		{
			if (i == Profile::machineProfile.machine_expanded_print_cartridgeIndex.value)
				continue;

			//ui.listWidget_cartridge_list->item(i)->setFlags(ui.listWidget_cartridge_list->item(i)->flags() & ~Qt::ItemIsEnabled);
		}
	}
}

void ProfileSettingMultiDlg::showMessage(QString pMessage)
{
	CommonDialog comDlg(this, pMessage, CommonDialog::Warning);
}

void ProfileSettingMultiDlg::setOriginalValue()
{
	for (int j = 0; j < m_tempSliceProfile->size(); j++)
	{
		setOriginalValue(j);
	}

	ProfileControl::generateCommonProfileFromSliceProfile(
		m_originalProfile, 
		m_originalProfile_common, 
		m_tempSliceProfile_common->platform_adhesion.value, 
		m_tempSliceProfile_common->adhesion_cartridge_index.value, 
		m_tempSliceProfile_common->support_placement.value, 
		m_tempSliceProfile_common->support_main_cartridge_index.value);
}

void ProfileSettingMultiDlg::setOriginalValue(int idx)
{
	int l_profileNameIdx = -1;
	for (int i = 0; i < m_profileList.size(); i++)
	{
		if (m_profileList.at(i).name == m_tempSliceProfile->at(idx).profile_name.value)
		{
			l_profileNameIdx = i;
			break;
		}
	}

	if (l_profileNameIdx == -1)
	{
		//�ش� ���������� ����Ʈ�� ���� ��� ������ Standard ���������� ã�� ������ ã�� �� ���� ��� Unknown ���� ó��
		QString l_material = m_tempSliceProfile->at(idx).filament_material.value;
		if (!m_originalProfile->at(idx).loadSliceProfile(Generals::qstringTowchar_t(Profile::profilePath + "profile_setting_value_" + Generals::getMaterialShortName(l_material) + ".ini")))
		{
			m_originalProfile->at(idx) = m_sliceProfile_multi->at(idx);
		}
		else
		{
			m_tempSliceProfile->at(idx).profile_name = m_originalProfile->at(idx).profile_name;
		}
	}
	else
	{
		QString l_fileName = m_profileList.at(l_profileNameIdx).fileLocation;
		QString l_profileDir;
		if (m_profileList.at(l_profileNameIdx).custom)
			l_profileDir = Profile::customProfilePath;
		else
			l_profileDir = Profile::profilePath;

		bool rtn = m_originalProfile->at(idx).loadSliceProfile(Generals::qstringTowchar_t(l_profileDir + l_fileName));
	}
}

void ProfileSettingMultiDlg::showCustomProfileEditor()
{
	//���� ȭ�鿡 �ִ� ���� profile�� ������ �Ŀ� ����
	ui.widget->saveProfileValue(&m_tempSliceProfile->at(selectedCartridgeIndex));
	//ui.widget_common->saveProfileCommonValue();
	ui.widget->saveProfileCommonValue();

	CustomProfileEditor *customProfileEditor = new CustomProfileEditor(this);
	customProfileEditor->loadProfile(
		m_profileList.at(m_selectedProfileIdx).fileLocation,
		m_profileList.at(m_selectedProfileIdx).name,
		&m_tempSliceProfile->at(selectedCartridgeIndex),
		m_tempSliceProfile_common,
		&m_originalProfile->at(selectedCartridgeIndex),
		m_originalProfile_common);

	customProfileEditor->show();

	connect(customProfileEditor, SIGNAL(edit_finished_sig(SliceProfile*, SliceProfileForCommon*)), this, SLOT(resetOriginalValue(SliceProfile*, SliceProfileForCommon*)));
	connect(customProfileEditor, SIGNAL(edit_ok_sig(SliceProfile*, SliceProfileForCommon*)), this, SLOT(resetUIValue(SliceProfile*, SliceProfileForCommon*)));
	//connect(customProfileEditor, SIGNAL(destroyed(QObject *)), this, SLOT(comboBox_profileNameList_currentIndexChanged()));
}

