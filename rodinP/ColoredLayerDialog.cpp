#include "stdafx.h"
#include "ColoredLayerDialog.h"
#include "CartridgeInfo.h"
#include "MeshModel.h"
ColoredLayerDialog::ColoredLayerDialog(QWidget* _parent)
	: QDialog(_parent)
	, m_maxLayer(0)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_DeleteOnClose, true);
}


ColoredLayerDialog::~ColoredLayerDialog()
{
}

void ColoredLayerDialog::init(ViewerModule* _viewerModule)
{
	viewerModule = _viewerModule;
	modelContainer = _viewerModule->modelContainer;
	setConnection();
	setContents();
}

void ColoredLayerDialog::setConnection()
{
	qDebug() << "setConnection called!!";
	connect(this, SIGNAL(signal_updateGL()), viewerModule, SLOT(updateGL()));
	connect(modelContainer, SIGNAL(signal_modelDeleted()), this, SLOT(model_changed()));
	connect(modelContainer, SIGNAL(signal_modelAdded()), this, SLOT(model_changed()));
	connect(this, SIGNAL(signal_updateGL()), this, SLOT(checkVerticalScrollBarVisible()));

	connect(ui.verticalSlider_layer_height, SIGNAL(valueChanged(int)), this, SLOT(verticalSlider_layer_height_valueChanged(int)));
	//connect(ui.verticalSlider_layer_height, SIGNAL(valueChanged(int)), m_parent->ui.viewer, SLOT(SetLayer(int))); //to do sj
	connect(ui.spinBox_current_layer, SIGNAL(valueChanged(int)), this, SLOT(spinBox_current_layer_valueChanged(int)));
	connect(ui.pushButton_add, SIGNAL(clicked()), this, SLOT(pushButton_add_clicked()));
	connect(ui.pushButton_delete, SIGNAL(clicked()), this, SLOT(pushButton_delete_clicked()));
	connect(ui.pushButton_reset, SIGNAL(clicked()), this, SLOT(pushButton_reset_clicked()));
	connect(ui.pushButton_equalize, SIGNAL(clicked()), this, SLOT(pushButton_equalize_clicked()));
	connect(ui.tableWidget_layer_view, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableWidget_layer_view_itemClicked(QTableWidgetItem*)));
	connect(ui.tableWidget_layer_view, SIGNAL(cellChanged(int, int)), this, SLOT(tableWidget_layer_view_cellChanged(int, int)));
	connect(ui.spinBox_equalize_num, SIGNAL(enterPressed()), this, SLOT(spinBox_equalize_num_enterPressed()));
}

void ColoredLayerDialog::setContents()
{
	setInitValue();
	setLayerFromLayerList();
	emit signal_updateGL();
}

void ColoredLayerDialog::setInitValue()
{
	m_layerColorList = &CartridgeInfo::layerColorList;
	setMaxLayer();
	ui.verticalSlider_layer_height->setMinimum(0);
	ui.verticalSlider_layer_height->setValue(m_maxLayer / 2);
	ui.spinBox_current_layer->setMinimum(0);
	ui.spinBox_current_layer->setValue(m_maxLayer / 2);
	ui.spinBox_equalize_num->setMinimum(2);
	ui.spinBox_equalize_num->setValue(2);

	viewerModule->changeLayerIndexForLayerColor(m_maxLayer / 2);
}

void ColoredLayerDialog::setMaxLayer()
{
	AABB aabb = AABBGetter()(modelContainer->models);
	m_maxLayer = aabb.getMaximum().z / Profile::sliceProfileCommon.layer_height.value;
	ui.verticalSlider_layer_height->setMaximum(m_maxLayer);
	ui.spinBox_current_layer->setMaximum(m_maxLayer);
	int max_equalize_num = m_maxLayer / 2;
	if (max_equalize_num > 100)
		max_equalize_num = 100;
	ui.spinBox_equalize_num->setMaximum(max_equalize_num);
	ui.label_max_layer->setNum(m_maxLayer);
}

