#pragma once

class ProgressHandler : public QObject
{
	Q_OBJECT
public:
	ProgressHandler(QWidget *parent_);
	~ProgressHandler();
	void show();
	bool wasCanceled();

public slots:
	void setMinimum(int minimum_);
	void setMaximum(int maximum_);
	void setValue(int value_);
	void setCustomValue(int _value);
	void setInitValue(int _value);
	void setTargetValue(int value);
	void setValue(float value_);
	void setLabelText(QString label_);
	void setWindowTitle(QString title_);
	void close();
	void tempProgress(int from_, int to_, int msSec_);
private:
	QProgressDialog *progressDialog;
	int initValue;
	int targetValue;
signals:
	void signal_canceled();
};