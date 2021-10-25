/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#include "stdafx.h"
#include "SupportData.h"
#include "settings.h"
#include "ModelContainer.h"

template<typename T> inline void t_swap(T& p0, T& p1)
{
	T tmp = p0;
	p0 = p1;
	p1 = tmp;
}

int cmp_SupportInfo(const void* a, const void* b)
{
	return ((SupportInfo*)a)->z - ((SupportInfo*)b)->z;
}

SupportData::SupportData()
	: m_type(nullptr)
	, grid(nullptr)
	, b_grid(nullptr)
	, generated(false)
	, b_changed(true)
{
}

SupportData::~SupportData()
{
	if (m_type != nullptr)
		delete[] m_type;
	if (grid) delete[] grid;
	if (b_grid) delete[] b_grid;
}

void SupportData::checkNeedSupport()
{
	for (int32_t x = 0; x < gridWidth; x++)
	{
		for (int32_t y = 0; y < gridHeight; y++)
		{
			unsigned int n = x + y * gridWidth;
			for (int i = 0; i < b_grid[n].size(); ++i)
			{
				if (b_grid[n][i] && grid[n][i].z != 0)
				{
					generated = true;
					return;
				}
			}
		}
	}

	generated = false;
}
bool SupportData::checkOverhang()
{
	double cosAngle = sin(angle / 180.0 * M_PI);

	//by hyungjun --> support가 none일 경우, default angle값으로 overhang을 계산함 : 60 deg//
	if (angle == -1)
		cosAngle = sin(60 / 180.0 * M_PI);

	for (int32_t x = 0; x < gridWidth; x++)
	{
		for (int32_t y = 0; y < gridHeight; y++)
		{
			unsigned int n = x + y * gridWidth;

			for (int i = 0; i < b_grid[n].size(); ++i)
			{
				if (-grid[n][i].cosAngle > cosAngle && !b_grid[n][i] && grid[n][i].z > 0)
					return true;
			}
		}
	}

	return false;
}

void SupportData::generateSupportGrid(std::vector<IMeshModel*> models_, int supportAngle_, bool supportEverywhere_, int supportXYDistance_, int supportZDistance_, int supportHorizontalExpansion_, int layerHeight_)
{
	if (!b_changed)
		return;

	clear();

	generated = true;
	b_changed = false;
	gridScale = 400;
	int offset_scale = 1;
	int plane_offset = gridScale * offset_scale;

	AABB modelsaabb = AABBGetter()(models_);
	qglviewer::Vec vMin = modelsaabb.getMinimum() * 1000;
	qglviewer::Vec vMax = modelsaabb.getMaximum() * 1000;
	gridOffset.X = vMin.x - plane_offset;
	gridOffset.Y = vMin.y - plane_offset;
	gridWidth = ((vMax.x + plane_offset - gridOffset.X) / gridScale) + 1;
	gridHeight = ((vMax.y + plane_offset - gridOffset.Y) / gridScale) + 1;
	grid = new vector<SupportInfo>[gridWidth * gridHeight];
	b_grid = new vector<bool>[gridWidth * gridHeight];

	angle = supportAngle_;
	everywhere = supportEverywhere_;
	XYDistance = supportXYDistance_;
	ZDistance = supportZDistance_;
	horizontalExpansion = supportHorizontalExpansion_;
	layerHeight = layerHeight_;

	outlines.clear();

	for (std::vector<IMeshModel*>::iterator vit = models_.begin(); vit != models_.end(); ++vit)
	{
		AABB aabb = (*vit)->getAABB();
		qglviewer::Vec modelMin = aabb.getMinimum() * 1000;
		qglviewer::Vec modelMax = aabb.getMaximum() * 1000;
		engine::Polygons polys;
		engine::PolygonRef poly = polys.newPoly();

		Point min = Point((modelMin.x - plane_offset - gridOffset.X) / gridScale, (modelMin.y - plane_offset - gridOffset.Y) / gridScale);
		Point max = Point((modelMax.x + plane_offset - gridOffset.X) / gridScale, (modelMax.y + plane_offset - gridOffset.Y) / gridScale);

		poly.add(Point(min.X, min.Y));
		poly.add(Point(max.X, min.Y));
		poly.add(Point(max.X, max.Y));
		poly.add(Point(min.X, max.Y));

		outlines = outlines.unionPolygons(polys);

		Mesh * mesh_ = (*vit)->getMesh();
		generateSupportGrid(mesh_);
	}


	double cosAngle = sin(angle / 180.0 * M_PI);
	b_inside.clear();
	b_inside.resize(gridWidth*gridHeight, false);
	for (int32_t x = 0; x < gridWidth; x++)
	{
		for (int32_t y = 0; y < gridHeight; y++)
		{
			unsigned int n = x + y * gridWidth;
			qsort(grid[n].data(), grid[n].size(), sizeof(SupportInfo), cmp_SupportInfo);
			b_grid[n].resize(grid[n].size(), false);

			if (everywhere)	// everywhere
			{
				for (int i = 0; i < b_grid[n].size(); ++i)
				{
					if (-grid[n][i].cosAngle > cosAngle)
						b_grid[n][i] = true;
				}
			}
			else if (angle > 0)// touchingPlane // Not none
			{
				if (grid[n].size() > 0)
				{
					if (-grid[n][0].cosAngle > cosAngle)
						b_grid[n][0] = true;
				}
			}

			if (grid[n].size() > 0)
				b_inside[n] = true;
			else
			{
				for (int i = 0; i < outlines.size(); ++i)
					if (outlines[i].inside(Point(x, y)))
					{
						b_inside[n] = true;
						break;
					}
			}

		}
	}

	gridOffset.X += gridScale / 2;
	gridOffset.Y += gridScale / 2;
}

