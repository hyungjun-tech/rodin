#include "stdafx.h"
#include "MeshModel.h"
#include "PickingIdxManager.h"
#include "BottomSurfaceManager.h"
#include "LayerDatasSet.h"
#include "CartridgeInfo.h"
#include "LayerColorPlane.h"
#include "ConvexHullStatic.h"
#include "ManipulatorHandle.h"
#include "SaveFileUIMode.h"
#include "MeshModelCalculator.h"
//#define CROSS_SECTION

class Renderer;
class BottomSurfaceManager;
class PickingIdxManager;
class Mesh;

/// Interface Class =================================================================================
IMeshModel::IMeshModel()
{
}
IMeshModel::IMeshModel(IMeshModel &model_)
	: box(model_.box)
	, totalBox(model_.totalBox)
{
}

IMeshModel::~IMeshModel()
{
}
void IMeshModel::savePosition()
{
	orientPosition = box.getFloorCenter();
}
void IMeshModel::resetPosition()
{
	qglviewer::Vec move = orientPosition - box.getFloorCenter();
	translate(move[0], move[1], move[2]);
}

// BasicMeshModel Class ============================================================================
BasicMeshModel::BasicMeshModel()
	: outer(nullptr),
	originalOuter(nullptr),
	worldOuter(nullptr),
	scaledFactor(1, 1, 1),
	material(-1),
	cartridgeIndex(0)
{
	init();
}

BasicMeshModel::BasicMeshModel(Mesh *outer_, Mesh *originalOuter_)
	: outer(outer_),
	originalOuter(originalOuter_),
	worldOuter(nullptr),
	scaledFactor(1, 1, 1),
	material(-1),
	cartridgeIndex(0)
{
	init();
	if (originalOuter == nullptr)
		originalOuter = new Mesh(*outer);
	//qDebug() << "2 mesh_id : " << id << "/ indexedColor : " << intIndexedColor[0] << intIndexedColor[1] << intIndexedColor[2];

	calcAABB();
}

BasicMeshModel::BasicMeshModel(BasicMeshModel &model_)
	: IMeshModel(model_)
	, outer(new Mesh(*model_.outer)),
	originalOuter(new Mesh(*model_.originalOuter)),
	worldOuter(nullptr),

	scaledFrame(model_.scaledFrame),
	fileinfo(model_.fileinfo),
	scaledFactor(model_.scaledFactor),
	material(model_.material),
	cartridgeIndex(model_.cartridgeIndex)
{
	init();

	qglviewer::Vec intIndexedColor = PickingIdxManager::encodeID(id);
	//qDebug() << "3 mesh_id : " << id << "/ indexedColor : " << intIndexedColor[0] << intIndexedColor[1] << intIndexedColor[2];


	/*int r = id / 65536;
	int g = (id - r * 65536) / 256;
	int b = (id - r * 65536 - g * 256);

	double v_r = r / 255.0;
	double v_g = g / 255.0;
	double v_b = b / 255.0;

	qDebug() << "r : " << r << ", g : " << g << ", b : " << b;
	qDebug() << "v : " << v_r << v_g << v_b;*/

}

BasicMeshModel::~BasicMeshModel()
{
	delete outer;
	if (worldOuter)
		delete worldOuter;
	if (originalOuter)
		delete originalOuter;

	delete matteRenderer;
	delete normalMapOuterRenderer;

	delete phongRenderer;
	//delete phongDimmer;
	delete layerColorRenderer;
	if (layerColorPlane)
		delete layerColorPlane;
	delete selectionHandle;
}

void BasicMeshModel::init()
{
	matteRenderer = new NonShadedRenderer();
	normalMapOuterRenderer = new NormalMapRGBRenderer();

	phongRenderer = new PhongShadedRenderer();
	wireRenderer = new NonShadedRenderer();
	//phongDimmer = new PhongShadedRenderer();
	layerColorRenderer = new PhongShadedRenderer();

	isTransparent = false;
	alphaShader = 1.0;
	alphaDimmer = 0.0;

	id = PickingIdxManager::getMeshID();

	selected = false;
	layerColorPlane = new LayerColorPlane();

	qglviewer::Vec intIndexedColor = PickingIdxManager::encodeID(id);
	matteRenderer->setColor(intIndexedColor[0], intIndexedColor[1], intIndexedColor[2]);

	normalMapOuterRenderer->setColor(0, 0, 0);

	phongRenderer->setColor(0.7, 0.7, 0.7, alphaShader);
	wireRenderer->setColor(0.0, 0.0, 0.0);
	//phongDimmer->setColor(0.7, 0.7, 0.7, alphaDimmer);
	layerColorRenderer->setColor(0.7, 0.7, 0.7);
	selectionHandle = new SelectionHandle();

	connect(&scaledFrame.frame, SIGNAL(modified()), this, SIGNAL(signal_frameModified()));
	//CalcAABB();
	if (Profile::machineProfile.machine_expanded_print_mode.value)
		setCartridgeIndex(Profile::machineProfile.machine_expanded_print_cartridgeIndex.value);
	setActiveColor();
}

