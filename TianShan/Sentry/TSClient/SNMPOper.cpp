

#pragma warning(disable:4786)

#include "Log.h"
#include "SNMPOper.h"
#include "VarCache.h"

#ifdef ZQ_OS_MSWIN
#include "StdAfx.h"
#include "windows.h"
#include <snmp.h>
#include <mgmtapi.h>
#include "ZQSnmpUtil.h"
#else
#include <net-snmp/net-snmp-config.h>
#include "SnmpUtil.h"
#endif

#define LOG_MODULE_NAME			"SNMP"


SNMPOper::SNMPOper()
{
	_pLog = &_nullLoger;
}

SNMPOper::~SNMPOper()
{
	unInit();
}

#ifdef ZQ_OS_MSWIN

//////////////////////////////////////////////////////////////////////////
static std::string getMgmtError(DWORD errcode)
{
    switch(errcode)
    {
    case SNMP_MGMTAPI_TIMEOUT:
        return "SNMP_MGMTAPI_TIMEOUT";
    case SNMP_MGMTAPI_SELECT_FDERRORS:
        return "SNMP_MGMTAPI_SELECT_FDERRORS";
    case SNMP_MGMTAPI_TRAP_ERRORS:
        return "SNMP_MGMTAPI_TRAP_ERRORS";
    case SNMP_MGMTAPI_TRAP_DUPINIT:
        return "SNMP_MGMTAPI_TRAP_DUPINIT";
    case SNMP_MGMTAPI_NOTRAPS:
        return "SNMP_MGMTAPI_NOTRAPS";
    case SNMP_MGMTAPI_AGAIN:
        return "SNMP_MGMTAPI_AGAIN";
    case SNMP_MGMTAPI_INVALID_CTL:
        return "SNMP_MGMTAPI_INVALID_CTL";
    case SNMP_MGMTAPI_INVALID_SESSION:
        return "SNMP_MGMTAPI_INVALID_SESSION";
    case SNMP_MGMTAPI_INVALID_BUFFER:
        return "SNMP_MGMTAPI_INVALID_BUFFER";
    default:
        return "Unknown error code";
    }
}
//////////////////////////////////////////////////////////////////////////

bool SNMPOper::init(const char* szIpAddress, 
		ZQ::common::Log* logger,
		const char* szCommunity, 		
		int nConnectTimeout,
		int nRetryTime)
{
	if (logger)
		_pLog = logger;

	SNLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Connect to SNMP ip[%s], community[%s], timeout[%d]ms, retry[%d]times"),
		szIpAddress, szCommunity, nConnectTimeout, nRetryTime);

	DWORD dwTime1 = GetTickCount();
	_handle = SnmpMgrOpen((char*)szIpAddress, (char*)szCommunity, nConnectTimeout, nRetryTime);
	if (_handle == NULL)
	{
		DWORD dwRet = GetLastError();
        /*
		char msgBuf[512] = {0};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL, dwRet, 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msgBuf, 511,NULL );
		msgBuf[511] = '\0';
        */
		SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to connect to SNMP %s, error code [%d], %s, spent %d ms"), 
			szIpAddress, dwRet, getMgmtError(dwRet).c_str(), GetTickCount()-dwTime1);
		return false;
	}
	SNLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Success to SnmpMgrOpen %s, spent %d ms"), szIpAddress, GetTickCount()-dwTime1);	
	
	return true;
}

