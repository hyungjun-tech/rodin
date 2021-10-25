#pragma once
#include "Mesh.h"

class IMeshModel;
class MeshExporter
{
public:
	MeshExporter();
	~MeshExporter();
	bool saveModels(std::vector<IMeshModel*> models_, QFileInfo qFileInfo_);

private:
	bool saveBinarySTLAll(std::vector<IMeshModel*> models_, QFileInfo qFileInfo_);
	bool saveBinarySTL(IMeshModel* model_, QString qFileName_);
	aiScene* generateScene(std::vector<IMeshModel*> models_);

	bool save3MF(std::vector<IMeshModel*> models_, QFileInfo qFileInfo_);
	Lib3MF::PModel generateModel(std::vector<IMeshModel*> models_);

	/*bool Save(Mesh* mesh_, QString qFileName_);
	bool Save(std::vector<Mesh*>, QString qFileName_);*/
};
