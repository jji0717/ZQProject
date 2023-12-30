#ifndef __ModuleMIB_H__
#define __ModuleMIB_H__

#include <map>

#ifdef ZQ_OS_MSWIN
typedef UINT SNMP_oid_t;
typedef SnmpVarBindList       SNMP_VarList;
#else
typedef oid  SNMP_oid_t;
typedef netsnmp_variable_list SNMP_VarList;
#endif

// ----------------------
// SnmpVar
// ----------------------
class SnmpVar
{
public:
	SnmpVar(const std::string& varname, void* varaddr, uint32_t vartype, uint32_t access)
		: _varname(varname), _varaddr(varaddr), _vartype(vartype), _access(access)
	{}

	virtual ~SnmpVar();

	ZQSNMP_STATUS set(const SNMP_VarList* pSnmpVarList);
	ZQSNMP_STATUS get(uint_t field, SNMP_VarList* pSnmpVarList) const;

private:
	std::string _varname;
	void*       _varaddr;
	uint32_t    _vartype;
	uint32_t    _access;
};

// ----------------------
// ModuleMIB
// ----------------------
class ModuleMIB
{
public:

public:
	ModuleMIB(uint32_t serviceOid, uint32_t svcInstanceId);
	virtual ~ModuleMIB();

	void regVar(SNMP_oid_t subOid, const SnmpVar& variable);

	ZQSNMP_STATUS get(SNMP_VarList* pSnmpVarList);
	ZQSNMP_STATUS getNext(SNMP_VarList* pSnmpVarList);
	ZQSNMP_STATUS get(SNMP_VarList* pSnmpVarList);

	uint32_t getServiceID() const {  return _serviceOid; }
	uint32_t getSvcProcess() const  { return m_svcProcess; }

	bool isDescendant(const SNMP_oid_t* Oid, size_t OidLen);

public:
	static ZQSNMP_STATUS memToAny(uint32_t type, const void *varaddr, SNMP_VarList* pSnmpVarList);
	static ZQSNMP_STATUS anyToMem(uint32_t type, const SNMP_VarList* pSnmpVarList, void *varaddr);

	static std::string oidToStr(const SNMP_oid_t* Oid, size_t OidLen);
	static size_t strToOid(const char* strOid, SNMP_oid_t* Oid, size_t OidMaxLen);

protected:
	typedef std::map <SNMP_oid_t, SnmpVar> VariableMap; // string subOid to variable map

	const uint32_t _serviceOid, _svcInstanceId;
	VariableMap       _varMap;
	ZQ::common::Mutex _lock;

	SNMP_oid_t _baseOid[ZQSNMP_OID_LEN_SVCPREFIX];
	size_t _baseOidSize;
};

// ----------------------
// SnmpSubagent
// ----------------------
class SnmpSubagent : public ZQ::common::NativeThread
{
public:
	SnmpSubagent(ZQ::common::Log& log, ModuleMIB& module);
	virtual ~SnmpSubagent();

protected: // impl of Thread
    bool init();

	bool run();

	timeout_t setTimeout(timeout_t timeOut) { return _selectTimeout = timeOut;}

private:

    ZQSNMP_STATUS processMessage(const void *pRequestMsg, int len, std::string *pResponseMsg);
    ZQSNMP_STATUS processRequest(BYTE pduType, SnmpVarBind *pVb);

    ZQSNMP_STATUS processGetRequest(SnmpVarBind *pVb);
    ZQSNMP_STATUS processGetNextRequest(SnmpVarBind *pVb);
    ZQSNMP_STATUS processSetRequest(SnmpVarBind *pVb);

    bool init();
    void unInit();
    static DWORD WINAPI ThreadProc(LPVOID lpParameter);

protected:

	ModuleMIB&       _mib;
	ZQ::common::Log& _log;

    bool   m_bRunning;
    HANDLE m_hPipe;

    HANDLE  m_thread;
	UINT    m_serviceID;
	UINT    m_svcProcess;
	UINT    m_serviceInstanceId;
	uint32  m_snmpUdpBasePort;
	timeout_t m_selectTimeout;

