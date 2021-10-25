#pragma once

class ViewerModule;
class ModelContainer;
class ContextMenu : public QObject
{
	Q_OBJECT

public:
	ContextMenu(ViewerModule* viewerModule_);
	~ContextMenu();

	QMenu* getMenu();
private slots:
	void moveToCenter();
	void deleteObject();
	void multiplyObject();
	void layFlat();
	void mergeObjects();
	void splitObjects();
	void deleteAllObjects();
	void reloadAllObjects();
	void deleteAllSupports();
	void resetAllSupports();
private:
	ViewerModule *viewerModule;
	ModelContainer *modelContainer;
	void createActions();
	QMenu* createMenu_selectedModel();
	QMenu* createMenu_noSelectedModel();
	QMenu* createMenu_previewUIMode();
	QMenu* createMenu_supportUIMode();
	QMenu *menu;

	QAction *moveToCenterAct;
	QAction *deleteObjectAct;
	QAction *multiplyObjectAct;
	QAction *layFlatAct;
	QAction *mergeObjectsAct;
	QAction *splitObjectsAct;
	QAction *deleteAllObjectsAct;
	QAction *reloadAllObjectsAct;
	QAction *deleteAllSupportsAct;
	QAction *resetAllSupportsAct;
};
