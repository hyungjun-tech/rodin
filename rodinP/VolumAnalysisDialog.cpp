#include "stdafx.h"
#include "VolumAnalysisDialog.h"
#include "VolumeAnalysis.h"
#include "settings.h"
#include "SliceProcessor.h"
#include "ViewerModule.h"

VolumeAnalysisDialog::VolumeAnalysisDialog(QWidget* _parent)
	: QDialog(_parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);
	ui.horizontalSlider_overhang->setEnabled(false);
	ui.spinBox_overhang->setEnabled(false);

	//viewerModule->SetAnalysisUIMode();
}

VolumeAnalysisDialog::~VolumeAnalysisDialog()
{
	if (m_VolumeAnalysis)
		m_VolumeAnalysis->clear();
	
	viewerModule->setTranslateUIMode();
	viewerModule->update();
}


bool VolumeAnalysisDialog::init(ViewerModule* const _viewerModule)
{
	viewerModule = _viewerModule;
	connect(ui.doubleSpinBox_minthick, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBox_minthick_valueChanged(double)));
	connect(ui.horizontalSlider_minthick, SIGNAL(valueChanged(int)), this, SLOT(horizontalSlider_minthick_valueChanged(int)));
	connect(ui.horizontalSlider_overhang, SIGNAL(valueChanged(int)), this, SLOT(horizontalSlider_overhang_valueChanged(int)));
	connect(ui.checkBox_overhang, SIGNAL(toggled(bool)), this, SLOT(checkBox_overhang_toggled(bool)));

	connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(close()));
	connect(this, SIGNAL(rejected()), this, SLOT(close()));
	connect(this, SIGNAL(signal_updateGL()), viewerModule, SLOT(updateGL()));

	IMeshModel* model = viewerModule->modelContainer->getSelectedModels().front();
	//m_VolumeAnalysis = new VolumeAnalysis(model);
	m_VolumeAnalysis = viewerModule->getVolumeAnalysis();
	m_VolumeAnalysis->setMeshModel(model);

	ProgressHandler progressHandler(viewerModule);
	//m_VolumeAnalysis = volumeAnalysis_;
	m_VolumeAnalysis->setProgressHandler(&progressHandler);

	progressHandler.setWindowTitle(MessageProgress::tr("Calculating.."));
	progressHandler.setLabelText(MessageProgress::tr("Calculating Thickness.."));
	SliceProcessor processor;
	processor.init(viewerModule->modelContainer, true);
	processor.processingForPrintOptimum();

	if (!m_VolumeAnalysis->calcThickness())
	{
		m_VolumeAnalysis->clear();
		return false;
	}
	m_VolumeAnalysis->initializeOverhang();

	progressHandler.setValue(100);

	ui.horizontalSlider_minthick->setValue(40);
	ui.horizontalSlider_overhang->setValue(60);
	viewerModule->setAnalysisUIMode();
	viewerModule->update();

	//ui.checkBox_overhang->setChecked(false);

	//checkBox_overhang_toggled(false);
	return true;
}

void VolumeAnalysisDialog::doubleSpinBox_minthick_valueChanged(double value)
{
	ui.horizontalSlider_minthick->setValue(round(value * 100));
}

void VolumeAnalysisDialog::horizontalSlider_minthick_valueChanged(int value)
{
	double minThick = value*0.01;
	int compareValue = round(ui.doubleSpinBox_minthick->value() * 100);

	if (compareValue != value)
		ui.doubleSpinBox_minthick->setValue(minThick);

	m_VolumeAnalysis->assignPolygonColor(minThick);
	emit signal_updateGL();
}

void VolumeAnalysisDialog::horizontalSlider_overhang_valueChanged(int value)
{
	m_VolumeAnalysis->calcOverhangeRegion(value);
	emit signal_updateGL();
}

void VolumeAnalysisDialog::checkBox_overhang_toggled(bool state)
{
	ui.horizontalSlider_minthick->setEnabled(!state);
	ui.doubleSpinBox_minthick->setEnabled(!state);

	ui.horizontalSlider_overhang->setEnabled(state);
	ui.spinBox_overhang->setEnabled(state);

	m_VolumeAnalysis->setOverhangMode(state);
	emit signal_updateGL();
}