    ServiceMIB m_svcMib;
    ZQ::common::Mutex m_lockMib;
};


#endif // __ModuleMIB_H__

#include <cstdlib>
#include <cstring>

#ifndef MAPSET
#  define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL
#endif // MAPSET

// ----------------------
// SnmpVar
// ----------------------
ZQSNMP_STATUS SnmpVar::get(uint_t field, SNMP_VarList* pSnmpVarList) const
{
	if(NULL == pSnmpVarList)
		return ZQSNMP_E_FAILURE;

	switch (field)
	{
	case ZQSNMP_VARINFOTYPE_VALUE: // variable value
		return ModuleMIB::memToAny(_type, m_address, pSnmpVarList);

	case ZQSNMP_VARINFOTYPE_NAME: //variable name
		return ModuleMIB::memToAny(ZQSNMP_VARTYPE_STDSTRING, &_varname, pSnmpVarList);

	case ZQSNMP_VARINFOTYPE_ACCESS: //access
		return ModuleMIB::memToAny(ZQSNMP_VARTYPE_INT32, &_access, pSnmpVarList);

	default:
		return ZQSNMP_E_FAILURE;
	}

	return ZQSNMP_E_NOERROR;
}

ZQSNMP_STATUS SnmpVar::set(const SNMP_VarList* pSnmpVarList)
{
	if(NULL == pSnmpVarList)
		return ZQSNMP_E_FAILURE;

	//check access type
	if(ZQSNMP_ACCESS_READWRITE != m_access)
		return ZQSNMP_E_READONLY;

	ZQSNMP_STATUS status = ZQSNMP::Util::anyToMem(m_type, pSnmpVarList, m_address);
	if(ZQSNMP_E_NOERROR == status && g_callback)
		g_callback(m_name.c_str());

	return status;
}

// ----------------------
// ModuleMIB
// ----------------------
static const oid_t ZQSNMPOIDPREFIX[] = {1,3,6,1,4,1,22839,4,1};
#define ZQSNMPOIDPREFIX_LEN          (9)
ModuleMIB::ModuleMIB(uint32_t serviceOid, uint32_t svcInstanceId)
:_serviceOid(serviceOid), _svcInstanceId(svcInstanceId)
{
	// build up the _baseOid
	memcpy(_baseOid, ZQSNMPOIDPREFIX, sizeof(ZQSNMPOIDPREFIX));
	_baseOidSize = sizeof(ZQSNMPOIDPREFIX) / sizeof(oid_t);
	_baseOid[_baseOidSize++] = _serviceOid;
	_baseOid[_baseOidSize++] = _svcInstanceId;
}

ModuleMIB::~ModuleMIB()
{
}

bool ModuleMIB::isDescendant(const SNMP_oid_t* Oid, size_t OidLen) const
{
	if (NULL ==Oid || OidLen <= _baseOidSize)
		return false;

	return (0 == memcmp(_baseOid, Oid, _baseOidSize));
}

ZQSNMP_STATUS ModuleMIB::memToAny(uint32_t type, const void *varaddr, SNMP_VarList* pSnmpVarList)
{
	if(NULL == pSnmpVarList || NULL == varaddr)
		return ZQSNMP_E_FAILURE;

	switch (type)
	{
	case ZQSNMP_VARTYPE_INT32:
		{
			if(0 != snmp_set_var_typed_value(pSnmpVarList, ASN_INTEGER, (u_char*)varaddr, sizeof(int32_t)))
				return ZQSNMP_E_FAILURE;
		}
		break;

	case ZQSNMP_VARTYPE_CSTRING:
		{
			size_t len = strlen((const char*)varaddr);
			if(0 != snmp_set_var_typed_value(pSnmpVarList, ASN_OCTET_STR, (u_char*)varaddr, len))
				return ZQSNMP_E_FAILURE;
		}
		break;

	case ZQSNMP_VARTYPE_STDSTRING:
		{
			std::string* pstr = (std::string *)varaddr;
			if(0 != snmp_set_var_typed_value(pSnmpVarList, ASN_OCTET_STR, (u_char*)pstr->c_str(), pstr->length()))
				return ZQSNMP_E_FAILURE;
		}
		break;

	default:
		return ZQSNMP_E_FAILURE;
	}

	return ZQSNMP_E_NOERROR;
}