void SupportData::generateSupportGrid(Mesh* mesh_)
{
	for (Mesh::FaceIter f_it = mesh_->faces_begin(); f_it != mesh_->faces_end(); ++f_it)
	{
		Mesh::FaceHalfedgeIter fh_it = mesh_->fh_iter(f_it.handle());
		std::vector<Mesh::Point> points;
		for (; fh_it.is_valid(); ++fh_it) {
			Mesh::VertexHandle vh = mesh_->from_vertex_handle(*fh_it);
			points.push_back(mesh_->point(vh) * 1000);
		}
		if (points.size() != 3)
			continue;

		Point3 v0 = Point3(points[0][0], points[0][1], points[0][2]);
		Point3 v1 = Point3(points[1][0], points[1][1], points[1][2]);
		Point3 v2 = Point3(points[2][0], points[2][1], points[2][2]);

		FPoint3 fv0 = FPoint3((double)v0.x, (double)v0.y, (double)v0.z);
		FPoint3 fv1 = FPoint3((double)v1.x, (double)v1.y, (double)v1.z);
		FPoint3 fv2 = FPoint3((double)v2.x, (double)v2.y, (double)v2.z);

		FPoint3 normal = (fv1 - fv0).cross(fv2 - fv0);
		double normalSize = normal.vSize();

		double cosAngle = double(normal.z) / double(normalSize);

		v0.x = (v0.x - gridOffset.X) / gridScale;
		v0.y = (v0.y - gridOffset.Y) / gridScale;
		v1.x = (v1.x - gridOffset.X) / gridScale;
		v1.y = (v1.y - gridOffset.Y) / gridScale;
		v2.x = (v2.x - gridOffset.X) / gridScale;
		v2.y = (v2.y - gridOffset.Y) / gridScale;

		if (v0.x > v1.x) t_swap(v0, v1);
		if (v1.x > v2.x) t_swap(v1, v2);
		if (v0.x > v1.x) t_swap(v0, v1);
		for (int64_t x = v0.x; x < v1.x; x++)
		{
			int64_t y0 = v0.y + (v1.y - v0.y) * (x - v0.x) / (v1.x - v0.x);
			int64_t y1 = v0.y + (v2.y - v0.y) * (x - v0.x) / (v2.x - v0.x);
			int64_t z0 = v0.z + (v1.z - v0.z) * (x - v0.x) / (v1.x - v0.x);
			int64_t z1 = v0.z + (v2.z - v0.z) * (x - v0.x) / (v2.x - v0.x);

			if (y0 > y1) { t_swap(y0, y1); t_swap(z0, z1); }
			for (int64_t y = y0; y < y1; y++)
			{
				int index = x + y * gridWidth;
				if (index < 0) continue;
				if (index >= gridWidth * gridHeight) continue;
				grid[index].push_back(SupportInfo(z0 + (z1 - z0) * (y - y0) / (y1 - y0), cosAngle, false));
			}
		}
		for (int64_t x = v1.x; x < v2.x; x++)
		{
			int64_t y0 = v1.y + (v2.y - v1.y) * (x - v1.x) / (v2.x - v1.x);
			int64_t y1 = v0.y + (v2.y - v0.y) * (x - v0.x) / (v2.x - v0.x);
			int64_t z0 = v1.z + (v2.z - v1.z) * (x - v1.x) / (v2.x - v1.x);
			int64_t z1 = v0.z + (v2.z - v0.z) * (x - v0.x) / (v2.x - v0.x);

			if (y0 > y1) { t_swap(y0, y1); t_swap(z0, z1); }
			for (int64_t y = y0; y < y1; y++)
			{
				int index = x + y * gridWidth;
				if (index < 0) continue;
				if (index >= gridWidth * gridHeight) continue;
				grid[index].push_back(SupportInfo(z0 + (z1 - z0) * (y - y0) / (y1 - y0), cosAngle, false));
			}
		}
	}
}

double SupportData::calSupportVol(IMeshModel * model_, int supportAngle, bool supportEverywhere, int supportXYDistance, int supportZDistance, int supportHorizontalExpansion, double& overhangArea)
{
	clear();
	generated = true;

	qglviewer::Vec vMin = model_->getAABB().getMinimum() * 1000;
	qglviewer::Vec vMax = model_->getAABB().getMaximum() * 1000;
	gridOffset.X = vMin.x;
	gridOffset.Y = vMin.y;
	gridScale = 400;
	gridWidth = ((vMax.x - vMin.x) / gridScale) + 1;
	gridHeight = ((vMax.y - vMin.y) / gridScale) + 1;
	grid = new vector<SupportInfo>[gridWidth * gridHeight];
	b_grid = new vector<bool>[gridWidth * gridHeight];
	angle = supportAngle;
	everywhere = supportEverywhere;
	XYDistance = supportXYDistance;
	ZDistance = supportZDistance;
	horizontalExpansion = supportHorizontalExpansion;

	generateSupportGrid(model_->getMesh());

	double cosAngle = sin(supportAngle / 180.0 * M_PI);
	for (int32_t x = 0; x < gridWidth; x++)
	{
		for (int32_t y = 0; y < gridHeight; y++)
		{
			unsigned int n = x + y * gridWidth;
			qsort(grid[n].data(), grid[n].size(), sizeof(SupportInfo), cmp_SupportInfo);
		}
	}

	double sum = 0;
	for (int i = 0; i < gridWidth * gridHeight; i++)
	{
		for (int j = 0; j < grid[i].size(); j += 2)
		{
			if (-grid[i][j].cosAngle > cosAngle)
			{
				if (j == 0) { sum += (grid[i][j].z*0.001)*(gridScale*0.01)*(gridScale*0.01); }
				else { sum += (grid[i][j].z - grid[i][j - 1].z)*(0.001)*(gridScale * 0.01)*(gridScale * 0.01); }
			}
		}
	}

	for (int i = 0; i < gridWidth * gridHeight; i++) //위랑 합칠거임
	{
		vector<SupportInfo> tmpVec;
		for (int j = 0; j < grid[i].size(); j++)
		{
			////vector 생성
			if (j == 0) //vector z 같은거로 split //처음엔 일단 너줘야함
				tmpVec.push_back(grid[i][j]);
			else if (grid[i][j - 1].z == grid[i][j].z) //전거랑 비교해서 같으면 push_back
				tmpVec.push_back(grid[i][j]);
			else //z 다름 새로운 벡터 시작
			{
				//tmpVec 다 쌓였을 때 처리
				bool b_overlap = false;
				for (int k = 0; k < tmpVec.size() - 1; k++)						//bubble sort 유사
				{
					for (int m = k + 1; m < tmpVec.size(); m++)
					{
						int val = tmpVec[k].cosAngle * tmpVec[m].cosAngle;
						if (val < 0)
							b_overlap = true;									//하나라두 OVERLAP<0인조건 있으면 무시 --> 이게 AMF FILE 아닌 경우 수직단면에서 일어남
					}
				}
				vector<SupportInfo>* p = &(grid[i]);
				if (!b_overlap && (-grid[i][j - 1].cosAngle > cosAngle) && (grid[i][j - 1].z > 1)) //j==0안들어옴
				{
					double cos = (-grid[i][j - 1].cosAngle);
					overhangArea += 1 / cos; //grid 면적 1 가정 -->cos 가중치 부여 
				}
				tmpVec.clear();
				tmpVec.push_back(grid[i][j]); //tmpVec 지우고 새로운거 시작
			}
		}
	}

	return sum;
}


int SupportData::checkSupportType(Point p)
{
	// return 0 --> non support
	// return 1 --> don't generate support
	// return 2 --> generate support

	if (p.X < 1) return 0;
	if (p.Y < 1) return 0;
	if (p.X >= gridWidth - 1) return 0;
	if (p.Y >= gridHeight - 1) return 0;

	unsigned int n = p.X + p.Y*gridWidth;


	for (unsigned int i = 0; i < grid[n].size(); i++)
	{
		if (grid[n][i].z >= m_z && (i == 0 || grid[n][i - 1].z < m_z))
		{
			if (b_grid[n][i])
				return 2;
			else
				return 1;
		}
	}
	return 0;
}
void SupportData::init()
{
	if (m_type != nullptr)
		delete[] m_type;

	m_type = new int[gridWidth*gridHeight];

	for (int i = 0; i < gridWidth; ++i)
	{
		for (int j = 0; j < gridHeight; ++j)
			m_type[j*gridWidth + i] = checkSupportType(Point(i, j));
	}
}

