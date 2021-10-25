#include "stdafx.h"
#include "OptimizedDialog.h"
//#include "rodinP.h"

#include "qcustomplot.h"
#include "ModelContainer.h"
#include "settings.h"
#include "ProgressHandler.h"

OptimizedDialog::OptimizedDialog(QWidget *parent_, ModelContainer *modelContainer_)
	: QDialog(parent_)
	, parent(parent_)
	, modelContainer(modelContainer_)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);
	setModal(true);

	m_PrintOptimum = new PrintOptimum(this);
	int checkCount = m_PrintOptimum->getCheckCount();
	normal_thicknessRigionLength.resize(checkCount);
	normal_overhangLength.resize(checkCount);
	normal_supportFilamentLength.resize(checkCount);

	cost_thicknessRigionLength.resize(checkCount);
	cost_overhangLength.resize(checkCount);
	cost_supportFilamentLength.resize(checkCount);

	Eindex_direction.resize(checkCount);

	connect(ui.pushButton_startOptimize, SIGNAL(clicked()), this, SLOT(optimizedStart(void)));

	connect(ui.pushButton_select_0, SIGNAL(clicked()), this, SLOT(selectButtonSend()));
	connect(ui.pushButton_select_1, SIGNAL(clicked()), this, SLOT(selectButtonSend()));
	connect(ui.pushButton_select_2, SIGNAL(clicked()), this, SLOT(selectButtonSend()));
	connect(ui.pushButton_select_3, SIGNAL(clicked()), this, SLOT(selectButtonSend()));
	connect(ui.pushButton_select_4, SIGNAL(clicked()), this, SLOT(selectButtonSend()));
	connect(ui.pushButton_select_5, SIGNAL(clicked()), this, SLOT(selectButtonSend()));

	/////////////////////////////////////////////
	customPlot.resize(checkCount);

	for (int i = 0; i < checkCount; i++)
	{
		customPlot[i] = new QCustomPlot();
		customPlot[i]->setFixedSize(65, 200);
		customPlot[i]->setBaseSize(QSize(65, 200));
	}

	customPlot[0]->setParent(ui.widget_chart_bar_0);
	customPlot[1]->setParent(ui.widget_chart_bar_1);
	customPlot[2]->setParent(ui.widget_chart_bar_2);
	customPlot[3]->setParent(ui.widget_chart_bar_3);
	customPlot[4]->setParent(ui.widget_chart_bar_4);
	customPlot[5]->setParent(ui.widget_chart_bar_5);

	labelVec.push_back(ui.label_eindex_0);
	labelVec.push_back(ui.label_eindex_1);
	labelVec.push_back(ui.label_eindex_2);
	labelVec.push_back(ui.label_eindex_3);
	labelVec.push_back(ui.label_eindex_4);
	labelVec.push_back(ui.label_eindex_5);

	boxVec.push_back(ui.direction_0);
	boxVec.push_back(ui.direction_1);
	boxVec.push_back(ui.direction_2);
	boxVec.push_back(ui.direction_3);
	boxVec.push_back(ui.direction_4);
	boxVec.push_back(ui.direction_5);

	imageVec.push_back(ui.thumbnailImage_0);
	imageVec.push_back(ui.thumbnailImage_1);
	imageVec.push_back(ui.thumbnailImage_2);
	imageVec.push_back(ui.thumbnailImage_3);
	imageVec.push_back(ui.thumbnailImage_4);
	imageVec.push_back(ui.thumbnailImage_5);

	buttonVec.push_back(ui.pushButton_select_0);
	buttonVec.push_back(ui.pushButton_select_1);
	buttonVec.push_back(ui.pushButton_select_2);
	buttonVec.push_back(ui.pushButton_select_3);
	buttonVec.push_back(ui.pushButton_select_4);
	buttonVec.push_back(ui.pushButton_select_5);

	for (int i = 0; i < buttonVec.size(); i++)
	{
		buttonVec[i]->setVisible(false);
	}

	initPlot();
}
OptimizedDialog::~OptimizedDialog()
{
	parent->setAcceptDrops(true);
	delete m_PrintOptimum;
}

