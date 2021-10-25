#include "stdafx.h"
#include "FileTransfer.h"
#include "ConnectionControl.h"
#include "CommonMessages.h"

FileTransfer::FileTransfer()
	: mCancelFlag(false)
{
}


FileTransfer::~FileTransfer()
{
}




NetworkTransfer::NetworkTransfer()
{
}
NetworkTransfer::~NetworkTransfer()
{
	if (mSocket.isOpen())
		mSocket.close();
}
bool NetworkTransfer::createConnection(QString IPorSerial_)
{
	NetworkConnection conn;
	if (!NetworkConnection::checkNetwork())
	{
		emit signal_dialogContents(MessageAlert::tr("no_network_connection"), CommonDialog::Warning);
		return false;
	}
	mSocket.setProxy(QNetworkProxy::NoProxy);
	mSocket.connectToHost(IPorSerial_, 9100);
	if (!mSocket.waitForConnected(5000))
	{
		QString message = MessageAlert::tr("cannot_connect_printer")
			+ QString("\n%1 (%2)").arg(PrinterInfo::currentPrinterName, PrinterInfo::currentIP);
		emit signal_dialogContents(message, CommonDialog::Warning);
		return false;
	}
	return true;
}
bool NetworkTransfer::WriteDataToSocket(QTcpSocket* socket, const char* buffer, int lenToSend)
{
	do {
		int lenSent = socket->write(buffer, lenToSend);
		if (lenSent == -1 || mCancelFlag) {//error
			if (!mCancelFlag)
			{
				Logger() << "ERROR : socket write fails";
				emit signal_dialogContents(MessageError::tr("temparary_error_and_cancel"), CommonDialog::Critical);
			}
			return false;
		}
		///Logger() << "Sent Length =" << lenSent << " length to send = " << lenToSend;
		buffer += lenSent;
		lenToSend -= lenSent;
	} while (lenToSend > 0);
	//Logger() << "WriteDataToSocket Success";
	return true;
}

bool NetworkTransfer::WaitForSocketWrite(QTcpSocket* socket)
{
	if (mCancelFlag) return false;

	int retry = 0;
	bool bRet = true;
	//QTime qtime;
	//qtime.start();
	QString errorMessage;
	while ((!socket->waitForBytesWritten()) || (socket->bytesToWrite() != 0))
	{
		switch (mSocket.error())
		{
		case QAbstractSocket::ConnectionRefusedError:
			errorMessage = "Socket Error : Connection is refused";
			break;
		case QAbstractSocket::RemoteHostClosedError:
			errorMessage = "Socket Error : Remote host closed";
			break;
		case QAbstractSocket::HostNotFoundError:
			errorMessage = "Socket Error : Host not found";
			break;
		case QAbstractSocket::SocketAccessError:
			errorMessage = "Socket Error : Access error";
			break;
		case QAbstractSocket::SocketResourceError:
			errorMessage = "Socket Error : Resource error";
			break;
		case QAbstractSocket::SocketTimeoutError:
			errorMessage = "Socket Error : Time out error";
			break;
		case QAbstractSocket::DatagramTooLargeError:
			errorMessage = "Socket Error : Datagram Tool large";
			break;
		case QAbstractSocket::NetworkError:
			errorMessage = "Socket Error : network error";
			break;
		case QAbstractSocket::AddressInUseError:
			errorMessage = "Socket Error : address in use error";
			break;
		case QAbstractSocket::SocketAddressNotAvailableError:
			errorMessage = "Socket Error : address not available";
			break;
		case QAbstractSocket::UnsupportedSocketOperationError:
			errorMessage = "Socket Error : unsupported socket operation";
			break;
		case QAbstractSocket::UnfinishedSocketOperationError:
			errorMessage = "Socket Error : unfinished socket operation";
			break;
		case QAbstractSocket::OperationError:
			errorMessage = "Socket Error : operation error";
			break;
		case QAbstractSocket::TemporaryError:
			errorMessage = "Socket Error : temporary error";
			break;
		case QAbstractSocket::UnknownSocketError:
			errorMessage = "Socket Error : unknown error";
			break;
		default:
			errorMessage = "Other Socket Error :" + QString::number(mSocket.error());
			break;
		}

		retry++;
		if (retry == 1000 || mCancelFlag)
		{
			bRet = false;
			if (!mCancelFlag)
			{
				Logger() << "ERROR : Wait for write fails";
				Logger() << errorMessage;
				emit signal_dialogContents(MessageError::tr("temparary_error_and_cancel"), CommonDialog::Critical);
			}
			break;
		}
	}
	Logger() << "WaitForSocketWrite retrying = " << retry;
	//int elapsed = qtime.elapsed();
	//if (bRet)
	//	Logger() << "WaitForSocketWrite Success";

	// 100MB를 10분간 전달할 때 전송 속도는 174763 byte/sec
	// 32768*8 = 262144 byte를 같은 속도로 전달 시 필요 시간은 1.5 sec임.
	// 반올림하여 262144 byte를 보내는데 2초 이상 소요 시 속도 느린 것으로 표시
	/*if (elapsed >= 2000) {
		float speed = (float)262144 * 1000.f / 1024.f / (float)elapsed;		// kbyte / sec

		QString msg = QString(tr("network_too_slow_message"));
		msg += QString::number(speed) + QString(" kbyte / sec");
		emit signal_dialogContents(msg, CommonDialog::Warning, false, true, false, false, false);
	}*/
	return bRet;
}

