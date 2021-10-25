#include "stdafx.h"
#include "AABB.h"

AABB::AABB()
	: x0(FLT_MAX),
	y0(FLT_MAX),
	z0(FLT_MAX),
	x1(-FLT_MAX),
	y1(-FLT_MAX),
	z1(-FLT_MAX)
{

}
AABB::AABB(const AABB &aabb_) 
	: x0(aabb_.x0),
	y0(aabb_.y0),
	z0(aabb_.z0),
	x1(aabb_.x1),
	y1(aabb_.y1),
	z1(aabb_.z1)
{

}
AABB::~AABB()
{

}

AABB& AABB::operator=(const AABB &rhs_)
{
	x0 = rhs_.x0;
	y0 = rhs_.y0;
	z0 = rhs_.z0;

	x1 = rhs_.x1;
	y1 = rhs_.y1;
	z1 = rhs_.z1;

	return *this;
}

void AABB::clear()
{
	x0 = FLT_MAX;
	y0 = FLT_MAX;
	z0 = FLT_MAX;

	x1 = -FLT_MAX;
	y1 = -FLT_MAX;
	z1 = -FLT_MAX;
}

void AABB::expand(qglviewer::Vec v_)
{
	if (x0 > v_[0])
		x0 = v_[0];
	if (y0 > v_[1])
		y0 = v_[1];
	if (z0 > v_[2])
		z0 = v_[2];

	if (x1 < v_[0])
		x1 = v_[0];
	if (y1 < v_[1])
		y1 = v_[1];
	if (z1 < v_[2])
		z1 = v_[2];
}

void AABB::translate(qglviewer::Vec v_)
{
	x0 += v_.x;
	y0 += v_.y;
	z0 += v_.z;

	x1 += v_.x;
	y1 += v_.y;
	z1 += v_.z;
}