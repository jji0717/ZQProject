#ifndef __SnmpClient_H__
#define __SnmpClient_H__
#include <snmp.h>
#include <mgmtapi.h>
#include <utility>
#include <string>
#include <list>

class SnmpClient {
public:
    SnmpClient();
    ~SnmpClient();
    bool open(const std::string& server, const std::string& community, size_t timeout, size_t retries);
    void close();
    typedef std::string Name; // oid: 1.2.3.4...
    typedef std::pair<std::string, std::string> Variable; // oid, value pair
    typedef std::list<Variable> Variables;
    typedef std::string Type; // i for integer, I for I64, U for unsigned I64, s for string
    bool get(const Name& name, Variable& value);
    bool walk(const Name& root, Variables& values);
    bool set(const Variable& value, const Type& type, Variable& current);
    const char* getLastError() const;
private:
    LPSNMP_MGR_SESSION session_;
    std::string lastError_;
};
#endif

