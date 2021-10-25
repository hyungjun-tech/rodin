#include "stdafx.h"
#include "rodinP.h"

#include "MultiplyDialog.h"
#include "VolumAnalysisDialog.h"
#include "OptimizedDialog.h"
#include "ShortcutInfoDialog.h"
#include "InformationDialog.h"
#include "ScaleDialog.h"
#include "MoveDialog.h"
#include "RotationDialog.h"
#include "SelectModelDialog.h"
#include "ProfileSettingMultiDlg.h"
#include "ProfileSettingEasymodeDlg.h"
#include "ProfileSettingMachineDlg.h"
#include "EditStartEndcodeDlg.h"
#include "SelectCartridgeDialog.h"
#include "SelectCartridgePopDialog.h"
#include "MyPrinters.h"
#include "AddMyPrinter.h"
#include "AddIPAddress.h"
#include "CartridgeDialog.h"
#include "AuthentificationDlg.h"
#include "PrintConfirm.h"
#include "ColoredLayerDialog.h"
#include "CustomProfileList.h"
#include "PreviewThumbnailDialog.h"
#include "ExperimentalFunctions.h"

#include "FindPrinterInfo.h"
#include "Switch.h"
#include "CartridgeInfo.h"
#include "ProfileControl.h"

#include "WebControl.h"
#include "ConnectionControl.h"

#include "SliceProcessor.h"
#include "GCodeGenerator.h"
#include "GCodeParser.h"
#include "SaveFileUIMode.h"
#include "FileCopier.h"

#include "ProfileToConfig.h"

#include "PrinterInfo.h"
#include "ProfileControl.h"
#include "printControl.h"

#include "ModelMatchList.h"

#define RELEASE_TYPE 0	// 0=PACKAGING TYPE, 1=FOLDER TYPE

rodinP::rodinP(QWidget *parent)
	: QMainWindow(parent)
	//, m_VolumeOverlapCheck(false)
	, m_fileDialogOpen(false)
	, layerCount(0)
	, sidePopupDialog(nullptr)
	, modelControlDialog(nullptr)
{
	//슬라이스 프로파일 버전 관리.
	//프로파일의 항목이 추가되거나 이름이 변경된 경우 버전을 올려주세요.
	profileVersion = "2.9";

	getInitValue();
	setUI();
	init();
	setConnection();
	Logger() << "Start Application";
}

rodinP::~rodinP()
{
	QString usingCustomColor = CartridgeInfo::usingCustomColor == true ? "Y" : "N";
	Generals::setProps("usingCustomColor", usingCustomColor);

	for (int i = 0; i < CartridgeInfo::customColors.size(); i++)
	{
		Generals::setProps("CustomColor_" + QString::number(i), CartridgeInfo::getCustomColorString(i));
	}

	ProfileControl::saveRecentProfile(m_settingMode);
	ProfileControl::saveRecentMachineProfile(Profile::machineProfile);

	Generals::setProps("profileVersion", profileVersion);
	Generals::setProps("auth_mode", QString::number(UserProperties::authentification_print_mode));
	Generals::setProps("auth_method", QString::number(UserProperties::authentification_print_setting_method));

	QFileInfoList allFiles = QDir(Generals::getTempFolderPath()).entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
	for (auto it : allFiles)
	{
		if (it.isFile())
			QFile::remove(it.absoluteFilePath());
	}
	QFile::remove(modelContainer->printingInfo->getGcodeTempFileName());
	Logger() << "Exit Application";
}



#pragma region initialize
void rodinP::setUI()
{
	ui.setupUi(this);
	modelContainer = ui.viewerModule->modelContainer;

	QString platform_;
#if (x64_PLATFORM == 1)
	platform_ = " (x64 build)";
#endif
	//this->setWindowTitle(AppInfo::getAppName());
	this->setWindowTitle(AppInfo::getAppName() + platform_);
	QString companyName = AppInfo::getCompanyName();

	if (companyName != "Sindoh")
	{
		ui.label_ci->setPixmap(QPixmap(QString::fromUtf8(":/rodinP/Resources/ci_ver_s.png")));
		this->setWindowIcon(QPixmap(QString::fromUtf8(":/rodinP/Resources/logo_s.png")));
		ui.label_ci->setFixedHeight(27);
		ui.label_ci->setMargin(0);

		ui.pushButton_isometric->setIcon(QIcon(QString::fromUtf8(":/rodinP/Resources/isometric-grey.png")));
		ui.pushButton_front->setIcon(QIcon(QString::fromUtf8(":/rodinP/Resources/front-grey.png")));
		ui.pushButton_top->setIcon(QIcon(QString::fromUtf8(":/rodinP/Resources/top-grey.png")));
		ui.pushButton_left->setIcon(QIcon(QString::fromUtf8(":/rodinP/Resources/left-grey.png")));
		ui.pushButton_right->setIcon(QIcon(QString::fromUtf8(":/rodinP/Resources/right-grey.png")));
		
		QString menuBarStyle("#menubarFrame, #pilaInfo, #rightMenuFrame{background-color: rgb(0, 0, 0);}\n\nQPushButton\n{\nborder-radius: 0px;\n}\nQPushButton:hover:enabled\n{\nbackground-color:grey;\n}\nQPushButton:checked \n{\nbackground-color: rgb(110, 110, 110);\nborder-style: inset;\nborder-color: rgb(70,70,70);\nborder-width: 1px\n}\n\nQPushButton:pressed \n{\nbackground-color: rgb(110, 110, 110);\nborder-style: inset;\nborder-color: rgb(70,70,70);\nborder-width: 1px\n}\n");
		ui.menubarFrame->setStyleSheet(menuBarStyle);
		ui.rightMenuFrame->setStyleSheet(menuBarStyle);

		ui.menuLanguage->removeAction(ui.actionKorean);
		ui.menuLanguage->removeAction(ui.actionChinese);
		ui.menuLanguage->removeAction(ui.actionJapanese);
		ui.menuLanguage->removeAction(ui.actionGerman);

	}

	if (AppInfo::getAppName() == "Rhea")
		ui.actionUpdate->setEnabled(false);

	ui.actionEasy_Mode->setVisible(true);
	ui.actionLoad_GCode->setVisible(true);
	ui.actionSlice->setVisible(false);

	//양산을 위해 replication 기능 unvisible..//
	ui.actionReplication_Print->setVisible(false);
	
	setEnableMenuForModel();

	setAcceptDrops(true);

	QIcon radSize1, radSize2, radSize3, radSize4, radSize5;
	radSize1.addFile(":/rodinP/Resources/radiusSize1.png", QSize(), QIcon::Normal, QIcon::Off);
	radSize2.addFile(":/rodinP/Resources/radiusSize2.png", QSize(), QIcon::Normal, QIcon::Off);
	radSize3.addFile(":/rodinP/Resources/radiusSize3.png", QSize(), QIcon::Normal, QIcon::Off);
	radSize4.addFile(":/rodinP/Resources/radiusSize4.png", QSize(), QIcon::Normal, QIcon::Off);
	radSize5.addFile(":/rodinP/Resources/radiusSize5.png", QSize(), QIcon::Normal, QIcon::Off);
	ui.comboBox_radiusSize->addItem(radSize1, "1");
	ui.comboBox_radiusSize->addItem(radSize2, "2");
	ui.comboBox_radiusSize->addItem(radSize3, "3");
	ui.comboBox_radiusSize->addItem(radSize4, "4");
	ui.comboBox_radiusSize->addItem(radSize5, "5");
	ui.comboBox_radiusSize->setCurrentIndex(2);
	setBrushRadius(2);

	ui.actionNormal_Viewer->setEnabled(false);
	QActionGroup* menuModeGroup = new QActionGroup(ui.menuMode);
	menuModeGroup->setExclusive(true);
	menuModeGroup->addAction(ui.actionEasy_Mode);
	menuModeGroup->addAction(ui.actionAdvanced_Mode);
	connect(menuModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(menuModeGroup_triggered(QAction *)));

	ui.checkBox_showOnlyCurrentPath->setChecked(false);


	QButtonGroup* viewerGroup = new QButtonGroup(this);
	viewerGroup->setExclusive(true);
	viewerGroup->addButton(ui.pushButton_normalViewer);
	viewerGroup->addButton(ui.pushButton_editViewer);
	viewerGroup->addButton(ui.pushButton_layerViewer);
	//connect(viewerGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(viewerButtonClicked(QAbstractButton *)));


	//experimenatal function unvisible..//
	ui.actionReplace_Profile->setVisible(false);
	ui.actionExperimental_Function->setVisible(false);
}

void rodinP::getInitValue()
{
	Logger::setLogFileName(true);
	QCoreApplication::translate("MAC_APPLICATION_MENU", "Services");
	QCoreApplication::translate("MAC_APPLICATION_MENU", "Hide %1");
	QCoreApplication::translate("MAC_APPLICATION_MENU", "Hide Others");
	QCoreApplication::translate("MAC_APPLICATION_MENU", "Show All");
	QCoreApplication::translate("MAC_APPLICATION_MENU", "Preferences...");
	QCoreApplication::translate("MAC_APPLICATION_MENU", "Quit %1");
	QCoreApplication::translate("MAC_APPLICATION_MENU", "About %1");
	QCoreApplication::translate("rodinPClass", "EditSliderHint_Mac");
	QCoreApplication::translate("ShortcutInfoDialog", "fn + Delete");
	QCoreApplication::translate("ShortcutInfoDialog", "command + Left");
	Generals::appPath = Generals::getAppPath();
	Generals::appDataPath = Generals::getAppDataPath();
	
	defaultLocale = Generals::getProps("lang");
	//props 파일에 정보가 없는 locale정보 활용
	if (defaultLocale == "")
	{
		
		if (AppInfo::getCompanyName() != "Sindoh")
		{
			defaultLocale = "en";
			Generals::setProps("lang", defaultLocale);
		}
		else
		{
			//qDebug() << QLocale::system().name();
			defaultLocale = QLocale::system().name(); // e.g. "de_DE"
			defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
			Generals::setProps("lang", defaultLocale);
		}
	}

	QString transFileName = "rodinp_%1.qm";
	transFileName = transFileName.arg(defaultLocale);

	if (translator.load(transFileName, Generals::appPath + "/languages"))
	{
		bool rtn = QApplication::installTranslator(&translator);
		if (!rtn)
		{
			Logger() << "Cannot find translate file";
			return;
		}
	}
	//파일을 찾지 못하는 경우 en 파일을 defualt로 찾아줌.
	else if (translator.load("rodinp_en.qm", Generals::appPath + "/languages"))
	{
		bool rtn = QApplication::installTranslator(&translator);
		if (!rtn)
		{
			Logger() << "Cannot find translate file";
			return;
		}
	}

	//mode property//////////////////
	m_settingMode = Generals::getProps("mode");
	if (m_settingMode == "")
	{
		m_settingMode = QString("easy");
		Generals::setProps("mode", m_settingMode);
	}
	//현재 easy mode는 구현중...//
	//m_settingMode = "advanced";

	//easy mode property//////////////////
	//m_easyModeProp = props.value("EasyProp").toString();
	m_easyModeProp = QString("normal");

	//temperature unit//////////////////
	Generals::temperatureUnit = Generals::getProps("temperatureUnit");
	if (Generals::temperatureUnit == "")
	{
		Generals::temperatureUnit = QString("C");
		Generals::setProps("temperatureUnit", Generals::temperatureUnit);
	}
	ModelMatchList::setMatchList();

	UserProperties::authentification_print_mode = Generals::getProps("auth_mode").toInt();
	UserProperties::authentification_print_setting_method = Generals::getProps("auth_method").toInt();
}

void rodinP::init()
{
	CustomProfileList::copyCustomProfileFiles();
	if (profileVersion != Generals::getProps("profileVersion"))
	{
		QDir d = QDir(Generals::appDataPath + "/profile");
		d.removeRecursively();
	}

	//////////////////////////////////////////////////////////////////////////////////
	//기존에 연결된 프린터가 있으면 defaultModel로 설정..//
	getDefaultPrinter();
	//////////////////////////////////////////////////////////////////////////////////

	

	//////////////////////////////////////////////////////////////////////////////////
	//machine model setting --> getCartridgeCountTotal() value setting..//
	//연결된 프린터가 없을 경우, l_defaultModel로 설정..//
	if (Profile::machineProfile.machine_model.isEmpty())
	{
		QString l_defaultModel = Generals::getProps("model");
		//defaultModel = getProps("model");
		if (l_defaultModel != "")
		{
			QStringList dirs = QDir(Generals::appPath + "\\profile").entryList(QDir::Dirs | QDir::NoDotAndDotDot);
			bool check = false;
			for (int i = 0; i < dirs.size(); i++)
			{
				if (dirs.at(i) == l_defaultModel)
					check = true;
			}
			if (!check)
				l_defaultModel = "";
		}

		if (l_defaultModel == "")
		{
			SelectModelDialog selectModelDlg(this);
			if (selectModelDlg.isCancel())
			{
				exit(0);
			}
			l_defaultModel = Generals::getProps("model");
		}
		PrinterInfo::clear(l_defaultModel);
		changeMachine(l_defaultModel);
	}
	//////////////////////////////////////////////////////////////////////////////////


	//multi-cartridge dialog///////////////////////////////////////////////////////
	/*int extruderCount = Profile::getCartridgeTotalCount();
	Profile::sliceProfile.resize(extruderCount);
	Profile::configSettings.resize(extruderCount);*/


	//registry 등록된 값을 카트리지 정보에 등록
	CartridgeInfo::usingCustomColor = Generals::getProps("usingCustomColor") == "Y" ? true : false;
	for (int i = 0; i < Profile::getCartridgeTotalCount(); i++)
	{
		CartridgeInfo::customColors.at(i) = Generals::getProps("CustomColor_" + QString::number(i));
	}
	//m_Processor3.setConfig(m_Config_multi);//to do sj

	if (AppInfo::getAppName() != "Rhea")
		checkVersion *chkVer = new checkVersion(this, AppInfo::getCurrentVersion(), false);

	this->setSettingMode(m_settingMode);

	this->setTemperatureUnit();
	this->createLangMenu(defaultLocale);
	setEnableMenuForModel();



	//////////////////////////////////////////////////////////////////////////////////
	//profile reset patch//
	QString props_version = QString("initial_profile_reset_") + AppInfo::getCurrentVersion();
	QString init_profile_reset = Generals::getProps(props_version);
	if (init_profile_reset == "" || init_profile_reset == "false")
	{
		ProfileControl::resetProfile();
		afterProfileChanged();

		init_profile_reset = QString("true");
		Generals::setProps(props_version, init_profile_reset);
	}
	//////////////////////////////////////////////////////////////////////////////////

}

