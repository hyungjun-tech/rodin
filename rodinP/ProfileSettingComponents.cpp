#include "stdafx.h"
#include "ProfileSettingComponents.h"
#include "ProfileData.h"

ProfileSettingComponents::ProfileSettingComponents(QObject *parent)
	: QObject(parent)
{

}

ProfileSettingComponents::~ProfileSettingComponents()
{

}


CustomDoubleSpinBox::CustomDoubleSpinBox(QWidget *parent)
	: QDoubleSpinBox(parent)
	, usingOriginalValue(true)
	, changed(false)
	, changeTracking(false)
{
	connect(this, SIGNAL(valueChanged(double)), this, SLOT(checkValueChanged(double)));
	connect(this, SIGNAL(editingFinished()), this, SLOT(checkEditingFinished()));
	this->installEventFilter(this);
	this->setFont(QFont("Malgun Gothic", 9, QFont::Normal));
}

void CustomDoubleSpinBox::setUsingOriginalValue(bool _flag)
{
	usingOriginalValue = _flag;
	setStyle();
}

void CustomDoubleSpinBox::setChangeTracking(bool _flag)
{
	changeTracking = _flag;
}

void CustomDoubleSpinBox::setOriginalValue(double value)
{
	if (!usingOriginalValue) return;

	originalValue = value;
	setStyle();
}

double CustomDoubleSpinBox::getOriginalValue()
{
	return originalValue;
}

void CustomDoubleSpinBox::setProfileValue(ProfileDataD profileValue)
{
	double value = profileValue.value;
	double minValue = profileValue.min;
	double maxValue = profileValue.max;

	if (minValue < maxValue)
	{
		this->setMinimum(minValue);
		this->setMaximum(maxValue);
	}
	this->setValue(value);
}

void CustomDoubleSpinBox::getProfileValue(ProfileDataD *profileValue)
{
	profileValue->value = value();
}

void CustomDoubleSpinBox::resetValue()
{
	if (!usingOriginalValue) return;
	setValue(originalValue);
}

void CustomDoubleSpinBox::checkValueChanged(double value)
{
	if (changeTracking)
		changed = true;

	if (!usingOriginalValue) return;
	setStyle();
}

void CustomDoubleSpinBox::checkEditingFinished()
{
	if (!changeTracking || !changed)
		return;

	emit signal_dataChanged();
	changed = false;
}

bool CustomDoubleSpinBox::eventFilter(QObject * o, QEvent * e)
{
	if (e->type() == QEvent::Wheel && qobject_cast<QAbstractSpinBox*>(o))
	{
		e->ignore();
		return true;
	}

	return QWidget::eventFilter(o, e);
}

void CustomDoubleSpinBox::setEnabled(bool _enabled)
{
	QDoubleSpinBox::setEnabled(_enabled);
	if (!usingOriginalValue) return;
	setStyle();
}

void CustomDoubleSpinBox::setStyle()
{
	if (!isEnabled() || !usingOriginalValue)
	{
		setStyleSheet("");
		return;
	}
	int l_round = qPow(10, decimals());
	if (round(originalValue * l_round) != round(value() * l_round))
		setStyleSheet("QDoubleSpinBox{ background-color: rgb(255, 255, 127); }");
	else
		setStyleSheet("");
}







CustomSpinBox::CustomSpinBox(QWidget *parent)
	: QSpinBox(parent)
	, usingOriginalValue(true)
	, usingTemperatureTransfer(false)
	, changed(false)
	, changeTracking(false)
{
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(checkValueChanged(int)));
	connect(this, SIGNAL(editingFinished()), this, SLOT(checkEditingFinished()));
	this->installEventFilter(this);
	this->setFont(QFont("Malgun Gothic", 9, QFont::Normal));
}

void CustomSpinBox::setUsingOriginalValue(bool _flag)
{
	usingOriginalValue = _flag;
	setStyle();
}
void CustomSpinBox::setUsingTemperatureTransfer(bool _flag)
{
	usingTemperatureTransfer = _flag;
}
void CustomSpinBox::setChangeTracking(bool _flag)
{
	changeTracking = _flag;
}

void CustomSpinBox::setOriginalValue(int value)
{
	if (!usingOriginalValue) return;


	if (Generals::temperatureUnit == "F")
	{
		if (usingTemperatureTransfer)
		{
			value = Generals::convertTemperatureUnitCtoF(value);
		}
	}

	originalValue = value;
	setStyle();
}

