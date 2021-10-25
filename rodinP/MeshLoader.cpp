#include "stdafx.h"
#include "MeshLoader.h"

using namespace Lib3MF;

// ------------------------------------------------------------------------------

using namespace LoadMesh;

TransformMatrix4X4::TransformMatrix4X4(int scale)
{
	mMat[0][0] = scale; mMat[0][1] = 0; mMat[0][2] = 0; mMat[0][3] = 0;
	mMat[1][0] = 0; mMat[1][1] = scale; mMat[1][2] = 0; mMat[1][3] = 0;
	mMat[2][0] = 0; mMat[2][1] = 0; mMat[2][2] = scale; mMat[2][3] = 0;
	mMat[3][0] = 0; mMat[3][1] = 0; mMat[3][2] = 0; mMat[3][3] = 1;

	assert(isRowMajorHomogeneousForm());
}

void TransformMatrix4X4::set(Lib3MF::sTransform transform)
{	// Lib3mf transfom
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 3; j++)
			mMat[i][j] = transform.m_Fields[i][j];
	mMat[0][3] = mMat[1][3] = mMat[2][3] = 0.0;  mMat[3][3] = 1.0;

	// Transpose it.
	// NOTE: libe3mf 3D matrices assumes that the last column is [0, 0, 0, 1]. So change it
	// to general homogeneous transformation form assuming that the last row is [0, 0, 0, 1].
	transpose();

	assert(isRowMajorHomogeneousForm());
}

void TransformMatrix4X4::transpose()
{
	VertexCoordType transposedMat[4][4];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i == j)
				transposedMat[i][j] = mMat[i][j];
			else
				transposedMat[i][j] = mMat[j][i];
		}
	}

	memcpy(mMat, transposedMat, sizeof(VertexCoordType) * 4 * 4);

	assert(isRowMajorHomogeneousForm());
}

bool TransformMatrix4X4::isUnit()
{
	bool bUnit = (mMat[0][0] == 1 && mMat[0][1] == 0 && mMat[0][2] == 0 && mMat[0][3] == 0) &&
		(mMat[1][0] == 0 && mMat[1][1] == 1 && mMat[1][2] == 0 && mMat[1][3] == 0) &&
		(mMat[2][0] == 0 && mMat[2][1] == 0 && mMat[2][2] == 1 && mMat[2][3] == 0) &&
		(mMat[3][0] == 0 && mMat[3][1] == 0 && mMat[3][2] == 0 && mMat[3][3] == 1);

	return bUnit;
}

bool TransformMatrix4X4::isRowMajorHomogeneousForm()
{
	bool bHomogen = (mMat[3][0] == 0 && mMat[3][1] == 0 && mMat[3][2] == 0 && mMat[3][3] == 1);
	return bHomogen;
}

// ----------------------------------------------------------------------------------

inline TriVertex::TriVertex()
{
	mXYZ[0] = mXYZ[1] = mXYZ[2] = 0;
}

void TriVertex::transform(TransformMatrix4X4 transform)
{

	assert(transform.isRowMajorHomogeneousForm());
	VertexCoordType transformedXYZ[3];

	transformedXYZ[0] = transform.mMat[0][0] * mXYZ[0] + transform.mMat[0][1] * mXYZ[1] + transform.mMat[0][2] * mXYZ[2] + transform.mMat[0][3] * 1.0f;
	transformedXYZ[1] = transform.mMat[1][0] * mXYZ[0] + transform.mMat[1][1] * mXYZ[1] + transform.mMat[1][2] * mXYZ[2] + transform.mMat[1][3] * 1.0f;
	transformedXYZ[2] = transform.mMat[2][0] * mXYZ[0] + transform.mMat[2][1] * mXYZ[1] + transform.mMat[2][2] * mXYZ[2] + transform.mMat[2][3] * 1.0f;

#ifdef _DEBUG
	double weightPoint = transform.mMat[3][0] * mXYZ[0] + transform.mMat[3][1] * mXYZ[1] + transform.mMat[3][2] * mXYZ[2] + transform.mMat[3][3] * 1.0f;
	assert(weightPoint == 1.0f);
#endif

	mXYZ[0] = transformedXYZ[0], mXYZ[1] = transformedXYZ[1], mXYZ[2] = transformedXYZ[2];
}

void TriVertex::transform(std::vector<TransformMatrix4X4>& transforms)
{

	int nTransforms = transforms.size();
	for (int i = 0; i < nTransforms; i++)
	{
		TransformMatrix4X4& transform_ = transforms[i];
		if (!transform_.isUnit())
			transform(transform_);
	}
}

inline IndexedTriFace::IndexedTriFace()
{
	vertexIndices[0] = vertexIndices[1] = vertexIndices[2] = -1;
}

void IndexedFaceMesh::transform(TransformMatrix4X4 transform)
{
	for (std::vector<TriVertex>::iterator itr = mVertices.begin(); itr != mVertices.end(); itr++)
	{
		TriVertex& triVertex = *itr;

		triVertex.transform(transform);
	}
}

void IndexedFaceMesh::transform(std::vector<TransformMatrix4X4>& transforms)
{
	for (std::vector<TriVertex>::iterator itr = mVertices.begin(); itr != mVertices.end(); itr++)
	{
		TriVertex& triVertex = *itr;

		triVertex.transform(transforms);
	}
}


// Interface Class ----------------------------------------------------------------------------------
IMeshLoader::IMeshLoader()
{
}

IMeshLoader::~IMeshLoader()
{
}

// MeshLoader Class -------------------------------------------------------------------------------
MeshLoader::MeshLoader()
	: m_ratio(-1),
	progressHandler(nullptr)
{
}

MeshLoader::~MeshLoader()
{
	if (progressHandler)
		progressHandler->close();
}

Mesh* MeshLoader::loadFirstMesh(QString qFileName_, bool usingProgress_)
{
	std::vector<Mesh*> Meshes;
	int numMeshes = load(qFileName_, &Meshes, usingProgress_);
	assert(numMeshes > 0 && numMeshes == Meshes.size());

	if (numMeshes > 0)
		return Meshes[0];
	else
		return nullptr;
}

int MeshLoader::load(QString qFileName_, std::vector<Mesh*>* pLoadedMesh, bool usingProgress_)
{
	// Convert var type : [QString] → [wchar_t]
	const wchar_t* wFileName = Generals::qstringTowchar_t(qFileName_);
	assert(wFileName);

	// Extract Filename Extension
	const char* suffix = getSuffix(qFileName_);
	assert(suffix);

	int nMeshes = load(wFileName, suffix, pLoadedMesh, usingProgress_);

	// memory release
	delete wFileName;
	delete suffix;

	return nMeshes;
}

