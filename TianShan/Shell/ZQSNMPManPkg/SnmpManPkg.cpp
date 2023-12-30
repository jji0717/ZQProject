#include <cstring>
#include <cstdlib>
#include <syslog.h>
#include "SnmpManPkg.h"
#include "SnmpSubagent.h"
#include "SnmpUtil.h"
#include <sstream>

#include "XMLPreferenceEx.h"
#include "strHelper.h"

typedef struct _MIBExpose
{
	char* varname;
	int   subOid;
} MIBExpose;

#include "MibExpose.cpp" // the definition of gTblMibExpose

namespace {
    const char* CONFIG = "/etc/TianShan.xml";
    SnmpSubagent* _subagent = 0;
}

ZQ::common::FileLog* _snmp_logger = 0;

using namespace ZQ::common;

extern XMLPreferenceEx* getPreferenceNode(const std::string&, XMLPreferenceDocumentEx&); 

ZQSNMP_CALLBACK g_callback = 0;

static void cleanup() {
    if(_subagent) {
        delete _subagent;
        _subagent = 0;
    }
    g_callback = 0;
}

bool SNMPOpenSession(
         const std::string& serviceName,
         uint32_t processOid, 
         ZQSNMP_CALLBACK fCallback,
         bool loggingMask,
         const char* logPath,
		 uint32_t  svcInstanceId) 
{
	syslog(LOG_INFO, "SNMPOpenSession entry.");
    if(serviceName.empty()) 
    {
	    syslog(LOG_WARNING, "SnmpManPkg SNMPOpenSession: service[NULL] OID[%u]", processOid);
        return false;
    }

    g_callback = fCallback;

    if(loggingMask) 
	{
        std::string logFilename = std::string(logPath) + "/" + serviceName + ".snmp.log";
        try{
            _snmp_logger = new FileLog(logFilename.c_str(), Log::L_DEBUG, 3/* filenum */);
        }
        catch(ZQ::common::FileLogException& ex) {
			syslog(LOG_ERR, "SnmpManPkg failed to init log [%s]", logFilename.c_str());
			return false;
        }

        SNMPLOG(Log::L_INFO, CLOGFMT(SnmpManPkg, "SNMPOpenSession: [service: (%s) log: (%s)"),serviceName.c_str(), logFilename.c_str());
    }
    else {
		syslog(LOG_WARNING, "SnmpManPkg SNMPOpenSession: service[%s] OID[%u]", serviceName.c_str(), processOid);
    }

    uint32_t svcId = 0; 

    if(!GetSvcOid(serviceName, svcId))
	{ 
		// this is a bug need to be fixed with SystemShell.cpp(1048): the parameter serviceName should be the daemon name instead of executable name
        return false;
    }

    if(_subagent)
	{
		syslog(LOG_WARNING, "SnmpManPkg SNMPOpenSession: service[%s] OID[%u], Session had been initialized before.", serviceName.c_str(), processOid);
        return false;
    }

    _subagent = new SnmpSubagent(svcId, processOid, svcInstanceId);
	SNMPLOG(Log::L_NOTICE, CLOGFMT(SnmpManPkg,"initialize SnmpSubAgent with SvcId[%u] processOid[%u] instanceId[%u]"), 	svcId,processOid,svcInstanceId);
    if(_subagent)
	{
        if(_subagent->Run())
		{
            SNMPLOG(Log::L_INFO, CLOGFMT(SnmpManPkg, "SNMPOpenSession() Successful"));
            return true;
        }

		cleanup();
    }

    SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpManPkg, "SNMPOpenSession() failed"));
    return false;
}

bool SNMPManageVariable(const char* varName, void* address, uint32_t type, bool readOnly)
{
	return SNMPManageVariable(varName, address, type, readOnly, 0);
}

bool SNMPManageVariable(const char *varName, void *address, uint32_t type, bool readOnly, int subOid)
{
	if(!varName || !address)
		return false;

	if(!_subagent)
	{
		SNMPLOG(Log::L_WARNING, CLOGFMT(SnmpManPkg, "SNMPManageVariable() adding var[%s] failed: NULL subagent"), varName);
		return false;
	}
	
	// fix up the zero subOid by searching the MIB table
	if (subOid <=0 && NULL != gTblMibExpose)
	{
		for (int i=0; gTblMibExpose[i].varname; i++)
		{
			if (0 == stricmp(gTblMibExpose[i].varname, varName))
			{
				subOid = gTblMibExpose[i].subOid;
				SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpManPkg, "SNMPManageVariable() found in MIB: var[%s] subOid[%d]"), varName, subOid);
				break;
			}
		}
	}
	
	if (subOid <=0)
		subOid = 0;

	SNMPLOG(Log::L_INFO, CLOGFMT(SnmpManPkg, "SNMPManageVariable() adding var[%s] subOid[%d] type[%d] readonly[%s]"), varName, subOid, type, (readOnly ? "T" : "F"));

	uint32_t access = readOnly ? ZQSNMP_ACCESS_READONLY : ZQSNMP_ACCESS_READWRITE;
	return _subagent->ManageVariable(varName, address, type, access, subOid);
}

bool SNMPCloseSession()
{
    cleanup();

    if(_snmp_logger)
	{
		delete _snmp_logger;
		_snmp_logger = 0;
	}

    SNMPLOG(Log::L_INFO, CLOGFMT(SnmpManPkg, "SNMPCloseSession() successful"));
    return true;
}

namespace {
bool GetSvcOid(const std::string& serviceName, uint32_t& svcId)
{
   XMLPreferenceDocumentEx doc; 
    if (!doc.open(CONFIG))
	{
        SNMPLOG(Log::L_ERROR, "failed to load configuration from (%s)", CONFIG);
        return false;
    }

    std::ostringstream os;
    os << "SNMP/" << serviceName;
    XMLPreferenceEx* node = getPreferenceNode(os.str().c_str(), doc);
    if(!node)
	{
        SNMPLOG(Log::L_ERROR, "GetSvcOid() failed to get configuration for service[%s]", serviceName.c_str());
        return false;
    }

    char value[256];
    memset(value, '\0', 256);

    bool res = false;

    // enable
    res = node->getAttributeValue("oid", value);
    if (res)
	{
        std::istringstream is(value);
        is >> svcId;
    }
    else
        SNMPLOG(Log::L_ERROR, "failed to get oid for service (%s)", serviceName.c_str());

    node->free();
    return res;
}
}

