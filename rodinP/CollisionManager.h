#pragma once

#include "AABB.h"

class IMeshModel;
class CollisionManager
{
public:
	CollisionManager();
	~CollisionManager();

	std::vector<qglviewer::Vec> resolveCollision(
		const AABB &fixed_, const std::vector<AABB> &movable_);

	void detectCollision(std::vector<IMeshModel*> fix_, std::vector<IMeshModel*> move_);
	void arrangeModels(std::vector<IMeshModel*> fix_, std::vector<IMeshModel*> move_);
private:
	std::map<int, qglviewer::Vec> response;
	std::map<int, int> priority;
	std::vector<AABB> aabb;

	void resolveCollision(AABB &pushed_, int id_);
	bool isColliding(const AABB &a_, const AABB &b_);
	bool isOverlapped(float min1, float max1, float min2, float max2);
	qglviewer::Vec calcMinTraslationVector(
		const AABB &fixed_, const AABB &movable_);


	class ShorterPointFirst
	{
	public:
		bool operator()(const std::pair<QPoint, int>& lhs_,
			const std::pair<QPoint, int> rhs_)
		{
			return lhs_.second < rhs_.second;
		}
	};
};