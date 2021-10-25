#include "stdafx.h"
#include "snmpControl.h"
#pragma comment (lib, "WSNMP32.LIB")


snmpControl::snmpControl()
{
	if (SnmpStartup(&nMajorVersion, &nMinorVersion, &nLevel, &nTranslateMode, &nRetransmitMode) != SNMPAPI_SUCCESS){
		qDebug("initilization failure");
	}
	if (SnmpSetTranslateMode(SNMPAPI_UNTRANSLATED_V1) != SNMPAPI_SUCCESS)
		qDebug("SetTranslateMode failure");
	if (SnmpSetRetransmitMode(SNMPAPI_OFF) != SNMPAPI_SUCCESS)
		qDebug("SetRetransmitMode failure");
	createSession();
}


snmpControl::~snmpControl()
{
	if (session != NULL)
		SnmpClose(session);
	SnmpCleanup();
}

SNMPAPI_STATUS CALLBACK MySnmpCallback(HSNMP_SESSION hSession,HWND hWnd,UINT wMsg,WPARAM wParam,LPARAM lParam,LPVOID lpClientData)
{
	snmpControl * a = (snmpControl*)lpClientData;
	return a->snmpReceive(hSession);

	//delete a;
	//return SNMPAPI_SUCCESS;
}

QString snmpControl::getSnmpValue(QString ipAddress, QString pOid, QString pCommunity, int tmo)
{
	//qDebug() << tmo;
	vResult = "";
	snmpSend(ipAddress, pOid, pCommunity);

	//tmo父怒 wait
	/*HANDLE _hWait;
	_hWait = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	DWORD waitRet = ::WaitForSingleObject(_hWait, tmo);
	::ResetEvent(_hWait);*/
	int vCnt = 0;
	while (true)
	{
		HANDLE _hWait;
		_hWait = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		DWORD waitRet = ::WaitForSingleObject(_hWait, 10);
		::ResetEvent(_hWait);
		vCnt++;
		if (vCnt*10 >= tmo) break;
		if (vResult != "") break;
	}
	//qDebug() << vCnt;

	return vResult;
}

void  snmpControl::createSession()
{
	SNMPAPI_CALLBACK  callBackNum;
	session = SnmpCreateSession(0, 0, &MySnmpCallback, this);
	if (SNMPAPI_FAILURE == session) {
		qDebug("Could not create WinSNMP session");
	}
}

bool snmpControl::snmpSend(QString ipAddress, QString pOid, QString pCommunity)
{
	//Entity 汲沥
	dstEntity = SnmpStrToEntity(session, ipAddress.toUtf8().constData());

	//oid汲沥
	if (SnmpStrToOid(pOid.toUtf8().constData(), &oid) == SNMPAPI_FAILURE)
	{
		qDebug("SnmpStrToOid failure");
		return false;
	}

	//vbl积己
	if ((_vbl = SnmpCreateVbl(session, &oid, NULL)) == SNMPAPI_FAILURE)
	{
		qDebug("SnmpCreateVbl failure");
		return false;
	}

	//pdu积己
	if ((_pdu = SnmpCreatePdu(session, SNMP_PDU_GET, 0, 0, 0, _vbl)) == SNMPAPI_FAILURE)
	{
		qDebug("SnmpCreatePdu failure");
		return false;
	}
	smiINT32 id;
	if (SnmpGetPduData(_pdu, NULL, &id, NULL, NULL, NULL) == SNMPAPI_FAILURE) {
		qDebug() << "getPduData Failure check 2";
	}

	//context积己
	std::string comTemp = pCommunity.toUtf8().constData();
	community.len = (smiUINT32)strlen(comTemp.c_str());
	community.ptr = (smiLPBYTE)comTemp.c_str();
	//community.len = (smiUINT32)strlen(pCommunity.toStdString().c_str());
	//community.ptr = (smiLPBYTE)pCommunity.toStdString().c_str();
	if ((context = SnmpStrToContext(session, &community)) == SNMPAPI_FAILURE)
	{
		qDebug("SnmpStrToContext failure");
		return false;
	}

	//msg send
	if (SnmpSendMsg(session, 0, dstEntity, context, _pdu) != SNMPAPI_SUCCESS)
	{
		qDebug("SnmpSendMsg failure");
		qDebug() << SnmpGetLastError(session);
	}

	//memory free
	if (dstEntity != NULL)
		SnmpFreeEntity(dstEntity);
	if (_vbl != NULL)
		SnmpFreeVbl(_vbl);
	if (_pdu != NULL)
		SnmpFreePdu(_pdu);
	if (context != NULL)
		SnmpFreeContext(context);
	//SnmpFreeDescriptor(SNMP_SYNTAX_OID, (smiLPOPAQUE)&oid);
	return true;
}

