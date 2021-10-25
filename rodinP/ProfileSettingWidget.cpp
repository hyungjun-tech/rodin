#include "stdafx.h"
#include "ProfileSettingWidget.h"
#include "SliceProfile.h"
#include "ProfileData.h"
#include "ProfileControl.h"
#include "ProfileToConfig.h"
#include "CartridgeInfo.h"

ProfileSettingWidget::ProfileSettingWidget(QWidget *parent)
	: QWidget(parent)
	, m_selectedCartridgeIndex(0)
	, m_doNotChange(true)
{
	ui.setupUi(this);
	setUI();
	setLinkedUI();
	setConnection();
}

ProfileSettingWidget::~ProfileSettingWidget()
{

}

void ProfileSettingWidget::setParameter(std::vector<SliceProfile> *sliceProfile_multi, SliceProfileForCommon *sliceProfile_common, std::vector<SliceProfile> *originalProfile_multi, SliceProfileForCommon *originalProfile_common, std::vector<int> usedCartIdx, int selectedCartridgeIndex)
{
	if (Profile::machineProfile.machine_expanded_print_mode.value)
	{
		sliceProfile_common->support_main_cartridge_index.value = Profile::machineProfile.machine_expanded_print_cartridgeIndex.value;
		sliceProfile_common->adhesion_cartridge_index.value = Profile::machineProfile.machine_expanded_print_cartridgeIndex.value;
	}

	m_pre_support_cart_idx = sliceProfile_common->support_main_cartridge_index.value;
	m_pre_adhesion_cart_idx = sliceProfile_common->adhesion_cartridge_index.value;

	m_sliceProfile = &(sliceProfile_multi->at(selectedCartridgeIndex));
	m_sliceProfile_multi = sliceProfile_multi;
	m_sliceProfile_common = sliceProfile_common;
	m_originalProfile_multi = originalProfile_multi;
	m_originalProfile_common = originalProfile_common;
	m_selectedCartridgeIndex = selectedCartridgeIndex;
	m_usedCartIdx = usedCartIdx;

	setUIcontrolByMachineModel();
	setTempUI();

	loadProfileValue(sliceProfile_multi->at(selectedCartridgeIndex));
	loadProfileCommonValue(*sliceProfile_common);

	setConnectControlforApplyButton();
}

//for custom profile..//
void ProfileSettingWidget::setParameter(SliceProfile *sliceProfile, SliceProfileForCommon *sliceProfile_common, std::vector<int> usedCartIdx)
{
	m_pre_support_cart_idx = sliceProfile_common->support_main_cartridge_index.value;
	m_pre_adhesion_cart_idx = sliceProfile_common->adhesion_cartridge_index.value;

	m_sliceProfile = sliceProfile;
	m_sliceProfile_multi = new std::vector<SliceProfile>;
	m_sliceProfile_multi->push_back(*sliceProfile);

	m_sliceProfile_common = sliceProfile_common;

	m_usedCartIdx = usedCartIdx;

	setUIcontrolByMachineModel();
	setTempUI();

	loadProfileValue(*sliceProfile);
	loadProfileCommonValue(*sliceProfile_common);

	setConnectControlforApplyButton();
}

void ProfileSettingWidget::setProfilesLink(std::vector<SliceProfile> *sliceProfile_multi, SliceProfileForCommon *sliceProfile_common, int seletedCartridgeIndex)
{
	m_sliceProfile = &(sliceProfile_multi->at(seletedCartridgeIndex));
	m_sliceProfile_multi = sliceProfile_multi;
	m_sliceProfile_common = sliceProfile_common;
}

void ProfileSettingWidget::setUI()
{
	//화씨 섭씨 전환이 필요한 항목들
	ui.spinBox_print_temperature->setUsingTemperatureTransfer(true);
	ui.spinBox_print_bed_temperature->setUsingTemperatureTransfer(true);
	ui.spinBox_operating_standby_temperature->setUsingTemperatureTransfer(true);
	ui.spinBox_initial_standby_temperature->setUsingTemperatureTransfer(true);
	ui.spinBox_raft_base_temperature->setUsingTemperatureTransfer(true);
	ui.spinBox_raft_interface_temperature->setUsingTemperatureTransfer(true);
	ui.spinBox_raft_surface_initial_temperature->setUsingTemperatureTransfer(true);
	ui.spinBox_raft_surface_last_temperature->setUsingTemperatureTransfer(true);
	ui.spinBox_temperature_setpoint_temperature->setUsingTemperatureTransfer(true);

	//Original Value 저장하지 않는 항목들 (노란색 표시 없음)
	ui.comboBox_profileNameList->setUsingOriginalValue(false);
	ui.comboBox_Material->setUsingOriginalValue(false);
	ui.comboBox_support->setUsingOriginalValue(false);
	ui.comboBox_support_main_cartridge_index->setUsingOriginalValue(false);
	ui.comboBox_platform_adhesion->setUsingOriginalValue(false);
	ui.comboBox_adhesion_cart_index->setUsingOriginalValue(false);
	ui.comboBox_wipetower_outer_cartridge_index->setUsingOriginalValue(false);
	ui.comboBox_wipetower_inner_cartridge_index->setUsingOriginalValue(false);
	ui.spinBox_temperature_setpoint_layer_number->setUsingOriginalValue(false);
	ui.spinBox_temperature_setpoint_temperature->setUsingOriginalValue(false);

	//데이타가 변경되고 editingFinished 되면 signal_dataChanged 시그널 발생.
	//ui.doubleSpinBox_filament_diameter->setChangeTracking(true);
	ui.spinBox_print_temperature->setChangeTracking(true);
	ui.doubleSpinBox_initial_layer_speed->setChangeTracking(true);
	ui.doubleSpinBox_wipetower_outer_size->setChangeTracking(true);
	ui.doubleSpinBox_wipetower_outer_wall_thickness->setChangeTracking(true);
	ui.doubleSpinBox_wipetower_outer_inner_gap->setChangeTracking(true);
	ui.doubleSpinBox_wipetower_base_size->setChangeTracking(true);
	ui.doubleSpinBox_raft_airgap_initial_layer->setChangeTracking(true);



	if (!(Profile::machineProfile.has_heated_bed.value))
	{
		ui.spinBox_print_bed_temperature->setEnabled(false);
		ui.label_bed_temperature->setEnabled(false);
	}
	else
	{
		ui.spinBox_print_bed_temperature->setEnabled(true);
		ui.label_bed_temperature->setEnabled(true);
	}
	ui.comboBox_Material->clear();
	ui.comboBox_Material->addItems(Profile::machineProfile.available_material_list);

	///////////////
	QStringList cartridgeIndexList;
	for (int i = 0; i < Profile::machineProfile.extruder_count.value; i++)
	{
		cartridgeIndexList.push_back(CustomTranslate::tr("Cartridge") + QString("(%1)").arg(i + 1));
	}
	QString comboBox_style = "QComboBox { background-color: rgb(105, 210, 0); selection-background-color: rgb(105, 210, 0); selection-color: black;}";
	ui.comboBox_support_main_cartridge_index->clear();
	ui.comboBox_support_main_cartridge_index->addItems(cartridgeIndexList);
	ui.comboBox_support_main_cartridge_index->setStyleSheet(comboBox_style);
	//ui.comboBox_support_main_cartridge_index->setFocusPolicy(Qt::FocusPolicy::NoFocus);


	ui.comboBox_adhesion_cart_index->clear();
	ui.comboBox_adhesion_cart_index->addItems(cartridgeIndexList);
	ui.comboBox_adhesion_cart_index->setStyleSheet(comboBox_style);
	//ui.comboBox_adhesion_cart_index->setFocusPolicy(Qt::FocusPolicy::NoFocus);


	ui.comboBox_wipetower_outer_cartridge_index->setStyleSheet(comboBox_style);
	ui.comboBox_wipetower_inner_cartridge_index->setStyleSheet(comboBox_style);
	//ui.comboBox_wipetower_outer_cartridge_index->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	//ui.comboBox_wipetower_inner_cartridge_index->setFocusPolicy(Qt::FocusPolicy::NoFocus);



	///////////////
	ui.tableWidget_temperature_layer_list->setColumnCount(2);
	ui.tableWidget_temperature_layer_list->setColumnWidth(0, 70);
	ui.tableWidget_temperature_layer_list->setColumnWidth(1, 115);
	ui.tableWidget_temperature_layer_list->setSelectionBehavior(QAbstractItemView::SelectRows);


	//////////////
	if (Profile::machineProfile.machine_expanded_print_mode.value)
	{
		//expanded mode//
		//ui.comboBox_support_main_cartridge_index->setCurrentIndex(Profile::machineProfile.machine_expanded_print_cartridgeIndex.value);
		ui.comboBox_support_main_cartridge_index->setEnabled(false);

		//ui.comboBox_adhesion_cart_index->setCurrentIndex(Profile::machineProfile.machine_expanded_print_cartridgeIndex.value);
		ui.comboBox_adhesion_cart_index->setEnabled(false);
	}

	//checkBox_layer0_temperature_enabled_toggled(false);
	checkBox_wipetower_enable_toggled(false);
	checkBox_standby_temperature_enabled_toggled(false);
	checkBox_raft_temperature_control_toggled(false);
	checkBox_raft_inset_toggled(false);
	checkBox_support_interface_enabled_toggled(false);


	//filament diameter는 일단 disable
	//해당 값이 slice profile 속성이 맞는지 의문.....노즐에 속한 속성인데 machine정보에서 관리하는게 낫지 않을까....
	//ui.doubleSpinBox_filament_diameter->setEnabled(false);

	ui.tabWidget_profile->setCurrentIndex(0);

	//skin top, bottom removal width --> unvisible 처리..//
	ui.label_top_skin_removal_width->setVisible(false);
	ui.doubleSpinBox_top_skin_removal_width->setVisible(false);
	ui.label_bottom_skin_removal_width->setVisible(false);
	ui.doubleSpinBox_bottom_skin_removal_width->setVisible(false);
	//code 명시화..//왜 ui에서 안됨?
	ui.tableWidget_temperature_layer_list->horizontalHeader()->setVisible(true);

	//temperature type은 일단 사용하지 않는 것으로..cartridge만 사용하도록 함..//
	checkBox_temperature_layer_setting_enabled(false);

	//flow control//
	checkBox_overall_flow_control_enabled(Profile::sliceProfile.at(0).overall_flow_control_enabled.value);

	//////////////////////////////////////////////////////
	//test ui//
	ui.label_skirt_width_factor->setVisible(false);
	ui.spinBox_skirt_width_factor->setVisible(false);

	ui.label_skirt_flow->setVisible(false);
	ui.spinBox_skirt_flow->setVisible(false);

	ui.label_skirt_speed->setVisible(false);
	ui.doubleSpinBox_skirt_speed->setVisible(false);

	ui.label_brim_speed->setVisible(false);
	ui.doubleSpinBox_brim_speed->setVisible(false);

	ui.label_brim_width_factor->setVisible(false);
	ui.spinBox_brim_width_factor->setVisible(false);

	ui.label_brim_flow->setVisible(false);
	ui.spinBox_brim_flow->setVisible(false);

}

void ProfileSettingWidget::setLinkedUI()
{
	//Quality//
	ui.label_layer_height->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Layer Height (mm)") + "</p>");
	ui.label_initial_layer_height->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Initial Layer Height (mm)") + "</p>");
	ui.label_initial_layer_width_factor->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Initial Layer Line Width (%)") + "</p>");
	ui.label_z_offset_raft->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Z Offset (RAFT) (mm)") + "</p>");
	ui.label_z_offset_except_raft->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Z Offset (without RAFT) (mm)") + "</p>");

	//Nozzle Swith//
	ui.label_toolchange_lowering_bed->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Lowering Bed (mm)") + "</p>");

	//Retraction//
	ui.label_retraction_combing->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Retraction Combing") + "</p>");

	//Shape Error Correction//
	ui.label_spiralize_common->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Spiralize Outer Contour") + "</p>");
	ui.label_simple_mode_common->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Only Follow Mesh Surface") + "</p>");
	ui.label_fix_horrible_union_all_type_a_common->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Combination Mode A") + "</p>");
	ui.label_fix_horrible_union_all_type_b_common->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Combination Mode B") + "</p>");
	ui.label_fix_horrible_use_open_bits_common->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Close Open Face") + "</p>");
	ui.label_fix_horrible_extensive_stitching_common->setText("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Extensive Stitching") + "</p>");
}

void ProfileSettingWidget::setTempUI()
{
	QString unitStr;
	if (Generals::temperatureUnit == "C")
		unitStr = QString::fromWCharArray(L" (\u2103)");
	else if (Generals::temperatureUnit == "F")
		unitStr = QString::fromWCharArray(L" (\u2109)");

	ui.label_nozzle_temperature->setText(tr("Nozzle Temperature") + unitStr);

	QString tempStr("<p><img src=\":/rodinP/Resources/linked.png\"/>" + tr("Bed Temperature") + unitStr + "</p>");
	ui.label_bed_temperature->setText(tempStr);

	ui.label_operating_standby_temperature->setText(tr("Operating Standby Temperature") + unitStr);
	ui.label_initial_standby_temperature->setText(tr("Initial Standby Temperature") + unitStr);
	//ui.label_layer0_temperature->setText(tr("Initial Layer Temperature") + unitStr);

	ui.label_raft_base_temperature->setText(tr("Base Temperature") + unitStr);
	ui.label_raft_interface_temperature->setText(tr("Interface Temperature") + unitStr);
	ui.label_raft_surface_initial_temperature->setText(tr("Surface Initial Temperature") + unitStr);
	ui.label_raft_surface_last_temperature->setText(tr("Surface Last Temperature") + unitStr);

	ui.label_temperature_setpoint_temperature->setText(tr("Temperature") + unitStr);
}

void ProfileSettingWidget::setTabStyle(QString _cartrige_color, QString _common_color)
{
	QString preTabStyle = "";
	QString preTabStyle_common = "";
	QString tabStyle = "{\nbackground-color: rgb(255, 255, 255);\nborder-style: solid;\nborder-width: 3px;\nborder-color: rgb(" + _cartrige_color + ");\n}";
	QString tabStyle_common = "{\nbackground-color: rgb(255, 255, 255);\nborder-style: solid;\nborder-width: 3px;\nborder-color: rgb(" + _common_color + ");\n}";

	for (int i = 0; i < ui.tabWidget_profile->count(); i++)
	{
		if (ui.tabWidget_profile->widget(i)->objectName() == "tab_bed_adhesion" ||
			ui.tabWidget_profile->widget(i)->objectName() == "tab_support" ||
			ui.tabWidget_profile->widget(i)->objectName() == "tab_wipe_tower")
		{
			if (preTabStyle_common == "") preTabStyle_common += "#";
			else preTabStyle_common += ",#";
			preTabStyle_common += ui.tabWidget_profile->widget(i)->objectName();
		}
		else
		{
			if (preTabStyle == "") preTabStyle += "#";
			else preTabStyle += ",#";
			preTabStyle += ui.tabWidget_profile->widget(i)->objectName();
		}
	}

	ui.tabWidget_profile->setStyleSheet(preTabStyle + tabStyle + "\n" + preTabStyle_common + tabStyle_common);
}

