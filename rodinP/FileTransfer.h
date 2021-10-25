#pragma once
#include <QtNetwork/QTcpSocket>
#include "Communication.h"

class FileTransfer : public QObject
{
	Q_OBJECT
public:
	FileTransfer();
	~FileTransfer();
	virtual bool createConnection(QString IPorSerial_) = 0;
	virtual bool transferFile(QString fileName, QString ip) = 0;
	virtual bool transferFile(QString fileName) = 0;
	virtual	bool transferData(QByteArray data_) = 0;
	virtual bool sendPassword(QString password_) = 0;
	virtual bool sendSaveToSSD() = 0;
	/*virtual bool sendUel() = 0;
	virtual bool sendJobStart() = 0;
	virtual bool sendJobEnd() = 0;
	virtual bool sendFileSize(QString filename_) = 0;
	virtual bool sendJobCancel() = 0;*/
	bool mCancelFlag;
public slots:
	void cancel_transfer() { mCancelFlag = true; };
signals:
	void signal_dialogContents(QString p_message, int p_type, bool p_OkVisible = true, bool p_cancelVisible = false, bool p_yesVisible = false, bool p_noVisible = false, bool p_delay = false);
	void signal_progressValue(int);
};

class NetworkTransfer : public FileTransfer
{
	Q_OBJECT
public:
	NetworkTransfer();
	~NetworkTransfer();
	virtual bool createConnection(QString IPorSerial_);
	virtual bool transferFile(QString fileName, QString ip);
	virtual bool transferFile(QString fileName);
	virtual	bool transferData(QByteArray data_);
	virtual bool sendPassword(QString password_);
	virtual bool sendSaveToSSD();
	/*virtual bool sendUel();
	virtual bool sendJobStart();
	virtual bool sendJobEnd();
	virtual bool sendFileSize(QString filename_);
	virtual bool sendJobCancel();*/
private:
	bool WriteDataToSocket(QTcpSocket* socket, const char* buffer, int lenToSend);
	bool WaitForSocketWrite(QTcpSocket* socket);
	QTcpSocket mSocket;
};

class USBTransfer : public FileTransfer
{
	Q_OBJECT
public:
	USBTransfer();
	~USBTransfer();
	virtual bool createConnection(QString IPorSerial_);
	virtual bool transferFile(QString fileName, QString ip);
	virtual bool transferFile(QString fileName);
	virtual	bool transferData(QByteArray data_);
	virtual bool sendPassword(QString password_);
	virtual bool sendSaveToSSD();
	/*virtual bool sendUel();
	virtual bool sendJobStart();
	virtual bool sendJobEnd();
	virtual bool sendFileSize(QString filename_);
	virtual bool sendJobCancel();*/
private:
	PrinterInterface mSelectedPrinter;
};