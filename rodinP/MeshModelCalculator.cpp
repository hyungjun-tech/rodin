#include "stdafx.h"
#include "MeshModelCalculator.h"
#include "AABB.h"
#include "MeshModel.h"
#include "SaveFileUIMode.h"

MeshModelCalculator::MeshModelCalculator()
{

}

/// Check Range Validation Function
bool MeshModelCalculator::checkModelRange(IMeshModel* model_)
{
	if (!isInsidePlatform(model_))
		return false;
	if (!checkXYLimit(model_))
		return false;
	if (!checkZLimit(model_))
		return false;
	return true;
}

bool MeshModelCalculator::isInsidePlatform(IMeshModel* model_)
{
	float margin = 0.0001;
	AABB totalBox = model_->getTotalBox();
	AABB machineBox = Profile::getMachineAABB();
	qglviewer::Vec machineMin = machineBox.getMinimum();
	qglviewer::Vec machineMax = machineBox.getMaximum();
	qglviewer::Vec vmin = totalBox.getMinimum();
	qglviewer::Vec vmax = totalBox.getMaximum();
	for (int i = 0; i < 3; i++)
	{
		if (vmin[i] + margin < machineMin[i] ||
			vmax[i] - margin > machineMax[i])
			return false;
	}
	return true;
}

// check back corner quad region
bool MeshModelCalculator::checkXYLimit(IMeshModel* model_)
{
	if (!Profile::machineProfile.restrict_rearQuadRegion_enabled.value)
		return true;
	qglviewer::Vec quad = Profile::getRearQuad();
	qglviewer::Vec modelMargin = Profile::getMargin();
	qglviewer::Vec rearMargin;
	for (int i = 0; i < 3; i++)
	{
		rearMargin[i] = quad[i] + trunc(modelMargin[i]);
	}
	QImage upper_image = SaveFileUIMode::saveFullUpperImage(model_->getModels());
	//upper_image.save("upper_FBO.png");
	for (uint x = 0; x < rearMargin[0]; ++x)
	{
		for (uint y = 0; y < rearMargin[1]; ++y)
		{
			//image의 blue값이 index에 따라 변경되어 값이 포화되는 문제 발생 --> 일단 green으로 검사..// 200527 swyang
			//green값은 변화폭이 작아 포화되는 문제 해당 없음..// 
			if (qGreen(upper_image.pixel(x, y)) > 0 ||
				qGreen(upper_image.pixel(upper_image.width() - x - 1, y)) > 0)
				return false;
		}
	}
	return true;
}

// check front upper block region //
bool MeshModelCalculator::checkZLimit(IMeshModel* model_)
{
	if (!Profile::machineProfile.restrict_frontUpperRegion_enabled.value)
		return true;

	qglviewer::Vec restrictedBox = Profile::getUpperRestrictedBox();
	QImage right_image = SaveFileUIMode::saveFullRightImage(model_->getModels());
	//right_image.save("D:\\FBO_temp\\right_FBO.png");
	for (uint x = 0; x < restrictedBox[1]; ++x)
	{
		for (uint y = 0; y < restrictedBox[2]; ++y)
		{
			if (qGreen(right_image.pixel(x, y)) > 0)
				return false;
		}
	}
	return true;
}

bool MeshModelCalculator::maxSize(IMeshModel* model_)
{
	fitMachineSize(model_);
	bool checkVolume = checkModelRange(model_);
	if (!Profile::machineProfile.restrict_rearQuadRegion_enabled.value &&
		!Profile::machineProfile.restrict_rearQuadRegion_enabled.value)
		return checkVolume;

	if (checkVolume)
		return true;

	int loopCount = 0;
	while (true)
	{
		loopCount++;
		if (loopCount > 20)
			return false;
		if (maxSize_XYLimit(model_))
			if (checkModelRange(model_))
				return true;
		if (maxSize_ZLimit(model_))
			if (checkModelRange(model_))
				return true;
	}
	return false;
}

void MeshModelCalculator::fitMachineSize(IMeshModel* model_)
{
	float margin = 0.0001;
	AABB machineBox = Profile::getMachineAABB();
	AABB box = model_->getAABB();
	qglviewer::Vec modelBox = box.getMaximum() - box.getMinimum();
	qglviewer::Vec modelMargin = Profile::getMargin();
	modelMargin[0] = modelMargin[0] * 2;
	modelMargin[1] = modelMargin[1] * 2;
	qglviewer::Vec machine = machineBox.getMaximum() - machineBox.getMinimum();

	float fitScale[3];
	for (int i = 0; i < 3; i++) {
		fitScale[i] = (machine[i] - modelMargin[i] - margin) / modelBox[i];
	}
	double ratio = *std::min_element(fitScale, fitScale + 3);

	// scaling.. //
	model_->setScale(qglviewer::Vec(ratio, ratio, ratio));
	model_->calcAABB();
	//move to center
	qglviewer::Vec v1 = model_->getTotalBox().getFloorCenter();
	qglviewer::Vec v2 = machineBox.getFloorCenter();
	qglviewer::Vec v = v2 - v1;
	model_->translate(v[0], v[1], v[2]);
	model_->calcAABB();
}

void MeshModelCalculator::move_Limit(IMeshModel* model_, qglviewer::Vec target_)
{
	AABB machineBox = Profile::getMachineAABB();
	AABB totalBox = model_->getTotalBox();
	qglviewer::Vec diff = target_ - totalBox.getFloorCenter();
	//이동후 베드밖으로 나가는 경우 모서리로 정렬
	if (totalBox.y0 + diff[1] < 0)
		diff[1] = machineBox.y0 - totalBox.y0;
	if (totalBox.y1 + diff[1] > machineBox.y1)
		diff[1] = machineBox.y1 - totalBox.y1;

	model_->translate(diff[0], diff[1], 0);
	model_->calcAABB();
}

