#include "stdafx.h"
#include "USBCommunication.h"

//implementation of callUSBDriver class


CallUSBDriver::CallUSBDriver(const TCHAR* printerName, bool writeFlag) :m_Handle(INVALID_HANDLE_VALUE)
{
	//strcpy_s(m_printerName, SIZE_LEN, printerName);
	wcscpy_s(m_printerName, SIZE_LEN, printerName);

	m_Handle = getHandleToPrinter(m_printerName, writeFlag);
	if (INVALID_HANDLE_VALUE == m_Handle)
		throw USBException("Error in opening File");
}


CallUSBDriver::~CallUSBDriver()
{
	if (INVALID_HANDLE_VALUE != m_Handle)    
		CloseHandle(m_Handle);
}



//File을 read할때와 write할때 handle을 분리하여 관리
HANDLE CallUSBDriver::getHandleToPrinter(const TCHAR* printerName, bool writeFlag)
{
	if (writeFlag)
		return CreateFile(printerName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL |
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	else
		return CreateFile(printerName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED, NULL);
}

bool CallUSBDriver::writeDataToPrinter(const char* pData, const int& size)
{
	DWORD bytes_written = 0;
	BOOL bRet = WriteFile(m_Handle, pData, size, &bytes_written, NULL);
	return bytes_written == size ? TRUE : FALSE;
	/*overlapped = { 0, };
	if(!WriteFile(m_Handle, pData, size, &bytes_written, &overlapped)){
		if (!AnalyseError(GetLastError(), &bytes_written)){
			CancelIo(m_Handle);
		}

	}
	return bytes_written == size ? TRUE : FALSE;*/
}

bool CallUSBDriver::writeDataToPrinterOL(const char* pData, const int& size){
	DWORD bytes_written = 0;
	//BOOL bRet = WriteFile(m_Handle, pData, size, &bytes_written, NULL);
	//return bytes_written == size ? TRUE : FALSE;
	overlapped = { 0, };
	if(!WriteFile(m_Handle, pData, size, &bytes_written, &overlapped))
	{
		if (!analyseError(GetLastError(), &bytes_written))
		{
			CancelIo(m_Handle);
		}
	}
	return bytes_written == size ? TRUE : FALSE;
}

unsigned int CallUSBDriver::readDataFromPrinter(char** pData, const int& size)
{
	DWORD byteRead = 0;
	char buf[10240];
	overlapped = { 0, };
	if (!ReadFile(m_Handle, buf, size, &byteRead, &overlapped))
	{
		if (!analyseError(GetLastError(), &byteRead))
		{
			CancelIo(m_Handle);
		}
	}
	*pData = buf;
	return byteRead;
}


bool CallUSBDriver::analyseError(DWORD error, DWORD* pTransferedByte)
{
	BOOL bRet = false;
	switch (error)
	{
	case ERROR_IO_PENDING:
		Sleep(10);
		bRet = GetOverlappedResult(m_Handle, &overlapped, pTransferedByte, FALSE);
		/*if (!bRet){
			error = GetLastError();
		}*/
		break;
	default:
		break;
		
	}
	return bRet ? true:false;
}







 