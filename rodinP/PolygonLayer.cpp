#include "stdafx.h"
#include "PolygonLayer.h"
#include "LayerDatasSet.h"
#include "settings.h"

#ifndef std_max
#define std_max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

//PolygonLayer
bool PolygonLayer::makeSegmentList(int minZ_, int maxZ_, std::vector<Mesh::Point> points_, int faceIdx)
{
	if (z < minZ_) return false;
	if (z > maxZ_) return false;

	LayerSegment s;
	if (points_[0][2] < z && points_[1][2] >= z && points_[2][2] >= z)
		s = project2D(points_[0], points_[2], points_[1], z);
	else if (points_[0][2] > z && points_[1][2] < z && points_[2][2] < z)
		s = project2D(points_[0], points_[1], points_[2], z);
	else if (points_[1][2] < z && points_[0][2] >= z && points_[2][2] >= z)
		s = project2D(points_[1], points_[0], points_[2], z);
	else if (points_[1][2] > z && points_[0][2] < z && points_[2][2] < z)
		s = project2D(points_[1], points_[2], points_[0], z);
	else if (points_[2][2] < z && points_[1][2] >= z && points_[0][2] >= z)
		s = project2D(points_[2], points_[1], points_[0], z);
	else if (points_[2][2] > z && points_[1][2] < z && points_[0][2] < z)
		s = project2D(points_[2], points_[0], points_[1], z);
	else
		return false;
	faceToSegmentIndex[faceIdx] = segmentList.size();
	s.faceIndex = faceIdx;
	s.addedToPolygon = false;
	segmentList.push_back(s);

	return true;
}
void PolygonLayer::makePolygons(Mesh * mesh_, bool keepNoneClosed, bool extensiveStitching, bool clearSegList /*= true*/)
{
	engine::Polygons openPolygonList;

	for (unsigned int startSegment = 0; startSegment < segmentList.size(); startSegment++)
	{
		if (segmentList[startSegment].addedToPolygon)
			continue;

		engine::Polygon poly;
		poly.add(segmentList[startSegment].start);

		unsigned int segmentIndex = startSegment;
		bool canClose;
		while (true)
		{
			canClose = false;
			segmentList[segmentIndex].addedToPolygon = true;
			Point p0 = segmentList[segmentIndex].end;
			poly.add(p0);
			int nextIndex = -1;

			Mesh::FaceHandle fh = mesh_->face_handle(segmentList[segmentIndex].faceIndex);
			Mesh::FaceHalfedgeIter fh_it = mesh_->fh_iter(fh);

			for (; fh_it.is_valid(); ++fh_it) {
				Mesh::FaceHandle ofh = mesh_->opposite_face_handle(fh_it);
				if (!ofh.is_valid())
					continue;
				if (faceToSegmentIndex.find(ofh.idx()) == faceToSegmentIndex.end())
					continue;

				Point p1 = segmentList[faceToSegmentIndex[ofh.idx()]].start;
				Point diff = p0 - p1;
				if (shorterThen(diff, MM2INT(0.01)))
				{
					if (faceToSegmentIndex[ofh.idx()] == static_cast<int>(startSegment))
						canClose = true;
					if (segmentList[faceToSegmentIndex[ofh.idx()]].addedToPolygon)
						continue;
					nextIndex = faceToSegmentIndex[ofh.idx()];
				}
			}
			if (nextIndex == -1)
				break;
			segmentIndex = nextIndex;
		}
		if (canClose)
			polygonList.add(poly);
		else
			openPolygonList.add(poly);
	}
	//Clear the segmentList to save memory, it is no longer needed after this point.

	if (clearSegList)
		segmentList.clear();


	//Connecting polygons that are not closed yet, as models are not always perfect manifold we need to join some stuff up to get proper polygons
	//First link up polygon ends that are within 2 microns.
	for (unsigned int i = 0; i < openPolygonList.size(); i++)
	{
		if (openPolygonList[i].size() < 1) continue;
		for (unsigned int j = 0; j < openPolygonList.size(); j++)
		{
			if (openPolygonList[j].size() < 1) continue;

			Point diff = openPolygonList[i][openPolygonList[i].size() - 1] - openPolygonList[j][0];
			int64_t distSquared = vSize2(diff);

			if (distSquared < MM2INT(0.02) * MM2INT(0.02))
			{
				if (i == j)
				{
					polygonList.add(openPolygonList[i]);
					openPolygonList[i].clear();
					break;
				}
				else
				{
					for (unsigned int n = 0; n < openPolygonList[j].size(); n++)
						openPolygonList[i].add(openPolygonList[j][n]);

					openPolygonList[j].clear();

					// 누락되는 경우가 생김 by 주성
				}
			}
		}
	}

	//Next link up all the missing ends, closing up the smallest gaps first. This is an inefficient implementation which can run in O(n*n*n) time.
	while (1)
	{
		int64_t bestScore = MM2INT(10.0) * MM2INT(10.0);
		unsigned int bestA = -1;
		unsigned int bestB = -1;
		bool reversed = false;
		for (unsigned int i = 0; i < openPolygonList.size(); i++)
		{
			if (openPolygonList[i].size() < 1) continue;
			for (unsigned int j = 0; j < openPolygonList.size(); j++)
			{
				if (openPolygonList[j].size() < 1) continue;

				Point diff = openPolygonList[i][openPolygonList[i].size() - 1] - openPolygonList[j][0];
				int64_t distSquared = vSize2(diff);
				if (distSquared < bestScore)
				{
					bestScore = distSquared;
					bestA = i;
					bestB = j;
					reversed = false;
				}

				if (i != j)
				{
					Point diff = openPolygonList[i][openPolygonList[i].size() - 1] - openPolygonList[j][openPolygonList[j].size() - 1];
					int64_t distSquared = vSize2(diff);
					if (distSquared < bestScore)
					{
						bestScore = distSquared;
						bestA = i;
						bestB = j;
						reversed = true;
					}
				}
			}
		}

		if (bestScore >= MM2INT(10.0) * MM2INT(10.0))
			break;

		if (bestA == bestB)
		{
			polygonList.add(openPolygonList[bestA]);
			openPolygonList[bestA].clear();
		}
		else {
			if (reversed)
			{
				if (openPolygonList[bestA].polygonLength() > openPolygonList[bestB].polygonLength())
				{
					for (unsigned int n = openPolygonList[bestB].size() - 1; int(n) >= 0; n--)
						openPolygonList[bestA].add(openPolygonList[bestB][n]);
					openPolygonList[bestB].clear();
				}
				else {
					for (unsigned int n = openPolygonList[bestA].size() - 1; int(n) >= 0; n--)
						openPolygonList[bestB].add(openPolygonList[bestA][n]);
					openPolygonList[bestA].clear();
				}
			}
			else {
				for (unsigned int n = 0; n < openPolygonList[bestB].size(); n++)
					openPolygonList[bestA].add(openPolygonList[bestB][n]);
				openPolygonList[bestB].clear();
			}
		}
	}

	if (extensiveStitching)
	{
		//For extensive stitching find 2 open polygons that are touching 2 closed polygons.
		// Then find the shortest path over this polygon that can be used to connect the open polygons,
		// And generate a path over this shortest bit to link up the 2 open polygons.
		// (If these 2 open polygons are the same polygon, then the final result is a closed polyon)

		while (1)
		{
			unsigned int bestA = -1;
			unsigned int bestB = -1;
			gapCloserPolygon bestResult;
			bestResult.len = POINT_MAX;
			bestResult.polygonIdx = -1;
			bestResult.pointIdxA = -1;
			bestResult.pointIdxB = -1;

			for (unsigned int i = 0; i < openPolygonList.size(); i++)
			{
				if (openPolygonList[i].size() < 1) continue;

				{
					gapCloserPolygon res = findPolygonGapCloser(openPolygonList[i][0], openPolygonList[i][openPolygonList[i].size() - 1]);
					if (res.len > 0 && res.len < bestResult.len)
					{
						bestA = i;
						bestB = i;
						bestResult = res;
					}
				}

				for (unsigned int j = 0; j < openPolygonList.size(); j++)
				{
					if (openPolygonList[j].size() < 1 || i == j) continue;

					gapCloserPolygon res = findPolygonGapCloser(openPolygonList[i][0], openPolygonList[j][openPolygonList[j].size() - 1]);
					if (res.len > 0 && res.len < bestResult.len)
					{
						bestA = i;
						bestB = j;
						bestResult = res;
					}
				}
			}

			if (bestResult.len < POINT_MAX)
			{
				if (bestA == bestB)
				{
					if (bestResult.pointIdxA == bestResult.pointIdxB)
					{
						polygonList.add(openPolygonList[bestA]);
						openPolygonList[bestA].clear();
					}
					else if (bestResult.AtoB)
					{
						engine::PolygonRef poly = polygonList.newPoly();
						for (unsigned int j = bestResult.pointIdxA; j != bestResult.pointIdxB; j = (j + 1) % polygonList[bestResult.polygonIdx].size())
							poly.add(polygonList[bestResult.polygonIdx][j]);
						for (unsigned int j = openPolygonList[bestA].size() - 1; int(j) >= 0; j--)
							poly.add(openPolygonList[bestA][j]);
						openPolygonList[bestA].clear();
					}
					else
					{
						unsigned int n = polygonList.size();
						polygonList.add(openPolygonList[bestA]);
						for (unsigned int j = bestResult.pointIdxB; j != bestResult.pointIdxA; j = (j + 1) % polygonList[bestResult.polygonIdx].size())
							polygonList[n].add(polygonList[bestResult.polygonIdx][j]);
						openPolygonList[bestA].clear();
					}
				}
				else
				{
					if (bestResult.pointIdxA == bestResult.pointIdxB)
					{
						for (unsigned int n = 0; n < openPolygonList[bestA].size(); n++)
							openPolygonList[bestB].add(openPolygonList[bestA][n]);
						openPolygonList[bestA].clear();
					}
					else if (bestResult.AtoB)
					{
						engine::Polygon poly;
						for (unsigned int n = bestResult.pointIdxA; n != bestResult.pointIdxB; n = (n + 1) % polygonList[bestResult.polygonIdx].size())
							poly.add(polygonList[bestResult.polygonIdx][n]);
						for (unsigned int n = poly.size() - 1; int(n) >= 0; n--)
							openPolygonList[bestB].add(poly[n]);
						for (unsigned int n = 0; n < openPolygonList[bestA].size(); n++)
							openPolygonList[bestB].add(openPolygonList[bestA][n]);
						openPolygonList[bestA].clear();
					}
					else
					{
						for (unsigned int n = bestResult.pointIdxB; n != bestResult.pointIdxA; n = (n + 1) % polygonList[bestResult.polygonIdx].size())
							openPolygonList[bestB].add(polygonList[bestResult.polygonIdx][n]);
						for (unsigned int n = openPolygonList[bestA].size() - 1; int(n) >= 0; n--)
							openPolygonList[bestB].add(openPolygonList[bestA][n]);
						openPolygonList[bestA].clear();
					}
				}
			}
			else
			{
				break;
			}
		}
	}

	if (keepNoneClosed)
	{
		for (unsigned int n = 0; n < openPolygonList.size(); n++)
		{
			if (openPolygonList[n].size() > 0)
				polygonList.add(openPolygonList[n]);
		}
	}
	for (unsigned int i = 0; i < openPolygonList.size(); i++)
	{
		if (openPolygonList[i].size() > 0)
			openPolygons.newPoly() = openPolygonList[i];
	}

	//Remove all the tiny polygons, or polygons that are not closed. As they do not contribute to the actual print.
	int snapDistance = MM2INT(1.0);
	for (unsigned int i = 0; i < polygonList.size(); i++)
	{
		int length = 0;

		for (unsigned int n = 1; n < polygonList[i].size(); n++)
		{
			length += vSize(polygonList[i][n] - polygonList[i][n - 1]);
			if (length > snapDistance)
				break;
		}
		if (length < snapDistance)
		{
			polygonList.remove(i);
			i--;
		}
	}

	//Finally optimize all the polygons. Every point removed saves time in the long run.
	optimizePolygons(polygonList);
}