void BasicMeshModel::resetRenderer()
{
	matteRenderer->deleteReference();
	normalMapOuterRenderer->deleteReference();
	wireRenderer->deleteReference();

	phongRenderer->deleteReference();
	//phongDimmer->deleteReference();
	layerColorRenderer->deleteReference();
}

/// [BasicMeshModel] Mesh Processing Functions -----------------------------------------------------
Mesh * BasicMeshModel::getMesh()
{
	if (worldOuter)
		delete worldOuter;
	worldOuter = ToWorldMesh()(outer, scaledFrame);
	return worldOuter;
}
Mesh * BasicMeshModel::getOuter()
{
	return outer;
}

std::vector<qglviewer::Vec> BasicMeshModel::findBottomTriMesh(qglviewer::Vec _worldPos, qglviewer::Vec _onNormal)
{
	// Back up Original Meshes
	std::vector<qglviewer::Vec> vertexList_1;
	qglviewer::Vec b_v;

	BottomSurfaceManager bm;
	qglviewer::Vec worldNormal = scaledFrame.toWorldVector(_onNormal);

	vertexList_1 = bm.GenerateBottomMesh(getMesh(), _worldPos, worldNormal);
#ifdef _DEBUG
	qglviewer::Vec temp;
	temp = vertexList_1[0];
	temp = vertexList_1[1];
	temp = vertexList_1[2];
#endif
	return vertexList_1;
}

/// [BasicMeshModel] Selection Functions -----------------------------------------------------------
void BasicMeshModel::select()
{
	if (selected)
		return;
	selected = true;
	refreshColor();
	Q_EMIT signal_modelSelect();
}

void BasicMeshModel::deselect()
{
	if (!selected)
		return;
	selected = false;
	refreshColor();
	Q_EMIT signal_modelDeselect();
}

bool BasicMeshModel::isSelected()
{
	return selected;
}

bool BasicMeshModel::isSameID(int id_)
{
	return (id == id_) ? (true) : (false);
}

bool BasicMeshModel::isDisabled()
{
	return disabled;
}

void BasicMeshModel::checkModelRange()
{
	setDisable(!MeshModelCalculator::checkModelRange(this));
}
void BasicMeshModel::setDisable(bool flag_)
{
	disabled = flag_;
}

/// [BasicMeshModel] Manipulation Functions --------------------------------------------------------
void BasicMeshModel::manipulated()
{
	calcAABB();
	if (box.z0 != 0)
	{
		translate(0, 0, -box.z0);
		calcAABB();
	}

	if (layerColorPlane)
	{
		layerColorPlane->setPosition(box.getCenter());
		layerColorPlane->setScale(box.getMaximum() - box.getMinimum());
	}
	checkModelRange();
	refreshColor();
	selectionHandle->refreshHandle(box);
}

void BasicMeshModel::adjustPosition()
{
	AABB beforeAABB = box;
	calcAABB();
	AABB afterAABB = box;
	qglviewer::Vec tempVec = (beforeAABB.getFloorCenter() - afterAABB.getFloorCenter());
	translate(tempVec[0], tempVec[1], 0);
	manipulated();
}

void BasicMeshModel::translate(float dx_, float dy_, float dz_)
{
	scaledFrame.moveFrame(dx_, dy_, dz_);
	selectionHandle->moveFrame(dx_, dy_, dz_);
	if (layerColorPlane)
		layerColorPlane->moveFrame(dx_, dy_);
}

void BasicMeshModel::resetZTranslate()
{
	qglviewer::Vec pos = scaledFrame.frame.position();
	pos.z = 0;
	scaledFrame.frame.setPosition(pos);
}

void BasicMeshModel::rotateAroundAPoint(float x_rad, float y_rad, float z_rad)
{
	if (x_rad == 0 && y_rad == 0 && z_rad == 0)
		return;
	qglviewer::Vec center = box.getCenter();

	if (x_rad != 0)
		rotateAroundAPoint(1, 0, 0, center[0], center[1], center[2], x_rad);

	if (y_rad != 0)
		rotateAroundAPoint(0, 1, 0, center[0], center[1], center[2], y_rad);

	if (z_rad != 0)
		rotateAroundAPoint(0, 0, 1, center[0], center[1], center[2], z_rad);

	adjustPosition();
	//Manipulated();
	//setSavedView(currentView);
}
void BasicMeshModel::rotateAroundAPoint(float vecX_, float vecY_, float vecZ_,
	float posX_, float posY_, float posZ_, float rotAngle_)
{
	qglviewer::Vec rotAxis(vecX_, vecY_, vecZ_);
	qglviewer::Vec point(posX_, posY_, posZ_);

	// Rotation to be oriented to the Destination Orientation
	qglviewer::Quaternion qObject(scaledFrame.frame.transformOf(rotAxis), rotAngle_);
	scaledFrame.frame.rotate(qObject);

	// Translation to the Position
	qglviewer::Quaternion qWorld(rotAxis, rotAngle_);
	scaledFrame.frame.setPosition(point +
		qWorld.rotate(scaledFrame.frame.position() - point));
}
void BasicMeshModel::rotateAroundPoint(qglviewer::Vec axisVec_, float rotAngle_, qglviewer::Vec position_)
{
	qglviewer::Quaternion rot_q(axisVec_, rotAngle_);

	scaledFrame.frame.rotateAroundPoint(rot_q, position_);
}