void rodinP::setConnection()
{
	//menuFile
	connect(ui.actionLoad_model_file, SIGNAL(triggered(bool)), this, SLOT(actionLoad_model_file_triggered()));
	connect(ui.actionSave_model_file, SIGNAL(triggered(bool)), this, SLOT(actionSave_model_file_triggered()));
	connect(ui.actionLoad_GCode, SIGNAL(triggered(bool)), this, SLOT(actionLoad_GCode_triggered()));
	connect(ui.actionSave_GCode, SIGNAL(triggered(bool)), this, SLOT(actionSave_GCode_triggered()));
	connect(ui.actionSave_GCode_printer, SIGNAL(triggered(bool)), this, SLOT(actionSave_GCode_printer_triggered()));
	connect(ui.actionInsert_image_toGcode, SIGNAL(triggered(bool)), this, SLOT(actionInsert_image_toGcode_triggered()));
	connect(ui.actionDelete_image_inGcode, SIGNAL(triggered(bool)), this, SLOT(actionDelete_image_inGcode_triggered()));
	connect(ui.actionPrint, SIGNAL(triggered(bool)), this, SLOT(actionPrint_triggered()));

	//menuProfile
	connect(ui.actionCustom_Profile, SIGNAL(triggered(bool)), this, SLOT(actionCustom_Profile_triggered()));
	connect(ui.actionImport_Profile, SIGNAL(triggered(bool)), this, SLOT(actionImport_Profile_triggered()));
	connect(ui.actionExport_Profile, SIGNAL(triggered(bool)), this, SLOT(actionExport_Profile_triggered()));
	connect(ui.actionReset_Profile, SIGNAL(triggered(bool)), this, SLOT(actionReset_Profile_triggered()));
	connect(ui.actionReplace_Profile, SIGNAL(triggered(bool)), this, SLOT(actionReplace_Profile_triggered()));

	//menuSetting
	connect(ui.actionMachine_Setting, SIGNAL(triggered(bool)), this, SLOT(actionMachine_Setting_triggered()));
	connect(ui.actionCartridge_Information, SIGNAL(triggered(bool)), this, SLOT(actionCartridge_Information_triggered()));
	connect(ui.actionStartEnd_Gcode, SIGNAL(triggered(bool)), this, SLOT(actionStartEnd_Gcode_triggered()));

	//menuAction
	connect(ui.actionHome, SIGNAL(triggered(bool)), this, SLOT(actionHome_triggered()));
	connect(ui.actionPosition, SIGNAL(triggered(bool)), this, SLOT(actionPosition_triggered()));
	connect(ui.actionScale, SIGNAL(triggered(bool)), this, SLOT(actionScale_triggered()));
	connect(ui.actionRotation, SIGNAL(triggered(bool)), this, SLOT(actionRotation_triggered()));
	connect(ui.actionNormal_Viewer, SIGNAL(triggered(bool)), this, SLOT(actionNormal_Viewer_triggered()));
	connect(ui.actionEdit_Support, SIGNAL(triggered(bool)), this, SLOT(actionEdit_Support_triggered()));
	connect(ui.actionLayer_Viewer, SIGNAL(triggered(bool)), this, SLOT(actionLayer_Viewer_triggered()));

	//menuDevice
	connect(ui.actionMy_Printers, SIGNAL(triggered(bool)), this, SLOT(actionMy_Printers_triggered()));
	connect(ui.actionAdd_My_Printer, SIGNAL(triggered(bool)), this, SLOT(actionAdd_My_Printer_triggered()));
	connect(ui.actionAdd_Local_Printer, SIGNAL(triggered(bool)), this, SLOT(actionAdd_Local_Printer_triggered()));
	connect(ui.actionAdd_IP_Address, SIGNAL(triggered(bool)), this, SLOT(actionAdd_IP_Address_triggered()));//initForm
	connect(ui.actionWeb_Browser, SIGNAL(triggered(bool)), this, SLOT(actionWeb_Browser_triggered()));
	connect(ui.actionReplication_Print, SIGNAL(toggled(bool)), this, SLOT(actionReplication_Print_triggered()));

	//menuAnalysis
	connect(ui.actionThickness_Overhang, SIGNAL(triggered(bool)), this, SLOT(actionThickness_Overhang_triggered()));
	connect(ui.actionOptimized_Direction, SIGNAL(triggered(bool)), this, SLOT(actionOptimized_Direction_triggered()));
	
	connect(ui.actionOnline_FAQ, SIGNAL(triggered(bool)), this, SLOT(actionOnline_FAQ_triggered()));
	connect(ui.actionShortcutInfo, SIGNAL(triggered(bool)), this, SLOT(actionShortcutInfo_triggered()));
	connect(ui.actionUpdate, SIGNAL(triggered(bool)), this, SLOT(actionUpdate_triggered()));
	connect(ui.actionInformation, SIGNAL(triggered(bool)), this, SLOT(actionInformation_triggered()));
	
	//left menubar
	connect(ui.pushButton_loadIcon, SIGNAL(clicked()), this, SLOT(pushButton_loadIcon_clicked()));
	connect(ui.pushButton_settingIcon, SIGNAL(clicked()), this, SLOT(pushButton_settingIcon_clicked()));
	//right menubar
	connect(ui.pushButton_normalViewer, SIGNAL(clicked(bool)), this, SLOT(pushButton_normalViewer_clicked(bool)));
	connect(ui.pushButton_editViewer, SIGNAL(clicked(bool)), this, SLOT(pushButton_editViewer_clicked(bool)));
	connect(ui.pushButton_layerViewer, SIGNAL(clicked(bool)), this, SLOT(pushButton_layerViewer_clicked(bool)));
	connect(ui.pushButton_print, SIGNAL(clicked()), this, SLOT(pushButton_print_clicked()));
	connect(ui.pushButton_move, SIGNAL(clicked()), this, SLOT(pushButton_move_clicked()));
	connect(ui.pushButton_scale, SIGNAL(clicked()), this, SLOT(pushButton_scale_clicked()));
	connect(ui.pushButton_rotate, SIGNAL(clicked()), this, SLOT(pushButton_rotate_clicked()));

	connect(ui.switch_coloredLayer, SIGNAL(signal_switchChanged(bool)), this, SLOT(switch_coloredLayer_changed(bool)));
	connect(ui.slider_layer, SIGNAL(valueChanged(int)), this, SLOT(slider_layer_valueChanged(int)));
	connect(ui.comboBox_radiusSize, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_radiusSize_currentIndexChanged(int)));
	connect(ui.slider_supportEdit, SIGNAL(valueChanged(int)), this, SLOT(slider_supportEdit_valueChanged(int)));
	connect(ui.checkBox_MoveLine, SIGNAL(toggled(bool)), this, SLOT(checkBox_MoveLine_toggled(bool)));
	connect(ui.checkBox_showOnlyCurrentPath, SIGNAL(toggled(bool)), this, SLOT(checkBox_showOnlyCurrentPath_toggled(bool)));
	
	//top menu
	connect(ui.pushButton_isometric, SIGNAL(clicked(bool)), this, SLOT(pushButton_isometric_clicked()));
	connect(ui.pushButton_top, SIGNAL(clicked(bool)), this, SLOT(pushButton_top_clicked()));
	connect(ui.pushButton_left, SIGNAL(clicked(bool)), this, SLOT(pushButton_left_clicked()));
	connect(ui.pushButton_right, SIGNAL(clicked(bool)), this, SLOT(pushButton_right_clicked()));
	connect(ui.pushButton_front, SIGNAL(clicked(bool)), this, SLOT(pushButton_front_clicked()));
	connect(ui.pushButton_selectCartridge, SIGNAL(clicked()), this, SLOT(pushButton_selectCartridge_clicked()));


	//others
	connect(ui.viewerModule, SIGNAL(signal_viewer_changed(Generals::ViewerMode)), this, SLOT(afterViewerChanged(Generals::ViewerMode)));
	connect(ui.viewerModule, SIGNAL(signal_updateMaxLayerForPreview(int)), this, SLOT(updateMaxLayerForPreview(int)));

	connect(modelContainer, SIGNAL(signal_modelSelect()), this, SLOT(setEnableMenuForModel()));
	connect(modelContainer, SIGNAL(signal_modelDeselect()), this, SLOT(setEnableMenuForModel()));
	connect(modelContainer, SIGNAL(signal_modelDeleted()), this, SLOT(setEnableMenuForModel()));
	
	connect(ui.actionExperimental_Function, SIGNAL(triggered(bool)), this, SLOT(actionExperimental_Function_triggered()));


	//connect(ui.actionReload_All, SIGNAL(triggered(bool)), this, SLOT(ReloadVolume_All()));
	//connect(ui.actionCheck_OpenGL_ver, SIGNAL(triggered(bool)), this, SLOT(CheckOpenGLversion()));
	//connect(ui.actionSlice, SIGNAL(triggered(bool)), this, SLOT(Slice()));
	//connect(ui.actionDelete_All, SIGNAL(triggered(bool)), ui.viewer, SLOT(DeleteAllModels()));
	//connect(ui.btn_coloredLayer, SIGNAL(coloredLayerSignal()), this, SLOT(setNormalViewerMode()));
}
#pragma endregion




#pragma region private slots for UI
void rodinP::pushButton_loadIcon_clicked() { loadFiles(); }
void rodinP::pushButton_settingIcon_clicked() {
	if (m_settingMode == "advanced")
		viewProfileSettingMultiDlg();
	else
		viewProfileSettingEasymodeDlg();
}
void rodinP::pushButton_normalViewer_clicked(bool checked) { if (!checked) return; setNormalViewerMode(); }
void rodinP::pushButton_editViewer_clicked(bool checked) { if (!checked) return; setEditViewerMode(); }
void rodinP::pushButton_layerViewer_clicked(bool checked) { if (!checked) return; setLayerViewerMode(); }
void rodinP::pushButton_print_clicked() { printModel(); }
void rodinP::pushButton_move_clicked() { showModelControlDlg(Generals::ModelControl::Move); }
void rodinP::pushButton_scale_clicked() { showModelControlDlg(Generals::ModelControl::Scale); }
void rodinP::pushButton_rotate_clicked() { showModelControlDlg(Generals::ModelControl::Rotate); }

void rodinP::pushButton_isometric_clicked() { ui.viewerModule->isometricCamera(); }
void rodinP::pushButton_top_clicked() { ui.viewerModule->topView(); }
void rodinP::pushButton_left_clicked() { ui.viewerModule->leftView(); }
void rodinP::pushButton_right_clicked() { ui.viewerModule->rightView(); }
void rodinP::pushButton_front_clicked() { ui.viewerModule->frontView(); }
void rodinP::pushButton_selectCartridge_clicked()
{
	if (ui.pushButton_selectCartridge->isChecked())
		showSelectCartridgeDlg();
	else
		closeControlDlg();
}

void rodinP::switch_coloredLayer_changed(bool switch_)
{
	Generals::isLayerColorMode = switch_;
	setNormalViewerMode();
	/*if (switch_)
	{
		ui.viewerModule->SetLayerColorUIMode();
	}
	else
		ui.viewerModule->SetTranslateUIMode();*/
}

void rodinP::checkBox_showOnlyCurrentPath_toggled(bool check_) { ui.viewerModule->toggleCurrentLayerOnly(check_); }
void rodinP::checkBox_MoveLine_toggled(bool check_) { ui.viewerModule->toggleShowTravelPath(check_); }
void rodinP::slider_layer_valueChanged(int value_)
{
	ui.label_CurLayerNo->setNum(value_);
	ui.viewerModule->changeLayerIndexForPreview(value_);
}
void rodinP::slider_supportEdit_valueChanged(int value_) { ui.viewerModule->changeLayerIndexForSupport(value_); }
void rodinP::comboBox_radiusSize_currentIndexChanged(int index_) { setBrushRadius(index_); }

void rodinP::actionLoad_model_file_triggered() { loadModelFile(); }
void rodinP::actionSave_model_file_triggered() { saveModelFile(); }
void rodinP::actionLoad_GCode_triggered() { loadPathfromGCode(); }
void rodinP::actionSave_GCode_triggered() { saveGCode(); }
void rodinP::actionSave_GCode_printer_triggered() { saveGCodeToPrinter(); }
void rodinP::actionInsert_image_toGcode_triggered() { insertImageToGCode(); }
void rodinP::actionDelete_image_inGcode_triggered() { deleteImageInGCode(); }
void rodinP::actionPrint_triggered() { printModel(); }

void rodinP::actionCustom_Profile_triggered() { viewProfileSettingCustomProfileDlg(); }
void rodinP::actionImport_Profile_triggered() { importProfile(); }
void rodinP::actionExport_Profile_triggered() { exportProfile(); }
void rodinP::actionReset_Profile_triggered() { resetProfile(); }
void rodinP::actionReplace_Profile_triggered() { replaceProfile(); }

void rodinP::actionMachine_Setting_triggered() { viewProfileSettingMachineDlg(); }
void rodinP::actionCartridge_Information_triggered() { showCartridgeInfoDlg(); }
void rodinP::actionStartEnd_Gcode_triggered() { viewStartEndGcodeDlg(); }

void rodinP::actionHome_triggered() { ui.viewerModule->isometricCamera(); }
void rodinP::actionPosition_triggered() { showModelControlDlg(Generals::ModelControl::Move); }
void rodinP::actionScale_triggered() { showModelControlDlg(Generals::ModelControl::Scale); }
void rodinP::actionRotation_triggered() { showModelControlDlg(Generals::ModelControl::Rotate); }
void rodinP::actionNormal_Viewer_triggered() { setNormalViewerMode(); }
void rodinP::actionEdit_Support_triggered() { setEditViewerMode(); }
void rodinP::actionLayer_Viewer_triggered() { setLayerViewerMode(); }

