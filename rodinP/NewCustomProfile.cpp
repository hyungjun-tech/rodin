#include "stdafx.h"
#include "NewCustomProfile.h"
#include "CustomProfileList.h"
#include "CustomProfileEditor.h"
#include "SliceProfile.h"

NewCustomProfile::NewCustomProfile(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);

	setNewUI();
	
	connect(ui.addPro_cancel, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.addPro_OK, SIGNAL(clicked()), this, SLOT(saveNewProfile()));
	connect(ui.addPro_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(setText(QString)));
	connect(ui.addPro_plainTextEdit, SIGNAL(textChanged()), this, SLOT(setTextLength()));
	connect(ui.addPro_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(addPro_comboBox_currentIndexChanged(int)));
}

NewCustomProfile::~NewCustomProfile()
{

}

void NewCustomProfile::setTextLength()
{
	if (ui.addPro_plainTextEdit->toPlainText().length() > 300)
	{
		QString text = ui.addPro_plainTextEdit->toPlainText();

		text.chop(text.length() - 300); // Cut off at 300 characters
		ui.addPro_plainTextEdit->setPlainText(text); // Reset text

		// This code just resets the cursor back to the end position
		// If you don't use this, it moves back to the beginning.
		// This is helpful for really long text edits where you might
		// lose your place.
		QTextCursor cursor = ui.addPro_plainTextEdit->textCursor();
		cursor.setPosition(ui.addPro_plainTextEdit->document()->characterCount() - 1);
		ui.addPro_plainTextEdit->setTextCursor(cursor);

		// This is your "action" to alert the user.
		CommonDialog comDlg(this, MessageAlert::tr("custom_profile_description_too_long"), CommonDialog::Warning);
		/*QMessageBox::critical(this,
			"Error",
			"Please be sure that you keep the description under 300 characters.");*/
	}

}

//Replacing certain characters and Cutting off at 30 characters
bool NewCustomProfile::setText(QString text)
{

	QStringList list{ "<", ">", "|", "\\", "/", "\"", "*", "?", "[", "]", ":" }; //"\"" ->  " ����

	for (const auto& character : list)
	{
		if (text.contains(character, Qt::CaseInsensitive) == true)
		{
			CommonDialog comDlg(this, MessageAlert::tr("custom_profile_name_cannot_contain_special_char"), CommonDialog::Warning);
			ui.addPro_lineEdit->backspace(); //Ư������ ����
			return false;
		}
	}

	//�̸�(=���ϸ�) 30�� ����
	if (ui.addPro_lineEdit->text().length() > 30)
	{
		text.chop(text.length() - 30); // Cut off at 30 characters
		ui.addPro_lineEdit->setText(text); // Reset text

		// This is "action" to alert the user.
		CommonDialog comDlg(this, MessageAlert::tr("custom_profile_name_too_long"), CommonDialog::Warning);
		return false;
	}

	return true;
}

void NewCustomProfile::setNewUI()
{
	ui.addPro_comboBox->clear();
	CustomProfileList customprofile;
	entireProfileList = customprofile.getProfileList(true);
	int i = 0;
	for (std::vector<Generals::ProfileList>::iterator it = entireProfileList.begin(); it != entireProfileList.end(); ++it)
	{
		ui.addPro_comboBox->addItem(entireProfileList.at(i).name);
		i++;
	}
	//ui.comboBox_newMaterial->addItems(Profile::machineProfile.available_material_list);
	ui.addPro_comboBox->addItem(tr("Exported Profile"));

	//'Name'�� �ʼ� �Է¶��̱� ������ Ŀ�� �߻�
	ui.addPro_lineEdit->setFocus(Qt::OtherFocusReason);
}

void NewCustomProfile::addPro_comboBox_currentIndexChanged(int idx)
{
	if (idx == entireProfileList.size())
	{
		ExportedProfile();
	}
}

bool NewCustomProfile::ExportedProfile()
{
	char szFilter[] = "Profile Information File (*.ini)";

	importProfileName = QFileDialog::getOpenFileName(this, tr("Select profile"), "", szFilter);

	if (!(importProfileName.size())){
		//������ importProfileName ���� ���ϸ�, combobox ù��° ������ ���� �ǰ� �ϱ�
		ui.addPro_comboBox->setCurrentIndex(0);
		return false;
	}
	return true;
}

//�������� '�̸�'�� ���Ͽ� ���� ���� ����
QString NewCustomProfile::rtrimInPlace(QString &str) {
	for (int n = str.size() - 1; n >= 0; --n)
		if (!str.at(n).isSpace()) {
			str.truncate(n + 1);
			break;
		}
	return str;
}