void ProfileSettingWidget::setConnection()
{
	//connect(ui.checkBox_layer0_temperature_enabled, SIGNAL(toggled(bool)), this, SLOT(checkBox_layer0_temperature_enabled_toggled(bool)));
	connect(ui.checkBox_wipetower_enable, SIGNAL(toggled(bool)), this, SLOT(checkBox_wipetower_enable_toggled(bool)));
	connect(ui.checkBox_standby_temperature_enabled, SIGNAL(toggled(bool)), this, SLOT(checkBox_standby_temperature_enabled_toggled(bool)));
	connect(ui.checkBox_preheat_enabled, SIGNAL(toggled(bool)), this, SLOT(checkBox_preheat_enabled_toggled(bool)));

	connect(ui.comboBox_profileNameList, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_profileNameList_currentIndexChanged(int)));
	connect(ui.comboBox_profileNameList, SIGNAL(currentIndexChanged(int)), this, SIGNAL(comboBox_profileNameList_change_forUI(int)));
	connect(ui.pushButton_edit, SIGNAL(clicked()), this, SIGNAL(pushButton_edit_clicked_sig()));
	connect(ui.pushButton_reset, SIGNAL(clicked()), this, SIGNAL(pushButton_reset_clicked_sig()));
	connect(ui.comboBox_wipetower_outer_cartridge_index, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_wipetower_cartridgeIndex_currentIndexChanged(int)));
	connect(ui.comboBox_wipetower_inner_cartridge_index, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_wipetower_cartridgeIndex_currentIndexChanged(int)));
	connect(this, SIGNAL(pushButton_reset_clicked_sig()), this, SLOT(pushButton_reset_clicked()));

	connect(ui.checkBox_raft_inset_enabled, SIGNAL(toggled(bool)), this, SLOT(checkBox_raft_inset_toggled(bool)));
	connect(ui.checkBox_support_interface_enable, SIGNAL(toggled(bool)), this, SLOT(checkBox_support_interface_enabled_toggled(bool)));
	connect(ui.checkBox_raft_temperature_control, SIGNAL(toggled(bool)), this, SLOT(checkBox_raft_temperature_control_toggled(bool)));
	//connect(ui.comboBox_support, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_support_currentIndexChanged_forUI(int)));
	//connect(ui.comboBox_platform_adhesion, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_platform_adhesion_currentIndexChanged_forUI(int)));

	//화면 최초 로드시 메세지 뜨는 것을 방지하기 위해 m_donotchange로 control
	connect(ui.comboBox_platform_adhesion, SIGNAL(currentIndexChanged(int)), this, SLOT(warningMessageBedAdhesionType(int)));
	connect(ui.comboBox_support, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_support_currentIndexChanged(int)));
	connect(ui.comboBox_platform_adhesion, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_platform_adhesion_currentIndexChanged(int)));
	connect(ui.comboBox_adhesion_cart_index, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_adhesion_cart_index_currentIndexChanged(int)));
	connect(ui.comboBox_support_main_cartridge_index, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_support_cart_index_currentIndexChanged(int)));

	connect(ui.checkBox_temperature_layer_setting_enabled, SIGNAL(toggled(bool)), this, SLOT(checkBox_temperature_layer_setting_enabled(bool)));
	//connect(ui.pushButton_perlayer_temperature_setting_add, SIGNAL(clicked()), this, SIGNAL(pushButton_perlayer_temperature_setting_add_clicked_sig()));
	connect(ui.pushButton_temperature_setpoint_add, SIGNAL(clicked()), this, SLOT(addTemperatureLayerSetPoint()));
	connect(ui.pushButton_temperature_setpoint_delete, SIGNAL(clicked()), this, SLOT(deleteTemperatureLayerSetPoint()));
	connect(ui.tableWidget_temperature_layer_list, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(tableWidget_temperature_layer_list_itemChanged(QTableWidgetItem *)));
	connect(ui.tabWidget_profile, SIGNAL(currentChanged(int)), this, SIGNAL(tabWidget_profile_current_change_signal(int)));

	connect(ui.checkBox_overall_flow_control_enabled, SIGNAL(toggled(bool)), this, SLOT(checkBox_overall_flow_control_enabled(bool)));

	//UI sync//
	//connect(ui.spinBox_flow, SIGNAL(valueChanged(int)), this, SLOT(setFlowCurrentValue(int)));
	//connect(ui.doubleSpinBox_print_speed, SIGNAL(valueChanged(double)), this, SLOT(setSpeedCurrentValue(double)));
}

void ProfileSettingWidget::setConnectControlforApplyButton()
{
	//checkBox//
	Q_FOREACH(QCheckBox *checkBoxs, findChildren<QCheckBox*>())
	{
		connect(checkBoxs, SIGNAL(stateChanged(int)), this, SLOT(setEnableApplyButton()));
	}

	//comboBox//
	Q_FOREACH(QComboBox *comboBoxs, findChildren<QComboBox*>())
	{
		connect(comboBoxs, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableApplyButton()));
	}

	//DoubleSpinBox//
	Q_FOREACH(CustomDoubleSpinBox *doubleSpinBoxs, findChildren<CustomDoubleSpinBox*>())
	{
		connect(doubleSpinBoxs, SIGNAL(valueChanged(double)), this, SLOT(setEnableApplyButton()));
		connect(doubleSpinBoxs, SIGNAL(signal_dataChanged()), this, SLOT(checkMessage()));
	}

	//SpinBox//
	Q_FOREACH(CustomSpinBox *spinBoxs, findChildren<CustomSpinBox*>())
	{
		connect(spinBoxs, SIGNAL(valueChanged(int)), this, SLOT(setEnableApplyButton()));
		connect(spinBoxs, SIGNAL(signal_dataChanged()), this, SLOT(checkMessage()));
	}

	disconnect(ui.spinBox_temperature_setpoint_layer_number, SIGNAL(valueChanged(int)), this, SLOT(setEnableApplyButton()));
	disconnect(ui.spinBox_temperature_setpoint_temperature, SIGNAL(valueChanged(int)), this, SLOT(setEnableApplyButton()));
}

void ProfileSettingWidget::setEnableApplyButton()
{
	emit enableApplyButton(true);
}

void ProfileSettingWidget::setDisableApplyButton()
{
	emit enableApplyButton(false);
}

void ProfileSettingWidget::pushButton_reset_clicked()
{
	//checkBox//
	Q_FOREACH(CustomCheckBox *checkBoxs, findChildren<CustomCheckBox*>())
	{
		checkBoxs->resetValue();

		if (checkBoxs->objectName() == "checkBox_temperature_layer_setting_enabled")
			clearTemperatureLayerListWidget();
	}

	//comboBox//
	Q_FOREACH(CustomComboBox *comboBoxs, findChildren<CustomComboBox*>())
	{
		comboBoxs->resetValue();
	}
	//comboBox//
	ui.comboBox_Material->resetValue();

	//DoubleSpinBox//
	Q_FOREACH(CustomDoubleSpinBox *doubleSpinBoxs, findChildren<CustomDoubleSpinBox*>())
	{
		doubleSpinBoxs->resetValue();
	}

	//SpinBox//
	Q_FOREACH(CustomSpinBox *spinBoxs, findChildren<CustomSpinBox*>())
	{
		spinBoxs->resetValue();
	}

	//value update..//
	showWipeTowerInnerSize();
}
//

void ProfileSettingWidget::showWipeTowerInnerSize()
{
	calculateWipeTowerSize();

	ui.lineEdit_wipetower_inner_size->setText(QString::number(this->wipeTowerInnerSize_calculated));

}

void ProfileSettingWidget::checkMessage()
{
	if (!m_doNotChange)
	{
		QWidget* widget = dynamic_cast<QWidget*>(sender());
		widget->blockSignals(true);

		//if (widget == ui.doubleSpinBox_filament_diameter)
		//{
		//	CustomDoubleSpinBox* filamentDiameterWidget = dynamic_cast<CustomDoubleSpinBox*>(sender());
		//	double originalValue = filamentDiameterWidget->getOriginalValue();
		//	if (originalValue != ui.doubleSpinBox_filament_diameter->value())
		//	{
		//		emit warningMessage(MessageAlert::tr("default_value_setting_alert"));
		//	}
		//}
		if (widget == ui.doubleSpinBox_raft_airgap_initial_layer)
		{
			QString l_material = m_sliceProfile_multi->at(ui.comboBox_adhesion_cart_index->currentIndex()).filament_material.value;
			if (l_material == "PLA")
			{
				if (ui.doubleSpinBox_raft_airgap_initial_layer->value() > 0.5)
				{
					emit warningMessage(MessageAlert::tr("come_off_bed_alert"));
				}
			}
			else if (l_material == "ABS")
			{
				if (ui.doubleSpinBox_raft_airgap_initial_layer->value() > 0.2)
				{
					emit warningMessage(MessageAlert::tr("come_off_bed_alert"));
				}
			}
		}
		//else if (ui_spinBox == ui.doubleSpinBox_print_temperature)
		//{
		//	double printTempValue = ui.doubleSpinBox_print_temperature->value();
		//	if (originalValue != printTempValue)
		//	{
		//		QString l_material = ui.comboBox_Material->currentText();

		//		if (Generals::temperatureUnit == "F")
		//			printTempValue = Generals::convertTemperatureUnitFtoC(printTempValue);

		//		bool b_warningDlg = false;
		//		if (l_material == "PLA" && (printTempValue < 190 || printTempValue > 210)) b_warningDlg = true;
		//		else if (l_material == "ABS" && (printTempValue < 220 || printTempValue > 240)) b_warningDlg = true;
		//		//else if (l_material == "FLEXIBLE" && (printTempValue < 215 || printTempValue > 235)) b_warningDlg = true;
		//		else if (l_material == "PVA" && (printTempValue < 225 || printTempValue > 240)) b_warningDlg = true;

		//		if (b_warningDlg) emit warningMessage(tr("SettingMsgB"));
		//	}
		//}
		//else if (ui_spinBox == ui.doubleSpinBox_bottom_layer_speed)
		//{
		//	if (originalValue != ui.doubleSpinBox_bottom_layer_speed->value())
		//	{
		//		if (ui.doubleSpinBox_bottom_layer_speed->value() > 30)
		//		{
		//			emit warningMessage(tr("come_off_bed_alert"));
		//		}
		//	}
		//}
		else if (widget == ui.doubleSpinBox_wipetower_base_size
			|| widget == ui.doubleSpinBox_wipetower_outer_size
			|| widget == ui.doubleSpinBox_wipetower_outer_wall_thickness
			|| widget == ui.doubleSpinBox_wipetower_outer_inner_gap)
		{
			if (validateWipeTowerSize())
			{
				showWipeTowerInnerSize();
			}
		}
		//else if (ui_spinBox == ui.doubleSpinBox_wipetower_inner_size)
		//{
		//	if (!validateWipeTowerSize())
		//	{
		//		emit warningMessage(tr(profileValue_error.errorMessage.toStdString().c_str()));
		//	}
		//}

		widget->blockSignals(false);
	}
}

bool ProfileSettingWidget::validateWipeTowerSize()
{
	const double wipeTowerBaseSize = ui.doubleSpinBox_wipetower_base_size->value() * 1000;
	const double wipeTowerOuterSize = ui.doubleSpinBox_wipetower_outer_size->value() * 1000;
	const double wipeTowerOuterWallThickness = ui.doubleSpinBox_wipetower_outer_wall_thickness->value() * 1000;
	const double wipeTowerOuterInnerGap = ui.doubleSpinBox_wipetower_outer_inner_gap->value() * 1000;

	///i) base > outer
	int extrusionWidth = MM2INT(ProfileToConfig::calculateEdgeWidth(m_sliceProfile, m_sliceProfile->wipe_tower_outer_cartridge_index.value));
	int outlineOffset = wipeTowerOuterSize * 0.5 + extrusionWidth * 5;

	///ii) innersize > 0
	const double wipeTowerInnerSize_calculated = wipeTowerOuterSize - 2 * wipeTowerOuterWallThickness - 2 * wipeTowerOuterInnerGap;

	if (wipeTowerBaseSize < (outlineOffset * 2) ||
		wipeTowerInnerSize_calculated <= 0)
	{
		//warning..//
		emit warningMessage(MessageAlert::tr("invalid_value_for_wipe_tower"));
		return false;
	}

	return true;
}

void ProfileSettingWidget::calculateWipeTowerSize()
{
	double wipeTowerOuterSize = ui.doubleSpinBox_wipetower_outer_size->value();
	double wipeTowerOuterWallThickness = ui.doubleSpinBox_wipetower_outer_wall_thickness->value();
	double wipeTowerOuterInnerGap = ui.doubleSpinBox_wipetower_outer_inner_gap->value();

	this->wipeTowerInnerSize_calculated = wipeTowerOuterSize - 2 * wipeTowerOuterWallThickness - 2 * wipeTowerOuterInnerGap;
}

void ProfileSettingWidget::setUIcontrolByMachineModel()
{
	//cooling groupbox enabled//
	if (Profile::machineProfile.cooling_control_enabled.value) ui.groupBox_cool->setEnabled(true);
	else ui.groupBox_cool->setEnabled(false);

	//UI control by cartridge count//
	if (Profile::machineProfile.extruder_count.value > 1 &&
		!Profile::machineProfile.replication_print.value &&
		!Profile::machineProfile.machine_expanded_print_mode.value) //multi nozzle condition//
	{
		if (Generals::isLayerColorModeOn())
		{
			//support//
			ui.comboBox_support_main_cartridge_index->setVisible(false);
			ui.label_support_cart_index->setVisible(false);

			//adhesion//
			if (ui.comboBox_platform_adhesion->currentIndex() == Generals::PlatformAdhesion::Raft)
			{
				ui.comboBox_adhesion_cart_index->setVisible(true);
				ui.label_adhesion_cart_index->setVisible(true);
			}
			else
			{
				ui.comboBox_adhesion_cart_index->setVisible(false);
				ui.label_adhesion_cart_index->setVisible(false);
			}

			//Multi Nozzle tab//
			ui.tabWidget_profile->setTabEnabled(6, true);

			//wipe tower//
			m_sliceProfile_common->wipe_tower_enabled.value = false;
			ui.tabWidget_profile->setTabEnabled(7, false);

			//preheat//
			ui.line_preheat->setVisible(false);
			ui.label_preheat_enabled->setVisible(false);
			ui.checkBox_preheat_enabled->setVisible(false);
			ui.label_preheat_threshold_time->setVisible(false);
			ui.doubleSpinBox_preheat_threshold_time->setVisible(false);
		}
		else if (Generals::isReplicationUIMode())
		{
			//cartridge를 1번만 사용해야 함..//

			//support//
			ui.comboBox_support_main_cartridge_index->setCurrentIndex(0);
			ui.comboBox_support_main_cartridge_index->setEnabled(false);
			ui.label_support_cart_index->setEnabled(false);

			//adhesion//
			ui.comboBox_adhesion_cart_index->setCurrentIndex(0);
			ui.comboBox_adhesion_cart_index->setEnabled(false);
			ui.label_adhesion_cart_index->setEnabled(false);

			//Multi Nozzle tab//
			ui.tabWidget_profile->setTabEnabled(6, false);

			//wipe tower//
			m_sliceProfile_common->wipe_tower_enabled.value = false;
			ui.tabWidget_profile->setTabEnabled(7, false);

			//preheat//
			ui.line_preheat->setVisible(false);
			ui.label_preheat_enabled->setVisible(false);
			ui.checkBox_preheat_enabled->setVisible(false);
			ui.label_preheat_threshold_time->setVisible(false);
			ui.doubleSpinBox_preheat_threshold_time->setVisible(false);

		}
		else
		{
			//general multi nozzle condition//

			//support//
			if (ui.comboBox_support->currentIndex() == Generals::SupportPlacement::SupportNone)
			{
				ui.comboBox_support_main_cartridge_index->setVisible(false);
				ui.label_support_cart_index->setVisible(false);
			}
			else
			{
				ui.comboBox_support_main_cartridge_index->setVisible(true);
				ui.label_support_cart_index->setVisible(true);
			}

			//adhesion//
			if (ui.comboBox_platform_adhesion->currentIndex() == Generals::PlatformAdhesion::Raft)
			{
				ui.comboBox_adhesion_cart_index->setVisible(true);
				ui.label_adhesion_cart_index->setVisible(true);
			}
			else
			{
				ui.comboBox_adhesion_cart_index->setVisible(false);
				ui.label_adhesion_cart_index->setVisible(false);
			}

			//Multi Nozzle tab//
			ui.tabWidget_profile->setTabEnabled(6, true);

			//wipe tower//
			ui.tabWidget_profile->setTabEnabled(7, true);

			//preheat//
			ui.line_preheat->setVisible(true);
			ui.label_preheat_enabled->setVisible(true);
			ui.checkBox_preheat_enabled->setVisible(true);
			ui.label_preheat_threshold_time->setVisible(true);
			ui.doubleSpinBox_preheat_threshold_time->setVisible(true);
		}
	}
	else //single nozzle condition//
	{
		//support//
		ui.comboBox_support_main_cartridge_index->setVisible(false);
		ui.label_support_cart_index->setVisible(false);

		//adhesion//
		ui.comboBox_adhesion_cart_index->setVisible(false);
		ui.label_adhesion_cart_index->setVisible(false);

		//Multi Nozzle tab//
		ui.tabWidget_profile->setTabEnabled(6, false);

		//Wipe Tower tab//
		m_sliceProfile_common->wipe_tower_enabled.value = false;
		ui.tabWidget_profile->setTabEnabled(7, false);
	}

	ui.checkBox_raft_temperature_control->setEnabled(Profile::machineProfile.has_heated_bed.value);
	ui.label_raft_temperature_control->setEnabled(Profile::machineProfile.has_heated_bed.value);
}