void rodinP::actionMy_Printers_triggered() { showMyPrinters(); }
void rodinP::actionAdd_My_Printer_triggered() { showAddNetPrinter(); }
void rodinP::actionAdd_Local_Printer_triggered() { showAddUSBPrinter(); }
void rodinP::actionAdd_IP_Address_triggered() { showFindIPPrinter(); }
void rodinP::actionWeb_Browser_triggered() { viewBrowser(); }
void rodinP::actionReplication_Print_triggered() { setReplicationPrintMode(); }

void rodinP::actionThickness_Overhang_triggered() { showAnalysisDialog(); }
void rodinP::actionOptimized_Direction_triggered() { showOptimizedDialog(); }

void rodinP::actionOnline_FAQ_triggered() { faqOpen(); }
void rodinP::actionShortcutInfo_triggered() { showShortcutInfoDialog(); }
void rodinP::actionUpdate_triggered() { chkVersion(); }
void rodinP::actionInformation_triggered() { showInformationDialog(); }
void rodinP::actionExperimental_Function_triggered() { excuteExperimentalFunction(); }
void rodinP::pushButton_cartridgeInfoClicked() { showCartridgeInfoDlg(); }

void rodinP::menuLanguageGroup_triggered(QAction* action)
{
	QString lang = action->property("lang").toString();
	QString fileName = "rodinp_%1.qm";
	fileName = fileName.arg(lang);

	QApplication::removeTranslator(&translator);

	if (translator.load(fileName, Generals::appPath + "/languages"))
	{
		qDebug() << fileName;
		QApplication::installTranslator(&translator);
	}

	//retranslate
	ui.retranslateUi(this);

	//if (icons->getPrintStringVisible()) setPrintString();

	//todo//setPrintString()변경 할 것..//
	if (Generals::currentViewerMode == Generals::ViewerMode::PreviewUIMode)
	{
		setPrintString();
	}

	if (!ui.switch_coloredLayer->isOn())
		ui.lab_coloredLayer->setText("<p align=""center""><span style=""color:#ffffff;"">" + CustomTranslate::tr("Layer Color") + "</span></p>");
	else
		ui.lab_coloredLayer->setText("<p align=""center""><span style=""color:#00f0d4;"">" + CustomTranslate::tr("Layer Color") + "</span></p>");

	ui.label_printingMode->setText(getCurrentMode());

	closeControlDlg();
	//props에 저장
	Generals::setProps("lang", lang);
}

void rodinP::menuModeGroup_triggered(QAction *action)
{
	QString temp_settingMode;
	temp_settingMode = action->property("mode").toString();

	if (m_settingMode == temp_settingMode)
		return;

	QString msg;
	if (temp_settingMode == "easy") msg = MessageQuestion::tr("change_to_easy_mode_confirm");
	else if (temp_settingMode == "advanced") msg = MessageQuestion::tr("change_to_advanced_mode_confirm");

	CommonDialog comDlg(this, msg, CommonDialog::Question, false, false, true, true);

	if (comDlg.isYes())
	{
		//advanced --> easy//
		setSettingMode(temp_settingMode);
		if (temp_settingMode == "easy")
		{
			//현재 프로파일을 recent 프로파일에 저장
			ProfileControl::saveRecentProfile(m_settingMode);

			m_easyModeProp = "normal";
			Generals::setProps("EasyProp", m_easyModeProp);
			ProfileControl::resetEasyProfile();
		}
		//easy --> advanced//
		else if (temp_settingMode == "advanced")
		{
			//recent load
			if (!ProfileControl::loadRecentProfile())
			{
				if (m_settingMode == "easy") ui.menuMode->actions().at(0)->setChecked(true);
				else if (m_settingMode == "advanced")  ui.menuMode->actions().at(1)->setChecked(true);
				return;
			}
		}
		m_settingMode = temp_settingMode;
		afterProfileChanged();

		CommonDialog comDlg(this, MessageInfo::tr("mode_changed"), CommonDialog::Information);
	}
	else if (comDlg.isNo()) //No할 경우, check는 그대로 유지..//
	{
		if (m_settingMode == "easy") ui.menuMode->actions().at(0)->setChecked(true);
		else if (m_settingMode == "advanced")  ui.menuMode->actions().at(1)->setChecked(true);

		return;
	}
}

void rodinP::menuTemperatureGroup_triggered(QAction *action)
{
	Generals::temperatureUnit = action->property("temperatureUnit").toString();
	//props에 저장
	Generals::setProps("temperatureUnit", Generals::temperatureUnit);
}
#pragma endregion




#pragma region events
void rodinP::dragEnterEvent(QDragEnterEvent *e)
{
	if (!ui.menuBar->isEnabled())
		return;
	if (m_fileDialogOpen)
		return;
	if (e->mimeData()->hasUrls())
	{
		e->acceptProposedAction();
	}
}

void rodinP::dropEvent(QDropEvent *e)
{
	QList<QUrl> url = e->mimeData()->urls();

	if (url.empty())
		return;

	QStringList fileList;
	for (auto file : url)
		fileList.push_back(file.toLocalFile());
	loadFiles(fileList);

	e->acceptProposedAction();
}

void rodinP::mousePressEvent(QMouseEvent* e)
{
	/*if (ui.viewer->isFocusInTriggered())
	{
		QPoint global = mapToGlobal(e->pos());
		QPoint viewerPt = ui.viewer->mapFromGlobal(global);
		ui.viewer->updateViewerState(viewerPt);
		ui.viewer->setFocusInTrigger(false);
		ui.viewer->updateGL();
	}*///to do sj

}

void rodinP::closeEvent(QCloseEvent * event)
{
	event->ignore();

	CommonDialog comDlg(this, MessageQuestion::tr("exit_confirm"), CommonDialog::Question, false, false, true, true);//종료하시겠습니까?
	if (comDlg.isYes())
	{
		closeControlDlg();
		event->accept();
	}
}

void rodinP::resizeEvent(QResizeEvent *e)
{
	//movePopupDialog();
	closeControlDlg();
}

void rodinP::moveEvent(QMoveEvent *e)
{
	//movePopupDialog();
	closeControlDlg();
}

void rodinP::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Escape)
		closeControlDlg();

	if (e->modifiers() == Qt::NoModifier)
	{
		if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)
		{
			if (ui.slider_layer->isVisible() || ui.slider_supportEdit->isVisible())//스크롤을 움직이는 경우 camera 이동 제한함.
			{
				int move;
				if (e->key() == Qt::Key_Up) move = 1;
				else if (e->key() == Qt::Key_Down) move = -1;
				else return;

				if (ui.slider_layer->isVisible())
					ui.slider_layer->setValue(ui.slider_layer->value() + move);
				if (ui.slider_supportEdit->isVisible())
					ui.slider_supportEdit->setValue(ui.slider_supportEdit->value() + move);
				return;
			}
		}
	}

	ui.viewerModule->keyPressEvent(e);
}

#pragma endregion




#pragma region load or save
void rodinP::loadFiles()
{
	char szFilter[] = "All supported formats (*.stl *3mf *.ply *.obj *.gcode);;Stereolithography (*.stl);;3D Manufacturing Format (*.3mf);;Stanford Polygon Library (*.ply);;Object (*.obj);;G-code (*.gcode)";
	QStringList tempStrs = QFileDialog::getOpenFileNames(this, CustomTranslate::tr("Load 3D model"), "", szFilter);

	if (tempStrs.empty()) return;
	loadFiles(tempStrs);
}
void rodinP::loadFiles(QStringList fileList_)
{
	bool hasModelFile = false;
	bool hasGcodeFile = false;
	QStringList modelFileList;
	QString gcodeFile;
	for (auto file : fileList_)
	{
		QFileInfo fileInfo = QFileInfo(file);
		QString suffix = fileInfo.suffix();
		if (suffix.compare("stl", Qt::CaseInsensitive) == 0 ||
			suffix.compare("obj", Qt::CaseInsensitive) == 0 ||
			suffix.compare("ply", Qt::CaseInsensitive) == 0 ||
			suffix.compare("3mf", Qt::CaseInsensitive) == 0)
		{
			modelFileList.push_back(fileInfo.absoluteFilePath());
			hasModelFile = true;
		}
		else if (suffix.compare("gcode", Qt::CaseInsensitive) == 0)
		{
			gcodeFile = fileInfo.absoluteFilePath();
			hasGcodeFile = true;
		}
	}
	
	//1. 모델파일과 gcode이 섞여서 로드된 경우는 모델파일만 오픈.
	//2. 여러개의 gcode파일이 로드된 경우는 마지막에 로드된 gcode 파일만 오픈.
	if (hasModelFile)
		loadModelFile(modelFileList);
	else if (hasGcodeFile)
		loadPathfromGCode(gcodeFile);
	ui.viewerModule->updateGL();
}
void rodinP::loadModelFile()
{
	char szFilter[] = "All supported formats (*.stl *3mf *.ply *.obj);;Stereolithography (*.stl);;3D Manufacturing Format (*.3mf);;Stanford Polygon Library (*.ply);;Object (*.obj)";
	QStringList tempStrs = QFileDialog::getOpenFileNames(this, CustomTranslate::tr("Load 3D model"), "", szFilter);

	if (tempStrs.empty()) return;
	loadModelFile(tempStrs);
}

void rodinP::loadModelFile(QStringList fileList_)
{
	ui.viewerModule->loadModels(fileList_);
	modelContainer->printingInfo->clear();
	UserProperties::thinMessageCheck = false;
	setNormalViewerMode();
}

void rodinP::loadModelFile(QString fileName)
{
	loadModelFile(QStringList(fileName));
}

void rodinP::loadPathfromGCode()
{
	char szFilter[] = "All supported formats (*.gcode);;G-code (*.gcode)";
	QString gcodePath = QFileDialog::getOpenFileName(this, CustomTranslate::tr("Open G-code"), "", szFilter);

	if (gcodePath.isEmpty()) return;

	loadPathfromGCode(gcodePath);
}

void rodinP::loadPathfromGCode(QString fileName)
{
	closeControlDlg();
	ui.label_printString->setText("");
	ui.viewerModule->deleteAllModels();

	GCodeParser gcodeParser(modelContainer->printingInfo);
	ProgressHandler progressHandler(this);
	progressHandler.setLabelText(MessageProgress::tr("Loading G-code.."));
	gcodeParser.setProgressHandler(&progressHandler);

	if (gcodeParser.loadPathfromGCode(fileName))
	{
		ModelDataStorage* dataStorage = gcodeParser.getDataStorage();
		ui.viewerModule->setPreviewUIMode(dataStorage);

		ui.actionInsert_image_toGcode->setEnabled(true);
		if (modelContainer->printingInfo->hasEncodedImage())
		{
			showPreviewThumbnailDialog();
			ui.actionDelete_image_inGcode->setEnabled(true);
		}
	}
	else
		modelContainer->printingInfo->clear();
}

void rodinP::saveGCode()
{
	//printing area validation//
	if (!checkModels(true))
		return;

	if (Generals::getProps("emailAddress") != "" && Profile::machineProfile.has_web_camera.value == true)
	{
		QString tempMessage;
		tempMessage.append("- " + MessageInfo::tr("email_will_be_sent") + "\n");
		tempMessage.append("  " + MessageQuestion::tr("would_you_like_to_continue"));
		CommonDialog comDlg(this, tempMessage, CommonDialog::Question, false, false, true, true);

		if (!comDlg.isYes())
			return;
	}

	QFileInfo strPathInfo = modelContainer->models.back()->getFileInfo();

	char szFilter[] = "gcode type(*.gcode)";
	QString gcodeFileName = QFileDialog::getSaveFileName(this, CustomTranslate::tr("Save G-code"), strPathInfo.absolutePath() + "/" + strPathInfo.completeBaseName(), szFilter);
	if (gcodeFileName.size() == 0)
		return;

	Logger() << "SaveGCode : " << gcodeFileName;

	//authentification print mode 대응//
	if (UserProperties::authentification_print_mode
		&& UserProperties::authentification_print_setting_method == Generals::AuthentificationMethod::Everytime)
	{
		AuthentificationDlg *authentificationDlg = new AuthentificationDlg(Generals::AuthentificationMethod::Everytime, this);
		if (authentificationDlg->exec() != QDialog::Accepted)
			return;
	}

	bool isNeedOverwrite = false;
	if (!Generals::isPreviewUIMode())
	{
		closeControlDlg();
		if (!slice(false)) //slicing
			return;
	}
	else//slicing 되어 있는 상태이면 authentification Code 만 다시 씀.
	{
		if (UserProperties::authentification_print_mode)
			isNeedOverwrite = true;
	}

	QString gcodeTempFileName = modelContainer->printingInfo->getGcodeTempFileName();
	FileCopier fileCopier;
	ProgressHandler progressHandler(this);
	progressHandler.setLabelText(MessageProgress::tr("Saving G-code.."));
	fileCopier.setProgressHandler(&progressHandler);
	if (isNeedOverwrite)
	{
		if (!fileCopier.overwriteAuthentificationCode(gcodeTempFileName, gcodeFileName))
			return;
	}
	else
	{
		if (!fileCopier.copy(gcodeTempFileName, gcodeFileName))
			return;
	}

	CommonDialog comDlg(this, MessageInfo::tr("success_save_gcode"), CommonDialog::Information);
}