int MeshLoader::load(const wchar_t* wFileName, const char* suffix, std::vector<Mesh*>* pLoadedMesh, bool usingProgress_)
{
	// file extension to lowercase
	std::string fileExt(suffix);
	std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);

	int numMeshes = 0;
	if (fileExt.compare("3mf") == 0)
	{	// equal
										// progressHandler is NOT supported here.
		numMeshes = load_lib3mf(wFileName, pLoadedMesh, usingProgress_);
	}
	else
	{
		numMeshes = load_Assimp(wFileName, suffix, pLoadedMesh, usingProgress_);
	}

	return numMeshes;
}

int MeshLoader::generateMeshesFromIndexedFaceMeshes(std::vector<LoadMesh::IndexedFaceMesh*>& idxFaceMeshes, std::vector<Mesh*>* pMeshes)
{
	assert(pMeshes->empty());	// empty container is assumed. Otherwise, a little revision is necessary.

	// error count
	int errGeomCnt = 0, errKernelCnt = 0;

	//int numAiMeshes = scene_->mNumMeshes;
	int nIdxFcMeshes = idxFaceMeshes.size();
	pMeshes->resize(nIdxFcMeshes);

	// for each mesh
	for (int i = 0; i < nIdxFcMeshes; i++)
	{
		Mesh* pCurMesh = (*pMeshes)[i] = new Mesh();
		std::unordered_map<uint32_t, std::vector<Mesh::VertexHandle>> vertex_hash_map;

		//#define _DEBUG_NONMANIFOLD
#ifdef _DEBUG_NONMANIFOLD
		// To check non-manifold edges
		const int MaxEdge = 1000000;
		class Edge {
		public:
			std::pair<int, int> Vertex;
			int Use;
		public:
			Edge() {
				Vertex = std::pair<int, int>(-1, -1);
				Use = 0;
			}
		};
		std::unordered_map<int, Edge> edgeMap;

#endif

		//aiMesh *aimesh = scene_->mMeshes[i];
		LoadMesh::IndexedFaceMesh* pIdxFcMesh = idxFaceMeshes[i];
		//int numAiFaces = aimesh->mNumFaces;
		//int numAiVertices = aimesh->mNumVertices;	// for debugging
		int nFaces = pIdxFcMesh->mFaces.size();
		int nVertices = pIdxFcMesh->mVertices.size();

		Mesh::Point vertexPoints[3];
		for (int j = 0; j < nFaces; j++)
		{
			//aiFace f = aimesh->mFaces[j];
			LoadMesh::IndexedTriFace& rIdxTriFace = pIdxFcMesh->mFaces[j];

			std::vector<Mesh::VertexHandle> face_vhandles;
			for (int k = 0; k < 3; k++)
			{
				//int idx = f.mIndices[k];
				int idx = rIdxTriFace.vertexIndices[k];
				//aiVector3D v = aimesh->mVertices[idx];
				LoadMesh::TriVertex& rTriVertex = pIdxFcMesh->mVertices[idx];
				//Mesh::Point p(v[0], v[1], v[2]);
				//vertexPoints[k] = Mesh::Point(v[0], v[1], v[2]);
				vertexPoints[k] = Mesh::Point(rTriVertex.mXYZ[0], rTriVertex.mXYZ[1], rTriVertex.mXYZ[2]);
				// Finding an existing (already-created) vertx handle to the point or create a new one
				Mesh::VertexHandle vHandle = findVertexHandle2(vertexPoints[k], vertex_hash_map, pCurMesh);
				face_vhandles.push_back(vHandle);
			}
			assert(face_vhandles.size() == 3);

#define _CHECK_DEGENERATE_CASE
#ifdef _CHECK_DEGENERATE_CASE
			// The original version has it.
			// The following degenerate case check doesn't have much influence on computation time. Just about 1~2 sec.
			if (face_vhandles[0].idx() == face_vhandles[1].idx() || face_vhandles[1].idx() == face_vhandles[2].idx() || face_vhandles[2].idx() == face_vhandles[0].idx())
			{
				errGeomCnt++;	// Geometric compatibility error

				printf("MeshLoader::GenerateMeshesFromIndexedFaceMeshes() - error: degenerate face.\n");

				continue;		// skip the face with geometric problems and go to the next;
			}
#endif

			// Add the face to the mesh
#ifdef _DEBUG_NONMANIFOLD
			// Check if there are nonmanifold edges
			for (int temp_ii = 0; temp_ii < 3; temp_ii++) {
				int startV = face_vhandles[temp_ii].idx();
				int endV = face_vhandles[(temp_ii + 1) % 3].idx();
				if (startV > endV)	// accending order
					std::swap(startV, endV);
				assert(startV < endV);

				// search the edge in the DB: 
				int hash = startV * MaxEdge + endV;	// temporary key
				auto edgeFound = edgeMap.find(hash);
				if (edgeFound == edgeMap.end()) {	// not found, new edge
					edgeMap[hash].Vertex.first = startV;
					edgeMap[hash].Vertex.second = endV;
					edgeMap[hash].Use++;
					assert(edgeMap[hash].Use == 1);
				}
				else {
					assert(edgeMap[hash].Vertex.first == startV);
					assert(edgeMap[hash].Vertex.second == endV);

					edgeMap[hash].Use++;

					if (edgeMap[hash].Use > 2) {	// non-manifold
						printf("GenerateMeshesFromIndexedFaceMeshes() - Nonmanifold edge found.");
					}
				}
			}
#endif

			if (pCurMesh->add_face(face_vhandles) == Mesh::InvalidFaceHandle)
			{
				printf("MeshLoader::GenerateMeshesFromIndexedFaceMeshes() - error: add_face() failed, (mesh,face) = (%d,%d)\n", i, j);

				errKernelCnt++;		// error in openmesh kernel, in add_face()

#ifdef _DEBUG_OFF
				// retry with inverse
				std::swap(face_vhandles[1], face_vhandles[2]);
				Mesh::FaceHandle faceHandle = pCurMesh->add_face(face_vhandles);
#endif

				// Retry to add the face with newly-created vertex handles.
				// In this try, we will create new, separate vertex handles for the face WITHOUT looking up the vertex table. 
				// So, the face will be added WITHOUT any connectivity relation to the existing faces as an isolated face.
				// Note that the number of vertices will increase as much since new vertices are separately created and 
				// added even though there are already duplicate ones.

				// The erroneous face may have any geometric incompatibility, but in loading a file, it would be better to 
				// read in all faces anyway even though it is geometrically incompatible. Another fixing process will remove them later.

				std::vector<Mesh::VertexHandle> retry_face_vhandles(3);
				Mesh::Point retry_vertexPoints[3];
				for (int x = 0; x < 3; x++)
				{
					int idx = x;
					Mesh::VertexHandle newVHandle = face_vhandles[idx];
					retry_vertexPoints[idx] = pCurMesh->point(newVHandle);
					retry_face_vhandles[idx] = pCurMesh->add_vertex(retry_vertexPoints[idx]);
				}

				if (pCurMesh->add_face(retry_face_vhandles) == Mesh::InvalidFaceHandle)
				{
					// This should NEVER happen!
					// As mentioned above, in this try we use all new vertex handles. So it SHOULD always succeed!
					assert(false);
					printf("MeshLoader::GenerateMeshesFromAIScene2() - error: add_face(), retrying also failed. Will be skipped.\n");

					Logger() << "cannot load this face : " << face_vhandles[0].idx() << ":" << face_vhandles[1].idx() << ":" << face_vhandles[2].idx();
				}
				else {
					// OK, though it is added as an separate, independent face.
					printf("Retrying add_face() as a separate one... OK.\n");
				}
				//QString forDebug = QString::number(face_vhandles[0].idx()) + " " + QString::number(face_vhandles[1].idx()) + " " + QString::number(face_vhandles[2].idx());
				//Logger() << "2 error : " << forDebug;
				//errorCnt++;

			}
			//Logger() << face_vhandles[0].first.idx() << ":" << face_vhandles[1].first.idx() << ":" << face_vhandles[2].first.idx();
		}

		(*pMeshes)[i]->request_face_normals();
		(*pMeshes)[i]->request_vertex_normals();
		(*pMeshes)[i]->update_normals();

#ifdef _DBG_MSG_OFF
		// Check if the numbers of faces and vertices of lopen-source oader and OpenMesh are identical.
		// NOTE: It is quite normal that the number of OpenMesh's vertices reduces than that of the loader. 
		// Assimp - Enven though we use aiProcess_JoinIdenticalVertices option to remove identical vertices in assimp, 
		// assimp uses no tolerance or very small, really small (maybe less than 1e-6 or 1e-10), a lot of identicla 
		// vertices still remains as different one. But in OpenMesh, it probably use a normal tolerance for checking the 
		// same points so that many vertices are identified as the same vertices. So the number of vertices 
		// in OpenMesh reduces than assimp's.
		int numFacesOpenMesh = pCurMesh->n_faces();
		int numVerticesOpenMesh = pCurMesh->n_vertices();
		printf("Indexed face mesh #%d/%d (faces = %d, vertices = %d) ---> OpenMesh (faces = %d, vertices = %d).\n",
			i, nIdxFcMeshes, nFaces, nVertices, numFacesOpenMesh, numVerticesOpenMesh);
#endif
	}

	// set error count
	errorCnt += (errGeomCnt + errKernelCnt);

	if (errorCnt > 0)
		Logger() << "This file has some geometric errors - error count : " << errorCnt;

	return pMeshes->size();
}


