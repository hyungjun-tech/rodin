#include "stdafx.h"
#include "ModelContainer.h"
#include "CartridgeInfo.h"
//#include "dataStorage.h"

ModelContainer::ModelContainer()
	: supportData(new SupportData())
	, printingInfo(new PrintingInfo())
{
}

ModelContainer::~ModelContainer()
{
	// Release Memories of model storage ...
	for (int i = 0; i < models.size(); i++) {
		disconnect(models[i], 0, 0, 0);
		delete models[i];
	}
	delete supportData;
	delete printingInfo;
}

/// Model ADD & DELETE Functions -------------------------------------------------------------------
void ModelContainer::addNewModel(IMeshModel* model_)
{
	models.push_back(model_);
	//DeselectAll();
	//model_->Select();
	setConnection(model_);
}

void ModelContainer::deleteSelectedModels()
{
	for (int i = 0; i < models.size(); i++)
	{
		if (models[i]->isSelected())
		{
			IMeshModel *tempModel = models[i];
			models.erase(models.begin() + i);
			delete tempModel;
			i--;
		}
	}
}

void ModelContainer::deleteAllModels()
{
	for (int i = 0; i < models.size(); i++)
	{
		IMeshModel *tempModel = models[i];
		models.erase(models.begin() + i);
		delete tempModel;
		i--;
	}
}

void ModelContainer::deleteModel(IMeshModel* target_)
{
	std::vector<IMeshModel*>::iterator iter = std::find(models.begin(), models.end(), target_);
	if (iter == models.end())
		return;

	models.erase(iter);
	delete target_;
}

void ModelContainer::erase(std::vector<IMeshModel*> list_)
{
	for (int i = 0; i < models.size(); i++)
	{
		if (std::find(list_.begin(), list_.end(), models[i]) == list_.end())
			continue;

		models.erase(models.begin() + i);
		i--;
	}
}

void ModelContainer::replace(IMeshModel* successor_, IMeshModel* predecessor_)
{
	for (int i = 0; i < models.size(); i++)
	{
		if (models[i] == predecessor_)
		{
			IMeshModel *temp = models[i];
			setConnection(successor_);
			models[i] = successor_;
			delete temp;
		}
	}
	successor_->select();
}

/// Model SELECTION Functions ----------------------------------------------------------------------
bool ModelContainer::hasAnyModel()
{
	if (models.size() == 0)
		return false;
	else
		return true;
}
bool ModelContainer::hasAnySelectedModel()
{
	for (auto model : models)
	{
		if (model->isSelected())
			return true;
	}
	return false;
}

bool ModelContainer::isSelected(int id_)
{
	for (auto model : models)
	{
		if (model->isSameID(id_))
			if (model->isSelected())
				return true;
	}
	return false;
}

void ModelContainer::toggleModelSelection(int id_)
{
	for (auto model : models)
	{
		if (model->isSameID(id_))
			if (model->isSelected())
				model->deselect();
			else
				model->select();
	}
}

void ModelContainer::select(int id_)
{
	for (auto model : models)
	{
		if (model->isSameID(id_))
			model->select();
	}
}
void ModelContainer::selectExceptOthers(int id_)
{
	for (auto model : models)
	{
		if (model->isSameID(id_))
			model->select();
		else
			model->deselect();
	}
}

void ModelContainer::selectExceptOthers(std::vector<int> ids_)
{
	for (auto id : ids_)
	{
		for (auto model : models)
			if (!model->isSameID(id))
				model->deselect();
	}
	for (auto id : ids_)
		select(id);
}

void ModelContainer::deSelect(int id_)
{
	for (auto model : models)
	{
		if (model->isSameID(id_))
			model->deselect();
	}
}

bool ModelContainer::isMultiSelected()
{
	int count = 0;
	for (auto model : models)
	{
		if (model->isSelected()) {
			count++;
			if (count >= 2)
				return true;
		}
	}
	return false;
}

void ModelContainer::deselectAll()
{
	for (auto model : models)
		model->deselect();
}

