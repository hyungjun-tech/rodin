#pragma once
#include "gcodeExport.h"
#include "InfillVoxel.h"
#include "CartridgeInfo.h"

class PairIndexes
{
public:
	PairIndexes(int cartridgeIndex_, int index_)
	{
		cartridgeIndex = cartridgeIndex_;
		index = index_;
	}
	int cartridgeIndex;
	int index;
};
class GCodePathforDraw
{
public:
	int pathNo;
	int polyNo;
	int mode;
	Mesh::Point start;
	Mesh::Point end;
	int extruderNo;
};

class ModelContainer;
class ModelDataStorage;
class PrintingInfo;
class ProgressHandler;
class GCodeGenerator : public QObject
{
	Q_OBJECT
public:
	GCodeGenerator(QWidget *parent_);
	GCodeGenerator();
	~GCodeGenerator();

	void init(ModelContainer* modelContainer_, ModelDataStorage* dataStorage_);
	void setConfig(std::vector<ConfigSettings>& p_configs);
	bool processing();
	QString getWarningMessage();
	static QString generateAuthentificationCode();
	static QString generateImageTextCode(QString filePath);
	static QString generateImageTextCode(QByteArray encodedCode_);
	static QString generateFileNameCode(QString fileName_);
	static QString generateEmailAddressCode(QString address_);
	static QString generateCodeForOtherSlicer(PrintingInfo* printingInfo_);

	QString generateM104Code();
	QString generateM140Code();
	QString generateM109Code();
	QString generateM190Code();
public slots:
	void cancel_processor() { abort = true; }
private:
	ProgressHandler* progressHandler;
	//ModelContainer* modelContainer;
	std::vector<IMeshModel*> models;
	ModelDataStorage* dataStorage;
	PrintingInfo* printingInfo;

	bool abort;

	engine::GCodeExport gcode;
	int currentLayers_with_adhesion;

	std::vector<ConfigSettings> configs;
	
	std::vector<engine::GCodePathConfig> pathconfigs_outer_wall;
	std::vector<engine::GCodePathConfig> pathconfigs_inner_wall;
	std::vector<engine::GCodePathConfig> pathconfigs_infill;
	std::vector<engine::GCodePathConfig> pathconfigs_skin;
	std::vector<engine::GCodePathConfig> pathconfigs_skirt;

	engine::GCodePathConfig pathconfigs_support_main;
	engine::GCodePathConfig pathconfigs_support_interface_roof;
	engine::GCodePathConfig pathconfigs_support_interface_floor;
	//engine::GCodePathConfig pathconfigs_brim;
	engine::GCodePathConfig pathconfigs_wipe_tower;

	std::vector<InfillVoxel> infill_tree;

	bool b_reverseExtruderForSupport;
	bool b_raftPathLengthLimit;
	bool b_firstMultipleWipetower;
	bool b_needWipetowerFirst;
	bool b_skirtPrinted;

	int support_minX;
	int support_minY;
	int support_segmentLength;

	std::vector<int> firstToolChangeInfo;
	//engine::LayerPathG gcodePaths;

	ConfigsSetForFlow configs_set_flow;
	ConfigsSetForSpeed configs_set_speed;
	ConfigSetForRetraction configs_set_retraction;
	ConfigSetForCooling configs_set_cooling;

private:
	void clear();
	bool writeGCode();
	void setPrintingInfo();
	bool writeStartEndCode(QString filePath_);
	void addVolumeLayerToGCode(engine::GCodePlanner& gcodeLayer, int volumeIdx, int layerNr, std::vector<PairIndexes> pairInfo_cart_wipetower);
	void addSupportToGCode(engine::GCodePlanner& gcodeLayer, int layerNr, std::vector<PairIndexes> pairInfo_cart_wipetower);
	void addWipeTower(engine::GCodePlanner& gcodeLayer, int layerNr, int setExtruder, std::vector<PairIndexes> pairInfo_cart_wipetower);
	void writeAdjustZGapCode();
	void writeRaftGcode();

	void writeStartCode();
	void writeCurrentProfile();
	void writeAuthentificationCode();
	void writeModelInfo();
	void writeCartridgeInfo();
	void writeRaftCartridgeCode();
	void writeDefaultStartCode();
	void writeDefaultStartCode_New();
	void writeZOffsetCode();
	void writeRetractionCodeForStart();
	void writeTempPathCodeToGcode(QString _pathCode, FILE* _out);
	//std::string generateEndCode();
	void writeTemperatureCode();
	void writeWaitTemperatureCode();
	void writeEmailCode();

	void writeImageText(QString filePath);
	void writeFileName(QString fileName_);
	void writeEmailAddress(QString address_);
	void splitEncodedCode(QByteArray inputCode_, std::string preCode_);
	static QString generateSplitEncodedCode(QByteArray encodedCode_, QString preCode_);

	void writeM104Code(int toolIndex_);
	void writeM109Code(int toolIndex_);

	QString regenerateCodeByProfileKey(std::string _default_code);
	   	 
	double getFilamentDensity(QString material_)
	{
		const double filamentDensity_PLA = 0.00124;
		const double filamentDensity_ABS = 0.00104;
		const double filamentDensity_WOOD = 0.00070;
		const double filamentDensity_PETG = 0.00123;
		const double filamentDensity_HIPS = 0.00105;
		const double filamentDensity_FLEXIBLE = 0.00121;
		const double filamentDensity_PVA = 0.00125;
		const double filamentDensity_PVA_plus = 0.00125;
		const double filamentDensity_ASA = 0.00107;
		const double filamentDensity_TPU = 0.00120;
		const double filamentDensity_RZCB = 0.00102;
		const double filamentDensity_RZGF = 0.00102;
		const double filamentDensity_RZSU = 0.00102;

		if (material_ == "PLA")	return filamentDensity_PLA;
		else if (material_ == "ABS") return filamentDensity_ABS;
		else if (material_ == "WOOD") return filamentDensity_WOOD;
		else if (material_ == "PETG") return filamentDensity_PETG;
		else if (material_ == "HIPS") return filamentDensity_HIPS;
		else if (material_ == "FLEXIBLE") return filamentDensity_FLEXIBLE;
		else if (material_ == "PVA") return filamentDensity_PVA;
		else if (material_ == "PVA+") return filamentDensity_PVA_plus;
		else if (material_ == "ASA") return filamentDensity_ASA;
		else if (material_ == "TPU") return filamentDensity_TPU;
		else if (material_ == "RZCB") return filamentDensity_RZCB;
		else if (material_ == "RZGF") return filamentDensity_RZGF;
		else if (material_ == "RZSU") return filamentDensity_RZSU;
		
		return filamentDensity_PLA;
	}

	int getExtrusionWidthByLayer(int _layer_number, int _config_extusion_width);
	void calculatePrintingArea();

signals:
	void signal_setLabelText(QString);
	void signal_setValue(int);
};

