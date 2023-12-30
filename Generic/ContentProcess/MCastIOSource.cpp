// ============================================================================================
// Copyright (c) 2006, 2007 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Ken Qian
// Desc  : Implement the RFTLib Filter
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/MCastIOSource.cpp 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/MCastIOSource.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 13    08-01-16 15:22 Fei.huang
// add: adjust session time 
// 
// 12    08-01-02 18:35 Ken.qian
// change since minAvailBuffCount was move to Graph
// 
// 11    07-12-12 16:31 Fei.huang
// skip bitrate check if bitrate fluctuate not set
// 
// 10    07-12-12 12:12 Ken.qian
// move initialization codes before multicast capturing
// 
// 9     07-12-10 12:35 Fei.huang
// 
// 8     07-11-28 10:54 Fei.huang
// add bitrate limit 
// 
// 7     07-11-19 16:23 Fei.huang
// 
// 6     07-11-16 20:36 Ken.qian
// add log to state the capture start time
// 
// 5     07-11-16 20:25 Ken.qian
// 
// 4     07-11-16 19:16 Ken.qian
// change getTotalStuff()
// 
// 3     07-11-15 18:55 Ken.qian
// checkin after pass unittest with utility
// 
// 2     07-11-15 15:16 Ken.qian
// check in after passing unit test
// 
// 1     07-11-14 21:09 Ken.qian
// Initial check in based on Fei Huang's codes

#include "MCastIOSource.h"
#include <sstream>
#include "urlstr.h"
#include "TimeUtil.h"
#include "GraphFilter.h"

namespace {
	const unsigned BITRATE_THRESHOLD = 2*1024*1024; /* in bytes */
}

#define  MIN_FILE_SIZE_FOR_TIMEOUT    1024*1024   // 1M


#define PACKET_SIZE           65536
#define ETHERNET_HEADER_SIZE  14
#define READ_TIMEOUT          1000  /* ms */

/******************************
*	MCastCapture
*/

pcap_if_t* MCastCapture::_allDevs = NULL;
pcap_if_t* MCastCapture::_bindDev = NULL;
bpf_u_int32 MCastCapture::_netmask = 0;
std::string  MCastCapture::_localIP = "";

MCastCapture::MCastCapture():
_socket(INVALID_SOCKET),
_handle(NULL),
_stopCap(false) {
}

MCastCapture::~MCastCapture() {
	close();
}

bool MCastCapture::initReceiver(std::string& localIP, std::string& errstr)
{
	_localIP = localIP;

	WSADATA WSAData;

	// starts use of WS2_32.DLL
	if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) {

		std::string lastError;
		ZQ::Content::Process::getSystemErrorText(lastError);

		errstr = "MCastIOSource: WSAStartup failed with error " + lastError;

		return false;
	}

	if(_localIP.empty()) {
		char host[101];
		gethostname(host, 100);

		hostent* entry = gethostbyname(host);
		in_addr addr;

		addr.s_addr = *(u_int*)(entry->h_addr);
		_localIP = inet_ntoa(addr);

		localIP = _localIP;
	}

	// initialize winpcap to bind local ip
	char buf[PCAP_ERRBUF_SIZE];
	if(pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &_allDevs, buf) == (-1)) {

		errstr = buf;
		
		uninitReceiver();
		return false;
	}

	// get the local binded device
	for(pcap_if_t* dev = _allDevs; dev != NULL; dev = dev->next) {

		pcap_addr_t* addr;
		for(addr = dev->addresses; addr; addr = addr->next) {

			if(addr->addr->sa_family == AF_INET) {
				
				char* interfaceIP = inet_ntoa(((struct sockaddr_in*)addr->addr)->sin_addr);
				if(localIP == interfaceIP) {
					_bindDev = dev;
					_netmask = ((struct sockaddr_in*)(addr->netmask))->sin_addr.S_un.S_addr;
	
					break;
				}
			}
		} /* end for (addresses) */
	} /* end for (devices) */

	if(!_bindDev) {
		errstr = "can't find a interface to bind";
		return false;
	}

	return true;	
}

