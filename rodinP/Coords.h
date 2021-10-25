#pragma once

struct PixelCoords
{
	PixelCoords(int i_, int j_) : i(i_), j(j_) {}
	int i, j;
};
struct World2DCoords
{
	World2DCoords(float x_, float y_) : x(x_), y(y_) {}
	float x, y;
};

class Volume3DCoords
{
public:
	Volume3DCoords() : row(0), col(0), layerN(0) {}
	Volume3DCoords(int row_, int col_, int layerN_)
		: row(row_), col(col_), layerN(layerN_) {}

	int row, col, layerN;
};

class World3DCoords
{
public:
	World3DCoords() : x(0), y(0), z(0) {}
	World3DCoords(float x_, float y_, float z_)
		: x(x_), y(y_), z(z_) {}

	float x, y, z;
};