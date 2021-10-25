#pragma once

#include "MeshModel.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel  K;
typedef CGAL::Polyhedron_3<K>                     Polyhedron_3;
typedef K::Segment_3                              Segment_3;
// define point creator
typedef K::Point_3                                Point_3;
typedef Polyhedron_3::Facet_iterator			  Facet_iterator;
typedef Polyhedron_3::Vertex_iterator			  Vertex_iterator;
typedef Polyhedron_3::Vertex_handle				  Vertex_handle;
typedef Polyhedron_3::Facet_handle				  Facet_handle;
typedef Polyhedron_3::Facet						  Facet;
typedef Polyhedron_3::Halfedge_handle			  Halfedge_handle;
//a functor computing the plane containing a triangular facet

struct Plane_from_facet 
{
	Polyhedron_3::Plane_3 operator()(Polyhedron_3::Facet& f) 
	{
		Polyhedron_3::Halfedge_handle h = f.halfedge();

		return Polyhedron_3::Plane_3(h->vertex()->point(),
			h->next()->vertex()->point(),
			h->opposite()->vertex()->point());
	}
};


class ConvexHullStatic
{
public:
	ConvexHullStatic();
	~ConvexHullStatic();
	
	void setMesh(Mesh* mesh_)
	{
		ch_mesh = mesh_;
	}

	void setModel(IMeshModel* model_)
	{
		ch_model = model_;
	}

	void run(float rotationMatrix[4], float vec[4]);
	void clear();
	//void draw();

	struct FacetInfo
	{
		Facet facet;
		double area;
		double angle;
	};
	std::vector<FacetInfo> facetInfoSet;
	void getRotationMatrix(float mat[4], float vec[4], int i);
private:
	Polyhedron_3 poly;
	std::vector<Point_3> inputPts;

	//Volume* ch_Volume;
	Mesh* ch_mesh;
	IMeshModel* ch_model;

	void init();
	void sortFaceHandles(float rotationMatrix[4], float vec[4]);
	void setUpRotationMatrix(float mat[4], float vec[4], FacetInfo fa);
	
	void facetInfoUpdate(Facet_iterator fit, float vec[4]);	
	double calcF_Area(Point_3& _p, Point_3& _np, Point_3& _op);
	Point_3 getNormal(const Point_3& _p, const Point_3& _np, const Point_3& _op);
	Point_3 getNormal(const Point_3& a, const Point_3& b);
};

