#pragma once
#include "ScaledFrame.h"

struct MeshTraits : public OpenMesh::DefaultTraits
{
	typedef OpenMesh::Vec3f Point;
	typedef OpenMesh::Vec3f Normal;
	typedef OpenMesh::Vec3f Color;
	
	VertexAttributes(
		OpenMesh::Attributes::Status |
		OpenMesh::Attributes::Normal |
		OpenMesh::Attributes::Color);
	FaceAttributes(
		OpenMesh::Attributes::Status |
		OpenMesh::Attributes::Normal |
		OpenMesh::Attributes::Color);
	EdgeAttributes(OpenMesh::Attributes::Status);
};

class Mesh : public OpenMesh::TriMesh_ArrayKernelT<MeshTraits>
{
public:
	Mesh();
	~Mesh();

	void append(const Mesh &mesh_);
	void flipWinding();
	void scaled(qglviewer::Vec scale_);
	void worldScaled(qglviewer::Vec scale_, ScaledFrame &frame_);
	void translate(qglviewer::Vec trans_);
private:
	//현재 rodin은 serialize 사용하지 않으므로 일단 제거
	/*friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & n_vertices();
		ar & n_faces();

		for (Mesh::ConstVertexIter vit = vertices_begin();
			vit != vertices_end(); ++vit)
		{
			Mesh::Point p = point(vit);
			for (int i = 0; i < 3; i++)
				ar & p[i];
		}
		for (Mesh::ConstFaceIter fit = faces_begin(); fit != faces_end(); ++fit)
		{
			for (Mesh::CFVIter fvit = cfv_begin(fit); fvit != cfv_end(fit); ++fvit)
			{
				ar & (fvit->idx());
			}
		}
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		int verticesN;
		ar & verticesN;
		int facesN;
		ar &facesN;

		for (int i = 0; i < verticesN; i++)
		{
			float x, y, z;
			ar & x;
			ar & y;
			ar & z;

			add_vertex(Mesh::Point(x, y, z));
		}

		for (int i = 0; i < facesN; i++)
		{
			int idx1, idx2, idx3;
			ar & idx1;
			ar & idx2;
			ar & idx3;
			add_face(
				vertex_handle(idx1),
				vertex_handle(idx2),
				vertex_handle(idx3));
		}

		update_normals();
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()*/
};