void BasicMeshModel::rotate(qglviewer::Vec axisVec_, float rotAngle_)
{
	scaledFrame.rotateFrame(axisVec_, rotAngle_);
}

void BasicMeshModel::rotate(float pitch_, float yaw_, float roll_)
{
	QVector3D axis;
	float angle;
	QQuaternion quaternion = QQuaternion::fromEulerAngles(pitch_, yaw_, roll_);
	quaternion.getAxisAndAngle(&axis, &angle);

	scaledFrame.rotateFrame(qglviewer::Vec(axis[0], axis[1], axis[2]), angle);
}

void BasicMeshModel::resetRotation()
{
	qglviewer::Quaternion q;
	scaledFrame.frame.getOrientation(q[0], q[1], q[2], q[3]);
	if (q[0] == 0 && q[1] == 0 && q[2] == 0 && q[3] == 1)
		return;
	scaledFrame.frame.setOrientation(qglviewer::Quaternion());
}

void BasicMeshModel::setScale(qglviewer::Vec scale_)
{
	outer->worldScaled(scale_, scaledFrame);
	scaledFactor = { scaledFactor[0] * scale_[0], scaledFactor[1] * scale_[1], scaledFactor[2] * scale_[2] };
	resetRenderer();
}

void BasicMeshModel::resetScale()
{
	delete outer;
	outer = new Mesh(*originalOuter);
	scaledFactor = { 1, 1, 1 };
	resetRenderer();
}
void BasicMeshModel::setColorPlaneHeight(float z_)
{
	if (layerColorPlane)
		layerColorPlane->setZ(z_);
}
int BasicMeshModel::compareModelSize(float refSize)
{
	qglviewer::Vec size = box.getMaximum() - box.getMinimum();
	float ratio = 999;

	for (int i = 0; i < 3; i++)
	{
		double temp = refSize / size[i];
		if (temp < ratio)
			ratio = temp;
	}

	if (ratio == 1)
		return 0;
	else if (ratio < 1)
		return 1;
	else
		return -1;
}

bool BasicMeshModel::maxSize()
{
	if (!MeshModelCalculator::maxSize(this))
		return false;
	manipulated();
	return true;
}

/// [BasicMeshModel] AABB Functions ----------------------------------------------------------------
void BasicMeshModel::calcAABB()
{
	box.clear();
	//qDebug() << "n_vertices : " << outer->n_vertices();
	for (int i = 0; i < outer->n_vertices(); i++)
	{
		Mesh::Point p = outer->point(outer->vertex_handle(i));
		qglviewer::Vec vLocal(p[0], p[1], p[2]);
		qglviewer::Vec vWorld = scaledFrame.toWorldCoords(vLocal);

		box.expand(vWorld);
	}
	totalBox.clear();
	qglviewer::Vec margin = Profile::getMargin();
	totalBox.clear();
	totalBox = box;
	totalBox.expand(box.getMaximum() + margin);
	totalBox.expand(box.getMinimum() - qglviewer::Vec(margin[0], margin[1], 0));

}

AABB BasicMeshModel::getAABB()
{
	return box;
}

AABB BasicMeshModel::getTotalBox()
{
	return totalBox;
}

/// [BasicMeshModel] Rendering Functions -----------------------------------------------------------
void BasicMeshModel::drawLayoutColoredIndexMap(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);
	matteRenderer->draw(outer, mvp);
}

void BasicMeshModel::drawNormalVectorMap(QMatrix4x4 proj_, QMatrix4x4 view_, bool nB_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	normalMapOuterRenderer->draw(outer, mvp);
}

void BasicMeshModel::drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	//glFuncs->glEnable(GL_CULL_FACE);
	//glFuncs->glCullFace(GL_BACK);

	// Draw Outer Surface
	phongRenderer->draw(outer, mvp);

	// Dimmer ...
	///**/glFuncs->glDepthMask(GL_FALSE);
	///**/
	///**/glFuncs->glDepthFunc(GL_ALWAYS);
	///**/phongDimmer->draw(outer, mvp);
	///**/glFuncs->glDepthFunc(GL_LESS);
	///**/
	///**/glFuncs->glDepthMask(GL_TRUE);

	glFuncs->glDisable(GL_CULL_FACE);
}

