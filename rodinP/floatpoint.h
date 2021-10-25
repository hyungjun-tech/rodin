/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#pragma once

/*
Floating point 3D points are used during model loading as 3D vectors.
They represent millimeters in 3D space.
*/

#include "intpoint.h"

#include <stdint.h>
#include <math.h>

class FPoint3
{
public:
	double x, y, z;
	FPoint3() {}
	FPoint3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
	FPoint3(const Point3& p) : x(p.x*.001), y(p.y*.001), z(p.z*.001) {}

	FPoint3 operator+(const FPoint3& p) const { return FPoint3(x + p.x, y + p.y, z + p.z); }
	FPoint3 operator-(const FPoint3& p) const { return FPoint3(x - p.x, y - p.y, z - p.z); }
	FPoint3 operator*(const double f) const { return FPoint3(x*f, y*f, z*f); }
	FPoint3 operator/(const double f) const { return FPoint3(x / f, y / f, z / f); }

	FPoint3& operator += (const FPoint3& p) { x += p.x; y += p.y; z += p.z; return *this; }
	FPoint3& operator -= (const FPoint3& p) { x -= p.x; y -= p.y; z -= p.z; return *this; }

	bool operator==(FPoint3& p) const { return x == p.x&&y == p.y&&z == p.z; }
	bool operator!=(FPoint3& p) const { return x != p.x || y != p.y || z != p.z; }

	double& operator[](int n){ return getPtr()[n]; }
	double operator[](int n) const { return getPtr()[n]; }

	double * getPtr() { return (double*)&x; }
	const double * getPtr() const { return (const double *)&x; }
	
	double max()
	{
		if (x > y && x > z) return x;
		if (y > z) return y;
		return z;
	}

	bool shorterThen(double len)
	{
		return vSize2() <= len*len;
	}

	double vSize2()
	{
		return x*x + y*y + z*z;
	}

	double vSize()
	{
		return sqrt(vSize2());
	}
	FPoint3 cross(const FPoint3& p)
	{
		return FPoint3(
			y*p.z - z*p.y,
			z*p.x - x*p.z,
			x*p.y - y*p.x);
	}

	static FPoint3 cross(const Point3& a, const Point3& b)
	{
		return FPoint3(a).cross(FPoint3(b));
	}

	double dot(const FPoint3& p)
	{
		return x*p.x + y*p.y + z*p.z;
	}
	Point3 toPoint3()
	{
		return Point3(x * 1000, y * 1000, z * 1000);
	}
	void normalize()
	{
		double len = vSize();
		*this = *this/len;
	}
	FPoint3 normalized()
	{
		double len = vSize();
		return *this/len;
	}
};

inline const FPoint3 operator-(const FPoint3 &vector) { return FPoint3(-vector.x, -vector.y, -vector.z); }
inline float operator*(FPoint3 lhs, const FPoint3& rhs) {
	return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}
class FMatrix3x3
{
public:
	double m[3][3];

	FMatrix3x3()
	{
		m[0][0] = 1.0;
		m[1][0] = 0.0;
		m[2][0] = 0.0;
		m[0][1] = 0.0;
		m[1][1] = 1.0;
		m[2][1] = 0.0;
		m[0][2] = 0.0;
		m[1][2] = 0.0;
		m[2][2] = 1.0;
	}

	Point3 apply(FPoint3 p)
	{
		return Point3(
			MM2INT(p.x * m[0][0] + p.y * m[1][0] + p.z * m[2][0]),
			MM2INT(p.x * m[0][1] + p.y * m[1][1] + p.z * m[2][1]),
			MM2INT(p.x * m[0][2] + p.y * m[1][2] + p.z * m[2][2]));
	}
};