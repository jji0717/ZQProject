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

#ifndef ZQTS_CPE_MULTICASTCAPTURE_H
#define ZQTS_CPE_MULTICASTCAPTURE_H


#include "CaptureInterface.h"
#include <string>

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

class MediaSample;

class MediaSamplePool
{
public:
    virtual ~MediaSamplePool() {}
	virtual MediaSample* acquireOutputBuffer() = 0;
	virtual void releaseOutputBuffer(MediaSample* pSample) = 0;
};

class MCastCapture: public CaptureContext
{
public:
	friend class CaptureThread;
	
	MCastCapture();
	virtual ~MCastCapture();
	
	void setLog(ZQ::common::Log* pLog);
	void setSockRecvBufSize(int nBufSize){_recvBufSize = nBufSize;}
	void setCaptureTimeout(int nSeconds);

	void setMediaSamplePool(MediaSamplePool* pSamplePool);

	bool open(const std::string& localIP, const std::string& MCastIP, int port);
	
	bool start();
	
	void stop();
	
	void close();
	
	std::string getLastError() const 
	{
		return _lastError;
	}

	virtual std::string getLocalIp();

	virtual unsigned int getMulticastAddr();

	// the network byte sequence of port
	virtual unsigned int getMulticastPort();

protected:
	virtual SOCKET getSocket();

	bool joinMulticast();
	void dropMulticast();
	
	void setLastError(const std::string& error) 
	{
		_lastError.assign(error);
	}

	//return true means there is data parsed
	bool		parseUDP(const unsigned char* pRawData, int nRawLen, const unsigned char*& pUDPData, int& nUDPLen);

	//return false only if the buffer allocate error
	bool		processData(const unsigned char* pUDPData, int nUDPLen);

private:
	
	bool				_bMulticastJoined;

	SOCKET				_socket;
	
	std::string			_lastError;

	bool				_stopCap;


	std::string			_localIp;
	std::string			_mcastIp;

	int32				_nMcastIp;
	u_short				_mcastPortHost;
	u_short				_mcastPortNet;
	int					_recvBufSize;

	ZQ::common::Log*	_log;

	MediaSample*		_pMediaSample;
	MediaSamplePool*	_pMediaSamplePool;
};




}}


#endif



