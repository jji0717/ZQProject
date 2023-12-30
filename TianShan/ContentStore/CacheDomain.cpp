// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: CacheDomain.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheDomain.cpp $
// 
// 22    8/25/16 4:27p Hui.shao
// 
// 21    8/25/16 3:59p Hui.shao
// 
// 20    8/17/16 2:58p Hui.shao
// 
// 19    7/10/13 3:15p Hui.shao
// 
// 18    7/05/13 2:16p Hui.shao
// centos6
// 
// 17    8/09/12 12:21p Hui.shao
// corrected logging
// 
// 16    8/03/12 3:03p Hui.shao
// 
// 15    8/02/12 4:17p Hongquan.zhang
// 
// 14    7/27/12 4:36p Hongquan.zhang
// bind groupaddr instead of local address in MULTICAST mode
// 
// 13    7/26/12 6:43p Hui.shao
// firxed domain barker/probe's bind address
// 
// 12    7/26/12 6:23p Li.huang
// 
// 11    7/26/12 5:14p Hui.shao
// 
// 10    6/20/12 2:13p Hui.shao
// 
// 9     6/07/12 11:30a Hui.shao
// configuration schema
// 
// 8     6/06/12 4:22p Hui.shao
// 
// 7     6/06/12 4:19p Hui.shao
// reduce log messages
// 
// 6     4/26/12 6:37p Hui.shao
// 
// 5     4/26/12 6:32p Hui.shao
// the map of external source stores: reg exp of pid => c2 interface
// 
// 4     1/18/12 6:24p Li.huang
// 
// 3     1/06/12 2:01p Hui.shao
// 
// 2     1/06/12 11:53a Hui.shao
// ===========================================================================

#include "CacheStoreImpl.h"
#include "CacheDomain.h"

#include "SystemUtils.h"

#undef max // the following regex.hpp has "max" in different meaning
#include "boost/regex.hpp"

namespace ZQTianShan {
namespace ContentStore {

#define storelog (_store._log)

// -----------------------------
// class CacheStoreProbe
// -----------------------------
CacheStoreProbe::CacheStoreProbe(CacheStoreImpl& store)
 : _store(store), _bQuit(false)
 // UDPReceive((store._groupAddr.family() == PF_INET6) ? "::0" : "0.0.0.0", store._groupPort), 
{
}

CacheStoreProbe::~CacheStoreProbe()
{
}

void CacheStoreProbe::stop()
{
	_bQuit = true;
    endReceiver();
}

bool CacheStoreProbe::init(void)	
{
	std::string bindaddr= _store._groupBind.getHostAddress();
	_groupDesc= _store._contentStore._replicaGroupId + "@" + _store._groupAddr.getHostAddress();

	storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreProbe, "initializing at: group[%s]:%d, bind[%s]"), _groupDesc.c_str(), _store._groupPort, bindaddr.c_str());

#ifdef ZQ_OS_MSWIN
	UDPReceive::bind(_store._groupBind, _store._groupPort);
#elif defined ZQ_OS_LINUX
	UDPReceive::bind(_store._groupAddr, _store._groupPort);
#endif

	UDPReceive::setMulticast(true);
	ZQ::common::Socket::Error err = UDPReceive::join(_store._groupAddr);
	UDPReceive::setCompletion(true); // make the socket block-able

	if (err != ZQ::common::Socket::errSuccess)
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreProbe, "initialze failed at: group:[%s]:%d"), _groupDesc.c_str(), _store._groupPort);
		return false;
	}

	return true;
}

int CacheStoreProbe::run()
{
	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreProbe, "probe[%s] starts"), _groupDesc.c_str());

	ZQ::common::InetHostAddress from;
	int sport;
	while (!_bQuit)
	{
		try {
			int len = UDPReceive::receiveFrom(_buf, MAX_ADSMSG_LEN, from, sport);
			if (_bQuit)
				break;

			if (len <=0)
			{
				storelog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStoreProbe, "ignoring 0-byte message"));
				continue;
			}

			OnCapturedData(_buf, len, from, sport);
		}
		catch(...)
		{
			storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreProbe, "exception occured"));
		}
	}

	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreProbe, "probe[%s] quits at %c"), _groupDesc.c_str(), (_bQuit?'T':'F'));
	return 0;
}

