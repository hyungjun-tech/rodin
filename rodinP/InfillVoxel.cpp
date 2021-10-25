#include "stdafx.h"
#include "InfillVoxel.h"
#include "AABB.h"

InfillVoxel::InfillVoxel(void)
	: mesh(nullptr)
	, voxel_cell(nullptr)
{
	wall[XY] = nullptr;
	wall[YZ] = nullptr;
	wall[ZX] = nullptr;

	search[XY] = nullptr;
	search[YZ] = nullptr;
	search[ZX] = nullptr;

	R[0][0] = sqrt(6.0)/3.0;
	R[1][0] = -sqrt(6.0)/6.0;
	R[2][0] = -sqrt(6.0)/6.0;

	R[0][1] = 0;
	R[1][1] = sqrt(2.0)/2.0;
	R[2][1] = -sqrt(2.0)/2.0;

	R[0][2] = sqrt(3.0)/3.0;
	R[1][2] = sqrt(3.0)/3.0;
	R[2][2] = sqrt(3.0)/3.0;

	Rinv[0][0] = sqrt(6.0)/3.0;
	Rinv[0][1] = -sqrt(6.0)/6.0;
	Rinv[0][2] = -sqrt(6.0)/6.0;
	
	Rinv[1][0] = 0;
	Rinv[1][1] = sqrt(2.0)/2.0;
	Rinv[1][2] = -sqrt(2.0)/2.0;
	
	Rinv[2][0] = sqrt(3.0)/3.0;
	Rinv[2][1] = sqrt(3.0)/3.0;
	Rinv[2][2] = sqrt(3.0)/3.0;

	double scale = 1.0;
	double a = (sqrt(24 - 8 * scale*scale) + 2 * scale) / 6;
	double b = (-sqrt(24 - 8 * scale*scale) + 4 * scale) / 12;

	skew_R[0][0] = a;
	skew_R[1][0] = b;
	skew_R[2][0] = b;

	skew_R[0][1] = b;
	skew_R[1][1] = a;
	skew_R[2][1] = b;

	skew_R[0][2] = b;
	skew_R[1][2] = b;
	skew_R[2][2] = a;

	double c = (a*a - b*b) / (a*a*a + 2 * b*b*b - 3 * a*b*b);
	double d = (b*b - a*b) / (a*a*a + 2 * b*b*b - 3 * a*b*b);


	skew_Rinv[0][0] = c;
	skew_Rinv[1][0] = d;
	skew_Rinv[2][0] = d;

	skew_Rinv[0][1] = d;
	skew_Rinv[1][1] = c;
	skew_Rinv[2][1] = d;

	skew_Rinv[0][2] = d;
	skew_Rinv[1][2] = d;
	skew_Rinv[2][2] = c;

	temp_wall[XY] = nullptr;
	temp_wall[YZ] = nullptr;
	temp_wall[ZX] = nullptr;
}

void InfillVoxel::clear()
{
	if(voxel_cell != nullptr)
	{
		delete[] voxel_cell;
		voxel_cell = nullptr;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (wall[i] != nullptr)
		{
			delete[] wall[i];
			wall[i] = nullptr;
		}
		if (search[i] != nullptr)
		{
			delete[] search[i];
			search[i] = nullptr;
		}
		if (temp_wall[i] != nullptr)
		{
			delete[] temp_wall[i];
			temp_wall[i] = nullptr;
		}
	}
}


InfillVoxel::~InfillVoxel(void)
{
	clear();
}

void InfillVoxel::rotateMesh()
{
	pts.clear();
	for (Mesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		Mesh::Point p = mesh->point(v_it);

		FPoint3 temp1 = FPoint3(p[0], p[1], p[2]);
		FPoint3 temp2;
		temp2.x = R[0][0] * temp1.x + R[0][1] * temp1.y + R[0][2] * temp1.z;
		temp2.y = R[1][0] * temp1.x + R[1][1] * temp1.y + R[1][2] * temp1.z;
		temp2.z = R[2][0] * temp1.x + R[2][1] * temp1.y + R[2][2] * temp1.z;

		temp1.x = skew_R[0][0] * temp2.x + skew_R[0][1] * temp2.y + skew_R[0][2] * temp2.z;
		temp1.y = skew_R[1][0] * temp2.x + skew_R[1][1] * temp2.y + skew_R[1][2] * temp2.z;
		temp1.z = skew_R[2][0] * temp2.x + skew_R[2][1] * temp2.y + skew_R[2][2] * temp2.z;

		pts.push_back(temp1);
	}
}

void InfillVoxel::setMesh(Mesh* mesh_)
{
	mesh = mesh_;
}

void InfillVoxel::makeVoxel( float s )
{
	if (pts.size() == 0)
		return;
	clear();
	space = s;

	FPoint3 max_pt, min_pt;

	max_pt = min_pt = pts[0];
	for (int i = 1; i < pts.size(); ++i)
	{
		if (max_pt.x < pts[i].x) max_pt.x = pts[i].x;
		if (max_pt.y < pts[i].y) max_pt.y = pts[i].y;
		if (max_pt.z < pts[i].z) max_pt.z = pts[i].z;
		
		if (min_pt.x > pts[i].x) min_pt.x = pts[i].x;
		if (min_pt.y > pts[i].y) min_pt.y = pts[i].y;
		if (min_pt.z > pts[i].z) min_pt.z = pts[i].z;
	}

	max_pt = max_pt + FPoint3(s, s, s) * 3;
	min_pt = min_pt - FPoint3(s, s, s) * 3;

	origin = min_pt;

	Nx = ceil((max_pt.x - min_pt.x)/s);
	Ny = ceil((max_pt.y - min_pt.y)/s);
	Nz = ceil((max_pt.z - min_pt.z)/s);

	voxel_cell = new int[(Nx-1)*(Ny-1)*(Nz-1)];
	for (int i = 0; i < 3; ++i)
		wall[i] = new int[(Nx-1)*(Ny-1)*(Nz-1)];

	for(int i = 0; i < (Nx-1)*(Ny-1)*(Nz-1); ++i)
	{
		voxel_cell[i] = INSIDE;		
		wall[XY][i] = INSIDE;
		wall[YZ][i] = INSIDE;
		wall[ZX][i] = INSIDE;
	}

	search[XY] = new bool[(Nx - 1)*(Ny - 1)];
	search[YZ] = new bool[(Ny - 1)*(Nz - 1)];
	search[ZX] = new bool[(Nz - 1)*(Nx - 1)];

	temp_wall[XY] = new int[(Nx - 1)*(Ny - 1)];
	temp_wall[YZ] = new int[(Ny - 1)*(Nz - 1)];
	temp_wall[ZX] = new int[(Nz - 1)*(Nx - 1)];

	printf("Number of voxel for infill : %d, %d, %d\n", Nx, Ny, Nz);
}