void MCastCapture::uninitReceiver()
{
	if(_allDevs) {
		pcap_freealldevs(_allDevs);
		_allDevs = 0;
		_bindDev = 0;
	}

	// terminates use of WS2_32.DLL
	WSACleanup();
}

bool MCastCapture::open(const std::string localIP, const std::string& MCastIP, int port) {
	
	if(MCastIP.empty() || port <= 0) {
		setLastError("invalid IP address or port number");

		return false;
	}

	if(!_bindDev) {
		setLastError("no interface bind");

		return false;
	}

	std::ostringstream msg;
	// create socket handle for join multicast
	if ((_socket = socket (AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		msg <<  "failed to create socket: (" << WSAGetLastError() << ")\n";
		setLastError(msg.str());
		msg.str("");

		return false;
	}

	// bind socket to local IP
	SOCKADDR_IN local_sin;
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = htons(INADDR_ANY);  
	local_sin.sin_addr.s_addr = inet_addr(localIP.c_str());

	if (::bind (_socket, 
            (struct sockaddr*)&local_sin, 
            sizeof (local_sin)) == SOCKET_ERROR) {
		msg << "failed to bind socket: (" << WSAGetLastError() << ")";
		setLastError(msg.str());
		msg.str("");

		close();
		
		return false;
	}
	
	// join the multicast
	struct ip_mreq mreq; 
	mreq.imr_multiaddr.s_addr = inet_addr (MCastIP.c_str());
	mreq.imr_interface.s_addr = INADDR_ANY;

	if (setsockopt (_socket, 
                  IPPROTO_IP, 
                  IP_ADD_MEMBERSHIP, 
                  (const char*)&mreq, 
                  sizeof (mreq)) == SOCKET_ERROR) {
		msg << "setsockopt failed: (" << WSAGetLastError() << ")";
		setLastError(msg.str());

        close();
		
		return false;
	}

	// open winpcap handle
	char buf[PCAP_ERRBUF_SIZE];
	if((_handle = pcap_open(
					_bindDev->name, 
					PACKET_SIZE, 
					PCAP_OPENFLAG_PROMISCUOUS,
					READ_TIMEOUT, 
					NULL, 
					buf)) == NULL) {
		
		setLastError(buf);
		
		close();

		return false;
	}

    if(pcap_datalink(_handle) != DLT_EN10MB) {
		setLastError("only support Ethernet network.");
        
		close();
        
		return false;
    }

	std::ostringstream filter;
	filter << "ip multicast and dst host " << MCastIP << " and dst port " << port;

	struct bpf_program code;
	if(pcap_compile(_handle, &code, const_cast<char*>(filter.str().c_str()), 1, _netmask) < 0) {
		setLastError(pcap_geterr(_handle));
		
		close();

		return false;
	}

	if(pcap_setfilter(_handle, &code) < 0) {
		setLastError(pcap_geterr(_handle));

		close();
		
		return false;
	}
	
	// set the flag
	_stopCap = false;

	return true;
}

void MCastCapture::close() {

	if(NULL != _handle)	{
		pcap_close(_handle);
		_handle = 0;
	}

	if(_socket != INVALID_SOCKET) {
		closesocket(_socket);
	}
}

bool MCastCapture::capture(int timeOutMs) {

	if(NULL == _handle) {
		setLastError("invalid handle");

		return false;
	}

	struct pcap_pkthdr* header;
	const u_char* data;

	PWPCAPBUF   curWAPBUF = NULL;
	DWORD       recdBytes = 0;

	DWORD       cpySize = 0;
	DWORD       leftSize = 0;

	DWORD       lastPackTKCount = GetTickCount();

	/*
	*  get WPCAPBUF buffer for output
	*/
	curWAPBUF = acquireOutputBuffer();
	if(NULL == curWAPBUF) {
		return false;
	}

	u_int lastTimer = GetTickCount();
	while(!_stopCap) {

		/*
		*   check timeout 
		*/
		if( (GetTickCount() - lastPackTKCount) > (DWORD)timeOutMs ) {

			if(NULL != curWAPBUF) {

				releaseOutputBuffer(curWAPBUF, recdBytes);

				setLastError("capture data timeout");

				return true;
			}
		}

		/*
		*   capture packet
		*/
		int res = pcap_next_ex(_handle, &header, &data);

		if(res == 0) {
			continue;
		}
		else if(res == (-1)) {
			setLastError(pcap_geterr(_handle));
			return false;
		}

		/*
		*	decode data from packet
		*/

		/* position of ip header */
		ip_header* ih = (ip_header*)(data + ETHERNET_HEADER_SIZE);

		/* position of udp header */
		udp_header* uh = (udp_header*)((u_char*)(ih) + ((ih->ver_ihl & 0xf) * 4));
		
		/* position of data */
		u_char* packet = (u_char*)(uh) + sizeof(udp_header);
		u_int len = header->caplen - (packet - data);
				
		// no packet was captured, continue to capture
		if(len == 0) {
			continue;
		}
		
		//
		// check after receive this buffer, whether exceed the buffer size
		//
		if(curWAPBUF->dwLength > (recdBytes + len) ) {
			// curWAPBUF does not full yet 
			memcpy(curWAPBUF->pBuffer+recdBytes, packet, len);
			recdBytes += len;
		}
		else {
			cpySize = curWAPBUF->dwLength - recdBytes;
			leftSize = len - cpySize;

			// make the buffer full
			memcpy(curWAPBUF->pBuffer+recdBytes, packet, cpySize);
			recdBytes = curWAPBUF->dwLength;
			
			// release the full buffer
			releaseOutputBuffer(curWAPBUF, recdBytes);
			
			/*
			*	check bandwidth before acquire new buffer
			*/
			if(!checkBitrate(lastTimer)) {
				return false;
			}

			// acquire new buffer
			curWAPBUF = acquireOutputBuffer();
			if(NULL == curWAPBUF) {
				return false;
			}

			// copy the left buffer
			recdBytes = 0;
			if(leftSize > 0) {
				memcpy(curWAPBUF->pBuffer, packet+cpySize, leftSize);
				recdBytes = leftSize;
			}
		}

		// remember the last capture time
		lastPackTKCount = GetTickCount();
	}

	// release the buffer
	releaseOutputBuffer(curWAPBUF, recdBytes);

	return true;
}


/******************************
*	MCastIOSource
*/

namespace ZQ { 
namespace Content { 
namespace Process {

MCastIOSource::MCastIOSource(
		Graph& graph, 
		u_int timeout,
		u_int bitrateFluctuate,
		const std::string& proto):
SourceFilter(graph, proto, "MCastIOSource"), 
_hNotify(0),
_hQuit(0),
_quit(false),
_abortedBy(0),
_readSize(0),
_maxbps(0),
_timeout(timeout),
_processedBytes(0), 
_totalBytes(0),
_started(false),
_bitrateFluctuate(bitrateFluctuate) {
	
	_readSize = _pool.getPreAllocBufferSize();

	_hQuit = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);
	_timeChanged = CreateEvent(NULL, false, false, NULL);


	start();
}

MCastIOSource::~MCastIOSource() {
	if(isRunning()) {
		
		SetEvent(_hQuit);
		
		resume();
		
		waitHandle(INFINITE);
	}
	
	close();
	
	if(_hQuit != NULL) {
		CloseHandle(_hQuit);
		_hQuit = NULL;
	}

	if(_hNotify != NULL) {
		CloseHandle(_hNotify);
		_hNotify = NULL;
	}

	if(_timeChanged) {
	   CloseHandle(_timeChanged);
	   _timeChanged = 0;
	}
}

bool MCastIOSource::initMCastReceiver(std::string& localIP, std::string& errstr)
{
	return MCastCapture::initReceiver(localIP, errstr);
}

void MCastIOSource::uninitMCastReceiver()
{
	MCastCapture::uninitReceiver();
}

void MCastIOSource::notifyTimeChanged() {
	if(_started) {
		_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), 
						"unable to change start time of session (%s), already started",
						_graph.getContentName().c_str());
		return;
	}
	SetEvent(_timeChanged);
}

bool MCastIOSource::begin() {
	_graph.writeLog(ZQ::common::Log::L_INFO, 
			GetCurrentThreadId(), "MCastIOSource::begin() enter");
	
	if(_processStatus == ACTIVE) {
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), 
				"MCastIOSource: task not completed yet, can not start new work");

		return false;
	}
	
	if(_processStatus != PAUSED) {
		
		_endofStream = false;
		_buffCount = 0;
		_processedBytes = 0;
		_totalBytes = 0;

		_maxbps = _graph.getMaxbps();
		
	} /* _processStatus != PAUSED */
	else {
		start();
	}
	
	_processStatus = ACTIVE;
	
	SetEvent(_hNotify);		

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "MCastIOSource::begin() leave");
	
	return true;
}