void ProfileSettingWidget::loadOriginalValue(SliceProfile sliceProfile)
{
	//ui.comboBox_Material->setOriginalValue(sliceProfile.filament_material.value);

	//Quality//
	ui.spinBox_wall_width_factor->setOriginalValue(sliceProfile.wall_width_factor.value);
	ui.spinBox_infill_width_factor->setOriginalValue(sliceProfile.infill_width_factor.value);
	ui.spinBox_top_bottom_width_factor->setOriginalValue(sliceProfile.top_bottom_width_factor.value);

	//Shell//
	ui.doubleSpinBox_wall_thickness->setOriginalValue(sliceProfile.wall_thickness.value);
	ui.doubleSpinBox_top_bottom_thickness->setOriginalValue(sliceProfile.top_bottom_thickness.value);
	ui.checkBox_solid_top->setOriginalValue(sliceProfile.solid_top.value);
	ui.checkBox_solid_bottom->setOriginalValue(sliceProfile.solid_bottom.value);
	ui.comboBox_wall_printing_direction->setOriginalValue(sliceProfile.wall_printing_direction.value);
	
	//Temperature//
	ui.spinBox_print_temperature->setOriginalValue(sliceProfile.print_temperature.value);
	ui.checkBox_temperature_layer_setting_enabled->setOriginalValue(sliceProfile.temperature_layer_setting_enabled.value);
	ui.spinBox_temperature_setpoint_temperature->setOriginalValue(sliceProfile.print_temperature.value);

	//Flow//
	ui.checkBox_overall_flow_control_enabled->setOriginalValue(sliceProfile.overall_flow_control_enabled.value);
	ui.spinBox_flow->setOriginalValue(sliceProfile.overall_flow.value);
	ui.spinBox_initial_layer_flow->setOriginalValue(sliceProfile.initial_layer_flow.value);
	ui.spinBox_infill_flow->setOriginalValue(sliceProfile.infill_flow.value);
	ui.spinBox_outer_wall_flow->setOriginalValue(sliceProfile.outer_wall_flow.value);
	ui.spinBox_inner_wall_flow->setOriginalValue(sliceProfile.inner_wall_flow.value);
	ui.spinBox_top_bottom_flow->setOriginalValue(sliceProfile.top_bottom_flow.value);
	   
	
	//Infill//
	ui.comboBox_infill_pattern->setOriginalValue(sliceProfile.infill_pattern.value);
	ui.spinBox_infill_density->setOriginalValue(sliceProfile.infill_density.value);
	ui.spinBox_infill_overlap->setOriginalValue(sliceProfile.infill_overlap.value);
	ui.checkBox_infill_before_wall->setOriginalValue(sliceProfile.infill_before_wall.value);
	ui.checkBox_skin_outline->setOriginalValue(sliceProfile.skin_outline.value);
	ui.comboBox_skin_type->setOriginalValue(sliceProfile.skin_type.value);
	ui.spinBox_skin_overlap->setOriginalValue(sliceProfile.skin_overlap.value);
	ui.doubleSpinBox_top_skin_removal_width->setOriginalValue(sliceProfile.skin_removal_width_top.value);
	ui.doubleSpinBox_bottom_skin_removal_width->setOriginalValue(sliceProfile.skin_removal_width_bottom.value);
	
	//Speed//
	ui.doubleSpinBox_print_speed->setOriginalValue(sliceProfile.print_speed.value);
	ui.doubleSpinBox_initial_layer_speed->setOriginalValue(sliceProfile.initial_layer_speed.value);
	ui.doubleSpinBox_outer_wall_speed->setOriginalValue(sliceProfile.outer_wall_speed.value);
	ui.doubleSpinBox_inner_wall_speed->setOriginalValue(sliceProfile.inner_wall_speed.value);
	ui.doubleSpinBox_infill_speed->setOriginalValue(sliceProfile.infill_speed.value);
	ui.doubleSpinBox_top_bottom_speed->setOriginalValue(sliceProfile.top_bottom_speed.value);
	ui.doubleSpinBox_travel_speed->setOriginalValue(sliceProfile.travel_speed.value);
	ui.spinBox_slower_layers_count->setOriginalValue(sliceProfile.slower_layers_count.value);
	
	//Retraction//
	ui.checkBox_retraction_enable->setOriginalValue(sliceProfile.retraction_enable.value);
	ui.doubleSpinBox_retraction_speed->setOriginalValue(sliceProfile.retraction_speed.value);
	ui.doubleSpinBox_retraction_amount->setOriginalValue(sliceProfile.retraction_amount.value);
	ui.doubleSpinBox_retraction_min_travel->setOriginalValue(sliceProfile.retraction_min_travel.value);
	ui.doubleSpinBox_retraction_minimal_extrusion->setOriginalValue(sliceProfile.retraction_minimal_extrusion.value);
	ui.doubleSpinBox_retraction_hop->setOriginalValue(sliceProfile.retraction_hop.value);
	ui.comboBox_internal_moving_area->setOriginalValue(sliceProfile.internal_moving_area.value);
	   
	//Cooling//
	ui.spinBox_fan_speed_regular->setOriginalValue(sliceProfile.fan_speed_regular.value);
	ui.spinBox_fan_speed_max->setOriginalValue(sliceProfile.fan_speed_max.value);
	ui.doubleSpinBox_cool_min_layer_time->setOriginalValue(sliceProfile.cool_min_layer_time.value);
	ui.checkBox_fan_enabled->setOriginalValue(sliceProfile.fan_enabled.value);
	ui.doubleSpinBox_fan_full_height->setOriginalValue(sliceProfile.fan_full_height.value);
	ui.doubleSpinBox_cool_min_feedrate->setOriginalValue(sliceProfile.cool_min_feedrate.value);
	ui.checkBox_cool_head_lift->setOriginalValue(sliceProfile.cool_head_lift.value);

	//Multi Nozzle//
	ui.doubleSpinBox_toolchange_retraction_amount->setOriginalValue(sliceProfile.toolchange_retraction_amount.value);
	ui.doubleSpinBox_toolchange_retraction_speed->setOriginalValue(sliceProfile.toolchange_retraction_speed.value);
	ui.doubleSpinBox_toolchange_extra_restart_amount->setOriginalValue(sliceProfile.toolchange_extra_restart_amount.value);
	ui.doubleSpinBox_toolchange_extra_restart_speed->setOriginalValue(sliceProfile.toolchange_extra_restart_speed.value);
	ui.checkBox_standby_temperature_enabled->setOriginalValue(sliceProfile.standby_temperature_enabled.value);
	ui.spinBox_operating_standby_temperature->setOriginalValue(sliceProfile.operating_standby_temperature.value);
	ui.spinBox_initial_standby_temperature->setOriginalValue(sliceProfile.initial_standby_temperature.value);
	ui.checkBox_preheat_enabled->setOriginalValue(sliceProfile.preheat_enabled.value);
	ui.doubleSpinBox_preheat_threshold_time->setOriginalValue(sliceProfile.preheat_threshold_time.value);

	//checking preheat toggled
	checkBox_preheat_enabled_toggled(ui.checkBox_preheat_enabled->isChecked());
}

void ProfileSettingWidget::loadOriginalCommonValue(SliceProfileForCommon sliceProfileCommon)
{
	//Quality//
	ui.doubleSpinBox_layer_height->setOriginalValue(sliceProfileCommon.layer_height.value);
	ui.doubleSpinBox_initial_layer_height->setOriginalValue(sliceProfileCommon.initial_layer_height.value);
	ui.spinBox_initial_layer_width_factor->setOriginalValue(sliceProfileCommon.initial_layer_width_factor.value);
	ui.spinBox_support_width_factor->setOriginalValue(sliceProfileCommon.support_main_width_factor.value);
	ui.spinBox_support_interface_roof_width_factor->setOriginalValue(sliceProfileCommon.support_interface_roof_width_factor.value);
	ui.spinBox_support_interface_floor_width_factor->setOriginalValue(sliceProfileCommon.support_interface_floor_width_factor.value);
	ui.doubleSpinBox_z_offset_raft->setOriginalValue(sliceProfileCommon.z_offset_raft.value);
	ui.doubleSpinBox_z_offset_except_raft->setOriginalValue(sliceProfileCommon.z_offset_except_raft.value);

	//Temperature//
	ui.spinBox_print_bed_temperature->setOriginalValue(sliceProfileCommon.print_bed_temperature.value);

	//Retraction//
	ui.comboBox_retraction_combing->setOriginalValue(sliceProfileCommon.retraction_combing.value);

	//Multi Nozzle//
	ui.doubleSpinBox_toolchange_lowering_bed->setOriginalValue(sliceProfileCommon.toolchange_lowering_bed.value);

	//Wipe Tower//
	ui.checkBox_wipetower_enable->setOriginalValue(sliceProfileCommon.wipe_tower_enabled.value);
	ui.comboBox_wipetower_position->setOriginalValue(sliceProfileCommon.wipe_tower_position.value);
	ui.spinBox_wipetower_infill_density->setOriginalValue(sliceProfileCommon.wipe_tower_infill_density.value);
	ui.doubleSpinBox_wipetower_raft_margin->setOriginalValue(sliceProfileCommon.wipe_tower_raft_margin.value);
	//ui.comboBox_wipetower_outer_cartridge_index->setOriginalValue(sliceProfileCommon.wipe_tower_outer_cartridge_index.value);
	ui.doubleSpinBox_wipetower_base_size->setOriginalValue(sliceProfileCommon.wipe_tower_base_size.value);
	ui.spinBox_wipetower_base_layer_count->setOriginalValue(sliceProfileCommon.wipe_tower_base_layer_count.value);
	ui.doubleSpinBox_wipetower_outer_size->setOriginalValue(sliceProfileCommon.wipe_tower_outer_size.value);
	ui.doubleSpinBox_wipetower_outer_wall_thickness->setOriginalValue(sliceProfileCommon.wipe_tower_outer_wall_thickness.value);
	ui.doubleSpinBox_wipetower_outer_inner_gap->setOriginalValue(sliceProfileCommon.wipe_tower_outer_inner_gap.value);
	ui.spinBox_wipetower_flow->setOriginalValue(sliceProfileCommon.wipe_tower_flow.value);
	ui.doubleSpinBox_wipetower_speed->setOriginalValue(sliceProfileCommon.wipe_tower_speed.value);


	//inner_size doubleSpinBox is replaced by line edit for only read-only diplay..//
	//ui.doubleSpinBox_wipetower_inner_size->setOriginalValue(sliceProfileCommon.wipe_tower_inner_size.value);

	//Support//
	loadOriginalSupportValue(sliceProfileCommon);

	//Adhesion//
	loadOriginalAdhesionValue(sliceProfileCommon);

	//Shape Error Correction
	ui.checkBox_spiralize_common->setOriginalValue(sliceProfileCommon.spiralize.value);
	ui.checkBox_simple_mode_common->setOriginalValue(sliceProfileCommon.simple_mode.value);
	ui.checkBox_fix_horrible_union_all_type_a_common->setOriginalValue(sliceProfileCommon.fix_horrible_union_all_type_a.value);
	ui.checkBox_fix_horrible_union_all_type_b_common->setOriginalValue(sliceProfileCommon.fix_horrible_union_all_type_b.value);
	ui.checkBox_fix_horrible_use_open_bits_common->setOriginalValue(sliceProfileCommon.fix_horrible_use_open_bits.value);
	ui.checkBox_fix_horrible_extensive_stitching_common->setOriginalValue(sliceProfileCommon.fix_horrible_extensive_stitching.value);
}

void ProfileSettingWidget::loadOriginalSupportValue(SliceProfileForCommon sliceProfileCommon)
{
	ui.comboBox_support_main_pattern->setOriginalValue(sliceProfileCommon.support_main_pattern.value);
	ui.spinBox_support_width_factor->setOriginalValue(sliceProfileCommon.support_main_width_factor.value);
	ui.spinBox_support_infill_density->setOriginalValue(sliceProfileCommon.support_infill_density.value);
	ui.doubleSpinBox_support_angle->setOriginalValue(sliceProfileCommon.support_angle.value);
	ui.doubleSpinBox_support_speed->setOriginalValue(sliceProfileCommon.support_main_speed.value);
	ui.spinBox_support_flow->setOriginalValue(sliceProfileCommon.support_main_flow.value);
	ui.doubleSpinBox_support_horizontal_expansion->setOriginalValue(sliceProfileCommon.support_horizontal_expansion.value);
	ui.doubleSpinBox_support_xy_distance->setOriginalValue(sliceProfileCommon.support_xy_distance.value);
	ui.doubleSpinBox_support_z_distance->setOriginalValue(sliceProfileCommon.support_z_distance.value);

	ui.checkBox_support_interface_enable->setOriginalValue(sliceProfileCommon.support_interface_enabled.value);
	ui.comboBox_support_interface_pattern->setOriginalValue(sliceProfileCommon.support_interface_pattern.value);
	ui.spinBox_support_interface_roof_layers_count->setOriginalValue(sliceProfileCommon.support_interface_roof_layers_count.value);
	ui.spinBox_support_interface_roof_width_factor->setOriginalValue(sliceProfileCommon.support_interface_roof_width_factor.value);
	ui.spinBox_support_interface_roof_flow->setOriginalValue(sliceProfileCommon.support_interface_roof_flow.value);
	ui.doubleSpinBox_support_interface_roof_speed->setOriginalValue(sliceProfileCommon.support_interface_roof_speed.value);
	ui.spinBox_support_interface_floor_layers_count->setOriginalValue(sliceProfileCommon.support_interface_floor_layers_count.value);
	ui.spinBox_support_interface_floor_width_factor->setOriginalValue(sliceProfileCommon.support_interface_floor_width_factor.value);
	ui.spinBox_support_interface_floor_flow->setOriginalValue(sliceProfileCommon.support_interface_floor_flow.value);
	ui.doubleSpinBox_support_interface_floor_speed->setOriginalValue(sliceProfileCommon.support_interface_floor_speed.value);

}
void ProfileSettingWidget::loadOriginalAdhesionValue(SliceProfileForCommon sliceProfileCommon)
{
	ui.spinBox_skirt_line_count->setOriginalValue(sliceProfileCommon.skirt_line_count.value);
	//ui.spinBox_skirt_width_factor->setOriginalValue(sliceProfileCommon.skirt_width_factor.value);
	ui.doubleSpinBox_skirt_gap->setOriginalValue(sliceProfileCommon.skirt_gap.value);
	ui.doubleSpinBox_skirt_minimal_length->setOriginalValue(sliceProfileCommon.skirt_minimal_length.value);
	//ui.doubleSpinBox_skirt_speed->setOriginalValue(sliceProfileCommon.skirt_speed.value);
	//ui.spinBox_skirt_flow->setOriginalValue(sliceProfileCommon.skirt_flow.value);

	ui.spinBox_brim_line_count->setOriginalValue(sliceProfileCommon.brim_line_count.value);
	//ui.spinBox_brim_width_factor->setOriginalValue(sliceProfileCommon.brim_width_factor.value);
	//ui.doubleSpinBox_brim_speed->setOriginalValue(sliceProfileCommon.brim_speed.value);
	//ui.spinBox_brim_flow->setOriginalValue(sliceProfileCommon.brim_flow.value);

	ui.doubleSpinBox_raft_margin->setOriginalValue(sliceProfileCommon.raft_margin.value);
	ui.doubleSpinBox_raft_line_spacing->setOriginalValue(sliceProfileCommon.raft_line_spacing.value);
	ui.doubleSpinBox_raft_airgap_all->setOriginalValue(sliceProfileCommon.raft_airgap_all.value);
	ui.doubleSpinBox_raft_airgap_initial_layer->setOriginalValue(sliceProfileCommon.raft_airgap_initial_layer.value);
	ui.checkBox_raft_inset_enabled->setOriginalValue(sliceProfileCommon.raft_inset_enabled.value);
	ui.doubleSpinBox_raft_inset_offset->setOriginalValue(sliceProfileCommon.raft_inset_offset.value);
	ui.checkBox_raft_temperature_control->setOriginalValue(sliceProfileCommon.raft_temperature_control.value);
	ui.checkBox_raft_incline_enabled->setOriginalValue(sliceProfileCommon.raft_incline_enabled.value);

	ui.doubleSpinBox_raft_base_thickness->setOriginalValue(sliceProfileCommon.raft_base_thickness.value);
	ui.doubleSpinBox_raft_base_line_width->setOriginalValue(sliceProfileCommon.raft_base_line_width.value);
	ui.doubleSpinBox_raft_base_speed->setOriginalValue(sliceProfileCommon.raft_base_speed.value);
	ui.spinBox_raft_base_temperature->setOriginalValue(sliceProfileCommon.raft_base_temperature.value);
	ui.doubleSpinBox_raft_interface_thickness->setOriginalValue(sliceProfileCommon.raft_interface_thickness.value);
	ui.doubleSpinBox_raft_interface_line_width->setOriginalValue(sliceProfileCommon.raft_interface_line_width.value);
	ui.doubleSpinBox_raft_interface_speed->setOriginalValue(sliceProfileCommon.raft_interface_speed.value);
	ui.spinBox_raft_interface_temperature->setOriginalValue(sliceProfileCommon.raft_interface_temperature.value);
	ui.spinBox_raft_surface_layers->setOriginalValue(sliceProfileCommon.raft_surface_layers.value);
	ui.doubleSpinBox_raft_surface_thickness->setOriginalValue(sliceProfileCommon.raft_surface_thickness.value);
	ui.doubleSpinBox_raft_surface_line_width->setOriginalValue(sliceProfileCommon.raft_surface_line_width.value);
	ui.doubleSpinBox_raft_surface_speed->setOriginalValue(sliceProfileCommon.raft_surface_speed.value);
	ui.spinBox_raft_surface_initial_temperature->setOriginalValue(sliceProfileCommon.raft_surface_initial_temperature.value);
	ui.spinBox_raft_surface_last_temperature->setOriginalValue(sliceProfileCommon.raft_surface_last_temperature.value);
}

