/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#pragma once

#include "settings.h"
#include "comb.h"
#include "intpoint.h"
#include "polygon.h"
#include "timeEstimate.h"

//#define round(fp) (int)((fp) >= 0 ? (fp) + 0.5 : (fp) - 0.5)

enum PathMode { NA = -1, TRAVEL, SKIRT, BRIM, SKIN, WALL_INNER, WALL_OUTER, FILL, SUPPORT_MAIN, PRERETRACTION0, OVERMOVE0, PRERETRACTIONX, OVERMOVEX, RAFT, SUPPORT_INTERFACE_ROOF, SUPPORT_INTERFACE_FLOOR, WIPE_TOWER, EXTRUDER_T0, EXTRUDER_T1, ADJUST_Z_GAP };
namespace engine {

	//The GCodeExport class writes the actual GCode. This is the only class that knows how GCode looks and feels.
	//  Any customizations on GCodes flavors are done in this class.



	struct pathG
	{
		vector<FPoint3> poly;
		PathMode mode;
		int extruderNr;
	};

	struct LayerPathG
	{
		vector<pathG> paths;
	};

	struct GCodeSectionStorage
	{
		std::string sectionGcodeStrings;

		double sectionEndPrintTime;
	};

	class PathPrintTimeInfo_accumulated
	{
	public:
		int pathIndex;
		double pathPrintTime_accumulated;
		int pathExtruderNr;

		void clear()
		{
			pathIndex = 0;
			pathPrintTime_accumulated = 0.0;
		};
	};

	class GCodeStorage
	{
	public:
		std::vector<GCodeSectionStorage> gcodeSectionList;

		std::string gcodeStrings_original;
		std::string gcodeStrings_modified;

		int layerNr;
		double totalPrintTime;
		double sectionPrintTime;
		double linePrintTime;

		void clear();
	};

	class GCodeExport
	{
	public:
		FILE* f;

		GCodeStorage gcodeStorage;

		double supportAmount;
		double supportAmount_timeCalculate;
		double extrusionAmount;
		double extrusionAmount_timeCalculate;	//TODO//for time calculate..

		//extrusion_per_MM//
		double extrusion_per_MM_overall[MAX_EXTRUDERS];
		double extrusion_per_MM_initial_layer[MAX_EXTRUDERS];
		double extrusion_per_MM_infill[MAX_EXTRUDERS];
		double extrusion_per_MM_outer_wall[MAX_EXTRUDERS];
		double extrusion_per_MM_inner_wall[MAX_EXTRUDERS];
		double extrusion_per_MM_top_bottom[MAX_EXTRUDERS];
		double extrusion_per_MM_support_main;
		double extrusion_per_MM_support_interface_roof;
		double extrusion_per_MM_support_interface_floor;
		//double extrusion_per_MM_skirt;
		//double extrusion_per_MM_brim;
		double extrusion_per_MM_wipe_tower;

		double retractionAmount[MAX_EXTRUDERS];
		double retractionAmountPrime[MAX_EXTRUDERS];
		int retractionZHop[MAX_EXTRUDERS];
		//double extruderSwitchRetraction;
		double minimalExtrusionBeforeRetraction[MAX_EXTRUDERS];
		double extrusionAmountAtPreviousRetraction;
		double extrusionAmountAtPreviousRetraction_timeCalculate; //TODO//for time calculate..


		//tool_change//
		double toolchangeRetractionAmount[MAX_EXTRUDERS];
		int	toolchangeRetractionSpeed[MAX_EXTRUDERS];
		double toolchangeExtraRestartAmount[MAX_EXTRUDERS];
		int toolchangeExtraRestartSpeed[MAX_EXTRUDERS];
		double toolchangeZlifting;
		double toolchangeLoweringBed;

		//bool T0_firstToolChanged;
		//bool T1_firstToolChanged;
		bool firstToolChanged[MAX_EXTRUDERS];

		int platformAdhesion;

		//int T0_material;
		//int T1_material;
		//int material[MAX_EXTRUDERS];



