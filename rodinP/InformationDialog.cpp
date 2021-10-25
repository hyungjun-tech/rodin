#include "stdafx.h"
#include "InformationDialog.h"

InformationDialog::InformationDialog(QWidget *parent)
	: QDialog(parent)
{
	//parentClass = dynamic_cast<rodinP*>(parent);
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));

	QString platform;
#if (x86_PLATFORM == 1)
	platform = " (x86 build)";
#elif (x64_PLATFORM == 1)
	platform = " (x64 build)";
#endif

	setContents(platform);
	exec();
}

InformationDialog::~InformationDialog()
{
}

void InformationDialog::setContents(QString platform)
{
	ui.label_app_name->setText(AppInfo::getAppName());

	QString strTmp;
	//strTmp.append(AppInfo::getAppName());
	strTmp.append("\n");
	strTmp.append(QString("Version : ") + AppInfo::getCurrentVersion() + platform);

	strTmp.append("\n\n");
	strTmp.append(QString("OpenGL Version : "));
	strTmp.append(QString((const char*)glGetString(GL_VERSION)));
	strTmp.append("\n\n");
	strTmp.append(AppInfo::getAppName() + " is developed by Sindoh Co., Ltd.");
	strTmp.append("\n");
	//hjkim add 
	strTmp.append(AppInfo::getAppName() + " uses the following open source projects:");
	//strTmp.append("2020 Sindoh Co., Ltd. All rights reserved.");
	ui.label->setText(strTmp);

	//hjkim add  2021.06.01
	ui.label_3->setOpenExternalLinks(true);
	ui.label_3->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	QString url3("<a href=\'" + QString("https://onsindoh-my.sharepoint.com/:f:/g/personal/onedrive1_sindoh_com/EngbUABHErxCjLvNXBxemZ0BXP3NUFv_T2IXVPRCClvV6g?e=Ib6shG") + "'>" + QString("3DWoxDesktop")) ;
	url3.append("<br>");
	url3.append ("<a href=\'" + QString("https://github.com/Ultimaker/CuraEngine/releases/tag/15.01") + "'>" + QString("CuraEngine"));
	url3.append("<br>");
	url3.append("<a href=\'" + QString("https://download.qt.io/archive/qt/5.9/5.9.8/") + "'>" + QString("Qt"));
	url3.append("<br>");
	url3.append("<a href=\'" + QString("https://github.com/GillesDebunne/libQGLViewer/releases/tag/v2.6.1") + "'>" + QString("LibQGLViewer"));
	url3.append("<br>");
	url3.append("<a href=\'" + QString("https://github.com/CGAL/cgal/releases/tag/releases%2FCGAL-4.14.3") + "'>" + QString("CGAL"));
	url3.append("<br>");
	url3.append("<a href=\'" + QString("https://github.com/assimp/assimp/releases/tag/v5.0.0") + "'>" + QString("Assimp"));
	url3.append("<br>");
	url3.append("<a href=\'" + QString("https://github.com/3MFConsortium/lib3mf/releases/tag/v2.0.0") + "'>" + QString("Lib3mf"));
	url3.append("<br>");
	url3.append("<a href=\'" + QString("https://www.graphics.rwth-aachen.de/software/openmesh/download/") + "'>" + QString("OpenMesh"));
	url3.append("<br>");
	url3.append("<a href=\'" + QString("https://www.cryptopp.com/release820.html") + "'>" + QString("Cryptopp"));
	url3.append("<br>");
	url3.append("<a href=\'" + QString("http://www.angusj.com/delphi/") + "'>" + QString("Clipper"));

	ui.label_3->setText(url3);



	QString label4("3DWox Desktop\n");
	label4.append("G-code Generator\n");
	label4.append("Graphical User Interface\n");
	label4.append("3D Graphical User Interface\n");
	label4.append("Mesh Data Handling\n");
	label4.append("Import Export stl / obj / ply\n");
	label4.append("Import Export 3mf\n");
	label4.append("Mesh Data Handling\n");
	label4.append("Data Encryption\n");
	label4.append("Polygon Data Handling");
	ui.label_4->setText(label4);

	QString label5("AGPL\n");
	label5.append("AGPL\n");
	label5.append("(L)GPL\n");
	label5.append("GPL\n");
	label5.append("GPL\n");
	label5.append("BSD\n");
	label5.append("BSD\n");
	label5.append("BSD\n");
	label5.append("Boost Software License\n");
	label5.append("Boost Software License");
	ui.label_5->setText(label5); 
	//hjkim add end

	ui.label_2->setOpenExternalLinks(true);
	ui.label_2->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	QString url("<a href=\'" + AppInfo::getMainURL(Profile::machineProfile.company_code.value) + "'>" + AppInfo::getMainURL(Profile::machineProfile.company_code.value) + "</a>");
	ui.label_2->setText(url);

}