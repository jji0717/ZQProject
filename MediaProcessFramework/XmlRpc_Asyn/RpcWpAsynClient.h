
// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Dev  : Microsoft Developer Studio
// Name  : RpcWpAsynClient.h
// Author: Lorenzo
// Date  : 2005-6-8
// Desc  : rpc asyn client
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/XmlRpc_Asyn/RpcWpAsynClient.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 2     05-06-09 7:01p Daniel.wang
// 
// 1     05-06-09 17:15 Lin.ouyang
// init version
// 
// ===========================================================================

#ifndef _RPC_WP_ASYNCLIENT_H_
#define _RPC_WP_ASYNCLIENT_H_

#include "rpcwrapper.h"
#include "RpcWpValue.h"
#include <list>
#include "../../common/Locks.h"
#include "RpcClientThread.h"

namespace XmlRpc
{
	class XmlRpcClient;
}

namespace ZQ
{
	namespace rpc
	{
		class RpcValue;
		
		typedef void (*response_callback)(const char *server_url, \
                           const char *method_name, \
                           RpcValue &params, \
                           RpcValue &result);
		typedef std::list<RpcClientThread *> thread_list;
		/////////////////////////////////////////////////////////////////////////////
		///RpcAsynClient
		class RPCWAPPER_API RpcAsynClient
		{
		public:
			friend class RpcClientThread;
			///constructor\n
			///a client to connect to the server at the specified host:port address
			///@param host - The name of the remote machine hosting the server
			///@param port - The port on the remote machine where the server is listening
			///@param uri - XML uri
			RpcAsynClient(const char *host, int port, const char* uri=0, int type = ST_TCP);

			///destructor
			virtual ~RpcAsynClient();

			///setResponseTimeout\n
			///set response timeout while send method to server
			///@param timeout - wait for response time 
			void setResponseTimeout(int timeout =-1);

			///execute\n
			///Execute the named procedure on the remote server.\n
			///Currently this is a synchronous (blocking) implementation (execute\n
			///does not return until it receives a response or an error). Use isFault()\n
			///to determine whether the result is a fault response.
			///@param method - The name of the remote procedure to execute
			///@param params - An array of the arguments for the method
			///@return true if the request was sent and a result received
			///(although the result might be a fault).
			bool execute(const char *method, RpcValue& params, response_callback);

			///wait for asynchronic thread
			void waitAsyn();
		protected:
			///setTcp\n
			///specify the socket as tcp type
			void setTcp();

			///setUdp\n
			///specify the socket as tcp type
			void setUdp();


		public:
			static int _sock_type;
			static const char *_host;
			static int _port;
			static const char *_uri;
			static int _timeout;
			static RpcValue _params;
			static const char *_method;
			static response_callback _callback;
			static thread_list _list;
			static ZQ::common::Mutex _mutex;


		private:
			void deleteInactiveThread();

		private:
			thread_list _cur_list;
		};
	}
}

#endif//_RPC_WP_ASYNCLIENT_H_