		//config value//
		double layerHeight;
		
		int travel_speed[MAX_EXTRUDERS];
		int print_speed[MAX_EXTRUDERS];
		//double bottomLayerSpeed;
		//double bottomLayerExtrusionWidth;
		int initial_layer_speed[MAX_EXTRUDERS];
		int initial_layer_extrusion_width[MAX_EXTRUDERS];

		bool b_multiExtruder;
		bool b_spiralize;

		int nozzle_print_temperature[MAX_EXTRUDERS];
		int bed_temperature;
		bool standby_temperature_enabled[MAX_EXTRUDERS];
		int nozzle_operating_standby_temperature[MAX_EXTRUDERS];
		int nozzle_initial_standby_temperature[MAX_EXTRUDERS];
		//double nozzle_layer0_temperature[MAX_EXTRUDERS];
		//bool nozzle_layer0_temperature_enabled[MAX_EXTRUDERS];
		int nozzle_current_temperature[MAX_EXTRUDERS];
		std::vector<bool> temperature_layer_setting_enabled;

		//TemperatureProfile temperatureProfile;
		std::vector< std::vector<TemperatureLayerSetPoint> > temperature_layer_list;


		std::vector<bool> b_usesCartridgeState_index;

		//preheat information//
		bool preheat_OR_enabled = false;
		bool preheat_enabled[MAX_EXTRUDERS];
		double preheat_threshold_time[MAX_EXTRUDERS];
		double preheat_temperature_falling_rate[MAX_EXTRUDERS];
		double preheat_temperature_rising_rate[MAX_EXTRUDERS];

		//std::vector<double> temperature_print;
		//std::vector<double> temperature_standby;
		//std::vector<double> rate_temperature_fall;
		//std::vector<double> rate_temperature_rise;
		//std::vector<double> time_threshold;
		std::vector<double> time_end;
		std::vector<double> time_standby;
		std::vector<double> time_preheat;

		//double T0_nozzleTemperautre;
		//double T1_nozzleTemperautre;
		//double T0_nozzleStandByTemperature;
		//double T1_nozzleStandByTemperature;


		int cartridgeCountTotal;

		//config value//
		double raftBaseSpeed;
		double raftBaseLineWidth;
		bool raftBaseFanControlEnabled;
		bool raftTemperatureControl;
		int raftSurfaceLastTemperature;
		bool b_initialMoving;
		bool platform_NONE;
		bool b_notExtruderSet;

		//initLayer//
		bool limitBedlifting;

		//wipe tower bed lifting flag..//
		bool b_wipeTower_bedLifting;
		bool b_lastConifg_WIPETOWER;

		std::string preSwitchExtruderCode[MAX_EXTRUDERS];
		std::string postSwitchExtruderCode[MAX_EXTRUDERS];


		Point3 currentPosition;
		Point3 currentPosition_timeCalculate;
		Point3 startPosition;
		Point3 startPosition_timeCalculate;

		Point extruderOffset[MAX_EXTRUDERS];
		//char extruderCharacter[MAX_EXTRUDERS];
		char extruderCharacter;

		int currentSpeed;
		int currentSpeed_timeCalculate;
		int	retractionSpeed[MAX_EXTRUDERS];
		int zPos;
		int zPos_timeCalculate;
		bool isRetracted;
		bool isRetracted_timeCalculate;
		bool isPreRetracted;
		int extruderNr;
		int currentFanSpeed[MAX_EXTRUDERS];
		//int flavor;

		LayerPathG gcodePaths;
		LayerPathG gcodePaths_timeCalculate;
		PathMode current_mode;
		PathMode current_mode_last;

		double totalFilament[MAX_EXTRUDERS];
		double totalPrintTime;
		double totalPrintTime_Storage;
		double totalPrintTime_timeCalculate;
		double sectionPrintTime_Storage;

		double pathPrintTime_timeCalculate;
		std::vector<double> pathPrintTime_accumulated; // parts = path + path + ...//
		std::vector<double> pathEndPrintTime;