void ProfileSettingWidget::loadProfileValue(SliceProfile sliceProfile)
{
	m_doNotChange = true;

	////////////////////////////////////////////////////////////
	ui.comboBox_profileNameList->setProfileValue(sliceProfile.profile_name.value);
	ui.comboBox_Material->setProfileValue(sliceProfile.filament_material.value);

	//Quality//
	ui.spinBox_wall_width_factor->setProfileValue(sliceProfile.wall_width_factor);
	ui.spinBox_infill_width_factor->setProfileValue(sliceProfile.infill_width_factor);
	ui.spinBox_top_bottom_width_factor->setProfileValue(sliceProfile.top_bottom_width_factor);

	//Shell//
	ui.doubleSpinBox_wall_thickness->setProfileValue(sliceProfile.wall_thickness);
	ui.doubleSpinBox_top_bottom_thickness->setProfileValue(sliceProfile.top_bottom_thickness);
	ui.checkBox_solid_top->setProfileValue(sliceProfile.solid_top);
	ui.checkBox_solid_bottom->setProfileValue(sliceProfile.solid_bottom);
	ui.comboBox_wall_printing_direction->setProfileValue(sliceProfile.wall_printing_direction);

	//Temperature//
	ui.spinBox_print_temperature->setProfileValue(sliceProfile.print_temperature);
	ui.checkBox_temperature_layer_setting_enabled->setProfileValue(sliceProfile.temperature_layer_setting_enabled);
	ui.spinBox_temperature_setpoint_temperature->setProfileValue(sliceProfile.print_temperature);
	ui.spinBox_temperature_setpoint_layer_number->setProfileValue_min_max(sliceProfile.temperature_setpoint_layer_number);

	//Flow//
	ui.checkBox_overall_flow_control_enabled->setProfileValue(sliceProfile.overall_flow_control_enabled);
	ui.spinBox_flow->setProfileValue(sliceProfile.overall_flow);
	ui.spinBox_initial_layer_flow->setProfileValue(sliceProfile.initial_layer_flow);
	ui.spinBox_infill_flow->setProfileValue(sliceProfile.infill_flow);
	ui.spinBox_outer_wall_flow->setProfileValue(sliceProfile.outer_wall_flow);
	ui.spinBox_inner_wall_flow->setProfileValue(sliceProfile.inner_wall_flow);
	ui.spinBox_top_bottom_flow->setProfileValue(sliceProfile.top_bottom_flow);


	//Infill//
	ui.comboBox_infill_pattern->setProfileValue(sliceProfile.infill_pattern);
	ui.spinBox_infill_density->setProfileValue(sliceProfile.infill_density);
	ui.spinBox_infill_overlap->setProfileValue(sliceProfile.infill_overlap);
	ui.checkBox_infill_before_wall->setProfileValue(sliceProfile.infill_before_wall);
	ui.comboBox_skin_type->setProfileValue(sliceProfile.skin_type);
	ui.checkBox_skin_outline->setProfileValue(sliceProfile.skin_outline);
	ui.spinBox_skin_overlap->setProfileValue(sliceProfile.skin_overlap);
	ui.doubleSpinBox_top_skin_removal_width->setProfileValue(sliceProfile.skin_removal_width_top);
	ui.doubleSpinBox_bottom_skin_removal_width->setProfileValue(sliceProfile.skin_removal_width_bottom);

	//Speed//
	ui.doubleSpinBox_print_speed->setProfileValue(sliceProfile.print_speed);
	ui.doubleSpinBox_initial_layer_speed->setProfileValue(sliceProfile.initial_layer_speed);
	ui.doubleSpinBox_outer_wall_speed->setProfileValue(sliceProfile.outer_wall_speed);
	ui.doubleSpinBox_inner_wall_speed->setProfileValue(sliceProfile.inner_wall_speed);
	ui.doubleSpinBox_infill_speed->setProfileValue(sliceProfile.infill_speed);
	ui.doubleSpinBox_top_bottom_speed->setProfileValue(sliceProfile.top_bottom_speed);
	ui.doubleSpinBox_travel_speed->setProfileValue(sliceProfile.travel_speed);
	ui.spinBox_slower_layers_count->setProfileValue(sliceProfile.slower_layers_count);

	//Retraction//
	ui.checkBox_retraction_enable->setProfileValue(sliceProfile.retraction_enable);
	ui.doubleSpinBox_retraction_speed->setProfileValue(sliceProfile.retraction_speed);
	ui.doubleSpinBox_retraction_amount->setProfileValue(sliceProfile.retraction_amount);
	ui.doubleSpinBox_retraction_min_travel->setProfileValue(sliceProfile.retraction_min_travel);
	ui.doubleSpinBox_retraction_minimal_extrusion->setProfileValue(sliceProfile.retraction_minimal_extrusion);
	ui.doubleSpinBox_retraction_hop->setProfileValue(sliceProfile.retraction_hop);
	ui.comboBox_internal_moving_area->setProfileValue(sliceProfile.internal_moving_area);

	//Cooling//
	ui.spinBox_fan_speed_regular->setProfileValue(sliceProfile.fan_speed_regular);
	ui.spinBox_fan_speed_max->setProfileValue(sliceProfile.fan_speed_max);
	ui.doubleSpinBox_cool_min_layer_time->setProfileValue(sliceProfile.cool_min_layer_time);
	ui.checkBox_fan_enabled->setProfileValue(sliceProfile.fan_enabled);
	ui.doubleSpinBox_fan_full_height->setProfileValue(sliceProfile.fan_full_height);
	ui.doubleSpinBox_cool_min_feedrate->setProfileValue(sliceProfile.cool_min_feedrate);
	ui.checkBox_cool_head_lift->setProfileValue(sliceProfile.cool_head_lift);

	//Multi Nozzle//
	ui.doubleSpinBox_toolchange_retraction_amount->setProfileValue(sliceProfile.toolchange_retraction_amount);
	ui.doubleSpinBox_toolchange_retraction_speed->setProfileValue(sliceProfile.toolchange_retraction_speed);
	ui.doubleSpinBox_toolchange_extra_restart_amount->setProfileValue(sliceProfile.toolchange_extra_restart_amount);
	ui.doubleSpinBox_toolchange_extra_restart_speed->setProfileValue(sliceProfile.toolchange_extra_restart_speed);
	ui.checkBox_standby_temperature_enabled->setProfileValue(sliceProfile.standby_temperature_enabled);
	ui.spinBox_operating_standby_temperature->setProfileValue(sliceProfile.operating_standby_temperature);
	ui.spinBox_initial_standby_temperature->setProfileValue(sliceProfile.initial_standby_temperature);
	ui.checkBox_preheat_enabled->setProfileValue(sliceProfile.preheat_enabled);
	ui.doubleSpinBox_preheat_threshold_time->setProfileValue(sliceProfile.preheat_threshold_time);
	////////////////////////////////////////////////////////////
	
	m_doNotChange = false;
}

void ProfileSettingWidget::loadProfileCommonValue(SliceProfileForCommon sliceProfileCommon)
{
	m_doNotChange = true;

	////////////////////////////////////////////////////////////
	if (!Profile::machineProfile.has_heated_bed.value) 
		sliceProfileCommon.print_bed_temperature.value = 0;
	sliceProfileCommon.wipe_tower_inner_cartridge_index.value = std::abs(sliceProfileCommon.wipe_tower_outer_cartridge_index.value - 1);

	//Quality//
	ui.doubleSpinBox_layer_height->setProfileValue(sliceProfileCommon.layer_height);
	ui.doubleSpinBox_initial_layer_height->setProfileValue(sliceProfileCommon.initial_layer_height);
	ui.spinBox_initial_layer_width_factor->setProfileValue(sliceProfileCommon.initial_layer_width_factor);
	ui.doubleSpinBox_z_offset_raft->setProfileValue(sliceProfileCommon.z_offset_raft);
	ui.doubleSpinBox_z_offset_except_raft->setProfileValue(sliceProfileCommon.z_offset_except_raft);

	//Temperature//
	ui.spinBox_print_bed_temperature->setProfileValue(sliceProfileCommon.print_bed_temperature);

	//Retraction//
	ui.comboBox_retraction_combing->setProfileValue(sliceProfileCommon.retraction_combing);
	
	//Multi Nozzle
	ui.doubleSpinBox_toolchange_lowering_bed->setProfileValue(sliceProfileCommon.toolchange_lowering_bed);

	//Wipe Tower
	ui.checkBox_wipetower_enable->setProfileValue(sliceProfileCommon.wipe_tower_enabled);
	ui.comboBox_wipetower_position->setProfileValue(sliceProfileCommon.wipe_tower_position);
	ui.spinBox_wipetower_infill_density->setProfileValue(sliceProfileCommon.wipe_tower_infill_density);
	ui.doubleSpinBox_wipetower_raft_margin->setProfileValue(sliceProfileCommon.wipe_tower_raft_margin);
	ui.comboBox_wipetower_outer_cartridge_index->setProfileValue(sliceProfileCommon.wipe_tower_outer_cartridge_index);
	ui.doubleSpinBox_wipetower_base_size->setProfileValue(sliceProfileCommon.wipe_tower_base_size);
	ui.spinBox_wipetower_base_layer_count->setProfileValue(sliceProfileCommon.wipe_tower_base_layer_count);
	ui.doubleSpinBox_wipetower_outer_size->setProfileValue(sliceProfileCommon.wipe_tower_outer_size);
	ui.doubleSpinBox_wipetower_outer_wall_thickness->setProfileValue(sliceProfileCommon.wipe_tower_outer_wall_thickness);
	ui.doubleSpinBox_wipetower_outer_inner_gap->setProfileValue(sliceProfileCommon.wipe_tower_outer_inner_gap);
	ui.comboBox_wipetower_inner_cartridge_index->setProfileValue(sliceProfileCommon.wipe_tower_inner_cartridge_index);
	ui.spinBox_wipetower_flow->setProfileValue(sliceProfileCommon.wipe_tower_flow);
	ui.doubleSpinBox_wipetower_speed->setProfileValue(sliceProfileCommon.wipe_tower_speed);
	//ui.doubleSpinBox_wipetower_inner_size->setProfileValue(sliceProfileCommon.wipe_tower_inner_size);

	
	//Support
	ui.comboBox_support->setProfileValue(sliceProfileCommon.support_placement);
	ui.comboBox_support_main_cartridge_index->setProfileValue(sliceProfileCommon.support_main_cartridge_index);
	loadProfileSupportValue(sliceProfileCommon);

	//Adhesion
	ui.comboBox_platform_adhesion->setProfileValue(sliceProfileCommon.platform_adhesion);
	ui.comboBox_adhesion_cart_index->setProfileValue(sliceProfileCommon.adhesion_cartridge_index);
	loadProfileAdhesionValue(sliceProfileCommon);

	//ui.doubleSpinBox_raft_airgap_all->setProfileValue(sliceProfileCommon.raft_airgap_all);

	//Shape Error Correction
	ui.checkBox_spiralize_common->setProfileValue(sliceProfileCommon.spiralize);
	ui.checkBox_simple_mode_common->setProfileValue(sliceProfileCommon.simple_mode);
	ui.checkBox_fix_horrible_union_all_type_a_common->setProfileValue(sliceProfileCommon.fix_horrible_union_all_type_a);
	ui.checkBox_fix_horrible_union_all_type_b_common->setProfileValue(sliceProfileCommon.fix_horrible_union_all_type_b);
	ui.checkBox_fix_horrible_use_open_bits_common->setProfileValue(sliceProfileCommon.fix_horrible_use_open_bits);
	ui.checkBox_fix_horrible_extensive_stitching_common->setProfileValue(sliceProfileCommon.fix_horrible_extensive_stitching);
	showWipeTowerInnerSize();
	////////////////////////////////////////////////////////////

	m_doNotChange = false;
}

void ProfileSettingWidget::loadProfileAdhesionValue(SliceProfileForCommon sliceProfileCommon)
{
	ui.spinBox_skirt_line_count->setProfileValue(sliceProfileCommon.skirt_line_count);
	//ui.spinBox_skirt_width_factor->setProfileValue(sliceProfileCommon.skirt_width_factor);
	ui.doubleSpinBox_skirt_gap->setProfileValue(sliceProfileCommon.skirt_gap);
	ui.doubleSpinBox_skirt_minimal_length->setProfileValue(sliceProfileCommon.skirt_minimal_length);
	//ui.doubleSpinBox_skirt_speed->setProfileValue(sliceProfileCommon.skirt_speed);
	//ui.spinBox_skirt_flow->setProfileValue(sliceProfileCommon.skirt_flow);

	ui.spinBox_brim_line_count->setProfileValue(sliceProfileCommon.brim_line_count);
	//ui.spinBox_brim_width_factor->setProfileValue(sliceProfileCommon.brim_width_factor);
	//ui.doubleSpinBox_brim_speed->setProfileValue(sliceProfileCommon.brim_speed);
	//ui.spinBox_brim_flow->setProfileValue(sliceProfileCommon.brim_flow);

	ui.doubleSpinBox_raft_margin->setProfileValue(sliceProfileCommon.raft_margin);
	ui.doubleSpinBox_raft_line_spacing->setProfileValue(sliceProfileCommon.raft_line_spacing);
	ui.doubleSpinBox_raft_airgap_all->setProfileValue(sliceProfileCommon.raft_airgap_all);
	ui.doubleSpinBox_raft_airgap_initial_layer->setProfileValue(sliceProfileCommon.raft_airgap_initial_layer);
	ui.checkBox_raft_inset_enabled->setProfileValue(sliceProfileCommon.raft_inset_enabled);
	ui.doubleSpinBox_raft_inset_offset->setProfileValue(sliceProfileCommon.raft_inset_offset);
	ui.checkBox_raft_temperature_control->setProfileValue(sliceProfileCommon.raft_temperature_control);
	ui.checkBox_raft_incline_enabled->setProfileValue(sliceProfileCommon.raft_incline_enabled);
	ui.doubleSpinBox_raft_base_thickness->setProfileValue(sliceProfileCommon.raft_base_thickness);
	ui.doubleSpinBox_raft_base_line_width->setProfileValue(sliceProfileCommon.raft_base_line_width);
	ui.doubleSpinBox_raft_base_speed->setProfileValue(sliceProfileCommon.raft_base_speed);
	ui.spinBox_raft_base_temperature->setProfileValue(sliceProfileCommon.raft_base_temperature);
	ui.doubleSpinBox_raft_interface_thickness->setProfileValue(sliceProfileCommon.raft_interface_thickness);
	ui.doubleSpinBox_raft_interface_line_width->setProfileValue(sliceProfileCommon.raft_interface_line_width);
	ui.doubleSpinBox_raft_interface_speed->setProfileValue(sliceProfileCommon.raft_interface_speed);
	ui.spinBox_raft_interface_temperature->setProfileValue(sliceProfileCommon.raft_interface_temperature);
	ui.spinBox_raft_surface_layers->setProfileValue(sliceProfileCommon.raft_surface_layers);
	ui.doubleSpinBox_raft_surface_thickness->setProfileValue(sliceProfileCommon.raft_surface_thickness);
	ui.doubleSpinBox_raft_surface_line_width->setProfileValue(sliceProfileCommon.raft_surface_line_width);
	ui.doubleSpinBox_raft_surface_speed->setProfileValue(sliceProfileCommon.raft_surface_speed);
	ui.spinBox_raft_surface_initial_temperature->setProfileValue(sliceProfileCommon.raft_surface_initial_temperature);
	ui.spinBox_raft_surface_last_temperature->setProfileValue(sliceProfileCommon.raft_surface_last_temperature);
}