void PolygonLayer::clearSegmentData()
{
	segmentList.clear();
	faceToSegmentIndex.clear();
}
void PolygonLayer::clearPolygonData()
{
	polygonList.clear();
	openPolygons.clear();
}

void PolygonLayer::createLayerWithParts(SliceLayer& storageLayer, int unionAllType)
{
	storageLayer.openLines = openPolygons;

	if (unionAllType & FIX_HORRIBLE_UNION_ALL_TYPE_B)
	{
		for (unsigned int i = 0; i < polygonList.size(); i++)
		{
			if (polygonList[i].orientation())
				polygonList[i].reverse();
		}
	}

	vector<engine::PolygonsPart> result;

	if (unionAllType & FIX_HORRIBLE_UNION_ALL_TYPE_C)
	{
		result = polygonList.offset(1000).splitIntoParts(unionAllType);
	}
	else
	{
		result = polygonList.splitIntoParts(unionAllType);
	}

	for (unsigned int i = 0; i < result.size(); i++)
	{
		storageLayer.parts.push_back(SliceLayerPart());

		if (unionAllType & FIX_HORRIBLE_UNION_ALL_TYPE_C)
		{
			storageLayer.parts[i].outline.add(result[i][0]);
			storageLayer.parts[i].outline = storageLayer.parts[i].outline.offset(-1000);
		}
		else
		{
			storageLayer.parts[i].outline = result[i];
		}

		storageLayer.parts[i].boundaryBox.calculate(storageLayer.parts[i].outline);
	}
}