int CustomSpinBox::getOriginalValue()
{
	return originalValue;
}

void CustomSpinBox::setProfileValue(ProfileDataI profileValue)
{
	int value = profileValue.value;
	int minValue = profileValue.min;
	int maxValue = profileValue.max;

	if (Generals::temperatureUnit == "F")
	{
		if (usingTemperatureTransfer)
		{
			value = Generals::convertTemperatureUnitCtoF(value);
			minValue = Generals::convertTemperatureUnitCtoF(minValue);
			maxValue = Generals::convertTemperatureUnitCtoF(maxValue);
		}
	}

	if (minValue < maxValue)
	{
		this->setMinimum(minValue);
		this->setMaximum(maxValue);
	}
	this->setValue(value);
}

void CustomSpinBox::getProfileValue(ProfileDataI *profileValue)
{
	int val = value();
	if (Generals::temperatureUnit == "F")
	{
		if (usingTemperatureTransfer)
		{
			val = Generals::convertTemperatureUnitFtoC(val);
		}
	}

	profileValue->value = val;
}

void CustomSpinBox::setProfileValue_min_max(ProfileDataI profileValue)
{
	int minValue = profileValue.min;
	int maxValue = profileValue.max;

	if (Generals::temperatureUnit == "F")
	{
		if (usingTemperatureTransfer)
		{
			minValue = Generals::convertTemperatureUnitCtoF(minValue);
			maxValue = Generals::convertTemperatureUnitCtoF(maxValue);
		}
	}

	if (minValue < maxValue)
	{
		this->setMinimum(minValue);
		this->setMaximum(maxValue);
	}
	//this->setValue(value);
}

void CustomSpinBox::resetValue()
{
	if (!usingOriginalValue) return;
	setValue(originalValue);
}

void CustomSpinBox::checkValueChanged(int value)
{
	if (changeTracking)
		changed = true;

	if (!usingOriginalValue) return;
	setStyle();
}

void CustomSpinBox::checkEditingFinished()
{
	if (!changeTracking || !changed)
		return;

	emit signal_dataChanged();
	changed = false;
}

bool CustomSpinBox::eventFilter(QObject * o, QEvent * e)
{
	if (e->type() == QEvent::Wheel && qobject_cast<QAbstractSpinBox*>(o))
	{
		e->ignore();
		return true;
	}

	return QWidget::eventFilter(o, e);
}

void CustomSpinBox::setEnabled(bool _enabled)
{
	QSpinBox::setEnabled(_enabled);
	if (!usingOriginalValue) return;
	setStyle();
}

void CustomSpinBox::setStyle()
{
	if (!isEnabled() || !usingOriginalValue)
	{
		setStyleSheet("");
		return;
	}

	if (originalValue != value())
		setStyleSheet("QSpinBox{ background-color: rgb(255, 255, 127); }");
	else
		setStyleSheet("");
}






CustomCheckBox::CustomCheckBox(QWidget *parent)
	: QCheckBox(parent)
	, usingOriginalValue(true)
{
	connect(this, SIGNAL(stateChanged(int)), this, SLOT(checkValueChanged(int)));
}

void CustomCheckBox::setUsingOriginalValue(bool _flag)
{
	usingOriginalValue = _flag;
}

void CustomCheckBox::setOriginalValue(bool value)
{
	if (!usingOriginalValue) return;
	originalValue = value;
	setStyle();
}

bool CustomCheckBox::getOriginalValue()
{
	return originalValue;
}

void CustomCheckBox::setProfileValue(ProfileDataB profileValue)
{
	setChecked(profileValue.value);
	setStyle();
}

void CustomCheckBox::getProfileValue(ProfileDataB *profileValue)
{
	profileValue->value = isChecked();
}

void CustomCheckBox::resetValue()
{
	if (!usingOriginalValue) return;
	setChecked(originalValue);
}

void CustomCheckBox::checkValueChanged(int value)
{
	if (!usingOriginalValue) return;
	setStyle();
}

void CustomCheckBox::setEnabled(bool _enabled)
{
	QCheckBox::setEnabled(_enabled);
	if (!usingOriginalValue) return;
	setStyle();
}

