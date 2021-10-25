#include "stdafx.h"
#include "CollisionManager.h"
#include "SaveFileUIMode.h"
#include "MeshModel.h"

CollisionManager::CollisionManager()
{

}
CollisionManager::~CollisionManager()
{

}

std::vector<qglviewer::Vec> CollisionManager::resolveCollision(
	const AABB &fixed_, const std::vector<AABB> &movable_)
{
	aabb.push_back(fixed_);
	for (int i = 0; i < movable_.size(); i++)
	{
		aabb.push_back(movable_[i]);
		priority[i + 1] = movable_.size() + 1;
		response[i + 1] = qglviewer::Vec(0, 0, 0);
	}

	priority[0] = 1;
	response[0] = qglviewer::Vec(0, 0, 0);

	for (int i = 1; i < aabb.size(); i++)
	{
		if (!isColliding(aabb[0], aabb[i]))
			continue;

		qglviewer::Vec mtv = calcMinTraslationVector(aabb[0], aabb[i]);
		aabb[i].translate(mtv);
		priority[i] = priority[0] + 1;
		response[i] = response[i] + mtv;
		resolveCollision(aabb[i], i);
	}

	std::vector<qglviewer::Vec> ret;
	for (int i = 0; i < movable_.size(); i++)
		ret.push_back(response[i + 1]);
	
	return ret;
}

void CollisionManager::resolveCollision(AABB &pushed_, int id_)
{
	for (int i = 0; i < aabb.size(); i++)
	{
		if (i == id_)
			continue;

		if (!isColliding(pushed_, aabb[i]))
			continue;


		if (priority[id_] <= priority[i])
		{
			qglviewer::Vec mtv = calcMinTraslationVector(pushed_, aabb[i]);
			aabb[i].translate(mtv);
			priority[i] = priority[id_] + 1;
			response[i] = response[i] + mtv;
			resolveCollision(aabb[i], i);
		}
		else
		{
			qglviewer::Vec mtv = calcMinTraslationVector(aabb[i], pushed_);
			pushed_.translate(mtv);
			priority[id_] = priority[i] + 1;
			response[id_] = response[id_] + mtv;
			resolveCollision(pushed_, id_);
		}
	}
}

bool CollisionManager::isColliding(const AABB &a_, const AABB &b_)
{
	qglviewer::Vec minV1 = a_.getMinimum();
	qglviewer::Vec maxV1 = a_.getMaximum();

	qglviewer::Vec minV2 = b_.getMinimum();
	qglviewer::Vec maxV2 = b_.getMaximum();

	if (!isOverlapped(minV1.x, maxV1.x, minV2.x, maxV2.x))
	{
		return false;
	}
	if (!isOverlapped(minV1.y, maxV1.y, minV2.y, maxV2.y))
	{
		return false;
	}

	return true;
}

bool CollisionManager::isOverlapped(float min1, float max1, float min2, float max2)
{
	if (min1 > max2 || max1 < min2)
		return false;
	else
		return true;
}

qglviewer::Vec CollisionManager::calcMinTraslationVector(
	const AABB &fixed_, const AABB &movable_)
{
	float tvs[2];
	for (int i = 0; i < 2; i++) {
		float movableProjMin = movable_.getMinimum()[i];
		float movableProjMax = movable_.getMaximum()[i];
		float movableProjCen = (movableProjMin + movableProjMax) / 2.0;

		float fixedProjMin = fixed_.getMinimum()[i];
		float fixedProjMax = fixed_.getMaximum()[i];
		float fixedProjCen = (fixedProjMin + fixedProjMax) / 2.0;

		float lenTargetAABB = fixedProjMax - fixedProjMin;
		float lenMyAABB = movableProjMax - movableProjMin;
		float lenSum = lenTargetAABB + lenMyAABB;

		float projMin = std::min(fixed_.getMinimum()[i], movable_.getMinimum()[i]);
		float projMax = std::max(fixed_.getMaximum()[i], movable_.getMaximum()[i]);
		float mergedLen = projMax - projMin;

		tvs[i] = (lenSum - mergedLen);
		//fixed이 movable를 포함하는 경우
		if (projMax == fixedProjMax && projMin == fixedProjMin) {
			if ((fixedProjMax - movableProjMax) >
				(movableProjMin - fixedProjMin)) {
				tvs[i] += movableProjMin - fixedProjMin;
			}
			else {
				tvs[i] += fixedProjMax - movableProjMax;
			}
		}
		//movable가 fixed을 포함하는 경우
		else if (projMax == movableProjMax && projMin == movableProjMin) {
			if ((movableProjMax - fixedProjMax) >
				(fixedProjMin - movableProjMin)) {
				tvs[i] += fixedProjMin - movableProjMin;
			}
			else {
				tvs[i] += movableProjMax - fixedProjMax;
			}
		}

		if (movableProjCen < fixedProjCen)
			tvs[i] = -tvs[i];
	}

	qglviewer::Vec mtv;

	float margin = 3.0;

	//각 축의 translation vector 중 크기가 가장 작은 축으로 movable를 움직임
	//가장 효율적
	if (abs(tvs[0]) > abs(tvs[1])) {
		tvs[1] = tvs[1] > 0 ? tvs[1] + margin : tvs[1] - margin; // y축으로의 이동
		mtv = qglviewer::Vec(0, tvs[1], 0);
	}
	else {
		tvs[0] = tvs[0] > 0 ? tvs[0] + margin : tvs[0] - margin; // x축으로의 이동
		mtv = qglviewer::Vec(tvs[0], 0, 0);
	}

	return mtv;
}

