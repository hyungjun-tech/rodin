#include "stdafx.h"
#include "SliceProcessor.h"
#include "ModelContainer.h"
#include "settings.h"
#include "CartridgeInfo.h"
#include "ProgressHandler.h"

SliceProcessor::SliceProcessor(QWidget *parent_)
	: progressHandler(new ProgressHandler(parent_))
	, calculator(nullptr)
	, b_errorCode(0)
	, abort(false)
{
	dataStorage = new ModelDataStorage();
	progressHandler->setWindowTitle(MessageProgress::tr("Slicing.."));
	connect(this, SIGNAL(signal_setLabelText(QString)), progressHandler, SLOT(setLabelText(QString)));
	connect(this, SIGNAL(signal_setValue(int)), progressHandler, SLOT(setValue(int)));
}

SliceProcessor::SliceProcessor()
	: progressHandler(nullptr)
	, calculator(nullptr)
	, b_errorCode(0)
	, abort(false)
{
	dataStorage = new ModelDataStorage();
}

SliceProcessor::~SliceProcessor()
{
	delete dataStorage;
	if (calculator)
		delete calculator;
	if (progressHandler)
		delete progressHandler;
}

void SliceProcessor::init(ModelContainer* modelContainer_, bool forSelectedModels_)
{
	//modelContainer = modelContainer_;
	if (forSelectedModels_)
		models = modelContainer_->getSelectedModels();
	else
		models = modelContainer_->getBasicModels();

	setConfig(Profile::configSettings);
	calculator = new SliceCalculator(models);
	dataStorage->supportData = modelContainer_->supportData;
}

void SliceProcessor::setConfig(vector<ConfigSettings>& p_configs)
{
	int cartridgeCount = p_configs.size();
	if (cartridgeCount == 0)
		return;

	configs.clear();
	configs = p_configs;
}

bool SliceProcessor::processingForSupportEdit()
{
	//positioning();
	//generatingSupportDataStructure();
	//서포트옵션과 상관없이 무조건 서포트 구역을 계산해야 할때
	/*ConfigSettings config = configs.front();
	dataStorage->supportData->generateSupportGrid(models, config.supportAngle, config.supportEverywhere, config.support_xy_distance, config.supportZDistance, config.support_horizontal_expansion, config.layer_height);*/
	generatingSupportDataStructure();
	return true;
}
bool SliceProcessor::processing()
{
	//positioning();
	//generatingSupportDataStructure();
	generatingSupportDataStructure();

	b_overhang = dataStorage->supportData->checkOverhang();
	if (b_overhang)
		std::printf("there is overhang\n");
	else
		std::printf("there isn't overhang\n");

	//don't need below code for checking max z height because of new wipe-tower modification..//
	//if (!checkModelMaxSizeZforwipetowerSize())
	//	return false;

	if (!generatingPolygonLayers())
		return false;
	if (!generatingSliceLayers())
	{
		/*if (b_memoryOver)
			return 1;
		else*/
			return false;
	}

	if (!generatingSupportLayers())
		return false;
	if (!applyingRaftThickness())
		return false;
	if (!generatingInsetFirstLayer())
		return false;
	if (!generatingInset())
		return false;

	if (!generatingSkinAndSparse())
		return false;

	if (!generatingWipeTower())
		return false;

	if (!generatingSkirtRaft())
		return false;

	//skip oozzeshield..//

	return (!abort);
}

QString SliceProcessor::getWarningMessage()
{
	if (UserProperties::thinMessageCheck)
		return "";
	
	if (b_thin || b_overhang)
	{
		QString messageStr;
		if (b_thin)
			messageStr = MessageAlert::tr("thin_shape_alert");
		if (b_overhang)
		{
			if (!messageStr.isEmpty())
				messageStr.append("\n");
			messageStr.append(MessageAlert::tr("overhang_region_exists"));
		}
		return messageStr;
	}
	return "";
}
/*error code
1 : Memory Over
2 : wipe_tower_out_of_range
3 : skirt_out_of_range
4 : brim_out_of_range
5 : raft_out_of_range
*/
QString SliceProcessor::getErrorMessage()
{
	QString messageStr;
	switch (b_errorCode)
	{
	case 0:
		return "";
		break;
	case 1:
		messageStr = "Memory Over";
	case 2:
		messageStr = MessageError::tr("wipe_tower_out_of_range");
		break;
	case 3:
		messageStr = MessageError::tr("skirt_out_of_range");
		break;
	case 4:
		messageStr = MessageError::tr("brim_out_of_range");
		break;
	case 5:
		messageStr = MessageError::tr("raft_out_of_range");
		break;
	default:
		break;
	}
	return messageStr;
}

