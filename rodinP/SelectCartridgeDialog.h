#pragma once

#include "ui_SelectCartridgeDialog.h"
#include "CartridgeDialog.h"
#include "ViewerModule.h"

class SelectCartridgeDialog : public QDialog
{
	Q_OBJECT

public:
	SelectCartridgeDialog(QWidget* _parent = 0);
	~SelectCartridgeDialog();
	void init(ViewerModule* _viewerModule);
	void setContents();
public slots:
	void changeCartridgeColor();
private slots:
	void refreshContents();
	void volumeSelected();
	void mouseIsOnVolumeMaterial(int, int);
	void mouseIsOutVolumeMaterial(int, int);
	void mouseEnter(int, int);
	void mouseLeave(int, int);
	void frameClicked(int, int);
	void frameShiftClicked(int, int);
	void frameControlClicked(int, int);
	void cartridgeChanged(int);
	void pushButton_cartridgeInfoClicked();
private:
	Ui::SelectCartridgeDialog ui;
	//QIcon filaDown;
	//QIcon filaUp;
	QIcon filaDownIcon;
	QIcon filaUpIcon;
	ViewerModule* viewerModule;
	ModelContainer* modelContainer;

	void setConnection();
	void addNoModelLabel();
	void removeAllWidgets();
	void addObjectWidget(int idx);
	void addMeshWidget(int objectIdx, int meshIdx);
	void setSelectedFrameStyle(int objectIdx, int meshIdx, QColor meshColor, bool selected = false);
signals:
	void signal_pushButton_cartridgeInfoClicked();
};



class frameMouseEvent : public QFrame
{
	Q_OBJECT
public:
	frameMouseEvent(QWidget *parent = 0);
	virtual void enterEvent(QEvent * e);
	virtual void leaveEvent(QEvent * e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
signals:
	void frameMouseEnter(int, int);
	void frameMouseLeave(int, int);
	void signal_frameClicked(int, int);
	void signal_frameShiftClicked(int, int);
	void signal_frameControlClicked(int, int);
private:
	int m_objectIdx;
	int m_meshIdx;
};