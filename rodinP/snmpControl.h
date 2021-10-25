#pragma once
#include "WinSnmp.h"

class snmpControl
{
	friend SNMPAPI_STATUS CALLBACK MySnmpCallback(HSNMP_SESSION hSession,HWND hWnd,UINT wMsg,WPARAM wParam,LPARAM lParam,LPVOID lpClientData);
public:
	snmpControl();
	~snmpControl();

	QString getSnmpValue(QString ipAddress, QString pOid, QString pCommunity = "public", int tmo = 500);
	void createSession();
	bool snmpSend(QString ipAddress, QString pOid, QString pCommunity);
	bool snmpReceive(HSNMP_SESSION pSession);

	smiUINT32  nMajorVersion;
	smiUINT32  nMinorVersion;
	smiUINT32  nLevel;
	smiUINT32  nTranslateMode;
	smiUINT32  nRetransmitMode;

	HSNMP_SESSION session;

	HSNMP_ENTITY dstEntity;
	smiOID oid;
	HSNMP_VBL _vbl;
	HSNMP_PDU _pdu;
	smiOCTETS community;
	HSNMP_CONTEXT context;


	QString vResult = "";



	HSNMP_VBL m_hvbl;
	HSNMP_PDU m_hpdu;
	smiOID name;
	SNMPAPI_STATUS lstat;
};

