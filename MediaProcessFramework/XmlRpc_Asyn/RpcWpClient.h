
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
// Name  : RpcWpClient.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-26
// Desc  : rpc client
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/XmlRpc_Asyn/RpcWpClient.h $
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
// 7     05-05-25 17:42 Daniel.wang
// 
// 6     5/16/05 4:32p Daniel.wang
// 
// 5     5/12/05 10:30p Daniel.wang
// 
// 4     05-04-26 16:44 Daniel.wang
// ===========================================================================

#ifndef _RPC_WP_CLIENT_H_
#define _RPC_WP_CLIENT_H_

#include "rpcwrapper.h"

namespace XmlRpc
{
	class XmlRpcClient;
}

namespace ZQ
{
	namespace rpc
	{
		class RpcValue;
		
		//////////////////////////////////////////////////////////////////////////
		///RpcClient
		///the client instance 
		class RPCWAPPER_API RpcClient
		{
		private:
			void*	m_pInst;
			bool	m_bClosed;

		public:
			///constructor\n
			///a client to connect to the server at the specified host:port address
			///@param host - The name of the remote machine hosting the server
			///@param port - The port on the remote machine where the server is listening
			///@param uri - XML uri
			RpcClient(const char* host, int port, const char* uri=0, int type = ST_TCP);

			///destructor
			virtual ~RpcClient();

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
			///@param result[out] - The result value to be returned to the client
			///@return true if the request was sent and a result received
			///(although the result might be a fault).
			bool execute(const char* method, const RpcValue& params, RpcValue& result);

			///isFault
			///@return - Returns true if the result of the last execute() was a fault response.
			bool isFault() const;

			///close\n
			///interface implementation\n
			///Close the connection
			virtual void close();

			///handleEvent\n
			///Handle server responses. Called by the event dispatcher during execute.\n
			///@param eventType - The type of event that occurred. 
			///@return - event type
			virtual unsigned handleEvent(unsigned eventType);

			// added by lorenzo, 2005-05-30
			///setTcp\n
			///specify the socket as tcp type
			void setTcp();

			///setUdp\n
			///specify the socket as tcp type
			void setUdp();

			///setSockType\n
			///specify the socket as specified type
			void setSockType(int socktype);
			// add end

		protected:
			// Execution processing helpers
			virtual bool doConnect();
			virtual bool setupConnection();

			virtual bool generateRequest(const char* method, const RpcValue& params);
			virtual char* generateHeader(const char* body, char* strBuffer, int nMax);
			virtual bool writeRequest();
			virtual bool readHeader();
			virtual bool readResponse();
			virtual bool parseResponse(RpcValue& result);
		};
	}
}

#endif//_RPC_WP_CLIENT_H_
