#include "stdafx.h"
#include "CustomProfileEditor.h"
#include "SliceProfile.h"
#include "SliceProfileForCommon.h"
#include "CartridgeInfo.h"

CustomProfileEditor::CustomProfileEditor(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);

	setUI();
	setConnection();
	m_sliceProfile = new SliceProfile;
	m_sliceProfile_common = new SliceProfileForCommon;
	ui.widget->setEditVisible(false);
}

CustomProfileEditor::~CustomProfileEditor()
{

}

void CustomProfileEditor::setUI()
{
}

void CustomProfileEditor::setConnection()
{
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(pushButton_ok_clicked()));
	//connect(ui.pushButton_cancel, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.pushButton_saveProfile, SIGNAL(clicked()), this, SLOT(pushButton_apply_clicked()));

	connect(ui.widget, SIGNAL(raftValueChanged()), ui.widget, SLOT(setRaftValue()));
	connect(ui.widget, SIGNAL(commonValueChanged()), ui.widget, SLOT(setCommonValue()));

	connect(ui.widget, SIGNAL(warningMessage(QString)), this, SLOT(showMessage(QString)));

	connect(ui.widget, SIGNAL(enableApplyButton(bool)), ui.pushButton_saveProfile, SLOT(setEnabled(bool)));
}

void CustomProfileEditor::showMessage(QString pMessage)
{
	CommonDialog comDlg(this, pMessage, CommonDialog::Warning);
}

bool CustomProfileEditor::loadProfile(QString pProfileFileName, QString pProfileName)
{
	m_profileFileName = pProfileFileName;
	QString l_profilePath = Profile::customProfilePath + pProfileFileName;
	if (!m_sliceProfile->loadSliceProfile(Generals::qstringTowchar_t(l_profilePath)))
		return false;

	if (!m_sliceProfile->loadTemperatureLayerList(Generals::qstringTowchar_t(l_profilePath)))
		return false;

	m_sliceProfile->profile_name.value = pProfileName;
	m_sliceProfile->saveSliceProfile(Generals::qstringTowchar_t(l_profilePath));
	m_sliceProfile->saveTemperatureLayerList(Generals::qstringTowchar_t(l_profilePath));
	//파일에서 다시 Load하지 않고 sliceProfile에서 추출
	/*if (!m_sliceProfile_common->SetProfileDataDefault2(Generals::qstringTowchar_t(l_profilePath)))
	{
		return false;
	}*/
	m_sliceProfile_common->getCommonProfileFromSliceProfile(m_sliceProfile);
	ui.widget->resetProfileNameList(pProfileName);
	loadSettingValue(m_sliceProfile, m_sliceProfile_common, m_sliceProfile, m_sliceProfile_common);
	return true;
}

bool CustomProfileEditor::loadProfile(QString pProfileFileName, QString pProfileName, SliceProfile *pSliceProfile, SliceProfileForCommon *pSliceProfile_common, SliceProfile *pOriginalSliceProfile, SliceProfileForCommon *pOriginalSliceProfileCommon)
{
	m_profileFileName = pProfileFileName;
	m_sliceProfile = pSliceProfile;
	m_sliceProfile_common = pSliceProfile_common;

	ui.widget->resetProfileNameList(pProfileName);
	loadSettingValue(pSliceProfile, pSliceProfile_common, pOriginalSliceProfile, pOriginalSliceProfileCommon);
	return true;
}

void CustomProfileEditor::loadSettingValue(SliceProfile *pSliceProfile, SliceProfileForCommon *pSliceProfile_common, SliceProfile *pOriginalSliceProfile, SliceProfileForCommon *pOriginalSliceProfileCommon)
{
	//SJ
	ui.widget->setParameter(pSliceProfile, pSliceProfile_common, CartridgeInfo::getUsedCartIdx());
	//ui.widget_common->setParameter(*pSliceProfile, pSliceProfile_common, false, { 0 });
	/*ui.widget->loadProfileValue(m_sliceProfile_multi->at(selectedCartridgeIndex));
	ui.widget->loadProfileCommonValue(*m_sliceProfile_common);
	ui.widget_common->loadProfileCommonValue(*m_sliceProfile_common);*/
	ui.widget->loadProfileValue(*pSliceProfile);
	ui.widget->loadOriginalValue(*pOriginalSliceProfile);
	ui.widget->loadOriginalCommonValue(*pOriginalSliceProfileCommon);
	//ui.widget_common->loadOriginalValue(*pOriginalSliceProfileCommon);
	ui.pushButton_saveProfile->setEnabled(false);
	ui.widget->setProfileNameListDisable();
}

void CustomProfileEditor::pushButton_ok_clicked()	//accept() overloaded
{
	this->saveSettingValue();

	emit edit_ok_sig(m_sliceProfile, m_sliceProfile_common);

	this->close();
}

void CustomProfileEditor::pushButton_apply_clicked()
{
	saveSettingValue();

	//original 다시 설정 후 색상 다시 칠해야 함.
	ui.widget->loadOriginalValue(*m_sliceProfile);
	ui.widget->loadOriginalCommonValue(*m_sliceProfile_common);
	//ui.widget_common->loadOriginalValue(*m_sliceProfile_common);


	ui.pushButton_saveProfile->setEnabled(false);
}

void CustomProfileEditor::saveSettingValue()
{
	ui.widget->saveProfileValue(m_sliceProfile);
	ui.widget->saveProfileCommonValue();
	ui.widget->saveTemperatureListForCustom(m_sliceProfile);

	//프로파일 대상 파일에 저장하는 동작 필요
	m_sliceProfile_common->setSliceProfileDataFromCommon(m_sliceProfile);
	m_sliceProfile->saveSliceProfile(Generals::qstringTowchar_t(Profile::customProfilePath + m_profileFileName));
	m_sliceProfile->saveTemperatureLayerList(Generals::qstringTowchar_t(Profile::customProfilePath + m_profileFileName));

	emit edit_finished_sig(m_sliceProfile, m_sliceProfile_common);
}