// ZQSnmpMasterAgent.cpp: implementation of the ZQSnmpMasterAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZQSnmpMasterAgent.h"
#include "ZQSnmpUtil.h"

#include "UDPSocket.h"
#include "SnmpInteractive.h"


#define LOG_MODULE_NAME         "MasterAgent"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ZQSnmpMasterAgent::ZQSnmpMasterAgent()
{
	agentRecvCount_  = 0;
	snmpUdpBasePort_ = 10000;
	snmpAgentPendTimeOut_  =  5000;
    regionTbl_.refresh(snmpUdpBasePort_, snmpAgentPendTimeOut_);
}

ZQSnmpMasterAgent::~ZQSnmpMasterAgent()
{

}

ZQSNMP_STATUS ZQSnmpMasterAgent::ProcessRequest(BYTE pdutype, SnmpVarBindList *pVbl, AsnInteger32 *pErrStat, AsnInteger32 *pErrIdx)
{
    if(NULL == pVbl || NULL == pErrStat)
        return ZQSNMP_E_FAILURE;

    *pErrStat = SNMP_ERRORSTATUS_NOERROR;
    *pErrIdx = 0;
    for(UINT i = 0; i < pVbl->len; ++i)
    {
        SALOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "VarBindList: [index = %u, oid = %s]"), i, SnmpUtilOidToA(&(pVbl->list[i].name)));
        MibRegion region = {0};
        //get region
        if(!getRegion(&(pVbl->list[i]), &region))
        {
            *pErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            *pErrIdx = i + 1;//
            break;
        }

        do
        {
            //dispatch request
            ZQSNMP_STATUS status = dispatchRequest(region, pdutype, &(pVbl->list[i]), pErrStat);
            if(ZQSNMP_E_NOERROR != status)
            {
                SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "dispatchRequest Failed"));

                *pErrStat = SNMP_ERRORSTATUS_GENERR;
            }

        }while(
            SNMP_PDU_GETNEXT == pdutype &&
            *pErrStat == SNMP_ERRORSTATUS_NOSUCHNAME &&
            nextRegion(&region)
            );

        if(*pErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
            *pErrIdx = i + 1;//

            break;
        }
    }

    return ZQSNMP_E_NOERROR;
}

