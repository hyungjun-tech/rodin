#pragma once

#include "ui_rodinP.h"

class rodinP : public QMainWindow
{
	Q_OBJECT

public:
	rodinP(QWidget *parent = 0);
	~rodinP();

	void loadModelFile(QString fileName);
	void loadFiles(QStringList fileList_);

public slots:
	void afterProfileChanged(bool sliceFlag = true);
	void changeMachine(QString machine_, bool machineChanged = false);
private slots:
	void pushButton_loadIcon_clicked();
	void pushButton_settingIcon_clicked();
	void pushButton_normalViewer_clicked(bool checked);
	void pushButton_editViewer_clicked(bool checked);
	void pushButton_layerViewer_clicked(bool checked);
	void pushButton_print_clicked();
	void pushButton_move_clicked();
	void pushButton_scale_clicked();
	void pushButton_rotate_clicked();

	void pushButton_isometric_clicked();
	void pushButton_top_clicked();
	void pushButton_left_clicked();
	void pushButton_right_clicked();
	void pushButton_front_clicked();
	void pushButton_selectCartridge_clicked();

	void switch_coloredLayer_changed(bool switch_);

	void checkBox_showOnlyCurrentPath_toggled(bool);
	void checkBox_MoveLine_toggled(bool);
	void slider_layer_valueChanged(int value_);
	void slider_supportEdit_valueChanged(int value_);
	void comboBox_radiusSize_currentIndexChanged(int index_);

	void actionLoad_model_file_triggered();
	void actionSave_model_file_triggered();
	void actionLoad_GCode_triggered();
	void actionSave_GCode_triggered();
	void actionSave_GCode_printer_triggered();
	void actionInsert_image_toGcode_triggered();
	void actionDelete_image_inGcode_triggered();
	void actionPrint_triggered();

	void actionCustom_Profile_triggered();
	void actionImport_Profile_triggered();
	void actionExport_Profile_triggered();
	void actionReset_Profile_triggered();
	void actionReplace_Profile_triggered();

	void actionMachine_Setting_triggered();
	void actionCartridge_Information_triggered();
	void actionStartEnd_Gcode_triggered();

	void actionHome_triggered();
	void actionPosition_triggered();
	void actionScale_triggered();
	void actionRotation_triggered();
	void actionNormal_Viewer_triggered();
	void actionEdit_Support_triggered();
	void actionLayer_Viewer_triggered();

	void actionMy_Printers_triggered();
	void actionAdd_My_Printer_triggered();
	void actionAdd_Local_Printer_triggered();
	void actionAdd_IP_Address_triggered();
	void actionWeb_Browser_triggered();
	void actionReplication_Print_triggered();
	void actionThickness_Overhang_triggered();
	void actionOptimized_Direction_triggered();
	void actionOnline_FAQ_triggered();
	void actionShortcutInfo_triggered();
	void actionUpdate_triggered();
	void actionInformation_triggered();
	void actionExperimental_Function_triggered();

	void pushButton_cartridgeInfoClicked();

	void menuLanguageGroup_triggered(QAction* action);
	void menuModeGroup_triggered(QAction*);
	void menuTemperatureGroup_triggered(QAction*);

	void finishedDialog();
	void afterViewerChanged(Generals::ViewerMode viewerMode_);
	void setEnableMenuForModel();
	void afterFilamentInfoUpdated();
	void afterExpandedPrintModeChanged();
	void afterCurrentPrinterChanged();

	void updateMaxLayerForPreview(int maxLayer_);
	void showFindIPPrinter();

private:
	void loadFiles();
	void loadModelFile();
	void loadModelFile(QStringList fileList_);
	void saveModelFile();

	void loadPathfromGCode();
	void saveGCode();
	void saveGCodeToPrinter();
	void insertImageToGCode();
	void deleteImageInGCode();

	void viewProfileSettingMultiDlg();
	void viewProfileSettingEasymodeDlg();
	void viewProfileSettingMachineDlg();
	void viewProfileSettingCustomProfileDlg(); 
	void viewStartEndGcodeDlg();

	void resetProfile();
	void importProfile();
	void exportProfile();
	void replaceProfile();

	void showMyPrinters();
	void showAddNetPrinter();
	void showAddUSBPrinter();
	void getDefaultPrinter();

	void showMultiplyDialog();
	
	void setBrushRadius(int r_int);

	void showAnalysisDialog();
	void showOptimizedDialog();

	void showShortcutInfoDialog();
	void showInformationDialog();
	void showPreviewThumbnailDialog();

	void checkOpenGLversion();
	void viewBrowser();
	bool checkBeforePrint();
	bool checkMaterialBeforePrint();
	bool checkMaterialBeforeSlice();
	bool showAuthentificationPrint();
	void showPrintConfirm();
	void printModel();
	//void setCurLayerNo(int a) { ui.CurLayerNo->setNum(a); }

	void setNormalViewerMode();
	void setEditViewerMode();
	bool setLayerViewerMode();
	void setReplicationPrintMode();

	//MODE//
	void setSettingMode(QString);

	//Temperature Unit//
	void setTemperatureUnit();

	void showModelControlDlg(Generals::ModelControl controlType_);

	void deleteFilaInfo();
	void addFilaInfo(int idx, int filaRemainCnt, QString rgb, QString material);

	void showCartridgeInfoDlg();
	void chkVersion();
	void faqOpen();
	void showSelectCartridgeDlg();
	void afterMachineChanged();