void ProfileSettingWidget::loadProfileSupportValue(SliceProfileForCommon sliceProfileCommon)
{
	ui.comboBox_support_main_pattern->setProfileValue(sliceProfileCommon.support_main_pattern);
	ui.spinBox_support_width_factor->setProfileValue(sliceProfileCommon.support_main_width_factor);
	ui.spinBox_support_infill_density->setProfileValue(sliceProfileCommon.support_infill_density);
	ui.doubleSpinBox_support_angle->setProfileValue(sliceProfileCommon.support_angle);
	ui.doubleSpinBox_support_speed->setProfileValue(sliceProfileCommon.support_main_speed);
	ui.spinBox_support_flow->setProfileValue(sliceProfileCommon.support_main_flow);
	ui.doubleSpinBox_support_horizontal_expansion->setProfileValue(sliceProfileCommon.support_horizontal_expansion);
	ui.doubleSpinBox_support_xy_distance->setProfileValue(sliceProfileCommon.support_xy_distance);
	ui.doubleSpinBox_support_z_distance->setProfileValue(sliceProfileCommon.support_z_distance);

	ui.checkBox_support_interface_enable->setProfileValue(sliceProfileCommon.support_interface_enabled);
	ui.comboBox_support_interface_pattern->setProfileValue(sliceProfileCommon.support_interface_pattern);
	ui.spinBox_support_interface_roof_layers_count->setProfileValue(sliceProfileCommon.support_interface_roof_layers_count);
	ui.spinBox_support_interface_roof_width_factor->setProfileValue(sliceProfileCommon.support_interface_roof_width_factor);
	ui.spinBox_support_interface_roof_flow->setProfileValue(sliceProfileCommon.support_interface_roof_flow);
	ui.doubleSpinBox_support_interface_roof_speed->setProfileValue(sliceProfileCommon.support_interface_roof_speed);
	ui.spinBox_support_interface_floor_layers_count->setProfileValue(sliceProfileCommon.support_interface_floor_layers_count);
	ui.spinBox_support_interface_floor_width_factor->setProfileValue(sliceProfileCommon.support_interface_floor_width_factor);
	ui.spinBox_support_interface_floor_flow->setProfileValue(sliceProfileCommon.support_interface_floor_flow);
	ui.doubleSpinBox_support_interface_floor_speed->setProfileValue(sliceProfileCommon.support_interface_floor_speed);
}

void ProfileSettingWidget::saveProfileValue(SliceProfile *sliceProfile)
{
	ui.comboBox_profileNameList->getProfileValue(&sliceProfile->profile_name.value);
	ui.comboBox_Material->getProfileValue(&sliceProfile->filament_material.value);


	//Quality//
	ui.spinBox_wall_width_factor->getProfileValue(&sliceProfile->wall_width_factor);
	ui.spinBox_infill_width_factor->getProfileValue(&sliceProfile->infill_width_factor);
	ui.spinBox_top_bottom_width_factor->getProfileValue(&sliceProfile->top_bottom_width_factor);

	//Shell//
	ui.doubleSpinBox_wall_thickness->getProfileValue(&sliceProfile->wall_thickness);
	ui.doubleSpinBox_top_bottom_thickness->getProfileValue(&sliceProfile->top_bottom_thickness);
	ui.checkBox_solid_top->getProfileValue(&sliceProfile->solid_top);
	ui.checkBox_solid_bottom->getProfileValue(&sliceProfile->solid_bottom);
	ui.comboBox_wall_printing_direction->getProfileValue(&sliceProfile->wall_printing_direction);

	//Temperature//
	ui.spinBox_print_temperature->getProfileValue(&sliceProfile->print_temperature);
	ui.checkBox_temperature_layer_setting_enabled->getProfileValue(&sliceProfile->temperature_layer_setting_enabled);

	//Flow//
	ui.checkBox_overall_flow_control_enabled->getProfileValue(&sliceProfile->overall_flow_control_enabled);
	ui.spinBox_flow->getProfileValue(&sliceProfile->overall_flow);
	ui.spinBox_initial_layer_flow->getProfileValue(&sliceProfile->initial_layer_flow);
	ui.spinBox_infill_flow->getProfileValue(&sliceProfile->infill_flow);
	ui.spinBox_outer_wall_flow->getProfileValue(&sliceProfile->outer_wall_flow);
	ui.spinBox_inner_wall_flow->getProfileValue(&sliceProfile->inner_wall_flow);
	ui.spinBox_top_bottom_flow->getProfileValue(&sliceProfile->top_bottom_flow);


	//Infill//
	ui.comboBox_infill_pattern->getProfileValue(&sliceProfile->infill_pattern);
	ui.spinBox_infill_density->getProfileValue(&sliceProfile->infill_density);
	ui.spinBox_infill_overlap->getProfileValue(&sliceProfile->infill_overlap);
	ui.checkBox_infill_before_wall->getProfileValue(&sliceProfile->infill_before_wall);
	ui.comboBox_skin_type->getProfileValue(&sliceProfile->skin_type);
	ui.checkBox_skin_outline->getProfileValue(&sliceProfile->skin_outline);
	ui.spinBox_skin_overlap->getProfileValue(&sliceProfile->skin_overlap);
	ui.doubleSpinBox_top_skin_removal_width->getProfileValue(&sliceProfile->skin_removal_width_top);
	ui.doubleSpinBox_bottom_skin_removal_width->getProfileValue(&sliceProfile->skin_removal_width_bottom);

	//Speed//
	ui.doubleSpinBox_print_speed->getProfileValue(&sliceProfile->print_speed);
	ui.doubleSpinBox_initial_layer_speed->getProfileValue(&sliceProfile->initial_layer_speed);
	ui.doubleSpinBox_outer_wall_speed->getProfileValue(&sliceProfile->outer_wall_speed);
	ui.doubleSpinBox_inner_wall_speed->getProfileValue(&sliceProfile->inner_wall_speed);
	ui.doubleSpinBox_infill_speed->getProfileValue(&sliceProfile->infill_speed);
	ui.doubleSpinBox_top_bottom_speed->getProfileValue(&sliceProfile->top_bottom_speed);
	ui.doubleSpinBox_travel_speed->getProfileValue(&sliceProfile->travel_speed);
	ui.spinBox_slower_layers_count->getProfileValue(&sliceProfile->slower_layers_count);

	//Retraction//
	ui.checkBox_retraction_enable->getProfileValue(&sliceProfile->retraction_enable);
	ui.doubleSpinBox_retraction_speed->getProfileValue(&sliceProfile->retraction_speed);
	ui.doubleSpinBox_retraction_amount->getProfileValue(&sliceProfile->retraction_amount);
	ui.doubleSpinBox_retraction_min_travel->getProfileValue(&sliceProfile->retraction_min_travel);
	ui.doubleSpinBox_retraction_minimal_extrusion->getProfileValue(&sliceProfile->retraction_minimal_extrusion);
	ui.doubleSpinBox_retraction_hop->getProfileValue(&sliceProfile->retraction_hop);
	ui.comboBox_internal_moving_area->getProfileValue(&sliceProfile->internal_moving_area);

	//Cooling//
	ui.spinBox_fan_speed_regular->getProfileValue(&sliceProfile->fan_speed_regular);
	ui.spinBox_fan_speed_max->getProfileValue(&sliceProfile->fan_speed_max);
	ui.doubleSpinBox_cool_min_layer_time->getProfileValue(&sliceProfile->cool_min_layer_time);
	ui.checkBox_fan_enabled->getProfileValue(&sliceProfile->fan_enabled);
	ui.doubleSpinBox_fan_full_height->getProfileValue(&sliceProfile->fan_full_height);
	ui.doubleSpinBox_cool_min_feedrate->getProfileValue(&sliceProfile->cool_min_feedrate);
	ui.checkBox_cool_head_lift->getProfileValue(&sliceProfile->cool_head_lift);

	//Multi Nozzle//
	ui.doubleSpinBox_toolchange_retraction_amount->getProfileValue(&sliceProfile->toolchange_retraction_amount);
	ui.doubleSpinBox_toolchange_retraction_speed->getProfileValue(&sliceProfile->toolchange_retraction_speed);
	ui.doubleSpinBox_toolchange_extra_restart_amount->getProfileValue(&sliceProfile->toolchange_extra_restart_amount);
	ui.doubleSpinBox_toolchange_extra_restart_speed->getProfileValue(&sliceProfile->toolchange_extra_restart_speed);
	ui.checkBox_standby_temperature_enabled->getProfileValue(&sliceProfile->standby_temperature_enabled);
	ui.spinBox_operating_standby_temperature->getProfileValue(&sliceProfile->operating_standby_temperature);
	ui.spinBox_initial_standby_temperature->getProfileValue(&sliceProfile->initial_standby_temperature);
	ui.checkBox_preheat_enabled->getProfileValue(&sliceProfile->preheat_enabled);
	ui.doubleSpinBox_preheat_threshold_time->getProfileValue(&sliceProfile->preheat_threshold_time);

	saveProfileSupportValue(ui.comboBox_support_main_cartridge_index->currentIndex());
	saveProfileAdhesionValue(ui.comboBox_adhesion_cart_index->currentIndex());
}


void ProfileSettingWidget::saveProfileAdhesionValue(int targetCartridgeIdx)
{
	saveProfileAdhesionValue(&m_sliceProfile_multi->at(targetCartridgeIdx));
}

void ProfileSettingWidget::saveProfileAdhesionValue(SliceProfile *sliceProfile)
{
	ui.spinBox_skirt_line_count->getProfileValue(&sliceProfile->skirt_line_count);
	//ui.spinBox_skirt_width_factor->getProfileValue(&sliceProfile->skirt_width_factor);
	ui.doubleSpinBox_skirt_gap->getProfileValue(&sliceProfile->skirt_gap);
	ui.doubleSpinBox_skirt_minimal_length->getProfileValue(&sliceProfile->skirt_minimal_length);
	//ui.doubleSpinBox_skirt_speed->getProfileValue(&sliceProfile->skirt_speed);
	//ui.spinBox_skirt_flow->getProfileValue(&sliceProfile->skirt_flow);

	ui.spinBox_brim_line_count->getProfileValue(&sliceProfile->brim_line_count);
	//ui.spinBox_brim_width_factor->getProfileValue(&sliceProfile->brim_width_factor);
	//ui.doubleSpinBox_brim_speed->getProfileValue(&sliceProfile->brim_speed);
	//ui.spinBox_brim_flow->getProfileValue(&sliceProfile->brim_flow);

	ui.doubleSpinBox_raft_margin->getProfileValue(&sliceProfile->raft_margin);
	ui.doubleSpinBox_raft_line_spacing->getProfileValue(&sliceProfile->raft_line_spacing);
	ui.doubleSpinBox_raft_airgap_all->getProfileValue(&sliceProfile->raft_airgap_all);
	ui.doubleSpinBox_raft_airgap_initial_layer->getProfileValue(&sliceProfile->raft_airgap_initial_layer);
	ui.checkBox_raft_inset_enabled->getProfileValue(&sliceProfile->raft_inset_enabled);
	ui.doubleSpinBox_raft_inset_offset->getProfileValue(&sliceProfile->raft_inset_offset);
	ui.checkBox_raft_temperature_control->getProfileValue(&sliceProfile->raft_temperature_control);
	ui.checkBox_raft_incline_enabled->getProfileValue(&sliceProfile->raft_incline_enabled);

	ui.doubleSpinBox_raft_base_thickness->getProfileValue(&sliceProfile->raft_base_thickness);
	ui.doubleSpinBox_raft_base_line_width->getProfileValue(&sliceProfile->raft_base_line_width);
	ui.doubleSpinBox_raft_base_speed->getProfileValue(&sliceProfile->raft_base_speed);
	ui.spinBox_raft_base_temperature->getProfileValue(&sliceProfile->raft_base_temperature);
	ui.doubleSpinBox_raft_interface_thickness->getProfileValue(&sliceProfile->raft_interface_thickness);
	ui.doubleSpinBox_raft_interface_line_width->getProfileValue(&sliceProfile->raft_interface_line_width);
	ui.doubleSpinBox_raft_interface_speed->getProfileValue(&sliceProfile->raft_interface_speed);
	ui.spinBox_raft_interface_temperature->getProfileValue(&sliceProfile->raft_interface_temperature);
	ui.spinBox_raft_surface_layers->getProfileValue(&sliceProfile->raft_surface_layers);
	ui.doubleSpinBox_raft_surface_thickness->getProfileValue(&sliceProfile->raft_surface_thickness);
	ui.doubleSpinBox_raft_surface_line_width->getProfileValue(&sliceProfile->raft_surface_line_width);
	ui.doubleSpinBox_raft_surface_speed->getProfileValue(&sliceProfile->raft_surface_speed);
	ui.spinBox_raft_surface_initial_temperature->getProfileValue(&sliceProfile->raft_surface_initial_temperature);
	ui.spinBox_raft_surface_last_temperature->getProfileValue(&sliceProfile->raft_surface_last_temperature);
}

void ProfileSettingWidget::saveProfileSupportValue(int targetCartridgeIdx)
{
	saveProfileSupportValue(&m_sliceProfile_multi->at(targetCartridgeIdx));
}

void ProfileSettingWidget::saveProfileSupportValue(SliceProfile *sliceProfile)
{
	ui.comboBox_support_main_pattern->getProfileValue(&sliceProfile->support_main_pattern);
	ui.spinBox_support_width_factor->getProfileValue(&sliceProfile->support_main_width_factor);
	ui.spinBox_support_infill_density->getProfileValue(&sliceProfile->support_infill_density);
	ui.doubleSpinBox_support_angle->getProfileValue(&sliceProfile->support_angle);
	ui.doubleSpinBox_support_speed->getProfileValue(&sliceProfile->support_main_speed);
	ui.spinBox_support_flow->getProfileValue(&sliceProfile->support_main_flow);
	ui.doubleSpinBox_support_horizontal_expansion->getProfileValue(&sliceProfile->support_horizontal_expansion);
	ui.doubleSpinBox_support_xy_distance->getProfileValue(&sliceProfile->support_xy_distance);
	ui.doubleSpinBox_support_z_distance->getProfileValue(&sliceProfile->support_z_distance);

	ui.checkBox_support_interface_enable->getProfileValue(&sliceProfile->support_interface_enabled);
	ui.spinBox_support_interface_roof_layers_count->getProfileValue(&sliceProfile->support_interface_roof_layers_count);
	ui.comboBox_support_interface_pattern->getProfileValue(&sliceProfile->support_interface_pattern);
	ui.spinBox_support_interface_roof_width_factor->getProfileValue(&sliceProfile->support_interface_roof_width_factor);
	ui.spinBox_support_interface_roof_flow->getProfileValue(&sliceProfile->support_interface_roof_flow);
	ui.doubleSpinBox_support_interface_roof_speed->getProfileValue(&sliceProfile->support_interface_roof_speed);
	ui.spinBox_support_interface_floor_layers_count->getProfileValue(&sliceProfile->support_interface_floor_layers_count);
	ui.spinBox_support_interface_floor_width_factor->getProfileValue(&sliceProfile->support_interface_floor_width_factor);
	ui.spinBox_support_interface_floor_flow->getProfileValue(&sliceProfile->support_interface_floor_flow);
	ui.doubleSpinBox_support_interface_floor_speed->getProfileValue(&sliceProfile->support_interface_floor_speed);

}

