#include "stdafx.h"
#include "ModelDataStorage.h"

ModelDataStorage::ModelDataStorage()
	: layerCount(0)
{
}


ModelDataStorage::~ModelDataStorage()
{
}

void ModelDataStorage::wipeTowerClear()
{
	wipeTower_layers.clear();
	wipeTower_mainOutline_for_skirt.clear();
	wipeTopwer_points.clear();
}

void ModelDataStorage::insertGcodePath(engine::LayerPathG gcodePaths_)
{
	if (gcodePaths_.paths.size() == 0)
		return;

	std::vector<GCodePathforDraw> tempArray;
	for (int i = 0; i < gcodePaths_.paths.size(); i++)
	{
		engine::pathG path = gcodePaths_.paths[i];
		vector<FPoint3> polys = gcodePaths_.paths[i].poly;

		GCodePathforDraw tempPath;
		tempPath.extruderNo = path.extruderNr;
		tempPath.mode = ( path.mode == -1 ? TRAVEL : path.mode );
		//tempPath.layerNo = layerNo_;
		tempPath.pathNo = i;
		for (int j = 0; j < polys.size() - 1; j++)
		{
			tempPath.polyNo = j;
			tempPath.start = Mesh::Point(polys[j].x, polys[j].y, polys[j].z);
			tempPath.end = Mesh::Point(polys[j + 1].x, polys[j + 1].y, polys[j + 1].z);
			tempArray.push_back(tempPath);
		}

		if (path.mode == WALL_INNER || path.mode == WALL_OUTER || path.mode == SKIRT)
		{
			tempPath.polyNo = polys.size();
			tempPath.start = Mesh::Point(polys.back().x, polys.back().y, polys.back().z);
			tempPath.end = Mesh::Point(polys.front().x, polys.front().y, polys.front().z);
			tempArray.push_back(tempPath);
		}
	}

	gcodePath.push_back(tempArray);
	layerCount++;
}