void SupportData::setZ(int32_t Z)
{
	m_z = Z;
	init();
}
int32_t SupportData::getZ()
{
	return m_z;
}
void SupportData::setRadius(int r)
{
	radius = r;
}

Point3 SupportData::getPoint(int x, int y)
{
	Point3 pt(0, 0, 0);
	if (x > 0 && x < gridWidth - 1 && y > 0 && y < gridHeight - 1)
	{
		pt.x = gridOffset.X + x * gridScale;
		pt.y = gridOffset.Y + y * gridScale;
		pt.z = m_z;
	}


	return pt;
}

int SupportData::getType(int x, int y)
{
	return m_type[y*gridWidth + x];
}
int SupportData::getType(int n)
{
	return m_type[n];
}

void SupportData::enable(double x, double y)
{
	Point p;
	int r = radius;
	p.X = (MM2INT(x) - gridOffset.X) / gridScale;
	p.Y = (MM2INT(y) - gridOffset.Y) / gridScale;

	int searchMinX = p.X - r > 0 ? p.X - r : 0;
	int searchMaxX = p.X + r < gridWidth ? p.X + r : gridWidth - 1;
	int searchMinY = p.Y - r > 0 ? p.Y - r : 0;
	int searchMaxY = p.Y + r < gridHeight ? p.Y + r : gridHeight - 1;

	for (int xx = searchMinX; xx <= searchMaxX; ++xx)
		for (int yy = searchMinY; yy <= searchMaxY; ++yy)
		{
			if ((p.X - xx)*(p.X - xx) + (p.Y - yy)*(p.Y - yy) > r*r)
				continue;
			int n = yy * gridWidth + xx;
			for (unsigned int i = 0; i < grid[n].size(); i++)
			{
				if (grid[n][i].z >= m_z && (i == 0 || grid[n][i - 1].z < m_z))
				{
					b_grid[n][i] = true;
					if (m_type[n] != 0)
						m_type[n] = 2;
					break;
				}
			}
		}
}

void SupportData::disable(double x, double y)
{
	Point p;
	int r = radius;
	p.X = (MM2INT(x) - gridOffset.X) / gridScale;
	p.Y = (MM2INT(y) - gridOffset.Y) / gridScale;

	int searchMinX = p.X - r > 0 ? p.X - r : 0;
	int searchMaxX = p.X + r < gridWidth ? p.X + r : gridWidth - 1;
	int searchMinY = p.Y - r > 0 ? p.Y - r : 0;
	int searchMaxY = p.Y + r < gridHeight ? p.Y + r : gridHeight - 1;


	for (int xx = searchMinX; xx <= searchMaxX; ++xx)
		for (int yy = searchMinY; yy <= searchMaxY; ++yy)
		{
			if ((p.X - xx)*(p.X - xx) + (p.Y - yy)*(p.Y - yy) > r*r)
				continue;
			int n = yy * gridWidth + xx;
			for (unsigned int i = 0; i < grid[n].size(); i++)
			{
				if (grid[n][i].z >= m_z && (i == 0 || grid[n][i - 1].z < m_z))
				{
					b_grid[n][i] = false;
					if (m_type[n] != 0)
						m_type[n] = 1;
					break;
				}
			}
		}
}

void SupportData::deleteAllsupports()
{
	for (int n = 0; n < gridWidth*gridHeight; ++n)
	{
		for (unsigned int i = 0; i < b_grid[n].size(); i++)
		{
			b_grid[n][i] = false;
		}
	}
	init();
}

void SupportData::resetAllsupports(int m)
{
	deleteAllsupports();

	double cosAngle = sin(angle / 180.0 * M_PI);
	// m == 1   touching
	// m == 2	everywheresupport
	if (m == 1)
	{
		for (int n = 0; n < gridWidth*gridHeight; ++n)
		{
			if (grid[n].size() > 0)
			{
				if (-grid[n][0].cosAngle > cosAngle)
					b_grid[n][0] = true;
			}
		}
	}
	else if (m == 2)
	{
		for (int n = 0; n < gridWidth*gridHeight; ++n)
		{
			for (unsigned int i = 0; i < b_grid[n].size(); i++)
			{
				if (-grid[n][i].cosAngle > cosAngle)
					b_grid[n][i] = true;
			}
		}
	}
	init();
}





SupportPolygonGenerator::SupportPolygonGenerator(SupportData* supportData_)
	: supportData(supportData_)
{

}

engine::Polygons SupportPolygonGenerator::generateSupportPolygons(int32_t z_)
{
	if (!supportData->generated)
		return polygons;
	nr = 0;
	z = z_;
	done = new int[supportData->gridWidth*supportData->gridHeight];
	memset(done, 0, sizeof(int) * supportData->gridWidth*supportData->gridHeight);

	for (int32_t y = 1; y < supportData->gridHeight; y++)
	{
		for (int32_t x = 1; x < supportData->gridWidth; x++)
		{
			if (done[x + y * supportData->gridWidth] == 0)
			{
				if (needSupportAt(Point(x, y)))
				{
					lazyFill(Point(x, y));
				}
			}
		}
	}

	delete[] done;

	//XY distance로 offset을 하는 방식에서, horizontal expansion의 값으로 offset하는 것으로 변경됨..//
	polygons = polygons.offset(supportData->horizontalExpansion);
	return polygons;
}

bool SupportPolygonGenerator::needSupportAt(Point p)
{
	if (p.X < 1) return false;
	if (p.Y < 1) return false;
	if (p.X >= supportData->gridWidth - 1) return false;
	if (p.Y >= supportData->gridHeight - 1) return false;
	if (done[p.X + p.Y * supportData->gridWidth]) return false;

	unsigned int n = p.X + p.Y*supportData->gridWidth;

	bool ok = false;
	for (unsigned int i = 0; i < supportData->grid[n].size(); i++)
	{
		if (supportData->ZDistance == 0)
		{
			if (supportData->grid[n][i].z + (supportData->layerHeight / 2) >= z && (i == 0 || supportData->grid[n][i - 1].z + supportData->ZDistance < z))
			{
				if (supportData->b_grid[n][i])
					ok = true;
				else
					ok = false;
				break;
			}
		}
		else
		{
			if (supportData->grid[n][i].z - supportData->ZDistance >= z && (i == 0 || supportData->grid[n][i - 1].z + supportData->ZDistance < z))
			{
				if (supportData->b_grid[n][i])
					ok = true;
				else
					ok = false;
				break;
			}
		}
	}

	return ok;
}

