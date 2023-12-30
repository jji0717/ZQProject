
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
// Name  : RpcWpServerMethod.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-26
// Desc  : rpc server method
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/XmlRpc_Asyn/RpcWpServerMethod.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-06-09 17:15 Lin.ouyang
// init version
// 
// 4     5/12/05 10:30p Daniel.wang
// 
// 3     05-04-26 16:05 Daniel.wang
// ===========================================================================

#ifndef _RPC_WP_SERVERMETHOD_H_
#define _RPC_WP_SERVERMETHOD_H_

#include "rpcwrapper.h"

namespace XmlRpc
{
	class XmlRpcServerMethod;
	class XmlRpcValue;
}

namespace ZQ
{
	namespace rpc
	{
		class RpcValue;

		class RpcServer;

		//////////////////////////////////////////////////////////////////////////
		///RpcServerMethod
		///RpcServerMethod is XML-RPC remote method in server end
		///It provide method function for client end
		class RPCWAPPER_API RpcServerMethod
		{
			friend class RpcServerMethodNest;
			friend class RpcServer;
		private:
			void*	m_pInst;

		public:
			///constructor
			///@param name - method name
			///@param server - server instance
			RpcServerMethod(const char* name, RpcServer* server = 0);

			///destructor
			virtual ~RpcServerMethod();

			///name\n
			///return server method name
			///@param strBuffer - the string buffer for method name
			///@param nMax - buffer size
			///@return - the string buffer pointer
			char* name(char* strBuffer, int nMax);

			///execute\n
			///usage : inherit
			///@param params - rpc method parameters
			///@param params - rpc method return result
			virtual void execute(RpcValue& params, RpcValue& result) = 0;

			///help\n
			///help string for server method
			///@param strBuffer - buffer for help string
			///@param nMax - buffer size
			///@return - the string buffer pointer
			virtual char* help(char* strBuffer, int nMax);


		private:
			void _execute(void* params, void* result);
		};
	}
}

#endif//_RPC_WP_SERVERMETHOD_H_
