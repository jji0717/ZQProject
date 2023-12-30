// ZQSNMPManPkg.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ZQSNMPManPkg.h"
#include "ZQSnmpSubagent.h"
#include "ZQSnmpUtil.h"
#include "zqcfgpkg.h"
#include "FileLog.h"

#define LOG_MODULE_NAME			"ManPkg"

ZQ::common::Log		_nullLog;
ZQ::common::Log*	_pReporter = &_nullLog;

//////////////////////////////////////////////////////////////////////////
#define LOGGINGMASK_VERBOSITY 0x0000000F
//////////////////////////////////////////////////////////////////////////
//
ZQSNMP_CALLBACK g_callback = NULL;
static ZQSnmpSubagent *m_pSubagent = NULL;
static void cleanup()
{
    if(m_pSubagent)
    {
        delete m_pSubagent;
        m_pSubagent = NULL;
    }
    g_callback = NULL;
}
//////////////////////////////////////////////////////////////////////////
static bool GetSvcOid(const char *szServiceName, const char *szProductName, UINT *pSvcId);
//////////////////////////////////////////////////////////////////////////


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:

            if(_pReporter != &_nullLog)
            {
                delete _pReporter;
                _pReporter = &_nullLog;
            }
            cleanup();
			break;
    }
    return TRUE;
}

//need to support multiple session?
bool SNMPOpenSession(
                     const char *szServiceName,
                     const char *szProductName,
                     UINT processOid,
					 ZQSNMP_CALLBACK fCallback/* = NULL */,
                     const char *szLogFileName/* = NULL */,
                     DWORD dwLoggingMask/* = 0 */,
                     int nLogFileSize/* = 0xA00000 */,
					 DWORD nProcessInstanceId/* = 0 */
                     )
{
    if(NULL == szServiceName || NULL == szProductName)
        return false;

    //////////////////////////////////////////////////////////////////////////
    //get service oid
    UINT svcid = 0; 
    if(!GetSvcOid(szServiceName, szProductName, &svcid))
        return false;
    //////////////////////////////////////////////////////////////////////////

    //set callback
    g_callback = fCallback;

    //create FileLog object
    if(szLogFileName)
    {
        DWORD loglevel = dwLoggingMask & LOGGINGMASK_VERBOSITY;
        try{
            _pReporter = new ZQ::common::FileLog(szLogFileName, loglevel, 3/* filenum */, nLogFileSize);
        }
        catch(ZQ::common::FileLogException& ex)
        {
            _pReporter = &_nullLog;
        }
    }
    else
    {
        //output log to null log if no log file name specified
        _pReporter = &_nullLog;
    }
    SMLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "SNMPOpenSession. [service name = %s], [product name = %s], [logfilepath = %s]"), szServiceName, szProductName, szLogFileName);

    //allocate resource and initialize it
    if(m_pSubagent)
    {
        //the session had been initialized
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Session had been initialized before."));
        return false;
    }

	UINT serviceInstanceId = nProcessInstanceId;
    m_pSubagent = new ZQSnmpSubagent(svcid, processOid, serviceInstanceId);
    if(m_pSubagent)
    {
        if(m_pSubagent->Run())
        {
            SMLOG(Log::L_NOTICE, CLOGFMT(LOG_MODULE_NAME, "===================== SNMPOpenSession Successful ======================"));
            return true;
        }
        else
        {
            SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ZQSnmpSubagent::Run failed."));
            cleanup();
        }
    }

    SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "===================== SNMPOpenSession Failed ======================"));
    return false;
}

bool SNMPManageVariable(const char *szVarName, void *address, DWORD type, BOOL bReadOnly, UINT instId)
{
    if(NULL == szVarName || NULL == address)
        return false;

    SMLOG(Log::L_NOTICE, CLOGFMT(LOG_MODULE_NAME, "SNMPManageVariable. [varname = %s], [type = %s], [readonly = %s]"), szVarName, ZQSnmpUtil::GetVariableType(type).c_str(), bReadOnly ? "True" : "False");
    if(m_pSubagent)
    {
        DWORD access = bReadOnly ? ZQSNMP_ACCESS_READONLY : ZQSNMP_ACCESS_READWRITE;
        return m_pSubagent->ManageVariable(szVarName, address, type, access, instId);
    }
    else
    {
        //must open session first
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "SNMPManageVariable Failed. Session is closed."));
        return false;
    }
}

bool SNMPCloseSession()
{
    cleanup();

    SMLOG(Log::L_NOTICE, CLOGFMT(LOG_MODULE_NAME, "===================== SNMPCloseSession Successful ======================"));
    return true;
}

static bool GetSvcOid(const char *szServiceName, const char *szProductName, UINT *pSvcId)
{
    if(NULL == szServiceName || NULL == szProductName || NULL == pSvcId)
        return false;

    //std::string svcShellName = szServiceName;
    //svcShellName += "_shell";

    HANDLE hCfg = NULL;
    DWORD cfg_value;
    //hCfg = CFG_INITEx((char *)svcShellName.c_str(), &cfg_value, (char *)szProductName);
    hCfg = CFG_INITEx((char *)szServiceName, &cfg_value, "SNMPOID");
    if(NULL == hCfg)
        return false;
    DWORD cfg_size = sizeof(DWORD);
    DWORD cfg_type = REG_NONE;
    //CFGSTATUS cfgstat = CFG_GET_VALUE(hCfg, "OID", (BYTE *)(&cfg_value), &cfg_size, &cfg_type);
    CFGSTATUS cfgstat = CFG_GET_VALUE(hCfg, "ServiceOID", (BYTE *)(&cfg_value), &cfg_size, &cfg_type);

    CFG_TERM(hCfg);

    if(CFG_SUCCESS == cfgstat && REG_DWORD == cfg_type)
    {
        *pSvcId = cfg_value;
        return true;
    }
    return false;
}
