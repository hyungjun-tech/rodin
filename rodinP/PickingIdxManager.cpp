#include "stdafx.h"
#include "PickingIdxManager.h"

const int PickingIdxManager::MESH_ID_START = 1000;
const int PickingIdxManager::MESH_ID_END = 2000;

const int PickingIdxManager::SUPPORT_POINT_START = 3000;
const int PickingIdxManager::SUPPORT_POINT_END = 9000;

const int PickingIdxManager::DRAINHOLE_POINT_START = 10000;
const int PickingIdxManager::DRAINHOLE_POINT_END = 11000;

const int PickingIdxManager::TRANS_Z_ID = 2;
const int PickingIdxManager::TRANS_XY_ID = 7;

const int PickingIdxManager::ROT_X_ID = 3;
const int PickingIdxManager::ROT_Y_ID = 4;
const int PickingIdxManager::ROT_Z_ID = 5;

const int PickingIdxManager::BED_ID = 1;
const int PickingIdxManager::PLANE_ID = 6;

int PickingIdxManager::meshID = PickingIdxManager::MESH_ID_START;
int PickingIdxManager::supportPointID = PickingIdxManager::SUPPORT_POINT_START;
int PickingIdxManager::drainHolePointID = PickingIdxManager::DRAINHOLE_POINT_START;

PickingIdxManager::PickingIdxManager()
{

}
PickingIdxManager::~PickingIdxManager()
{

}

int PickingIdxManager::getMeshID()
{
	return meshID++;
}
int PickingIdxManager::getSupportPointID()
{
	return supportPointID++;
}
int PickingIdxManager::getDrainHolePointID()
{
	return drainHolePointID++;
}

qglviewer::Vec PickingIdxManager::encodeID(int index_)
{
	return ToIndexedRGB()(index_);
}
int PickingIdxManager::decodeID(qglviewer::Vec rgb_)
{
	return  ToIntIndex()(rgb_);
}

bool PickingIdxManager::isMeshID(int id_)
{
	if (id_ >= MESH_ID_START && id_ <= MESH_ID_END)
		return true;
	else
		return false;
}
bool PickingIdxManager::isSupportPointID(int id_)
{
	if (id_ >= SUPPORT_POINT_START && id_ <= SUPPORT_POINT_END)
		return true;
	else
		return false;
}
bool PickingIdxManager::isDrainHolePointID(int id_)
{
	if (id_ >= DRAINHOLE_POINT_START && id_ <= DRAINHOLE_POINT_END)
		return true;
	else
		return false;
}
bool PickingIdxManager::isTransZID(int id_)
{
	return id_ == TRANS_Z_ID ? true : false;
}
bool PickingIdxManager::isTransXYID(int id_)
{
	return id_ == TRANS_XY_ID ? true : false;
}

bool PickingIdxManager::isRotXID(int id_)
{
	return id_ == ROT_X_ID ? true : false;
}
bool PickingIdxManager::isRotYID(int id_)
{
	return id_ == ROT_Y_ID ? true : false;
}
bool PickingIdxManager::isRotZID(int id_)
{
	return id_ == ROT_Z_ID ? true : false;
}
bool PickingIdxManager::isPlaneID(int id_)
{
	return id_ == PLANE_ID ? true : false;
}