void ModelContainer::setConnection(IMeshModel *model_)
{
	connect(model_, SIGNAL(signal_modelSelect()), this, SIGNAL(signal_modelSelect()));
	connect(model_, SIGNAL(signal_modelDeselect()), this, SIGNAL(signal_modelDeselect()));
	connect(model_, SIGNAL(destroyed()), this, SIGNAL(signal_modelDeleted()));
	//connect(model_, SIGNAL(signal_manipulated()), this, SIGNAL(signal_manipulated()));
	//connect(model_, SIGNAL(signal_updateGL()), this, SIGNAL(signal_updateGL()));
	connect(model_, SIGNAL(signal_frameModified()), this, SLOT(modelChanged()));
	connect(model_, SIGNAL(destroyed()), this, SLOT(modelChanged()));

	emit signal_modelAdded();
}

int ModelContainer::getSelectedModelNumber(int id_)
{
	int ret = 0;
	for (int i = 0; i < models.size(); i++)
	{
		if (models[i]->isSameID(id_))
			ret = i;
	}
	return ret;
}

bool ModelContainer::isDisabled()
{
	for (auto model : models)
	{
		if (model->isDisabled())
			return true;
	}
	return false;
}

void ModelContainer::checkModelRange()
{
	for (auto model : models)
		model->checkModelRange();
}
void ModelContainer::refreshColor(int id_)
{
	for (auto model : models)
	{
		if (model->isSameID(id_))
			model->refreshColor();
	}
}
void ModelContainer::changeColorDark(int id_)
{
	for (auto model : models)
	{
		if (model->isSameID(id_))
			model->setModelColorDark();
	}
}
void ModelContainer::refreshModelColor(int modelId_)
{
	if (modelId_ < 0)
		return;
	if (modelId_ >= models.size())
		return;
	models[modelId_]->refreshColor();
}
void ModelContainer::changeModelColorDark(int modelId_, int subModelId_)
{
	if (modelId_ < 0)
		return;
	if (modelId_ >= models.size())
		return;
	models[modelId_]->setModelColorDark(subModelId_);
}
void ModelContainer::getModelIndex(int id_, int& modelId_, int& subModelId_)
{
	modelId_ = -1;
	subModelId_ = 0;
	for (int i = 0; i < models.size(); i++)
	{
		if (models[i]->isSameID(id_))
		{
			modelId_ = i;
			std::vector<IMeshModel*> subModels = models[i]->getModels();
			int size = subModels.size();
			if (size == 1)
				return;
			for (int j = 0; j < size; j++)
			{
				if (subModels[j]->isSameID(id_))
				{
					subModelId_ = j;
					return;
				}
			}
		}
	}
}
IMeshModel* ModelContainer::findModel(int id_)
{
	for (auto model : models)
		if (model->isSameID(id_))
			return model;

	return nullptr;
}

void ModelContainer::setCartridgeIndex(int _modelId, int _cartridgeIndex, int _partIndex)
{
	if (_modelId < 0)
		return;
	if (_modelId >= models.size())
		return;
	models[_modelId]->setCartridgeIndex(_cartridgeIndex, _partIndex);
	CartridgeInfo::setUseStateForModel(this);
}
void ModelContainer::setCartridgeIndexAll(int _cartridgeIndex)
{
	for (auto model : models)
		model->setCartridgeIndex(_cartridgeIndex);
	CartridgeInfo::setUseStateForModel(this);
}
void ModelContainer::resetCartridge()
{
	int cartridgeCount = CartridgeInfo::getCartCount();
	int cartridgeIndex = 0;
	if (Profile::machineProfile.machine_expanded_print_mode.value)
		cartridgeIndex = Profile::machineProfile.machine_expanded_print_cartridgeIndex.value;
	for (auto model : models)
	{
		for (auto part : model->getModels())
		{
			int modelCart = part->getCartridgeIndexes().front();
			if (modelCart >= cartridgeCount)
				part->setCartridgeIndex(cartridgeIndex);
		}
	}
}