void CollisionManager::detectCollision(std::vector<IMeshModel*> fix_, std::vector<IMeshModel*> move_)
{
	for (int i = 0; i < move_.size(); i++)
	{
		if (!isColliding(AABBGetter()(fix_), AABBGetter()(std::vector<IMeshModel*>{move_[i]})))
			continue;
		std::vector<IMeshModel*> temp = fix_;
		for (int j = 0; j < move_.size(); j++)
		{
			if (move_[j] == move_[i])
				continue;
			temp.push_back(move_[j]);
		}
		arrangeModels(temp, std::vector<IMeshModel*>{move_[i]});
	}
}

/*void CollisionManager::ArrangeModels(std::vector<IMeshModel*> fix_, std::vector<IMeshModel*> move_)
{
	if (fix_.size() < 1)
		return;
	if (move_.size() < 1)
		return;
	SaveFileUIMode saveUI;
	QImage temImage = saveUI.SaveUpperImage(fix_);
	//temImage.save("fbo_all.png", "PNG");
	int margin = 3;
	for (int i = 0; i < margin; ++i)
		temImage = saveUI.dilationImage(temImage);

	QImage temImage2 = saveUI.SaveCroppedUpperImage(move_);
	//temImage2.save("fbo_one.png", "PNG");
	QPoint center2(temImage2.width()*0.5, temImage2.height()*0.5);

	//get destination location
	AABB aabb = AABBGetter()(move_);
	QVector3D destLoc = QVector3D(aabb.GetFloorCenter()[0], aabb.GetFloorCenter()[1], 0);


	std::vector<QPoint> printedPixels;
	printedPixels.reserve(temImage2.width()*temImage2.height());
	for (int x = 0; x < temImage2.width(); ++x)
	{
		for (int y = 0; y < temImage2.height(); ++y)
		{
			QRgb c = temImage2.pixel(x, y);
			int alpha = qAlpha(c);
			if (alpha > 0)
				printedPixels.push_back(QPoint(x, y));
		}
	}

	destLoc[1] = temImage.height() - destLoc[1];


	int checkRow = temImage.width() - temImage2.width();
	int checkCol = temImage.height() - temImage2.height();

	QVector3D minC(0, 0, 0);
	if (checkCol < 0 || checkRow < 0)
		minC = destLoc + QVector3D(temImage2.width(), temImage2.height(), 0)*0.1;
	else
	{
		std::vector<std::vector<int>> overlapCount(checkRow, std::vector<int>(checkCol, 0));

		float minDistSquare = 1000 * 1000;
		for (int x = 0; x < checkRow; ++x)
		{
			for (int y = 0; y < checkCol; ++y)
			{
				int cnt = 0;
				for (int i = 0; i < printedPixels.size(); ++i)
				{
					int x2 = printedPixels[i].x();
					int y2 = printedPixels[i].y();
					int x3 = x + x2;
					int y3 = y + y2;


					QRgb c1 = temImage.pixel(x3, y3);
					QRgb c2 = temImage2.pixel(x2, y2);
					int alpha1 = qAlpha(c1);
					int alpha2 = qAlpha(c2);
					if (alpha1 > 0 && alpha2 > 0)
					{
						++cnt;
					}
				}
				overlapCount[x][y] = cnt;

				if (cnt > 0) break;

				QVector3D center(x + center2.x(), y + center2.y(), destLoc[2]);
				float distSquare = (center - destLoc).lengthSquared();

				if (distSquare == 0) break;

				if (distSquare < minDistSquare)
				{
					minDistSquare = distSquare;
					minC = center;
				}
			}
		}

		if (minC.lengthSquared() == 0)
		{
			qDebug() << "too big";
			int minCount = temImage2.width()*temImage2.height();
			for (size_t x = 0; x < overlapCount.size(); ++x)
			{
				for (size_t y = 0; y < overlapCount[x].size(); ++y)
				{
					if (overlapCount[x][y] < minCount)
					{
						minCount = overlapCount[x][y];
						minC[0] = x + center2.x();
						minC[1] = y + center2.y();
					}
				}
			}
		}
	}

	for (int i = 0; i < move_.size(); i++) {
		move_[i]->Translate(minC[0] - aabb.GetFloorCenter()[0], temImage.height() - minC[1] - aabb.GetFloorCenter()[1], 0);
		move_[i]->Manipulated();
	}
}*/