void OptimizedDialog::optimizedStart()
{
	if (!modelContainer->hasAnySelectedModel())
		return;

	ProgressHandler progressHandler(parent);
	m_PrintOptimum->setProgressHandler(&progressHandler);
	if (m_PrintOptimum->optimize(modelContainer, Profile::sliceProfileCommon.support_angle.value, Profile::configSettings.front()))
	{
		QPixmap thumbnail0 = QPixmap::fromImage(m_PrintOptimum->thumbnailImages[0]);
		QPixmap thumbnail1 = QPixmap::fromImage(m_PrintOptimum->thumbnailImages[1]);
		QPixmap thumbnail2 = QPixmap::fromImage(m_PrintOptimum->thumbnailImages[2]);
		QPixmap thumbnail3 = QPixmap::fromImage(m_PrintOptimum->thumbnailImages[3]);
		QPixmap thumbnail4 = QPixmap::fromImage(m_PrintOptimum->thumbnailImages[4]);
		QPixmap thumbnail5 = QPixmap::fromImage(m_PrintOptimum->thumbnailImages[5]);

		ui.thumbnailImage_0->setPixmap(thumbnail0.scaled(ui.thumbnailImage_0->size().width(), ui.thumbnailImage_0->size().height(), Qt::IgnoreAspectRatio, Qt::FastTransformation));
		ui.thumbnailImage_1->setPixmap(thumbnail1.scaled(ui.thumbnailImage_1->size().width(), ui.thumbnailImage_1->size().height(), Qt::IgnoreAspectRatio, Qt::FastTransformation));
		ui.thumbnailImage_2->setPixmap(thumbnail2.scaled(ui.thumbnailImage_2->size().width(), ui.thumbnailImage_2->size().height(), Qt::IgnoreAspectRatio, Qt::FastTransformation));
		ui.thumbnailImage_3->setPixmap(thumbnail3.scaled(ui.thumbnailImage_3->size().width(), ui.thumbnailImage_3->size().height(), Qt::IgnoreAspectRatio, Qt::FastTransformation));
		ui.thumbnailImage_4->setPixmap(thumbnail4.scaled(ui.thumbnailImage_4->size().width(), ui.thumbnailImage_4->size().height(), Qt::IgnoreAspectRatio, Qt::FastTransformation));
		ui.thumbnailImage_5->setPixmap(thumbnail5.scaled(ui.thumbnailImage_5->size().width(), ui.thumbnailImage_5->size().height(), Qt::IgnoreAspectRatio, Qt::FastTransformation));

		cost_thicknessRigionLength = m_PrintOptimum->cost_thicknessRigionLength;
		cost_overhangLength = m_PrintOptimum->cost_overhangLength;
		cost_supportFilamentLength = m_PrintOptimum->cost_supportFilamentLength;

		//get normal inverse
		normal_thicknessRigionLength = this->calLinearNormalizationInverse(cost_thicknessRigionLength);
		normal_overhangLength = this->calLinearNormalizationInverse(cost_overhangLength);
		normal_supportFilamentLength = this->calLinearNormalizationInverse(cost_supportFilamentLength);

		//get evaluation index_direction
		for (int i = 0; i < Eindex_direction.size(); i++)
		{
			Eindex_direction[i] = this->getEvaluationIndex(i);

			printf("Evaluation index by %d direction : %.4f\n", i, Eindex_direction[i]);
		}

		tempPrintValue();

		this->plotEindex();

		for (int i = 0; i < buttonVec.size(); i++)
		{
			buttonVec[i]->setVisible(true);
		}

		setRank(Eindex_direction);

		this->setGroupBoxHighlight(this->getMaxIndex_vector(Eindex_direction));
	}
}
void OptimizedDialog::selectButtonSend()
{
	if (!modelContainer->hasAnySelectedModel())
		return;

	QPushButton* buttonIcon = dynamic_cast<QPushButton*>(sender());
	int direction;

	if (buttonIcon == ui.pushButton_select_0)		direction = 0;
	else if (buttonIcon == ui.pushButton_select_1)	direction = 1;
	else if (buttonIcon == ui.pushButton_select_2)	direction = 2;
	else if (buttonIcon == ui.pushButton_select_3)	direction = 3;
	else if (buttonIcon == ui.pushButton_select_4)	direction = 4;
	else direction = 5;

	m_PrintOptimum->rotateModelSelected(direction);

	this->close();
}