bool SliceProcessor::processingForPrintOptimum()
{
	if (models.size() < 1)
		return false;
	//generatingSupportDataStructure();
	if (!generatingPolygonLayers())
		return false;

	IMeshModel* model = models.front();
	int totalLayers = model->polygonLayers.size();
	int maxThreadNumb = omp_get_max_threads();
	int progressMax = totalLayers / maxThreadNumb;
	omp_set_num_threads(maxThreadNumb);

	model->sliceLayers.clear();
	model->sliceLayers.resize(totalLayers);

	//qDebug() << "polygonLayers size : " << model->polygonLayers.size();
#pragma omp parallel for
	for (int layerNr = 0; layerNr < totalLayers; layerNr++)
	{
		if (!abort)
		{
			if (omp_get_thread_num() == 0)
			{
				int percent = (float)layerNr / progressMax * 100;
				emit signal_setValue(percent);
			}
			model->sliceLayers[layerNr].sliceZ = model->polygonLayers[layerNr].z;
			model->sliceLayers[layerNr].printZ = model->polygonLayers[layerNr].z;
			model->polygonLayers[layerNr].createLayerWithParts(model->sliceLayers[layerNr],
				configs[0].fix_horrible & (FIX_HORRIBLE_UNION_ALL_TYPE_A | FIX_HORRIBLE_UNION_ALL_TYPE_B | FIX_HORRIBLE_UNION_ALL_TYPE_C));

			model->polygonLayers[layerNr].clearPolygonData();
		}
	}
	return !abort;
}

//to do sj
//왜 해야하는지 모르겠음.........
//editPlaneOffsets는 나중에 필요할때 계산해서 쓰도록. (draw할때 필요함)
void SliceProcessor::positioning()
{
	/*int tic = clock();

	Point3 vOffset;
	if (configs[0].autoCenter == 1)
	{
		AABB aabb = AABBGetter()(modelContainer->models);
		qglviewer::Vec modelMax = aabb.GetMaximum();
		qglviewer::Vec modelMin = aabb.GetMinimum();
		vOffset = Point3((modelMin.x + modelMax.x) / 2, (modelMin.y + modelMax.y) / 2, modelMin.z);
		Point3 center(configs[0].objectPosition.X, configs[0].objectPosition.Y, -configs[0].objectSink);
		vOffset -= center;
	}
	else
		vOffset = Point3(configs[0].objectPosition.X, configs[0].objectPosition.Y, configs[0].objectSink);

	editPlaneOffset = FPoint3(INT2MM(vOffset.x), INT2MM(vOffset.y), INT2MM(vOffset.z));

	for (auto vit = model->volumes.begin(); vit != model->volumes.end(); ++vit)
	{
		if (vOffset != Point3(0, 0, 0))
		{
			for (unsigned int n = 0; n < vit->points.size(); n++)
				vit->points[n].transformed_p -= vOffset;
		}
	}

	std::printf("positioning time : %lfsec\n", (clock() - tic) / (double)CLOCKS_PER_SEC);*/
}


void SliceProcessor::generatingSupportDataStructure()
{
	int tic = clock();
	if (dataStorage->supportData->b_changed) //모델에 변경 내용이 있을때만 수행
	{
		ConfigSettings config = configs.front();
		dataStorage->supportData->generateSupportGrid(models, config.support_angle, config.support_everywhere, config.support_xy_distance, config.support_z_distance, config.support_horizontal_expansion, config.layer_height);
	}
	//support 구역을 재생성하지 않더라도 support는 무조건 체크.
	dataStorage->supportData->checkNeedSupport();
	std::printf("supporting time : %lfsec\n", (clock() - tic) / (double)CLOCKS_PER_SEC);
}

