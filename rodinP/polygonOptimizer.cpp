/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#include "stdafx.h"
#include "polygonOptimizer.h"

void optimizePolygon(engine::PolygonRef poly)
{
    Point p0 = poly[poly.size()-1];
    for(unsigned int i=0;i<poly.size();i++)
    {
        Point p1 = poly[i];
        if (shorterThen(p0 - p1, MICRON2INT(10)))
        {
            poly.remove(i);
            i --;
        }else if (shorterThen(p0 - p1, MICRON2INT(500)))
        {
            Point p2;
            if (i < poly.size() - 1)
                p2 = poly[i+1];
            else
                p2 = poly[0];
            
            Point diff0 = normal(p1 - p0, 10000000);
            Point diff2 = normal(p1 - p2, 10000000);
          
            int64_t d = dot(diff0, diff2);
            if (d < -99999999999999LL)		// 일직선에 있는 불필요한 경로를 제거 3점을 2점으로
            {
                poly.remove(i);
                i --;
            }else{
                p0 = p1;
            }
        }else{
            p0 = p1;
        }
    }
}

void optimizePolygons(engine::Polygons& polys)
{
    for(unsigned int n=0;n<polys.size();n++)
    {
        optimizePolygon(polys[n]);
        if (polys[n].size() < 3)
        {
            polys.remove(n);
            n--;
        }
    }
}
