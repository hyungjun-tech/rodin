#include "stdafx.h"
#include "ViewerModule.h"

#include "MeshLoader.h"
#include "MeshExporter.h"
#include "ModelContainer.h"
#include "ModelDataStorage.h"

#include "TranslateUIMode.h"
#include "RotateUIMode.h"
#include "ChangeBottomUIMode.h"
#include "PreviewUIMode.h"
#include "SupportUIMode.h"
#include "AnalysisUIMode.h"
#include "LayerColorUIMode.h"

#include "IOManager.h"
#include "UserProperties.h"
#include "CollisionManager.h"
#include "ContextMenu.h"
#include "gcodeExport.h"
#include "SaveFileUIMode.h"

#include "ProgressHandler.h"
#include "PrinterBody.h"

class Mesh;
class BasicMeshModel;

const float ViewerModule::camMaxDistance = 2000.0f;
const float ViewerModule::camMinDistance = 10.0f;
 
ViewerModule::ViewerModule(QWidget *parent)
	: QGLViewer(parent)
	, modelContainer(new ModelContainer())
	, printerBody(nullptr)
	, uiMode(nullptr)
	, viewerCamera(new Camera(this->devicePixelRatioF()))
	, manipulatedFrame(nullptr)
	, translateUIMode(nullptr)
	, rotateUIMode(nullptr)
	, previewUIMode(nullptr)
	, changeBottomUIMode(nullptr)
	, supportUIMode(nullptr)
	, analysisUIMode(nullptr)
	, layerColorUIMode(nullptr)
{
	setCamera(viewerCamera);
	setContextMenuPolicy(Qt::CustomContextMenu);
	initializeManipulatedFrame();

	printerBody = new PrinterBody();

	translateUIMode = new TranslateUIMode(this);
	rotateUIMode = new RotateUIMode(this);
	previewUIMode = new PreviewUIMode(this);
	changeBottomUIMode = new ChangeBottomUIMode(this);
	supportUIMode = new SupportUIMode(this);
	analysisUIMode = new AnalysisUIMode(this);
	layerColorUIMode = new LayerColorUIMode(this);

	setToggleAll(false);

	//connect(modelContainer, SIGNAL(signal_manipulated()), this, SLOT(refreshUI()));
	connect(modelContainer, SIGNAL(signal_updateGL()), this, SLOT(updateGL()));
	//connect(modelContainer, SIGNAL(signal_modelSelect()), this, SLOT(refreshUI()));
	//connect(modelContainer, SIGNAL(signal_modelDeselect()), this, SLOT(refreshUI()));
	//connect(modelContainer, SIGNAL(signal_modelDeleted()), this, SLOT(refreshUI()));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
	//connect(this, SIGNAL(signal_viewer_changed(Generals::ViewerMode)), this, SLOT(ViewerChanged(Generals::ViewerMode)));
}

ViewerModule::~ViewerModule()
{
	if (printerBody)
		delete printerBody;
	if (modelContainer)
		delete modelContainer;
	if (manipulatedFrame)
		delete manipulatedFrame;

	if (translateUIMode)
		delete translateUIMode;
	if (rotateUIMode)
		delete rotateUIMode;
	if (previewUIMode)
		delete previewUIMode;
	if (changeBottomUIMode)
		delete changeBottomUIMode;
	if (supportUIMode)
		delete supportUIMode;
	if (analysisUIMode)
		delete analysisUIMode;
	if (layerColorUIMode)
		delete layerColorUIMode;
}

void ViewerModule::setToggleAll(bool state)
{
	m_toggle.clear();
	m_toggle.resize(10, state);
}
void ViewerModule::setToggle(int index_)
{
	m_toggle[index_] = !m_toggle[index_];
}
bool ViewerModule::getToggle(int index_)
{
	if (index_ < 0 || index_ >= m_toggle.size())
		return false;
	return m_toggle[index_];
}

// Model IO Functions -------------------------------------------------------------------------------
void ViewerModule::loadModels(QStringList fileList_)
{
	if (fileList_.size() == 0)
		return;

	// set busy cursor - Not working, why?
	this->setCursor(Qt::BusyCursor);

	// Note: we need to classify erros into more details like assimp-file open error (fail to open a file) or geometric incompability error.
	// At present, errorCnt seems to count only geometric incompability errors.
	//makeCurrent();
	//modelContainer->DeselectAll();
	//doneCurrent();

	int errorCnt = 0;
	for (int i = 0; i < fileList_.size(); i++)
	{
		int retVal = loadModel(fileList_.at(i));

		if (retVal == -1)
		{	// abort, critical error
			printf("MyronEngine::LoadModel() stopped.\n");
			return;
		}

		errorCnt += retVal;		// count geometric errors
	}

	// set the last one as selected if any.
	// The following code may generate an indexing error (negative indexing) when the models size is zero (no mesh was read).
	//makeCurrent();
	modelContainer->deselectAll();
	if (modelContainer->models.size())
		modelContainer->models[modelContainer->models.size() - 1]->select();
	//doneCurrent();

	if (errorCnt > 0)
		Q_EMIT signal_mesh_has_errors();
	else
		Q_EMIT signal_processDone();
}

