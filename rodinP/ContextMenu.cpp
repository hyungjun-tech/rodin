#include "stdafx.h"
#include "ContextMenu.h"
#include "ViewerModule.h"
#include "MultiplyDialog.h"

ContextMenu::ContextMenu(ViewerModule* viewerModule_)
	: viewerModule(viewerModule_)
	, modelContainer(viewerModule_->modelContainer)
	, menu(nullptr)
	, moveToCenterAct(nullptr)
	, deleteObjectAct(nullptr)
	, multiplyObjectAct(nullptr)
	, layFlatAct(nullptr)
	, mergeObjectsAct(nullptr)
	, splitObjectsAct(nullptr)
	, deleteAllObjectsAct(nullptr)
	, reloadAllObjectsAct(nullptr)
	, deleteAllSupportsAct(nullptr)
	, resetAllSupportsAct(nullptr)
{
	//createMenus();
}

ContextMenu::~ContextMenu()
{
	if (menu) delete menu;

	if (moveToCenterAct) delete moveToCenterAct;
	if (deleteObjectAct) delete deleteObjectAct;
	if (multiplyObjectAct) delete multiplyObjectAct;
	if (layFlatAct) delete layFlatAct;
	if (mergeObjectsAct) delete mergeObjectsAct;
	if (splitObjectsAct) delete splitObjectsAct;
	if (deleteAllObjectsAct) delete deleteAllObjectsAct;
	if (reloadAllObjectsAct) delete reloadAllObjectsAct;
	if (deleteAllSupportsAct) delete deleteAllSupportsAct;
	if (resetAllSupportsAct) delete resetAllSupportsAct;
}
void ContextMenu::createActions()
{
	moveToCenterAct = new QAction(tr("Move to center"));
	deleteObjectAct = new QAction(tr("Delete object"));
	multiplyObjectAct = new QAction(tr("Multiply object"));
	layFlatAct = new QAction(tr("Lay Flat"));
	mergeObjectsAct = new QAction(tr("Merge objects"));
	splitObjectsAct = new QAction(tr("Split objects"));
	deleteAllObjectsAct = new QAction(tr("Delete all objects"));
	reloadAllObjectsAct = new QAction(tr("Reload all objects"));
	deleteAllSupportsAct = new QAction(tr("Delete all supports"));
	resetAllSupportsAct = new QAction(tr("Reset all supports"));

	//reloadObjectAct = new QAction(tr("Reload object"));
	connect(moveToCenterAct, SIGNAL(triggered()), this, SLOT(moveToCenter()));
	connect(layFlatAct, SIGNAL(triggered()), this, SLOT(layFlat()));
	connect(multiplyObjectAct, SIGNAL(triggered()), this, SLOT(multiplyObject()));
	connect(deleteObjectAct, SIGNAL(triggered()), this, SLOT(deleteObject()));
	connect(mergeObjectsAct, SIGNAL(triggered()), this, SLOT(mergeObjects()));
	connect(splitObjectsAct, SIGNAL(triggered()), this, SLOT(splitObjects()));
	connect(deleteAllObjectsAct, SIGNAL(triggered()), this, SLOT(deleteAllObjects()));
	connect(reloadAllObjectsAct, SIGNAL(triggered()), this, SLOT(reloadAllObjects()));
	connect(deleteAllSupportsAct, SIGNAL(triggered()), this, SLOT(deleteAllSupports()));
	connect(resetAllSupportsAct, SIGNAL(triggered()), this, SLOT(resetAllSupports()));

	/*newAct = new QAction(tr("&New"), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Create a new file"));*/
	//connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
}
QMenu* ContextMenu::createMenu_selectedModel()
{
	createActions();
	QList<QAction*> actionList;
	bool isMultiSelected = modelContainer->isMultiSelected();
	bool isJoinedModel = (!isMultiSelected && modelContainer->getSelectedModels().front()->getModels().size() > 1);

	moveToCenterAct->setDisabled(isMultiSelected);
	mergeObjectsAct->setEnabled(isMultiSelected);
	splitObjectsAct->setEnabled(isJoinedModel);

	actionList.append(moveToCenterAct);
	actionList.append(deleteObjectAct);
	actionList.append(multiplyObjectAct);
	actionList.append(layFlatAct);
	actionList.append(mergeObjectsAct);
	actionList.append(splitObjectsAct);

	actionList.append(deleteAllObjectsAct);
	actionList.append(reloadAllObjectsAct);
	
	menu = new QMenu();
	menu->addActions(actionList);
	return menu;
}
QMenu* ContextMenu::createMenu_noSelectedModel()
{
	createActions();
	QList<QAction*> actionList;
	actionList.append(deleteAllObjectsAct);
	actionList.append(reloadAllObjectsAct);

	menu = new QMenu();
	menu->addActions(actionList);
	return menu;
}
QMenu* ContextMenu::createMenu_previewUIMode()
{
	createActions();
	QList<QAction*> actionList;
	actionList.append(deleteAllObjectsAct);

	menu = new QMenu();
	menu->addActions(actionList);
	return menu;
}
QMenu* ContextMenu::createMenu_supportUIMode()
{
	createActions();
	QList<QAction*> actionList;
	actionList.append(deleteAllSupportsAct);
	actionList.append(resetAllSupportsAct);

	menu = new QMenu();
	menu->addActions(actionList);
	return menu;
}
QMenu* ContextMenu::getMenu()
{
	if (Generals::isTranslateUIMode() || Generals::isRotateUIMode())
	{
		if (!modelContainer->hasAnyModel())
			return nullptr;

		if (modelContainer->hasAnySelectedModel())
			return createMenu_selectedModel();
		else
			return createMenu_noSelectedModel();
	}
	else if (Generals::isPreviewUIMode())
		return createMenu_previewUIMode();
	else if (Generals::isSupportUIMode())
		return createMenu_supportUIMode();
	else
		return nullptr;
}

void ContextMenu::moveToCenter()
{
	viewerModule->placeSelectedModelsToBedCenter();
	viewerModule->update();
}
void ContextMenu::deleteObject()
{
	viewerModule->deleteSelectedModels();
	viewerModule->update();
}
void ContextMenu::multiplyObject()
{
	MultiplyDialog multiplyDialog(viewerModule);
	multiplyDialog.exec();
}
void ContextMenu::layFlat()
{
	viewerModule->layFlat();
	viewerModule->update();
}
void ContextMenu::mergeObjects()
{
	viewerModule->joinModels();
	viewerModule->update();
}
void ContextMenu::splitObjects()
{
	viewerModule->undoJoinModels();
	viewerModule->update();
}
void ContextMenu::deleteAllObjects()
{
	viewerModule->deleteAllModels();
	viewerModule->update();
}
void ContextMenu::reloadAllObjects()
{
	viewerModule->reloadAllModels();
	viewerModule->update();
}
void ContextMenu::deleteAllSupports()
{
	viewerModule->deleteAllSupports();
}
void ContextMenu::resetAllSupports()
{
	viewerModule->resetAllSupports();
}