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
// Name  : notify.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ISAAlarmImpl/corbalib/notify.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-11 3:34p Daniel.wang
// 
// 1     5/14/05 2:57p Daniel.wang
// 
// 1     05-03-29 21:28 Daniel.wang
// 
// 2     05-03-22 14:06 Daniel.wang
// ===========================================================================

#include "notify.h"

TaoNotifyEventChannel::TaoNotifyEventChannel(TaoNamingService* tns, const char* strName)
{
	SetNamingService(tns);
	Init(strName);
}

TaoNotifyEventChannel::~TaoNotifyEventChannel()
{
	Destroy();
}

bool TaoNotifyEventChannel::SetNamingService(TaoNamingService* tns)
{
	m_tns = tns;
	return NULL != m_tns;
}

bool TaoNotifyEventChannel::Init(const char* strName)
{
	m_EventChannelFactory = NARROW_VAR(CosNotifyChannelAdmin::EventChannelFactory, m_tns->ResolveObject(strName));
	m_bChannelLoaded = true;

	return true;
}

bool TaoNotifyEventChannel::IsLoaded()
{
	return m_bChannelLoaded;
}

void TaoNotifyEventChannel::Destroy()
{
	if (m_bChannelLoaded)
		m_bChannelLoaded = false;
}

bool TaoNotifyEventChannel::GetStructuredPushConsumer(TaoNotifyEventChannel::StructuredPushConsumer* tpc, const char* strChannelName)
{
	if (tpc->IsLoaded())
		return false;

	CosNotifyChannelAdmin::EventChannel_var eventchannel = NARROW_VAR(CosNotifyChannelAdmin::EventChannel, m_tns->ResolveObject(strChannelName));

	if (NULL == eventchannel.ptr())
		return false;


	CosNotifyChannelAdmin::AdminID adminid = 0;
	CosNotifyChannelAdmin::InterFilterGroupOperator ifgo = CosNotifyChannelAdmin::AND_OP;

	CosNotifyChannelAdmin::ConsumerAdmin_var consumer_admin = eventchannel->new_for_consumers(ifgo, adminid);

	CosNotifyChannelAdmin::ProxyID proxyid;
	CosNotifyChannelAdmin::ProxySupplier_var supplieradmin = consumer_admin->obtain_notification_push_supplier(CosNotifyChannelAdmin::STRUCTURED_EVENT, proxyid);

	CosNotifyChannelAdmin::StructuredProxyPushSupplier_var sppc = NARROW_VAR(CosNotifyChannelAdmin::StructuredProxyPushSupplier, supplieradmin);

	tpc->SetSupplier(sppc);
	return true;
}

bool TaoNotifyEventChannel::GetStructuredPushSupplier(TaoNotifyEventChannel::StructuredPushSupplier* tps, const char* strChannelName)
{
	if (tps->IsLoaded())
		return false;

	CosNotification::QoSProperties qos;
	CosNotification::AdminProperties admin;
	CosNotifyChannelAdmin::ChannelID id = 0;
	
	CosNotifyChannelAdmin::EventChannel_var eventchannel = m_EventChannelFactory->create_channel(qos, admin, id);

	m_tns->ReBind(strChannelName, eventchannel);

	CosNotifyChannelAdmin::AdminID adminid = 0;
	CosNotifyChannelAdmin::InterFilterGroupOperator ifgo = CosNotifyChannelAdmin::AND_OP;

	CosNotifyChannelAdmin::SupplierAdmin_var supplier_admin = eventchannel->new_for_suppliers(ifgo, adminid);

	CosNotifyChannelAdmin::ProxyID proxyid;
	CosNotifyChannelAdmin::ProxyConsumer_var consumeradmin = supplier_admin->obtain_notification_push_consumer(CosNotifyChannelAdmin::STRUCTURED_EVENT, proxyid);

	CosNotifyChannelAdmin::StructuredProxyPushConsumer_var sppc = NARROW_VAR(CosNotifyChannelAdmin::StructuredProxyPushConsumer, consumeradmin);

	tps->SetConsumer(eventchannel, sppc);
	return true;
}