struct FVertexList {
	int idx;
	double angle;
};
static
double getAngle(int cur, int next, int before, aiMesh *aimesh)
{
	aiVector3D Cur_p = aimesh->mVertices[cur];
	aiVector3D Next_p = aimesh->mVertices[next];
	aiVector3D Before_p = aimesh->mVertices[before];
	FPoint3 C_B = FPoint3(Before_p[0], Before_p[1], Before_p[2]) - FPoint3(Cur_p[0], Cur_p[1], Cur_p[2]);
	FPoint3 C_N = FPoint3(Next_p[0], Next_p[1], Next_p[2]) - FPoint3(Cur_p[0], Cur_p[1], Cur_p[2]);

	if (C_B.vSize() == 0 | C_N.vSize() == 0)
		return 0; //예외 처리 // 같은 점이 두개 들어있는 경우 있음............

	double angle = abs((atan2(C_B.y, C_B.x) - atan2(C_N.y, C_N.x)) * 180 / M_PI);
	return angle;
}

// Loader using Assimp lib ------------------------------------------------------------
static
int getIndexedFaceMeshesfromAIScene(const aiScene* scene_, std::vector<LoadMesh::IndexedFaceMesh*>* pMeshes)
{
	int nPrevMeshes = pMeshes->size();

#ifdef _DEBUG
	// To check the contents of the root and the child nodes if there are transforms set to non-unity matrix.
	// As of 2019-10-25, assimp v5.0.0, 
	// (1) it looks like transforms of lib3mf is not supported.
	// (2) it is not capable of parsing <components></components>
	// (3) it cannot handle units (meter in 3mf is treated as milimeter, leading to a very small object.
	aiNode* rootNode = scene_->mRootNode;
	assert(rootNode);
	aiMatrix4x4 transform = rootNode->mTransformation;
	int numMeshes = rootNode->mNumMeshes;
	for (int i = 0; i < numMeshes; i++) {
		int meshIndex = rootNode->mMeshes[i];
		assert(meshIndex >= 0);
	}

	int numChildren = rootNode->mNumChildren;
	for (int i = 0; i < numChildren; i++) {
		aiNode* child = rootNode->mChildren[i];
		aiMatrix4x4 transformChild = child->mTransformation;
		assert(transformChild.IsIdentity());
		// At lease for 3mf, it seems that it is alwasys Unity, even though
		// 3mf has transform. Is it a bug?

		// As of 2019-10-25, assimp v5.0.0, for <mesh></mesh>, it extract its mesh. 
		// But for <components></components>, it does not extract its child reference to a mesh or meshes. 
		// mNumMeshes is just zero, which prevent us from doing anything.
		int numMeshesChild = child->mNumMeshes;	// for <components></components> of 3mf, it is zero, unfortunately.
		for (int j = 0; j < numMeshesChild; j++) {
			int meshIndex = child->mMeshes[j];	// index to the list of aiScene	
			assert(meshIndex >= 0 && meshIndex < scene_->mNumMeshes);
		}

		int numChildren2 = child->mNumChildren;
		assert(numChildren2 == 0); // if fails, recursive children...
	}

#endif

	// for each mesh
	int numAiMeshes = scene_->mNumMeshes;
	for (int i = 0; i < numAiMeshes; i++) {

		aiMesh *aimesh = scene_->mMeshes[i];
		int numAiFaces = aimesh->mNumFaces;
		int numAiVertices = aimesh->mNumVertices;

		// build a new indexed face mesh
		LoadMesh::IndexedFaceMesh* idxFcMesh = new LoadMesh::IndexedFaceMesh;

		// copy faces
		idxFcMesh->mFaces.clear();
		for (int j = 0; j < numAiFaces; j++) {
			aiFace f = aimesh->mFaces[j];

			LoadMesh::IndexedTriFace idxTriFc;
			int numIndices = aimesh->mFaces[j].mNumIndices;
			//for triangles
			if (numIndices == 3)
			{
				for (int k = 0; k < 3; k++)
					idxTriFc.vertexIndices[k] = f.mIndices[k];

				idxFcMesh->mFaces.push_back(idxTriFc);
			}
			//for polygons
			else if (numIndices > 3)
			{
				std::vector<FVertexList> FVLists;
				for (int k = 0; k < numIndices; k++)
				{
					FVertexList fv;
					fv.idx = f.mIndices[k];
					FVLists.push_back(fv);
				}
				//FOR GL_POLYGONS Mode --> earcut algorithm
				while (FVLists.size() >= 3)
				{
					for (int i = 0; i < FVLists.size(); i++)
					{
						int cur = i;
						int next, before;
						i + 1 == FVLists.size() ? next = 0 : next = i + 1;
						i - 1 == -1 ? before = FVLists.size() - 1 : before = i - 1;

						double angle = getAngle(FVLists[cur].idx, FVLists[next].idx, FVLists[before].idx, aimesh);
						FVLists[cur].angle = angle;
					}

					int idx = -1;
					double max(FLT_MAX);
					for (int i = 0; i < FVLists.size(); i++)
					{
						if (FVLists[i].angle < max)
						{
							idx = i;
							max = FVLists[i].angle;
						}
					}
					if (idx == -1)//가장 작은거 못찾을 경우
						break;

					idxTriFc.vertexIndices[0] = FVLists[idx].idx;
					int next, before;
					idx + 1 == FVLists.size() ? next = 0 : next = idx + 1;
					idx - 1 == -1 ? before = FVLists.size() - 1 : before = idx - 1;

					idxTriFc.vertexIndices[1] = FVLists[next].idx;
					idxTriFc.vertexIndices[2] = FVLists[before].idx;
					idxFcMesh->mFaces.push_back(idxTriFc);

					FVLists.erase(FVLists.begin() + idx);
				}
			}
		}

		// copy vertices
		idxFcMesh->mVertices.resize(numAiVertices);
		for (int j = 0; j < numAiVertices; j++) {
			aiVector3D v = aimesh->mVertices[j];

			LoadMesh::TriVertex triVtx;
			triVtx.mXYZ[0] = v[0], triVtx.mXYZ[1] = v[1], triVtx.mXYZ[2] = v[2];

			idxFcMesh->mVertices[j] = triVtx;
		}

		pMeshes->push_back(idxFcMesh);	// save
	}

	int nPresentMeshes = pMeshes->size();

	return nPresentMeshes - nPrevMeshes;
}

