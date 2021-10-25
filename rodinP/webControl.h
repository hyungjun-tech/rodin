#pragma once

class WebControl : public QObject
{
	Q_OBJECT
public:
	WebControl();
	~WebControl();
public slots:
	void faqOpen(int = 0);
};

class checkVersion : public QObject
{
	Q_OBJECT
public:
	checkVersion(QWidget *parent, QString p_currentVersion, bool p_fromMenu = false);
	~checkVersion();
	QString m_currentVersion;
	QWidget *parent;
public slots:
	void replyFinished(QNetworkReply*);
	void fetch();

private:
	QNetworkAccessManager* m_manager;
	bool m_fromMenu;
};