int ViewerModule::loadModel(QString fileName_)
{
	printf("File loading started...\n", fileName_.toStdString().c_str());

	//QGuiApplication::setOverrideCursor(Qt::WaitCursor);
	//this->setCursor(Qt::WaitCursor);
	//this->update();

	ProgressHandler progressHandler(this);
	progressHandler.setLabelText(MessageProgress::tr("Model Loading.."));
	progressHandler.setMaximum(100);
	progressHandler.setTargetValue(90);
	// Load models: a file may have multiple meshes 
	MeshLoader myLoader;
	myLoader.setProgressHandler(&progressHandler);

	std::vector<Mesh*> loadedMeshes;
	std::vector<IMeshModel*> tempModels;
	int errorCnt;
	try
	{
		int numMeshes = myLoader.load(fileName_, &loadedMeshes, true);	// false: progress handler & callback mechanism OFF
		if (numMeshes == 0)
			throw - 1;
		errorCnt = myLoader.getErrorCount();

		if (progressHandler.wasCanceled())
			throw -1;
		progressHandler.tempProgress(91, 95, 10);

		if (progressHandler.wasCanceled())
			throw - 1;
		// In case that there are more than 2 meshes, get user's response whether user wish to merge meshes, or not.
		Mesh * mergedMesh;

		QString ext = QFileInfo(fileName_).suffix();
		if (numMeshes > 1 && ext.toLower() == "stl")
		{
			/*QMessageBox msgBox;
			msgBox.setText(tr("warn_loaded_mesh_no") + "\n - " + fileName_);
			msgBox.setInformativeText(tr("question_merge"));
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::YesToAll);
			msgBox.setIcon(QMessageBox::Question);

			int res = msgBox.exec();
			if (res == QMessageBox::Yes)
			{
				mergedMesh = new Mesh();
				for (int i = 0; i < numMeshes; i++)
				{
					mergedMesh->Append(*(loadedMeshes[i]));
				}
				loadedMeshes.clear();
				loadedMeshes.push_back(mergedMesh);
			}*/ // to do sj
		}
		// Even though no mesh loaded, just follow the procedure and return later if it is critical.
		//if (loadedMeshes.size() == 0)
		//	return 0;

		// Build mesh models from meshes
		//makeCurrent();
		for (int i = 0; i < loadedMeshes.size(); i++) {
			tempModels.push_back(new BasicMeshModel(loadedMeshes[i]));
			QString meshName;
			if (loadedMeshes.size() == 1)
				meshName = QString(fileName_);		// If only one mesh for a file, use the file name as its model name
			else
				meshName = QString(fileName_ + "-Model%1").arg(i);		// Use different names for individual meshes
			tempModels.back()->setFileName(meshName);							// Actually, SetModelName() or SetMeshName(), not SetFileName()

		}
		//doneCurrent();
		//delete mergedMesh;

		Logger() << "File load complete. - " << fileName_;
		printf("File loading ended : No. Models (meshes)  = %zd (errCNT = %d)\n", loadedMeshes.size(), errorCnt);

#if  0
		QMessageBox msgBox;
		msgBox.setText(QString("File loaded : ") + fileName_ + QString("\n\nNo. models = %1. No. errorCNT = %2\n").arg(loadedMeshes.size()).arg(errorCnt));
		msgBox.exec();
#endif

		//makeCurrent();
	 //   uiMode->refreshUI();
		//doneCurrent();

		progressHandler.tempProgress(96, 99, 10);
		if (progressHandler.wasCanceled())
			throw - 1;

	}
	catch (int e)
	{
		for (int i = 0; i < loadedMeshes.size(); i++)
		{
			if (i < tempModels.size())
				delete tempModels[i];
			else
				delete loadedMeshes[i];
		}
		return e;
	}
	catch (std::exception e)
	{
		for (int i = 0; i < loadedMeshes.size(); i++)
		{
			if (i < tempModels.size())
				delete tempModels[i];
			else
				delete loadedMeshes[i];
		}
		Logger() << "exception : " << e.what();
		return -1;
	}
	//makeCurrent();

	IMeshModel * model;
	if (tempModels.size() == 1)
		model = tempModels.front();
	else
		model = new JoinedMeshModel(tempModels);
	modelContainer->addNewModel(model);
	translateModelsToFittablePlace(std::vector<IMeshModel*> {model});
	if (modelContainer->models.size() > 1)
		arrangeModels(modelContainer->getModelsExceptOne(model));
	autoScaling_inchToMM(model);
	model = autoScaling_maxSize(model);
	model->manipulated();

	model->savePosition();
	//doneCurrent();
	// No cancel service is available, too late :)
	return errorCnt;
}

bool ViewerModule::saveModels(QString fileName_)
{
	MeshExporter myExporter;
	return myExporter.saveModels(modelContainer->models, QFileInfo(fileName_));
}


void ViewerModule::deleteSelectedModels()
{
	modelContainer->deleteSelectedModels();
	updateGL();
}
void ViewerModule::deleteAllModels()
{
	modelContainer->deleteAllModels();
	modelContainer->printingInfo->clear();

	if (uiMode == previewUIMode)
		previewUIMode->clear();

	setTranslateUIMode();
	updateGL();
}
void ViewerModule::reloadAllModels()
{
	std::vector<IMeshModel*> models = modelContainer->models;
	for (auto model : models)
	{
		model->resetRotation();
		model->resetScale();
		model->calcAABB();
		autoScaling_inchToMM(model);
		model = autoScaling_maxSize(model);
		translateModelsToFittablePlace(std::vector<IMeshModel*> {model});
		arrangeModels(modelContainer->getModelsExceptOne(model));
	}
	//select signal을 발생시켜주기 위함. (scale dialog refresh 해줌)
	if (modelContainer->hasAnySelectedModel())
	{
		IMeshModel* tempModel = modelContainer->getSelectedModels().front();
		tempModel->deselect();
		tempModel->select();
	}

	if (Generals::isRotateUIMode())
	{
		rotateUIMode->refreshUI();
	}
}

void ViewerModule::saveScene(QString fileName_)
{
	if (modelContainer->models.size() == 0)
		return;

	MeshModelExporter()(modelContainer->models, fileName_);
	printf("save finished\n");
}
void ViewerModule::loadScene(QString fileName_)
{
	MeshModelImporter()(modelContainer->models, fileName_);

	std::vector<IMeshModel*> models = modelContainer->models;
	
	QFileInfo fInfo(fileName_);
	QString fPath = fInfo.absolutePath();
	QString fName = fInfo.completeBaseName();

	for (int i = 0; i < models.size(); i++) {
		modelContainer->setConnection(models[i]);
		models[i]->select();
		models[i]->setFileName(fPath + "/" + fName + QString("_%1").arg(i, 2, 10, QLatin1Char('0')));
	}
	printf("load finished\n");
}

int ViewerModule::loadModel(QString fileName_, QMatrix4x4 Mat)
{
	MeshLoader myLoader;
	std::vector<Mesh*> loadedMeshes;
	int numMeshes = myLoader.load(fileName_, &loadedMeshes);
	int errorCnt = myLoader.getErrorCount();

	if (loadedMeshes.size() == 0)
		return 0;

	for (int i = 0; i < loadedMeshes.size(); i++) {

		BasicMeshModel* tempModel = new BasicMeshModel(loadedMeshes[i]);
		tempModel->setFileName(fileName_);
		modelContainer->addNewModel(tempModel);

		qglviewer::Vec scaleTemp;
		scaleTemp[0] = sqrt(Mat(0, 0)*Mat(0, 0) + Mat(0, 1)*Mat(0, 1) + Mat(0, 2)*Mat(0, 2));
		scaleTemp[1] = sqrt(Mat(1, 0)*Mat(1, 0) + Mat(1, 1)*Mat(1, 1) + Mat(1, 2)*Mat(1, 2));
		scaleTemp[2] = sqrt(Mat(2, 0)*Mat(2, 0) + Mat(2, 1)*Mat(2, 1) + Mat(2, 2)*Mat(2, 2));
		tempModel->setScale(scaleTemp);

		qglviewer::Vec rotFrom(0, 0, 1);
		qglviewer::Vec rotTo;
		rotTo[0] = Mat(0, 2) / scaleTemp[0];
		rotTo[1] = Mat(1, 2) / scaleTemp[1];
		rotTo[2] = Mat(2, 2) / scaleTemp[2];

		qglviewer::Quaternion rotateQ = qglviewer::Quaternion(rotFrom, rotTo);
		tempModel->rotate(rotateQ.axis(), rotateQ.angle());
		
		translateModelsToFittablePlace();
		arrangeModels(modelContainer->getModelsExceptOne(tempModel));
	}

	Logger() << "File load complete. - " << fileName_;	
	uiMode->refreshUI();

	return errorCnt;
}