void ProfileSettingWidget::reloadProfileCommonValue(int p_selectedCartridgeIndex)
{
	setRaftValueUI(p_selectedCartridgeIndex);
	setSupportValueUI(p_selectedCartridgeIndex);
	setCommonValueUI();

	loadProfileCommonValue(*m_sliceProfile_common);
}

void ProfileSettingWidget::saveProfileCommonValue()
{
	saveProfileCommonValue(m_sliceProfile_common);
}

void ProfileSettingWidget::saveProfileCommonValue(SliceProfileForCommon *sliceProfileCommon)
{
	//Quality//
	ui.doubleSpinBox_layer_height->getProfileValue(&sliceProfileCommon->layer_height);
	ui.doubleSpinBox_initial_layer_height->getProfileValue(&sliceProfileCommon->initial_layer_height);
	ui.spinBox_initial_layer_width_factor->getProfileValue(&sliceProfileCommon->initial_layer_width_factor);
	ui.doubleSpinBox_z_offset_raft->getProfileValue(&sliceProfileCommon->z_offset_raft);
	ui.doubleSpinBox_z_offset_except_raft->getProfileValue(&sliceProfileCommon->z_offset_except_raft);

	//Temperature//
	ui.spinBox_print_bed_temperature->getProfileValue(&sliceProfileCommon->print_bed_temperature);

	//Retraction//
	ui.comboBox_retraction_combing->getProfileValue(&sliceProfileCommon->retraction_combing);

	//Multi Nozzle
	ui.doubleSpinBox_toolchange_lowering_bed->getProfileValue(&sliceProfileCommon->toolchange_lowering_bed);

	//Wipe Tower
	ui.checkBox_wipetower_enable->getProfileValue(&sliceProfileCommon->wipe_tower_enabled);
	ui.comboBox_wipetower_position->getProfileValue(&sliceProfileCommon->wipe_tower_position);
	ui.spinBox_wipetower_infill_density->getProfileValue(&sliceProfileCommon->wipe_tower_infill_density);
	ui.doubleSpinBox_wipetower_raft_margin->getProfileValue(&sliceProfileCommon->wipe_tower_raft_margin);
	ui.comboBox_wipetower_outer_cartridge_index->getProfileValue(&sliceProfileCommon->wipe_tower_outer_cartridge_index);
	ui.doubleSpinBox_wipetower_base_size->getProfileValue(&sliceProfileCommon->wipe_tower_base_size);
	ui.spinBox_wipetower_base_layer_count->getProfileValue(&sliceProfileCommon->wipe_tower_base_layer_count);
	ui.doubleSpinBox_wipetower_outer_size->getProfileValue(&sliceProfileCommon->wipe_tower_outer_size);
	ui.doubleSpinBox_wipetower_outer_wall_thickness->getProfileValue(&sliceProfileCommon->wipe_tower_outer_wall_thickness);
	ui.doubleSpinBox_wipetower_outer_inner_gap->getProfileValue(&sliceProfileCommon->wipe_tower_outer_inner_gap);
	ui.comboBox_wipetower_inner_cartridge_index->getProfileValue(&sliceProfileCommon->wipe_tower_inner_cartridge_index);
	ui.spinBox_wipetower_flow->getProfileValue(&sliceProfileCommon->wipe_tower_flow);
	ui.doubleSpinBox_wipetower_speed->getProfileValue(&sliceProfileCommon->wipe_tower_speed);
	//ui.doubleSpinBox_wipetower_inner_size->getProfileValue(&sliceProfileCommon->wipe_tower_inner_size);
	sliceProfileCommon->wipe_tower_inner_size.value = ui.lineEdit_wipetower_inner_size->text().toDouble();

	//Support
	ui.comboBox_support->getProfileValue(&sliceProfileCommon->support_placement);
	ui.comboBox_support_main_cartridge_index->getProfileValue(&sliceProfileCommon->support_main_cartridge_index);
	ui.comboBox_support_main_pattern->getProfileValue(&sliceProfileCommon->support_main_pattern);
	ui.spinBox_support_width_factor->getProfileValue(&sliceProfileCommon->support_main_width_factor);
	ui.spinBox_support_infill_density->getProfileValue(&sliceProfileCommon->support_infill_density);
	ui.doubleSpinBox_support_angle->getProfileValue(&sliceProfileCommon->support_angle);
	ui.doubleSpinBox_support_speed->getProfileValue(&sliceProfileCommon->support_main_speed);
	ui.spinBox_support_flow->getProfileValue(&sliceProfileCommon->support_main_flow);
	ui.doubleSpinBox_support_horizontal_expansion->getProfileValue(&sliceProfileCommon->support_horizontal_expansion);
	ui.doubleSpinBox_support_xy_distance->getProfileValue(&sliceProfileCommon->support_xy_distance);
	ui.doubleSpinBox_support_z_distance->getProfileValue(&sliceProfileCommon->support_z_distance);

	ui.checkBox_support_interface_enable->getProfileValue(&sliceProfileCommon->support_interface_enabled);
	ui.comboBox_support_interface_pattern->getProfileValue(&sliceProfileCommon->support_interface_pattern);
	ui.spinBox_support_interface_roof_layers_count->getProfileValue(&sliceProfileCommon->support_interface_roof_layers_count);
	ui.spinBox_support_interface_roof_width_factor->getProfileValue(&sliceProfileCommon->support_interface_roof_width_factor);
	ui.spinBox_support_interface_roof_flow->getProfileValue(&sliceProfileCommon->support_interface_roof_flow);
	ui.doubleSpinBox_support_interface_roof_speed->getProfileValue(&sliceProfileCommon->support_interface_roof_speed);
	ui.spinBox_support_interface_floor_layers_count->getProfileValue(&sliceProfileCommon->support_interface_floor_layers_count);
	ui.spinBox_support_interface_floor_width_factor->getProfileValue(&sliceProfileCommon->support_interface_floor_width_factor);
	ui.spinBox_support_interface_floor_flow->getProfileValue(&sliceProfileCommon->support_interface_floor_flow);
	ui.doubleSpinBox_support_interface_floor_speed->getProfileValue(&sliceProfileCommon->support_interface_floor_speed);


	//Adehesion
	ui.comboBox_platform_adhesion->getProfileValue(&sliceProfileCommon->platform_adhesion);
	ui.comboBox_adhesion_cart_index->getProfileValue(&sliceProfileCommon->adhesion_cartridge_index);

	//Raft
	ui.spinBox_skirt_line_count->getProfileValue(&sliceProfileCommon->skirt_line_count);
	//ui.spinBox_skirt_width_factor->getProfileValue(&sliceProfileCommon->skirt_width_factor);
	ui.doubleSpinBox_skirt_gap->getProfileValue(&sliceProfileCommon->skirt_gap);
	ui.doubleSpinBox_skirt_minimal_length->getProfileValue(&sliceProfileCommon->skirt_minimal_length);
	//ui.doubleSpinBox_skirt_speed->getProfileValue(&sliceProfileCommon->skirt_speed);
	//ui.spinBox_skirt_flow->getProfileValue(&sliceProfileCommon->skirt_flow);

	ui.spinBox_brim_line_count->getProfileValue(&sliceProfileCommon->brim_line_count);
	//ui.spinBox_brim_width_factor->getProfileValue(&sliceProfileCommon->brim_width_factor);
	//ui.doubleSpinBox_brim_speed->getProfileValue(&sliceProfileCommon->brim_speed);
	//ui.spinBox_brim_flow->getProfileValue(&sliceProfileCommon->brim_flow);

	ui.doubleSpinBox_raft_margin->getProfileValue(&sliceProfileCommon->raft_margin);
	ui.doubleSpinBox_raft_line_spacing->getProfileValue(&sliceProfileCommon->raft_line_spacing);
	ui.doubleSpinBox_raft_airgap_all->getProfileValue(&sliceProfileCommon->raft_airgap_all);
	ui.doubleSpinBox_raft_airgap_initial_layer->getProfileValue(&sliceProfileCommon->raft_airgap_initial_layer);
	ui.checkBox_raft_inset_enabled->getProfileValue(&sliceProfileCommon->raft_inset_enabled);
	ui.doubleSpinBox_raft_inset_offset->getProfileValue(&sliceProfileCommon->raft_inset_offset);
	ui.checkBox_raft_temperature_control->getProfileValue(&sliceProfileCommon->raft_temperature_control);
	ui.checkBox_raft_incline_enabled->getProfileValue(&sliceProfileCommon->raft_incline_enabled);
	ui.doubleSpinBox_raft_base_thickness->getProfileValue(&sliceProfileCommon->raft_base_thickness);
	ui.doubleSpinBox_raft_base_line_width->getProfileValue(&sliceProfileCommon->raft_base_line_width);
	ui.doubleSpinBox_raft_base_speed->getProfileValue(&sliceProfileCommon->raft_base_speed);
	ui.spinBox_raft_base_temperature->getProfileValue(&sliceProfileCommon->raft_base_temperature);
	ui.doubleSpinBox_raft_interface_thickness->getProfileValue(&sliceProfileCommon->raft_interface_thickness);
	ui.doubleSpinBox_raft_interface_line_width->getProfileValue(&sliceProfileCommon->raft_interface_line_width);
	ui.doubleSpinBox_raft_interface_speed->getProfileValue(&sliceProfileCommon->raft_interface_speed);
	ui.spinBox_raft_interface_temperature->getProfileValue(&sliceProfileCommon->raft_interface_temperature);
	ui.spinBox_raft_surface_layers->getProfileValue(&sliceProfileCommon->raft_surface_layers);
	ui.doubleSpinBox_raft_surface_thickness->getProfileValue(&sliceProfileCommon->raft_surface_thickness);
	ui.doubleSpinBox_raft_surface_line_width->getProfileValue(&sliceProfileCommon->raft_surface_line_width);
	ui.doubleSpinBox_raft_surface_speed->getProfileValue(&sliceProfileCommon->raft_surface_speed);
	ui.spinBox_raft_surface_initial_temperature->getProfileValue(&sliceProfileCommon->raft_surface_initial_temperature);
	ui.spinBox_raft_surface_last_temperature->getProfileValue(&sliceProfileCommon->raft_surface_last_temperature);

	//Shape Error Correction
	ui.checkBox_spiralize_common->getProfileValue(&sliceProfileCommon->spiralize);
	ui.checkBox_simple_mode_common->getProfileValue(&sliceProfileCommon->simple_mode);
	ui.checkBox_fix_horrible_union_all_type_a_common->getProfileValue(&sliceProfileCommon->fix_horrible_union_all_type_a);
	ui.checkBox_fix_horrible_union_all_type_b_common->getProfileValue(&sliceProfileCommon->fix_horrible_union_all_type_b);
	ui.checkBox_fix_horrible_use_open_bits_common->getProfileValue(&sliceProfileCommon->fix_horrible_use_open_bits);
	ui.checkBox_fix_horrible_extensive_stitching_common->getProfileValue(&sliceProfileCommon->fix_horrible_extensive_stitching);

	//SJ
	//common 항목 중 Support/raft와 관련된 항목은 각각의 카트리지에 해당되는 항목에 저장되어야 함.
	//common항목 중 recent에 반영되어야 하는것은 여기에
	/*for (int i = 0; i < sliceProfile_multi->size(); i++)
	{
		ui.comboBox_support->getProfileValue(&sliceProfile_multi->at(i).support);
		ui.comboBox_support_main_cartridge_index->getProfileValue(&sliceProfile_multi->at(i).support_main_cartridge_index);
		ui.comboBox_platform_adhesion->getProfileValue(&sliceProfile_multi->at(i).platform_adhesion);
		ui.comboBox_adhesion_cart_index->getProfileValue(&sliceProfile_multi->at(i).adhesion_cartridge_index);
	}*/
}

void ProfileSettingWidget::setRaftValue()
{
	//다시 부를때 original도 다시 설정
	//original을 받아서해야 하나?? //SJ
	ui.spinBox_print_bed_temperature->setOriginalValue(m_sliceProfile_common->print_bed_temperature.value);
	ui.spinBox_print_bed_temperature->setProfileValue(m_sliceProfile_common->print_bed_temperature);

	ui.spinBox_initial_layer_width_factor->setOriginalValue(m_sliceProfile_common->initial_layer_width_factor.value);
	ui.spinBox_initial_layer_width_factor->setProfileValue(m_sliceProfile_common->initial_layer_width_factor);
}

void ProfileSettingWidget::setCommonValue()
{
	loadProfileCommonValue(*m_sliceProfile_common);

	/*ui.checkBox_wipetower_enable->setProfileValue(m_sliceProfile_common->wipe_tower);
	ui.doubleSpinBox_wipetower_size->setProfileValue(m_sliceProfile_common->wipe_tower_size);
	ui.doubleSpinBox_wipetower_thickness->setProfileValue(m_sliceProfile_common->wipe_tower_thickness);
	ui.doubleSpinBox_initial_layer_height->setProfileValue(m_sliceProfile_common->layer0_thickness);*/
}

void ProfileSettingWidget::setRaftValueUI(int targetCartridgeIdx)
{
	// hjkim modify 2021.10.13 if문 버그 수정 항상 current idx로 나옴.
	if (targetCartridgeIdx == -1) targetCartridgeIdx = ui.comboBox_adhesion_cart_index->currentIndex();
	if (ui.comboBox_adhesion_cart_index->isVisible() && targetCartridgeIdx != ui.comboBox_adhesion_cart_index->currentIndex()) return;
	int raftType, raftIdx;
	raftType = ui.comboBox_platform_adhesion->currentIndex();
	raftIdx = targetCartridgeIdx;
	//raftIdx = ui.comboBox_adhesion_cart_index->currentIndex();
	resetProfileAdhesionValue(raftType, raftIdx);
	resetOriginalAdhesionValue(raftType, raftIdx);
	//ui.widget->setRaftValue(raftType, m_sliceProfile_common->print_bed_temperature);
}
void ProfileSettingWidget::setCommonValueUI()
{
	resetProfileCommonValue();
	resetOriginalCommonValue();
}

void ProfileSettingWidget::setAdhesionCurrentIdx(int idx)
{
	ui.comboBox_platform_adhesion->setCurrentIndex(idx);
}

void ProfileSettingWidget::setSupportCurrentIdx(int idx)
{
	ui.comboBox_support->setCurrentIndex(idx);
}

void ProfileSettingWidget::resetProfileAdhesionValue(int raftType, int raftIdx)
{
	if (raftType == -1) raftType = ui.comboBox_platform_adhesion->currentIndex();
	if (raftIdx == -1) raftIdx = ui.comboBox_adhesion_cart_index->currentIndex();
	ProfileControl::setRaftProfile(m_sliceProfile_multi, m_sliceProfile_common, raftType, raftIdx);
	loadProfileAdhesionValue(*m_sliceProfile_common);
}

void ProfileSettingWidget::resetOriginalAdhesionValue(int raftType, int raftIdx)
{
	if (raftType == -1) raftType = ui.comboBox_platform_adhesion->currentIndex();
	if (raftIdx == -1) raftIdx = ui.comboBox_adhesion_cart_index->currentIndex();
	//original value 다시 불러오기
	ProfileControl::setRaftProfile(m_originalProfile_multi, m_originalProfile_common, raftType, raftIdx);
	loadOriginalAdhesionValue(*m_originalProfile_common);
}

