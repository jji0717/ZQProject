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
// Ident : $Id: McastSniffer.cpp,v 1.5 2004/11/03 09:40:04 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : impl the multicast sniffer
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastSniffer2.cpp $
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

#include "McastSniffer.h"

// -----------------------------
// class McastMsgDispatcher
// -----------------------------

McastMsgDispatcher::McastMsgDispatcher(McastSniffer& Sniffer)
:_bQuit(false), _sniffer(Sniffer)
{
#ifdef SNIFFER_THRD_TEST
	_thid = _th_count++;
#endif // SNIFFER_THRD_TEST
	_hStopped = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

McastMsgDispatcher::~McastMsgDispatcher()
{
	if (!_bQuit)
		stop();

	if (_hStopped !=NULL)
		::CloseHandle(_hStopped);
	_hStopped =NULL;
}

void McastMsgDispatcher::stop()
{
	_bQuit = true;

	if (_hStopped != NULL)
		::WaitForSingleObject(_hStopped, 1500);
}

#ifdef SNIFFER_THRD_TEST
int McastMsgDispatcher::_th_count =0;
#endif // SNIFFER_THRD_TEST

int McastMsgDispatcher::run()
{
	while (!_bQuit)
	{
		McastSniffer::msg_node_t msg;

		if (_sniffer.popMessage(msg) && msg.msgInfo.data !=NULL && msg.msgInfo.datalen>0)
		{
#ifdef SNIFFER_THRD_TEST
			if (msg.msgInfo.datalen>3)
			{
				msg.msgInfo.data[0]='T';
				msg.msgInfo.data[1]= _thid + '0';
			}
#endif // SNIFFER_THRD_TEST
			if (!_bQuit && msg.pSubscriber != NULL)
				msg.pSubscriber->OnMcastMessage(msg.msgInfo);

			delete[] msg.msgInfo.data;
			msg.msgInfo.data = NULL;
		}
	}

	if (_hStopped !=NULL)
		::SetEvent(_hStopped);

	return 0;
}

void McastMsgDispatcher::final()
{
	if (_hStopped !=NULL)
		::SetEvent(_hStopped);
}


// -----------------------------
// class McastSniffer
// -----------------------------

McastSniffer::McastSniffer(int MsgDispatcherNum)
:_bQuit(false), _maxQueueSize(DEFAULT_QUEUE_SIZE),_dispatcherNum(MsgDispatcherNum)
{
	_hWakeUp = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	_hEvtMessage = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	_bQuit = (_hWakeUp ==NULL || _hEvtMessage ==NULL);

	if (_dispatcherNum<=0)
		_dispatcherNum = 1;
}

McastSniffer::~McastSniffer()
{
	if (!_bQuit)
		stop();

	// clean up the receivers
	{
		ZQ::common::MutexGuard guard(_recvsLock);
		for ( recvs_t::iterator it= _recvs.begin( ); it != _recvs.end( ); it++ )
		{
			recv_t* pRecv = *it;
			if (pRecv !=NULL)
				delete pRecv;
		}
	}

	_recvs.clear();

	// clean up the pending queue
	{
		ZQ::common::MutexGuard msgguard(_msgQueueLock);
		while(!_msgQueue.empty())
		{
			msg_node_t& msg = _msgQueue.front();

			if (msg.msgInfo.data !=NULL && msg.msgInfo.datalen>0)
			{
				delete[] msg.msgInfo.data;
				msg.msgInfo.data = NULL;
			}

			_msgQueue.pop();
		}
	}

	if (_hWakeUp !=NULL)
		::CloseHandle(_hWakeUp);
	_hWakeUp =NULL;

	if (_hEvtMessage !=NULL)
		::CloseHandle(_hEvtMessage);
	_hEvtMessage =NULL;
}

bool McastSniffer::open(ZQ::common::InetMcastAddress group, int group_port, ZQ::common::InetHostAddress bind, McastSubscriber* pSubscriber)
{
	if (_bQuit)
		return false;

	recv_t* newRecv =NULL;
	
	try
	{
		newRecv = new recv_t(group, group_port, bind, pSubscriber);

		if (newRecv ==NULL)
			return false;

		newRecv->recv.setMulticast(true);

		if (newRecv->recv.join(group) != ZQ::common::Socket::errSuccess)
		{
			delete newRecv;
			return false;
		}

		newRecv->recv.setCompletion(true); // make the socket blockable
	}
	catch(...)
	{
		return false;
	}

	SOCKET so = newRecv->recv.getReceiver();

	if (so == INVALID_SOCKET)
	{
		delete newRecv;
		return false;
	}
	
	SYSTEMTIME SystemTime ;
	GetSystemTime(&SystemTime);
	sprintf(newRecv->openTime,
		"%4d-%2d-%2d %2d:%2d:%2d",
		SystemTime.wYear,
		SystemTime.wMonth,
		SystemTime.wDay,
		SystemTime.wHour,
		SystemTime.wMinute,
		SystemTime.wSecond);

	ZQ::common::MutexGuard guard(_recvsLock);
	_recvs.push_back(newRecv);
	::SetEvent(_hWakeUp);

	return true;
}

void McastSniffer::stop()
{
	_bQuit = true;
	stopDispatchers();

	if (_hEvtMessage !=NULL)
		::SetEvent(_hEvtMessage);

	if (_hWakeUp != NULL)
		::SetEvent(_hWakeUp);

	::Sleep(100);
}

int McastSniffer::run()
{
	ZQ::common::InetHostAddress from;
	int sport;

	for (int i =0; !_bQuit && i < _dispatcherNum; i++)
	{
		McastMsgDispatcher* pDispatcher = new McastMsgDispatcher(*this);
		if (pDispatcher != NULL)
		{
			_MsgDispatchers.push_back(pDispatcher);
			pDispatcher->start();
		}
	}

	while (!_bQuit) // TODO: ScThreadPool doesn't support thread::terminate
	{
		struct timeval timeout;
		fd_set inp, err;
		SOCKET so;

		FD_ZERO(&inp);
		FD_ZERO(&err);

		int maxFD =0, nFD =0;

		// fill in the FD sets
		for ( recvs_t::iterator it= _recvs.begin( ); it != _recvs.end( ); it++ )
		{
			recv_t* pRecv = *it;
			if (pRecv == NULL)
				continue;

			so = pRecv->recv.getReceiver();
			if (so == INVALID_SOCKET)
				continue;

			FD_SET(so, &inp);
			FD_SET(so, &err);
			maxFD = (maxFD >= (int) so) ? maxFD : so;
			++nFD;
		}

		if (_bQuit)
			return 0;

		// if there is no valid FD yet, wait for open() to put one in
		if (nFD <=0 || maxFD<=0)
		{
			if (WAIT_FAILED == ::WaitForSingleObject(_hWakeUp, INFINITE))
				_bQuit = true;
			else
				::ResetEvent(_hWakeUp);

			if (_bQuit)
				return 0;

			continue;
		}

		// wait for 2 sec maximal 
		timeout.tv_sec  = 2;
		timeout.tv_usec = 0;

		int fn =::select(maxFD, &inp, NULL, &err, &timeout);

		if (fn >0)
		{
			for ( recvs_t::iterator it= _recvs.begin( ); !_bQuit && it != _recvs.end( ); it++ )
			{
				recv_t* pRecv = *it;
				if (pRecv == NULL)
					continue;

				so = pRecv->recv.getReceiver();
				if (so == INVALID_SOCKET)
					continue;

				if(FD_ISSET(so, &err))
				{
					//TODO: remove this node from _recvs
					continue;
				}

				if (FD_ISSET(so, &inp))
				{
					int len = pRecv->recv.receiveFrom(_recvbuf, UDP_MAX_BUFFER_SIZE, from, sport);
					if (len >0)
					{
						pushMessage(_recvbuf, len, from, sport, *pRecv);
					}
				}
			}
		}

	}

	_bQuit = true;

	// stop all dispatchers
	for (McastMsgDispatcher_v::iterator it = _MsgDispatchers.begin();
		it < _MsgDispatchers.end(); it++)
	{
		if (*it !=NULL)
			delete (*it);
	}
	_MsgDispatchers.clear();

	if (_hEvtMessage !=NULL)
		::SetEvent(_hEvtMessage);

	return 0;
}

void McastSniffer::stopDispatchers()
{
	// stop all dispatchers
	for (McastMsgDispatcher_v::iterator it = _MsgDispatchers.begin();
		it < _MsgDispatchers.end(); it++)
	{
		if (*it !=NULL)
			(*it)->stop();
//		*it = NULL;
	}
	for (McastMsgDispatcher_v::iterator it = _MsgDispatchers.begin();
		it < _MsgDispatchers.end(); it++)
	{
		if (*it !=NULL)
			delete (*it);
	}
	_MsgDispatchers.clear();
}

int McastSniffer::pushMessage(const void* data, const int datalen,
			                const ZQ::common::InetHostAddress& source, int sport,
							const recv_t& receiver)
{
	if (data ==NULL || datalen<=0 || datalen > UDP_MAX_BUFFER_SIZE)
		return 0;

	if (_msgQueue.size() >= _maxQueueSize )
		return 0;

	msg_node_t msg;

	msg.msgInfo.source = source;
	msg.msgInfo.sport= sport;
	msg.msgInfo.group = receiver.group;
	msg.msgInfo.gport = receiver.gport;

	msg.pSubscriber =	receiver.pSubscriber;

	msg.msgInfo.datalen = datalen;
	msg.msgInfo.data = new char[msg.msgInfo.datalen];

	if (msg.msgInfo.data == NULL)
		return 0;
	memcpy(msg.msgInfo.data, data, msg.msgInfo.datalen);

	_msgQueue.push(msg);
	::SetEvent(_hEvtMessage);

	return  msg.msgInfo.datalen;
}

bool McastSniffer::popMessage(msg_node_t& msg)
{
	if (_msgQueue.empty())
	{
		if (WAIT_FAILED == ::WaitForSingleObject(_hEvtMessage, INFINITE))
			_bQuit = true;
	}

	if (_bQuit)
		return false;

	::ResetEvent(_hEvtMessage);
	// now has messages in the queue
	{
		ZQ::common::MutexGuard guard(_msgQueueLock);

		if (_bQuit)
			return false;

		msg = _msgQueue.front();
		_msgQueue.pop();
	}

	return true;
}

