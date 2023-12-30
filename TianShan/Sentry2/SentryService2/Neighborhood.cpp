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
// Ident : $Id: Neighborhood.cpp,v 1.5 2004/07/27 09:40:04 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : impl the node Neighborhood
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/Neighborhood.cpp $
// 
// 4     3/16/16 10:53a Ketao.zhang
// 
// 3     3/19/15 9:58p Zhiqiang.niu
// use ZQSnmp instead of old snmp
// 
// 2     7/27/12 4:55p Zonghuan.xiao
// implement  table of TianShan Modules(Sentry)  Oid =
// .1.3.6.1.4.1.22839.4.1.1100.3.1.1.1
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:40 Admin
// Created.
// 
// 12    09-12-16 15:31 Fei.huang
// 
// 12    09-12-08 13:02 Fei.huang
// * fix: move InetAddr obj out of the loop, to avoid crash while socked
// shutdown
// 
// 11    09-11-23 15:19 Xiaohui.chai
// make the SentryListener and SentryBarker NativeThread 
// 
// 10    09-08-06 14:11 Yixin.tian
// 
// 9     09-07-06 14:31 Fei.huang
// * linux port
// 
// 8     08-11-24 19:18 Xiaohui.chai
// add multithread access protection for PeerInfoRefreshing::_pendingNodes
// 
// 7     08-06-10 11:31 Xiaohui.chai
// added machine type to multicast message
// 
// 6     07-11-16 10:41 Xiaohui.chai
// 
// 5     07-11-06 18:34 Xiaohui.chai
// 
// 4     07-11-05 15:47 Xiaohui.chai
// 
// 3     07-08-01 17:12 Hongquan.zhang
// 
// 2     07-05-22 17:30 Hui.shao
// added exporting logger information
// ===========================================================================
#include "SentryEnv.h"
#include <boost/thread.hpp>
#include "Neighborhood.h"
#include "SentryCommand.h"
#include "SystemUtils.h"

namespace ZQTianShan {
namespace Sentry {

// -----------------------------
// class SentryListener
// -----------------------------
///
SentryListener::SentryListener(SentryEnv& env)
:UDPReceive((env._groupAddr.family() == PF_INET6) ? "::0" : "0.0.0.0", env._groupPort), 
 _env(env), _bQuit(false), _bMsgDump(false)
{
}

SentryListener::~SentryListener()
{
    envlog(ZQ::common::Log::L_INFO, CLOGFMT(Neighborhood, "~SentryListener() Signal the listener to stop..."));
	quit();
    envlog.flush();
    waitHandle(-1);
    envlog(ZQ::common::Log::L_INFO, CLOGFMT(Neighborhood, "~SentryListener() The listener is stopped."));
    envlog.flush();
}

void SentryListener::quit()
{
	_bQuit = true;
    endReceiver();
}

bool SentryListener::init(void)	
{
	std::string bindaddr= _env._groupBind.getHostAddress(), grpaddr= _env._groupAddr.getHostAddress();
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "open group listener: group=[%s]:%d"), grpaddr.c_str(), _env._groupPort);
	UDPReceive::setMulticast(true);
	ZQ::common::Socket::Error err = UDPReceive::join(_env._groupAddr);
	UDPReceive::setCompletion(true); // make the socket block-able

	if (err != ZQ::common::Socket::errSuccess)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryListener, "initialze failed: group:[%s]:%d"), _env._groupAddr.getHostAddress(), _env._groupPort);
		return false;
	}

	return true;
}

int SentryListener::run()
{
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(SentryListener, "listener starts"));
//#pragma message(__MSGLOC__"TODO:MUST BE RESTORE")
//	return 1;

	ZQ::common::InetHostAddress from;
	int sport;
	while (!_bQuit)
	{
		int len = UDPReceive::receiveFrom(_buf, MAX_NODEMSG_LEN, from, sport);
		if (_bQuit || len <=0)
			break;

		OnCapturedData(_buf, len, from, sport);
	}
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(SentryListener, "listener quit"));
	return 0;
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int SentryListener::OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress source, int sport)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryListener, "OnCapturedData() len:%d, source:[%s]:%d"), datalen, source.getHostAddress(), sport);
	if (_bMsgDump)
		envlog.hexDump(ZQ::common::Log::L_DEBUG, data, datalen, "OnCapturedData() ");

	SentryBarker::NodeMcastMsg* pMsg = (SentryBarker::NodeMcastMsg *) data;
	if (MSGCODE_NEIGHBOR !=	pMsg->msgCode || (uint32)datalen < sizeof(SentryBarker::NodeMcastMsg) || (uint32)datalen < pMsg->msgLen)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryListener, "OnCapturedData() unrecognized message, code=%04x, msgLen=%d; ignore"), pMsg->msgCode, pMsg->msgLen);
		return datalen;
	}

	// ignore if this is from self
	if (0 ==_env._selfInfo.id.compare(pMsg->nodeId))
    {
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryListener, "OnCapturedData() from self"));
#ifndef _DEBUG
		return datalen;
