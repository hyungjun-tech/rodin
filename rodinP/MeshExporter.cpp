#include "stdafx.h"
#include "MeshExporter.h"
#include "MeshModel.h"

// --------------------------------------------------
// MeshExporter

MeshExporter::MeshExporter()
{

}

MeshExporter::~MeshExporter()
{

}
bool MeshExporter::saveModels(std::vector<IMeshModel*> models_, QFileInfo qFileInfo_)
{
	QString suffix = qFileInfo_.suffix().toLower();
	if (suffix == "stl")
		return saveBinarySTLAll(models_, qFileInfo_);
	else if (suffix == "3mf")
		return save3MF(models_, qFileInfo_);
	else
		return false;
}

bool MeshExporter::saveBinarySTLAll(std::vector<IMeshModel*> models_, QFileInfo qFileInfo_)
{
	if (qFileInfo_.suffix() != "stl")
		return false;

	QString filename;

	for (int i = 0; i < models_.size(); i++) {
		if (i == 0)
			filename = qFileInfo_.absoluteFilePath();
		else
			filename = qFileInfo_.absolutePath() + "/" + qFileInfo_.completeBaseName() + "_" + QString::number(i) + "." + qFileInfo_.suffix();

		if (!saveBinarySTL(models_[i], filename))
			return false;
	}
	return true;
}

bool MeshExporter::saveBinarySTL(IMeshModel* model_, QString qFileName_)
{
	std::vector<IMeshModel*> tempModels;
	for (auto model : model_->getModels())
	{
		tempModels.push_back(model);
	}

	aiScene* scene = generateScene(tempModels);

	if (scene)
	{
		Assimp::Exporter exporter;
		const aiExportDataBlob *blob = exporter.ExportToBlob(scene, "stlb");
		if (!blob)
		{
			Logger() << "MeshExporter::SaveBinary Failure : " << exporter.GetErrorString();
			printf("MeshExporter::SaveBinary Failure : %s\n", exporter.GetErrorString());
			return false;
		}
		FILE * pFile;
		pFile = _wfopen(Generals::qstringTowchar_t(qFileName_), L"wb");
		if (pFile == NULL)
			return false;
		fwrite(blob->data, blob->size, 1, pFile);
		fclose(pFile);
		delete scene;
	}
	return true;
}
aiScene* MeshExporter::generateScene(std::vector<IMeshModel*> models_)
{
	aiScene* scene = new aiScene();
	scene->mRootNode = new aiNode();
	//scene->mRootNode->mMeshes = new unsigned int[1];
	//scene->mRootNode->mMeshes[0] = 0;
	scene->mRootNode->mNumMeshes = 0;


	scene->mMaterials = new aiMaterial*[1];
	scene->mMaterials[0] = nullptr;
	scene->mNumMaterials = 1;
	scene->mMaterials[0] = new aiMaterial();

	scene->mRootNode->mChildren = new aiNode*[models_.size()];
	scene->mRootNode->mNumChildren = models_.size();
	scene->mMeshes = new aiMesh*[models_.size()];
	scene->mNumMeshes = models_.size();

	for (int i = 0; i < models_.size(); i++)
	{
		scene->mRootNode->mChildren[i] = new aiNode();
		aiNode* child = scene->mRootNode->mChildren[i];
		child->mMeshes = new unsigned int[1];
		child->mMeshes[0] = i;
		child->mNumMeshes = 1;

		QMatrix4x4 mat = models_[i]->getFrame().getModelMatrix();
		aiMatrix4x4 aiMat(mat.row(0)[0], mat.row(0)[1], mat.row(0)[2], mat.row(0)[3],
						 mat.row(1)[0], mat.row(1)[1], mat.row(1)[2], mat.row(1)[3],
						 mat.row(2)[0], mat.row(2)[1], mat.row(2)[2], mat.row(2)[3],
						 mat.row(3)[0], mat.row(3)[1], mat.row(3)[2], mat.row(3)[3]);
		child->mTransformation = aiMat;

		scene->mMeshes[i] = nullptr;
		scene->mMeshes[i] = new aiMesh;
		//scene->mMeshes[0]->mMaterialIndex = 0;

		auto pMesh = scene->mMeshes[i];
		Mesh* mesh_ = ((BasicMeshModel*)models_[i])->getOuter();

		pMesh->mVertices = new aiVector3D[mesh_->n_vertices()];
		//pMesh->mNormals = new aiVector3D[mesh_->n_faces()];
		pMesh->mNumVertices = mesh_->n_vertices();

		//pMesh->mTextureCoords[i] = new aiVector3D[mesh_->n_vertices()];

		//pMesh->mNumUVComponents[0] = mesh_->n_vertices();

		for (Mesh::VertexIter vit = mesh_->vertices_begin();
			vit != mesh_->vertices_end();
			vit++)
		{
			Mesh::Point p = mesh_->point(*vit);
			//const auto& t = itr->texCoord;
			pMesh->mVertices[(*vit).idx()] = aiVector3D(p[0], p[1], p[2]);


		}
		//qDebug() << "n_vertices : " << mesh_->n_vertices();
		//qDebug() << "n_faces : " << mesh_->n_faces();

		pMesh->mFaces = new aiFace[mesh_->n_faces()];
		pMesh->mNumFaces = mesh_->n_faces();

		for (Mesh::FaceIter fit = mesh_->faces_begin();
			fit != mesh_->faces_end();
			fit++)
		{
			//qDebug() << "fit.idx : " << (*fit).idx();
			aiFace& pFace = pMesh->mFaces[(*fit).idx()];
			pFace.mIndices = new unsigned int[3];
			pFace.mNumIndices = 3;
			int idx = 0;
			//Mesh::Point n = mesh_->normal(*fit);
			//pMesh->mNormals[(*fit).idx()] = aiVector3D(n[0], n[1], n[2]);

			//Mesh::Point n = mesh_->normal(*fit);
			//pMesh->mNormals[(*fit).idx()] = aiVector3D(n[0], n[1], n[2]);
			//qDebug() << "mNormals : " << n[0] << ", " << n[1] << ", " << n[2];

			for (Mesh::FaceVertexIter fvit = mesh_->fv_begin(*fit);
				fvit != mesh_->fv_end(*fit);
				fvit++)
			{
				pFace.mIndices[idx] = (*fvit).idx();
				idx++;
				if (idx >= 3)
					idx = 0;
			}
			//qDebug() << "pFace.mIndices : " << pFace.mIndices[0] << ", " << pFace.mIndices[1] << ", " << pFace.mIndices[2];
		}

	}
	return scene;
}



