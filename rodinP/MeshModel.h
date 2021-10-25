#pragma once
#include "Mesh.h"
#include "Renderer.h"
#include "ScaledFrame.h"
#include "AABB.h"
#include "PolygonLayer.h"
#include "UserProperties.h"

class LayerColorPlane;
class SliceLayer;
class SelectionHandle;
// Interface Class -------------------------------------------------------------
class IMeshModel : public QObject
{
	Q_OBJECT
public:
	IMeshModel();
	IMeshModel(IMeshModel &model_);
	virtual ~IMeshModel();

	virtual void resetRenderer() = 0;

	/// Mesh Processing Functions for a Model
	virtual Mesh* getMesh() = 0;
	virtual Mesh* getOuter() = 0;
	virtual std::vector<IMeshModel*> getModels() = 0;
	virtual std::vector<IMeshModel*> disJoin() = 0;
	virtual IMeshModel *clone() = 0;

	virtual std::vector<qglviewer::Vec> findBottomTriMesh(qglviewer::Vec _worldPos, qglviewer::Vec _onNormal) = 0;

	// Selection Functions for a Model
	virtual void select() = 0;
	virtual void deselect() = 0;
	virtual bool isSelected() = 0;
	virtual bool isSameID(int id_) = 0;
	virtual const int getID() = 0;
	virtual bool isDisabled() = 0;
	virtual void checkModelRange() = 0;
	virtual void setDisable(bool flag_) = 0;


	// Manipulation Functions for a Model
	virtual void manipulated() = 0;
	virtual void adjustPosition() = 0;
	virtual void translate(float dx_, float dy_, float dz_) = 0;
	virtual void resetZTranslate() = 0;
	virtual void rotate(qglviewer::Vec axisVec_, float rotAngle_) = 0;
	virtual void rotate(float pitch_, float yaw_, float roll_) = 0;
	virtual void rotateAroundAPoint(float x_rad, float y_rad, float z_rad) = 0;
	virtual void rotateAroundAPoint(float vecX_, float vecY_, float vecZ_,
		float posX_, float posY_, float posZ_, float angle_) = 0;
	virtual void rotateAroundPoint(qglviewer::Vec axisVec_, float rotAngle_, qglviewer::Vec position_) = 0;
	virtual void resetRotation() = 0;
	virtual void setScale(qglviewer::Vec scale_) = 0;
	virtual void resetScale() = 0;
	virtual void setScaledFactor(qglviewer::Vec scaledFactor_) = 0;
	virtual qglviewer::Vec getScaledFactor() = 0;
	virtual void setColorPlaneHeight(float z_) = 0;
	//return value
	//-1 : smaller than refSize
	//1 : larger than refSize
	virtual int compareModelSize(float refSize) = 0;
	virtual bool maxSize() = 0;

	virtual ScaledFrame getFrame() = 0;
	virtual void setFrame(ScaledFrame frame_) = 0;

	/// AABB Functions for a Model
	virtual void calcAABB() = 0;
	virtual AABB getAABB() = 0;
	virtual AABB getTotalBox() = 0;

	/// Rendering Functions for a Model
	virtual void drawLayoutColoredIndexMap(QMatrix4x4 proj_, QMatrix4x4 view_) = 0;
	virtual void drawNormalVectorMap(QMatrix4x4 proj_, QMatrix4x4 view_, bool nB_) = 0;

	virtual void drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_) = 0;
	virtual void drawPolygon(QMatrix4x4 proj_, QMatrix4x4 view_) = 0;
	virtual void drawPreviewMode(QMatrix4x4 proj_, QMatrix4x4 view_, float zHeightBase_ = 0) = 0;
	virtual void drawLayerColorMode(QMatrix4x4 proj_, QMatrix4x4 view_) = 0;
	virtual void drawLayerColorPlane(QMatrix4x4 proj_, QMatrix4x4 view_) = 0;
	virtual void drawSelectionHandle(QMatrix4x4 proj_, QMatrix4x4 view_) = 0;
	virtual void drawSelectionHandleForPicking(QMatrix4x4 proj_, QMatrix4x4 view_) = 0;

	/// Color Setting Functions for a Model
	virtual void toggleTransparency() = 0;
	virtual void refreshColor() = 0;
	virtual void setModelColor() = 0;
	virtual void setModelColorDark(int partIndex_ = -1) = 0;
	virtual void setModelColorDisable() = 0;
	virtual void setActiveColor() = 0;
	virtual void setColor(QColor color_) = 0;
	virtual void setLayerColor() = 0;
	//virtual void SetProposedOrientation() = 0;

	virtual void setFileName(QString filename_) = 0;
	virtual QString getFileName(bool usingFullFilePath_ = UserProperties::usingFullFilePath) = 0;
	virtual QString getOnlyFileName() = 0;
	virtual QFileInfo getFileInfo() = 0;
	virtual SelectionHandle* getSelectionHandle() = 0;

	virtual void setCartridgeIndex(int cartridgeIndex_, int partIndex_ = -1) = 0;
	virtual std::vector<int> getCartridgeIndexes() = 0;

	/// Lay Flat
	virtual void layFlat() = 0;

	virtual void savePosition();
	virtual void resetPosition();

	std::vector<SliceLayer> sliceLayers;
	std::vector<PolygonLayer> polygonLayers;