bool SliceProcessor::generatingPolygonLayers()
{
	if (abort)
		return false;
	int32_t initial_layerThickness = configs[0].initial_layer_height;
	int32_t layerThickness = configs[0].layer_height;
	bool keepNoneClosed = configs[0].fix_horrible & FIX_HORRIBLE_KEEP_NONE_CLOSED;
	bool extensiveStitching = configs[0].fix_horrible & FIX_HORRIBLE_EXTENSIVE_STITCHING;
	int unionAllType = configs[0].fix_horrible & (FIX_HORRIBLE_UNION_ALL_TYPE_A | FIX_HORRIBLE_UNION_ALL_TYPE_B | FIX_HORRIBLE_UNION_ALL_TYPE_C);
	bool clearSegList = true;

	for (std::vector<IMeshModel*>::iterator vit = models.begin(); vit != models.end(); ++vit)
	{
		double modelMaxZ = (*vit)->getAABB().getMaximum().z * 1000;
		double modelMinZ = (*vit)->getAABB().getMinimum().z * 1000;
		if (modelMaxZ - initial_layerThickness < 0)
			initial_layerThickness = modelMaxZ;

		//modelMaxZ값의 유효수 범위를 scale dialog UI와 동일하게 맞춤. decimal 2 효과 --> *1000을 한 다음에 일의자리에서 반올림것과 같음.//swyang
		const int layerCount = (Generals::roundDouble(modelMaxZ, -1) - initial_layerThickness) / layerThickness + 1;

		if (layerCount == 0)
			continue;

		(*vit)->polygonLayers.clear();
		(*vit)->polygonLayers.resize(layerCount);

		//slicing tolerance : MIDDLE --> cura 4.2 concept//
		(*vit)->polygonLayers[0].z = initial_layerThickness / 2;
		const int adjusted_layer_offset = initial_layerThickness + (layerThickness / 2);

#pragma omp parallel for
		for (int layerNr = 1; layerNr < layerCount; layerNr++)
			(*vit)->polygonLayers[layerNr].z = adjusted_layer_offset + (layerThickness * (layerNr - 1));

		Mesh * outer_ = (*vit)->getMesh();
		for (Mesh::FaceIter f_it = outer_->faces_begin(); f_it != outer_->faces_end(); ++f_it)
		{
			if (abort)
				return false;
			Mesh::FaceHalfedgeIter fh_it = outer_->fh_iter(f_it.handle());
			std::vector<Mesh::Point> points;
			for (; fh_it.is_valid(); ++fh_it) {
				Mesh::VertexHandle vh = outer_->from_vertex_handle(*fh_it);
				points.push_back(outer_->point(vh) * 1000);
			}
			if (points.size() != 3)
				continue;
			double minZ = modelMaxZ;
			double maxZ = modelMinZ;
			
			//현재 face의 minz, maxz구함
			for (std::vector<Mesh::Point>::iterator it = points.begin(); it != points.end(); ++it)
			{
				if ((*it)[2] < minZ) minZ = (*it)[2];
				if ((*it)[2] > maxZ) maxZ = (*it)[2];
			}

			int maxCnt = ceil((float)(maxZ - adjusted_layer_offset) / layerThickness) + 1;
			int minCnt = floor((float)(minZ - adjusted_layer_offset) / layerThickness) + 1;
			if (minCnt < 0) minCnt = 0;
			if (maxCnt > (*vit)->polygonLayers.size()) maxCnt = (*vit)->polygonLayers.size();
#pragma omp parallel for
			for (int layerNr = 0; layerNr < (*vit)->polygonLayers.size(); layerNr++)
			{
				if (!(*vit)->polygonLayers[layerNr].makeSegmentList(minZ, maxZ, points, f_it->idx()))
					continue;
			}
		}
#pragma omp parallel for
		for (int layerNr = 0; layerNr < (*vit)->polygonLayers.size(); layerNr++)
		{
			(*vit)->polygonLayers[layerNr].makePolygons(outer_, keepNoneClosed, extensiveStitching, clearSegList);
			(*vit)->polygonLayers[layerNr].clearSegmentData();
		}
	}
	return (!abort);
}

bool SliceProcessor::generatingSliceLayers()
{
	if (abort)
		return false;
	int tic = clock();

	int totalLayers = 0;

	for (std::vector<IMeshModel*>::iterator vit = models.begin(); vit != models.end(); ++vit)
	{
		(*vit)->sliceLayers.clear();
		(*vit)->sliceLayers.resize((*vit)->polygonLayers.size());

		if ((*vit)->polygonLayers.size() > totalLayers)
		{
			totalLayers = (*vit)->polygonLayers.size();
		}
	}

	int maxThreadNumb = omp_get_max_threads();
	int progressMax = totalLayers / maxThreadNumb;
	int progressValue = 0;
	omp_set_num_threads(maxThreadNumb);

	emit signal_setLabelText(MessageProgress::tr("Generating polygons.."));

	dataStorage->totalLayer = totalLayers;
	// 	model->layers.clear();
	// 	model->layers.resize(totalLayers);
	// 	Slicer sl;
	// 	sl.layers.resize(totalLayers);

#pragma omp parallel for
	for (int i = 0; i < totalLayers; ++i)
	{
//#pragma omp flush (abort)
		if (!abort)
		{
			if (progressHandler)
			{
				if (progressHandler->wasCanceled())
				{
					//dataStorage->b_slicing_flag = false;
					std::printf("canceled while generating polygons!\n");
					abort = true;
//#pragma  omp flush(abort)
				}
			}

			if (omp_get_thread_num() == 0)
			{
				emit signal_setValue(((double)i / (double)progressMax * 100));
			}

			for (std::vector<IMeshModel*>::iterator it = models.begin(); it != models.end(); ++it)
			{
				if (i < (*it)->polygonLayers.size())
				{
					(*it)->sliceLayers[i].sliceZ = (*it)->polygonLayers[i].z;
					(*it)->sliceLayers[i].printZ = (*it)->polygonLayers[i].z;

					// 					sl.layers[i].polygonList = sl.layers[i].polygonList.unionPolygons(it->slicer.layers[i].polygonList);
					// 					sl.layers[i].openPolygons = sl.layers[i].openPolygons.unionPolygons(it->slicer.layers[i].openPolygons);
					(*it)->polygonLayers[i].createLayerWithParts((*it)->sliceLayers[i],
						configs[0].fix_horrible & (FIX_HORRIBLE_UNION_ALL_TYPE_A | FIX_HORRIBLE_UNION_ALL_TYPE_B | FIX_HORRIBLE_UNION_ALL_TYPE_C));

					(*it)->polygonLayers[i].clearPolygonData();
				}
			}
			// 			sl.createLayerWithParts(model->layers[i], &sl.layers[i],
			// 				config.fixHorrible & (FIX_HORRIBLE_UNION_ALL_TYPE_A | FIX_HORRIBLE_UNION_ALL_TYPE_B | FIX_HORRIBLE_UNION_ALL_TYPE_C));

		}
	}
	emit signal_setValue(100);

	std::printf("slicing time : %lfsec\n", (clock() - tic) / (double)CLOCKS_PER_SEC);
	return !abort;
}

