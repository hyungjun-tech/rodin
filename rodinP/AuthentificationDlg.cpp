#include "stdafx.h"
#include "AuthentificationDlg.h"
#include "Cryptopp.h"

AuthentificationDlg::AuthentificationDlg(int mode, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	this->setModal(true);

	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setAttribute(Qt::WA_DeleteOnClose, true);

	m_id = Generals::getProps("id");
	m_pwd = Generals::getProps("password");


	if (mode == Generals::AuthentificationMethod::Everytime)
	{
		m_id.clear();
		m_pwd.clear();

		ui.lineEdit_id->clear();
		ui.lineEdit_pwd->clear();
	}
	else if (mode == Generals::AuthentificationMethod::SaveToPC)
	{
		if (!m_id.isEmpty())
		{
			QString dec_id;
			QString dec_pwd;

			try
			{
				dec_id = QString::fromStdString(Cryptopp::decryptAES256(m_id.toStdString()));
				dec_pwd = QString::fromStdString(Cryptopp::decryptAES256(m_pwd.toStdString()));
				ui.lineEdit_id->setText(dec_id);
				ui.lineEdit_pwd->setText(dec_pwd + "AAAAAAAAAAAAAAAA");
			}
			catch (...)
			{
				dec_id = "";
				dec_pwd = "";
			}
		}
	}

	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(pushButton_ok_clicked()));
	connect(ui.pushButton_cancel, SIGNAL(clicked()), this, SLOT(close()));
}

AuthentificationDlg::~AuthentificationDlg()
{

}

void AuthentificationDlg::pushButton_ok_clicked()
{
	m_id = ui.lineEdit_id->text();
	m_pwd = ui.lineEdit_pwd->text();

	if (m_pwd.endsWith("AAAAAAAAAAAAAAAA"))
	{
		m_pwd = m_pwd.replace("AAAAAAAAAAAAAAAA", "");
	}

	QString enc_id = QString::fromStdString(Cryptopp::encryptAES256(m_id.toStdString()));
	QString enc_pwd = QString::fromStdString(Cryptopp::encryptAES256(m_pwd.toStdString()));
	Generals::setProps("id", enc_id);
	Generals::setProps("password", enc_pwd);
	//m_parent->afterProfileChanged(true);

	//authentification_setting_complete : Authentification setting is completed//
	CommonDialog comDlg(this, MessageInfo::tr("authentification_setting_complete"), CommonDialog::Information);
	accept();
}
void AuthentificationDlg::clear()
{
	Generals::setProps("id", "");
	Generals::setProps("password", "");

	m_id.clear();
	m_pwd.clear();
}

void AuthentificationDlg::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_Escape:
		this->close();
		break;

	case Qt::Key_Enter:
		this->pushButton_ok_clicked();
		break;

	case Qt::Key_Return:
		this->pushButton_ok_clicked();
		break;
	}
}