int MeshLoader::load_Assimp(const wchar_t * wFileName_, const char* suffix, std::vector<Mesh*>* pLoadedMesh, bool usingProgress_)
{
	// Get Buffer Length and Pointer
	int length = getBufferLen(wFileName_);
	char *buffer = getBufferPtr(wFileName_, length);
	assert(buffer);

	// Using Assimp
	Assimp::Importer importer;
	if (usingProgress_)
	{	// register callback
		AssimpProgressHandler* pHandler = new AssimpProgressHandler(this);
		importer.SetProgressHandler(pHandler);
		// If pHandler is set, pHandler->Update() will be periodically called while reading a file by the importer.
		// Deleting pHandler will be performed by the importer. So just forget about it.
	}

	const aiScene* scene = importer.ReadFileFromMemory((void *)buffer, length,
		aiProcess_JoinIdenticalVertices
		/*| aiProcess_PreTransformVertices
		  | aiProcess_FindDegenerates */, suffix);

		  // After Importing, release the memory
	delete buffer;

	// Check if importing fails or not. 
	// fail = error, aborted or cancelled by the progress handler callback (when returning false)
	if (scene == nullptr)
	{
		const char* error = importer.GetErrorString();
		printf("Assimp error::%s\n", error);
		printf("mesh loading failed\n");

		return -1;	// reading error, aborted, cancelled
	}
	// Sometimes, even though there is no error, there happen cases where the number of meshes is zero.
	// 

	// Convert to neutral, intermediate indexed-face mesh
	std::vector<LoadMesh::IndexedFaceMesh*> idxFaceMeshes;
	int numIdxFaceMeshes = getIndexedFaceMeshesfromAIScene(scene, &idxFaceMeshes);

	// Generate Mesh from the indexed face meshes
	int numMeshes = generateMeshesFromIndexedFaceMeshes(idxFaceMeshes, pLoadedMesh);

	// release memory
	for (std::vector<LoadMesh::IndexedFaceMesh*>::iterator itr = idxFaceMeshes.begin(); itr != idxFaceMeshes.end(); itr++)
	{
		LoadMesh::IndexedFaceMesh* idxFcMesh = *itr;
		delete idxFcMesh;
	}

	assert(numMeshes == scene->mNumMeshes);

	// Clear
	// it seems that the importer will take of cleaning up the progress handler. So we don't need to delete it here.
	//delete pHandler;

	// Will be destroyed together with the importer. We don't have to take care of it.
	//delete scene;

	//printf("MeshLoader - OpenMesh construction done\n");

	return numMeshes;
}


/// Private Functions -------------------------------------------------------------------------------
char* MeshLoader::getSuffix(QString fileName_)
{
	QString suffix = QFileInfo(fileName_).suffix();
	std::string str = suffix.toStdString();
	char* chr = new char[str.size() + 1];
	strcpy(chr, str.c_str());

	return chr;
}

std::string MeshLoader::findExtension(std::string sFilename)
{
	// this emulates Windows' PathFindExtension
	std::string::size_type idx;
	idx = sFilename.rfind('.');

	if (idx != std::string::npos)
	{
		return sFilename.substr(idx);
	}
	else
	{
		return "";
	}
}

std::wstring MeshLoader::findExtension(std::wstring wsFilename)
{
	// this emulates Windows' PathFindExtension
	std::wstring::size_type idx;
	idx = wsFilename.rfind('.');

	if (idx != std::wstring::npos)
	{
		return wsFilename.substr(idx);
	}
	else
	{
		return std::wstring();
	}
}

int MeshLoader::getBufferLen(const wchar_t* fileName_)
{
	std::ifstream is;

	is.open(fileName_, std::ios::binary);
	is.seekg(0, std::ios::end);
	int len = is.tellg();
	is.close();

	return len;
}

char* MeshLoader::getBufferPtr(const wchar_t* fileName_, const int len_)
{
	char *buff = new char[len_];
	std::ifstream is;
	is.open(fileName_, std::ios::binary);
	is.read(buff, len_);
	is.close();

	return buff;
}

// -------------------------------------------

float MeshLoader::getRatio()
{
	assert(m_ratio == -1 || m_ratio >= 0 && m_ratio <= 1.0);
	return m_ratio;
}

