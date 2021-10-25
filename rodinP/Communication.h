#pragma once

#include "CommunicationInterface.h"

class Communication;
typedef  CommunicationInterface<Communication>  PrinterInterface;


std::vector<PrinterInterface> getAllCommunicationInterface(const char* vid, bool writeFlag);

class Communication
{
public:
	virtual bool writeDataToPrinter(const char* pData, const int& size) = 0;
	virtual bool writeDataToPrinterOL(const char* pData, const int& size) = 0;
	virtual unsigned int readDataFromPrinter(char** pData, const int& size) =0;
	virtual ~Communication(){}

};





class Exception
{
	virtual const char* What() = 0;
};