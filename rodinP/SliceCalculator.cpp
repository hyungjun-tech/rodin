#include "stdafx.h"
#include "SliceCalculator.h"
#include "polygonOptimizer.h"
#include "settings.h"
#include "ModelContainer.h"
#include "MeshModel.h"
#define MIN_AREA_SIZE (0.4 * 0.4)



SliceCalculator::SliceCalculator(std::vector<IMeshModel*> models_)
	: models (models_)
{
}

SliceCalculator::~SliceCalculator()
{
}

//inset
void SliceCalculator::generateInsetsLayerPart(SliceLayerPart* _part, int _offset, int _inset_count, int _internal_moving_area)
{
	double combOffsetFactor = 1.0;

	switch (_internal_moving_area)
	{
	case Generals::InternalMoving_Area::Narrow:
		combOffsetFactor = 1.5;
		break;
	case Generals::InternalMoving_Area::Defualt:
		combOffsetFactor = 1.0;
		break;
	case Generals::InternalMoving_Area::Wide:
		combOffsetFactor = 0.5;
		break;
	}

	//combing boundary adapted by combOffsetFactor//
	_part->combBoundery = _part->outline.offset(-_offset * combOffsetFactor);


	//inset setting//
	_part->insets.clear();
	if (_inset_count == 0)
	{
		_part->insets.push_back(_part->outline);		//?? part->outline.offset(-offset/2) 아닌가?		// spiralizeMode
		return;
	}

	for (int i = 0; i < _inset_count; i++)
	{
		_part->insets.push_back(engine::Polygons());
		_part->insets[i] = _part->outline.offset(-_offset * i - _offset / 2);		// offset에서 알아서 중복라인은 제거하는듯.

		optimizePolygons(_part->insets[i]);

		if (_part->insets[i].size() < 1)
		{
			_part->insets.pop_back();
			break;
		}
	}
}

void SliceCalculator::generateInsets(SliceLayer* _layer, int _outer_offset, int _inset_count, int _internal_moving_area, bool _simple_mode)
{
	for (unsigned int partNr = 0; partNr < _layer->parts.size(); partNr++)
	{
		generateInsetsLayerPart(&_layer->parts[partNr], _outer_offset, _inset_count, _internal_moving_area);
	}

	//Remove the parts which did not generate an inset. As these parts are too small to print,
	// and later code can now assume that there is always minimal 1 inset line.
	for (unsigned int partNr = 0; partNr < _layer->parts.size(); partNr++)
	{
		if (_layer->parts[partNr].insets.size() < 1 && !_simple_mode)
		{
			_layer->parts.erase(_layer->parts.begin() + partNr);		//?? 아예 part를 지워버리는 효과가 있네?? 에러가 발생할 수 있을듯.
			partNr -= 1;
		}
	}
}

void SliceCalculator::generateSupportInsetsLayerPart(SupportLayerPart* support_part, int offset, int insetCount)
{
	//틈공간 test양호//
	//part->combBoundery = part->outline.offset(-offset);

	//기본 설정 : spiral모델같은 얇은 날개 형상에 유리..//
	//support_part->combBoundery = support_part->outline.offset(-offset / 3);

	support_part->support_insets.clear();
	if (insetCount == 0)
	{
		support_part->support_insets.push_back(support_part->support_outline);		//?? part->outline.offset(-offset/2) 아닌가?		// spiralizeMode
		return;
	}

	for (int i = 0; i < insetCount; i++)
	{
		support_part->support_insets.push_back(engine::Polygons());
		support_part->support_insets[i] = support_part->support_outline.offset(-offset * i - offset / 2);		// offset에서 알아서 중복라인은 제거하는듯.
		optimizePolygons(support_part->support_insets[i]);

		if (support_part->support_insets[i].size() < 1)
		{
			support_part->support_insets.pop_back();
			break;
		}
	}
}