//데이타는 vector에 가지고 있다가 필요한 경우 화면에 뿌려줌.
//화면은 메모리에 계속 올려놓을 필요가 없으니 닫으면 delete하도록 함.
void ColoredLayerDialog::setLayerFromLayerList()
{
	ui.tableWidget_layer_view->clearContents();
	ui.tableWidget_layer_view->setRowCount(0);
	int rowSize = m_layerColorList->size();
	int layerNr;
	int cartridgeIdx;
	int cartCnt = CartridgeInfo::cartridges.size();
	for (int i = 0; i < rowSize; ++i)
	{
		layerNr = m_layerColorList->at(i).layerNr;
		if (layerNr < 0) layerNr = 0;
		else if (layerNr > m_maxLayer) layerNr = m_maxLayer;
		if (cartCnt <= 1)
		{
			if (layerNr < 1 || layerNr == m_maxLayer)
				m_layerColorList->at(i).cartridgeIndex = 0;
			else
				m_layerColorList->at(i).cartridgeIndex = -1;
		}
		//pause 제거용 로직 추가
		else
		{
			if (m_layerColorList->at(i).cartridgeIndex == -1)
				m_layerColorList->at(i).cartridgeIndex = 0;
		}
		//
		cartridgeIdx = m_layerColorList->at(i).cartridgeIndex;
		addLayer(layerNr, cartridgeIdx, false);
	}
}

void ColoredLayerDialog::updateSpinBox(int layer)
{
	//ui.label_current_layer->setNum(layer);
	if (layer != ui.spinBox_current_layer->value())	ui.spinBox_current_layer->setValue(layer);

	viewerModule->changeLayerIndexForLayerColor(layer);
}

void ColoredLayerDialog::updateSlider(int layer)
{
	if (layer != ui.verticalSlider_layer_height->value())
		ui.verticalSlider_layer_height->setValue(layer);
}

void ColoredLayerDialog::addLayer(int pLayerNr, int pCartIdx, bool pInsertFlag)
{
	int layerNr;
	int cartCnt = CartridgeInfo::cartridges.size();
	if (pLayerNr == -999)
		layerNr = ui.verticalSlider_layer_height->value();
	else
		layerNr = pLayerNr;

	if (cartCnt < pCartIdx) pCartIdx = 0;

	int rowCnt = ui.tableWidget_layer_view->rowCount();
	int targetRow = 0;//들어갈 위치 찾아주기
	int insertedLayer;
	for (int i = 0; i < rowCnt; i++)
	{
		insertedLayer = ui.tableWidget_layer_view->item(i, 0)->text().toInt();
		if (insertedLayer == layerNr)
			return;
		else if (insertedLayer < layerNr)
			targetRow = i + 1;
		else if (insertedLayer > layerNr)
		{
			QComboBox* combo_ = (QComboBox*)ui.tableWidget_layer_view->cellWidget(i, 1);
			combo_->setProperty("row", i + 1);
			combo_->setFont(QFont("Malgun Gothic", 9, QFont::Normal));
		}
	}
	ui.tableWidget_layer_view->insertRow(targetRow);
	ui.tableWidget_layer_view->setItem(targetRow, 0, new QTableWidgetItem(QString::number(layerNr)));
	ui.tableWidget_layer_view->setItem(targetRow, 1, new QTableWidgetItem(QString::number(pCartIdx)));
	//0과 max layer는 layer number 수정 불가
	if (layerNr == 0 || layerNr == m_maxLayer)
	{
		QTableWidgetItem *item = ui.tableWidget_layer_view->item(targetRow, 0);
		item->setFlags(item->flags() &= ~Qt::ItemIsEditable);
	}

	QComboBox* combo = new QComboBox();
	combo->setFont(QFont("Malgun Gothic", 9, QFont::Normal));
	if (layerNr == m_maxLayer)
	{
		combo->addItem(CustomTranslate::tr("End"));
		combo->setCurrentIndex(0);
		combo->setEnabled(false);
	}
	else if (cartCnt <= 1)
	{
		if (layerNr == 0)
		{
			combo->addItem(CustomTranslate::tr("Start"));
			pCartIdx = 0;
		}
		else
		{
			combo->addItem(CustomTranslate::tr("Pause"));
			pCartIdx = -1;
		}

		combo->setCurrentIndex(0);
		combo->setEnabled(false);
	}
	else
	{
		int currIdx = pCartIdx;
		//pause 제거용 로직 삭제
		/*if (layerNr != 0)
		{
			combo->addItem(tr("Pause"));
			currIdx = currIdx + 1;
		}*/
		//
		for (int i = 0; i < CartridgeInfo::cartridges.size(); i++)
		{
			combo->addItem(CustomTranslate::tr("Cartridge") + " " + QString::number(i + 1));
			combo->setFont(QFont("Malgun Gothic", 9, QFont::Normal));
		}
		combo->setCurrentIndex(currIdx);
	}
	combo->setProperty("row", targetRow);
	connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(cartridgeComboBox_currentIndexChanged(int)));
	ui.tableWidget_layer_view->setCellWidget(targetRow, 1, combo);

	if (pInsertFlag)
		m_layerColorList->insert(m_layerColorList->begin() + targetRow, LayerColor(layerNr, pCartIdx));
}

