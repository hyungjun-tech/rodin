#pragma once

#include "floatpoint.h"
#include "AABB.h"
//#include "Slicer.h"
//#include "VboVolume.h"


class SliceLayerPart
{
public:
	AABB2D boundaryBox;
	engine::Polygons outline;
	engine::Polygons combBoundery;
	std::vector<engine::Polygons> insets;
	engine::Polygons skinOutline;
	std::vector<engine::PolygonsPart> skinPartOutline;
	std::vector<int> bridges;
	engine::Polygons sparseOutline;
};

class SliceLayer
{
public:
	int sliceZ;
	int printZ;
	std::vector<SliceLayerPart> parts;
	engine::Polygons openLines;
};

class SupportLayerPart
{
public:
	AABB2D support_boundaryBox;
	engine::Polygons support_outline;		//island
	std::vector<engine::Polygons> support_insets;
	engine::Polygons support_interfaceOutline;
	engine::Polygons support_interface_outline_roof;
	engine::Polygons support_interface_outline_floor;
	std::vector<engine::PolygonsPart> support_interfacePartOutline;
	std::vector<engine::PolygonsPart> support_interface_part_roof;
	std::vector<engine::PolygonsPart> support_interface_part_floor;
	std::vector<int> support_bridges;
	std::vector<int> support_bridges_roof;
	std::vector<int> support_bridges_floor;
	engine::Polygons support_sparseOutline;
};

class SupportLayer
{
public:
	int support_sliceZ;
	int support_printZ;
	std::vector<SupportLayerPart> support_parts;
};

class WipeTowerLayer
{
public:
	engine::Polygons wipeTower_mainOutline;
	engine::Polygons wipeTower_mainInnerWall;

	engine::Polygons wipeTower_subOutline;

	void clear();
};