void SliceCalculator::generateSupportInsets(SupportLayer* support_layer, int offset, int insetCount)
{
	for (unsigned int partNr = 0; partNr < support_layer->support_parts.size(); partNr++)
	{
		generateSupportInsetsLayerPart(&support_layer->support_parts[partNr], offset, insetCount);
	}

	//Remove the parts which did not generate an inset. As these parts are too small to print,
	// and later code can now assume that there is always minimal 1 inset line.
	for (unsigned int partNr = 0; partNr < support_layer->support_parts.size(); partNr++)
	{
		if (support_layer->support_parts[partNr].support_insets.size() < 1)
		{
			support_layer->support_parts.erase(support_layer->support_parts.begin() + partNr);		//?? 아예 part를 지워버리는 효과가 있네?? 에러가 발생할 수 있을듯.
			partNr -= 1;
		}
	}
}






//skin
void SliceCalculator::generateSkins(int layerNr, std::vector<SliceLayer> *layers, int extrusionWidth, int downSkinCount, int upSkinCount, int infillOverlap)
{
	if (layers->size() <= layerNr)
		return;
	SliceLayer* layer = &layers->at(layerNr);

	for (unsigned int partNr = 0; partNr < layer->parts.size(); partNr++)
	{
		SliceLayerPart* part = &layer->parts[partNr];
		part->skinOutline.clear();
		part->skinPartOutline.clear();
		part->bridges.clear();

		engine::Polygons upskin = part->insets[part->insets.size() - 1].offset(-extrusionWidth / 2);
		engine::Polygons downskin = upskin;

		if (part->insets.size() > 1)
		{
			//Add thin wall filling by taking the area between the insets.
			engine::Polygons thinWalls = part->insets[0].offset(-extrusionWidth / 2 - extrusionWidth * infillOverlap / 100).difference(part->insets[1].offset(extrusionWidth * 6 / 10));
			upskin.add(thinWalls);
			downskin.add(thinWalls);
		}
		if (static_cast<int>(layerNr - downSkinCount) >= 0)
		{
			SliceLayer* layer2 = &layers->at(layerNr - downSkinCount);
			for (unsigned int partNr2 = 0; partNr2 < layer2->parts.size(); partNr2++)
			{
				if (part->boundaryBox.hit(layer2->parts[partNr2].boundaryBox))
					downskin = downskin.difference(layer2->parts[partNr2].insets[layer2->parts[partNr2].insets.size() - 1]);
			}
		}
		if (static_cast<int>(layerNr + upSkinCount) < static_cast<int>(layers->size()))
		{
			SliceLayer* layer2 = &layers->at(layerNr + upSkinCount);
			for (unsigned int partNr2 = 0; partNr2 < layer2->parts.size(); partNr2++)
			{
				if (part->boundaryBox.hit(layer2->parts[partNr2].boundaryBox))
					upskin = upskin.difference(layer2->parts[partNr2].insets[layer2->parts[partNr2].insets.size() - 1]);
			}
		}

		part->skinOutline = upskin.unionPolygons(downskin);

		double minAreaSize = (2 * M_PI * INT2MM(extrusionWidth) * INT2MM(extrusionWidth)) * 0.3;
		for (unsigned int i = 0; i < part->skinOutline.size(); i++)
		{
			double area = INT2MM(INT2MM(fabs(part->skinOutline[i].area())));
			if (area < minAreaSize) // Only create an up/down skin if the area is large enough. So you do not create tiny blobs of "trying to fill"
			{
				part->skinOutline.remove(i);
				i -= 1;
			}
		}

		part->skinPartOutline = part->skinOutline.splitIntoParts();
		part->bridges.resize(part->skinPartOutline.size());

		for (int i = 0; i < part->skinPartOutline.size(); ++i)
		{
			engine::Polygons outline = part->skinPartOutline[i];
			part->bridges[i] = -1;
			if (layerNr > 0)
				part->bridges[i] = bridgeAngle(outline, &layers->at(layerNr - 1));
		}
	}
}