void ColoredLayerDialog::updateLayer(int row_)
{
	QTableWidgetItem* targetCell = ui.tableWidget_layer_view->item(row_, 0);
	if (!targetCell->isSelected())
		return;
	//첫번째와 마지막 row는 대상에서 제외
	if (row_ == 0 || row_ == m_layerColorList->size() - 1)
		return;

	int layer = targetCell->text().toInt();
	int beforeLayer = m_layerColorList->at(row_).layerNr;
	if (layer == beforeLayer)
		return;
	if (layer > m_maxLayer)
	{
		targetCell->setText(QString::number(beforeLayer));
		return;
	}
	if (layer <= 0)
	{
		targetCell->setText(QString::number(beforeLayer));
		return;
	}

	LayerColor* layerColor = &m_layerColorList->at(row_);
	int cartridgeIdx = m_layerColorList->at(row_).cartridgeIndex;

	for (auto &&it : *m_layerColorList)
	{
		if (&it == layerColor)
			continue;
		else if (it.layerNr == layer)
		{
			targetCell->setText(QString::number(beforeLayer));
			return;
		}
	}
	qDebug() << "preLayerNr : " << beforeLayer << " : layerNr : " << layer;
	deleteLayer();
	addLayer(layer, cartridgeIdx, true);
}

void ColoredLayerDialog::deleteLayer()
{
	QList<QTableWidgetItem *> selected = ui.tableWidget_layer_view->selectedItems();
	int insertedLayer;
	int rowCnt;
	int selectedRow;
	int selectedLayer;
	if (selected.size() == 0) return;
	qSort(selected);
	for (int i = selected.size() - 1; i >= 0; i--)
	{
		rowCnt = ui.tableWidget_layer_view->rowCount();
		selectedLayer = selected.at(i)->text().toInt();
		selectedRow = selected.at(i)->row();
		for (int j = rowCnt - 1; j >= 0; j--)
		{
			insertedLayer = ui.tableWidget_layer_view->item(j, 0)->text().toInt();
			if (insertedLayer > selectedLayer)
			{
				QComboBox* combo = (QComboBox*)ui.tableWidget_layer_view->cellWidget(j, 1);
				combo->setProperty("row", j - 1);
			}
			else if (selectedLayer == 0) continue;
			else if (selectedLayer == m_maxLayer) continue;
			else if (insertedLayer == selectedLayer)
			{
				ui.tableWidget_layer_view->removeRow(selectedRow);
				m_layerColorList->erase(m_layerColorList->begin() + selectedRow);
			}
		}

	}
}

void ColoredLayerDialog::resetLayer()
{
	ui.tableWidget_layer_view->clearContents();
	ui.tableWidget_layer_view->setRowCount(0);
	addLayer(0);
	addLayer(m_maxLayer);

	m_layerColorList->clear();
	m_layerColorList->push_back(LayerColor(-10, 0));
	m_layerColorList->push_back(LayerColor(10000, 0));

	ui.verticalSlider_layer_height->setValue(m_maxLayer / 2);
}
bool ColoredLayerDialog::checkReset()
{
	if (m_layerColorList->size() > 2)
		return true;
	return false;
}
void ColoredLayerDialog::equalize(int equalizeNum_)
{
	if (equalizeNum_ == 0 || equalizeNum_ == 1)
		return;

	int cartCnt = CartridgeInfo::getCartCount();
	int cartNo;

	int currentRemainLayer = m_maxLayer;
	int divCount = equalizeNum_;
	int divLayer;
	int targetLayer = 0;
	int initCart = CartridgeInfo::layerColorList.front().cartridgeIndex;
	//마지막 값이 너무 커지거나 작아지는 것을 방지하기 위해 매번 새로 계산함.
	for (int i = 1; i < equalizeNum_; i++)
	{
		divLayer = (((double)currentRemainLayer / divCount) + 0.5);
		targetLayer = targetLayer + divLayer;
		divCount--;
		currentRemainLayer = currentRemainLayer - divLayer;
		cartNo = (i + initCart) % cartCnt;
		addLayer(targetLayer, cartNo);
	}
}

