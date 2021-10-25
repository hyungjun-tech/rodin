#include "stdafx.h"
#include "Logger.h"

QString Logger::logFileName = QString();
void Logger::setLogFileName(bool deleteFlag)
{
	QDateTime dt = QDateTime::currentDateTime();
	QString logDate = dt.date().toString("yyyyMMdd");
	QString logPath = Generals::getTempFolderPath() + "/log";
	Generals::checkPath(logPath);
	logFileName = logPath + "/" + logDate + ".log";

	if (deleteFlag)
	{
		QStringList allFiles = QDir(logPath).entryList(QDir::NoDotAndDotDot | QDir::Files);
		//오늘 log를 제외하고 삭제
		for (int i = 0; i < allFiles.size(); i++)
		{
			if (!allFiles.at(i).startsWith(logDate))
				QFile::remove(logPath + "/" + allFiles.at(i));
		}
	}
}