void SliceCalculator::generateSparse(SliceLayer* layer, int extrusionWidth)
{
	for (unsigned int partNr = 0; partNr < layer->parts.size(); partNr++)
	{
		SliceLayerPart* part = &layer->parts[partNr];
		part->sparseOutline.clear();

		engine::Polygons original_outline = part->insets[part->insets.size() - 1].offset(-extrusionWidth / 2);

		part->sparseOutline = original_outline.difference(part->skinOutline);
		part->sparseOutline.removeSmallAreas(MIN_AREA_SIZE);
	}
}
void SliceCalculator::generateSupportInterface(int _layer_number, std::vector<SupportLayer>* _support_layers, int _interface_roof_extrusion_width, int _interface_floor_extrusion_width, int _interface_roof_count, int _interface_floor_count, int _support_infill_overlap)
{
	if (_support_layers->size() <= _layer_number)
		return;
	SupportLayer* supportLayer = &_support_layers->at(_layer_number);

	for (unsigned int partNr = 0; partNr < supportLayer->support_parts.size(); partNr++)
	{
		SupportLayerPart* support_part = &supportLayer->support_parts[partNr];
		support_part->support_interfaceOutline.clear();
		support_part->support_interfacePartOutline.clear();
		support_part->support_bridges.clear();

		engine::Polygons support_interface_roof = support_part->support_insets[support_part->support_insets.size() - 1].offset(-_interface_roof_extrusion_width / 2);	//inset 대신에 test 해봄..//
		engine::Polygons support_interface_floor = support_part->support_insets[support_part->support_insets.size() - 1].offset(-_interface_floor_extrusion_width / 2);

		if (support_part->support_insets.size() > 1)	//??
		{
			//Add thin wall filling by taking the area between the insets.
			engine::Polygons thin_walls_roof = support_part->support_insets[0].offset(-_interface_roof_extrusion_width / 2 - _interface_roof_extrusion_width * _support_infill_overlap / 100).difference(support_part->support_insets[1].offset(_interface_roof_extrusion_width * 6 / 10));
			engine::Polygons thin_walls_floor = support_part->support_insets[0].offset(-_interface_floor_extrusion_width / 2 - _interface_floor_extrusion_width * _support_infill_overlap / 100).difference(support_part->support_insets[1].offset(_interface_floor_extrusion_width * 6 / 10));
			support_interface_roof.add(thin_walls_roof);
			support_interface_floor.add(thin_walls_floor);
		}

		//interface floor//
		if (static_cast<int>(_layer_number - _interface_floor_count) >= 0)
		{
			SupportLayer* tempLayer = &_support_layers->at(_layer_number - _interface_floor_count);

			for (unsigned int partNr2 = 0; partNr2 < tempLayer->support_parts.size(); partNr2++)
			{
				//inset이 없으니, 일단 임시로 outline의 offset으로 대체..//
				if (support_part->support_boundaryBox.hit(tempLayer->support_parts[partNr2].support_boundaryBox))
					support_interface_floor = support_interface_floor.difference(tempLayer->support_parts[partNr2].support_insets[tempLayer->support_parts[partNr2].support_insets.size() - 1]);
			}
		}

		//interface roof//
		if (static_cast<int>(_layer_number + _interface_roof_count) < static_cast<int>(_support_layers->size()))
		{
			SupportLayer* tempLayer = &_support_layers->at(_layer_number + _interface_roof_count);

			for (unsigned int partNr2 = 0; partNr2 < tempLayer->support_parts.size(); partNr2++)
			{
				if (support_part->support_boundaryBox.hit(tempLayer->support_parts[partNr2].support_boundaryBox))
					support_interface_roof = support_interface_roof.difference(tempLayer->support_parts[partNr2].support_insets[tempLayer->support_parts[partNr2].support_insets.size() - 1]);
			}
		}

		// roof + floor --> interface outline//
		//support_part->support_interfaceOutline = support_interface_roof.unionPolygons(support_interface_floor);
		support_part->support_interface_outline_roof = support_interface_roof;
		support_part->support_interface_outline_floor = support_interface_floor;




		//removing small area of interface polygons//
		const double min_area_size_floor = (2 * M_PI * INT2MM(_interface_floor_extrusion_width) * INT2MM(_interface_floor_extrusion_width)) * 0.3;
		const double min_area_size_roof = (2 * M_PI * INT2MM(_interface_roof_extrusion_width) * INT2MM(_interface_roof_extrusion_width)) * 0.3;

		//for (unsigned int i = 0; i < support_part->support_interfaceOutline.size(); i++)
		//{
		//	double area = INT2MM(INT2MM(fabs(support_part->support_interfaceOutline[i].area())));
		//	if (area < min_area_size) // Only create an up/down skin if the area is large enough. So you do not create tiny blobs of "trying to fill"
		//	{
		//		support_part->support_interfaceOutline.remove(i);
		//		i -= 1;
		//	}
		//}


		for (unsigned int i = 0; i < support_part->support_interface_outline_floor.size(); i++)
		{
			double area = INT2MM(INT2MM(fabs(support_part->support_interface_outline_floor[i].area())));
			if (area < min_area_size_floor) // Only create an up/down skin if the area is large enough. So you do not create tiny blobs of "trying to fill"
			{
				support_part->support_interface_outline_floor.remove(i);
				i -= 1;
			}
		}
		for (unsigned int i = 0; i < support_part->support_interface_outline_roof.size(); i++)
		{
			double area = INT2MM(INT2MM(fabs(support_part->support_interface_outline_roof[i].area())));
			if (area < min_area_size_roof) // Only create an up/down skin if the area is large enough. So you do not create tiny blobs of "trying to fill"
			{
				support_part->support_interface_outline_roof.remove(i);
				i -= 1;
			}
		}


		//support interface part outline//
		//support_part->support_interfacePartOutline = support_part->support_interfaceOutline.splitIntoParts();
		//
		//support_part->support_bridges.resize(support_part->support_interfacePartOutline.size());

		//for (int i = 0; i < support_part->support_interfacePartOutline.size(); ++i)
		//{
		//	engine::Polygons outline = support_part->support_interfacePartOutline[i];

		//	support_part->support_bridges[i] = -1;

		//	if (_layer_number > 0)
		//	{
		//		support_part->support_bridges[i] = bridgeAngle(outline, &_support_layers->at(_layer_number - 1));
		//	}

		//}


		//floor//
		support_part->support_interface_part_floor = support_part->support_interface_outline_floor.splitIntoParts();
		support_part->support_bridges_floor.resize(support_part->support_interface_part_floor.size());
		for (int i = 0; i < support_part->support_interface_part_floor.size(); ++i)
		{
			engine::Polygons outline = support_part->support_interface_part_floor[i];

			support_part->support_bridges_floor[i] = -1;

			if (_layer_number > 0)
			{
				support_part->support_bridges_floor[i] = bridgeAngle(outline, &_support_layers->at(_layer_number - 1));
			}
		}
		//roof//
		support_part->support_interface_part_roof = support_part->support_interface_outline_roof.splitIntoParts();
		support_part->support_bridges_roof.resize(support_part->support_interface_part_roof.size());
		for (int i = 0; i < support_part->support_interface_part_roof.size(); ++i)
		{
			engine::Polygons outline = support_part->support_interface_part_roof[i];

			support_part->support_bridges_roof[i] = -1;

			if (_layer_number > 0)
			{
				support_part->support_bridges_roof[i] = bridgeAngle(outline, &_support_layers->at(_layer_number - 1));
			}
		}

	}
}

