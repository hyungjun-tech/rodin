#pragma once

class SearchNetworkPrinter :
	public QThread
{
	Q_OBJECT
public:
	SearchNetworkPrinter(int myNum);
	void run();
	//QList<QString> getSnmpValue(QString ip, QString community, QList<QString> oids);

	bool Stop;
	QHostAddress ip;
	QList<QHostAddress> targetIPs;
private:
	int num;
	QList<int> CNums;
	QString maker = "SINDOH";
	QString category = "3D Machine";
signals:
	void signal_findPrinter(QString, QString, QString);
	void signal_finishSearch();
};


class SearchUSBPrinter :
	public QThread
{
	Q_OBJECT
public:
	SearchUSBPrinter();
	void run();
signals:
	void signal_findPrinter(QString, QString, QString);
	void signal_finishSearch();
};