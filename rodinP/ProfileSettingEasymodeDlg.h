#pragma once

#include "ui_ProfileSettingEasymodeDlg.h"
#include "SliceProfile.h"
#include "SliceProfileForCommon.h"

//mode tooltip values : layer height, infill density, print speed//
struct ModeTooltipValues
{
	double layer_height = 0.0;
	int infill_density = 0;
	double print_speed = 0.0;
};

class ProfileSettingEasymodeDlg : public QDialog
{
	Q_OBJECT

public:
	ProfileSettingEasymodeDlg(QWidget *parent, QString& easyModeProp_);
	~ProfileSettingEasymodeDlg();

private slots:
	void setEnabledApplyButton();
	void pushButton_ok_clicked();
	void pushButton_apply_clicked();
	void comboBox_support_cartridge_index_currentIndexChanged(int index_);
	void comboBox_material_cartridge_1_currentTextChanged(QString text_);
	void comboBox_material_cartridge_2_currentTextChanged(QString text_);
	void comboBox_wipeToewer_cartridgeIndex_currentIndexChanged(int);
	void checkBox_wipetower_enable_toggled(bool);
	void comboBox_support_currentIndexChanged(int _index);
	void radioButtonChanged();

private:
	void setUI();
	void setConnectControlforApplyButton();
	void setDisabledApplyButton();
	void loadSettingValue();
	//machine model change//
	void setUIcontrolByMachineModel();
	void setModeTooltipString();

	void setSupportIndex(int supportIndex);
	void setMaterialIndex_cart(int _cartridge_index, QString materialStr_, QString supportMaterialStr_ = "");

	void resetCartridgeUI(int cartridgeCount);
	void resetMaterialUI(int cartridgeCount);

	void saveSettingValue(void);
	void setSpinBoxValue(QDoubleSpinBox *targetUI, ProfileDataD profileValue);
	bool checkBedAdhesionVisible(QString _check_material, int _cartridge_index);
private:
	Ui::ProfileSettingEasymodeDlg ui;

	QString& easyModeProp;
	QString m_modeStr;
	std::vector<SliceProfile> *m_sliceProfile_multi;
	SliceProfileForCommon *m_sliceProfileForCommon;
	bool b_enabledApplyButton;
	QString m_bed_type_material;

signals:
	void signal_profileChanged(bool flag_ = true);
	void signal_componentChanged();
};

