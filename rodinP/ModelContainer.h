#pragma once
#include "MeshModel.h"
#include "SupportData.h"
#include "PrintingInfo.h"

//class DataStorage;
class ModelContainer : public QObject
{
	Q_OBJECT
public:
	ModelContainer();
	~ModelContainer();

	std::vector<IMeshModel*> models;
	SupportData* supportData;
	PrintingInfo* printingInfo;
	//DataStorage* dataStorage;

	//joinedMeshModel을 쪼개서 반환함.
	std::vector<IMeshModel*> getBasicModels()
	{
		std::vector<IMeshModel*> basicModels;
		for (auto it : models)
		{
			for (auto model : it->getModels())
			{
				basicModels.push_back(model);
			}
		}
		return basicModels;
	}

	std::vector<IMeshModel*> getSelectedModels()
	{
		std::vector<IMeshModel*> selecteds;
		for (int i = 0; i < models.size(); i++)
		{
			if (!models[i]->isSelected())
				continue;

			selecteds.push_back(models[i]);
		}
		return selecteds;
	}
	std::vector<IMeshModel*> getNoSelectedModels()
	{
		std::vector<IMeshModel*> selecteds;
		for (int i = 0; i < models.size(); i++)
		{
			if (models[i]->isSelected())
				continue;

			selecteds.push_back(models[i]);
		}
		return selecteds;
	}
	std::vector<IMeshModel*> getModelsExceptOne(IMeshModel* model_)
	{
		std::vector<IMeshModel*> selecteds;
		std::vector<IMeshModel*>::iterator it;
		for (int i = 0; i < models.size(); i++)
		{
			if (model_ == models[i])
				continue;
			selecteds.push_back(models[i]);
		}
		return selecteds;
	}
	std::vector<IMeshModel*> getModelsExceptOne(std::vector<IMeshModel*> models_)
	{
		std::vector<IMeshModel*> selecteds;
		std::vector<IMeshModel*>::iterator it;
		for (int i = 0; i < models.size(); i++)
		{
			it = std::find(models_.begin(), models_.end(), models[i]);
			if (it != models_.end())
				continue;
			selecteds.push_back(models[i]);
		}
		return selecteds;
	}

	std::vector<IMeshModel*> getLatestModel(int no)		// ijeong	// 2019 08
	{
		std::vector<IMeshModel*> selected;
		selected.push_back(models[no]);
		return selected;
	}

	/// Model ADD & DELETE Functions
	void addNewModel(IMeshModel* model_);
	void deleteSelectedModels();
	void deleteAllModels();
	void erase(std::vector<IMeshModel*> list_);
	void deleteModel(IMeshModel* target_);
	void replace(IMeshModel* successor_, IMeshModel* predecessor_);

	/// Model Selections Functions
	bool hasAnyModel();
	bool hasAnySelectedModel();
	bool isSelected(int id_);
	bool isMultiSelected();
	void toggleModelSelection(int id_);
	void select(int id_);
	void selectExceptOthers(int id_);
	void selectExceptOthers(std::vector<int> ids_);
	void deSelect(int id_);
	void deselectAll();
	void setConnection(IMeshModel* model_);
	int getSelectedModelNumber(int id_);		// ijeong	// 2019 08

	bool isDisabled();
	void checkModelRange();

	void refreshColor(int Id_);
	void changeColorDark(int Id_);
	void refreshModelColor(int modelId_);
	void changeModelColorDark(int modelId_, int subModelId_ = -1);
	void getModelIndex(int id_, int& modelId_, int& subModelId_);
	IMeshModel* findModel(int id_);
	void setCartridgeIndex(int _modelId, int _cartridgeIndex, int _partIndex);
	void setCartridgeIndexAll(int _cartridgeIndex);
	void resetCartridge();
private slots:
	void modelChanged() { supportData->modelChanged(); }
Q_SIGNALS:
	void signal_modelSelect();
	void signal_modelDeselect();
	void signal_modelDeleted();
	void signal_modelAdded();
	//void signal_manipulated();
	void signal_updateGL();
	void signal_dialogContents(QString p_message, int p_type, bool p_OkVisible = true, bool p_cancelVisible = false, bool p_yesVisible = false, bool p_noVisible = false, bool p_delay = false);
};
