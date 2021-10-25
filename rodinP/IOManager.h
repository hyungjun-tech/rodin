#pragma once

#include "MeshModel.h"
#include "Mesh.h"

class MeshModelImporter
{
public:
	void operator()(std::vector<IMeshModel*>& models_, QString fileName_);
};

class MeshModelExporter
{
public:
	void operator()(const std::vector<IMeshModel*>& models_, QString fileName_);
};