void SNMPOper::unInit()
{
	if (_handle)
	{
		SNLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Closing the SNMP connection..."));
		SnmpMgrClose(_handle);
		_handle = NULL;
		SNLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "SNMP connection closed"));
		_pLog = &_nullLoger;
	}	
}
/*
const char* SNMPOper::getStatusError(int nErrorStatus)
{	
	const char* szErrorMsg = NULL;
	switch(nErrorStatus)
	{
	case SNMP_ERRORSTATUS_NOERROR:
		szErrorMsg = "SNMP_ERRORSTATUS_NOERROR";
		break;
	case SNMP_ERRORSTATUS_TOOBIG:
		szErrorMsg = "SNMP_ERRORSTATUS_TOOBIG";
		break;
	case SNMP_ERRORSTATUS_NOSUCHNAME:
		szErrorMsg = "SNMP_ERRORSTATUS_NOSUCHNAME";
		break;
	case SNMP_ERRORSTATUS_BADVALUE:
		szErrorMsg = "SNMP_ERRORSTATUS_BADVALUE";
		break;
	case SNMP_ERRORSTATUS_READONLY:
		szErrorMsg = "SNMP_ERRORSTATUS_READONLY";
		break;
	case SNMP_ERRORSTATUS_GENERR:
		szErrorMsg = "SNMP_ERRORSTATUS_GENERR";
		break;
	case SNMP_ERRORSTATUS_NOACCESS:
		szErrorMsg = "SNMP_ERRORSTATUS_NOACCESS";
		break;
	case SNMP_ERRORSTATUS_WRONGTYPE:
		szErrorMsg = "SNMP_ERRORSTATUS_WRONGTYPE";
		break;
	case SNMP_ERRORSTATUS_WRONGLENGTH:
		szErrorMsg = "SNMP_ERRORSTATUS_WRONGLENGTH";
		break;
	case SNMP_ERRORSTATUS_WRONGENCODING:
		szErrorMsg = "SNMP_ERRORSTATUS_WRONGENCODING";
		break;
	case SNMP_ERRORSTATUS_WRONGVALUE:
		szErrorMsg = "SNMP_ERRORSTATUS_WRONGVALUE";
		break;
	case SNMP_ERRORSTATUS_NOCREATION:
		szErrorMsg = "SNMP_ERRORSTATUS_NOCREATION";
		break;
	case SNMP_ERRORSTATUS_INCONSISTENTVALUE:
		szErrorMsg = "SNMP_ERRORSTATUS_INCONSISTENTVALUE";
		break;
	case SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE:
		szErrorMsg = "SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE";
		break;
	case SNMP_ERRORSTATUS_COMMITFAILED:
		szErrorMsg = "SNMP_ERRORSTATUS_COMMITFAILED";
		break;
	case SNMP_ERRORSTATUS_UNDOFAILED:
		szErrorMsg = "SNMP_ERRORSTATUS_UNDOFAILED";
		break;
	case SNMP_ERRORSTATUS_AUTHORIZATIONERROR:
		szErrorMsg = "SNMP_ERRORSTATUS_AUTHORIZATIONERROR";
		break;
	case SNMP_ERRORSTATUS_NOTWRITABLE:
		szErrorMsg = "SNMP_ERRORSTATUS_NOTWRITABLE";
		break;
	case SNMP_ERRORSTATUS_INCONSISTENTNAME:
		szErrorMsg = "SNMP_ERRORSTATUS_INCONSISTENTNAME";
		break;
	default:
		szErrorMsg = "Unknown Error";
	}
	return szErrorMsg;
}
*/
//we use the string value to transport the message
bool SNMPOper::getAllVarValue(const char* szOid, std::vector<SNMPValue>& values)
{
    std::string valueOid = szOid;
    std::string strNameOid  = std::string(szOid) + ".2";
    bool bStartTraverseName = false;
    bool bEndOfTraverse = false;

    if(NULL == szOid)
        return false;

    //initialize start oid
    AsnObjectIdentifier startOid = {0};
    if(!SnmpMgrStrToOid((char *)szOid, &startOid))
    {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to convert [%s] to OID"), szOid);
        return false;
    }

    AsnObjectIdentifier nameOid = {0};
    if(!SnmpMgrStrToOid((char *)strNameOid.c_str(), &nameOid))
    {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to convert [%s] to OID"), szOid);
        return false;
    }

    UINT svcid = 0;
    if(!ZQSnmpUtil::ParseOid(&startOid, &svcid, NULL, NULL, NULL))
    {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Invalid oid. [oid = %s]"), SnmpUtilOidToA(&startOid));
        SnmpUtilOidFree(&startOid);
        return false;
    }
    //initialize vbl
    SnmpVarBindList vbl = {0};
    vbl.len = 1;
    vbl.list = (SnmpVarBind *)SnmpUtilMemAlloc(sizeof(SnmpVarBind) * vbl.len);
    if(NULL == vbl.list)
    {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "SnmpUtilMemAlloc failed. [memsize = %u]"), sizeof(SnmpVarBind) * vbl.len);

        SnmpUtilOidFree(&startOid);
        return false;
    }
    if(!SnmpUtilOidCpy(&(vbl.list[0].name), &startOid))
    {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "SnmpUtilOidCpy failed."));

        SnmpUtilMemFree(&(vbl.list));
        SnmpUtilOidFree(&startOid);
        return false;
    }
    vbl.list[0].value.asnType = ASN_NULL;

    VarCache varcache(svcid);
    //get all value under start oid
    UINT requestCount = 0;//for log
    while(!bEndOfTraverse)
    {
        AsnInteger32 errstat = SNMP_ERRORSTATUS_NOERROR;
        AsnInteger32 erridx  = 0;
        //send request
        ++requestCount;
        if(!SnmpMgrRequest(_handle, SNMP_PDU_GETNEXT, &vbl, &errstat, &erridx))
        {
            DWORD error = GetLastError();
            SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "SnmpMgrRequest failed. [error = %s], [request count = %u]"), getMgmtError(error).c_str(), requestCount);
            break;
        }
        if(SNMP_ERRORSTATUS_NOERROR != errstat)
        {
            if(SNMP_ERRORSTATUS_NOSUCHNAME == errstat)
            {
                if (!bStartTraverseName)
                { // start traverse name
                    bStartTraverseName = true;
                    if(!SnmpUtilOidCpy(&(vbl.list[0].name), &nameOid))
                    {
                        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "SnmpUtilOidCpy failed."));

                        SnmpUtilMemFree(&(vbl.list));
                        SnmpUtilOidFree(&startOid);
                        return false;
                    }
                    continue;
                }else{
                    bEndOfTraverse = true;
                    SNLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "No more vars, stop request. [request count = %u]"), requestCount);
                }             
            }            
            else
            {
                SNLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "Unexpected error status. [ErrorStatus = %s], [request count = %u]"), ZQSnmpUtil::GetErrorStatus(errstat).c_str(), requestCount);
            }
        }

        if(0 == SnmpUtilOidNCmp(&(vbl.list[0].name), &startOid, startOid.idLength))
        {
            //parse response
            if (vbl.list[0].name.idLength <= ZQSNMP_OID_LEN_VARINSTANCE)
            {
                if(!varcache.Add(vbl.list))
                {
                    SNLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "Fail to cache var. [oid = %s], [request count = %u]"), SnmpUtilOidToA(&(vbl.list[0].name)), requestCount);
                }
            }     
        }
        else
        {
            //out of scope
            SNLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Out of scope. [start oid = %s, current oid = %s], [request count = %u]"), szOid, SnmpUtilOidToA(&(vbl.list[0].name)), requestCount);
            break;
        }
        //value's asnType field will be set to ASN_NULL during the api call
        SnmpUtilAsnAnyFree(&(vbl.list[0].value));
    }

    varcache.Get(values);
    //cleanup
    SnmpUtilVarBindListFree(&vbl);
    SnmpUtilOidFree(&startOid);

    return true;
}


