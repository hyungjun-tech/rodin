#pragma once
#include <QFile>
#include <QIODevice>
#include <QString>
#include <QTextStream>
#include <QDateTime>
#include <qdebug>
class Logger
{
public:
	Logger() {}
	~Logger()
	{
		QFile *logFile = new QFile(logFileName);
		if (!logFile->open(QIODevice::Append | QIODevice::Text))
			return;
		QTextStream logStream(logFile);
		logStream.setCodec("UTF-8");
		logStream << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ") << comment << "\n";
		logFile->close();
	}

	template<typename T>
	Logger& operator<<(T value)
	{
		write(value);
		return *this;
	}
	Logger& operator<<(std::string value)
	{
		write(QString::fromStdString(value));
		return *this;
	}

	template<typename T>
	Logger& operator<(T value)
	{
		if (UserProperties::detailedLogMode)
			write(value);
		return *this;
	}

	template<typename T>
	void write(const T value)
	{
		QTextStream commentStream(&comment);
		commentStream << value;
	}



	static void setLogFileName(bool deleteFlag = false);
	static QString logFileName;
private:
	QString comment;
};