void SupportPolygonGenerator::lazyFill(Point startPoint)
{
	nr++;

	engine::PolygonRef poly = polygons.newPoly();
	engine::Polygon tmpPoly;

	while (1)
	{
		Point p = startPoint;
		done[p.X + p.Y * supportData->gridWidth] = nr;
		while (needSupportAt(p + Point(1, 0)))
		{
			p.X++;
			done[p.X + p.Y * supportData->gridWidth] = nr;
		}
		if (tmpPoly.size() == 0)
			tmpPoly.add(startPoint * supportData->gridScale + supportData->gridOffset - Point(supportData->gridScale / 2, supportData->gridScale / 2));
		else
			tmpPoly.add(startPoint * supportData->gridScale + supportData->gridOffset - Point(supportData->gridScale / 2, 0));
		if (poly.size() == 0)
			poly.add(p * supportData->gridScale + supportData->gridOffset - Point(0, supportData->gridScale / 2));
		else
			poly.add(p * supportData->gridScale + supportData->gridOffset);

		startPoint.Y++;
		while (!needSupportAt(startPoint) && startPoint.X <= p.X)
			startPoint.X++;
		if (startPoint.X > p.X)
		{
			for (unsigned int n = 0; n < tmpPoly.size(); n++)
			{
				poly.add(tmpPoly[tmpPoly.size() - n - 1]);
			}
			//polygons.add(poly);
			return;
		}
		while (needSupportAt(startPoint - Point(1, 0)) && startPoint.X > 1)
			startPoint.X--;
	}
}



int compareSeg_int64_t(const void* a, const void* b)
{

	int64_t n = ((segment_Info*)a)->y - ((segment_Info*)b)->y;
	if (n < 0) return -1;
	if (n > 0) return 1;
	return 0;
}

int reverse_compareSeg_int64_t(const void* a, const void* b)
{
	int64_t n = ((segment_Info*)b)->y - ((segment_Info*)a)->y;
	if (n < 0) return -1;
	if (n > 0) return 1;
	return 0;
}

void NEWSupport::generateNEWSupport(int& _minX, int& _minY, int& _segmentLength, int _partition_length, int _spacingLength, bool _b_partition)
{
	init(_minX, _minY, _segmentLength, _partition_length, _spacingLength, _b_partition);

	registerCutList();

	modifyCutList();

	partitioning(_b_partition);

	connect();
}

void NEWSupport::initParams(int& _minX, int& _minY, int& _segmentLength, int _partition_length, int _spacingLength)
{
	//minX,minY 조정 
	if (_minX == 0 && _minY == 0)
	{
		NEWSupport::minX = INT_MAX;
		NEWSupport::minY = INT_MAX;
	}

	boundary = (*in_outline);
	_minY = boundary.min.Y;	 //static --> generateNEWSupport기능 구현될때마다 _minY값 변경되는거 막음 
	_minX = boundary.min.X;	 //static 쓰려고 했는데 모델 바뀌면 문제가 생김 

	int _maxY = boundary.max.Y;
	int _maxX = boundary.max.X;

	int n_Vsegment = (_maxY - _minY) / _partition_length + 1; // spacing은 n_Vsegment-1;
	_segmentLength = ((_maxY - _minY) - _spacingLength * 2 * (n_Vsegment - 1)) / n_Vsegment;

	if (NEWSupport::minX > _minX)
		NEWSupport::minX = _minX;

	if (NEWSupport::minY > _minY)
		NEWSupport::minY = _minY;

	////////////////////////걍 시작 0으로, min말고... 추후 수정
	NEWSupport::minX = 0;
	NEWSupport::minY = 0;
	////////////////////////
	NEWSupport::segmentLength = _segmentLength;
	NEWSupport::partitionLength = _partition_length;
	NEWSupport::spacingLength = _spacingLength;
}

void NEWSupport::init(int& _minX, int& _minY, int& _segmentLength, int _partition_length, int _spacingLength, bool _b_partition)
{
	in_outline->offset(extrusionWidth * infillOverlap / 100);
	matrix = rotation;
	//outline.offset(-3000);
	in_outline->applyMatrix(matrix);

	boundary = (*in_outline);

	boundary.min.X = ((boundary.min.X / lineSpacing) - 1) * lineSpacing;
	int lineCount = (boundary.max.X - boundary.min.X + (lineSpacing - 1)) / lineSpacing;

	cutList.clear();
	for (int n = 0; n < lineCount; n++)
		cutList.push_back(vector<segment_Info>());

	NEWSupport::minX = _minX;
	NEWSupport::minY = _minY;
	////////////////////////걍 시작 0으로, min말고... 추후 수정
	NEWSupport::minX = 0;
	NEWSupport::minY = 0;
	////////////////////////
	//NEWSupport::segmentLength = _segmentLength;
	NEWSupport::segmentLength = _partition_length;

	NEWSupport::partitionLength = _partition_length;
	NEWSupport::spacingLength = _spacingLength;
	NEWSupport::b_partition = _b_partition;

	maxLineLength = lineSpacing * 3; // what for? 너무 연결하는선이 길면 연결하지마
}

void NEWSupport::registerCutList()
{
	int start, end;
	for (unsigned int polyNr = 0; polyNr < in_outline->size(); polyNr++)
	{
		Point p1 = (*in_outline)[polyNr][(*in_outline)[polyNr].size() - 1];
		start = (*in_outline)[polyNr].size() - 1;
		for (unsigned int i = 0; i < (*in_outline)[polyNr].size(); i++)
		{
			Point p0 = (*in_outline)[polyNr][i];
			end = i;
			int idx0 = (p0.X - boundary.min.X) / lineSpacing;
			int idx1 = (p1.X - boundary.min.X) / lineSpacing;
			int64_t xMin = p0.X, xMax = p1.X;
			if (p0.X > p1.X) { xMin = p1.X; xMax = p0.X; }
			if (idx0 > idx1) { int tmp = idx0; idx0 = idx1; idx1 = tmp; }

			for (int idx = idx0; idx <= idx1; idx++)
			{
				int x = (idx * lineSpacing) + boundary.min.X + lineSpacing / 2;
				if (x < xMin) continue;
				if (x >= xMax) continue;
				int y = p0.Y + (p1.Y - p0.Y) * (x - p0.X) / (p1.X - p0.X);

				segment_Info seg;
				seg.polyNr = polyNr;
				seg.x = x;
				seg.y = y;
				seg.st_Line_idx = start;
				seg.end_Line_idx = end;
				seg.level = (y - minY) / segmentLength;
				cutList[idx].push_back(seg);
			}
			p1 = p0;
			start = start + 1 == (*in_outline)[polyNr].size() ? 0 : start + 1;
		}
	}

	for (int i = 0; i < cutList.size(); i++)
		qsort(cutList[i].data(), cutList[i].size(), sizeof(segment_Info), compareSeg_int64_t);
}