bool NetworkTransfer::transferFile(QString fileName, QString ip)
{
	bool rtn;
	mSocket.setProxy(QNetworkProxy::NoProxy);
	mSocket.connectToHost(ip, 9100);
	rtn = transferFile(fileName);
	return rtn;
}

bool NetworkTransfer::transferFile(QString fileName)
{
	// we need to wait...
	if (mSocket.state() != QAbstractSocket::ConnectedState)
	{
		if (!mSocket.waitForConnected(5000))
		{
			emit signal_dialogContents(MessageError::tr("temporary_error_and_retry"), CommonDialog::Critical);
			return false;
		}
	}

	QFile f(fileName);
	if (!f.exists())
		return false;
	QByteArray read;
	qDebug() << "Ready for transfer file : " << fileName;
	int readtotal = 0;
	int fileSize = f.size();
	int per = 0;
	emit signal_dialogContents(MessageProgress::tr("print_file_sending"), CommonDialog::Progress, false, true);

	if (!f.open(QIODevice::ReadOnly)) {
		qDebug() << "fail to file open : " << fileName;
		return false;
	}
	while (true)
	{
		//보내기전 cancel여부 확인 추가
		if (mCancelFlag)
		{
			emit signal_dialogContents(MessageAlert::tr("user_cancelled_message"), CommonDialog::Warning);
			f.close();
			return false;
		}
		read.clear();
		read = f.read(32768 * 8);
		readtotal = readtotal + read.size();
		Logger() << "Read size : " << read.size();
		Logger() << "Read total : " << readtotal;
		per = (double(readtotal) / double(fileSize)) * 100;

		if (read.size() == 0)
		{
			break;
		}

		//bool check = true;
		//int retry = 0;
		if (!transferData(read)) {
			if (mCancelFlag)
				emit signal_dialogContents(MessageAlert::tr("user_cancelled_message"), CommonDialog::Warning);
			f.close();
			return false;
		}
		emit signal_progressValue(per);

		read.clear();
		if (mCancelFlag)
		{
			emit signal_dialogContents(MessageAlert::tr("user_cancelled_message"), CommonDialog::Warning);
			f.close();
			return false;
		}
	}
	f.close();
	return true;
}
bool NetworkTransfer::transferData(QByteArray data_)
{
	//내 컴퓨터 네트워크 연결 안됨
	if (!NetworkConnection::checkNetwork())
	{
		//네트워크 연결이 되지 않았습니다.\n네트워크 상태를 확인해 주세요.
		emit signal_dialogContents(MessageAlert::tr("no_network_connection"), CommonDialog::Warning);
		return false;
	}

	if (!WriteDataToSocket(&mSocket, data_.constData(), data_.size()))
	{
		return false;
	}
	if (!WaitForSocketWrite(&mSocket))
	{
		return false;
	}
	return true;
}