bool SliceProcessor::generatingSupportLayers()
{
	if (abort)
		return false;
	dataStorage->support_layers.clear();
	if (!dataStorage->supportGenerated())
		return true;

	int totalLayers = dataStorage->totalLayer;

	for (unsigned int layerNr = 0; layerNr < totalLayers; layerNr++)
	{
		if (abort)
			return false;
		dataStorage->support_layers.push_back(SupportLayer());

		int cart_index;

		if (Generals::isLayerColorModeOn())
		{
			cart_index = CartridgeInfo::getCartIndexFromCartridgeLayerList(layerNr);
		}
		else
		{
			cart_index = configs[0].support_main_cartridge_index;
		}

		int32_t z = configs[0].initial_layer_height + layerNr * configs[0].layer_height;

		//volume별로 하지 않고, 전체 model에서 가지고 있기로..//
		SupportPolygonGenerator supportGenerator(dataStorage->supportData);
		engine::Polygons supportPolygons = supportGenerator.generateSupportPolygons(z);
		for (auto vit = models.begin(); vit != models.end(); ++vit)
		{
			if (layerNr >= (*vit)->sliceLayers.size())
				continue;


			SliceLayer* layer = &((*vit)->sliceLayers[layerNr]);

			for (unsigned int n = 0; n < layer->parts.size(); n++)
			{
				//supportGenerator.polygons = supportGenerator.polygons.difference(layer->parts[n].outline.offset(configs[0].support_xy_distance));
				supportPolygons = supportPolygons.difference(layer->parts[n].outline.offset(configs[0].support_xy_distance));
			}
		}

		//Contract and expand the support Polygons so small sections are removed and the final Polygon is smoothed a bit.
		supportPolygons = supportPolygons.offset(-configs[cart_index].extrusion_width * 3);
		supportPolygons = supportPolygons.offset(configs[cart_index].extrusion_width * 3);


		//supportGenerator에서 생성된 polygons를 split하면 여러개의 support island로 생성함..//
		std::vector<engine::PolygonsPart> supportIslands = supportPolygons.splitIntoParts();

		for (unsigned int i = 0; i < supportIslands.size(); i++)
		{
			//SupportLayerPart 생성..//
			SupportLayerPart tempLayerPart;
			//SupportLayerPart outline에 넣기..//
			tempLayerPart.support_outline = supportIslands.at(i);
			//SupportLayerPart boundary box에 넣기..//
			tempLayerPart.support_boundaryBox.calculate(supportIslands.at(i));
			dataStorage->support_layers[layerNr].support_parts.push_back(tempLayerPart);
		}

		//path optimize 부분은 다른 곳에서...??//
		//engine::PathOrderOptimizer islandOrderOptimizer(gcode.getPositionXY());
		//for (unsigned int n = 0; n<supportIslands.size(); n++)
		//{
		//	islandOrderOptimizer.addPolygon(supportIslands[n][0]);
		//}
		//islandOrderOptimizer.optimize();
	}
	return (!abort);
}
bool SliceProcessor::applyingRaftThickness()
{
	if (abort)
		return false;
	int tic = clock();

	int offset0 = configs[0].raft_base_thickness + configs[0].raft_interface_thickness + configs[0].raft_surface_thickness * configs[0].raft_surface_layers + configs[0].raft_airgap_initial_layer;
	int offset = configs[0].raft_base_thickness + configs[0].raft_interface_thickness + configs[0].raft_surface_thickness * configs[0].raft_surface_layers + configs[0].raft_airgap_all;
	for (std::vector<IMeshModel*>::iterator it = models.begin(); it != models.end(); ++it)
	{
		if ((*it)->sliceLayers.size() != 0)
			(*it)->sliceLayers[0].printZ = (*it)->sliceLayers[0].sliceZ + offset0;
		for (unsigned int layerNr = 1; layerNr < (*it)->sliceLayers.size(); layerNr++)
			(*it)->sliceLayers[layerNr].printZ = (*it)->sliceLayers[layerNr].sliceZ + offset;
	}

	std::printf("applyingRaftThickness time : %lfsec\n", (clock() - tic) / (double)CLOCKS_PER_SEC);
	return (!abort);
}