// Update the present progress ration and call callbacks of progress handler if any
bool MeshLoader::updateProgress(float ratio)
{
	assert(ratio == -1 || ratio >= 0 && ratio <= 1.0);
	//printf("percentage = %.2f\n", ratio);

	// Update ratio of progress
	m_ratio = ratio;

	// Call registered callback if any
	bool bContinue = true;
	if (progressHandler)
	{
		bContinue = !progressHandler->wasCanceled();
		progressHandler->setValue(ratio);
	}
	return bContinue;
};

void MeshLoader::setProgressHandler(ProgressHandler* handler_)
{
	progressHandler = handler_;
}

// Mesh loader utility functions --------------------------------

#define MM2INT(n) (int64_t((n) * 1000))

const int vertex_meld_distance = MM2INT(0.03);
static inline uint32_t pointHash(Mesh::Point& p)
{
	return (((int32_t)p[0] + vertex_meld_distance / 2) / vertex_meld_distance) ^
		((((int32_t)p[1] + vertex_meld_distance / 2) / vertex_meld_distance) << 10) ^
		((((int32_t)p[2] + vertex_meld_distance / 2) / vertex_meld_distance) << 20);
}

Mesh::VertexHandle MeshLoader::findVertexHandle(Mesh::Point& v, std::unordered_map<uint32_t, std::vector<Mesh::VertexHandle>>& vertex_hash_map, Mesh* mesh_)
{
	uint32_t hash = pointHash(Mesh::Point(MM2INT(v[0]), MM2INT(v[1]), MM2INT(v[2])));
	for (unsigned int idx = 0; idx < vertex_hash_map[hash].size(); idx++)
	{
		Mesh::Point tempP = mesh_->point(vertex_hash_map[hash][idx]) - v;
		if (tempP == Mesh::Point(0, 0, 0))
			return vertex_hash_map[hash][idx];
		/*else if (ShorterThen(tempP, vertex_meld_distance*0.000001))
		return vertex_hash_map[hash][idx];*/
	}

	Mesh::VertexHandle vh = mesh_->add_vertex(v);
	vertex_hash_map[hash].push_back(vh);

	return vh;
}

Mesh::VertexHandle MeshLoader::findVertexHandle2(Mesh::Point& v, std::unordered_map<uint32_t, std::vector<Mesh::VertexHandle>>& vertex_hash_map, Mesh* mesh_)
{
	uint32_t hash = pointHash(Mesh::Point(MM2INT(v[0]), MM2INT(v[1]), MM2INT(v[2])));
	// Try to find the vertex handle of the input point v, in the map.
	int mappedVHandleSize = vertex_hash_map[hash].size();
	for (unsigned int idx = 0; idx < mappedVHandleSize; idx++)
	{
		Mesh::VertexHandle mappedVHandle = vertex_hash_map[hash][idx];
		Mesh::Point mappedP = mesh_->point(mappedVHandle);
		Mesh::Point diffP = mappedP - v;
		if (diffP == Mesh::Point(0, 0, 0))
			return mappedVHandle;	//	both hash key and also the point are the same. 
		else {
			;
#if _DEBUG_OFF
			diffP = diffP + Mesh::Point(0, 0, 0);
#endif
		}
		/*else if (ShorterThen(tempP, vertex_meld_distance*0.000001))
		return vertex_hash_map[hash][idx];*/
	}

	// If not found, create a new vertex handle, return it, saving it to the map. (the hash may have multiple vertex handles with slightly different points)
	// if the key is NOT in the map (one hash - one vertex handle), 
	// or the key is in the map but its mapped values (points) does NOT match the given input point. (one hash - multiple vertex handles)

	Mesh::VertexHandle vh = mesh_->add_vertex(v);
	vertex_hash_map[hash].push_back(vh);

#if _DEBUG_OFF
	// To see multiple mapped elements (points of vertex handles of hash)
	std::vector<Mesh::VertexHandle>& dbgVHandlesOfHash = vertex_hash_map[hash];
	int dbgNumVHandlesOfHash = dbgVHandlesOfHash.size();
	assert(dbgNumVHandlesOfHash < 100);
	Mesh::Point dbgMappedP[100];
	if (dbgNumVHandlesOfHash > 1) {
		for (int dbg_i = 0; dbg_i < mappedVHandleSize; dbg_i++)
		{
			Mesh::VertexHandle dbgMappedVHandle = vertex_hash_map[hash][dbg_i];
			dbgMappedP[dbg_i] = mesh_->point(dbgMappedVHandle);
		}
	}
#endif

	return vh;
}

bool MeshLoader::shorterThen(Mesh::Point& p, double len)
{
	bool rtn;
	if (p[0] > len || p[0] < -len)
		return false;
	if (p[1] > len || p[1] < -len)
		return false;
	if (p[2] > len || p[2] < -len)
		return false;
	rtn = p[0] * p[0] + p[1] * p[1] + p[2] * p[2] <= len * len;
	/*if (rtn)
		qDebug() << p[0] << p[1] << p[2] << len;*/
	return rtn;
}


// Loader using lib3mf ----------------------------------------------------------------

//#define _DEBUG_LIB3MF