//void generateSupportSparse(int _layer_number, std::vector<SupportLayer>* _support_layers, int _interface_roof_extrusion_width, int _interface_floor_extrusion_width, int _interface_roof_count, int _interface_floor_count);
void SliceCalculator::generateSupportSparse(int _layer_number, std::vector<SupportLayer>* _support_layers, int _interface_roof_extrusion_width, int _interface_floor_extrusion_width, int _interface_roof_count, int _interface_floor_count)
{
	if (_support_layers->size() <= _layer_number)
		return;

	SupportLayer* support_layer = &_support_layers->at(_layer_number);

	for (unsigned int partNr = 0; partNr < support_layer->support_parts.size(); partNr++)
	{
		SupportLayerPart* support_part = &support_layer->support_parts[partNr];
		support_part->support_sparseOutline.clear();

		//engine::Polygons supportSparse = support_part->support_insets[support_part->support_insets.size() - 1].offset(-extrusionWidth / 2);
		
		engine::Polygons support_sparse_floor = support_part->support_insets[support_part->support_insets.size() - 1].offset(-_interface_floor_extrusion_width / 2);
		engine::Polygons support_sparse_roof = support_part->support_insets[support_part->support_insets.size() - 1].offset(-_interface_roof_extrusion_width / 2);
		engine::Polygons support_interface_floor = support_sparse_floor;
		engine::Polygons support_interface_roof = support_sparse_roof;

		//down skin sparse//
		if (static_cast<int>(_layer_number - _interface_floor_count) >= 0)
		{
			SupportLayer* tempLayer = &_support_layers->at(_layer_number - _interface_floor_count);
			for (unsigned int partNr2 = 0; partNr2 < tempLayer->support_parts.size(); partNr2++)
			{
				if (support_part->support_boundaryBox.hit(tempLayer->support_parts[partNr2].support_boundaryBox))
				{
					if (tempLayer->support_parts[partNr2].support_insets.size() > 1)
					{
						support_interface_floor = support_interface_floor.difference(tempLayer->support_parts[partNr2].support_insets[tempLayer->support_parts[partNr2].support_insets.size() - 2]);
					}
					else
					{
						support_interface_floor = support_interface_floor.difference(tempLayer->support_parts[partNr2].support_insets[tempLayer->support_parts[partNr2].support_insets.size() - 1]);
					}
				}
			}
		}

		//up skin sparse//
		if (static_cast<int>(_layer_number + _interface_roof_count) < static_cast<int>(_support_layers->size()))
		{
			SupportLayer* tempLayer = &_support_layers->at(_layer_number + _interface_roof_count);

			for (unsigned int partNr2 = 0; partNr2 < tempLayer->support_parts.size(); partNr2++)
			{
				if (support_part->support_boundaryBox.hit(tempLayer->support_parts[partNr2].support_boundaryBox))
				{
					if (tempLayer->support_parts[partNr2].support_insets.size() > 1)
					{
						support_interface_roof = support_interface_roof.difference(tempLayer->support_parts[partNr2].support_insets[tempLayer->support_parts[partNr2].support_insets.size() - 2]);
					}
					else
					{
						support_interface_roof = support_interface_roof.difference(tempLayer->support_parts[partNr2].support_insets[tempLayer->support_parts[partNr2].support_insets.size() - 1]);
					}
				}
			}
		}

		engine::Polygons result = support_interface_roof.unionPolygons(support_interface_floor);

		double minAreaSize = 3.0;//(2 * M_PI * INT2MM(config.extrusion_width) * INT2MM(config.extrusion_width)) * 3;
		for (unsigned int i = 0; i < result.size(); i++)
		{
			double area = INT2MM(INT2MM(fabs(result[i].area())));
			if (area < minAreaSize) /* Only create an up/down skin if the area is large enough. So you do not create tiny blobs of "trying to fill" */
			{
				result.remove(i);
				i -= 1;
			}
		}

		engine::Polygons support_sparse = support_sparse_roof.unionPolygons(support_sparse_floor);

		//resut sparse outline..//
		support_part->support_sparseOutline = support_sparse.difference(result);
	}
}