bool SNMPOper::setVarValue(const char* szOid, const char* Value, int type)
{
	SnmpVarBindList snmpVarList;
	snmpVarList.len = 1;
	snmpVarList.list = (SnmpVarBind *)SnmpUtilMemAlloc(sizeof(SnmpVarBind)*snmpVarList.len); 
	
	bool bDone = false;
	do{
		snmpVarList.list[0].value.asnType = ASN_NULL;
		
		if (!SnmpMgrStrToOid((char*)szOid, &snmpVarList.list[0].name))
		{			
			SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to convert [%s] to OID"), szOid);
			break;
		}
	
        if(SNMPATTR_VARTYPE_INT == type)
        {
            snmpVarList.list[0].value.asnValue.number = atoi(Value);
            snmpVarList.list[0].value.asnType = ASN_INTEGER32;
        }
        else if(SNMPATTR_VARTYPE_STRING == type)
        {
            uint32 len = (uint32)strlen(Value);
            BYTE *data = NULL;
            BOOL dynamic = FALSE;
            if(len)
            {
                data = (BYTE *)SnmpUtilMemAlloc(len);
                if(NULL == data)
                {
                    SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "SnmpUtilMemAlloc failed. [memsize = %u]"), len);
                    break;
                }
                memcpy(data, Value, len);
                dynamic = TRUE;
            }
            snmpVarList.list[0].value.asnValue.string.stream = data;
            snmpVarList.list[0].value.asnValue.string.length = len;
            snmpVarList.list[0].value.asnValue.string.dynamic = dynamic;
            snmpVarList.list[0].value.asnType = ASN_OCTETSTRING;
        }
        else
        {
            SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Unexpected vartype. [vartype = %d]"), type);
            break;
        }
		
		AsnInteger	errorStatus=0;  	// Error type that is returned if encountered
		AsnInteger	errorIndex=0;		// Works with variable above
		DWORD dwRet = SnmpMgrRequest(_handle, SNMP_PDU_SET, &snmpVarList,&errorStatus,&errorIndex);
		if (!dwRet)
		{		
			DWORD dwRet = GetLastError();

			SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to SnmpMgrRequest for [%s], error code [%d], %s"), 
				szOid, dwRet, getMgmtError(dwRet).c_str());

			break;
		}

		if (errorStatus)
		{
            SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to set [%s], [error status = %s]"), szOid, ZQSnmpUtil::GetErrorStatus(errorStatus).c_str());
			break;
		}

		bDone = true;
	}while(0);
	
	SnmpUtilVarBindListFree(&snmpVarList);	

	return bDone;
}