#endif // ! _DEBUG
    }
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryListener, "OnCapturedData() peer node[%s]: lastChange="FMT64"; servAddrs=\"%s\" proxy=\"%s\""),
		   pMsg->nodeId, pMsg->stampLastChange, pMsg->servAddrs, pMsg->sentrysvcPrx);
	
	{
		::ZQ::common::MutexGuard gd(_env._lockNeighbors);
		
		if (_env._neighbors.end() != _env._neighbors.find(pMsg->nodeId) && _env._neighbors[pMsg->nodeId].lastChange >= pMsg->stampLastChange)
		{
			_env._neighbors[pMsg->nodeId].memTotalPhys    = pMsg->memTotalPhys;
			_env._neighbors[pMsg->nodeId].memAvailPhys    = pMsg->memAvailPhys;
			_env._neighbors[pMsg->nodeId].memTotalVirtual = pMsg->memTotalVirtual;
			_env._neighbors[pMsg->nodeId].memAvailVirtual = pMsg->memAvailVirtual;
			_env._neighbors[pMsg->nodeId].lastHeartbeat = now();
			
			//_env.refreshNeighborsModulesTable();
			//_env.refreshParticipantMachineTable();
//			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryListener, "OnCapturedData() peer node[%s]: updated, no interface changed; phy-mem=%u/%u; vmem=%u/%u"),
//				   pMsg->nodeId, pMsg->memAvailPhys, pMsg->memTotalPhys, pMsg->memAvailVirtual, pMsg->memTotalVirtual);
//			return datalen;
		}
		else
        {
            SentryEnv::NodeInfo neighbor;
		    neighbor.id			= pMsg->nodeId;
		    neighbor.name		= pMsg->name;
		    neighbor.sentrysvcPrx	= pMsg->sentrysvcPrx;
		    neighbor.lastChange		= pMsg->stampLastChange; // preset this stamp to avoid dup refresh
		    neighbor.memTotalPhys    = pMsg->memTotalPhys;
		    neighbor.memAvailPhys    = pMsg->memAvailPhys;
		    neighbor.memTotalVirtual = pMsg->memTotalVirtual;
		    neighbor.memAvailVirtual = pMsg->memAvailVirtual;
		    neighbor.lastHeartbeat	 = now();
            neighbor.osStartup = pMsg->stampOSStartup;
            neighbor.type = pMsg->type;
		    
		    _env._neighbors.erase(neighbor.id);
		    _env._neighbors.insert(SentryEnv::NodeMap::value_type(neighbor.id, neighbor));
			//_env.refreshParticipantMachineTable();
        }		
	}
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryListener, "OnCapturedData() peer node[%s]: updated, inteface changed; lastChange="FMT64", phy-mem=%u/%u; vmem=%u/%u"),
 			pMsg->nodeId, pMsg->stampLastChange, pMsg->memAvailPhys, pMsg->memTotalPhys, pMsg->memAvailVirtual, pMsg->memTotalVirtual);

    if (!PeerInfoRefreshing::isPending(pMsg->nodeId))
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryListener, "OnCapturedData() peer node[%s]: creating a PeerInfoRefreshing request to refresh details"),	pMsg->nodeId);
		(new PeerInfoRefreshing(_env, pMsg->nodeId, pMsg->sentrysvcPrx, pMsg->stampLastChange))->execute();
	}

	return datalen;
}

// -----------------------------
// class SentryBarker
// -----------------------------
///
SentryBarker::SentryBarker(SentryEnv& env)
:UDPMulticast(env._groupBind, 0), 
 _env(env), _bQuit(false), _bMsgDump(false)
{
	refreshMsg();
}

