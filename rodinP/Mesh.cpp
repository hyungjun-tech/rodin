#include "stdafx.h"
#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::append(const Mesh &new_)
{
	int n = n_vertices();

	// Append Vertices
	for (Mesh::VertexIter vit = new_.vertices_begin();
		vit != new_.vertices_end(); vit++)
		add_vertex(new_.point(vit));	

	// Append Faces
	std::vector<Mesh::VertexHandle> new_triangle(3);	
	for (Mesh::ConstFaceIter fit = new_.faces_begin();
		fit != new_.faces_end(); fit++) {
		Mesh::ConstFaceVertexIter fvit = new_.cfv_begin(fit);
		new_triangle[0] = vertex_handle(n + fvit++->idx());
		new_triangle[1] = vertex_handle(n + fvit++->idx());
		new_triangle[2] = vertex_handle(n + fvit->idx());

		add_face(new_triangle);
	}	

	update_normals();
}

void Mesh::flipWinding()
{
	Mesh *copied = new Mesh(*this);

	clear();
	for (Mesh::VertexIter vit = copied->vertices_begin(); vit != copied->vertices_end(); ++vit)
		add_vertex(copied->point(vit));
	
	for (Mesh::FaceIter fit = copied->faces_begin(); fit != copied->faces_end(); ++fit) {
		std::vector<int> vertexIDs;
		for (Mesh::FVIter fvit = copied->fv_begin(fit); fvit != copied->fv_end(fit); ++fvit)
			vertexIDs.push_back(fvit->idx());

		std::vector<VertexHandle> vhs;
		for (std::vector<int>::reverse_iterator rit = vertexIDs.rbegin(); 
			rit != vertexIDs.rend(); 
			++rit)
			vhs.push_back(vertex_handle(*rit));
		add_face(vhs);
	}

	update_normals();

	delete copied;
}

void Mesh::scaled(qglviewer::Vec scale_)
{
	qglviewer::Vec factor(scale_.x, scale_.y, scale_.z);
	for (Mesh::VertexIter vIter = vertices_begin(); 
		vIter != vertices_end(); 
		vIter++)
	{
		Mesh::Point originalPt = point(vIter);
		Mesh::Point scaleApplied(
			originalPt[0] * factor[0],
			originalPt[1] * factor[1], 
			originalPt[2] * factor[2]);

		set_point(vIter, scaleApplied);
	}

	// Update Normals
	update_normals();
}

void Mesh::worldScaled(qglviewer::Vec scale_, ScaledFrame &frame_)
{
	qglviewer::Vec factor(scale_.x, scale_.y, scale_.z);
	qglviewer::Vec pos = frame_.frame.position();
	//qDebug() << pos[0] << pos[1] << pos[2];
	for (Mesh::VertexIter vit = vertices_begin();
		vit != vertices_end();
		vit++)
	{
		Mesh::Point lp = point(vit);
		qglviewer::Vec wp = frame_.toWorldCoords(qglviewer::Vec(lp[0], lp[1], lp[2]));
		//qDebug() << wp[0] << wp[1] << wp[2];
		Mesh::Point scaleApplied(
			(wp[0] - pos[0]) * factor[0] + pos[0],
			(wp[1] - pos[1]) * factor[1] + pos[1],
			(wp[2] - pos[2]) * factor[2] + pos[2]);
		qglviewer::Vec scaledLp =
			frame_.toLocalCoords(
				qglviewer::Vec(scaleApplied[0], scaleApplied[1], scaleApplied[2]));

		set_point(vit, Mesh::Point(scaledLp[0], scaledLp[1], scaledLp[2]));
	}
	update_normals();
}

void Mesh::translate(qglviewer::Vec trans_)
{
	//qglviewer::Vec factor(scale_.x, scale_.y, scale_.z);
	for (Mesh::VertexIter vIter = vertices_begin();
		vIter != vertices_end();
		vIter++)
	{
		Mesh::Point originalPt = point(vIter);
		Mesh::Point translated(
			originalPt[0] + trans_[0],
			originalPt[1] + trans_[1],
			originalPt[2] + trans_[2]);

		set_point(vIter, translated);
	}

	// Update Normals
	//update_normals();
}