// Mesh Processing Functions ------------------------------------------------------------------------
void ViewerModule::translateSelectedModels(float x_, float y_, float z_)
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();

	float machineWidth = Profile::getMachineWidth_calculated();
	float machineDepth = Profile::getMachineDepth_calculated();
	float bedMinX = Profile::machineProfile.machine_width_offset.value;
	float bedMinY = Profile::machineProfile.machine_depth_offset.value;
	float bedMaxX = machineWidth + bedMinX;
	float bedMaxY = machineDepth + bedMinY;
	//AABB aabb = AABBGetter()(selecteds);
	AABB aabb = TotalBoxGetter()(selecteds);		// (2019.09.05) 이동 시 Raft, support 형상을 반영하도록 변경
	float minX = aabb.getMinimum().x + x_;
	float maxX = aabb.getMaximum().x + x_;
	float minY = aabb.getMinimum().y + y_;
	float maxY = aabb.getMaximum().y + y_;
	if (minX < bedMinX)
		x_ = x_ - (minX - bedMinX);
	if (maxX > bedMaxX)
		x_ = x_ - (maxX - bedMaxX);
	if (minY < bedMinY)
		y_ = y_ - (minY - bedMinY);
	if (maxY > bedMaxY)
		y_ = y_ - (maxY - bedMaxY);

	for (int i = 0; i < selecteds.size(); i++)
	{
		selecteds[i]->translate(x_, y_, z_);
		selecteds[i]->manipulated();
	}
	if (UserProperties::usingDetectCollision)
		uiMode->detectCollision();
	
	uiMode->refreshUI();
	//updateGL();
}
void ViewerModule::placeSelectedModelsToBedCenter()
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();

	qglviewer::Vec v1 = TotalBoxGetter()(selecteds).getFloorCenter();
	qglviewer::Vec v2 = Profile::getMachineAABB().getFloorCenter();
	qglviewer::Vec v = v2 - v1;
	translateSelectedModels(v[0], v[1], 0);
}
void ViewerModule::translateModelsToFittablePlace(std::vector<IMeshModel*> models_)
{
	if (models_.size() == 0)
		models_ = modelContainer->getSelectedModels();

	qglviewer::Vec v1 = TotalBoxGetter()(models_).getFloorCenter();
	qglviewer::Vec v2 = Profile::getMachineAABB().getFloorCenter();
	qglviewer::Vec v = v2 - v1;

	for (int i = 0; i < models_.size(); i++) {
		models_[i]->translate(v[0], v[1], 0);
		models_[i]->manipulated();
	}
}
void ViewerModule::resetTranslateSelectedModels()
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	if (selecteds.size() != 1)
		return;
	for (auto selected : selecteds)
	{
		selected->resetPosition();
		selected->manipulated();
		if (selected->isDisabled())
		{
			placeSelectedModelsToBedCenter();
			updateGL();
		}
	}
	if (UserProperties::usingDetectCollision)
		uiMode->detectCollision();
	//uiMode->refreshUI();
}
//void ViewerModule::rotateSelectedModels(float x_deg, float y_deg, float z_deg)
void ViewerModule::rotateSelectedModels(float x_rad, float y_rad, float z_rad)		// radian으로 변경	// ijeong 2019 08
{
	for (auto model : modelContainer->getSelectedModels())
	{
		model->rotateAroundAPoint(x_rad, y_rad, z_rad);
		//model->adjustPosition();
	}

	if (UserProperties::usingDetectCollision)
		uiMode->detectCollision();
	uiMode->refreshUI();
}
void ViewerModule::resetRotateSelectedModels()
{
	for (auto model : modelContainer->getSelectedModels()) 
	{
		model->resetRotation();
		model->adjustPosition();
	}
	if (UserProperties::usingDetectCollision)
		uiMode->detectCollision();
	uiMode->refreshUI();
}
void ViewerModule::scaleSelectedModels(float x_, float y_, float z_)
{
	if (x_ <= 0 || y_ <= 0 || z_ <= 0)
		return;
	for (auto model : modelContainer->getSelectedModels())
	{
		model->setScale(qglviewer::Vec(x_, y_, z_));
		model->adjustPosition();
	}
	if (UserProperties::usingDetectCollision)
		uiMode->detectCollision();
	uiMode->refreshUI();
	updateGL();
}
void ViewerModule::scalemmSelectedModels(float x_, float y_, float z_)
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	AABB aabb = AABBGetter()(selecteds);
	qglviewer::Vec factor = qglviewer::Vec(x_ / aabb.getLengthX(), y_ / aabb.getLengthY(), z_ / aabb.getLengthZ());

	for (auto model : selecteds)
	{
		model->setScale(factor);
		model->adjustPosition();
	}
	if (UserProperties::usingDetectCollision)
		uiMode->detectCollision();
	uiMode->refreshUI();
	updateGL();
}
void ViewerModule::resetScaleSelectedModels()
{
	for (auto model : modelContainer->getSelectedModels())
	{
		model->resetScale();
		model->adjustPosition();
	}
	if (UserProperties::usingDetectCollision)
		uiMode->detectCollision();
	uiMode->refreshUI();
	updateGL();
}
void ViewerModule::maxSizeSelectedMeshes()
{
	if (!modelContainer->hasAnySelectedModel())
		return;
	if (modelContainer->isMultiSelected())
		return;

	for (auto model : modelContainer->getSelectedModels())
	{
		IMeshModel* maxModel = model->clone();
		if (!maxModel->maxSize())
		{
			delete maxModel;
			//ViewerMsg_cannotMaxSize : Due to model issues, the maximum size function can not be performed.
			//Returns to the scale of the original model.//
			CommonDialog comDlg(this, MessageAlert::tr("fail_max_size"), CommonDialog::Warning);
			return;
		}
		modelContainer->replace(maxModel, model);
	}
	if (UserProperties::usingDetectCollision)
		uiMode->detectCollision();
	//PlaceSelectedModelsToBedCenter();
}
qglviewer::Vec ViewerModule::getScaleSelectedModels()
{
	qglviewer::Vec scale;
	if (checkScaleRatio())
	{
		std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
		if (selecteds.size() == 0)
			scale = { 1, 1, 1 };

		else 
			scale = selecteds[0]->getScaledFactor();
	}
	else
		scale = { 1, 1, 1 };

	return scale;
}
bool ViewerModule::checkScaleRatio()
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	qglviewer::Vec preScale = { 0, 0, 0 };
	if (selecteds.size() == 1) return true;
	for (int i = 0; i < selecteds.size(); i++)
	{
		qglviewer::Vec temp = selecteds[i]->getScaledFactor();
		if (preScale == qglviewer::Vec(0, 0, 0))
			preScale = temp;
		else if (preScale != temp)
			return false;
	}
	return true;
}
void ViewerModule::autoScaling_inchToMM(IMeshModel* model_)
{
	if (model_->compareModelSize(5.0f) < 0)
	{
		CommonDialog comDlg(this, MessageQuestion::tr("too_small_model_alert"), CommonDialog::Question, false, false, true, true);
		if (comDlg.isYes())
			model_->setScale(qglviewer::Vec(25.4, 25.4, 25.4));
	}
}
IMeshModel* ViewerModule::autoScaling_maxSize(IMeshModel* model_)
{
	if (model_->compareModelSize(500) > 0)
	{
		CommonDialog comDlg(this, MessageQuestion::tr("too_large_model_alert"), CommonDialog::Question, false, false, true, true);
		if (comDlg.isYes())
		{
			IMeshModel* maxModel = model_->clone();
			if (!maxModel->maxSize())
			{
				delete maxModel;
				//ViewerMsg_cannotMaxSize : Due to model issues, the maximum size function can not be performed.
				//Returns to the scale of the original model.//
				CommonDialog comDlg(this, MessageAlert::tr("fail_max_size"), CommonDialog::Warning);
				return model_;
			}
			modelContainer->replace(maxModel, model_);
			return maxModel;
		}
	}
	return model_;
}

