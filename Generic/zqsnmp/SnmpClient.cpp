#include "SnmpClient.h"
#include <sstream>

#pragma warning (disable: 4267 4996)

static std::string showSystemError(int err);
static bool str2oid(const std::string& s, AsnObjectIdentifier& oid);
static std::string oid2str(const AsnObjectIdentifier& oid);
static std::string any2str(const AsnAny& any);
static bool allocVBL(size_t len, SnmpVarBindList& vbl);
static std::string snmpErrorStatus(int err);

SnmpClient::SnmpClient()
    :session_(NULL) {
}
SnmpClient::~SnmpClient() {
    if(session_) {
        close();
    }
}
bool SnmpClient::open(const std::string& server, const std::string& community, size_t timeout, size_t retries) {
    lastError_.clear();
    session_ = SnmpMgrOpen((char*)server.c_str(), (char*)community.c_str(), timeout, retries);
    if(NULL != session_) {
        return true;
    } else {
        lastError_ = showSystemError(GetLastError());
        return false;
    }
}
void SnmpClient::close() {
    if(session_) {
        SnmpMgrClose(session_);
		session_ = NULL;
        lastError_ = "Session Closed";
    }
}
bool SnmpClient::get(const Name& name, Variable& value) {
    if(NULL == session_) {
        lastError_ = "No session opened";
        return false;
    }
    lastError_.clear();
    // prepare the request
    SnmpVarBindList vbl = {0};
    if(!allocVBL(1, vbl)) {
        lastError_ = "Failed to allocate VariableBindingList";
        return false;
    }
    if(!str2oid(name, vbl.list[0].name)) {
        SnmpUtilVarBindListFree(&vbl);
        lastError_ = "Invalid oid:" + name;
        return false;
    }
    vbl.list[0].value.asnType = ASN_NULL;

    bool result = false;
    AsnInteger32 errstat = SNMP_ERRORSTATUS_NOERROR;
    AsnInteger32 erridx  = 0;
    //send request
    if(SnmpMgrRequest(session_, SNMP_PDU_GET, &vbl, &errstat, &erridx)) {
        if(SNMP_ERRORSTATUS_NOERROR == errstat) {
            // copy the value
            value.first = oid2str(vbl.list[0].name);
            value.second = any2str(vbl.list[0].value);
            result = true;
        } else {
            lastError_ = snmpErrorStatus(errstat);
            result = false;
        }
    } else {
        lastError_ = showSystemError(GetLastError());
        result = false;
    }
    SnmpUtilVarBindListFree(&vbl);
    return result;
}
bool SnmpClient::walk(const Name& root, Variables& values) {
    if(NULL == session_) {
        lastError_ = "No session opened";
        return false;
    }
    lastError_.clear();
    AsnObjectIdentifier rootOid = {0};
    if(!str2oid(root, rootOid)) {
        lastError_ = "Invalid oid:" + root;
        return false;
    }
    // prepare the request
    SnmpVarBindList vbl = {0};
    if(!allocVBL(1, vbl)) {
        SnmpUtilOidFree(&rootOid);
        lastError_ = "Failed to allocate VariableBindingList";
        return false;
    }
    if(!SnmpUtilOidCpy(&(vbl.list[0].name), &rootOid)) {
        SnmpUtilVarBindListFree(&vbl);
        SnmpUtilOidFree(&rootOid);
        lastError_ = "Failed to allocate SnmpUtilOidCpy";
        return false;
    }
    bool result = true;
    while(result) {
        vbl.list[0].value.asnType = ASN_NULL;
        AsnInteger32 errstat = SNMP_ERRORSTATUS_NOERROR;
        AsnInteger32 erridx  = 0;
        //send request
        if(SnmpMgrRequest(session_, SNMP_PDU_GETNEXT, &vbl, &errstat, &erridx)) {
            if(SNMP_ERRORSTATUS_NOERROR == errstat) {
                if(0 == SnmpUtilOidNCmp(&(vbl.list[0].name), &rootOid, rootOid.idLength)) {
                    // copy the value
                    values.push_back(std::make_pair(oid2str(vbl.list[0].name), any2str(vbl.list[0].value)));
                    result = true;
                } else { //out of scope
                    result = true;
                    break;
                }
            } else if (SNMP_ERRORSTATUS_NOSUCHNAME == errstat) { // end of mib
                result = true;
                break;
            } else {
                lastError_ = showSystemError(GetLastError());
                result = false;
                break;
            }
        } else {
            lastError_ = showSystemError(GetLastError());
            result = false;
            break;
        }
        SnmpUtilAsnAnyFree(&(vbl.list[0].value));
    }
    SnmpUtilVarBindListFree(&vbl);
    return result;
}