bool MCastIOSource::pause() {
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "MCastIOSource::pause()");

	_processStatus = PAUSED;

	suspend();
		
	return true;
}

bool MCastIOSource::abort() {
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "MCastIOSource::abort()");
		
	_abortedBy = GetCurrentThreadId();

	_processStatus = ABORTED;

	// stop the capture in another thread (called by Graph)
	MCastCapture::stop();

 	SetEvent(_hNotify);

	return true;
}

void MCastIOSource::stop() {
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "MCastIOSource::stop() enter");
		
	_abortedBy = GetCurrentThreadId();

	// stop the capture in another thread (called by Graph)
	MCastCapture::stop();

	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "MCastIOSource::stop() leave");
}


void MCastIOSource::quit() {
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "MCastIOSource::quit() enter");

	SetEvent(_hQuit);

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "MCastIOSource::quit() leave");
}

void MCastIOSource::endOfStream(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "MCastIOSource: Get end of stream notification");

	_endofStream = true;

	// stop the capture in another thread (called by Graph)
	MCastCapture::stop();

	SetEvent(_hNotify);
}

int MCastIOSource::run() {
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "MCastIOSource::run() enter");

	HANDLE handles[3] = { _hQuit, _hNotify, _timeChanged };
	
	u_int status = 0;
	
	u_long timeout = INFINITE;
	time_t startPoint = 0;

	while(!_quit) {

		status = WaitForMultipleObjects(3, handles, false, timeout);
		
		switch(status) {
		
		case WAIT_OBJECT_0:
			_quit = true;
			
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), 
						"MCastIOSource: get a thread exit event");
			break;

		case WAIT_OBJECT_0 + 1:
			{
				// control the avaliable buffer count
				while( (STOPPED != _processStatus || ABORTED != _processStatus)
					&& (_pool.getPoolSize() - _pool.getUsedCount()) < _graph.getReservedBuffCount() ) {
					
					Sleep(DEFAULT_SLEEP_TIME);
				}

				if(STOPPED == _processStatus || ABORTED == _processStatus) {

					// close the capture handle					
					MCastCapture::close();

					if(ABORTED == _processStatus)
					{
						_graph.writeLog(ZQ::common::Log::L_INFO, id(), "MCastIOSource: It was aborted by Graph, triggered by thread 0x%08X", _abortedBy);
					}
					
					// notify graph this filter processing completed
					_graph.notifyCompletion(*this);

					_started = false;

					continue;

				}

				if(_started) {
					/*
					*	init the capture
					*/
					// capture initialization (MCastCapture::open::open) must be invoked before capturing(MCastCapture::capture())
					// coz if in begin, there is possible sleep between initialization and capturing, that will cause duty data captured, 
					// and fail to do trick generation. 
					std::string srcPath = _graph.getSourceURL();
					ZQ::common::URLStr srcUrl(srcPath.c_str());
					
					std::string mcIP = srcUrl.getHost();
					int mcPort = srcUrl.getPort();

					bool capRet = true;

					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "MCastIOSource: init capture on LocalIP=%s, MulticastIP=%s, MulticastPort=%d", 
						_localIP.c_str(), mcIP.c_str(), mcPort);

					if(!MCastCapture::open(_localIP, mcIP, mcPort)) {

						_graph.writeLog(ZQ::common::Log::L_ERROR, 
								GetCurrentThreadId(), "MCastIOSource: open WinpCAP capture failed with error %s", MCastCapture::getLastError().c_str());

						_graph.setLastError(ERRCODE_INVALID_URL, MCastCapture::getLastError());

						capRet = false;
					}
					
					// reach the starTime, just do capture
					// capture is a block function, it returns when MCastCapture::stop() was called or it met error/timeout
					if(capRet)
					{
						_graph.writeLog(ZQ::common::Log::L_INFO, id(), "MCastIOSource: it's time to capture packet");

						capRet = MCastCapture::capture(_timeout);
					}

					// capture() return true, if timeout, check whether there already received some bytes.
					if(!capRet || (capRet && (_processedBytes < MIN_FILE_SIZE_FOR_TIMEOUT)) ) {

						_graph.setLastError(ERRCODE_READFILE_FAIL, MCastCapture::getLastError());
						
						_graph.writeLog(ZQ::common::Log::L_ERROR, id(), 
								"MCastIOSource: capture packet failed with error: (%s) trigger Graph abort()", 
								MCastCapture::getLastError().c_str());
						
						_graph.abortProvision();

						continue;	
					}
					
					// set this after reach the end of stream
					_totalBytes = _processedBytes;

					// update the FileSize to Graph
					ContentProperty filesizecp;
					filesizecp.insert(ContentProperty::value_type(CNTPRY_FILESIZE, _totalBytes));
					_graph.reportProperty(_graph.getContentName(), filesizecp);

					// end of stream 
					notifyEndOfStream();

					_buffCount = 0;
					_endofStream = false;
					
					// close the capture handle					
					MCastCapture::close();
					
					// set the status
					_processStatus = STOPPED;
					
					_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "MCastIOSource: reaches the end of stream, there are %d buff read, total size is %I64d", 
						_buffCount, _totalBytes);

					// notify graph this filter processing completed
					_graph.notifyCompletion(*this);

					continue;
				}
				else {
					// to start capture at provision statTime
					SYSTEMTIME __t;
					GetLocalTime(&__t);

					__int64 now = (time(0) * 1000) + __t.wMilliseconds;
					__int64 start = _graph.getStartTime() * 1000;

					if(start > now) {
						timeout = static_cast<DWORD>(start - now);

						_graph.writeLog(ZQ::common::Log::L_DEBUG, id(),
								"not the time to start, will sleep for (%u) millseconds", timeout);

						continue;
					}
					_started = true;
					SetEvent(_hNotify);
				}
			}
			break;
			
		case WAIT_OBJECT_0 + 2:
			/* 
			*	only start time change makes sense here,
			*	cause we don't care about duration and will
			*	be notified by graph once time is due.
			*	
			*	if the capture started already, nothing to do here.
			*	
			*/
			{
			char d[100];
			ZQ::common::TimeUtil::Time2Str(_graph.getStartTime(), d, 100);

			_graph.writeLog(ZQ::common::Log::L_DEBUG, id(),
					"now: %u\tnewStart: %u\tnewStart: %s\ttimeout: %u",
					time(0), _graph.getStartTime(), d, timeout);
		   

			SYSTEMTIME __t;
			GetLocalTime(&__t);

			__int64 now = (time(0) * 1000) + __t.wMilliseconds;
			__int64 start = _graph.getStartTime() * 1000;

			/* postponed */
			if(start > now) {
				timeout = static_cast<DWORD>(start - now);	
			
				_graph.writeLog(ZQ::common::Log::L_DEBUG, id(),
					"(%s) session start time changed, sleep for another (%ums)", 
					_graph.getContentName().c_str(),
					timeout);
			}
			/* ahead */
			else {
				timeout = 0;

				_graph.writeLog(ZQ::common::Log::L_DEBUG, id(),
					"(%s) session start time changed, start immediately", 
					_graph.getContentName().c_str());
			}
			
			break;
			}

		case WAIT_TIMEOUT:
			timeout = INFINITE;
			_started = true;
			SetEvent(_hNotify);

			break;

		case WAIT_FAILED:
		default:
			_quit = true;
			break;
		}		
	}

	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "MCastIOSource::run() leave");

	return 1;
}

