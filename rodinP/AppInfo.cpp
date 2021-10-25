#include "stdafx.h"
#include "AppInfo.h"
#include "Cryptopp.h"

//build하고자 하는 case를 확인하고 하나만 define해주세요.
#define SINDOH
//#define RHEA
//#define ADDIN
//#define STANLEY

#pragma comment(lib,"version.lib")

QString AppInfo::getCompanyName()
{
#if defined SINDOH
	return "Sindoh";
#elif defined STANLEY
	return "STANLEY";
#else
	return "Sindoh";
#endif
}
QString AppInfo::getAppName()
{
#if defined SINDOH
	return "3DWOX Desktop";
#elif defined RHEA
	return "Rhea";
#elif defined ADDIN
	return "3DWOX for SolidWorks";
#elif defined STANLEY
	return "STANLEY 3D";
#endif
}
QString AppInfo::getProductName()
{
#if defined SINDOH
	return "3DWOX";
#elif defined RHEA
	return "Rhea";
#elif defined STANLEY
	return "MODEL 1";
#else 
	return "3DWOX";
#endif
}
QString AppInfo::getDownloadURL()
{
#if defined SINDOH
	return "http://3dprinter.sindoh.com/Downloads?rodin_lang=";
#elif defined STANLEY
	//수정 필요
	return "http://3dprinter.sindoh.com/STANLEYDownloads?rodin_lang=";
#else
	return "";
#endif
}
QString AppInfo::getCheckVersionURL()
{
#if defined SINDOH
	return "http://3dprinter.sindoh.com/checkDesktopVersion?type=";
#elif defined STANLEY
	//수정 필요
	return "http://3dprinter.sindoh.com/checkSTANLEYVersion?type=";
#else
	return "";
#endif
}
QString AppInfo::getFaqURL(int company_code)
{
#if defined SINDOH
	if (company_code == 2)
		return "http://www.leon-3d.es/lion-2-faq";
	else
		return "http://3dprinter.sindoh.com/FAQs?rodin_lang=";
#elif defined STANLEY
	//수정 필요getSystemIpAddress
	return "http://3dprinter.sindoh.com/STANLEYFAQs?rodin_lang=";
#else
	return "";
#endif
}
QString AppInfo::getMainURL(int company_code)
{
#if defined SINDOH
	if (company_code == 2)
		return "http://www.leon-3d.es";
	else
	{
		QSettings props(AppInfo::getCompanyName(), AppInfo::getAppName());
		QString locale = props.value("lang").toString();
		if (locale == "ko") return "http://www.sindoh.com";
		else return "https://3dprinter.sindoh.com";
	}
#elif defined STANLEY
	//수정 필요
	return "http://www.stanleytools.com";
#else
	return "";
#endif
}
AppInfo::AppCode AppInfo::getAppCode()
{
#if defined SINDOH
	return AppCode::SINDOH_APP;
#elif defined RHEA
	return AppCode::RHEA_APP;
#elif defined ADDIN
	return AppCode::ADDIN_APP;
#elif defined STANLEY
	return AppCode::STANLEY_APP;
#endif
}

QString AppInfo::getCurrentVersion()
{
	// 버전정보를 담을 버퍼
	char* buffer = NULL;
	// 버전을 확인할 파일
	//char* name = "D:\01.work\VisualStudio\Rodin-P\trunk\Release\rodinP.exe";
	TCHAR name[MAX_PATH];
	GetModuleFileName(NULL, name, MAX_PATH);
	//LPCWSTR name = L"rodinP.exe";

	DWORD infoSize = 0;
	QString currentVersion;


	// 파일로부터 버전정보데이터의 크기가 얼마인지를 구합니다.
	infoSize = GetFileVersionInfoSize(name, 0);
	if (infoSize == 0) return NULL;

	// 버퍼할당
	buffer = new char[infoSize];
	if (buffer)
	{
		// 버전정보데이터를 가져옵니다.
		if (GetFileVersionInfo(name, 0, infoSize, buffer) != 0)
		{
			VS_FIXEDFILEINFO* pFineInfo = NULL;
			UINT bufLen = 0;
			// buffer로 부터 VS_FIXEDFILEINFO 정보를 가져옵니다.
			if (VerQueryValue(buffer, L"\\", (LPVOID*)&pFineInfo, &bufLen) != 0)
			{
				WORD majorVer, minorVer, buildNum, revisionNum;
				majorVer = HIWORD(pFineInfo->dwFileVersionMS);
				minorVer = LOWORD(pFineInfo->dwFileVersionMS);
				buildNum = HIWORD(pFineInfo->dwFileVersionLS);
				revisionNum = LOWORD(pFineInfo->dwFileVersionLS);

				// 파일버전 출력
				//printf("version : %d,%d,%d,%d\n", majorVer, minorVer, buildNum, revisionNum);
				currentVersion = QString::number(majorVer) + "." + QString::number(minorVer) + "." + QString::number(buildNum) + "." + QString::number(revisionNum);
				return currentVersion;
			}
		}
		delete[] buffer;
	}
	return NULL;
}

QString AppInfo::getSystemHostName()
{
	QString rtn = "";
	rtn = QString::fromStdString(Cryptopp::encryptAES256(QHostInfo::localHostName().toLocal8Bit().toStdString()));
	return rtn;
}

QString AppInfo::getSystemIpAddress()
{
	QString rtn = "";
	foreach(const QNetworkInterface &netInterface, QNetworkInterface::allInterfaces())
	{
		QNetworkInterface::InterfaceFlags flags = netInterface.flags();

		if ((bool)(flags & QNetworkInterface::IsRunning) && !(bool)(flags & QNetworkInterface::IsLoopBack))
		{
			foreach(const QNetworkAddressEntry &address, netInterface.addressEntries())
			{
				if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
				{
					qDebug() << address.ip().toString();
					rtn = address.ip().toString();
				}
			}
		}
	}

	rtn = QString::fromStdString(Cryptopp::encryptAES256(rtn.toStdString()));
	return rtn;
}