std::vector<qglviewer::Vec> ViewerModule::findBottomMesh(int no, qglviewer::Vec _worldPos, qglviewer::Vec _onNormal)
{
	std::vector<qglviewer::Vec> triVertices;
	std::vector<IMeshModel*> selected = modelContainer->getLatestModel(no);

	triVertices = selected[0]->findBottomTriMesh(_worldPos, _onNormal);
	return triVertices;
}

void ViewerModule::joinModels()
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	if (selecteds.size() < 2)
		return;

	// Pre-Calculate center
	qglviewer::Vec center(0, 0, 0);
	for (int i = 0; i < selecteds.size(); i++)
		center += selecteds[i]->getAABB().getFloorCenter();
	center /= selecteds.size();

	std::vector<IMeshModel*> parts;
	// Move back to Origin
	for (auto select : selecteds)
	{
		disconnect(select, 0, 0, 0);
		select->deselect();
		//qglviewer::Vec toOrigin = -select->getModels().front()->GetFrame().frame.position();
		//select->translate(toOrigin[0], toOrigin[1], toOrigin[2]);
		for (auto part : select->getModels())
		{
			qglviewer::Vec toOrigin = -part->getFrame().frame.position();
			part->translate(toOrigin[0], toOrigin[1], toOrigin[2]);
			parts.push_back(part);
		}
	}
	
	// Place output Model to Pre-Calculated center
	IMeshModel *joinedModel = new JoinedMeshModel(parts);
	joinedModel->calcAABB();
	AABB aabb = joinedModel->getAABB();
	qglviewer::Vec move = center - aabb.getFloorCenter();
	joinedModel->translate(move[0], move[1], move[2]);
	joinedModel->manipulated();

	modelContainer->erase(selecteds);

	std::vector<IMeshModel*> fix = modelContainer->models;
	modelContainer->addNewModel(joinedModel);
	joinedModel->select();
	if (fix.size() != 0)
		arrangeModels(fix);
	joinedModel->savePosition();
	uiMode->refreshUI();

	updateGL();
}

void ViewerModule::undoJoinModels()
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	if (selecteds.size() == 0)
		return;
	//multi selection인 경우 동작엔 문제 없지만 평가팀 사양이므로 일단 유지
	if (selecteds.size() > 1)
		return;
	

	std::vector<IMeshModel*> partsList;
	for (auto select : selecteds)
	{
		std::vector<IMeshModel*> parts = select->disJoin();
		if (parts.empty() || parts.size() == 1)
			continue;
		disconnect(select, 0, 0, 0);

		partsList.insert(partsList.end(), parts.begin(), parts.end());
		modelContainer->deleteModel(select);
		for (auto part : parts)
		{
			std::vector<IMeshModel*> fix = modelContainer->models;
			modelContainer->addNewModel(part);
			part->manipulated();
			part->select();
			if (fix.size() != 0)
				arrangeModels(fix);
		}
	}

	uiMode->refreshUI();
	updateGL();
}

void ViewerModule::multiplySelectedModels(int numofCopies_)
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	if (selecteds.size() == 0)
		return;

	modelContainer->deselectAll();
	for (int i = 0; i < numofCopies_; i++)
	{
		for (int j = 0; j < selecteds.size(); j++)
		{
			IMeshModel* model = selecteds[j]->clone();
			modelContainer->addNewModel(model);
			model->select();
			arrangeModels(modelContainer->getModelsExceptOne(model));
		}
	}

	/*for (int k = 0; k < selecteds.size() * numofCopies_; k++)
	{
		modelContainer->models[modelContainer->models.size() - 1 - k]->Select();
	}*/
}

void ViewerModule::layFlat()
{
	std::vector<IMeshModel*> selecteds = modelContainer->getSelectedModels();
	if (selecteds.size() == 0)
		return;

	for (int i = 0; i < selecteds.size(); i++) 
		selecteds[i]->layFlat();

	uiMode->refreshUI();
}

void ViewerModule::deleteAllSupports()
{
	modelContainer->supportData->deleteAllsupports();
	if (uiMode == supportUIMode)
	{
		supportUIMode->updateData();
	}
}
void ViewerModule::resetAllSupports()
{
	modelContainer->supportData->resetAllsupports(Profile::sliceProfileCommon.support_placement.value);
	if (uiMode == supportUIMode)
	{
		supportUIMode->updateData();
	}
}

// Mode Setting Functions ------------------------------------------------------------------------------
void ViewerModule::refreshUI()
{
	makeCurrent();
	uiMode->refreshUI();
	//uiMode->Draw();
	draw();
	swapBuffers();
}
void ViewerModule::resetBed()
{
	Profile::setMachineAABB();
	if (uiMode != nullptr)
		printerBody->setSize();
}