#else

bool SNMPOper::init(const char* addr, ZQ::common::Log* logger, const char* community, int nConnectTimeout, int nRetryTime) { 
	if (logger) {
		_pLog = logger;
    }

	SNLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Connect to SNMP"));

    init_snmp("TianShanApp");

    netsnmp_session ss;
    snmp_sess_init(&ss);

    ss.version = SNMP_VERSION_1;
    ss.peername = strdup(addr);
    ss.community = (unsigned char*)community;
    ss.community_len = strlen(community);


    SOCK_STARTUP;

	_session = snmp_open(&ss);
	if (_session == NULL) {
		SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "fail to connect to SNMP"));
        snmp_close(_session);
        _session = 0;

        SOCK_CLEANUP;

		return false;
	}
	SNLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Connect to Snmp successfully"));	
	
	return true;
}

void SNMPOper::unInit() {
	if (_session) {
		SNLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Closing the SNMP connection..."));
		snmp_close(_session);
		_session = 0;
		SNLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "SNMP connection closed"));
		_pLog = &_nullLoger;
	}	
    SOCK_CLEANUP;
}

bool SNMPOper::getAllVarValue(const char* szOid, std::vector<SNMPValue>& values) {
    if(!szOid){
        return false;
    }

    std::string valueOid = szOid;
    std::string strNameOid  = std::string(szOid) + ".2";
    bool bStartTraverseName = false;
    bool bEndOfTraverse = false;

    oid startOid[MAX_OID_LEN];
    size_t len = MAX_OID_LEN;
    if(!snmp_parse_oid(szOid, startOid, &len)) {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to convert [%s] to OID"), szOid);
        return false;
    }

    uint32_t svcid, processoid, varinfotype, varinstid;
    if(!ZQSNMP::Util::parseOid(startOid, len, svcid, processoid, varinfotype, varinstid)) {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Invalid oid. [oid = %s]"), szOid);
        return false;
    }

    VarCache varcache(svcid);

    int requestCount = 0;

    netsnmp_pdu *pdu, *response;
    netsnmp_variable_list* vars;

    while(!bEndOfTraverse) {
        pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
        snmp_add_null_var(pdu, startOid, len);

        ++requestCount;

        int status = snmp_synch_response(_session, pdu, &response);
        if(status != STAT_SUCCESS) {
            char buff[255];
            memset(buff, '\0', 255);
            char* p = buff;
            int snmp_err = 0;
            snmp_error(_session, 0, &snmp_err, &p);
            
            SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, 
                  "Request failed. error(%s), snmp_error(%d), request(%d)"), buff, snmp_err, requestCount);
            break;
        }
        
        if(response->errstat != SNMP_ERR_NOERROR) {
            if (response->errstat == SNMP_ERR_NOSUCHNAME)
            {
                if (!bStartTraverseName)
                { // start traverse name
                    bStartTraverseName = true;
                    if(!snmp_parse_oid(strNameOid.c_str(), startOid, &len)) {
                        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to convert [%s] to OID"), strNameOid.c_str());
                        return false;
                    }
                    continue;
                }else{
                    bEndOfTraverse = true;
                    SNLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "No more vars, stop request. [request count = %u]"), requestCount);
                } 
            }else{
                SNLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "Error in response: status(%d), error(%s) request(%d)"), response->errstat, snmp_errstring(response->errstat), requestCount);
            }
        }

        vars = response->variables;
        if(snmp_oid_compare(startOid, len, vars->name, vars->name_length) < 0) {
            if (vars->name_length <= ZQSNMP_OID_LEN_VARINSTANCE)
            {
                if(!varcache.Add(vars)) {
                    SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "add cache failed."));
                }
            }

            memmove((char*)startOid, (char*)vars->name, vars->name_length*sizeof(oid));
            len = vars->name_length;
        }
        else {
            break;
        }

        if(response) {
            snmp_free_pdu(response);
            response = 0;
        }
     }

    if(response) {
        snmp_free_pdu(response);
        response = 0;
    }
    varcache.Get(values);

    SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "value size: %d."), values.size());
    return true;
}