//bridge
int SliceCalculator::bridgeAngle(engine::Polygons outline, SliceLayer* prevLayer)
{
	AABB2D boundaryBox(outline);
	//To detect if we have a bridge, first calculate the intersection of the current layer with the previous layer.
	// This gives us the islands that the layer rests on.
	engine::Polygons islands;
	for (int i = 0; i < prevLayer->parts.size(); ++i)
	{
		auto prevLayerPart = prevLayer->parts[i];
		if (!boundaryBox.hit(prevLayerPart.boundaryBox))
			continue;

		islands.add(outline.intersection(prevLayerPart.outline));
	}
	if (islands.size() > 5 || islands.size() < 1)
		return -1;

	//Next find the 2 largest islands that we rest on.
	double area1 = 0;
	double area2 = 0;
	int idx1 = -1;
	int idx2 = -1;
	for (unsigned int n = 0; n < islands.size(); n++)
	{
		//Skip internal holes
		if (!islands[n].orientation())
			continue;
		double area = fabs(islands[n].area());
		if (area > area1)
		{
			if (area1 > area2)
			{
				area2 = area1;
				idx2 = idx1;
			}
			area1 = area;
			idx1 = n;
		}
		else if (area > area2)
		{
			area2 = area;
			idx2 = n;
		}
	}

	if (idx1 < 0 || idx2 < 0)
		return -1;

	Point center1 = islands[idx1].centerOfMass();
	Point center2 = islands[idx2].centerOfMass();

	return angle(center2 - center1);
}

