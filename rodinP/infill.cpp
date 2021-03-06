/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#include "stdafx.h"
#include "infill.h"
#include "AABB.h"

namespace engine {

void generateConcentricInfill(Polygons outline, Polygons& result, int inset_value)
{
    while(outline.size() > 0)
    {
        for (unsigned int polyNr = 0; polyNr < outline.size(); polyNr++)
        {
            PolygonRef r = outline[polyNr];
            result.add(r);
        }
        outline = outline.offset(-inset_value);
    }
}

void generateAutomaticInfill(const Polygons& in_outline, Polygons& result,
                             int extrusionWidth, int lineSpacing,
                             int infillOverlap, double rotation)
{
    if (lineSpacing > extrusionWidth * 4)
    {
        generateGridInfill(in_outline, result, extrusionWidth, lineSpacing,
                           infillOverlap, rotation);
    }
    else
    {
        generateLineInfill(in_outline, result, extrusionWidth, lineSpacing,
                           infillOverlap, rotation);
    }
}

void generateGridInfill(const Polygons& in_outline, Polygons& result,
                        int extrusionWidth, int lineSpacing, int infillOverlap,
                        double rotation)
{
    generateLineInfill(in_outline, result, extrusionWidth, lineSpacing * 2,
                       infillOverlap, rotation);
    generateLineInfill(in_outline, result, extrusionWidth, lineSpacing * 2,
                       infillOverlap, rotation + 90);
}

int compare_int64_t(const void* a, const void* b)
{
    int64_t n = (*(int64_t*)a) - (*(int64_t*)b);
    if (n < 0) return -1;
    if (n > 0) return 1;
    return 0;
}

void generateLineInfill(const Polygons& in_outline, Polygons& result, int extrusionWidth, int lineSpacing, int infillOverlap, double rotation)
{
    Polygons outline = in_outline.offset(extrusionWidth * infillOverlap / 100);
    PointMatrix matrix(rotation);
    
    outline.applyMatrix(matrix);
    
    AABB2D boundary(outline);
    
    boundary.min.X = ((boundary.min.X / lineSpacing) - 1) * lineSpacing;
    int lineCount = (boundary.max.X - boundary.min.X + (lineSpacing - 1)) / lineSpacing;
    vector<vector<int64_t> > cutList;
    for(int n=0; n<lineCount; n++)
        cutList.push_back(vector<int64_t>());

    for(unsigned int polyNr=0; polyNr < outline.size(); polyNr++)
    {
        Point p1 = outline[polyNr][outline[polyNr].size()-1];
        for(unsigned int i=0; i < outline[polyNr].size(); i++)
        {
            Point p0 = outline[polyNr][i];
            int idx0 = (p0.X - boundary.min.X) / lineSpacing;
            int idx1 = (p1.X - boundary.min.X) / lineSpacing;
            int64_t xMin = p0.X, xMax = p1.X;
            if (p0.X > p1.X) { xMin = p1.X; xMax = p0.X; }
            if (idx0 > idx1) { int tmp = idx0; idx0 = idx1; idx1 = tmp; }
            for(int idx = idx0; idx<=idx1; idx++)
            {
                int x = (idx * lineSpacing) + boundary.min.X + lineSpacing / 2;
                if (x < xMin) continue;
                if (x >= xMax) continue;
                int y = p0.Y + (p1.Y - p0.Y) * (x - p0.X) / (p1.X - p0.X);
                cutList[idx].push_back(y);
            }
            p1 = p0;
        }
    }
    
    int idx = 0;
    for(int64_t x = boundary.min.X + lineSpacing / 2; x < boundary.max.X; x += lineSpacing)
    {
        qsort(cutList[idx].data(), cutList[idx].size(), sizeof(int64_t), compare_int64_t);
        for(unsigned int i = 0; i + 1 < cutList[idx].size(); i+=2)
        {
            if (cutList[idx][i+1] - cutList[idx][i] < extrusionWidth / 5)
                continue;
            PolygonRef p = result.newPoly();
            p.add(matrix.unapply(Point(x, cutList[idx][i])));
            p.add(matrix.unapply(Point(x, cutList[idx][i+1])));
        }
        idx += 1;
    }
}

void generateLineInfill_offset(const Polygons& in_outline, Polygons& result, int extrusionWidth, int lineSpacing, int infillOverlap, double rotation, int offset)
{
	Polygons outline = in_outline.offset(extrusionWidth * infillOverlap / 100);
	PointMatrix matrix(rotation);

	outline.applyMatrix(matrix);

	AABB2D boundary(outline);

	boundary.min.X = ((boundary.min.X / lineSpacing) - 1) * lineSpacing;
	int lineCount = (boundary.max.X - boundary.min.X + (lineSpacing - 1)) / lineSpacing;
// 	vector<vector<int64_t> > cutList;
// 	for (int n = 0; n < lineCount; n++)
// 		cutList.push_back(vector<int64_t>());

	vector<vector<int64_t> > cutList2;
	for (int n = 0; n < lineCount; n++)
		cutList2.push_back(vector<int64_t>());

// 	for (unsigned int polyNr = 0; polyNr < outline.size(); polyNr++)
// 	{
// 		Point p1 = outline[polyNr][outline[polyNr].size() - 1];
// 		for (unsigned int i = 0; i < outline[polyNr].size(); i++)
// 		{
// 			Point p0 = outline[polyNr][i];
// 			int idx0 = (p0.X - boundary.min.X) / lineSpacing;
// 			int idx1 = (p1.X - boundary.min.X) / lineSpacing;
// 			int64_t xMin = p0.X, xMax = p1.X;
// 			if (p0.X > p1.X) { xMin = p1.X; xMax = p0.X; }
// 			if (idx0 > idx1) { int tmp = idx0; idx0 = idx1; idx1 = tmp; }
// 			for (int idx = idx0; idx <= idx1; idx++)
// 			{
// 				int x = (idx * lineSpacing) + boundary.min.X + lineSpacing / 2;
// 				if (x < xMin) continue;
// 				if (x >= xMax) continue;
// 				int y = p0.Y + (p1.Y - p0.Y) * (x - p0.X) / (p1.X - p0.X);
// 				cutList[idx].push_back(y);
// 			}
// 			p1 = p0;
// 		}
// 	}

	Polygons offsetBoundary = outline.offset(-offset);

	for (unsigned int polyNr = 0; polyNr < offsetBoundary.size(); polyNr++)
	{
		Point p1 = offsetBoundary[polyNr][offsetBoundary[polyNr].size() - 1];
		for (unsigned int i = 0; i < offsetBoundary[polyNr].size(); i++)
		{
			Point p0 = offsetBoundary[polyNr][i];
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
				cutList2[idx].push_back(y);
			}
			p1 = p0;
		}
	}

//	int idx = 0;
// 	for (int64_t x = boundary.min.X + lineSpacing / 2; x < boundary.max.X; x += lineSpacing)
// 	{
// 		qsort(cutList[idx].data(), cutList[idx].size(), sizeof(int64_t), compare_int64_t);
// 		for (unsigned int i = 0; i + 1 < cutList[idx].size(); i += 2)
// 		{
// 			if (cutList[idx][i + 1] - cutList[idx][i] < extrusionWidth / 5)
// 				continue;
// 			PolygonRef p = result.newPoly();
// 			p.add(matrix.unapply(Point(x, cutList[idx][i])));
// 			p.add(matrix.unapply(Point(x, cutList[idx][i + 1])));
// 		}
// 		idx += 1;
// 	}