bool SnmpClient::set(const Variable& value, const Type& type, Variable& current) {
    if(NULL == session_) {
        lastError_ = "No session opened";
        return false;
    }
    lastError_.clear();
    // prepare the request
    SnmpVarBindList vbl = {0};
    if(!allocVBL(1, vbl)) {
        lastError_ = "Failed to allocate VariableBindingList";
        return false;
    }

    if(type == "i") { // integer
        vbl.list[0].value.asnType = ASN_INTEGER;
        vbl.list[0].value.asnValue.number = strtol(value.second.c_str(), NULL, 10);
    } else if (type == "I") { // unsigned integer
        vbl.list[0].value.asnType = ASN_UINTEGER32;
        vbl.list[0].value.asnValue.number = strtoul(value.second.c_str(), NULL, 10);
    } else if (type == "s") { // string
        AsnOctetString asnstr = {0};
        asnstr.length = value.second.size();
        asnstr.stream = (BYTE*)value.second.data();
        asnstr.dynamic = FALSE;
        if(SnmpUtilOctetsCpy(&(vbl.list[0].value.asnValue.string), &asnstr)) {
            vbl.list[0].value.asnType = ASN_OCTETSTRING;
        } else {
            SnmpUtilVarBindListFree(&vbl);
            lastError_ = "Failed to create octet string with length:" + asnstr.length;
            return false;
        }
    } else if (type == "U") { // unsigned I64
        vbl.list[0].value.asnType = ASN_COUNTER64;
        vbl.list[0].value.asnValue.counter64.QuadPart = _strtoui64(value.second.c_str(), NULL, 10);
    } else {
        SnmpUtilVarBindListFree(&vbl);
        lastError_ = "Unsupported type:" + type;
        return false;
    }
    if(!str2oid(value.first, vbl.list[0].name)) {
        SnmpUtilVarBindListFree(&vbl);
        lastError_ = "Invalid oid:" + value.first;
        return false;
    }

    bool result = false;
    AsnInteger32 errstat = SNMP_ERRORSTATUS_NOERROR;
    AsnInteger32 erridx  = 0;
    //send request
    if(SnmpMgrRequest(session_, SNMP_PDU_SET, &vbl, &errstat, &erridx)) {
        if(SNMP_ERRORSTATUS_NOERROR == errstat) {
            // copy the value
            current.first = oid2str(vbl.list[0].name);
            current.second = any2str(vbl.list[0].value);
            result = true;
        } else {
            lastError_ = snmpErrorStatus(errstat);
            result = false;
        }
    } else {
        lastError_ = showSystemError(GetLastError());
        result = false;
    }
    SnmpUtilVarBindListFree(&vbl);
    return result;
}
const char* SnmpClient::getLastError() const {
    return lastError_.c_str();
}

// utilities
static std::string showSystemError(int err) {
    LPVOID lpMsgBuf = NULL;
    if (FormatMessage( 
                      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                      FORMAT_MESSAGE_FROM_SYSTEM | 
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      err,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                      (LPTSTR) &lpMsgBuf,
                      0,
                      NULL )) {
        std::string error = (const char*)lpMsgBuf;
        // Free the buffer.
        LocalFree( lpMsgBuf );
        return error;
    } else {
        // Handle the error.
        switch(err) {
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
            char buf[32] = {0};
            sprintf(buf, "error(%d)", err);
            return buf;
        }
    }
}
static bool str2oid(const std::string& s, AsnObjectIdentifier& oid) {
    memset(&oid, 0, sizeof(oid));
    if(s.empty()) {
        return false;
    }
    std::string oidStr;
    // fix the SnmpMgrStrToOid format issue
    if(s[0] != '.') {
        oidStr = "." + s;
    } else {
        oidStr = s;
    }

    if(SnmpMgrStrToOid((char *)oidStr.c_str(), &oid)) {
        return true;
    } else {
        memset(&oid, 0, sizeof(oid));
        return false;
    }
}

static std::string oid2str(const AsnObjectIdentifier& oid) {
    if(NULL == oid.ids || 0 == oid.idLength) {
        return "";
    }
    std::ostringstream buf;
    for(size_t i = 0; i < oid.idLength; ++i) {
        if(i != 0)
            buf << ".";
        buf << oid.ids[i];
    }
    return buf.str();
}
static std::string any2str(const AsnAny& any) {
    std::ostringstream buf;
    switch(any.asnType) {
    case ASN_INTEGER:
    case ASN_UNSIGNED32:
        buf << any.asnValue.number;
        return buf.str();
    case ASN_OCTETSTRING:
        if(NULL == any.asnValue.string.stream || 0 == any.asnValue.string.length) {
            return "";
        } else {
            return std::string((const char *)(any.asnValue.string.stream), any.asnValue.string.length);
        }
        break;
    case ASN_COUNTER64:
        buf << any.asnValue.counter64.QuadPart;
        return buf.str();
    default:
        buf << "Unsupported type(" << ((unsigned int)any.asnType) << ")";
        return buf.str();
    }
}

static bool allocVBL(size_t len, SnmpVarBindList& vbl) {
    memset(&vbl, 0, sizeof(vbl));
    if(0 == len) {
        return false;
    }

    size_t memsize = sizeof(SnmpVarBind) * len;
    vbl.list = (SnmpVarBind *)SnmpUtilMemAlloc(memsize);
    if(vbl.list) {
        memset(vbl.list, 0, memsize);
        vbl.len = len;
        return true;
    } else {
        vbl.len = 0;
        return false;
    }
}

static std::string snmpErrorStatus(int err) {
    switch(err) {
    case SNMP_ERRORSTATUS_NOERROR:
        return "SNMP_ERRORSTATUS_NOERROR";
    case SNMP_ERRORSTATUS_TOOBIG:
        return "SNMP_ERRORSTATUS_TOOBIG";
    case SNMP_ERRORSTATUS_NOSUCHNAME:
        return "SNMP_ERRORSTATUS_NOSUCHNAME";
    case SNMP_ERRORSTATUS_BADVALUE:
        return "SNMP_ERRORSTATUS_BADVALUE";
    case SNMP_ERRORSTATUS_READONLY:
        return "SNMP_ERRORSTATUS_READONLY";
    case SNMP_ERRORSTATUS_GENERR :
        return "SNMP_ERRORSTATUS_GENERR";
    default:
        {
            char buf[64] = {0};
            sprintf(buf, "ERRORSTATUS(%d)", err);
            return buf;
        }
    }
}
