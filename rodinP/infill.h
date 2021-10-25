/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#pragma once

#include "polygon.h"

namespace engine {

void generateConcentricInfill(Polygons outline, Polygons& result, int inset_value);
void generateAutomaticInfill(const Polygons& in_outline, Polygons& result, int extrusionWidth, int lineSpacing, int infillOverlap, double rotation);
void generateGridInfill(const Polygons& in_outline, Polygons& result, int extrusionWidth, int lineSpacing, int infillOverlap, double rotation);
void generateLineInfill(const Polygons& in_outline, Polygons& result, int extrusionWidth, int lineSpacing, int infillOverlap, double rotation);
void generateLineInfill_vertical_offset(const Polygons& in_outline, Polygons& result, int extrusionWidth, int lineSpacing, int infillOverlap, double rotation, int offset);
void generateLineInfill_offset(const Polygons& in_outline, Polygons& result, int extrusionWidth, int lineSpacing, int infillOverlap, double rotation, int offset);

}//namespace engine