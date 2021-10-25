#pragma once
#include "ModelContainer.h"
#include "ManipulatedFrame.h"
#include "GCodeGenerator.h"
#include "CameraModule.h"

class UIMode;
class TranslateUIMode;
class RotateUIMode;
class PreviewUIMode;
class ChangeBottomUIMode;
class SupportUIMode;
class AnalysisUIMode;
class LayerColorUIMode;
class VolumeAnalysis;
class ProgressHandler;
class PrinterBody;

class ViewerModule : public QGLViewer
{
	Q_OBJECT
public:
	ViewerModule(QWidget *parent = 0);
	~ViewerModule();
	bool getToggle(int index_);
	PrinterBody* getPrinterBody() { return printerBody; }

	/// UIMode Set Functions
	void setTranslateUIMode();
	void setRotateUIMode();
	void setChangeBottomUIMode();
	void setSupportUIMode();
	void setAnalysisUIMode();
	void setLayerColorUIMode();
	Generals::ViewerMode getUIModeType();
	QString getOnlyFileName();

	/// Model IO Functions
	void loadModels(QStringList fileList_);
	int loadModel(QString fileName_);
	bool saveModels(QString fileName_);
	void deleteSelectedModels();
	void reloadAllModels();
	void saveScene(QString fileName_);
	void loadScene(QString fileName_);
	int loadModel(QString fileName_, QMatrix4x4 transMatrix);		// Added to load exsocad files

	/// Mesh Processing Functions
	void translateSelectedModels(float x_, float y_, float z_);
	void placeSelectedModelsToBedCenter();
	void translateModelsToFittablePlace(std::vector<IMeshModel*> models_ = std::vector<IMeshModel*>{});
	void resetTranslateSelectedModels();
	//void rotateSelectedModels(float x_deg, float y_deg, float z_deg);
	void rotateSelectedModels(float rad_x, float rad_y, float rad_z);		// radian으로 변경	// ijeong 2019 08
	void resetRotateSelectedModels();
	void scaleSelectedModels(float x_, float y_, float z_);
	void scalemmSelectedModels(float x_, float y_, float z_);
	void resetScaleSelectedModels();
	void maxSizeSelectedMeshes();
	qglviewer::Vec getScaleSelectedModels();
	bool checkScaleRatio();
	void autoScaling_inchToMM(IMeshModel* model_);
	IMeshModel* autoScaling_maxSize(IMeshModel* model_);
	std::vector<qglviewer::Vec> findBottomMesh(int no, qglviewer::Vec _worldPos, qglviewer::Vec _onNormal);


	void joinModels();
	void undoJoinModels();
	void multiplySelectedModels(int numofCopies_);
	//void SetProposedOrientationSelectedModels();
	
	void layFlat();
	void deleteAllSupports();
	void resetAllSupports();

	/// UI Response Functions
	//void UpdateGCodePath(std::vector<std::vector<GCodePathforDraw>> path_);
	//void SetZHeightBase(float zHeightBase_);
	void arrangeModels(std::vector<IMeshModel*> fix_ = std::vector<IMeshModel*>{});
	void rearrangeModels();

	/// Slice Functions
	//void SliceAll();

	/// LDI Functions
	ModelContainer* modelContainer;
	ManipulatedFrame* manipulatedFrame;

	void rotateSurfaceToBottom();
	void setModelColor();

	VolumeAnalysis* getVolumeAnalysis();

	void fitToModel();
	void fitToModel(IMeshModel* model);
	void fitToModel(AABB aabb_);

	void afterViewerChanged();
	Camera* camera() { return viewerCamera; };
	virtual void keyPressEvent(QKeyEvent *e);
private:
	UIMode *uiMode;
	TranslateUIMode *translateUIMode;
	RotateUIMode *rotateUIMode;
	PreviewUIMode *previewUIMode;
	ChangeBottomUIMode *changeBottomUIMode;
	SupportUIMode *supportUIMode;
	AnalysisUIMode *analysisUIMode;
	LayerColorUIMode *layerColorUIMode;
	PrinterBody* printerBody;
	Camera* viewerCamera;

	/// QGLViewer Private Functions
	virtual void init();
	virtual void preDraw();
	virtual void draw();

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent *e);
	virtual void wheelEvent(QWheelEvent *e);

	virtual void resizeEvent(QResizeEvent *e = nullptr);

	void constraintMouse();
	void initializeManipulatedFrame();

	void setView(qglviewer::Vec pViewDir, qglviewer::Vec pCam, double camDistance_ = -1);
	qglviewer::Vec vectorRotate(qglviewer::Vec pBeforeVec, int pDegree);

	void drawCornerAxis();
	void drawAxis(int _size, qreal _length /*= 1*/);
	void drawAxisText(int _size, qreal _length /*= 1*/);
	void drawFileName();
	void drawExpandedModeText();

	static const float camMaxDistance;
	static const float camMinDistance;
	QPoint pressed;
	std::vector<bool> m_toggle;
	void setToggleAll(bool state);
	void setToggle(int index_);

public Q_SLOTS:
	void resetBed();
	void setPreviewUIMode(ModelDataStorage* dataStorage_);
	//view direction
	void isometricCamera();
	void topView();
	void rightView();
	void leftView();
	void frontView();

	/// Model IO Functions
	void deleteAllModels();

	/// UIMode Set Functions
	void refreshUI();
	void showContextMenu(QPoint pos);
	void changeLayerIndexForSupport(int index_);
	void changeLayerIndexForPreview(int index_);
	void changeLayerIndexForLayerColor(int index_ = -1);
	void toggleExtruderMode(bool flag_);
	void toggleCurrentLayerOnly(bool flag_);
	void toggleShowTravelPath(bool flag_);

Q_SIGNALS:
	void signal_viewer_changed(Generals::ViewerMode);
	void signal_updateMaxLayerForPreview(int);
	void signal_mesh_has_errors();
	//void sig_processStart();
	void signal_processDone();
	void signal_mouseOnModel(int, int);
	void signal_mouseOutModel(int, int);
};
