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
// Name  : notify.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : TAO\CORBA notification service
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ISAAlarmImpl/corbalib/notify.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-11 3:34p Daniel.wang
// 
// 5     05-06-29 12:38 Yan.zheng
// 
// 4     05-06-27 19:33 Yan.zheng
// 
// 3     5/14/05 4:39p Hui.shao
// merged to ZQ dev environment
// 
// 2     5/14/05 3:28p Daniel.wang
// add comment
// 
// 1     5/14/05 2:57p Daniel.wang
// 
// 2     5/14/05 12:29p Hui.shao
// 
// 1     05-03-29 21:28 Daniel.wang
// 
// 2     05-03-22 14:06 Daniel.wang
// ===========================================================================

#ifndef _TAO_CORBA_NOTIFY_H_
#define _TAO_CORBA_NOTIFY_H_

#include <orbsvcs/orbsvcs/CosNotifyCommC.h>
#include <ORBSVCS/orbsvcs/CosNotifyChannelAdminS.h>
#include <ORBSVCS/orbsvcs/CosNotifyChannelAdminC.h>

#include "name.h"

/// -----------------------------
/// TaoNotifyEventChannel
/// -----------------------------
/// Desc : TAO\CORBA notification service event channel
/// @template - 
/// -----------------------------
class TaoNotifyEventChannel
{
public:
	class StructuredPushConsumer;
	class StructuredPushSupplier;
private:
	TaoNamingService*	m_tns;
	bool	m_bChannelLoaded;

	CosNotifyChannelAdmin::EventChannelFactory_var	m_EventChannelFactory;
public:

	/// constructor
	/// @param tns - naming service object
	/// @param strName - notification event service name
	/// @return - 
	TaoNotifyEventChannel(TaoNamingService* tns, const char* strName = "NotifyEventChannelFactory");

	/// destructor
	/// @param  - 
	/// @return - 
	~TaoNotifyEventChannel();


	/// set naming service object
	/// @param tns - naming service object
	/// @return - return true if ok
	bool SetNamingService(TaoNamingService* tns);


	/// initialize notification service
	/// @param strName - notification service name
	/// @return - return true if ok
	bool Init(const char* strName);


	/// judge if loaded
	/// @param  - 
	/// @return - return true if loaded
	bool IsLoaded();


	/// destroy notificaiton service
	/// @param  - 
	/// @return - 
	void Destroy();


	/// get a structured push consumer from notification service\n
	/// this function must called after GetStructuredPushSupplier
	/// @param tpc - [out]structured push consumer
	/// @param strChannelName - event channel name
	/// @return - return true if ok
	bool GetStructuredPushConsumer(TaoNotifyEventChannel::StructuredPushConsumer* tpc, const char* strChannelName);


	/// get a structured push supplier from notification service
	/// this function must called before GetStructuredPushConsumer
	/// @param tps - [out]structured push supplier
	/// @param strName - event channel name
	/// @return - return true if ok
	bool GetStructuredPushSupplier(TaoNotifyEventChannel::StructuredPushSupplier* tps, const char* strName);
};


/// -----------------------------
/// StructuredPushConsumer
/// -----------------------------
/// Desc : structured push consumer for notification service
/// @template - 
/// -----------------------------
class TaoNotifyEventChannel::StructuredPushConsumer : public POA_CosNotifyComm::StructuredPushConsumer
{
private:
	CosNotifyChannelAdmin::StructuredProxyPushSupplier_var	m_StructuredProxyPushSupplier;

	bool	m_bLoaded;
protected:
	virtual void push_structured_event(const CosNotification::StructuredEvent& event ACE_ENV_ARG_DECL)
  ACE_THROW_SPEC ((
                   CORBA::SystemException,
                   CosEventComm::Disconnected
                   ));

	virtual void disconnect_structured_push_consumer(ACE_ENV_SINGLE_ARG_DECL)
      ACE_THROW_SPEC ((
        CORBA::SystemException
      ));