static
bool getObjectInformationFromLib3mfModel(Lib3MF::PModel& model,
	std::vector<Lib3mfIntf::MeshObj*>* lmIdxFaceMeshes, std::vector<Lib3mfIntf::ComponentsObj*>* lmCompsObjs)
{
	PObjectIterator objectIterator = model->GetObjects();

	while (objectIterator->MoveNext())
	{

		PObject object = objectIterator->GetCurrentObject();

		// Get mesh information
		if (object->IsMeshObject())
		{

			int objId = object->GetResourceID();
			PMeshObject meshObject = model->GetMeshObjectByID(objId);

#ifdef _DEBUG_LIB3MF
			printf("Mesh, object id = %d\n", objId);
#endif
			// get lib3mf vertices & faces

			Lib3MF_uint64 nVertexCount = meshObject->GetVertexCount();
			Lib3MF_uint64 nTriangleCount = meshObject->GetTriangleCount();

			std::vector<sPosition> lib3mfVertices;
			meshObject->GetVertices(lib3mfVertices);
			assert(nVertexCount == lib3mfVertices.size());

			std::vector<sTriangle> lib3mfIndicesTriangles;
			meshObject->GetTriangleIndices(lib3mfIndicesTriangles);
			assert(nTriangleCount == lib3mfIndicesTriangles.size());

			// conver to LoadMesh-type mesh object (vertices and faces)

			Lib3mfIntf::MeshObj* loadMesh = new Lib3mfIntf::MeshObj;

			loadMesh->mObjId = objId;
			loadMesh->mIdxFaceMesh.mFaces.resize(nTriangleCount);
			loadMesh->mIdxFaceMesh.mVertices.resize(nVertexCount);
#ifdef _DEBUG_LIB3MF
			printf("\tv = %d, f = %d\n", nVertexCount, nTriangleCount);
#endif
			// Build vertex table
			for (int i = 0; i < nVertexCount; i++)
			{
				sPosition lib3mfVertex = lib3mfVertices[i];

				LoadMesh::TriVertex loadMeshVertex;
				for (int j = 0; j < 3; j++)
					loadMeshVertex.mXYZ[j] = lib3mfVertex.m_Coordinates[j];

				loadMesh->mIdxFaceMesh.mVertices[i] = loadMeshVertex;
			}

			// build indexed faces
			for (int i = 0; i < nTriangleCount; i++)
			{
				sTriangle lib3mfIdxTriFace = lib3mfIndicesTriangles[i];

				LoadMesh::IndexedTriFace loadMeshFace;
				for (int j = 0; j < 3; j++)
					loadMeshFace.vertexIndices[j] = lib3mfIdxTriFace.m_Indices[j];

				loadMesh->mIdxFaceMesh.mFaces[i] = loadMeshFace;
			}

			// add to mesh object container
			lmIdxFaceMeshes->push_back(loadMesh);

		}
		// Get component information
		else if (object->IsComponentsObject())
		{

			int objId = object->GetResourceID();
			PComponentsObject componentsObject = model->GetComponentsObjectByID(objId);

#ifdef _DEBUG_LIB3MF
			printf("Components, object id = %d\n", objId);
#endif

			// Convert to LoadMesh-type Components object
			Lib3mfIntf::ComponentsObj* lmComponents = new Lib3mfIntf::ComponentsObj;
			lmComponents->mObjId = objId;

			for (Lib3MF_uint32 nIndex = 0; nIndex < componentsObject->GetComponentCount(); nIndex++)
			{
				PComponent component = componentsObject->GetComponent(nIndex);

				// conver to LoadMesh-type mesh (vertices and faces)
				Lib3mfIntf::Component* lmComp = new Lib3mfIntf::Component;

				lmComp->mRefObjId = component->GetObjectResourceID();

#ifdef _DEBUG_LIB3MF
				printf("\tComponent #%d, ref obj id = %d\n", nIndex, lmComp->mRefObjId);
#endif

				if (component->HasTransform())
				{

					sTransform lib3mfTransform = component->GetTransform();

					lmComp->bTransformed = true;
					lmComp->mMat.set(lib3mfTransform);

#ifdef _DEBUG_LIB3MF
					printf("\t\tTransformed.\n", lmComp->mRefObjId);
#endif
				}

				// add to components object
				lmComponents->mComps.push_back(lmComp);
			}

			// add to the container of componets objects
			lmCompsObjs->push_back(lmComponents);

		}
		else
		{
			assert(false);
			std::cout << "unknown object #" << object->GetResourceID() << ": " << std::endl;
		}
	}

	return true;
}

static
int getBuildItemsLib3mfModel(Lib3MF::PModel& model, std::vector<Lib3mfIntf::Item*>* lmItems)
{

	PBuildItemIterator buildItemIterator = model->GetBuildItems();

	int numItems = 0;

	while (buildItemIterator->MoveNext())
	{

		numItems++;

		PBuildItem buildItem = buildItemIterator->GetCurrent();

		// Conver to LoadMesh-type Items
		Lib3mfIntf::Item* lmItem = new Lib3mfIntf::Item;

		lmItem->mRefObjId = buildItem->GetObjectResourceID();

#ifdef _DEBUG_LIB3MF
		printf("Build item %d, ref obj id = %d\n", numItems, lmItem->mRefObjId);
#endif
		if (buildItem->HasObjectTransform())
		{
			sTransform lib3mfTransform = buildItem->GetObjectTransform();

			lmItem->bTransformed = true;
			lmItem->mMat.set(lib3mfTransform);

#ifdef _DEBUG_LIB3MF
			printf("\tTransformed.\n");
#endif
		}

		// add to the container of items
		lmItems->push_back(lmItem);
	}

	return numItems;
}


