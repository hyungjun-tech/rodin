/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#include "stdafx.h"
#include <map>

#include "pathOrderOptimizer.h"

namespace engine {

static uint32_t hashPoint(const Point& p)
{
    return (p.X / 20000) ^ (p.Y / 20000) << 8;
}

void PathOrderOptimizer::optimize()
{
    const float incommingPerpundicularNormalScale = 0.0001f;
    
    std::map<uint32_t, std::vector<unsigned int>> location_to_polygon_map;
    std::vector<bool> picked;
    for(unsigned int i=0;i<polygons.size(); i++)
    {
        int best = -1;
        float bestDist = 0xFFFFFFFFFFFFFFFF;
        PolygonRef poly = polygons[i];
        for(unsigned int j=0; j<poly.size(); j++)
        {
            float dist = vSize2f(poly[j] - startPoint);
            if (dist < bestDist)
            {
                best = j;
                bestDist = dist;
            }
        }
        polyStart.push_back(best);
        picked.push_back(false);
        
        if (poly.size() == 2)
        {
            location_to_polygon_map[hashPoint(poly[0])].push_back(i);
            location_to_polygon_map[hashPoint(poly[1])].push_back(i);
        }
    }

    Point incommingPerpundicularNormal(0, 0);
    Point p0 = startPoint;
    for(unsigned int n=0; n<polygons.size(); n++)
    {
        int best = -1;
        float bestDist = 0xFFFFFFFFFFFFFFFF;
        
        for(unsigned int i = 0; i < location_to_polygon_map[hashPoint(p0)].size(); ++i)
        {
			int ii = location_to_polygon_map[hashPoint(p0)][i];
            if (picked[ii] || polygons[ii].size() < 1)
                continue;

            float dist = vSize2f(polygons[ii][0] - p0);
            dist += abs(dot(incommingPerpundicularNormal, normal(polygons[ii][1] - polygons[ii][0], 1000))) * incommingPerpundicularNormalScale;
            if (dist < bestDist)
            {
                best = ii;
                bestDist = dist;
                polyStart[ii] = 0;
            }
            dist = vSize2f(polygons[ii][1] - p0);
            dist += abs(dot(incommingPerpundicularNormal, normal(polygons[ii][0] - polygons[ii][1], 1000))) * incommingPerpundicularNormalScale;
            if (dist < bestDist)
            {
                best = ii;
                bestDist = dist;
                polyStart[ii] = 1;
            }
        }
        
        if (best == -1)
        {
            for(unsigned int i=0;i<polygons.size(); i++)
            {
                if (picked[i] || polygons[i].size() < 1)
                    continue;
                if (polygons[i].size() == 2)
                {
                    float dist = vSize2f(polygons[i][0] - p0);
                    dist += abs(dot(incommingPerpundicularNormal, normal(polygons[i][1] - polygons[i][0], 1000))) * incommingPerpundicularNormalScale;
                    if (dist < bestDist)
                    {
                        best = i;
                        bestDist = dist;
                        polyStart[i] = 0;
                    }
                    dist = vSize2f(polygons[i][1] - p0);
					dist += abs(dot(incommingPerpundicularNormal, normal(polygons[i][0] - polygons[i][1], 1000))) * incommingPerpundicularNormalScale;
                    if (dist < bestDist)
                    {
                        best = i;
                        bestDist = dist;
                        polyStart[i] = 1;
                    }
                }
				else
				{
                    float dist = vSize2f(polygons[i][polyStart[i]] - p0);
                    if (dist < bestDist)
                    {
                        best = i;
                        bestDist = dist;
                    }
                }
            }
        }
        
        if (best > -1)
        {
            if (polygons[best].size() == 2)
            {
                int endIdx = (polyStart[best] + 1) % 2;
                p0 = polygons[best][endIdx];
                incommingPerpundicularNormal = crossZ(normal(polygons[best][endIdx] - polygons[best][polyStart[best]], 1000));
            }
			else
			{
                p0 = polygons[best][polyStart[best]];
                incommingPerpundicularNormal = Point(0, 0);
            }
            picked[best] = true;
            polyOrder.push_back(best);
        }
    }
    
    p0 = startPoint;
    for(int nr = 0; nr < polyOrder.size(); ++nr)
    {
        PolygonRef poly = polygons[polyOrder[nr]];
        if (poly.size() > 2)
        {
            int best = -1;
            float bestDist = 0xFFFFFFFFFFFFFFFF;
            bool orientation = poly.orientation();
            for(unsigned int i=0;i<poly.size(); i++)
            {
                const int64_t dot_score_scale = 2000;
                float dist = vSize2f(polygons[polyOrder[nr]][i] - p0);
				Point n0 = normal(poly[(i + poly.size() - 1) % poly.size()] - poly[i], dot_score_scale);	
				Point n1 = normal(poly[i] - poly[(i + 1) % poly.size()], dot_score_scale);
                float dot_score = dot(n0, n1) - dot(crossZ(n0), n1);	// �𼭸��� ���� ����
            
				if (orientation)
                    dot_score = -dot_score;
                
				dist += dot_score;
                
				if (dist < bestDist)
                {
                    best = i;
                    bestDist = dist;
                }
            }
            polyStart[polyOrder[nr]] = best;
        }
        if (poly.size() <= 2)
            p0 = poly[(polyStart[polyOrder[nr]] + 1) % 2];
        else
            p0 = poly[polyStart[polyOrder[nr]]];
    }
}

void PathOrderOptimizer::optimize_overmoving(int overmoving)
{
	const float incommingPerpundicularNormalScale = 0.0001f;

	double m_pi = acos(-1.0);

	std::vector<bool> picked;
	for (unsigned int i = 0; i < polygons.size(); i++)
	{
		int best = -1;
		float bestCost = 0xFFFFFFFFFFFFFFFF;
		PolygonRef poly = polygons[i];
		if (pathOptimizationParameter[0] == 0 && pathOptimizationParameter[1] == 0 && pathOptimizationParameter[2] == 0)
		{
			for (unsigned int j = 0; j < poly.size(); j++)
			{
				float dist = vSize2f(poly[j] - startPoint);
				if (dist < bestCost)
				{
					best = j;
					bestCost = dist;
				}
			}
		}
		else
		{
			for (unsigned int j = 0; j < poly.size(); j++)
			{
				Point v0 = startPoint - prevPoint;
				Point v1 = poly[j] - startPoint;
				Point v2 = poly[(j + 1) % poly.size()] - poly[j];
				double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
				double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
				double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

				double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
				double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
				double dist = len1*0.001 / 210.0;

				if (len0 == 0)
					theta01 = 0;

				if (len1 == 0)
				{
					theta01 = 0;
					theta12 = 0;
				}
				float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;

				if (cost < bestCost)
				{
					best = j;
					bestCost = cost;
				}
			}
		}
		polyStart.push_back(best);
		picked.push_back(false);
	}

	Point incommingPerpundicularNormal(0, 0);
	Point p0 = startPoint;
	for (unsigned int n = 0; n < polygons.size(); n++)
	{
		int best = -1;
		float bestDist = 0xFFFFFFFFFFFFFFFF;
	
		for (unsigned int i = 0; i < polygons.size(); i++)
		{
			if (picked[i] || polygons[i].size() < 1)
				continue;
			
			float dist = vSize(polygons[i][polyStart[i]] - p0);
			if (dist < bestDist)
			{
				best = i;
				bestDist = dist;
			}
		}

		if (best > -1)
		{
			Point pre_temp;
			p0 = getOvermovingPoint(polygons[best], polyStart[best], overmoving, pre_temp);	// calculate overmoving
			incommingPerpundicularNormal = Point(0, 0);

			picked[best] = true;
			polyOrder.push_back(best);
		}
	}

	p0 = startPoint;
	Point pre = prevPoint;
	for (int nr = 0; nr < polyOrder.size(); ++nr)
	{
		PolygonRef poly = polygons[polyOrder[nr]];
		if (poly.size() > 2)
		{
			int best = -1;
			float bestDist = 0xFFFFFFFFFFFFFFFF;
			bool orientation = poly.orientation();
			for (unsigned int i = 0; i < poly.size(); i++)
			{
				if (pathOptimizationParameter[0] == 0 && pathOptimizationParameter[1] == 0 && pathOptimizationParameter[2] == 0)
				{
					const int64_t dot_score_scale = 2000;
					float dist = vSize2f(polygons[polyOrder[nr]][i] - p0);
					Point n0 = normal(poly[(i + poly.size() - 1) % poly.size()] - poly[i], dot_score_scale);
					Point n1 = normal(poly[i] - poly[(i + 1) % poly.size()], dot_score_scale);
					float dot_score = dot(n0, n1) - dot(crossZ(n0), n1);	// �𼭸��� ���� ����
					if (orientation)
						dot_score = -dot_score;
					dist += dot_score;
					if (dist < bestDist)
					{
						best = i;
						bestDist = dist;
					}
				}
				else
				{
					Point v0 = p0 - pre;
					Point v1 = poly[i] - p0;
					Point v2 = poly[(i + 1) % poly.size()] - poly[i];
					double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
					double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
					double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

					double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
					double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
					double dist = len1*0.001 / 210.0;

					if (len0 == 0)
						theta01 = 0;

					if (len1 == 0)
					{
						theta01 = 0;
						theta12 = 0;
					}

					float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;

					if (cost < bestDist)
					{
						best = i;
						bestDist = cost;
					}
				}
			}
			polyStart[polyOrder[nr]] = best;
		}

		p0 = getOvermovingPoint(poly, polyStart[polyOrder[nr]], overmoving, pre); // calculate overmoving
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//inset -> infill optimization + weight value parameter
//optimization adapted on infill start point 
void PathOrderOptimizer::optimize_insetToinfill()
{
	const float incommingPerpundicularNormalScale = 0.0001f;
	const double m_pi = acos(-1.0);

	std::map<uint32_t, std::vector<unsigned int>> location_to_polygon_map;
	std::vector<bool> picked;

	for (unsigned int i = 0; i<polygons.size(); i++)
	{
		int best = -1;
		float bestDist = 0xFFFFFFFFFFFFFFFF;
		PolygonRef poly = polygons[i];

		if (pathOptimizationParameter[0] == 0 && pathOptimizationParameter[1] == 0 && pathOptimizationParameter[2] == 0)
		{
			for (unsigned int j = 0; j<poly.size(); j++)
			{
				float dist = vSize2f(poly[j] - startPoint);
				if (dist < bestDist)
				{
					best = j;
					bestDist = dist;
				}
			}

		}
		else
		{
			for (unsigned int j = 0; j < poly.size(); j++)
			{
				Point v0 = startPoint - prevPoint;
				Point v1 = poly[j] - startPoint;
				Point v2 = poly[(j + 1) % poly.size()] - poly[j];
				double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
				double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
				double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

				double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
				double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
				double dist = len1*0.001 / 210.0;

				if (len0 == 0)
					theta01 = 0;

				if (len1 == 0)
				{
					theta01 = 0;
					theta12 = 0;
				}
				float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;

				//cost�� dist���� dimensiton���̴� ���?? �񱳰� �����Ѱ�?
				if (cost < bestDist)
				{
					best = j;
					bestDist = cost;
				}
			}
		}

		polyStart.push_back(best);
		picked.push_back(false);

		if (poly.size() == 2)
		{
			location_to_polygon_map[hashPoint(poly[0])].push_back(i);
			location_to_polygon_map[hashPoint(poly[1])].push_back(i);
		}
	}

	Point incommingPerpundicularNormal(0, 0);
	Point p0 = startPoint;

	for (unsigned int n = 0; n<polygons.size(); n++)
	{
		int best = -1;
		float bestDist = 0xFFFFFFFFFFFFFFFF;

		//���� hashpoint�ȿ��� best point�ĺ� ã��..
		for (unsigned int i = 0; i < location_to_polygon_map[hashPoint(p0)].size(); ++i)
		{
			int ii = location_to_polygon_map[hashPoint(p0)][i];

			if (picked[ii] || polygons[ii].size() < 1)
				continue;

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//vector error code//
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			PolygonRef poly_temp = polygons[ii];

			//n==0�� ��.. inset���� infill�� ���� ���� ���� ���� ������ �Ÿ� ����� �ƴ϶� parameter����� �̿���.
			if (n == 0)
			{
				for (unsigned int j = 0; j < poly_temp.size(); j++)
				{
					Point v0 = startPoint - prevPoint;
					Point v1 = poly_temp[j] - startPoint;
					Point v2 = poly_temp[(j + 1) % poly_temp.size()] - poly_temp[j];
					double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
					double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
					double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

					double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
					double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
					double dist = len1*0.001 / 210.0;

					if (len0 == 0)
						theta01 = 0;

					if (len1 == 0)
					{
						theta01 = 0;
						theta12 = 0;
					}
					float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;

					if (cost < bestDist)
					{
						best = ii;
						bestDist = cost;
						polyStart[ii] = j;
					}
				}
			}
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			else
			{
				float dist = vSize2f(polygons[ii][0] - p0);
				dist += abs(dot(incommingPerpundicularNormal, normal(polygons[ii][1] - polygons[ii][0], 1000))) * incommingPerpundicularNormalScale;
				if (dist < bestDist)
				{
					best = ii;
					bestDist = dist;
					polyStart[ii] = 0;
				}
				dist = vSize2f(polygons[ii][1] - p0);
				dist += abs(dot(incommingPerpundicularNormal, normal(polygons[ii][0] - polygons[ii][1], 1000))) * incommingPerpundicularNormalScale;
				if (dist < bestDist)
				{
					best = ii;
					bestDist = dist;
					polyStart[ii] = 1;
				}
			}
		}

		//������ ������ �ĺ��� ã�� ���� ���.. ��ü�� polygons�� ���ؼ� Ž���� ��.
		if (best == -1)
		{
			for (unsigned int i = 0; i<polygons.size(); i++)
			{
				if (picked[i] || polygons[i].size() < 1)
					continue;

				PolygonRef poly = polygons[i];

				if (polygons[i].size() == 2)
				{
					if (n == 0)
					{
						for (unsigned int j = 0; j < poly.size(); j++)
						{
							Point v0 = startPoint - prevPoint;
							Point v1 = poly[j] - startPoint;
							Point v2 = poly[(j + 1) % poly.size()] - poly[j];
							double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
							double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
							double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

							double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
							double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
							double dist = len1*0.001 / 210.0;

							if (len0 == 0)
								theta01 = 0;

							if (len1 == 0)
							{
								theta01 = 0;
								theta12 = 0;
							}
							float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;

							if (cost < bestDist)
							{
								best = j;
								bestDist = cost;
							}
						}
					}
					else
					{
						float dist = vSize2f(polygons[i][0] - p0);
						dist += abs(dot(incommingPerpundicularNormal, normal(polygons[i][1] - polygons[i][0], 1000))) * incommingPerpundicularNormalScale;
						if (dist < bestDist)
						{
							best = i;
							bestDist = dist;
							polyStart[i] = 0;
						}
						dist = vSize2f(polygons[i][1] - p0);
						dist += abs(dot(incommingPerpundicularNormal, normal(polygons[i][0] - polygons[i][1], 1000))) * incommingPerpundicularNormalScale;
						if (dist < bestDist)
						{
							best = i;
							bestDist = dist;
							polyStart[i] = 1;
						}
					}


				}
				else
				{
					if (n == 0)
					{
						for (unsigned int j = 0; j < poly.size(); j++)
						{
							Point v0 = startPoint - prevPoint;
							Point v1 = poly[j] - startPoint;
							Point v2 = poly[(j + 1) % poly.size()] - poly[j];
							double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
							double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
							double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

							double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
							double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
							double dist = len1*0.001 / 210.0;

							if (len0 == 0)
								theta01 = 0;

							if (len1 == 0)
							{
								theta01 = 0;
								theta12 = 0;
							}
							float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;

							if (cost < bestDist)
							{
								best = j;
								bestDist = cost;
							}
						}
					}
					else
					{

						float dist = vSize2f(polygons[i][polyStart[i]] - p0);
						if (dist < bestDist)
						{
							best = i;
							bestDist = dist;
						}
					}
				}
			}
		}

		if (best > -1)
		{
			if (polygons[best].size() == 2)
			{
				int endIdx = (polyStart[best] + 1) % 2;
				p0 = polygons[best][endIdx];
				incommingPerpundicularNormal = crossZ(normal(polygons[best][endIdx] - polygons[best][polyStart[best]], 1000));
			}
			else
			{
				p0 = polygons[best][polyStart[best]];
				incommingPerpundicularNormal = Point(0, 0);
			}
			picked[best] = true;
			polyOrder.push_back(best);
		}
	}

	p0 = startPoint;
	Point pre = prevPoint;

	for (int nr = 0; nr < polyOrder.size(); ++nr)
	{
		PolygonRef poly = polygons[polyOrder[nr]];
		if (poly.size() > 2)
		{
			int best = -1;
			float bestDist = 0xFFFFFFFFFFFFFFFF;
			bool orientation = poly.orientation();

			for (unsigned int i = 0; i < poly.size(); i++)
			{
				if (pathOptimizationParameter[0] == 0 && pathOptimizationParameter[1] == 0 && pathOptimizationParameter[2] == 0)
				{
					const int64_t dot_score_scale = 2000;
					float dist = vSize2f(polygons[polyOrder[nr]][i] - p0);
					Point n0 = normal(poly[(i + poly.size() - 1) % poly.size()] - poly[i], dot_score_scale);
					Point n1 = normal(poly[i] - poly[(i + 1) % poly.size()], dot_score_scale);
					float dot_score = dot(n0, n1) - dot(crossZ(n0), n1);	// �𼭸��� ���� ����
					if (orientation)
						dot_score = -dot_score;
					dist += dot_score;
					if (dist < bestDist)
					{
						best = i;
						bestDist = dist;
					}
				}
				else
				{
					Point v0 = p0 - pre;
					Point v1 = poly[i] - p0;
					Point v2 = poly[(i + 1) % poly.size()] - poly[i];
					double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
					double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
					double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

					double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
					double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
					double dist = len1*0.001 / 210.0;

					if (len0 == 0)
						theta01 = 0;

					if (len1 == 0)
					{
						theta01 = 0;
						theta12 = 0;
					}

					float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;

					if (cost < bestDist)
					{
						best = i;
						bestDist = cost;
					}
				}
			}
			polyStart[polyOrder[nr]] = best;
		}

		if (poly.size() <= 2)
			p0 = poly[(polyStart[polyOrder[nr]] + 1) % 2];
		else
			p0 = poly[polyStart[polyOrder[nr]]];
	}
}

void PathOrderOptimizer::optimize_overmoving_filteringTheta(int overmoving, int filteringTheta)
{
	const float incommingPerpundicularNormalScale = 0.0001f;
	const double m_pi = acos(-1.0);

	std::vector<bool> picked;
	for (unsigned int i = 0; i < polygons.size(); i++)
	{
		int best = -1;
		float bestCost = 0xFFFFFFFFFFFFFFFF;
		PolygonRef poly = polygons[i];
		//if (pathOptimizationParameter[0] == 0 && pathOptimizationParameter[1] == 0 && pathOptimizationParameter[2] == 0)
		if ((direct.X == 0 && direct.Y == 0) || (pathOptimizationParameter[0] == 0 && pathOptimizationParameter[1] == 0 && pathOptimizationParameter[2] == 0))
		{
			for (unsigned int j = 0; j < poly.size(); j++)
			{
				float dist = vSize2f(poly[j] - startPoint);
				if (dist < bestCost)
				{
					best = j;
					bestCost = dist;
				}
			}
		}
		else
		{
			std::vector<float> cost_vec;
			bool isAllSkipped = true;

			for (unsigned int j = 0; j < poly.size(); j++)
			{
				Point v0 = startPoint - prevPoint;
				Point v1 = poly[j] - startPoint;
				Point v2 = poly[(j + 1) % poly.size()] - poly[j];

				double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
				double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
				double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

				double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
				double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
				double dist = len1*0.001 / 210.0;

				if (len0 == 0)
					theta01 = 0;

				if (len1 == 0)
				{
					theta01 = 0;
					theta12 = 0;
				}

				float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;
				cost_vec.push_back(cost);

				if ((theta01 * 180) < filteringTheta)
				{
					if (cost < bestCost)
					{
						best = j;
						bestCost = cost;
					}

					isAllSkipped = false;
				}

				//if (cost < bestDist)
				//{
				//	best = i;
				//	bestDist = cost;
				//}

			}
			//��� over�� �Ǹ�..(isAllFiltered == ture) cost_vec���� ã�ƾ� ��.
			if (isAllSkipped)
			{
				int minIndex = std::min_element(cost_vec.begin(), cost_vec.end()) - cost_vec.begin();

				best = minIndex;
				bestCost = cost_vec[minIndex];
			}

			cost_vec.clear();

		}
		polyStart.push_back(best);
		picked.push_back(false);
	}

	Point incommingPerpundicularNormal(0, 0);
	Point p0 = startPoint;
	for (unsigned int n = 0; n < polygons.size(); n++)
	{
		int best = -1;
		float bestDist = 0xFFFFFFFFFFFFFFFF;

		for (unsigned int i = 0; i < polygons.size(); i++)
		{
			if (picked[i] || polygons[i].size() < 1)
				continue;

			float dist = vSize(polygons[i][polyStart[i]] - p0);
			if (dist < bestDist)
			{
				best = i;
				bestDist = dist;
			}
		}

		if (best > -1)
		{
			Point pre_temp;
			p0 = getOvermovingPoint(polygons[best], polyStart[best], overmoving, pre_temp);	// calculate overmoving
			incommingPerpundicularNormal = Point(0, 0);

			picked[best] = true;
			polyOrder.push_back(best);
		}
	}

	p0 = startPoint;
	Point pre = prevPoint;
	for (int nr = 0; nr < polyOrder.size(); ++nr)
	{
		PolygonRef poly = polygons[polyOrder[nr]];
		if (poly.size() > 2)
		{
			int best = -1;
			float bestDist = 0xFFFFFFFFFFFFFFFF;
			bool orientation = poly.orientation();

			std::vector<float> cost_vec;
			bool isAllSkipped = true;
			bool isOptimizedParameter = false;

			for (unsigned int i = 0; i < poly.size(); i++)
			{
				if (pathOptimizationParameter[0] == 0 && pathOptimizationParameter[1] == 0 && pathOptimizationParameter[2] == 0)
				{
					const int64_t dot_score_scale = 2000;
					float dist = vSize2f(polygons[polyOrder[nr]][i] - p0);
					Point n0 = normal(poly[(i + poly.size() - 1) % poly.size()] - poly[i], dot_score_scale);
					Point n1 = normal(poly[i] - poly[(i + 1) % poly.size()], dot_score_scale);
					float dot_score = dot(n0, n1) - dot(crossZ(n0), n1);	// �𼭸��� ���� ����
					if (orientation)
						dot_score = -dot_score;
					dist += dot_score;
					if (dist < bestDist)
					{
						best = i;
						bestDist = dist;
					}

					isAllSkipped = false;
				}
				else
				{
					isOptimizedParameter = true;

					Point v0 = p0 - pre;
					Point v1 = poly[i] - p0;
					Point v2 = poly[(i + 1) % poly.size()] - poly[i];
					double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
					double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
					double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

					double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
					double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
					double dist = len1*0.001 / 210.0;

					if (len0 == 0)
						theta01 = 0;

					if (len1 == 0)
					{
						theta01 = 0;
						theta12 = 0;
					}

					float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;
					cost_vec.push_back(cost);


					if (theta01 * 180 < filteringTheta)
					{
						if (cost < bestDist)
						{
							best = i;
							bestDist = cost;
						}

						isAllSkipped = false;
					}

					//if (cost < bestDist)
					//{
					//	best = i;
					//	bestDist = cost;
					//}
				}
			}

			//��� over�� �Ǹ�..(isAllFiltered == ture) cost_vec���� ã�ƾ� ��.
			if (isAllSkipped && isOptimizedParameter)
			{
				int minIndex = std::min_element(cost_vec.begin(), cost_vec.end()) - cost_vec.begin();

				best = minIndex;
				bestDist = cost_vec[minIndex];

			}

			polyStart[polyOrder[nr]] = best;

			cost_vec.clear();
		}

		p0 = getOvermovingPoint(poly, polyStart[polyOrder[nr]], overmoving, pre); // calculate overmoving
	}
}

void PathOrderOptimizer::optimize_insetToinfill_filteringTheta(int filteringTheta)
{
	const float incommingPerpundicularNormalScale = 0.0001f;
	const double m_pi = acos(-1.0);

	std::vector<float> cost_vec;

	std::map<uint32_t, std::vector<unsigned int>> location_to_polygon_map;
	std::vector<bool> picked;

	// line�鸸 ���� ��쿣 ���ʿ��� ������ ����	by JS
	for (unsigned int i = 0; i<polygons.size(); i++)
	{
		polyStart.push_back(0);
		picked.push_back(false);

		location_to_polygon_map[hashPoint(polygons[i][0])].push_back(i);
		location_to_polygon_map[hashPoint(polygons[i][1])].push_back(i);
	}

	Point incommingPerpundicularNormal(0, 0);
	Point p0 = startPoint;

	for (unsigned int n = 0; n<polygons.size(); n++)
	{
		int best = -1;
		float bestDist = 0xFFFFFFFFFFFFFFFF;

		bool isfilteringTheta = false;

		//���� hashpoint�ȿ��� best point�ĺ� ã��..
		// hash�� ����� ������ �������� ã�°ǵ�.. ������ ���� ��쿣 �̷��� �ϸ� �ȵ�.	by JS
		if (n != 0 || (pathOptimizationParameter[0] == 0 && pathOptimizationParameter[1] == 0 && pathOptimizationParameter[2] == 0))
		{
			for (unsigned int i = 0; i < location_to_polygon_map[hashPoint(p0)].size(); ++i)
			{
				int ii = location_to_polygon_map[hashPoint(p0)][i];
				if (picked[ii] || polygons[ii].size() < 1)
					continue;

				PolygonRef poly_temp = polygons[ii];

				float dist = vSize2f(polygons[ii][0] - p0);
				dist += abs(dot(incommingPerpundicularNormal, normal(polygons[ii][1] - polygons[ii][0], 1000))) * incommingPerpundicularNormalScale;
				if (dist < bestDist)
				{
					best = ii;
					bestDist = dist;
					polyStart[ii] = 0;
				}
				dist = vSize2f(polygons[ii][1] - p0);
				dist += abs(dot(incommingPerpundicularNormal, normal(polygons[ii][0] - polygons[ii][1], 1000))) * incommingPerpundicularNormalScale;
				if (dist < bestDist)
				{
					best = ii;
					bestDist = dist;
					polyStart[ii] = 1;
				}
			}
		}

		//������ ������ �ĺ��� ã�� ���� ���.. ��ü�� polygons�� ���ؼ� Ž���� ��.
		if (best == -1)
		{
			for (unsigned int i = 0; i < polygons.size(); i++)
			{
				if (picked[i] || polygons[i].size() < 1)
					continue;

				PolygonRef poly = polygons[i];

				if (n == 0 && !(pathOptimizationParameter[0] == 0 && pathOptimizationParameter[1] == 0 && pathOptimizationParameter[2] == 0))
				{
					bool isAllFiltered = true;

					for (unsigned int j = 0; j < poly.size(); j++)
					{
						Point v0 = startPoint - prevPoint;
						Point v1 = poly[j] - startPoint;
						Point v2 = poly[(j + 1) % poly.size()] - poly[j];
						double len0 = sqrt(v0.X*v0.X + v0.Y*v0.Y);
						double len1 = sqrt(v1.X*v1.X + v1.Y*v1.Y);
						double len2 = sqrt(v2.X*v2.X + v2.Y*v2.Y);

						double theta01 = acos(((double)v0.X*(double)v1.X + (double)v0.Y*(double)v1.Y) / (len0*len1)) / m_pi;
						double theta12 = acos(((double)v1.X*(double)v2.X + (double)v1.Y*(double)v2.Y) / (len1*len2)) / m_pi;
						double dist = len1*0.001 / 210.0;

						if (len0 == 0)
							theta01 = 0;

						if (len1 == 0)
						{
							theta01 = 0;
							theta12 = 0;
						}

						float cost = pathOptimizationParameter[0] * theta01 + pathOptimizationParameter[1] * theta12 + pathOptimizationParameter[2] * dist;
						cost_vec.push_back(cost);

						if ((theta01 * 180) < filteringTheta)
						{
							if (cost < bestDist || !isfilteringTheta)
							{
								best = i;
								bestDist = cost;
								polyStart[i] = j;
								isfilteringTheta = true;
								isAllFiltered = false;
							}
						}
					}

					if (isAllFiltered && !isfilteringTheta)		// ���͸��� ����� �ϱ� ���ؼ��� �̷��� ������ �߰� �Ǿ�� ��
					{
						int minIndex = std::min_element(cost_vec.begin(), cost_vec.end()) - cost_vec.begin();

						if (cost_vec[minIndex] < bestDist)
						{
							best = i;
							bestDist = cost_vec[minIndex];
							polyStart[i] = minIndex;	// �̷� �κп� �ݺ����� ������ �־���. by JS
						}
					}

					cost_vec.clear();
				}
				else
				{
					float dist = vSize2f(polygons[i][0] - p0);
					dist += abs(dot(incommingPerpundicularNormal, normal(polygons[i][1] - polygons[i][0], 1000))) * incommingPerpundicularNormalScale;
					if (dist < bestDist)
					{
						best = i;
						bestDist = dist;
						polyStart[i] = 0;
					}
					dist = vSize2f(polygons[i][1] - p0);
					dist += abs(dot(incommingPerpundicularNormal, normal(polygons[i][0] - polygons[i][1], 1000))) * incommingPerpundicularNormalScale;
					if (dist < bestDist)
					{
						best = i;
						bestDist = dist;
						polyStart[i] = 1;
					}
				}
			}
		}

		if (best > -1)
		{
			if (polygons[best].size() == 2)
			{
				int endIdx = (polyStart[best] + 1) % 2;
				p0 = polygons[best][endIdx];
				incommingPerpundicularNormal = crossZ(normal(polygons[best][endIdx] - polygons[best][polyStart[best]], 1000));
			}
			else
			{
				p0 = polygons[best][polyStart[best]];
				incommingPerpundicularNormal = Point(0, 0);
			}
			picked[best] = true;
			polyOrder.push_back(best);
		}
	}

	// �ؿ� line�鿡���� ���ʿ��� ���� ����. by JS
}

Point PathOrderOptimizer::getOvermovingPoint(PolygonRef polygon, int start, int overmoving, Point& pre)
{
	if (overmoving == 0)
	{
		pre = polygon[(start + polygon.size() - 1) % polygon.size()];
		return polygon[start];
	}

	float total_dist = 0;
	Point pt;
	int iter = 0;
	while (1)
	{
		float dist = vSize(polygon[(start + iter + 1) % polygon.size()] - polygon[(start + iter) % polygon.size()]);
		if (total_dist + dist >= overmoving)
		{
			pre = polygon[(start + iter) % polygon.size()];
			float a = (overmoving - total_dist) / dist;
			pt.X = (1 - a)*polygon[(start + iter) % polygon.size()].X + a*polygon[(start + iter + 1) % polygon.size()].X;
			pt.Y = (1 - a)*polygon[(start + iter) % polygon.size()].Y + a*polygon[(start + iter + 1) % polygon.size()].Y;
			break;
		}
		total_dist += dist;
		iter++;
	}
	return pt;
}



}//namespace engine