gapCloserPolygon PolygonLayer::findPolygonGapCloser(Point ip0, Point ip1)
{
	gapCloserPolygon ret;
	closestPolygonResult c1 = findPolygonPointClosestTo(ip0);
	closestPolygonResult c2 = findPolygonPointClosestTo(ip1);
	if (c1.polygonIdx < 0 || c1.polygonIdx != c2.polygonIdx)
	{
		ret.len = -1;
		return ret;
	}
	ret.polygonIdx = c1.polygonIdx;
	ret.pointIdxA = c1.pointIdx;
	ret.pointIdxB = c2.pointIdx;
	ret.AtoB = true;

	if (ret.pointIdxA == ret.pointIdxB)
	{
		//Connection points are on the same line segment.
		ret.len = vSize(ip0 - ip1);
	}
	else {
		//Find out if we have should go from A to B or the other way around.
		Point p0 = polygonList[ret.polygonIdx][ret.pointIdxA];
		int64_t lenA = vSize(p0 - ip0);
		for (unsigned int i = ret.pointIdxA; i != ret.pointIdxB; i = (i + 1) % polygonList[ret.polygonIdx].size())
		{
			Point p1 = polygonList[ret.polygonIdx][i];
			lenA += vSize(p0 - p1);
			p0 = p1;
		}
		lenA += vSize(p0 - ip1);

		p0 = polygonList[ret.polygonIdx][ret.pointIdxB];
		int64_t lenB = vSize(p0 - ip1);
		for (unsigned int i = ret.pointIdxB; i != ret.pointIdxA; i = (i + 1) % polygonList[ret.polygonIdx].size())
		{
			Point p1 = polygonList[ret.polygonIdx][i];
			lenB += vSize(p0 - p1);
			p0 = p1;
		}
		lenB += vSize(p0 - ip0);

		if (lenA < lenB)
		{
			ret.AtoB = true;
			ret.len = lenA;
		}
		else {
			ret.AtoB = false;
			ret.len = lenB;
		}
	}
	return ret;
}

