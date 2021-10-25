#include "stdafx.h"
#include "CustomProfileList.h"
#include "CustomProfileEditor.h"
//#include "rodinP.h"

CustomProfileList::CustomProfileList(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setEnabledApplyButton(false);

	setUI();
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);

	checkCustomPath(); //appData-Local�� xml ���� ��� �ִ���
	ProfileXml = getProfileList(false); //��⸦ �о�� Profile XML List �� �����´�
	showXmlList();

	connect(ui.listBtn_new, SIGNAL(clicked(bool)), this, SLOT(viewAddProfileList()));
	connect(ui.listBtn_delete, SIGNAL(clicked(bool)), this, SLOT(listBtn_delete_clicked()));
	connect(ui.listBtn_edit, SIGNAL(clicked(bool)), this, SLOT(listBtn_edit_clicked()));
	connect(ui.listBtn_OK, SIGNAL(clicked(bool)), this, SLOT(listBtn_OK_clicked()));
	connect(ui.listBtn_apply, SIGNAL(clicked(bool)), this, SLOT(saveProfileList()));
	connect(ui.profile_list, SIGNAL(cellChanged(int, int)), this, SLOT(onDataChanged(int, int)));
	connect(ui.profile_list, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(blockColumn(int, int)));
}

CustomProfileList::CustomProfileList()
{
	standradFilePath = Profile::profilePath + "profile_list.xml";
	customFilePath = Profile::customProfilePath + "profile_list.xml";
}

CustomProfileList::~CustomProfileList()
{

}

void CustomProfileList::setUI()
{
	ui.profile_list->setColumnCount(2);

	//m_TableHeader << "Name" << "Description";
	//ui.profile_list->setHorizontalHeaderLabels(m_TableHeader);
	ui.profile_list->verticalHeader()->setVisible(false);
	ui.profile_list->setShowGrid(false); //true
	ui.profile_list->setSelectionBehavior(QAbstractItemView::SelectRows);

	ui.profile_list->setColumnWidth(0, 200);
	ui.profile_list->setColumnWidth(1, 300);
}

bool CustomProfileList::checkCustomPath()
{ 
	//�����/AppData/Local/rodinP �� ��⺰ (custom)��ΰ� �ִ��� Ȯ��
	QString createCustomfilePath = Profile::customProfilePath + "profile_list.xml";

	//Check if there is a folder in path and Create a folder 
	Generals::checkPath(Profile::customProfilePath);

	/////// creat file in local path
	if (!QFileInfo::exists(createCustomfilePath))
	{
		QDomDocument doc;
		//������ profile_list.xml ���Ͽ� �� ����
		QDomProcessingInstruction process = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
		doc.appendChild(process);

		QDomElement documentElement = doc.documentElement();
		doc.appendChild(doc.createElement("profiles"));

		QDomElement docElem = doc.documentElement();
		QDomElement customElement = doc.createElement("custom");
		docElem.appendChild(customElement);

		QFile outFile(createCustomfilePath);
		if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			qDebug("Failed to open file for writing.");
			return false;
		}

		QTextStream stream(&outFile);
		stream << doc.toString();

		outFile.close();
	}

	standradFilePath = Profile::profilePath + "profile_list.xml";
	customFilePath = createCustomfilePath;
	return true;
}

std::vector<Generals::ProfileList> CustomProfileList::getProfileList(bool withStandard, bool advancedMode)
{
	//QString appPath = QCoreApplication::applicationDirPath();
	//QString model = Profile::machineProfile.machine_model;
	std::vector<Generals::ProfileList> l_standardProfileXml;

	if (withStandard)
	{
		QFile *standardFile = new QFile(standradFilePath);

		if (!standardFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
			return l_standardProfileXml;
		}
		QDomDocument l_domDoc;
		//QDomDocument domDoc("");
		if (!l_domDoc.setContent(standardFile))
		{
			standardFile->close();
			return l_standardProfileXml;
		}

		QDomElement l_docElem = l_domDoc.documentElement();

		l_standardProfileXml = getProfileList(l_docElem, withStandard, advancedMode);
		standardFile->close();
	}


	std::vector<Generals::ProfileList> l_profileXml;

	//standardXmlFile = new QFile(standradFilePath);
	cusXmlFile = new QFile(customFilePath);
	
	if (!cusXmlFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
		return l_standardProfileXml;
	}
	if (!m_domDoc.setContent(cusXmlFile))
	{
		cusXmlFile->close();
		return l_standardProfileXml;
	}
	QDomElement docElem = m_domDoc.documentElement();

	l_profileXml = getProfileList(docElem, withStandard, advancedMode);

	cusXmlFile->close();


	l_standardProfileXml.insert(l_standardProfileXml.end(), l_profileXml.begin(), l_profileXml.end());

	return l_standardProfileXml;
}