void rodinP::saveGCodeToPrinter()
{
	closeControlDlg();
	if (PrinterInfo::currentConnectionType.isEmpty())
	{
		//선택된 Printer가 없음.
		//사용할 프린터를 선택해 주세요.
		CommonDialog comDlg(this, MessageAlert::tr("please_select_printer"), CommonDialog::Warning);
		showMyPrinters();
		return;
	}

	int ssdRemain; //7X일 때 SSD 남은 용량 체크
	//SSD 용량 체크 : MB로 여유 용량 회신 GCODE가 더 작으면됨
	if (PrinterInfo::currentConnectionType == "Network")
	{
		if (!NetworkConnection::checkNetwork())
			return;
		//해당 ip로 접속 안됨
		if (!NetworkConnection::checkConnection(this, PrinterInfo::currentIP, 7000))
			return;
		//SSD 용량 체크
		NetworkControl netCheck;
		ssdRemain = netCheck.getSSDRemain(PrinterInfo::currentIP);
	}
	else if (PrinterInfo::currentConnectionType == "USB")
	{
		PJLControl pjlCheck;
		PrinterInterface selectedPrinter = pjlCheck.getSelectPrinter(PrinterInfo::currentIP, false);
		//선택된 Printer를 찾을 수 없음.
		if (!PJLControl::checkConnection(this, selectedPrinter))
			return;
		//SSD 용량 체크
		ssdRemain = pjlCheck.getSSDRemain(selectedPrinter);
	}
	if (ssdRemain == 0)
	{
		//기기에 남은 용량이 없습니다.
		CommonDialog comDlg(this, MessageAlert::tr("insufficient_SSD_storage_space"), CommonDialog::Warning);
		return;
	}

	if (!checkBeforePrint())
		return;
	if (!checkMaterialBeforeSlice())
		return;
	if (!showAuthentificationPrint())
		return;

	if (!Generals::isPreviewUIMode())
	{
		ProfileToConfig::convertToConfig();
		SaveFileUIMode::saveThumbnail(modelContainer->models).save(Generals::getTempFolderPath() + "\\thumbnail_image.png");
	}

	PrintControl * printControl = new PrintControl(ui.viewerModule, true);
	printControl->start();
}

void rodinP::saveModelFile()
{
	if (!checkModels(true))
		return;

	QFileInfo strPathInfo = modelContainer->models.back()->getFileInfo();
	/*QString strFullNamePath(model.getFullFileName());

	QString strPathWithoutFilename = strFullNamePath.section(strPathInfo.completeBaseName(), 0, 0);
	QString strChangedFullNamePath = strPathWithoutFilename + strPathInfo.completeBaseName();*/

	m_fileDialogOpen = true;
	char szFilter[] = "Stereolithography (*.stl) ;;3D Manufacturing Format (*.3mf)";
	QString modelFileName = QFileDialog::getSaveFileName(this, CustomTranslate::tr("Save 3D model as"), strPathInfo.absolutePath() + "/" + strPathInfo.completeBaseName(), szFilter);
	m_fileDialogOpen = false;

	if (modelFileName.size() == 0) return;

	Logger() << "Save_Model : " << modelFileName;
	if (ui.viewerModule->saveModels(modelFileName))
		CommonDialog comDlg(this, MessageInfo::tr("success_save_model"), CommonDialog::Information);
	else
		CommonDialog comDlg(this, MessageAlert::tr("fail_to_save_model"), CommonDialog::Warning);
}

void rodinP::insertImageToGCode()
{
	PrintingInfo* printingInfo = modelContainer->printingInfo;
	//loaded gcode export or loaded gcode print//
	if (!printingInfo->getIsLoadedGcode())
		return;

	if (printingInfo->hasEncodedImage())
	{
		//여기서 교체할지, 취소할 지 물어볼 것..//
		CommonDialog comDlg(this, MessageQuestion::tr("gcode_already_has_image"), CommonDialog::Question, false, false, true, true);
		if (!comDlg.isYes())
			return;
	}
	//image loading or 3d model loading..//
	char szFilter[] = "All supported formats (*.stl *.3mf *.ply *.obj *.bmp *.jpeg *.jpg *.png);;Stereolithography (*.stl);;3D Manufacturing Format (*.3mf);;Stanford Polygon Library (*.ply);;Object (*.obj);;Bitmap (*.bmp);;PNG (*.png);;JPEG (*.jpeg *.jpg)";

	QString sourcePath = QFileDialog::getOpenFileName(this, CustomTranslate::tr("Open for adding to gcode"), "", szFilter);
	if (sourcePath.isEmpty())
		return;

	QFileInfo sourcePathInfo(sourcePath);
	QString imagePath;
	QString suffix = sourcePathInfo.suffix();
	if (suffix.compare("stl", Qt::CaseInsensitive) == 0 ||
		suffix.compare("obj", Qt::CaseInsensitive) == 0 ||
		suffix.compare("ply", Qt::CaseInsensitive) == 0 ||
		suffix.compare("3mf", Qt::CaseInsensitive) == 0)
	{
		ui.viewerModule->loadModel(sourcePath);
		imagePath = Generals::getTempFolderPath() + "\\thumbnail_image.png";
		SaveFileUIMode::saveThumbnail(modelContainer->models).save(imagePath);
		modelContainer->deleteAllModels();
	}
	else
	{
		QImage sourceImage(sourcePath);
		QImage tempImage_processed;
		imagePath = Generals::getTempFolderPath() + "\\thumbnail_image_processed.jpeg";
		tempImage_processed = sourceImage.scaled(Profile::machineProfile.thumbnail_image_width.value, Profile::machineProfile.thumbnail_image_height.value, Qt::KeepAspectRatio);
		tempImage_processed.save(imagePath, nullptr, 100); //image quality를 일단 그대로..//

		//////////////////////////////////////////
		//thumbnail image regenerating..//
		QFile imageFile(imagePath);
		//초기 jpeg quality.. 100보다 아래로 시작.//
		int initial_quality = 90;
		//image file size checking..//
		//image file 크기가 110kb이하로 해야 encoded text가 2000 line 아래로 내려감..// -> 펌웨어 제약. -_-;;
		if (imageFile.size() > 110000)
		{
			while (true)
			{
				tempImage_processed.save(imagePath, nullptr, initial_quality);
				qDebug() << imageFile.size();
				if (imageFile.size() < 110000)
					break;
				initial_quality -= 10;
			}
		}
		//////////////////////////////////////////
	}
	FileCopier fileCopier;
	ProgressHandler progressHandler(this);
	progressHandler.setLabelText(MessageProgress::tr("Inserting image to Gcode.."));
	fileCopier.setProgressHandler(&progressHandler);
	if (!fileCopier.insertImageToGCode(printingInfo, imagePath))
	{
		if (progressHandler.wasCanceled()) return;
		else progressHandler.close();
		CommonDialog comDlg(this, MessageError::tr("common_fail_message"), CommonDialog::Critical);
		Logger() << "Fail - Class(FileCopier) insertImageToGCode";
		return;
	}

	showPreviewThumbnailDialog();
	CommonDialog comDlg(this, MessageInfo::tr("gcode_regeneration_complete"), CommonDialog::Information);

	ui.actionDelete_image_inGcode->setEnabled(true);
}

void rodinP::deleteImageInGCode()
{
	PrintingInfo* printingInfo = modelContainer->printingInfo;
	if (!printingInfo->getIsLoadedGcode())
		return;

	if (!printingInfo->hasEncodedImage())
	{
		CommonDialog comDlg(this, MessageAlert::tr("no_image_in_gcode"), CommonDialog::Warning);
		return;
	}
	closeControlDlg();

	FileCopier fileCopier;
	ProgressHandler progressHandler(this);
	progressHandler.setLabelText(MessageProgress::tr("Deleting G-code Image.."));
	fileCopier.setProgressHandler(&progressHandler);
	if (!fileCopier.deleteImageInGCode(printingInfo))
	{
		if (progressHandler.wasCanceled()) return;
		else progressHandler.close();
		CommonDialog comDlg(this, MessageError::tr("common_fail_message"), CommonDialog::Critical);
		Logger() << "Fail - Class(FileCopier) deleteImageInGCode";
		return;
	}

	printingInfo->deleteEncodedImage();
	CommonDialog comDlg(this, MessageInfo::tr("gcode_image_deletion_complete"), CommonDialog::Information);

	ui.actionDelete_image_inGcode->setEnabled(false);
}

#pragma endregion




QString rodinP::getCurrentMode()
{
	QString tempMode;
	QString tempModeProp;

	if (m_settingMode == "easy")
	{
		tempMode = CustomTranslate::tr("Easy Mode");

		if (m_easyModeProp == "fast")
		{
			tempModeProp = CustomTranslate::tr("Fast Speed - Low Quality print");
		}
		else if (m_easyModeProp == "normal")
		{
			tempModeProp = CustomTranslate::tr("Normal Speed - Normal Quality print");
		}
		else if (m_easyModeProp == "silent")
		{
			tempModeProp = CustomTranslate::tr("Silent Speed - Normal Quality print");
		}
		else if (m_easyModeProp == "fine")
		{
			tempModeProp = CustomTranslate::tr("Slow Speed - High Quality print");
		}

		tempMode.append(" ");
		tempMode.append("(");
		tempMode.append(tempModeProp);
		tempMode.append(")");
	}
	else
	{
		tempMode = CustomTranslate::tr("Advanced Mode");
	}

	return tempMode;
}

bool rodinP::editSupport()
{
	if (!checkModels(false))
		return false;

	ProfileToConfig::convertToConfig();
	SliceProcessor p;
	p.init(modelContainer);
	if (!p.processingForSupportEdit())
		return false;

	AABB aabb = AABBGetter()(modelContainer->models);
	int max_v = MM2INT(aabb.getLengthZ()) / Profile::configSettings.front().layer_height;
	updateMaxLayerForSupport(max_v);
	ui.viewerModule->setSupportUIMode();

	return true;
}

bool rodinP::slice(bool withMessage_)
{
	if (!checkModels(false))
		return false;
	if (!checkMaterialBeforeSlice())
		return false;

	ProfileToConfig::convertToConfig();
	SaveFileUIMode::saveThumbnail(modelContainer->models).save(Generals::getTempFolderPath() + "\\thumbnail_image.png");
	//ui.viewerModule->updateGL();

	//modelContainer->GetDataStorage()->b_slicing_flag = true;
	SliceProcessor processor(this);
	processor.init(modelContainer);
	//p.setConfig(m_Config_multi);
	if (!processor.processing())
	{
		QString messageStr = processor.getErrorMessage();
		if (messageStr.isEmpty())
			return false;
		CommonDialog comDlg(this, messageStr, CommonDialog::Warning);
		return false;
	}

	ModelDataStorage* dataStorage = processor.getDataStorage();

	GCodeGenerator generator(this);
	generator.init(modelContainer, dataStorage);
	//g.setConfig(m_Config_multi);
	if (!generator.processing())
		return false;
	if (withMessage_)
	{
		QString messageStr;
		QString generatorMessage = generator.getWarningMessage();
		QString processorMessage = processor.getWarningMessage();
		if (!generatorMessage.isEmpty())
		{
			messageStr.append(generatorMessage);
		}
		if (!processorMessage.isEmpty())
		{
			if (!messageStr.isEmpty())
				messageStr.append("\n");
			messageStr.append(processorMessage);
		}
		if (!messageStr.isEmpty())
		{
			CommonDialog comDlg(this);
			comDlg.setCheckbox(true, MessageAlert::tr("show_me_again"), true);
			comDlg.setDialogContents(messageStr, CommonDialog::Warning);
			comDlg.exec();

			UserProperties::thinMessageCheck = comDlg.isCheck();
		}
	}

	ui.viewerModule->setPreviewUIMode(dataStorage);
	return true;
}

void rodinP::setPrintString()
{
	PrintingInfo *printingInfo = modelContainer->printingInfo;
	int totalPrintTime = printingInfo->getTotalPrintTime();
	QString printString = "";
	if (totalPrintTime != 0)
	{
		QString hr = QString::number(int(totalPrintTime) / 3600);
		QString min = QString::number(int(totalPrintTime) % 3600 / 60);
		QString sec = QString::number(int(totalPrintTime) % 60);

		printString = (hr == "0" ? "" : hr + CustomTranslate::tr("hr") + " ") + min + CustomTranslate::tr("min") + "  ";
	}

	////////////////////////////////////////////////////////////////////////////////////////
	std::vector<float> tempCartridgeFilamentAmount = printingInfo->getFilaAmount();
	std::vector<float> tempCartridgeFilamentMass = printingInfo->getFilaMass();
	int tempCartridgeTotalCount = printingInfo->getCartridgeCount();
	std::vector<bool> tempCartridgeUsedState = printingInfo->getUsedState();
	QString filamentString;
	for (int i = 0; i < tempCartridgeTotalCount; i++)
	{
		if (tempCartridgeUsedState.at(i) == true)
		{
			//cartridge를 셋팅상 설정하여도 실제 사용량이 0인 경우는 안보여주기로...//
			if (tempCartridgeFilamentAmount[i] != 0) 
			{
				if (!filamentString.isEmpty())
					filamentString.append("\n");

				char temcartFilamentAmountStr[50];
				sprintf(temcartFilamentAmountStr, "%.2f", tempCartridgeFilamentAmount[i] / 1000);

				char temcartFilamentMassStr[50];
				sprintf(temcartFilamentMassStr, "%.1f", tempCartridgeFilamentMass[i]);
				QString tempGram = QString(temcartFilamentMassStr);
				//printString = printString + tr("CARTRIDGE") + QString("_%1 : ").arg(i + 1) + QString(temcartFilamentAmountStr) + tr("meter");
				filamentString = filamentString + CustomTranslate::tr("Cartridge") + QString("(%1) : ").arg(i + 1) + QString(temcartFilamentAmountStr) + CustomTranslate::tr("meter");
				filamentString.append(" ");
				if (tempGram != "0.0") filamentString = filamentString + QString(temcartFilamentMassStr) + CustomTranslate::tr("gram");

				//replication일 경우에는 count가 1이므로 아래는 한번만 들어감..위에 카트리지 정보 복사..//
				if (Generals::isReplicationUIMode())
				{
					//printString = printString + tr("CARTRIDGE") + QString("_%1 : ").arg(i + 2) + QString(temcartFilamentAmountStr) + tr("meter");
					filamentString = filamentString + CustomTranslate::tr("Cartridge") + QString("(%1) : ").arg(i + 2) + QString(temcartFilamentAmountStr) + CustomTranslate::tr("meter");
					filamentString.append(" ");
					if (tempGram != "0.0") filamentString = filamentString + QString(temcartFilamentMassStr) + CustomTranslate::tr("gram");
				}
			}
		}
	}

	ui.label_printString->setText(printString + filamentString);
	qDebug() << printString;
	Logger() << "print info : " << printString;
	qDebug() << "slicing 19";
}