closestPolygonResult PolygonLayer::findPolygonPointClosestTo(Point input)
{
	closestPolygonResult ret;
	for (unsigned int n = 0; n < polygonList.size(); n++)
	{
		Point p0 = polygonList[n][polygonList[n].size() - 1];
		for (unsigned int i = 0; i < polygonList[n].size(); i++)
		{
			Point p1 = polygonList[n][i];

			//Q = A + Normal( B - A ) * ((( B - A ) dot ( P - A )) / VSize( A - B ));
			Point pDiff = p1 - p0;
			int64_t lineLength = vSize(pDiff);
			if (lineLength > 1)
			{
				int64_t distOnLine = dot(pDiff, input - p0) / lineLength;
				if (distOnLine >= 0 && distOnLine <= lineLength)
				{
					Point q = p0 + pDiff * distOnLine / lineLength;
					if (shorterThen(q - input, 100))
					{
						ret.intersectionPoint = q;
						ret.polygonIdx = n;
						ret.pointIdx = i;
						return ret;
					}
				}
			}
			p0 = p1;
		}
	}
	ret.polygonIdx = -1;
	return ret;
}

LayerSegment PolygonLayer::project2D(Mesh::Point& p0, Mesh::Point& p1, Mesh::Point& p2, int32_t z) const
{
	LayerSegment seg;
	seg.start.X = int64_t(p0[0] + (p1[0] - p0[0]) * (z - p0[2]) / (p1[2] - p0[2]));
	seg.start.Y = int64_t(p0[1] + (p1[1] - p0[1]) * (z - p0[2]) / (p1[2] - p0[2]));
	seg.end.X = int64_t(p0[0] + (p2[0] - p0[0]) * (z - p0[2]) / (p2[2] - p0[2]));
	seg.end.Y = int64_t(p0[1] + (p2[1] - p0[1]) * (z - p0[2]) / (p2[2] - p0[2]));
	return seg;
}



