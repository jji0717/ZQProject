#ifndef __SNMP_SUBAGENT__
#define __SNMP_SUBAGENT__


#include "Mib.h"


class SnmpSubagent {

public:

	SnmpSubagent(uint32_t serviceID, uint32_t svcProcess, uint32_t svcInstanceId = 0);
	~SnmpSubagent();

	bool ManageVariable(const char *name, void *address, uint32_t type, uint32_t access, int subOid);
	bool Run();

	uint32_t getServiceID(void)const
	{
		return _serviceID;
	} 

	uint32_t getSvcProcess(void)const
	{
		return _svcProcess;
	}

	uint32_t getSvcInstanceId(void)const
	{
		return _svcInstanceId;
	}
	
	uint32_t getSnmpUdpBasePort(void)const
	{
		return _snmpUdpBasePort;
	}

    timeout_t setTimeout(timeout_t timeOut){ return _selectTimeout = timeOut;}

private:

    ZQSNMP_STATUS processMessage(const u_char* request, int len, std::string& response);
    ZQSNMP_STATUS processRequest(u_char mode, netsnmp_variable_list* vars);

    ZQSNMP_STATUS processGetRequest(netsnmp_variable_list* vars);
    ZQSNMP_STATUS processGetNextRequest(netsnmp_variable_list* vars);
    ZQSNMP_STATUS processSetRequest(netsnmp_variable_list* vars);

    bool init();
    void unInit();

private:

    bool _running;

    static void* ThreadProc(void*);
    static pthread_mutex_t _mutex;
    pthread_t _id;

	timeout_t _selectTimeout;
	uint32_t  _serviceID;
	uint32_t  _svcProcess;
	uint32_t  _snmpUdpBasePort;
	uint32_t  _svcInstanceId;
    static std::string _pipeName;
    ServiceMIB _mib;
};

#endif 
