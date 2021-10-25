/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#pragma once

#include <stdint.h>
#include "polygon.h"

namespace engine {

class PathOrderOptimizer
{
public:
    Point startPoint;
	Point direct;

    vector<PolygonRef> polygons;
    vector<int> polyStart;
    vector<int> polyOrder;

	int pathOptimizationParameter[3];		// add jusung
	Point prevPoint;						// add jusung

    PathOrderOptimizer(Point startPoint)
    {
        this->startPoint = startPoint;
		this->direct = Point(0, 0);
    }

	PathOrderOptimizer(Point startPoint, Point dir)
	{
		this->startPoint = startPoint;
		this->direct = dir;
	}

    void addPolygon(PolygonRef polygon)
    {
        this->polygons.push_back(polygon);
    }
    
    void addPolygons(Polygons& polygons)
    {
        for(unsigned int i=0;i<polygons.size(); i++)
            this->polygons.push_back(polygons[i]);
    }
    
    void optimize();
	void optimize_overmoving(int overmoving);
	void optimize_overmoving_filteringTheta(int overmoving, int filteringTheta);
	void optimize_insetToinfill();
	void optimize_insetToinfill_filteringTheta(int filteringTheta);

	Point getOvermovingPoint(PolygonRef polygon, int start, int overmovingm, Point& pre);
	
};

}//namespace engine