int CacheStoreProbe::OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress source, int sport)
{
	CacheStoreBarker::AdvertizingMsg* pMsg = (CacheStoreBarker::AdvertizingMsg *) data;
	if (datalen < sizeof(CacheStoreBarker::AdvertizingMsg) || MSGCODE_CacheStore != pMsg->msgCode)
	{
		storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreProbe, "OnCapturedData() ignore broken message: len[%d] from source[%s]:%d"), datalen, source.getHostAddress(), sport);
		return datalen;
	}

	if (CacheStoreImpl::sfLoggingDomainMessages & _store._flags)
		storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreProbe, "OnCapturedData() len:%d, source:[%s]:%d"), datalen, source.getHostAddress(), sport);

	if (CacheStoreImpl::sfEnableDomainMessageDump & _store._flags)
		storelog.hexDump(ZQ::common::Log::L_DEBUG, data, datalen, "CacheStoreProbe::OnCapturedData() ");

	if ((uint32)datalen < sizeof(CacheStoreBarker::AdvertizingMsg) || (uint32)datalen < pMsg->msgLen)
	{
		if (CacheStoreImpl::sfLoggingDomainMessages & _store._flags)
			storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreProbe, "OnCapturedData() unrecognized message, code=%04x, msgLen=%d; ignore"), pMsg->msgCode, pMsg->msgLen);
		return datalen;
	}

	// ignore if this is from self and not the same domain
	if (0 != _store._contentStore._replicaGroupId.compare(pMsg->domain))
	{
		if (CacheStoreImpl::sfLoggingDomainMessages & _store._flags)
			storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreProbe, "OnCapturedData() ignore message netId[%s] that from domain[%s] other than[%s]"), pMsg->netId, pMsg->domain, _store._contentStore._replicaGroupId.c_str());
		return datalen;
	}

	if (0 ==_store._contentStore._netId.compare(pMsg->netId))
	{
		if (CacheStoreImpl::sfLoggingDomainMessages & _store._flags)
			storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreProbe, "OnCapturedData() ignore that from self "));

#ifndef _DEBUG
		return datalen;