void ViewerModule::setTranslateUIMode()
{
	emit signal_viewer_changed(Generals::ViewerMode::TranslateUIMode);
	uiMode = translateUIMode;

	refreshUI();
}

void ViewerModule::setRotateUIMode()
{
	uiMode = rotateUIMode;
	refreshUI();
	emit signal_viewer_changed(Generals::ViewerMode::RotateUIMode);
}

void ViewerModule::setPreviewUIMode(ModelDataStorage* dataStorage_)
{
	emit signal_updateMaxLayerForPreview(dataStorage_->layerCount);
	uiMode = previewUIMode;

	previewUIMode->setZHeightBase(INT2MM(dataStorage_->z_height_base));
	previewUIMode->updateGCodePath(dataStorage_->gcodePath);
	previewUIMode->changeLayerIndex(dataStorage_->layerCount);
	refreshUI();
	emit signal_viewer_changed(Generals::ViewerMode::PreviewUIMode);
}

void ViewerModule::setChangeBottomUIMode()
{
	uiMode = changeBottomUIMode;
	changeBottomUIMode->isDummyVisible = false;
	refreshUI();
	emit signal_viewer_changed(Generals::ViewerMode::ChangeBottomUIMode);
}

void ViewerModule::setSupportUIMode()
{
	uiMode = supportUIMode;

	supportUIMode->setSupportData(modelContainer->supportData);
	AABB aabb = AABBGetter()(modelContainer->models);
	int max_v = MM2INT(aabb.getLengthZ()) / Profile::configSettings.front().layer_height;
	supportUIMode->changeLayerIndex(max_v / 2);
	refreshUI();
	emit signal_viewer_changed(Generals::ViewerMode::SupportUIMode);
}

void ViewerModule::setAnalysisUIMode()
{
	uiMode = analysisUIMode;
	refreshUI();
	emit signal_viewer_changed(Generals::ViewerMode::AnalysisUIMode);
}

void ViewerModule::setLayerColorUIMode()
{
	uiMode = layerColorUIMode;
	changeLayerIndexForLayerColor();
	refreshUI();
	emit signal_viewer_changed(Generals::ViewerMode::LayerColorUIMode);
}

Generals::ViewerMode ViewerModule::getUIModeType()
{
	if (uiMode == translateUIMode)
		return Generals::ViewerMode::TranslateUIMode;
	else if (uiMode == rotateUIMode)
		return Generals::ViewerMode::RotateUIMode;
	else if (uiMode == previewUIMode)
		return Generals::ViewerMode::PreviewUIMode;
	else if (uiMode == changeBottomUIMode)
		return Generals::ViewerMode::ChangeBottomUIMode;
	else if (uiMode == supportUIMode)
		return Generals::ViewerMode::SupportUIMode;
	else if (uiMode == analysisUIMode)
		return Generals::ViewerMode::AnalysisUIMode;
	else if (uiMode == layerColorUIMode)
		return Generals::ViewerMode::LayerColorUIMode;
	else 
		return Generals::ViewerMode::None;
}

QString ViewerModule::getOnlyFileName()
{
	if (modelContainer->models.size() == 0) return "";
	QString rtn;
	QString filename = modelContainer->models.back()->getOnlyFileName();
	rtn = Generals::removeSpecialChar(filename);
	return rtn;
}

// QGLViewer Functions ------------------------------------------------------------------------------
void ViewerModule::init()
{
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_MATERIAL);

	setMouseTracking(true);
	constraintMouse();

	printerBody = new PrinterBody();

	translateUIMode = new TranslateUIMode(this);
	rotateUIMode = new RotateUIMode(this);
	previewUIMode = new PreviewUIMode(this);
	changeBottomUIMode = new ChangeBottomUIMode(this);
	supportUIMode = new SupportUIMode(this);
	analysisUIMode = new AnalysisUIMode(this);
	layerColorUIMode = new LayerColorUIMode(this);

	printerBody->init();

	int w = this->width();
	int h = this->height();
	translateUIMode->initialize(w, h);
	rotateUIMode->initialize(w, h);
	previewUIMode->initialize(w, h);
	changeBottomUIMode->initialize(w, h);
	supportUIMode->initialize(w, h);
	analysisUIMode->initialize(w, h);
	layerColorUIMode->initialize(w, h);

	/*int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	translateUIMode->Initialize(viewport[2], viewport[3]);
	rotateUIMode->Initialize(viewport[2], viewport[3]);
	previewUIMode->Initialize(viewport[2], viewport[3]);
	changeBottomUIMode->Initialize(viewport[2], viewport[3]);
	supportUIMode->Initialize(viewport[2], viewport[3]);
	analysisUIMode->Initialize(viewport[2], viewport[3]);*/

	//camera()->frame()->setZoomsOnPivotPoint(true);
	camera()->setZNearCoefficient(0.0001);
	camera()->setZClippingCoefficient(5);
	camera()->frame()->setSpinningSensitivity(1000);
	camera()->frame()->setPivotPoint(qglviewer::Vec(Profile::getMachineWidth_calculated() * 0.5,
		Profile::getMachineDepth_calculated() * 0.5, 0));
	isometricCamera();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//setBufferDevicePixelRatio(QWidget::devicePixelRatio());
	//updateGL();
	setTranslateUIMode();
}

void ViewerModule::preDraw()
{
}

void ViewerModule::draw()
{
	if (!printerBody)
		return;

	uiMode->draw();
	drawCornerAxis();
	drawFileName();
	drawExpandedModeText();
}

void ViewerModule::mousePressEvent(QMouseEvent* e)
{
	uiMode->mousePressEvent(e);
	QGLViewer::mousePressEvent(e);
	QWidget::mousePressEvent(e);

	if (e->button() == Qt::RightButton)
		pressed = e->pos();
}

void ViewerModule::mouseMoveEvent(QMouseEvent* e)
{
	uiMode->mouseMoveEvent(e);
	QGLViewer::mouseMoveEvent(e);
}

void ViewerModule::mouseReleaseEvent(QMouseEvent* e)
{
    uiMode->mouseReleaseEvent(e);
    QGLViewer::mouseReleaseEvent(e);
    QWidget::mouseReleaseEvent(e);
}
void ViewerModule::mouseDoubleClickEvent(QMouseEvent *e)
{
	uiMode->mouseDoubleClickEvent(e);
}