bool SliceProcessor::generatingInsetFirstLayer()
{
	if (abort)
		return false;
	int tic = clock();

	//generate support inset//
	int support_cart_index = configs[0].support_main_cartridge_index;

	for (auto it : models)
	{
		int cart_index = it->getCartridgeIndexes()[0];
		if (it->sliceLayers.size() != 0)
			calculator->generateInsets(&it->sliceLayers[0], configs[cart_index].initial_layer_extrusion_width, configs[cart_index].inset_count, configs[cart_index].internal_moving_area, configs[0].simple_mode);
		if (dataStorage->support_layers.size() != 0)
			calculator->generateSupportInsets(&dataStorage->support_layers[0], configs[support_cart_index].initial_layer_extrusion_width, configs[support_cart_index].inset_count);
	}

	std::printf("generatingInsetFirstLayer time : %lfsec\n", (clock() - tic) / (double)CLOCKS_PER_SEC);
	return (!abort);
}

bool SliceProcessor::generatingInset(bool test_thin)
{
	if (abort)
		return false;
	int tic = clock();

	int maxThreadNumb = omp_get_max_threads();
	int totalLayer = dataStorage->totalLayer;
	int progressMax = totalLayer / maxThreadNumb;
	int progressValue = 0;
	omp_set_num_threads(maxThreadNumb);

	emit signal_setLabelText(MessageProgress::tr("Generating Inset.."));

	b_thin = false;
#pragma omp parallel for
	for (int layerNr = 1; layerNr < totalLayer; layerNr++)
	{
//#pragma omp flush (abort)
		if (!abort)
		{
			if (progressHandler)
			{
				if (progressHandler->wasCanceled())
				{
					//dataStorage->b_slicing_flag = false;
					std::printf("canceled while generating polygons!\n");
					abort = true;
//#pragma  omp flush(abort)
				}
			}

			if (omp_get_thread_num() == 0)
			{
				emit signal_setValue(((double)layerNr / (double)progressMax * 100));
			}

			///////////////////////////////////////////////////////////////////////////////
			//generate layer inset//

			for (std::vector<IMeshModel*>::iterator it = models.begin(); it != models.end(); ++it)
			{
				int cart_index = (*it)->getCartridgeIndexes()[0];

				int insetCount = configs[cart_index].inset_count;
				if (configs[0].spiralize_mode && static_cast<int>(layerNr) < configs[cart_index].down_skin_count && layerNr % 2 == 1)//Add extra insets every 2 layers when spiralizing, this makes bottoms of cups watertight.
					insetCount += 5;

				if (layerNr < (*it)->sliceLayers.size())
					calculator->generateInsets(&(*it)->sliceLayers[layerNr], configs[cart_index].wall_extrusion_width, insetCount, configs[cart_index].internal_moving_area, configs[0].simple_mode);
			}
			///////////////////////////////////////////////////////////////////////////////

			if (dataStorage->supportGenerated())
			{
				///////////////////////////////////////////////////////////////////////////////
				//generate support inset//
				int support_cart_index = configs[0].support_main_cartridge_index;

				//int insetCount = configs[support_cart_index].inset_count;
				//if (configs[0].spiralize_mode && static_cast<int>(layerNr) < configs[support_cart_index].down_skin_count && layerNr % 2 == 1)//Add extra insets every 2 layers when spiralizing, this makes bottoms of cups watertight.
				//	insetCount += 5;
				//if (layerNr < vit->layers.size())

				//support inset은 0으로 해야 support ouline을 기준으로 생성됨..//
				calculator->generateSupportInsets(&dataStorage->support_layers[layerNr], configs[support_cart_index].support_main_extrusion_width, 0);
				///////////////////////////////////////////////////////////////////////////////
			}

			//얇은 형상 검출..//
			if (!b_thin && test_thin && !configs[0].spiralize_mode && !configs[0].simple_mode)
			{
				for (std::vector<IMeshModel*>::iterator it = models.begin(); it != models.end(); ++it)
				{
					int cart_index = (*it)->getCartridgeIndexes()[0];

					if (layerNr < (*it)->sliceLayers.size())
					{
						for (int i = 0; i < (*it)->sliceLayers[layerNr].parts.size(); ++i)
						{
							engine::Polygons p = (*it)->sliceLayers[layerNr].parts[i].insets[0].offset(configs[cart_index].extrusion_width*1.5);
							engine::Polygons p2 = (*it)->sliceLayers[layerNr].parts[i].outline.difference(p);
							if (p2.size() != 0)
							{
								b_thin = true;
								//break;
							}
						}
					}
				}
				/*
				if (b_thin)
				{
				parent->commonDlg->setDialogContents("Output may not be correct due to thin shape.\nAre you sure you want to continue slicing?", CommonDialog::Question, false, false, true, true);
				parent->commonDlg->exec();
				if (parent->commonDlg->isYes()) {}
				else if (parent->commonDlg->isNo()) { return false; }
				}
				*/
			}
		}
	}
	emit signal_setValue(100);

	std::printf("generatingInset time : %lfsec\n", (clock() - tic) / (double)CLOCKS_PER_SEC);
	return !abort;
}

