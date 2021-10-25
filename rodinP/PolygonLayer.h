#pragma once
#include <map>
#include "polygon.h"
#include "polygonOptimizer.h"
#include "Mesh.h"

class SliceLayer;

class LayerSegment
{
public:
	Point start, end;
	int faceIndex;
	bool addedToPolygon;
};

class closestPolygonResult
{   //The result of trying to find a point on a closed polygon line. This gives back the point index, the polygon index, and the point of the connection.
	//The line on which the point lays is between pointIdx-1 and pointIdx
public:
	Point intersectionPoint;
	int polygonIdx;
	unsigned int pointIdx;
};

class gapCloserPolygon
{
public:
	int64_t len;
	int polygonIdx;
	unsigned int pointIdxA;
	unsigned int pointIdxB;
	bool AtoB;
};

class PolygonLayer
{
public:
	double z;
	bool makeSegmentList(int minZ_, int maxZ_, std::vector<Mesh::Point> points_, int faceIdx);
	void clearSegmentData();
	void clearPolygonData();
	void makePolygons(Mesh * mesh_, bool keepNoneClosed, bool extensiveStitching, bool clearSegList = true);
	void createLayerWithParts(SliceLayer& storageLayer, int unionAllType);
private:
	engine::Polygons polygonList;
	engine::Polygons openPolygons;
	std::vector<LayerSegment> segmentList;
	std::map<int, int> faceToSegmentIndex;

	gapCloserPolygon findPolygonGapCloser(Point ip0, Point ip1);
	closestPolygonResult findPolygonPointClosestTo(Point input);
	LayerSegment project2D(Mesh::Point& p0, Mesh::Point& p1, Mesh::Point& p2, int32_t z) const;
};