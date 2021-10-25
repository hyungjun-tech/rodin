#pragma once

#include "Mesh.h"

namespace PMP = CGAL::Polygon_mesh_processing;
namespace params = PMP::parameters;

class BottomSurfaceManager
{
public:
	BottomSurfaceManager();
	~BottomSurfaceManager();

	std::vector<qglviewer::Vec> GenerateBottomMesh(Mesh *outer_, qglviewer::Vec wPos_, qglviewer::Vec wNorm_);

private:

	typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
	typedef CGAL::Surface_mesh<K::Point_3> CGALMesh;

	typedef boost::graph_traits<CGALMesh>::vertex_descriptor vertex_descriptor;
	typedef boost::graph_traits<CGALMesh>::edge_descriptor edge_descriptor;
	typedef boost::graph_traits<CGALMesh>::face_descriptor face_descriptor;
	typedef boost::graph_traits<CGALMesh>::halfedge_descriptor halfedge_descriptor;

	/// CGAL ¡ê OpenMesh Conversion Function ---------------------------------------------------
//	CGALMesh *MeshToCGAL(Mesh* mesh_);
//	Mesh *CGALToMesh(CGALMesh* cgal_);

	typedef CGAL::Simple_cartesian<double> K2;
	typedef K2::FT FT;
//	typedef K2::Ray_3 Ray;
//	typedef K2::Line_3 Line;
	typedef K2::Point_3 Point;
	typedef K2::Triangle_3 Triangle;
	typedef std::list<Triangle>::iterator Iterator;
	typedef CGAL::AABB_triangle_primitive<K2, Iterator> Primitive;
	typedef CGAL::AABB_traits<K2, Primitive> AABB_triangle_traits;
	typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;
//	typedef boost::optional< Tree::Intersection_and_primitive_id<Ray>::Type > Ray_intersection;

	typedef Tree::Point_and_primitive_id Point_and_primitive_id;
};