#pragma once

class ProfileDataD;
class ProfileDataI;
class ProfileDataB;
class ProfileSettingComponents : public QObject
{
	Q_OBJECT

public:
	ProfileSettingComponents(QObject *parent);
	~ProfileSettingComponents();

private:

};


class CustomDoubleSpinBox : public QDoubleSpinBox
{
	Q_OBJECT
public:
	CustomDoubleSpinBox(QWidget *parent = 0);

	void setUsingOriginalValue(bool _flag);
	void setChangeTracking(bool _flag);

	void setOriginalValue(double value);
	double getOriginalValue();
	void setProfileValue(ProfileDataD profileValue);
	void getProfileValue(ProfileDataD *profileValue);
	void resetValue();

public slots:
	void setEnabled(bool _enabled);
private slots:
	void checkValueChanged(double value);
	void checkEditingFinished();
private:
	bool eventFilter(QObject * o, QEvent * e);
	void setStyle();

	double originalValue;
	bool usingOriginalValue;
	bool changed;
	bool changeTracking;
signals:
	void signal_dataChanged();
};

class CustomSpinBox : public QSpinBox
{
	Q_OBJECT
public:
	CustomSpinBox(QWidget *parent = 0);

	void setUsingOriginalValue(bool _flag);
	void setUsingTemperatureTransfer(bool _flag);
	void setChangeTracking(bool _flag);

	void setOriginalValue(int value);
	int getOriginalValue();
	void setProfileValue(ProfileDataI profileValue);
	void getProfileValue(ProfileDataI *profileValue);
	void setProfileValue_min_max(ProfileDataI profileValue);
	void resetValue();
public slots:
	void setEnabled(bool _enabled);
private slots:
	void checkValueChanged(int value);
	void checkEditingFinished();
private:
	bool eventFilter(QObject * o, QEvent * e);
	void setStyle();

	int originalValue;
	bool usingOriginalValue;
	bool usingTemperatureTransfer;
	bool changed;
	bool changeTracking;
signals:
	void signal_dataChanged();
};

class CustomCheckBox : public QCheckBox
{
	Q_OBJECT
public:
	CustomCheckBox(QWidget *parent = 0);

	void setUsingOriginalValue(bool _flag);

	void setOriginalValue(bool value);
	bool getOriginalValue();
	void setProfileValue(ProfileDataB profileValue);
	void getProfileValue(ProfileDataB *profileValue);
	void resetValue();

public slots:
	void setEnabled(bool _enabled);
private slots:
	void checkValueChanged(int value);
private:
	void setStyle();

	bool originalValue;
	bool usingOriginalValue;
};

class CustomComboBox : public QComboBox
{
	Q_OBJECT
public:
	CustomComboBox(QWidget *parent = 0);

	void setUsingOriginalValue(bool _flag);

	void setOriginalValue(int value);
	int getOriginalValue();
	void setProfileValue(ProfileDataI profileValue);
	void getProfileValue(ProfileDataI *profileValue);
	void resetValue();

public slots:
	void setEnabled(bool _enabled);
private slots:
	void checkValueChanged(int value);
private:
	void setStyle();

	int originalValue;
	bool usingOriginalValue;
};


class CustomStringComboBox : public QComboBox
{
	Q_OBJECT
public:
	CustomStringComboBox(QWidget *parent = 0);

	void setUsingOriginalValue(bool _flag);

	void setOriginalValue(QString value);
	QString getOriginalValue();
	void setProfileValue(QString profileValue);
	void getProfileValue(QString *profileValue);
	void resetValue();

public slots:
	void setEnabled(bool _enabled);
private slots:
	void checkValueChanged(QString value);
private:
	void setStyle();

	QString originalValue;
	bool usingOriginalValue;
};