bool SNMPOper::setVarValue(const char* szOid, const char* value, int type) {
    if(!szOid) {
        return false;
    }

	oid startOid[MAX_OID_LEN];
    size_t len = MAX_OID_LEN;
    if(!snmp_parse_oid(szOid, startOid, &len)) {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to convert [%s] to OID"), szOid);
        return false;
    }

    if(type != SNMPATTR_VARTYPE_INT && type != SNMPATTR_VARTYPE_STRING) {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Unexpected vartype. [vartype = %d]"), type);
        return false;
    }
    
    netsnmp_pdu *pdu, *response;
    pdu = response = 0;
    
    pdu = snmp_pdu_create(SNMP_MSG_SET);
    if(snmp_add_var(pdu, startOid, len, type, value)) {
        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to add var")); 
        return false;
    }

    int status = snmp_synch_response(_session, pdu, &response);
    if(status != STAT_SUCCESS) {
        char buff[255];
        char* p = buff;
        int snmp_err = 0;
        snmp_error(_session, 0, &snmp_err, &p);

        SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, 
              "Request failed. error(%s), snmp_error(%d)"), buff, snmp_err);
        
        if(response) snmp_free_pdu(response);
        return false;
    }

    if(response->errstat != SNMP_ERR_NOERROR) {
        SNLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, 
             "Error in response: status(%d), error(%s)"), 
             response->errstat, snmp_errstring(response->errstat));
        if(response) snmp_free_pdu(response);
        return false;
    }

    if(response) snmp_free_pdu(response);
	return true;
}

#endif

bool SNMPOper::setVarValues(const char* szOid[], const char* Values[], const int types[], int nCount)
{
    SNLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "setVarValues. [var count = %d]"), nCount);
            
    for(int i = 0; i < nCount; ++i)
    {
        if(!setVarValue(szOid[i], Values[i], types[i]))
        {
			SNLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Encounter error during setVarValues. [oid = %s], [index = %d, total = %d]"), szOid[i], i, nCount);
            return false;
        }
    }
    return true;
}
