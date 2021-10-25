#pragma once
#include "stdafx.h"
#include "ConvexHullStatic.h"

ConvexHullStatic::ConvexHullStatic()
	:ch_mesh(nullptr),
	ch_model(nullptr)
{
	
}

ConvexHullStatic::~ConvexHullStatic()
{
}

void ConvexHullStatic::clear()
{
	facetInfoSet.clear();
	inputPts.clear();
	//ch_Volume = NULL;
	ch_mesh = nullptr;
	ch_model = nullptr;
	poly.clear();
	
}

void ConvexHullStatic::init()
{
	inputPts.clear();

	for (int i = 0; i < ch_mesh->n_vertices(); ++i)
	{
		Mesh::Point mesh_point = ch_mesh->point(ch_mesh->vertex_handle(i));

		Point_3 p(mesh_point[0], mesh_point[1], mesh_point[2]);
		inputPts.push_back(p);
	}
}

void ConvexHullStatic::run(float rotationMatrix[4], float vec[4])
{
	init();

	int tic = clock();
	
	// compute convex hull of non-collinear InputPts //
	CGAL::convex_hull_3(inputPts.begin(), inputPts.end(), poly);

	// assign a plane equation to each polyhedron facet using functor // Plane_from_facet
	std::transform(poly.facets_begin(), poly.facets_end(), poly.planes_begin(), Plane_from_facet());
	printf("Make ConvexHull : %lf sec\n", (clock() - tic) / (double)CLOCKS_PER_SEC);

	sortFaceHandles(rotationMatrix, vec);
	setUpRotationMatrix(rotationMatrix, vec, facetInfoSet[0]);
}

void ConvexHullStatic::sortFaceHandles(float rotationMatrix[4], float vec[4])
{
	facetInfoSet.clear();

	for (Facet_iterator fit = poly.facets_begin(); fit != poly.facets_end(); fit++)
	{
		facetInfoUpdate(fit, vec);
	}

	std::sort(facetInfoSet.begin(), facetInfoSet.end(), [](const FacetInfo& a, const FacetInfo& b)->bool
	{
		return a.area / (180 - a.angle) > b.area / (180 - b.angle);
	});
}

void ConvexHullStatic::facetInfoUpdate(Facet_iterator fit, float vec[4])
{
	Facet_handle f = fit;
	Facet facet = *f;
	Halfedge_handle h = facet.halfedge();
	Point_3 p = h->vertex()->point();
	Point_3 np = h->next()->vertex()->point();
	Point_3 op = h->opposite()->vertex()->point();

	FacetInfo fInfo;
	fInfo.facet = facet;
	fInfo.area = calcF_Area(p, np, op);

	////CGAL FAcet 자료구조에 normal update 안되있음....  -> 되있는듯 Rendering 때매 헷갈렸음
	////Normal Update 위해 안전하게 젤 먼 놈 찾음, 시간 오래 소요 안할것, 하면 가까운놈 찾아도되지만 Error원인이 될수도 있으니... Tight한 ConvexHull에서
	//Point_3 farthestPt_from_facet;
	//double farthest_dist = -1;
	//for (int i = 0; i < ChullPts.size(); i++)
	//{
	//	double dist_ = dist(p, ChullPts[i]);
	//	if (dist_ > farthest_dist)
	//	{
	//		farthest_dist = dist_;
	//		farthestPt_from_facet = ChullPts[i];
	//	}
	//}
	//
	//Point_3 n = getNormal(p, np, op);
	//Point_3 diff(farthestPt_from_facet.x() - p.x(), farthestPt_from_facet.y() - p.y(), farthestPt_from_facet.z() - p.z());
	//if (dot(n, diff) > 0)
	//{
	//	printf("역시 정렬이 잘 안되어 있군\n");
	//	fInfo.normal = Point_3(-n.x(), -n.y(), -n.z());
	//}
	//else
	//{
	//	printf(" 정렬 잘 됬는데???\n");
	//	fInfo.normal = Point_3(n.x(), n.y(), n.z());
	//}
	//	
	//
	//n = fInfo.normal;

	Point_3 n = getNormal(p, np, op);
	fInfo.angle = acos(n.x() * vec[0] + n.y() * vec[1] + n.z() * vec[2]) * 180 / M_PI;

	facetInfoSet.push_back(fInfo);
}

