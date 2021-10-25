#pragma once

class ConfigSettings;
class ModelContainer;
class SliceLayer;
class ProgressHandler;
class PrintOptimum : public QObject
{
	Q_OBJECT
public:
	PrintOptimum(QWidget* parent_);
	~PrintOptimum();

	void setProgressHandler(ProgressHandler* handler_);
	bool optimize(ModelContainer* modelContainer_, double overhang_angle, ConfigSettings config_);
	void rotateModelSelected(int direction);

	std::vector<double> angle;
	std::vector<QVector3D> axis;

private:
	QWidget* parent;
	ModelContainer* modelContainer;
	ProgressHandler* progressHandler;

	//void orienting();
	bool slicing();
	double calcThickness(std::vector<SliceLayer> sliceLayers);
	int checkCount;
public:
	std::vector<double> cost_supportFilamentLength;			//cost_supportVolume;	// mm*mm
	std::vector<double> cost_overhangLength;				// mm*mm
	std::vector<double> cost_thicknessRigionLength;			// mm*mm
	std::vector<QImage> thumbnailImages;
	int getCheckCount() { return checkCount; }
};