bool ZQSnmpMasterAgent::getRegion(const SnmpVarBind *pVb, MibRegion *pRegion)
{
    if(NULL == pVb || NULL == pRegion)
        return false;

    UINT svcid, svcprocess;
    if(ZQSnmpUtil::ParseOid(&(pVb->name), &svcid, &svcprocess, NULL, NULL))
    {
        pRegion->m_svcId = svcid;
        pRegion->m_svcProcess = svcprocess;

        return true;
    }
    else
    {
        return false;
    }
}
bool ZQSnmpMasterAgent::nextRegion(MibRegion *pRegion)
{
    if(NULL == pRegion)
        return false;
    return regionTbl_.next(*pRegion);
}
ZQSNMP_STATUS ZQSnmpMasterAgent::dispatchRequest(MibRegion targetRegion, BYTE pdutype, SnmpVarBind *pVb, AsnInteger32 *pErrStat)
{
    if(NULL == pVb || NULL == pErrStat)
        return ZQSNMP_E_FAILURE;

    SALOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "dispatchRequest. [target region: service id = %u, process = %u]"), targetRegion.m_svcId, targetRegion.m_svcProcess);

    //encode request message
    std::string requestMsg;
    bool bSuccess = ZQSnmpUtil::EncodeMsg(pdutype, SNMP_ERRORSTATUS_NOERROR, pVb, &requestMsg);
    if(!bSuccess)
    {
        SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "EncodeMsg failed."));
        return ZQSNMP_E_FAILURE;
    }
    if(requestMsg.size() > ZQSNMP_MSG_LEN_MAX)
    {
        SALOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "message size is too big. [message size = %u]"), requestMsg.size());

        *pErrStat = SNMP_ERRORSTATUS_TOOBIG;
        return ZQSNMP_E_NOERROR;
    }
    //send request message and get response
    //////////////////////////////////////////////////////////////////////////
    //get service's pipe name
	DWORD nResponseMsg = 0;
	BYTE  responseMsgBuf[ZQSNMP_NP_BUFSIZE] = {0};

	timeout_t        timeout         = snmpAgentPendTimeOut_;
	int32            snmpServerPort  = targetRegion.m_svcId + targetRegion.m_svcProcess + snmpUdpBasePort_;
	unsigned long    requestLinePos  = (unsigned long) &((((struct InterActive*) 0)->_content).request);
	InetHostAddress  udpSvcAddress("127.0.0.1");
	UDPSocket        udpAgnet;

	udpAgnet.setCompletion(false);
	//		SALOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "udpSvcAddress[%s]  snmpServerPort[%d]"), udpSvcAddress.getHostAddress(), snmpServerPort);

	if (!udpAgnet.isPending(Socket::pendingOutput, timeout))
	{
		SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "agent Output timeout[%d]"), timeout);
		return ZQSNMP_E_FAILURE;
	}

	{
		// SALOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "sending to snmpServerPort[%d] udpSvcAddress[%s]"), snmpServerPort, udpSvcAddress.getHostAddress());
		InterActive sendToService = {0};
		sendToService._head._agentRecvSeq = agentRecvCount_;
		memcpy((sendToService._content.request), requestMsg.data(), requestMsg.size());
		int sentState = udpAgnet.sendto(&sendToService, requestMsg.size() + requestLinePos, udpSvcAddress, snmpServerPort);
		if (0 >= sentState)
		{
			SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed send to udpSvcAddress[%s], snmpServerPort[%d] buff[%s]"), udpSvcAddress.getHostAddress(), snmpServerPort, requestMsg.data());
			return ZQSNMP_E_FAILURE;
		}
	}

	if (!udpAgnet.isPending(Socket::pendingInput, timeout))
	{
		SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "agent processing snmpServerPort[%d], output timeout[%d] failed"), snmpServerPort, timeout);
		return ZQSNMP_E_FAILURE;
	}

	{
		// SALOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME,  "receiving from snmpServerPort[%d] udpSvcAddress[%s]"), snmpServerPort, udpSvcAddress.getHostAddress());
		int          reslen          = 0;
		InterActive  recvFromService = {0};
		reslen = udpAgnet.receiveFrom(&recvFromService, sizeof(InterActive), udpSvcAddress, snmpServerPort);
		if (0 >= reslen || (recvFromService._head._agentRecvSeq) != agentRecvCount_
			|| 0 >= (reslen - requestLinePos))
		{
			SALOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "snmp recv err udpserver[%d], snmpServerPort[%d], agentRecvCount[%d]"), udpSvcAddress.getHostAddress(), snmpServerPort, agentRecvCount_);
			return ZQSNMP_E_FAILURE;
		}

		++agentRecvCount_;
		nResponseMsg = reslen - requestLinePos;
		memcpy(responseMsgBuf, (recvFromService._content.request), nResponseMsg);
		// SALOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "response from subagent reslen[%u], nResponseMsg[%d], agentRecvCount[%d]"), reslen, nResponseMsg, agentRecvCount_);
	}

    SnmpVarBind responseVb = {0};
    //decode message
    //pdutype is used as a temporary variable here.
    bSuccess = ZQSnmpUtil::DecodeMsg(responseMsgBuf, nResponseMsg, &pdutype, pErrStat, &responseVb);
    if(!bSuccess)
    {
        SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "DecodeMsg failed."));
        return ZQSNMP_E_FAILURE;
    }

    //update varbind
    SnmpUtilVarBindFree(pVb);
    *pVb = responseVb;

    
    SALOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "dispatchRequest successful. [error status = %s]"), ZQSnmpUtil::GetErrorStatus(*pErrStat).c_str());

    return ZQSNMP_E_NOERROR;
}

// RegionTable
bool ZQSnmpMasterAgent::RegionTable::next(MibRegion& r) const
{
    for(size_t iSvc = 0; iSvc < svcs_.size(); ++iSvc)
    {
        const Svc& svc = svcs_[iSvc];
        if(svc.id == r.m_svcId)
        {
            for(size_t iMod = 0; iMod < svc.mods.size(); ++iMod)
            {
                const Mod& mod = svc.mods[iMod];
                if(mod.id > r.m_svcProcess)
                {
                    r.m_svcProcess = mod.id;
                    return true;
                } // else continue;
            }
            // not in this service
        }
        else if( svc.id > r.m_svcId)
            break;
#pragma message(__MSGLOC__"Not support the cross-service travel")
        /*
        { // get the first module
            if(!svc.mods.empty())
            {
                r.m_svcId = svc.id;
                r.m_svcProcess = svc.mods[0].id;
                return true;
            } // else continue;
        } // else continue;
        */
    }
    return false;
}
template<typename SeqT, typename ElemT, typename MemP>
void orderedInsert(SeqT& seq, const ElemT& el, MemP field, bool unique = true)
{
    SeqT::iterator it;
    for(it = seq.begin(); it != seq.end(); ++it)
    {
        if(el.*field < (*it).*field)
        {
            seq.insert(it, el);
            return;
        }
        else if(el.*field == (*it).*field)
        {
            if(unique)
                return;
        }
    }
    seq.insert(seq.end(), el);
}

