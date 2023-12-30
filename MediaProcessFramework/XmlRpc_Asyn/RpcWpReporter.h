
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
// Name  : RpcWpReporter.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-26
// Desc  : error and log reporter
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/XmlRpc_Asyn/RpcWpReporter.h $
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
// 3     05-04-26 16:27 Daniel.wang
// ===========================================================================

#ifndef _RPC_WP_H_
#define _RPC_WP_H_

#include "rpcwrapper.h"

namespace ZQ
{
	namespace rpc
	{
		//////////////////////////////////////////////////////////////////////////
		///RpcErrorHandler
		class RPCWAPPER_API RpcErrorHandler
		{
		public:
			static RpcErrorHandler* getErrorHandler();

			static void setErrorHandler(RpcErrorHandler* eh);

			virtual void error(const char* msg) = 0;

		protected:
			static RpcErrorHandler* _errorHandler;
		};

		//////////////////////////////////////////////////////////////////////////
		///RpcLogHandler
		class RPCWAPPER_API RpcLogHandler
		{
		public:
			static RpcLogHandler* getLogHandler();

			static void setLogHandler(RpcLogHandler* lh);

			static int getVerbosity();

			static void setVerbosity(int v);

			virtual void log(int level, const char* msg) = 0;

		protected:
			static RpcLogHandler* _logHandler;
			static int _verbosity;
		};

		///getVerbosity
		///get log level
		///@return - log level
		int RPCWAPPER_API getVerbosity();

		///setVerbosity
		///set log level
		///@param level - log level
		void RPCWAPPER_API setVerbosity(int level);
	}
}

#endif//_RPC_WP_H_