void CustomCheckBox::setStyle()
{
	if (!isEnabled() || !usingOriginalValue)
	{
		setStyleSheet("");
		return;
	}

	bool ui_value = isChecked();
	if (originalValue != ui_value)
	{
		QString style = "QCheckBox::indicator\n{\n	";
		if (ui_value) style = style + "image: url(:/rodinP/Resources/checkMarker.png);\n	";
		style = style + "background-color: rgb(255, 255, 127);\n	width: 12px; height: 12px;\n	border-color: rgb(0, 0, 0);\n	border-width: 1px;\n	border-style: solid;\n};";
		setStyleSheet(style);
	}
	//ui_widget->setStyleSheet("QCheckBox::indicator\n{\n	background-color:#ffff7f;\n	border-color: rgb(0, 0, 0);\n	border-width: 1px;\n	border-style: solid;\n}");
	else
		setStyleSheet("");
}




CustomComboBox::CustomComboBox(QWidget *parent)
	: QComboBox(parent)
	, usingOriginalValue(true)
{
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(checkValueChanged(int)));
}

void CustomComboBox::setUsingOriginalValue(bool _flag)
{
	usingOriginalValue = _flag;
	setStyle();
}

void CustomComboBox::setOriginalValue(int value)
{
	if (!usingOriginalValue) return;
	originalValue = value;
	setStyle();
}

int CustomComboBox::getOriginalValue()
{
	return originalValue;
}

void CustomComboBox::setProfileValue(ProfileDataI profileValue)
{
	int val = profileValue.value;
	setCurrentIndex(val);
}

void CustomComboBox::getProfileValue(ProfileDataI *profileValue)
{
	int val = currentIndex();
	profileValue->value = val;
}

void CustomComboBox::resetValue()
{
	if (!usingOriginalValue) return;
	setCurrentIndex(originalValue);
}

void CustomComboBox::checkValueChanged(int value)
{
	if (!usingOriginalValue) return;
	setStyle();
}

void CustomComboBox::setEnabled(bool _enabled)
{
	QComboBox::setEnabled(_enabled);
	if (!usingOriginalValue) return;
	setStyle();
}

void CustomComboBox::setStyle()
{
	if (!isEnabled() || !usingOriginalValue)
	{
		setStyleSheet("");
		return;
	}

	if (originalValue != currentIndex())
		setStyleSheet("QComboBox{ background-color: rgb(255, 255, 127);\nborder-width: 1px;\nborder-style: solid;\nborder-color: rgb(152, 152, 152); }");
	else
		setStyleSheet("");
}





CustomStringComboBox::CustomStringComboBox(QWidget *parent)
	: QComboBox(parent)
	, usingOriginalValue(true)
{
	connect(this, SIGNAL(currentTextChanged(QString)), this, SLOT(checkValueChanged(QString)));
}

void CustomStringComboBox::setUsingOriginalValue(bool _flag)
{
	usingOriginalValue = _flag;
	setStyle();
}

void CustomStringComboBox::setOriginalValue(QString value)
{
	if (!usingOriginalValue) return;
	originalValue = value;
	setStyle();
}

QString CustomStringComboBox::getOriginalValue()
{
	return originalValue;
}

void CustomStringComboBox::setProfileValue(QString profileValue)
{
	if (objectName() == "comboBox_profileNameList")
	{
		if (this->findText(profileValue) == -1)
		{
			profileValue = Generals::unknownProfileName;
			if (this->findText(profileValue) == -1)
			{
				this->addItem(profileValue);
			}
		}
	}
	setCurrentText(profileValue);
}

void CustomStringComboBox::getProfileValue(QString *profileValue)
{
	if (objectName() == "comboBox_profileNameList")
	{
		if (currentText() == Generals::unknownProfileName)
		{
			return;
		}
	}
	*profileValue = currentText();
}

void CustomStringComboBox::resetValue()
{
	if (!usingOriginalValue) return;
	setCurrentText(originalValue);
}

void CustomStringComboBox::checkValueChanged(QString value)
{
	if (!usingOriginalValue) return;
	setStyle();
}

void CustomStringComboBox::setEnabled(bool _enabled)
{
	QComboBox::setEnabled(_enabled);
	if (!usingOriginalValue) return;
	setStyle();
}

void CustomStringComboBox::setStyle()
{
	if (!isEnabled() || !usingOriginalValue)
	{
		setStyleSheet("");
		return;
	}

	if (originalValue != currentText())
		setStyleSheet("QComboBox{ background-color: rgb(255, 255, 127);\nborder-width: 1px;\nborder-style: solid;\nborder-color: rgb(152, 152, 152); }");
	else
		setStyleSheet("");
}