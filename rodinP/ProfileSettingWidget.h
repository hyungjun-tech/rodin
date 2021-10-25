#pragma once

#include <QWidget>
#include "ui_ProfileSettingWidget.h"
#include "Generals.h"

class SliceProfile;
class SliceProfileForCommon;
class ProfileDataD;
class ProfileDataB;
class ProfileDataI;
//class TemperatureProfile;

class ProfileSettingWidget : public QWidget
{
	Q_OBJECT

public:
	ProfileSettingWidget(QWidget *parent = 0);
	~ProfileSettingWidget();

	double wipeTowerInnerSize_calculated;
	int m_selectedCartridgeIndex;

	void setParameter(std::vector<SliceProfile> *sliceProfile_multi, SliceProfileForCommon *sliceProfile_common, std::vector<SliceProfile> *originalProfile_multi, SliceProfileForCommon *originalProfile_common, std::vector<int> usedCartIdx, int selectedCartIdx);
	void setParameter(SliceProfile *sliceProfile, SliceProfileForCommon *sliceProfile_common, std::vector<int> usedCartIdx);
	void setProfilesLink(std::vector<SliceProfile> *sliceProfile_multi, SliceProfileForCommon *sliceProfile_common, int seletedCartridgeIndex);
	void setTabStyle(QString _cartridge_color, QString _common_color);

	void loadOriginalValue(SliceProfile sliceProfile);
	void loadOriginalCommonValue(SliceProfileForCommon sliceProfileCommon);
	void loadOriginalSupportValue(SliceProfileForCommon sliceProfileCommon);
	void loadOriginalAdhesionValue(SliceProfileForCommon sliceProfileCommon);
	void loadProfileValue(SliceProfile sliceProfile);
	void loadProfileCommonValue(SliceProfileForCommon sliceProfileCommon);
	void loadProfileAdhesionValue(SliceProfileForCommon sliceProfileCommon);
	void loadProfileSupportValue(SliceProfileForCommon sliceProfileCommon);

	void saveProfileCommonValue();
	void saveProfileCommonValue(SliceProfileForCommon *sliceProfileCommon);
	void saveProfileValue(SliceProfile *sliceProfile);
	void saveProfileAdhesionValue(int targetCartridgeIdx);
	void saveProfileAdhesionValue(SliceProfile *sliceProfile);
	void saveProfileSupportValue(int targetCartridgeIdx);
	void saveProfileSupportValue(SliceProfile *sliceProfile);
	void saveTemperatureListForCustom(SliceProfile *sliceProfile);

	void reloadProfileCommonValue(int p_selectedCartridgeIndex);
	void resetProfileAdhesionValue(int raftType = -1, int raftIdx = -1);
	void resetOriginalAdhesionValue(int raftType = -1, int raftIdx = -1);
	void setSupportValueUI(int targetCartridgeIdx = -1);
	void resetProfileSupportValue(int supportType = -1, int supportIdx = -1);
	void resetOriginalSupportValue(int supportType = -1, int supportIdx = -1);
	void resetProfileCommonValue();
	void resetOriginalCommonValue();

	void setRaftValueUI(int targetCartridgeIdx = -1);
	void setCommonValueUI();
	void setAdhesionCurrentIdx(int idx);
	void setSupportCurrentIdx(int idx);

	void resetProfileNameList(QString pProfileName);
	void resetProfileNameList(std::vector<Generals::ProfileList> pProfileList);

	void setProfileNameListDisable() { ui.comboBox_profileNameList->setEnabled(false); };
	void setMaterialListDisable() { ui.comboBox_Material->setEnabled(false); };
	void setResetVisible(bool flag) { ui.pushButton_reset->setVisible(flag); };
	void setEditVisible(bool flag) { ui.pushButton_edit->setVisible(flag); };
	bool validateWipeTowerSize();
	bool isTemperatureLayerEnabled() { return ui.checkBox_temperature_layer_setting_enabled->isChecked(); };