bool NetworkTransfer::sendPassword(QString password_)
{
	if (password_.isEmpty())
		return true;
	qDebug() << "sendPassword";
	QString commend = ";PASSWORD:" + password_ + "\n";
	return transferData(commend.toLocal8Bit());
}
bool NetworkTransfer::sendSaveToSSD()
{
	qDebug() << "sendSaveToSSD";
	QString commend = ";SAVE_TO_SSD\n";
	return transferData(commend.toLocal8Bit());
}
/*
bool NetworkTransfer::sendUel()
{
	qDebug() << "sendUEL";
	QString commend = "\x1b%-12345X";
	return transferData(commend.toLocal8Bit());
}
bool NetworkTransfer::sendJobStart()
{
	qDebug() << "sendJobStart";
	QString commend = "@PJL SET JOBSTART\r\n";
	return transferData(commend.toLocal8Bit());
}
bool NetworkTransfer::sendJobEnd()
{
	qDebug() << "sendJobEnd";
	QString commend = "@PJL SET JOBEND\r\n";
	return transferData(commend.toLocal8Bit());
}
bool NetworkTransfer::sendFileSize(QString filename_)
{
	qDebug() << "sendFileSize : " << filename_;
	QFile f(filename_);
	int filesize = f.size();
	Logger() << "file size = " << filesize << "Byte";
	QString commend = "@PJL SET SIZE=" + QString::number(filesize) + "\r\n";
	return transferData(commend.toLocal8Bit());
}
bool NetworkTransfer::sendJobCancel()
{
	qDebug() << "sendJobCancel";
	QString commend = "@PJL SET SJOBCANCEL\r\n";
	return transferData(commend.toLocal8Bit());
}
*/




