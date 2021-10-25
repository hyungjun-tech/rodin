#pragma once

#include "ui_OptimizedDialog.h"
#include "PrintOptimum.h"

//class rodinP;
class QCustomPlot;
class ModelContainer;
class OptimizedDialog :	public QDialog
{
	Q_OBJECT

public:
	OptimizedDialog(QWidget *parent, ModelContainer *modelContainer_);
	~OptimizedDialog();

private slots:
	void optimizedStart();
	void selectButtonSend();

private:
	double getMinValue_vector(const std::vector<double>& factor);
	double getMaxValue_vector(const std::vector<double>& factor);
	int getMaxIndex_vector(const std::vector<double>& value);
	std::vector<double> calLinearNormalization(const std::vector<double>& factor);
	std::vector<double> calLinearNormalizationInverse(const std::vector<double>& factor);
	double getStandardDeviation(const std::vector<double>& factor);

	double getNormalValuewithSTDEV(int direction, int factor);
	double getEvaluationIndex(int direction);
	
	void setGroupBoxHighlight(int index);
	void initGroupBoxHighlight();
	void initPlot();
	void plotEindex();

	void tempPrintValue();
	void setRank(const std::vector<double>& rankVec);
private:
	Ui::OptimizedDialog ui;
	QWidget* parent;
	ModelContainer* modelContainer;

	PrintOptimum* m_PrintOptimum;

	std::vector<QLabel*> labelVec;
	std::vector<QFrame*> boxVec;
	std::vector<QLabel*> imageVec;
	std::vector<QPushButton*> buttonVec;
	std::vector<QCustomPlot*> customPlot;
		
	std::vector<double> normal_thicknessRigionLength;
	std::vector<double> normal_overhangLength;
	std::vector<double> normal_supportFilamentLength;
			
	std::vector<double> cost_thicknessRigionLength;			// mm
	std::vector<double> cost_overhangLength;				// mm
	std::vector<double> cost_supportFilamentLength;			// mm

	std::vector<double> Eindex_direction;
};

