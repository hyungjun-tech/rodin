#include "stdafx.h"
#include "FindPrinterInfo.h"
#include "CartridgeInfo.h"

FindPrinterInfo::FindPrinterInfo()
{
}

FindPrinterInfo::~FindPrinterInfo()
{

}

void FindPrinterInfo::run()
{
	CartridgeInfo::findCartridgeData();
	emit signal_filamentInfoUpdated();
}