bool MCastIOSource::checkBitrate(u_int lastTimer) {
	/* 
	*	return immediately if bps not set or check already passed 
	*/
	/*
	if(!_maxbps || 
	   (_processedBytes % BITRATE_THRESHOLD) || 
	   (_processedBytes > BITRATE_THRESHOLD)) {
	
		return true;
	}
	*/
	if(!_maxbps || !_bitrateFluctuate || (_processedBytes != BITRATE_THRESHOLD)) {
		return true;
	}

	DWORD now = GetTickCount();
	
	/*
	*	to avoid LARGE value for unsigned variable or divided by zero later
	*/
	DWORD interval = (now > lastTimer) ? (now - lastTimer) : 0u;
	if(interval) {
		u_int actual = static_cast<unsigned>((_processedBytes*8)/((now-lastTimer)/1000));

		_graph.writeLog(ZQ::common::Log::L_INFO, id(), 
					"checking bitrate [expect (%ubps) actual (%ubps)", 
					_maxbps, actual);

		if(actual > _maxbps + _bitrateFluctuate) {
			std::ostringstream os;
			os << "MCastIOSource: expect bitrate (" 
			   << _maxbps 
			   << "bps) but ("
		       << actual
		       << "bps) used";

			setLastError(os.str());
		
			return false;
		}
	}

	return true;
}

