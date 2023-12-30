#ifndef __RpcGeekUtils_H__
#define __RpcGeekUtils_H__

#include "RpcGeekCommon.h"

extern "C"
{
#include <stdlib.h>
#include <stdio.h>
}

//#include "config.h"  // information about this build environment
#include <xmlrpc-c/base.h>

namespace ZQ {
namespace RpcGeek {

class XValueGuard
{
public:
    xmlrpc_value * valueP;
    XValueGuard(xmlrpc_value * valueP) : valueP(valueP) {}
    ~XValueGuard() { xmlrpc_DECREF(valueP); }
};

xmlrpc_value* _convert2xmlrpc_parameters(xmlrpc_env* pEnv, ZQ::common::Variant& paramArray);
xmlrpc_value* _convert2xmlrpc(xmlrpc_env* pEnv, ZQ::common::Variant& value);
bool _convert2variant(xmlrpc_env* pEnv, xmlrpc_value* value, ZQ::common::Variant& var);
//void throw_if_fault_occurred (const xmlrpc_env *penv, const char* prompt_str =NULL);

} // namespace RpcGeek
} // namespace ZQ

#endif // __RpcGeekUtils_H__