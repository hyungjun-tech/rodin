#pragma once

#include "ui_CustomProfileList.h"
#include "NewCustomProfile.h"

class CustomProfileList : public QDialog
{
	Q_OBJECT

		//�������� ����
	struct greater 
	{
		template<class T>
		bool operator()(T const &a, T const &b) const { return a > b; }
	};

public:
	CustomProfileList(QWidget *parent);
	CustomProfileList();
	~CustomProfileList();

	bool enabledApplyBtn = false;

	//rodinP *m_parent;
	std::vector<Generals::ProfileList> ProfileXml;
	std::vector<Generals::ProfileList> getProfileList(bool withStandard = true, bool advancedMode = false);
	void showXmlList();
	static void copyCustomProfileFiles();

public slots:
	void viewAddProfileList();
	void saveProfileList();

	void listBtn_delete_clicked();
	void listBtn_edit_clicked();
	void listBtn_OK_clicked();
	void setEnabledApplyButton(bool enabledApplyButton);
	bool onDataChanged(int row, int column);
	void resetProfileList();

private slots:
	void blockColumn(int, int);

private:
	Ui::CustomProfileList ui;

	QString standradFilePath;
	QString customFilePath;
	//QDomElement docElem;
	QStringList m_TableHeader;
	QFile* cusXmlFile;
	QDomDocument m_domDoc; 

	void setUI();
	static std::vector<Generals::ProfileList>  getProfileList(QDomElement pDomElem, bool withStandard = true, bool advancedMode = false);
	// �����ؾ� �� ���� ��θ� �����ص״ٰ� OK��ư ������ �� ���� �����ؾ�
	QStringList deletePathList;
	void deleteProfile(std::vector<int>);

	bool checkCustomPath();
	bool deleteFile(int i);
	bool deleteAction = false;
	bool deleteToNewAction = false;

};