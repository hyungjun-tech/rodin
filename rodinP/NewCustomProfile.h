#pragma once

#include "ui_NewCustomProfile.h"
#include "CustomProfileList.h"

class NewCustomProfile : public QDialog
{
	Q_OBJECT

public:
	NewCustomProfile(QWidget *parent = 0);
	~NewCustomProfile();

public slots:
	void saveNewProfile();

private slots:
	bool setText(QString text);
	void setTextLength();
	void addPro_comboBox_currentIndexChanged(int);

private:
	Ui::NewCustomProfile ui;

	QString profile_name;
	QString file_name; //=new custom profile name
	QString description;
	QString newProfileLocation;
	QString rtrimInPlace(QString &str);
	void setNewUI();

	//bool generateCustomProfile(); ÇÔ¼ö¸í ¹Ù²Þ
	bool checkValidName(QString fileName);
	bool AddToXmlList();
	bool generateProfileFile(QString profile_locate);
	bool ExportedProfile();
	
	std::vector<Generals::ProfileList> entireProfileList;

	QString importProfileName;
};