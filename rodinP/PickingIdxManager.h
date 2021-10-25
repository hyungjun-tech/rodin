#pragma once

class PickingIdxManager
{
public:
	PickingIdxManager();
	~PickingIdxManager();
	
	class ToIndexedRGB 
	{
	public:
		qglviewer::Vec operator()(int index_)
		{
			int r = index_ / 65536;
			int g = (index_ - r * 65536) / 256;
			int b = (index_ - r * 65536 - g * 256);

			return qglviewer::Vec(r / 255.0, g / 255.0, b / 255.0);
		}
	};

	class ToIntIndex
	{
	public:
		int operator()(qglviewer::Vec rgb_)
		{
			rgb_ = rgb_ * 255.0;
			return rgb_[2] + rgb_[1] * 256 + rgb_[0] * 256 * 256;
		}
	};

	static int getMeshID();
	static int getSupportPointID();
	static int getDrainHolePointID();

	static qglviewer::Vec encodeID(int id_);
	static int decodeID(qglviewer::Vec clr_);

	static bool isMeshID(int id_);
	static bool isSupportPointID(int id_);
	static bool isDrainHolePointID(int id_);
	static bool isTransZID(int id_);
	static bool isTransXYID(int id_);
	static bool isRotXID(int id_);
	static bool isRotYID(int id_);
	static bool isRotZID(int id_);
	static bool isPlaneID(int id_);

	static const int TRANS_Z_ID;
	static const int TRANS_XY_ID;

	static const int ROT_X_ID;
	static const int ROT_Y_ID;
	static const int ROT_Z_ID;

	static const int BED_ID;
	static const int PLANE_ID;

private:
	static const int MESH_ID_START;
	static const int MESH_ID_END;

	static const int SUPPORT_POINT_START;
	static const int SUPPORT_POINT_END;

	static const int DRAINHOLE_POINT_START;
	static const int DRAINHOLE_POINT_END;

	static int meshID;
	static int supportPointID;
	static int drainHolePointID;
};