void TaoNotifyEventChannel::StructuredPushConsumer::push_structured_event (const CosNotification::StructuredEvent& event ACE_ENV_ARG_DECL)
{
	this->OnPush(event);
}

void TaoNotifyEventChannel::StructuredPushConsumer::disconnect_structured_push_consumer(ACE_ENV_SINGLE_ARG_DECL)
{
	this->OnDisconnect();
}

void TaoNotifyEventChannel::StructuredPushConsumer::OnConnect()
{
	//empty
}

void TaoNotifyEventChannel::StructuredPushConsumer::OnDisconnect()
{
	m_StructuredProxyPushSupplier = CosNotifyChannelAdmin::StructuredProxyPushSupplier::_nil();
}

TaoNotifyEventChannel::StructuredPushConsumer::StructuredPushConsumer()
:m_bLoaded(false)
{
}

TaoNotifyEventChannel::StructuredPushConsumer::~StructuredPushConsumer()
{
	if (IsLoaded())
	{
		//this->Disconnect();
	}
}

void TaoNotifyEventChannel::StructuredPushConsumer::SetSupplier(CosNotifyChannelAdmin::StructuredProxyPushSupplier_var& supplier)
{
	m_StructuredProxyPushSupplier = supplier;

	m_bLoaded = true;
}

bool TaoNotifyEventChannel::StructuredPushConsumer::Connect()
{
	CosNotifyComm::StructuredPushConsumer_var self = this->_this();
	this->m_StructuredProxyPushSupplier->connect_structured_push_consumer(self.in());

	this->OnConnect();
	return true;
}

void TaoNotifyEventChannel::StructuredPushConsumer::Disconnect()
{
	this->m_StructuredProxyPushSupplier->disconnect_structured_push_supplier();
}


void TaoNotifyEventChannel::StructuredPushSupplier::disconnect_structured_push_supplier(ACE_ENV_SINGLE_ARG_DECL)
{
	this->OnDisconnect();
}

void TaoNotifyEventChannel::StructuredPushSupplier::OnConnect()
{
	//empty
}

void TaoNotifyEventChannel::StructuredPushSupplier::OnDisconnect()
{
	m_StructuredProxyPushConsumer = CosNotifyChannelAdmin::StructuredProxyPushConsumer::_nil();
}

TaoNotifyEventChannel::StructuredPushSupplier::StructuredPushSupplier()
:m_bLoaded(false)
{
}

TaoNotifyEventChannel::StructuredPushSupplier::~StructuredPushSupplier()
{
	if (IsLoaded())
	{
		//this->Disconnect();
	}
}

void TaoNotifyEventChannel::StructuredPushSupplier::SetConsumer(CosNotifyChannelAdmin::EventChannel_var &eventchannel, CosNotifyChannelAdmin::StructuredProxyPushConsumer_var& consumer)
{
	m_StructuredProxyPushConsumer = consumer;
	m_EventChannel = eventchannel;

	m_bLoaded = true;
}

bool TaoNotifyEventChannel::StructuredPushSupplier::Connect()
{
	CosNotifyComm::StructuredPushSupplier_var self = this->_this();
	this->m_StructuredProxyPushConsumer->connect_structured_push_supplier(self.in());

	this->OnConnect();
	return true;
}

void TaoNotifyEventChannel::StructuredPushSupplier::Disconnect()
{
	m_StructuredProxyPushConsumer->disconnect_structured_push_consumer();

	if (CORBA::is_nil(m_EventChannel.in()))
	{
		m_EventChannel->destroy();
	}
}

void TaoNotifyEventChannel::StructuredPushSupplier::Push(const CosNotification::StructuredEvent& event)
{
	m_StructuredProxyPushConsumer->push_structured_event(event);
}
