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
// Name  : event.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/8/2005
// Desc  : TAO\CORBA event service
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ISAAlarmImpl/corbalib/event.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-11 3:34p Daniel.wang
// 
// 3     05-06-27 19:33 Yan.zheng
// 
// 2     5/14/05 4:39p Hui.shao
// merged to ZQ dev environment
// 
// 1     5/14/05 2:57p Daniel.wang
// 
// 2     5/14/05 12:29p Hui.shao
// 
// 1     05-03-29 21:28 Daniel.wang
// 
// 2     05-03-22 14:06 Daniel.wang
// ===========================================================================


#ifndef _TAO_CORBA_EVENT_H_
#define _TAO_CORBA_EVENT_H_

#include <orbsvcs/orbsvcs/CosEventCommS.h>
#include <ORBSVCS/orbsvcs/CosEventChannelAdminC.h>
#include <ORBSVCS/orbsvcs/CosEventChannelAdminS.h>

#include "name.h"


/// -----------------------------
/// TaoEventChannel
/// -----------------------------
/// Desc : the event channel from TAO\CORBA cosevent service
/// naming service must running before TaoEventChannel created
/// @template - 
/// -----------------------------
class TaoEventChannel
{
public:
	class PushSupplier;
	class PushConsumer;
private:
	TaoNamingService*	m_tns;
	bool	m_bChannelLoaded;

	CosEventChannelAdmin::EventChannel_var	m_EventChannel;
public:
	/// constructor
	/// @param tns - the naming serivce
	/// @param strName - CosEvent service name
	/// @return - 
	TaoEventChannel(TaoNamingService* tns, const char* strName = "CosEventSerivce");

	/// destructor
	/// @param  - 
	/// @return - 
	~TaoEventChannel();

	/// set naming service instance
	/// @param tns - the naming serivce
	/// @return - return true if ok
	bool SetNamingService(TaoNamingService* tns);

	/// initialize CosEvent service
	/// @param strName - CosEvent service name
	/// @return - return true if ok
	bool Init(const char* strName);

	/// judge CosEvent serivce if loaded
	/// @param  - 
	/// @return - return true if loaded and false other wise
	bool IsLoaded();

	/// destroy service
	/// @param  - 
	/// @return - 
	void Destroy();

	/// get push consumer from CosEvent Service 
	/// @param tpc [out] pushconsumer pointer
	/// @return return true if ok
	bool GetPushConsumer(PushConsumer* tpc);

	/// get push supplier from CosEvent Service
	/// @param tps [out] push supplier pointer
	/// @return return true if ok
	bool GetPushSupplier(PushSupplier* tps);
};


/// -----------------------------
/// PushConsumer
/// -----------------------------
/// Desc : a push consumer object by a tao event channel
/// @template - 
/// -----------------------------
class TaoEventChannel::PushConsumer : public POA_CosEventComm::PushConsumer
{
private:
	CosEventChannelAdmin::ProxyPushSupplier_var	m_ProxyPushSupplier;

	bool	m_bLoaded;

protected:

	void push(const CORBA::Any& data ACE_ENV_ARG_DECL_NOT_USED)
		throw (CORBA::SystemException);
	void disconnect_push_consumer(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
		throw (CORBA::SystemException);

protected:

	/// call back function, OnPush will be called when an event pushed
	/// @param  - 
	/// @return - 
	virtual void OnPush(const CORBA::Any& data) = 0;

	/// call back function, OnConnect will be called when connected to supplier
	/// @param  - 
	/// @return - 
	virtual void OnConnect();

	/// call back function, OnDisconnect will be called when disconnected to supplier
	/// @param  - 
	/// @return - 
	virtual void OnDisconnect();

public:

	/// constructor
	/// @param  - 
	/// @return - 
	PushConsumer();


	/// set a proxy push supplier 
	/// @param supplier - push supplier object
	/// @return - 
	void SetSupplier(CosEventChannelAdmin::ProxyPushSupplier_var supplier);


	/// connect to cos event channel
	/// @param  - 
	/// @return - return true if connected
	virtual bool Connect();

	/// disconnect to cosevent channel
	/// @param  - 
	/// @return - 
	virtual void Disconnect();


	/// judge if this object loaded
	/// @param  - 
	/// @return - return true if loaded
	bool IsLoaded()
	{ return m_bLoaded; }
};


/// -----------------------------
/// PushSupplier
/// -----------------------------
/// Desc : a push supplier object by a tao event channel
/// @template - 
/// -----------------------------
class TaoEventChannel::PushSupplier : public POA_CosEventComm::PushSupplier
{
private:
	CosEventChannelAdmin::ProxyPushConsumer_var	m_ProxyPushConsumer;

	bool m_bLoaded;
protected:
	virtual void disconnect_push_supplier();
protected:

	/// call back function, OnConnect will be called when connected
	/// @param  - 
	/// @return - 
	virtual void OnConnect();

	/// call back function, OnDisconnect will be called when disconnected
	/// @param  - 
	/// @return - 
	virtual void OnDisconnect();
public:

	/// constructor
	/// @param  - 
	/// @return - 
	PushSupplier();


	/// set a proxy consumer
	/// @param consumer - porxy consumer object
	/// @return - 
	void SetConsumer(CosEventChannelAdmin::ProxyPushConsumer_var consumer);

	/// connect to cosevent channel
	/// @param  - 
	/// @return - return true if ok
	virtual bool Connect();

	/// disconnect to cosevent channel
	/// @param  - 
	/// @return - 
	virtual void Disconnect();

	/// push a data to event channel
	/// @param data - the data will be pushed
	/// @return - 
	void Push(const CORBA::Any& data);


	/// judge if this object loaded
	/// @param  - 
	/// @return - return ture if object loaded
	bool IsLoaded()
	{ return m_bLoaded; }
};

#endif//_TAO_CORBA_EVENT_H_