bool snmpControl::snmpReceive(HSNMP_SESSION pSession)
{
	HSNMP_ENTITY srcEntity; 
	HSNMP_ENTITY dstEntity;  

	HSNMP_CONTEXT context;
	HSNMP_PDU pPdu;
	smiLPINT PDU_type = new smiINT;
	smiLPINT32 request_id = new smiINT32;
	smiLPINT error_status = new smiINT;
	smiLPINT error_index = new smiINT;

	HSNMP_VBL varbindlist;
	int cnt;
	vResult = "";

	smiOID dRetName;
	smiVALUE dRetValue;

	if (SnmpRecvMsg(pSession, &srcEntity, &dstEntity, &context, &pPdu) != SNMPAPI_SUCCESS){
		qDebug("SnmpRecvMsg failure");
		return false;
	}

	if (SnmpGetPduData(pPdu, PDU_type, request_id, error_status, error_index, &varbindlist) != SNMPAPI_SUCCESS)
	{
		qDebug("SnmpGetPduData failure");
		return false;
	}

	if (SnmpGetVb(varbindlist, 1, &dRetName, &dRetValue) != SNMPAPI_SUCCESS){
		qDebug("SnmpGetVb failure");
		return false;
	}

	if (dRetValue.syntax == SNMP_SYNTAX_INT)
	{
		vResult = QString::number(dRetValue.value.sNumber);
	}
	else if (dRetValue.syntax == SNMP_SYNTAX_OCTETS)
	{
		if (dRetValue.value.string.ptr != NULL)
		{
			for (int j = 0; j < dRetValue.value.string.len; j++) {
				vResult.append((QString)dRetValue.value.string.ptr[j]);
			}
			//qDebug() << vResult;
		}
	}


	/*if ((cnt = SnmpCountVbl(varbindlist)) == SNMPAPI_FAILURE){
		qDebug("SnmpCountVbl failure");
		return false;
	}

	for (int i = 1; i <= cnt; i++){          // get the result   
		if (SnmpGetVb(varbindlist, i, &dRetName, &dRetValue) != SNMPAPI_SUCCESS){
			qDebug("SnmpGetVb failure");
			return false;
		}
		for (int j = 0; j<dRetValue.value.string.len; j++) {
			vResult.append((QString)dRetValue.value.string.ptr[j]);
		}
		qDebug() << vResult;
	}*/

	//memory free
	SnmpFreeEntity(srcEntity);
	SnmpFreeEntity(dstEntity);
	SnmpFreeContext(context);
	SnmpFreePdu(pPdu);
	SnmpFreeVbl(varbindlist);
	//SnmpFreeDescriptor(SNMP_SYNTAX_OID, (smiLPOPAQUE)&dRetName);
	//SnmpFreeDescriptor(dRetValue.syntax, (smiLPOPAQUE)&dRetValue.value.string);
	//SnmpFreeDescriptor(SNMP_SYNTAX_OID,*pOid);   
	delete PDU_type;
	delete request_id;
	delete error_status;
	delete error_index;
	return true;
}