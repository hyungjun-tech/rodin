#pragma once

class ModelMatchList
{
public:
	static QString getModel(QString tli_);
	static std::vector<std::pair<QString, QString>> getMatchList(QString model_ = "");
	static void setMatchList();
private:
	static std::vector<std::pair<QString, QString>> modelMatchList;
};