void NEWSupport::modifyCutList()
{
	//cutList[idx] --> partitioning by grid length
	//spacing 범위 안에 있는 점들 조정 -> spacing 경계로 밀어내기 for example -> spacing 600, gap 15000 --> 120138 ->119400(홀수 point num 아래로, 짝수 point num 위로)로 보내야함
	for (int i = 0; i < cutList.size(); i++)
	{
		//int pointNum = cutList[i].size();

		for (int j = 0; j < cutList[i].size(); j++)
		{
			if (j == 0 || j == cutList[i].size() - 1)
				continue;			// 시작점과 끝점은 바꾸지 마라
			int y = cutList[i][j].y;
			int k = (y - minY) / segmentLength;
			if (j % 2 == 0) //even 
			{
				//예외처리
				if (j + 1 < cutList[i].size())
				{
					int next_y = cutList[i][j + 1].y;
					//if ( ((y - minY) % segmentLength < spacingLength || (y - minY) % segmentLength>segmentLength - spacingLength) && ((next_y - minY) % segmentLength < spacingLength || (next_y - minY) % segmentLength>segmentLength - spacingLength) )
					//{
					//	if (abs(y-next_y) <= 2 * spacingLength)
					//	{
					//		cutList[i].erase(cutList[i].begin() + j);
					//		cutList[i].erase(cutList[i].begin() + j);
					//		j -= 1;
					//		continue;
					//	}
					//}
					//else
					//{
					//	if (abs(y - next_y) <= spacingLength) //임의 아주작은 선분 날려라
					//	{
					//		cutList[i].erase(cutList[i].begin() + j);
					//		cutList[i].erase(cutList[i].begin() + j);
					//		j -= 1;
					//		continue;
					//	}
					//}
					if (abs(y - next_y) <= spacingLength) //임의 아주작은 선분 날려라
					{
						cutList[i].erase(cutList[i].begin() + j);
						cutList[i].erase(cutList[i].begin() + j);
						j -= 1;
						continue;
					}
				}

				/////

				if ((y - minY) % segmentLength < spacingLength) //split line 기준 위
					cutList[i][j].y = k * segmentLength + minY + spacingLength;
				else if ((y - minY) % segmentLength > segmentLength - spacingLength) //split line 기준 아래
					cutList[i][j].y = (k + 1)*segmentLength + minY + spacingLength;
			}
			else //odd
			{
				//예외처리
				if (j + 1 < cutList[i].size())
				{
					int next_y = cutList[i][j + 1].y;
					if (abs(y - next_y) <= spacingLength)
					{
						cutList[i].erase(cutList[i].begin() + j);
						cutList[i].erase(cutList[i].begin() + j);
						j -= 1;
						continue;
					}
				}
				//
				if ((y - minY) % segmentLength < spacingLength) //split line 기준 위
					cutList[i][j].y = k * segmentLength + minY - spacingLength;
				else if ((y - minY) % segmentLength > segmentLength - spacingLength) //split line 기준 아래
					cutList[i][j].y = (k + 1)*segmentLength + minY - spacingLength;
			}
		}
	}
}

void NEWSupport::partitioning(bool _b_partition)
{
	//partitioning 점 추가
	b_partition = _b_partition;
	if (b_partition)
	{
		for (int i = 0; i < cutList.size(); i++)
		{
			int ith_size = cutList[i].size();
			for (int j = 0; j < ith_size; j += 2)
			{
				if (j + 1 < ith_size)
				{
					for (int k = (cutList[i][j].y - minY) / segmentLength + 1; k <= (cutList[i][j + 1].y - minY) / segmentLength; k++)
					{
						segment_Info lower_p, higher_p;
						higher_p.polyNr = lower_p.polyNr = -1; // 새로생긴거니까 //cutList[i][j].polyNr;
						higher_p.x = lower_p.x = cutList[i][j].x;

						higher_p.y = k * segmentLength + minY + spacingLength;
						lower_p.y = k * segmentLength + minY - spacingLength;
						higher_p.level = (higher_p.y - minY) / segmentLength;
						lower_p.level = (lower_p.y - minY) / segmentLength;

						higher_p.st_Line_idx = lower_p.st_Line_idx = -1;
						higher_p.end_Line_idx = lower_p.end_Line_idx = -1; //임의로 없으
						int last_idx = cutList[i].size() - 1;

						if (lower_p.y > cutList[i][j].y && higher_p.y < cutList[i][j + 1].y)
						{
							cutList[i].push_back(lower_p);
							cutList[i].push_back(higher_p);
						}
					}
				}
			}//짝수개, even num, odd num 사이에 grid legnth로 2개 offset해서 추가 
		}
		for (int i = 0; i < cutList.size(); i++)
			qsort(cutList[i].data(), cutList[i].size(), sizeof(segment_Info), compareSeg_int64_t);
	}

}

void NEWSupport::connect()
{
	int idx = 0;
	for (int64_t x = boundary.min.X + lineSpacing / 2; x < boundary.max.X; x += lineSpacing)
	{
		/*
		if (layerNr >= 30 && layerNr <= 165)
		{
		int bridgeLength = 4000;
		if (idx % 2 == 0)
		{
		for (unsigned int i = 0; i + 1 < cutList[idx].size(); i += 2)
		{
		if (cutList[idx][i + 1].y - cutList[idx][i].y < extrusionWidth / 5)
		continue;
		PolygonRef p = result.newPoly();
		p.add(matrix.unapply(Point(x, cutList[idx][i].y)));
		p.add(matrix.unapply(Point(x, cutList[idx][i].y + bridgeLength)));

		p = result.newPoly();
		p.add(matrix.unapply(Point(x, cutList[idx][i + 1].y)));
		p.add(matrix.unapply(Point(x, cutList[idx][i + 1].y - bridgeLength)));
		}
		}
		else
		{
		for (int i = cutList[idx].size() - 1; i>0; i -= 2)
		{
		if (i - 1 < 0)
		break;
		if (cutList[idx][i].y - cutList[idx][i - 1].y < extrusionWidth / 5)
		continue;
		PolygonRef p = result.newPoly();
		p.add(matrix.unapply(Point(x, cutList[idx][i].y)));
		p.add(matrix.unapply(Point(x, cutList[idx][i].y - bridgeLength)));
		p = result.newPoly();
		p.add(matrix.unapply(Point(x, cutList[idx][i - 1].y)));
		p.add(matrix.unapply(Point(x, cutList[idx][i - 1].y + bridgeLength)));
		}
		}
		}
		else*/

		{
			if (idx % 2 == 0)
			{
				for (unsigned int i = 0; i + 1 < cutList[idx].size(); i += 2)
				{
					if (cutList[idx][i + 1].y - cutList[idx][i].y < extrusionWidth / 5)
						continue;
					engine::PolygonRef p = result->newPoly();
					p.add(matrix.unapply(Point(x, cutList[idx][i].y)));
					p.add(matrix.unapply(Point(x, cutList[idx][i + 1].y)));
				}
			}
			else
			{
				for (int i = cutList[idx].size() - 1; i > 0; i -= 2)
				{
					if (i - 1 < 0)
						break;
					if (cutList[idx][i].y - cutList[idx][i - 1].y < extrusionWidth / 5)
						continue;
					engine::PolygonRef p = result->newPoly();
					p.add(matrix.unapply(Point(x, cutList[idx][i].y)));
					p.add(matrix.unapply(Point(x, cutList[idx][i - 1].y)));
				}
			}
		}
		// till here 직선 잇기
		//from here line to line 직선 잇기
		//			bool b_connect = 1;
		//			if (connect)

		if (idx < cutList.size() - 1 && cutList[idx].size()>1 && cutList[idx + 1].size() > 1)// && idx%10!=0)//idx % ((int)(segmentLength / spacingLength / 2)) != 0) // 마지막 horizontal spacing
			connectSplittedLines(idx, x); // 0 == odd 1 == even    짝수 idx -> 홀수 point

		idx += 1;
	}
	cutList.clear();
}

double NEWSupport::getDistance(Point& a, Point& b)
{
	return sqrtf((a.X - b.X)*(a.X - b.X) + (a.Y - b.Y)*(a.Y - b.Y));
}