void BasicMeshModel::drawPolygon(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);
	glPolygonOffset(-1.0, -1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0);
	wireRenderer->draw(outer, mvp);
	glPolygonOffset(0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
//void BasicMeshModel::DrawAnalysisMode(QMatrix4x4 proj_, QMatrix4x4 view_)
//{
//	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
//	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);
//	MVPMatrices floorMvp(floorFrame.getModelMatrix(), view_, proj_);
//
//	glFuncs->glEnable(GL_DEPTH_TEST);
//
//	glFuncs->glEnable(GL_CULL_FACE);
//	glFuncs->glCullFace(GL_BACK);
//
//	//glFuncs->glEnable(GL_BLEND);
//	//glFuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	analysisMeshRenderer->setColor(0.5, 0.5, 0.2);
//
//	// Draw Outer Surface
//	analysisMeshRenderer->Draw(outer, mvp);
//
//	// Dimmer ...
//	///**/glfuncs->gldepthmask(gl_false);
//	///**/
//	///**/glfuncs->gldepthfunc(gl_always);
//	///**/phongdimmer->draw(outer, mvp);
//	///**/glfuncs->gldepthfunc(gl_less);
//	///**/
//	///**/glfuncs->gldepthmask(gl_true);
//
//	//glFuncs->glDisable(GL_BLEND);
//	glFuncs->glDisable(GL_CULL_FACE);
//	glFuncs->glDisable(GL_DEPTH_TEST);
//
//}

void BasicMeshModel::drawPreviewMode(QMatrix4x4 proj_, QMatrix4x4 view_, float zHeightBase_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	QMatrix4x4 mat = scaledFrame.getModelMatrix();
	mat.translate(0, 0, zHeightBase_);
	MVPMatrices mvp(mat, view_, proj_);
	//MVPMatrices floorMvp(floorFrame.getModelMatrix(), view_, proj_);

	//glFuncs->glEnable(GL_DEPTH_TEST);

	// Draw Outer Surface
	glFuncs->glEnable(GL_CULL_FACE);
	glFuncs->glCullFace(GL_BACK);
	phongRenderer->draw(outer, mvp);
	glFuncs->glDisable(GL_CULL_FACE);
	//glFuncs->glDisable(GL_DEPTH_TEST);	
}

void BasicMeshModel::drawLayerColorMode(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);
	//for test
	setLayerColor();
	glFuncs->glEnable(GL_CULL_FACE);
	glFuncs->glCullFace(GL_BACK);
	layerColorRenderer->draw(outer, mvp);
	glFuncs->glDisable(GL_CULL_FACE);
}

void BasicMeshModel::drawLayerColorPlane(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	layerColorPlane->drawOnScreenCanvas(proj_, view_);
}

void BasicMeshModel::drawSelectionHandle(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	if (isSelected())
		selectionHandle->drawLayoutEditMode(proj_, view_);
}

void BasicMeshModel::drawSelectionHandleForPicking(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	if (isSelected())
		selectionHandle->drawForColorPicking(proj_, view_);
}


/// [BasicMeshModel] Color Setting Functions -------------------------------------------------------
void BasicMeshModel::toggleTransparency()
{
	if (isTransparent)
	{
		alphaShader = 1.0;
		alphaDimmer = 0.0;
		isTransparent = false;
	}
	else
	{
		alphaShader = 0.75;
		alphaDimmer = 0.25;
		isTransparent = true;
	}

	refreshColor();
}

void BasicMeshModel::refreshColor()
{
	if (isDisabled())
	{
		setModelColorDisable();
		/*if (isSelected())
			SetColorDarkRed();
		else
			SetColorRed();*/
	}
	else
	{
		if (isSelected())
			setModelColorDark();
		else
			setModelColor();
	}
}

void BasicMeshModel::setModelColor()
{
	phongRenderer->setColor(modelColor, alphaShader);
	//phongDimmer->setColor(modelColor, alphaDimmer);
}
void BasicMeshModel::setModelColorDark(int partIndex_)
{
	if (isDisabled())
	{
		setModelColorDisable();
		return;
	}
	int adjustValue = 40;
	QColor temp = QColor((modelColor.red() - adjustValue < 0 ? 0 : modelColor.red() - adjustValue)
		, (modelColor.green() - adjustValue < 0 ? 0 : modelColor.green() - adjustValue)
		, (modelColor.blue() - adjustValue < 0 ? 0 : modelColor.blue() - adjustValue));
	phongRenderer->setColor(temp, alphaShader);
	//phongDimmer->setColor(temp, alphaDimmer);
}
void BasicMeshModel::setModelColorDisable()
{
	phongRenderer->setColor(0.7, 0.2, 0.2, alphaShader);
	//phongDimmer->setColor(0.7, 0.2, 0.2, alphaDimmer);
}

void BasicMeshModel::setActiveColor()
{
	modelColor = CartridgeInfo::getCartColor(cartridgeIndex);
	refreshColor();
}

void BasicMeshModel::setColor(QColor color_)
{
	phongRenderer->setColor(color_);
	layerColorRenderer->setColor(color_);
	modelColor = color_;
	//qDebug() << modelColor.redF() << " : " << modelColor.greenF() << " : " << modelColor.blueF();
	//SetColorGreen();
}