void MeshModelCalculator::moveRear(IMeshModel* model_)
{
	float upperBox_y = Profile::getUpperRestrictedBox()[1];
	qglviewer::Vec machineCenter = Profile::getMachineAABB().getFloorCenter();
	machineCenter[1] = machineCenter[1] + upperBox_y / 2;
	move_Limit(model_, machineCenter);
}
void MeshModelCalculator::moveFront(IMeshModel* model_)
{
	float rearQuad_y = Profile::getRearQuad()[1];
	qglviewer::Vec machineCenter = Profile::getMachineAABB().getFloorCenter();
	machineCenter[1] = machineCenter[1] - rearQuad_y / 2;
	move_Limit(model_, machineCenter);
}
void MeshModelCalculator::moveBetweenLimits(IMeshModel* model_)
{
	qglviewer::Vec rearQuad = Profile::getRearQuad();
	float rearQuad_y = Profile::getRearQuad()[1];
	float upperBox_y = Profile::getUpperRestrictedBox()[1];
	qglviewer::Vec machineCenter = Profile::getMachineAABB().getFloorCenter();
	machineCenter[1] = machineCenter[1] - rearQuad_y / 2 + upperBox_y / 2;
	move_Limit(model_, machineCenter);
}

bool MeshModelCalculator::maxSize_XYLimit(IMeshModel* model_)
{
	AABB machineBox = Profile::getMachineAABB();
	qglviewer::Vec machine = machineBox.getMaximum() - machineBox.getMinimum();
	if (Generals::isReplicationUIMode())
		machine[0] = machine[0] * 0.5;
	qglviewer::Vec quadMargin = Profile::getRearQuad();
	qglviewer::Vec modelMargin = Profile::getMargin();
	modelMargin[0] = ceil(modelMargin[0]);
	modelMargin[1] = ceil(modelMargin[1]);

	quadMargin[0] = quadMargin[0] * 2;
	quadMargin[1] = quadMargin[1];

	std::vector<double> candidateRatio;
	double ratio;
	AABB box = model_->getAABB();
	qglviewer::Vec modelBox = box.getMaximum() - box.getMinimum();
	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < modelMargin[j]; i++)
		{
			ratio = (machine[j] - quadMargin[j] - i) / modelBox[j];
			if (ratio < 1)
				candidateRatio.push_back(ratio);
			else
				candidateRatio.push_back(1);
		}
	}

	std::sort(candidateRatio.begin(), candidateRatio.end(), greater<double>());
	candidateRatio.erase(std::unique(candidateRatio.begin(), candidateRatio.end()), candidateRatio.end());
	//double ratio = *std::max_element(candidateRatio.begin(), candidateRatio.end());

	for (auto r : candidateRatio)
	{
		if (r < 1)
		{
			// scaling.. //
			model_->setScale(qglviewer::Vec(r, r, r));
			model_->calcAABB();
		}
		moveFront(model_);
		if (checkXYLimit(model_))
			return true;
		moveRear(model_);
		if (checkXYLimit(model_))
			return true;
	}

	return false;
}

bool MeshModelCalculator::maxSize_ZLimit(IMeshModel* model_)
{
	AABB machineBox = Profile::getMachineAABB();
	qglviewer::Vec machine = machineBox.getMaximum() - machineBox.getMinimum();
	qglviewer::Vec quadMargin = Profile::getRearQuad();
	qglviewer::Vec upperBox = Profile::getUpperRestrictedBox();
	qglviewer::Vec modelMargin = Profile::getMargin();
	modelMargin[0] = ceil(modelMargin[0]);
	modelMargin[1] = ceil(modelMargin[1]);
	modelMargin[2] = ceil(modelMargin[2]);

	quadMargin[0] = quadMargin[0] * 2;
	quadMargin[1] = quadMargin[1];
	upperBox[1] = upperBox[1];
	upperBox[2] = upperBox[2];

	std::vector<double> candidateRatio;
	double ratio;
	AABB box = model_->getAABB();
	qglviewer::Vec modelBox = box.getMaximum() - box.getMinimum();

	//case 1. 상단바 뒤쪽 + 후면 클립 사이에 위치
	ratio = (machine[1] - upperBox[1] - modelMargin[1]) / modelBox[1];
	if (ratio < 1) candidateRatio.push_back(ratio);
	else candidateRatio.push_back(1);

	for (int i = 0; i < modelMargin[0]; i++)
	{
		ratio = (machine[0] - quadMargin[0] - i) / modelBox[0];
		if (ratio < 1) candidateRatio.push_back(ratio);
		else candidateRatio.push_back(1);
	}
	//case 2. 상단바 아래 + 후면 클립 앞쪽
	for (int i = 0; i < modelMargin[2]; i++)
	{
		ratio = (machine[2] - upperBox[2] - i) / modelBox[2];
		if (ratio < 1) candidateRatio.push_back(ratio);
		else candidateRatio.push_back(1);
	}

	std::sort(candidateRatio.begin(), candidateRatio.end(), greater<double>());
	candidateRatio.erase(std::unique(candidateRatio.begin(), candidateRatio.end()), candidateRatio.end());
	//double ratio = *std::max_element(candidateRatio.begin(), candidateRatio.end());

	for (auto r : candidateRatio)
	{
		if (r < 1)
		{
			// scaling.. //
			model_->setScale(qglviewer::Vec(r, r, r));
			model_->calcAABB();
		}
		moveFront(model_);
		if (checkZLimit(model_))
			return true;
		moveRear(model_);
		if (checkZLimit(model_))
			return true;
		moveBetweenLimits(model_);
		if (checkZLimit(model_))
			return true;
	}
	return false;
}