// Construct final transformed, separate meshes from build items and object resources
static
int constructIndexedFaceMeshfromLib3mfItems(std::vector<Lib3mfIntf::MeshObj*>& lib3mfMeshObjs,
	std::vector<Lib3mfIntf::ComponentsObj*>& lib3mfCompsObjs, std::vector<Lib3mfIntf::Item*> lib3mfBuildItems,
	std::vector<LoadMesh::IndexedFaceMesh*>* pMeshes)
{
	int nPrevMeshes = pMeshes->size();

	// Make a map of Mesh objecct to map object id to its index in the std::vector
	std::map<int, int> meshObjectIdMap;	// Object id -> index 
	for (int i = 0; i < lib3mfMeshObjs.size(); i++)
	{
		Lib3mfIntf::MeshObj* pMeshObjId = lib3mfMeshObjs[i];
		assert(pMeshObjId->Type() == Lib3mfIntf::MeshObjectType);
		int objId = pMeshObjId->mObjId;
		assert(meshObjectIdMap.count(objId) == 0);
		meshObjectIdMap[objId] = i;	// loc, index
	}

	// Make a map of components object to map object id to its index in the std::vector
	std::map<int, int> componentsObjectIdMap;	// Object id -> index 
	for (int i = 0; i < lib3mfCompsObjs.size(); i++)
	{
		Lib3mfIntf::ComponentsObj* pCompsObj = lib3mfCompsObjs[i];
		assert(pCompsObj->Type() == Lib3mfIntf::ComponentsObjectType);
		int objId = pCompsObj->mObjId;
		assert(componentsObjectIdMap.count(objId) == 0);
		componentsObjectIdMap[objId] = i; // loc, index
	}

	// Get mesh objects ids and concatenated transforms for individual components-objects.
	// Forward propagation (assumption, rule) : A components-object SHOULD refer to predefined
	// components objects. So the updating will be in assending order, which means that the earlier 
	// one will be predefined than the latter.
	int nCompsObjs = lib3mfCompsObjs.size();
	for (int i = 0; i < nCompsObjs; i++)
	{
		Lib3mfIntf::ComponentsObj*  pCompsObj = lib3mfCompsObjs[i];
		assert(pCompsObj->Type() == Lib3mfIntf::ComponentsObjectType);

		// Collects the meshes of components and concatenate the transform
		int nComps = pCompsObj->mComps.size();

		for (int j = 0; j < nComps; j++)
		{
			Lib3mfIntf::Component* pComp = pCompsObj->mComps[j];

			int refObjId = pComp->mRefObjId;

			// if the referenced one is a mesh object, get its mesh and transform, 
			if (meshObjectIdMap.count(refObjId) > 0)
			{
				pCompsObj->meshObjIds.push_back(refObjId);	// add to the end
				std::vector<LoadMesh::TransformMatrix4X4> vecTransform;

				if (pComp->bTransformed)
					vecTransform.push_back(pComp->mMat);	// concatenate the present transform
				else
					vecTransform.push_back(LoadMesh::TransformMatrix4X4(1));	// a unit matrix

				pCompsObj->vecConcatTransforms.push_back(vecTransform);	// add to the end

			}

			// else if other components-object, get its underlying meshes
			else if (componentsObjectIdMap.count(refObjId) > 0)
			{
				// get referenced components-objects
				int locIndex = componentsObjectIdMap[refObjId];
				Lib3mfIntf::ComponentsObj* pOtherCompsObj = lib3mfCompsObjs[locIndex];

				// get the mesh objects of the referenced components-objects, adding to the end
				pCompsObj->meshObjIds.insert(pCompsObj->meshObjIds.end(),
					pOtherCompsObj->meshObjIds.begin(), pOtherCompsObj->meshObjIds.end());

				// get the transform array for the individual mesh objects
				int nOtherMeshObjs = pOtherCompsObj->meshObjIds.size();
				for (int k = 0; k < nOtherMeshObjs; k++)
				{
					std::vector<LoadMesh::TransformMatrix4X4>* vecTransforms = &(pOtherCompsObj->vecConcatTransforms[k]);
					// first concatenate (add) the present transform, if any
					if (pComp->bTransformed)
						vecTransforms->push_back(pComp->mMat);
					else
						vecTransforms->push_back(LoadMesh::TransformMatrix4X4(1));
					// copy
					pCompsObj->vecConcatTransforms.push_back(*vecTransforms);
				}
			}
			else
			{
				assert(false);
				printf("Unexpected case encountered\n");
			}

			assert(pCompsObj->meshObjIds.size() == pCompsObj->vecConcatTransforms.size());

		}

	}

#ifdef _DEBUG_LIB3MF
	for (int i = 0; i < lib3mfCompsObjs.size(); i++) {
		Lib3mfIntf::ComponentsObj* pCompsObj = lib3mfCompsObjs[i];
		assert(pCompsObj->Type() == Lib3mfIntf::ComponentsObjectType);
		printf("Components #%d, obj id = %d\n", i, pCompsObj->mObjId);

		printf("\tMesh obj ids = ");
		for (int j = 0; j < pCompsObj->meshObjIds.size(); j++) {
			printf("%d, ", pCompsObj->meshObjIds[j]);
		}
		printf("\n");

		printf("\tTransforms : \n");
		for (int j = 0; j < pCompsObj->vecConcatTransforms.size(); j++) {
			for (int k = 0; k < pCompsObj->vecConcatTransforms[j].size(); k++) {
				for (int m = 0; m < 4; m++) {
					for (int n = 0; n < 4; n++) {
						printf("%.3f, ", pCompsObj->vecConcatTransforms[j][k].mMat[m][n]);
					}
					printf("\n");
				}
				printf("\n");
			}
			printf("\n");
		}
	}

#endif

	// Build the final, transformed mesh (real geometry) for individual items

	int nItems = lib3mfBuildItems.size();
	for (int i = 0; i < nItems; i++)
	{
		Lib3mfIntf::Item* pItem = lib3mfBuildItems[i];

		int refObjId = pItem->mRefObjId;

		// if the referenced one is a mesh object, get its mesh and transform directly.
		if (meshObjectIdMap.count(refObjId) > 0) {

			// get referenced mesh object
			int locIndex = meshObjectIdMap[refObjId];
			Lib3mfIntf::MeshObj* pMeshObj = lib3mfMeshObjs[locIndex];

			// build a new indexed face mesh
			LoadMesh::IndexedFaceMesh* idxFcMesh = new LoadMesh::IndexedFaceMesh;
			*idxFcMesh = pMeshObj->mIdxFaceMesh;	// copy

			if (pItem->bTransformed)	// transform it if any
				idxFcMesh->transform(pItem->mMat);

			pMeshes->push_back(idxFcMesh);	// save
		}

		// else if other components-object, get its underlying meshes
		else if (componentsObjectIdMap.count(refObjId) > 0) {

			// get referenced components-objects
			int locIndex = componentsObjectIdMap[refObjId];
			Lib3mfIntf::ComponentsObj* pCompsObj = lib3mfCompsObjs[locIndex];

			// Construct its underlying mesh objects (real geometries)
			int nMeshObjIds = pCompsObj->meshObjIds.size();
			for (int i = 0; i < nMeshObjIds; i++) {

				int meshObjId = pCompsObj->meshObjIds[i];

				// get referenced mesh object
				int locIndex = meshObjectIdMap[meshObjId];
				Lib3mfIntf::MeshObj* pMeshObj = lib3mfMeshObjs[locIndex];

				// build a new indexed face mesh
				LoadMesh::IndexedFaceMesh* idxFcMesh = new LoadMesh::IndexedFaceMesh;
				*idxFcMesh = pMeshObj->mIdxFaceMesh;	// copy

				// apply concatenated transforms
				std::vector<LoadMesh::TransformMatrix4X4>& concatTransform = pCompsObj->vecConcatTransforms[i];
				idxFcMesh->transform(concatTransform);

				//idxFcMesh->transform()

				if (pItem->bTransformed)	// apply item's transform if any
					idxFcMesh->transform(pItem->mMat);

				pMeshes->push_back(idxFcMesh);	// save

			}
		}
	}

	int nPresentMeshes = pMeshes->size();

	return nPresentMeshes - nPrevMeshes;
}

bool convertMeshUnitsToMilliMeterUsingLib3mfModelUnitInfo(Lib3MF::PModel& model, std::vector<LoadMesh::IndexedFaceMesh*>* pIdxFcMeshes)
{
	float scaleFactor = 1;

	Lib3MF::eModelUnit unitModel = model->GetUnit();
	switch (unitModel)
	{
	case Lib3MF::eModelUnit::MicroMeter:
		scaleFactor = 1e-3f;
		break;
	case Lib3MF::eModelUnit::MilliMeter:
		scaleFactor = 1.0f;
		return false;	// Already millimeter, OK. Let's return.
	case Lib3MF::eModelUnit::CentiMeter:
		scaleFactor = 10.0f;
		break;
	case Lib3MF::eModelUnit::Inch:
		scaleFactor = 25.4f;
		break;
	case Lib3MF::eModelUnit::Meter:
		scaleFactor = 1e3f;
		break;
	}

	TransformMatrix4X4 scaleTransform(scaleFactor);

	for (int i = 0; i < pIdxFcMeshes->size(); i++)
	{
		LoadMesh::IndexedFaceMesh* pIdxFcMesh = pIdxFcMeshes->at(i);
		pIdxFcMesh->transform(scaleTransform);
	}

	return true;
}

