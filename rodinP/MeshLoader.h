#pragma once

#include "Mesh.h"

#include "ProgressHandler.h"

using namespace Lib3MF;

// ----------------------------------------------------------------
// Neutral format for mesh importing
//
// Indexed-face mesh & transformation matrix 

namespace LoadMesh{

	typedef float VertexCoordType;

	// Homogeneous transformation matrix
	struct TransformMatrix4X4
	{
		VertexCoordType mMat[4][4];

		TransformMatrix4X4(int scale = 1);

		void set(Lib3MF::sTransform transform);	// lib3mf
		void transpose();

		bool isUnit();
		bool isRowMajorHomogeneousForm();
	};

	// Vertetx
	struct TriVertex
	{
		VertexCoordType mXYZ[3];

		TriVertex();

		void transform(TransformMatrix4X4 transform);
		void transform(std::vector<TransformMatrix4X4>& transforms);
	};

	// Indexed face
	struct IndexedTriFace
	{
		int vertexIndices[3];

		IndexedTriFace();
	};

	struct IndexedFaceMesh
	{
		// indexed face table
		std::vector<IndexedTriFace> mFaces;
		// (x, y, z) of vertices
		std::vector<TriVertex> mVertices;

		void transform(TransformMatrix4X4 transform);
		void transform(std::vector<TransformMatrix4X4>& transforms);
	};
}

// ----------------------------------------------------------------
// Data structures for lib3mf interface
//

// --- mesh with id and transform (for lib3mf, ..., etc)

using namespace LoadMesh;

namespace Lib3mfIntf {

	enum ObjectType
	{
		NotAssigned = 0,
		MeshObjectType = 1,
		ComponentsObjectType = 2,
	};

	struct Object
	{
	public:
		int mObjId;	// id for mesh obj or components obj (reference to other objs)

		Object() : mObjId(-1) { }

		virtual ObjectType Type() = 0;
	};

	// Mesh object with geometry (indexed face + id)
	struct MeshObj : public Object
	{
		LoadMesh::IndexedFaceMesh mIdxFaceMesh;

		MeshObj() { }
		~MeshObj() {
			//printf("~Mesh() called. obj id = %d\n", this->mObjId);
		}
		ObjectType Type() {
			return MeshObjectType;
		}
	};

	// component (pre-defined object) with transformation
	struct Component
	{
		int mRefObjId;	// reference to mesh-objects (one mesh)
						// or other components-objects (one or more meshes)
		bool bTransformed;
		TransformMatrix4X4 mMat;

		Component() : bTransformed(false), mMat(1) {}
		~Component() {
			//printf("~Component() called. ref obj id = %d\n", this->mRefObjId);
		}
	};

	// container for components
	struct ComponentsObj : public Object
	{
		std::vector<Component*> mComps;

		// underlying mesh objects: can be more than its components
		std::vector<int> meshObjIds;	// object id to underlying mesh geometry
		std::vector<std::vector<TransformMatrix4X4>> vecConcatTransforms; // array of transforms for individual mesh objects

		~ComponentsObj()
		{
			//for (std::vector<Component*>::iterator itr = mComps.begin(); itr != mComps.end(); itr++)
			//	delete *itr;
			//printf("~Components() called. obj id = %d\n", this->mObjId);
			for (int i = 0; i < mComps.size(); i++)
				delete mComps[i];
		}
		ObjectType Type()
		{
			return ComponentsObjectType;
		}

	};

	// item
	struct Item
	{
		int mRefObjId;	// reference to components-objects 
		bool bTransformed;
		TransformMatrix4X4 mMat;
		Item() : bTransformed(false), mMat(1) { }
	};

	// container for all build items
	//struct build {
	//	std::vector<Item> mItems;
	//};

}

// -----------------------------------------------------

class IMeshLoader : public QObject
{
	Q_OBJECT
public:
	IMeshLoader();
	~IMeshLoader();

