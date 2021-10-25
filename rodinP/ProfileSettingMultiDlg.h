#pragma once
#include "ui_ProfileSettingMultiDlg.h"
#include "SliceProfile.h"
#include "SliceProfileForCommon.h"

class QStringListModel;

class ProfileSettingMultiDlg : public QDialog
{
	Q_OBJECT

public:
	ProfileSettingMultiDlg(QWidget *parent);
	~ProfileSettingMultiDlg();

	bool b_enabledApplyButton;
	int cartridgeTotalcnt;
	int selectedCartridgeIndex;
	int pre_selectedCartridgeIndex;

	void loadSettingValue();

	void setUI();
	void setConnection();

	void saveTempProfileToCartridgeProfile();
	void saveCartirdgeProfileToTempProfile();

	////machine model change//
	void setUIcontrolByMachineModel();
	//void changeValueByBedSide_All(int bedInfo);
	//void changeValueByBedSide_originalValue(int bedInfo);
	//void changeValueByBedSide_settingValue(int bedInfo);
	//UI setting by platforom adhesion change//

	void resetCartridgeUI(int cartridgeCount);

	//public slots:
	//	void setProfileByBedSide(bool advancedMode);

private slots:
	void accept();
	void cancel();
	void apply();

	//void temp();
	////void CheckRaftTemperatureControl(int);

	//void comboBox_Material_currentIndexChanged(int);
	void comboBox_profileNameList_currentIndexChanged(int idx = -1);
	//void SetMaterialIndex2(int materialIdx, int print_bed_temperature);

	void updateCartridgeUIvalue(std::vector<SliceProfile>*, int); //cartridge index가 변경될때 i번째 cartridge를 화면에 보여줌..

	//void loadValueFromProfile();


	void saveSettingValue();

	void setEnabledApplyButton(bool enabledApplyButton);

	//void listWidget_cartridge_list_currentRowChanged(int);
	void comboBox_cartridge_list_currentIndexChanged(int idx);
	void comboBox_profileNameList_currentIndexChanged_forUI(int);
	void showMessage(QString pMessage);
	void showCustomProfileEditor();
	void resetOriginalValue(SliceProfile* sliceProfile, SliceProfileForCommon* sliceProfileCommon);
	void resetUIValue(SliceProfile* sliceProfile, SliceProfileForCommon* sliceProfileCommon);
	void setOriginalValue();
	void tabWidget_profile_currentChanged(int _idx);
private:
	Ui::ProfileSettingMultiDlg ui;

	std::vector<SliceProfile> *m_sliceProfile_multi;
	std::vector<SliceProfile> *m_tempSliceProfile; //UI를 위한 임시용 profile
	SliceProfileForCommon *m_sliceProfile_common;
	SliceProfileForCommon *m_tempSliceProfile_common;

	std::vector<SliceProfile> *m_originalProfile; //originalValue
	SliceProfileForCommon *m_originalProfile_common;
	int m_selectedProfileIdx;

	void initProfile();

	//QList<QColor> m_colorList;
	QStringList m_colorList;
	QString m_color_common;
	bool m_do_not_change;

	////temperature Unit change function//
	//void SetUIcontrolByBedAdhesion(int idx = -1);

	std::vector<Generals::ProfileList> m_profileList;
	void setOriginalValue(int idx);
signals:
	void signal_profileChanged(bool flag_ = true);
};