	//etc//
	void excuteExperimentalFunction();
	

protected:
	//Events
	void dragEnterEvent(QDragEnterEvent *e);
	void dropEvent(QDropEvent *e);
	virtual void mousePressEvent(QMouseEvent* e);

	void closeEvent(QCloseEvent * event);
	virtual void resizeEvent(QResizeEvent * e);
	virtual void moveEvent(QMoveEvent *e);
	void keyPressEvent(QKeyEvent *e);
private:
	Ui::rodinPClass ui;
	QString m_settingMode;
	QString m_easyModeProp;
	QString defaultLocale;
	QTranslator translator;
	bool m_fileDialogOpen;
	QString profileVersion;
	int layerCount;
	ModelContainer * modelContainer;
	QWidget* sidePopupDialog;
	QWidget* modelControlDialog;

	void setUI();
	void getInitValue();
	void init();
	void setConnection();

	void loadPathfromGCode(QString fileName);
	
	QRect getReferenceGeometric(QWidget* target_, QWidget* parent_ = nullptr);
	void movePopupDialog();

	QString getCurrentMode();
	bool editSupport();
	bool slice(bool withMessage_ = true);

	void getPrinterInfo();

	void refreshFilaInfo();

	void closeControlDlg();
	void updateMaxLayerForSupport(int maxLayer_);


	bool checkModels(bool withMessage_ = false);
	bool checkSelectedModels(bool withMessage_);

	//UI enable control
	void setEnableMenuForViewer();
	void setAllMenuEnabled(bool flag_);
	void setControlMenuEnabled(bool flag_);
	void setSelectCartDlgVisible();

	void setPrintString();

	bool validateMaterialCombination();

	void createLangMenu(QString seletedLang);
	void setIconLang(QString lang);
signals:
	void signal_filaColorChanged();
	void signal_profileChanged();

private:
	//검토 필요
	//void signal_viewer_changed();


	//삭제됨
	//float m_calFilaUsed;
	//QString gcodeFileName;

	//ProfileSettingMachineDlg *m_ProfileMachineDlg;

	//VolumeAnalysisDialog* m_AnalysisDlg;
	//VolumeAnalysis m_VolumeAnalysis;
	//layerSliderDialog *m_layerSliderDlg;

	//QString m_temperatureUnit;
	//std::vector<int> m_filaRemains;

	//rodin machine setting//
	//QString m_profilePath;//Profile::profilePath로 변경
	//QString m_machineModel;
	//QString appPath;
	//std::vector<int> m_materialAvailableIndex;
	//std::vector<const char*> m_materialList;

	//validate slicing condition by material selection for dual printing//
	//bool b_isAnalysisDlgMode = false;
	//bool b_isAuthentificationCodeFromGcode = false;

	//bool fileToPrinter();
	//std::vector<bool> m_usesCartridgeState;

	//bool resized;

	//int startX;
	//int startY;
	//QString _connectionType;
	//QString _printerName;
	//QString currentPrtIP;

	//QString filaColor = "230,240,50";	//필라멘트 색상 (필라멘트 정보가 없을 때 기본색상)
	//int filaTotalLen;			//필라멘트 총 길이
	//int filaUsedLen;			//필라멘트 사용길이
	//QString filaType;		//필라멘트 종류

	//std::vector<bool> getUsedCartridgeStateFromIndex(); //actually used cartridge index using index setting.//
	//std::vector<bool> getUsedCartridgeStateFromSlicing(); //actually used cartridge index using slicing.//
	//std::string getTFstringFromCartridgeState(std::vector<bool>);
	//int getUsedCountFromCartridgeState(std::vector<bool>);
	//calculating printing area min, max..//
	/*void calculatePrintingArea(int mode);
	double min_X_printing;
	double max_X_printing;
	double min_Y_printing;
	double max_Y_printing;
	double min_Z_printing;
	double max_Z_printing;

	//calculating volumes area min, max..//*/

	//void updateLayerViewerAfterSlicing();

	//cartridge change information vector//
	//size == cartridge size//(T0 first change layer Nr, T1 first change Nr)//
	//int getExtrudeNrLaterNozzleTemperatureControl(const SliceProfile sliceProfile, std::vector<bool> usesCartridgeState, std::vector<int> firstToolChangeInfo);


	//by_OJT
	//void ConvexHullRotateRun(VboVolume* vol);->viewer.h로 이동
	//bool m_VolumeOverlapCheck;
	//bool isVolumeOverlap();
	//SelectCartridgeDialog *m_SelectCartDlg;
	//ColoredLayerDialog *m_ColoredLayerDlg;

	//colored layer list//
	//m_ColoredLayerList = m_cartridgeLayerList + m_pauseLayerList//
	//std::vector<ColoredLayer> m_ColoredLayerList;
	//std::vector<ColoredLayer> m_cartridgeLayerList;

	//authentification print mode//
	//bool b_insertAuthentificationCodeCompleted = false;

	//void setFilaRemains(QString rgb, int a);

	//modified print time calculating..//
	//int m_adjustedPrintTime;
	//std::vector<bool> m_usesCartridgeStateFromIndex; //--> False만 확정적.. True는 확정을 못함..예)support가 실제로 나오지 않을 수 있음..//


	//int startcodeIndex = 0;

	//int m_leftYOffset = 40;
	//int m_rightYOffset = 20;
	//std::vector<Volume> splitVolume(const Volume* volume);
	//std::vector<Volume> splitVolume_onlyMaterial(const Volume* volume);
	//void mergingVolumes(std::vector<Volume*>& volumes);
};