Q_SIGNALS:
	void signal_modelSelect();
	void signal_modelDeselect();
	//void signal_manipulated();
	//void signal_Orientation(int value);
	//void signal_updateGL();
	void signal_frameModified();
protected:
	AABB box;
	AABB totalBox;
	qglviewer::Vec orientPosition;
private:
	//현재 rodin은 serialize 사용하지 않으므로 일단 제거
	/*friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{

	}*/
};

// BasicMeshModel Class --------------------------------------------------------
class BasicMeshModel : public IMeshModel
{
	Q_OBJECT
private:
	BasicMeshModel();

public:
	BasicMeshModel(Mesh *outer_, Mesh *originalOuter_ = nullptr);
	BasicMeshModel(BasicMeshModel &model_);
	virtual ~BasicMeshModel();

	virtual void resetRenderer();

	/// Mesh Processing Functions for a Model
	virtual Mesh* getMesh();
	virtual Mesh* getOuter();
	virtual std::vector<IMeshModel*> getModels() { return std::vector<IMeshModel*> {this}; }
	virtual std::vector<IMeshModel*> disJoin()
	{
		return std::vector<IMeshModel*>();
	}
	virtual IMeshModel *clone()
	{
		return new BasicMeshModel(*this);
	}

	virtual std::vector<qglviewer::Vec> findBottomTriMesh(qglviewer::Vec _worldPos, qglviewer::Vec _onNormal);

	/// Selection Functions for a Model
	virtual void select();
	virtual void deselect();
	virtual bool isSelected();
	virtual bool isSameID(int id_);
	virtual const int getID() { return id; };
	virtual bool isDisabled();
	virtual void checkModelRange();
	virtual void setDisable(bool flag_);

	/// Manipulation Functions for a Model
	virtual void manipulated();
	virtual void adjustPosition();
	virtual void translate(float dx_, float dy_, float dz_);
	virtual void resetZTranslate();
	virtual void rotate(qglviewer::Vec axisVec_, float rotAngle_);
	virtual void rotate(float pitch_, float yaw_, float roll_);
	virtual void rotateAroundAPoint(float x_rad, float y_rad, float z_rad);
	virtual void rotateAroundAPoint(float vecX_, float vecY_, float vecZ_,
		float posX_, float posY_, float posZ_, float angle_);
	virtual void rotateAroundPoint(qglviewer::Vec axisVec_, float rotAngle_, qglviewer::Vec position_);
	virtual void resetRotation();
	virtual void setScale(qglviewer::Vec scale_);
	virtual void resetScale();
	virtual void setScaledFactor(qglviewer::Vec scaledFactor_) { scaledFactor = scaledFactor_; };
	virtual qglviewer::Vec getScaledFactor() { return scaledFactor; };
	qglviewer::Vec scaledFactor;
	virtual void setColorPlaneHeight(float z);
	virtual int compareModelSize(float refSize);
	virtual bool maxSize();

	virtual ScaledFrame getFrame()
	{
		return scaledFrame;
	}
	virtual void setFrame(ScaledFrame frame_)
	{
		scaledFrame = frame_;
		manipulated();
	}

	/// AABB Functions for a Model
	virtual void calcAABB();
	virtual AABB getAABB();
	virtual AABB getTotalBox();

	/// Check Range Validation Function
	//virtual int CheckModelRange();

