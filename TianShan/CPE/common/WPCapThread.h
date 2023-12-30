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

#ifndef ZQTS_CPE_WINPCAPTURETHREAD_H
#define ZQTS_CPE_WINPCAPTURETHREAD_H


#include "NativeThread.h"

#ifdef ZQ_OS_MSWIN
#include <hash_map>
#elif (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 4)
 #include <backward/hash_map>
#else
//#include <tr1/unordered_map>
 #include <backward/hash_map.h>
#endif

#include <map>
#include <vector>
#include "Locks.h"
#include "CaptureInterface.h"
#include "MulticastWPCAP.h"



namespace ZQTianShan 
{
namespace ContentProvision
{


class WinpCapThread : protected ZQ::common::NativeThread
{
public:
	WinpCapThread();

	virtual ~WinpCapThread()
	{
		close();
	}

	void setMinBytesToCopy(int nBytes);
	void setKernelBufferBytes(int nBytes);

	bool initialize();
	void setLog(ZQ::common::Log* pLog);
	void setLocalIP(const std::string& strLocalIp);
	std::string getLocalIP();

	void reg(CaptureContext* pContext);

	void unreg(CaptureContext* pContext);

	bool start();
	void close();

protected:
	virtual int run(void);

protected:

#ifdef ZQ_OS_MSWIN
	typedef stdext::hash_multimap<int64, CaptureContext*>	CaptureContextMap;
#else
//	typedef std::tr1::unordered_map<int64, CaptureContext*>	CaptureContextMap;
	typedef __gnu_cxx::hash_multimap<int64, CaptureContext*>	CaptureContextMap;
#endif

	CaptureContextMap				_captureContexts;

	ZQ::common::Mutex				_lock;
	
	MulticastWPCAP					_multiCap;
	
	std::string						_strLocalIp;
	

	bool							_bQuit;
	// ZQ::common::Log*				_log;
};



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//
class WinpCapThreadInterface : public MulticastCaptureInterface
{
public:
	WinpCapThreadInterface();

	void setMinBytesToCopy(int nBytes);
	void setKernelBufferBytes(int nBytes);

	virtual bool init();
	virtual void close();

	virtual void reg(CaptureContext* pContext);
	virtual void unreg(CaptureContext* pContext);

protected:

	typedef std::vector<WinpCapThread*> CaptureThreadList;
	CaptureThreadList			_capThreads;
	
	typedef std::map<CaptureContext*, WinpCapThread*>	RegisterInfo;
	RegisterInfo			_regInfo;

	int32							_nMiniBytesToCopy;
	int32							_nKernelBufferByte;

	ZQ::common::Mutex				_lock;
};


}
}



#endif
