// ===========================================================================
// Copyright (c) 2005 by
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
// Ident : $Id: Daemon.h,v 1.0 2005/05/08 16:34:35 Gu Exp $
// Branch: $Name:  $
// Author: Hongye Gu
// Desc  : work node daemon,send info to manage node by heartbeat
//
// Revision History: 
// ---------------------------------------------------------------------------
#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "NativeThread.h"
using namespace ZQ::common;

#pragma warning(disable : 4786)
#include <vector>
#include <string>
using namespace std;

#include "TaskAcceptor.h"

#include "NativeThread.h"
USE_MPF_NAMESPACE

#include "SystemInfo.h"
USE_MPF_SYSTEMINFO_NAMESPACE

#define MPF_DEFAULT_WORKNODE_TAPORT		10000
#define	MPF_DEFAULT_HEARTBEAT_TIME		1000*60		// 1 min
#define MPF_MINIMUM_HEARTBEAT_TIME		1000*5		// 5 seconds
#define MPF_DEFAULT_LEASE_FACTOR		750/1000	// 0.75
#define	MPF_DEFAULT_WORKNODE_TIME_OUT	5			// 5 seconds

MPF_WORKNODE_NAMESPACE_BEGIN

///@mainpage MPF : Work Node SDK
///@section Background
/// An MPF worker node is a machine that has at least one application invoking MPF Task control running on it. Those applications are called as MPF worker application
/// MPF Work Node SDK provide basic and scalable APIs for work management and function integreation with MPF SRM applications
///@section References
/// Help file for MPF SRM SDK \n
/// MPF application design doc \n
/// XML-RPC interface document \n

//////////////////////////////////////////////////////////////////////////
// class heartbeat
class Daemon;
/// HbTrigger resides in Daemon object, running as a thread to trigger heartbeat
class DLL_PORT HbTrigger: public ZQ::common::NativeThread
{
public:
	/// constructor
	///param[in]		daemon		-the daemon object which gives birth to this
	HbTrigger(Daemon& daemon);

	/// destructor
	virtual ~HbTrigger();

protected:
	//////////////////////////////////////////////////////////////////////////
	/// derived from NativeThread
	virtual bool				init(void);

	/// derived from NativeThread
	virtual int					run(void);

	/// derived from NativeThread
	virtual void				final(void);

private:
	/// reference to daemon object which gives birth to this
	Daemon&						_daemon;
};

//////////////////////////////////////////////////////////////////////////
// class Deamon
///@brief Deamon runs as a singleton on a work node, which handles heartbeat issues
/// and works as the contact window for manager nodes
class DLL_PORT Daemon : public ZQ::common::NativeThread,
						public ZQ::rpc::RpcServer	
{
friend class HbTrigger;
public:
	/// constructor
	///param[in]		worknodeUrl		-the URL string containing work node information where this Daemon runs
	Daemon(const char* worknodeUrl);

	/// destructor
	virtual ~Daemon();

public:

	bool						query(const char* mgmURL, const RpcValue& param, RpcValue& result);

	/// register task acceptor object
	///param[in]	pAcceptor		-the pointer of the task acceptor
	void						regAcceptor(TaskAcceptor* pAcceptor);

	/// stop this daemon
	void						stop();

	//////////////////////////////////////////////////////////////////////////
	
	/// user derived Daemon class could override this function to compose customized heartbeat information
	///@param[out]	out			-output parameter to received the composed data
	virtual void				OnUserHeartbeat(RpcValue& out){}

	//////////////////////////////////////////////////////////////////////////
	
	/// get work node ID of which this daemon runs on
	///@return					-the work node ID string
	const char*					getWorkNodeID() { return m_WorkNodeID; }

	/// get the URL which stands for this work node, usually 'MPF:://<ip>:<port>/...'
	/// this string is passed in via constructor
	///@return					-the work node URL string
	const char*					getWorkNodeURL() { return m_WorkNodeURL.c_str(); }

	//////////////////////////////////////////////////////////////////////////
	// manage node control

	/// add a new manager node to this daemon
	///param[in]	mgmURL		-the URL which stands for manager node, usually 'MPF:://ip:port/...'
	///return					-True if successfully, False else
	bool						addMgmNode(const char* mgmURL);

	/// remove a new manager node from this daemon
	///param[in]	mgmURL		-the URL which stands for manager node, usually 'MPF:://ip:port/...'
	///return					-True if successfully, False else
	bool						removeMgmNode(const char* mgmURL);

	//////////////////////////////////////////////////////////////////////////
	/// force timeout, maybe useful for dummy session
	///param[in]	timeout		-timeout in msec, -1 indicating dummy session (not send heartbeat)
	void						setLeaseTerm(timeout_t timeout) { m_LeaseTerm = timeout; }
	
	/// get timeout
	///return					-timeout in msec, -1 indicating dummy session (not send heartbeat)
	timeout_t					getLeaseTerm() { return m_LeaseTerm; }

protected:
	//////////////////////////////////////////////////////////////////////////
	/// derived from NativeThread
	virtual bool				init(void);

	/// derived from NativeThread
	virtual int					run(void);

	/// derived from NativeThread
	virtual void				final(void);

private:
	/// send heartbeat to all manage nodes
	///return					-zero if success, non-zero if failed
	int							heartbeat();
	
private:
	/// pointer to related heartbeat trigger object
	HbTrigger*					m_pHeartbeat;

	/// if should stop
	BOOL						m_bStop;

	/// event for wake up from sleeping
	HANDLE						m_hWakeUp;

	/// lease term (timeout) for this daemon, heartbeat frequency should be less than this value
	timeout_t					m_LeaseTerm;
protected:	
	/// pointer to related task acceptor object
	TaskAcceptor*				m_pTaskAcceptor;

	
	/// manage nodes info
	vector<string>				m_ManageNodes;

	/// work node id
	char						m_WorkNodeID[MPF_NODEID_LEN];

	/// work node url, containing ip and port at least
	std::string					m_WorkNodeURL;

	/// work node network interface index number
	int							m_nInterfaceIndex;

};

MPF_WORKNODE_NAMESPACE_END

#endif