	/// Rendering Functions for a Model
	virtual void drawLayoutColoredIndexMap(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawNormalVectorMap(QMatrix4x4 proj_, QMatrix4x4 view_, bool nB_);

	virtual void drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawPolygon(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawPreviewMode(QMatrix4x4 proj_, QMatrix4x4 view_, float zHeightBase_ = 0);
	virtual void drawLayerColorMode(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawLayerColorPlane(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawSelectionHandle(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawSelectionHandleForPicking(QMatrix4x4 proj_, QMatrix4x4 view_);

	/// Color Setting Functions for a Model
	virtual void toggleTransparency();
	virtual void refreshColor();
	virtual void setModelColor();
	virtual void setModelColorDark(int partIndex_ = -1);
	virtual void setModelColorDisable();
	virtual void setActiveColor();
	virtual void setColor(QColor color_);
	virtual void setLayerColor();
	//virtual void SetProposedOrientation();
	virtual void setFileName(QString filename_);
	virtual QString getFileName(bool usingFullFilePath_ = UserProperties::usingFullFilePath);
	virtual QString getOnlyFileName();
	virtual QFileInfo getFileInfo();
	virtual SelectionHandle* getSelectionHandle() { return selectionHandle; }

	virtual void setCartridgeIndex(int cartridgeIndex_, int partIndex_ = -1);
	virtual std::vector<int> getCartridgeIndexes();

	///Lay Flat
	virtual void layFlat();
private:
	Mesh *outer, *originalOuter, *worldOuter;
	bool isTransparent;
	ScaledFrame scaledFrame;
	QColor modelColor;
	SelectionHandle* selectionHandle;

	Renderer *matteRenderer;
	Renderer *normalMapOuterRenderer;

	Renderer *phongRenderer;
	Renderer *wireRenderer;
	//Renderer *phongDimmer;
	Renderer *layerColorRenderer;

	LayerColorPlane* layerColorPlane;

	float alphaShader;
	float alphaDimmer;

	int id;
	bool disabled;
	bool selected;
	QFileInfo fileinfo;
	int material;
	//QColor color;
	int cartridgeIndex;

	void init();
	//현재 rodin은 serialize 사용하지 않으므로 일단 제거
	/*friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<IMeshModel>(*this);
		ar & outer;
		ar & originalOuter;
		ar & scaledFrame;
		ar & floorFrame;
		ar & box;
		ar & totalBox;
		for (int i = 0; i < 3; i++)
			ar & scaledFactor[i];
	}*/
};

// JoinedMeshModel Class -------------------------------------------------------
class JoinedMeshModel : public IMeshModel 
{
	Q_OBJECT
private:
	JoinedMeshModel();

public:
	JoinedMeshModel(std::vector<IMeshModel*> joined_);
	JoinedMeshModel(JoinedMeshModel &model_);
	virtual ~JoinedMeshModel();

	virtual void resetRenderer() {};

	/// Mesh Processing Functions for a Model
	virtual Mesh* getMesh();
	virtual Mesh* getOuter();
	virtual std::vector<IMeshModel*> getModels() { return parts; }
	virtual std::vector<IMeshModel*> disJoin();
	virtual IMeshModel *clone()
	{
		return new JoinedMeshModel(*this);
	}

	virtual std::vector<qglviewer::Vec> findBottomTriMesh(qglviewer::Vec _worldPos, qglviewer::Vec _onNormal);

	/// Selection Functions for a Model
	virtual void select();
	virtual void deselect();
	virtual bool isSelected();
	virtual bool isSameID(int id_);
	virtual const int getID() { return parts.front()->getID(); };
	virtual bool isDisabled();
	virtual void checkModelRange();
	virtual void setDisable(bool flag_);

	/// Manipulation Functions for a Model
	virtual void manipulated();
	virtual void adjustPosition();
	virtual void translate(float dx_, float dy_, float dz_);
	virtual void resetZTranslate() {}
	virtual void rotate(qglviewer::Vec axisVec_, float rotAngle_);
	virtual void rotate(float pitch_, float yaw_, float roll_);
	virtual void rotateAroundAPoint(float x_rad, float y_rad, float z_rad);
	virtual void rotateAroundAPoint(float vecX_, float vecY_, float vecZ_,
		float posX_, float posY_, float posZ_, float angle_);
	virtual void rotateAroundPoint(qglviewer::Vec axisVec_, float rotAngle_, qglviewer::Vec position_);
	virtual void resetRotation();
	virtual void setScale(qglviewer::Vec scale_);
	virtual void resetScale();
	virtual void setScaledFactor(qglviewer::Vec scaledFactor_) {};
	virtual qglviewer::Vec getScaledFactor() { return parts.front()->getScaledFactor(); };
	virtual void setColorPlaneHeight(float z_);
	virtual int compareModelSize(float refSize);
	virtual bool maxSize();

	virtual ScaledFrame getFrame()
	{
		return scaledFrame;
	}
	virtual void setFrame(ScaledFrame frame_)
	{
		scaledFrame = frame_;
		manipulated();
	}

	/// AABB Functions for a Model
	virtual void calcAABB();
	virtual AABB getAABB();
	virtual AABB getTotalBox();

	/// Check Range Validation Function
	//virtual int CheckModelRange();

	/// Rendering Functions for a Model
	virtual void drawLayoutColoredIndexMap(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawNormalVectorMap(QMatrix4x4 proj_, QMatrix4x4 view_, bool nB_);

	virtual void drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawPolygon(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawPreviewMode(QMatrix4x4 proj_, QMatrix4x4 view_, float zHeightBase_ = 0);
	virtual void drawLayerColorMode(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawLayerColorPlane(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawSelectionHandle(QMatrix4x4 proj_, QMatrix4x4 view_);
	virtual void drawSelectionHandleForPicking(QMatrix4x4 proj_, QMatrix4x4 view_);

	/// Color Setting Functions for a Model
	virtual void toggleTransparency();
	virtual void refreshColor();
	virtual void setModelColor();
	virtual void setModelColorDark(int partIndex_ = -1);
	virtual void setModelColorDisable();
	virtual void setActiveColor();
	virtual void setColor(QColor color_);
	virtual void setLayerColor();
	//virtual void SetProposedOrientation();
	virtual void setFileName(QString filename_) {};
	virtual QString getFileName(bool usingFullFilePath_ = UserProperties::usingFullFilePath);
	virtual QString getOnlyFileName();
	virtual QFileInfo getFileInfo();
	virtual SelectionHandle* getSelectionHandle() { return selectionHandle; }

	virtual void setCartridgeIndex(int cartridgeIndex_, int partIndex_ = -1);
	virtual std::vector<int> getCartridgeIndexes();

	/// Lay Flat
	virtual void layFlat();
private:
	Mesh *worldOuter;
	bool isTransparent;
	bool isDisjoined;
	bool selected;
	std::vector<IMeshModel*> parts;
	ScaledFrame scaledFrame;
	LayerColorPlane* layerColorPlane;
	SelectionHandle* selectionHandle;

	void init();

	//현재 rodin은 serialize 사용하지 않으므로 일단 제거
	/*friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<IMeshModel>(*this);
		ar & parts;
		ar & floorFrame;
		ar & box;
		ar & totalBox;
	}*/
};

class AABBGetter
{
public:
	AABB operator()(const std::vector<IMeshModel*> &models_);
};

class TotalBoxGetter
{
public:
	AABB operator()(const std::vector<IMeshModel*> &models_);
};

class ToWorldMesh
{
public:
	Mesh *operator()(Mesh* local_, ScaledFrame &scaledFrame_)
	{
		Mesh *world = new Mesh(*local_);
		for (Mesh::VertexIter vit = world->vertices_begin();
			vit != world->vertices_end();
			vit++
			) {
			Mesh::Point lp = local_->point(vit);
			qglviewer::Vec wp =
				scaledFrame_.toWorldCoords(
					qglviewer::Vec(lp[0], lp[1], lp[2]));
			world->set_point(vit, Mesh::Point(wp[0], wp[1], wp[2]));
		}
		world->update_normals();
		return world;
	}
};

class ToLocalMesh
{
public:
	Mesh *operator()(Mesh* world_, ScaledFrame &scaledFrame_)
	{
		Mesh *local = new Mesh(*world_);
		for (Mesh::VertexIter vit = local->vertices_begin();
			vit != local->vertices_end();
			vit++)
		{
			Mesh::Point wp = world_->point(vit);
			qglviewer::Vec lp =
				scaledFrame_.toLocalCoords(
					qglviewer::Vec(wp[0], wp[1], wp[2]));
			local->set_point(vit, Mesh::Point(lp[0], lp[1], lp[2]));
		}
		local->update_normals();
		return local;
	}
};