void ConvexHullStatic::setUpRotationMatrix(float mat[4], float vec[4], FacetInfo fa)
{
	Facet facet = fa.facet;
	Halfedge_handle h = facet.halfedge();
	Point_3 p = h->vertex()->point();
	Point_3 np = h->next()->vertex()->point();
	Point_3 op = h->opposite()->vertex()->point();
	Point_3 n_ = getNormal(p, np, op);
	Point_3 n(-n_.x(), -n_.y(), -n_.z());

	Point_3 z(vec[0], vec[1], vec[2]);
	Point_3 axis = getNormal(n, z);

	float angle, u, v, w;
	angle = acos(n.x() * vec[0] + n.y() * vec[1] + n.z() * vec[2]) * 180 / M_PI;

	mat[0] = angle;
	mat[1] = axis[0];
	mat[2] = axis[1];
	mat[3] = axis[2];
}

void ConvexHullStatic::getRotationMatrix(float mat[4], float vec[4], int i)
{
	Facet facet = facetInfoSet[i].facet;
	Halfedge_handle h = facet.halfedge();
	Point_3 p = h->vertex()->point();
	Point_3 np = h->next()->vertex()->point();
	Point_3 op = h->opposite()->vertex()->point();
	Point_3 n_ = getNormal(p, np, op);
	Point_3 n(-n_.x(), -n_.y(), -n_.z());

	Point_3 z(vec[0], vec[1], vec[2]);
	Point_3 axis = getNormal(n, z);

	float angle, u, v, w;
	angle = acos(n.x() * vec[0] + n.y() * vec[1] + n.z() * vec[2]) * 180 / M_PI;

	mat[0] = angle;
	mat[1] = axis[0];
	mat[2] = axis[1];
	mat[3] = axis[2];
}

double ConvexHullStatic::calcF_Area(Point_3& _p, Point_3& _np, Point_3& _op)
{
	double a = sqrtf((_p.x() - _np.x())*(_p.x() - _np.x()) + (_p.y() - _np.y())*(_p.y() - _np.y()) + (_p.z() - _np.z())*(_p.z() - _np.z()));
	double b = sqrtf((_np.x() - _op.x())*(_np.x() - _op.x()) + (_np.y() - _op.y())*(_np.y() - _op.y()) + (_np.z() - _op.z())*(_np.z() - _op.z()));
	double c = sqrtf((_op.x() - _p.x())*(_op.x() - _p.x()) + (_op.y() - _p.y())*(_op.y() - _p.y()) + (_op.z() - _p.z())*(_op.z() - _p.z()));

	double s = (a + b + c) / 2;

	return sqrt(s*(s - a)*(s - b)*(s - c));
}

Point_3 ConvexHullStatic::getNormal(const Point_3& _p, const Point_3& _np, const Point_3& _op)
{
	Point_3 a((_np.x() - _p.x()), (_np.y() - _p.y()), (_np.z() - _p.z()));
	Point_3 b((_op.x() - _np.x()), (_op.y() - _np.y()), (_op.z() - _np.z()));
	Point_3 normal((a.y()*b.z() - a.z()*b.y()), (a.z()*b.x() - a.x()*b.z()), (a.x()*b.y() - a.y()*b.x()));
	double size = sqrtf(normal.x()*normal.x() + normal.y()*normal.y() + normal.z()*normal.z());
	Point_3 normal_(normal.x() / size, normal.y() / size, normal.z() / size);
	return normal_;
}

Point_3 ConvexHullStatic::getNormal(const Point_3& a, const Point_3& b)
{
	Point_3 normal((a.y()*b.z() - a.z()*b.y()), (a.z()*b.x() - a.x()*b.z()), (a.x()*b.y() - a.y()*b.x()));
	return normal;
}

//void StaticChull::draw()
//{
//	glColor3f(1.0, 0, 0);
//	for (int i = 0; i < FacetInfoSet.size(); i++)
//	{
//		glBegin(GL_TRIANGLES);
//		Facet facet = FacetInfoSet[i].facet;
//		Halfedge_handle h = facet.halfedge();
//		Point_3 p = h->vertex()->point();
//		Point_3 np = h->next()->vertex()->point();
//		Point_3 op = h->opposite()->vertex()->point();
//		Point_3 nnp = h->next()->next()->vertex()->point();
//	
//		glVertex3f(p.x(), p.y(), p.z());
//		glVertex3f(np.x(), np.y(), np.z());
//		glVertex3f(op.x(), op.y(), op.z());
//
//		Point_3 n = getNormal(p, np, op);
//		glNormal3f(n.x(), n.y(), n.z());
//
//		glEnd();
//	}
//}