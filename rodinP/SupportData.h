/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#pragma once
#include "intpoint.h"
#include "polygon.h"
#include "AABB.h"

class ModelContainer;
class IMeshModel;
class Mesh;
class SupportInfo
{
public:
	int32_t z;
	double cosAngle;
	bool overhang;
	SupportInfo(int32_t z, double cosAngle, bool  overhang) : z(z), cosAngle(cosAngle), overhang(overhang) {}
};
class SupportData
{
public:
	SupportData();
	~SupportData();

	bool generated;
	bool b_changed;
	//bool enable;
	int angle;
	bool everywhere;
	int XYDistance;
	int ZDistance;
	int horizontalExpansion;
	int layerHeight;

	void clear()
	{
		generated = false;
		//enable = true;
		if (grid)
		{
			delete[] grid;
			grid = nullptr;
		}

		if (b_grid)
		{
			delete[] b_grid;
			b_grid = nullptr;
		}

	}

	Point gridOffset;
	int32_t gridScale;
	int32_t gridWidth, gridHeight;
	engine::Polygons outlines;
	std::vector<SupportInfo>* grid;
	std::vector<bool>* b_grid;
	std::vector<bool> b_inside;

	bool checkOverhang();
	void checkNeedSupport();
	void generateSupportGrid(std::vector<IMeshModel*> models_, int supportAngle, bool supportEverywhere, int supportXYDistance, int supportZDistance, int supportHorizontalExpansion, int layerThickness);
	void generateSupportGrid(Mesh* mesh_);
	double calSupportVol(IMeshModel * model_, int supportAngle, bool supportEverywhere, int supportXYDistance, int supportZDistance, int supportHorizontalExpansion, double& overhangArea);

	//for edit
	void init();
	void setZ(int32_t Z);
	void setRadius(int r);
	int32_t getZ();
	Point3 getPoint(int x, int y);
	int getType(int x, int y);
	int getType(int n);
	void enable(double x, double y);
	void disable(double x, double y);
	void deleteAllsupports();
	void resetAllsupports(int mode);
public slots:
	void modelChanged() { b_changed = true; }
private:
	int32_t m_z;
	int radius;
	int* m_type;
	int checkSupportType(Point p);
};


class SupportPolygonGenerator
{
public:
	SupportPolygonGenerator(SupportData *supportData_);
	engine::Polygons generateSupportPolygons(int32_t z_);
private:
	SupportData* supportData;
	engine::Polygons polygons;
	int* done;
	int nr;
	int32_t z;

	bool needSupportAt(Point p);
	void lazyFill(Point startPoint);
};


struct segment_Info
{
	int polyNr;
	int64_t x, y;
	int st_Line_idx, end_Line_idx;
	int level;
};

class NEWSupport
{
public:
	NEWSupport(engine::Polygons* _in_outline, engine::Polygons* _result, int _lineSpacing, int _extrusionWidth, int _infillOverlap, double _rotation, int _layerNr) : in_outline(_in_outline), result(_result), lineSpacing(_lineSpacing), extrusionWidth(_extrusionWidth), infillOverlap(_infillOverlap), rotation(_rotation), layerNr(_layerNr) {};
	~NEWSupport() {};

public:
	void generateNEWSupport(int& _minX, int& _minY, int& _segmentLength, int _partition_length, int _spacingLength, bool _b_partition);
	void initParams(int& _minX, int& _minY, int& _segmentLength, int _partition_length, int _spacingLength);
private:
	void init(int& _minX, int& _minY, int& _segmentLength, int _partition_length, int _spacingLength, bool _b_partition);
	void registerCutList();
	void modifyCutList();
	void partitioning(bool _b_partition);
	void connect();

	void connectSplittedLines(int idx, int x);
	void findClosestIdx(int& closest_pidx, Point& from_pt, const int idx, const int b_even);
	void connectOutline(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x, int b_even);

	bool betweenSplitSpace(const Point& before_p, const Point& next_p);
	void lineConnect(const Point& before_p, const Point& next_p);
	void sameIdxLineConnect(const Point& before_p, const Point& next_p);
	void diffZLineConnect(const Point& before_p, const Point& next_p);

	bool isOnSameOutlineEdge(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x);
	double getDistance(Point& a, Point& b);

	void connect_OUTLINE_OUTLINE(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x, int b_even);
	void connect_OUTLINE_INTERIOR(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x, int b_even);
	void connect_INTERIOR_INTERIOR(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x, int b_even);

private:
	engine::Polygons* in_outline;
	engine::Polygons* result;
	AABB2D boundary;

	int lineSpacing;
	int extrusionWidth;
	int infillOverlap;
	int layerNr;
	double rotation;
	PointMatrix matrix;
	vector<vector<segment_Info>> cutList;

	int minY;
	int minX;
	int segmentLength;
	int partitionLength;
	int spacingLength;

	int maxLineLength;	// what for? 너무 연결하는선이 길면 연결하지마
	bool b_partition;
};