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
// Ident : $Id: CacheDomain.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheDomain.h $
// 
// 6     7/10/13 3:15p Hui.shao
// 
// 5     6/20/12 2:13p Hui.shao
// 
// 4     4/26/12 6:32p Hui.shao
// the map of external source stores: reg exp of pid => c2 interface
// 
// 3     1/06/12 2:01p Hui.shao
// 
// 2     1/06/12 11:53a Hui.shao
// 
// 1     12/22/11 1:38p Hui.shao
// created
// ===========================================================================

#ifndef __ZQTianShan_CacheDomain_H__
#define __ZQTianShan_CacheDomain_H__

#include "../common/TianShanDefines.h"
#include "NativeThread.h"
#include "UDPSocket.h"

namespace ZQTianShan {
namespace ContentStore {

// message codes
#if defined(ZQ_OS_MSWIN) || defined(ZQ_OS_LINUX)
#  define _MSGCODE(_B)			((_B & 0xff)<<8 | 0x6e)
#else
#  define _MSGCODE(_B)			((_B & 0xff) | 0x6e00)
#endif

#define MSGCODE_CacheStore		_MSGCODE(2)
#define MAX_ADSMSG_LEN          (1024)

class CacheStoreImpl;
// -----------------------------
// class CacheStoreProbe
// -----------------------------
///
class CacheStoreProbe : virtual public ZQ::common::NativeThread, ZQ::common::UDPReceive
{
public:

	/// constructor
    CacheStoreProbe(CacheStoreImpl& store);
	virtual ~CacheStoreProbe();

	// stop listening
	void    stop();

protected:

	// impls of NativeThread
	virtual bool init(void);
	virtual int  run(void);

	// received message, UDPReceive
	int OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress source, int sport);

protected:

	CacheStoreImpl&    _store;
	bool		       _bQuit;
	std::string        _groupDesc;

	char	           _buf[MAX_ADSMSG_LEN+2];
};

// -----------------------------
// class CacheStoreBarker
// -----------------------------
///
class CacheStoreBarker : virtual public ZQ::common::NativeThread, ZQ::common::UDPMulticast
{
public:
	typedef CacheStoreBarker* Ptr;

	/// constructor
    CacheStoreBarker(CacheStoreImpl& store);
	virtual ~CacheStoreBarker();

	// stop barking
	void    stop();

protected:

	// impls of NativeThread
	virtual bool init(void);
	virtual int run(void);

	void wakeup();
	void refreshMsg();

protected:

	friend class CacheStoreProbe;

	typedef struct _AdvertizingMsg
	{
		uint16	msgCode; // should always be MSGCODE_CacheStore
		uint16	msgLen;
		uint8	msgVer;
		char	netId[32]; ///< the unique netId of the CacheStore within the domain
		char	domain[80]; ///< the domain name of the CacheStore that belongs to
		uint16  state;      ///< the state of the CacheStore
        int64   stampStartup;

		uint32  loadStream, loadImport, loadCacheWrite;

		char	endpoint[80];
		char	sessInterface[80];
		char	productNo[80];
	} AdvertizingMsg;

	CacheStoreImpl&    _store;
	std::string        _groupDesc;
	bool		       _bQuit;

    ZQ::common::Semaphore _hWakeup;
	void refreshMsg(AdvertizingMsg& msg);
//	AdvertizingMsg     _msg; 
//	ZQ::common::Mutex  _lockMsg;
};

// -----------------------------
// class SourceStores of external domain
// -----------------------------
class SourceStores
{
public:

	typedef struct _StoreInfo
	{
		std::string sessionInterface;
	} StoreInfo;

public:
	
	SourceStores() {}
	virtual ~SourceStores() {}

	bool find(const std::string& providerId, StoreInfo& storeInfo);
	void set(const std::string& providerIdPattern, const StoreInfo& storeInfo);
	void setDefault(const StoreInfo& storeInfo);
	void erase(const std::string& providerIdPattern);

protected:

	typedef std::map< std::string, StoreInfo > Map; // map of regex expression of providierId to StoreInfo
	ZQ::common::Mutex _lock;
	Map _map;
	StoreInfo _defaultSrcStore;

};


}} // namespace

#endif // __ZQTianShan_CacheDomain_H__