int SliceCalculator::bridgeAngle(engine::Polygons outline, SupportLayer* support_prevLayer)
{
	AABB2D boundaryBox(outline);
	//To detect if we have a bridge, first calculate the intersection of the current layer with the previous layer.
	// This gives us the islands that the layer rests on.
	engine::Polygons islands;
	for (int i = 0; i < support_prevLayer->support_parts.size(); ++i)
	{
		auto support_prevLayerPart = support_prevLayer->support_parts[i];
		if (!boundaryBox.hit(support_prevLayerPart.support_boundaryBox))
			continue;

		islands.add(outline.intersection(support_prevLayerPart.support_outline));
	}
	if (islands.size() > 5 || islands.size() < 1)
		return -1;

	//Next find the 2 largest islands that we rest on.
	double area1 = 0;
	double area2 = 0;
	int idx1 = -1;
	int idx2 = -1;
	for (unsigned int n = 0; n < islands.size(); n++)
	{
		//Skip internal holes
		if (!islands[n].orientation())
			continue;
		double area = fabs(islands[n].area());
		if (area > area1)
		{
			if (area1 > area2)
			{
				area2 = area1;
				idx2 = idx1;
			}
			area1 = area;
			idx1 = n;
		}
		else if (area > area2)
		{
			area2 = area;
			idx2 = n;
		}
	}

	if (idx1 < 0 || idx2 < 0)
		return -1;

	Point center1 = islands[idx1].centerOfMass();
	Point center2 = islands[idx2].centerOfMass();

	return angle(center2 - center1);
}