void rodinP::setSettingMode(QString selectedMode)
{
	if (selectedMode == "easy")
	{
		ui.actionStartEnd_Gcode->setEnabled(false);
		ui.actionReset_Profile->setEnabled(false);
		ui.actionImport_Profile->setEnabled(false);
		ui.actionExport_Profile->setEnabled(true);

		//m_ProfileSettingEasyDlg->UpdateSettingValueTosliceProfile_normal();

		ui.menuMode->actions().at(0)->setChecked(true);
	}
	else if (selectedMode == "advanced")
	{
		ui.actionStartEnd_Gcode->setEnabled(true);
		ui.actionReset_Profile->setEnabled(true);
		ui.actionImport_Profile->setEnabled(true);
		ui.actionExport_Profile->setEnabled(true);

		ui.menuMode->actions().at(1)->setChecked(true);
	}

	//props에 저장
	Generals::setProps("mode", selectedMode);

	ui.label_printingMode->setText(getCurrentMode());
}

void rodinP::setTemperatureUnit()
{
	QActionGroup* menuTemperatureGroup = new QActionGroup(ui.menuTemperature);
	menuTemperatureGroup->setExclusive(true);

	connect(menuTemperatureGroup, SIGNAL(triggered(QAction *)), this, SLOT(menuTemperatureGroup_triggered(QAction *)));
	menuTemperatureGroup->addAction(ui.actionCelsius);
	menuTemperatureGroup->addAction(ui.actionFahrenheit);
	if (Generals::temperatureUnit == "C")
	{
		ui.menuTemperature->actions().at(0)->setChecked(true);
	}
	else if (Generals::temperatureUnit == "F")
	{
		ui.menuTemperature->actions().at(1)->setChecked(true);
	}

	//props에 저장
	Generals::setProps("temperatureUnit", Generals::temperatureUnit);
}

void rodinP::getPrinterInfo()
{
	QString model = PrinterInfo::currentModel;
	QString printerName = PrinterInfo::currentPrinterName;

	//m_ProfileMachineDlg->setModelEnabled(false);
	//m_ProfileMachineDlg->changeModel(model);
	/*
	##Printer 찾을때 사용##
	제조사			Public	SINDOH				1.3.6.1.2.1.1.1.0
	Category		Public	3D Machine			1.3.6.1.2.1.25.3.2.1.3.1
	기종(TLI)		Public	RODIN-P(추후 변경)	1.3.6.1.2.1.43.5.1.1.16.1
	프린터 이름		Public	HYUNGJUN'S 3DP		1.3.6.1.2.1.1.5.0

	##Slicing 할때 사용-N인 경우 Print버튼 Diable##
	프린트 가능 여부	Public	Y/N					1.3.6.1.2.1.25.3.5.1.1.1

	##Application 시작할때, 사용 프린터 지정했을때 사용##
	필라멘트 색상	Private	123,123,123(RGB)	1.3.6.1.4.1.27278.3.1.0
	필라멘트 총 길이	Private	300(Meter)			1.3.6.1.4.1.27278.3.2.0
	필라멘트 사용길이	Private	120(Meter)			1.3.6.1.4.1.27278.3.3.0
	필라멘트 종류	Private	PLA/ABS				1.3.6.1.4.1.27278.3.4.0
	*/

	//QString filaColor;	//필라멘트 색상
	//int filaTotalLen;			//필라멘트 총 길이
	//int filaUsedLen;			//필라멘트 사용길이
	//QString filaType;		//필라멘트 종류
	//int filaUsedPer; //필라멘트 잔량 % = (filaTotalLen - filaUsedLen) / filaTotalLen * 100;
	//int filaUsed;// 10개중 몇개에 색을 표현할지

	//프린터 변경 후 프린터 정보 찾아오도록 순서 수정 적용.
	if (model != "")
	{
		if (Profile::machineProfile.machine_model != model)
		{
			changeMachine(model);
		}
	}

	ui.textBrowser_printerName->setText(printerName);
	deleteFilaInfo();
	addFilaInfo(0, 0, "", "Finding..");

	FindPrinterInfo *findPrinterTrd = new FindPrinterInfo();
	connect(findPrinterTrd, SIGNAL(finished()), findPrinterTrd, SLOT(deleteLater()));
	//connect(findPrinterTrd, SIGNAL(finished()), this, SLOT(setVolumeColor()));  //to do sj
	connect(findPrinterTrd, SIGNAL(signal_filamentInfoUpdated()), this, SLOT(afterFilamentInfoUpdated()));
	/*if (m_SelectCartDlg != nullptr)
		connect(findPrinterTrd, SIGNAL(finished()), m_SelectCartDlg, SLOT(volumeSelected()));*/
	//to do sj

	findPrinterTrd->start();

}

void rodinP::refreshFilaInfo()
{
	int cartridgeCnt = CartridgeInfo::cartridges.size();
	if (cartridgeCnt == 0) return;
	deleteFilaInfo();
	for (int i = 0; i < cartridgeCnt; i++)
	{
		addFilaInfo(i, CartridgeInfo::getCartridgeRemainBar(i), CartridgeInfo::getCartColorString(i), CartridgeInfo::cartridges.at(i).material);
	}
	ui.viewerModule->setModelColor();
	emit signal_filaColorChanged();
}

void rodinP::deleteFilaInfo()
{
	qDeleteAll(ui.frame_filaInfo->children());
	//frame_filaInfo 하위 object를 삭제할때 vertical layout이 같이 삭제되므로 추가해줌.
	QVBoxLayout *vertical_filaInfo;
	vertical_filaInfo = new QVBoxLayout(ui.frame_filaInfo);
	vertical_filaInfo->setSpacing(2);
	vertical_filaInfo->setContentsMargins(11, 11, 11, 11);
	vertical_filaInfo->setObjectName(QStringLiteral("vertical_filaInfo"));
	vertical_filaInfo->setContentsMargins(0, 0, 0, 0);
}

void rodinP::addFilaInfo(int idx, int filaRemainCnt, QString rgb, QString material)
{
	QVBoxLayout *vertical_filaInfo = findChild<QVBoxLayout *>("vertical_filaInfo");
	QFrame *filaRemains;
	filaRemains = new QFrame(ui.frame_filaInfo);
	filaRemains->setObjectName("filaRemains_" + QString::number(idx));
	filaRemains->setMinimumSize(QSize(0, 20));
	filaRemains->setFrameShape(QFrame::NoFrame);
	filaRemains->setFrameShadow(QFrame::Plain);
	filaRemains->setLineWidth(0);

	QHBoxLayout *horizontal_filaRemains;
	horizontal_filaRemains = new QHBoxLayout(filaRemains);
	horizontal_filaRemains->setSpacing(0);
	horizontal_filaRemains->setObjectName("horizontal_filaRemains_" + QString::number(idx));
	horizontal_filaRemains->setContentsMargins(0, 0, 0, 0);

	if (material != "No Info." && material != "Finding..")
	{
		if (Profile::machineProfile.openMode)
		{
			QSpacerItem *horizontalSpacer_6;
			horizontalSpacer_6 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
			horizontal_filaRemains->addItem(horizontalSpacer_6);

			QLabel *label_openMaterialImg;
			label_openMaterialImg = new QLabel(filaRemains); //
			label_openMaterialImg->setObjectName(QStringLiteral("label_openMaterialImg"));
			QSizePolicy sizePolicy5(QSizePolicy::Maximum, QSizePolicy::Maximum);
			sizePolicy5.setHorizontalStretch(0);
			sizePolicy5.setVerticalStretch(0);
			sizePolicy5.setHeightForWidth(label_openMaterialImg->sizePolicy().hasHeightForWidth());
			label_openMaterialImg->setSizePolicy(sizePolicy5);
			//label_openMaterialImg->setMinimumSize(QSize(20, 20));
			label_openMaterialImg->setMaximumSize(QSize(25, 25));
			label_openMaterialImg->setAutoFillBackground(false);
			label_openMaterialImg->setStyleSheet(QStringLiteral("background-color: rgba(0,0,0,0%)"));
			label_openMaterialImg->setPixmap(QPixmap(QString::fromUtf8(":/rodinP/Resources/open_material.png")));
			label_openMaterialImg->setScaledContents(true);

			horizontal_filaRemains->addWidget(label_openMaterialImg);

		}
		else
		{
			QFrame *filaRemainsSub;
			filaRemainsSub = new QFrame(filaRemains);
			filaRemainsSub->setObjectName("filaRemainsSub_" + QString::number(idx));
			filaRemainsSub->setMinimumSize(QSize(0, 20));
			filaRemainsSub->setFrameShape(QFrame::NoFrame);
			filaRemainsSub->setFrameShadow(QFrame::Plain);
			filaRemainsSub->setLineWidth(0);
			filaRemainsSub->setStyleSheet("background-color: rgb(36, 78, 120);");

			QHBoxLayout *horizontal_filaRemainsSub;
			horizontal_filaRemainsSub = new QHBoxLayout(filaRemainsSub);
			horizontal_filaRemainsSub->setSpacing(1);
			horizontal_filaRemainsSub->setObjectName("horizontal_filaRemainsSub_" + QString::number(idx));
			horizontal_filaRemainsSub->setContentsMargins(1, 1, 1, 1);

			horizontal_filaRemains->addWidget(filaRemainsSub);

			for (int i = 0; i < 10; i++)
			{
				QLabel *fila;
				fila = new QLabel(filaRemainsSub);
				fila->setObjectName("fila_" + QString::number(idx) + QString::number(i));
				fila->setMaximumSize(QSize(4, 16777215));

				if (i < filaRemainCnt)
					fila->setStyleSheet("background-color: rgb(" + rgb + ");");/* border-style: solid; border-color: rgb(70,70,70); border-width: 1px; */
				else
				{
					fila->setStyleSheet("background-color: rgb(36, 78, 120);");
				}

				horizontal_filaRemainsSub->addWidget(fila);

			}
		}
	}

	QLabel *filaInfo;
	filaInfo = new QLabel(filaRemains);
	filaInfo->setObjectName(QStringLiteral("filaInfo"));
	QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy3.setHeightForWidth(filaInfo->sizePolicy().hasHeightForWidth());
	filaInfo->setSizePolicy(sizePolicy3);
	filaInfo->setMinimumSize(QSize(0, 0));
	QFont font1;
	font1.setFamily(QStringLiteral("Malgun Gothic"));
	font1.setPointSize(8);
	font1.setBold(true);
	font1.setWeight(75);
	filaInfo->setFont(font1);
	filaInfo->setStyleSheet(QLatin1String("background-color: none;\n color: rgb(255, 255, 255);"));
	filaInfo->setText(material);
	filaInfo->setAlignment(Qt::AlignCenter);
	filaInfo->setWordWrap(true);

	horizontal_filaRemains->addWidget(filaInfo);

	vertical_filaInfo->addWidget(filaRemains);
}

void rodinP::setBrushRadius(int r_int)
{
	ui.viewerModule->modelContainer->supportData->setRadius((r_int + 1) * 4);//to do sj
}

void rodinP::checkOpenGLversion()
{
	QString openGLver((const char*)glGetString(GL_VERSION));
	CommonDialog comDlg(this, openGLver, CommonDialog::Information);
	//QMessageBox::information(this, tr("3Dwox"), openGLver, QMessageBox::Ok);
}

