/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#include "stdafx.h"
#include <stdarg.h>

#include "gcodeExport.h"
#include "pathOrderOptimizer.h"
#include "timeEstimate.h"
#include "settings.h"

namespace engine {


	void GCodeStorage::clear()
	{
		this->gcodeStrings_original.clear();
		this->gcodeStrings_modified.clear();
	}

	GCodeExport::GCodeExport()
	{
		clear();
	}

	GCodeExport::~GCodeExport()
	{
		if (f && f != stdout)
			fclose(f);
	}

	void GCodeExport::setExtruderOffset(int id, Point p)
	{
		extruderOffset[id] = p;
	}

	void GCodeExport::setSwitchExtruderCode(std::string preSwitchExtruderCode, std::string postSwitchExtruderCode, int index)
	{
		this->preSwitchExtruderCode[index] = preSwitchExtruderCode;
		this->postSwitchExtruderCode[index] = postSwitchExtruderCode;
	}

	void GCodeExport::setToolChangeSettings(int toolchangeRetractionAmount, int toolchangeRetractionSpeed, int toolchangeExtraRestartAmount, int toolchangeExtraRestartSpeed, int index)
	{
		this->toolchangeRetractionAmount[index] = INT2MM(toolchangeRetractionAmount);
		this->toolchangeRetractionSpeed[index] = toolchangeRetractionSpeed;
		this->toolchangeExtraRestartAmount[index] = INT2MM(toolchangeExtraRestartAmount);
		this->toolchangeExtraRestartSpeed[index] = toolchangeExtraRestartSpeed;
	}

	void GCodeExport::setFlavor()
	{
		//this->flavor = flavor;
		//if (flavor == GCODE_FLAVOR_MACH3)
		//    for(int n=0; n<MAX_EXTRUDERS; n++)
		//        extruderCharacter[n] = 'A' + n;
		//else
		//    for(int n=0; n<MAX_EXTRUDERS; n++)
		//        extruderCharacter[n] = 'E';

		extruderCharacter = 'E';
	}
	//int GCodeExport::getFlavor()
	//{
	//	return this->flavor;
	//}

	void GCodeExport::setFilename(const wchar_t* filename)
	{
		f = _wfopen(filename, L"w+");
	}

	bool GCodeExport::isOpened()
	{
		return f != nullptr;
	}

	//extrusion_perMM_ set value..//
	//void GCodeExport::setExtrusion(int layerThickness, int filamentDiameter, std::vector<int> layerFlow, int wipeTowerFlow)
	//{
	//	double filamentArea = M_PI * (INT2MM(filamentDiameter) / 2.0) * (INT2MM(filamentDiameter) / 2.0);
	//	//if (flavor == GCODE_FLAVOR_ULTIGCODE || flavor == GCODE_FLAVOR_REPRAP_VOLUMATRIC)//UltiGCode uses volume extrusion as E value, and thus does not need the filamentArea in the mix.
	//	//    extrusionPerMM = INT2MM(layerThickness);
	//	//else
	//	//    extrusionPerMM = INT2MM(layerThickness) / filamentArea * double(flow) / 100.0;

	//	//�и��� ���� extrusionPerMM�� �迭ȭ��..//
	//	for (int i = 0; i < layerFlow.size(); i++)
	//	{
	//		extrusionPerMM[i] = INT2MM(layerThickness) / filamentArea * double(layerFlow.at(i)) / 100.0;

	//		
	//	}

	//	extrusionPerMM_wipetower = INT2MM(layerThickness) / filamentArea * double(wipeTowerFlow) / 100.0;

	//}