void BasicMeshModel::setLayerColor()
{
	std::vector<QVector3D> colors;
	QColor tempColor = CartridgeInfo::getCartColor(0);
	colors.push_back(QVector3D(tempColor.redF(), tempColor.greenF(), tempColor.blueF()));
	tempColor = CartridgeInfo::getCartColor(1);
	colors.push_back(QVector3D(tempColor.redF(), tempColor.greenF(), tempColor.blueF()));
	layerColorRenderer->setLayerColors(colors);

	layerColorRenderer->setLayerHeights(CartridgeInfo::getLayerColorList());
}

void BasicMeshModel::setFileName(QString filename_)
{
	fileinfo = QFileInfo(filename_);
}

QString BasicMeshModel::getFileName(bool usingFullFilePath_)
{
	QString name;
	if (usingFullFilePath_)
		name = fileinfo.absoluteFilePath();
	else
		name = fileinfo.fileName();
	return name;
}

QString BasicMeshModel::getOnlyFileName()
{
	QString name = fileinfo.completeBaseName();
	return name;
}

QFileInfo BasicMeshModel::getFileInfo()
{
	return fileinfo;
}

void BasicMeshModel::setCartridgeIndex(int cartridgeIndex_, int partIndex_)
{
	cartridgeIndex = cartridgeIndex_;
	setActiveColor();
}
std::vector<int> BasicMeshModel::getCartridgeIndexes()
{
	return std::vector<int> { cartridgeIndex };
}

void BasicMeshModel::layFlat()
{
	if (this->originalOuter->vertices_empty())
		return;

	ConvexHullStatic convex_hull;

	convex_hull.clear();
	convex_hull.setMesh(this->originalOuter);

	qglviewer::Quaternion orientation = this->getFrame().frame.orientation();
	qglviewer::Vec newZ = orientation.inverseRotate(qglviewer::Vec(0, 0, 1));

	float vec[3], rotationMatrix[4];
	vec[0] = newZ.x;
	vec[1] = newZ.y;
	vec[2] = newZ.z;

	//convex_hull running..//
	convex_hull.run(rotationMatrix, vec);

	double rad = rotationMatrix[0] * M_PI / 180.0;

	//qDebug() << "rotation matrix _ 0 : " << rotationMatrix[0];
	//qDebug() << "rotation matrix _ 1 : " << rotationMatrix[1];
	//qDebug() << "rotation matrix _ 2 : " << rotationMatrix[2];
	//qDebug() << "rotation matrix _ 3 : " << rotationMatrix[3];
	//qDebug() << "rad : " << rad;
	//qDebug() << "vec : " << vec[0] << vec[1] << vec[2];

	rotateAroundPoint(qglviewer::Vec(rotationMatrix[1], rotationMatrix[2], rotationMatrix[3]), rad, box.getCenter());

	manipulated();
}

// JoinedMeshModel Class ===========================================================================
JoinedMeshModel::JoinedMeshModel()
	: isDisjoined(false)
	, worldOuter(nullptr)
	, selected(false)
{
	init();
}

JoinedMeshModel::JoinedMeshModel(std::vector<IMeshModel*> parts_)
	: isDisjoined(false)
	, worldOuter(nullptr)
	, selected(false)
	, parts(parts_)
{
	init();
	for (auto part : parts_)
	{
		part->resetScale();
		part->resetRotation();
		part->resetZTranslate();
	}
	calcAABB();
}

JoinedMeshModel::JoinedMeshModel(JoinedMeshModel &model_)
	: isDisjoined(false)
	, worldOuter(nullptr)
	, selected(false)
	, scaledFrame(model_.scaledFrame)
{
	for (int i = 0; i < model_.parts.size(); ++i)
		parts.push_back(model_.parts[i]->clone());

	/*if (model_.ldni != nullptr)
		ldni = new LDNI(*model_.ldni);*/
	init();
	calcAABB();

}
void JoinedMeshModel::init()
{
	selectionHandle = new SelectionHandle();
	layerColorPlane = new LayerColorPlane();
	connect(&scaledFrame.frame, SIGNAL(modified()), this, SIGNAL(signal_frameModified()));
}

JoinedMeshModel::~JoinedMeshModel()
{
	if (!isDisjoined)
		for (int i = 0; i < parts.size(); i++)
			delete parts[i];
	if (worldOuter)
		delete worldOuter;
	delete selectionHandle;
	if (layerColorPlane)
		delete layerColorPlane;
}

/// [JoinedMeshModel] Mesh Processing Functions ----------------------------------------------------
Mesh * JoinedMeshModel::getMesh()
{
	if (worldOuter)
		delete worldOuter;
	worldOuter = new Mesh();
	//Mesh *joinedWorldMesh = new Mesh();
	for (int i = 0; i < parts.size(); i++)
	{
		worldOuter->append(*parts[i]->getMesh());
	}
	return worldOuter;
}
Mesh * JoinedMeshModel::getOuter()
{
	return nullptr;
}

