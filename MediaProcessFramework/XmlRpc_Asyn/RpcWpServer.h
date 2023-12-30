
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
// Name  : RpcWpServer.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-26
// Desc  : rpc server instance
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/XmlRpc_Asyn/RpcWpServer.h $
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
// 7     05-05-26 14:15 Daniel.wang
// 
// 6     5/12/05 10:30p Daniel.wang
// 
// 5     05-04-26 16:23 Daniel.wang
// ===========================================================================

#ifndef _RPC_WP_SERVER_H_
#define _RPC_WP_SERVER_H_

#include "rpcwrapper.h"

namespace XmlRpc
{
	class XmlRpcServer;
}

namespace ZQ
{
	namespace rpc
	{
		
		class RpcValue;
		class RpcServerMethod;

		//////////////////////////////////////////////////////////////////////////
		///RpcServer
		///rpc server is XML rpc server end instance to handle XML RPC requests
		class RPCWAPPER_API RpcServer
		{
			friend class RpcServerMethod;
		private:
			void*	m_pInst;
			bool	m_bExit;
			bool	m_bShutdown;

		public:
			///constructor 
			RpcServer(SockType st = ST_TCP);

			///destructor
			virtual ~RpcServer();

			///enableIntrospection\n
			///Specify whether introspection is enabled or not. Default is not enabled.
			///@param enabled
			void enableIntrospection(bool enabled=true);

			///addMethod\n
			///Add a command to the RPC server
			///@param method - method instance
			void addMethod(RpcServerMethod* method);

			///removeMethod\n
			///Remove a command from the RPC server
			///@param method - method instance
			void removeMethod(RpcServerMethod* method);

			///removeMethod\n
			///Remove a command from the RPC server by name
			///@param methodName - rpc method name
			void removeMethod(const char* methodName);

			///findMethod\n
			///Look up a method by name
			///@param name - rpc method name
			///@param rsm - rpc method instance
			///@return - rpc method instance
			//RpcServerMethod* findMethod(const char* name, RpcServerMethod*& rsm) const;

			///bindAndListen\n
			///Create a socket, bind to the specified port, and\n
			///set it in listen mode to make it available for clients.
			///@param port - socket port for rpc server
			///@param strip - socket IP address for rpc server
			///@param backlog - log leave
			///@return - return true if ok
			bool bindAndListen(int port, const char* strip = 0, int backlog = 5);

			///work\n
			///Process client requests for the specified time
			///@param msTime - work time by second, -1.0 is infinite
			void work(double msTime);

			///exit \n
			///Temporarily stop processing client requests and exit the work() method.
			void exit();

			///shutdown\n
			///Close all connections with clients and the socket file descriptor
			void shutdown();

			///listMethods\n
			///Introspection support
			///@param result - method names
			void listMethods(RpcValue& result);

		protected:
			// added by lorenzo, 2005-05-30
			///setTcp\n
			///specify the socket as tcp type
			void setTcp();

			///setUdp\n
			///specify the socket as tcp type
			void setUdp();
			// add end
		};
	}
}

#endif//_RPC_WP_SERVER_H_
