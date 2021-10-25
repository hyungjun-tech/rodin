#pragma once
#include "intpoint.h"
#include "polygon.h"

class AABB
{
public:
	AABB();
	AABB(const AABB &aabb_);
	~AABB();

	AABB& operator=(const AABB &rhs_);

	// x0/y1/z1---x1/y1/z1
	//      /           /|
	//     /           / |
	// x0/y0/z1 -- x1/y0/z1
	//   |           |   |
	//   |           |           
	//   |           |  /
	//   |           | /
	//x0/y0/z0 ----- x1/y0/z0

	void clear();
	void expand(qglviewer::Vec v_);

	void translate(qglviewer::Vec v_);

	qglviewer::Vec getMinimum() const
	{
		return qglviewer::Vec(x0, y0, z0);
	}
	qglviewer::Vec getMaximum() const
	{ 
		return qglviewer::Vec(x1, y1, z1); 
	}
	qglviewer::Vec getCenter() const
	{
		return qglviewer::Vec(x0+x1, y0+y1, z0+z1)/2.0; 
	}
	qglviewer::Vec getFloorCenter() const
	{
		return qglviewer::Vec((x0 + x1) / 2.0, (y0 + y1) / 2.0, z0); 
	}

	float getLengthX() const { return x1 - x0; }
	float getLengthY() const { return y1 - y0; }
	float getLengthZ() const { return z1 - z0; }

	float x0, x1;
	float y0, y1;
	float z0, z1;

//현재 rodin은 serialize 사용하지 않으므로 일단 제거
//private:
//	friend class boost::serialization::access;
//	template<class Archive>
//	void serialize(Archive & ar, const unsigned int version)
//	{
//		ar & x0;
//		ar & y0;
//		ar & z0;
//		ar & x1;
//		ar & y1;
//		ar & z1;
//	}
};


class AABB2D
{
public:
	Point min, max;

	AABB2D()
		: min(POINT_MIN, POINT_MIN), max(POINT_MIN, POINT_MIN)
	{
	}
	AABB2D(Point&min, Point& max)
		: min(min), max(max)
	{
	}
	AABB2D(engine::Polygons& polys)
		: min(POINT_MIN, POINT_MIN), max(POINT_MIN, POINT_MIN)
	{
		calculate(polys);
	}

	void calculate(engine::Polygons& polys)
	{
		min = Point(POINT_MAX, POINT_MAX);
		max = Point(POINT_MIN, POINT_MIN);
		for (unsigned int i = 0; i < polys.size(); i++)
		{
			for (unsigned int j = 0; j < polys[i].size(); j++)
			{
				if (min.X > polys[i][j].X) min.X = polys[i][j].X;
				if (min.Y > polys[i][j].Y) min.Y = polys[i][j].Y;
				if (max.X < polys[i][j].X) max.X = polys[i][j].X;
				if (max.Y < polys[i][j].Y) max.Y = polys[i][j].Y;
			}
		}
	}

	bool hit(const AABB2D& other) const
	{
		if (max.X < other.min.X) return false;
		if (min.X > other.max.X) return false;
		if (max.Y < other.min.Y) return false;
		if (min.Y > other.max.Y) return false;
		return true;
	}
};