void rodinP::viewBrowser()
{
	if (PrinterInfo::currentIP == NULL)
	{
		CommonDialog comDlg(this, MessageAlert::tr("please_select_printer"), CommonDialog::Warning);//사용할 프린터를 먼저 선택해 주세요.
		//QMessageBox::warning(this, tr("3Dwox"), tr("Please select the printer you want to use."));//사용할 프린터를 먼저 선택해 주세요.
		showMyPrinters();
		return;
	}
	if (PrinterInfo::currentConnectionType != "Network")
	{
		CommonDialog comDlg(this, MessageAlert::tr("webmonitoring_feature_not_support"), CommonDialog::Warning);//웹모니터링 기능은 네트워크 프린터만 지원합니다.
		//QMessageBox::warning(this, tr("3Dwox"), tr("Web monitoring feature is only supported on network printer."));//웹모니터링 기능은 네트워크 프린터만 지원합니다.
		return;
	}
	if (!NetworkConnection::checkNetwork(this))
		return;
	if (!NetworkConnection::checkConnection(this, PrinterInfo::currentIP, 7000))
		return;

	QDesktopServices::openUrl(QUrl("http://" + PrinterInfo::currentIP));
}
bool rodinP::checkBeforePrint()
{
	PrintingInfo* printingInfo = modelContainer->printingInfo;
	if (printingInfo->getIsLoadedGcode())
	{
		if (printingInfo->getMachineModel() != Profile::machineProfile.group_model)
		{
			if ((printingInfo->getMachineModel() == "NONE" && Profile::machineProfile.group_model == "DP201")
				|| printingInfo->getMachineModel() != "NONE")
			{
				QString message = MessageAlert::tr("gcode_cannot_be_printed").arg(Profile::machineProfile.machine_model);
				CommonDialog comDlg(this, message, CommonDialog::Warning);
				return false;
			}
		}
	}
	else
	{
		if (!checkModels(true))
			return false;
	}

	if (PrinterInfo::currentConnectionType.isEmpty())
	{
		//선택된 Printer가 없음.
		//사용할 프린터를 선택해 주세요.
		CommonDialog comDlg(this, MessageAlert::tr("please_select_printer"), CommonDialog::Warning);
		showMyPrinters();
		return false;
	}

	if (PrinterInfo::currentModel == "")
	{
		//Homepage에서 Profile을 다운받거나 최신 App으로 Update하라는 메세지 필요.
	}
	//선택된 Printer Model과 Machine Setting에 설정된 Model이 다른 경우 //이 경우는 더 이상 발생하지 않을듯. //삭제 예정 //to do sj
	else if (PrinterInfo::currentModel != Profile::machineProfile.machine_model)
	{
		//선택된 Printer의 모델과 Machine Setting의 모델이 다릅니다.
		//CommonDialog comDlg(this, tr("different_model"), CommonDialog::Question, false, false, true, true);
		CommonDialog comDlg(this, MessageAlert::tr("different_model"), CommonDialog::Warning);
		return false;
	}

	return true;
}
bool rodinP::checkMaterialBeforePrint()
{
	//카트리지 정보 초기화
	CartridgeInfo::findCartridgeData();
	CartridgeInfo::setUseState(modelContainer);

	std::vector<int> usedCartIdx;
	std::vector<QString> materials;
	if (Generals::isPreviewUIMode() && modelContainer->printingInfo->getSlicerType() == 0)
	{
		//Gcode생성 당시 사용된 재질/카트리지 확인
		usedCartIdx = modelContainer->printingInfo->getUsedCartridgeIndex();
		for (auto material : modelContainer->printingInfo->getFilaMaterial())
		{
			materials.push_back(Generals::getMaterialShortName(material));
		}
	}
	else
	{
		//셋팅된 재질 찾기
		for (auto profile : Profile::sliceProfile)
		{
			materials.push_back(Generals::getMaterialShortName(profile.filament_material.value));
		}
		//사용 예정 카트리지 확인하여 해당 카트리지만 비교
		usedCartIdx = CartridgeInfo::getUsedCartIdx();
	}

	if (Generals::isReplicationUIMode())
	{
		if (Profile::machineProfile.extruder_count.value > 1)
		{
			if (CartridgeInfo::cartridges.at(0).material != CartridgeInfo::cartridges.at(1).material)
			{
				CommonDialog comDlg(this, MessageAlert::tr("replication_print_not_available_different_material"), CommonDialog::Warning);
				return false;
			}
		}
	}
	
	//사용된 카트리지 정보를 기반으로 비교
	for (int i = 0; i < usedCartIdx.size(); i++)
	{
		/*if (materials.at(usedCartIdx.at(i)) == "ETC")
		{
		NULL;
		}
		else */
		if (CartridgeInfo::cartridges.at(usedCartIdx.at(i)).material != materials.at(usedCartIdx.at(i)))
		{
			QString message;
			if (CartridgeInfo::cartridges.at(usedCartIdx.at(i)).material == "No Info.")
				message = MessageAlert::tr("status_cartridge_not_detected");
			else if (modelContainer->printingInfo->getIsLoadedGcode())
				message = MessageAlert::tr("check_material_before_printing").arg(CartridgeInfo::cartridges.at(usedCartIdx.at(i)).material);
			else
				message = MessageAlert::tr("cartridge_material_different");

			CommonDialog comDlg(this, message, CommonDialog::Warning);
			return false;
		}
	}
	return true;
}
bool rodinP::checkMaterialBeforeSlice()
{
	if (Profile::machineProfile.machine_bed_selected_enabled.value)
	{
		//qDebug() << "재질 1 : " << Profile::sliceProfile.front().filament_material.value << "2 : " << Profile::sliceProfile.back().filament_material.value;
		//qDebug() << "배드 타입 1 : " << Profile::sliceProfile.front().bed_type.value << "2 : " << Profile::sliceProfile.back().bed_type.value;

		if ((Profile::sliceProfile.front().bed_type.value != 0 && Profile::sliceProfile.back().bed_type.value != 0) &&
			(Profile::sliceProfile.front().bed_type.value != Profile::sliceProfile.back().bed_type.value))
		{
			CommonDialog comDlg(this, MessageAlert::tr("different_bed_side"), CommonDialog::Warning);
			return false;
		}
	}

	//material combination check for multi-printing//
	if (!validateMaterialCombination())
	{
		CommonDialog comDlg(this, MessageAlert::tr("not_be_used_together").arg(Profile::sliceProfile.front().filament_material.value, Profile::sliceProfile.back().filament_material.value), CommonDialog::Warning);
		//if (isLayerViewerMode())
		//	setNormalViewerMode();
		return false;
	}
	return true;
}
bool rodinP::showAuthentificationPrint()
{
	//authentification print mode 대응//
	if (UserProperties::authentification_print_mode)
	{
		if (UserProperties::authentification_print_setting_method == Generals::AuthentificationMethod::Everytime)
		{
			AuthentificationDlg *authentificationDlg = new AuthentificationDlg(Generals::AuthentificationMethod::Everytime, this);
			if (authentificationDlg->exec() != QDialog::Accepted)
				return false;
		}
	}

	if (Generals::isPreviewUIMode())
	{
		FileCopier fileCopier;
		ProgressHandler progressHandler(this);
		progressHandler.setLabelText(MessageProgress::tr("Saving G-code.."));
		fileCopier.setProgressHandler(&progressHandler);
		if (UserProperties::authentification_print_mode)
		{
			PrintingInfo* printingInfo = modelContainer->printingInfo;
			printingInfo->setGcodeTempFileName();

			QString tempFileName = Generals::getTempFolderPath() + "/temp.gcode";
			QString targetFileName = printingInfo->getIsLoadedGcode() ? printingInfo->getGcodeFileName() : printingInfo->getGcodeTempFileName();
			//gcode 파일을 temp파일로 복사
			QFile tempFile(tempFileName);
			QFile targetFile(targetFileName);
			if (tempFile.exists()) tempFile.remove();
			if (!targetFile.copy(tempFileName))
				return false;

			if (!fileCopier.overwriteAuthentificationCode(tempFileName, targetFileName))
				return false;
		}
		/*else
		{
			if (!fileCopier.copy(gcodeTempFileName, gcodeFileName))
				return false;
		}*/
	}
	return true;
}
void rodinP::showPrintConfirm()
{
	PrintConfirm* prtConfirm = new PrintConfirm(ui.viewerModule);
	if (!prtConfirm->setContents())
		return;
	prtConfirm->show();
}
void rodinP::printModel()
{
	closeControlDlg();
	if (!checkBeforePrint())
		return;
	if (!PrinterInfo::checkMachine(this))
		return;
	if (!checkMaterialBeforePrint())
		return;
	if (!Generals::isPreviewUIMode())
	{
		if (!checkMaterialBeforeSlice())
			return;
	}
	if (!showAuthentificationPrint())
		return;
	showPrintConfirm();
}

void rodinP::createLangMenu(QString seletedLang)
{
	QActionGroup* menuLanguageGroup = new QActionGroup(ui.menuLanguage);
	menuLanguageGroup->setExclusive(true);

	connect(menuLanguageGroup, SIGNAL(triggered(QAction *)), this, SLOT(menuLanguageGroup_triggered(QAction *)));
	menuLanguageGroup->addAction(ui.actionKorean);
	menuLanguageGroup->addAction(ui.actionEnglish);
	menuLanguageGroup->addAction(ui.actionChinese);
	menuLanguageGroup->addAction(ui.actionFrench);
	menuLanguageGroup->addAction(ui.actionSpanish);
	menuLanguageGroup->addAction(ui.actionJapanese);
	menuLanguageGroup->addAction(ui.actionGerman);
	menuLanguageGroup->addAction(ui.actionDanish);

	for (int i = 0; i < menuLanguageGroup->actions().count(); i++)
	{
		if (menuLanguageGroup->actions().at(i)->property("lang").toString() == seletedLang)
			menuLanguageGroup->actions().at(i)->setChecked(true);
	}
}

void rodinP::setIconLang(QString lang)
{
	QString loadIcon = ":/rodinP/Resources/menu_load%1.png";
	QString settingIcon = ":/rodinP/Resources/menu_adjust%1.png";
	QString postFix = "";
	if (lang == "ko") postFix = "_" + lang;
	loadIcon = loadIcon.arg(postFix);
	settingIcon = settingIcon.arg(postFix);

	QIcon loadImg;
	QIcon settingImg;
	loadImg.addFile(loadIcon, QSize(), QIcon::Normal, QIcon::Off);
	settingImg.addFile(settingIcon, QSize(), QIcon::Normal, QIcon::Off);
	ui.pushButton_loadIcon->setIcon(loadImg);
	ui.pushButton_settingIcon->setIcon(settingImg);
}

void rodinP::chkVersion()
{
	checkVersion *chkVer = new checkVersion(this, AppInfo::getCurrentVersion(), true);
}

void rodinP::faqOpen()
{
	WebControl webCtrl;
	webCtrl.faqOpen(Profile::machineProfile.company_code.value);
}

void rodinP::setNormalViewerMode()
{
	closeControlDlg();
	if (ui.switch_coloredLayer->isOn())
	{
		if (!checkModels(true))
			ui.switch_coloredLayer->manualMouseRelease();
		else
			ui.viewerModule->setLayerColorUIMode();
	}
	else
		ui.viewerModule->setTranslateUIMode();
}

void rodinP::setEditViewerMode()
{
	if (Generals::isSupportUIMode())
		return;
	closeControlDlg();
	if (!editSupport())
	{
		setEnableMenuForViewer();
		return;
	}
}

bool rodinP::setLayerViewerMode()
{
	if (Generals::isPreviewUIMode())
		return true;
	closeControlDlg();
	if (!slice())
	{
		setEnableMenuForViewer();
		return false;
	}

	return true;
}

void rodinP::setReplicationPrintMode()
{
	bool isCheckedReplication = ui.actionReplication_Print->isChecked();

	Profile::machineProfile.replication_print.value = isCheckedReplication;
	ui.switch_coloredLayer->setVisible(!isCheckedReplication);
	ui.lab_coloredLayer->setVisible(!isCheckedReplication);
	
	//replication mode일 경우, support / raft cartirdge를 0으로 setting..//
	Profile::sliceProfileCommon.support_main_cartridge_index.value = 0;
	Profile::sliceProfileCommon.adhesion_cartridge_index.value = 0;

	if (isCheckedReplication && !modelContainer->printingInfo->getIsReplicationPrint()) ui.label_printString->setText(CustomTranslate::tr("Replication Print Mode"));
	else ui.label_printString->setText("");


	setSelectCartDlgVisible();

	/*if (model.volumes.size() > 0)
	{
		for (int i = 0; i < model.volumes.size(); i++)
		{
			ui.viewer->checkVolumeRange(&(model.getVolume(i)->vbo));
		}


		//cartridge가 1가지 이상으로 선택되었을 때, cartridge 1번으로 재 지정을 하고 replication으로 들어감..//
		if (Profile::getCartridgeTotalCount() > 1)
		{
			std::vector<int> cartridgeList;

			for (int i = 0; i < model.volumes.size(); i++)
			{
				list<Volume>::iterator v_iter = model.volumes.begin();
				std::advance(v_iter, i);
								
				cartridgeList.push_back(v_iter->meshInfo[0].mesh_cartridgeIndex);
			}

			//중복되는 cartridge들을 제거한 list 생성//
			cartridgeList.erase(std::unique(cartridgeList.begin(), cartridgeList.end()), cartridgeList.end());

			if (cartridgeList.size() > 1)
			{
				for (int i = 0; i < model.volumes.size(); i++)
				{
					list<Volume>::iterator v_iter = model.volumes.begin();
					std::advance(v_iter, i);
					
					//cartridge 0으로 setting..//
					v_iter->meshInfo[0].mesh_cartridgeIndex = 0;
				}
			}
		}
	}*///to do sj

	ui.viewerModule->updateGL();
}


bool rodinP::validateMaterialCombination()
{
	if (Profile::sliceProfile.size() < 2 || Generals::isReplicationUIMode())
		return true;
	
	//used cartridge방식 사용하지 않고, 무조건 combination 체크 하도록 수정.. 20190809//
	////사용된 카트리지가 1개면 체크할 필요 없음
	//std::vector<int> usedCartIdx = getUsedCartIdx(m_SliceProfile_common);
	//if (usedCartIdx.size() < 2) return true;

	int flag = Generals::checkMaterialCombination(Profile::sliceProfile.front().filament_material.value, Profile::sliceProfile.back().filament_material.value);
	if (flag == 0) return true;
	else if (flag == 1)
	{
		CartridgeInfo::setUseState(modelContainer);
		if (CartridgeInfo::getUsedCartIdx().size() > 1)
			CommonDialog comDlg(this, MessageAlert::tr("difficult_material_combination").arg(Profile::sliceProfile.front().filament_material.value, Profile::sliceProfile.back().filament_material.value), CommonDialog::Warning);
		return true;
	}
	else return false;
}

void rodinP::closeControlDlg()
{
	if (sidePopupDialog)
	{
		sidePopupDialog->close();
		sidePopupDialog = nullptr;
		ui.pushButton_selectCartridge->setChecked(false);
	}
	if (modelControlDialog)
	{
		QWidget* temp = modelControlDialog;
		modelControlDialog = nullptr;
		delete temp;
	}
}

void rodinP::changeMachine(QString machine_, bool machineChanged)
{
	if (!machineChanged)
	{
		if (!Profile::machineProfile.machine_model.isEmpty())
		{
			if (Profile::machineProfile.machine_model != machine_)
				ProfileControl::saveRecentProfile(m_settingMode);
		}
	}
	else
		ProfileControl::saveRecentProfile(m_settingMode);

	Profile::profilePath = Generals::appPath + "\\profile\\" + machine_ + "\\";
	Profile::recentProfilePath = Generals::appDataPath + "\\profile\\" + machine_ + "\\";
	Profile::customProfilePath = Generals::appDataPath + "\\customProfile\\" + machine_ + "\\";
	Generals::setProps("model", machine_);
	m_easyModeProp = "normal";
	Generals::setProps("EasyProp", m_easyModeProp);

	if (Profile::machineProfile.machine_model != machine_ || machineChanged)
		ProfileControl::loadMachineProfile();

	afterMachineChanged();
}

void rodinP::afterProfileChanged(bool sliceFlag)
{
	CartridgeInfo::setUseStateForProfile();
	ui.label_printingMode->setText(getCurrentMode());

	int supportCartridgeIdx = Profile::sliceProfileCommon.support_main_cartridge_index.value;
	ConfigSettings beforeConfig = Profile::configSettings.at(supportCartridgeIdx);
	
	//프로파일 변경을 config에 적용하는 동작을 여기에서만 하도록 수정. 이 외의 곳은 모두 삭제.
	ProfileControl::updateConfigForMultiCartridge();

	if (!modelContainer->hasAnyModel())
		return;
	emit signal_profileChanged();

	//변경된 프로파일 중에서서 서포트에 영향을 주는 부분이 있는지 확인.
	ConfigSettings currentConfig = Profile::configSettings.at(supportCartridgeIdx);
	if (ProfileControl::checkSupportProfileChanged(beforeConfig, currentConfig))
		modelContainer->supportData->b_changed = true;

	for (auto it : modelContainer->models)
		it->manipulated();
	if (Generals::isPreviewUIMode())
	{
		if (sliceFlag)
		{
			if (checkModels())
			{
				if (!slice())
					setNormalViewerMode();
			}
			else
				setNormalViewerMode();
		}
	}
	else if (Generals::isSupportUIMode())
		editSupport();
}