void NewCustomProfile::saveNewProfile()
{
	//xml,���Ͽ� ���� �� ����
	file_name = ui.addPro_lineEdit->text();
	description = ui.addPro_plainTextEdit->toPlainText();
	profile_name = ui.addPro_comboBox->currentText();

	file_name =  rtrimInPlace(file_name); //���� ���� ����
	file_name = file_name.remove(QRegExp("^[ ]*")); //���� ���� ����

	//�Է��׸� �Էµ� �� ��ȿ�� üũ 
	//���������� ����,���� ���� ���� ���¿��� �ߺ� üũ
	if (!checkValidName(file_name))
	{
		ui.addPro_lineEdit->setText(file_name);
		return;
	}

	QString absolutePath;
	///////���� ���õ� combobox ��ü size �� ���ٸ� (= Exported Profile), importProfileName �� ��� ����
	if (ui.addPro_comboBox->currentIndex() == entireProfileList.size())
	{
		absolutePath = importProfileName; //���� ���
	} else
	{
		QString profile_locate = entireProfileList.at(ui.addPro_comboBox->currentIndex()).fileLocation;
		QString dirPath;
		//����θ� ��� �� �� 
		if (entireProfileList.at(ui.addPro_comboBox->currentIndex()).custom)
			dirPath = Profile::customProfilePath;
		else
			dirPath = Profile::profilePath;

		absolutePath = dirPath + profile_locate;
	}

	//generateProfileFile �Լ��� ���� ��η� QString ������
	if (!generateProfileFile(absolutePath))
	{
		return;
	}

	if (!AddToXmlList())//xml�߰�
	{
		//CommonDialog comDlg(this, tr("An error occurred while adding the profile list."), CommonDialog::Warning);
		return;
	}

	close();

	CustomProfileEditor * profileEditor = new CustomProfileEditor(this);
	profileEditor->loadProfile(newProfileLocation, file_name);
	profileEditor->exec();

}

bool NewCustomProfile::checkValidName(QString fileName)
{
	if (fileName.trimmed().isEmpty())
	{
		CommonDialog comDlg(this, MessageAlert::tr("custom_profile_name_is_essential"), CommonDialog::Warning);
		return false;
	}

	CustomProfileList customprofile;
	std::vector<Generals::ProfileList> allList;	
	allList = customprofile.getProfileList(true);

	int i = 0;

	for (const auto &j : allList) {
		if (fileName == allList.at(i).name){
			CommonDialog comDlg(this, MessageAlert::tr("custom_profile_name_is_already_in_use"), CommonDialog::Warning);
			return false;
		}
		i++;
	}

	return true;
}

bool NewCustomProfile::AddToXmlList()
{
	//�����/AppData/Local/rodinP�� ��⺰ (custom) ����� profile_list.xml
	QString CustomfilePath = Profile::customProfilePath + "profile_list.xml";
	QFile* xmlFile = new QFile(CustomfilePath);

	////////// Open file
	if (!xmlFile->open(QIODevice::ReadOnly)) {
		CommonDialog comDlg(this, MessageAlert::tr("fail_to_import_custom_profile_list"), CommonDialog::Warning);
		return false;
	}

	////////// Parse file
	QDomDocument doc("");
	if (!doc.setContent(xmlFile))
	{
		CommonDialog comDlg(this, MessageAlert::tr("fail_to_import_custom_profile_list"), CommonDialog::Warning);
		return false;
	}

	xmlFile->close();

	////////// Modify content
	QDomElement docElem = doc.documentElement();

	QDomNodeList childList = docElem.elementsByTagName("custom");
	QDomElement customElement;
	if (childList.isEmpty())
	{
		customElement = doc.createElement("custom");
		docElem.appendChild(customElement);
	}
	else
		customElement = childList.at(0).toElement();

	QDomElement profileHeader = doc.createElement("profile");

	QDomElement tagName = doc.createElement("name");
	profileHeader.appendChild(tagName);
	QDomText textName = doc.createTextNode(file_name);
	tagName.appendChild(textName);

	QDomElement tagDescription = doc.createElement("description");
	profileHeader.appendChild(tagDescription);
	//if (description.trimmed().isEmpty()) { description = " ";  } edit �� ���װ� �ִ� ���
	QDomText textDescription = doc.createTextNode(description);
	tagDescription.appendChild(textDescription);

	QDomElement tagFileLocation = doc.createElement("fileLocation");
	profileHeader.appendChild(tagFileLocation);
	QDomText textFileLocation = doc.createTextNode(newProfileLocation);
	tagFileLocation.appendChild(textFileLocation);

	customElement.appendChild(profileHeader);

	////////// Save content back to the file
	// QIODevice::Truncate  = ���� content ����
	if (!xmlFile->open(QIODevice::Truncate | QIODevice::WriteOnly)) {
		CommonDialog comDlg(this, MessageAlert::tr("fail_to_save_custom_profile_list"), CommonDialog::Warning);
		return false;
	}

	QByteArray xml = doc.toByteArray();
	xmlFile->write(xml);
	xmlFile->close();

	return true;
}

bool NewCustomProfile::generateProfileFile(QString profile_locate)
{
	//����ڰ� ������ �������� ini ������ �ִ��� ���� üũ
	QFileInfo check_file(profile_locate);
	// check if file exists and if yes: Is it really a file and no directory?
	if (!check_file.exists() && check_file.isFile()) 
	{
		CommonDialog comDlg(this, MessageAlert::tr("fail_to_find_custom_profile_info"), CommonDialog::Warning);
		return false;
	}

	//Check if there is a folder in "custom" path
	/*QDir dir(Profile::profilePath + "custom"); 
	if (!dir.exists()) {
		dir.mkpath(Profile::profilePath + "custom");
	}*/

	//custom ������ ���� ���� ������ �����Ѵٸ� ���� ���� ���� ��,
	QString filePath = Profile::customProfilePath + "custom/" + file_name + ".ini";
	if (true == QFile::exists(filePath)) QFile::remove(filePath);

	if (!QFile::copy(profile_locate, Profile::customProfilePath + file_name + ".ini"))
	{
		CommonDialog comDlg(this, MessageAlert::tr("fail_to_create_new_custom_profile"), CommonDialog::Warning);
		return false;
	}

	newProfileLocation = file_name + ".ini";
	return true;
}