		TimeEstimateCalculator estimateCalculator;
		TimeEstimateCalculator estimateCalculator_forStorage;
		TimeEstimateCalculator estimateCalculator_timeCalculate;

		//layer number NA value//
		static const int LAYER_NUMBER_NA = -1;


	public:

		GCodeExport();

		~GCodeExport();

		void clear();
		void setConfig(vector<ConfigSettings> p_configs);

		void setExtruderOffset(int id, Point p);

		void setSwitchExtruderCode(std::string preSwitchExtruderCode, std::string postSwitchExtruderCode, int index);

		void setToolChangeSettings(int toolchangeRetractionAmount, int toolchangeRetractionSpeed, int toolchangeExtraRestartAmount, int toolchangeExtraRestartSpeed, int index);

		void setFlavor();
		//int getFlavor();

		//void setFilename(const char* filename);
		void setFilename(const wchar_t* filename);

		bool isOpened();


		//layerFlow부분을 어떻게 추가 할 것인가..??//
		//void setExtrusion(int layerThickness, int filamentDiameter, std::vector<int> layerFlow, int wipeTowerFlow);
		void setExtrusion(int layerThickness, int filamentDiameter, ConfigsSetForFlow _configs_set_flow);

		void setRetractionSettings(int retractionAmount, int retractionSpeed, int minimalExtrusionBeforeRetraction, int zHop, int retractionAmountPrime, int index);

		void setZ(int z);

		void setZ_timeCalculate(int z);

		Point getPositionXY();

		Point getPositionXY_timeCalculate();

		void resetStartPosition();

		Point getStartPositionXY();

		int getPositionZ();

		int getPositionZ_timeCalculate();

		int getExtruderNr();

		double getTotalFilamentUsed(int e);

		double getTotalPrintTime();

		double getExtrusionPerMM(PathMode _current_mode);
	
		void resetTotalPrintTime();

		void updateTotalPrintTime();
		//void updateTotalPrintTime_timeCalculate();


		void writeNewLine();

		void writeComment(const char* comment, ...);

		void writeLine(const char* line, ...);

		void resetExtrusionValue();

		void writeDelay(double timeAmount);

		void writeMove(Point p, int speed, int lineWidth, int layerNr, bool b_spiralize, bool initRaft = false);

		void writeMove_toolchanged(Point p, int speed, int lineWidth, int layerNr, bool b_spiralize, bool b_toolchanged, bool initRaft = false);

		double writeMove_extrusionPathLength(Point p, int speed, int lineWidth, int layerNr, bool initRaft = false);

		void writeRetraction(bool force = false);

		void switchExtruder(int newExtruder);

		void writeCode(const char* str);

		void writeExtrusion(int speed, double amount);

		void writeExtrusionReset(double amount);

		//void writeFanCommand(double speed);

		void writeFanCommand(int cartridgeIndex, double speed);

		void writeBedLifting(double speed, double position_z);

		void finalize();

		int getFileSize();
		void tellFileSize();

		/////////////////////////////////////////////////////////
		//write to std::string//
		void resetExtrusionValue_ToString(bool b_isToStorage, GCodeStorage *storage);

		void switchExtruder_ToStorage(bool b_isToStorage, GCodeStorage* storage, int newExtruder, bool b_firstNONEstatus = false);

		void writeNewLine_ToStorage(bool b_isToStorage, GCodeStorage* storage);

		void writeMove_ToStorage(bool b_isToStorage, GCodeStorage* storage, Point p, int speed, int lineWidth, bool initRaft = false);

		void writeRetraction_ToStorage(bool b_isToStorage, GCodeStorage* storage, bool force = false);

		void writeFanCommand_ToStorage(bool b_isToStorage, GCodeStorage* storage, int cartridgeIndex, double speed);

		void writeBedLifting_ToStorage(bool b_isToStorage, GCodeStorage* storage, double speed, double position_z);

