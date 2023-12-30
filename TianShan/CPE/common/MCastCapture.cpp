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
#include "MCastCapture.h"
#include <sstream>
#include "assert.h"
#include "BaseClass.h"
#include "CaptureInterface.h"
#include "IPPacket.h"

#ifdef ZQ_OS_MSWIN
#include <WS2TCPIP.H>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef ZQ_OS_MSWIN
#   ifndef socket_errno
#       define socket_errno WSAGetLastError()
#   endif
#else
#   ifndef socket_errno
#       define socket_errno errno
#   endif
#endif


#define McastImpl				"McastImpl"
#define MOLOG					(*_log)



using namespace ZQ::common;


namespace ZQTianShan 
{
	namespace ContentProvision
	{



MCastCapture::MCastCapture():
_socket(INVALID_SOCKET),
_stopCap(false) 
{
	_pMediaSample = NULL;
	_pMediaSamplePool = NULL;
//	ZQ::common::setGlogger();

	_bMulticastJoined = false;
}


MCastCapture::~MCastCapture()
{
	close();
}

bool MCastCapture::joinMulticast()
{
	std::ostringstream msg;

	// join the multicast
	struct ip_mreq mreq; 
	mreq.imr_multiaddr.s_addr = inet_addr (_mcastIp.c_str());
	mreq.imr_interface.s_addr = inet_addr(_localIp.c_str());

	if (setsockopt (_socket, 
		IPPROTO_IP, 
		IP_ADD_MEMBERSHIP, 
		(const char*)&mreq, 
		sizeof (mreq)) == INVALID_SOCKET) 
	{
		msg << "setsockopt failed: (" << socket_errno << ")";
		setLastError(msg.str());

		close();

		MOLOG(Log::L_INFO, CLOGFMT(McastImpl, 
                                "failed to join multicast group %s with interface %s, [%s]"), 
                                _mcastIp.c_str(), _localIp.c_str(), msg.str().c_str());
		return false;
	}

	_bMulticastJoined = true;
	MOLOG(Log::L_INFO, CLOGFMT(McastImpl, "join multicast group %s with interface %s successful"), _mcastIp.c_str(), _localIp.c_str());
	return true;
}

void MCastCapture::dropMulticast()
{
	if (_bMulticastJoined)
	{
		// drop the multicast
		struct ip_mreq mreq; 
		mreq.imr_multiaddr.s_addr = inet_addr (_mcastIp.c_str());
		mreq.imr_interface.s_addr = inet_addr(_localIp.c_str());

		if(_socket != INVALID_SOCKET) 
		{
			setsockopt (_socket, 
				IPPROTO_IP, 
				IP_DROP_MEMBERSHIP, 
				(const char*)&mreq, 
				sizeof (mreq)) ;

			MOLOG(Log::L_INFO, CLOGFMT(McastImpl, "drop multicast group %s with interface %s successful"), _mcastIp.c_str(), _localIp.c_str());
		}		

		_bMulticastJoined = false;
	}
}

bool MCastCapture::open(const std::string& localIP, const std::string& MCastIP, int port)
{
	if(MCastIP.empty() || port <= 0)
	{
		setLastError("invalid IP address or port number");
		return false;
	}

	if (!_pMediaSamplePool)
	{
		setLastError("MediaSamplePool not set");
		return false;
	}	
	_pMediaSample = _pMediaSamplePool->acquireOutputBuffer();
	if (!_pMediaSample)
	{
		setLastError("Failed to allocate media sample");
		return false;
	}

	std::ostringstream msg;
	// create socket handle for join multicast
	if ((_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_IP)) == INVALID_SOCKET) 
	{
		msg <<  "failed to create socket: (" << socket_errno << ")";
		setLastError(msg.str());
		msg.str("");

		return false;
	}

	// bind socket to local IP
	sockaddr_in local_sin;
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = 0;  
	local_sin.sin_addr.s_addr = inet_addr(localIP.c_str());

	if (::bind (_socket, 
            (struct sockaddr*)&local_sin, 
            sizeof (local_sin)) == INVALID_SOCKET) 
	{
		msg << "failed to bind socket: (" << socket_errno << ")";
		setLastError(msg.str());
		msg.str("");

		close();
		
		return false;
	}
	
	_mcastIp = MCastIP;
	_localIp = localIP;
	_mcastPortHost = (u_short)port;
	_mcastPortNet = ntohs(_mcastPortHost);
	_nMcastIp = inet_addr(MCastIP.c_str());

	return true;
}

void MCastCapture::close() 
{
	dropMulticast();

	if(_socket != INVALID_SOCKET) 
	{
#ifdef ZQ_OS_MSWIN
		closesocket(_socket);
#else
        ::close(_socket);
#endif
		_socket = INVALID_SOCKET;
	}
}

void MCastCapture::stop()
{
	if (MulticastCaptureInterface::instance())
	{
		MulticastCaptureInterface::instance()->unreg(this);
	}
	
	if (_pMediaSample)
	{
		_pMediaSamplePool->releaseOutputBuffer(_pMediaSample);
		_pMediaSample = NULL;
	}
}

bool MCastCapture::start()
{
	if (MulticastCaptureInterface::instance())
	{
		joinMulticast();

		MulticastCaptureInterface::instance()->reg(this);
	}

	return true;
}

SOCKET ZQTianShan::ContentProvision::MCastCapture::getSocket()
{
	return _socket;
}

std::string MCastCapture::getLocalIp()
{
	return _localIp;
}

unsigned int MCastCapture::getMulticastAddr()
{
	return _nMcastIp;
}

unsigned int MCastCapture::getMulticastPort()
{
	return _mcastPortNet;
}

void ZQTianShan::ContentProvision::MCastCapture::setLog( ZQ::common::Log* pLog )
{
	if (pLog)
	{
		_log = pLog;
	}
}

void ZQTianShan::ContentProvision::MCastCapture::setMediaSamplePool( MediaSamplePool* pSamplePool )
{
	_pMediaSamplePool = pSamplePool;
}

bool ZQTianShan::ContentProvision::MCastCapture::processData(const unsigned char* pUDPData, int nUDPLen)
{
	//
	// check after receive this buffer, whether exceed the buffer size
	//
	if(!_pMediaSample)
		return false;

	if( _pMediaSample->getFreeSize() > static_cast<size_t>(nUDPLen))
	{
		//not full 
		memcpy(_pMediaSample->getFreeBufferPointer(), pUDPData, nUDPLen);
		_pMediaSample->increaseDataLength(nUDPLen);
	}
	else 
	{			
		int cpySize = _pMediaSample->getFreeSize();
		int leftSize = nUDPLen - cpySize;

		// make the buffer full
		memcpy(_pMediaSample->getFreeBufferPointer(), pUDPData, cpySize);
		_pMediaSample->increaseDataLength(cpySize);
		
		// release the full buffer
		_pMediaSamplePool->releaseOutputBuffer(_pMediaSample);
		
		// acquire new buffer
		if (!(_pMediaSample = _pMediaSamplePool->acquireOutputBuffer()))
		{
			setLastError("Failed to allocate media sample");
			return false;
		}

		// copy the left buffer
		if(leftSize > 0) 
		{
			memcpy(_pMediaSample->getPointer(), pUDPData + cpySize, leftSize);
			_pMediaSample->setDataLength(leftSize);
		}							
	}

	return true;
}


}}