double OptimizedDialog::getMinValue_vector(const std::vector<double>& factor)
{
	auto it = std::min_element(factor.begin(), factor.end());
	return *it;
}
double OptimizedDialog::getMaxValue_vector(const std::vector<double>& factor)
{
	auto it = std::max_element(factor.begin(), factor.end());
	return *it;
}
int OptimizedDialog::getMaxIndex_vector(const std::vector<double>& value)
{
	return std::distance(value.begin(), std::max_element(value.begin(), value.end()));
}
std::vector<double> OptimizedDialog::calLinearNormalization(const std::vector<double>& factor)
{
	std::vector<double> tempNormal;
	tempNormal.resize(factor.size());

	double minValue = 0.0;
	double maxValue = 0.0;

	minValue = getMinValue_vector(factor);
	maxValue = getMaxValue_vector(factor);

	for (int i = 0; i < tempNormal.size(); i++)
	{
		if (minValue == maxValue) tempNormal[i] = 0.0;
		else tempNormal[i] = ((factor[i] - minValue) / (maxValue - minValue)) + 0.1;	// origin value shifting
	}

	return tempNormal;
}
std::vector<double> OptimizedDialog::calLinearNormalizationInverse(const std::vector<double>& factor)
{
	std::vector<double> tempNormal;
	tempNormal.resize(factor.size());

	double minValue = 0.0;
	double maxValue = 0.0;

	minValue = getMinValue_vector(factor);
	maxValue = getMaxValue_vector(factor);

	for (int i = 0; i < tempNormal.size(); i++)
	{
		if (minValue == maxValue) tempNormal[i] = 0.0;
		else
		{
			tempNormal[i] = (factor[i] - minValue) / (maxValue - minValue);

			tempNormal[i] = (1 - tempNormal[i]) + 0.1; //inverse
		}

	}

	return tempNormal;
}
double OptimizedDialog::getStandardDeviation(const std::vector<double>& factor)
{
	double deviation = 0.0;
	double sum = 0.0;
	double average = 0.0;

	if (factor.size() == 0) return 0.0;

	for (int i = 0; i < factor.size(); i++)
	{
		sum = sum + factor[i];
	}

	average = sum / factor.size();

	double temp_sum = 0.0;

	for (int j = 0; j < factor.size(); j++)
	{
		temp_sum = temp_sum + (factor[j] - average) * (factor[j] - average);
	}

	deviation = temp_sum / factor.size();

	return std::sqrt(deviation);
}

double OptimizedDialog::getNormalValuewithSTDEV(int direction, int factor)
{
	double tempValue = 0.0;

	switch (factor)
	{
	case 0:
		tempValue = normal_thicknessRigionLength[direction] * getStandardDeviation(normal_thicknessRigionLength);
		break;
	case 1:
		tempValue = normal_overhangLength[direction] * getStandardDeviation(normal_overhangLength);
		break;
	case 2:
		tempValue = normal_supportFilamentLength[direction] * getStandardDeviation(normal_supportFilamentLength);
		break;
	}

	return tempValue;
}
double OptimizedDialog::getEvaluationIndex(int direction)
{
	double tempValue = 0.0;

	//weight_sum = weight_thicknessRigionLength + weight_overhangLength + weight_supportFilamentLength;


	for (int i = 0; i < 3; i++)
	{
		tempValue = tempValue + getNormalValuewithSTDEV(direction, i);
	}

	return tempValue / 3;
}


void OptimizedDialog::plotEindex()
{
	std::vector<QCPBars*> bar;
	bar.resize(3);

	QPen pen;
	pen.setWidthF(0.5);
	pen.setColor(QColor(80, 80, 80));

	for (int i = 0; i < customPlot.size(); i++)
	{
		customPlot[i]->clearGraphs();
		customPlot[i]->clearItems();
		customPlot[i]->clearPlottables();
		customPlot[i]->clearFocus();
		customPlot[i]->clearMask();

		// create empty bar chart objects:
		for (int j = 0; j < bar.size(); j++)
		{
			bar[j] = new QCPBars(customPlot[i]->xAxis, customPlot[i]->yAxis);
			customPlot[i]->addPlottable(bar[j]);

			// set names and colors:
			bar[j]->setPen(pen);
			bar[j]->setWidth(1);
		}

		bar[0]->setBrush(QColor(181, 85, 82));
		bar[1]->setBrush(QColor(222, 174, 66));
		bar[2]->setBrush(QColor(132, 134, 189));


		// prepare x axis with country labels:
		QVector<double> ticks;
		QVector<QString> labels;
		ticks << 1 << 2 << 3;
		labels << "A" << "B" << "C";
		customPlot[i]->xAxis->setAutoTicks(false);
		customPlot[i]->xAxis->setAutoTickLabels(false);
		customPlot[i]->xAxis->setTickVector(ticks);
		customPlot[i]->xAxis->setTickVectorLabels(labels);
		customPlot[i]->xAxis->setTickLabelRotation(0);
		customPlot[i]->xAxis->setSubTickCount(0);
		customPlot[i]->xAxis->setTickLength(0, 1);
		customPlot[i]->xAxis->grid()->setVisible(false);
		customPlot[i]->xAxis->setRange(0, 4);


		// prepare y axis:
		customPlot[i]->yAxis->setVisible(false);
		customPlot[i]->yAxis->setRange(0, 0.7);

		// Add data:
		QVector<double> EindexData0;
		QVector<double> EindexData1;
		QVector<double> EindexData2;

		EindexData0 << getNormalValuewithSTDEV(i, 0);
		EindexData1 << 0 << getNormalValuewithSTDEV(i, 1);
		EindexData2 << 0 << 0 << getNormalValuewithSTDEV(i, 2);

		bar[0]->setData(ticks, EindexData0);
		bar[1]->setData(ticks, EindexData1);
		bar[2]->setData(ticks, EindexData2);

		customPlot[i]->replot();
	}
}
void OptimizedDialog::initPlot()
{
	for (int i = 0; i < customPlot.size(); i++)
	{
		customPlot[i]->clearGraphs();
		customPlot[i]->clearItems();
		customPlot[i]->clearPlottables();
		customPlot[i]->clearFocus();
		customPlot[i]->clearMask();

		// prepare x axis with country labels:
		QVector<double> ticks;
		QVector<QString> labels;
		ticks << 1 << 2 << 3;
		labels << "A" << "B" << "C";
		customPlot[i]->xAxis->setAutoTicks(false);
		customPlot[i]->xAxis->setAutoTickLabels(false);
		customPlot[i]->xAxis->setTickVector(ticks);
		customPlot[i]->xAxis->setTickVectorLabels(labels);
		customPlot[i]->xAxis->setTickLabelRotation(0);
		customPlot[i]->xAxis->setSubTickCount(0);
		customPlot[i]->xAxis->setTickLength(0, 1);
		customPlot[i]->xAxis->grid()->setVisible(false);
		customPlot[i]->xAxis->setRange(0, 4);

		// prepare y axis:
		customPlot[i]->yAxis->setVisible(false);
		customPlot[i]->yAxis->setRange(0, 1);


		customPlot[i]->setBackground(QBrush(QColor(200, 200, 200)));


		customPlot[i]->replot();

	}
}