//현재 모델의 위치를 중심으로 겹치지 않는 곳을 찾음. Min값을 찾을 필요가 없으므로 빨리 끝날것으로 예상.
void CollisionManager::arrangeModels(std::vector<IMeshModel*> fix_, std::vector<IMeshModel*> move_)
{
	if (fix_.size() < 1)
		return;
	if (move_.size() < 1)
		return;
	QImage temImage = SaveFileUIMode::saveFullUpperImage(fix_).mirrored();
	
	//cv::Mat temp(temImage.height(), temImage.width(), CV_8UC4, (uchar*)temImage.bits(), temImage.bytesPerLine());
	int margin = 3;
	for (int i = 0; i < margin; ++i)
		temImage = SaveFileUIMode::dilationImage(temImage);
	//temImage.save("fbo_all.png", "PNG");

	QImage temImage2 = SaveFileUIMode::saveCroppedUpperImage(move_).mirrored();
	//temImage2.save("fbo_one.png", "PNG");
	QPoint center2(temImage2.width()*0.5 + 0.5, temImage2.height()*0.5 + 0.5);
	if (temImage2.width() * temImage2.height() == 0)
	{
		temImage2 = QImage(1, 1, QImage::Format_RGB16);
	}
	//get destination location
	AABB aabb = AABBGetter()(move_);

	int descX = aabb.getFloorCenter()[0];
	int descY = aabb.getFloorCenter()[1];

	int count = 0;
	std::vector<QPoint> printedPixels;
	printedPixels.reserve(temImage2.width()*temImage2.height());
	if (printedPixels.size() == 1)
		printedPixels.reserve(1);
	for (int x = 0; x < temImage2.width(); ++x)
	{
		for (int y = 0; y < temImage2.height(); ++y)
		{
			QRgb c = temImage2.pixel(x, y);
			int alpha = qAlpha(c);
			if (alpha > 0)
			{
				printedPixels.push_back(QPoint(x, y));
				count++;
			}
		}
	}
	if (count == 0)
	{
		printedPixels.push_back(QPoint(descX, descY));
	}

	std::vector<std::pair<QPoint, int>> targetPoints;
	for (int x = 0; x < temImage.width(); x++)
	{
		if (x - center2.rx() < 0 || x + center2.rx() >= temImage.width())
			continue;
		for (int y = 0; y < temImage.height(); y++)
		{
			if (y - center2.ry() < 0 || y + center2.ry() >= temImage.height())
				continue;
			targetPoints.push_back(std::make_pair(QPoint(x, y), (x - descX)*(x - descX) + (y - descY)*(y - descY)));
		}
	}
	std::sort(targetPoints.begin(), targetPoints.end(), ShorterPointFirst());


	for (int k = 0; k < targetPoints.size(); k++)
	{
		int cnt = 0;
		int x = targetPoints[k].first.rx();
		int y = targetPoints[k].first.ry();

		for (int j = 0; j < printedPixels.size(); ++j)
		{
			int x2 = printedPixels[j].x();
			int y2 = printedPixels[j].y();
			int x3 = (x - center2.rx()) + x2;
			int y3 = (y - center2.ry()) + y2;

			QRgb c1 = temImage.pixel(x3, y3);
			QRgb c2 = temImage2.pixel(x2, y2);
			int alpha1 = qAlpha(c1);
			int alpha2 = qAlpha(c2);
			if (alpha1 > 0 && alpha2 > 0)
			{
				++cnt;
			}
		}
		if (cnt == 0)
		{
			for (int i = 0; i < move_.size(); i++) {
				move_[i]->translate(x - descX, y - descY, 0);
				move_[i]->manipulated();
			}
			return;
		}
	}

	//배치할 자리가 없을 경우
	AABB updatedBox = AABBGetter()(fix_);
	std::vector<AABB> movable{ AABBGetter()(move_) };

	std::vector<qglviewer::Vec> response = resolveCollision(updatedBox, movable);
	for (int i = 0; i < response.size(); i++)
	{
		qglviewer::Vec t = response[i];
		move_[i]->translate(t.x, t.y, t.z);
		move_[i]->manipulated();
	}
}