		void writeComment_ToStorage(bool b_isToStorage, GCodeStorage* storage, const char* comment, ...);

		void writeString_ToStorage(bool b_isToStorage, GCodeStorage* storage, const char* str, ...);

		void writeCode_ToStorage(bool b_isToStorage, GCodeStorage* storage, const char* str, ...);

		void writeExtrusion_ToStorage(bool b_isToStorage, GCodeStorage* storage, int speed, double amount);

		void writeExtrusionReset_ToStorage(bool b_isToStorage, GCodeStorage* storage, double amount);

		void writeDelay_ToStorage(bool b_isToStorage, GCodeStorage* storage, double timeAmount);

		/////////////////////////////////////////////////////////
		//for time calculating//

		void writeMove_timeCalculate(Point p, int speed, int lineWidth, int layerNr, bool initRaft = false);

		void writeRetraction_timeCalculate(bool force = false);

		void resetExtrusionValue_timeCalculate();

		void updatePathPrintTime_timeCalculate();

		void resetPathPrintTime_timeCalculate();

		void clear_timeCalculate();
		
		void reset_pathPrintTime_pathEndPrintTime();

	};

	//The GCodePathConfig is the configuration for moves/extrusion actions. This defines at which width the line is printed and at which speed.
	class GCodePathConfig
	{
	public:
		int speed;
		int lineWidth;
		const char* name;
		bool spiralize;

		GCodePathConfig() : speed(0), lineWidth(0), name(nullptr), spiralize(false) {}
		GCodePathConfig(int speed, int lineWidth, const char* name, bool spiralize) : speed(speed), lineWidth(lineWidth), name(name), spiralize(spiralize) {}

		void setData(int speed, int lineWidth, const char* name, bool spiralize)
		{
			this->speed = speed;
			this->lineWidth = lineWidth;
			this->name = name;
			this->spiralize = spiralize;
		}
	};

	class GCodePath
	{
	public:
		GCodePathConfig* config;
		bool retract;
		int extruder;
		vector<Point> points;
		bool done;//Path is finished, no more moves should be added, and a new path should be started instead of any appending done to this one.
	};

	//The GCodePlanner class stores multiple moves that are planned.
	// It facilitates the combing to keep the head inside the print.
	// It also keeps track of the print time estimate for this planning so speed adjustments can be made for the minimal-layer-time.
	class GCodePlanner
	{
	public:
		GCodeExport& gcode;

		int pathOptimizationParameter[3];
		int filteringAngle;

		Point prevPosition;
		Point lastPosition;

		bool retractionWithComb;

		std::vector<GCodePathConfig> pathconfigs_travel;
		std::vector<int> retractionMinimalDistance;

		Point startPoint;

		std::vector<PathPrintTimeInfo_accumulated> pathPrintTimeInfo_accumulated_vector;

	private:
		vector<GCodePath> paths;
		Comb* comb;

		std::vector<GCodePathConfig> preretractionConfigs;
		int extrudeSpeedFactor;
		int travelSpeedFactor;
		int currentExtruder;

		bool forceRetraction;
		bool alwaysRetract;
		double extraTime;
		double totalPrintTime;
		bool b_isToolchangedInLayer = false;


	private:
		GCodePath* getLatestPathWithConfig(GCodePathConfig* config);
		GCodePath* getLatestPathWithConfigAndUserExtruder(GCodePathConfig* config, int userExtruder);
		void forceNewPathStart();
	public:
		GCodePlanner(GCodeExport& gcode, std::vector<int> configsTravelSpeed, std::vector<int> configsRetractionMinimalDistance, bool spiralize);
		~GCodePlanner();

		vector<GCodePath>* getPaths()
		{
			return &paths;
		}

		bool setExtruder(int extruder)
		{
			if (extruder == currentExtruder)
				return false;
			currentExtruder = extruder;
			return true;
		}

		int getExtruder()
		{
			return currentExtruder;
		}