ZQSNMP_STATUS ModuleMIB::anyToMem(uint32_t type, const SNMP_VarList* pSnmpVarList, void* varaddr)
{
	if(!pSnmpVarList || !varaddr)
		return ZQSNMP_E_FAILURE;

	switch (type)
	{
	case ZQSNMP_VARTYPE_INT32:
		{
			if (ASN_INTEGER == pSnmpVarList->type)
				*((int32_t *)varaddr) = *(pSnmpVarList->val.integer);
			else
				return ZQSNMP_E_BADVALUE;
		}
		break;

	case ZQSNMP_VARTYPE_CSTRING: // C style string
		{
			if(ASN_OCTET_STR != pSnmpVarList->type)
				return ZQSNMP_E_BADVALUE;

			u_char *data = pSnmpVarList->val.string;
			size_t len = pSnmpVarList->val_len;
			if(len)
			{
				if(!data)
					return ZQSNMP_E_BADVALUE;

				memcpy(varaddr, data, len);
			}

			*((u_char *)varaddr + len) = 0;
		}
		break;

	case ZQSNMP_VARTYPE_STDSTRING: // std::string
		{
			if(ASN_OCTET_STR != pSnmpVarList->type)
				return ZQSNMP_E_BADVALUE;

			u_char *data = pSnmpVarList->val.string;
			size_t len = pSnmpVarList->val_len;
			if (!data) 
				return ZQSNMP_E_BADVALUE;

			((std::string *)varaddr)->assign((char*)data, len);
		}
		break;

	default: //unsupported type
		return ZQSNMP_E_FAILURE;
	}

	return ZQSNMP_E_NOERROR;
}

std::string ModuleMIB::oidToStr(const oid* Oid, size_t OidLen)
{
	std::string ret;
	char buf[16];
	for (int i=0; Oid && i< OidLen; i++)
	{
		snprintf(buf, sizeof(buf)-2, ".%d", Oid[i]);
		ret += buf;
	}

	return ret;
}

size_t ModuleMIB::strToOid(const char* strOid, oid* Oid, size_t OidMaxLen)
{
	if (NULL == strOid || NULL ==Oid || OidMaxLen <=0)
		return 0;
	std::string str = strOid;
	size_t i=0;
	for (i =0; i < OidMaxLen && !str.empty(); i++)
	{
		size_t pos = str.find('.');
		if (std::string::npos != pos)
			str = str.substr(pos+1);
		Oid[i] = (int)atol(str.c_str());
	}

	return i;
}

void ModuleMIB::add(const SnmpVar &var, SNMP_oid_t subOid =0)
{
	// TODO: lookup if subOid=0

	ZQ::common::MutexGuard g(_lock);

	MAPSET(VariableMap, _varMap, subOid, var);
}

ZQSNMP_STATUS ModuleMIB::get(SNMP_VarList* pSnmpVarList)
{
	if(NULL == pSnmpVarList)
		return ZQSNMP_E_FAILURE;

	SNMP_oid_t subOidType, subOidVar;

	if (ZQSNMP_E_NOERROR != chopForSubOid(pSnmpVarList->name, pSnmpVarList->name_length, subOidType, subOidVar))
		return ZQSNMP_E_NOSUCHNAME;

	ZQ::common::MutexGuard g(_lock);

	VariableMap::iterator itVar = _varMap.find(subOidVar);
	if (_varMap.end() == itVar)
		return ZQSNMP_E_NOSUCHNAME;

	return itVar->get(subOidType, pSnmpVarList);
}