void SentryBarker::refreshMsg()
{
	ZQ::common::MutexGuard gd(_lockMsg);

	memset(&_msg, 0x00, sizeof(_msg));
	_msg.msgCode = MSGCODE_NEIGHBOR;
	_msg.msgLen = sizeof(_msg);
    if(_env._selfInfo.id.size() < sizeof(_msg.nodeId))
    {
        strcpy(_msg.nodeId, _env._selfInfo.id.c_str());
    }
    else
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryBarker, "failed to fill message field nodeId. Attempted to copy [%s] into %d-bytes field."), _env._selfInfo.id.c_str(), sizeof(_msg.nodeId));
    }
	_msg.stampLastChange = now();
    _msg.stampOSStartup = _env._selfInfo.osStartup;

	std::string servAddrs;
	try{
		::TianShanIce::StrValues addrs = ZQTianShan::Adapter::getServeAddress();
		for (::TianShanIce::StrValues::iterator it = addrs.begin(); it < addrs.end(); it++)
			servAddrs += std::string(::ZQ::common::InetAddress(it->c_str()).getHostAddress()) + " ";
	}
	catch(...) {}

    if(_env._selfInfo.name.size() < sizeof(_msg.name))
    {
        strcpy(_msg.name, _env._selfInfo.name.c_str());
    }
    else
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryBarker, "failed to fill message field name. Attempted to copy [%s] into %d-bytes field."), _env._selfInfo.name.c_str(), sizeof(_msg.name));
    }
    if(servAddrs.size() < sizeof(_msg.servAddrs))
    {
        strcpy(_msg.servAddrs, servAddrs.c_str());
    }
    else
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryBarker, "failed to fill message field servAddrs. Attempted to copy [%s] into %d-bytes field."), servAddrs.c_str(), sizeof(_msg.servAddrs));
    }

    if(_env._selfInfo.sentrysvcPrx.size() < sizeof(_msg.sentrysvcPrx))
    {
        strcpy(_msg.sentrysvcPrx, _env._selfInfo.sentrysvcPrx.c_str());
    }
    else
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryBarker, "failed to fill message field sentrysvcPrx. Attempted to copy [%s] into %d-bytes field."), _env._selfInfo.sentrysvcPrx.c_str(), sizeof(_msg.sentrysvcPrx));
    }

    if(_env._selfInfo.type.size() < sizeof(_msg.type))
    {
        strcpy(_msg.type, _env._selfInfo.type.c_str());
    }
    else
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryBarker, "failed to fill message field type. Attempted to copy [%s] into %d-bytes field."), _env._selfInfo.type.c_str(), sizeof(_msg.type));
    }
}

SentryBarker::~SentryBarker()
{
    envlog(ZQ::common::Log::L_INFO, CLOGFMT(Neighborhood, "~SentryBarker() Signal the barker to stop..."));
	quit();
    envlog.flush();
    waitHandle(-1);
    envlog(ZQ::common::Log::L_INFO, CLOGFMT(Neighborhood, "~SentryBarker() The barker is stopped."));
    envlog.flush();
}

void SentryBarker::quit()
{
	_bQuit = true;
	wakeup();
}

void SentryBarker::wakeup()
{
    _wakeup.post();
}


bool SentryBarker::init(void)	
{
	std::string bindaddr= _env._groupBind.getHostAddress(), grpaddr= _env._groupAddr.getHostAddress();
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryBarker, "open group barker: bind=%s group=[%s]:%d"), bindaddr.c_str(), grpaddr.c_str(), _env._groupPort);

	UDPMulticast::setGroup(_env._groupAddr, _env._groupPort);
	UDPMulticast::setTimeToLive(32);
	return UDPMulticast::isActive();
}

int SentryBarker::run()
{
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(SentryBarker, "barker starts"));
	while (!_bQuit)
	{
		_env.refreshSystemUsage();
		{
			ZQ::common::MutexGuard gd(_lockMsg);
			_msg.memTotalPhys    = _env._selfInfo.memTotalPhys;
			_msg.memAvailPhys    = _env._selfInfo.memAvailPhys;
			_msg.memTotalVirtual = _env._selfInfo.memTotalVirtual;
			_msg.memAvailVirtual = _env._selfInfo.memAvailVirtual;
			_msg.stampLastChange = max(_msg.stampLastChange, _env._selfInfo.lastChange);
			
			if (_bMsgDump)
				envlog.hexDump(ZQ::common::Log::L_DEBUG, &_msg, _msg.msgLen, "SentryBarker() sending ");
			
			if (UDPMulticast::send(&_msg, _msg.msgLen) < (int)_msg.msgLen)
				envlog.hexDump(ZQ::common::Log::L_ERROR, &_msg, _msg.msgLen, "failed to send message ");
		}
		
		if (_bQuit)
			break;

		timeout_t sleepTime = (_env._timeout * 700); // sec to msec with 70% of the timeout value
		
		if (sleepTime < MIN_GROUP_TIMEOUT * 1000)
			sleepTime = MIN_GROUP_TIMEOUT * 1000;

		if (sleepTime > MAX_GROUP_TIMEOUT * 1000)
			sleepTime = MAX_GROUP_TIMEOUT * 1000;

        _wakeup.timedWait(sleepTime);
	}
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(SentryBarker, "barker quit"));
	return 0;
}

}} // namespace
