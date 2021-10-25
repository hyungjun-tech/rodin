#pragma once
#include "SliceCalculator.h"

class ModelContainer;
class ModelDataStorage;
class ConfigSettings;
class ProgressHandler;
class SliceProcessor : public QObject
{
	Q_OBJECT
public:
	SliceProcessor(QWidget *parent);
	SliceProcessor();
	~SliceProcessor();
	void init(ModelContainer* modelContainer_, bool forSelectedModels_ = false);
	void setConfig(std::vector<ConfigSettings>& p_configs);
	bool processingForSupportEdit();
	bool processing();
	QString getWarningMessage();
	QString getErrorMessage();
	bool processingForPrintOptimum();
	ModelDataStorage* getDataStorage() { return dataStorage; }
	//ConfigSettings config;
public slots:
	void cancel_processor() { abort = true; }
private:
	ProgressHandler* progressHandler;
	//ModelContainer* modelContainer;
	ModelDataStorage* dataStorage;
	SliceCalculator* calculator;
	std::vector<IMeshModel*> models;

	std::vector<ConfigSettings> configs;
	bool b_overhang, b_thin;
	int b_errorCode;
	bool abort;

	void positioning();

	void generatingSupportDataStructure();
	bool generatingPolygonLayers();
	bool generatingSliceLayers();
	bool generatingSupportLayers();
	bool applyingRaftThickness();
	bool generatingInsetFirstLayer();
	bool generatingInset(bool test_thin = true);
	bool generatingSkinAndSparse();
	bool generatingWipeTower();
	bool generatingSkirtRaft();

Q_SIGNALS:
	void signal_setLabelText(QString);
	void signal_setValue(int);
};