USBTransfer::USBTransfer()
{
}
USBTransfer::~USBTransfer()
{
}
bool USBTransfer::createConnection(QString IPorSerial_)
{
	PJLControl a;
	mSelectedPrinter = a.getSelectPrinter(IPorSerial_, true);
	if (mSelectedPrinter.getRawPointer() == NULL)
	{
		emit signal_dialogContents(MessageAlert::tr("cannot_connect_printer"), CommonDialog::Warning);
		return false;
	}
	else
		return true;
}
bool USBTransfer::transferFile(QString fileName, QString ip)
{
	bool rtn;
	PJLControl a;
	mSelectedPrinter = a.getSelectPrinter(ip, true);//IP = Serial
	if (mSelectedPrinter.getRawPointer() == NULL)
		return false;
	rtn = transferFile(fileName);
	return rtn;
}
bool USBTransfer::transferFile(QString fileName)
{
	if (mSelectedPrinter.getRawPointer() == NULL)
		return false;

	QFile f(fileName);
	if (!f.exists())
		return false;
	qDebug() << "Ready for transfer file : " << fileName;
	int readtotal = 0;
	int fileSize = f.size();
	int per = 0;
	QByteArray read;
	emit signal_dialogContents(MessageProgress::tr("print_file_sending"), CommonDialog::Progress, false, true);

	if (!f.open(QIODevice::ReadOnly)) {
		return false;
	};
	while (true)
	{
		if (mCancelFlag)
		{
			//emit sendNewDialogContents(tr("This operation has been cancelled.\n(If the printer is already printing please cancel.)"), CommonDialog::Warning);
			emit signal_dialogContents(MessageAlert::tr("user_cancelled_message"), CommonDialog::Warning);
			f.close();
			return false;
		}
		read.clear();
		read = f.read(32768 * 8);
		//read = filename->readAll();
		readtotal = readtotal + read.size();
		per = (double(readtotal) / double(fileSize)) * 100;
		Logger() << "Read size : " << read.size();
		Logger() << "Read total : " << readtotal;

		if (read.size() == 0)
		{
			//emit SendStatus(QString("Complete File Transfer!!"));
			break;
		}

		if (!transferData(read)) {
			if (mCancelFlag)
				emit signal_dialogContents(MessageAlert::tr("user_cancelled_message"), CommonDialog::Warning);
			f.close();
			return false;
		}
		emit signal_progressValue(per);
		read.clear();
		if (mCancelFlag)
		{
			//emit sendNewDialogContents(tr("This operation has been cancelled.\n(If the printer is already printing please cancel.)"), CommonDialog::Warning);
			emit signal_dialogContents(MessageAlert::tr("user_cancelled_message"), CommonDialog::Warning);
			f.close();
			return false;
		}
	}
	f.close();
	//commonDlg->setDialogContents(tr("Print file sent."), CommonDialog::Information);
	//emit signal_dialogContents(tr("file_send_complete_message"), CommonDialog::Information);
	return true;
}
bool USBTransfer::transferData(QByteArray data_)
{
	//선택된 Printer를 찾을 수 없음.
	if (mSelectedPrinter.getRawPointer() == NULL) {
		emit signal_dialogContents(MessageAlert::tr("cannot_connect_printer"), CommonDialog::Warning);
		return false;
	}

	int retry = 0;
	while (!mSelectedPrinter->writeDataToPrinter(data_.data(), data_.length()))
	{
		retry++;
		if (retry == 1000 || mCancelFlag)
		{
			if (!mCancelFlag)
			{
				Logger() << "WriteDataToPrinter failure";
				//commonDlg->setDialogContents(tr("The object cannot be printed due to temporary error.\nPlease try again.\n\n(If the printer is already printing please cancel."), CommonDialog::Critical);
				emit signal_dialogContents(MessageError::tr("temparary_error_and_cancel"), CommonDialog::Critical);
			}
			return false;
		}
	}
	Logger() << "retrying = " << retry;
	return true;
}

bool USBTransfer::sendPassword(QString password_)
{
	if (password_.isEmpty())
		return true;
	qDebug() << "sendPassword";
	QString commend = ";PASSWORD:" + password_ + "\n";
	return transferData(commend.toLocal8Bit());
}
bool USBTransfer::sendSaveToSSD()
{
	qDebug() << "sendSaveToSSD";
	QString commend = "@PJL SET SAVE2SSD\r\n";
	return transferData(commend.toLocal8Bit());
}
/*bool USBTransfer::sendUel()
{
	qDebug() << "sendUEL";
	QString commend = "\x1b%-12345X";
	return transferData(commend.toLocal8Bit());
}
bool USBTransfer::sendJobStart()
{
	qDebug() << "sendJobStart";
	QString commend = "@PJL SET JOBSTART\r\n";
	return transferData(commend.toLocal8Bit());
}
bool USBTransfer::sendJobEnd()
{
	qDebug() << "sendJobEnd";
	QString commend = "@PJL SET JOBEND\r\n";
	return transferData(commend.toLocal8Bit());
}

bool USBTransfer::sendFileSize(QString filename_)
{
	qDebug() << "sendFileSize : " << filename_;
	QFile f(filename_);
	int filesize = f.size();
	Logger() << "file size = " << filesize << "Byte";
	QString commend = "@PJL SET SIZE=" + QString::number(filesize) + "\r\n";
	return transferData(commend.toLocal8Bit());
}
bool USBTransfer::sendJobCancel()
{
	qDebug() << "sendJobCancel";
	QString commend = "@PJL SET SJOBCANCEL\r\n";
	return transferData(commend.toLocal8Bit());
}*/