		void setCombBoundary(Polygons* polygons)
		{
			if (comb)
				delete comb;
			if (polygons)
				comb = new Comb(*polygons);
			else
				comb = nullptr;
		}

		void setAlwaysRetract(bool alwaysRetract)
		{
			this->alwaysRetract = alwaysRetract;
		}

		void forceRetract()
		{
			forceRetraction = true;
		}

		void setExtrudeSpeedFactor(int speedFactor)
		{
			if (speedFactor < 1) speedFactor = 1;
			this->extrudeSpeedFactor = speedFactor;
		}
		int getExtrudeSpeedFactor()
		{
			return this->extrudeSpeedFactor;
		}
		void setTravelSpeedFactor(int speedFactor)
		{
			if (speedFactor < 1) speedFactor = 1;
			this->travelSpeedFactor = speedFactor;
		}
		int getTravelSpeedFactor()
		{
			return this->travelSpeedFactor;
		}

		void addTravel(Point p);
		void addTravelWithUserExtruder(Point p, int userExtruder);

		void addExtrusionMove(Point p, GCodePathConfig* config);
		void addExtrusionMoveWithUserExturder(Point p, GCodePathConfig* config, int userExtruder);

		void moveInsideCombBoundary(int distance);

		void addPolygon(PolygonRef polygon, int startIdx, GCodePathConfig* config);
		void addPolygonWithUserExtruder(PolygonRef polygon, int startIdx, GCodePathConfig* config, int userExtruder);
		void addPolygonWithPreretraction(PolygonRef polygon, int startIdx, GCodePathConfig* print_config, GCodePathConfig* retract_config, int length);
		void addPolygonWithOverMove(PolygonRef polygon, int startIdx, GCodePathConfig* overmove_config, int length);
		void addPolygonWithPreretractionAndOvermoving(PolygonRef polygon, int startIdx, GCodePathConfig* print_config, GCodePathConfig* retract_config, GCodePathConfig* overmove_config, int pre, int over);

		void addPolygonsByOptimizer(Polygons& polygons, GCodePathConfig* config);
		void addPolygonsByOptimizerWithUserExtruder(Polygons& polygons, GCodePathConfig* config, int userExtruder);
		void addPolygonsByOptimizer_insetToinfill_filteredTheta(Polygons& polygons, GCodePathConfig* config);
		void addPolygonsByOptimizerWithPreretractionAndOvermoving(Polygons& polygons, GCodePathConfig* print_config, GCodePathConfig* retract_config, GCodePathConfig* overmove_config, int pre, int over);

		void forceMinimalLayerTime(std::vector<double> configsMinTime, std::vector<int> configsMinimalSpeed);

		void writeGCode(bool liftHeadIfNeeded, int layerThickness, int layerNr, bool initRaft = false);

		double writeGcode_extrusionPathLength(bool liftHeadIfNeeded, int layerThickness, int layerNr, bool initRaft = false);

		//void writeGCode2(bool liftHeadIfNeeded, int layerThickness, int layerNr, std::vector<int>* firstToolChangeInfo, bool initRaft = false);

		void writeGCode2_ToStorage(bool liftHeadIfNeeded, int layerThickness, int layerNr, std::vector<int>* firstToolChangeInfo, bool initRaft = false);

		void writeGCode2(bool liftHeadIfNeeded, int layerThickness, int layerNr, std::vector<int>* firstToolChangeInfo, std::vector<PathPrintTimeInfo_accumulated> partsPrintTime_vector, std::vector<double> preheatStartTime, bool initRaft = false);

		double calculatePathPrintTime(GCodePath* path, bool liftHeadIfNeeded, int layerThickness, int layerNr, int pathIndex, bool initRaft);

		double calculateNaivePrintTotalTime(bool b_startFlag, bool b_endFlag, GCodePath* path, int pointIndex);

		void getPathPrintTimeInfo(bool liftHeadIfNeeded, int layerThickness, int layerNr, bool initRaft = false);

		std::vector<double> getPreheatStartTime();

	};

}//namespace engine