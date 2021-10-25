#include "stdafx.h"
#include "Communication.h"
#include "USBCommunication.h"

#include <initguid.h>
#include <tchar.h>

#include <usb.h>
#include <usbiodef.h>
#include <usbioctl.h>
#include <usbprint.h>
#include <SetupAPI.h>

#include <devguid.h>
#include <wdmguid.h>


#pragma comment(lib, "setupapi.lib")

std::vector<PrinterInterface> getAllCommunicationInterface(const char* vid, bool writeFlag)
{
	GUID intfce = GUID_DEVINTERFACE_USBPRINT;
	std::vector<PrinterInterface> iRetList;

	HDEVINFO devs = SetupDiGetClassDevs(&intfce, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (devs == INVALID_HANDLE_VALUE)
	{
		return iRetList;
	}

	DWORD devcount = 0;
	SP_DEVICE_INTERFACE_DATA devinterface;
	devinterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	while (SetupDiEnumDeviceInterfaces(devs, 0, &intfce, devcount, &devinterface))
	{
		devcount++;
		DWORD size = 0;

		/* See how large a buffer we require for the device interface details */
		SetupDiGetDeviceInterfaceDetail(devs, &devinterface, 0, 0, &size, 0);
		SP_DEVINFO_DATA devinfo;
		devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
		PSP_DEVICE_INTERFACE_DETAIL_DATA interface_detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)calloc(1, size);

		if (interface_detail) {
			interface_detail->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
			devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
			if (!SetupDiGetDeviceInterfaceDetail(devs, &devinterface, interface_detail,
				size, 0, &devinfo)) {
				free(interface_detail);
				SetupDiDestroyDeviceInfoList(devs);
				return iRetList;
			}
			char deviceP[256];
			WideCharToMultiByte(CP_ACP, 0, interface_detail->DevicePath, 256, deviceP, 256, NULL, NULL);
			if (strstr(deviceP, vid)){ //anil-> put proper condition to chek if it is the driver we looking for
			//if (strstr(interface_detail->DevicePath, vid)){ //anil-> put proper condition to chek if it is the driver we looking for

				try
				{
					iRetList.push_back(PrinterInterface(new CallUSBDriver(interface_detail->DevicePath, writeFlag)));
				}
				catch (const Exception& )
				{
					/// anil--> send this error to proper message channel
				}
				
			}
			free(interface_detail);
		}
	}

	SetupDiDestroyDeviceInfoList(devs);
	return iRetList;
}