	//for exception layer0_width_factor by UI//
	void setOriginValue_initial_layer_width_factor(double value) { ui.spinBox_initial_layer_width_factor->setOriginalValue(value); }
	void setProfileValue_initial_layer_width_factor(ProfileDataI data);// { ui.doubleSpinBox_initial_layer_width_factor->setProfileValue(data); }
	//void showBedSideFrame(bool flag) {
	//	ui.frame_bedside->setVisible(flag); };

public slots:
	void setRAFTstate(bool);
	void calculateWipeTowerSize();
	void addTemperatureLayerSetPoint(SliceProfile* sliceProfile_, int pLayerNr = -999, int pTemperature = -999, bool pInsertFlag = true);
	void addTemperatureLayerSetPoint();
	void deleteTemperatureLayerSetPoint(SliceProfile* sliceProfile_);
	void deleteTemperatureLayerSetPoint();
	void tableWidget_temperature_layer_list_itemChanged(QTableWidgetItem *);
	void clearTemperatureLayerListWidget();
	void updateTemperatureLayerListWidget(SliceProfile* sliceProfile_);

private:
	Ui::ProfileSettingWidget ui;

	SliceProfile *m_sliceProfile;
	std::vector<SliceProfile> *m_sliceProfile_multi;
	SliceProfileForCommon *m_sliceProfile_common;
	std::vector<SliceProfile> *m_originalProfile_multi;
	SliceProfileForCommon *m_originalProfile_common;
	std::vector<int> m_usedCartIdx;

	int m_pre_support_cart_idx;
	int m_pre_adhesion_cart_idx;
	bool m_doNotChange;
	bool m_isRAFTstate; 

	void setUI();
	void setLinkedUI();
	void setTempUI();
	void setConnection();
	void setConnectControlforApplyButton();
	void setUIcontrolByMachineModel();

	ProfileDataI getBedTemperature();
	std::vector<int> getUsedCardIdx();
	QString m_profileName;

	void widgetFocusChange(bool in);

	bool isVerticalScrollBarVisible_tableWidget_temperature();


	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
	void setEnableApplyButton();
	void setDisableApplyButton();
	void checkMessage();
	void setRaftValue();
	void setCommonValue();
	void comboBox_profileNameList_currentIndexChanged(int idx);
	//void checkBox_layer0_temperature_enabled_toggled(bool check);
	void checkBox_wipetower_enable_toggled(bool check);
	void comboBox_wipetower_cartridgeIndex_currentIndexChanged(int idx);
	void checkBox_standby_temperature_enabled_toggled(bool check);
	void checkBox_preheat_enabled_toggled(bool check);
	void checkBox_raft_inset_toggled(bool check);
	void checkBox_support_interface_enabled_toggled(bool check);
	void checkBox_raft_temperature_control_toggled(bool check);
	void comboBox_support_cart_index_currentIndexChanged(int);
	void comboBox_adhesion_cart_index_currentIndexChanged(int);
	void comboBox_platform_adhesion_currentIndexChanged(int idx = -1);
	void comboBox_support_currentIndexChanged(int idx = -1);
	void checkBox_temperature_layer_setting_enabled(bool check);
	void checkBox_overall_flow_control_enabled(bool _flag);


	void warningMessageBedAdhesionType(int);

	void showWipeTowerInnerSize();

	void pushButton_reset_clicked();

	//void setFlowCurrentValue(int _value);
	//void setSpeedCurrentValue(double _value);

signals:
	void enableApplyButton(bool);
	void warningMessage(QString);
	void comboBox_profileNameList_change_sig(int idx);
	void comboBox_profileNameList_change_forUI(int idx);
	void tabWidget_profile_current_change_signal(int idx);
	void pushButton_edit_clicked_sig();
	void pushButton_reset_clicked_sig();
	void raftValueChanged();
	void commonValueChanged();
};