#endif // ! _DEBUG
	}

	int64 stampNow = ZQ::common::now();

	if (CacheStoreImpl::sfLoggingDomainMessages & _store._flags)
		storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreProbe, "OnCapturedData() peer[%s]: sessInterface[%s] endpoint[%s]"),
		                    pMsg->netId, pMsg->sessInterface, pMsg->endpoint);
	
	do
	{
		bool bStatusChanged = false;
		try {
			::ZQ::common::MutexGuard gd(_store._lockerNeighbors);
			CacheStoreImpl::CacheStoreMap::iterator it = _store._neighbors.find(pMsg->netId);
			if (_store._neighbors.end() != it)
			{
				bStatusChanged = (it->second.desc.state != (TianShanIce::State)pMsg->state)
				|| (it->second.desc.loadStream != pMsg->loadStream)
				|| (it->second.desc.loadImport != pMsg->loadImport)
				|| (it->second.desc.loadCacheWrite  != pMsg->loadCacheWrite);
				
				it->second.desc.state = (TianShanIce::State)pMsg->state;
				it->second.desc.loadStream      = pMsg->loadStream;
				it->second.desc.loadImport      = pMsg->loadImport;
				it->second.desc.loadCacheWrite  = pMsg->loadCacheWrite;
				it->second.desc.stampAsOf       = stampNow;
				
				int oldpen = it->second.penalty;
				if (--it->second.penalty <0)
					it->second.penalty =0;
				
				if (oldpen != it->second.penalty)
					bStatusChanged = true;

				if (!it->second.desc.theStore || it->second.stampStartup < pMsg->stampStartup)
				{
					bStatusChanged = true;
					// the source node has been restarted, necessary to refresh more fields
					it->second.desc.theStore = ::TianShanIce::Storage::CacheStorePrx::checkedCast(_store._contentStore._adapter->getCommunicator()->stringToProxy(pMsg->endpoint));
					it->second.desc.sessionInterface = pMsg->sessInterface;
				}

				it->second.stampStartup = pMsg->stampStartup;

				if (bStatusChanged || (CacheStoreImpl::sfLoggingDomainMessages & _store._flags))
				storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreProbe, "OnCapturedData() peer[%s]: sessInterface[%s] endpoint[%s] updated, load[%d/%d/%d] connected[%c] penalty[%d->%d]"),
						pMsg->netId, pMsg->sessInterface, pMsg->endpoint, pMsg->loadStream, pMsg->loadImport, pMsg->loadCacheWrite, (!it->second.desc.theStore? 'F' :'T'), oldpen, it->second.penalty);

				break;
			}

			// to insert a new detected node
			CacheStoreImpl::CacheStoreDsptr newStore;
			newStore.desc.netId            = pMsg->netId;
			newStore.desc.domain           = _store._contentStore._replicaGroupId;
			newStore.desc.loadStream       = pMsg->loadStream;
			newStore.desc.loadImport       = pMsg->loadImport;
			newStore.desc.loadCacheWrite   = pMsg->loadCacheWrite;
			newStore.desc.stampAsOf        = stampNow;
			newStore.desc.sessionInterface = pMsg->sessInterface;
			newStore.desc.theStore         = ::TianShanIce::Storage::CacheStorePrx::checkedCast(_store._contentStore._adapter->getCommunicator()->stringToProxy(pMsg->endpoint));

			CacheStoreImpl::_calcHashKey(newStore.cskey, newStore.desc.netId.c_str(), newStore.desc.netId.length());
			newStore.penalty          = 0;
			newStore.stampLastPenalty = 0;
			newStore.stampStartup     = pMsg->stampStartup;

			MAPSET(CacheStoreImpl::CacheStoreMap, _store._neighbors, newStore.desc.netId, newStore);

			storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreProbe, "OnCapturedData() detected new peer[%s]: sessInterface[%s] endpoint[%s] updated, load[%d/%d/%d]"),
				pMsg->netId, pMsg->sessInterface, pMsg->endpoint, pMsg->loadStream, pMsg->loadImport, pMsg->loadCacheWrite);
		}
		catch(...) {}

	} while(0);

	return datalen;
}


// -----------------------------
// class CacheStoreBarker
// -----------------------------
///
CacheStoreBarker::CacheStoreBarker(CacheStoreImpl& store)
: _store(store), _bQuit(false)
 // UDPMulticast(store._groupBind, 0), 
{
}

void CacheStoreBarker::refreshMsg(AdvertizingMsg& msg)
{
	memset(&msg, 0x00, sizeof(msg));
	msg.msgCode = MSGCODE_CacheStore;
	msg.msgLen = sizeof(msg);
	msg.msgVer = 0x00;

	// about netId
	if (_store._thisDescriptor.desc.netId.length() >= sizeof(msg.netId))
	{
        storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreBarker, "failed to fill message field[netId]. Attempted to copy [%s] into %d-bytes field."), _store._thisDescriptor.desc.netId.c_str(), sizeof(msg.netId));
		return;
	}
	strncpy(msg.netId, _store._thisDescriptor.desc.netId.c_str(), sizeof(msg.netId)-1);

	// about domain
    if (_store._thisDescriptor.desc.domain.length() >= sizeof(msg.domain))
	{
        storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreBarker, "failed to fill message field[domain]. Attempted to copy [%s] into %d-bytes field."), _store._thisDescriptor.desc.domain.c_str(), sizeof(msg.domain));
		return;
	}

	strncpy(msg.domain, _store._thisDescriptor.desc.domain.c_str(), sizeof(msg.domain)-1);
	msg.state           = (uint16) _store._thisDescriptor.desc.state;
	msg.stampStartup    = _store._thisDescriptor.stampStartup;

	msg.loadStream      = _store._thisDescriptor.desc.loadStream;
	msg.loadImport      = _store._thisDescriptor.desc.loadImport;
	msg.loadCacheWrite  = _store._thisDescriptor.desc.loadCacheWrite;
	strncpy(msg.endpoint, _store._thisPrxStr.c_str(), sizeof(msg.endpoint)-2);
	strncpy(msg.sessInterface, _store._thisDescriptor.desc.sessionInterface.c_str(), sizeof(msg.sessInterface)-2);
	strncpy(msg.productNo, "DemoCache/0.1", sizeof(msg.productNo)-2);
}