std::vector<IMeshModel*> JoinedMeshModel::disJoin()
{
	isDisjoined = true;

	return parts;
}

std::vector<qglviewer::Vec> JoinedMeshModel::findBottomTriMesh(qglviewer::Vec _worldPos, qglviewer::Vec _onNormal)		// 이거 아냐. hollow 일 때와 똑같이 만들어야 해.
{
	std::vector<qglviewer::Vec> vertexList_1;
	// Back up Original Meshes
/*	Mesh* worldOuter = ToWorldMesh()(outer, scaledFrame);
	qglviewer::Vec* b_v;
	qglviewer::Vec* local_b_v;

	BottomSurfaceManager bm;
	qglviewer::Vec worldNormal = scaledFrame.ToWorldVector(_onNormal);

	try {
		//Mesh::Point *temp = bm.GenerateBottomMesh(worldOuter, _worldPos, worldNormal);
		qglviewer::Vec *temp = bm.GenerateBottomMesh(worldOuter, _worldPos, worldNormal);
		b_v = temp;
	}
	catch (QString &e) {
		QMessageBox msgBox;
		msgBox.setText(e);
		msgBox.setInformativeText("Choose another point.");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.exec();
	}
	local_b_v[0] = scaledFrame.ToLocalCoords(b_v[0]);
	local_b_v[1] = scaledFrame.ToLocalCoords(b_v[1]);
	local_b_v[2] = scaledFrame.ToLocalCoords(b_v[2]);
	*/
	//qglviewer::Vec b_v[3] = ;
	return vertexList_1;
}

/// [JoinedMeshModel] Selection Functions ----------------------------------------------------------
void JoinedMeshModel::select()
{
	/*for (auto it : parts)
		it->Select();*/
	if (selected)
		return;
	selected = true;
	refreshColor();
	Q_EMIT signal_modelSelect();
}

void JoinedMeshModel::deselect()
{
	/*for (auto it : parts)
		it->Deselect();*/
	if (!selected)
		return;
	selected = false;
	refreshColor();
	Q_EMIT signal_modelDeselect();
}

bool JoinedMeshModel::isSelected()
{
	/*for (auto it : parts)
		if (it->isSelected())
			return true;

	return false;*/
	return selected;
}

bool JoinedMeshModel::isSameID(int id_)
{
	for (auto it : parts)
		if (it->isSameID(id_))
			return true;

	return false;
}

bool JoinedMeshModel::isDisabled()
{
	for (auto it : parts)
		if (it->isDisabled())
			return true;

	return false;
}

void JoinedMeshModel::checkModelRange()
{
	setDisable(!MeshModelCalculator::checkModelRange(this));
}
void JoinedMeshModel::setDisable(bool flag_)
{
	for (auto it : parts)
		it->setDisable(flag_);
}

/// [JoinedMeshModel] Manipulation Functions -------------------------------------------------------
void JoinedMeshModel::manipulated()
{
	calcAABB();

	if (box.z0 != 0)
	{
		translate(0, 0, -box.z0);
		calcAABB();
	}

	if (layerColorPlane)
	{
		layerColorPlane->setPosition(box.getCenter());
		layerColorPlane->setScale(box.getMaximum() - box.getMinimum());
	}
	checkModelRange();
	refreshColor();
	selectionHandle->refreshHandle(box);
}

void JoinedMeshModel::adjustPosition()
{
	AABB beforeAABB = box;
	calcAABB();
	AABB afterAABB = box;
	qglviewer::Vec tempVec = (beforeAABB.getFloorCenter() - afterAABB.getFloorCenter());
	translate(tempVec[0], tempVec[1], 0);
	manipulated();
}

void JoinedMeshModel::translate(float dx_, float dy_, float dz_)
{
	scaledFrame.moveFrame(dx_, dy_, dz_);
	for (auto it : parts)
		it->translate(dx_, dy_, dz_);
	selectionHandle->moveFrame(dx_, dy_, dz_);
	if (layerColorPlane)
		layerColorPlane->moveFrame(dx_, dy_);
}

void JoinedMeshModel::rotateAroundAPoint(float x_rad, float y_rad, float z_rad)
{
	qglviewer::Vec center = box.getCenter();
	if (x_rad != 0)
		rotateAroundAPoint(1, 0, 0, center[0], center[1], center[2], x_rad);
	if (y_rad != 0)
		rotateAroundAPoint(0, 1, 0, center[0], center[1], center[2], y_rad);
	if (z_rad != 0)
		rotateAroundAPoint(0, 0, 1, center[0], center[1], center[2], z_rad);

	adjustPosition();
}

void JoinedMeshModel::rotateAroundAPoint(float vecX_, float vecY_, float vecZ_,
	float posX_, float posY_, float posZ_, float rotAngle_)
{
	for (auto it : parts)
		it->rotateAroundAPoint(vecX_, vecY_, vecZ_, posX_, posY_, posZ_, rotAngle_);
}

