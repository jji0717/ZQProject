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

#ifndef ZQTS_CPE_CAPTURETHREAD_H
#define ZQTS_CPE_CAPTURETHREAD_H


#include "NativeThread.h"
#include <vector>
#include "Locks.h"


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
		virtual SOCKET getSocket() = 0;
		virtual bool receive() = 0;
	};


	class CaptureThread : protected ZQ::common::NativeThread
	{
	private:
		CaptureThread();

	public:
		enum 
		{
			DefaultSelectTimeout = 100		//in milliseconds
		};

		virtual ~CaptureThread()
		{
			close();
		}

		void setLog(ZQ::common::Log* pLog);

		void reg(CaptureContext* pContext);

		void unreg(CaptureContext* pContext);

		bool start();
		void close();

		static CaptureThread& instance();
		static void destroyInstance();
	protected:
		virtual int run(void);

		//return false if no socket return, else return true
		inline int getFDSET(fd_set& fdset);

	protected:

		struct CaptureInfo
		{
			SOCKET				socket;
			CaptureContext*		capture;
		};
		typedef std::vector<CaptureInfo>	CaptureContextMap;
		CaptureContextMap				_captureContexts;
		ZQ::common::Mutex				_lock;

		bool							_bQuit;
		// ZQ::common::Log*				_log;

		static CaptureThread*			_captureThread;
	};





}
}



#endif