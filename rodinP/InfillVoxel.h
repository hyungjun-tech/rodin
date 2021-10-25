#pragma once

#include "Mesh.h"
#include "polygon.h"
#include "floatpoint.h"

enum CellType{ OUTSIDE = -1, BOUNDARY, FLAT_BOUND, INSIDE };
enum InfillType{ M1, M2 };
enum PlaneType{ XY, YZ, ZX };

class InfillVoxel
{
public:
	InfillVoxel(void);
	~InfillVoxel(void);

	void setMesh(Mesh* mesh_);
	void run(double s, int m = 0);
	void generateInfill(const engine::Polygons& in_outline, engine::Polygons& result, int extrusionWidth, int infillOverlap, double z);
	int infill_type;	// 0 : with non flat roof, 1 : only flat roof
	
private:
	Mesh* mesh;
	std::vector<FPoint3> pts;
	//std::map<int, FPoint3> ptsIndex;

	int* voxel_cell; // -1 : outside, 0 : boundary, 1: flat boundary, 2 : inside;
	int Nx, Ny, Nz;
	float space;
	FPoint3 origin;
	int* wall[3];

	double R[3][3];
	double Rinv[3][3];

	double skew_R[3][3];
	double skew_Rinv[3][3];

	void clear();
	
	void rotateMesh();
	void makeVoxel(float s);
	void updateVoxel();

	void checkOutside();
	void propagateOutside(int x, int y, int z);
	std::vector<unsigned int> outerCells;
	
	FPoint3 getInterPoint(FPoint3& a, FPoint3& b, double z);
	FPoint3 getPosition(int xIdx, int yIdx, int zIdx);

	std::vector<Point3> overhang_idx;
	void findOverhang();
	void updateOverhang();
	void calcPlane(int& plane_idx, int& cnt, Point3 coor);
	bool* search[3];
	int area[3];
	void propagatePlaneXY(int x, int y, int z, bool real = false);
	void propagatePlaneYZ(int x, int y, int z, bool real = false);
	void propagatePlaneZX(int x, int y, int z, bool real = false);
	
	void getLine(double z);
	void addLineSegment(Point& p1, Point& p2, engine::Polygons& poly, engine::Polygons& result);
	std::vector<Point> lines;

	bool step();

	// new method for NP-hard problem
	bool step2();
	double calcTotalAreaForOverhang();
	bool step3();
	double calcTotalAreaForOverhang2();
	bool stepAll();
	void updatePlanes();
	std::vector<int> plane_idx;
	std::vector<int> plane_area;
	std::vector<int> plane_cnt;
	std::vector<int> plane_idx_order;
	std::vector<double> mean_area;
	int* temp_wall[3];
	int total_area;
};