void ProfileSettingWidget::setSupportValueUI(int targetCartridgeIdx)
{
	if (targetCartridgeIdx = -1) targetCartridgeIdx = ui.comboBox_support_main_cartridge_index->currentIndex();
	if (ui.comboBox_support_main_cartridge_index->isVisible() && targetCartridgeIdx != ui.comboBox_support_main_cartridge_index->currentIndex()) return;
	if (targetCartridgeIdx != ui.comboBox_support_main_cartridge_index->currentIndex()) return;
	int supportType, supportIdx;
	supportType = ui.comboBox_support->currentIndex();
	//supportIdx = ui.comboBox_support_main_cartridge_index->currentIndex();
	supportIdx = targetCartridgeIdx;
	resetProfileSupportValue(supportType, supportIdx);
	resetOriginalSupportValue(supportType, supportIdx);
}
void ProfileSettingWidget::resetProfileSupportValue(int supportType, int supportIdx)
{
	if (supportType == -1) supportType = ui.comboBox_support->currentIndex();
	if (supportIdx == -1) supportIdx = ui.comboBox_support_main_cartridge_index->currentIndex();
	ProfileControl::setSupportProfile(m_sliceProfile_multi, m_sliceProfile_common, supportType, supportIdx);
	loadProfileSupportValue(*m_sliceProfile_common);
}

void ProfileSettingWidget::resetOriginalSupportValue(int supportType, int supportIdx)
{
	if (supportType == -1) supportType = ui.comboBox_support->currentIndex();
	if (supportIdx == -1) supportIdx = ui.comboBox_support_main_cartridge_index->currentIndex();
	//original value 다시 불러오기
	ProfileControl::setSupportProfile(m_originalProfile_multi, m_originalProfile_common, supportType, supportIdx);
	loadOriginalSupportValue(*m_originalProfile_common);
}

void ProfileSettingWidget::resetProfileCommonValue()
{
	ProfileControl::setCommonProfile(m_sliceProfile_multi, m_sliceProfile_common);
	ui.doubleSpinBox_raft_airgap_all->setProfileValue(m_sliceProfile_common->raft_airgap_all);
	emit commonValueChanged();
}

void ProfileSettingWidget::resetOriginalCommonValue()
{
	ProfileControl::setCommonProfile(m_originalProfile_multi, m_originalProfile_common);
	ui.doubleSpinBox_raft_airgap_all->setOriginalValue(m_originalProfile_common->raft_airgap_all.value);
}

//void ProfileSettingWidget::checkBox_layer0_temperature_enabled_toggled(bool check)
//{
//	ui.label_layer0_temperature->setEnabled(check);
//	ui.doubleSpinBox_layer0_temperature->setEnabled(check);
//}

void ProfileSettingWidget::checkBox_wipetower_enable_toggled(bool check)
{
	ui.label_wipetower_position->setEnabled(check);
	ui.comboBox_wipetower_position->setEnabled(check);

	ui.label_wipetower_infill_density->setEnabled(check);
	ui.spinBox_wipetower_infill_density->setEnabled(check);

	ui.label_wipetower_raft_margin->setEnabled(check);
	ui.doubleSpinBox_wipetower_raft_margin->setEnabled(check);

	ui.label_wipetower_flow->setEnabled(check);
	ui.spinBox_wipetower_flow->setEnabled(check);

	ui.label_wipetower_outer_cartirdge_index->setEnabled(check);
	ui.comboBox_wipetower_outer_cartridge_index->setEnabled(check);

	ui.label_wipetower_inner_cartridge_index->setEnabled(check);
	ui.comboBox_wipetower_inner_cartridge_index->setEnabled(check);

	ui.label_wipetower_base_size->setEnabled(check);
	ui.doubleSpinBox_wipetower_base_size->setEnabled(check);

	ui.label_wipetower_base_layer_count->setEnabled(check);
	ui.spinBox_wipetower_base_layer_count->setEnabled(check);

	ui.label_wipetower_outer_size->setEnabled(check);
	ui.doubleSpinBox_wipetower_outer_size->setEnabled(check);

	ui.label_wipetower_outer_wall_thickness->setEnabled(check);
	ui.doubleSpinBox_wipetower_outer_wall_thickness->setEnabled(check);

	ui.label_wipetower_outer_inner_gap->setEnabled(check);
	ui.doubleSpinBox_wipetower_outer_inner_gap->setEnabled(check);

	ui.label_wipetower_inner_size->setEnabled(check);
	//ui.doubleSpinBox_wipetower_inner_size->setEnabled(check);

	ui.label_wipe_tower_speed->setEnabled(check);
	ui.doubleSpinBox_wipetower_speed->setEnabled(check);
}

void ProfileSettingWidget::checkBox_standby_temperature_enabled_toggled(bool check)
{
	ui.label_operating_standby_temperature->setEnabled(check);
	ui.spinBox_operating_standby_temperature->setEnabled(check);

	ui.label_initial_standby_temperature->setEnabled(check);
	ui.spinBox_initial_standby_temperature->setEnabled(check);

	if (!check)
	{
		ui.checkBox_preheat_enabled->setChecked(check);
	}

	ui.label_preheat_enabled->setEnabled(check);
	ui.checkBox_preheat_enabled->setEnabled(check);
}

void ProfileSettingWidget::checkBox_raft_inset_toggled(bool check)
{
	ui.doubleSpinBox_raft_inset_offset->setEnabled(check);
	ui.label_raft_inset_offset->setEnabled(check);
}

void ProfileSettingWidget::checkBox_support_interface_enabled_toggled(bool check)
{
	ui.comboBox_support_interface_pattern->setEnabled(check);
	ui.label_support_interface_pattern->setEnabled(check);

	ui.spinBox_support_interface_roof_layers_count->setEnabled(check);
	ui.label_support_interface_roof_layers_count->setEnabled(check);

	ui.spinBox_support_interface_roof_width_factor->setEnabled(check);
	ui.label_support_interface_roof_width_factor->setEnabled(check);

	ui.spinBox_support_interface_roof_flow->setEnabled(check);
	ui.label_support_interface_roof_flow->setEnabled(check);

	ui.doubleSpinBox_support_interface_roof_speed->setEnabled(check);
	ui.label_support_interface_roof_speed->setEnabled(check);


	ui.spinBox_support_interface_floor_layers_count->setEnabled(check);
	ui.label_support_interface_floor_layers_count->setEnabled(check);

	ui.spinBox_support_interface_floor_width_factor->setEnabled(check);
	ui.label_support_interface_floor_width_factor->setEnabled(check);

	ui.spinBox_support_interface_floor_flow->setEnabled(check);
	ui.label_support_interface_floor_flow->setEnabled(check);

	ui.doubleSpinBox_support_interface_floor_speed->setEnabled(check);
	ui.label_support_interface_floor_speed->setEnabled(check);
}

void ProfileSettingWidget::checkBox_raft_temperature_control_toggled(bool check)
{
	ui.label_raft_base_temperature->setVisible(check);
	ui.spinBox_raft_base_temperature->setVisible(check);

	ui.label_raft_interface_temperature->setVisible(check);
	ui.spinBox_raft_interface_temperature->setVisible(check);

	ui.label_raft_surface_initial_temperature->setVisible(check);
	ui.spinBox_raft_surface_initial_temperature->setVisible(check);

	ui.label_raft_surface_last_temperature->setVisible(check);
	ui.spinBox_raft_surface_last_temperature->setVisible(check);

}

void ProfileSettingWidget::checkBox_overall_flow_control_enabled(bool _flag)
{
	//invert//
	_flag = !_flag;

	ui.label_flow->setEnabled(!_flag);
	ui.spinBox_flow->setEnabled(!_flag);

	ui.label_initial_layer_flow->setEnabled(_flag);
	ui.spinBox_initial_layer_flow->setEnabled(_flag);

	ui.label_infill_flow->setEnabled(_flag);
	ui.spinBox_infill_flow->setEnabled(_flag);

	ui.label_outer_wall_flow->setEnabled(_flag);
	ui.spinBox_outer_wall_flow->setEnabled(_flag);

	ui.label_inner_wall_flow->setEnabled(_flag);
	ui.spinBox_inner_wall_flow->setEnabled(_flag);

	ui.label_top_bottom_flow->setEnabled(_flag);
	ui.spinBox_top_bottom_flow->setEnabled(_flag);

}

void ProfileSettingWidget::resetProfileNameList(QString pProfileName)
{
	m_doNotChange = true;
	ui.comboBox_profileNameList->clear();
	ui.comboBox_profileNameList->addItem(pProfileName);
	ui.comboBox_profileNameList->setCurrentIndex(0);
	m_doNotChange = false;
}

void ProfileSettingWidget::resetProfileNameList(std::vector<Generals::ProfileList> pProfileList)
{
	m_doNotChange = true;

	QStringList profileNameList;
	for (int i = 0; i < pProfileList.size(); i++)
	{
		profileNameList.append(pProfileList.at(i).name);
	}
	ui.comboBox_profileNameList->clear();
	ui.comboBox_profileNameList->addItems(profileNameList);
	m_doNotChange = false;
}

//support의 cartridge를 변경하였을 때 호출..
void ProfileSettingWidget::comboBox_support_cart_index_currentIndexChanged(int supportCartIndex)
{
	if (m_doNotChange) return;
	//exception 처리..//
	if (supportCartIndex < 0) supportCartIndex = 0;

	//int materialIndex = tempSliceProfileForCartridge->at(cartridgeIndex).material.value;
	/*int materialIndex;
	//여기 수정...

	if (selectedCartridgeIndex == supportCartIndex)
	{
	materialIndex = getMaterialIdx(ui.comboBox_Material->currentIndex(), m_parent->m_materialAvailableIndex);
	}
	else
	{
	materialIndex = tempSliceProfile->at(supportCartIndex).material.value;
	}*/

	//tempSliceProfile_common->support_main_cartridge_index.value = supportCartIndex;
	if (m_pre_support_cart_idx == -1)
		m_pre_adhesion_cart_idx = ui.comboBox_support_main_cartridge_index->currentIndex();
	saveProfileSupportValue(m_pre_support_cart_idx);
	setSupportValueUI(supportCartIndex);
	//setCommonValueUI();
	m_pre_support_cart_idx = supportCartIndex;
	//setSupportUI(materialIndex);
}

void ProfileSettingWidget::comboBox_adhesion_cart_index_currentIndexChanged(int cartridgeIndex)
{
	if (m_doNotChange) return;
	//exception 처리..//
	if (cartridgeIndex < 0) cartridgeIndex = 0;

	if (m_pre_adhesion_cart_idx == -1)
		m_pre_adhesion_cart_idx = ui.comboBox_adhesion_cart_index->currentIndex();
	saveProfileAdhesionValue(m_pre_adhesion_cart_idx);
	//setCommonValueUI();
	setRaftValueUI(cartridgeIndex);
	m_pre_adhesion_cart_idx = cartridgeIndex;
	emit raftValueChanged();
	//setRaftUI(materialIndex);
}

void ProfileSettingWidget::comboBox_platform_adhesion_currentIndexChanged(int idx)
{
	if (idx == -1)
		idx = ui.comboBox_platform_adhesion->currentIndex();

	ui.frame_raft->setVisible(false);
	ui.frame_brim->setVisible(false);
	ui.frame_skirt->setVisible(false);

	/*if (idx == Generals::PlatformAdhesion::Raft || idx == Generals::PlatformAdhesion::Brim)
	{
		for (int i = 0; i < CartridgeInfo::cartDatas.cartridges.size(); i++)
		{
			if (CartridgeInfo::cartDatas.cartridges.at(i).material == "PVA")
			{
				QString message;

				return;
			}
		}
	}*/

	if (idx == Generals::PlatformAdhesion::Raft)
	{
		ui.frame_raft->setVisible(true);
	}
	else if (idx == Generals::PlatformAdhesion::Brim)
	{
		ui.frame_brim->setVisible(true);
	}
	else if (idx == Generals::PlatformAdhesion::Skirt)
	{
		ui.frame_skirt->setVisible(true);
	}

	//마지막에 Machine에 따른 제약 적용
	setUIcontrolByMachineModel();

	if (m_doNotChange) return;
	ProfileControl::setRaftProfile(m_sliceProfile_multi, m_sliceProfile_common, idx, ui.comboBox_adhesion_cart_index->currentIndex());
	emit raftValueChanged();
}

void ProfileSettingWidget::comboBox_support_currentIndexChanged(int idx)
{
	if (idx == -1)
		idx = ui.comboBox_support->currentIndex();

	if (idx == Generals::SupportPlacement::SupportNone)
	{
		ui.frame_support->setVisible(false);
	}
	else
	{
		ui.frame_support->setVisible(true);
	}
	//마지막에 Machine에 따른 제약 적용
	setUIcontrolByMachineModel();
}

void ProfileSettingWidget::comboBox_profileNameList_currentIndexChanged(int idx)
{
	if (!m_doNotChange)
	{
		if (ui.comboBox_profileNameList->itemText(idx) == Generals::unknownProfileName)
		{
			m_doNotChange = true;
			emit warningMessage(MessageAlert::tr("profile_not_available"));
			ui.comboBox_profileNameList->setProfileValue(m_profileName);
		}
		else
		{
			m_profileName = ui.comboBox_profileNameList->itemText(idx);
			emit comboBox_profileNameList_change_sig(idx);
		}
	}
	else
	{
		m_doNotChange = false;
		m_profileName = ui.comboBox_profileNameList->itemText(idx);
	}
}

void ProfileSettingWidget::comboBox_wipetower_cartridgeIndex_currentIndexChanged(int idx)
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

void ProfileSettingWidget::checkBox_preheat_enabled_toggled(bool check)
{
	ui.label_preheat_threshold_time->setEnabled(check);
	ui.doubleSpinBox_preheat_threshold_time->setEnabled(check);
}

void ProfileSettingWidget::mousePressEvent(QMouseEvent *event)
{
	event->ignore();
}

void ProfileSettingWidget::mouseMoveEvent(QMouseEvent *event)
{
	event->ignore();
}

void ProfileSettingWidget::setRAFTstate(bool state)
{
	m_isRAFTstate = state;
}

ProfileDataI ProfileSettingWidget::getBedTemperature()
{
	ProfileDataI rtn;
	if (!Profile::machineProfile.has_heated_bed.value)
	{
		rtn.value = 0;
		return rtn;
	}
	return ProfileControl::getMaxBedTemperature(*m_sliceProfile_multi);
}

std::vector<int> ProfileSettingWidget::getUsedCardIdx()
{
	std::vector<int> usedCartIdx = m_usedCartIdx;
	if (ui.comboBox_platform_adhesion->currentIndex() != Generals::PlatformAdhesion::NoneAdhesion)
		usedCartIdx.push_back(ui.comboBox_adhesion_cart_index->currentIndex());

	if (ui.comboBox_support->currentIndex() != Generals::SupportPlacement::SupportNone)
		usedCartIdx.push_back(ui.comboBox_support_main_cartridge_index->currentIndex());

	std::sort(usedCartIdx.begin(), usedCartIdx.end());
	usedCartIdx.erase(std::unique(usedCartIdx.begin(), usedCartIdx.end()), usedCartIdx.end());

	return usedCartIdx;
}

void ProfileSettingWidget::warningMessageBedAdhesionType(int idx)
{
	if (m_doNotChange) return;
	if (!(idx == Generals::PlatformAdhesion::Raft)) //RAFT가 없을 때..
	{
		//CommonDialog comDlg(this, tr("no_raft_alert"), CommonDialog::Warning);
		if (Profile::machineProfile.machine_bed_selected_enabled.value != 1)
			emit warningMessage(MessageAlert::tr("no_raft_alert"));
	}
}

void ProfileSettingWidget::clearTemperatureLayerListWidget()
{
	disconnect(ui.tableWidget_temperature_layer_list, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(tableWidget_temperature_layer_list_itemChanged(QTableWidgetItem*)));

	int preRowCount = ui.tableWidget_temperature_layer_list->rowCount();
	for (int i = preRowCount; i >= 0; --i)
	{
		ui.tableWidget_temperature_layer_list->removeRow(i);
	}
	preRowCount = ui.tableWidget_temperature_layer_list->rowCount();

	SliceProfile* profile = &m_sliceProfile_multi->at(m_selectedCartridgeIndex);
	profile->temperature_layer_list.clear();
	profile->temperature_setpoint_layer_number.value = 0;

	connect(ui.tableWidget_temperature_layer_list, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(tableWidget_temperature_layer_list_itemChanged(QTableWidgetItem*)));
}