bool SliceProcessor::generatingSkinAndSparse()
{
	if (configs[0].spiralize_mode || configs[0].simple_mode)
		return true;

	int tic = clock();

	int maxThreadNumb = omp_get_max_threads();
	int totalLayer = dataStorage->totalLayer;
	int progressMax = totalLayer / maxThreadNumb;
	int progressValue = 0;
	omp_set_num_threads(maxThreadNumb);

	emit signal_setLabelText(MessageProgress::tr("Generating Skin.."));

#pragma omp parallel for
	for (int layerNr = 0; layerNr < totalLayer; layerNr++)
	{
//#pragma omp flush (abort)
		if (!abort)
		{
			if (progressHandler)
			{
				if (progressHandler->wasCanceled())
				{
					//dataStorage->b_slicing_flag = false;
					std::printf("canceled while generating polygons!\n");
					abort = true;
//#pragma  omp flush(abort)
				}
			}

			if (omp_get_thread_num() == 0)
			{
				emit signal_setValue(((double)layerNr / (double)progressMax * 100));
			}

			//////////////////////////////////////////////////////////////////////////////////////
			//generating volume layer skin and sparse//
			for (auto vit = models.begin(); vit != models.end(); ++vit)
			{
				int cart_index = (*vit)->getCartridgeIndexes()[0];

				if (!configs[0].spiralize_mode || static_cast<int>(layerNr) < configs[cart_index].down_skin_count)    //Only generate up/downskin and infill for the first X layers when spiralize is choosen.
				{
					int extrusion_width = configs[cart_index].wall_extrusion_width;
					if (layerNr == 0)
						extrusion_width = configs[cart_index].initial_layer_extrusion_width;

					if (layerNr < (*vit)->sliceLayers.size())
					{
						calculator->generateSkins(layerNr, &(*vit)->sliceLayers, extrusion_width, configs[cart_index].down_skin_count, configs[cart_index].up_skin_count, configs[cart_index].infill_overlap);
						calculator->generateSparse(&(*vit)->sliceLayers[layerNr], extrusion_width);
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////

			if (dataStorage->supportGenerated())
			{
				//////////////////////////////////////////////////////////////////////////////////////
				//generating support skin and sparse//
				int support_cart_index;

				if (Generals::isLayerColorModeOn())
				{
					support_cart_index = CartridgeInfo::getCartIndexFromCartridgeLayerList(layerNr);
				}
				else
				{
					support_cart_index = configs[0].support_main_cartridge_index;
				}

				//if (static_cast<int>(layerNr) > configs[support_cart_index].down_skin_count)    //Only generate up/downskin and infill for the first X layers when spiralize is choosen.
				//{
				int support_interface_roof_extrusion_width = configs[support_cart_index].support_interface_roof_extrusion_width;
				int support_interface_floor_extrusion_width = configs[support_cart_index].support_interface_floor_extrusion_width;

				if (layerNr == 0)
				{
					support_interface_roof_extrusion_width = configs[support_cart_index].initial_layer_extrusion_width;
					support_interface_floor_extrusion_width = configs[support_cart_index].initial_layer_extrusion_width;
				}

				calculator->generateSupportInterface(layerNr, &dataStorage->support_layers, support_interface_roof_extrusion_width, support_interface_floor_extrusion_width, configs[support_cart_index].support_interface_roof_layers_count, configs[support_cart_index].support_interface_floor_layers_count, configs[support_cart_index].support_infill_overlap);
				calculator->generateSupportSparse(layerNr, &dataStorage->support_layers, support_interface_roof_extrusion_width, support_interface_floor_extrusion_width, configs[support_cart_index].support_interface_roof_layers_count, configs[support_cart_index].support_interface_floor_layers_count);
			}

			//}
			//////////////////////////////////////////////////////////////////////////////////////

		}
	}
	emit signal_setValue(100);

	std::printf("generatingSkinAndSparse time : %lfsec\n", (clock() - tic) / (double)CLOCKS_PER_SEC);
	return !abort;
}

bool SliceProcessor::generatingWipeTower()
{
	if (abort)
		return false;
	dataStorage->wipeTowerClear();
	if (!configs[0].wipe_tower_enable)
		return true;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//wipe-tower variations initializing..//
	AABB aabb = AABBGetter()(models);
	qglviewer::Vec max = aabb.getMaximum() * 1000;
	qglviewer::Vec min = aabb.getMinimum() * 1000;
	qglviewer::Vec center = aabb.getFloorCenter() * 1000;

	const Point model_min = Point(min[0], min[1]);
	const Point model_max = Point(max[0], max[1]);
	const Point model_center = Point(center[0], center[1]);

	const int space = MM2INT(5);

	const int wipeTowerBaseSize = configs[0].wipe_tower_base_size;
	const int wipeTowerBaseLayerCount = configs[0].wipe_tower_base_layer_count;
	const int wipeTowerOuterSize = configs[0].wipe_tower_outer_size;
	const int wipeTowerOuterWallThickness = configs[0].wipe_tower_outer_wall_thickness;
	const int wipeTowerOuterInnerGap = configs[0].wipe_tower_outer_inner_gap;
	const int wipeTowerInnerSize = configs[0].wipe_tower_inner_size;

	const int wipeTowerOuterSizeHalf = wipeTowerOuterSize * 0.5;
	const int wipeTowerBaseSizeHalf = wipeTowerBaseSize * 0.5;
	const int wipeTowerInnerSizeHalf = wipeTowerInnerSize * 0.5;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//wipe_tower center calculating..//
	Point wipeTowerCenter;
	int positionOffset = space + (wipeTowerBaseSize * 0.5);

	switch (configs[0].wipe_tower_position)
	{
	case Generals::WipetowerPosition::RearLeft:
		wipeTowerCenter = Point(model_min.X - positionOffset, model_max.Y + positionOffset);
		break;
	case Generals::WipetowerPosition::RearCenter:
		wipeTowerCenter = Point(model_center.X, model_max.Y + positionOffset);
		break;
	case Generals::WipetowerPosition::RearRight:
		wipeTowerCenter = Point(model_max.X + positionOffset, model_max.Y + positionOffset);
		break;
	case Generals::WipetowerPosition::MiddleLeft:
		wipeTowerCenter = Point(model_min.X - positionOffset, model_center.Y);
		break;
	case Generals::WipetowerPosition::MiddleRight:
		wipeTowerCenter = Point(model_max.X + positionOffset, model_center.Y);
		break;
	case Generals::WipetowerPosition::FrontLeft:
		wipeTowerCenter = Point(model_min.X - positionOffset, model_min.Y - positionOffset);
		break;
	case Generals::WipetowerPosition::FrontCenter:
		wipeTowerCenter = Point(model_center.X, model_min.Y - positionOffset);
		break;
	case Generals::WipetowerPosition::FrontRight:
		wipeTowerCenter = Point(model_max.X + positionOffset, model_min.Y - positionOffset);
		break;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//wipe-tower type//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::vector<WipeTowerLayer>* wipeTower_layers = &dataStorage->wipeTower_layers;


	//wipe_tower outline & sub-outline setting//
	for (unsigned int layerNr = 0; layerNr < dataStorage->totalLayer; layerNr++)
	{
		dataStorage->wipeTower_layers.push_back(WipeTowerLayer());

		engine::PolygonRef p_main_outline = dataStorage->wipeTower_layers.at(layerNr).wipeTower_mainOutline.newPoly();
		engine::PolygonRef p_main_innerWall = dataStorage->wipeTower_layers.at(layerNr).wipeTower_mainInnerWall.newPoly();
		engine::PolygonRef p_sub_outline = dataStorage->wipeTower_layers.at(layerNr).wipeTower_subOutline.newPoly();

		int mainOutlineOffset = 0;
		if (layerNr < wipeTowerBaseLayerCount)
			mainOutlineOffset = wipeTowerBaseSizeHalf;
		else if (layerNr >= wipeTowerBaseLayerCount && layerNr < (wipeTowerBaseLayerCount + 5))
			mainOutlineOffset = wipeTowerOuterSizeHalf + configs[0].extrusion_width * ((wipeTowerBaseLayerCount + 5) - layerNr);
		else
			mainOutlineOffset = wipeTowerOuterSizeHalf;

		p_main_outline.add(Point(wipeTowerCenter.X + mainOutlineOffset, wipeTowerCenter.Y - mainOutlineOffset));
		p_main_outline.add(Point(wipeTowerCenter.X + mainOutlineOffset, wipeTowerCenter.Y + mainOutlineOffset));
		p_main_outline.add(Point(wipeTowerCenter.X - mainOutlineOffset, wipeTowerCenter.Y + mainOutlineOffset));
		p_main_outline.add(Point(wipeTowerCenter.X - mainOutlineOffset, wipeTowerCenter.Y - mainOutlineOffset));

		p_main_innerWall.add(Point(wipeTowerCenter.X + wipeTowerOuterSizeHalf - wipeTowerOuterWallThickness, wipeTowerCenter.Y - wipeTowerOuterSizeHalf + wipeTowerOuterWallThickness));
		p_main_innerWall.add(Point(wipeTowerCenter.X + wipeTowerOuterSizeHalf - wipeTowerOuterWallThickness, wipeTowerCenter.Y + wipeTowerOuterSizeHalf - wipeTowerOuterWallThickness));
		p_main_innerWall.add(Point(wipeTowerCenter.X - wipeTowerOuterSizeHalf + wipeTowerOuterWallThickness, wipeTowerCenter.Y + wipeTowerOuterSizeHalf - wipeTowerOuterWallThickness));
		p_main_innerWall.add(Point(wipeTowerCenter.X - wipeTowerOuterSizeHalf + wipeTowerOuterWallThickness, wipeTowerCenter.Y - wipeTowerOuterSizeHalf + wipeTowerOuterWallThickness));

		//sub-outline setting..//
		p_sub_outline.add(Point(wipeTowerCenter.X + wipeTowerInnerSizeHalf, wipeTowerCenter.Y - wipeTowerInnerSizeHalf));
		p_sub_outline.add(Point(wipeTowerCenter.X + wipeTowerInnerSizeHalf, wipeTowerCenter.Y + wipeTowerInnerSizeHalf));
		p_sub_outline.add(Point(wipeTowerCenter.X - wipeTowerInnerSizeHalf, wipeTowerCenter.Y + wipeTowerInnerSizeHalf));
		p_sub_outline.add(Point(wipeTowerCenter.X - wipeTowerInnerSizeHalf, wipeTowerCenter.Y - wipeTowerInnerSizeHalf));

		if (layerNr == 0)
		{
			//0번 레이어 기준으로 skirt wipe tower layers 생성..한번만 입력하면 되므로..//
			dataStorage->wipeTower_mainOutline_for_skirt = dataStorage->wipeTower_layers.at(layerNr).wipeTower_mainOutline;

			//wipe point 생성..//
			dataStorage->wipeTopwer_points.push_back(Point(wipeTowerCenter.X + wipeTowerOuterSizeHalf - wipeTowerOuterWallThickness, wipeTowerCenter.Y - wipeTowerOuterSizeHalf + wipeTowerOuterWallThickness));
			dataStorage->wipeTopwer_points.push_back(Point(wipeTowerCenter.X + wipeTowerOuterSizeHalf - wipeTowerOuterWallThickness, wipeTowerCenter.Y + wipeTowerOuterSizeHalf - wipeTowerOuterWallThickness));
			dataStorage->wipeTopwer_points.push_back(Point(wipeTowerCenter.X - wipeTowerOuterSizeHalf + wipeTowerOuterWallThickness, wipeTowerCenter.Y + wipeTowerOuterSizeHalf - wipeTowerOuterWallThickness));
			dataStorage->wipeTopwer_points.push_back(Point(wipeTowerCenter.X - wipeTowerOuterSizeHalf + wipeTowerOuterWallThickness, wipeTowerCenter.Y - wipeTowerOuterSizeHalf + wipeTowerOuterWallThickness));
		}

		dataStorage->wipeTower_layers.at(layerNr).wipeTower_mainOutline = dataStorage->wipeTower_layers.at(layerNr).wipeTower_mainOutline.difference(dataStorage->wipeTower_layers.at(layerNr).wipeTower_mainInnerWall);

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	//check path range//
	const int wipetowerMargin = 0.5*sqrt(2)*configs[0].initial_layer_extrusion_width;

	if (!calculator->checkPolygonsPathRange(dataStorage->wipeTower_layers.at(0).wipeTower_mainOutline, wipetowerMargin))
	{
		b_errorCode = 2;
		return false;
	}

	return true;
}

bool SliceProcessor::generatingSkirtRaft()
{
	int tic = clock();

	int adhesionCartIndex = configs[0].adhesion_cartridge_index;
	int adhesionValue = configs[0].platform_adhesion;

	if (adhesionValue == Generals::PlatformAdhesion::Skirt || adhesionValue == Generals::PlatformAdhesion::Brim)
		calculator->generateSkirt(dataStorage, configs[0]);
	else if (Generals::PlatformAdhesion::Raft)
		calculator->generateRaft(dataStorage, configs[adhesionCartIndex]);

	if (Generals::isReplicationUIMode())
	{
		calculator->generateAdjustZGap(dataStorage, configs[adhesionCartIndex]);
	}

	//raft, skirt margin 추가//
	if (adhesionValue != Generals::PlatformAdhesion::NoneAdhesion)
	{
		const int skirtMargin = 0.5*sqrt(2)*configs[0].initial_layer_extrusion_width;
		const int raftMargin = 0.5*sqrt(2)*configs[adhesionCartIndex].raft_base_line_width;

		if (!calculator->checkPolygonsPathRange(dataStorage->skirt, skirtMargin))
		{
			if (configs[0].platform_adhesion == Generals::PlatformAdhesion::Skirt)
			{
				b_errorCode = 3;
				return false;
			}
			else
			{
				b_errorCode = 4;
				return false;
			}
		}
		if (!calculator->checkPolygonsPathRange(dataStorage->raftOutline, raftMargin))
		{
			b_errorCode = 5;
			return false;
		}
	}

	std::printf("generatingSkirtRaft time : %lfsec\n", (clock() - tic) / (double)CLOCKS_PER_SEC);

	return true;
}