void rodinP::afterViewerChanged(Generals::ViewerMode viewerMode_)
{
	Generals::ViewerMode beforeViewerMode = Generals::currentViewerMode;
	Generals::currentViewerMode = viewerMode_;
	
	if (!modelContainer->hasAnyModel())
	{
		closeControlDlg();
		if (ui.switch_coloredLayer->isOn())
			ui.switch_coloredLayer->manualMouseRelease();
	}
	setEnableMenuForViewer();
	ui.viewerModule->afterViewerChanged();
}

void rodinP::updateMaxLayerForSupport(int maxLayer_)
{
	ui.slider_supportEdit->setMinimum(0);
	ui.slider_supportEdit->setMaximum(maxLayer_);
	ui.slider_supportEdit->setValue(maxLayer_ / 2);
}

void rodinP::updateMaxLayerForPreview(int maxLayer_)
{
	//ui.viewer->SetLayer(maxLayer);
	ui.slider_layer->setMinimum(1);
	ui.slider_layer->setMaximum(maxLayer_);
	ui.slider_layer->setValue(maxLayer_);
	ui.label_MaxLayerNo->setNum(maxLayer_);
}
void rodinP::setEnableMenuForViewer()
{
	ui.label_printString->setText("");
	setSelectCartDlgVisible();

	if (Generals::isTranslateUIMode() || Generals::isRotateUIMode())
	{
		ui.lab_coloredLayer->setText("<p align=""center""><span style=""color:#ffffff;"">" + CustomTranslate::tr("Layer Color") + "</span></p>");
		ui.pushButton_normalViewer->setChecked(true);
		ui.pushButton_editViewer->setChecked(false);
		ui.pushButton_layerViewer->setChecked(false);
		ui.layerFrame->setVisible(false);
		ui.editFrame->setVisible(false);
		ui.actionNormal_Viewer->setEnabled(false);
		ui.actionEdit_Support->setEnabled(true);
		ui.actionLayer_Viewer->setEnabled(true);
		ui.actionInsert_image_toGcode->setEnabled(false);
		ui.actionDelete_image_inGcode->setEnabled(false);

		if (Profile::machineProfile.machine_expanded_print_mode.value)
			ui.frm_switch->setVisible(false);
		else
			ui.frm_switch->setVisible(true);

		//ui.actionReplication_Print->setEnabled((1 < Profile::getCartridgeTotalCount()));
		setAllMenuEnabled(true);
		setControlMenuEnabled(true);
		setEnableMenuForModel();

		if (m_settingMode == "easy") ui.actionStartEnd_Gcode->setEnabled(false);
		else ui.actionStartEnd_Gcode->setEnabled(true);
	}
	else if (Generals::isPreviewUIMode())
	{
		ui.pushButton_normalViewer->setChecked(false);
		ui.pushButton_editViewer->setChecked(false);
		ui.pushButton_layerViewer->setChecked(true);
		ui.layerFrame->setVisible(true);
		ui.editFrame->setVisible(false);
		ui.actionNormal_Viewer->setEnabled(true);
		ui.actionEdit_Support->setEnabled(true);
		ui.actionLayer_Viewer->setEnabled(false);
		ui.actionStartEnd_Gcode->setEnabled(false);
		ui.actionInsert_image_toGcode->setEnabled(false);
		ui.actionDelete_image_inGcode->setEnabled(false);
		ui.frm_switch->setVisible(false);

		setPrintString();
		setAllMenuEnabled(true);
		setControlMenuEnabled(false);
		setEnableMenuForModel();
		if (modelContainer->printingInfo->getIsLoadedGcode())
		{
			/*if (!b_isReplicationPrintfromGcode)
			{
				ui.actionReplication_Print->setChecked(false);
				ui.actionReplication_Print->setEnabled(false);
			}*/
			ui.pushButton_settingIcon->setEnabled(false);
			ui.pushButton_normalViewer->setEnabled(false);
			ui.pushButton_editViewer->setEnabled(false);
			ui.pushButton_layerViewer->setEnabled(false);
			ui.menuMode->setEnabled(false);
			ui.menuAnalysis->setEnabled(false);

			ui.actionNormal_Viewer->setEnabled(false);
			ui.actionEdit_Support->setEnabled(false);
			ui.actionSave_GCode->setEnabled(false);
			ui.actionSave_GCode_printer->setEnabled(true);
			//ui.actionInsert_image_toGcode->setEnabled(true);
		}
	}
	else if (Generals::isAnalysisUIMode())
	{
		setAllMenuEnabled(false);
		setControlMenuEnabled(false);
	}
	else if (Generals::isLayerColorUIMode())
	{
		showSelectCartridgeDlg();
		ui.lab_coloredLayer->setText("<p align=""center""><span style=""color:#00f0d4;"">" + CustomTranslate::tr("Layer Color") + "</span></p>");
		//ui.btn_coloredLayer->manualMouseRelease();
		ui.pushButton_normalViewer->setChecked(true);
		ui.pushButton_editViewer->setChecked(false);
		ui.pushButton_layerViewer->setChecked(false);
		ui.layerFrame->setVisible(false);
		ui.editFrame->setVisible(false);
		ui.actionNormal_Viewer->setEnabled(false);
		ui.actionEdit_Support->setEnabled(true);
		ui.actionLayer_Viewer->setEnabled(true);
		ui.actionInsert_image_toGcode->setEnabled(false);
		ui.actionDelete_image_inGcode->setEnabled(false);
		ui.frm_switch->setVisible(true);

		setAllMenuEnabled(true);
		setControlMenuEnabled(false);

		if (m_settingMode == "easy") ui.actionStartEnd_Gcode->setEnabled(false);
		else ui.actionStartEnd_Gcode->setEnabled(true);
	}
	else if (Generals::isSupportUIMode())
	{
		ui.pushButton_normalViewer->setChecked(false);
		ui.pushButton_editViewer->setChecked(true);
		ui.pushButton_layerViewer->setChecked(false);
		ui.layerFrame->setVisible(false);
		ui.editFrame->setVisible(true);

		ui.actionNormal_Viewer->setEnabled(true);
		ui.actionEdit_Support->setEnabled(false);
		ui.actionLayer_Viewer->setEnabled(true);
		ui.label_printString->setText("");
		ui.actionInsert_image_toGcode->setEnabled(false);
		ui.actionDelete_image_inGcode->setEnabled(false);
		ui.frm_switch->setVisible(false);

		setAllMenuEnabled(true);
		setControlMenuEnabled(false);

		if (m_settingMode == "easy") ui.actionStartEnd_Gcode->setEnabled(false);
		else ui.actionStartEnd_Gcode->setEnabled(true);
	}
	else if (Generals::isReplicationUIMode())
	{
		ui.label_printString->setText(CustomTranslate::tr("Replication Print Mode"));
	}
}

void rodinP::setAllMenuEnabled(bool flag_)
{
	ui.menuBar->setEnabled(flag_);
	ui.menuMode->setEnabled(flag_);
	ui.pushButton_loadIcon->setEnabled(flag_);
	ui.pushButton_settingIcon->setEnabled(flag_);
	ui.pushButton_normalViewer->setEnabled(flag_);
	ui.pushButton_editViewer->setEnabled(flag_);
	ui.pushButton_layerViewer->setEnabled(flag_);
	ui.pushButton_print->setEnabled(flag_);
	ui.switch_coloredLayer->setEnabled(flag_);
}

void rodinP::setEnableMenuForModel()
{
	if (modelContainer->hasAnyModel())
	{
		ui.actionSave_model_file->setEnabled(true);
		ui.actionSave_GCode->setEnabled(true);
		ui.actionSave_GCode_printer->setEnabled(true);
		ui.actionPrint->setEnabled(true);
		if (Generals::isTranslateUIMode() || Generals::isRotateUIMode())
		{
			if (modelContainer->hasAnySelectedModel())
			{
				if (modelContainer->getSelectedModels().size() == 1)
					ui.menuAnalysis->setEnabled(true);
				else
					ui.menuAnalysis->setEnabled(false);
			}
			else
				ui.menuAnalysis->setEnabled(false);
		}
	}
	else
	{
		ui.actionSave_model_file->setEnabled(false);
		ui.actionSave_GCode->setEnabled(false);
		ui.actionSave_GCode_printer->setEnabled(false);
		ui.menuAnalysis->setEnabled(false);
		if (modelContainer->printingInfo->getIsLoadedGcode())
			ui.actionPrint->setEnabled(true);
		else
			ui.actionPrint->setEnabled(false);
	}
}

void rodinP::setControlMenuEnabled(bool flag_)
{
	ui.menuAnalysis->setEnabled(flag_);
	ui.pushButton_move->setEnabled(flag_);
	ui.pushButton_scale->setEnabled(flag_);
	ui.pushButton_rotate->setEnabled(flag_);
	ui.actionPosition->setEnabled(flag_);
	ui.actionScale->setEnabled(flag_);
	ui.actionRotation->setEnabled(flag_);
}

void rodinP::setSelectCartDlgVisible()
{
	if (Generals::isTranslateUIMode() || Generals::isRotateUIMode())
	{
		if (CartridgeInfo::cartridges.size() > 1)
			ui.pushButton_selectCartridge->setVisible(true);
		else
		{
			//closeControlDlg();
			ui.pushButton_selectCartridge->setVisible(false);
		}
	}
	else if (Generals::isLayerColorUIMode())
		ui.pushButton_selectCartridge->setVisible(true);
	else
	{
		//closeControlDlg();
		ui.pushButton_selectCartridge->setVisible(false);
	}
}

bool rodinP::checkModels(bool withMessage_)
{
	if (modelContainer->hasAnyModel())
	{
		modelContainer->checkModelRange();
		if (modelContainer->isDisabled())
		{
			//if (withMessage_) //무조건 메세지 나오도록함.
				CommonDialog comDlg(this, MessageAlert::tr("out_of_range"), CommonDialog::Warning);
			return false;
		}
	}
	else
	{
		if (withMessage_)
			CommonDialog comDlg(this, MessageAlert::tr("open_model_file"), CommonDialog::Warning);
		return false;
	}
	return true;
}

bool rodinP::checkSelectedModels(bool withMessage_)
{
	if (!checkModels(withMessage_))
		return false;
	if (!modelContainer->hasAnySelectedModel())
	{
		if (withMessage_)
			CommonDialog comDlg(this, MessageAlert::tr("no_selected_model"), CommonDialog::Warning);
		return false;
	}
	return true;
}

void rodinP::afterCurrentPrinterChanged()
{
	getPrinterInfo();
}
void rodinP::afterFilamentInfoUpdated()
{
	refreshFilaInfo();
}
void rodinP::afterMachineChanged()
{
	closeControlDlg();
	ProfileControl::loadProfileByMachine(m_settingMode);

	ui.textBrowser_printerName->setText(""); //일단 초기화함. 추후 프린터 정보로 업데이트.
	
	if (Profile::machineProfile.extruder_count.value > 1)
	{
		ui.actionReplication_Print->setEnabled(true);
		ui.actionReplication_Print->setChecked(false);
	}
	else
	{
		ui.actionReplication_Print->setEnabled(false);
		ui.actionReplication_Print->setChecked(false);
	}

	ui.actionSave_GCode_printer->setVisible(Profile::machineProfile.has_SSD_storage.value);
	//brower UI setting..//
	ui.actionWeb_Browser->setEnabled(Profile::machineProfile.has_web_camera.value);
	ui.viewerModule->resetBed();
	//Machine Profile이 변경될 때 extruder 갯수에 따라 cartridge의 갯수 변경 후 UI 초기화

	if (Profile::machineProfile.machine_expanded_print_mode.value)
		Profile::machineProfile.machine_expanded_print_mode.value = false;
	
	modelContainer->resetCartridge();
	refreshFilaInfo();

	afterProfileChanged();
	setSelectCartDlgVisible();
	afterExpandedPrintModeChanged();

	ui.viewerModule->isometricCamera();
}

void rodinP::afterExpandedPrintModeChanged()
{
	if (Profile::machineProfile.machine_expanded_print_mode.value)
	{
		modelContainer->setCartridgeIndexAll(Profile::machineProfile.machine_expanded_print_cartridgeIndex.value);

		//support, adhesion cartridge index를 모두 expanded cartridge index로 변환..//
		Profile::sliceProfileCommon.support_main_cartridge_index.value = Profile::machineProfile.machine_expanded_print_cartridgeIndex.value;
		Profile::sliceProfileCommon.adhesion_cartridge_index.value = Profile::machineProfile.machine_expanded_print_cartridgeIndex.value;

		//wipe tower disable//
		Profile::sliceProfileCommon.wipe_tower_enabled.value = false;

		closeControlDlg();

		ui.frm_switch->setVisible(false);
	}
	else
	{
		if(Generals::isTranslateUIMode() || Generals::isRotateUIMode())
			ui.frm_switch->setVisible(true);
	}

	//colored layer..//
	if (ui.switch_coloredLayer->isOn())
		ui.switch_coloredLayer->manualMouseRelease();

	ui.viewerModule->resetBed();
}


#pragma region dialog
void rodinP::viewProfileSettingMultiDlg()
{
	CartridgeInfo::setUseStateForModel(modelContainer);
	ProfileSettingMultiDlg *profileSettingMulti = new ProfileSettingMultiDlg(this);
	connect(profileSettingMulti, SIGNAL(signal_profileChanged(bool)), this, SLOT(afterProfileChanged(bool)));
	profileSettingMulti->show();
}

void rodinP::viewProfileSettingEasymodeDlg()
{
	ProfileSettingEasymodeDlg *profileSettingEasy = new ProfileSettingEasymodeDlg(this, m_easyModeProp);
	connect(profileSettingEasy, SIGNAL(signal_profileChanged(bool)), this, SLOT(afterProfileChanged(bool)));
	profileSettingEasy->show();
}

void rodinP::viewProfileSettingCustomProfileDlg()
{
	CustomProfileList *CusProfileList = new CustomProfileList(this);
	CusProfileList->show();
}

