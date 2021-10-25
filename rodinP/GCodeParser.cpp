#include "stdafx.h"
#include "GCodeParser.h"
#include "ModelDataStorage.h"

GCodeParser::GCodeParser(PrintingInfo* printingInfo_)
	: dataStorage(new ModelDataStorage())
	, printingInfo(printingInfo_)
	, progressHandler(nullptr)
	, countLine(0)
{
}


GCodeParser::~GCodeParser()
{
	if (progressHandler)
		progressHandler->close();
}

void GCodeParser::setProgressHandler(ProgressHandler* handler_)
{
	progressHandler = handler_;
}

//void GCodeParser::init(ModelDataStorage* dataStorage_)
//{
//	dataStorage = dataStorage_;
//	dataStorage->gcodePath.clear();
//	dataStorage->layerCount = 0;
//}

QString GCodeParser::getInnerText(QString input_)
{
	QString temp = input_.split(": ").back().trimmed();

	QRegExp RegularEx("[\\\[\\\]]");
	return temp.remove(RegularEx);
}
QStringList GCodeParser::getInnerTextList(QString input_)
{
	QString temp = getInnerText(input_);
	return temp.split(":");
}
std::vector<float> GCodeParser::getInnerNumberList(QString input_)
{
	std::vector<float> rtn;
	const QRegExp rx(QLatin1Literal("[^0-9.]+"));
	const auto&& parts = input_.split(rx, QString::SkipEmptyParts);
	for (auto num : parts)
	{
		rtn.push_back(num.toFloat());
	}
	return rtn;
}
void GCodeParser::insertGcodePath(PathMode cur_mode_, int extruderNr_, FPoint3& pre_pos_, FPoint3 cur_pos_, float extruder_amount_)
{
	if (pre_pos_ == cur_pos_)
		return;
	//if (pre_pos_.z != cur_pos_.z)
	//{
	//	dataStorage->insertGcodePath(gcode.gcodePaths);
	//	gcode.gcodePaths.paths.clear();
	//}
	//Travel을 G0코드로 쓰지 않고 E값이 없는 G1코드로 사용한 경우 Travel로 인식시킴.
	if (cur_mode_ != TRAVEL && extruder_amount_ == 0)
		cur_mode_ = TRAVEL;

	//이전에 입력된 mode가 -1인 경우 다음 mode로 일치시켜줌. 첫번째 G1코드에 대해 mode확인 불가.
	if (gcode.gcodePaths.paths.size() != 0 && gcode.gcodePaths.paths.back().mode == -1)
		gcode.gcodePaths.paths.back().mode = cur_mode_;

	//값이 없거나 다른 mode인 경우 시작값을 넣어줌
	if (gcode.gcodePaths.paths.size() == 0
		|| gcode.gcodePaths.paths.back().mode != cur_mode_)
	{
		gcode.gcodePaths.paths.push_back(engine::pathG());
		gcode.gcodePaths.paths.back().mode = cur_mode_;
		gcode.gcodePaths.paths.back().poly.push_back(FPoint3(pre_pos_.x, pre_pos_.y, pre_pos_.z));
		gcode.gcodePaths.paths.back().extruderNr = extruderNr_;
	}

	gcode.gcodePaths.paths.back().poly.push_back(FPoint3(cur_pos_.x, cur_pos_.y, cur_pos_.z));
	pre_pos_ = cur_pos_;
}
void GCodeParser::setCurrentPosition(QString line_, FPoint3& cur_pos_, float& extruder_amount_)
{
	QStringList datas = line_.split(" ");
	extruder_amount_ = 0;
	for (auto it : datas)
	{
		if (it.size() == 1)
			continue;
		if (it[0] == "X")
			cur_pos_.x = it.mid(1).toFloat();
		else if (it[0] == "Y")
			cur_pos_.y = it.mid(1).toFloat();
		else if (it[0] == "Z")
			cur_pos_.z = it.mid(1).toFloat();
		else if (it[0] == "E")
			extruder_amount_ = it.mid(1).toFloat();
	}
}