void ViewerModule::wheelEvent(QWheelEvent *e)
{
	qglviewer::Vec camPos = camera()->position();
	qglviewer::Vec targetPos = camera()->pivotPoint();
	qglviewer::Vec viewDir = camera()->viewDirection();
	float camDistance = viewDir *(targetPos - camPos);

	if (camDistance > camMaxDistance && e->delta() < 0)
		return;
	else if (camDistance < camMinDistance && e->delta() > 0)
		return;

	uiMode->wheelEvent(e);
	QGLViewer::wheelEvent(e);
}

void ViewerModule::keyPressEvent(QKeyEvent *e)
{ 
	if (e->modifiers() == Qt::NoModifier)
	{
		if (e->key() >= 48 && e->key() <= 57)
		{
			int index = e->key() - 48;
			setToggle(index);
			updateGL();
			return;
		}
	}

	if (//(e->modifiers() == Qt::NoModifier && (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right)) || //Moves camera
		(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_C)//Copies a snapshot to clipboard
		|| (e->modifiers() == Qt::AltModifier && e->key() == Qt::Key_Return)//Toggles full screen display
		|| (e->modifiers() == Qt::ShiftModifier && e->key() == Qt::Key_Question))//Toggles the display of the text
	{
		QGLViewer::keyPressEvent(e);
		return;
	}

	uiMode->keyPressEvent(e);
	QWidget::keyPressEvent(e);
}

void ViewerModule::resizeEvent(QResizeEvent *e)
{
	if (uiMode != nullptr)
		uiMode->resizeEvent(e);
	QGLViewer::resizeEvent(e);
}

void ViewerModule::constraintMouse()
{
	setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME,
		QGLViewer::NO_MOUSE_ACTION);
	setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::CAMERA,
		QGLViewer::ROTATE);
	/*setMouseBinding(Qt::NoModifier, Qt::MiddleButton, QGLViewer::FRAME,
		QGLViewer::NO_MOUSE_ACTION);*///middle button zoom in/out 기능 활성
	setWheelBinding(Qt::NoModifier, QGLViewer::CAMERA,
		QGLViewer::ZOOM);

	setMouseBinding(Qt::ControlModifier, Qt::LeftButton, QGLViewer::FRAME,
		QGLViewer::NO_MOUSE_ACTION);
	setMouseBinding(Qt::ControlModifier, Qt::RightButton, QGLViewer::CAMERA,
		QGLViewer::NO_MOUSE_ACTION);
	setMouseBinding(Qt::ControlModifier, Qt::MiddleButton, QGLViewer::FRAME,
		QGLViewer::NO_MOUSE_ACTION);
	setWheelBinding(Qt::ControlModifier, QGLViewer::CAMERA,
		QGLViewer::NO_MOUSE_ACTION);

	setMouseBinding(Qt::ShiftModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::NO_MOUSE_ACTION);
	setMouseBinding(Qt::ShiftModifier, Qt::RightButton, QGLViewer::CAMERA,
		QGLViewer::TRANSLATE);
	/*setMouseBinding(Qt::ShiftModifier, Qt::MiddleButton, QGLViewer::FRAME,
		QGLViewer::NO_MOUSE_ACTION);*///middle 영역 지정 확대 기능 활성
	setWheelBinding(Qt::ShiftModifier, QGLViewer::CAMERA,
		QGLViewer::NO_MOUSE_ACTION);

	setMouseBinding(Qt::ControlModifier, Qt::LeftButton, ClickAction::NO_CLICK_ACTION, true);
	setMouseBinding(Qt::ShiftModifier, Qt::LeftButton, ClickAction::NO_CLICK_ACTION, true);
}

void ViewerModule::initializeManipulatedFrame()
{
	manipulatedFrame = new ManipulatedFrame(modelContainer, this->camera());
	manipulatedFrame->setSpinningSensitivity(1000.0);
	setManipulatedFrame(manipulatedFrame);
}

// UI Response Functions ------------------------------------------------------------------------------
void ViewerModule::changeLayerIndexForSupport(int index_)
{
	if (!Generals::isSupportUIMode())
		return;
	supportUIMode->changeLayerIndex(index_);
	updateGL();
}
void ViewerModule::changeLayerIndexForPreview(int index_)
{
	if (!Generals::isPreviewUIMode())
		return;
	previewUIMode->changeLayerIndex(index_);
	updateGL();
}
void ViewerModule::changeLayerIndexForLayerColor(int index_)
{
	if (!Generals::isLayerColorUIMode())
		return;
	if (index_ == -1)
	{
		AABB aabb = AABBGetter()(modelContainer->models);
		int maxLayer = aabb.getMaximum().z / Profile::sliceProfileCommon.layer_height.value;
		index_ = maxLayer / 2;
	}
	layerColorUIMode->changeLayerIndex(index_);
	updateGL();
}
void ViewerModule::toggleExtruderMode(bool flag_)
{
	previewUIMode->toggleExtruderMode(flag_);
	updateGL();
}
void ViewerModule::toggleCurrentLayerOnly(bool flag_)
{
	previewUIMode->toggleCurrentLayerOnly(flag_);
	updateGL();
}
void ViewerModule::toggleShowTravelPath(bool flag_)
{
	previewUIMode->toggleShowTravelPath(flag_);
	updateGL();
}

VolumeAnalysis* ViewerModule::getVolumeAnalysis()
{
	return analysisUIMode->getVolumeAnalysis();
}

//void ViewerModule::UpdateGCodePath(std::vector<std::vector<GCodePathforDraw>> path_)
//{
//	previewUIMode->UpdateGCodePath(path_);
//}
//
//void ViewerModule::SetZHeightBase(float zHeightBase_)
//{
//	previewUIMode->SetZHeightBase(zHeightBase_);
//}

void ViewerModule::arrangeModels(std::vector<IMeshModel*> fix_)
{
	if (modelContainer->models.size() <= 1)
		return;

	if (fix_.size() == 0)
		fix_ = modelContainer->getSelectedModels();

	CollisionManager cm;
	cm.detectCollision(fix_, modelContainer->getModelsExceptOne(fix_));
}

void ViewerModule::rearrangeModels()
{
	std::vector<IMeshModel*> models = modelContainer->models;
	if (models.size() < 1)
		return;

	std::vector <IMeshModel*> fix_;
	CollisionManager cm;
	for (int i = 0; i < models.size(); i++)
	{
		std::vector<IMeshModel*> move{ models[i] };
		translateModelsToFittablePlace(move);
		cm.arrangeModels(fix_, move);
		fix_.push_back(models[i]);
	}
	updateGL();
}

