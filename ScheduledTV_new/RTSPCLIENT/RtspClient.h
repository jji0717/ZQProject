// RtspClient.h: interface for the RtspClient class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _RTSPCLIENT_H_
#define _RTSPCLIENT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RtspRequest.h"
#include "RtspResponse.h"

#define	SOCKTRACE(x)	ZQ::common::glog(ZQ::common::Log::L_DEBUG, x)
class RtspDaemon;

class RtspClient  
{
friend class RtspDaemon;
	
public:
	enum Stat {
		CLIENT_DISCONNECT=0,	// client not connected with server
		CLIENT_CONNECTED,		// client connected, nothing sent
		CLIENT_READY,			// SETUP ok
		CLIENT_PLAYING,			// PLAY ok, streaming
		CLIENT_SUSPEND			// PAUSE, or end-of-stream/begin-of-stream ANNOUNCE reached
	};
	
private:
	/// constructor
	RtspClient(RtspDaemon& daemon);

public:
	/// destructor
	virtual ~RtspClient();

public:
	//////////////////////////////////////////////////////////////////////////
	// connection operations

	/// initialize the connection and connect to host, ready for operations
	///@param[in] host		the name or ip address of the host
	///@param[in] port		the destination port of the host Default value is 554
	///@param[in] nsec		the timeout, in msec. Default value is 20 seconds
	///@return				true if success, else false
	bool	open(const char* host, int port=RTSP_DEFAULT_PORT, DWORD nsec=RTSP_DEFAULT_NSEC);

	/// terminate the connection
	///@return				true if success, else false
	bool	close();

	/// terminate current connection, and initialize new connection
	///@return				true if success, else false
	bool	reset();
	
public:
	//////////////////////////////////////////////////////////////////////////
	// client operations

	/// send a RTSP request message to server and get response
	///@param[in] msgin		the request message that needs sent
	///@param[out] msgout	the response of this message from server
	///@return				true if success, false if failure or no response within _nsec time limit
	///@remarks		you may ignore the "CSeq" field \n
	/// if previous SETUP message had a valid session id return, you can also ignore "Session" field, this function will automatically fill
	bool	sendMsg(const RtspRequest& msgin, RtspResponse& msgout);
	
	/// calculate the mac addr of multicast ip
	///@param[in] ipaddr	the multicast ip address
	///@return				the mac address
	static std::string getMulticastMac(const char* ipaddr);
public:
	//////////////////////////////////////////////////////////////////////////
	// attribute functions (get/set)

	std::string&	host() { return _host; }

	int&			port() { return _port; }

	DWORD&			nsec() { return _nsec; }

	RtspDaemon&		daemon() { return _daemon; }

	SOCKET&			sd() { return _sd; }

	Stat&			status() { return _status; }

	int&			envMask() { return _envMask; }

	bool&			isProcessing() { return _isProcessing; }

	int&			sendingORrecving() { return _sendingORrecving; }
	
	bool&			traceFlag() { return _traceFlag; }

	std::string&	session() { return _szSess; }

	std::string&	seq() { return _szSeq; }

	DWORD&			purId() { return _dwPurId; }

	DWORD&			homeId() { return _dwHomeId; }

	DWORD&			hb() { return _dwHb; }

protected:
	//////////////////////////////////////////////////////////////////////////
	// message handle functions

	/// handle exception
	///@return				new envelop mask
	int		handleExcp(int errcode=0);

	/// handle message sending
	///@return				new envelop mask
	int		handleSend();

	/// handle message receiving
	///@return				new envelop mask
	int		handleRecv();

	/// handle heartbeat
	///@return				new envelop mask
	int		handleAlive();

protected:
	//////////////////////////////////////////////////////////////////////////
	// virtual callbacks

	/// called by myself, before sending request
	///@return				true if success, false else
	virtual bool	OnRequest(const RtspRequest& req);

	/// called by RtspDaemon, when response got
	///@return				true if success, false else
	virtual bool	OnResponse(const RtspResponse& res);

	/// called by RtspDaemon, when announce got
	///@return				true if success, false else
	virtual bool	OnAnnounce(const RtspRequest& ann);

	/// called by RtspDaemon, when exception got
	///@return				true if success, false else
	virtual bool	OnException();

protected:
	//////////////////////////////////////////////////////////////////////////
	// internal functions

	std::string		incSeq() 
	{ 	char buff[8]; std::string ret=_szSeq; _szSeq = itoa(atoi(_szSeq.c_str())+1, buff, 10); return ret; }
	
	void			trace(const char *fmt, ...);

private:	
	//////////////////////////////////////////////////////////////////////////
	// connection attributes

	/// server ip address
	std::string		_host;

	/// server port
	int				_port;

	/// timeout, in msec
	DWORD			_nsec;

private:
	//////////////////////////////////////////////////////////////////////////
	// client attributes
	
	/// reference to Daemon object this client belongs to
	RtspDaemon&		_daemon;
	
	/// socket descriptor
	SOCKET			_sd;

	/// status
	Stat			_status;

	/// client envelop mask
	int				_envMask;

	/// whether is sending data
	bool			_isProcessing;

	/// if _isProcessing==true, this value indicate client is sending(1) or recving(2) data. Otherwise 0
	int				_sendingORrecving;

	/// track flag, if set, log socket communication
	bool			_traceFlag;

private:
	//////////////////////////////////////////////////////////////////////////
	// message stubs

	/// last sending request
	RtspRequest		_msgRequest;

	/// last received response
	RtspResponse	_msgResponse;
private:
	//////////////////////////////////////////////////////////////////////////
	// rtsp attributes

	/// session info
	std::string		_szSess;

	/// sequence info
	std::string		_szSeq;

	/// heartbeat time interval, in seconds
	DWORD			_dwHb;

	/// purchase id (in STV, purchase id stands for channel id. purchseID/max_subchanel = channel id; purchaseID%max_subchanel = sub-channel id) \n
	/// i.e.   if max_subchanel=100 (default value):
	///        purchase id 300  --> channel 3, no sub-channel
	///        purchase id 1205 --> channel 12, sub-channel 5
	DWORD			_dwPurId;

	/// home id (in STV, home id stands for playlist type)
	DWORD			_dwHomeId;

private:	
	//////////////////////////////////////////////////////////////////////////
	// synchronize objects

	/// handle for event that inform data sending complete
	HANDLE			_hProCmpl;

	/// mutex for synchronize waiting response between different threads
	ZQ::common::Mutex		_lkWaitBlock;

};

#endif // _RTSPCLIENT_H_
