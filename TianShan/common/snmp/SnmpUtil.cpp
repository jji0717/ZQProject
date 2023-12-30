#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>

#include <sstream>
#include "SnmpUtil.h"
#include "FileLog.h"

namespace ZQSNMP {
namespace Util {

oid ZQSNMPOIDPREFIX[] = {1,3,6,1,4,1,22839,4,1};

bool createNamedPipe(const char* path)
{
	unlink(path);
    if (mkfifo(path, 0666) >= 0)
		return true;
	
	return (errno == EEXIST);
}

bool parseOid(
        const oid* myOid, 
        size_t len, 
        uint32_t& svcId, 
        uint32_t& procId, 
        uint32_t& infoType,
        uint32_t& idx) {

    if(!myOid || len < ZQSNMP_OID_LEN_SVCPREFIX) {
        return false;
    }

    if(len >= ZQSNMP_OID_LEN_SVCINSTANCE) {
        svcId = myOid[ZQSNMP_OID_IDX_SVCINSTANCE];
    }
    if(len >= ZQSNMP_OID_LEN_SVCPROCESS) {
        procId = myOid[ZQSNMP_OID_IDX_SVCPROCESS];
    }
    if(len >= ZQSNMP_OID_LEN_VARINFOTYPE) {
        infoType = myOid[ZQSNMP_OID_IDX_VARINFOTYPE];
    }
    if(len >= ZQSNMP_OID_LEN_VARINSTANCE) {
        idx = myOid[ZQSNMP_OID_IDX_VARINSTANCE];
    }

    return true;
}

std::string genPipeName(uint32_t svcId, uint32_t procId) {
    std::ostringstream os;

    os << "snmp." << svcId << '.' << procId << ".fifo";

    return os.str();
}

/*
bool callNamedPipe(
    const std::string& writePipe, 
    u_char* request, 
    size_t reqlen, 
    u_char* response, 
    size_t& reslen)
{

    syslog(LOG_INFO, "opening pipe[%s] for write", writePipe.c_str());
	//int writefd = open(writePipe.c_str(), O_WRONLY, S_IWGRP);
    int writefd = open(writePipe.c_str(), O_RDWR);

    if (writefd < 0) 
	{
        syslog(LOG_INFO, "failed to open (%s): %s", writePipe.c_str(), strerror(errno));
        return false;
    }

    ssize_t bytes = write(writefd, request, reqlen);
    syslog(LOG_INFO, "%ld bytes written to (%s)", bytes, writePipe.c_str());

    if (bytes <= 0)
	{
        syslog(LOG_INFO, "failed to write to pipe (%s): %s", writePipe.c_str(), strerror(errno));
        close(writefd);
        return false;
    }

	close(writefd);

    // reading response 
    syslog(LOG_INFO, "open (%s) for reading", AGENT_PIPE);
    int readfd = open(AGENT_PIPE, O_RDONLY);
    if (readfd < 0)
	{
        syslog(LOG_INFO, "failed to open pipe (%s): (%s)", AGENT_PIPE, strerror(errno));
        return false;
    }

    bytes = read(readfd, response, reslen);

    if (bytes <= 0)
	{
        syslog(LOG_INFO, "failed to read from pipe (%s): (%s)", AGENT_PIPE, strerror(errno));
        close(readfd);
        return false;
    }

    syslog(LOG_INFO, "%ld bytes read from (%s)", bytes, AGENT_PIPE);

    reslen = bytes;

    close(readfd);

    return true;
}
*/

bool request(const std::string& pipeName, u_char* request, size_t reqlen, u_char* response, size_t& reslen)
{
	syslog(LOG_INFO, "requesting thru pipe[%s]", pipeName.c_str());

	int fd =-1;
	bool succ =false;

	do {
		fd = open(pipeName.c_str(), O_WRONLY);

		if (fd < 0) 
		{
			syslog(LOG_ERR, "failed to open pipe[%s] to write: %s", pipeName.c_str(), strerror(errno));
			break;
		}

		// step 1. sending the query thru the pipe
		syslog(LOG_INFO, "pipe[%s] sending request %dByte", pipeName.c_str(), reqlen);
		ssize_t bytes = write(fd, request, reqlen);
		syslog(LOG_INFO, "%d bytes written to pipe[%s]", bytes, pipeName.c_str());

		if (bytes <= 0)
		{
			syslog(LOG_ERR, "failed to write to pipe[%s]: %s", pipeName.c_str(), strerror(errno));
			break;
		}

	} while(0);

	if (fd >=0)
		close(fd);

	do {

		fd = open(pipeName.c_str(), O_RDONLY); // will be blocked till response comes

		if (fd < 0) 
		{
			syslog(LOG_ERR, "failed to open pipe[%s] to read: %s", pipeName.c_str(), strerror(errno));
			break;
		}

/*
		// step 2 wait for a timeout until response comes
		fd_set fdsRead, fdsErr;
		FD_ZERO(&fdsRead); FD_SET(fd, &fdsRead);
		FD_ZERO(&fdsErr); FD_SET(fd, &fdsErr);

		struct timeval timeout;
		timeout.tv_sec =8;
		timeout.tv_usec =0;
		if (::select(fd+1, &fdsRead, NULL, &fdsErr,&timeout)<=0)
		{
			syslog(LOG_ERR, "timeout/failed at waiting for the response from[%s]", pipeName.c_str());
			break;
		}
*/
		// step 2 reading the response
		ssize_t bytes =0;
		bytes = read(fd, response, reslen);
		if (bytes <= 0)
		{
			syslog(LOG_INFO, "failed to read from pipe[%s]: (%s)", pipeName.c_str(), strerror(errno));
			break;
		}

		reslen = bytes;
		succ =true;
		syslog(LOG_INFO, "response received from pipe[%s] %uB", pipeName.c_str(), bytes);

	} while (0);

	if (fd>=0)
		close(fd);

	return succ;
}


u_char* asn_parse_any(u_char *data, size_t *datalength, const u_char *type, netsnmp_variable_list *pvb) {
    u_char var_type;

    if(type) {
        var_type = *type;
    }
    else {
        // get the type
        size_t len = *datalength;
        if(NULL == asn_parse_header(data, &len, &var_type)) {
            return NULL;
        }
    }
    u_char val_buf[1024]; 
    size_t val_len = 0;
    u_char *p = NULL;
    switch(var_type) {

    case ASN_INTEGER:
        val_len = sizeof(long);
        p = asn_parse_int(data, datalength, &var_type, (long *) val_buf, val_len);
        break;
    case ASN_COUNTER:
    case ASN_GAUGE:
    case ASN_TIMETICKS:
    case ASN_UINTEGER:
        val_len = sizeof(u_long);
        p = asn_parse_unsigned_int(data, datalength, &var_type, (u_long *)val_buf, val_len);
        break;
    case ASN_COUNTER64:
        val_len = sizeof(struct counter64);
        p = asn_parse_unsigned_int64(data, datalength, &var_type, (struct counter64 *) val_buf, val_len);
        break;
    case ASN_OCTET_STR:
    case ASN_IPADDRESS:
    case ASN_OPAQUE:
    case ASN_NSAP:
        val_len = sizeof(val_buf);
        p = asn_parse_string(data, datalength, &var_type, val_buf,&val_len);
        break;
    case ASN_OBJECT_ID:
        val_len = sizeof(val_buf) / sizeof(oid);
        p = asn_parse_objid(data, datalength, &var_type, (oid*)val_buf,&val_len);
        val_len *= sizeof(oid);
        break;
    case ASN_NULL:
        p = asn_parse_null(data, datalength, &var_type);
        break;
    case ASN_BIT_STR:
        val_len = sizeof(val_buf);
        p = asn_parse_bitstring(data, datalength, &var_type, val_buf,&val_len);
        break;
    default:
        return NULL;
    }

    if(p && 0 == snmp_set_var_typed_value(pvb, var_type, val_buf, val_len)) {
        return p;
    }

    return NULL;
}

bool encodeMsg(u_char* stream, size_t* len, u_char mode, int32_t err, netsnmp_variable_list* vars) {
    if(!stream || !len /* || *len < xxx || */ || !vars) {
        return false;
    }

    u_char* buff = stream;

    *buff = mode;
    buff += sizeof(u_char);

    *((int32_t*)buff) = err;
    buff += sizeof(int32_t);

    size_t bufLen = *len - (buff - stream);

    if(snmp_build_var_op(buff, vars->name, &vars->name_length, vars->type, vars->val_len, vars->val.string, &bufLen)) {
        *len -= bufLen;
        return true;
    }
    return false;
}

bool decodeMsg(const u_char* buff, size_t len,  u_char *mode, int32_t *err, netsnmp_variable_list *vars) {
    if(!buff || /* len < xxx || */ !mode || !err || !vars) {
        return false;
    }
    u_char* stream = const_cast<u_char*>(buff);

    *mode = *stream;
    stream += sizeof(u_char);

    *err = *((int32_t*)stream);
    stream += sizeof(int32_t);

    len -= (stream - buff);

    oid tmpoid[MAX_OID_LEN];
    size_t oidlen = 0, vallen = 0;
    u_char type = 0, *val = NULL;

    u_char *p = snmp_parse_var_op(stream, tmpoid, &oidlen, &type, &vallen, &val, &len);
    if(p) {
        size_t var_len = p - val;
        if(asn_parse_any(val, &var_len, &type, vars) == p && 0 == var_len) {
            if(!snmp_set_var_objid(vars, tmpoid, oidlen)) {
                return true;
            }
            else {
                snmp_set_var_typed_value(vars, ASN_NULL, NULL, 0); // clear data
            }
        }
    }
    return false;
}

ZQSNMP_STATUS memToAny(uint32_t type, const void *address, netsnmp_variable_list *pvb) {
    if(NULL == pvb || NULL == address) {
        return ZQSNMP_E_FAILURE;
    }

    switch(type) {
    case ZQSNMP_VARTYPE_INT32:
        {
            if(0 != snmp_set_var_typed_value(pvb, ASN_INTEGER, (u_char*)address, sizeof(int32_t)))
                return ZQSNMP_E_FAILURE;
        }
        break;
    case ZQSNMP_VARTYPE_CSTRING:
        {
            size_t len = strlen((const char*)address);
            if(0 != snmp_set_var_typed_value(pvb, ASN_OCTET_STR, (u_char*)address, len))
                return ZQSNMP_E_FAILURE;
        }
        break;
    case ZQSNMP_VARTYPE_STDSTRING:
        {
            std::string *val = (std::string *)address;
            if(0 != snmp_set_var_typed_value(pvb, ASN_OCTET_STR, (u_char*)val->data(), val->size()))
                return ZQSNMP_E_FAILURE;
        }
        break;
    default:
        return ZQSNMP_E_FAILURE;
    }
    return ZQSNMP_E_NOERROR;
}
ZQSNMP_STATUS anyToMem(uint32_t type, const netsnmp_variable_list *pvb, void *address) {
    if(!pvb || !address) {
        return ZQSNMP_E_FAILURE;
    }

    switch(type) {

    case ZQSNMP_VARTYPE_INT32:
        {
            if(ASN_INTEGER == pvb->type)
                *((int32_t *)address) = *(pvb->val.integer);
            else
                return ZQSNMP_E_BADVALUE;
        }
        break;
    case ZQSNMP_VARTYPE_CSTRING: // C style string
        {
            if(ASN_OCTET_STR != pvb->type)
                return ZQSNMP_E_BADVALUE;

            u_char *data = pvb->val.string;
            size_t len = pvb->val_len;
            if(len) {
                if(data) {
                    memcpy(address, data, len);
                }
                else {
                    return ZQSNMP_E_BADVALUE;
                }
            }
            *((u_char *)address + len) = 0;
        }
        break;
    case ZQSNMP_VARTYPE_STDSTRING: // std::string
        {
            if(ASN_OCTET_STR != pvb->type)
                return ZQSNMP_E_BADVALUE;

            u_char *data = pvb->val.string;
            size_t len = pvb->val_len;
            if(data) {
                ((std::string *)address)->assign((char*)data, len);
            }
            else {
                return ZQSNMP_E_BADVALUE;
            }
        }
        break;
    default: //unsupported type
        return ZQSNMP_E_FAILURE;
    }

    return ZQSNMP_E_NOERROR;
}

bool createOid(
        oid* buff, 
        size_t& len, 
        uint32_t svcId, 
        uint32_t procId, 
        const uint32_t* varType, 
        const uint32_t* pVarInstId) {

    if(!buff) {
        return false;
    }

    if(len < ZQSNMP_OID_LEN_SVCPROCESS) {
        return false;
    }

    memcpy(buff, ZQSNMPOIDPREFIX, sizeof(oid)*ZQSNMP_OID_LEN_SVCPREFIX);
    buff[ZQSNMP_OID_IDX_SVCINSTANCE] = svcId;
    buff[ZQSNMP_OID_IDX_SVCPROCESS] = procId;
    size_t len2 = ZQSNMP_OID_LEN_SVCPROCESS;

    if(varType) {
        if(len < ZQSNMP_OID_LEN_VARINFOTYPE) {
            return false;
        }

        buff[ZQSNMP_OID_IDX_VARINFOTYPE] = *varType;
        len2 = ZQSNMP_OID_LEN_VARINFOTYPE;

        if(pVarInstId) {
            if(len < ZQSNMP_OID_LEN_VARINSTANCE) {
                return false;
            }

            buff[ZQSNMP_OID_IDX_VARINSTANCE] = *pVarInstId;
            len2 = ZQSNMP_OID_LEN_VARINSTANCE;
        }
    }
    else if(pVarInstId) {
        return false;
    }

    len = len2;
    return true;
}

}}

// vim: ts=4 sw=4 nu bg=dark