ZQSNMP_STATUS ModuleMIB::getNext(SNMP_VarList* pSnmpVarList)
{
	if(NULL == pSnmpVarList)
		return ZQSNMP_E_FAILURE;

	SNMP_oid_t subOidType, subOidVar;
	if (ZQSNMP_E_NOERROR != chopForSubOid(pSnmpVarList->name, pSnmpVarList->name_length, subOidType, subOidVar))
		return ZQSNMP_E_NOSUCHNAME;

	ZQ::common::MutexGuard g(_lock);

	VariableMap::iterator itVar = _varMap.upper_bound(subOidVar);
	if (_varMap.end() == itVar)
		return ZQSNMP_E_NOSUCHNAME;

	if(0 != snmp_set_var_objid(pSnmpVarList, next, nextLen))
		return ZQSNMP_E_FAILURE;

	return itVar->get(subOidType, pSnmpVarList);
}

ZQSNMP_STATUS ModuleMIB::set(SNMP_VarList* pSnmpVarList)
{
	if(NULL == pSnmpVarList)
		return ZQSNMP_E_FAILURE;

	SNMP_oid_t subOidType, subOidVar;

	if (ZQSNMP_E_NOERROR != chopForSubOid(pSnmpVarList->name, pSnmpVarList->name_length, subOidType, subOidVar))
		return ZQSNMP_E_NOSUCHNAME;

	if (ZQSNMP_VARINFOTYPE_VALUE != subOidType)
		return ZQSNMP_E_READONLY;

	ZQ::common::MutexGuard g(_lock);

	VariableMap::iterator itVar = _varMap.find(subOidVar);
	if (_varMap.end() == itVar)
		return ZQSNMP_E_NOSUCHNAME;

	return itVar->set(pSnmpVarList);
}

ZQSNMP_STATUS ModuleMIB::chopForSubOid(const SNMP_oid_t* pOid, size_t len, 
									   SNMP_oid_t& subOidType, SNMP_oid_t& subOidVar)
{
	if (!isDescendant(pOid, len))
		return ZQSNMP_E_NOSUCHNAME;

	subOidType = (len >= ZQSNMP_OID_IDX_VARINFOTYPE) ? pOid[ZQSNMP_OID_IDX_VARINFOTYPE]:0;
	subOidVar =  (len >= ZQSNMP_OID_IDX_VARINSTANCE) ? pOid[ZQSNMP_OID_IDX_VARINSTANCE]:0;

	return ZQSNMP_E_NOERROR;
}

// ----------------------
// SnmpSubagent
// ----------------------
bool SnmpSubagent::init() 
{
	SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "SnmpSubagent init enter ..."));
	std::istringstream       is;
	XMLPreferenceDocumentEx  doc;
	int                      nRev        = false;
	char                     value[256]  =  {0};
	const char*              CONFIG      = "/etc/TianShan.xml";

	if(!doc.open(CONFIG)) 
	{
		SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "init() serviceId[%d], moduleId[%d], failed to load configuration[%s]"), _serviceID, _svcProcess, CONFIG);
		return nRev;
	}

	ZQ::common::XMLPreferenceEx * node = getPreferenceNode("SNMP", doc);
	if(!node) 
	{
		SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "init() serviceId[%d], moduleId[%d], failed to get configuration[%s] SNMP node"), _serviceID, _svcProcess, CONFIG);
		return nRev;
	}

	nRev = node->getAttributeValue("SnmpUdpBasePort", value);
	if(nRev) 
	{
		const unsigned int portMaxLimit = 65535;
		is.str(value);
		is >> _snmpUdpBasePort;
		_snmpUdpBasePort = (_snmpUdpBasePort < portMaxLimit) ? _snmpUdpBasePort : 10000;
	}

    SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "init() serviceId[%d], moduleId[%d], configuration[%s], snmpUdpBasePort[%d], returnCode[%d]"), _serviceID, _svcProcess, CONFIG, _snmpUdpBasePort, nRev);
    return true;
}

