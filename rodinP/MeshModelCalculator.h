#pragma once
class AABB;
class IMeshModel;
class MeshModelCalculator
{
public:
	MeshModelCalculator();
	static bool checkModelRange(IMeshModel* model_);
	static bool maxSize(IMeshModel* model_);
private:
	static bool isInsidePlatform(IMeshModel* model_);
	static bool checkXYLimit(IMeshModel* model_);
	static bool checkZLimit(IMeshModel* model_);

	static void fitMachineSize(IMeshModel* model_);
	static void move_Limit(IMeshModel* model_, qglviewer::Vec target_);
	static void moveRear(IMeshModel* model_);
	static void moveFront(IMeshModel* model_);
	static void moveBetweenLimits(IMeshModel* model_);
	static bool maxSize_XYLimit(IMeshModel* model_);
	static bool maxSize_ZLimit(IMeshModel* model_);
};

