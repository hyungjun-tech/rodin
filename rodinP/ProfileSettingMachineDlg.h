#pragma once

#include "ui_ProfileSettingMachineDlg.h"

class ProfileSettingMachineDlg : public QDialog
{
	Q_OBJECT

public:
	ProfileSettingMachineDlg(QWidget *parent);
	~ProfileSettingMachineDlg();

private slots:
	void pushButton_ok_clicked();
	void pushButton_apply_clicked();
	void checkBox_machine_center_is_zero_stateChanged(int state);
	void comboBox_machineList_currentIndexChanged(int);
	void setEnabledApplyButton();
	void setDisabledApplyButton();
	void groupBox_expanded_print_mode_toggled(bool state);
	void groupBox_authentification_mode_toggled(bool state);
	void comboBox_authentificationMethod_currentIndexChanged(int);
	void pushButton_authentification_info_clicked();

private:
	Ui::ProfileSettingMachineDlg ui;

	MachineProfile m_machine_profile;

	bool b_enabledApplyButton;
	QString m_emailAddress;
	int m_emailCount;
	QStringList m_cartridgeList;

	//QString m_profilePath;
	void setUI();
	void setConnection();
	void setConnectControlforApplyButton();
	void setInitValue();
	void loadSettingValue();
	bool saveSettingValue();

	//int stringMatchCount(std::string, char);
	bool validateEmailSingleAddress(QString);
	//void changeModel(QString model);
	void setModelEnabled(bool);
	bool validateEmailString(QString _email);
	void resetCartridgeList();
	void setBedSize();

signals:
	void signal_changeMachine(QString, bool);
	void signal_profileChanged(bool flag_ = true);
	void signal_expandedPrintModeChanged();
	void signal_componentChanged();
};