void SnmpSubagent::final()
{
    SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "unInit(), snmpUdpBasePort[%d], svcProcess[%d], serviceID[%d]"), _snmpUdpBasePort, _svcProcess, _serviceID);
    unlink(_pipeName.c_str());
}

int SnmpSubagent::run()
{
	SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "thread start. [thread id = %u]"), pthread_self());
    unsigned long  requestLinePos  = (unsigned long) &((((struct InterActive*) 0)->_content).request);
	SnmpSubagent  *pSubagent       = (SnmpSubagent*)arg;
	uint32_t       msgid           = 0;
	unsigned long  lastLossCount   = 0;
	uint32         thrdId          = pthread_self();
	timeout_t      timeout         = pSubagent->_selectTimeout;
	uint32_t       svcInstanceId   = 10 * pSubagent->getSvcInstanceId();
	uint32_t       serviceId       = pSubagent->getServiceID();
	uint32         snmpServerPort  = (serviceId / 100) * 100 + pSubagent->getSvcProcess() + pSubagent->getSnmpUdpBasePort() + svcInstanceId;
	InetAddress    udpSvcAddress("127.0.0.1");
	UDPSocket      udpServer(udpSvcAddress, snmpServerPort);

	Socket::Error  svcErrorNum = udpServer.getErrorNumber();
	if(Socket::errSuccess != svcErrorNum)
	{
		SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "thread[%d] quit, snmp udp server port[%d+%d+%d+%d], snmpServerPort[%d], errorNumber[%d]"), 
			thrdId, serviceId, pSubagent->getSvcProcess(), pSubagent->getSnmpUdpBasePort(), pSubagent->getSvcInstanceId(), snmpServerPort, svcErrorNum);

		return false;
	}

	udpServer.setCompletion(false);
	pSubagent->_running = true;

	SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "thread[%d] running, snmp udp server port[%d+%d+%d+%d], snmpServerPort[%d], timeout[%d]"), 
		thrdId, serviceId, pSubagent->getSvcProcess(), pSubagent->getSnmpUdpBasePort(), pSubagent->getSvcInstanceId(), snmpServerPort, timeout);

	while(pSubagent->_running)
	{
		timeout = pSubagent->_selectTimeout;
		for(int nLoop = 1; pSubagent->_running; ++nLoop)
		{
			if (udpServer.isPending(Socket::pendingInput, timeout))
			{
				SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], input timeout[%d], loop[%d]"), thrdId, snmpServerPort, timeout, nLoop);
				break;
			}
		}

		if (!pSubagent->_running)
			break;

		InterActive      recvFromAgent = {0};
		InetHostAddress  udpClientAddress;
		int              peerPort = 0;
		int              bytes = udpServer.receiveFrom(&recvFromAgent, sizeof(InterActive), udpClientAddress, peerPort);
		if (0 >= bytes || 0 >= (bytes - requestLinePos))
		{
			SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d] recv bytes[%d] lastLossCount[%d], erro"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, bytes, lastLossCount);
			continue;
		}

		std::string response;
		SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d], msgId[%d], lastLossCount[%d]"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, msgid, lastLossCount);
		ZQSNMP_STATUS status = pSubagent->processMessage((recvFromAgent._content.request), (bytes - requestLinePos), response);
		++msgid;

		if (ZQSNMP_E_NOERROR != status)
		{
			SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d], msgid[%d] process erro"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, msgid);
			continue;
		}

		if (ZQSNMP_MSG_LEN_MAX < response.size()) 
		{
			SNMPLOG(Log::L_WARNING, CLOGFMT(SnmpSubagent, "response[%u] size[%u] exceeded max len, abort"), msgid, response.size()); 
			continue;
		}

		if (!udpServer.isPending(Socket::pendingOutput, timeout))
		{
			SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing msgid[%d] thread[%d], snmp udp server port[%d], Output timeout[%d]"), msgid, thrdId, snmpServerPort, timeout);
			continue;
		}

		InterActive sendToAgent = {0};
		lastLossCount                       = msgid - recvFromAgent._head._agentRecvSeq;
		sendToAgent._head._agentRecvSeq     = recvFromAgent._head._agentRecvSeq;
		sendToAgent._head._serviceSendSeq   = msgid;
		sendToAgent._head._lastLossCount    = lastLossCount;
		memcpy((sendToAgent._content.request), response.data(), response.size());
		SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "request[%u] processMessage successfully, sending response %uB  client[%s], peerPort[%d]"), msgid, response.size(), udpClientAddress.getHostAddress(), peerPort);
		int sentState = udpServer.sendto(&sendToAgent, (response.size() + requestLinePos), udpClientAddress, peerPort);
		if (0 >= sentState)
		{
			SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "failed sendto client[%s], peerPort[%d] response[%u] size[%u], lastLossCount[%d] %s"), udpClientAddress.getHostAddress(), peerPort, msgid, response.size(), lastLossCount, strerror(errno));
			continue;
		}

		SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "client[%s], peerPort[%d]  response[%u] %u bytes send, lastLossCount[%d]"), udpClientAddress.getHostAddress(), peerPort, msgid, response.size(), lastLossCount);
	}

	SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "leave the message processing thread %d  snmp udp server port[%d], lastLossCount[%d], msgId[%d]"), thrdId, snmpServerPort, lastLossCount, msgid);
	return 0;
}

