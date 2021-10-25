#pragma once

class rodinP;
class FindPrinterInfo : public QThread
{
	Q_OBJECT

public:
	FindPrinterInfo();
	~FindPrinterInfo();

	void run();
signals :
	void signal_filamentInfoUpdated();
};