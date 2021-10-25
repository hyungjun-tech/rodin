#include "stdafx.h"
#include "PrintOptimum.h"
#include "ModelContainer.h"
#include "SaveFileUIMode.h"
#include "SliceProcessor.h"
#include "SupportData.h"
#include "ProgressHandler.h"

#ifndef DEG2RAD
#define DEG2RAD M_PI/180.0
#endif

PrintOptimum::PrintOptimum(QWidget* parent_)
	: parent(parent_)
	, modelContainer(nullptr)
	, checkCount(6)
	, progressHandler(nullptr)
{
	axis.resize(checkCount);
	axis[0] = QVector3D(1, 0, 0);
	axis[1] = QVector3D(1, 0, 0);
	axis[2] = QVector3D(1, 0, 0);
	axis[3] = QVector3D(1, 0, 0);
	axis[4] = QVector3D(0, 1, 0);
	axis[5] = QVector3D(0, 1, 0);

	angle.resize(checkCount);
	angle[0] = 0 * DEG2RAD;
	angle[1] = 180 * DEG2RAD;
	angle[2] = 90 * DEG2RAD;
	angle[3] = -90 * DEG2RAD;
	angle[4] = 90 * DEG2RAD;
	angle[5] = -90 * DEG2RAD;

	cost_supportFilamentLength.resize(checkCount);
	cost_overhangLength.resize(checkCount);
	cost_thicknessRigionLength.resize(checkCount);
	thumbnailImages.resize(checkCount);
}
PrintOptimum::~PrintOptimum()
{
}

void PrintOptimum::setProgressHandler(ProgressHandler* handler_)
{
	progressHandler = handler_;
}
bool PrintOptimum::optimize(ModelContainer *modelContainer_, double overhang_angle, ConfigSettings config_)
{
	if (modelContainer_ == nullptr)
		return false;
	modelContainer = modelContainer_;
	IMeshModel* model = modelContainer_->getSelectedModels().front();

	if (overhang_angle == 0)
		overhang_angle += 1; //overhang region 에서 error 방지 cos 면적 곱해주는게 있는데 cos = 0.0001인 경우 면적 만배

	QTime t;
	t.start();
	if (progressHandler)
	{
		progressHandler->setWindowTitle(MessageProgress::tr("Calculating.."));
		progressHandler->setLabelText(MessageProgress::tr("Optimized Print Direction Calculating.."));
		progressHandler->setValue(0);
	}

	for (int i = 0; i < axis.size(); ++i)
	{
		model->rotateAroundAPoint(angle[i] * axis[i][0], angle[i] * axis[i][1], angle[i] * axis[i][2]);

		//orienting();

		double overhangArea = 0;
		SupportData support;
		cost_supportFilamentLength[i] = support.calSupportVol(model, overhang_angle, 1, config_.support_xy_distance, config_.support_z_distance, config_.support_horizontal_expansion, overhangArea);
		cost_overhangLength[i] = overhangArea;

		int from, to;
		if (progressHandler)
		{
			if (progressHandler->wasCanceled())
			{
				model->rotateAroundAPoint(-angle[i] * axis[i][0], -angle[i] * axis[i][1], -angle[i] * axis[i][2]);
				std::cout << "canceled!" << std::endl;
				return false;
			}
			int from = i * 100 / axis.size();
			int to = (i + 1) * 100 / axis.size();
			progressHandler->setInitValue(from);
			progressHandler->setTargetValue(to);
		}
		bool result = slicing();
		if (progressHandler)
		{
			if (progressHandler->wasCanceled()) //여러번 하는 이유 --> process가 길 수 있기 때문에 cancel누르면 최대한 빨리 멈추라고
			{
				model->rotateAroundAPoint(-angle[i] * axis[i][0], -angle[i] * axis[i][1], -angle[i] * axis[i][2]);
				std::cout << "canceled!" << std::endl;
				return false;
			}
		}
		cost_thicknessRigionLength[i] = calcThickness(model->sliceLayers);

		// save thumbnail
		//thumbnailImages[i] = parentClass->ui.viewer->saveThumbnail_SelectedVolume();
		thumbnailImages[i] = SaveFileUIMode::saveThumbnail(std::vector<IMeshModel*>{model});

		if (progressHandler)
			progressHandler->setValue(to);

		//model->LoadOrientation();

		model->rotateAroundAPoint(-angle[i] * axis[i][0], -angle[i] * axis[i][1], -angle[i] * axis[i][2]);
	}
	if (progressHandler)
		progressHandler->setValue(100);

	printf("PRINT OPTIMUM DURATION: %lf\n", t.elapsed() / 1000.0);
	//model->b_changed = true;
	//parentClass->ui.viewer->OnPlatform();

	return true;
}
void PrintOptimum::rotateModelSelected(int direction)
{
	modelContainer->getSelectedModels().front()->rotateAroundAPoint(angle[direction] * axis[direction][0], angle[direction] * axis[direction][1], angle[direction] * axis[direction][2]);
}

//void PrintOptimum::orienting()
//{
//	const VboVolume& vbo = model->vbo;
//
//	int32_t minZ = 999;
//	std::vector<QVector3D> pts = vbo.getPtsAfterScale_fromParent();
//	for (size_t i = 0; i < model->points.size(); i++)
//	{
//		const QVector3D& origin = pts[i];
//		qglviewer::Vec originVec(origin[0], origin[1], origin[2]);
//
//		qglviewer::Vec afterVec = vbo.getFrame().inverseCoordinatesOf(originVec);
//
//		model->points[i].transformed_p = Point3(MM2INT(afterVec[0]), MM2INT(afterVec[1]), MM2INT(afterVec[2]));
//
//		if (model->points[i].transformed_p.z < minZ)
//			minZ = model->points[i].transformed_p.z;
//	}
//
//	if (minZ != 0)
//	{
//		std::cout << "auto On Platform" << std::endl;
//		for (size_t i = 0; i < m_Volume->points.size(); i++)
//			m_Volume->points[i].transformed_p.z -= minZ;
//	}
//
//	m_Volume->vMax = m_Volume->max();
//	m_Volume->vMin = m_Volume->min();
//}

bool PrintOptimum::slicing()
{
	// slice
	//////////////////////////////////////////////////////////////////////////
	SliceProcessor processor;

	processor.init(modelContainer, true);
	connect(&processor, SIGNAL(signal_setValue(int)), progressHandler, SLOT(setCustomValue(int)));
	connect(progressHandler, SIGNAL(signal_canceled()), &processor, SLOT(cancel_processor()));

	return processor.processingForPrintOptimum();
}
double PrintOptimum::calcThickness(std::vector<SliceLayer> sliceLayers)
{
	std::vector<double> areas(sliceLayers.size(), 0);
#pragma omp parallel for
	for (int i = 0; i < sliceLayers.size(); ++i)
	{
		SliceLayer& layer = sliceLayers[i];
		for (uint j = 0; j < layer.parts.size(); ++j)
		{
			engine::Polygons p = layer.parts[j].outline.offset(-200).offset(250);
			engine::Polygons p2 = layer.parts[j].outline.difference(p);
			//for ()

			for (int k = 0; k < p2.size(); k++)
				areas[i] += p2[k].area();
		}
	}

	double thickness = 0;
	for (int i = 0; i < areas.size(); i++)
		thickness += areas[i];

	return thickness;
}