// Slice Functions ------------------------------------------------------------------------------
//void ViewerModule::SliceAll()
//{
//	float maxZ = -FLT_MAX;
//	for (int i = 0; i < modelContainer->models.size(); i++)
//	{
//		qglviewer::Vec p = modelContainer->models[i]->getAABB().GetMaximum();
//		if (maxZ < p.z)
//			maxZ = p.z;
//	}
//
//	Slicer slicer(modelContainer->models);
//	//float z = SlicerOptions::layerThickness / 2.0;// +SlicerOptions::raftThickness;
//	float z = SlicerOptions::platformLayerHeight / 2.0;
//	int imageCnt = 0;
//	int layerCnt = 0;
//	//for platform
//	while (1)
//	{
//		QString sliceFileName = SlicerOptions::workspacePath + "/output/raft/P" + QString::number(imageCnt + 1).rightJustified(6, '0') + ".png";
//		wchar_t *fileName = ToWideCharArray()(sliceFileName);
//		wprintf(L"%s\n", fileName);
//
//		cv::Mat slice = slicer.Slice(z);
//		printf("%d platform image exported\n", imageCnt);
//		slicer.Write1bitPNG(fileName, slice);
//		int repeatCnt = SlicerOptions::platformRepeatCount;
//		if (SlicerOptions::platformRepeatCountList.size() >= layerCnt + 1)
//			repeatCnt = SlicerOptions::platformRepeatCountList[layerCnt];
//
//		for (int i = 0; i < repeatCnt - 1; i++)
//		{
//			imageCnt++;
//			QString temp = SlicerOptions::workspacePath + "/output/raft/P" + QString::number(imageCnt + 1).rightJustified(6, '0') + ".png";
//			QFile::copy(sliceFileName, temp);
//		}
//
//		z += SlicerOptions::platformLayerHeight;
//		imageCnt++;
//		layerCnt++;
//		if (z > SlicerOptions::platformHeight)
//			break;
//	}
//
//	z = SlicerOptions::platformHeight + SlicerOptions::layerThickness / 2.0;
//	imageCnt = 0;
//	//for model
//	while (1)
//	{
//		wchar_t *fileName = ToWideCharArray()(SlicerOptions::workspacePath + "/output/model/P" + QString::number(imageCnt + 1).rightJustified(6, '0') + ".png");
//		wprintf(L"%s\n", fileName);
//
//		cv::Mat slice = slicer.Slice(z);
//		printf("%d slice image exported\n", imageCnt);
//		slicer.Write1bitPNG(fileName, slice);
//
//		z += SlicerOptions::layerThickness;
//		imageCnt++;
//		layerCnt++;
//		if (z > maxZ)
//			break;
//	}
//
//	refreshUI();
//}

void ViewerModule::isometricCamera()
{
	qglviewer::Vec viewDir(0, 1, -0.5);
	qglviewer::Vec cam(1, 0, 0);
	setView(viewDir, cam);
}
void ViewerModule::topView()
{
	qglviewer::Vec viewDir(0, 0, -1);
	qglviewer::Vec cam(1, 0, 0);
	setView(viewDir, cam);
}
void ViewerModule::rightView()
{
	qglviewer::Vec viewDir(-1, 0, 0);
	qglviewer::Vec cam(0, 1, 0);
	setView(viewDir, cam);
}
void ViewerModule::leftView()
{
	qglviewer::Vec viewDir(1, 0, 0);
	qglviewer::Vec cam(0, -1, 0);
	setView(viewDir, cam);
}
void ViewerModule::frontView()
{
	qglviewer::Vec viewDir(0, 1, 0);
	qglviewer::Vec cam(1, 0, 0);
	setView(viewDir, cam);
}
void ViewerModule::fitToModel()
{
	if (modelContainer->models.size() > 0)
		fitToModel(AABBGetter()(modelContainer->models));
	else
		isometricCamera();
}
void ViewerModule::fitToModel(IMeshModel* model)
{
	fitToModel(model->getAABB());
}
void ViewerModule::fitToModel(AABB aabb_)
{
	qglviewer::Vec viewDir(this->camera()->viewDirection());
	qglviewer::Vec upVec(this->camera()->upVector());

	qglviewer::Vec center = (aabb_.getMaximum() + aabb_.getMinimum())*0.5;
	qglviewer::Vec vSize = (aabb_.getMaximum() - aabb_.getMinimum())*0.5;

	float fov = this->camera()->fieldOfView();
	float dist = vSize.norm() / sin(fov*0.5) * 1.2;

	qglviewer::Vec newPos = center - viewDir * dist;

	this->setSceneRadius(160);
	this->camera()->setPosition(newPos);
	this->camera()->setUpVector(upVec);
	this->camera()->setViewDirection(viewDir);
	this->camera()->setSceneCenter(center);
	this->camera()->setPivotPoint(center);
	this->updateGL();
}
void ViewerModule::setView(qglviewer::Vec pViewDir, qglviewer::Vec pCam, double camDistance_)
{
	if (camDistance_ < 0)
		camDistance_ = Profile::machineProfile.view_cam_distance.value;
	const double camDistance = camDistance_;
	const int camDirection = 0;

	qglviewer::Vec center = Profile::getMachineAABB().getCenter();
	qglviewer::Vec centerFactor = Profile::getCamCenterFactor();
	center = qglviewer::Vec(center[0] * centerFactor[0], center[1] * centerFactor[1], center[2] * centerFactor[2]);
	qglviewer::Vec viewDir = vectorRotate(pViewDir, 90 * camDirection);
	viewDir.normalize();

	qglviewer::Vec camX = vectorRotate(pCam, 90 * camDirection);
	qglviewer::Vec upVec = cross(camX, viewDir);
	qglviewer::Vec camPos = center - viewDir * camDistance;

	this->camera()->setPosition(camPos);
	this->camera()->setViewDirection(viewDir);
	this->camera()->setUpVector(upVec);

	this->setSceneRadius(160);
	updateGL();
}
qglviewer::Vec ViewerModule::vectorRotate(qglviewer::Vec pBeforeVec, int pDegree)
{
	double rotate_rad = pDegree * M_PI / 180;

	qglviewer::Vec rtnVec(pBeforeVec.x * cos(rotate_rad) - pBeforeVec.y * sin(rotate_rad), pBeforeVec.x * sin(rotate_rad) + pBeforeVec.y * cos(rotate_rad), pBeforeVec.z);
	return rtnVec;
}

void ViewerModule::drawCornerAxis()
{
	int viewport[4];
	int scissor[4];

	// The viewport and the scissor are changed to fit the lower left
	// corner. Original values are saved.
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetIntegerv(GL_SCISSOR_BOX, scissor);

	glDisable(GL_LIGHTING);
	int size = 150;
	float length = 0.5 * devicePixelRatioF();
	drawAxis(size, length);
	drawAxisText(size, length);
	glEnable(GL_LIGHTING);

	// The viewport and the scissor are restored.
	glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}


