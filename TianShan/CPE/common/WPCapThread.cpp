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
#include "WPCapThread.h"
#include "IPPacket.h"


#define WCAPThread			"WCAPThread"
#define MOLOG					(glog)



using namespace ZQ::common;
using namespace std;


namespace ZQTianShan {
	namespace ContentProvision {


union NUM64
{
	int64	n64;
	int32	n32[2];
};


void WinpCapThread::setLocalIP(const std::string& strLocalIp)
{
	_strLocalIp = strLocalIp;
	_multiCap.setLocalIP(strLocalIp);
}

std::string WinpCapThread::getLocalIP()
{
	return _strLocalIp;
}

bool WinpCapThread::initialize()
{
	if (!_multiCap.init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(WCAPThread, "Failed to initialize winpcap with error: %s"), _multiCap.getLastError().c_str());
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(WCAPThread, "Winpcap thread initialized successful on NIC %s"), _strLocalIp.c_str());
	return true;
}

int WinpCapThread::run()
{
//	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	MOLOG(Log::L_INFO, CLOGFMT(WCAPThread, "thread enter"));

	//
	// main loop
	//
	const unsigned char* pIPPacket;
	int nPacketLen;
	std::pair<CaptureContextMap::iterator, CaptureContextMap::iterator> pos;
	std::vector<CaptureContext*>	vCapErr;	//capture error context list
	std::vector<CaptureContext*>::iterator capIt;

	while(!_bQuit)
	{
		if ((nPacketLen = _multiCap.capture(pIPPacket))>0)
		{
			//parse ip packet
			ip_header* ih = (ip_header*)(pIPPacket);

			/* position of udp header */
			udp_header* uh = (udp_header*)((u_char*)(ih) + ((ih->ver_ihl & 0xf) * 4));

			/* position of data */
			const u_char* packet = (const u_char*)(uh) + sizeof(udp_header);
			int nUDPLen = nPacketLen - (packet - (u_char*)pIPPacket);

			//find capture context
			NUM64 nKey;
			nKey.n32[0] = *((int32*)&ih->daddr);
			nKey.n32[1] = uh->dport;
		
			_lock.enter();
			pos=_captureContexts.equal_range(nKey.n64);
			while(pos.first!=pos.second)
			{
				if (!pos.first->second->processData(packet, nUDPLen))
				{
					vCapErr.push_back(pos.first->second);
				}
				++pos.first;
			}

			_lock.leave();
			//if capture process error exist, remove it
			if (vCapErr.size())
			{
				for(capIt=vCapErr.begin();capIt!=vCapErr.end();capIt++)
				{
					unreg(*capIt);					
				}

				//clear the capture process error list for next
				vCapErr.clear();
			}
		}
		else if (!nPacketLen)  
		{
			continue;
		}
		else
		{
			MOLOG(Log::L_ERROR, CLOGFMT(WCAPThread, "capture error, exiting..."));
			break;
		}
	}

	MOLOG(Log::L_INFO, CLOGFMT(WCAPThread, "thread leave"));

	return 0;
}

void WinpCapThread::reg( CaptureContext* pContext )
{
	MOLOG(Log::L_INFO, CLOGFMT(WCAPThread, "register socket[%d]"), pContext->getSocket());

	NUM64 nKey;
	nKey.n32[0] = pContext->getMulticastAddr();
	nKey.n32[1] = pContext->getMulticastPort();

	CaptureContextMap::iterator it;
	Guard<Mutex>	opt(_lock);		
	_captureContexts.insert(std::make_pair(nKey.n64, pContext));
}

void WinpCapThread::unreg( CaptureContext* pContext )
{
	MOLOG(Log::L_INFO, CLOGFMT(WCAPThread, " unregister socket[%d]"), pContext->getSocket());

	NUM64 nKey;
	nKey.n32[0] = pContext->getMulticastAddr();
	nKey.n32[1] = pContext->getMulticastPort();

	Guard<Mutex>	opt(_lock);

	CaptureContextMap::iterator it=_captureContexts.find(nKey.n64);
	while (it!=_captureContexts.end())
	{
		if (it->second == pContext)
		{
			_captureContexts.erase(it);
			break;
		}

		it++;
	}
}

void WinpCapThread::close()
{
	if (!_bQuit)
	{
		MOLOG(Log::L_INFO, CLOGFMT(WCAPThread, "close enter"));

		_bQuit = true;
		waitHandle(-1);

		MOLOG(Log::L_INFO, CLOGFMT(WCAPThread, "close leave"));
	}
}

WinpCapThread::WinpCapThread()
{
	_bQuit = false;
	//_log = &glog;
}

void WinpCapThread::setLog( ZQ::common::Log* pLog )
{
	ZQ::common::setGlogger(pLog);

//	if (pLog)
//	{
//		_log = pLog;
//	}
}

bool WinpCapThread::start()
{
	return NativeThread::start();
}

void WinpCapThread::setMinBytesToCopy( int nBytes )
{
	_multiCap.setMinBytesToCopy(nBytes);
	MOLOG(Log::L_INFO, CLOGFMT(WCAPThread, "NIC[%s] setMinBytesToCopy(%d)"), _strLocalIp.c_str(), nBytes);
}

void WinpCapThread::setKernelBufferBytes( int nBytes )
{
	_multiCap.setKernelBufferBytes(nBytes);
	MOLOG(Log::L_INFO, CLOGFMT(WCAPThread, "NIC[%s] setKernelBufferBytes(%d)"), _strLocalIp.c_str(), nBytes);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//


bool WinpCapThreadInterface::init()
{
	Guard<Mutex>	opt(_lock);

	MOLOG(Log::L_INFO, CLOGFMT(WinpCapThreadInterface, "winpcap multicast capture initialize"));

	for(size_t i=0;i<_vNICInfo.size();i++)
	{
		WinpCapThread* pThread = new WinpCapThread();
		pThread->setLog(_log);
		pThread->setLocalIP(_vNICInfo[i].strLocalIp);
		if (_nMiniBytesToCopy)
			pThread->setMinBytesToCopy(_nMiniBytesToCopy);
		if (_nKernelBufferByte)
			pThread->setKernelBufferBytes(_nKernelBufferByte);
		
		if (pThread->initialize())
		{
			_capThreads.push_back(pThread);
			pThread->start();
		}
	}

	return (_capThreads.size()>0);
}

void WinpCapThreadInterface::close()
{
	Guard<Mutex>	opt(_lock);

	CaptureThreadList::iterator it=_capThreads.begin();
	for(;it!=_capThreads.end();it++)
	{
		WinpCapThread* pCapThread = *it;
		pCapThread->close();
		delete pCapThread;
	}

	_capThreads.clear();
}

void WinpCapThreadInterface::reg( CaptureContext* pContext )
{
	Guard<Mutex>	opt(_lock);

	CaptureThreadList::iterator it=_capThreads.begin();
	for(;it!=_capThreads.end();it++)
	{
		WinpCapThread* pCapThread = *it;
		if (pCapThread->getLocalIP() == pContext->getLocalIp())
		{
			pCapThread->reg(pContext);
			_regInfo[pContext] = pCapThread;
			
			return;
		}
	}

	MOLOG(Log::L_ERROR, CLOGFMT(WinpCapThreadInterface, "failed to register capture context with local ip %s"), pContext->getLocalIp().c_str());
}

void WinpCapThreadInterface::unreg( CaptureContext* pContext )
{
	Guard<Mutex>	opt(_lock);

	RegisterInfo::iterator it = _regInfo.find(pContext);
	if (it != _regInfo.end())
	{
		it->second->unreg(pContext);
		_regInfo.erase(it);
	}
}

void WinpCapThreadInterface::setMinBytesToCopy( int nBytes )
{
	_nMiniBytesToCopy = nBytes;
}

void WinpCapThreadInterface::setKernelBufferBytes( int nBytes )
{
	_nKernelBufferByte = nBytes;
}

WinpCapThreadInterface::WinpCapThreadInterface()
{
	_nMiniBytesToCopy = 0;
	_nKernelBufferByte = 0;
}

}}