void OptimizedDialog::tempPrintValue()
{
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 6; i++)
		{
			switch (j)
			{
			case 0:
				printf("cost_thicknessRigionLength[%d] : %.4f\n", i, m_PrintOptimum->cost_thicknessRigionLength[i]);
				break;
			case 1:
				printf("cost_overhangLength[%d] : %.4f\n", i, m_PrintOptimum->cost_overhangLength[i]);
				break;
			case 2:
				printf("cost_supportFilamentLength[%d] : %.4f\n", i, m_PrintOptimum->cost_supportFilamentLength[i]);
				break;
			}
		}
		printf("////////////////////////////////////////////////////////\n");
	}
}

void OptimizedDialog::setGroupBoxHighlight(int index)
{
	this->initGroupBoxHighlight();

	for (int i = 0; i < labelVec.size(); i++)
	{
		if (labelVec[i]->property("rank").toInt() == 1)
		{
			boxVec[i]->setStyleSheet("QFrame{background-color: rgb(21, 44, 67);}");
			labelVec[i]->setStyleSheet("QLabel {color : #ffffff;}");
			imageVec[i]->setStyleSheet("background-color: rgb(224,224,224);");
			customPlot[i]->setBackground(QBrush(QColor(21, 44, 67)));
			customPlot[i]->xAxis->setTickLabelColor(Qt::white);
			customPlot[i]->xAxis->setTickPen(QPen(QColor(Qt::white)));
			customPlot[i]->xAxis->setBasePen(QPen(QColor(Qt::white)));
			customPlot[i]->replot();
		}
	}
}
void OptimizedDialog::initGroupBoxHighlight()
{
	for (int i = 0; i < labelVec.size(); i++)
	{
		boxVec[i]->setStyleSheet("QFrame{background-color: rgb(200, 200, 200);}");
		labelVec[i]->setStyleSheet("QLabel {color : #000000;}");
		imageVec[i]->setStyleSheet("background-color: rgb(224,224,224);");
		customPlot[i]->setBackground(QBrush(QColor(200, 200, 200)));
		customPlot[i]->xAxis->setTickLabelColor(Qt::black);
		customPlot[i]->xAxis->setTickPen(QPen(QColor(Qt::black)));
		customPlot[i]->xAxis->setBasePen(QPen(QColor(Qt::black)));
		customPlot[i]->replot();
	}
}

void OptimizedDialog::setRank(const std::vector<double>& rankVec)
{
	std::vector<double> idxRank = rankVec;

	auto sortingFunction = [](const double& a, const double& b)->bool
	{
		return a > b;
	};
	std::sort(idxRank.begin(), idxRank.end(), sortingFunction);

	for (int i = 0; i < rankVec.size(); i++)
	{
		for (int j = 0; j < idxRank.size(); j++)
		{
			if (idxRank[j] == rankVec[i])
			{
				labelVec[i]->setText(tr("Rank") + " " + QString::number(j + 1));
				labelVec[i]->setProperty("rank", QVariant(j + 1));
				//qDebug() << labelVec[i]->property("rank").toInt();
				break;
			}
		}
	}
}