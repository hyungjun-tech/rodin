#pragma once

#include "ui_CustomProfileEditor.h"

class SliceProfile;
class SliceProfileForCommon;
class CustomProfileEditor : public QDialog
{
	Q_OBJECT
public:
	CustomProfileEditor(QWidget *parent = 0);
	~CustomProfileEditor();

	bool loadProfile(QString pProfileFileName, QString pProfileName);
	bool loadProfile(QString pProfileFileName, QString pProfileName, SliceProfile *pSliceProfile, SliceProfileForCommon *pSliceProfile_common, SliceProfile *pOriginalSliceProfile, SliceProfileForCommon *pOriginalSliceProfileCommon);
private:
	Ui::CustomProfileEditor ui;
	QString m_profileFileName;

	void setConnection();
	void loadSettingValue(SliceProfile *pSliceProfile, SliceProfileForCommon *pSliceProfile_common, SliceProfile *pOriginalSliceProfile, SliceProfileForCommon *pOriginalSliceProfileCommon);
	SliceProfile *m_sliceProfile;
	SliceProfileForCommon *m_sliceProfile_common;

	void saveSettingValue();
private slots:
	void setUI();
	void showMessage(QString pMessage);
	void pushButton_ok_clicked();
	void pushButton_apply_clicked();
signals:
	void edit_finished_sig(SliceProfile*, SliceProfileForCommon*);
	void edit_ok_sig(SliceProfile*, SliceProfileForCommon*);
};