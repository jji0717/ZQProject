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


#include "Log.h"
#include "CaptureThread.h"


#define CaptureThd			"CaptureThd"
#define MOLOG					(glog)



using namespace ZQ::common;
using namespace std;


namespace ZQTianShan {
	namespace ContentProvision {


using namespace ZQ::common;


CaptureThread* CaptureThread::_captureThread = new CaptureThread();


int CaptureThread::run()
{
	//
	// local var
	// 
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = DefaultSelectTimeout*1000;

	fd_set	rFd;
	fd_set	rFdBak;

	MOLOG(Log::L_INFO, CLOGFMT(CaptureThread, "thread enter"));

	//
	// main loop
	//
	while(!_bQuit)
	{
		//get all sockets
		int nCount = getFDSET(rFd);
		if (!nCount)
		{
			//no socket
			sleep(DefaultSelectTimeout);
			continue;
		}

		//backup for iterator
		rFdBak.fd_count = nCount;
		memcpy(rFdBak.fd_array, rFd.fd_array, sizeof(SOCKET)*nCount);

		//wait for data
		if (::select(1 , &rFd , NULL, NULL, &timeout ) <= 0 )
		{
			continue;
		}

		SOCKET* pSocket = rFdBak.fd_array;
		SOCKET* pSocketMax = pSocket + nCount;
		Guard<Mutex>	opt(_lock);	
		CaptureContextMap::iterator it = _captureContexts.begin();		
		for(;pSocket<pSocketMax && it!=_captureContexts.end();pSocket++)
		{
			if (*pSocket != it->socket)
			{
				//current one removed
				continue;
			}

			if (!FD_ISSET(*pSocket, &rFd))
			{
				it++;
				continue;
			}
			
			if (it->capture->receive())
			{
				it++;
			}
			else
			{
				//remove from the list
				it = _captureContexts.erase(it);

				MOLOG(Log::L_ERROR, CLOGFMT(CaptureThd, "socket[%d] failed to receive data, unregister it"), *pSocket);
			}
		}
	}

	MOLOG(Log::L_INFO, CLOGFMT(CaptureThd, "thread leave"));

	return 0;
}

void CaptureThread::reg( CaptureContext* pContext )
{
	MOLOG(Log::L_INFO, CLOGFMT(CaptureThd, "register socket[%d]"), pContext->getSocket());

	CaptureInfo cInfo;
	cInfo.capture = pContext;
	cInfo.socket = pContext->getSocket();

	Guard<Mutex>	opt(_lock);	
	_captureContexts.push_back(cInfo);
}

void CaptureThread::unreg( CaptureContext* pContext )
{
	MOLOG(Log::L_INFO, CLOGFMT(CaptureThd, " unregister socket[%d]"), pContext->getSocket());

	Guard<Mutex>	opt(_lock);

	CaptureContextMap::iterator it=_captureContexts.begin();
	for (;it!=_captureContexts.end();it++)
	{
		if (it->capture == pContext)
		{
			_captureContexts.erase(it);
			break;
		}
	}
}

int CaptureThread::getFDSET(fd_set& fdset)
{
	Guard<Mutex>	opt(_lock);

	CaptureContextMap::iterator it=_captureContexts.begin();
	register int i;
	for(i=0;it!=_captureContexts.end()&&i<FD_SETSIZE;it++)
	{
		fdset.fd_array[i++] = it->socket;
	}
	fdset.fd_count = i;

	return i;
}

void CaptureThread::close()
{
	if (!_bQuit)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CaptureThd, "close enter"));

		_bQuit = true;
		waitHandle(INFINITE);

		MOLOG(Log::L_INFO, CLOGFMT(CaptureThd, "close leave"));
	}
}

CaptureThread::CaptureThread()
{
	_bQuit = false;
	// _log = &ZQ::common::NullLogger;
}

void CaptureThread::setLog( ZQ::common::Log* pLog )
{
	ZQ::common::setGlogger(pLog);
}

bool CaptureThread::start()
{
	return NativeThread::start();
}

CaptureThread& CaptureThread::instance()
{
	return *_captureThread;
}

void CaptureThread::destroyInstance()
{
	if (_captureThread)
	{
		delete _captureThread;
		_captureThread = NULL;
	}
}

}}