void ColoredLayerDialog::updateCartridge(int selectedCart)
{
	QComboBox* combo = qobject_cast<QComboBox*>(sender());
	int a = combo->rootModelIndex().row();
	int row = combo->property("row").toInt();
	ui.tableWidget_layer_view->item(row, 1)->setText(QString::number(selectedCart));
	if (row != 0)
	{
		//pause 제거용 로직 추가
		if (CartridgeInfo::cartridges.size() <= 1) //
			selectedCart = -1;
	}
	m_layerColorList->at(row).cartridgeIndex = selectedCart;
}

bool ColoredLayerDialog::isVerticalScrollBarVisible()
{
	bool isVisible = false;

	int HeightOfAllRows = 0;
	for (int i = 0; i < ui.tableWidget_layer_view->rowCount(); i++)
		HeightOfAllRows += ui.tableWidget_layer_view->rowHeight(i);

	int HeaderHeight = ui.tableWidget_layer_view->horizontalHeader()->height();
	int TableHeight = ui.tableWidget_layer_view->height();
	if (TableHeight < 100) TableHeight = 443;//생성자에서 호출될때 height를 제대로 찾지 못함.

	if ((HeightOfAllRows + HeaderHeight) > TableHeight)
		isVisible = true;

	return isVisible;
}
void ColoredLayerDialog::checkVerticalScrollBarVisible()
{
	bool isVisible = isVerticalScrollBarVisible();
	if (isVisible)
	{
		ui.tableWidget_layer_view->setColumnWidth(0, 60);
		ui.tableWidget_layer_view->setColumnWidth(1, 120);
		ui.tableWidget_layer_view->repaint();
	}
	else
	{
		ui.tableWidget_layer_view->setColumnWidth(0, 60);
		ui.tableWidget_layer_view->setColumnWidth(1, 138);
		ui.tableWidget_layer_view->repaint();
	}
}

void ColoredLayerDialog::layerClicked(QTableWidgetItem* clickedItem)
{
	if (clickedItem->column() == 0)
	{
		int row = clickedItem->text().toInt();
		ui.verticalSlider_layer_height->setValue(row);
	}
	else return;
}

void ColoredLayerDialog::cartridgeComboBox_currentIndexChanged(int value_)
{
	updateCartridge(value_);
	emit signal_updateGL();
}
void ColoredLayerDialog::model_changed()
{
	setMaxLayer();
	setLayerFromLayerList();
	//viewerModule->setLayerColorUIMode();//to do sj
}
void ColoredLayerDialog::verticalSlider_layer_height_valueChanged(int value_)
{
	updateSpinBox(value_);
}
void ColoredLayerDialog::spinBox_current_layer_valueChanged(int value_)
{
	updateSlider(value_);
}
void ColoredLayerDialog::pushButton_add_clicked()
{
	addLayer();
	emit signal_updateGL();
}
void ColoredLayerDialog::pushButton_delete_clicked()
{
	deleteLayer();
	emit signal_updateGL();
}
void ColoredLayerDialog::pushButton_reset_clicked()
{
	resetLayer();
	emit signal_updateGL();
}
void ColoredLayerDialog::pushButton_equalize_clicked()
{
	if (checkReset())
	{
		CommonDialog comDlg(viewerModule);
		comDlg.setDialogContents(MessageQuestion::tr("layer_color_reset_alert") + "\n" + MessageQuestion::tr("would_you_like_to_continue"), CommonDialog::Question, false, false, true, true);
		comDlg.exec();
		if (comDlg.isYes())
			resetLayer();
		else
			return;
	}
	equalize(ui.spinBox_equalize_num->value());
	emit signal_updateGL();
}
void ColoredLayerDialog::tableWidget_layer_view_itemClicked(QTableWidgetItem *tableWidgetItem_)
{
	layerClicked(tableWidgetItem_);
}
void ColoredLayerDialog::tableWidget_layer_view_cellChanged(int row_, int col_)
{
	if (col_ != 0)
		return;
	if (ui.tableWidget_layer_view->selectedItems().size() == 1)
	{
		updateLayer(row_);
		emit signal_updateGL();
	}
}
void ColoredLayerDialog::spinBox_equalize_num_enterPressed()
{
	pushButton_equalize_clicked();
}