bool MeshExporter::save3MF(std::vector<IMeshModel*> models_, QFileInfo qFileInfo_)
{
	if (qFileInfo_.suffix() != "3mf")
		return false;
	std::vector<IMeshModel*> tempModels;
	for (auto models : models_)
	{
		for (auto model : models->getModels())
		{
			tempModels.push_back(model);
		}
	}
	Lib3MF::PModel model = generateModel(tempModels);
	Lib3MF::PWriter writer = model->QueryWriter("3mf");
	std::vector<Lib3MF_uint8> buffer;
	writer->WriteToBuffer(buffer);

	if (buffer.size() == 0)
	{
		Logger() << "MeshExporter::Save3MF Failure : WriteToBuffer";
		printf("MeshExporter::Save3MF Failure : WriteToBuffer");
		return false;
	}

	FILE * pFile;
	pFile = _wfopen(Generals::qstringTowchar_t(qFileInfo_.absoluteFilePath()), L"wb");
	if (pFile == NULL)
		return false;
	fwrite((char*)&buffer[0], buffer.size(), 1, pFile);
	fclose(pFile);
	return true;
}


Lib3MF::PModel MeshExporter::generateModel(std::vector<IMeshModel*> models_)
{
	Lib3MF::PModel model;
	try {
		Lib3MF::PWrapper wrapper = Lib3MF::CWrapper::loadLibrary();
		model = wrapper->CreateModel();
		//Lib3MF::PWriter writer = model->QueryWriter("3mf");
		Lib3MF::PComponentsObject componentsObj = model->AddComponentsObject();
		for (auto tempModel : models_)
		{
			Lib3MF::PMeshObject meshObject = model->AddMeshObject();
			Mesh* mesh_ = tempModel->getOuter();

			for (Mesh::VertexIter vit = mesh_->vertices_begin();
				vit != mesh_->vertices_end();
				vit++)
			{
				Mesh::Point p = mesh_->point(*vit);
				Lib3MF::sPosition pos{ p[0], p[1], p[2] };
				uint32_t id = meshObject->AddVertex(pos);
			}

			for (Mesh::FaceIter fit = mesh_->faces_begin();
				fit != mesh_->faces_end();
				fit++)
			{
				Lib3MF::sTriangle tri;
				int idx = 0;
				for (Mesh::FaceVertexIter fvit = mesh_->fv_begin(*fit);
					fvit != mesh_->fv_end(*fit);
					fvit++)
				{
					tri.m_Indices[idx] = (*fvit).idx();
					idx++;
					if (idx >= 3)
						idx = 0;
				}
				meshObject->AddTriangle(tri);
			}
			QMatrix4x4 mat = tempModel->getFrame().getModelMatrix();
			Lib3MF::sTransform transform{ mat.row(0)[0], mat.row(1)[0], mat.row(2)[0],
				mat.row(0)[1], mat.row(1)[1], mat.row(2)[1],
				mat.row(0)[2], mat.row(1)[2], mat.row(2)[2],
				mat.row(0)[3], mat.row(1)[3], mat.row(2)[3] };
			componentsObj->AddComponent((Lib3MF::CObject*)&(*meshObject),  transform);
		}
		Lib3MF::sTransform tra{ 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 };
		Lib3MF::PBuildItem buildItem = model->AddBuildItem((Lib3MF::CObject*)&(*componentsObj), tra);
		
		//writer->WriteToFile("test.3mf");
	}
	catch (Lib3MF::ELib3MFException &e)
	{
		std::cout << "Error message : " << e.what() << std::endl;
		std::cout << "Error code : " << e.getErrorCode() << std::endl;

		return nullptr;
	}
	return model;
}

