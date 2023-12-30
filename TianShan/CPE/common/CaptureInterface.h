// ===========================================================================
// Copyright (c) 2008 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#ifndef ZQTS_CPE_CAPTUREINTERFACE_H
#define ZQTS_CPE_CAPTUREINTERFACE_H

#include "Socket.h"
#include <vector>

namespace ZQ
{
	namespace common{
		class Log;
	}
}


namespace ZQTianShan 
{
namespace ContentProvision
{

	class CaptureContext
	{
	public:
        virtual ~CaptureContext() {}
		virtual SOCKET getSocket() = 0;
		
		virtual bool processData(const unsigned char* pUDPData, int nUDPLen) = 0;

		virtual std::string getLocalIp() = 0;

		virtual unsigned int getMulticastAddr() = 0;

		// the network byte sequence of port
		virtual unsigned int getMulticastPort() = 0;
	};


	
	class MulticastCaptureInterface
	{
	public:
		MulticastCaptureInterface();
		virtual ~MulticastCaptureInterface();

		static void setInstance(MulticastCaptureInterface* pInterface);
		static MulticastCaptureInterface* instance();
		static void destroyInstance();
		
		void addNIC(const std::string& strLocalIp, int nBandwidth = 1000000000);
		void clearNIC();
		void setLog(ZQ::common::Log* pLog);
		std::string getLastError();

		virtual bool init() = 0;
		virtual void close() = 0;

		virtual void reg(CaptureContext* pContext) = 0;
		virtual void unreg(CaptureContext* pContext) = 0;
	
	protected:
		void setLastError(const std::string& strError);
	
	protected:

		std::string						_strLastErr;
		
		ZQ::common::Log*				_log;

		struct NICInfo
		{
			std::string strLocalIp;
			int			nBandwidth;
		};

		typedef std::vector<NICInfo>	NICInfos;
		NICInfos						_vNICInfo;

		static MulticastCaptureInterface*	_pInstance;
	};

}
}



#endif