CacheStoreBarker::~CacheStoreBarker()
{
    storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreBarker, "signals the barker to stop..."));
	stop();

    storelog.flush();
    waitHandle(-1);

    storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreBarker, "The barker is stopped."));
    storelog.flush();
}

void CacheStoreBarker::stop()
{
	_bQuit = true;
	wakeup();
}

void CacheStoreBarker::wakeup()
{
    _hWakeup.post();
}


bool CacheStoreBarker::init(void)	
{
	std::string bindaddr= _store._groupBind.getHostAddress();
	_groupDesc= _store._contentStore._replicaGroupId + "@" + _store._groupAddr.getHostAddress();
	storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreBarker, "open group barker: bind=%s group=[%s]:%d"), bindaddr.c_str(), _groupDesc.c_str(), _store._groupPort);

	UDPMulticast::bind(_store._groupBind);
	UDPMulticast::setGroup(_store._groupAddr, _store._groupPort);
	UDPMulticast::setTimeToLive(32);
	return UDPMulticast::isActive();
}

int CacheStoreBarker::run()
{
	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreBarker, "barker[%s] starts"), _groupDesc.c_str());
	while (!_bQuit)
	{
		try
		{
			AdvertizingMsg msg;
			refreshMsg(msg);

			if (CacheStoreImpl::sfEnableDomainMessageDump & _store._flags)
				storelog.hexDump(ZQ::common::Log::L_DEBUG, &msg, msg.msgLen, "CacheStoreBarker() sending ");
			else if (CacheStoreImpl::sfLoggingDomainMessages & _store._flags)
				storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreBarker, "CacheStoreBarker() advertizing this[%s] to domain[%s]: load[%d/%d/%d] endpoint[%s]"), msg.netId, msg.domain, msg.loadStream, msg.loadImport, msg.loadCacheWrite, msg.endpoint);
			
			if (UDPMulticast::send(&msg, msg.msgLen) < (int)msg.msgLen)
				storelog.hexDump(ZQ::common::Log::L_ERROR, &msg, msg.msgLen, "CacheStoreBarker() failed to send message");
			
		}
		catch(...) {}
		
		if (_bQuit)
			break;

		if (_store._heatbeatInterval <1000)
			_store._heatbeatInterval = 1000;

		timeout_t sleepTime = (timeout_t) (_store._heatbeatInterval * 0.7); // 70% of the timeout value
        _hWakeup.timedWait(sleepTime);
	}

	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreBarker, "barker[%s] quits"), _groupDesc.c_str());
	return 0;
}

// -----------------------------
// class SourceStores
// -----------------------------
bool SourceStores::find(const std::string& providerId, StoreInfo& storeInfo)
{
	ZQ::common::MutexGuard g(_lock);
	for (Map::const_iterator it = _map.begin(); it != _map.end(); it++)
	{
		if (it->first.empty())
			continue;

		try {
			boost::cmatch m; 
			boost::regex expression(it->first.c_str());

			if (!boost::regex_search(providerId.c_str(), m , expression) || !m[0].matched)
				continue;
		}
		catch(...) { continue; }

		storeInfo = it->second;
		if (!storeInfo.sessionInterface.empty())
			return true;
	}

	storeInfo = _defaultSrcStore;
	return !storeInfo.sessionInterface.empty();
}

void SourceStores::set(const std::string& providerIdPattern, const StoreInfo& storeInfo)
{
	if (providerIdPattern.empty())
		return;

	ZQ::common::MutexGuard g(_lock);
	MAPSET(Map, _map, providerIdPattern, storeInfo);
}

void SourceStores::setDefault(const StoreInfo& storeInfo)
{
	ZQ::common::MutexGuard g(_lock);
	 _defaultSrcStore = storeInfo;
}

void SourceStores::erase(const std::string& providerIdPattern)
{
	ZQ::common::MutexGuard g(_lock);
	 _map.erase(providerIdPattern);
}

}} // namespace