void ViewerModule::drawAxis(int _size, qreal _length /*= 1*/)
{
	// Axis viewport size, in pixels
	const int size = _size * devicePixelRatioF();
	glViewport(0, 0, size, size);
	glScissor(0, 0, size, size);

	// The Z-buffer is cleared to make the axis appear over the
	// original image.
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixd(camera()->orientation().inverse().matrix());

	QColor colorX(189, 26, 26);
	QColor colorY(26, 105, 26);
	QColor colorZ(26, 26, 187);
	double arrowRadius = _length * 0.05;
	// render arrow
	//////////////////////////////////////////////////////////////////////////
	qglColor(colorX);
	drawArrow(qglviewer::Vec(0, 0, 0), qglviewer::Vec(_length, 0, 0), arrowRadius);
	qglColor(colorY);
	drawArrow(qglviewer::Vec(0, 0, 0), qglviewer::Vec(0, _length, 0), arrowRadius);
	qglColor(colorZ);
	drawArrow(qglviewer::Vec(0, 0, 0), qglviewer::Vec(0, 0, _length), arrowRadius);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void ViewerModule::drawAxisText(int _size, qreal _length /*= 1*/)
{
	int viewport[4];
	int scissor[4];

	// The viewport and the scissor are changed to fit the lower left
	// corner. Original values are saved.
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetIntegerv(GL_SCISSOR_BOX, scissor);

	// Axis viewport size, in pixels
	const int size = _size;
	glViewport(0, 0, size, size);
	glScissor(0, 0, size, size);

	// The Z-buffer is cleared to make the axis appear over the
	// original image.
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixd(camera()->orientation().inverse().matrix());


	QColor colorX(189, 26, 26);
	QColor colorY(26, 105, 26);
	QColor colorZ(26, 26, 187);

	double textLoc = _length * 1.05;

	// render text
	//////////////////////////////////////////////////////////////////////////
	qglColor(colorZ);
	renderText(0, 0, textLoc, "Z");

	qglColor(colorX);
	renderText(textLoc, 0, 0, "X");

	qglColor(colorY);
	renderText(0, textLoc, 0, "Y");


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void ViewerModule::drawFileName()
{
	if (!UserProperties::showLoadedFileName)
		return;

	if (modelContainer->hasAnyModel())
	{
		std::vector<IMeshModel*> models = modelContainer->models;

		glPushMatrix();

		QColor color_selected(26, 105, 26);
		int yMargin = 20;
		QFont font("Malgun Gothic", 9, QFont::Normal);
		for (int i = 0; i < models.size(); i++)
		{
			if (models[i]->isSelected())
			{
				qglColor(color_selected);
				font.setBold(true);
			}
			else
			{
				qglColor(Qt::black);
				font.setBold(false);
			}

			int yPosition = (yMargin + 15 * i);
			drawText(10, yPosition, models[i]->getFileName(), font);
		}
	}
	else if (modelContainer->printingInfo->getIsLoadedGcode())
	{
		qglColor(Qt::black);
		QFont font("Malgun Gothic", 9, QFont::Normal);
		drawText(10, 20, modelContainer->printingInfo->getGcodeFileName(UserProperties::usingFullFilePath), font);
	}


	//QStringList fileNameList;

	//for (int i = 0; i < models.size(); ++i)
	//{
	//	fileNameList.append(models[i]->GetFileName());
	//}

	//qglColor(Qt::black);
	//QFont font("Arial", 9, QFont::Normal);
	//QString uiStr;

	//int j = 0; //로드 된 파일 갯수 체크
	//int bottomY;

	//if (fileNameList.size() <= 10) bottomY = 120 + 15 * (fileNameList.size() - 1);
	//else bottomY = 500; //파일명 10개까지만

	//					//drawText() 함수는 \n 을 인식 X
	//for (int i = fileNameList.size() - 1; i >= 0; --i) {
	//	uiStr = fileNameList.at(i);
	//	//if (uiStr.length() > 20) {
	//	//	uiStr.chop(uiStr.length() - 20); // Cut off at 20 characters
	//	//	uiStr.append("...");
	//	//}

	//	drawText(10, bottomY - 15 * j, uiStr, font);
	//	if (j == 9) break; //파일명 10개까지만 띄우기
	//	j++;
	//}

	glPopMatrix();
}
void ViewerModule::drawExpandedModeText()
{
	if (!Profile::machineProfile.machine_expanded_print_mode.value)
		return;

	qglColor(Qt::black);
	QString str(CustomTranslate::tr("Expanded Print Mode") + " : " + CustomTranslate::tr("Cartridge") + QString("(%1)").arg(Profile::machineProfile.machine_expanded_print_cartridgeIndex.value + 1));
	QFont font("Malgun Gothic", 9, QFont::Bold);
	drawText(10, this->geometry().height() - 15, str, font);
}

void ViewerModule::rotateSurfaceToBottom()
{
	if (changeBottomUIMode->isDummyVisible)
	{
		changeBottomUIMode->rotateToNewBottom();
		changeBottomUIMode->isDummyVisible = false;
	}
}

void ViewerModule::showContextMenu(QPoint pos)
{
	if (pressed != pos)
		return;
	
	ContextMenu contextMenu(this);
	QMenu *menu = contextMenu.getMenu();
	if (menu == nullptr)
		return;
	
	menu->exec(mapToGlobal(pos));
}

void ViewerModule::afterViewerChanged()
{
	if (Generals::isTranslateUIMode() || Generals::isLayerColorUIMode() || Generals::isRotateUIMode())
		setManipulatedFrame(manipulatedFrame);
	else
		setManipulatedFrame(nullptr);
	
	setModelColor();
}

void ViewerModule::setModelColor()
{
	if (Generals::isSupportUIMode())
	{
		for (auto it : modelContainer->models)
			it->setColor(QColor(68, 162, 255, 255));
	}
	else if (Generals::isPreviewUIMode())
	{
		for (auto it : modelContainer->models)
			it->setColor(QColor(0.7 * 255, 0.7 * 255, 0.7 * 255, 0.3 * 255));
	}
	else if (Generals::isAnalysisUIMode())
	{
		for (auto it : modelContainer->getSelectedModels())
			it->setColor(QColor(0.8 * 255, 0.8 * 255, 0.8 * 255, 1.0 * 255));
	}
	else if (Generals::isLayerColorUIMode())
	{
		for (auto it : modelContainer->models)
			it->setColor(CartridgeInfo::getCartColor(it->getCartridgeIndexes().front()));
	}
	else
	{
		for (auto it : modelContainer->models)
			it->setActiveColor();
	}
	updateGL();
}