#include "stdafx.h"
#include "SelectCartridgeDialog.h"
#include "CartridgeInfo.h"

SelectCartridgeDialog::SelectCartridgeDialog(QWidget* _parent)
	: QDialog(_parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_DeleteOnClose, true);
}

SelectCartridgeDialog::~SelectCartridgeDialog()
{
}
void SelectCartridgeDialog::init(ViewerModule* _viewerModule)
{
	viewerModule = _viewerModule;
	modelContainer = _viewerModule->modelContainer;
	setConnection();
	setContents();
}
void SelectCartridgeDialog::setConnection()
{
	connect(viewerModule, SIGNAL(signal_mouseOnModel(int, int)), this, SLOT(mouseIsOnVolumeMaterial(int, int)));
	connect(viewerModule, SIGNAL(signal_mouseOutModel(int, int)), this, SLOT(mouseIsOutVolumeMaterial(int, int)));
	connect(modelContainer, SIGNAL(signal_modelSelect()), this, SLOT(volumeSelected()));
	connect(modelContainer, SIGNAL(signal_modelDeselect()), this, SLOT(volumeSelected()));
	connect(modelContainer, SIGNAL(signal_modelDeleted()), this, SLOT(refreshContents()));
	connect(modelContainer, SIGNAL(signal_modelAdded()), this, SLOT(refreshContents()));
	connect(ui.pushButton_cartridgeInfo, SIGNAL(clicked(bool)), this, SLOT(pushButton_cartridgeInfoClicked()));
}
void SelectCartridgeDialog::setContents()
{
	refreshContents();
	volumeSelected();
}
void SelectCartridgeDialog::refreshContents()
{
	removeAllWidgets();
	if (!modelContainer->hasAnyModel()) addNoModelLabel();

	ui.body->setStyleSheet("");
	for (int i = 0; i < modelContainer->models.size(); i++)
	{
		addObjectWidget(i);
		std::vector<IMeshModel*> models = modelContainer->models[i]->getModels();
		for (int j = 0; j < models.size(); j++)
		{
			addMeshWidget(i, j);
			int cartridge_idx = models[j]->getCartridgeIndexes().front();
			QColor color = color = CartridgeInfo::getCartColor(cartridge_idx);
			QComboBox *selectedCart = findChild<QComboBox *>("selectCart_" + QString::number(i) + "_" + QString::number(j));
			selectedCart->setCurrentIndex(cartridge_idx);

			selectedCart->setEnabled(!Profile::machineProfile.machine_expanded_print_mode.value);

			connect(selectedCart, SIGNAL(currentIndexChanged(int)), this, SLOT(cartridgeChanged(int)));

			setSelectedFrameStyle(i, j, color);
		}
	}
}