bool GCodeParser::loadPathfromGCode(QString filepath_)
{
	bool b_3DWOXGcode = false;
	bool b_newType3DWOXGcode = false;
	bool b_cura = false;
	bool b_simplify3d = false;
	bool b_slic3r = false;
	bool b_etc_gcode = false;
	//
	printingInfo->clear();
	printingInfo->setGcodeFileName(filepath_);
	printingInfo->setIsLoadedGcode(true);

	float minX, minY, minZ;
	//

	QFile inputFile(filepath_);
	inputFile.open(QIODevice::ReadOnly);
	if (!inputFile.isOpen())
		return false;

	QTextStream stream(&inputFile);

	QString allFileContext = stream.readAll();
	int totalLineCount = allFileContext.split("\n").count() - 1;
	stream.seek(0);

	if (progressHandler)
		progressHandler->setMaximum(totalLineCount);

	QString appName = ";" + AppInfo::getAppName();
	QString line;
	int currentPos = 0;

	//상단 정보부분 파싱.
	//주석이 없는 부분이 시작하기 전까지만 파싱함.
	while (!stream.atEnd())
	{
		if (progressHandler->wasCanceled())
		{
			inputFile.close();
			return false;
		}
		line = stream.readLine();
		if (progressHandler)
			progressHandler->setValue(countLine++);

		if (line == "")
			continue;

		else if (!line.startsWith(";")) //상단 주석이 끝날때까지만
		{
			if (!(b_3DWOXGcode || b_cura || b_simplify3d || b_slic3r))
			{
				printingInfo->setSlicerType(ETC);
				b_etc_gcode = true;
			}
			int pos = stream.pos() - line.size();
			stream.seek(pos);
			break;
		}
		else if (line.startsWith(";IMAGE"))
			printingInfo->appendEncodedImage(line.split(" ").back());
		//for 3DWOX
		else if (b_3DWOXGcode)
		{
			//3DWOX gcode를 찾은 후에 old와 new를 분기해야 함..//
			if (line.startsWith(";[Cartridge_Number]"))
				b_newType3DWOXGcode = true;
			else if (line.startsWith(";PRINTER_MODEL:"))
				printingInfo->setMachineModel(getInnerText(line));
			else if (line.startsWith(";ESTIMATION_TIME:"))//get estimation time for 3DWOX
			{
				QString tempString = getInnerText(line);
				QStringList tempList = tempString.split(":");
				if (tempList.size() != 3)
					continue;
				int totalPrintTime = tempList[0].toInt() * 3600 + tempList[1].toInt() * 60 + tempList[2].toInt();
				printingInfo->setTotalPrintTime(totalPrintTime);
			}
			else if (line.startsWith(";OPERATINGZONE:"))
			{
				QString tempString = getInnerText(line);
				QStringList tempList = tempString.split(":");
				if (tempList.size() != 3)
					continue;
				qglviewer::Vec operatingZone(tempList[0].toFloat(), tempList[1].toFloat(), tempList[2].toFloat());
				printingInfo->setOperatingZone(operatingZone);
			}
			//for old code
			else if (line.startsWith(";ESTIMATION_FILAMENT:"))
			{
				if (!b_newType3DWOXGcode)
					printingInfo->setFilaAmount(0, getInnerText(line).toInt());
			}
			else if (line.startsWith(";MASS:"))
			{
				if (!b_newType3DWOXGcode)
					printingInfo->setFilaMass(0, getInnerText(line).toFloat());
			}
			else if (line.startsWith(";MATERIAL:"))
			{
				if (!b_newType3DWOXGcode)
					printingInfo->setMaterial(0, getInnerText(line));
			}
			//for new code
			else if (line.startsWith(";CARTRIDGE_COUNT_TOTAL:"))
			{
				printingInfo->setCartridgeCountWithInit(getInnerText(line).toInt());
			}
			else if (line.startsWith(";USEDNOZZLE:"))
				printingInfo->setUsedCartridgeCount(getInnerText(line).toInt());
			else if (line.startsWith(";CARTRIDGE_USED_STATE:"))
			{
				QStringList testList = getInnerTextList(line);
				for (int i = 0; i < testList.size(); i++)
				{
					printingInfo->setUsedState(i, (testList[i] == "T" ? true : false));
				}
			}
			else if (line.startsWith(";ESTIMATION_FILAMENT_CARTRIDGE_")
				|| line.startsWith(";MASS_CARTRIDGE_")
				|| line.startsWith(";MATERIAL_CARTRIDGE_"))
			{
				if (b_3DWOXGcode)
				{
					int cartridgeCount = printingInfo->getCartridgeCount();
					for (int i = 0; i < cartridgeCount; i++)
					{
						if (printingInfo->getUsedState(i))
						{
							if (line.startsWith(QString(";ESTIMATION_FILAMENT_CARTRIDGE_%1:").arg(i)))
								printingInfo->setFilaAmount(i, getInnerText(line).toInt());
							else if (line.startsWith(QString(";MASS_CARTRIDGE_%1:").arg(i)))
								printingInfo->setFilaMass(i, getInnerText(line).toFloat());
							else if (line.startsWith(QString(";MATERIAL_CARTRIDGE_%1:").arg(i)))
								printingInfo->setMaterial(i, getInnerText(line));
						}
					}
				}
			}
		}
		// for cura
		else if (b_cura)
		{
			if (line.startsWith(";TIME:")) //get estimation time for cura ";TIME:2750"
				printingInfo->setTotalPrintTime(line.split(":").back().toInt());
			else if (line.startsWith(";MINX:")) //get minX for cura ";MINX:"
				minX = line.split(":").back().toFloat();
			else if (line.startsWith(";MINY:")) //get minY for cura ";MINY:"
				minY = line.split(":").back().toFloat();
			else if (line.startsWith(";MINZ:")) //get minZ for cura ";MINZ:"
				minZ = line.split(":").back().toFloat();
			else if (line.startsWith(";MAXX:")) //get maxX for cura ";MAXX:"
				printingInfo->setOperatingZoneX(line.split(":").back().toFloat() - minX);
			else if (line.startsWith(";MAXY:")) //get maxY for cura ";MAXY:"
				printingInfo->setOperatingZoneY(line.split(":").back().toFloat() - minY);
			else if (line.startsWith(";MAXZ:")) //get maxZ for cura ";MAXZ:"
				printingInfo->setOperatingZoneZ(line.split(":").back().toFloat() - minZ);
			else if (line.startsWith(";Filament used:")) //get estimation filament for cura ";Filament used: 0.591316m, 0.534028m"
			{
				int i = 0;
				for (auto& num : getInnerNumberList(line))
				{
					printingInfo->setFilaAmount(i, num * 1000);
					i++;
				}
			}
		}
		// simplify3d나 slic3r로 확인되는 경우 다음으로 넘어감.
		else if (b_simplify3d || b_slic3r)
			break;

		//슬라이서 타입 확인
		else if (!(b_3DWOXGcode || b_cura || b_simplify3d || b_slic3r))
		{
			if (line.startsWith(appName))
			{
				printingInfo->setSlicerType(SINDOH);
				b_3DWOXGcode = true;
			}
			else if (line.startsWith(";FLAVOR"))
			{
				printingInfo->setSlicerType(CURA);
				b_cura = true;
			}
			else if (line.startsWith("; G-Code generated by Simplify3D(R)"))
			{
				printingInfo->setSlicerType(SIMPLIFY3D);
				b_simplify3d = true;
			}
			else if (line.startsWith("; generated by Slic3r"))
			{
				printingInfo->setSlicerType(SLIC3R);
				b_slic3r = true;
			}
		}
	}

	bool check = false;
	if (b_3DWOXGcode || b_cura)
		check = parserFor3DWOX(stream);
	else if (b_simplify3d)//for simplify3d
		check = parserForSimplify3D(stream);
	if (b_slic3r || b_etc_gcode)//for slic3r
		check = parserForSlic3r(stream);

	if (progressHandler)
		progressHandler->setValue(totalLineCount);
	inputFile.close();

	if (dataStorage->layerCount == 0)
		return false;
	return check;
}
//3DWOX & cura
bool GCodeParser::parserFor3DWOX(QTextStream& stream_)
{
	PathMode cur_mode = PathMode::NA;
	int extruderNr = 0;
	FPoint3 pre_pos = FPoint3(INT2MM(gcode.currentPosition.x), INT2MM(gcode.currentPosition.y), INT2MM(gcode.currentPosition.z));
	FPoint3 cur_pos = FPoint3(INT2MM(gcode.currentPosition.x), INT2MM(gcode.currentPosition.y), INT2MM(gcode.currentPosition.z));
	float extruder_amount = 0;
	bool b_initial_layer = true;//시작시 노즐 들어올리는 동작 무시하기 위함
	QString line;

	while (!stream_.atEnd())
	{
		if (progressHandler->wasCanceled())
			return false;

		line = stream_.readLine();
		if (progressHandler)
			progressHandler->setValue(countLine++);

		if (line == "")
			continue;

		else if (line.startsWith("G0"))
		{
			if (b_initial_layer)
			{
				b_initial_layer = false;
				continue;
			}
			setCurrentPosition(line, cur_pos, extruder_amount);
			insertGcodePath(TRAVEL, extruderNr, pre_pos, cur_pos, extruder_amount);
		}
		else if (line.startsWith("G1"))
		{
			if (b_initial_layer)
			{
				b_initial_layer = false;
				continue;
			}
			setCurrentPosition(line, cur_pos, extruder_amount);
			insertGcodePath(cur_mode, extruderNr, pre_pos, cur_pos, extruder_amount);
		}
		else if (line.startsWith("T0"))
			extruderNr = 0;
		else if (line.startsWith("T1"))
			extruderNr = 1;
		else if (line.startsWith(";TYPE:SKIRT"))
			cur_mode = SKIRT;
		else if (line.startsWith(";TYPE:BRIM"))
			cur_mode = BRIM;
		else if (line.startsWith(";TYPE:SKIN"))
			cur_mode = SKIN;
		else if (line.startsWith(";TYPE:WALL-INNER"))
			cur_mode = WALL_INNER;
		else if (line.startsWith(";TYPE:WALL-OUTER"))
			cur_mode = WALL_OUTER;
		else if (line.startsWith(";TYPE:FILL"))
			cur_mode = FILL;
		else if (line.startsWith(";TYPE:SUPPORT_INTERFACE_ROOF") || line.startsWith(";TYPE:SUPPORT_SKIN")) //new, old code 
			cur_mode = SUPPORT_INTERFACE_ROOF;
		else if (line.startsWith(";TYPE:SUPPORT_INTERFACE_FLOOR") || line.startsWith(";TYPE:SUPPORT_SKIN")) //new, old code 
			cur_mode = SUPPORT_INTERFACE_FLOOR;
		else if (line.startsWith(";TYPE:SUPPORT_MAIN") || line.startsWith(";TYPE:SUPPORT")) //new, old code
			cur_mode = SUPPORT_MAIN;
		else if (line.startsWith(";TYPE:PRERETRACT0"))
			cur_mode = PRERETRACTION0;
		else if (line.startsWith(";TYPE:OVERMOVE0"))
			cur_mode = OVERMOVE0;
		else if (line.startsWith(";TYPE:PRERETRACTX"))
			cur_mode = PRERETRACTIONX;
		else if (line.startsWith(";TYPE:OVERMOVEX"))
			cur_mode = OVERMOVEX;
		else if (line.startsWith(";TYPE:RAFT"))
			cur_mode = RAFT;
		else if (line.startsWith(";TYPE:WIPE_TOWER"))
			cur_mode = WIPE_TOWER;
		else if (line.startsWith(";TYPE:ADJUST_Z_GAP"))
			cur_mode = ADJUST_Z_GAP;
		else if (line.startsWith(";LAYER:"))
		{
			if (gcode.gcodePaths.paths.size() == 0)
				continue;
			dataStorage->insertGcodePath(gcode.gcodePaths);
			gcode.gcodePaths.paths.clear();
		}

		/////////////////////////////////////////////////////////////////////
		//M605 S2 ;Replication Print
		else if (line.startsWith("M605 S2"))
		{
			printingInfo->setIsReplicationPrint(true);
		}//현재 사용 안함. 코드만 copy
	}

	if (cur_mode != PathMode::NA)
	{
		setCurrentPosition(line, cur_pos, extruder_amount);
		insertGcodePath(cur_mode, extruderNr, pre_pos, cur_pos, extruder_amount);
	}
	if (gcode.gcodePaths.paths.size() != 0)
	{
		dataStorage->insertGcodePath(gcode.gcodePaths);
		gcode.gcodePaths.paths.clear();
	}
	return true;
}
//simplify3D
bool GCodeParser::parserForSimplify3D(QTextStream& stream_)
{
	PathMode cur_mode = PathMode::NA;
	int extruderNr = 0;
	FPoint3 pre_pos = FPoint3(INT2MM(gcode.currentPosition.x), INT2MM(gcode.currentPosition.y), INT2MM(gcode.currentPosition.z));
	FPoint3 cur_pos = FPoint3(INT2MM(gcode.currentPosition.x), INT2MM(gcode.currentPosition.y), INT2MM(gcode.currentPosition.z));
	float extruder_amount = 0;
	bool b_initial_layer = true;//시작시 노즐 들어올리는 동작 무시하기 위함
	QString line;
	bool isFirstUsed = false;
	bool isSecondUsed = false;
	while (!stream_.atEnd())
	{
		if (progressHandler->wasCanceled())
			return false;

		line = stream_.readLine();
		if (progressHandler)
			progressHandler->setValue(countLine++);

		if (line == "")
			continue;

		else if (line.startsWith("G0"))
		{
			if (b_initial_layer)
			{
				b_initial_layer = false;
				continue;
			}
			setCurrentPosition(line, cur_pos, extruder_amount);
			insertGcodePath(TRAVEL, extruderNr, pre_pos, cur_pos, extruder_amount);
		}
		else if (line.startsWith("G1"))
		{
			if (b_initial_layer)
			{
				b_initial_layer = false;
				continue;
			}
			setCurrentPosition(line, cur_pos, extruder_amount);
			insertGcodePath(cur_mode, extruderNr, pre_pos, cur_pos, extruder_amount);
		}
		else if (line.startsWith("T0"))
		{
			extruderNr = 0;
			if (!isFirstUsed)
				isFirstUsed = true;
		}
		else if (line.startsWith("T1"))
		{
			extruderNr = 1;
			if (!isSecondUsed)
				isSecondUsed = true;
		}
		//3DWOX || simplify3d
		else if (line.contains("; feature skirt")) //확인필요
			cur_mode = SKIRT;
		else if (line.startsWith("; feature solid"))
			cur_mode = SKIN;
		else if (line.startsWith("; feature inner"))
			cur_mode = WALL_INNER;
		else if (line.startsWith("; feature outer"))
			cur_mode = WALL_OUTER;
		else if (line.startsWith("; feature infill"))
			cur_mode = FILL;
		else if (line.startsWith("; feature support")) //확인필요
			cur_mode = SUPPORT_MAIN;
		else if (line.startsWith("; feature raft"))
			cur_mode = RAFT;

		//for simplify3d
		else if (line.startsWith(";   Build time:")) //get estimation time for simplify3d ";   Build time: 0 hours 34 minutes"
		{
			QString temp = line.split(":").back().trimmed();
			QStringList timeList = temp.split(" ");
			int totalPrintTime = timeList[0].toInt() * 3600 + timeList[2].toInt() * 60;
			printingInfo->setTotalPrintTime(totalPrintTime);
		}
		else if (line.startsWith(";   Filament length:")) //get estimation filament for simplify3d ";   Filament length: 1442.7 mm (1.44 m)"
		{
			bool isMulti = isFirstUsed && isSecondUsed;
			if (isMulti)
				continue;
			QString temp = line.split(":").back().trimmed();
			QStringList lengthList = temp.split(" ");
			if (isFirstUsed)
				printingInfo->setFilaAmount(0, lengthList[0].toFloat());
			else
				printingInfo->setFilaAmount(1, lengthList[0].toFloat());
		}
		else if (line.startsWith("; layer"))
		{
			if (gcode.gcodePaths.paths.size() == 0)
				continue;
			dataStorage->insertGcodePath(gcode.gcodePaths);
			gcode.gcodePaths.paths.clear();
		}
	}

	if (cur_mode != -1)
	{
		setCurrentPosition(line, cur_pos, extruder_amount);
		insertGcodePath(cur_mode, extruderNr, pre_pos, cur_pos, extruder_amount);
	}
	if (gcode.gcodePaths.paths.size() != 0)
	{
		dataStorage->insertGcodePath(gcode.gcodePaths);
		gcode.gcodePaths.paths.clear();
	}
	return true;
}
//Slic3r & etc
bool GCodeParser::parserForSlic3r(QTextStream& stream_)
{
	PathMode cur_mode = PathMode::WALL_OUTER; //mode를 특정할 수 없으므로 wall outer로 지정함.
	int extruderNr = 0;
	FPoint3 pre_pos = FPoint3(INT2MM(gcode.currentPosition.x), INT2MM(gcode.currentPosition.y), INT2MM(gcode.currentPosition.z));
	FPoint3 cur_pos = FPoint3(INT2MM(gcode.currentPosition.x), INT2MM(gcode.currentPosition.y), INT2MM(gcode.currentPosition.z));
	float extruder_amount = 0;
	bool b_initial_layer = true;//시작시 노즐 들어올리는 동작 무시하기 위함
	QString line;
	std::vector<int> filamentList;

	while (!stream_.atEnd())
	{
		if (progressHandler->wasCanceled())
			return false;

		line = stream_.readLine();
		if (progressHandler)
			progressHandler->setValue(countLine++);

		if (line == "")
			continue;

		else if (line.startsWith("G0"))
		{
			if (b_initial_layer)
			{
				b_initial_layer = false;
				continue;
			}
			setCurrentPosition(line, cur_pos, extruder_amount);
			insertGcodePath(TRAVEL, extruderNr, pre_pos, cur_pos, extruder_amount);
		}
		else if (line.startsWith("G1"))
		{
			if (b_initial_layer)
			{
				b_initial_layer = false;
				continue;
			}
			bool changeLayer = false;
			setCurrentPosition(line, cur_pos, extruder_amount);
			if (pre_pos.z < cur_pos.z)
				changeLayer = true;
			insertGcodePath(cur_mode, extruderNr, pre_pos, cur_pos, extruder_amount);
			if (gcode.gcodePaths.paths.size() > 1 && changeLayer)
			{
				dataStorage->insertGcodePath(gcode.gcodePaths);
				gcode.gcodePaths.paths.clear();
			}
		}
		else if (line.startsWith("T0"))
			extruderNr = 0;
		else if (line.startsWith("T1"))
			extruderNr = 1;
		else if (line.startsWith("; filament used")) //get estimation filament for slic3r "; filament used = 2118.4mm (5.1cm3)"
		{
			QString temp = line.split(" = ").back();
			float filamentAmount = temp.split("mm").front().toFloat();
			printingInfo->setFilaAmount(filamentList.size(), filamentAmount);
			filamentList.push_back(filamentAmount);
		}
	}

	if (cur_mode != -1)
	{
		setCurrentPosition(line, cur_pos, extruder_amount);
		insertGcodePath(cur_mode, extruderNr, pre_pos, cur_pos, extruder_amount);
	}
	if (gcode.gcodePaths.paths.size() != 0)
	{
		dataStorage->insertGcodePath(gcode.gcodePaths);
		gcode.gcodePaths.paths.clear();
	}
	return true;
}