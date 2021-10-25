/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#ifndef SLICE_DATA_STORAGE_H
#define SLICE_DATA_STORAGE_H

#include "intpoint.h"
#include "polygon.h"

/*
SliceData
+ Layers[]
  + LayerParts[]
    + OutlinePolygons[]
    + Insets[]
      + Polygons[]
    + SkinPolygons[]
*/
namespace cura {

class SliceLayerPart
{
public:
    AABB boundaryBox;
    Polygons outline;
    Polygons combBoundery;
    vector<Polygons> insets;
    Polygons skinOutline;
    Polygons sparseOutline;
};

class SliceLayer
{
public:
    int sliceZ;
    int printZ;
    vector<SliceLayerPart> parts;
    Polygons openLines;
};

/******************/
class SupportPoint
{
public:
    int32_t z;
    double cosAngle;
    
    SupportPoint(int32_t z, double cosAngle) : z(z), cosAngle(cosAngle) {}
};
class SupportStorage
{
public:
    bool generated;
    int angle;
    bool everywhere;
    int XYDistance;
    int ZDistance;

	void clear()
	{
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
    
    ClPoint gridOffset;
    int32_t gridScale;
    int32_t gridWidth, gridHeight;
    vector<SupportPoint>* grid;
	vector<bool>* b_grid;
	SupportStorage(){ grid = nullptr; b_grid = nullptr; generated = false; }
	~SupportStorage(){ if (grid) delete[] grid; if (b_grid) delete[] b_grid; }
	 
};
/******************/

class SliceVolumeStorage
{
public:
    vector<SliceLayer> layers;
};

class SliceDataStorage
{
public:
    Point3 modelSize, modelMin, modelMax;
    Polygons skirt;
    Polygons raftOutline;               //Storage for the outline of the raft. Will be filled with lines when the GCode is generated.
	vector<Polygons> raftInfill;
    vector<Polygons> oozeShield;        //oozeShield per layer
    vector<SliceVolumeStorage> volumes;
    
    SupportStorage support;
    Polygons wipeTower;
    ClPoint wipePoint;

	void clear()
	{
		skirt.clear();
		raftOutline.clear();

		for (int i = 0; i < raftInfill.size(); ++i)
			raftInfill[i].clear();
		raftInfill.clear();

		for (int i = 0; i < oozeShield.size(); ++i)
			oozeShield[i].clear();
		oozeShield.clear();

		for (int i = 0; i < volumes.size(); ++i)
			volumes[i].layers.clear();
		volumes.clear();
		
		support.clear();
		wipeTower.clear();
	}
};

}//namespace cura

#endif//SLICE_DATA_STORAGE_H