void SelectCartridgeDialog::volumeSelected()
{
	if (!modelContainer->hasAnySelectedModel())
	{
		ui.body->setStyleSheet("");
		return;
	}
	QString styleSheet = "";
	for (uint i = 0; i < modelContainer->models.size(); ++i)
	{
		if (modelContainer->models[i]->isSelected())
		{
			QFrame *meshFrame = findChild<QFrame *>("objectFrame_" + QString::number(i));
			if (meshFrame == NULL)
				refreshContents();

			if (styleSheet == "") styleSheet = "#objectFrame_" + QString::number(i);
			else styleSheet += ", #objectFrame_" + QString::number(i);
		}
	}

	ui.body->setStyleSheet(styleSheet + "{ background-color: rgba(255, 255, 0, 80); }");
	//ui.body->setStyleSheet("#objectFrame" + QString::number(idx) + "{ background-color: rgba(255, 255, 0, 80); }");
	//qDebug() << "Volume Selected";
}
void SelectCartridgeDialog::mouseIsOnVolumeMaterial(int objectIdx, int meshIdx)
{
	QFrame *meshFrame = findChild<QFrame *>("meshFrame_" + QString::number(objectIdx) + "_" + QString::number(meshIdx));
	if (meshFrame == NULL) return;
	QString styleSheet = "#meshFrame_" + QString::number(objectIdx) + "_" + QString::number(meshIdx) + "{ border-width: 2px; border-color: rgb(255, 0, 0); border-style: solid; }";
	meshFrame->setStyleSheet(styleSheet);
	//qDebug() << "mouseIsOnVolumeMaterial : " << objectIdx << meshIdx;
}
void SelectCartridgeDialog::mouseIsOutVolumeMaterial(int objectIdx, int meshIdx)
{
	QFrame *meshFrame = findChild<QFrame *>("meshFrame_" + QString::number(objectIdx) + "_" + QString::number(meshIdx));
	if (meshFrame == NULL) return;
	QString styleSheet = "#meshFrame_" + QString::number(objectIdx) + "_" + QString::number(meshIdx) + "{ border-width: 2px; border-color: rgba(255, 0, 0, 0); border-style: solid; }";
	meshFrame->setStyleSheet(styleSheet);
	//qDebug() << "mouseIsOutVolumeMaterial" << objectIdx << meshIdx;
}
void SelectCartridgeDialog::mouseEnter(int objectIdx, int meshIdx)
{
	modelContainer->changeModelColorDark(objectIdx, meshIdx);
	viewerModule->updateGL();
	//qDebug() << "mouseEnter" << objectIdx << meshIdx;
}
void SelectCartridgeDialog::mouseLeave(int objectIdx, int meshIdx)
{
	modelContainer->refreshModelColor(objectIdx);
	viewerModule->updateGL();
	//qDebug() << "mouseLeave" << objectIdx << meshIdx;
}
void SelectCartridgeDialog::frameClicked(int objectIdx, int meshIdx)
{
	//qDebug() << "frameClicked";
	if (objectIdx < 0)
		return;
	std::vector<IMeshModel*> models = modelContainer->models;
	if (objectIdx >= models.size())
		return;
	if (models[objectIdx]->isSelected())
		return;

	modelContainer->deselectAll();
	models[objectIdx]->select();
	viewerModule->updateGL();
}
void SelectCartridgeDialog::frameShiftClicked(int objectIdx, int meshIdx)
{
	//qDebug() << "frameShiftClicked";
	if (objectIdx < 0)
		return;
	std::vector<IMeshModel*> models = modelContainer->models;
	if (objectIdx >= models.size())
		return;

	if (models[objectIdx]->isSelected())
		models[objectIdx]->deselect();
	viewerModule->updateGL();
}
void SelectCartridgeDialog::frameControlClicked(int objectIdx, int meshIdx)
{
	//qDebug() << "frameControlClicked";
	if (objectIdx < 0)
		return;
	std::vector<IMeshModel*> models = modelContainer->models;
	if (objectIdx >= models.size())
		return;

	if (models[objectIdx]->isSelected())
		models[objectIdx]->deselect();
	else
		models[objectIdx]->select();
	viewerModule->updateGL();
}


void SelectCartridgeDialog::removeAllWidgets()
{
	qDeleteAll(ui.body->children());

	//body 하위 object를 삭제할때 vertical layout이 같이 삭제되므로 추가해줌.
	QVBoxLayout *verticalBodyLayout;
	verticalBodyLayout = new QVBoxLayout(ui.body);
	verticalBodyLayout->setSpacing(0);
	verticalBodyLayout->setContentsMargins(11, 11, 11, 11);
	verticalBodyLayout->setObjectName("verticalBodyLayout");
	verticalBodyLayout->setContentsMargins(0, 0, 0, 0);
}

void SelectCartridgeDialog::addNoModelLabel()
{
	QLabel *object_name;
	//새로 만들어진 Vertical Layout은 찾아서 사용
	QVBoxLayout *verticalBodyLayout = findChild<QVBoxLayout *>("verticalBodyLayout");

	object_name = new QLabel(ui.body);
	object_name->setObjectName("objectName_0");
	object_name->setMinimumSize(QSize(0, 20));
	QFont font1(QFont("Malgun Gothic", 9, QFont::Bold));
	object_name->setFont(font1);
	object_name->setMargin(3);
	verticalBodyLayout->addWidget(object_name);
	object_name->setText(CustomTranslate::tr("No loaded model."));
}

