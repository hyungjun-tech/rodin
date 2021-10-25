#pragma once

#include "ui_ColoredLayerDialog.h"
#include "ViewerModule.h"

//class rodinP;
class LayerColor;
class ColoredLayerDialog :	public QDialog
{
	Q_OBJECT

public:
	ColoredLayerDialog(QWidget* _parent = 0);
	~ColoredLayerDialog();
	void init(ViewerModule* _viewerModule);
	void resetLayer();
	bool checkReset();
	void equalize(int equalizeNum_);
public slots:
	void setContents();
private:
	Ui::ColoredLayerDialog ui;

	void setLayerFromLayerList();
	QIcon filaDownIcon;
	QIcon filaUpIcon;

	ViewerModule* viewerModule;
	ModelContainer* modelContainer;

	std::vector<LayerColor> *m_layerColorList;

	int m_maxLayer;

	void setInitValue();
	void setMaxLayer();
	void setConnection();
	void updateSpinBox(int);
	void updateSlider(int);

	void addLayer(int pLayerNr = -999, int pCartIdx = 0, bool pInsertFlag = true);
	void updateLayer(int row_);
	void deleteLayer();
	void updateCartridge(int);

	void layerClicked(QTableWidgetItem*);
	bool isVerticalScrollBarVisible();
private slots:
	void cartridgeComboBox_currentIndexChanged(int value_);
	void model_changed();
	void verticalSlider_layer_height_valueChanged(int value_);
	void spinBox_current_layer_valueChanged(int value_);
	void pushButton_add_clicked();
	void pushButton_delete_clicked();
	void pushButton_reset_clicked();
	void pushButton_equalize_clicked();
	void tableWidget_layer_view_itemClicked(QTableWidgetItem *tableWidgetItem_);
	void tableWidget_layer_view_cellChanged(int row_, int col_);
	void spinBox_equalize_num_enterPressed();
	void checkVerticalScrollBarVisible();

signals:
	void signal_updateGL();
};