	int idx = 0;
	for (int64_t x = boundary.min.X + lineSpacing / 2; x < boundary.max.X; x += lineSpacing)
	{
		qsort(cutList2[idx].data(), cutList2[idx].size(), sizeof(int64_t), compare_int64_t);
		for (unsigned int i = 0; i + 1 < cutList2[idx].size(); i += 2)
		{
			if (cutList2[idx][i + 1] - cutList2[idx][i] < extrusionWidth / 5)
				continue;
			PolygonRef p = result.newPoly();
			p.add(matrix.unapply(Point(x, cutList2[idx][i] - offset)));
			p.add(matrix.unapply(Point(x, cutList2[idx][i + 1] + offset)));
		}
		idx += 1;
	}
}

void generateLineInfill_vertical_offset(const Polygons& in_outline, Polygons& result, int extrusionWidth, int lineSpacing, int infillOverlap, double rotation, int offset)
{
	Polygons in_outline_origin = in_outline;
	Polygons offset_outline;

	Polygons offset_inline;
	for (int i = 0; i < in_outline_origin.size(); ++i)
	{
		int n_poly = in_outline_origin[i].size();
		if (n_poly < 3)
			continue;
		for (int j = 0; j < n_poly; ++j)
		{
			Point a = in_outline_origin[i][j];
			Point b = in_outline_origin[i][(j+1)%n_poly];

			if (a.X == b.X)	// vertical
			{
				engine::PolygonRef poly = offset_inline.newPoly();
				poly.add(a);
				poly.add(b);
			}
		}
	}

	offset_outline = in_outline.difference(offset_inline.offset_line(extrusionWidth*0.5));

	Polygons outline = offset_outline.offset(extrusionWidth * infillOverlap / 100);
	PointMatrix matrix(rotation);

	outline.applyMatrix(matrix);

	AABB2D boundary(outline);

	boundary.min.X = ((boundary.min.X / lineSpacing) - 1) * lineSpacing;
	int lineCount = (boundary.max.X - boundary.min.X + (lineSpacing - 1)) / lineSpacing;

	vector<vector<int64_t> > cutList;
	for (int n = 0; n < lineCount; n++)
		cutList.push_back(vector<int64_t>());


	for (unsigned int polyNr = 0; polyNr < outline.size(); polyNr++)
	{
		Point p1 = outline[polyNr][outline[polyNr].size() - 1];
		for (unsigned int i = 0; i < outline[polyNr].size(); i++)
		{
			Point p0 = outline[polyNr][i];
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
				cutList[idx].push_back(y);
			}
			p1 = p0;
		}
	}

	int idx = 0;
	for (int64_t x = boundary.min.X + lineSpacing / 2; x < boundary.max.X; x += lineSpacing)
	{
		qsort(cutList[idx].data(), cutList[idx].size(), sizeof(int64_t), compare_int64_t);
		for (unsigned int i = 0; i + 1 < cutList[idx].size(); i += 2)
		{
			if (cutList[idx][i + 1] - cutList[idx][i] < extrusionWidth / 5)
				continue;
			PolygonRef p = result.newPoly();
			p.add(matrix.unapply(Point(x, cutList[idx][i])));
			p.add(matrix.unapply(Point(x, cutList[idx][i + 1])));
		}
		idx += 1;
	}
}



}//namespace engine