void SelectCartridgeDialog::addObjectWidget(int idx)
{
	QFrame *objectFrame;
	QVBoxLayout *verticalObject;
	QLabel *object_name;
	//새로 만들어진 Vertical Layout은 찾아서 사용
	QVBoxLayout *verticalBodyLayout = findChild<QVBoxLayout *>("verticalBodyLayout");

	if (idx != 0)
	{
		QFrame *object_line;
		object_line = new QFrame(ui.body);
		object_line->setObjectName("objectLine_" + QString::number(idx));
		object_line->setStyleSheet(QStringLiteral("color: rgba(180,180,180);"));
		object_line->setFrameShadow(QFrame::Plain);
		object_line->setFrameShape(QFrame::HLine);
		//object_line->setFrameShadow(QFrame::Shadow::Sunken);
		verticalBodyLayout->addWidget(object_line);
	}
	objectFrame = new QFrame(ui.body);
	objectFrame->setObjectName("objectFrame_" + QString::number(idx));
	objectFrame->setFrameShape(QFrame::StyledPanel);
	objectFrame->setFrameShadow(QFrame::Raised);
	verticalObject = new QVBoxLayout(objectFrame);
	verticalObject->setSpacing(0);
	verticalObject->setContentsMargins(11, 11, 11, 11);
	verticalObject->setObjectName("verticalObject_" + QString::number(idx));
	verticalObject->setContentsMargins(0, 0, 0, 0);
	verticalBodyLayout->addWidget(objectFrame);

	object_name = new QLabel(objectFrame);
	object_name->setObjectName("objectName_" + QString::number(idx));
	object_name->setMinimumSize(QSize(0, 20));
	QFont font1(QFont("Malgun Gothic", 9, QFont::Bold));
	object_name->setFont(font1);
	object_name->setMargin(3);
	verticalObject->addWidget(object_name);
	object_name->setText(tr("Object") + QString::number(idx + 1));
}

void SelectCartridgeDialog::addMeshWidget(int objectIdx, int meshIdx)
{
	QFrame *mesh_frame;
	QHBoxLayout *horizontalMesh;
	QFrame *selectedInfo;
	//QVBoxLayout *verticalBodyLayout = findChild<QVBoxLayout *>("verticalBodyLayout");
	QLabel *mesh;
	QComboBox *selectCart;
	QFrame *objectFrame = findChild<QFrame *>("objectFrame_" + QString::number(objectIdx));
	QVBoxLayout *verticalObjectLayout = findChild<QVBoxLayout *>("verticalObject_" + QString::number(objectIdx));

	//mesh_frame = new QFrame(objectFrame);
	mesh_frame = new frameMouseEvent(objectFrame);
	mesh_frame->setObjectName("meshFrame_" + QString::number(objectIdx) + "_" + QString::number(meshIdx));
	mesh_frame->setFrameShape(QFrame::StyledPanel);
	mesh_frame->setFrameShadow(QFrame::Raised);
	QString styleSheet = "#meshFrame_" + QString::number(objectIdx) + "_" + QString::number(meshIdx) + "{ border-width: 2px; border-color: rgba(255, 0, 0, 0); border-style: solid; }";
	mesh_frame->setStyleSheet(styleSheet);
	connect(mesh_frame, SIGNAL(frameMouseEnter(int, int)), this, SLOT(mouseEnter(int, int)));
	connect(mesh_frame, SIGNAL(frameMouseLeave(int, int)), this, SLOT(mouseLeave(int, int)));

	connect(mesh_frame, SIGNAL(signal_frameClicked(int, int)), this, SLOT(frameClicked(int, int)));
	connect(mesh_frame, SIGNAL(signal_frameShiftClicked(int, int)), this, SLOT(frameShiftClicked(int, int)));
	connect(mesh_frame, SIGNAL(signal_frameControlClicked(int, int)), this, SLOT(frameControlClicked(int, int)));

	horizontalMesh = new QHBoxLayout(mesh_frame);
	horizontalMesh->setSpacing(6);
	horizontalMesh->setContentsMargins(11, 11, 11, 11);
	horizontalMesh->setObjectName("horizontalMesh_" + QString::number(objectIdx) + "_" + QString::number(meshIdx));
	horizontalMesh->setContentsMargins(-1, 0, -1, 0);
	selectedInfo = new QFrame(mesh_frame);
	selectedInfo->setObjectName("selectedInfo_" + QString::number(objectIdx) + "_" + QString::number(meshIdx));
	QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(selectedInfo->sizePolicy().hasHeightForWidth());
	selectedInfo->setSizePolicy(sizePolicy1);
	selectedInfo->setMinimumSize(QSize(12, 12));
	selectedInfo->setMaximumSize(QSize(12, 12));
	selectedInfo->setStyleSheet(QLatin1String("background-color: rgb(255, 255, 0);\n"
		//"image: url(:/rodinP/Resources/check.png);\n"
		"border - style: solid;"
		"border-radius: 6px;\n"
		"border-width: 1px;\n"
		"border-color: rgb(100,100,100);"));

	selectedInfo->setFrameShape(QFrame::Box);
	selectedInfo->setFrameShadow(QFrame::Plain);
	selectedInfo->setLineWidth(1);

	horizontalMesh->addWidget(selectedInfo);
	verticalObjectLayout->addWidget(mesh_frame);

	mesh = new QLabel(mesh_frame);
	mesh->setObjectName("mesh_" + QString::number(objectIdx) + "_" + QString::number(meshIdx));
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(mesh->sizePolicy().hasHeightForWidth());
	mesh->setSizePolicy(sizePolicy);
	QFont font2;
	font2.setFamily(QStringLiteral("Malgun Gothic"));
	font2.setPointSize(9);
	mesh->setFont(font2);

	horizontalMesh->addWidget(mesh);

	selectCart = new QComboBox(mesh_frame);
	selectCart->setObjectName("selectCart_" + QString::number(objectIdx) + "_" + QString::number(meshIdx));

	horizontalMesh->addWidget(selectCart);

	mesh->setText(tr("Mesh") + QString::number(meshIdx + 1));
	selectCart->setFont(font2);
	//selectCart->addItem("");

	for (int i = 0; i < CartridgeInfo::cartridges.size(); i++)
	{
		QString tempItem;
		tempItem.clear();
		tempItem.append(CustomTranslate::tr("Cartridge"));
		tempItem.append(QString(" (%1)").arg(i + 1));

		selectCart->addItem(tempItem);
	}
}

