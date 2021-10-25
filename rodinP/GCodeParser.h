#pragma once
#include "gcodeExport.h"
#include "PrintingInfo.h"
#include "ProgressHandler.h"

class ModelDataStorage;
class GCodeParser
{
public:
	GCodeParser(PrintingInfo* printingInfo_);
	~GCodeParser();

	void setProgressHandler(ProgressHandler* handler_);
	//void init(ModelDataStorage* dataStorage_);
	bool loadPathfromGCode(QString filepath_);
	ModelDataStorage* getDataStorage() { return dataStorage; }

	static QString getInnerText(QString input_);
	static QStringList getInnerTextList(QString input_);
	static std::vector<float> getInnerNumberList(QString input_);

private:
	void insertGcodePath(PathMode cur_mode_, int extruderNr_, FPoint3& pre_pos_, FPoint3 cur_pos_, float extruder_amount_);
	void setCurrentPosition(QString line_, FPoint3& cur_pos_, float& extruder_amount_);

	bool parserFor3DWOX(QTextStream& stream_); //3DWOX & cura
	bool parserForSimplify3D(QTextStream& stream_); //simplify3D
	bool parserForSlic3r(QTextStream& stream_); //Slic3r & etc

	ModelDataStorage* dataStorage;
	PrintingInfo* printingInfo;
	engine::GCodeExport gcode;
	ProgressHandler *progressHandler;
	int countLine;
};

