#pragma once

#include "Communication.h"

class CallUSBDriver : public Communication
{
	
	enum{ SIZE_LEN  = 256};
public:
	CallUSBDriver(const TCHAR* printerName, bool writeFlag);
	~CallUSBDriver();
	
	bool writeDataToPrinter(const char* pData, const int& size);
	bool writeDataToPrinterOL(const char* pData, const int& size);
	unsigned int readDataFromPrinter(char** pData, const int& size);
	

private:
	HANDLE getHandleToPrinter(const TCHAR* printerName, bool writeFlag);
	bool analyseError(DWORD error, DWORD* pTransferedByte);
private:
	HANDLE m_Handle;
	OVERLAPPED overlapped;
	TCHAR m_printerName[SIZE_LEN];

};

class USBException : public Exception
{
public:
	USBException(const char* error) :m_error(error){}
	const char* What(){ return m_error; }
private:
	const char* m_error;
};