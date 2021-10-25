#pragma once

#include <QString>

class AppInfo
{
public:
	enum AppCode { SINDOH_APP, RHEA_APP, ADDIN_APP, STANLEY_APP };

	static QString getCompanyName();
	static QString getAppName();
	static QString getProductName();
	static QString getDownloadURL();
	static QString getCheckVersionURL();
	static QString getFaqURL(int = 0);
	static QString getMainURL(int = 0);
	static AppCode getAppCode();
	static QString getCurrentVersion();
	static QString getSystemHostName();
	static QString getSystemIpAddress();
};