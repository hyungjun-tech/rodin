#include "stdafx.h"
#include "ModelMatchList.h"

std::vector<std::pair<QString, QString>> ModelMatchList::modelMatchList = std::vector<std::pair<QString, QString>>{};

QString ModelMatchList::getModel(QString tli_)
{
	if (modelMatchList.size() == 0)
		setMatchList();
	if (tli_ == "")
		return "No Model";
	else
	{
		auto it = std::find_if(modelMatchList.begin(), modelMatchList.end(),
			[&tli_](const std::pair<QString, QString>& element) { return element.second == tli_; });
		if (it == modelMatchList.end())
			return "No Model";
		return (*it).first;
		/*QString rtn;
		std::vector<std::pair<QString, QString>> temp;
		for (auto modelList : modelMatchList)
		{
			if (modelList.second == tli_)
				return modelList.first;
		}*/
	}
}

std::vector<std::pair<QString, QString>> ModelMatchList::getMatchList(QString model_)
{
	if (modelMatchList.size() == 0)
		setMatchList();
	if (model_ == "")
		return modelMatchList;
	else
	{
		std::vector<std::pair<QString, QString>> temp;
		for (auto modelList : modelMatchList)
		{
			if (modelList.first == model_)
				temp.push_back(modelList);
		}
		return temp;
	}
}

void ModelMatchList::setMatchList()
{
	QDir d = QDir(Generals::appPath + "\\profile");
	QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < dirs.size(); i++)
	{
		QFile inputFile(d.absolutePath() + "\\" + dirs.at(i) + "\\list.ini");
		if (inputFile.open(QIODevice::ReadOnly))
		{
			QTextStream in(&inputFile);
			while (!in.atEnd())
			{
				QString tli = in.readLine();
				if (tli != "")
					modelMatchList.push_back(std::make_pair(dirs[i], tli));
			}
			inputFile.close();
		}
	}
}