	void GCodeExport::setExtrusion(int layerThickness, int filamentDiameter, ConfigsSetForFlow _configs_set_flow)
	{
		const double filamentArea = M_PI * (INT2MM(filamentDiameter) / 2.0) * (INT2MM(filamentDiameter) / 2.0);

		//�и��� ���� extrusionPerMM�� �迭ȭ��..//
		for (int i = 0; i < Profile::configSettings.size(); ++i)
		{
			extrusion_per_MM_overall[i] = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.overall_flow.at(i)) / 100.0;
			extrusion_per_MM_initial_layer[i] = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.initial_layer_flow.at(i)) / 100.0;
			extrusion_per_MM_infill[i] = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.infill_flow.at(i)) / 100.0;
			extrusion_per_MM_outer_wall[i] = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.outer_wall_flow.at(i)) / 100.0;
			extrusion_per_MM_inner_wall[i] = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.inner_wall_flow.at(i)) / 100.0;
			extrusion_per_MM_top_bottom[i] = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.top_bottom_flow.at(i)) / 100.0;

		}

		extrusion_per_MM_support_main = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.support_main_flow) / 100.0;
		extrusion_per_MM_support_interface_roof = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.support_interface_roof_flow) / 100.0;
		extrusion_per_MM_support_interface_floor = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.support_interface_floor_flow) / 100.0;
		//extrusion_per_MM_skirt = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.skirt_flow) / 100.0;
		//extrusion_per_MM_brim = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.brim_flow) / 100.0;
		extrusion_per_MM_wipe_tower = INT2MM(layerThickness) / filamentArea * double(_configs_set_flow.wipe_tower_flow) / 100.0;
	}
	
	void GCodeExport::setRetractionSettings(int retractionAmount, int retractionSpeed, int minimalExtrusionBeforeRetraction, int zHop, int retractionAmountPrime, int index)
	{
		this->retractionAmount[index] = INT2MM(retractionAmount);
		this->retractionAmountPrime[index] = INT2MM(retractionAmountPrime);
		this->retractionSpeed[index] = retractionSpeed;
		//this->extruderSwitchRetraction = INT2MM(extruderSwitchRetraction);
		this->minimalExtrusionBeforeRetraction[index] = INT2MM(minimalExtrusionBeforeRetraction);
		this->retractionZHop[index] = zHop;
	}

	void GCodeExport::setZ(int z)
	{
		this->zPos = z;
	}

	void GCodeExport::setZ_timeCalculate(int z)
	{
		this->zPos_timeCalculate = z;
	}

	Point GCodeExport::getPositionXY()
	{
		return Point(currentPosition.x, currentPosition.y);
	}

	Point GCodeExport::getPositionXY_timeCalculate()
	{
		return Point(currentPosition_timeCalculate.x, currentPosition_timeCalculate.y);
	}

	void GCodeExport::resetStartPosition()
	{
		startPosition.x = INT32_MIN;
		startPosition.y = INT32_MIN;
	}

	Point GCodeExport::getStartPositionXY()
	{
		return Point(startPosition.x, startPosition.y);
	}

	int GCodeExport::getPositionZ()
	{
		return currentPosition.z;
	}

	int GCodeExport::getPositionZ_timeCalculate()
	{
		return currentPosition_timeCalculate.z;
	}

	int GCodeExport::getExtruderNr()
	{
		return extruderNr;
	}

	double GCodeExport::getTotalFilamentUsed(int e)
	{
		if (e == extruderNr)
			return totalFilament[e] + extrusionAmount;
		return totalFilament[e];
	}

	double GCodeExport::getTotalPrintTime()
	{
		return totalPrintTime;
	}

	void GCodeExport::resetTotalPrintTime()
	{
		estimateCalculator.reset();
	}

	void GCodeExport::updateTotalPrintTime()
	{
		totalPrintTime += estimateCalculator.calculate();

		estimateCalculator.reset();
	}

	void GCodeExport::updatePathPrintTime_timeCalculate()
	{
		pathPrintTime_timeCalculate = estimateCalculator_timeCalculate.calculate();

		estimateCalculator_timeCalculate.reset();

	}

	void GCodeExport::writeNewLine()
	{
		fprintf(f, "\n");
	}

	void GCodeExport::writeComment(const char* comment, ...)
	{
		va_list args;
		va_start(args, comment);
		fprintf(f, ";");
		vfprintf(f, comment, args);
		fprintf(f, "\n");
		va_end(args);
	}

	void GCodeExport::writeLine(const char* line, ...)
	{
		va_list args;
		va_start(args, line);
		vfprintf(f, line, args);
		fprintf(f, "\n");
		va_end(args);
	}

	void GCodeExport::resetExtrusionValue()
	{
		if (extrusionAmount != 0.0)
		{
			fprintf(f, "G92 %c0\n", extruderCharacter);
			totalFilament[extruderNr] += extrusionAmount;
			extrusionAmountAtPreviousRetraction -= extrusionAmount;
			extrusionAmount = 0.0;
		}
	}

	void GCodeExport::resetExtrusionValue_timeCalculate()
	{
		if (extrusionAmount_timeCalculate != 0.0)
		{
			//fprintf(f, "G92 %c0\n", extruderCharacter);
			//totalFilament[extruderNr] += extrusionAmount;
			extrusionAmountAtPreviousRetraction_timeCalculate -= extrusionAmount_timeCalculate;
			extrusionAmount_timeCalculate = 0.0;
		}
	}

	void GCodeExport::resetPathPrintTime_timeCalculate()
	{
		pathPrintTime_timeCalculate = 0.0;
	}

	void GCodeExport::clear_timeCalculate()
	{
		estimateCalculator_timeCalculate.clear();
	}

	void GCodeExport::writeDelay(double timeAmount)
	{
		fprintf(f, "G4 P%d\n", int(timeAmount * 1000));
		totalPrintTime += timeAmount;
	}
	
	void GCodeExport::writeMove(Point p, int speed, int lineWidth, int layerNr, bool b_spiralize, bool initRaft)
	{
		if (currentPosition.x == p.X && currentPosition.y == p.Y && currentPosition.z == zPos)
			return;

		double preAmount = extrusionAmount;
		bool b_add_support_filament = false;

		//���⼭ gcodePaths(LayerPathG).paths�� ������ �־���..//
		//���� path�� ������ gcode.path�� ������ ���߾���.. //
		//path->config->name ==> gcode.current_mode ==> gcodePaths(LayerPathG).paths.mode ������ �Է�..//
		//path�� cartridge �Ǵ� speed ������ ���� �� ���� �� ����.//
		if (gcodePaths.paths.size() == 0)
		{
			gcodePaths.paths.push_back(pathG());
			gcodePaths.paths.back().mode = current_mode;
			gcodePaths.paths.back().extruderNr = extruderNr;
			gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z)));
		}
		else if (gcodePaths.paths.back().mode != current_mode)
		{
			gcodePaths.paths.push_back(pathG());
			gcodePaths.paths.back().mode = current_mode;
			gcodePaths.paths.back().extruderNr = extruderNr;
			gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z)));
		}

		gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(p.X), INT2MM(p.Y), INT2MM(zPos)));


		if (b_wipeTower_bedLifting && !b_lastConifg_WIPETOWER && retractionZHop[0] == 0 && retractionZHop[1] == 0)
		{
			writeBedLifting(print_speed[extruderNr], currentPosition.z);

			b_wipeTower_bedLifting = false;
		}


		//Normal E handling.
		if (lineWidth != 0)
		{
			Point diff = p - getPositionXY();
			if (isRetracted)
			{
				if (retractionZHop[extruderNr] > 0)
					fprintf(f, "G1 Z%0.3f\n", float(currentPosition.z) / 1000);

				extrusionAmount += retractionAmountPrime[extruderNr];
				fprintf(f, "G1 F%i %c%0.5f\n", retractionSpeed[extruderNr] * 60, extruderCharacter, extrusionAmount);
				currentSpeed = retractionSpeed[extruderNr];
				estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount), currentSpeed);


				//������ extrusionAmount�� ��� ��å..//
				if (extrusionAmount > 10000.0) //According to https://github.com/Ultimaker/CuraEngine/issues/14 having more then 21m of extrusion causes inaccuracies. So reset it every 10m, just to be sure.
				{
					if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
					{
						supportAmount += extrusionAmount - preAmount;
						b_add_support_filament = true;
					}
					resetExtrusionValue();
				}

				isRetracted = false;
			}

			double extrusion_per_MM_ = getExtrusionPerMM(current_mode);

			//for initial layer..//
			if (layerNr == 0)
				extrusion_per_MM_ = extrusion_per_MM_initial_layer[extruderNr];


			//set extrusion amount..//
			extrusionAmount += extrusion_per_MM_ * INT2MM(lineWidth) * vSizeMM(diff);


			fprintf(f, "G1");
		}
		else if (initRaft)
		{
			extrusionAmount = 0;
			fprintf(f, "G1");
		}
		else
			fprintf(f, "G0");


		// speed ���
		if (currentSpeed != speed)
		{
			fprintf(f, " F%i", speed * 60);
			currentSpeed = speed;
		}


		if (b_spiralize)
		{
			//spiralize�� ���, ������ x y z ������� ��..�ϴ�..//

			if (currentPosition.x != p.X || currentPosition.y != p.Y)
				fprintf(f, " X%0.3f Y%0.3f", INT2MM(p.X - extruderOffset[extruderNr].X), INT2MM(p.Y - extruderOffset[extruderNr].Y));

			if (zPos != currentPosition.z)
				fprintf(f, " Z%0.3f", INT2MM(zPos));
		}
		else
		{
			//spiralize�� �ƴ� �Ϲ����� ��� : z -> x,y ���//

			//toolchanged or no toolchanged..//

			if (zPos != currentPosition.z)
			{
				fprintf(f, " Z%0.3f", INT2MM(zPos));

				fprintf(f, "\n");

				if (lineWidth != 0 || initRaft) fprintf(f, "G1");
				else fprintf(f, "G0");

				if (currentPosition.x != p.X || currentPosition.y != p.Y)
					fprintf(f, " X%0.3f Y%0.3f", INT2MM(p.X - extruderOffset[extruderNr].X), INT2MM(p.Y - extruderOffset[extruderNr].Y));
			}
			else if (currentPosition.x != p.X || currentPosition.y != p.Y)
			{
				fprintf(f, " X%0.3f Y%0.3f", INT2MM(p.X - extruderOffset[extruderNr].X), INT2MM(p.Y - extruderOffset[extruderNr].Y));
			}
		}


		if (lineWidth != 0 && !b_add_support_filament)
		{
			if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
				supportAmount += extrusionAmount - preAmount;
		}

		// E ���
		if (lineWidth != 0)
		{
			fprintf(f, " %c%0.5f", extruderCharacter, extrusionAmount);
		}
		else if (initRaft)
		{
			fprintf(f, " %c%0.5f", extruderCharacter, extrusionAmount);
		}


		fprintf(f, "\n");

		currentPosition = Point3(p.X, p.Y, zPos);
		startPosition = currentPosition;
		estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount), speed);

		//writeCode_ToStorage(gcodeStorage, "%s%f", "G1", 53.93);
	}

	void GCodeExport::writeMove_toolchanged(Point p, int speed, int lineWidth, int layerNr, bool b_spiralize, bool b_toolchanged, bool initRaft)
	{
		if (currentPosition.x == p.X && currentPosition.y == p.Y && currentPosition.z == zPos)
			return;

		double preAmount = extrusionAmount;
		bool b_add_support_filament = false;

		//���⼭ gcode�� ������ path ������ �־���..//
		//path�� cartridge �Ǵ� speed ������ ���� �� ���� �� ����.//
		if (gcodePaths.paths.size() == 0)
		{
			gcodePaths.paths.push_back(pathG());
			gcodePaths.paths.back().mode = current_mode;
			gcodePaths.paths.back().extruderNr = extruderNr;
			gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z)));
		}
		else if (gcodePaths.paths.back().mode != current_mode)
		{
			gcodePaths.paths.push_back(pathG());
			gcodePaths.paths.back().mode = current_mode;
			gcodePaths.paths.back().extruderNr = extruderNr;
			gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z)));
		}

		gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(p.X), INT2MM(p.Y), INT2MM(zPos)));


		if (b_wipeTower_bedLifting && !b_lastConifg_WIPETOWER && retractionZHop[0] == 0 && retractionZHop[1] == 0)
		{
			writeBedLifting(print_speed[extruderNr], currentPosition.z);

			b_wipeTower_bedLifting = false;
		}


		//Normal E handling.
		if (lineWidth != 0)
		{
			Point diff = p - getPositionXY();
			if (isRetracted)
			{
				if (retractionZHop[extruderNr] > 0)
					fprintf(f, "G1 Z%0.3f\n", float(currentPosition.z) / 1000);

				extrusionAmount += retractionAmountPrime[extruderNr];
				fprintf(f, "G1 F%i %c%0.5f\n", retractionSpeed[extruderNr] * 60, extruderCharacter, extrusionAmount);
				currentSpeed = retractionSpeed[extruderNr];
				estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount), currentSpeed);


				//������ extrusionAmount�� ��� ��å..//
				if (extrusionAmount > 10000.0) //According to https://github.com/Ultimaker/CuraEngine/issues/14 having more then 21m of extrusion causes inaccuracies. So reset it every 10m, just to be sure.
				{
					if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
					{
						supportAmount += extrusionAmount - preAmount;
						b_add_support_filament = true;
					}
					resetExtrusionValue();
				}

				isRetracted = false;
			}

			double extrusion_per_MM_ = getExtrusionPerMM(current_mode);

			//for initial layer..//
			if (layerNr == 0)
				extrusion_per_MM_ = extrusion_per_MM_initial_layer[extruderNr];


			//set extrusion amount..//
			extrusionAmount += extrusion_per_MM_ * INT2MM(lineWidth) * vSizeMM(diff);


			fprintf(f, "G1");
		}
		else if (initRaft)
		{
			extrusionAmount = 0;
			fprintf(f, "G1");
		}
		else
			fprintf(f, "G0");


		// speed ���
		if (currentSpeed != speed)
		{
			fprintf(f, " F%i", speed * 60);
			currentSpeed = speed;
		}

		
		if (b_spiralize)
		{
			//spiralize�� ���, ������ x y z ������� ��..�ϴ�..//

			if (currentPosition.x != p.X || currentPosition.y != p.Y)
				fprintf(f, " X%0.3f Y%0.3f", INT2MM(p.X - extruderOffset[extruderNr].X), INT2MM(p.Y - extruderOffset[extruderNr].Y));

			if (zPos != currentPosition.z)
				fprintf(f, " Z%0.3f", INT2MM(zPos));
		}
		else
		{
			//spiralize�� �ƴ� �Ϲ����� ��� : z -> x,y ���//

			//toolchanged or no toolchanged..//
			
			if (zPos != currentPosition.z)
			{
				if (b_toolchanged)
				{
					if (currentPosition.x != p.X || currentPosition.y != p.Y)
						fprintf(f, " X%0.3f Y%0.3f", INT2MM(p.X - extruderOffset[extruderNr].X), INT2MM(p.Y - extruderOffset[extruderNr].Y));

					fprintf(f, "\n");

					if (lineWidth != 0 || initRaft) fprintf(f, "G1");
					else fprintf(f, "G0");

					fprintf(f, " Z%0.3f", INT2MM(zPos));
				}
				else
				{
					fprintf(f, " Z%0.3f", INT2MM(zPos));

					fprintf(f, "\n");

					if (lineWidth != 0 || initRaft) fprintf(f, "G1");
					else fprintf(f, "G0");

					if (currentPosition.x != p.X || currentPosition.y != p.Y)
						fprintf(f, " X%0.3f Y%0.3f", INT2MM(p.X - extruderOffset[extruderNr].X), INT2MM(p.Y - extruderOffset[extruderNr].Y));
				}

			}
			else if (currentPosition.x != p.X || currentPosition.y != p.Y)
			{
				fprintf(f, " X%0.3f Y%0.3f", INT2MM(p.X - extruderOffset[extruderNr].X), INT2MM(p.Y - extruderOffset[extruderNr].Y));
			}
		}


		if (lineWidth != 0 && !b_add_support_filament)
		{
			if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
				supportAmount += extrusionAmount - preAmount;
		}

		// E ���
		if (lineWidth != 0)
		{
			fprintf(f, " %c%0.5f", extruderCharacter, extrusionAmount);
		}
		else if (initRaft)
		{
			fprintf(f, " %c%0.5f", extruderCharacter, extrusionAmount);
		}

		
		fprintf(f, "\n");

		currentPosition = Point3(p.X, p.Y, zPos);
		startPosition = currentPosition;
		estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount), speed);

		//writeCode_ToStorage(gcodeStorage, "%s%f", "G1", 53.93);
	}

	double GCodeExport::writeMove_extrusionPathLength(Point p, int speed, int lineWidth, int layerNr, bool initRaft)
	{
		double extrusionLength = 0.0;

		if (currentPosition.x == p.X && currentPosition.y == p.Y && currentPosition.z == zPos)
			return 0.0;

		double preAmount = extrusionAmount;
		bool b_add_support_filament = false;

		if (gcodePaths.paths.size() == 0)
		{
			gcodePaths.paths.push_back(pathG());
			gcodePaths.paths.back().mode = current_mode;
			gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z)));
		}
		else if (gcodePaths.paths.back().mode != current_mode)
		{
			gcodePaths.paths.push_back(pathG());
			gcodePaths.paths.back().mode = current_mode;
			gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z)));
		}

		gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(p.X), INT2MM(p.Y), INT2MM(zPos)));


		//Normal E handling.
		if (lineWidth != 0)
		{
			Point diff = p - getPositionXY();
			if (isRetracted)
			{
				if (retractionZHop[extruderNr] > 0)
					fprintf(f, "G1 Z%0.3f\n", float(currentPosition.z) / 1000);

				extrusionAmount += retractionAmountPrime[extruderNr];
				fprintf(f, "G1 F%i %c%0.5f\n", retractionSpeed[extruderNr] * 60, extruderCharacter, extrusionAmount);
				currentSpeed = retractionSpeed[extruderNr];
				estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount), currentSpeed);
				
				if (extrusionAmount > 10000.0) //According to https://github.com/Ultimaker/CuraEngine/issues/14 having more then 21m of extrusion causes inaccuracies. So reset it every 10m, just to be sure.
				{
					if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
					{
						supportAmount += extrusionAmount - preAmount;
						b_add_support_filament = true;
					}
					resetExtrusionValue();
				}
				isRetracted = false;
			}

			double extrusion_per_MM_ = getExtrusionPerMM(current_mode);

			//for initial layer..//
			if (layerNr == 0)
				extrusion_per_MM_ = extrusion_per_MM_initial_layer[extruderNr];


			//set extrusion amount..//
			extrusionAmount += extrusion_per_MM_ * INT2MM(lineWidth) * vSizeMM(diff);

			extrusionLength = vSizeMM(diff);

			fprintf(f, "G1");
		}
		else if (initRaft)
		{
			extrusionAmount = 0;
			fprintf(f, "G1");
		}
		else
			fprintf(f, "G0");


		// speed ���
		if (currentSpeed != speed)
		{
			fprintf(f, " F%i", speed * 60);
			currentSpeed = speed;
		}

		if (currentPosition.x != p.X || currentPosition.y != p.Y)
			fprintf(f, " X%0.3f Y%0.3f", INT2MM(p.X - extruderOffset[extruderNr].X), INT2MM(p.Y - extruderOffset[extruderNr].Y));

		if (lineWidth != 0 && !b_add_support_filament)
		{
			if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
				supportAmount += extrusionAmount - preAmount;
		}

		if (zPos != currentPosition.z)
			fprintf(f, " Z%0.3f", INT2MM(zPos));

		if (lineWidth != 0)
			fprintf(f, " %c%0.5f", extruderCharacter, extrusionAmount);
		else if (initRaft)
			fprintf(f, " %c%0.5f", extruderCharacter, extrusionAmount);

		fprintf(f, "\n");

		currentPosition = Point3(p.X, p.Y, zPos);
		startPosition = currentPosition;
		estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount), speed);

		return extrusionLength;
	}

	void GCodeExport::writeMove_ToStorage(bool b_isToStorage, GCodeStorage* storage, Point p, int speed, int lineWidth, bool initRaft)
	{
		if (currentPosition.x == p.X && currentPosition.y == p.Y && currentPosition.z == zPos)
			return;

		double preAmount = extrusionAmount;
		bool b_add_support_filament = false;

		//���⼭ gcode�� ������ path ������ �־���..//
		//path�� cartridge �Ǵ� speed ������ ���� �� ���� �� ����.//
		if (gcodePaths.paths.size() == 0)
		{
			gcodePaths.paths.push_back(pathG());
			gcodePaths.paths.back().mode = current_mode;
			gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z)));
		}
		else if (gcodePaths.paths.back().mode != current_mode)
		{
			gcodePaths.paths.push_back(pathG());
			gcodePaths.paths.back().mode = current_mode;
			gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z)));
		}

		gcodePaths.paths.back().poly.push_back(FPoint3(INT2MM(p.X), INT2MM(p.Y), INT2MM(zPos)));


		if (b_wipeTower_bedLifting && !b_lastConifg_WIPETOWER && retractionZHop[0] == 0 && retractionZHop[1] == 0)
		{
			writeBedLifting_ToStorage(b_isToStorage, storage, print_speed[extruderNr], currentPosition.z);

			b_wipeTower_bedLifting = false;
		}


		//Normal E handling.
		if (lineWidth != 0)
		{
			Point diff = p - getPositionXY();
			if (isRetracted)
			{
				if (retractionZHop[extruderNr] > 0)
				{
					writeString_ToStorage(b_isToStorage, storage, "G1 Z%0.3f\n", float(currentPosition.z) / 1000);
				}

				extrusionAmount += retractionAmountPrime[extruderNr];

				writeString_ToStorage(b_isToStorage, storage, "G1 F%i %c%0.5f\n", retractionSpeed[extruderNr] * 60, extruderCharacter, extrusionAmount);

				currentSpeed = retractionSpeed[extruderNr];
				estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount), currentSpeed);

				//������ extrusionAmount�� ��� ��å..//
				if (extrusionAmount > 10000.0) //According to https://github.com/Ultimaker/CuraEngine/issues/14 having more then 21m of extrusion causes inaccuracies. So reset it every 10m, just to be sure.
				{
					if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
					{
						supportAmount += extrusionAmount - preAmount;
						b_add_support_filament = true;
					}
					resetExtrusionValue();
				}

				isRetracted = false;
			}

			//set extrusion amount..//
			extrusionAmount += extrusion_per_MM_overall[extruderNr] * INT2MM(lineWidth) * vSizeMM(diff);

			writeString_ToStorage(b_isToStorage, storage, "G1");

		}
		else if (initRaft)
		{
			extrusionAmount = 0;

			writeString_ToStorage(b_isToStorage, storage, "G1");
		}
		else
		{
			writeString_ToStorage(b_isToStorage, storage, "G0");
		}

		// speed ���
		if (currentSpeed != speed)
		{
			writeString_ToStorage(b_isToStorage, storage, " F%i", speed * 60);

			currentSpeed = speed;
		}

		if (currentPosition.x != p.X || currentPosition.y != p.Y)
		{
			writeString_ToStorage(b_isToStorage, storage, " X%0.3f Y%0.3f", INT2MM(p.X - extruderOffset[extruderNr].X), INT2MM(p.Y - extruderOffset[extruderNr].Y));
		}



		if (lineWidth != 0 && !b_add_support_filament)
		{
			if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
				supportAmount += extrusionAmount - preAmount;
		}


		if (zPos != currentPosition.z)
		{
			writeString_ToStorage(b_isToStorage, storage, " Z%0.3f", INT2MM(zPos));

		}


		// E ���
		if (lineWidth != 0)
		{
			writeString_ToStorage(b_isToStorage, storage, " %c%0.5f", extruderCharacter, extrusionAmount);
		}
		else if (initRaft)
		{
			writeString_ToStorage(b_isToStorage, storage, " %c%0.5f", extruderCharacter, extrusionAmount);
		}


		writeString_ToStorage(b_isToStorage, storage, "\n");

		currentPosition = Point3(p.X, p.Y, zPos);
		startPosition = currentPosition;
		estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount), speed);
		//for storage//
		if (b_isToStorage)
		{
			estimateCalculator_forStorage.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount), speed);

			sectionPrintTime_Storage += estimateCalculator_forStorage.calculate();
		}
	}

	void GCodeExport::writeMove_timeCalculate(Point p, int speed, int lineWidth, int layerNr, bool initRaft)
	{
		if (currentPosition_timeCalculate.x == p.X && currentPosition_timeCalculate.y == p.Y && currentPosition_timeCalculate.z == zPos)
			return;

		double preAmount_timeCalculate = extrusionAmount_timeCalculate;
		bool b_add_support_filament_timeCalculate = false;

		//���⼭ gcode�� ������ path ������ �־���..//
		//path�� cartridge �Ǵ� speed ������ ���� �� ���� �� ����.//
		if (gcodePaths_timeCalculate.paths.size() == 0)
		{
			gcodePaths_timeCalculate.paths.push_back(pathG());
			gcodePaths_timeCalculate.paths.back().mode = current_mode;
			gcodePaths_timeCalculate.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition_timeCalculate.x), INT2MM(currentPosition_timeCalculate.y), INT2MM(currentPosition_timeCalculate.z)));
		}
		else if (gcodePaths_timeCalculate.paths.back().mode != current_mode)
		{
			gcodePaths_timeCalculate.paths.push_back(pathG());
			gcodePaths_timeCalculate.paths.back().mode = current_mode;
			gcodePaths_timeCalculate.paths.back().poly.push_back(FPoint3(INT2MM(currentPosition_timeCalculate.x), INT2MM(currentPosition_timeCalculate.y), INT2MM(currentPosition_timeCalculate.z)));
		}

		gcodePaths_timeCalculate.paths.back().poly.push_back(FPoint3(INT2MM(p.X), INT2MM(p.Y), INT2MM(zPos)));

		//Normal E handling.
		if (lineWidth != 0)
		{
			Point diff = p - getPositionXY_timeCalculate();
			if (isRetracted_timeCalculate)
			{
				extrusionAmount_timeCalculate += retractionAmountPrime[extruderNr];

				currentSpeed_timeCalculate = retractionSpeed[extruderNr];
				estimateCalculator_timeCalculate.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition_timeCalculate.x), INT2MM(currentPosition_timeCalculate.y), INT2MM(currentPosition_timeCalculate.z), extrusionAmount_timeCalculate), currentSpeed_timeCalculate);


				//������ extrusionAmount�� ��� ��å..//
				if (extrusionAmount_timeCalculate > 10000.0) //According to https://github.com/Ultimaker/CuraEngine/issues/14 having more then 21m of extrusion causes inaccuracies. So reset it every 10m, just to be sure.
				{
					if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
					{
						supportAmount_timeCalculate += extrusionAmount_timeCalculate - preAmount_timeCalculate;
						b_add_support_filament_timeCalculate = true;
					}
					resetExtrusionValue_timeCalculate();
				}

				isRetracted_timeCalculate = false;
			}


			double extrusion_per_MM_ = getExtrusionPerMM(current_mode);

			//for initial layer..//
			if (layerNr == 0)
				extrusion_per_MM_ = extrusion_per_MM_initial_layer[extruderNr];


			//set extrusion amount..//
			extrusionAmount_timeCalculate += extrusion_per_MM_ * INT2MM(lineWidth) * vSizeMM(diff);
		}
		else if (initRaft)
		{
			extrusionAmount_timeCalculate = 0;
		}

		// speed ���
		if (currentSpeed_timeCalculate != speed)
		{
			currentSpeed_timeCalculate = speed;
		}

		if (lineWidth != 0 && !b_add_support_filament_timeCalculate)
		{
			if (current_mode == PathMode::SUPPORT_MAIN || current_mode == PathMode::SUPPORT_INTERFACE_ROOF || current_mode == PathMode::SUPPORT_INTERFACE_FLOOR)
				supportAmount_timeCalculate += extrusionAmount_timeCalculate - preAmount_timeCalculate;
		}

		currentPosition_timeCalculate = Point3(p.X, p.Y, zPos);
		startPosition_timeCalculate = currentPosition_timeCalculate;
		estimateCalculator_timeCalculate.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition_timeCalculate.x), INT2MM(currentPosition_timeCalculate.y), INT2MM(currentPosition_timeCalculate.z), extrusionAmount_timeCalculate), speed);

	}

	void GCodeExport::writeRetraction(bool force)
	{
		if (retractionAmount[extruderNr] > 0 && !isRetracted && (extrusionAmountAtPreviousRetraction + minimalExtrusionBeforeRetraction[extruderNr] < extrusionAmount || force))
		{
			fprintf(f, "G1 F%i %c%0.5f\n", retractionSpeed[extruderNr] * 60, extruderCharacter, extrusionAmount - retractionAmount[extruderNr]);
			currentSpeed = retractionSpeed[extruderNr];
			estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount - retractionAmount[extruderNr]), currentSpeed);

			if (b_lastConifg_WIPETOWER && retractionZHop[0] == 0 && retractionZHop[1] == 0)
			{
				writeBedLifting(print_speed[extruderNr], currentPosition.z + layerHeight);

				b_wipeTower_bedLifting = true;
			}

			if (retractionZHop[extruderNr] > 0)
			{ 
				fprintf(f, "G1 Z%0.3f\n", INT2MM(currentPosition.z + retractionZHop[extruderNr]));
			}

			extrusionAmountAtPreviousRetraction = extrusionAmount;

			isRetracted = true;
		}
	}

	void GCodeExport::switchExtruder(int newExtruder)
	{
		if (extruderNr == newExtruder)
			return;
		
		resetExtrusionValue();	//<-- extrusionAmount == 0���� �ʱ�ȭ..

		int oldExtruder = extruderNr;

		///////////////////////////////////////////////////////////////////////////////////
		//engine extruder�� �Է�..//
		extruderNr = newExtruder;
		///////////////////////////////////////////////////////////////////////////////////


		//������ engine�� extruderNr setting�Ͽ�����, b_notExtruderSet flag�� false�� ��..//
		if (oldExtruder == 100)	b_notExtruderSet = true;
		else b_notExtruderSet = false;


		//b_notExtruderSet�� ó���� T100���� setting�Ǿ��� �����..//
		//������ �� extrudeNr�� setting�Ǳ� ������ start �κп��� �ѹ��� �����.//
		if (b_notExtruderSet)
		{
			//T100�� ���, currentSpeed�� �ϴ� 0 index�� �Է�..//
			currentSpeed = toolchangeRetractionSpeed[0];
		}
		else
		{
			fprintf(f, "G1 F%i %c%0.5f\n", toolchangeRetractionSpeed[extruderNr] * 60, extruderCharacter, extrusionAmount - toolchangeRetractionAmount[extruderNr]);

			if (retractionZHop[extruderNr] > 0)
				fprintf(f, "G1 Z%0.3f\n", INT2MM(currentPosition.z + retractionZHop[extruderNr]));

			currentSpeed = toolchangeRetractionSpeed[extruderNr];
		}


		isRetracted = true;

		fprintf(f, "\n");

		//writeCode(preSwitchExtruderCode[extruderNr].c_str());


		//bed lifting///////////////////////////////
		toolchangeZlifting = currentPosition.z + toolchangeLoweringBed;
		writeBedLifting(print_speed[extruderNr], toolchangeZlifting);

		currentPosition.z = toolchangeZlifting;

		
		fprintf(f, "T%i\n", extruderNr);


		fprintf(f, "\n");

		//writeCode(postSwitchExtruderCode[extruderNr].c_str());


		//nozzle change�� �ҿ�ð� �����ʿ�..//
		if (preheat_enabled[extruderNr] && standby_temperature_enabled[extruderNr])
		{
			//standby temperature + preheat ���۽�..//
			this->totalPrintTime += 30;
		}
		else if (standby_temperature_enabled[extruderNr])
		{
			//standby temperature ����� ����� ���.. 85�ʷ� �ϴ�..//
			this->totalPrintTime += 85;
		}
		else
		{
			this->totalPrintTime += 11;
		}

	}

	void GCodeExport::writeCode(const char* str)
	{
		fprintf(f, "%s", str);
	}

	//void GCodeExport::writeFanCommand(double speed)
	//{
	//	if (currentFanSpeed == speed)
	//		return;
	//
	//	if (speed > 0)
	//	{
	//		fprintf(f, "M106 S%d\n", int(speed * 255 / 100));
	//	}
	//	else
	//	{
	//		fprintf(f, "M107\n");
	//	}
	//
	//	currentFanSpeed = speed;
	//}

	void GCodeExport::writeFanCommand(int cartridgeIndex, double speed)
	{
		if (currentFanSpeed[cartridgeIndex] == speed)
			return;
		if (speed > 0)
		{
			fprintf(f, "M106 T%d S%d\n", cartridgeIndex, int(speed * 255 / 100));
		}
		else
		{
			fprintf(f, "M107 T%d\n", cartridgeIndex);
		}

		currentFanSpeed[cartridgeIndex] = speed;
	}

	void GCodeExport::writeExtrusion(int speed, double amount)
	{
		if (speed == 0 || amount == 0) return;

		fprintf(f, "G1 F%i %c%0.5f\n", (int)speed * 60, extruderCharacter, amount);
	}

	void GCodeExport::writeExtrusionReset(double amount)
	{
		fprintf(f, "G92 %c%0.5f\n", extruderCharacter, amount);
	}

	void GCodeExport::writeBedLifting(double speed, double position_z)
	{
		fprintf(f, "G0 F%i Z%0.3f\n", (int)speed * 60, (double)position_z / 1000);
	}

	int GCodeExport::getFileSize()
	{
		return ftell(f);
	}
	void GCodeExport::tellFileSize()
	{
		float fsize = ftell(f);
		if (fsize > 1024 * 1024) {
			fsize /= 1024.0*1024.0;
			printf("Wrote %5.1f MB.\n", fsize);
		}
		if (fsize > 1024) {
			fsize /= 1024.0;
			printf("Wrote %5.1f kilobytes.\n", fsize);
		}
	}

	void GCodeExport::finalize()//int maxObjectHeight, int moveSpeed, const char* endCode)
	{
		//engine���� �����ϱ�� �� - swyang
		//writeFanCommand(0);
		//writeRetraction();	
		//setZ(maxObjectHeight + 5000);
		//writeMove(getPositionXY(), moveSpeed, 0);
		fclose(f);

		if (int(getTotalPrintTime() / 3600) > 0)
			printf("Print time: %d hours %d mins %d sec\n", int(int(getTotalPrintTime()) / 3600), int((int(getTotalPrintTime()) % 3600) / 60), int(int(getTotalPrintTime()) % 60));
		else printf("Print time: %d mins %d sec\n", int((int(getTotalPrintTime()) % 3600) / 60), int(int(getTotalPrintTime()) % 60));

		float filamentDensity = 0.00125;
		float filamentMass = (3.141592*1.75*1.75 / 4)*getTotalFilamentUsed(0)*filamentDensity; //volume*length*density;
		float filamentMass_support = (3.141592*1.75*1.75 / 4)*supportAmount*filamentDensity; //volume*length*density;

		printf("Filament: %.2f meter / %.2f g\n", getTotalFilamentUsed(0) / 1000.0, filamentMass);

		printf("support Filament: %.2f meter / %.2f g\n", supportAmount / 1000.0, filamentMass_support);
	}


	void GCodeExport::clear()
	{
		currentPosition.x = 0;
		currentPosition.y = 0;
		currentPosition.z = 0;
		startPosition.x = INT32_MIN;
		startPosition.y = INT32_MIN;
		startPosition.z = 0;

		supportAmount = 0;
		extrusionAmount = 0;
		extrusionAmountAtPreviousRetraction = -10000;
		//extruderSwitchRetraction = 14.5;
		extruderNr = 0;

		totalPrintTime = 0.0;
		for (unsigned int e = 0; e < MAX_EXTRUDERS; e++)
		{
			currentFanSpeed[e] = -1;
			totalFilament[e] = 0.0;

			extrusion_per_MM_overall[e] = 0.0;
			extrusion_per_MM_initial_layer[e] = 0.0;
			extrusion_per_MM_infill[e] = 0.0;
			extrusion_per_MM_outer_wall[e] = 0.0;
			extrusion_per_MM_inner_wall[e] = 0.0;
			extrusion_per_MM_top_bottom[e] = 0.0;
		}

		extrusion_per_MM_support_main = 0.0;
		extrusion_per_MM_support_interface_roof = 0.0;
		extrusion_per_MM_support_interface_floor = 0.0;
		//extrusion_per_MM_skirt = 0.0;
		//extrusion_per_MM_brim = 0.0;
		extrusion_per_MM_wipe_tower = 0.0;


		currentSpeed = 0;
		isRetracted = false;
		isPreRetracted = false;
		setFlavor();
		memset(extruderOffset, 0, sizeof(extruderOffset));
		f = stdout;

		gcodePaths.paths.clear();
		estimateCalculator.clear();


		//////////
		currentPosition_timeCalculate = Point3(0, 0, 0);
		startPosition_timeCalculate = Point3(INT32_MIN, INT32_MIN, 0);

		supportAmount_timeCalculate = 0;
		extrusionAmount_timeCalculate = 0;
		
		totalPrintTime_timeCalculate = 0.0;


		currentSpeed_timeCalculate = 0;
		isRetracted_timeCalculate = false;
		
		estimateCalculator_timeCalculate.clear();

		preheat_OR_enabled = false;
		platform_NONE = false;
		b_initialMoving = true;
		b_notExtruderSet = false;
		limitBedlifting = false;
		b_wipeTower_bedLifting = false;
		b_lastConifg_WIPETOWER = false;

		estimateCalculator.updateInitValue();
		estimateCalculator_timeCalculate.updateInitValue();
	}
	void GCodeExport::setConfig(vector<ConfigSettings> _configs)
	{
		int cartridgeCount = _configs.size();
		if (cartridgeCount == 0) return;

		clear();
		setFlavor();
		b_multiExtruder = bool(cartridgeCount > 1);
		platformAdhesion = _configs.front().platform_adhesion;
		platform_NONE = bool(_configs.front().platform_adhesion == Generals::PlatformAdhesion::NoneAdhesion);

		layerHeight = _configs.front().layer_height;


		raftTemperatureControl = _configs.front().raft_temperature_control;
		raftSurfaceLastTemperature = _configs.front().raft_surface_last_temperature;

		toolchangeLoweringBed = _configs.front().toolchange_lowering_bed;
		cartridgeCountTotal = cartridgeCount;
		b_spiralize = _configs.front().spiralize_mode;

		raftBaseSpeed = _configs.front().raft_base_speed;
		raftBaseLineWidth = _configs.front().raft_base_line_width;
		raftBaseFanControlEnabled = Profile::machineProfile.raft_base_fan_control_enabled.value;

		switch (_configs.front().platform_adhesion)
		{
			case Generals::PlatformAdhesion::Raft:
				extruderNr = _configs.front().adhesion_cartridge_index;
				b_notExtruderSet = false;
				break;
				// RAFT�� �ƴ� ���, BRIM, SKIRT, NONE�� �����ϰ� T100����..//
			default:
				extruderNr = 100;
				b_notExtruderSet = true;
				break;
		}

		/////////////////////////////////////////////////////
		//vector initial setting.. --> ������ �̷� �溡���� �����..counter cartridge��� ��������..//
		temperature_layer_setting_enabled.clear();
		temperature_layer_setting_enabled.resize(_configs.size());
		/////////////////////////////////////////////////////


		for (int n = 0; n < cartridgeCount; ++n)
		{
			travel_speed[n] = _configs[n].travel_speed;
			print_speed[n] = _configs[n].print_speed;
			initial_layer_extrusion_width[n] = _configs[n].initial_layer_extrusion_width;
			initial_layer_speed[n] = _configs[n].initial_layer_speed;


			//tool_change
			toolchangeRetractionAmount[n] = INT2MM(_configs[n].toolchange_retraction_amount);
			toolchangeRetractionSpeed[n] = _configs[n].toolchange_retraction_speed;
			toolchangeExtraRestartAmount[n] = INT2MM(_configs[n].toolchange_extra_restart_amount);
			toolchangeExtraRestartSpeed[n] = _configs[n].toolchange_extra_restart_speed;
			//switch extruder code
			preSwitchExtruderCode[n] = _configs[n].pre_switch_extruder_code;
			postSwitchExtruderCode[n] = _configs[n].post_switch_extruder_code;
			//retraction settings
			retractionAmount[n] = INT2MM(_configs[n].retraction_amount);
			retractionSpeed[n] = _configs[n].retraction_speed;
			minimalExtrusionBeforeRetraction[n] = INT2MM(_configs[n].minimal_extrusion_before_retraction);
			retractionZHop[n] = _configs[n].retraction_z_hop;
			retractionAmountPrime[n] = INT2MM(_configs[n].retraction_amount_prime);
			//common profile setting//
			//nozzle_layer0_temperature_enabled[n] = _configs[n].layer0temperatureEnabled;
			//nozzle_layer0_temperature[n] = _configs[n].layer0temperature;
			firstToolChanged[n] = false;
			nozzle_print_temperature[n] = _configs[n].print_temperature;

			standby_temperature_enabled[n] = (b_multiExtruder? _configs[n].standby_temperature_enabled:false);
			nozzle_operating_standby_temperature[n] = _configs[n].operating_standby_temperature;
			nozzle_initial_standby_temperature[n] = _configs[n].initial_standby_temperature;

			if (_configs[n].preheat_enabled)
				preheat_OR_enabled = true;

			preheat_enabled[n] = (b_multiExtruder? _configs[n].preheat_enabled:false);
			preheat_threshold_time[n] = _configs[n].preheat_threshold_time;
			preheat_temperature_falling_rate[n] = _configs[n].preheat_temperature_falling_rate * 0.001;
			preheat_temperature_rising_rate[n] = _configs[n].preheat_temperature_rising_rate * 0.001;

			temperature_layer_setting_enabled.at(n) = _configs[n].temperature_layer_setting_enabled;
		}
	}

	void GCodeExport::resetExtrusionValue_ToString(bool b_isToStorage, GCodeStorage *storage)
	{
		if (extrusionAmount != 0.0)
		{
			if (b_isToStorage)
			{
				writeString_ToStorage(b_isToStorage, storage, "G92 %c0\n", extruderCharacter);
			}
			else
			{
				fprintf(f, "G92 %c0\n", extruderCharacter);
			}

			totalFilament[extruderNr] += extrusionAmount;
			extrusionAmountAtPreviousRetraction -= extrusionAmount;
			extrusionAmount = 0.0;
		}
	}

	void GCodeExport::switchExtruder_ToStorage(bool b_isToStorage, GCodeStorage *storage, int newExtruder, bool b_firstNONEstatus)
	{
		if (extruderNr == newExtruder)
			return;

		resetExtrusionValue_ToString(b_isToStorage, storage);	//<-- extrusionAmount == 0���� �ʱ�ȭ..

		if (!b_firstNONEstatus)
		{
			writeString_ToStorage(b_isToStorage, storage, "G1 F%i %c%0.5f\n", toolchangeRetractionSpeed[extruderNr] * 60, extruderCharacter, extrusionAmount - toolchangeRetractionAmount[extruderNr]);
		}

		currentSpeed = toolchangeRetractionSpeed[extruderNr];

		//extrudeNr�� 100�̶�� ���� platformAdehesion�� completeNONE�� ��� �ӽ÷� �־��� extruderNr��. rodinP����..//swyang
		//�Ʒ����� �� extrudeNr�� setting�Ǳ� ������ start �κп��� �ѹ��� �����.//
		if (!b_firstNONEstatus)
		{
			if (retractionZHop[extruderNr] > 0)
			{
				writeString_ToStorage(b_isToStorage, storage, "G1 Z%0.3f\n", INT2MM(currentPosition.z + retractionZHop[extruderNr]));
			}
		}

		extruderNr = newExtruder;

		isRetracted = true;

		writeNewLine_ToStorage(b_isToStorage, storage);


		//writeCode(preSwitchExtruderCode[extruderNr].c_str());

		//bed lifting//////////////////////swyang
		toolchangeZlifting = currentPosition.z + toolchangeLoweringBed;

		writeBedLifting_ToStorage(b_isToStorage, storage, print_speed[extruderNr], toolchangeZlifting);

		writeString_ToStorage(b_isToStorage, storage, "T%i\n", extruderNr);

		writeNewLine_ToStorage(b_isToStorage, storage);

		//writeCode(postSwitchExtruderCode[extruderNr].c_str());



		//nozzle change�� �ҿ�ð� �����ʿ�..//
		if (standby_temperature_enabled[extruderNr])
		{
			//standby temperature ����� ����� ���.. 85�ʷ� �ϴ�..
			this->totalPrintTime += 85;
		}
		else
		{
			this->totalPrintTime += 8.5;
		}

	}

	void GCodeExport::writeRetraction_ToStorage(bool b_isToStorage, GCodeStorage *storage, bool force)
	{
		if (retractionAmount[extruderNr] > 0 && !isRetracted && (extrusionAmountAtPreviousRetraction + minimalExtrusionBeforeRetraction[extruderNr] < extrusionAmount || force))
		{

			writeString_ToStorage(b_isToStorage, storage, "G1 F%i %c%0.5f\n", retractionSpeed[extruderNr] * 60, extruderCharacter, extrusionAmount - retractionAmount[extruderNr]);

			currentSpeed = retractionSpeed[extruderNr];
			estimateCalculator.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount - retractionAmount[extruderNr]), currentSpeed);

			if (b_isToStorage)
			{
				estimateCalculator_forStorage.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition.x), INT2MM(currentPosition.y), INT2MM(currentPosition.z), extrusionAmount - retractionAmount[extruderNr]), currentSpeed);
				sectionPrintTime_Storage += estimateCalculator_forStorage.calculate();
			}

			if (b_lastConifg_WIPETOWER && retractionZHop[0] == 0 && retractionZHop[1] == 0)
			{
				writeBedLifting_ToStorage(b_isToStorage, storage, print_speed[extruderNr], currentPosition.z + layerHeight);

				b_wipeTower_bedLifting = true;
			}
			

			if (retractionZHop[extruderNr] > 0)
			{
				writeString_ToStorage(b_isToStorage, storage, "G1 Z%0.3f\n", INT2MM(currentPosition.z + retractionZHop[extruderNr]));
			}

			extrusionAmountAtPreviousRetraction = extrusionAmount;

			isRetracted = true;
		}
	}

	void GCodeExport::writeRetraction_timeCalculate(bool force)
	{
		if (retractionAmount[extruderNr] > 0 && !isRetracted_timeCalculate && (extrusionAmountAtPreviousRetraction_timeCalculate + minimalExtrusionBeforeRetraction[extruderNr] < extrusionAmount_timeCalculate || force))
		{
			currentSpeed_timeCalculate = retractionSpeed[extruderNr];
			estimateCalculator_timeCalculate.plan(TimeEstimateCalculator::Position(INT2MM(currentPosition_timeCalculate.x), INT2MM(currentPosition_timeCalculate.y), INT2MM(currentPosition_timeCalculate.z), extrusionAmount_timeCalculate - retractionAmount[extruderNr]), currentSpeed_timeCalculate);

			extrusionAmountAtPreviousRetraction_timeCalculate = extrusionAmount_timeCalculate;

			isRetracted_timeCalculate = true;
		}
	}

	void GCodeExport::writeNewLine_ToStorage(bool b_isToStorage, GCodeStorage* storage)
	{
		if (b_isToStorage)
		{
			storage->gcodeStrings_original.append("\n");
		}
		else
		{
			fprintf(f, "\n");
		}
	}

	void GCodeExport::writeComment_ToStorage(bool b_isToStorage, GCodeStorage* storage, const char* format, ...)
	{
		char buf[1024] = { 0, };

		va_list lpStart;
		va_start(lpStart, format);

		if (b_isToStorage)
		{
			vsprintf(buf, format, lpStart);

			std::string outStr = buf;

			storage->gcodeStrings_original.append(";");
			storage->gcodeStrings_original.append(outStr);
			storage->gcodeStrings_original.append("\n");

		}
		else
		{
			fprintf(f, ";");
			vfprintf(f, format, lpStart);
			fprintf(f, "\n");
		}

		va_end(lpStart);
	}

	void GCodeExport::writeString_ToStorage(bool b_isToStorage, GCodeStorage* storage, const char* format, ...)
	{
		char buf[1024] = { 0, };

		va_list lpStart;
		va_start(lpStart, format);

		if (b_isToStorage)
		{
			vsprintf(buf, format, lpStart);

			std::string outStr = buf;

			storage->gcodeStrings_original.append(outStr);
		}
		else
		{
			vfprintf(f, format, lpStart);
		}

		va_end(lpStart);
	}

	void GCodeExport::writeCode_ToStorage(bool b_isToStorage, GCodeStorage* storage, const char* format, ...)
	{
		char buf[1024] = { 0, };

		va_list lpStart;
		va_start(lpStart, format);

		if (b_isToStorage)
		{
			vsprintf(buf, format, lpStart);

			std::string outStr = buf;

			storage->gcodeStrings_original.append(outStr);
			storage->gcodeStrings_original.append("\n");
		}
		else
		{
			vfprintf(f, format, lpStart);
			fprintf(f, "\n");
		}

		va_end(lpStart);
	}

	void GCodeExport::writeBedLifting_ToStorage(bool b_isToStorage, GCodeStorage* storage, double speed, double position_z)
	{
		if (b_isToStorage)
		{
			writeCode_ToStorage(b_isToStorage, storage, "G0 F%i Z%0.3f", (int)speed * 60, (double)position_z / 1000);
		}
		else
		{
			fprintf(f, "G0 F%i Z%0.3f\n", (int)speed * 60, (double)position_z / 1000);
		}
	}

	void GCodeExport::writeExtrusionReset_ToStorage(bool b_isToStorage, GCodeStorage* storage, double amount)
	{
		if (b_isToStorage)
		{
			writeCode_ToStorage(b_isToStorage, storage, "G92 %c%0.5f", extruderCharacter, amount);
		}
		else
		{
			fprintf(f, "G92 %c%0.5f", extruderCharacter, amount);
		}
	}

	void GCodeExport::writeExtrusion_ToStorage(bool b_isToStorage, GCodeStorage* storage, int speed, double amount)
	{
		if (speed == 0 || amount == 0) return;

		if (b_isToStorage)
		{
			writeString_ToStorage(b_isToStorage, storage, "G1 F%i %c%0.5f\n", (int)speed * 60, extruderCharacter, amount);
		}
		else
		{
			fprintf(f, "G1 F%i %c%0.5f\n", (int)speed * 60, extruderCharacter, amount);
		}
	}

	void GCodeExport::writeFanCommand_ToStorage(bool b_isToStorage, GCodeStorage* storage, int cartridgeIndex, double speed)
	{
		if (currentFanSpeed[cartridgeIndex] == speed)
			return;

		if (speed > 0)
		{
			writeString_ToStorage(b_isToStorage, storage, "M106 T%d S%d\n", cartridgeIndex, int(speed * 255 / 100));
		}
		else
		{
			writeString_ToStorage(b_isToStorage, storage, "M107 T%d\n", cartridgeIndex);
		}

		currentFanSpeed[cartridgeIndex] = speed;
	}

	void GCodeExport::writeDelay_ToStorage(bool b_isToStorage, GCodeStorage* storage, double timeAmount)
	{
		writeString_ToStorage(b_isToStorage, storage, "G4 P%d\n", int(timeAmount * 1000));

		totalPrintTime += timeAmount;
	}

	void GCodeExport::reset_pathPrintTime_pathEndPrintTime()
	{
		for (int i = 0; i < pathPrintTime_accumulated.size(); i++)
		{
			pathPrintTime_accumulated.at(i) = 0.0;
		}

		for (int i = 0; i < pathEndPrintTime.size(); i++)
		{
			pathEndPrintTime.at(i) = 0.0;
		}
	}

	double GCodeExport::getExtrusionPerMM(PathMode _current_mode)
	{
		double extrusion_per_MM_ = 0.0;

		switch (_current_mode)
		{
		case PathMode::SKIRT:
			extrusion_per_MM_ = extrusion_per_MM_initial_layer[extruderNr];
			break;
		case PathMode::BRIM:
			extrusion_per_MM_ = extrusion_per_MM_initial_layer[extruderNr];
			break;
		case PathMode::SKIN:
			extrusion_per_MM_ = extrusion_per_MM_top_bottom[extruderNr];
			break;
		case PathMode::WALL_INNER:
			extrusion_per_MM_ = extrusion_per_MM_inner_wall[extruderNr];
			break;
		case PathMode::WALL_OUTER:
			extrusion_per_MM_ = extrusion_per_MM_outer_wall[extruderNr];
			break;
		case PathMode::FILL:
			extrusion_per_MM_ = extrusion_per_MM_infill[extruderNr];
			break;
		case PathMode::SUPPORT_MAIN:
			extrusion_per_MM_ = extrusion_per_MM_support_main;
			break;
		case PathMode::SUPPORT_INTERFACE_ROOF:
			extrusion_per_MM_ = extrusion_per_MM_support_interface_roof;
			break;
		case PathMode::SUPPORT_INTERFACE_FLOOR:
			extrusion_per_MM_ = extrusion_per_MM_support_interface_floor;
			break;
		case PathMode::WIPE_TOWER:
			extrusion_per_MM_ = extrusion_per_MM_wipe_tower;
			break;
		default:
			extrusion_per_MM_ = extrusion_per_MM_overall[extruderNr];
			break;
		}

		return extrusion_per_MM_;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GCodePath* GCodePlanner::getLatestPathWithConfig(GCodePathConfig* config)
	{
		if (paths.size() > 0 && paths[paths.size() - 1].config == config && !paths[paths.size() - 1].done)
			return &paths[paths.size() - 1];
		paths.push_back(GCodePath());
		GCodePath* ret = &paths[paths.size() - 1];
		ret->retract = false;
		ret->config = config;
		ret->extruder = currentExtruder;
		ret->done = false;
		return ret;
	}
	GCodePath* GCodePlanner::getLatestPathWithConfigAndUserExtruder(GCodePathConfig* config, int userExtruder)
	{
		if (paths.size() > 0 && paths[paths.size() - 1].config == config && !paths[paths.size() - 1].done)
			return &paths[paths.size() - 1];
		paths.push_back(GCodePath());
		GCodePath* ret = &paths[paths.size() - 1];
		ret->retract = false;
		ret->config = config;
		ret->extruder = userExtruder;
		ret->done = false;
		return ret;
	}

	void GCodePlanner::forceNewPathStart()
	{
		if (paths.size() > 0)
			paths[paths.size() - 1].done = true;
	}

	GCodePlanner::GCodePlanner(GCodeExport& gcode, std::vector<int> configsTravelSpeed, std::vector<int> configsRetractionMinimalDistance, bool spiralize)
		: gcode(gcode)
	{
		prevPosition = gcode.getPositionXY();
		lastPosition = gcode.getPositionXY();
		comb = nullptr;
		extrudeSpeedFactor = 100;
		travelSpeedFactor = 100;
		extraTime = 0.0;
		totalPrintTime = 0.0;
		forceRetraction = false;
		alwaysRetract = false;
		currentExtruder = gcode.getExtruderNr();
		this->retractionMinimalDistance = configsRetractionMinimalDistance;
		retractionWithComb = false;

		//travelConfig(travelSpeed, 0, "travel", spiralize)
		pathconfigs_travel.clear();
		pathconfigs_travel.resize(configsTravelSpeed.size());
		for (int i = 0; i < configsTravelSpeed.size(); i++)
		{
			pathconfigs_travel.at(i).speed = configsTravelSpeed.at(i);
			pathconfigs_travel.at(i).lineWidth = 0;
			pathconfigs_travel.at(i).name = "travel";
			pathconfigs_travel.at(i).spiralize = spiralize;
		}
	}

	GCodePlanner::~GCodePlanner()
	{
		if (comb)
			delete comb;
	}

	void GCodePlanner::addTravel(Point p)
	{
		GCodePath* path = getLatestPathWithConfig(&pathconfigs_travel[currentExtruder]);
		if (retractionWithComb && comb != nullptr)
		{
			vector<Point> pointList;
			if (comb->calc(lastPosition, p, pointList))
			{
				for (unsigned int n = 0; n < pointList.size(); n++)
				{
					path->points.push_back(pointList[n]);
				}
			}
			path->retract = true;
		}
		else if (forceRetraction)
		{
			if (!shorterThen(lastPosition - p, retractionMinimalDistance[currentExtruder]))
			{
				path->retract = true;
			}
			forceRetraction = false;
		}
		else if (comb != nullptr)
		{
			vector<Point> pointList;
			if (comb->calc(lastPosition, p, pointList))
			{
				for (unsigned int n = 0; n<pointList.size(); n++)
				{
					path->points.push_back(pointList[n]);
				}
			}
			else{
				if (!shorterThen(lastPosition - p, retractionMinimalDistance[currentExtruder]))
					path->retract = true;
			}
		}
		else if (alwaysRetract)
		{
			if (!shorterThen(lastPosition - p, retractionMinimalDistance[currentExtruder]))
				path->retract = true;
		}

		path->points.push_back(p);
		prevPosition = lastPosition;
		lastPosition = p;
	}

	void GCodePlanner::addTravelWithUserExtruder(Point p, int userExtruder)
	{
		GCodePath* path = getLatestPathWithConfigAndUserExtruder(&pathconfigs_travel[currentExtruder], userExtruder);
		if (retractionWithComb && comb != nullptr)
		{
			vector<Point> pointList;
			if (comb->calc(lastPosition, p, pointList))
			{
				for (unsigned int n = 0; n < pointList.size(); n++)
				{
					path->points.push_back(pointList[n]);
				}
			}
			path->retract = true;
		}
		else if (forceRetraction)
		{
			if (!shorterThen(lastPosition - p, retractionMinimalDistance[currentExtruder]))
			{
				path->retract = true;
			}
			forceRetraction = false;
		}
		else if (comb != nullptr)
		{
			vector<Point> pointList;
			if (comb->calc(lastPosition, p, pointList))
			{
				for (unsigned int n = 0; n<pointList.size(); n++)
				{
					path->points.push_back(pointList[n]);
				}
			}
			else{
				if (!shorterThen(lastPosition - p, retractionMinimalDistance[currentExtruder]))
					path->retract = true;
			}
		}
		else if (alwaysRetract)
		{
			if (!shorterThen(lastPosition - p, retractionMinimalDistance[currentExtruder]))
				path->retract = true;
		}

		path->points.push_back(p);
		prevPosition = lastPosition;
		lastPosition = p;
	}

	void GCodePlanner::addExtrusionMove(Point p, GCodePathConfig* config)
	{
		getLatestPathWithConfig(config)->points.push_back(p);
		prevPosition = lastPosition;
		lastPosition = p;
	}
	void GCodePlanner::addExtrusionMoveWithUserExturder(Point p, GCodePathConfig* config, int userExtruder)
	{
		getLatestPathWithConfigAndUserExtruder(config, userExtruder)->points.push_back(p);
		prevPosition = lastPosition;
		lastPosition = p;
	}

	void GCodePlanner::moveInsideCombBoundary(int distance)
	{
		if (!comb || comb->inside(lastPosition)) return;
		Point p = lastPosition;
		if (comb->moveInside(&p, distance))
		{
			//Move inside again, so we move out of tight 90deg corners
			comb->moveInside(&p, distance);
			if (comb->inside(p))
			{
				addTravel(p);
				//Make sure the that any retraction happens after this move, not before it by starting a new move path.
				forceNewPathStart();
			}
		}
	}

	void GCodePlanner::addPolygon(PolygonRef polygon, int startIdx, GCodePathConfig* config)
	{
		Point p0 = polygon[startIdx];
		addTravel(p0);

		for (unsigned int i = 1; i<polygon.size(); i++)
		{
			Point p1 = polygon[(startIdx + i) % polygon.size()];
			addExtrusionMove(p1, config);
			p0 = p1;
		}

		if (polygon.size() > 2)
			addExtrusionMove(polygon[startIdx], config);
	}

	void GCodePlanner::addPolygonWithUserExtruder(PolygonRef polygon, int startIdx, GCodePathConfig* config, int userExtruder)
	{
		Point p0 = polygon[startIdx];
		addTravelWithUserExtruder(p0, userExtruder);

		for (unsigned int i = 1; i<polygon.size(); i++)
		{
			Point p1 = polygon[(startIdx + i) % polygon.size()];
			addExtrusionMoveWithUserExturder(p1, config, userExtruder);
			p0 = p1;
		}

		if (polygon.size() > 2)
			addExtrusionMoveWithUserExturder(polygon[startIdx], config, userExtruder);
	}

	void GCodePlanner::addPolygonWithPreretraction(PolygonRef polygon, int startIdx, GCodePathConfig* print_config, GCodePathConfig* retract_config, int length)
	{
		if (length == 0)
		{
			addPolygon(polygon, startIdx, print_config);
			return;
		}

		float total_poly_length = polygon.polygonLength();

		if (total_poly_length <= length)
		{
			addPolygon(polygon, startIdx, print_config);
			return;
		}

		float total_length = 0;
		int stop_idx = 0;

		Point p0 = polygon[startIdx];
		addTravel(p0);
		for (unsigned int i = 0; i < polygon.size(); i++)
		{
			Point p1 = polygon[(startIdx + i + 1) % polygon.size()];
			float dist = vSize(p1 - p0);
			if (dist + total_length >= total_poly_length - length)
			{
				float a = ((total_poly_length - length) - total_length) / dist;
				Point p2;
				p2.X = (1 - a)*p0.X + a*p1.X;
				p2.Y = (1 - a)*p0.Y + a*p1.Y;
				addExtrusionMove(p2, print_config);
				stop_idx = i;
				break;
			}
			addExtrusionMove(p1, print_config);
			total_length += dist;
			p0 = p1;
		}

		for (unsigned int i = stop_idx; i < polygon.size(); i++)
		{
			Point p1 = polygon[(startIdx + i + 1) % polygon.size()];
			addExtrusionMove(p1, retract_config);
		}
	}

	void GCodePlanner::addPolygonWithOverMove(PolygonRef polygon, int startIdx, GCodePathConfig* overmove_config, int length)
	{
		if (length == 0)
			return;

		double total_length = 0;
		Point p0 = polygon[startIdx];
		int iter = 0;
		while (1)
		{
			Point p1 = polygon[(startIdx + iter + 1) % polygon.size()];
			double dist = vSize(p1 - p0);
			if (dist + total_length >= length)
			{
				float a = (length - total_length) / dist;

				Point p2;
				p2.X = (1 - a)*p0.X + a*p1.X;
				p2.Y = (1 - a)*p0.Y + a*p1.Y;
				addExtrusionMove(p2, overmove_config);
				break;
			}
			addExtrusionMove(p1, overmove_config);
			total_length += dist;
			iter++;
			p0 = p1;
		}
	}

	void GCodePlanner::addPolygonsByOptimizer(Polygons& polygons, GCodePathConfig* config)
	{
		PathOrderOptimizer orderOptimizer(lastPosition);

		for (unsigned int i = 0; i<polygons.size(); i++)
			orderOptimizer.addPolygon(polygons[i]);

		orderOptimizer.optimize();

		for (unsigned int i = 0; i<orderOptimizer.polyOrder.size(); i++)
		{
			int nr = orderOptimizer.polyOrder[i];
			addPolygon(polygons[nr], orderOptimizer.polyStart[nr], config);
		}
	}

	void GCodePlanner::addPolygonsByOptimizerWithUserExtruder(Polygons& polygons, GCodePathConfig* config, int userExtruder)
	{
		PathOrderOptimizer orderOptimizer(lastPosition);

		for (unsigned int i = 0; i<polygons.size(); i++)
			orderOptimizer.addPolygon(polygons[i]);

		orderOptimizer.optimize();

		for (unsigned int i = 0; i<orderOptimizer.polyOrder.size(); i++)
		{
			int nr = orderOptimizer.polyOrder[i];
			addPolygonWithUserExtruder(polygons[nr], orderOptimizer.polyStart[nr], config, userExtruder);
		}
	}



	//addPolygonsByOptimizer2 : inset���� infill�� �� �� ������θ� ���� �Լ�//
	//optimize2 ���
	//parameter_out_in ���
	void GCodePlanner::addPolygonsByOptimizer_insetToinfill_filteredTheta(Polygons& polygons, GCodePathConfig* config)
	{
		//����ġ �Ķ���ͷ� ���� �͸� �����..  overmoving�� ������ ���..

		PathOrderOptimizer orderOptimizer(lastPosition);
		orderOptimizer.pathOptimizationParameter[0] = this->pathOptimizationParameter[0];
		orderOptimizer.pathOptimizationParameter[1] = this->pathOptimizationParameter[1];
		orderOptimizer.pathOptimizationParameter[2] = this->pathOptimizationParameter[2];
		orderOptimizer.prevPoint = prevPosition;

		for (unsigned int i = 0; i < polygons.size(); i++)
			orderOptimizer.addPolygon(polygons[i]);

		//inset -> infill optimization + weight value parameter
		//orderOptimizer.optimize_insetToinfill();
		orderOptimizer.optimize_insetToinfill_filteringTheta(this->filteringAngle);

		for (unsigned int i = 0; i<orderOptimizer.polyOrder.size(); i++)
		{
			int nr = orderOptimizer.polyOrder[i];
			addPolygon(polygons[nr], orderOptimizer.polyStart[nr], config);
		}
	}


	void GCodePlanner::addPolygonWithPreretractionAndOvermoving(PolygonRef polygon, int startIdx, GCodePathConfig* print_config, GCodePathConfig* retract_config, GCodePathConfig* overmove_config, int pre, int over)
	{
		addPolygonWithPreretraction(polygon, startIdx, print_config, retract_config, pre);
		addPolygonWithOverMove(polygon, startIdx, overmove_config, over);
	}

	//addPolygonsByOptimizerWithPreretractionAndOvermoving : ��������� inset���� ���� inset���� �� �� ������θ� ���� �Լ�//
	//optimizeWithovermoving ��� //overmoving�� ����� �Լ������� setting���� overmoving�� ��� ����..
	//parameter_in_out ���
	void GCodePlanner::addPolygonsByOptimizerWithPreretractionAndOvermoving(Polygons& polygons, GCodePathConfig* print_config, GCodePathConfig* retract_config, GCodePathConfig* overmove_config, int pre, int over)
	{
		PathOrderOptimizer orderOptimizer(lastPosition);
		orderOptimizer.pathOptimizationParameter[0] = this->pathOptimizationParameter[0];
		orderOptimizer.pathOptimizationParameter[1] = this->pathOptimizationParameter[1];
		orderOptimizer.pathOptimizationParameter[2] = this->pathOptimizationParameter[2];
		orderOptimizer.prevPoint = prevPosition;

		for (unsigned int i = 0; i < polygons.size(); i++)
			orderOptimizer.addPolygon(polygons[i]);

		//theta filtering test//////////////////////////////////////////////////////
		//orderOptimizer.optimize_overmoving(over);
		orderOptimizer.optimize_overmoving_filteringTheta(over, this->filteringAngle);
		////////////////////////////////////////////////////////////////////////////

		for (unsigned int i = 0; i < orderOptimizer.polyOrder.size(); i++)
		{
			int nr = orderOptimizer.polyOrder[i];
			addPolygonWithPreretractionAndOvermoving(polygons[nr], orderOptimizer.polyStart[nr], print_config, retract_config, overmove_config, pre, over);
		}
	}


	void GCodePlanner::forceMinimalLayerTime(std::vector<double> minTime, std::vector<int> minimalSpeed)
	{
		Point p0 = gcode.getPositionXY();
		double travelTime = 0.0;
		double extrudeTime = 0.0;
		for (unsigned int n = 0; n<paths.size(); n++)
		{
			GCodePath* path = &paths[n];
			for (unsigned int i = 0; i<path->points.size(); i++)
			{
				double thisTime = vSizeMM(p0 - path->points[i]) / double(path->config->speed);
				if (path->config->lineWidth != 0)
					extrudeTime += thisTime;
				else
					travelTime += thisTime;
				p0 = path->points[i];
			}
		}
		double totalTime = extrudeTime + travelTime;
		if (totalTime < minTime[currentExtruder] && extrudeTime > 0.0)
		{
			double minExtrudeTime = minTime[currentExtruder] - travelTime;
			if (minExtrudeTime < 1)
				minExtrudeTime = 1;
			double factor = extrudeTime / minExtrudeTime;
			for (unsigned int n = 0; n<paths.size(); n++)
			{
				GCodePath* path = &paths[n];
				if (path->config->lineWidth == 0)
					continue;
				int speed = path->config->speed * factor;
				if (speed < minimalSpeed[currentExtruder])
					factor = double(minimalSpeed[currentExtruder]) / double(path->config->speed);
			}

			//Only slow down with the minimal time if that will be slower then a factor already set. First layer slowdown also sets the speed factor.
			if (factor * 100 < getExtrudeSpeedFactor())
				setExtrudeSpeedFactor(factor * 100);
			else
				factor = getExtrudeSpeedFactor() / 100.0;

			if (minTime[currentExtruder] - (extrudeTime / factor) - travelTime > 0.1)
			{
				//TODO: Use up this extra time (circle around the print?)
				this->extraTime = minTime[currentExtruder] - (extrudeTime / factor) - travelTime;
			}
			this->totalPrintTime = (extrudeTime / factor) + travelTime;
		}
		else
		{
			this->totalPrintTime = totalTime;
		}
	}

	void GCodePlanner::writeGCode(bool _liftHeadIfNeeded, int _layerThickness, int _layerNr, bool _initRaft)
	{
		bool toolChanged = false;

		GCodePathConfig* lastConfig = nullptr;
		int extruder = gcode.getExtruderNr();

		gcode.gcodePaths.paths.clear();
		for (unsigned int n = 0; n<paths.size(); n++)
		{
			if (n != 0)
				_initRaft = false;
			GCodePath* path = &paths[n];
			if (extruder != path->extruder)
			{
				extruder = path->extruder;
				gcode.switchExtruder(extruder);

				toolChanged = true;
			}
			else if (path->retract)		// ���� else if�� �Ǿ��־���.
			{
				gcode.writeRetraction();	// before moving
			}

			if (strcmp(path->config->name, "travel") != 0 && lastConfig != path->config)
			{
				gcode.writeComment("TYPE:%s", path->config->name);
				gcode.writeComment("EXTRUDER : %d", gcode.extruderNr);

				lastConfig = path->config;
			}

			if (strcmp(path->config->name, "travel") == 0)
				gcode.current_mode = TRAVEL;
			else if (strcmp(path->config->name, "SKIRT") == 0)
				gcode.current_mode = SKIRT;
			else if (strcmp(path->config->name, "BRIM") == 0)
				gcode.current_mode = BRIM;
			else if (strcmp(path->config->name, "SKIN") == 0)
				gcode.current_mode = SKIN;
			else if (strcmp(path->config->name, "WALL-INNER") == 0)
				gcode.current_mode = WALL_INNER;
			else if (strcmp(path->config->name, "WALL-OUTER") == 0)
				gcode.current_mode = WALL_OUTER;
			else if (strcmp(path->config->name, "FILL") == 0)
				gcode.current_mode = FILL;
			else if (strcmp(path->config->name, "SUPPORT_MAIN") == 0)
				gcode.current_mode = SUPPORT_MAIN;
			else if (strcmp(path->config->name, "PRERETRACT0") == 0)
				gcode.current_mode = PRERETRACTION0;
			else if (strcmp(path->config->name, "OVERMOVE0") == 0)
				gcode.current_mode = OVERMOVE0;
			else if (strcmp(path->config->name, "PRERETRACTX") == 0)
				gcode.current_mode = PRERETRACTIONX;
			else if (strcmp(path->config->name, "OVERMOVEX") == 0)
				gcode.current_mode = OVERMOVEX;
			else if (strcmp(path->config->name, "RAFT") == 0)
				gcode.current_mode = RAFT;
			else if (strcmp(path->config->name, "SUPPORT_INTERFACE_ROOF") == 0)
				gcode.current_mode = SUPPORT_INTERFACE_ROOF;
			else if (strcmp(path->config->name, "SUPPORT_INTERFACE_FLOOR") == 0)
				gcode.current_mode = SUPPORT_INTERFACE_FLOOR;
			else if (strcmp(path->config->name, "WIPE_TOWER") == 0)
				gcode.current_mode = WIPE_TOWER;


			int speed = path->config->speed;

			if (path->config->lineWidth != 0)// Only apply the extrudeSpeedFactor to extrusion moves
				speed = speed * extrudeSpeedFactor / 100;
			else
				speed = speed * travelSpeedFactor / 100;

			if (path->points.size() == 1 && path->config != &pathconfigs_travel[currentExtruder] && shorterThen(gcode.getPositionXY() - path->points[0], path->config->lineWidth * 2))
			{
				//Check for lots of small moves and combine them into one large line
				Point p0 = path->points[0];
				unsigned int i = n + 1;
				while (i < paths.size() && paths[i].points.size() == 1 && shorterThen(p0 - paths[i].points[0], path->config->lineWidth * 2))
				{
					p0 = paths[i].points[0];
					i++;
				}
				if (paths[i - 1].config == &pathconfigs_travel[currentExtruder])
					i--;
				if (i > n + 2)
				{
					p0 = gcode.getPositionXY();
					for (unsigned int x = n; x<i - 1; x += 2)
					{
						int64_t oldLen = vSize(p0 - paths[x].points[0]);
						Point newPoint = (paths[x].points[0] + paths[x + 1].points[0]) / 2;
						int64_t newLen = vSize(gcode.getPositionXY() - newPoint);
						if (newLen > 0)
							gcode.writeMove(newPoint, speed, path->config->lineWidth * oldLen / newLen, _layerNr, gcode.b_spiralize);

						p0 = paths[x + 1].points[0];
					}
					gcode.writeMove(paths[i - 1].points[0], speed, path->config->lineWidth, _layerNr, gcode.b_spiralize);
					n = i - 1;
					continue;
				}
			}

			bool spiralize = path->config->spiralize;
			if (spiralize)
			{
				//Check if we are the last spiralize path in the list, if not, do not spiralize.
				for (unsigned int m = n + 1; m<paths.size(); m++)
				{
					if (paths[m].config->spiralize)
						spiralize = false;
				}
			}
			if (spiralize)
			{
				//If we need to spiralize then raise the head slowly by 1 layer as this path progresses.
				float totalLength = 0.0;
				int z = gcode.getPositionZ();
				Point p0 = gcode.getPositionXY();
				for (unsigned int i = 0; i<path->points.size(); i++)
				{
					Point p1 = path->points[i];
					totalLength += vSizeMM(p0 - p1);
					p0 = p1;
				}

				float length = 0.0;
				p0 = gcode.getPositionXY();
				for (unsigned int i = 0; i<path->points.size(); i++)
				{
					Point p1 = path->points[i];
					length += vSizeMM(p0 - p1);
					p0 = p1;
					gcode.setZ(z + _layerThickness * length / totalLength);
					gcode.writeMove(path->points[i], speed, path->config->lineWidth, _layerNr, gcode.b_spiralize);
				}
			}
			else
			{
				if (_initRaft && path->points.size() > 0)
				{
					//raft�� ��������.. brim�̳� skirt�� �� ���..raft�� 0�̹Ƿ� �ٸ� ������ �־���� �Ѵ�..
					//������ ���� ������ ���밪���� �־ ���� ��.. speed, lineWidth..
					//gcode.writeMove(path->points[0], gcode.raft_base_speed, gcode.raftBaseLineWidth);

					////DP201
					//if (gcode.raftBaseFanControlEnabled)
					//{
					//	gcode.writeMove(path->points[0], 10, 1000);
					//}
					//else //DP200
					//{
					//	gcode.writeMove(path->points[0], speed, 0, true);
					//}

					if (gcode.platformAdhesion == Generals::PlatformAdhesion::Raft)	gcode.writeMove(path->points[0], gcode.raftBaseSpeed, gcode.initial_layer_extrusion_width[currentExtruder], _layerNr, gcode.b_spiralize);
					else gcode.writeMove(path->points[0], gcode.initial_layer_speed[currentExtruder], gcode.initial_layer_extrusion_width[currentExtruder], _layerNr, gcode.b_spiralize);

					gcode.b_initialMoving = false;
				}

				for (unsigned int i = _initRaft ? 1 : 0; i<path->points.size(); i++)
				{

					if (gcode.b_initialMoving)
					{
						////DP201
						//if (gcode.raftBaseFanControlEnabled)
						//{
						//	gcode.writeMove(path->points[0], 10, 1000);
						//	gcode.writeFanCommand(gcode.extruderNr, 12.5);
						//}
						
						if (gcode.platformAdhesion == Generals::PlatformAdhesion::Raft)	gcode.writeMove(path->points[0], gcode.raftBaseSpeed, gcode.initial_layer_extrusion_width[currentExtruder], _layerNr, gcode.b_spiralize);
						else gcode.writeMove(path->points[0], gcode.initial_layer_speed[currentExtruder], gcode.initial_layer_extrusion_width[currentExtruder], _layerNr, gcode.b_spiralize);

						//DP201
						if (gcode.raftBaseFanControlEnabled)
						{
							gcode.writeFanCommand(gcode.extruderNr, 12.5);
						}

						gcode.b_initialMoving = false;
					}

					gcode.writeMove(path->points[i], speed, path->config->lineWidth, _layerNr, gcode.b_spiralize);

					if (toolChanged && (gcode.toolchangeZlifting != gcode.currentPosition.z) && !gcode.limitBedlifting)
					{
						//bed lowering -> tool chagne -> bed lifting//
						//tool change & raft airgap --> skip//
						gcode.writeBedLifting(gcode.print_speed[currentExtruder], (gcode.toolchangeZlifting - gcode.toolchangeLoweringBed));

						toolChanged = false;
					}
					gcode.limitBedlifting = false;
				}
			}
		}

		gcode.updateTotalPrintTime();
		if (_liftHeadIfNeeded && extraTime > 0.0)
		{
			gcode.writeComment("Small layer, adding delay of %f", extraTime);
			gcode.writeRetraction(true);
			gcode.setZ(gcode.getPositionZ() + MM2INT(3.0));
			gcode.writeMove(gcode.getPositionXY(), pathconfigs_travel.at(currentExtruder).speed, 0, _layerNr, gcode.b_spiralize);
			gcode.writeMove(gcode.getPositionXY() - Point(-MM2INT(20.0), 0), pathconfigs_travel.at(currentExtruder).speed, 0, _layerNr, gcode.b_spiralize);
			gcode.writeDelay(extraTime);
		}
	}

	void GCodePlanner::writeGCode2(bool _liftHeadIfNeeded, int _layerThickness, int _layerNr, std::vector<int>* _firstToolChangeInfo, std::vector<PathPrintTimeInfo_accumulated> _pathPrintTimeInfo_accumulated_vector, std::vector<double> _preheatStartTime, bool _initRaft)
	{
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		//writeGCode�� ������..//
		//- _layerNr, _firstToolChangeInfo ����� �߰���..//
		//- path segment ������ print time ����..//
		//- preheat ��� �߰�..//
		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		bool toolChanged = false;
		bool b_firstToolChange = false;
		bool b_firstNONEStatus = false;

		std::vector<bool> b_preheatCode_inserted;
		b_preheatCode_inserted.resize(gcode.cartridgeCountTotal);

		for (int i = 0; i < gcode.cartridgeCountTotal; i++)
			b_preheatCode_inserted.at(i) = false;


		GCodePathConfig* lastConfig = nullptr;

		int extruder = gcode.getExtruderNr();
		gcode.gcodePaths.paths.clear();
		for (unsigned int n = 0; n < paths.size(); n++)
		{
			if (n != 0)
				_initRaft = false;
			GCodePath* path = &paths[n];

			if (extruder != path->extruder)
			{
				extruder = path->extruder;

				if (gcode.firstToolChanged[extruder] == false)
				{
					_firstToolChangeInfo->push_back(extruder);
					_firstToolChangeInfo->push_back(_layerNr);

					///////////////////////////////////////////////////////////////////////////////////////////////////////////

					if (gcode.b_notExtruderSet)
					{
						//no raft//
						//platform adhesion�� NONE / BRIM / SKIRT�� ���, ���߿� ���� nozzle change�� firstToolChagneInfo size�� 4�� ����..//
						//writeGcode�� �� ��, extruderNr�� �����Ǿ� ���� �ʱ� ������(T100���� �Ǿ��ֱ⿡..), toolchanging�� �ϸ鼭 �����ϱ� ����..//
						//�� : T100 -> T0 -> T1 or T100 -> T1 -> T0
						if (_firstToolChangeInfo->size() == 4)
						{
							b_firstToolChange = true;
						}
					}
					else
					{
						//raft//
						//���߿� ���� nozzle change�� firstToolChagneInfo size�� 2�� ����..
						//�� : T0 -> T1
						if (_firstToolChangeInfo->size() == 2)
						{
							b_firstToolChange = true;
						}
					}

					///////////////////////////////////////////////////////////////////////////////////////////////////////////

					gcode.firstToolChanged[extruder] = true;
				}

				//first tool change �� ���..//
				if (b_firstToolChange)
				{
					QString temperatureStr_firstToolChange;

					//temperature table enabled ���� ..//
					if (gcode.temperature_layer_setting_enabled[extruder] && !gcode.temperature_layer_list.at(extruder).empty())
					{
						int temp_temperature = Profile::getTemperatureByLayer(&gcode.temperature_layer_list.at(extruder), _layerNr, gcode.nozzle_print_temperature[extruder]);

						if (_layerNr == 0)
							temp_temperature = gcode.nozzle_print_temperature[extruder];

						temperatureStr_firstToolChange.append(QString("M104 T%1 S%2		;Heat up the %3th nozzle - print temperature_table(first tool change)\n").arg(extruder).arg(temp_temperature).arg(extruder));
					}
					else
					{
						temperatureStr_firstToolChange.append(QString("M104 T%1 S%2		;Heat up the %3th nozzle - print temperature(first tool change)\n").arg(extruder).arg(gcode.nozzle_print_temperature[extruder]).arg(extruder));
					}


					int counter_extruder = std::abs(extruder - 1);
					if (gcode.standby_temperature_enabled[counter_extruder] && !(gcode.preheat_enabled[counter_extruder] && _preheatStartTime.at(extruder) == -1))
					{
						temperatureStr_firstToolChange.append(QString("M104 T%1 S%2		;Heat up the %3th nozzle - standby temperature(first tool change)\n").arg(counter_extruder).arg(gcode.nozzle_operating_standby_temperature[counter_extruder]).arg(counter_extruder));
					}

					gcode.writeCode(temperatureStr_firstToolChange.toStdString().c_str());
					gcode.writeNewLine();
				}


				QString temperatureStr;
				std::vector<bool> check_standbyTemperature_temp;
				check_standbyTemperature_temp.resize(gcode.cartridgeCountTotal);

				//�Ϲ����� ���..//
				for (int i = 0; i < gcode.cartridgeCountTotal; i++)
				{
					check_standbyTemperature_temp.at(i) = false;

					if (!b_firstToolChange)
					{
						//standby_temperature �� ��..//
						if (gcode.standby_temperature_enabled[i])
						{
							//���� extruder..//
							if (extruder == i)
							{
								if (gcode.temperature_layer_setting_enabled[i] && !gcode.temperature_layer_list.at(extruder).empty())
								{
									int temp_temperature = Profile::getTemperatureByLayer(&gcode.temperature_layer_list.at(i), _layerNr, gcode.nozzle_print_temperature[i]);
									temperatureStr.append(QString("M104 T%1 S%2		;Heat up the %3th nozzle - print temperature_table\n").arg(i).arg(temp_temperature).arg(i));
								}
								else
									temperatureStr.append(QString("M104 T%1 S%2		;Heat up the %3th nozzle - print temperature\n").arg(i).arg(gcode.nozzle_print_temperature[i]).arg(i));
							}
							//������ extruder//
							else if (!(gcode.preheat_enabled[i] && _preheatStartTime.at(extruder) == -1))
							{
								temperatureStr.append(QString("M104 T%1 S%2		;Heat up the %3th nozzle - standby temperature\n").arg(i).arg(gcode.nozzle_operating_standby_temperature[i]).arg(i));
							}

							check_standbyTemperature_temp.at(i) = true;
						}
						else if (gcode.temperature_layer_setting_enabled[extruder]) //standby_temperature�� �ƴ� �Ϲ����� ���..//
						{
							//���� extruder�� ���ؼ��� �˻�..//
							if (extruder == i)
							{
								//table�� set point�� ���� ��쿡�� �ش��ϵ���..//
								if (Profile::isTemperatureSetPointAtLayer(&gcode.temperature_layer_list.at(extruder), _layerNr))
								{
									int temp_temperature = Profile::getTemperatureByLayer(&gcode.temperature_layer_list.at(i), _layerNr, gcode.nozzle_print_temperature[i]);
									temperatureStr.append(QString("M104 T%1 S%2		;Heat up the %3th nozzle - print temperature_table\n").arg(i).arg(temp_temperature).arg(i));
								}
							}

						}
					}
				}

				if (!temperatureStr.isEmpty())
				{
					gcode.writeNewLine();
					gcode.writeCode(temperatureStr.toStdString().c_str());
					gcode.writeNewLine();
				}


				//when layer0 temperature is used, at _layerNr == 1, nozzle temperature is set to printer temeperature..//
				//temperature profile ������� �ϴ� �� �κ� ����..//

				//if (_layerNr == 1 && gcode.nozzle_layer0_temperature_enabled[extruder] && !check_standbyTemperature_temp.at(extruder))
				//{
				//	char temp_temperature_code[100];

				//	sprintf_s(temp_temperature_code, "M104 T%i S%d		;print temperature", extruder, int(gcode.nozzle_print_temperature[extruder]));

				//	gcode.writeCode(temp_temperature_code);
				//	gcode.writeNewLine();
				//}


				///////////////////////////////////////////////////////////////////////////////////////////////////////////
				//switching extruder//
				gcode.switchExtruder(extruder);
				///////////////////////////////////////////////////////////////////////////////////////////////////////////

				if (b_firstToolChange)
				{
					gcode.writeComment("first toolchange setting");
					gcode.writeExtrusionReset(-20);
					gcode.writeNewLine();
				}

				toolChanged = true;
			}
			else if (path->retract)		// ���� else if�� �Ǿ��־���.
			{
				gcode.writeRetraction();	// before moving
			}

			if (strcmp(path->config->name, "travel") != 0 && lastConfig != path->config)
			{
				gcode.writeComment("TYPE:%s", path->config->name);
				gcode.writeComment("EXTRUDER : %d", gcode.extruderNr);

				lastConfig = path->config;

				if (strcmp(lastConfig->name, "WIPE_TOWER") == 0)
				{
					gcode.b_lastConifg_WIPETOWER = true;
				}
				else
				{
					gcode.b_lastConifg_WIPETOWER = false;
				}
			}

			if (strcmp(path->config->name, "travel") == 0)
				gcode.current_mode = TRAVEL;
			else if (strcmp(path->config->name, "SKIRT") == 0)
				gcode.current_mode = SKIRT;
			else if (strcmp(path->config->name, "BRIM") == 0)
				gcode.current_mode = BRIM;
			else if (strcmp(path->config->name, "SKIN") == 0)
				gcode.current_mode = SKIN;
			else if (strcmp(path->config->name, "WALL-INNER") == 0)
				gcode.current_mode = WALL_INNER;
			else if (strcmp(path->config->name, "WALL-OUTER") == 0)
				gcode.current_mode = WALL_OUTER;
			else if (strcmp(path->config->name, "FILL") == 0)
				gcode.current_mode = FILL;
			else if (strcmp(path->config->name, "SUPPORT_MAIN") == 0)
				gcode.current_mode = SUPPORT_MAIN;
			else if (strcmp(path->config->name, "PRERETRACT0") == 0)
				gcode.current_mode = PRERETRACTION0;
			else if (strcmp(path->config->name, "OVERMOVE0") == 0)
				gcode.current_mode = OVERMOVE0;
			else if (strcmp(path->config->name, "PRERETRACTX") == 0)
				gcode.current_mode = PRERETRACTIONX;
			else if (strcmp(path->config->name, "OVERMOVEX") == 0)
				gcode.current_mode = OVERMOVEX;
			else if (strcmp(path->config->name, "RAFT") == 0)
				gcode.current_mode = RAFT;
			else if (strcmp(path->config->name, "SUPPORT_INTERFACE_ROOF") == 0)
				gcode.current_mode = SUPPORT_INTERFACE_ROOF;
			else if (strcmp(path->config->name, "SUPPORT_INTERFACE_FLOOR") == 0)
				gcode.current_mode = SUPPORT_INTERFACE_FLOOR;
			else if (strcmp(path->config->name, "WIPE_TOWER") == 0)
				gcode.current_mode = WIPE_TOWER;

			int speed = path->config->speed;

			if (_layerNr == 0)
				speed = gcode.initial_layer_speed[extruder];

			if (path->config->lineWidth != 0)// Only apply the extrudeSpeedFactor to extrusion moves
				speed = speed * extrudeSpeedFactor / 100;
			else
				speed = speed * travelSpeedFactor / 100;

			if (path->points.size() == 1 && path->config != &pathconfigs_travel[currentExtruder] && shorterThen(gcode.getPositionXY() - path->points[0], path->config->lineWidth * 2))
			{
				//Check for lots of small moves and combine them into one large line
				Point p0 = path->points[0];
				unsigned int i = n + 1;
				while (i < paths.size() && paths[i].points.size() == 1 && shorterThen(p0 - paths[i].points[0], path->config->lineWidth * 2))
				{
					p0 = paths[i].points[0];
					i++;
				}
				if (paths[i - 1].config == &pathconfigs_travel[currentExtruder])
					i--;
				if (i > n + 2)
				{
					p0 = gcode.getPositionXY();
					for (unsigned int x = n; x<i - 1; x += 2)
					{
						int64_t oldLen = vSize(p0 - paths[x].points[0]);
						Point newPoint = (paths[x].points[0] + paths[x + 1].points[0]) / 2;
						int64_t newLen = vSize(gcode.getPositionXY() - newPoint);
						if (newLen > 0)
						{
							gcode.writeMove(newPoint, speed, path->config->lineWidth * oldLen / newLen, _layerNr, gcode.b_spiralize);
							//gcode.writeComment("TEST_3");
						}

						p0 = paths[x + 1].points[0];
					}
					gcode.writeMove(paths[i - 1].points[0], speed, path->config->lineWidth, _layerNr, gcode.b_spiralize);
					//gcode.writeComment("TEST_4");

					n = i - 1;
					continue;
				}
			}

			bool spiralize = path->config->spiralize;
			if (spiralize)
			{
				//Check if we are the last spiralize path in the list, if not, do not spiralize.
				for (unsigned int m = n + 1; m<paths.size(); m++)
				{
					if (paths[m].config->spiralize)
						spiralize = false;
				}
			}
			if (spiralize)
			{
				//If we need to spiralize then raise the head slowly by 1 layer as this path progresses.
				float totalLength = 0.0;
				int z = gcode.getPositionZ();
				Point p0 = gcode.getPositionXY();
				for (unsigned int i = 0; i<path->points.size(); i++)
				{
					Point p1 = path->points[i];
					totalLength += vSizeMM(p0 - p1);
					p0 = p1;
				}

				float length = 0.0;
				p0 = gcode.getPositionXY();
				for (unsigned int i = 0; i<path->points.size(); i++)
				{
					Point p1 = path->points[i];
					length += vSizeMM(p0 - p1);
					p0 = p1;
					gcode.setZ(z + _layerThickness * length / totalLength);
					gcode.writeMove(path->points[i], speed, path->config->lineWidth, _layerNr, gcode.b_spiralize);
				}
			}
			else
			{
				if (_initRaft && path->points.size() > 0)
				{
					//raft�� ��������.. brim�̳� skirt�� �� ���..raft�� 0�̹Ƿ� �ٸ� ������ �־���� �Ѵ�..
					//������ ���� ������ ���밪���� �־ ���� ��.. speed, lineWidth..
					//gcode.writeMove(path->points[0], gcode.raft_base_speed, gcode.raftBaseLineWidth);

					////DP201
					//if (gcode.raftBaseFanControlEnabled)
					//{
					//	gcode.writeMove(path->points[0], 10, 1000);
					//	
					//}
					//else //DP200
					//{
					//	gcode.writeMove(path->points[0], speed, 0, true);
					//}
					if (gcode.platformAdhesion == Generals::PlatformAdhesion::Raft)	
					{
						gcode.writeMove(path->points[0], gcode.raftBaseSpeed, gcode.initial_layer_extrusion_width[extruder], _layerNr, gcode.b_spiralize);
					}
					else
					{
						gcode.writeMove(path->points[0], gcode.initial_layer_speed[extruder], gcode.initial_layer_extrusion_width[extruder], _layerNr, gcode.b_spiralize);
					}
										
					gcode.b_initialMoving = false;
				}
				
				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//main path points loop//
				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//_initRaft true -> i = 1
				//_initRaft false -> i = 0
				for (unsigned int i = _initRaft ? 1 : 0; i<path->points.size(); i++)
				{
					if (gcode.b_initialMoving)
					{
						if (gcode.platformAdhesion == Generals::PlatformAdhesion::Raft)
						{
							gcode.writeMove(path->points[0], gcode.raftBaseSpeed, gcode.initial_layer_extrusion_width[extruder], _layerNr, gcode.b_spiralize);
						}
						else
						{
							gcode.writeMove(path->points[0], gcode.initial_layer_speed[extruder], gcode.initial_layer_extrusion_width[extruder], _layerNr, gcode.b_spiralize);
						}

						//DP201
						if (gcode.raftBaseFanControlEnabled)
						{
							gcode.writeFanCommand(gcode.extruderNr, 12.5);
						}
						
						//gcode.b_initialMoving = false;
					}



					//toolchangeZlifting : currentPosition.z + toolchangeLoweringBed//
					if (toolChanged)
					{
						//tool change�� �Ǿ�����, initial moving�� �ƴ� ��쿡�� ��. initial moving�� ���, �ǳʶ�...//
						if (!gcode.b_initialMoving)
						{
							//path writing when toolChanged..//
							gcode.writeMove_toolchanged(path->points[i], speed, path->config->lineWidth, _layerNr, gcode.b_spiralize, toolChanged);
							///////////////////////////////////////

							if (!(b_firstToolChange || gcode.b_notExtruderSet))
							{
								//������ tool change retraction����ŭ���� �ʱ�ȭ ��//
								gcode.writeExtrusionReset((-1) * gcode.toolchangeRetractionAmount[gcode.getExtruderNr()]);
							}

							if (gcode.toolchangeExtraRestartAmount[gcode.getExtruderNr()] != 0)
							{
								//tool change�Ŀ� restart�� �ִ� ��� ��..cartridge���� �и��� �ؾ� �� //
								gcode.writeExtrusion(gcode.toolchangeExtraRestartSpeed[gcode.getExtruderNr()], gcode.toolchangeExtraRestartAmount[gcode.getExtruderNr()]);
								gcode.writeCode("G92 E0\n");
							}
						}

						toolChanged = false;

						//b_first tool change tag �ʱ�ȭ..//
						if (b_firstToolChange)
							b_firstToolChange = false;
					}


					///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					//main path writing//
					gcode.writeMove(path->points[i], speed, path->config->lineWidth, _layerNr, gcode.b_spiralize);
					///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	


					//b_inital moving �ʱ�ȭ..//
					gcode.b_initialMoving = false;
				}
			}


			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//Ư�� n�� ���.. ���⿡ preheat code inserting..//
			if (gcode.b_multiExtruder && gcode.preheat_OR_enabled)
			{
				int counter_extruder = std::abs(extruder - 1);

				if (_preheatStartTime.at(extruder) != -1 &&
					_preheatStartTime.at(extruder) < pathPrintTimeInfo_accumulated_vector.at(n).pathPrintTime_accumulated &&
					extruder == pathPrintTimeInfo_accumulated_vector.at(n).pathExtruderNr &&
					!b_preheatCode_inserted.at(extruder) &&
					gcode.preheat_enabled[counter_extruder] &&
					gcode.b_usesCartridgeState_index.at(counter_extruder) != false &&
					b_isToolchangedInLayer)
				{
					QString preheat_code = QString("M104 T%1 S%2	;------------------------> preheat code").arg(counter_extruder).arg(gcode.nozzle_print_temperature[counter_extruder]);

					gcode.writeCode(preheat_code.toStdString().c_str());
					gcode.writeNewLine();

					b_preheatCode_inserted.at(extruder) = true;
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		}

		//writeMove���� ������ plan���� �Ѳ����� ���--> update
		gcode.updateTotalPrintTime();

		if (_liftHeadIfNeeded && extraTime > 0.0)
		{
			gcode.writeComment("Small layer, adding delay of %f", extraTime);
			gcode.writeRetraction(true);
			gcode.setZ(gcode.getPositionZ() + MM2INT(3.0));
			gcode.writeMove(gcode.getPositionXY(), pathconfigs_travel.at(currentExtruder).speed, 0, _layerNr, gcode.b_spiralize);
			//gcode.writeComment("TEST_7");

			gcode.writeMove(gcode.getPositionXY() - Point(-MM2INT(20.0), 0), pathconfigs_travel.at(currentExtruder).speed, 0, _layerNr, gcode.b_spiralize);
			//gcode.writeComment("TEST_8");

			gcode.writeDelay(extraTime);
		}

	}

	void GCodePlanner::getPathPrintTimeInfo(bool liftHeadIfNeeded, int layerThickness, int layerNr, bool initRaft)
	{
		////////////////////////////////////////////////////////////////////////////////////////////
		//get path print time info & end time
		int extruder_timeCalculate = gcode.getExtruderNr();
		int pathExtruder_before;

		gcode.pathPrintTime_accumulated.clear();
		gcode.pathEndPrintTime.clear();
		gcode.pathPrintTime_accumulated.resize(gcode.cartridgeCountTotal);
		gcode.pathEndPrintTime.resize(gcode.cartridgeCountTotal);
		gcode.reset_pathPrintTime_pathEndPrintTime();

		for (unsigned int n = 0; n < paths.size(); n++)
		{
			GCodePath* path = &paths[n];
			PathPrintTimeInfo_accumulated pathPrintTimeInfo_accumulated;

			pathPrintTimeInfo_accumulated.pathExtruderNr = path->extruder;

			if (!b_isToolchangedInLayer)
			{
				if (n == 0)
					pathExtruder_before = path->extruder;

				if (pathExtruder_before != path->extruder)
					b_isToolchangedInLayer = true;
			}

			//���� ������� ������ �ݴ����� ã��..//--> extruder�� 2���� ��쿡�� ��ȿ..//
			int counter_extruder_timeCalculate = std::abs(path->extruder - 1);

			//�ݴ��� ������ ��� �µ� ����� Ȱ��ȭ �Ǿ� ������.. ����.//
			if (gcode.standby_temperature_enabled[counter_extruder_timeCalculate] && gcode.preheat_enabled[counter_extruder_timeCalculate])
			{
				//calculating path segemnt print time & accumulating//
				gcode.pathPrintTime_accumulated.at(path->extruder) += calculatePathPrintTime(path, liftHeadIfNeeded, layerThickness, layerNr, n, false);

				pathPrintTimeInfo_accumulated.pathIndex = n;
				pathPrintTimeInfo_accumulated.pathPrintTime_accumulated = gcode.pathPrintTime_accumulated.at(path->extruder);
			}
			else
			{
				pathPrintTimeInfo_accumulated.pathIndex = -1;
				pathPrintTimeInfo_accumulated.pathPrintTime_accumulated = -1;
			}

			pathPrintTimeInfo_accumulated_vector.push_back(pathPrintTimeInfo_accumulated);
		}

		gcode.pathEndPrintTime = gcode.pathPrintTime_accumulated;
	}

	std::vector<double> GCodePlanner::getPreheatStartTime()
	{
		gcode.time_end.clear();
		gcode.time_standby.clear();
		gcode.time_preheat.clear();

		gcode.time_end.resize(gcode.cartridgeCountTotal);
		gcode.time_standby.resize(gcode.cartridgeCountTotal);
		gcode.time_preheat.resize(gcode.cartridgeCountTotal);

		for (int i = 0; i < gcode.cartridgeCountTotal; i++)
		{
			int counter_extruder = std::abs(i - 1);

			//time_end : ���� extruder�� �н� ��� ���Ḧ ����ؾ���..//
			gcode.time_end.at(i) = gcode.pathEndPrintTime.at(i);

			//stand_by�� ���� �ݴ����� ���� �������� ����ؾ� ��. --> ������ ���� extruder��������.. ȥ������//
			//�ڵ� ��� �κ� �� ������ ����κ��� ���� extruder ����..//
			//��� �κ� ���� ������ �ݴ��� ������ �����ؾ� ��.. preheat ��� �κ�..//
			gcode.time_standby.at(i) = (gcode.nozzle_print_temperature[counter_extruder] - gcode.nozzle_operating_standby_temperature[counter_extruder]) * (1 / gcode.preheat_temperature_falling_rate[counter_extruder] + 1 / gcode.preheat_temperature_rising_rate[counter_extruder]);

			if (gcode.nozzle_operating_standby_temperature[counter_extruder] >= gcode.nozzle_print_temperature[counter_extruder])
			{
				//standby temperature�� print temperature���� ���ų� ���� ���.. preheat �ǹ̰� �����Ƿ� -1�� ��..//
				gcode.time_preheat.at(i) = -1;
				continue;
			}

			///////////////////////////////////////////////////////////////////////////////
			//time_print_end -> 3������ ������..
			//(i)	t < time_threshold					: threshold time���� ���� ��..//
			//(ii)	time_threshold <= t < time_standby	: threshold���ٴ� ũ�� standby������ time���� ���� ��..//
			//(iii) t > time_standby					: standby���� Ŭ ��..//
			///////////////////////////////////////////////////////////////////////////////
			//
			// (Temperature)
			// ^
			// ��  < preheat diagram >
			// ��
			// | 
			// |  (i)|  (ii) |            (iii)           |
			// |     |		 |							  | -> T_printing
			// |  \  |       /            /              /| 
			// |   \ |      /|           /              / |
			// |    \|     / |          /              /  |
			// |     \    /  |         /              /   |
			// |     |\  /   |        /              /    |
			// |     | \/____|_______/______________/     |	-> T_standby
			// |     |       |     t_preheat              |
			// ����������������������������������������������������������������> (time)
			//      t_th   t_standby                    t_end
			//
			///////////////////////////////////////////////////////////////////////////////

			if (gcode.time_end.at(i) < gcode.preheat_threshold_time[counter_extruder])
			{
				//(i) section
				gcode.time_preheat.at(i) = -1;
			}
			else if (gcode.time_end.at(i) >= gcode.preheat_threshold_time[counter_extruder] && gcode.time_end.at(i) < gcode.time_standby.at(i))
			{
				//(ii) section
				gcode.time_preheat.at(i) = ((gcode.preheat_temperature_falling_rate[counter_extruder] * gcode.preheat_temperature_rising_rate[counter_extruder]) / (gcode.preheat_temperature_falling_rate[counter_extruder] + gcode.preheat_temperature_rising_rate[i])) * gcode.time_end.at(i) / gcode.preheat_temperature_falling_rate[counter_extruder];
			}
			else if (gcode.time_end.at(i) >= gcode.time_standby.at(i))
			{
				//(iii) section
				gcode.time_preheat.at(i) = gcode.time_end.at(i) - (gcode.nozzle_print_temperature[counter_extruder] - gcode.nozzle_operating_standby_temperature[counter_extruder]) / gcode.preheat_temperature_rising_rate[counter_extruder];
			}
			else
			{
				gcode.time_preheat.at(i) = -1;
			}
		}

		return gcode.time_preheat;
	}

	void GCodePlanner::writeGCode2_ToStorage(bool liftHeadIfNeeded, int layerThickness, int layerNr, std::vector<int>* firstToolChangeInfo, bool initRaft)
	{
		//writeGCode�� ������..//
		//layerNr, firstToolChangeInfo ����� �߰���..//

		bool toolChanged = false;
		bool b_firstToolChange = false;
		bool b_firstNONEStatus = false;
		bool b_isToStorage;

		bool b_On_CalPrintTime = false;
		bool b_Off_CalPrintTime = false;


		GCodePathConfig* lastConfig = nullptr;

		int extruder = gcode.getExtruderNr();
		gcode.gcodePaths.paths.clear();
		gcode.totalPrintTime_Storage = 0.0;

		gcode.writeComment_ToStorage(true, &gcode.gcodeStorage, "writeGCode2_ToStorage start point..");
		gcode.writeComment_ToStorage(false, &gcode.gcodeStorage, "writeGCode2_ToStorage start point..");

		//�ϳ��� paths�� �Ѿ ������ �ʿ��� ���� ����..//
		//����, paths�� for�� �ۿ��� �ʱ�ȭ..//
		double accumulatedPrintTime = 0.0;

		if (layerNr == 31)
			int x = 5;

		if (layerNr == 165)
			int x = 5;


		for (unsigned int n = 0; n<paths.size(); n++)
		{
			if (n != 0)
				initRaft = false;

			GCodePath* path = &paths[n];

			b_isToStorage = true;
			gcode.sectionPrintTime_Storage = 0.0;



			if (extruder != path->extruder)
			{
				extruder = path->extruder;

				int count_cart_idx = std::abs(extruder - 1);
				//TODO//
				if (gcode.standby_temperature_enabled[count_cart_idx] == true && layerNr == 31)
				{
					b_isToStorage = true;
					gcode.writeComment_ToStorage(b_isToStorage, &gcode.gcodeStorage, "writeGCode2_isToStorage_TRUE");
				}
				else
				{
					b_isToStorage = false;
					gcode.writeComment_ToStorage(b_isToStorage, &gcode.gcodeStorage, "writeGCode2_isToStorage_FALSE");
				}


				if (gcode.firstToolChanged[extruder] == false)
				{
					firstToolChangeInfo->push_back(extruder);
					firstToolChangeInfo->push_back(layerNr);

					//platform adhesion�� NONE�� ���, ���߿� ���� nozzle change�� firstToolChagneInfo size�� 4�� ����..
					//wrigeGcode�� �� ��, extruderNr�� �����Ǿ� ���� �ʱ� ������ toolchanging�� �ϸ鼭 �����ϱ� ����..
					//�� : T100 -> T0 -> T1
					//T0 -> T1�� ���� ����Ͽ� b_firstNONEStatus�� �ٽ� �ʱ�ȭ�� �ؾ� ��..
					if (firstToolChangeInfo->size() == 4 && gcode.platform_NONE == true)
					{
						b_firstToolChange = true;
						b_firstNONEStatus = false;
					}
					//platform adhesion�� Raft/Brim/Skirt�� ���, ���߿� ���� nozzle change�� firstToolChagneInfo size�� 2�� ����..
					//�� : T0 -> T1
					else if (firstToolChangeInfo->size() == 2 && gcode.platform_NONE == false)
					{
						b_firstToolChange = true;
						b_firstNONEStatus = false;
					}
					//T100���� T0�� T1�� �� ��쵵 üũ�Ͽ� switchExtruder���� �б⸦ �ؾ���.. NONE�� ���..//
					else if (firstToolChangeInfo->size() == 2 && gcode.platform_NONE == true)
					{
						b_firstNONEStatus = true;
					}

					gcode.firstToolChanged[extruder] = true;
				}

				if (b_firstToolChange)
				{
					char str[1000];
					sprintf_s(str, sizeof(str), "M104 T%i S%i		;Heat up the %ith nozzle\n", extruder, gcode.nozzle_print_temperature[extruder], extruder);
					//gcode.writeCode("");
					//gcode.writeCode(str);
					gcode.writeNewLine_ToStorage(b_isToStorage, &gcode.gcodeStorage);
					gcode.writeString_ToStorage(b_isToStorage, &gcode.gcodeStorage, str);
				}

				//standby temperature function..//
				for (int i = 0; i < gcode.cartridgeCountTotal; i++)
				{
					if (gcode.standby_temperature_enabled[i] && !b_firstToolChange)
					{
						char str[1000];
						if (extruder == i)
						{
							sprintf_s(str, sizeof(str), "M104 T%i S%i		;Heat up the %ith nozzle\n", i, gcode.nozzle_print_temperature[i], i);

							b_Off_CalPrintTime = true;
						}
						else
						{
							sprintf_s(str, sizeof(str), "M104 T%i S%i		;Heat up the %ith nozzle\n", i, gcode.nozzle_operating_standby_temperature[i], i);

							b_On_CalPrintTime = true;
						}

						gcode.writeNewLine_ToStorage(b_isToStorage, &gcode.gcodeStorage);
						gcode.writeString_ToStorage(b_isToStorage, &gcode.gcodeStorage, str);
					}
				}

				gcode.switchExtruder_ToStorage(b_isToStorage, &gcode.gcodeStorage, extruder, b_firstNONEStatus);

				if (b_firstToolChange)
				{
					gcode.writeComment_ToStorage(b_isToStorage, &gcode.gcodeStorage, "first toolchange setting");
					gcode.writeExtrusionReset_ToStorage(b_isToStorage, &gcode.gcodeStorage, -20);
					gcode.writeNewLine_ToStorage(b_isToStorage, &gcode.gcodeStorage);
				}

				toolChanged = true;
			}
			else if (path->retract)		// ���� else if�� �Ǿ��־���.
			{
				//gcode.writeRetraction();	// before moving
				gcode.writeRetraction_ToStorage(b_isToStorage, &gcode.gcodeStorage);
			}



			if (strcmp(path->config->name, "travel") != 0 && lastConfig != path->config)
			{
				gcode.writeComment_ToStorage(b_isToStorage, &gcode.gcodeStorage, "TYPE:%s", path->config->name);
				gcode.writeComment_ToStorage(b_isToStorage, &gcode.gcodeStorage, "EXTRUDER : %d", gcode.extruderNr);

				lastConfig = path->config;

				if (strcmp(lastConfig->name, "WIPE_TOWER") == 0)
				{
					gcode.b_lastConifg_WIPETOWER = true;
				}
				else
				{
					gcode.b_lastConifg_WIPETOWER = false;
				}
			}

			if (strcmp(path->config->name, "travel") == 0)
				gcode.current_mode = TRAVEL;
			else if (strcmp(path->config->name, "SKIRT") == 0)
				gcode.current_mode = SKIRT;
			else if (strcmp(path->config->name, "BRIM") == 0)
				gcode.current_mode = BRIM;
			else if (strcmp(path->config->name, "SKIN") == 0)
				gcode.current_mode = SKIN;
			else if (strcmp(path->config->name, "WALL-INNER") == 0)
				gcode.current_mode = WALL_INNER;
			else if (strcmp(path->config->name, "WALL-OUTER") == 0)
				gcode.current_mode = WALL_OUTER;
			else if (strcmp(path->config->name, "FILL") == 0)
				gcode.current_mode = FILL;
			else if (strcmp(path->config->name, "SUPPORT_MAIN") == 0)
				gcode.current_mode = SUPPORT_MAIN;
			else if (strcmp(path->config->name, "PRERETRACT0") == 0)
				gcode.current_mode = PRERETRACTION0;
			else if (strcmp(path->config->name, "OVERMOVE0") == 0)
				gcode.current_mode = OVERMOVE0;
			else if (strcmp(path->config->name, "PRERETRACTX") == 0)
				gcode.current_mode = PRERETRACTIONX;
			else if (strcmp(path->config->name, "OVERMOVEX") == 0)
				gcode.current_mode = OVERMOVEX;
			else if (strcmp(path->config->name, "RAFT") == 0)
				gcode.current_mode = RAFT;
			else if (strcmp(path->config->name, "SUPPORT_INTERFACE_ROOF") == 0)
				gcode.current_mode = SUPPORT_INTERFACE_ROOF;
			else if (strcmp(path->config->name, "SUPPORT_INTERFACE_FLOOR") == 0)
				gcode.current_mode = SUPPORT_INTERFACE_FLOOR;
			else if (strcmp(path->config->name, "WIPE_TOWER") == 0)
				gcode.current_mode = WIPE_TOWER;

			int speed = path->config->speed;

			if (path->config->lineWidth != 0)// Only apply the extrudeSpeedFactor to extrusion moves
				speed = speed * extrudeSpeedFactor / 100;
			else
				speed = speed * travelSpeedFactor / 100;

			if (path->points.size() == 1 && path->config != &pathconfigs_travel[currentExtruder] && shorterThen(gcode.getPositionXY() - path->points[0], path->config->lineWidth * 2))
			{
				//Check for lots of small moves and combine them into one large line
				Point p0 = path->points[0];
				unsigned int i = n + 1;
				while (i < paths.size() && paths[i].points.size() == 1 && shorterThen(p0 - paths[i].points[0], path->config->lineWidth * 2))
				{
					p0 = paths[i].points[0];
					i++;
				}
				if (paths[i - 1].config == &pathconfigs_travel[currentExtruder])
					i--;
				if (i > n + 2)
				{
					p0 = gcode.getPositionXY();
					for (unsigned int x = n; x<i - 1; x += 2)
					{
						int64_t oldLen = vSize(p0 - paths[x].points[0]);
						Point newPoint = (paths[x].points[0] + paths[x + 1].points[0]) / 2;
						int64_t newLen = vSize(gcode.getPositionXY() - newPoint);
						if (newLen > 0)
						{
							gcode.writeMove_ToStorage(b_isToStorage, &gcode.gcodeStorage, newPoint, speed, path->config->lineWidth * oldLen / newLen);

							//gcode.writeComment("TEST_3");
						}

						p0 = paths[x + 1].points[0];
					}
					gcode.writeMove_ToStorage(b_isToStorage, &gcode.gcodeStorage, paths[i - 1].points[0], speed, path->config->lineWidth);

					//gcode.writeComment("TEST_4");

					n = i - 1;
					continue;
				}
			}

			bool spiralize = path->config->spiralize;
			if (spiralize)
			{
				//Check if we are the last spiralize path in the list, if not, do not spiralize.
				for (unsigned int m = n + 1; m<paths.size(); m++)
				{
					if (paths[m].config->spiralize)
						spiralize = false;
				}
			}
			if (spiralize)
			{
				//If we need to spiralize then raise the head slowly by 1 layer as this path progresses.
				float totalLength = 0.0;
				int z = gcode.getPositionZ();
				Point p0 = gcode.getPositionXY();
				for (unsigned int i = 0; i<path->points.size(); i++)
				{
					Point p1 = path->points[i];
					totalLength += vSizeMM(p0 - p1);
					p0 = p1;
				}

				float length = 0.0;
				p0 = gcode.getPositionXY();
				for (unsigned int i = 0; i<path->points.size(); i++)
				{
					Point p1 = path->points[i];
					length += vSizeMM(p0 - p1);
					p0 = p1;
					gcode.setZ(z + layerThickness * length / totalLength);
					gcode.writeMove_ToStorage(b_isToStorage, &gcode.gcodeStorage, path->points[i], speed, path->config->lineWidth);
				}
			}
			else
			{
				if (initRaft && path->points.size() > 0)
				{
					//raft�� ��������.. brim�̳� skirt�� �� ���..raft�� 0�̹Ƿ� �ٸ� ������ �־���� �Ѵ�..
					//������ ���� ������ ���밪���� �־ ���� ��.. speed, lineWidth..
					//gcode.writeMove(path->points[0], gcode.raft_base_speed, gcode.raftBaseLineWidth);

					//DP201
					if (gcode.raftBaseFanControlEnabled)
					{
						gcode.writeMove_ToStorage(b_isToStorage, &gcode.gcodeStorage, path->points[0], 10, 1000);
					}
					else //DP200
					{
						gcode.writeMove_ToStorage(b_isToStorage, &gcode.gcodeStorage, path->points[0], speed, 0, true);
					}

					gcode.b_initialMoving = false;
				}

				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//main path points loop//
				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//initRaft true -> i = 1
				//initRaft false -> i = 0





				for (unsigned int i = initRaft ? 1 : 0; i<path->points.size(); i++)
				{
					if (gcode.b_initialMoving)
					{
						//DP201
						if (gcode.raftBaseFanControlEnabled)
						{
							gcode.writeMove_ToStorage(b_isToStorage, &gcode.gcodeStorage, path->points[0], 10, 1000);

							gcode.writeFanCommand_ToStorage(b_isToStorage, &gcode.gcodeStorage, gcode.extruderNr, 12.5);
						}
						gcode.b_initialMoving = false;
					}

					///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					//main path writing//
					gcode.writeMove_ToStorage(b_isToStorage, &gcode.gcodeStorage, path->points[i], speed, path->config->lineWidth);
					///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

					//toolchangeZlifting : currentPosition.z + toolchangeLoweringBed//
					if (toolChanged)
					{
						if (!(b_firstToolChange || b_firstNONEStatus))
						{
							//������ tool change retraction����ŭ���� �ʱ�ȭ ��//
							gcode.writeExtrusionReset_ToStorage(b_isToStorage, &gcode.gcodeStorage, (-1) * gcode.toolchangeRetractionAmount[gcode.getExtruderNr()]);
						}

						if (gcode.toolchangeExtraRestartAmount[gcode.getExtruderNr()] != 0)
						{
							//tool change�Ŀ� restart�� �ִ� ��� ��..cartridge���� �и��� �ؾ� �� //
							gcode.writeExtrusion_ToStorage(b_isToStorage, &gcode.gcodeStorage, gcode.toolchangeExtraRestartSpeed[gcode.getExtruderNr()], gcode.toolchangeExtraRestartAmount[gcode.getExtruderNr()]);
						}

						if (gcode.toolchangeZlifting != gcode.currentPosition.z)
						{
							gcode.writeBedLifting_ToStorage(b_isToStorage, &gcode.gcodeStorage, gcode.print_speed[extruder], gcode.currentPosition.z);
						}

						toolChanged = false;

						if (b_firstToolChange)
							b_firstToolChange = false;
					}



					//accumulatedPrintTime += calculateNaivePrintTotalTime(b_startFlag, b_endFlag, &path, pointIndex);

					//inserting preheat code..

				}
				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			}

			//gcode.updateSectionPrintTime_forStorage();



		}

		//writeMove���� ������ plan���� �Ѳ����� ���--> update
		gcode.updateTotalPrintTime();


		if (liftHeadIfNeeded && extraTime > 0.0)
		{
			gcode.writeComment_ToStorage(b_isToStorage, &gcode.gcodeStorage, "Small layer, adding delay of %f", extraTime);

			gcode.writeRetraction_ToStorage(b_isToStorage, &gcode.gcodeStorage, true);

			gcode.setZ(gcode.getPositionZ() + MM2INT(3.0));
			gcode.writeMove_ToStorage(b_isToStorage, &gcode.gcodeStorage, gcode.getPositionXY(), pathconfigs_travel.at(currentExtruder).speed, 0);
			//gcode.writeComment("TEST_7");

			gcode.writeMove_ToStorage(b_isToStorage, &gcode.gcodeStorage, gcode.getPositionXY() - Point(-MM2INT(20.0), 0), pathconfigs_travel.at(currentExtruder).speed, 0);
			//gcode.writeComment("TEST_8");

			gcode.writeDelay_ToStorage(b_isToStorage, &gcode.gcodeStorage, extraTime);
		}


		//TODO//
		if (layerNr == 31 && b_isToStorage)
			int x = 5;

		//storage clear..//
		gcode.gcodeStorage.clear();
	}

	double GCodePlanner::writeGcode_extrusionPathLength(bool _liftHeadIfNeeded, int _layerThickness, int _layerNr, bool _initRaft)
	{
		GCodePathConfig* lastConfig = nullptr;
		int extruder = gcode.getExtruderNr();

		double extrusionPathLength = 0.0;

		gcode.gcodePaths.paths.clear();
		for (unsigned int n = 0; n<paths.size(); n++)
		{
			if (n != 0)
				_initRaft = false;
			GCodePath* path = &paths[n];
			if (extruder != path->extruder)
			{
				extruder = path->extruder;
				gcode.switchExtruder(extruder);
			}
			else if (path->retract)		// ���� else if�� �Ǿ��־���.
			{
				gcode.writeRetraction();	// before moving
			}

			if (strcmp(path->config->name, "travel") != 0 && lastConfig != path->config)
			{
				gcode.writeComment("TYPE:%s", path->config->name);
				lastConfig = path->config;
			}

			if (strcmp(path->config->name, "travel") == 0)
				gcode.current_mode = TRAVEL;
			else if (strcmp(path->config->name, "SKIRT") == 0)
				gcode.current_mode = SKIRT;
			else if (strcmp(path->config->name, "BRIM") == 0)
				gcode.current_mode = BRIM;
			else if (strcmp(path->config->name, "SKIN") == 0)
				gcode.current_mode = SKIN;
			else if (strcmp(path->config->name, "WALL-INNER") == 0)
				gcode.current_mode = WALL_INNER;
			else if (strcmp(path->config->name, "WALL-OUTER") == 0)
				gcode.current_mode = WALL_OUTER;
			else if (strcmp(path->config->name, "FILL") == 0)
				gcode.current_mode = FILL;
			else if (strcmp(path->config->name, "SUPPORT_MAIN") == 0)
				gcode.current_mode = SUPPORT_MAIN;
			else if (strcmp(path->config->name, "PRERETRACT0") == 0)
				gcode.current_mode = PRERETRACTION0;
			else if (strcmp(path->config->name, "OVERMOVE0") == 0)
				gcode.current_mode = OVERMOVE0;
			else if (strcmp(path->config->name, "PRERETRACTX") == 0)
				gcode.current_mode = PRERETRACTIONX;
			else if (strcmp(path->config->name, "OVERMOVEX") == 0)
				gcode.current_mode = OVERMOVEX;
			else if (strcmp(path->config->name, "RAFT") == 0)
				gcode.current_mode = RAFT;
			else if (strcmp(path->config->name, "SUPPORT_INTERFACE_ROOF") == 0)
				gcode.current_mode = SUPPORT_INTERFACE_ROOF;
			else if (strcmp(path->config->name, "SUPPORT_INTERFACE_FLOOR") == 0)
				gcode.current_mode = SUPPORT_INTERFACE_FLOOR;
			else if (strcmp(path->config->name, "WIPE_TOWER") == 0)
				gcode.current_mode = WIPE_TOWER;

			int speed = path->config->speed;

			if (path->config->lineWidth != 0)// Only apply the extrudeSpeedFactor to extrusion moves
				speed = speed * extrudeSpeedFactor / 100;
			else
				speed = speed * travelSpeedFactor / 100;

			if (path->points.size() == 1 && path->config != &pathconfigs_travel[currentExtruder] && shorterThen(gcode.getPositionXY() - path->points[0], path->config->lineWidth * 2))
			{
				//Check for lots of small moves and combine them into one large line
				Point p0 = path->points[0];
				unsigned int i = n + 1;
				while (i < paths.size() && paths[i].points.size() == 1 && shorterThen(p0 - paths[i].points[0], path->config->lineWidth * 2))
				{
					p0 = paths[i].points[0];
					i++;
				}
				if (paths[i - 1].config == &pathconfigs_travel[currentExtruder])
					i--;
				if (i > n + 2)
				{
					p0 = gcode.getPositionXY();
					for (unsigned int x = n; x<i - 1; x += 2)
					{
						int64_t oldLen = vSize(p0 - paths[x].points[0]);
						Point newPoint = (paths[x].points[0] + paths[x + 1].points[0]) / 2;
						int64_t newLen = vSize(gcode.getPositionXY() - newPoint);
						if (newLen > 0)
						{
							gcode.writeMove(newPoint, speed, path->config->lineWidth * oldLen / newLen, _layerNr, gcode.b_spiralize);
						}

						p0 = paths[x + 1].points[0];
					}
					gcode.writeMove(paths[i - 1].points[0], speed, path->config->lineWidth, _layerNr, gcode.b_spiralize);
					n = i - 1;
					continue;
				}
			}

			bool spiralize = path->config->spiralize;
			if (spiralize)
			{
				//Check if we are the last spiralize path in the list, if not, do not spiralize.
				for (unsigned int m = n + 1; m<paths.size(); m++)
				{
					if (paths[m].config->spiralize)
						spiralize = false;
				}
			}
			if (spiralize)
			{
				//If we need to spiralize then raise the head slowly by 1 layer as this path progresses.
				float totalLength = 0.0;
				int z = gcode.getPositionZ();
				Point p0 = gcode.getPositionXY();
				for (unsigned int i = 0; i<path->points.size(); i++)
				{
					Point p1 = path->points[i];
					totalLength += vSizeMM(p0 - p1);
					p0 = p1;
				}

				float length = 0.0;
				p0 = gcode.getPositionXY();
				for (unsigned int i = 0; i<path->points.size(); i++)
				{
					Point p1 = path->points[i];
					length += vSizeMM(p0 - p1);
					p0 = p1;
					gcode.setZ(z + _layerThickness * length / totalLength);
					gcode.writeMove(path->points[i], speed, path->config->lineWidth, _layerNr, gcode.b_spiralize);

				}
			}
			else
			{
				if (_initRaft && path->points.size() > 0)
				{
					//iniraft�̸鼭 ���� �ʱ������..
					//DP201
					//if (gcode.raftBaseFanControlEnabled)
					//{
					//	gcode.writeMove(path->points[0], 10, 1000);
					//	gcode.writeFanCommand(gcode.extruderNr, 12.5);
					//}
					//else
					//{
					//	gcode.writeMove(path->points[0], speed, 0, true);
					//}
					
					if (gcode.platformAdhesion == Generals::PlatformAdhesion::Raft)	gcode.writeMove(path->points[0], gcode.raftBaseSpeed, gcode.initial_layer_extrusion_width[extruder], _layerNr, gcode.b_spiralize);
					else gcode.writeMove(path->points[0], gcode.initial_layer_speed[extruder], gcode.initial_layer_extrusion_width[extruder], _layerNr, gcode.b_spiralize);

					gcode.b_initialMoving = false;
				}

				for (unsigned int i = _initRaft ? 1 : 0; i<path->points.size(); i++)
				{
					//extrusion�� �κи� path ���̸� ���� ��Ŵ..//swyang
					extrusionPathLength += gcode.writeMove_extrusionPathLength(path->points[i], speed, path->config->lineWidth, _layerNr);
				}
			}
		}

		gcode.updateTotalPrintTime();
		if (_liftHeadIfNeeded && extraTime > 0.0)
		{
			gcode.writeComment("Small layer, adding delay of %f", extraTime);
			gcode.writeRetraction(true);
			gcode.setZ(gcode.getPositionZ() + MM2INT(3.0));
			gcode.writeMove(gcode.getPositionXY(), pathconfigs_travel.at(currentExtruder).speed, 0, _layerNr, gcode.b_spiralize);
			gcode.writeMove(gcode.getPositionXY() - Point(-MM2INT(20.0), 0), pathconfigs_travel.at(currentExtruder).speed, 0, _layerNr, gcode.b_spiralize);
			gcode.writeDelay(extraTime);
		}

		return extrusionPathLength;
	}

	double GCodePlanner::calculateNaivePrintTotalTime(bool b_startFlag, bool b_endFlag, GCodePath* path, int pointIndex)
	{
		Point startPoint;
		if (pointIndex == 0) startPoint = path->points[pointIndex];
		else startPoint = path->points[pointIndex - 1];

		Point targetPoint = path->points[pointIndex];

		double segmentPrintTime;
		double retractedTravelTime = 0.0;

		double pathLength = vSizeMM(startPoint - targetPoint);
		segmentPrintTime = pathLength / path->config->speed;

		//retraction�� �ϴ� ���߿� �����սô�..//TODO//
		//if (gcode.current_mode == TRAVEL && path->retract)
		//{
		//	retractedTravelTime = gcode.retraction_amount[path->extruder] / gcode.retractionSpeed[path->extruder];

		//	segmentPrintTime += retractedTravelTime;
		//}

		return segmentPrintTime;
	}


	double GCodePlanner::calculatePathPrintTime(GCodePath* path, bool liftHeadIfNeeded, int layerThickness, int layerNr, int pathIndex, bool initRaft)
	{
		//path segment print time�� �����--> moving���� estimate_time calculate�� ����//

		gcode.gcodePaths.paths.clear();
		gcode.totalPrintTime_Storage = 0.0;

		gcode.writeComment_ToStorage(true, &gcode.gcodeStorage, ";calculate part print_end_time start...");

		gcode.resetPathPrintTime_timeCalculate();

		if (path->retract)		// ���� else if�� �Ǿ��־���.
		{
			//gcode.writeRetraction();	// before moving
			gcode.writeRetraction_timeCalculate();
		}

		if (strcmp(path->config->name, "travel") == 0)
			gcode.current_mode = TRAVEL;
		else if (strcmp(path->config->name, "SKIRT") == 0)
			gcode.current_mode = SKIRT;
		else if (strcmp(path->config->name, "BRIM") == 0)
			gcode.current_mode = BRIM;
		else if (strcmp(path->config->name, "SKIN") == 0)
			gcode.current_mode = SKIN;
		else if (strcmp(path->config->name, "WALL-INNER") == 0)
			gcode.current_mode = WALL_INNER;
		else if (strcmp(path->config->name, "WALL-OUTER") == 0)
			gcode.current_mode = WALL_OUTER;
		else if (strcmp(path->config->name, "FILL") == 0)
			gcode.current_mode = FILL;
		else if (strcmp(path->config->name, "SUPPORT_MAIN") == 0)
			gcode.current_mode = SUPPORT_MAIN;
		else if (strcmp(path->config->name, "PRERETRACT0") == 0)
			gcode.current_mode = PRERETRACTION0;
		else if (strcmp(path->config->name, "OVERMOVE0") == 0)
			gcode.current_mode = OVERMOVE0;
		else if (strcmp(path->config->name, "PRERETRACTX") == 0)
			gcode.current_mode = PRERETRACTIONX;
		else if (strcmp(path->config->name, "OVERMOVEX") == 0)
			gcode.current_mode = OVERMOVEX;
		else if (strcmp(path->config->name, "RAFT") == 0)
			gcode.current_mode = RAFT;
		else if (strcmp(path->config->name, "SUPPORT_INTERFACE_ROOF") == 0)
			gcode.current_mode = SUPPORT_INTERFACE_ROOF;
		else if (strcmp(path->config->name, "SUPPORT_INTERFACE_FLOOR") == 0)
			gcode.current_mode = SUPPORT_INTERFACE_FLOOR;
		else if (strcmp(path->config->name, "WIPE_TOWER") == 0)
			gcode.current_mode = WIPE_TOWER;

		int speed = path->config->speed;

		if (path->config->lineWidth != 0)// Only apply the extrudeSpeedFactor to extrusion moves
			speed = speed * extrudeSpeedFactor / 100;
		else
			speed = speed * travelSpeedFactor / 100;


		bool spiralize = path->config->spiralize;
		if (spiralize)
		{
			//If we need to spiralize then raise the head slowly by 1 layer as this path progresses.
			float totalLength = 0.0;
			int z = gcode.getPositionZ_timeCalculate();
			Point p0 = gcode.getPositionXY_timeCalculate();
			for (unsigned int i = 0; i<path->points.size(); i++)
			{
				Point p1 = path->points[i];
				totalLength += vSizeMM(p0 - p1);
				p0 = p1;
			}

			float length = 0.0;
			p0 = gcode.getPositionXY_timeCalculate();
			for (unsigned int i = 0; i<path->points.size(); i++)
			{
				Point p1 = path->points[i];
				length += vSizeMM(p0 - p1);
				p0 = p1;
				gcode.setZ_timeCalculate(z + layerThickness * length / totalLength);
				gcode.writeMove_timeCalculate(path->points[i], speed, path->config->lineWidth, layerNr);
			}
		}
		else
		{
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//main path points loop//
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//initRaft true -> i = 1
			//initRaft false -> i = 0
			for (unsigned int i = initRaft ? 1 : 0; i<path->points.size(); i++)
			{
				//main path writing//
				gcode.writeMove_timeCalculate(path->points[i], speed, path->config->lineWidth, layerNr);
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}


		if (liftHeadIfNeeded && extraTime > 0.0)
		{
			gcode.writeRetraction_timeCalculate(true);

			gcode.setZ_timeCalculate(gcode.getPositionZ_timeCalculate() + MM2INT(3.0));
			gcode.writeMove_timeCalculate(gcode.getPositionXY_timeCalculate(), pathconfigs_travel.at(currentExtruder).speed, 0, layerNr);

			gcode.writeMove_timeCalculate(gcode.getPositionXY_timeCalculate() - Point(-MM2INT(20.0), 0), pathconfigs_travel.at(currentExtruder).speed, 0, layerNr);
		}

		gcode.updatePathPrintTime_timeCalculate();

		return gcode.pathPrintTime_timeCalculate;
	}







}//namespace engine