void JoinedMeshModel::rotateAroundPoint(qglviewer::Vec axisVec_, float rotAngle_, qglviewer::Vec position_)
{
	for (auto it : parts)
		it->rotateAroundPoint(axisVec_, rotAngle_, position_);
}

void JoinedMeshModel::rotate(qglviewer::Vec axisVec_, float rotAngle_)
{
	for (auto it : parts)
		it->rotate(axisVec_, rotAngle_);
}

void JoinedMeshModel::rotate(float pitch_, float yaw_, float roll_)
{
	for (auto it : parts)
		it->rotate(pitch_, yaw_, roll_);
}

void JoinedMeshModel::resetRotation()
{
	for (auto it : parts)
		it->resetRotation();
}

void JoinedMeshModel::setScale(qglviewer::Vec scale_)
{
	for (auto it : parts)
		it->setScale(scale_);
}

void JoinedMeshModel::resetScale()
{
	for (auto it : parts)
		it->resetScale();
}
void JoinedMeshModel::setColorPlaneHeight(float z_)
{
	if (layerColorPlane)
		layerColorPlane->setZ(z_);
}
int JoinedMeshModel::compareModelSize(float refSize)
{
	qglviewer::Vec size = box.getMaximum() - box.getMinimum();
	float ratio = 999;

	for (int i = 0; i < 3; i++)
	{
		double temp = refSize / size[i];
		if (temp < ratio)
			ratio = temp;
	}

	if (ratio == 1)
		return 0;
	else if (ratio < 1)
		return 1;
	else
		return -1;
}

bool JoinedMeshModel::maxSize()
{
	if (!MeshModelCalculator::maxSize(this))
		return false;
	manipulated();
	return true;
}

/// [JoinedMeshModel] AABB Functions ---------------------------------------------------------------
void JoinedMeshModel::calcAABB()
{
	box.clear();
	totalBox.clear();
	for (int i = 0; i < parts.size(); i++)
	{
		parts[i]->calcAABB();
		qglviewer::Vec pMax = parts[i]->getAABB().getMaximum();
		qglviewer::Vec pMin = parts[i]->getAABB().getMinimum();
		qglviewer::Vec pTotalMax = parts[i]->getTotalBox().getMaximum();
		qglviewer::Vec pTotalMin = parts[i]->getTotalBox().getMinimum();

		box.expand(pMax);
		box.expand(pMin);
		totalBox.expand(pTotalMax);
		totalBox.expand(pTotalMin);
	}
}

AABB JoinedMeshModel::getAABB()
{
	return box;
}

AABB JoinedMeshModel::getTotalBox()
{
	return totalBox;
}

/// [JoinedMeshModel] Rendering Functions ----------------------------------------------------------
void JoinedMeshModel::drawLayoutColoredIndexMap(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	for (int i = 0; i < parts.size(); i++)
		parts[i]->drawLayoutColoredIndexMap(proj_, view_);
}

void JoinedMeshModel::drawNormalVectorMap(QMatrix4x4 proj_, QMatrix4x4 view_, bool nB_)
{
	for (int i = 0; i < parts.size(); i++)
		parts[i]->drawNormalVectorMap(proj_, view_, nB_);
}

void JoinedMeshModel::drawLayoutEditMode(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	for (int i = 0; i < parts.size(); i++)
		parts[i]->drawLayoutEditMode(proj_, view_);
}

void JoinedMeshModel::drawPolygon(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	for (int i = 0; i < parts.size(); i++)
		parts[i]->drawPolygon(proj_, view_);
}

//void JoinedMeshModel::DrawAnalysisMode(QMatrix4x4 proj_, QMatrix4x4 view_)
//{
//	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
//	MVPMatrices mvp(floorFrame.getModelMatrix(), view_, proj_);
//
//	glFuncs->glEnable(GL_DEPTH_TEST);
//
//	glFuncs->glEnable(GL_CULL_FACE);
//	glFuncs->glCullFace(GL_BACK);
//
//	//glFuncs->glEnable(GL_BLEND);
//	//glFuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	glFuncs->glDisable(GL_BLEND);
//	glFuncs->glDisable(GL_CULL_FACE);
//	glFuncs->glDisable(GL_DEPTH_TEST);
//
//	for (int i = 0; i < parts.size(); i++)
//		parts[i]->DrawAnalysisMode(proj_, view_);
//}

void JoinedMeshModel::drawPreviewMode(QMatrix4x4 proj_, QMatrix4x4 view_, float zHeightBase_)
{
	for (auto it : parts)
		it->drawPreviewMode(proj_, view_, zHeightBase_);
}

void JoinedMeshModel::drawLayerColorMode(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	for (auto it : parts)
		it->drawLayerColorMode(proj_, view_);
}

void JoinedMeshModel::drawLayerColorPlane(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	layerColorPlane->drawOnScreenCanvas(proj_, view_);
}

void JoinedMeshModel::drawSelectionHandle(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	if (isSelected())
		selectionHandle->drawLayoutEditMode(proj_, view_);
}

void JoinedMeshModel::drawSelectionHandleForPicking(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	if (isSelected())
		selectionHandle->drawForColorPicking(proj_, view_);
}