void InfillVoxel::updateVoxel()
{
	double angle = sin(45.0 / 180.0*M_PI);

	for (Mesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
	{
		Mesh::FaceHalfedgeIter fh_it = mesh->fh_iter(f_it.handle());
		std::vector<Mesh::Point> points;
		std::vector<int> idx;
		for (; fh_it.is_valid(); ++fh_it) {
			Mesh::VertexHandle vh = mesh->from_vertex_handle(*fh_it);
			points.push_back(mesh->point(vh));
			idx.push_back(vh.idx());
		}
		if (points.size() != 3)
			continue;

		FPoint3 pt[3];
		pt[0] = FPoint3(points[0][0], points[0][1], points[0][2]);
		pt[1] = FPoint3(points[1][0], points[1][1], points[1][2]);
		pt[2] = FPoint3(points[2][0], points[2][1], points[2][2]);

		FPoint3 nor = (pt[1] - pt[0]).cross(pt[2] - pt[0]).normalized();
		FPoint3 index_f[3];

		// for points
		for (int j = 0; j < 3; ++j)
		{
			index_f[j].x = (pts[idx[j]].x - origin.x) / space;
			index_f[j].y = (pts[idx[j]].y - origin.y) / space;
			index_f[j].z = (pts[idx[j]].z - origin.z) / space;
			int x = (int)(index_f[j].x);
			int y = (int)(index_f[j].y);
			int z = (int)(index_f[j].z);
			if (infill_type == M1 || (infill_type == M2 && nor.z <= angle))
			{
				if (voxel_cell[x*(Ny - 1)*(Nz - 1) + y*(Nz - 1) + z] != FLAT_BOUND)
					voxel_cell[x*(Ny - 1)*(Nz - 1) + y*(Nz - 1) + z] = BOUNDARY;	// non-flat boundary
			}
			else
				voxel_cell[x*(Ny - 1)*(Nz - 1) + y*(Nz - 1) + z] = FLAT_BOUND;	// flat boundary
		}

		// for edges
		for (int j = 0; j < 3; ++j)
		{
			FPoint3 p1 = index_f[j];
			FPoint3 p2 = index_f[(j+1)%3];
			for (int k = 0; k < 3; ++k)
			{
				if ((int)p1[k] == (int)p2[k])	continue;
				if (p1[k] > p2[k])	swap(p1, p2);

				for (int m = ceil(p1[k]); m < p2[k]; m++)
				{
					double t = (m - p1[k]) / (p2[k]- p1[k]);
					double u = t*((p2[(k + 1) % 3] - p1[(k + 1) % 3])) + p1[(k + 1) % 3];
					double v = t*((p2[(k + 2) % 3] - p1[(k + 2) % 3])) + p1[(k + 2) % 3];
					Point3 temp1, temp2;
					temp1[k] = m - 1;
					temp2[k] = m;
					temp1[(k + 1) % 3] = int(u);
					temp2[(k + 1) % 3] = int(u);
					temp1[(k + 2) % 3] = int(v);
					temp2[(k + 2) % 3] = int(v);

					if (infill_type == 0 || (infill_type == 1 && nor.z <= angle))
					{
						if (voxel_cell[temp1.x*(Ny - 1)*(Nz - 1) + temp1.y*(Nz - 1) + temp1.z] != FLAT_BOUND)
							voxel_cell[temp1.x*(Ny - 1)*(Nz - 1) + temp1.y*(Nz - 1) + temp1.z] = BOUNDARY;	// non-flat boundary
						if (voxel_cell[temp2.x*(Ny - 1)*(Nz - 1) + temp2.y*(Nz - 1) + temp2.z] != FLAT_BOUND)
							voxel_cell[temp2.x*(Ny - 1)*(Nz - 1) + temp2.y*(Nz - 1) + temp2.z] = BOUNDARY;	// non-flat boundary
					}
					else
					{
						voxel_cell[temp1.x*(Ny - 1)*(Nz - 1) + temp1.y*(Nz - 1) + temp1.z] = FLAT_BOUND;	// flat boundary
						voxel_cell[temp2.x*(Ny - 1)*(Nz - 1) + temp2.y*(Nz - 1) + temp2.z] = FLAT_BOUND;	// flat boundary
					}
				}
			}
		}

		// for face
		int diff_cnt = 0;
		FPoint3 min_index = index_f[0];
		FPoint3 max_index = index_f[0];
		for (int j = 1; j < 3; ++j)
		for (int k = 0; k < 3; ++k)
		{
			if (max_index[k] < index_f[j][k])	max_index[k] = index_f[j][k];
			if (min_index[k] > index_f[j][k])	min_index[k] = index_f[j][k];
		}
		for (int j = 0; j < 3; ++j)
		{
			int diff = floor(max_index[j]) - floor(min_index[j]);
			if (diff != 0)	diff_cnt++;
		}
		if (diff_cnt < 2)	continue; // only edges and points

		int normal_index = 0;
		double max_normal = 0;
		FPoint3 face_nor = (FPoint3(pts[idx[1]]) - FPoint3(pts[idx[0]])).cross(FPoint3(pts[idx[2]]) - FPoint3(pts[idx[0]])).normalized();
		max_normal = abs(face_nor[0]);
		for (int j = 1; j < 3; ++j)
		{
			if (max_normal < abs(face_nor[j]))
			{
				max_normal = abs(face_nor[j]);
				normal_index = j;
			}
		}

		int shift = 2 - normal_index;
		for (int j = 0; j < 3; ++j)
		{
			FPoint3 temp;
			temp[(0 + shift) % 3] = index_f[j][0];
			temp[(1 + shift) % 3] = index_f[j][1];
			temp[(2 + shift) % 3] = index_f[j][2];
			index_f[j] = temp;
		}

		if (index_f[0].x > index_f[1].x) swap(index_f[0], index_f[1]);
		if (index_f[1].x > index_f[2].x) swap(index_f[1], index_f[2]);
		if (index_f[0].x > index_f[1].x) swap(index_f[0], index_f[1]);
		for (int x = ceil(index_f[0].x); x < index_f[1].x; x++)
		{
			double y0 = index_f[0].y + (index_f[1].y - index_f[0].y) * (x - index_f[0].x) / (index_f[1].x - index_f[0].x);
			double y1 = index_f[0].y + (index_f[2].y - index_f[0].y) * (x - index_f[0].x) / (index_f[2].x - index_f[0].x);
			double z0 = index_f[0].z + (index_f[1].z - index_f[0].z) * (x - index_f[0].x) / (index_f[1].x - index_f[0].x);
			double z1 = index_f[0].z + (index_f[2].z - index_f[0].z) * (x - index_f[0].x) / (index_f[2].x - index_f[0].x);

			if (y0 > y1) { swap(y0, y1); swap(z0, z1); }
			for (int y = ceil(y0); y < y1; y++)
			{
				int z = z0 + (z1 - z0) * (y - y0) / (y1 - y0);
				Point3 result[4];
				result[0] = Point3(x - 1, y - 1, z);
				result[1] = Point3(x, y - 1, z);
				result[2] = Point3(x - 1, y, z);
				result[3] = Point3(x, y, z);

				for (int j = 0; j < 4; ++j)
				{
					Point3 temp;
					temp[0] = result[j][(0 + shift) % 3];
					temp[1] = result[j][(1 + shift) % 3];
					temp[2] = result[j][(2 + shift) % 3];

					if (infill_type == M1 || (infill_type == M2 && nor.z <= angle))
					{
						if (voxel_cell[temp.x*(Ny - 1)*(Nz - 1) + temp.y*(Nz - 1) + temp.z] != FLAT_BOUND)
							voxel_cell[temp.x*(Ny - 1)*(Nz - 1) + temp.y*(Nz - 1) + temp.z] = BOUNDARY;	// non-flat boundary
					}
					else
						voxel_cell[temp.x*(Ny - 1)*(Nz - 1) + temp.y*(Nz - 1) + temp.z] = FLAT_BOUND;	// flat boundary
				}
			}
		}
		for (int x = ceil(index_f[1].x); x < index_f[2].x; x++)
		{
			double y0 = index_f[1].y + (index_f[2].y - index_f[1].y) * (x - index_f[1].x) / (index_f[2].x - index_f[1].x);
			double y1 = index_f[0].y + (index_f[2].y - index_f[0].y) * (x - index_f[0].x) / (index_f[2].x - index_f[0].x);
			double z0 = index_f[1].z + (index_f[2].z - index_f[1].z) * (x - index_f[1].x) / (index_f[2].x - index_f[1].x);
			double z1 = index_f[0].z + (index_f[2].z - index_f[0].z) * (x - index_f[0].x) / (index_f[2].x - index_f[0].x);

			if (y0 > y1) { swap(y0, y1); swap(z0, z1); }
			for (int y = ceil(y0); y < y1; y++)
			{
				int z = z0 + (z1 - z0) * (y - y0) / (y1 - y0);
				Point3 result[4];
				result[0] = Point3(x - 1, y - 1, z);
				result[1] = Point3(x, y - 1, z);
				result[2] = Point3(x - 1, y, z);
				result[3] = Point3(x, y, z);

				for (int j = 0; j < 4; ++j)
				{
					Point3 temp;
					temp[0] = result[j][(0 + shift) % 3];
					temp[1] = result[j][(1 + shift) % 3];
					temp[2] = result[j][(2 + shift) % 3];

					if (infill_type == M1 || (infill_type == M2 && nor.z <= angle))
					{
						if (voxel_cell[temp.x*(Ny - 1)*(Nz - 1) + temp.y*(Nz - 1) + temp.z] != FLAT_BOUND)
							voxel_cell[temp.x*(Ny - 1)*(Nz - 1) + temp.y*(Nz - 1) + temp.z] = BOUNDARY;	// non-flat boundary
					}
					else
						voxel_cell[temp.x*(Ny - 1)*(Nz - 1) + temp.y*(Nz - 1) + temp.z] = FLAT_BOUND;	// flat boundary
				}
			}
		}
	}
}

FPoint3 InfillVoxel::getPosition( int xIdx, int yIdx, int zIdx )
{
	FPoint3 pt = origin + FPoint3(xIdx*space, yIdx*space, zIdx*space);
	FPoint3 result;

	result.x = skew_Rinv[0][0] * pt.x + skew_Rinv[0][1] * pt.y + skew_Rinv[0][2] * pt.z;
	result.y = skew_Rinv[1][0] * pt.x + skew_Rinv[1][1] * pt.y + skew_Rinv[1][2] * pt.z;
	result.z = skew_Rinv[2][0] * pt.x + skew_Rinv[2][1] * pt.y + skew_Rinv[2][2] * pt.z;

	pt = result;

	result.x = Rinv[0][0]*pt.x + Rinv[0][1]*pt.y + Rinv[0][2]*pt.z;
	result.y = Rinv[1][0]*pt.x + Rinv[1][1]*pt.y + Rinv[1][2]*pt.z;
	result.z = Rinv[2][0]*pt.x + Rinv[2][1]*pt.y + Rinv[2][2]*pt.z;

 	return result;
}

void InfillVoxel::checkOutside()
{
	outerCells.push_back(1*(Ny-1)*(Nz-1) + 1*(Nz-1) + 1);
	while (outerCells.size() > 0)
	{
		unsigned int idx = outerCells[outerCells.size() - 1];
		outerCells.pop_back();
		int x_idx = idx / ((Ny - 1)*(Nz - 1));
		int y_idx = (idx - x_idx*(Ny - 1)*(Nz - 1)) / (Nz - 1);
		int z_idx = idx % (Nz - 1);
		propagateOutside(x_idx, y_idx, z_idx);
	}

	// outside
	for (int xx = 1; xx < Nx - 2; ++xx)
	for (int yy = 1; yy < Ny - 2; ++yy)
	for (int zz = 1; zz < Nz - 2; ++zz)
	{
		if (voxel_cell[xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] == OUTSIDE)
		{
			wall[XY][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = OUTSIDE;
			wall[XY][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz + 1] = OUTSIDE;
			wall[YZ][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = OUTSIDE;
			wall[YZ][(xx + 1)*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = OUTSIDE;
			wall[ZX][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = OUTSIDE;
			wall[ZX][xx*(Ny - 1)*(Nz - 1) + (yy + 1)*(Nz - 1) + zz] = OUTSIDE;
		}
	}

	// non flat roof
	for (int xx = 1; xx < Nx - 2; ++xx)
	for (int yy = 1; yy < Ny - 2; ++yy)
	for (int zz = 1; zz < Nz - 2; ++zz)
	{
		if (voxel_cell[xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] == BOUNDARY)
		{
			wall[XY][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = BOUNDARY;
			wall[XY][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz + 1] = BOUNDARY;
			wall[YZ][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = BOUNDARY;
			wall[YZ][(xx + 1)*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = BOUNDARY;
			wall[ZX][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = BOUNDARY;
			wall[ZX][xx*(Ny - 1)*(Nz - 1) + (yy + 1)*(Nz - 1) + zz] = BOUNDARY;
		}
	}

	// flat roof
	if (infill_type == M2)
	{
		for (int xx = 1; xx < Nx - 2; ++xx)
		for (int yy = 1; yy < Ny - 2; ++yy)
		for (int zz = 1; zz < Nz - 2; ++zz)
		{
			if (voxel_cell[xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] == infill_type)
			{
				wall[XY][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = infill_type;
				wall[XY][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz + 1] = infill_type;
				wall[YZ][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = infill_type;
				wall[YZ][(xx + 1)*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = infill_type;
				wall[ZX][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] = infill_type;
				wall[ZX][xx*(Ny - 1)*(Nz - 1) + (yy + 1)*(Nz - 1) + zz] = infill_type;
			}
		}
	}
}


void InfillVoxel::propagateOutside(int x, int y, int z)
{
	if (x == 0 || y == 0 || z == 0 || x == Nx - 2 || y == Ny - 2 || z == Nz - 2)
		return;

	if (voxel_cell[x*(Ny - 1)*(Nz - 1) + y*(Nz - 1) + z] < INSIDE)
		return;

	voxel_cell[x*(Ny - 1)*(Nz - 1) + y*(Nz - 1) + z] = OUTSIDE;


	outerCells.push_back((x + 1)*(Ny - 1)*(Nz - 1) + (y + 0)*(Nz - 1) + (z + 0));
	outerCells.push_back((x - 1)*(Ny - 1)*(Nz - 1) + (y + 0)*(Nz - 1) + (z + 0));
	outerCells.push_back((x + 0)*(Ny - 1)*(Nz - 1) + (y + 1)*(Nz - 1) + (z + 0));
	outerCells.push_back((x + 0)*(Ny - 1)*(Nz - 1) + (y - 1)*(Nz - 1) + (z + 0));
	outerCells.push_back((x + 0)*(Ny - 1)*(Nz - 1) + (y + 0)*(Nz - 1) + (z + 1));
	outerCells.push_back((x + 0)*(Ny - 1)*(Nz - 1) + (y + 0)*(Nz - 1) + (z - 1));
}


void InfillVoxel::getLine( double zz )
{
	lines.clear();
	int x = 0, y = 0, z = 0;

	FPoint3 pt1, pt2, pt3, pt12, pt13;

	bool b_find = false;

	for (int i = 0; i < Nx - 1; ++i)
	{
		x = i;
		pt1 = getPosition(x, y, z);
		pt2 = getPosition(x + 1, y, z);
		if (pt1.z <= zz && pt2.z > zz)
		{
			b_find = true;
			break;
		}
	}

	if (!b_find)
	{
		for (int i = 0; i < Ny - 1; ++i)
		{
			y = i;
			pt1 = getPosition(x, y, z);
			pt2 = getPosition(x, y + 1, z);
			if (pt1.z <= zz && pt2.z > zz)
			{
				b_find = true;
				break;
			}
		}
	}

	if (!b_find)
	{
		for (int i = 0; i < Nz - 1; ++i)
		{
			z = i;
			pt1 = getPosition(x, y, z);
			pt2 = getPosition(x, y, z + 1);
			if (pt1.z <= zz && pt2.z > zz)
			{
				b_find = true;
				break;
			}
		}
	}

	int sum1 = x + y + z;
	for (int i = 0; i < Nx - 1; ++i)
	{
		bool start_seg = false;
		int j = 0;
		int k = sum1 - i - j;
		if (k < 0)
			continue;
		if (k >= Nz - 1)
		{
			k = Nz - 2;
			j = sum1 - i - k;
		}
		if (j < 0)
			continue;

		while (j < Ny - 1 && k > -1)
		{
			if (i + j + k == sum1)
			{
				if (wall[YZ][i*(Ny - 1)*(Nz - 1) + j*(Nz - 1) + k] == infill_type)
				{
					pt1 = getPosition(i, j, k);
					if (!start_seg)
					{
						start_seg = true;

						pt2 = getPosition(i, j, k + 1);
						pt3 = getPosition(i, j + 1, k);

						pt12 = getInterPoint(pt1, pt2, zz);
						pt13 = getInterPoint(pt1, pt3, zz);
					}
					else
					{
						pt3 = getPosition(i, j + 1, k);
						pt13 = getInterPoint(pt1, pt3, zz);
					}
				}
				else
				{
					if (start_seg)
					{
						start_seg = false;
						lines.push_back(Point(pt12.x * 1000, pt12.y * 1000));
						lines.push_back(Point(pt13.x * 1000, pt13.y * 1000));
					}
				}

				k--;
			}
			else
			{
				if (wall[YZ][i*(Ny - 1)*(Nz - 1) + j*(Nz - 1) + k] == infill_type)
				{
					pt1 = getPosition(i, j + 1, k + 1);
					if (!start_seg)
					{
						start_seg = true;

						pt2 = getPosition(i, j, k + 1);
						pt3 = getPosition(i, j + 1, k);

						pt12 = getInterPoint(pt2, pt1, zz);
						pt13 = getInterPoint(pt3, pt1, zz);
					}
					else
					{
						pt3 = getPosition(i, j + 1, k);
						pt13 = getInterPoint(pt3, pt1, zz);
					}
				}
				else
				{
					if (start_seg)
					{
						start_seg = false;
						lines.push_back(Point(pt12.x * 1000, pt12.y * 1000));
						lines.push_back(Point(pt13.x * 1000, pt13.y * 1000));
					}
				}
				j++;
			}
		}

		if (start_seg)
		{
			start_seg = false;
			lines.push_back(Point(pt12.x * 1000, pt12.y * 1000));
			lines.push_back(Point(pt13.x * 1000, pt13.y * 1000));
		}
	}

	for (int j = 0; j < Ny - 1; ++j)
	{
		bool start_seg = false;
		int k = 0;
		int i = sum1 - j - k;
		if (i < 0)
			continue;
		if (i >= Nx - 1)
		{
			i = Nx - 2;
			k = sum1 - j - i;
		}
		if (k < 0)
			continue;

		while (k < Nz - 1 && i > -1)
		{
			if (i + j + k == sum1)
			{
				if (wall[ZX][i*(Ny - 1)*(Nz - 1) + j*(Nz - 1) + k] == infill_type)
				{
					pt1 = getPosition(i, j, k);
					if (!start_seg)
					{
						start_seg = true;

						pt2 = getPosition(i + 1, j, k);
						pt3 = getPosition(i, j, k + 1);

						pt12 = getInterPoint(pt1, pt2, zz);
						pt13 = getInterPoint(pt1, pt3, zz);
					}
					else
					{
						pt3 = getPosition(i, j, k + 1);
						pt13 = getInterPoint(pt1, pt3, zz);
					}
				}
				else
				{
					if (start_seg)
					{
						start_seg = false;
						lines.push_back(Point(pt12.x * 1000, pt12.y * 1000));
						lines.push_back(Point(pt13.x * 1000, pt13.y * 1000));
					}
				}

				i--;
			}
			else
			{
				if (wall[ZX][i*(Ny - 1)*(Nz - 1) + j*(Nz - 1) + k] == infill_type)
				{
					pt1 = getPosition(i + 1, j, k + 1);
					if (!start_seg)
					{
						start_seg = true;

						pt2 = getPosition(i + 1, j, k);
						pt3 = getPosition(i, j, k + 1);

						pt12 = getInterPoint(pt2, pt1, zz);
						pt13 = getInterPoint(pt3, pt1, zz);
					}
					else
					{
						pt3 = getPosition(i, j, k + 1);
						pt13 = getInterPoint(pt3, pt1, zz);
					}
				}
				else
				{
					if (start_seg)
					{
						start_seg = false;
						lines.push_back(Point(pt12.x * 1000, pt12.y * 1000));
						lines.push_back(Point(pt13.x * 1000, pt13.y * 1000));
					}
				}
				k++;
			}
		}

		if (start_seg)
		{
			start_seg = false;
			lines.push_back(Point(pt12.x * 1000, pt12.y * 1000));
			lines.push_back(Point(pt13.x * 1000, pt13.y * 1000));
		}
	}

	for (int k = 0; k < Nz - 1; ++k)
	{
		bool start_seg = false;
		int i = 0;
		int j = sum1 - k - i;
		if (j < 0)
			continue;
		if (j >= Ny - 1)
		{
			j = Ny - 2;
			i = sum1 - k - j;
		}
		if (i < 0)
			continue;

		while (i < Nx - 1 && j > -1)
		{
			if (i + j + k == sum1)
			{
				if (wall[XY][i*(Ny - 1)*(Nz - 1) + j*(Nz - 1) + k] == infill_type)
				{
					pt1 = getPosition(i, j, k);
					if (!start_seg)
					{
						start_seg = true;

						pt2 = getPosition(i, j + 1, k);
						pt3 = getPosition(i + 1, j, k);

						pt12 = getInterPoint(pt1, pt2, zz);
						pt13 = getInterPoint(pt1, pt3, zz);
					}
					else
					{
						pt3 = getPosition(i + 1, j, k);
						pt13 = getInterPoint(pt1, pt3, zz);
					}
				}
				else
				{
					if (start_seg)
					{
						start_seg = false;
						lines.push_back(Point(pt12.x * 1000, pt12.y * 1000));
						lines.push_back(Point(pt13.x * 1000, pt13.y * 1000));
					}
				}

				j--;
			}
			else
			{
				if (wall[XY][i*(Ny - 1)*(Nz - 1) + j*(Nz - 1) + k] == infill_type)
				{
					pt1 = getPosition(i + 1, j + 1, k);
					if (!start_seg)
					{
						start_seg = true;

						pt2 = getPosition(i, j + 1, k);
						pt3 = getPosition(i + 1, j, k);

						pt12 = getInterPoint(pt2, pt1, zz);
						pt13 = getInterPoint(pt3, pt1, zz);
					}
					else
					{
						pt3 = getPosition(i + 1, j, k);
						pt13 = getInterPoint(pt3, pt1, zz);
					}
				}
				else
				{
					if (start_seg)
					{
						start_seg = false;
						lines.push_back(Point(pt12.x * 1000, pt12.y * 1000));
						lines.push_back(Point(pt13.x * 1000, pt13.y * 1000));
					}
				}
				i++;
			}
		}

		if (start_seg)
		{
			start_seg = false;
			lines.push_back(Point(pt12.x * 1000, pt12.y * 1000));
			lines.push_back(Point(pt13.x * 1000, pt13.y * 1000));
		}
	}
}
FPoint3 InfillVoxel::getInterPoint(FPoint3& a, FPoint3& b, double z)
{
	double alpha = z - a.z;
	double beta = b.z - z;
	double w = alpha/(alpha + beta);

	return a*(1-w) + b*w;
}

void InfillVoxel::findOverhang()
{
	int tic = clock();
	overhang_idx.clear();

	int cnt = 0;
	int cnt2 = 0;
	for(int xx = 1; xx < Nx - 2; ++xx)
	for(int yy = 1; yy < Ny - 2; ++yy)
	for(int zz = 1; zz < Nz - 2; ++zz)
	{
		if (voxel_cell[(xx - 1)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + zz - 0] == OUTSIDE
			|| voxel_cell[(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 1)*(Nz - 1) + zz - 0] == OUTSIDE
			|| voxel_cell[(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + zz - 1] == OUTSIDE)		// outside
			continue;

		if (wall[XY][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] == infill_type && wall[YZ][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] == infill_type && wall[ZX][xx*(Ny - 1)*(Nz - 1) + yy*(Nz - 1) + zz] == infill_type)
		{
			if (wall[XY][(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 1)*(Nz - 1) + (zz - 0)] != infill_type &&
				wall[XY][(xx - 1)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + (zz - 0)] != infill_type &&
				wall[YZ][(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + (zz - 1)] != infill_type &&
				wall[YZ][(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 1)*(Nz - 1) + (zz - 0)] != infill_type &&
				wall[ZX][(xx - 1)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + (zz - 0)] != infill_type &&
				wall[ZX][(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + (zz - 1)] != infill_type)
			{
				overhang_idx.push_back(Point3(xx, yy, zz));
			}
		}
	}

 	//printf("Time of find overhang : %lfsec, ", (clock() - tic)/(double)CLOCKS_PER_SEC);
 	//printf("Number of overhang : %d\n", overhang_idx.size());
}

void InfillVoxel::updateOverhang()
{
	std::vector<Point3> overhang_idx_temp;
	for(int i = 0; i < overhang_idx.size(); ++i)
	{
		int xx = overhang_idx[i].x;
		int yy = overhang_idx[i].y;
		int zz = overhang_idx[i].z;

		if(wall[XY][xx*(Ny-1)*(Nz-1) + yy*(Nz-1) + zz] == infill_type && wall[YZ][xx*(Ny-1)*(Nz-1) + yy*(Nz-1) + zz] == infill_type && wall[ZX][xx*(Ny-1)*(Nz-1) + yy*(Nz-1) + zz] == infill_type)
		{
			if (wall[XY][(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 1)*(Nz - 1) + (zz - 0)] != infill_type &&
				wall[XY][(xx - 1)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + (zz - 0)] != infill_type &&
				wall[YZ][(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + (zz - 1)] != infill_type &&
				wall[YZ][(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 1)*(Nz - 1) + (zz - 0)] != infill_type &&
				wall[ZX][(xx - 1)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + (zz - 0)] != infill_type &&
				wall[ZX][(xx - 0)*(Ny - 1)*(Nz - 1) + (yy - 0)*(Nz - 1) + (zz - 1)] != infill_type)
				overhang_idx_temp.push_back(Point3(xx,yy,zz));
		}
	}

	overhang_idx.clear();
	overhang_idx = overhang_idx_temp;

	//printf("Number of overhang : %d\n", overhang_idx.size());
}

void InfillVoxel::calcPlane( int& plane_idx, int& cnt, Point3 coor )
{
	for(int i = 0; i < (Nx-1)*(Ny-1); ++i)
		search[XY][i] = false;
	for(int i = 0; i < (Ny-1)*(Nz-1); ++i)
		search[YZ][i] = false;
	for(int i = 0; i < (Nz-1)*(Nx-1); ++i)
		search[ZX][i] = false;
	
	area[XY] = area[YZ] = area[ZX] = 0;

	propagatePlaneXY(coor.x-1, coor.y, coor.z);
	propagatePlaneYZ(coor.x, coor.y-1, coor.z);
	propagatePlaneZX(coor.x, coor.y, coor.z-1);

	if (area[XY] < area[YZ])
	{
		if (area[XY] < area[ZX])
		{
			cnt = area[XY];
			plane_idx = PlaneType::XY;
		}
		else
		{
			cnt = area[ZX];
			plane_idx = PlaneType::ZX;
		}
	}
	else
	{
		if (area[YZ] < area[ZX])
		{
			cnt = area[YZ];
			plane_idx = PlaneType::YZ;
		}
		else
		{
			cnt = area[ZX];
			plane_idx = PlaneType::ZX;
		}
	}
}

void InfillVoxel::propagatePlaneXY(int x, int y, int z, bool real)
{
	if (real)
		wall[XY][(x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (z)] = infill_type;
	else
		search[XY][x*(Ny-1) + y] = true;
	area[XY]++;

	if(x < 1 || x > Nx-3)	return;
	if(y < 1 || y > Ny-3)	return;

	if( (!search[XY][(x-1)*(Ny-1) + y] || real)
		&& wall[XY][(x-1)*(Ny-1)*(Nz-1) + (y-0)*(Nz-1) + (z-0)]%2 == 0 // 0 or 2
		&& wall[XY][(x-1)*(Ny-1)*(Nz-1) + (y-0)*(Nz-1) + (z-0)] != infill_type
		&& wall[YZ][(x-0)*(Ny-1)*(Nz-1) + (y-0)*(Nz-1) + (z-0)] != infill_type)
		propagatePlaneXY(x-1, y, z, real);
	if((!search[XY][(x+1)*(Ny-1) + y] || real)
		&& wall[XY][(x + 1)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] % 2 == 0 // 0 or 2
		&& wall[XY][(x + 1)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] != infill_type
		&& wall[YZ][(x + 1)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] != infill_type)
		propagatePlaneXY(x + 1, y, z, real);
	if((!search[XY][(x+0)*(Ny-1) + y-1] || real)
		&& wall[XY][(x+0)*(Ny-1)*(Nz-1) + (y-1)*(Nz-1) + (z-0)]%2 == 0 // 0 or 2
		&& wall[XY][(x+0)*(Ny-1)*(Nz-1) + (y-1)*(Nz-1) + (z-0)] != infill_type
		&& wall[ZX][(x+0)*(Ny-1)*(Nz-1) + (y-0)*(Nz-1) + (z-0)] != infill_type)
		propagatePlaneXY(x, y - 1, z, real);
	if((!search[XY][(x+0)*(Ny-1) + y+1] || real)
		&& wall[XY][(x + 0)*(Ny - 1)*(Nz - 1) + (y + 1)*(Nz - 1) + (z - 0)] % 2 == 0 // 0 or 2
		&& wall[XY][(x + 0)*(Ny - 1)*(Nz - 1) + (y + 1)*(Nz - 1) + (z - 0)] != infill_type
		&& wall[ZX][(x + 0)*(Ny - 1)*(Nz - 1) + (y + 1)*(Nz - 1) + (z - 0)] != infill_type)
		propagatePlaneXY(x, y + 1, z, real);
}

void InfillVoxel::propagatePlaneYZ(int x, int y, int z, bool real)
{
	if (real)
		wall[YZ][(x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (z)] = infill_type;
	else
		search[YZ][y*(Nz-1) + z] = true;
	area[YZ]++;

	if(z < 1 || z > Nz-3)	return;
	if(y < 1 || y > Ny-3)	return;

	if ((!search[YZ][(y - 1)*(Nz - 1) + z] || real)
		&& wall[YZ][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 1)*(Nz - 1) + (z - 0)] % 2 == 0 // 0 or 2
		&& wall[YZ][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 1)*(Nz - 1) + (z - 0)] != infill_type
		&& wall[ZX][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] != infill_type)
		propagatePlaneYZ(x, y - 1, z, real);
	if ((!search[YZ][(y + 1)*(Nz - 1) + z] || real)
		&& wall[YZ][(x - 0)*(Ny - 1)*(Nz - 1) + (y + 1)*(Nz - 1) + (z - 0)] % 2 == 0 // 0 or 2
		&& wall[YZ][(x - 0)*(Ny - 1)*(Nz - 1) + (y + 1)*(Nz - 1) + (z - 0)] != infill_type
		&& wall[ZX][(x - 0)*(Ny - 1)*(Nz - 1) + (y + 1)*(Nz - 1) + (z - 0)] != infill_type)
		propagatePlaneYZ(x, y + 1, z, real);
	if ((!search[YZ][(y - 0)*(Nz - 1) + z - 1] || real)
		&& wall[YZ][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 1)] % 2 == 0 // 0 or 2
		&& wall[YZ][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 1)] != infill_type
		&& wall[XY][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] != infill_type)
		propagatePlaneYZ(x, y, z - 1, real);
	if ((!search[YZ][(y + 0)*(Nz - 1) + z + 1] || real)
		&& wall[YZ][(x - 0)*(Ny - 1)*(Nz - 1) + (y + 0)*(Nz - 1) + (z + 1)] % 2 == 0 // 0 or 2
		&& wall[YZ][(x - 0)*(Ny - 1)*(Nz - 1) + (y + 0)*(Nz - 1) + (z + 1)] != infill_type
		&& wall[XY][(x - 0)*(Ny - 1)*(Nz - 1) + (y + 0)*(Nz - 1) + (z + 1)] != infill_type)
		propagatePlaneYZ(x, y, z + 1, real);
}

void InfillVoxel::propagatePlaneZX(int x, int y, int z, bool real)
{
	if (real)
		wall[ZX][(x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (z)] = infill_type;
	else
		search[ZX][z*(Nx-1) + x] = true;
	area[ZX]++;

	if(z < 1 || z > Nz-3)	return;
	if(x < 1 || x > Nx-3)	return;

	if ((!search[ZX][(z - 1)*(Nx - 1) + x] || real)
		&& wall[ZX][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 1)] % 2 == 0 // 0 or 2
		&& wall[ZX][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 1)] != infill_type
		&& wall[XY][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] != infill_type)
		propagatePlaneZX(x, y, z - 1, real);
	if ((!search[ZX][(z + 1)*(Nx - 1) + x] || real)
		&& wall[ZX][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z + 1)] % 2 == 0 // 0 or 2
		&& wall[ZX][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z + 1)] != infill_type
		&& wall[XY][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z + 1)] != infill_type)
		propagatePlaneZX(x, y, z + 1, real);
	if ((!search[ZX][(z - 0)*(Nx - 1) + x - 1] || real)
		&& wall[ZX][(x - 1)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] % 2 == 0 // 0 or 2
		&& wall[ZX][(x - 1)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] != infill_type
		&& wall[YZ][(x - 0)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] != infill_type)
		propagatePlaneZX(x - 1, y, z, real);
	if ((!search[ZX][(z + 0)*(Nx - 1) + x + 1] || real)
		&& wall[ZX][(x + 1)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z + 0)] % 2 == 0 // 0 or 2
		&& wall[ZX][(x + 1)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z + 0)] != infill_type
		&& wall[YZ][(x + 1)*(Ny - 1)*(Nz - 1) + (y - 0)*(Nz - 1) + (z - 0)] != infill_type)
		propagatePlaneZX(x + 1, y, z, real);
}

bool InfillVoxel::step()
{
	updateOverhang();
	if(overhang_idx.size() == 0)
		return false;

	int max_area = 0;
	int max_plane;
	int max_coor;

	for(int i = 0; i < overhang_idx.size(); ++i)
	{
		int p_idx;
		int area;
		calcPlane(p_idx, area, overhang_idx[i]);
		if (max_area < area)
		{
			max_area = area;
			max_plane = p_idx;
			max_coor = i;
		}
	}

	area[XY] = area[YZ] = area[ZX] = 0;

	double area1 = calcTotalAreaForOverhang();

	if (max_plane == PlaneType::XY)
		propagatePlaneXY(overhang_idx[max_coor].x - 1, overhang_idx[max_coor].y, overhang_idx[max_coor].z, true);
	else if (max_plane == PlaneType::YZ)
		propagatePlaneYZ(overhang_idx[max_coor].x, overhang_idx[max_coor].y - 1, overhang_idx[max_coor].z, true);
	else if (max_plane == PlaneType::ZX)
		propagatePlaneZX(overhang_idx[max_coor].x, overhang_idx[max_coor].y, overhang_idx[max_coor].z - 1, true);	

	double area2 = calcTotalAreaForOverhang() + max_area;

	printf("before : %lf, after : %lf\n", area1, area2);

	return true;
}

bool InfillVoxel::step2()
{
	double cur_area = calcTotalAreaForOverhang();

	if (overhang_idx.size() == 0)
		return false;

	std::vector<Point3> temp_overhang_idx = overhang_idx;
	std::vector<int> temp_plane_idx = plane_idx;
	std::vector<int> temp_plane_area = plane_area;
	std::vector<int> temp_plane_cnt = plane_cnt;

	int optimal_plane_idx = 0;
	double optimal_plane_area = INT_MAX;
	for (int i = 0; i < temp_plane_idx.size(); ++i)
	{
		if (temp_plane_idx[i] != i) continue;

		if (i % 3 == PlaneType::XY)
		{
			for (int x = 0; x < Nx - 1; x++)
				for (int y = 0; y < Ny - 1; y++)
					temp_wall[XY][x*(Ny-1) + y] = wall[XY][(x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (temp_overhang_idx[i / 3].z)];
			propagatePlaneXY(temp_overhang_idx[i / 3].x - 1, temp_overhang_idx[i / 3].y, temp_overhang_idx[i / 3].z, true);
		}
		else if (i % 3 == PlaneType::YZ)
		{
			for (int y = 0; y < Ny - 1; y++)
				for (int z = 0; z < Nz - 1; z++)
					temp_wall[YZ][y*(Nz - 1) + z] = wall[YZ][(temp_overhang_idx[i / 3].x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (z)];
			propagatePlaneYZ(temp_overhang_idx[i / 3].x, temp_overhang_idx[i / 3].y - 1, temp_overhang_idx[i / 3].z, true);
		}
		else if (i % 3 == PlaneType::ZX)
		{
			for (int z = 0; z < Nz - 1; z++)
				for (int x = 0; x < Nx - 1; x++)
					temp_wall[ZX][z*(Nx - 1) + x] = wall[ZX][(x)*(Ny - 1)*(Nz - 1) + (temp_overhang_idx[i / 3].y)*(Nz - 1) + (z)];
			propagatePlaneZX(temp_overhang_idx[i / 3].x, temp_overhang_idx[i / 3].y, temp_overhang_idx[i / 3].z - 1, true);
		}

		overhang_idx = temp_overhang_idx;
		double next_area = calcTotalAreaForOverhang();
		if (next_area - cur_area + temp_plane_area[i] < optimal_plane_area)
		{
			optimal_plane_area = next_area - cur_area + temp_plane_area[i];
			optimal_plane_idx = i;
		}
		else if (next_area - cur_area + temp_plane_area[i] == optimal_plane_area)
		{
			if (temp_plane_area[optimal_plane_idx] > temp_plane_area[i])
			{
				optimal_plane_area = next_area - cur_area + temp_plane_area[i];
				optimal_plane_idx = i;
			}
		}

		if (i % 3 == PlaneType::XY)
		{
			for (int x = 0; x < Nx - 1; x++)
				for (int y = 0; y < Ny - 1; y++)
					wall[XY][(x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (temp_overhang_idx[i / 3].z)] = temp_wall[XY][x*(Ny - 1) + y];
		}
		else if (i % 3 == PlaneType::YZ)
		{
			for (int y = 0; y < Ny - 1; y++)
				for (int z = 0; z < Nz - 1; z++)
					wall[YZ][(temp_overhang_idx[i / 3].x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (z)] = temp_wall[YZ][y*(Nz - 1) + z];
		}
		else if (i % 3 == PlaneType::ZX)
		{
			for (int z = 0; z < Nz - 1; z++)
				for (int x = 0; x < Nx - 1; x++)
					wall[ZX][(x)*(Ny - 1)*(Nz - 1) + (temp_overhang_idx[i / 3].y)*(Nz - 1) + (z)] = temp_wall[ZX][z*(Nx - 1) + x];
		}
	}

// 	if (temp_plane_area[optimal_plane_idx] == 3)
// 		return false;

	if (optimal_plane_idx % 3 == PlaneType::XY)
	{
		propagatePlaneXY(temp_overhang_idx[optimal_plane_idx / 3].x - 1, temp_overhang_idx[optimal_plane_idx / 3].y, temp_overhang_idx[optimal_plane_idx / 3].z, true);
	}
	else if (optimal_plane_idx % 3 == PlaneType::YZ)
	{
		propagatePlaneYZ(temp_overhang_idx[optimal_plane_idx / 3].x, temp_overhang_idx[optimal_plane_idx / 3].y - 1, temp_overhang_idx[optimal_plane_idx / 3].z, true);
	}
	else if (optimal_plane_idx % 3 == PlaneType::ZX)
	{
		propagatePlaneZX(temp_overhang_idx[optimal_plane_idx / 3].x, temp_overhang_idx[optimal_plane_idx / 3].y, temp_overhang_idx[optimal_plane_idx / 3].z - 1, true);
	}

	overhang_idx = temp_overhang_idx;
	double done_area = calcTotalAreaForOverhang() + temp_plane_area[optimal_plane_idx];

	updateOverhang();
	total_area += temp_plane_area[optimal_plane_idx];
	printf("before : %lf, after : %lf, num of overhang : %i\n", cur_area + total_area, done_area + total_area, (int)overhang_idx.size());

	overhang_idx = temp_overhang_idx;
	return true;
}

double InfillVoxel::calcTotalAreaForOverhang()
{
	updateOverhang();
	if (overhang_idx.size() == 0)
		return 0;

	plane_idx.clear();
	plane_idx.resize(overhang_idx.size() * 3);
	std::iota(plane_idx.begin(), plane_idx.end(), 0);

	plane_area.clear();
	plane_area.resize(overhang_idx.size() * 3, 0);

	plane_cnt.clear();
	plane_cnt.resize(overhang_idx.size() * 3, 0);

	// Removing duplicated plane
	for (int i = 0; i < plane_idx.size(); ++i)
	{
		if (plane_idx[i] != i) continue;

		int d_idx = i % 3;
		int over_idx = i / 3;

		if (d_idx == PlaneType::XY)
		{
			area[d_idx] = 0;
			for (int j = 0; j < (Nx - 1)*(Ny - 1); ++j)
				search[XY][j] = false;
			propagatePlaneXY(overhang_idx[over_idx].x - 1, overhang_idx[over_idx].y, overhang_idx[over_idx].z);
			for (int j = i + 3; j < plane_idx.size(); j += 3)
			{
				if (overhang_idx[j / 3].z == overhang_idx[over_idx].z)
				{
					int x = overhang_idx[j / 3].x;
					int y = overhang_idx[j / 3].y;
					if (search[XY][(x - 1)*(Ny - 1) + y] || search[XY][(x)*(Ny - 1) + y - 1])
						plane_idx[j] = i;
				}
			}
		}
		else if (d_idx == PlaneType::YZ)
		{
			area[d_idx] = 0;
			for (int j = 0; j < (Ny - 1)*(Nz - 1); ++j)
				search[YZ][j] = false;
			propagatePlaneYZ(overhang_idx[over_idx].x, overhang_idx[over_idx].y - 1, overhang_idx[over_idx].z);
			for (int j = i + 3; j < plane_idx.size(); j += 3)
			{
				if (overhang_idx[j / 3].x == overhang_idx[over_idx].x)
				{
					int y = overhang_idx[j / 3].y;
					int z = overhang_idx[j / 3].z;
					if (search[YZ][(y - 1)*(Nz - 1) + z] || search[YZ][(y)*(Nz - 1) + z - 1])
						plane_idx[j] = i;
				}
			}
		}
		else if (d_idx == PlaneType::ZX)
		{
			area[d_idx] = 0;
			for (int j = 0; j < (Nz - 1)*(Nx - 1); ++j)
				search[ZX][j] = false;
			propagatePlaneZX(overhang_idx[over_idx].x, overhang_idx[over_idx].y, overhang_idx[over_idx].z - 1);
			for (int j = i + 3; j < plane_idx.size(); j += 3)
			{
				if (overhang_idx[j / 3].y == overhang_idx[over_idx].y)
				{
					int z = overhang_idx[j / 3].z;
					int x = overhang_idx[j / 3].x;
					if (search[ZX][(z - 1)*(Nx - 1) + x] || search[ZX][(z)*(Nx - 1) + x - 1])
						plane_idx[j] = i;
				}
			}
		}
		plane_area[i] = area[d_idx];
	}
	for (int i = 0; i < plane_cnt.size(); ++i)
		plane_cnt[plane_idx[i]]++;
	// End Removing duplicated plane

	mean_area.clear();
	mean_area.resize(plane_idx.size(), 0);
	for (int i = 0; i < mean_area.size(); ++i)
		if (plane_cnt[i] != 0)
			mean_area[i] = plane_area[i] / (double)plane_cnt[i];


	double result = 0;
	for (int i = 0; i < plane_idx.size(); i += 3)
	{
		result += min(min(mean_area[plane_idx[i]], mean_area[plane_idx[i + 1]]), mean_area[plane_idx[i + 2]]);
	}

	return result;
}

void InfillVoxel::run(double s, int m/* = 0*/)
{
	if (s <= 0)
		return;

	int tic = clock();

	s = 3 * s / 1.2;

	rotateMesh();
	makeVoxel(s);
	int tic2 = clock();
	updateVoxel();
	printf("(InfillVoxel Class) Time of boundary unit block detection : %lfsec\n", (clock() - tic2) / (double)CLOCKS_PER_SEC);
	int tic3 = clock();
	checkOutside();
	printf("(InfillVoxel Class) Time of outside propagation : %lfsec\n", (clock() - tic3) / (double)CLOCKS_PER_SEC);
	findOverhang();

	if (m  == 0)
		while (step());
	else
	{
		total_area = 0;
		while (step3());
		//stepAll();
	}
		
	if(voxel_cell != nullptr)
	{
		delete[] voxel_cell;
		voxel_cell = nullptr;
	}

	printf("Total Area : %d\n", total_area);
	printf("(InfillVoxel Class) Total time of infill generation: %lfsec\n", (clock() - tic)/(double)CLOCKS_PER_SEC);
}

void InfillVoxel::addLineSegment(Point& p1, Point& p2, engine::Polygons& poly, engine::Polygons& result)
{
// 	engine::PolygonRef p = result.newPoly();
// 	p.add(p1);
// 	p.add(p2);
// 	return;

	if (poly.size() == 0)
		return;

	bool b_inside;

	std::vector<double> intersect_list;
	intersect_list.push_back(0);
	intersect_list.push_back(1);

	double a, b, c, d, e, f, alpha, beta;
	int idx1, idx2;

	for (unsigned int i = 0; i < poly.size(); i++)
	{
		for (unsigned int j = 0; j < poly[i].size(); j++)
		{
			idx1 = j;
			idx2 = (j + 1) % poly[i].size();

			a = p2.X - p1.X;
			b = poly[i][idx1].X - poly[i][idx2].X;
			c = p2.Y - p1.Y;
			d = poly[i][idx1].Y - poly[i][idx2].Y;
			e = poly[i][idx1].X - p1.X;
			f = poly[i][idx1].Y - p1.Y;

			alpha = (e*d - b*f) / (a*d - b*c);
			beta = (a*f - c*e) / (a*d - b*c);

			if (alpha > 0 && alpha < 1 && beta > 0 && beta <= 1)
				intersect_list.push_back(alpha);
		}
	}

	Point pp1, pp2, pp3;
	std::sort(intersect_list.begin(), intersect_list.end());

	for (int i = 1; i < intersect_list.size(); i += 1)
	{
		pp1.X = (1 - intersect_list[i - 1])*p1.X + intersect_list[i - 1] * p2.X;
		pp1.Y = (1 - intersect_list[i - 1])*p1.Y + intersect_list[i - 1] * p2.Y;
		pp2.X = (1 - intersect_list[i])*p1.X + intersect_list[i] * p2.X;
		pp2.Y = (1 - intersect_list[i])*p1.Y + intersect_list[i] * p2.Y;
		pp3.X = pp1.X - pp2.X;
		pp3.Y = pp1.Y - pp2.Y;

// 		if (sqrt(pp3.X*pp3.X + pp3.Y*pp3.Y) < 1000)
// 		{
// 			//printf("e");
// 			continue;
// 		}

		pp3.X = 0.5*(pp1.X + pp2.X);
		pp3.Y = 0.5*(pp1.Y + pp2.Y);
		b_inside = poly.inside(pp3);

		if (b_inside)
		{
			engine::PolygonRef p = result.newPoly();
			p.add(pp1);
			p.add(pp2);
		}

	}
}

void InfillVoxel::generateInfill(const engine::Polygons& in_outline, engine::Polygons& result, int extrusionWidth, int infillOverlap, double z)
{
	engine::Polygons outline = in_outline.offset(extrusionWidth * infillOverlap / 100);
	AABB2D boundary(outline);

	getLine(z);

	for (int i = 0; i < lines.size(); i += 2)
		addLineSegment(lines[i], lines[i + 1], outline, result);
}

double InfillVoxel::calcTotalAreaForOverhang2()
{
	updateOverhang();
	if (overhang_idx.size() == 0)
		return 0;

	plane_idx.clear();
	plane_idx.resize(overhang_idx.size() * 3);
	std::iota(plane_idx.begin(), plane_idx.end(), 0);

	plane_area.clear();
	plane_area.resize(overhang_idx.size() * 3, 0);

	plane_cnt.clear();
	plane_cnt.resize(overhang_idx.size() * 3, 0);

	std::vector<std::vector<int>> plane_overhang_idx;
	plane_overhang_idx.resize(overhang_idx.size() * 3);

	int num_planes = 0;
	// Removing duplicated plane
	for (int i = 0; i < plane_idx.size(); ++i)
	{
		if (plane_idx[i] != i) continue;

		int d_idx = i % 3;
		int over_idx = i / 3;
		num_planes++;

		plane_overhang_idx[i].push_back(i/3);

		if (d_idx == PlaneType::XY)
		{
			area[d_idx] = 0;
			for (int j = 0; j < (Nx - 1)*(Ny - 1); ++j)
				search[XY][j] = false;
			propagatePlaneXY(overhang_idx[over_idx].x - 1, overhang_idx[over_idx].y, overhang_idx[over_idx].z);
			for (int j = i + 3; j < plane_idx.size(); j += 3)
			{
				if (overhang_idx[j / 3].z == overhang_idx[over_idx].z)
				{
					int x = overhang_idx[j / 3].x;
					int y = overhang_idx[j / 3].y;
					if (search[XY][(x - 1)*(Ny - 1) + y] || search[XY][(x)*(Ny - 1) + y - 1])
					{
						plane_idx[j] = i;
						plane_overhang_idx[i].push_back(j / 3);
					}
				}
			}
		}
		else if (d_idx == PlaneType::YZ)
		{
			area[d_idx] = 0;
			for (int j = 0; j < (Ny - 1)*(Nz - 1); ++j)
				search[YZ][j] = false;
			propagatePlaneYZ(overhang_idx[over_idx].x, overhang_idx[over_idx].y - 1, overhang_idx[over_idx].z);
			for (int j = i + 3; j < plane_idx.size(); j += 3)
			{
				if (overhang_idx[j / 3].x == overhang_idx[over_idx].x)
				{
					int y = overhang_idx[j / 3].y;
					int z = overhang_idx[j / 3].z;
					if (search[YZ][(y - 1)*(Nz - 1) + z] || search[YZ][(y)*(Nz - 1) + z - 1])
					{
						plane_idx[j] = i;
						plane_overhang_idx[i].push_back(j / 3);
					}
				}
			}
		}
		else if (d_idx == PlaneType::ZX)
		{
			area[d_idx] = 0;
			for (int j = 0; j < (Nz - 1)*(Nx - 1); ++j)
				search[ZX][j] = false;
			propagatePlaneZX(overhang_idx[over_idx].x, overhang_idx[over_idx].y, overhang_idx[over_idx].z - 1);
			for (int j = i + 3; j < plane_idx.size(); j += 3)
			{
				if (overhang_idx[j / 3].y == overhang_idx[over_idx].y)
				{
					int z = overhang_idx[j / 3].z;
					int x = overhang_idx[j / 3].x;
					if (search[ZX][(z - 1)*(Nx - 1) + x] || search[ZX][(z)*(Nx - 1) + x - 1])
					{
						plane_idx[j] = i;
						plane_overhang_idx[i].push_back(j / 3);
					}
				}
			}
		}
		plane_area[i] = area[d_idx];
	}
	for (int i = 0; i < plane_cnt.size(); ++i)
		plane_cnt[plane_idx[i]]++;
	// End Removing duplicated plane

	int cnt = 0;
	std::vector<bool> b_visit;
	b_visit.resize(overhang_idx.size(), false);
	std::vector<int> min_plane_idx;
	min_plane_idx.resize(overhang_idx.size());

	for (int i = 0; i < overhang_idx.size(); ++i)
	{
		int idx1 = plane_idx[3 * i + 0];
		int idx2 = plane_idx[3 * i + 1];
		int idx3 = plane_idx[3 * i + 2];

		if (plane_area[idx1] < plane_area[idx2])
		{
			if (plane_area[idx1] < plane_area[idx3])
				min_plane_idx[i] = idx1;
			else
				min_plane_idx[i] = idx3;
		}
		else
		{
			if (plane_area[idx2] < plane_area[idx3])
				min_plane_idx[i] = idx2;
			else
				min_plane_idx[i] = idx3;
		}
	}

	double result = 0;

	while (cnt != overhang_idx.size())
	{
		int optimal_plane_idx = -1;
		int optimal_plane_area = 0;
		for (int i = 0; i < min_plane_idx.size(); ++i)
		{
			if (b_visit[i]) continue;

			if (plane_area[min_plane_idx[i]] > optimal_plane_area)
			{
				optimal_plane_idx = min_plane_idx[i];
				optimal_plane_area = plane_area[min_plane_idx[i]];
			}
		}

		result += optimal_plane_area;
		for (int i = 0; i < plane_overhang_idx[optimal_plane_idx].size(); ++i)
		{
			int idx = plane_overhang_idx[optimal_plane_idx][i];
			if (!b_visit[idx])
			{
				b_visit[idx] = true;
				cnt++;
			}
		}
	}

	return result;
}

bool InfillVoxel::step3()
{
	double cur_area = calcTotalAreaForOverhang2();

	if (overhang_idx.size() == 0)
		return false;

	std::vector<Point3> temp_overhang_idx = overhang_idx;
	std::vector<int> temp_plane_idx = plane_idx;
	std::vector<int> temp_plane_area = plane_area;
	std::vector<int> temp_plane_cnt = plane_cnt;

	int optimal_plane_idx = 0;
	double optimal_plane_area = INT_MAX;
	for (int i = 0; i < temp_plane_idx.size(); ++i)
	{
		if (temp_plane_idx[i] != i) continue;

		if (i % 3 == PlaneType::XY)
		{
			for (int x = 0; x < Nx - 1; x++)
				for (int y = 0; y < Ny - 1; y++)
					temp_wall[XY][x*(Ny - 1) + y] = wall[XY][(x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (temp_overhang_idx[i / 3].z)];
			propagatePlaneXY(temp_overhang_idx[i / 3].x - 1, temp_overhang_idx[i / 3].y, temp_overhang_idx[i / 3].z, true);
		}
		else if (i % 3 == PlaneType::YZ)
		{
			for (int y = 0; y < Ny - 1; y++)
				for (int z = 0; z < Nz - 1; z++)
					temp_wall[YZ][y*(Nz - 1) + z] = wall[YZ][(temp_overhang_idx[i / 3].x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (z)];
			propagatePlaneYZ(temp_overhang_idx[i / 3].x, temp_overhang_idx[i / 3].y - 1, temp_overhang_idx[i / 3].z, true);
		}
		else if (i % 3 == PlaneType::ZX)
		{
			for (int z = 0; z < Nz - 1; z++)
				for (int x = 0; x < Nx - 1; x++)
					temp_wall[ZX][z*(Nx - 1) + x] = wall[ZX][(x)*(Ny - 1)*(Nz - 1) + (temp_overhang_idx[i / 3].y)*(Nz - 1) + (z)];
			propagatePlaneZX(temp_overhang_idx[i / 3].x, temp_overhang_idx[i / 3].y, temp_overhang_idx[i / 3].z - 1, true);
		}

		overhang_idx = temp_overhang_idx;
		double next_area = calcTotalAreaForOverhang2();
		if (next_area - cur_area + temp_plane_area[i] < optimal_plane_area)
		{
			optimal_plane_area = next_area - cur_area + temp_plane_area[i];
			optimal_plane_idx = i;
		}
		else if (next_area - cur_area + temp_plane_area[i] == optimal_plane_area)
		{
			if (temp_plane_area[optimal_plane_idx] > temp_plane_area[i])
			{
				optimal_plane_area = next_area - cur_area + temp_plane_area[i];
				optimal_plane_idx = i;
			}
		}

		if (i % 3 == PlaneType::XY)
		{
			for (int x = 0; x < Nx - 1; x++)
				for (int y = 0; y < Ny - 1; y++)
					wall[XY][(x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (temp_overhang_idx[i / 3].z)] = temp_wall[XY][x*(Ny - 1) + y];
		}
		else if (i % 3 == PlaneType::YZ)
		{
			for (int y = 0; y < Ny - 1; y++)
				for (int z = 0; z < Nz - 1; z++)
					wall[YZ][(temp_overhang_idx[i / 3].x)*(Ny - 1)*(Nz - 1) + (y)*(Nz - 1) + (z)] = temp_wall[YZ][y*(Nz - 1) + z];
		}
		else if (i % 3 == PlaneType::ZX)
		{
			for (int z = 0; z < Nz - 1; z++)
				for (int x = 0; x < Nx - 1; x++)
					wall[ZX][(x)*(Ny - 1)*(Nz - 1) + (temp_overhang_idx[i / 3].y)*(Nz - 1) + (z)] = temp_wall[ZX][z*(Nx - 1) + x];
		}
	}

	// 	if (temp_plane_area[optimal_plane_idx] == 3)
	// 		return false;

	if (optimal_plane_idx % 3 == PlaneType::XY)
	{
		propagatePlaneXY(temp_overhang_idx[optimal_plane_idx / 3].x - 1, temp_overhang_idx[optimal_plane_idx / 3].y, temp_overhang_idx[optimal_plane_idx / 3].z, true);
	}
	else if (optimal_plane_idx % 3 == PlaneType::YZ)
	{
		propagatePlaneYZ(temp_overhang_idx[optimal_plane_idx / 3].x, temp_overhang_idx[optimal_plane_idx / 3].y - 1, temp_overhang_idx[optimal_plane_idx / 3].z, true);
	}
	else if (optimal_plane_idx % 3 == PlaneType::ZX)
	{
		propagatePlaneZX(temp_overhang_idx[optimal_plane_idx / 3].x, temp_overhang_idx[optimal_plane_idx / 3].y, temp_overhang_idx[optimal_plane_idx / 3].z - 1, true);
	}

	overhang_idx = temp_overhang_idx;
	double done_area = calcTotalAreaForOverhang2();

	updateOverhang();
	total_area += temp_plane_area[optimal_plane_idx];
	printf("before : %lf, after : %lf, num of overhang : %i\n", cur_area + total_area, done_area + total_area, (int)overhang_idx.size());

	overhang_idx = temp_overhang_idx;
	return true;
}

bool InfillVoxel::stepAll()
{
	int* t_wall[3];
	for (int i = 0; i < 3; ++i)
		t_wall[i] = new int[(Nx - 1)*(Ny - 1)*(Nz - 1)];

	for (int i = 0; i < (Nx - 1)*(Ny - 1)*(Nz - 1); ++i)
	{
		t_wall[XY][i] = wall[XY][i];
		t_wall[YZ][i] = wall[YZ][i];
		t_wall[ZX][i] = wall[ZX][i];
	}

	total_area = INT_MAX;

	std::vector<Point3> temp_overhang_idx = overhang_idx;
	
	std::vector<int> iter;
	iter.resize(overhang_idx.size()*3, 0);


	for (int iter = 0; iter < INT_MAX; iter++)
	{
		int result = 0;
		while (overhang_idx.size() != 0)
		{
			updatePlanes();
			if (plane_idx.size() == 0)
				break;

			int idx = plane_idx_order[rand()%plane_idx_order.size()];
			result += plane_area[idx];
			if (idx % 3 == PlaneType::XY)
			{
				propagatePlaneXY(overhang_idx[idx / 3].x - 1, overhang_idx[idx / 3].y, overhang_idx[idx / 3].z, true);
			}
			else if (idx % 3 == PlaneType::YZ)
			{
				propagatePlaneYZ(overhang_idx[idx / 3].x, overhang_idx[idx / 3].y - 1, overhang_idx[idx / 3].z, true);
			}
			else if (idx % 3 == PlaneType::ZX)
			{
				propagatePlaneZX(overhang_idx[idx / 3].x, overhang_idx[idx / 3].y, overhang_idx[idx / 3].z - 1, true);
			}
		}
		if (total_area > result)
		{
			total_area = result;
			printf("area : %d, iter : %d\n", total_area, iter);
		}

// 		if (iter%100000 == 0)
// 			printf("iter : %d\n", iter);
		
		memcpy(wall[XY], t_wall[XY], (Nx - 1)*(Ny - 1)*(Nz - 1)*sizeof(int));
		memcpy(wall[YZ], t_wall[YZ], (Nx - 1)*(Ny - 1)*(Nz - 1)*sizeof(int));
		memcpy(wall[ZX], t_wall[ZX], (Nx - 1)*(Ny - 1)*(Nz - 1)*sizeof(int));
	
		overhang_idx = temp_overhang_idx;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (t_wall[i] != nullptr)
		{
			delete[] t_wall[i];
			t_wall[i] = nullptr;
		}
	}

	return true;
}

void InfillVoxel::updatePlanes()
{
	updateOverhang();

	plane_idx.clear();
	plane_area.clear();
	plane_cnt.clear();
	plane_idx_order.clear();

	if (overhang_idx.size() == 0)
		return;

	
	plane_idx.resize(overhang_idx.size() * 3);
	std::iota(plane_idx.begin(), plane_idx.end(), 0);
	plane_area.resize(overhang_idx.size() * 3, 0);
	plane_cnt.resize(overhang_idx.size() * 3, 0);

	// Removing duplicated plane
	for (int i = 0; i < plane_idx.size(); ++i)
	{
		if (plane_idx[i] != i) continue;

		int d_idx = i % 3;
		int over_idx = i / 3;

		plane_idx_order.push_back(i);

		if (d_idx == PlaneType::XY)
		{
			area[d_idx] = 0;
			for (int j = 0; j < (Nx - 1)*(Ny - 1); ++j)
				search[XY][j] = false;
			propagatePlaneXY(overhang_idx[over_idx].x - 1, overhang_idx[over_idx].y, overhang_idx[over_idx].z);
			for (int j = i + 3; j < plane_idx.size(); j += 3)
			{
				if (overhang_idx[j / 3].z == overhang_idx[over_idx].z)
				{
					int x = overhang_idx[j / 3].x;
					int y = overhang_idx[j / 3].y;
					if (search[XY][(x - 1)*(Ny - 1) + y] || search[XY][(x)*(Ny - 1) + y - 1])
						plane_idx[j] = i;
				}
			}
		}
		else if (d_idx == PlaneType::YZ)
		{
			area[d_idx] = 0;
			for (int j = 0; j < (Ny - 1)*(Nz - 1); ++j)
				search[YZ][j] = false;
			propagatePlaneYZ(overhang_idx[over_idx].x, overhang_idx[over_idx].y - 1, overhang_idx[over_idx].z);
			for (int j = i + 3; j < plane_idx.size(); j += 3)
			{
				if (overhang_idx[j / 3].x == overhang_idx[over_idx].x)
				{
					int y = overhang_idx[j / 3].y;
					int z = overhang_idx[j / 3].z;
					if (search[YZ][(y - 1)*(Nz - 1) + z] || search[YZ][(y)*(Nz - 1) + z - 1])
						plane_idx[j] = i;
				}
			}
		}
		else if (d_idx == PlaneType::ZX)
		{
			area[d_idx] = 0;
			for (int j = 0; j < (Nz - 1)*(Nx - 1); ++j)
				search[ZX][j] = false;
			propagatePlaneZX(overhang_idx[over_idx].x, overhang_idx[over_idx].y, overhang_idx[over_idx].z - 1);
			for (int j = i + 3; j < plane_idx.size(); j += 3)
			{
				if (overhang_idx[j / 3].y == overhang_idx[over_idx].y)
				{
					int z = overhang_idx[j / 3].z;
					int x = overhang_idx[j / 3].x;
					if (search[ZX][(z - 1)*(Nx - 1) + x] || search[ZX][(z)*(Nx - 1) + x - 1])
						plane_idx[j] = i;
				}
			}
		}
		plane_area[i] = area[d_idx];
	}
// 	for (int i = 0; i < plane_cnt.size(); ++i)
// 		plane_cnt[plane_idx[i]]++;
	// End Removing duplicated plane
}