bool NEWSupport::isOnSameOutlineEdge(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x)
{
	if (cutList[cur_idx][cur_idx_num].polyNr == -1 || cutList[next_idx][next_idx_num].polyNr == -1)
		return false;

	if (cutList[cur_idx][cur_idx_num].polyNr == cutList[next_idx][next_idx_num].polyNr && cutList[cur_idx][cur_idx_num].st_Line_idx == cutList[next_idx][next_idx_num].st_Line_idx && cutList[cur_idx][cur_idx_num].end_Line_idx == cutList[next_idx][next_idx_num].end_Line_idx)
	{
		engine::PolygonRef p = result->newPoly();
		p.add(matrix.unapply(Point(x, cutList[cur_idx][cur_idx_num].y)));
		p.add(matrix.unapply(Point(x + lineSpacing, cutList[next_idx][next_idx_num].y)));
		return true;
	}
	else
		return false;
}


//지금 둘다 안씀 
void NEWSupport::diffZLineConnect(const Point& before_p, const Point& next_p)
{
	if ((before_p.Y - minY) % segmentLength > segmentLength - spacingLength || (before_p.Y - minY) % segmentLength< spacingLength || (next_p.Y - minY) % segmentLength > segmentLength - spacingLength || (next_p.Y - minY) % segmentLength < spacingLength)
		return;
	if (length(before_p, next_p) < maxLineLength)
	{
		engine::PolygonRef p = result->newPoly();
		Point pass_p1(before_p.X + Profile::configSettings[0].extrusion_width, before_p.Y);
		Point pass_p2(before_p.X + Profile::configSettings[0].extrusion_width, next_p.Y);
		p.add(matrix.unapply(before_p));
		p.add(matrix.unapply(pass_p1));

		p = result->newPoly();
		p.add(matrix.unapply(pass_p1));
		p.add(matrix.unapply(pass_p2));

		p = result->newPoly();
		p.add(matrix.unapply(pass_p2));
		p.add(matrix.unapply(next_p));
		//before_p = pass_p2; //update for next function
	}
}
void NEWSupport::sameIdxLineConnect(const Point& before_p, const Point& next_p)
{
	//spacing 기준으로부터 spacingLength 사이에 있는 point들 연결 안함.....
	if ((before_p.Y - minY) % segmentLength > segmentLength - spacingLength || (before_p.Y - minY) % segmentLength< spacingLength || (next_p.Y - minY) % segmentLength > segmentLength - spacingLength || (next_p.Y - minY) % segmentLength < spacingLength)
		return;

	if (length(before_p, next_p) < maxLineLength)
	{
		engine::PolygonRef p = result->newPoly();
		Point pass_p1(before_p.X + Profile::configSettings[0].extrusion_width, before_p.Y);
		Point pass_p2(before_p.X + Profile::configSettings[0].extrusion_width, next_p.Y);
		p.add(matrix.unapply(before_p));
		p.add(matrix.unapply(pass_p1));

		p = result->newPoly();
		p.add(matrix.unapply(pass_p1));
		p.add(matrix.unapply(pass_p2));

		//before_p = pass_p2;	//update for next function
	}
}
/////////////

bool NEWSupport::betweenSplitSpace(const Point& before_p, const Point& next_p)
{
	//spacing 기준으로부터 spacingLength 사이에 있는 point들 연결 안함.....
	int b_level = (before_p.Y - minY) / segmentLength;
	int n_level = (next_p.Y - minY) / segmentLength;
	if ((before_p.Y - minY) % segmentLength > segmentLength - spacingLength || (before_p.Y - minY) % segmentLength< spacingLength || (next_p.Y - minY) % segmentLength > segmentLength - spacingLength || (next_p.Y - minY) % segmentLength < spacingLength || b_level != n_level)
		return true;
	else
		return false;
}

void NEWSupport::lineConnect(const Point& before_p, const Point& next_p)
{
	if (length(before_p, next_p) < maxLineLength)
	{
		engine::PolygonRef p = result->newPoly();
		p.add(matrix.unapply(before_p));
		p.add(matrix.unapply(next_p));
	}
}