PWPCAPBUF MCastIOSource::acquireOutputBuffer()
{
	ZQ::Content::BufferData* pBuffData = _pool.alloc(DEFAULT_BUFF_WAIT_TIMEOUT);	
	if(0 == _inUseBufferNo)
		_inUseBufferNo = 1;
	else
		_inUseBufferNo = 0;

	_wpcapBuffers[_inUseBufferNo].buffContext = (PVOID) pBuffData;
	_wpcapBuffers[_inUseBufferNo].pBuffer = pBuffData->getPointerForWrite(_readSize);
	_wpcapBuffers[_inUseBufferNo].dwLength = _readSize;

	return &_wpcapBuffers[_inUseBufferNo];
}

void MCastIOSource::releaseOutputBuffer(PWPCAPBUF wpcapBuffer, DWORD dataLen)
{
	ZQ::Content::BufferData* pBuffData = (ZQ::Content::BufferData*) wpcapBuffer->buffContext;

	if(0 == dataLen)
	{
		_pool.free(pBuffData);  // free directly
		return;
	}

	pBuffData->setActualLength(dataLen);

	// pass the buff data to renders
	deliverBuffer(pBuffData);
	
	// release the buffdata
	bool bReleased = releaseBuffer(pBuffData);
	if(bReleased)
	{
		_graph.traceLog(id(), "MCastIOSource: free buffData from pool. [BuffData Address: 0x%08X]", pBuffData);
	}

	if(_buffCount % DEFAULT_FILETER_LOGING_FEQ == 0)
	{
		_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "MCastIOSource: processBuffer No.%d", 
			_buffCount);
	}

	_buffCount++;
	_processedBytes += dataLen;
}


}}}