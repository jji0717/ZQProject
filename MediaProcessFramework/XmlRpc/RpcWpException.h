
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
// Name  : RpcWpException.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-26
// Desc  : exception
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/XmlRpc/RpcWpException.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 3     05-04-26 16:31 Daniel.wang
// ===========================================================================

#ifndef _RPC_WP_EXCEPTION_H_
#define _RPC_WP_EXCEPTION_H_

#include "rpcwrapper.h"

#define RpcExpString const char*

namespace XmlRpc
{
	class XmlRpcException;
}

namespace ZQ
{
	namespace rpc
	{
		//////////////////////////////////////////////////////////////////////////
		///RpcException
		class RPCWAPPER_API RpcException
		{
		private:
			XmlRpc::XmlRpcException*	m_pInst;
		public:
			XmlRpc::XmlRpcException*& GetInst(void)
			{ return m_pInst; }

			const XmlRpc::XmlRpcException* GetInst(void) const
			{ return m_pInst; }

			///constructor
			///@param strMessage - exception message
			///@param nCode - exception code
			RpcException(const char* strMessage, int nCode = -1) throw();

			///copy constructor
			///@param exp - exception instance
			RpcException(const RpcException& exp) throw();

			///operator=
			///@param exp - exception instance
			///@return - return current exception instance
			RpcException& operator=(const RpcException& exp) throw();

			///getMessage
			///@return - exception message string
			RpcExpString getMessage() const throw();

			///getCode
			///@return - exception code
			int getCode() const throw();
		};

		RpcException Exception_Conv(const XmlRpc::XmlRpcException& pInst);
	}
}

#endif//_RPC_WP_EXCEPTION_H_