/*bool MeshExporter::Save(Mesh* mesh_, QString qFileName_)
{
	// Convert var type : [QString] ¡æ [wchar_t]
	const wchar_t* wFileName = ToWideCharArray()(qFileName_);
	FILE* f = _wfopen(wFileName, L"w");

	if (f == nullptr)
		return false;

	fprintf(f, "%s %ls\n", "solid", wFileName);

	for (Mesh::FaceIter fit = mesh_->faces_begin();
		fit != mesh_->faces_end();
		fit++)
	{
		Mesh::Point n = mesh_->normal(*fit);
		fprintf(f, "\t%s %lf %lf %lf\n", "facet normal", n[0], n[1], n[2]);
		fprintf(f, "\t\t%s\n", "outer loop");
		for (Mesh::FaceVertexIter fvit = mesh_->fv_begin(*fit);
			fvit != mesh_->fv_end(*fit);
			fvit++)
		{
			Mesh::Point p = mesh_->point(*fvit);
			fprintf(f, "\t\t\t%s %lf %lf %lf\n", "vertex", p[0], p[1], p[2]);
		}
		fprintf(f, "\t\t%s\n", "endloop");
		fprintf(f, "\t%s\n", "endfacet");
	}

	fprintf(f, "%s %ls\n", "endsolid", wFileName);
	fclose(f);

	return true;
}

bool MeshExporter::Save(std::vector<Mesh*> meshes_, QString qFileName_)
{
	// Convert var type : [QString] ¡æ [wchar_t]
	const wchar_t* wFileName = ToWideCharArray()(qFileName_);
	FILE* f = _wfopen(wFileName, L"w");

	if (f == nullptr)
		return false;

	fprintf(f, "%s %ls\n", "solid", wFileName);

	for (int i = 0; i < meshes_.size(); i++)
	{
		Mesh* mesh_ = meshes_[i];

		for (Mesh::FaceIter fit = mesh_->faces_begin();
			fit != mesh_->faces_end();
			fit++)
		{
			Mesh::Point n = mesh_->normal(*fit);
			fprintf(f, "\t%s %lf %lf %lf\n", "facet normal", n[0], n[1], n[2]);
			fprintf(f, "\t\t%s\n", "outer loop");
			for (Mesh::FaceVertexIter fvit = mesh_->fv_begin(*fit);
				fvit != mesh_->fv_end(*fit);
				fvit++)
			{
				Mesh::Point p = mesh_->point(*fvit);
				fprintf(f, "\t\t\t%s %lf %lf %lf\n", "vertex", p[0], p[1], p[2]);
			}
			fprintf(f, "\t\t%s\n", "endloop");
			fprintf(f, "\t%s\n", "endfacet");
		}
	}
	fprintf(f, "%s %ls\n", "endsolid", wFileName);
	fclose(f);

	return true;
}*/