static
int getIndexedFaceMeshesfromLib3mfModel(Lib3MF::PModel& model, std::vector<LoadMesh::IndexedFaceMesh*>* pIdxFcMeshes)
{
#ifdef _DEBUG
	// Check unit: only milimeter is assumed. If not, we need to convert it properly.
	eModelUnit unitModel = model->GetUnit();
	//printf("Unit = %s\n", (unitModel == eModelUnit::MilliMeter) ? "MiliMeter" : "NOT MiliMeter");
	//assert(unitModel == eModelUnit::MilliMeter);	// unit conversion added.

	// In the unit conversion below, all meshes will be converted. So if some previously-added meshes will also be 
	// converted. To avoid it, you need to some range indices, or use some empty container, finally adding it to the pMesh.
	assert(pIdxFcMeshes->empty());
#endif
	// Object Resources (resuable) - Mesh objects & Components objects 

	// Get information on mesh objects (Triangle and vertices, and object ID)
	std::vector<Lib3mfIntf::MeshObj*> lib3mfMeshObjs;
	std::vector<Lib3mfIntf::ComponentsObj*> lib3mfCompsObjs;
	bool retVal = getObjectInformationFromLib3mfModel(model, &lib3mfMeshObjs, &lib3mfCompsObjs);

	// Get build items
	std::vector<Lib3mfIntf::Item*> lib3mfBuildItems;
	int numBuildItems = getBuildItemsLib3mfModel(model, &lib3mfBuildItems);

	// Construct final transformed, separate intermediate indexed-face meshes from build items and object resources
	int numMeshes = constructIndexedFaceMeshfromLib3mfItems(lib3mfMeshObjs, lib3mfCompsObjs, lib3mfBuildItems, pIdxFcMeshes);

	// Unit conversion
	bool bUnitConverted = convertMeshUnitsToMilliMeterUsingLib3mfModelUnitInfo(model, pIdxFcMeshes);

	// release memory
	for (int i = 0; i < lib3mfMeshObjs.size(); i++)
		delete lib3mfMeshObjs[i];
	for (int i = 0; i < lib3mfCompsObjs.size(); i++)
		delete lib3mfCompsObjs[i];
	for (int i = 0; i < lib3mfBuildItems.size(); i++)
		delete lib3mfBuildItems[i];

	return numMeshes;
}


void progressCallback(bool * shouldAbort, Lib3MF_double ratio, Lib3MF::eProgressIdentifier, Lib3MF_pvoid progressHandler)
{
	assert(ratio == -1 || ratio >= 0 && ratio <= 1.0);
	//printf("percentage = %.2f\n", ratio);
	if (shouldAbort)
	{
		ProgressHandler* temp = (ProgressHandler*)progressHandler;
		temp->setValue((float)ratio);
		*shouldAbort = temp->wasCanceled();
	}
}

int MeshLoader::load_lib3mf(const wchar_t* wFileName, std::vector<Mesh*>* pLoadedMesh, bool usingProgress_)
{
	// Get Buffer Length and Pointer
	int length = getBufferLen(wFileName);
	char *buffer = getBufferPtr(wFileName, length);
	assert(buffer);

	// 	Using lib3mf 2.0.0-beta  
	int numIdxFaceMeshes = 0;
	std::vector<LoadMesh::IndexedFaceMesh*> idxFaceMeshes;
	Lib3MF::PModel model;
	try
	{
		Lib3MF::PWrapper wrapper = Lib3MF::CWrapper::loadLibrary();

		model = wrapper->CreateModel();

		Lib3MF::PReader reader = model->QueryReader("3mf");		// supporting stl and 3mf as of Sep. 2019

		if (usingProgress_)
			reader->SetProgressCallback(progressCallback, progressHandler);

		// buffer

		Lib3MF_uint8* in_data = (Lib3MF_uint8*)buffer;
		size_t in_size = length;		// by char
		CInputVector<Lib3MF_uint8> inputvecBuffer(in_data, length);

		std::cout << "File reading " << wFileName << "..." << std::endl;

		// Read lib3mf model from buffer
		reader->ReadFromBuffer(inputvecBuffer);

		// Neutral indexed-face model: Get intermediate indexed face meshes from the imported 3mf file
		numIdxFaceMeshes = getIndexedFaceMeshesfromLib3mfModel(model, &idxFaceMeshes);

		// release  memory
		delete buffer;

#ifdef _DEBUG_LIB3MF_OFF
		// Just to check if the read-in file is OK
		Lib3MF::PWriter writer = model->QueryWriter("stl");
		std::string testOutFileName("test_lib3mf.stl");
		std::cout << "writing " << testOutFileName << "..." << std::endl;
		writer->WriteToFile(testOutFileName);
		std::cout << "done." << std::endl;
#endif
	}
	catch (ELib3MFException &e)
	{
		std::cout << "Error message : " << e.what() << std::endl;
		std::cout << "Error code : " << e.getErrorCode() << std::endl;

		// release the memory
		delete buffer;

		return -1;
	}

	// Generate Mesh from the indexed face meshes
	int numMeshes = generateMeshesFromIndexedFaceMeshes(idxFaceMeshes, pLoadedMesh);

	// release memory
	for (std::vector<LoadMesh::IndexedFaceMesh*>::iterator itr = idxFaceMeshes.begin(); itr != idxFaceMeshes.end(); itr++)
	{
		LoadMesh::IndexedFaceMesh* idxFcMesh = *itr;
		delete idxFcMesh;
	}

	return numMeshes;
}


// ------------------------------------------
// Progress callback interface for Assimp 

AssimpProgressHandler::AssimpProgressHandler(MeshLoader* pImporter)
{
	assert(pImporter);	 // null is not allow.
	m_importer = pImporter;
}

AssimpProgressHandler::~AssimpProgressHandler()
{
	// Just release m_importer. Do not clean up it here. It will take care of itself.
	m_importer = nullptr;
}

// Callback - called by the assimp importer periodically (though not so periodical) 
// while reading a file
bool AssimpProgressHandler::Update(float percentage)
{
	// As of 2019-08-22, percentage comes out like 0, and 0.5 ~ 1.0, periodically, but not regualaryly. 
	// Maybe Assimp also use some other external library for read reading operation. It may not
	// estimate the first part, putting out zero, waiting a while and then starting from 0.5 to 1.0. 
	assert(percentage == -1 || percentage >= 0 && percentage <= 1.0);

	assert(m_importer);

	// Call MeshLoader's update handler that takes care of whatever it is.
	if (m_continue)
		m_continue = m_importer->updateProgress(percentage);

	// In the case of Assimp, if we return false, the reading process will abort, performing 
	// necesary cleanup. So we can use it for caneling the reading proess of any large file.
#if _DEBUG
	else
		printf("Assimp importing will be aborted.\n");
#endif

	return m_continue;
};

// ------------------------------------------
// Progress callback interface for Others

// .....
