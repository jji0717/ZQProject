
#ifndef _ZQ_XMLRPC_H_
#define _ZQ_XMLRPC_H_

#define USE_XMLRPC using namespace ZQ::rpc

#include "RpcWpClient.h"
#include "RpcWpException.h"
#include "RpcWpReporter.h"
#include "RpcWpServer.h"
#include "RpcWpServerMethod.h"
#include "RpcWpValue.h"

/*
usage:
1. server end
create a RPC server instance, and register some RPC server method instance into it, then work and wait for client action

2. client end
create RPC client instance, and run execute function to linke server and do something
*/

#endif//_ZQ_XMLRPC_H_