std::vector<Generals::ProfileList> CustomProfileList::getProfileList(QDomElement pDomElem, bool withStandard, bool advancedMode)
{
	
	std::vector<Generals::ProfileList> l_profileXml;
	//standradFilePath = Profile::profilePath + "profile_list.xml";
	//customFilePath = Profile::customProfilePath + "profile_list.xml";

	QDomNodeList childList = pDomElem.childNodes();
	for (int i = 0; i < childList.size(); i++)
	{
		if (childList.at(i).nodeName() == "standard" && withStandard)
		{
			QDomNodeList nodeList = childList.at(i).toElement().elementsByTagName("profile");
			for (int j = 0; j < nodeList.count(); j++)
			{
				Generals::ProfileList profileList;
				QDomElement el = nodeList.at(j).toElement();
				QDomNode pEntries = el.firstChild();
				profileList.custom = false;
				while (!pEntries.isNull()) {
					QDomElement peData = pEntries.toElement();
					QString tagNam = peData.tagName();
					if (tagNam == "name") profileList.name = peData.text();
					else if (tagNam == "description") profileList.description = peData.text();
					else if (tagNam == "fileLocation") profileList.fileLocation = peData.text();
					else if (tagNam == "visible") profileList.visible = peData.text();
					//qDebug() << "profileList.visible : " << profileList.visible;
					pEntries = pEntries.nextSibling();
				}
				//advancede mode �� ���� etc profile�� �������� �ȵȴ�
				if ( advancedMode == false)
				{					
					l_profileXml.push_back(profileList); 
				}
				else
				{
					if (profileList.visible != "N") { l_profileXml.push_back(profileList); }
				} //if profileMultiDlg.multiDlg end	
			}
		}
		else if (childList.at(i).nodeName() == "custom")
		{
			QDomNodeList nodeList = childList.at(i).toElement().elementsByTagName("profile");
			for (int j = 0; j < nodeList.count(); j++)
			{
				Generals::ProfileList profileList;
				QDomElement el = nodeList.at(j).toElement();
				QDomNode pEntries = el.firstChild();
				profileList.custom = true;
				while (!pEntries.isNull()) {
					QDomElement peData = pEntries.toElement();
					QString tagNam = peData.tagName();
					if (tagNam == "name") profileList.name = peData.text();
					else if (tagNam == "description") profileList.description = peData.text();
					else if (tagNam == "fileLocation") profileList.fileLocation = peData.text();
					pEntries = pEntries.nextSibling();
				}
				l_profileXml.push_back(profileList);
			}
		}
	}

	return l_profileXml;
}

void CustomProfileList::deleteProfile(std::vector<int> pIdxList)
{
	//�������� ����
	sort(pIdxList.begin(), pIdxList.end(), greater());

	QDomElement docElem = m_domDoc.documentElement();
	QDomNodeList nodeList = docElem.elementsByTagName("custom");
	if (!nodeList.isEmpty())
	{
		QDomElement childElem = nodeList.at(0).toElement();
		QDomNodeList childList = childElem.childNodes();
		for (int i = 0; i < pIdxList.size(); i++)
		{
			int l_idx = pIdxList.at(i);
			//QString fildLoc = childList.at(l_idx).toElement().elementsByTagName("fileLocation").at(0).toElement().text();
				//file �����ؾ� ��. ���� ���� ���� �� removeChild ����
			deleteFile(l_idx);
			childElem.removeChild(childList.at(l_idx));
		}
	}

	ProfileXml = getProfileList(docElem, false);
	setEnabledApplyButton(false);
	showXmlList();
}

