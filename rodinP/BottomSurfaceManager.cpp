#include "stdafx.h"
#include "BottomSurfaceManager.h"

//#include <Eigen/Dense>
//#include <stdio.h>

BottomSurfaceManager::BottomSurfaceManager() {}
BottomSurfaceManager::~BottomSurfaceManager() {}

/// CGAL ¡ê OpenMesh Conversion Function ---------------------------------------------------
/*BottomSurfaceManager::CGALMesh *BottomSurfaceManager::MeshToCGAL(Mesh* input_)
{
	// Reset CGALMesh
	CGALMesh* output = new CGALMesh;

	// Conver Vertices
	Mesh::Point p;
	for (int v = 0; v < input_->n_vertices(); v++) {
		p = input_->point(input_->vertex_handle(v));
		output->add_vertex(K::Point_3(p[0], p[1], p[2]));
	}

	// Convert Faces
	CGAL::SM_Vertex_index vidx[3];
	for (int f = 0; f < input_->n_faces(); f++) {
		auto Cfv_it = input_->cfv_iter(Mesh::FaceHandle(input_->face_handle(f)));
		vidx[0] = CGAL::SM_Vertex_index(Cfv_it->idx()); // ¡æ¡æ¡æ // Cfv_it++;
		vidx[1] = CGAL::SM_Vertex_index(Cfv_it->idx()); // ¡æ¡æ¡æ // Cfv_it++;
		vidx[2] = CGAL::SM_Vertex_index(Cfv_it->idx());

		output->add_face(vidx[0], vidx[1], vidx[2]);
	}

	//delete input_;
	return output;
}*/

/*Mesh *BottomSurfaceManager::CGALToMesh(CGALMesh* input_)
{
	// Reset Mesh
	Mesh* output = new Mesh();

	// Convert Vertices
	int n_vertices = (int)input_->number_of_vertices();
	K::Point_3 p;
	CGALMesh::Vertex_range v = input_->vertices();
	CGALMesh::Vertex_range::iterator  vb, ve;
	vb = v.begin(), ve = v.end();
	for (boost::tie(vb, ve) = v; vb != ve; ++vb) {
		p = input_->point(*vb);
		output->add_vertex(Mesh::Point(p.x(), p.y(), p.z()));
	}

	// Convert Faces
	int idx[3];
	std::vector<Mesh::VertexHandle> triangle(3);

	CGALMesh::Face_range::iterator fb, fe;
	fe = input_->faces_end();
	for (fb = input_->faces_begin(); fb != fe; ++fb) {
		halfedge_descriptor heh = input_->halfedge(*fb);
		CGAL::Vertex_around_face_iterator<CGALMesh> fvb, fve;
		int i = 0;
		for (boost::tie(fvb, fve) = vertices_around_face(heh, *input_);
			fvb != fve;
			++fvb) {
			idx[i] = (int)(*fvb);
			triangle[i] = output->vertex_handle(idx[i]);
			i++;
		}
		output->add_face(triangle);
	}
	output->request_vertex_normals();
	output->request_face_normals();
	output->update_normals();

	delete input_;
	return output;
}*/

std::vector<qglviewer::Vec> BottomSurfaceManager::GenerateBottomMesh(Mesh *outer_, qglviewer::Vec wPos_, qglviewer::Vec wNorm_)
{
	qglviewer::Vec b_v[3];
	std::list<Triangle> triangles;

	for (Mesh::FaceIter fit = outer_->faces_begin(); fit != outer_->faces_end(); fit++)
	{
		Mesh::FVIter fvit = outer_->fv_begin(fit);
		Mesh::Point p[3];
		p[0] = outer_->point(fvit++);
		p[1] = outer_->point(fvit++);
		p[2] = outer_->point(fvit);
		Point pt[3];
		pt[0] = Point(p[0][0], p[0][1], p[0][2]);
		pt[1] = Point(p[1][0], p[1][1], p[1][2]);
		pt[2] = Point(p[2][0], p[2][1], p[2][2]);
		triangles.push_back(Triangle(pt[0], pt[1], pt[2]));
	}

	Tree tree(triangles.begin(), triangles.end());

	Point query(wPos_.x, wPos_.y, wPos_.z);
	Triangle closestTriangle;

	try {
		FT sqd = tree.squared_distance(query);
		Point_and_primitive_id pp = tree.closest_point_and_primitive(query);
		Point closest_point = pp.first;
		Iterator& closestTriangleIter = pp.second;

		closestTriangle = *closestTriangleIter;
		Point v0 = closestTriangle.vertex(0);
		Point v1 = closestTriangle.vertex(1);
		Point v2 = closestTriangle.vertex(2);

		b_v[0].x = v0[0];
		b_v[0].y = v0[1];
		b_v[0].z = v0[2];
		b_v[1].x = v1[0];
		b_v[1].y = v1[1];
		b_v[1].z = v1[2];
		b_v[2].x = v2[0];
		b_v[2].y = v2[1];
		b_v[2].z = v2[2];
	}
	catch (QString) {
		throw;
	}
	std::vector<qglviewer::Vec> vertexList;
	vertexList.push_back(b_v[0]);
	vertexList.push_back(b_v[1]);
	vertexList.push_back(b_v[2]);

#ifdef _DEBUG
	qglviewer::Vec temp;
	temp = vertexList[0];
	temp = vertexList[1];
	temp = vertexList[2];
	//temp;
#endif // 

	return vertexList;
}