void rodinP::viewProfileSettingMachineDlg()
{
	ProfileSettingMachineDlg *profileMachine = new ProfileSettingMachineDlg(this);
	connect(profileMachine, SIGNAL(signal_profileChanged(bool)), this, SLOT(afterProfileChanged(bool)));
	connect(profileMachine, SIGNAL(signal_changeMachine(QString, bool)), this, SLOT(changeMachine(QString, bool)));
	connect(profileMachine, SIGNAL(signal_expandedPrintModeChanged()), this, SLOT(afterExpandedPrintModeChanged()));
	profileMachine->show();
}

void rodinP::viewStartEndGcodeDlg()
{
	EditStartEndcodeDlg editCodeDlg(this);
}

//support cartridge는 카트리지 기본을 0으로 선택..//
void rodinP::resetProfile()
{
	CommonDialog comDlg(this, MessageQuestion::tr("reset_profile_setting_confirm"), CommonDialog::Question, false, false, true, true);

	if (comDlg.isYes())
	{
		ProfileControl::resetProfile();
		afterProfileChanged();

		CommonDialog comDlg(this, MessageInfo::tr("profile_reset_complete"), CommonDialog::Information);
	}
	else return;
}

void rodinP::importProfile()
{
	SelectCartridgePopDialog *selectCartDlg = new SelectCartridgePopDialog(this);
	connect(selectCartDlg, SIGNAL(signal_profileChanged(bool)), this, SLOT(afterProfileChanged(bool)));
	selectCartDlg->showImportDialog();
}

void rodinP::exportProfile()
{
	SelectCartridgePopDialog *selectCartDlg = new SelectCartridgePopDialog(this);
	selectCartDlg->showExportDialog();
}

//hidden feature : only for developer//
void rodinP::replaceProfile()
{
	char szFilter[] = "Profile Information File (*.ini)";
	QStringList replace_profile_list = QFileDialog::getOpenFileNames(this->parentWidget(), tr("Replace profile to new profile"), "", szFilter);
	
	if (replace_profile_list.empty())
		return;

	SliceProfile slice_profile_temp;
	
	for (int i = 0; i < replace_profile_list.size(); ++i)
	{
		slice_profile_temp.loadSliceProfile(Generals::qstringTowchar_t(replace_profile_list.at(i)));

		Profile::sliceProfile.at(0) = slice_profile_temp;
		Profile::sliceProfileCommon.getCommonProfileFromSliceProfile(&Profile::sliceProfile.at(0));

		QFileInfo profile_path_info(replace_profile_list.at(i));
		QString replace_profile_path = replace_profile_list.at(i).section(profile_path_info.completeBaseName(), 0, 0); //파일 이름을 제외한 full path
		replace_profile_path.append("new_profile/");
		Generals::checkPath(replace_profile_path);
		

		QString file_name = replace_profile_list.at(i).section('/', -1);// .section('.', 0, -2); // file name extraction
		replace_profile_path.append(file_name);


		//export file//
		ProfileControl::exportProfile(replace_profile_path, 0);
	}

	CommonDialog comDlg(this, "complete replacing profile..", CommonDialog::Information);
}

void rodinP::excuteExperimentalFunction()
{
	//experimental functions..//

	char szFilter[] = "Profile Information File (*.ini)";
	QStringList profile_list = QFileDialog::getOpenFileNames(this->parentWidget(), tr("Experimental Function for Profile #1"), "", szFilter);

	if (profile_list.empty())
		return;

	for (int i = 0; i < profile_list.size(); ++i)
	{
		//ExperimentalFunctions::profileEdit_ByKey(profile_list.at(i), QString("inner_wall_width_factor"), QString("outer_wall_width_factor"), QString("wall_width_factor"));
		ExperimentalFunctions::profileEdit_insertBeforeKey(profile_list.at(i), QString("overall_flow"), QString("overall_flow_control_enabled(0=False,1=True): 1"));
	}

	//CommonDialog comDlg(this, "complete Experimental Function for Profile #1..", CommonDialog::Information);


	CommonDialog comDlg(this, "complete Experimental Function for Profile..\n(Y:again/N:end)", CommonDialog::Question, false, false, true, true);
	
	if (comDlg.isYes())
	{
		this->excuteExperimentalFunction();
	}
	else
	{
		return;
	}
}

void rodinP::showMyPrinters()
{
	MyPrinters *myPrt = new MyPrinters(this);
	//connect(myPrt, SIGNAL(sigSelectPrinter(QString, QString, QString, QString)), this, SLOT(getPrinterInfo(QString, QString, QString, QString)));
	myPrt->setCurrentIP(PrinterInfo::currentIP);
	//connect(myPrt, SIGNAL(signal_printerSelected()), this, SLOT(afterCurrentPrinterChanged()));
	myPrt->showMyPrinters();
}

void rodinP::showAddNetPrinter()
{
	AddMyPrinter *addPrt = new AddMyPrinter(this);
	addPrt->showAddNetPrinter();
}

void rodinP::showAddUSBPrinter()
{
	AddMyPrinter *addPrt = new AddMyPrinter(this);
	addPrt->showAddUSBPrinter();
}

void rodinP::showFindIPPrinter()
{
	AddIPAddress *addIP = new AddIPAddress(this);
	addIP->initForm();
}

void rodinP::getDefaultPrinter()
{
	QFile inputFile(Generals::appDataPath + "/myPrinters.ini");
	QList<QString> txtList;
	int columenCnt = 5;

	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			txtList.append(in.readLine());
		}
		int listCount = txtList.count();
		for (int j = 0; j < listCount / columenCnt; j++)
		{
			QString connectionType;
			QString printerName;
			QString ip;
			QString model;
			QString primary;

			connectionType = txtList.at(j * columenCnt);
			printerName = txtList.at(j * columenCnt + 1);
			model = txtList.at(j * columenCnt + 2);
			ip = txtList.at(j * columenCnt + 3);
			primary = txtList.at(j * columenCnt + 4);

			if (primary == "Y")
			{
				PrinterInfo::setPrinterInfo(connectionType, ip, printerName, model);
				getPrinterInfo();
				inputFile.close();		// modified Jusung
				return;
			}
		}
		inputFile.close();
	}
}

void rodinP::showMultiplyDialog()
{
	MultiplyDialog* dlg = new MultiplyDialog(ui.viewerModule);
	dlg->show();
}

void rodinP::showAnalysisDialog()
{
	closeControlDlg();

	if (modelContainer->models.empty())
	{
		CommonDialog comDlg(this, MessageAlert::tr("open_model_file_before_printing"), CommonDialog::Warning);
		std::cout << "no model!" << std::endl;
		return;
	}

	if (modelContainer->getSelectedModels().empty())
	{
		std::cout << "no model selected!" << std::endl;
		CommonDialog comDlg(this, MessageAlert::tr("no_selected_model"), CommonDialog::Warning);
		return;
	}

	//todo//이게 왜 없지??...// --> //VolumeAnalysisDialog 생성자에서 하네..//
	//PrintOptimum에서 하는 듯... 다시 검토..//

	VolumeAnalysisDialog *l_analysisDlg = new VolumeAnalysisDialog(this);
	connect(l_analysisDlg, SIGNAL(finished(int)), this, SLOT(finishedDialog()));
	l_analysisDlg->setParent(this);
	if (!l_analysisDlg->init(ui.viewerModule))
		return;

	sidePopupDialog = l_analysisDlg;
	movePopupDialog();
	l_analysisDlg->show();
}

void rodinP::showOptimizedDialog()
{
	if (!checkSelectedModels(true))
		return;

	ProfileToConfig::convertToConfig();
	OptimizedDialog* optimizedDlg = new OptimizedDialog(this, modelContainer);
	optimizedDlg->show();

	setAcceptDrops(false);
}

void rodinP::showShortcutInfoDialog()
{
	ShortcutInfoDialog m_shortcutInfoDlg(this);
}

void rodinP::showInformationDialog()
{
	InformationDialog m_informationDlg(this);

}

QRect rodinP::getReferenceGeometric(QWidget* target_, QWidget* parent_)
{
	QPoint pos;
	if (parent_ == nullptr)
		pos = target_->mapToGlobal(QPoint(0, 0));
	else
		pos = target_->mapTo(parent_, QPoint(0, 0));
	QRect rtn(pos, target_->size());
	return rtn;
}

void rodinP::movePopupDialog()
{
	QWidget* object;
	if (modelControlDialog)
		object = modelControlDialog;
	else if (sidePopupDialog)
		object = sidePopupDialog;
	else
		return;

	QString type = object->objectName();
	QRect rect;
	QPoint pos;
	if (type == "MoveDialog")
	{
		pos = ui.pushButton_move->mapToGlobal(QPoint(0, 0));
		pos = QPoint(pos.x() - object->width() - 20, pos.y());
	}
	else if (type == "ScaleDialog")
	{
		pos = ui.pushButton_scale->mapToGlobal(QPoint(0, 0));
		pos = QPoint(pos.x() - object->width() - 20, pos.y());
	}
	else if (type == "RotationDialog") {
		pos = ui.pushButton_rotate->mapToGlobal(QPoint(0, 0));
		pos = QPoint(pos.x() - object->width() - 20, pos.y());
	}
	else if (type == "PreviewThumbnailDialog" || type == "VolumAnalysisDialogClass") {
		rect = getReferenceGeometric(ui.viewerModule);
		pos = QPoint(rect.right() - object->width() - 10, rect.top());
	}
	else if (type == "SelectCartridgeDialog" || type == "ColoredLayerDialog") {
		rect = getReferenceGeometric(ui.pushButton_selectCartridge);
		pos = QPoint(rect.right() - object->width() + 1, rect.bottom());
	}
	else
		return;
	object->move(pos);
}

void rodinP::finishedDialog()
{
	if (modelControlDialog)
		modelControlDialog = nullptr;
	else if (sidePopupDialog)
	{
		if (ui.pushButton_selectCartridge->isChecked())
			ui.pushButton_selectCartridge->setChecked(false);
		sidePopupDialog = nullptr;
	}
}

void rodinP::showModelControlDlg(Generals::ModelControl controlType_)
{
	if (modelControlDialog)
	{
		bool isSameType = false;
		if (controlType_ == Generals::ModelControl::Move)
			isSameType = dynamic_cast<MoveDialog*>(modelControlDialog);
		else if (controlType_ == Generals::ModelControl::Scale)
			isSameType = dynamic_cast<ScaleDialog*>(modelControlDialog);
		else if (controlType_ == Generals::ModelControl::Rotate)
			isSameType = dynamic_cast<RotationDialog*>(modelControlDialog);
		else return;

		if (isSameType)
		{
			closeControlDlg();
			return;
		}
	}

	closeControlDlg();
	if (controlType_ == Generals::ModelControl::Move)
	{
		MoveDialog* moveDialog = new MoveDialog(this);
		moveDialog->setContents(ui.viewerModule);
		modelControlDialog = moveDialog;
	}
	else if (controlType_ == Generals::ModelControl::Scale)
	{
		ScaleDialog* scaleDialog = new ScaleDialog(this);
		scaleDialog->setContents(ui.viewerModule);
		modelControlDialog = scaleDialog;
	}
	else if (controlType_ == Generals::ModelControl::Rotate)
	{
		RotationDialog* rotationDialog = new RotationDialog(this);
		rotationDialog->setContents(ui.viewerModule);
		modelControlDialog = rotationDialog;
	}
	else return;
	connect(modelControlDialog, SIGNAL(finished(int)), this, SLOT(finishedDialog()));
	//modelControlDialog->setParent(ui.viewerModule);
	movePopupDialog();
	modelControlDialog->show();
}

void rodinP::showSelectCartridgeDlg()
{
	if (sidePopupDialog)
		return;

	closeControlDlg();

	if (Generals::isLayerColorUIMode())
	{
		ColoredLayerDialog *coloredLayerDlg = new ColoredLayerDialog(this);
		coloredLayerDlg->init(ui.viewerModule);
		sidePopupDialog = (QWidget*)coloredLayerDlg;
		connect(coloredLayerDlg, SIGNAL(finished(int)), this, SLOT(finishedDialog()));
		connect(this, SIGNAL(signal_profileChanged()), coloredLayerDlg, SLOT(setContents()));
	}
	else
	{
		/*CartridgeInfo::cartInit();
		//선택된 프린터의 기종이 선택된 기종과 일치할때만 수행
		if (prtData->model == Profile::machineProfile.machine_model)
		{
			prtInfo->setFilamentData(prtData);
			refreshFilaInfo();
		}*///to do sj 왜 필요한지 모르겠음...필요한 경우 getPrinterInfo(); 호출
		SelectCartridgeDialog *selectCartridgeDlg = new SelectCartridgeDialog(this);
		selectCartridgeDlg->init(ui.viewerModule);

		connect(this, SIGNAL(signal_filaColorChanged()), selectCartridgeDlg, SLOT(changeCartridgeColor()));
		connect(selectCartridgeDlg, SIGNAL(signal_pushButton_cartridgeInfoClicked()), this, SLOT(pushButton_cartridgeInfoClicked()));
		sidePopupDialog = (QWidget*)selectCartridgeDlg;
		connect(selectCartridgeDlg, SIGNAL(finished(int)), this, SLOT(finishedDialog()));
	}
	if (!ui.pushButton_selectCartridge->isChecked())
		ui.pushButton_selectCartridge->setChecked(true);
	movePopupDialog();
	sidePopupDialog->show();
}

void rodinP::showCartridgeInfoDlg()
{
	CartridgeDialog *cartInfo = new CartridgeDialog(this);
	connect(cartInfo, SIGNAL(signal_customColorChanged()), this, SLOT(afterFilamentInfoUpdated()));
	cartInfo->show();
}

void rodinP::showPreviewThumbnailDialog()
{
	if (!modelContainer->printingInfo->hasEncodedImage())
		return;
	closeControlDlg();

	QImage imageFromGcode;
	imageFromGcode.loadFromData(QByteArray::fromBase64(modelContainer->printingInfo->getEncodedImage().toUtf8()));
	PreviewThumbnailDialog* previewThumbnailDlg = new PreviewThumbnailDialog(this);
	connect(previewThumbnailDlg, SIGNAL(finished(int)), this, SLOT(finishedDialog()));
	previewThumbnailDlg->setImage(imageFromGcode);
	previewThumbnailDlg->setParent(this);
	sidePopupDialog = previewThumbnailDlg;
	movePopupDialog();
	previewThumbnailDlg->show();
}
#pragma endregion