void SelectCartridgeDialog::setSelectedFrameStyle(int objectIdx, int meshIdx, QColor meshColor, bool selected)
{
	int red;
	int green;
	int blue;
	int lightness;

	if (meshColor != nullptr)
	{
		red = meshColor.red();
		green = meshColor.green();
		blue = meshColor.blue();
		lightness = meshColor.lightness();
	}
	else
	{
		red = 255;
		green = 255;
		blue = 0;
		lightness = 255;
	}
	QFrame *selectedFrame = findChild<QFrame *>("selectedInfo_" + QString::number(objectIdx) + "_" + QString::number(meshIdx));
	if (selectedFrame == NULL) return;
	QString styleSheet = "border-radius: 6px;\n";
	if (selected)
	{
		if (lightness <= 100) styleSheet += "image: url(:/rodinP/Resources/check_white.png);\n";
		else styleSheet += "image: url(:/rodinP/Resources/check.png);\n";
	}
	styleSheet += "background-color: rgb(" + QString::number(red) + "," + QString::number(green) + "," + QString::number(blue) + ");\n";

	selectedFrame->setStyleSheet(styleSheet);
}

void SelectCartridgeDialog::cartridgeChanged(int idx)
{
	QComboBox* selectedCart = dynamic_cast<QComboBox*>(sender());

	QStringList tempList = selectedCart->objectName().split("_");
	int objectIdx = tempList.at(1).toInt();
	int meshIdx = tempList.at(2).toInt();

	setSelectedFrameStyle(objectIdx, meshIdx, CartridgeInfo::getCartColor(idx));
	modelContainer->setCartridgeIndex(objectIdx, idx, meshIdx);
	//viewerModule->SetModelColor();
	viewerModule->updateGL();
}

void SelectCartridgeDialog::pushButton_cartridgeInfoClicked()
{
	emit signal_pushButton_cartridgeInfoClicked();
}

void SelectCartridgeDialog::changeCartridgeColor()
{
	auto models = modelContainer->models;
	for (int i = 0; i < models.size(); i++)
	{
		auto parts = models[i]->getModels();
		for (int j = 0; j < parts.size(); j++)
		{
			int cartIndex = parts[j]->getCartridgeIndexes().front();
			setSelectedFrameStyle(i, j, CartridgeInfo::getCartColor(cartIndex));
		}
	}
}




frameMouseEvent::frameMouseEvent(QWidget *parent) :
	QFrame(parent)
{

}

void frameMouseEvent::enterEvent(QEvent * e)
{
	QString objectName = this->objectName();
	QStringList tempList = objectName.split("_");
	m_objectIdx = tempList.at(1).toInt();
	m_meshIdx = tempList.at(2).toInt();
	emit frameMouseEnter(m_objectIdx, m_meshIdx);
}


void frameMouseEvent::leaveEvent(QEvent *e)
{
	emit frameMouseLeave(m_objectIdx, m_meshIdx);
}

void frameMouseEvent::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton)
		return;

	if (e->modifiers() == Qt::NoModifier) {
		emit signal_frameClicked(m_objectIdx, m_meshIdx);
	}
	else if (e->modifiers() == Qt::ShiftModifier) {
		emit signal_frameShiftClicked(m_objectIdx, m_meshIdx);
	}
	else if (e->modifiers() == Qt::ControlModifier) {
		emit signal_frameControlClicked(m_objectIdx, m_meshIdx);
	}
}