bool CustomProfileList::deleteFile(int l_idx)
{
	QString strFileLocation;
		
	QDomElement docElem = m_domDoc.documentElement();
	QDomNodeList nodeList = docElem.elementsByTagName("custom");
	QDomElement childElem = nodeList.at(0).toElement();
	QDomNodeList childList = childElem.childNodes();
	strFileLocation = childList.at(l_idx).firstChildElement("fileLocation").text();

	qDebug() << "strFileLocation:" << QString(strFileLocation);
	qDebug() << "File Location for delete:" << QString(Profile::profilePath + strFileLocation);

	QString pathForDelete = Profile::customProfilePath + strFileLocation;
	//pathForDelete�� customProfilePath(root path)��� ���� ��ο� ���� value�� �߸� �� ���̴� �����ش�.
	if (pathForDelete != Profile::customProfilePath)
	{  
		deletePathList.append(pathForDelete);
	}  

	return true;
}

void CustomProfileList::showXmlList()
{
	deleteAction = true;
	int i = 0;
	ui.profile_list->clearContents();
	ui.profile_list->setRowCount(0);
	for (std::vector<Generals::ProfileList>::iterator it = ProfileXml.begin(); it != ProfileXml.end(); ++it)
	{
		ui.profile_list->insertRow(i);
		QString set_name = ProfileXml.at(i).name;
		QString set_description = ProfileXml.at(i).description;
		//tableWidget->setItem(row, column, newItem);
		ui.profile_list->setItem(i, 0, new QTableWidgetItem(set_name));
		ui.profile_list->setItem(i, 1, new QTableWidgetItem(set_description));
		i++;
	}
	deleteAction = false;
}

void CustomProfileList::saveProfileList()
{
	//��� �����ش��� Custom �� .ini ���� ����
	for (const auto& deletePath : deletePathList)
	{
		if (!QFile::remove(deletePath))
			CommonDialog comDlg(this, MessageAlert::tr("custom_profile_file_deletion_failed") + "\n" + deletePath, CommonDialog::Warning);
	}

	deletePathList.clear();

	//xml ���Ͽ� ������� ����
	if (!cusXmlFile->open(QIODevice::Truncate | QIODevice::WriteOnly)) {
		return;
	}
	QByteArray xml = m_domDoc.toByteArray();
	cusXmlFile->write(xml);
	cusXmlFile->close();

	setEnabledApplyButton(false);
}

void CustomProfileList::blockColumn(int row, int column) //QTableWidgetItem *item
{
	//name �� ���� ����
	QTableWidgetItem *item = new QTableWidgetItem;
	item = ui.profile_list->item(row, column);

	if (column != 1)
		ui.profile_list->item(row, column)->setFlags(ui.profile_list->item(row, column)->flags() & ~Qt::ItemIsEditable);
}

bool CustomProfileList::onDataChanged(int row, int column) //QTableWidgetItem *item
{
	if (deleteAction == false)
	{ 
	//XML profile ���� ����
	//onDataChanged() emited, use pointer item to get row/column
	QString changedValues = ui.profile_list->item(row, column)->text();
	qDebug() << "changedValues : " << changedValues;

	if (column == 1 && changedValues.length() > 300)
	{
		changedValues.chop(changedValues.length() - 300); // Cut off at 300 characters
		ui.profile_list->item(row, column)->setText(changedValues); // Reset text

		// This is your "action" to alert the user.
		CommonDialog comDlg(this, MessageAlert::tr("custom_profile_description_too_long"), CommonDialog::Warning);
	}

	///////// Modify content
	QDomElement docElem = m_domDoc.documentElement();
	QDomNodeList nodeList = docElem.elementsByTagName("custom");
	QDomElement childElem = nodeList.at(0).toElement();
	QDomNodeList childList = childElem.childNodes();
	QDomElement nodeTag = childList.at(row).firstChildElement("description");
	qDebug() << "nodeTag : " << nodeTag.tagName();
	QDomText nodeValue = nodeTag.firstChild().toText();

		qDebug() << "before data : " << nodeValue.data();

		if (nodeValue.data().trimmed() == NULL || nodeValue.data().trimmed() == "")
		{
			//Description �� null�̳� ������ ��
			QDomText newNodeText = m_domDoc.createTextNode(QString(changedValues));
			nodeTag.appendChild(newNodeText);
			qDebug() << "after data : " << newNodeText.data();
		} 
		else
		{
			//������ node Value �� ������ ��
			nodeValue.setData(changedValues);
			qDebug() << "after data : " << nodeValue.data();
		
		}
		
	ProfileXml = getProfileList(docElem, false);
	setEnabledApplyButton(true);

	} //if (deleteAction == false) end

	return true;
}

