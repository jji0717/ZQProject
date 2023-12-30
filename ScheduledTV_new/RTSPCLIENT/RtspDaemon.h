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
// Name  : RtspDaemon.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-6-9
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspDaemon.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// ===========================================================================


#ifndef _RTSPDAEMON_H_
#define _RTSPDAEMON_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif
#include <vector>

#include "nativethread.h"
#include "RtspClient.h"
#include "ScLog.h"

#define	ENVMASK_NONE	0
#define	ENVMASK_EXCP	1
#define	ENVMASK_SEND	2
#define	ENVMASK_RECV	4

typedef struct _cltEnv{
	RtspClient*	pClient;
	int			dwEnvMask;

	_cltEnv() { pClient = NULL; dwEnvMask = ENVMASK_SEND|ENVMASK_EXCP; }
	_cltEnv(RtspClient* pclt, int mask) { pClient = pclt; dwEnvMask = mask; }
	_cltEnv(const _cltEnv& _right) {	pClient = _right.pClient;  dwEnvMask = _right.dwEnvMask; }

	RtspClient*	getClient() { return pClient; }
	int&		mask() { return dwEnvMask; }
} cltEnv;

class RtspDaemon : public ZQ::common::NativeThread 
{

public:
	/// constructor
	RtspDaemon(int daemonid);

	/// destructor
	virtual ~RtspDaemon();

	static void setTrace(bool trace)
	{
		_trace = trace;
	}

public:
	//////////////////////////////////////////////////////////////////////////
	// deamon operations

	/// update the minimum heartbeat interval
	///@param[in]	hb		the heartbeat interval to set, in seconds
	void updateHb(DWORD hb);

	/// signal daemon to stop
	void signalStop(void);

	/// signal daemon to wakeup and re-scan the envelops
	void signalWakeup(void);

	/// get id of this daemon
	///@return		id number
	int	daemonId() { return _daemonId; }

public:
	//////////////////////////////////////////////////////////////////////////
	// client operations

	/// create a new RtspClient object and reg into this daemon
	///@param[in]	purchaseid		the purchase id of the client (=channel id in STV)
	///@param[in]	createnew		if true and there is already an client with this purchase id, return the exited one; if false, create a new one anyway 
	///@return						the pointer to the newly created client, or existed client if createnew=false, or NULL
	RtspClient*		createClient(DWORD	purchaseid, bool createnew = false);

	/// destroy an RtspClient object
	///@param[in]	exclient		the pointer to the client that need remove
	///@return						true if success, false else
	bool			destroyClient(RtspClient* exclient);

	/// query an existed RtspClient object according to purchase id
	///@param[in]	purchaseid		the purchase id of the client (=channel id in STV)
	///@return						the pointer to the existed client, or NULL if not found
	RtspClient*		queryClientByPurchase(DWORD	purchaseid);

	/// query an existed RtspClient object according to socket description
	///@param[in]	fd				the socket description of the client
	///@return						the pointer to the existed client, or NULL if not found
	RtspClient*		queryClientBySocket(SOCKET fd);

	/// destroy all RtspClient objects that belong to this daemon
	///@return						the number of clients destroyed
	int				destroyAllClients();

	/// get number of RtspClients that belong to this daemon
	///@return						the number of clients
	int				getClientNum();


private:
	friend bool RtspClient::sendMsg(const RtspRequest& msgin, RtspResponse& msgout);
	friend bool RtspClient::reset();
	// the following 3 envelop functions could only be called in sendMsg() methods if in class RtspClient
	
	/// register client envelop into daemon
	///@param[in]	client	the pointer to the client
	///@param[in]	mask	the env mask for this client, default is ENVMASK_NONE
	///@return				true if success, false else
	bool regEnv(RtspClient* client, int mask=ENVMASK_NONE);

	/// unregister client envelop from daemon
	///@param[in]	client	the pointer to the client
	///@return				true if success, false else
	bool unregEnv(RtspClient* client);

	/// update client envelop
	///@param[in]	client	the pointer to the client
	///@param[in]	mask	the env mask for this client
	///@param[in]	append	indicate if this new mask should be appended to origianl mask ('|' operator)
	///@return				true if success, false else
	bool updateEnv(RtspClient* client, int mask, bool append=false);

	/// deamon trace
	void trace(const char *fmt, ...);

protected:
	/// override function of nativeThread
	///@return true if pass the initialization steps
	virtual bool init(void);

	/// override function of nativeThread
	///@return the return value will also be passed as the thread exit code
	virtual int run(void);

	/// override function of nativeThread
	virtual void final(void);

private:
	/// minimum heartbeat interval of all clients that belong to this daemon, in seconds
	DWORD						_dwMinHb;
	
	/// identifier for this daemon object
	int							_daemonId;

	/// lock for client vector
	ZQ::common::Mutex			_cltLock;

	/// vector containing client envelops and mask
	std::vector<cltEnv>			_clients;

	/// indicate if daemon is running
	bool						_bIsRunning;
	
	/// handle of event that indicate the end of daemon
	HANDLE						_hExit;

	/// handle of event that indicate new request from client
	HANDLE						_hWakeup;

	/// time var to record last heartbeat sent time
	time_t						_lastHbTime;

	ZQ::common::ScLog*			_pTrace;

private:
	static  bool				_trace;
};

#endif // _RTSPDAEMON_H_