void NEWSupport::connect_OUTLINE_OUTLINE(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x, int b_even)
{
	//if (cutList[cur_idx][cur_idx_num].polyNr == cutList[next_idx][next_idx_num].polyNr)
	//{
	//	Point backup_p = Point(x, cutList[cur_idx][cur_idx_num].y);
	//	int begin_idx, end_idx;
	//	if (!b_even)  //짝수코드
	//	{
	//		int begin_idx = cutList[cur_idx][cur_idx_num].st_Line_idx;
	//		int end_idx = cutList[next_idx][next_idx_num].end_Line_idx;
	//
	//		if (begin_idx != -1 && end_idx != -1 && cutList[cur_idx][cur_idx_num].polyNr != -1 && cutList[next_idx][next_idx_num].polyNr != -1) // outline에 안걸치면 begin_idx // end_idx -1
	//		{
	//			int outlineSize = (*in_outline)[cutList[cur_idx][cur_idx_num].polyNr].size();
	//			ConfigSettings config;
	//			if (begin_idx >= end_idx)
	//			{
	//				for (int n = begin_idx; n >= end_idx; n--)
	//				{
	//					Point pt = (*in_outline)[cutList[cur_idx][cur_idx_num].polyNr][n];
	//					if (pt.X >= x && pt.X <= x + lineSpacing) // && p[p.size()-1].x
	//					{
	//						if (!betweenSplitSpace(backup_p, pt))
	//						{
	//							lineConnect(backup_p, pt);
	//							backup_p = pt;
	//						}
	//					}
	//				}
	//			}
	//			else
	//			{
	//				for (int n = begin_idx;; n--)
	//				{
	//					Point pt = (*in_outline)[cutList[cur_idx][cur_idx_num].polyNr][n];
	//
	//					if (pt.X >= x && pt.X <= x + lineSpacing) // && p[p.size()-1].x
	//					{
	//						if (!betweenSplitSpace(backup_p, pt)) //splitlineSpace영역에 없을 시 이어주고 backup_pt update 
	//						{
	//							lineConnect(backup_p, pt);
	//							backup_p = pt;
	//						}
	//					}
	//					if (n == 0)
	//					{
	//						n = outlineSize;
	//						continue; //n-> outlineSize-1됨 맨끝됨
	//					}
	//					if (n == end_idx)
	//						break;
	//				}
	//			}
	//		}
	//	}
	//	else //홀수코드
	//	{
	//		begin_idx = cutList[cur_idx][cur_idx_num].end_Line_idx;
	//		end_idx = cutList[next_idx][next_idx_num].st_Line_idx;
	//		if (begin_idx != -1 && end_idx != -1 && cutList[cur_idx][cur_idx_num].polyNr != -1 && cutList[next_idx][next_idx_num].polyNr != -1)
	//		{
	//			int outlineSize = (*in_outline)[cutList[cur_idx][cur_idx_num].polyNr].size();
	//
	//			if (begin_idx <= end_idx)
	//			{
	//				for (int n = begin_idx; n <= end_idx; n++)
	//				{
	//					Point pt = (*in_outline)[cutList[cur_idx][cur_idx_num].polyNr][n];
	//					ConfigSettings config;
	//
	//					if (pt.X >= x && pt.X <= x + lineSpacing)
	//					{
	//						if (!betweenSplitSpace(backup_p, pt))
	//						{
	//							lineConnect(backup_p, pt);
	//							backup_p = pt;
	//						}
	//					}
	//				}
	//			}
	//			else
	//			{
	//				for (int n = begin_idx; n < outlineSize; n++)
	//				{
	//					Point pt = (*in_outline)[cutList[cur_idx][cur_idx_num].polyNr][n];
	//					ConfigSettings config;
	//
	//					if (pt.X >= x && pt.X <= x + lineSpacing)
	//					{
	//						if (!betweenSplitSpace(backup_p, pt))
	//						{
	//							lineConnect(backup_p, pt);
	//							backup_p = pt;
	//						}
	//					}
	//
	//					if (n == outlineSize - 1)
	//					{
	//						n = -1;
	//						continue;
	//					}
	//					if (n == end_idx)
	//						break;
	//				}
	//			}
	//
	//		}
	//	}
	//	Point next_line_p(x + lineSpacing, cutList[next_idx][next_idx_num].y);
	//	if (!betweenSplitSpace(backup_p, next_line_p))
	//		lineConnect(backup_p, next_line_p);
	//}
	//else
	//{
	//	connect_OUTLINE_INTERIOR(cur_idx, cur_idx_num, next_idx, next_idx_num, x, b_even);
	//}

	int st_polyNr = cutList[cur_idx][cur_idx_num].polyNr;
	int end_polyNr = cutList[next_idx][next_idx_num].polyNr;


	if (st_polyNr == end_polyNr)
	{
		int st_idx = cutList[cur_idx][cur_idx_num].st_Line_idx;
		int end_idx = cutList[cur_idx][cur_idx_num].end_Line_idx;

		int criteria_x = cutList[cur_idx][cur_idx_num].x;

		Point st_pt = (*in_outline)[st_polyNr][st_idx];
		Point end_pt = (*in_outline)[st_polyNr][end_idx];

		bool b_outlineidx_increase;

		if (st_pt.X >= criteria_x && end_pt.X >= criteria_x)
		{
			if (st_pt.X >= end_pt.X)
				b_outlineidx_increase = false;
			else
				b_outlineidx_increase = true;
		}
		else if (st_pt.X >= criteria_x)
			b_outlineidx_increase = false;
		else if (end_pt.X >= criteria_x)
			b_outlineidx_increase = true;
		else { cout << "IMPOSSIBLE" << endl; }

		if (b_outlineidx_increase)
		{
			int idx = cutList[cur_idx][cur_idx_num].end_Line_idx;
			Point backup_p = Point(x, cutList[cur_idx][cur_idx_num].y);

			while ((*in_outline)[st_polyNr][idx].X < cutList[next_idx][next_idx_num].x)
			{
				Point pt = (*in_outline)[cutList[cur_idx][cur_idx_num].polyNr][idx];
				if (pt.X >= x && pt.X <= x + lineSpacing) // && p[p.size()-1].x
				{
					if (!betweenSplitSpace(backup_p, pt))
					{
						lineConnect(backup_p, pt);
						backup_p = pt;
					}
					else
					{
						pt.Y = backup_p.Y;
						lineConnect(backup_p, pt);
						backup_p = pt;
					}
				}
				idx = idx == (*in_outline)[st_polyNr].size() - 1 ? 0 : idx + 1;
			}

			Point p2 = (*in_outline)[st_polyNr][idx];
			int bef_idx = idx == 0 ? (*in_outline)[st_polyNr].size() - 1 : idx - 1;
			Point p1 = (*in_outline)[st_polyNr][bef_idx];
			int interpol_y = (double(p2.Y - p1.Y)) / (double(p2.X - p1.X)) * (double(x + lineSpacing - p1.X)) + (double)p1.Y;
			Point interpolate_p(x + lineSpacing, interpol_y);

			if (!betweenSplitSpace(backup_p, interpolate_p))
				lineConnect(backup_p, interpolate_p);
			else
			{
				interpolate_p.Y = backup_p.Y;
				lineConnect(backup_p, interpolate_p);
			}
		}
		else
		{
			int idx = cutList[cur_idx][cur_idx_num].st_Line_idx;
			Point backup_p = Point(x, cutList[cur_idx][cur_idx_num].y);

			while ((*in_outline)[st_polyNr][idx].X < cutList[next_idx][next_idx_num].x)
			{
				Point pt = (*in_outline)[cutList[cur_idx][cur_idx_num].polyNr][idx];
				if (pt.X >= x && pt.X <= x + lineSpacing) // && p[p.size()-1].x
				{
					if (!betweenSplitSpace(backup_p, pt))
					{
						lineConnect(backup_p, pt);
						backup_p = pt;
					}
					else
					{
						pt.Y = backup_p.Y;
						lineConnect(backup_p, pt);
						backup_p = pt;
					}
				}
				idx = idx == 0 ? (*in_outline)[st_polyNr].size() - 1 : idx - 1;
			}

			Point p2 = (*in_outline)[st_polyNr][idx];
			int nextidx = idx == (*in_outline)[st_polyNr].size() - 1 ? 0 : idx + 1;
			Point p1 = (*in_outline)[st_polyNr][nextidx];
			int interpol_y = (double(p2.Y - p1.Y)) / (double(p2.X - p1.X)) * (double(x + lineSpacing - p1.X)) + (double)p1.Y;
			Point interpolate_p(x + lineSpacing, interpol_y);

			if (!betweenSplitSpace(backup_p, interpolate_p))
				lineConnect(backup_p, interpolate_p);
			else
			{
				interpolate_p.Y = backup_p.Y;
				lineConnect(backup_p, interpolate_p);
			}

		}


	}

}

void NEWSupport::connect_OUTLINE_INTERIOR(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x, int b_even) //b_even -->idx_even
{
	//outline for문 도는거 무의미

	Point backup_p = Point(x, cutList[cur_idx][cur_idx_num].y);
	Point next_line_p(x + lineSpacing, cutList[next_idx][next_idx_num].y);

	if ((backup_p.Y - minY) % segmentLength > segmentLength - spacingLength || (backup_p.Y - minY) % segmentLength< spacingLength || (next_line_p.Y - minY) % segmentLength > segmentLength - spacingLength || (next_line_p.Y - minY) % segmentLength < spacingLength)
		return;

	if (length(backup_p, next_line_p) < maxLineLength)
	{
		engine::PolygonRef p = result->newPoly();

		Point pass_p1;//(backup_p.X + config.extrusion_width, backup_p.Y);
		Point pass_p2;//(backup_p.X + config.extrusion_width, next_line_p.Y);
		if ((b_even && backup_p.Y >= next_line_p.Y) || (!b_even && backup_p.Y <= next_line_p.Y))//짝수-->high points compare 
		{
			pass_p1 = Point(next_line_p.X - Profile::configSettings[0].extrusion_width, backup_p.Y);
			pass_p2 = Point(next_line_p.X - Profile::configSettings[0].extrusion_width, next_line_p.Y);
		}
		else if ((b_even && backup_p.Y < next_line_p.Y) || (!b_even && backup_p.Y > next_line_p.Y))
		{
			pass_p1 = Point(backup_p.X + Profile::configSettings[0].extrusion_width, backup_p.Y);
			pass_p2 = Point(backup_p.X + Profile::configSettings[0].extrusion_width, next_line_p.Y);
		}

		p.add(matrix.unapply(backup_p));
		p.add(matrix.unapply(pass_p1));

		p = result->newPoly();
		p.add(matrix.unapply(pass_p1));
		p.add(matrix.unapply(pass_p2));

		p = result->newPoly();
		p.add(matrix.unapply(pass_p2));
		p.add(matrix.unapply(next_line_p));
	}

	//diffZLineConnect(backup_p, next_line_p);
}