/// [JoinedMeshModel] Color Setting Functions ------------------------------------------------------
void JoinedMeshModel::toggleTransparency()
{
	for (int i = 0; i < parts.size(); i++)
		parts[i]->toggleTransparency();
}

void JoinedMeshModel::refreshColor()
{
	if (parts.size() == 0)
		return;

	if (isDisabled())
		setModelColorDisable();
	else
	{
		if (isSelected())
			setModelColorDark();
		else
			setModelColor();
	}
}

void JoinedMeshModel::setModelColor()
{
	for (auto it : parts)
		it->setModelColor();
}

void JoinedMeshModel::setModelColorDark(int partIndex_)
{
	if (partIndex_ < 0)
	{
		for (auto it : parts)
			it->setModelColorDark();
		return;
	}

	if (partIndex_ >= parts.size())
		return;
	parts[partIndex_]->setModelColorDark();
}

void JoinedMeshModel::setModelColorDisable()
{
	for (auto it : parts)
		it->setModelColorDisable();
}

void JoinedMeshModel::setActiveColor()
{
	for (auto it : parts)
		it->setActiveColor();
}

void JoinedMeshModel::setColor(QColor color_)
{
	for (auto it : parts)
		it->setColor(color_);
}

void JoinedMeshModel::setLayerColor()
{
	for (auto it : parts)
		it->setLayerColor();
}

QString JoinedMeshModel::getFileName(bool usingFullFilePath_)
{
	if (parts.size() == 0)
		return "";

	QString rtn = parts.front()->getFileName(false);
	for (int i = 1; i < parts.size(); i++)
		rtn = rtn + " + " + parts[i]->getFileName(false);

	return rtn;
}

QFileInfo JoinedMeshModel::getFileInfo()
{
	if (parts.size() == 0)
		return QFileInfo();
	return parts.back()->getFileInfo();
}

QString JoinedMeshModel::getOnlyFileName()
{
	QString name = parts.back()->getOnlyFileName();
	return name;
}

void JoinedMeshModel::setCartridgeIndex(int cartridgeIndex_, int partIndex_)
{
	if (partIndex_ < 0)
	{
		for (auto it : parts)
			it->setCartridgeIndex(cartridgeIndex_);
		return;
	}

	if (partIndex_ >= parts.size())
		return;
	parts[partIndex_]->setCartridgeIndex(cartridgeIndex_);
}
std::vector<int> JoinedMeshModel::getCartridgeIndexes()
{
	if (parts.size() == 0)
		return std::vector<int>{0};

	std::vector<int> rtn;
	for (int i = 0; i < parts.size(); i++)
	{
		rtn.push_back(parts[i]->getCartridgeIndexes()[0]);
	}
	return rtn;
}

void JoinedMeshModel::layFlat()
{
	if (getMesh()->vertices_empty())
		return;

	ConvexHullStatic convex_hull;

	convex_hull.clear();
	convex_hull.setMesh(getMesh());

	qglviewer::Quaternion orientation = this->getFrame().frame.orientation();
	qglviewer::Vec newZ = orientation.inverseRotate(qglviewer::Vec(0, 0, 1));

	float vec[3], rotationMatrix[4];
	vec[0] = newZ.x;
	vec[1] = newZ.y;
	vec[2] = newZ.z;

	//convex_hull running..//
	convex_hull.run(rotationMatrix, vec);

	double rad = rotationMatrix[0] * M_PI / 180.0;
	//qDebug() << "rotation matrix _ 0 : " << rotationMatrix[0];
	//qDebug() << "rotation matrix _ 1 : " << rotationMatrix[1];
	//qDebug() << "rotation matrix _ 2 : " << rotationMatrix[2];
	//qDebug() << "rotation matrix _ 3 : " << rotationMatrix[3];
	//qDebug() << "rad : " << rad;
	//qDebug() << "vec : " << vec[0] << vec[1] << vec[2];

	rotateAroundPoint(qglviewer::Vec(rotationMatrix[1], rotationMatrix[2], rotationMatrix[3]), rad, box.getCenter());

	manipulated();
}

AABB AABBGetter::operator()(const std::vector<IMeshModel*> &models_)
{
	AABB temp;
	for (int i = 0; i < models_.size(); i++) {
		AABB aabb = models_[i]->getAABB();
		qglviewer::Vec pmax = aabb.getMaximum();
		qglviewer::Vec pmin = aabb.getMinimum();

		temp.expand(pmax);
		temp.expand(pmin);
	}

	return temp;
}

AABB TotalBoxGetter::operator()(const std::vector<IMeshModel*> &models_)
{
	AABB temp;
	for (int i = 0; i < models_.size(); i++) {
		AABB aabb = models_[i]->getTotalBox();
		qglviewer::Vec pmax = aabb.getMaximum();
		qglviewer::Vec pmin = aabb.getMinimum();

		temp.expand(pmax);
		temp.expand(pmin);
	}

	return temp;
}