
#include "snmpwrapper.h"
#include "../../implinclude.h"
#include "loghandler.h"

#define FIXED_KEY_ALARM_ID		"AlarmId"
#define FIXED_KEY_TIME			"Time"
#define FIXED_KEY_HOSTNAME		"HostName"
#define FIXED_KEY_TEXT			"Text"
#define FIXED_KEY_ADDRESS		"Address"

#define MAX_DEST_ADDRESS_COUNT	256

ZQ::SNMP::SnmpInst	g_snmpInst;
int					g_nSeqNumber = 0;

bool Init_Proc()
{
	return true;
}

void UnInit_Proc()
{
}

bool Action_Proc(char* const* key, char* const* data, size_t count)
{
	//todo : add lock code here

	Counter32 seq(g_nSeqNumber++);
	int nAlarmId								= 0;
	int nTime									= 0;
	const char* strHostName						= NULL;
	const char* strText							= NULL;

	CTarget* pTarget[MAX_DEST_ADDRESS_COUNT]	= {NULL};
	int nAddressCount							= 0;
	for (size_t i = 0; i < count; ++i)
	{
		if (0 == stricmp(key[i], FIXED_KEY_ALARM_ID))
			nAlarmId = atoi(data[i]);
		else if (0 == stricmp(key[i], FIXED_KEY_TIME))
		{
			time_t curtime;
			time(&curtime);
			tm logtime = *localtime(&curtime);
			sscanf(data[i], "%2d/%2d %2d:%2d:%2d", &logtime.tm_mon, &logtime.tm_mday,
				&logtime.tm_hour, &logtime.tm_min, &logtime.tm_sec);
			if (12 == logtime.tm_mon)
				logtime.tm_mon = 0;
			curtime = mktime(&logtime);
			nTime = curtime;
		}
		else if (0 == stricmp(key[i], FIXED_KEY_HOSTNAME))
			strHostName = data[i];
		else if (0 == stricmp(key[i], FIXED_KEY_TEXT))
			strText = data[i];
		else if (0 == strnicmp(key[i], FIXED_KEY_ADDRESS, strlen(FIXED_KEY_ADDRESS)))
		{
			std::string strFullAddr(data[i]);
			int nDivPos			= strFullAddr.find(':');
			std::string strAddr	= strFullAddr.substr(0, nDivPos);
			std::string strPort	= strFullAddr.substr(nDivPos+1);
			UdpAddress addTrg(strAddr.c_str());
			addTrg.set_port(atoi(strPort.c_str()));
			pTarget[nAddressCount] = new CTarget(addTrg);
			g_snmpInst.GetTarget(*pTarget[nAddressCount]);
			++nAddressCount;
		}
	}

	Vb vbSeqNumber, vbAlarmId, vbTime, vbHost, vbText;
	vbSeqNumber.set_oid(SEQ_NO_ID);
	vbSeqNumber.set_value(seq);
	vbAlarmId.set_oid(ALARM_ID);
	vbAlarmId.set_value(nAlarmId);
	vbTime.set_oid(TIME_ID);
	vbTime.set_value(nTime);
	vbHost.set_oid(HOST_ID);
	vbHost.set_value(strHostName);
	vbText.set_oid(TEXT_ID);
	vbText.set_value(strText);
	
	Pdu pdu;
	pdu += vbSeqNumber;
	pdu += vbAlarmId;
	pdu += vbTime;
	pdu += vbHost;
	pdu += vbText;

	g_snmpInst.GetPdu(pdu, g_snmpInst.getAddress().c_str());

	bool bRtn = false;
	for (i = 0; i < nAddressCount; ++i)
	{
		SnmpTarget* st = pTarget[i];
		if (g_snmpInst.Send(&pdu, st))
		{
			bRtn = true;
		}
		else
		{
			Log(LogHandler::L_ERROR, "[Action_Proc] can not send data");
		}
	}

	return bRtn;
}