void NEWSupport::connect_INTERIOR_INTERIOR(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x, int b_even)
{
	Point backup_p = Point(x, cutList[cur_idx][cur_idx_num].y);
	Point next_line_p(x + lineSpacing, cutList[next_idx][next_idx_num].y);
	lineConnect(backup_p, next_line_p);
}

void NEWSupport::connectOutline(int cur_idx, int cur_idx_num, int next_idx, int next_idx_num, int x, int b_even) //매우 중요 
{
	enum { OUTLINE_OUTLINE = 1, OUTLINE_INTERIOR, INTERIOR_INTERIOR };
	int key;
	if (cutList[cur_idx][cur_idx_num].polyNr != -1 && cutList[next_idx][next_idx_num].polyNr != -1)
		key = OUTLINE_OUTLINE;
	else if (cutList[cur_idx][cur_idx_num].polyNr != -1 || cutList[next_idx][next_idx_num].polyNr != -1)
		key = OUTLINE_INTERIOR;
	else
		key = INTERIOR_INTERIOR;

	switch (key)
	{
	case OUTLINE_OUTLINE:
		connect_OUTLINE_OUTLINE(cur_idx, cur_idx_num, next_idx, next_idx_num, x, b_even);
		break;
	case OUTLINE_INTERIOR:
		connect_OUTLINE_INTERIOR(cur_idx, cur_idx_num, next_idx, next_idx_num, x, b_even);
		break;
	case INTERIOR_INTERIOR:
		connect_INTERIOR_INTERIOR(cur_idx, cur_idx_num, next_idx, next_idx_num, x, b_even);
		break;
	}
}

void NEWSupport::findClosestIdx(int& closest_pidx, Point& from_pt, const int idx, const int b_even)	//b_even -->idx_even
{
	//그냥 pt랑 어느줄에 있는거 젤 가까운거 찾음 //closest_idx는 그 줄 위 가장가까운 Point idx
	double LeastDist(DBL_MAX);
	for (int j = 0; j < cutList[idx].size(); j++)
	{
		if (j % 2 == b_even)
		{
			Point next_p(cutList[idx][j].x, cutList[idx][j].y);
			double dist = getDistance(from_pt, next_p);
			if (b_partition)	/// if partition -> 다른 partition과 연결하면 안됨 -->아래 if 문 뒷쪽거
			{
				if (dist < LeastDist && ((next_p.Y - minY) / segmentLength == (from_pt.Y - minY) / segmentLength))
				{
					LeastDist = dist;
					closest_pidx = j;
				}
			}
			else
			{
				if (dist < LeastDist)
				{
					LeastDist = dist;
					closest_pidx = j;
				}
			}
		}
	}
}

void NEWSupport::connectSplittedLines(int idx, int x)
{
	if (cutList[idx].size() <= cutList[idx + 1].size())
	{
		for (int i = 0; i < cutList[idx].size(); i++) // i ->point idx_even --> idx_even 
		{
			//cutlist 순서가 아닌 --> x 나눈거 기준으로 -->Simplify처럼 layer별로 꼬이는 현상 방지하기 위해서
			int idx_even = !((x / lineSpacing) % 2); // if even ->true odd->false
			if (i % 2 != idx_even) // idx_even 1 -> i 홀수   idx_even 0 -> i  짝수 
			{
				// offset 판단문 만들어야함   //공유start,end --> 그냥 이음
				Point cur_pt(x, cutList[idx][i].y);
				int toClosest_idx = -1;
				findClosestIdx(toClosest_idx, cur_pt, idx + 1, i % 2); //홀수는 홀수만 찾게 수정

				if (toClosest_idx != -1)	//for safety
				{
					Point to_pt(x + lineSpacing, cutList[idx + 1][toClosest_idx].y);
					int fromClosest_idx = -1;
					findClosestIdx(fromClosest_idx, to_pt, idx, i % 2);

					if (i == fromClosest_idx) //물어본 본인이냐 이거임
					{
						if (isOnSameOutlineEdge(idx, i, idx + 1, toClosest_idx, x)) {}
						else { connectOutline(idx, i, idx + 1, toClosest_idx, x, idx_even); }
					}
				}
			}
		}
	}
	////todo
	//else if (cutList[idx].size() < cutList[idx + 1].size())
	//{
	//	for (int i = 0; i < cutList[idx].size(); i++) //pointsize
	//	{
	//		bool idx_even = !((x / lineSpacing) % 2); // if even ->true odd->false
	//		if (i % 2 != idx_even)
	//		{
	//			Point cur_pt(x, cutList[idx][i].y);
	//			int toClosest_idx = -1;
	//			findClosestIdx(toClosest_idx, cur_pt, idx + 1, i % 2);
	//
	//			if (toClosest_idx != -1)	//for safety
	//			{
	//				Point to_pt(x + lineSpacing, cutList[idx + 1][toClosest_idx].y);
	//				int fromClosest_idx = -1;
	//				findClosestIdx(fromClosest_idx, to_pt, idx, i % 2);
	//				if (i == fromClosest_idx)
	//				{
	//					if (isOnSameOutlineEdge(idx, i, idx + 1, toClosest_idx, x)){}
	//					else { connectOutline(idx, i, idx + 1, toClosest_idx, x, idx_even); }
	//				}
	//			}
	//		}
	//	}
	//}

	else //cur보다 next가 size 작음 //next 기준
	{
		for (int i = 0; i < cutList[idx + 1].size(); i++)
		{
			int idx_even = !((x / lineSpacing) % 2); // if even ->true odd->false
			if (i % 2 != idx_even)
			{
				Point next_p(x + lineSpacing, cutList[idx + 1][i].y);
				int toClosest_idx = -1;
				findClosestIdx(toClosest_idx, next_p, idx, i % 2);

				if (toClosest_idx != -1)	//for safety
				{
					Point to_pt(x, cutList[idx][toClosest_idx].y);
					int fromClosest_idx = -1;
					findClosestIdx(fromClosest_idx, to_pt, idx + 1, i % 2);
					if (i == fromClosest_idx)
					{
						if (isOnSameOutlineEdge(idx, toClosest_idx, idx + 1, i, x)) {}
						else { connectOutline(idx, toClosest_idx, idx + 1, i, x, idx_even); }
					}
				}
			}
		}
	}
}