#ifndef __SNMP_UTIL__
#define __SNMP_UTIL__

#include <string>
#include <inttypes.h>
#include "ZQSnmp.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/utilities.h>

#define SVC_PIPE_PATH "/var/run/"
#define AGENT_PIPE SVC_PIPE_PATH"snmp.fifo"


namespace ZQSNMP {
namespace Util {

extern bool createNamedPipe(const char*);

/* writing to pipe */
/*
extern bool callNamedPipe(const std::string& writePipe, 
u_char* request, 
size_t reqlen, 
u_char* response, 
size_t& reslen);
*/
#define callNamedPipe request

extern bool request(const std::string& pipeName, u_char* request, size_t reqlen, u_char* response, size_t& reslen);

extern std::string getPduType();

extern std::string genPipeName(uint32_t svcId, uint32_t procId);

extern bool parseOid(
            const oid*, 
            size_t len, 
            uint32_t& svcId, 
            uint32_t& procId, 
            uint32_t& infoType, 
            uint32_t& idx);

extern bool createOid(
            oid*, 
            size_t& len, 
            uint32_t svcInstId, 
            uint32_t svcProcess, 
            const uint32_t* pVarInfoType, 
            const uint32_t *pVarInstId);

extern bool encodeMsg(
            u_char* stream, 
            size_t* len, 
            u_char mode, 
            int32_t err, 
            netsnmp_variable_list* vars); 

extern bool decodeMsg(const u_char* stream, size_t  len, u_char* mode, int32_t* err, netsnmp_variable_list* vars);

extern ZQSNMP_STATUS memToAny(uint32_t type, const void *address, netsnmp_variable_list *pvb);

extern ZQSNMP_STATUS anyToMem(uint32_t type, const netsnmp_variable_list *pvb, void *address); 


}}

#endif