void SliceCalculator::generateSkirt(ModelDataStorage* storage, ConfigSettings config)
{
	storage->skirt.clear();

	int adhesionValue = config.platform_adhesion;
	if (adhesionValue == Generals::PlatformAdhesion::Skirt || adhesionValue == Generals::PlatformAdhesion::Brim) {}
	else
		return;

	int distance = config.skirt_distance;
	int extrusionWidth = config.initial_layer_extrusion_width;
	int count = config.skirt_line_count;
	int minLength = config.skirt_min_length;
	int initialLayerHeight = config.initial_layer_height;

	bool externalOnly = (distance > 0);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//generating skirt line first polygon..//

	int offsetDistance_0 = distance; /*+ extrusionWidth / 2;*/

	engine::Polygons skirtFirstPolygons;

	if (!storage->wipeTower_layers.empty())
	{
		skirtFirstPolygons = storage->wipeTower_mainOutline_for_skirt.offset(offsetDistance_0);
		//skirtFirstPolygons = storage->wipeTower_layers.at(0).wipeTower_mainOutline.offset(offsetDistance_0);
	}

	if (storage->supportGenerated())
	{
		SupportPolygonGenerator supportGenerator(storage->supportData);
		engine::Polygons supportPolygons = supportGenerator.generateSupportPolygons(initialLayerHeight);
		//Contract and expand the support polygons so small sections are removed and the final polygon is smoothed a bit.
		supportPolygons = supportPolygons.offset(-extrusionWidth);
		supportPolygons = supportPolygons.offset(extrusionWidth);

		skirtFirstPolygons = skirtFirstPolygons.unionPolygons(supportPolygons.offset(offsetDistance_0));
	}

	for (auto vit = models.begin(); vit != models.end(); ++vit)
	{
		if ((*vit)->sliceLayers.size() == 0)
			continue;
		for (unsigned int i = 0; i < (*vit)->sliceLayers[0].parts.size(); i++)
		{
			skirtFirstPolygons = skirtFirstPolygons.unionPolygons((*vit)->sliceLayers[0].parts[i].outline.offset(offsetDistance_0));
		}
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//generating skirt line polygon by iterating..//

	engine::Polygons skirtPolygons;

	for (int skirtNr = 0; skirtNr < count; skirtNr++)
	{
		int offsetDistance = extrusionWidth * skirtNr + extrusionWidth / 2;

		for (auto vit = models.begin(); vit != models.end(); ++vit)
		{
			if ((*vit)->sliceLayers.size() == 0)
				continue;
			for (unsigned int i = 0; i < (*vit)->sliceLayers[0].parts.size(); i++)
			{
				if (externalOnly)
				{
					//skirt, raft//
					//engine::Polygons p;
					//p.add(vit->layers[0].parts[i].outline[0]);
					//storage->skirt = storage->skirt.unionPolygons(p.offset(offsetDistance));
					skirtPolygons = skirtPolygons.unionPolygons(skirtFirstPolygons.offset(offsetDistance));

				}
				else
				{
					//brim//
					//storage->skirt = storage->skirt.unionPolygons(vit->layers[0].parts[i].outline.offset(offsetDistance));

					skirtPolygons = skirtPolygons.unionPolygons(skirtFirstPolygons.offset(offsetDistance));
					//skirtPolygons = skirtPolygons.unionPolygons(vit->layers[0].parts[i].outline.offset(offsetDistance));
				}
			}
		}

		//Remove small inner skirt holes. Holes have a negative area, remove anything smaller then 100x extrusion "area"
		for (unsigned int n = 0; n < skirtPolygons.size(); n++)
		{
			double area = skirtPolygons[n].area();
			if (area < 0 && area > -extrusionWidth * extrusionWidth * 100)
				skirtPolygons.remove(n--);
		}

		storage->skirt.add(skirtPolygons);

		int lenght = storage->skirt.polygonLength();
		if (skirtNr + 1 >= count && lenght > 0 && lenght < minLength)
			count++;
	}
}

void SliceCalculator::generateRaft(ModelDataStorage* storage, ConfigSettings config)
{
	storage->raftOutline.clear();

	int adhesionValue = config.platform_adhesion;
	if (adhesionValue != Generals::PlatformAdhesion::Raft)
		return;

	int distance = config.raft_margin;
	int extrusionWidth = config.extrusion_width;
	int wipeTowerRaftMargin = config.wipe_tower_raft_margin;

	for (auto vit = models.begin(); vit != models.end(); ++vit)
	{
		if ((*vit)->sliceLayers.size() == 0)
			continue;
		SliceLayer* layer = &(*vit)->sliceLayers[0];
		for (unsigned int i = 0; i < layer->parts.size(); i++)
		{
			storage->raftOutline = storage->raftOutline.unionPolygons(layer->parts[i].outline.offset(distance));
		}
	}

	// 	for (unsigned int i = 0; i < storage->layers[0].parts.size(); i++)
	// 		storage->raftOutline = storage->raftOutline.unionPolygons(storage->layers[0].parts[i].outline.offset(distance));

	if (storage->supportGenerated())
	{
		SupportPolygonGenerator supportGenerator(storage->supportData);
		engine::Polygons supportPolygons = supportGenerator.generateSupportPolygons(0);
		//매우 작은 support영역은 raft 생성하지 않도록 하기..
		supportPolygons = supportPolygons.offset(-extrusionWidth * 3);
		supportPolygons = supportPolygons.offset(extrusionWidth * 3);
		//support 외곽 영역 -> raft outline에 추가..
		storage->raftOutline = storage->raftOutline.unionPolygons(supportPolygons.offset(distance));
	}

	//wipeTower 외곽 영역 -> raft outline에 추가..
	//storage->raftOutline = storage->raftOutline.unionPolygons(storage->wipeTower.offset(wipeTowerRaftMargin));
	if (!storage->wipeTower_layers.empty())
		storage->raftOutline = storage->raftOutline.unionPolygons(storage->wipeTower_layers.at(0).wipeTower_mainOutline.offset(wipeTowerRaftMargin));
}


void SliceCalculator::generateAdjustZGap(ModelDataStorage* storage, ConfigSettings config)
{
	storage->adjustZGapOutline_origin.clear();
	storage->adjustZGapOutline_moved.clear();

	int distance = config.raft_margin;
	int extrusionWidth = config.extrusion_width;
	int wipeTowerRaftMargin = config.wipe_tower_raft_margin;
	int adjustPosition = config.adjust_postion;

	//adjust Z Gap line : origin 준비..//
	for (auto vit = models.begin(); vit != models.end(); ++vit)
	{
		if ((*vit)->sliceLayers.size() == 0)	continue;
		SliceLayer* layer = &(*vit)->sliceLayers[0];
		for (unsigned int i = 0; i < layer->parts.size(); i++)
		{
			storage->adjustZGapOutline_origin = storage->adjustZGapOutline_origin.unionPolygons(layer->parts[i].outline.offset(distance));
		}
	}

	if (storage->supportGenerated())
	{
		SupportPolygonGenerator supportGenerator(storage->supportData);
		engine::Polygons supportPolygons = supportGenerator.generateSupportPolygons(0);
		//매우 작은 support영역은 raft 생성하지 않도록 하기..
		supportPolygons = supportPolygons.offset(-extrusionWidth * 3);
		supportPolygons = supportPolygons.offset(extrusionWidth * 3);

		//support 외곽 영역 -> raft outline에 추가..
		storage->adjustZGapOutline_origin = storage->adjustZGapOutline_origin.unionPolygons(supportPolygons.offset(distance));
	}

	//wipeTower 외곽 영역 -> adjustZgap outline에 추가..
	if (!storage->wipeTower_layers.empty())
		storage->adjustZGapOutline_origin = storage->adjustZGapOutline_origin.unionPolygons(storage->wipeTower_layers.at(0).wipeTower_mainOutline.offset(wipeTowerRaftMargin));


	//adjust Z Gap line : moved 준비..//
	storage->adjustZGapOutline_moved = storage->adjustZGapOutline_origin;

	//adjust layer position offset//
	// 0 : left
	// 1 : right --> x offset
	if (adjustPosition)
	{
		for (int i = 0; i < storage->adjustZGapOutline_origin.size(); ++i)
		{
			engine::Polygon polygonsPart = storage->adjustZGapOutline_origin[i];

			for (int j = 0; j < polygonsPart.size(); ++j)
			{
				polygonsPart[j].X = polygonsPart[j].X + Profile::getMachineWidth_calculated() * 1000 * 0.5;

				storage->adjustZGapOutline_moved[i][j].X = polygonsPart[j].X;
			}
		}

	}


}

bool SliceCalculator::checkPolygonsPathRange(engine::Polygons polygons, const int margin)
{
	Point minRange, maxRange;

	if (Profile::machineProfile.machine_center_is_zero.value)
	{
		maxRange.X = Profile::getMachineWidth_calculated() * 1000 * 0.5;
		maxRange.Y = Profile::getMachineDepth_calculated() * 1000 * 0.5;

		minRange.X = -Profile::getMachineWidth_calculated() * 1000 * 0.5;
		minRange.Y = -Profile::getMachineDepth_calculated() * 1000 * 0.5;
	}
	else
	{
		maxRange.X = Profile::getMachineWidth_calculated() * 1000;
		maxRange.Y = Profile::getMachineDepth_calculated() * 1000;

		minRange.X = 0;
		minRange.Y = 0;
	}

	int widthOffset = Profile::machineProfile.machine_width_offset.value * 1000;
	int depthOffset = Profile::machineProfile.machine_depth_offset.value * 1000;

	minRange.X = minRange.X + widthOffset;
	minRange.Y = minRange.Y + depthOffset;
	maxRange.X = maxRange.X + widthOffset;
	maxRange.Y = maxRange.Y + depthOffset;


	for (int i = 0; i < polygons.size(); ++i)
	{
		for (int j = 0; j < polygons[i].size(); ++j)
		{
			if (polygons[i][j].X < minRange.X + margin || polygons[i][j].Y < minRange.Y + margin)
				return false;
			if (polygons[i][j].X > maxRange.X - margin || polygons[i][j].Y > maxRange.Y - margin)
				return false;
		}
	}

	return true;
}
