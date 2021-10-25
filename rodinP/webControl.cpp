#include "stdafx.h"
#include "WebControl.h"

WebControl::WebControl()
{
}


WebControl::~WebControl()
{
}

void WebControl::faqOpen(int v_company_code)
{
	QString locale = Generals::getProps("lang");
	QString link = AppInfo::getFaqURL(v_company_code) + locale; //faq link
	QDesktopServices::openUrl(QUrl(link));
}

checkVersion::checkVersion(QWidget *pParent, QString p_currentVersion, bool p_fromMenu)
	:parent(pParent)
{
	m_fromMenu = p_fromMenu;
	m_currentVersion = p_currentVersion;
	fetch();
}
checkVersion::~checkVersion()
{
	delete m_manager;
}

void checkVersion::fetch()
{ 
	QString osCheck = "";
#ifdef Q_OS_WIN
	osCheck = "Window";
#elif Q_OS_MAC
	osCheck = "Mac";
#else
	osCheck = "";
#endif
	m_manager = new QNetworkAccessManager(this);
	connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

	QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(AppInfo::getCheckVersionURL() + osCheck)));
}

void checkVersion::replyFinished(QNetworkReply* pReply)
{
	QByteArray data = pReply->readAll();
	QString latestVer(data);

	//페이지를 호출하지 못한 경우 종료
	if (latestVer == "")
	{
		if (m_fromMenu)
		{
			//네트워크 연결 확인 안됨
			CommonDialog comDlg(parent, MessageAlert::tr("no_network_connection"), CommonDialog::Warning);
		}
		return;
	}
	//application의 version을 찾지 못한 경우 무조건 종료
	else if (m_currentVersion == "") return;

	//Webpage에서 예외사항이 나오는 경우 version 확인 안됨 메세지
	if (latestVer == "No Type" || latestVer == "Invalid Type" || latestVer == "No Result")
	{
		if (m_fromMenu)
		{
			//최신 버전 정보가 확인되지 않습니다.
			CommonDialog comDlg(parent, MessageAlert::tr("cannot_verify_update_version"), CommonDialog::Warning);
		}
		return;
	}

	if (m_currentVersion == latestVer)
	{
		if (m_fromMenu)
			CommonDialog comDlg(parent, MessageInfo::tr("current_version_up_to_date"), CommonDialog::Information);
	}
	else
	{
		try
		{
			bool isLatest = true;
			QStringList latestVers = latestVer.split(".");
			QStringList currentVers = m_currentVersion.split(".");
			//둘중에 더 짧은 걸로...
			int targetSize;
			if (latestVers.size() > currentVers.size()) targetSize = currentVers.size();
			else targetSize = latestVers.size();

			for (int i = 0; i < targetSize; i++)
			{
				if (currentVers.at(i).toInt() > latestVers.at(i).toInt())
				{
					isLatest = true;
					break;
				}
				else if (currentVers.at(i).toInt() == latestVers.at(i).toInt())
				{
					continue;
				}
				else 
				{
					isLatest = false;
					break;
				}
			}

			if (isLatest)
			{
				if (m_fromMenu) CommonDialog comDlg(parent, MessageInfo::tr("current_version_up_to_date"), CommonDialog::Information);
			}
			else
			{
				QString message = MessageQuestion::tr("update_verstion_exists");
				message = message.arg(latestVer);
				CommonDialog comDlg(parent, message, CommonDialog::Question, false, false, true, true);
				if (comDlg.isYes())
				{
					QString locale = Generals::getProps("lang");
					QString link = AppInfo::getDownloadURL() +locale;
					QDesktopServices::openUrl(QUrl(link));
				}
			}
		}
		catch (const std::exception& e)
		{
			if (m_fromMenu)
			{
				//최신 버전 정보가 확인되지 않습니다.
				CommonDialog comDlg(parent, MessageAlert::tr("cannot_verify_update_version"), CommonDialog::Warning);
			}
			return;
		}
	}
	//process str any way you like!

}