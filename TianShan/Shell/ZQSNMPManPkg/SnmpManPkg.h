#ifndef __ZQSNMPMANPKG_H__
#define __ZQSNMPMANPKG_H__

#include "ZQSnmp.h"
#include <cstdio>
#include <inttypes.h>
#include "FileLog.h"

extern bool SNMPOpenSession(
            const std::string& serviceName, 
            uint32_t processOid, 
            ZQSNMP_CALLBACK fCallback=0,
            bool loggingMask=false,
            const char* logPath=0,
			uint32_t  svcInstanceId = 0);

extern bool SNMPManageVariable(const char* varName, void* address, uint32_t type, bool readOnly);
extern bool SNMPManageVariable(const char* varName, void* address, uint32_t type, bool readOnly, int id);
extern bool SNMPCloseSession();

namespace {
extern bool GetSvcOid(const std::string& serviceName, uint32_t& svcId);
}

extern ZQ::common::FileLog* _snmp_logger;
#define SNMPLOG  if(NULL != _snmp_logger)(*_snmp_logger)


#endif 
