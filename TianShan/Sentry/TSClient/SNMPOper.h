
#ifndef _SNMPOPER_
#define _SNMPOPER_

#include <string>
#include <vector>
#include <map>
#include "TsLayout.h"
#include "Log.h"

#ifdef ZQ_OS_LINUX
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#endif

using namespace ZQ::common;

class SNMPOper
{
public:

	typedef struct _SNMPValue
	{
		char	name[SNMPATTR_VARNAME_MAXLEN];
		char	value[SNMPATTR_VARVALUE_MAXLEN];	
		char	oid[SNMPATTR_OID_MAXLEN];
		int		type;				
		int		readonly;
	}SNMPValue;

	SNMPOper();

	~SNMPOper();

	bool init(const char* szIpAddress, 
		ZQ::common::Log* logger = NULL,
		const char* szCommunity = "public", 		
		int nConnectTimeout = 1200,		// in ms
		int nRetryTime = 3);

	void unInit();

	bool getAllVarValue(const char* szOid, std::vector<SNMPValue>& values);
	bool setVarValues(const char* Oids[], const char* Values[], const int types[], int nCount);
	bool setVarValue(const char* Oids, const char* Value, int type);

	//static const char* getStatusError(int nErrorStatus);
private:

#ifdef ZQ_OS_MSWIN
	HANDLE					_handle;
#else
    netsnmp_session*        _session;
#endif

	ZQ::common::Log*		_pLog;
	ZQ::common::Log			_nullLoger;

#define SNLOG				(*_pLog)
};




#endif