void ProfileSettingWidget::updateTemperatureLayerListWidget(SliceProfile* sliceProfile_)
{
	disconnect(ui.tableWidget_temperature_layer_list, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(tableWidget_temperature_layer_list_itemChanged(QTableWidgetItem *)));

	//temperature layer의 data들 모두 삭제..//

	//ui.tableWidget_perlayer_temperature_setting->clear();
	int preRowCount = ui.tableWidget_temperature_layer_list->rowCount();
	for (int i = preRowCount; i >= 0; --i)
	{
		ui.tableWidget_temperature_layer_list->removeRow(i);
	}

	//다시 slice profile에서 temperature layer관련 항목 가져와서 재구성..//

	int rowSize = sliceProfile_->temperature_layer_list.size();
	//set tableWidget_temeprature from data..//
	std::vector<TemperatureLayerSetPoint> temp_temperatureLayerList;
	temp_temperatureLayerList = sliceProfile_->temperature_layer_list;

	int layerNr;
	int temperature;
	for (int i = 0; i < rowSize; ++i)
	{
		layerNr = temp_temperatureLayerList.at(i).layerNr;
		temperature = temp_temperatureLayerList.at(i).temperature;

		if (Generals::temperatureUnit == "F")
			temperature = Generals::convertTemperatureUnitCtoF(temperature);

		//아래 tableWidget_temperature_layer_list_itemChanged()에서 막아주긴 하는데.. //
		//기존 data에 있는 것 검증 차원에서..//
		if (layerNr < 0)
			continue;
		if (layerNr > sliceProfile_->temperature_setpoint_layer_number.max)
			continue;

		addTemperatureLayerSetPoint(sliceProfile_, layerNr, temperature, false);
	}

	connect(ui.tableWidget_temperature_layer_list, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(tableWidget_temperature_layer_list_itemChanged(QTableWidgetItem *)));
}

void ProfileSettingWidget::checkBox_temperature_layer_setting_enabled(bool check)
{
	ui.frame_temperature_layer_list_setting->setVisible(check);

	if (check)
		updateTemperatureLayerListWidget(&m_sliceProfile_multi->at(m_selectedCartridgeIndex));

}

void ProfileSettingWidget::addTemperatureLayerSetPoint(SliceProfile* sliceProfile_, int pLayerNr, int pTemperature, bool pInsertFlag)
{
	int cartridgeCount = CartridgeInfo::cartridges.size();
	int layerNr;
	int temperature;

	if (pLayerNr == -999)
	{
		layerNr = ui.spinBox_temperature_setpoint_layer_number->value();
		if (layerNr == 0) return;
	}
	else
	{
		layerNr = pLayerNr;
	}

	if (pTemperature == -999)
	{
		temperature = ui.spinBox_temperature_setpoint_temperature->value();
	}
	else
	{
		temperature = pTemperature;
	}

	int rowCount = ui.tableWidget_temperature_layer_list->rowCount();
	int targetRow = 0;
	int insertedLayer = 0;

	for (int i = 0; i < rowCount; i++)
	{
		insertedLayer = ui.tableWidget_temperature_layer_list->item(i, 0)->text().toInt();

		if (insertedLayer == layerNr)
		{
			emit warningMessage(MessageAlert::tr("duplicated_layer_number"));

			setDisableApplyButton();
			return;
		}
		else if (insertedLayer < layerNr)
			targetRow = i + 1;
		//else if (insertedLayer > layerNr)
		//{
		//	QWidget *widget_temperature_ = ui.tableWidget_temperature_layer_list->cellWidget(i, 1);
		//	widget_temperature_->setProperty("row", i + 1);
		//	widget_temperature_->setFont(QFont("Malgun Gothic", 9, QFont::Normal));
		//}
	}

	ui.tableWidget_temperature_layer_list->insertRow(targetRow);
	QTableWidgetItem *item_layerNr = new QTableWidgetItem(QString::number(layerNr));
	QTableWidgetItem *item_temperature = new QTableWidgetItem(QString::number(temperature));
	item_layerNr->setFont(QFont("Malgun Gothic", 9, QFont::Normal));
	item_temperature->setFont(QFont("Malgun Gothic", 9, QFont::Normal));
	item_layerNr->setTextAlignment(Qt::AlignRight);
	item_temperature->setTextAlignment(Qt::AlignRight);
	ui.tableWidget_temperature_layer_list->setItem(targetRow, 0, item_layerNr);
	ui.tableWidget_temperature_layer_list->setItem(targetRow, 1, item_temperature);
	ui.tableWidget_temperature_layer_list->setRowHeight(targetRow, 25);


	//item_layerNr->setFlags(item_layerNr->flags() | Qt::ItemIsEditable);
	//item_temperature->setFlags(item_temperature->flags() | Qt::ItemIsEditable);

	//layer 0은 수정 불가.. 값은 layer0_temperature에서 가져옴..//
	/*if (layerNr == 0)
	{
		QTableWidgetItem *item = ui.tableWidget_temperature_layer_list->item(targetRow, 0);
		item->setFlags(item->flags() &= ~Qt::ItemIsEditable);
	}*/

	//doubleSpinbox를 사용하지 않고, widget 기본값으로..//
	/*QWidget* widget_temperature = new QWidget();
	widget_temperature->setProperty("row", targetRow);
	ui.tableWidget_temperature_layer_list->setCellWidget(targetRow, 1, widget_temperature);*/

	//add, delete시 UI에서 처리하고 최종 결과값만 저장하고 동기화하는 것으로 할려고 했으나 하나의 tableWidget에서 2가지 이상을 //
	//보여주어야 하므로, data 동기화 방식을 해야함..--> 취소에 대비하기 위한 temp data방식 적용해야 함..profile과 같이..//
	if (pInsertFlag)
	{
		//if (ui.comboBox_temperature_layer_type->currentIndex() == 0) //bed
		//{
		//	m_temperatureProfile->temperatureLayerList_bed.insert(m_temperatureProfile->temperatureLayerList_bed.begin() + targetRow, TemperatureLayer(layerNr, temperature));
		//}
		//else //cartridge
		//{
		//	m_temperatureProfile->temperatureLayerList_cartridge.at(m_selectedCartridgeIndex).insert(m_temperatureProfile->temperatureLayerList_cartridge.at(m_selectedCartridgeIndex).begin() + targetRow, TemperatureLayer(layerNr, temperature));
		//}
		if (Generals::temperatureUnit == "F")
			temperature = Generals::convertTemperatureUnitFtoC(temperature);

		sliceProfile_->temperature_layer_list.insert(sliceProfile_->temperature_layer_list.begin() + targetRow, TemperatureLayerSetPoint(layerNr, temperature));
		setEnableApplyButton();
	}

	if (!isVerticalScrollBarVisible_tableWidget_temperature())
	{
		ui.tableWidget_temperature_layer_list->setColumnWidth(0, 70);
		ui.tableWidget_temperature_layer_list->setColumnWidth(1, 115);
		ui.tableWidget_temperature_layer_list->repaint();
	}
}

void ProfileSettingWidget::addTemperatureLayerSetPoint()
{
	addTemperatureLayerSetPoint(&m_sliceProfile_multi->at(m_selectedCartridgeIndex));
}

void ProfileSettingWidget::deleteTemperatureLayerSetPoint(SliceProfile* sliceProfile_)
{
	QList<QTableWidgetItem*> selected = ui.tableWidget_temperature_layer_list->selectedItems();

	if (selected.size() == 0)
		return;

	int selectedRow = selected.at(0)->row();

	ui.tableWidget_temperature_layer_list->removeRow(selectedRow);
	sliceProfile_->temperature_layer_list.erase(sliceProfile_->temperature_layer_list.begin() + selectedRow);
	setEnableApplyButton();

	//int insertedLayer;
	//int rowCount;
	//int selectedRow;
	//int selectedLayer;

	//if (selected.size() == 0)
	//	return;

	//qSort(selected);

	//for (int i = selected.size()-1; i >= 0; i--)
	//{
	//	rowCount = ui.tableWidget_temperature_layer_list->rowCount();
	//	selectedLayer = selected.at(i)->text().toInt();
	//	selectedRow = selected.at(i)->row();

	//	for (int j = rowCount - 1; j >= 0; j--)
	//	{
	//		insertedLayer = ui.tableWidget_temperature_layer_list->item(j, 0)->text().toInt();

	//		if (insertedLayer > selectedLayer)
	//		{
	//			QWidget* widget_temperature = (QWidget*)ui.tableWidget_temperature_layer_list->cellWidget(j, 1);
	//			widget_temperature->setProperty("row", j - 1);
	//		}
	//		else if (insertedLayer == selectedLayer)
	//		{
	//			ui.tableWidget_temperature_layer_list->removeRow(selectedRow);

	//			//if (ui.comboBox_temperature_layer_type->currentIndex() == 0) //bed
	//			//{
	//			//	//layerNr = -10 때문에 1 이동..//
	//			//	m_temperatureProfile->temperatureLayerList_bed.erase(m_temperatureProfile->temperatureLayerList_bed.begin() + selectedRow);
	//			//}
	//			//else //cartridge
	//			//{
	//			//	m_temperatureProfile->temperatureLayerList_cartridge.at(m_selectedCartridgeIndex).erase(m_temperatureProfile->temperatureLayerList_cartridge.at(m_selectedCartridgeIndex).begin() + selectedRow);
	//			//}

	//			m_sliceProfile_multi->at(m_selectedCartridgeIndex).temperature_layer_list.erase(m_sliceProfile_multi->at(m_selectedCartridgeIndex).temperature_layer_list.begin() + selectedRow);
	//		}
	//	}
	//}

	if (!isVerticalScrollBarVisible_tableWidget_temperature())
	{
		ui.tableWidget_temperature_layer_list->setColumnWidth(0, 70);
		ui.tableWidget_temperature_layer_list->setColumnWidth(1, 115);
		ui.tableWidget_temperature_layer_list->repaint();
	}

}

void ProfileSettingWidget::deleteTemperatureLayerSetPoint()
{
	deleteTemperatureLayerSetPoint(&m_sliceProfile_multi->at(m_selectedCartridgeIndex));
}

void ProfileSettingWidget::tableWidget_temperature_layer_list_itemChanged(QTableWidgetItem *changedItem)
{
	// 1) 처음부터 아무것도 없는 상태에서 add 할 때 호출..//
	// 2) 기존에 있는 것에서 add 할 때..//
	// 3) 기존에 있는 것에서 delete 할 때..//

	int row = changedItem->row();

	std::vector<TemperatureLayerSetPoint> *temp_temperatureLayerList;
	//if (ui.comboBox_temperature_layer_type->currentIndex() == 0) //bed..//
	//{
	//	temp_temperatureLayerList = &m_temperatureProfile->temperatureLayerList_bed;
	//}
	//else if (ui.comboBox_temperature_layer_type->currentIndex() > 0)//cartridges..//
	//{
	//	temp_temperatureLayerList = &(m_temperatureProfile->temperatureLayerList_cartridge.at(m_selectedCartridgeIndex));
	//}

	temp_temperatureLayerList = &(m_sliceProfile_multi->at(m_selectedCartridgeIndex).temperature_layer_list);

	if (temp_temperatureLayerList->size() == ui.tableWidget_temperature_layer_list->rowCount())
	{
		if (changedItem->column() == 0) //layer
		{
			int layerNr = changedItem->text().toInt();
			int preLayerNr = temp_temperatureLayerList->at(row).layerNr;

			//layerNr 0은 막음..//
			if (layerNr <= 0)
				layerNr = preLayerNr;

			//layerNr의 최대값은 layerNr spinbox와 동일하게 max값 설정..//
			if (layerNr > m_sliceProfile_multi->at(m_selectedCartridgeIndex).temperature_setpoint_layer_number.max)
				layerNr = preLayerNr;

			if (temp_temperatureLayerList->size() != row)
			{
				int rowCount = ui.tableWidget_temperature_layer_list->rowCount();
				int insertedLayer;

				for (int i = 0; i < rowCount; i++)
				{
					insertedLayer = ui.tableWidget_temperature_layer_list->item(i, 0)->text().toInt();

					if (i == row) continue;
					else if (insertedLayer == layerNr)
					{
						emit warningMessage(MessageAlert::tr("duplicated_layer_number"));

						ui.tableWidget_temperature_layer_list->setItem(row, 0, new QTableWidgetItem(QString::number(preLayerNr)));

						setDisableApplyButton();

						return;
					}
				}

				//초기 datat생성 시, 예외처리..//
				if (ui.tableWidget_temperature_layer_list->item(row, 1) != NULL)
				{
					QTableWidgetItem *widgetItem_temperature = ui.tableWidget_temperature_layer_list->item(row, 1);
					int perLayer_temperature = widgetItem_temperature->text().toInt();

					this->deleteTemperatureLayerSetPoint(&(m_sliceProfile_multi->at(m_selectedCartridgeIndex)));
					this->addTemperatureLayerSetPoint(&(m_sliceProfile_multi->at(m_selectedCartridgeIndex)), layerNr, perLayer_temperature, true);
				}
			}

			setEnableApplyButton();
		}
		else if (changedItem->column() == 1) //temperature 
		{
			int temperature = changedItem->text().toInt();
			int preTemperature = temp_temperatureLayerList->at(row).temperature;
			int temperatureMin = m_sliceProfile_multi->at(m_selectedCartridgeIndex).print_temperature.min;
			int temperatureMax = m_sliceProfile_multi->at(m_selectedCartridgeIndex).print_temperature.max;
			if (Generals::temperatureUnit == "F")
			{
				preTemperature = Generals::convertTemperatureUnitCtoF(preTemperature);
				temperatureMin = Generals::convertTemperatureUnitCtoF(temperatureMin);
				temperatureMax = Generals::convertTemperatureUnitCtoF(temperatureMax);
			}

			if (temperature < temperatureMin)
				temperature = preTemperature;

			if (temperature > temperatureMax)
				temperature = preTemperature;


			if (temp_temperatureLayerList->size() != row)
			{
				QTableWidgetItem *widgetItem_layerNr = changedItem->tableWidget()->item(row, 0);
				int temp_layerNr = widgetItem_layerNr->text().toInt();

				this->deleteTemperatureLayerSetPoint(&(m_sliceProfile_multi->at(m_selectedCartridgeIndex)));
				this->addTemperatureLayerSetPoint(&(m_sliceProfile_multi->at(m_selectedCartridgeIndex)), temp_layerNr, temperature, true);
			}

			setEnableApplyButton();
		}
	}
}

bool ProfileSettingWidget::isVerticalScrollBarVisible_tableWidget_temperature()
{
	bool IsVisible = false;

	int HeightOfAllRows = 0;
	for (int i = 0; i < ui.tableWidget_temperature_layer_list->rowCount(); i++)
		HeightOfAllRows += ui.tableWidget_temperature_layer_list->rowHeight(i);

	int HeaderHeight = ui.tableWidget_temperature_layer_list->horizontalHeader()->height();
	int TableHeight = ui.tableWidget_temperature_layer_list->height();

	if ((HeightOfAllRows + HeaderHeight) > TableHeight)
		IsVisible = true;

	return IsVisible;
}

void ProfileSettingWidget::saveTemperatureListForCustom(SliceProfile *sliceProfile)
{
	//for custom profile..//
	sliceProfile->temperature_layer_list = m_sliceProfile_multi->at(0).temperature_layer_list;

}

void ProfileSettingWidget::setProfileValue_initial_layer_width_factor(ProfileDataI data)
{
	ui.spinBox_initial_layer_width_factor->setOriginalValue(data.value);
	ui.spinBox_initial_layer_width_factor->setProfileValue(data);
}

//void ProfileSettingWidget::setFlowCurrentValue(int _value)
//{
//	ui.spinBox_initial_layer_flow->setValue(_value);
//	ui.spinBox_infill_flow->setValue(_value);
//	ui.spinBox_outer_wall_flow->setValue(_value);
//	ui.spinBox_inner_wall_flow->setValue(_value);
//	ui.spinBox_top_bottom_flow->setValue(_value);
//}
//
//void ProfileSettingWidget::setSpeedCurrentValue(double _value)
//{
//	ui.doubleSpinBox_initial_layer_speed->setValue(_value);
//	ui.doubleSpinBox_outer_wall_speed->setValue(_value);
//	ui.doubleSpinBox_inner_wall_speed->setValue(_value);
//	ui.doubleSpinBox_infill_speed->setValue(_value);
//	ui.doubleSpinBox_top_bottom_speed->setValue(_value);
//}