bool ZQSnmpMasterAgent::RegionTable::refresh(uint32& snmpUdpBasePort, timeout_t& snmpAgentPendTimeOut)
{
    svcs_.clear();
    const char* root = "SOFTWARE\\ZQ Interactive\\SNMPOID\\CurrentVersion\\Services";

    HKEY hRoot;
    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, root, 0, KEY_READ, &hRoot))
    {
        SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "RegionTable::refresh() Failed to open %s"), root);
        return false;
    }

    bool ret = true; 

    // enumerate the services
    DWORD     keyIndex = 0;
    const int BUFSIZE = 256;
	DWORD type;
	DWORD pendType;
	char  value[BUFSIZE];
	char  svcName[BUFSIZE] = {0};
	DWORD nName            = BUFSIZE - 1;
	DWORD nValue           = BUFSIZE - 1;

	if(ERROR_SUCCESS == RegQueryValueEx(hRoot, "SnmpUdpBasePort", NULL, &type, (LPBYTE)value, &nValue) 
		&& REG_DWORD == type)
	{
		const int portMaxLimit = 65535;
		snmpUdpBasePort = *(uint32 *)value;
		snmpUdpBasePort = (snmpUdpBasePort < portMaxLimit) ? snmpUdpBasePort : 10000;
		SALOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "refresh() SnmpUdpBasePort[%d] RegQueryValueEx succeed"), snmpUdpBasePort);
	}else{
		SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "refresh() SnmpUdpBasePort[%d] RegQueryValueEx failed"), snmpUdpBasePort);
	}

	nValue = BUFSIZE - 1;
	memset(value, 0, sizeof(value));
	if(ERROR_SUCCESS == RegQueryValueEx(hRoot, "SnmpAgentPendTimeOut", NULL, &pendType, (LPBYTE)value, &nValue) 
		&& REG_DWORD == pendType)
	{
		snmpAgentPendTimeOut = *(uint32 *)value;
		snmpAgentPendTimeOut = (snmpAgentPendTimeOut > 0) ? snmpAgentPendTimeOut : 5000;
		SALOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "refresh() SnmpAgentPendTimeOut[%d] RegQueryValueEx succeed"), snmpAgentPendTimeOut);
	}else{
		SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "refresh() SnmpAgentPendTimeOut[%d] RegQueryValueEx failed"), snmpAgentPendTimeOut);
	}

    while(ERROR_SUCCESS == RegEnumKeyEx(hRoot, keyIndex, svcName, &nName, NULL, NULL, NULL, NULL))
    {
        HKEY hSvc;
        if(ERROR_SUCCESS == RegOpenKeyEx(hRoot, svcName, 0, KEY_READ, &hSvc))
        {
			type = 0;
			nValue = BUFSIZE - 1;
			memset(value, 0, sizeof(value));
            // get the service oid
            if(ERROR_SUCCESS == RegQueryValueEx(hSvc, "ServiceOID", NULL, &type, (LPBYTE)value, &nValue) && REG_DWORD == type)
            {
                Svc svc;
                svc.id = *(DWORD*)value;
                svc.name = svcName;
                SALOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "RegionTable::refresh() Get OID %d of service %s"), svc.id, svc.name.c_str());

                // enumerate the modules
                DWORD entryIndex = 0;
                char entry[BUFSIZE];
                entry[BUFSIZE - 1] = '\0';

                DWORD nEntry = BUFSIZE - 1;
                nValue = BUFSIZE - 1;
                while(ERROR_SUCCESS == RegEnumValue(hSvc, entryIndex, entry, &nEntry, NULL, &type, (LPBYTE)&value, &nValue))
                {
                    if(REG_DWORD == type && 0 == strncmp(entry, "mod", 3))
                    { // insert the module in to the proper position
                        DWORD modId = *(DWORD*)value;
                        SALOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "RegionTable::refresh() Get OID %d of modude %s, service=%s"), modId, entry, svc.name.c_str());
                        orderedInsert(svc.mods, Mod(modId, entry), &Mod::id);
                    }
                    nEntry = BUFSIZE - 1;
                    nValue = BUFSIZE - 1;
                    ++entryIndex;
                }

                // add the default modules
                orderedInsert(svc.mods, Mod(1, "shell"), &Mod::id);
                orderedInsert(svc.mods, Mod(2, "app"), &Mod::id);
                orderedInsert(svc.mods, Mod(3, "ext"), &Mod::id);

                // store the service data
                orderedInsert(svcs_, svc, &Svc::id);
            }
            else
            {
                SALOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "RegionTable::refresh() Failed to query the OID of service %s"), svcName);
            }
            RegCloseKey(hSvc);
        }
        else
        {
            SALOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "RegionTable::refresh() Failed to open subkey %s under %s"), svcName, root);
        }
        // update the parameter
        nName = BUFSIZE - 1;
        ++keyIndex;
    }
    RegCloseKey(hRoot);
    return ret;
}