void CustomProfileList::viewAddProfileList()
{
	//delete �� add profile �� ������ ���� ���¸� ����
	if (deleteToNewAction == true) saveProfileList();
	deleteToNewAction = false;

	NewCustomProfile *NewCusProfile = new NewCustomProfile(this);
	connect(NewCusProfile, SIGNAL(destroyed(QObject *)), this, SLOT(resetProfileList()));
	NewCusProfile->show();
	//deleteToNewAction = false;
}

void CustomProfileList::setEnabledApplyButton(bool enabledApplyButton)
{
	enabledApplyBtn = enabledApplyButton;

	ui.listBtn_apply->setEnabled(enabledApplyButton);
}

void CustomProfileList::listBtn_delete_clicked()
{
	QModelIndexList selection = ui.profile_list->selectionModel()->selectedRows();
	if (selection.size() == 0)
	{
		qDebug() << "no selected row";
		return;
	}

	deleteToNewAction = true;

	std::vector<int> l_profileNoList;
	for (int i = 0; i< selection.count(); i++)
	{
		QModelIndex index = selection.at(i);
		l_profileNoList.push_back(index.row());
	}
	deleteProfile(l_profileNoList);
	setEnabledApplyButton(true);
}

void CustomProfileList::listBtn_edit_clicked()
{
	QModelIndexList selection = ui.profile_list->selectionModel()->selectedRows();
	if (selection.size() == 0)
	{
		qDebug() << "no selected row";
		CommonDialog comDlg(this, MessageAlert::tr("custom_profile_no_selected"), CommonDialog::Warning);
		return;
	}
	if (selection.size() > 1) //2�� �̻� ���ý� ����
	{	
		qDebug() << "too many selected rows : " << selection.size();
		//�� ���� �� ���� �������ϸ� ������ �����մϴ�.
		CommonDialog comDlg(this, MessageAlert::tr("custom_profile_select_multi_rows"), CommonDialog::Warning);
		return;
	}

	int row = selection.at(0).row();
	enabledApplyBtn = true;

	std::vector<Generals::ProfileList> vectorGetCustom;


	QDomElement docElem = m_domDoc.documentElement();
	vectorGetCustom = getProfileList(docElem, false);
	//vectorGetCustom = getProfileList(false);

	qDebug() << vectorGetCustom.at(row).fileLocation << " - " << row;
	CustomProfileEditor * profileEditor = new CustomProfileEditor(this);

	QString RelativePath = vectorGetCustom.at(row).fileLocation; 
	profileEditor->loadProfile(RelativePath, vectorGetCustom.at(row).name);
	profileEditor->show();

}


void CustomProfileList::listBtn_OK_clicked()
{
	saveProfileList();
	close();
}

void CustomProfileList::resetProfileList()
{
	ProfileXml = getProfileList(false); //��⸦ �о�� Profile XML List �� �����´�
	showXmlList();
}

void CustomProfileList::copyCustomProfileFiles()
{
	QString customPathString = Generals::appDataPath + "/customProfile";
	QDir customPath(customPathString);
	if (customPath.exists())
		return;
	else
	{
		Generals::checkPath(customPathString);
		QString profilePathString = Generals::appDataPath + "/profile";
		QDir profilePath(profilePathString);
		QStringList dirs = profilePath.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (int i = 0; i < dirs.size(); i++)
		{
			std::vector<Generals::ProfileList> l_profileXml;
			QFile cusXmlFile(profilePathString + "/" + dirs.at(i) + "/profile_list.xml");
			if (!cusXmlFile.exists())
				continue;
			if (!cusXmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
				continue;
			QDomDocument l_domDoc;
			if (!l_domDoc.setContent(&cusXmlFile))
			{
				cusXmlFile.close();
				continue;
			}

			QDomElement docElem = l_domDoc.documentElement();
			l_profileXml = getProfileList(docElem, false);
			cusXmlFile.close();

			if (l_profileXml.size() != 0)
			{
				Generals::checkPath(customPathString + "/" + dirs.at(i));
				cusXmlFile.rename(customPathString + "/" + dirs.at(i) + "/profile_list.xml");
			}

			for (int j = 0; j < l_profileXml.size(); j++)
			{
				QFile customFile(profilePathString + "/" + dirs.at(i) + "/" + l_profileXml.at(j).fileLocation);
				qDebug() << l_profileXml.at(j).fileLocation;
				if (customFile.exists())
				{
					customFile.rename(customPathString + "/" + dirs.at(i) + "/" + l_profileXml.at(j).fileLocation);
				}
			}
		}
	}
}