	virtual void offer_change (
        const CosNotification::EventTypeSeq & added,
        const CosNotification::EventTypeSeq & removed
        ACE_ENV_ARG_DECL
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException,
        CosNotifyComm::InvalidEventType
      ))
	{}
	
protected:

	/// call back function, OnPush called if push a structured event
	/// @param event - the event object
	/// @return - 
	virtual void OnPush(const CosNotification::StructuredEvent& event) = 0;

	/// call back function, OnConnect called if connected to notification service
	/// @param  - 
	/// @return - 
	virtual void OnConnect();

	/// call back function, OnDisconnect called if disconnected
	/// @param  - 
	/// @return - 
	virtual void OnDisconnect();

public:

	/// constructor
	/// @param  - 
	/// @return - 
	StructuredPushConsumer();

	/// destructor
	/// @param  - 
	/// @return - 
	~StructuredPushConsumer();

	/// set structured proxy push supplier
	/// @param supplier - push supplier object
	/// @return - 
	void SetSupplier(CosNotifyChannelAdmin::StructuredProxyPushSupplier_var& supplier);
	

	/// connect to notification service
	/// @param  - 
	/// @return - return true if connected
	virtual bool Connect();

	/// disconnect to notification service
	/// @param  - 
	/// @return - 
	virtual void Disconnect();

	/// judge if consumer loaded
	/// @param  - 
	/// @return - return true if loaded
	bool IsLoaded()
	{ return m_bLoaded; }
};


/// -----------------------------
/// StructuredPushSupplier
/// -----------------------------
/// Desc : structured push supplier for notitication service
/// @template - 
/// -----------------------------
class TaoNotifyEventChannel::StructuredPushSupplier : public POA_CosNotifyComm::StructuredPushSupplier
{
private:
	CosNotifyChannelAdmin::StructuredProxyPushConsumer_var	m_StructuredProxyPushConsumer;
	CosNotifyChannelAdmin::EventChannel_var	m_EventChannel;

	bool	m_bLoaded;
protected:
	virtual void disconnect_structured_push_supplier(ACE_ENV_SINGLE_ARG_DECL)ACE_THROW_SPEC ((
        CORBA::SystemException
      ));

	virtual void subscription_change (
        const CosNotification::EventTypeSeq & added,
        const CosNotification::EventTypeSeq & removed
        ACE_ENV_ARG_DECL
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException,
        CosNotifyComm::InvalidEventType
      ))
	{}
protected:

	/// call back function, OnConnect called if connected
	/// @param  - 
	/// @return - 
	virtual void OnConnect();

	/// call back function, OnDisconnect called if disconnected
	/// @param  - 
	/// @return - 
	virtual void OnDisconnect();

public:

	/// constructor
	/// @param  - 
	/// @return - 
	StructuredPushSupplier();

	/// destructor
	/// @param  - 
	/// @return - 
	~StructuredPushSupplier();

	/// set structured proxy push consumer
	/// @param eventchannel - event channel
	/// @param consumer - structured proxy push consumer
	/// @return - 
	void SetConsumer(CosNotifyChannelAdmin::EventChannel_var &eventchannel, CosNotifyChannelAdmin::StructuredProxyPushConsumer_var& consumer);


	/// connect to notification service event channel
	/// @param  - 
	/// @return - return true if connected
	virtual bool Connect();

	/// disconnect notification service event channel
	/// @param  - 
	/// @return - 
	virtual void Disconnect();


	/// push a event to notificaiton serivce event channel
	/// @param event - event object
	/// @return - 
	void Push(const CosNotification::StructuredEvent& event);


	/// judge if supplier loaded
	/// @param  - 
	/// @return - 
	bool IsLoaded()
	{ return m_bLoaded; }
};

#endif//_TAO_CORBA_NOTIFY_H_