	//virtual std::vector<Mesh*> Load(QString fileName_, bool progressHandler = false) = 0;
	virtual int load(QString qFileName_, std::vector<Mesh*>* pLoadedMesh, bool progressHandler = false) = 0;
};

// Progress handler base abstract class. 
// To define a callback, inherate the class, adding more data variable like a pointer 
// to the progress bar, and then define the virtual funciton of Progress(), wherein you
// can do whatever you want to do like increasing the progress bar.

class MeshLaoderProgress
{
public:
	MeshLaoderProgress() {};
	virtual ~MeshLaoderProgress() {};

	// Progress callback: pure virtual function. It MUSt be defined in the subclass.
	virtual bool progress(float ratio = -1) = 0;

};

class MeshLoader : public IMeshLoader
{
	Q_OBJECT
public:
	MeshLoader();
	virtual ~MeshLoader();

	//virtual std::vector<Mesh*> Load(QString qFileName_, bool progressHandler = false);
	Mesh* loadFirstMesh(QString qFileName_, bool usingProgress_ = false);
	int load(QString qFileName_, std::vector<Mesh*>* pLoadedMesh, bool usingProgress_ = false);

	int getErrorCount() { return errorCnt; };

	// --- progress handler
private:
	// present ratio of progress (0 ~ 1 is assumed)
	float m_ratio;

	// progress handler class
	ProgressHandler* progressHandler;

public:
	float getRatio();
	void setProgressHandler(ProgressHandler* processing_);
	bool updateProgress(float ratio = -1);

	// interface for Assimp
	friend class AssimpProgressHandler;

private:
	int load(const wchar_t* wFileName, const char* suffix, std::vector<Mesh*>* pLoadedMesh, bool usingProgress_ = false);
	int load_Assimp(const wchar_t * wFileName_, const char* suffix, std::vector<Mesh*>* pLoadedMesh, bool usingProgress_ = false);
	int load_lib3mf(const wchar_t* wFileName, std::vector<Mesh*>* pLoadedMesh, bool usingProgress_ = false);

	char* getSuffix(QString fileName_);
	std::string findExtension(std::string wsFilename);
	std::wstring findExtension(std::wstring sFilename);

	int getBufferLen(const wchar_t* fileName_);
	char* getBufferPtr(const wchar_t* fileName_, const int len_);
	//std::vector<Mesh*> GenerateMeshesFromAIScene(const aiScene* scene_);
	//int GenerateMeshesFromAIScene(const aiScene* scene_, std::vector<Mesh*>* pMeshes);
	//int GenerateMeshesFromAIScene2(const aiScene* scene_, std::vector<Mesh*>* pMeshes);

	int generateMeshesFromIndexedFaceMeshes(std::vector<LoadMesh::IndexedFaceMesh*>& idxFaceMeshes, std::vector<Mesh*>* pLoadedMesh);

	Mesh::VertexHandle findVertexHandle(Mesh::Point& v, std::unordered_map<uint32_t, std::vector<Mesh::VertexHandle>>& vertex_hash_map, Mesh* mesh_);
	Mesh::VertexHandle findVertexHandle2(Mesh::Point& v, std::unordered_map<uint32_t, std::vector<Mesh::VertexHandle>>& vertex_hash_map, Mesh* mesh_);

	bool shorterThen(Mesh::Point& p, double len);
	int errorCnt = 0;
Q_SIGNALS:
	void signal_updateProgress(int);
};

//-----------------------------------------------------------
// Progress handler provided by assimp

class AssimpProgressHandler : public Assimp::ProgressHandler
{
public:
	explicit AssimpProgressHandler(MeshLoader* pImporter);
	~AssimpProgressHandler();

	// callback : if it is set to the importer, it will be call back periodically while reading a file.
	virtual bool Update(float percentage = -1.f);

private:
	MeshLoader* m_importer;
	bool m_continue;
};