ZQSNMP_STATUS SnmpSubagent::processMessage(const u_char* request, int len, std::string& response) 
{
    if(NULL == request || len <= 0) 
        return ZQSNMP_E_FAILURE;
    
    u_char pdutype = ZQSNMP_PDU_UNKNOWN;
    int errstat = SNMP_ERR_NOERROR;

    SnmpVarBindList SnmpVarList = {0};
    if(!ZQSNMP::Util::decodeMsg(request, len, &pdutype, &errstat, &SnmpVarList)
	{
        SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "ZQSnmpUtil::DecodeMsg Failed."));
        return ZQSNMP_E_FAILURE;
    }
 
	ZQSNMP_STATUS ret = ZQSNMP_E_FAILURE;
	//   SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processRequest() %d type: %d"), *(vars->val.integer), vars->type);

	switch(method) 
	{
	case ZQSNMP_PDU_GET:
		ret = _mib.get(&SnmpVarList);
		break;

	case ZQSNMP_PDU_GETNEXT:
		ret = _mib.getNext(&SnmpVarList);
		// ???
		// if(ZQSNMP_E_NOERROR == ret)
		//     pSnmpVarList = pSnmpVarList->next_variable;

		break;

	case ZQSNMP_PDU_SET:
		ret = _mib.set(&SnmpVarList);
		break;

	default:
		SNMPLOG(Log::L_WARNING, CLOGFMT(SnmpSubagent, "processRequest() unknown PDU method[%d]"), method);
		ret = ZQSNMP_E_FAILURE;
	}

	// convert ZQSNMP_E_xxx to SNMP_ERR_xxx
	switch(status)
	{
    case ZQSNMP_E_NOERROR:
        errstat = SNMP_ERR_NOERROR;
        break;

	case ZQSNMP_E_NOSUCHNAME:
        errstat = SNMP_ERR_NOSUCHNAME;
        break;

	case ZQSNMP_E_BADVALUE: //for set-pdu only
        errstat = SNMP_ERR_BADVALUE;
        break;

	case ZQSNMP_E_READONLY: //for set-pdu only
        errstat = SNMP_ERR_READONLY;
        break;

	default:
        errstat = SNMP_ERR_GENERR;
        break;
    }

    size_t reslen = ZQSNMP_MSG_LEN_MAX;
    u_char res[ZQSNMP_MSG_LEN_MAX];
    if(!ZQSNMP::Util::encodeMsg(res, &reslen, pdutype, errstat, &vb))
	{
        SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "ZQSnmpUtil::EncodeMsg failed"));
        return ZQSNMP_E_FAILURE;
    }

    response.assign((const char*)res, reslen);
	return ret;
}

