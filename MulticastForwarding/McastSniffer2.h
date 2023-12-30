// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: McastSniffer.h,v 1.4 2004/11/03 09:40:04 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : define the multicast sniffer
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastSniffer2.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     04-11-22 20:07 Hui.shao
// 1 procedure vs m consumers mode
// 
// 4     04-11-18 16:21 Hui.shao
// renamed openListener()
// 
// 3     04-11-08 15:39 Hui.shao
// defined MsgInfo, added comments
// 
// 2     04-11-04 20:59 Hui.shao
// fire the event in the stop()
// 
// 1     04-11-03 17:05 Hui.shao
// 
// ===========================================================================

#ifndef	__ZQ_McastSniffer_H__
#define	__ZQ_McastSniffer_H__


#include "MCastFwdConf.h"
#include "Thread.h"
#include "UDPSocket.h"
#include "Locks.h"

#include <map>
#include <queue>

// -----------------------------
// class McastSubscriber
// -----------------------------
/// 
class McastSubscriber
{
public:
	typedef struct _msg_info
	{
		ZQ::common::InetHostAddress  source;
		int							 sport;
		ZQ::common::InetMcastAddress group;
		int							 gport;
		int                          datalen;
		char*                        data;
	} McastMsgInfo;

	virtual int OnMcastMessage(const McastMsgInfo& MsgInfo) { return MsgInfo.datalen; }
};

typedef McastSubscriber::McastMsgInfo McastMsgInfo;

// -----------------------------
// class McastMsgDispatcher
// -----------------------------
/// Thread McastMsgDispatcher launched by McastSniffer to read from message queue then
/// fire event to McastSubscribers
#define SNIFFER_THRD_TEST

#ifndef _DEBUG
#  undef SNIFFER_THRD_TEST
#endif // _DEBUG

class McastMsgDispatcher : public ZQ::common::Thread
{
	friend class McastSniffer;

protected:

	bool _bQuit;
	HANDLE _hStopped;

private:
	// accept accesses only from McastSniffer

	McastMsgDispatcher(McastSniffer& Sniffer);
	virtual ~McastMsgDispatcher();

	void stop();

protected:

	/// primary processes to read from the queue then calls the subscribers
	virtual int run();
	virtual void final();

	McastSniffer& _sniffer;

#ifdef SNIFFER_THRD_TEST
	int _thid;
	static int _th_count;
#endif // SNIFFER_THRD_TEST
};

// -----------------------------
// class McastSniffer
// -----------------------------
/// control class to sniff multicast traffic, support multiple group/port combinations.
class McastSniffer : public ZQ::common::Thread
{
	friend class McastMsgDispatcher;

public:
	/// constructor
	McastSniffer(int MsgDispatcherNum =3);
	/// destructor
	~McastSniffer();

	/// open a group/port of multicast traffic interested
	///@param group       the interest multicast IP address
	///@param group_port  the interest multicast port number
	///@param bind        the local IP address to bind the listener
	///@param pSubscriber pointer to the subcsriber that interest this traffic
	///@return            true if open succesfully
	bool open(ZQ::common::InetMcastAddress group, int group_port, ZQ::common::InetHostAddress bind, McastSubscriber* pSubscriber =NULL);

	/// stop the sniffer, will also stop the dispatcher thread
	void stop();

	/// get the count of the pending message in the queue
	int getPendingMsgCount() { return _msgQueue.size(); }

protected:

	// mcast receiver management
	class recv_t
	{
	public:
		ZQ::common::UDPReceive       recv;   // the UDP receiver instance
		ZQ::common::InetMcastAddress group;
		int                          gport;
		ZQ::common::InetHostAddress  bind;
		char						 openTime[32]; // when this listener is opened
		McastSubscriber*             pSubscriber;  // pointer to the subscriber
		
		recv_t(ZQ::common::InetMcastAddress mgroup, int group_port, ZQ::common::InetHostAddress bind_addr, McastSubscriber* pMcastSubscriber)
			: group(mgroup), gport(group_port), bind(bind_addr), pSubscriber(pMcastSubscriber), recv(bind_addr, group_port)
		{
		}
	};

	typedef std::vector < recv_t* > recvs_t;
	recvs_t _recvs;
	ZQ::common::Mutex _recvsLock;
	char _recvbuf[UDP_MAX_BUFFER_SIZE];

	// thread control
	bool _bQuit;
	HANDLE _hWakeUp;

	// message queue management
	typedef struct _msg_node_t
	{
		McastMsgInfo				msgInfo;
		McastSubscriber*			pSubscriber; //TODO: multiple subscribers?
	} msg_node_t;

	typedef std::queue< msg_node_t > msgQueue_t;
	msgQueue_t _msgQueue;
	ZQ::common::Mutex _msgQueueLock;
	HANDLE _hEvtMessage;
	int _maxQueueSize;

	virtual int pushMessage(const void* data, const int datalen,
			                const ZQ::common::InetHostAddress& source, int sport,
							const recv_t& receiver);

	virtual bool popMessage(msg_node_t& msginfo);

	typedef std::vector < McastMsgDispatcher* > McastMsgDispatcher_v;
    McastMsgDispatcher_v _MsgDispatchers;
	void stopDispatchers();
	int _dispatcherNum;

public:
	typedef recvs_t::const_iterator iterator;

	// browse the active listeners

	/// get the first listener iterator
	iterator begin()  const { return _recvs.begin(); }

	/// get the last listener iterator
	iterator end()    const { return _recvs.end(); }

	/// get the count of active listeners
	size_t   size()   const { return _recvs.size(); }

	size_t   dispatchNum() const { return _MsgDispatchers.size(); }

protected:

	/// the primary process to receive traffic on multiple interfaces
	virtual int run();

};


#endif // __ZQ_McastSniffer_H__

