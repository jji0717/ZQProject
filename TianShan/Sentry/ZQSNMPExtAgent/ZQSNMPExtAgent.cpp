// ZQSNMPExtAgent.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ZQSNMPExtAgent.h"
#include "ZQSnmpMasterAgent.h"
#include "ZQSnmpUtil.h"
#include "zqcfgpkg.h"
#include "FileLog.h"

#define LOG_MODULE_NAME			"ExtAgent"

ZQ::common::Log		_nullLog;
ZQ::common::Log*	_pReporter = &_nullLog;


//////////////////////////////////////////////////////////////////////////

static UINT m_ZQTianShanOidPrefixData[] = {1, 3, 6, 1, 4, 1, 22839, 4};
static AsnObjectIdentifier  m_ZQTianShanOidPrefix = {sizeof(m_ZQTianShanOidPrefixData) / sizeof(UINT), m_ZQTianShanOidPrefixData};
static ZQSnmpMasterAgent    *m_pZQMasterAgent = NULL;

static std::string  m_logfilename;
static DWORD        m_loggingmask = 0;

#define LOGGINGMASK_VERBOSITY 0x0000000F
//////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain( HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved
                      )
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
        if(_pReporter!=&_nullLog)
        {
            delete _pReporter;
            _pReporter = &_nullLog;
        }
        break;
    }
    return TRUE;
}

static bool GetConfig();
// When exported function will be called during DLL loading and initialization
BOOL SNMP_FUNC_TYPE SnmpExtensionInit(DWORD dwUptimeReference,
                                      HANDLE *phSubagentTrapEvent,
                                      AsnObjectIdentifier *pFirstSupportedRegion)
{
    GetConfig();
    if(!m_logfilename.empty())
    {
        DWORD loglevel = m_loggingmask & LOGGINGMASK_VERBOSITY;
        try{
            _pReporter = new ZQ::common::FileLog(m_logfilename.c_str(), loglevel);
        
        }
        catch(ZQ::common::FileLogException&)
        {
            _pReporter = &_nullLog;
        }
    }
    if(m_pZQMasterAgent = new ZQSnmpMasterAgent)
    {
        *phSubagentTrapEvent = NULL;
        *pFirstSupportedRegion = m_ZQTianShanOidPrefix;
        SALOG(Log::L_NOTICE, CLOGFMT(LOG_MODULE_NAME, "===================== SnmpExtensionInit Successful ======================"));
        
        return TRUE;
    }

    SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "===================== SnmpExtensionInit Failed   ======================"));
    return FALSE;
}

// this export is to query the MIB table and fields
BOOL SNMP_FUNC_TYPE SnmpExtensionQuery(BYTE bPduType, 
                                       SnmpVarBindList *pVarBindList, 
                                       AsnInteger32 *pErrorStatus, AsnInteger32 *pErrorIndex)
{
    // SALOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "SnmpExtensionQuery... [pdutype = %s], [VarBindList: length = %u]"), ZQSnmpUtil::GetPduType(bPduType).c_str(), pVarBindList->len);
    if(m_pZQMasterAgent)
    {
        ZQSNMP_STATUS status = m_pZQMasterAgent->ProcessRequest(bPduType, pVarBindList, pErrorStatus, pErrorIndex);

        if(ZQSNMP_E_NOERROR == status)
        {
            SALOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "SnmpExtensionQuery succeed. pdutype[%s], error[%s(%d)]"), ZQSnmpUtil::GetPduType(bPduType).c_str(), ZQSnmpUtil::GetErrorStatus(*pErrorStatus).c_str(), (*pErrorIndex));
            return TRUE;
        }
    }
    
    SALOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "SnmpExtensionQuery failed: pdutype[%s] length[%u] error[%s(%d)]"), ZQSnmpUtil::GetPduType(bPduType).c_str(), pVarBindList->len, ZQSnmpUtil::GetErrorStatus(*pErrorStatus).c_str(), (*pErrorIndex)); 
    return FALSE;
}

// this function just simulate traps
//   Traps just a 2 variables value from MIB
//  Trap is kind of event from server to client
///         When ever the event is set service will call this function and gets the parameters filled.
//       After filling the parameters service willsend the trap to all the client connected
BOOL SNMP_FUNC_TYPE SnmpExtensionTrap(AsnObjectIdentifier *pEnterpriseOid, AsnInteger32 *pGenericTrapId, AsnInteger32 *pSpecificTrapId, AsnTimeticks *pTimeStamp, SnmpVarBindList *pVarBindList)
{
    return FALSE;
}
 
VOID SNMP_FUNC_TYPE SnmpExtensionClose()
{
    if(m_pZQMasterAgent) {
        delete m_pZQMasterAgent;
        m_pZQMasterAgent = NULL;
    }
    SALOG(Log::L_NOTICE, CLOGFMT(LOG_MODULE_NAME, "===================== SnmpExtensionClose ======================"));
}

static bool GetConfig()
{
    std::string logDir;
    DWORD loggingMask = 0;

    DWORD cfg_val = 0;//temp use, not care the value
    HANDLE hCfg = CFG_INITEx("ZQSnmpExtension", &cfg_val, NULL, FALSE);
    if(hCfg)
    {
        DWORD cfg_type  = REG_NONE;
        DWORD bufsize   = 0;
        //LogDir
        {
            char strBuf[MAX_PATH] = {0};
            bufsize = MAX_PATH;

            CFGSTATUS cfgstat = CFG_GET_VALUE(hCfg, "LogDir", (BYTE*)(strBuf), &bufsize, &cfg_type, FALSE);
            if(CFG_SUCCESS == cfgstat && REG_SZ == cfg_type)
            {
                logDir = strBuf;
            }
        }

        //LoggingMask
        {
            DWORD dwBuf = 0;
            bufsize = sizeof(DWORD);

            CFGSTATUS cfgstat = CFG_GET_VALUE(hCfg, "LoggingMask", (BYTE*)(&dwBuf), &bufsize, &cfg_type, FALSE);
            if((CFG_SUCCESS == cfgstat) && (REG_DWORD == cfg_type))
            {
                loggingMask = dwBuf;
            }
        }
    }
    else
    {
        return false;
    }
    CFG_TERM(hCfg);

    //get config data successful
    if(!logDir.empty())
    {
        //construct the full log path
        if(logDir[logDir.size() - 1] == '\\')
        {
            m_logfilename = logDir + "ZQSnmpExtensionAgent.log";
        }
        else
        {
            m_logfilename = logDir + "\\ZQSnmpExtensionAgent.log";
        }
    }

    m_loggingmask = loggingMask;

    return true;
}
