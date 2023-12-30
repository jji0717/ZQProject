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
// Name  : event.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ISAAlarmImpl/corbalib/event.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-11 3:33p Daniel.wang
// 
// 1     5/14/05 2:57p Daniel.wang
// 
// 1     05-03-29 21:28 Daniel.wang
// 
// 2     05-03-22 14:06 Daniel.wang
// ===========================================================================

#include "event.h"

TaoEventChannel::TaoEventChannel(TaoNamingService* tns, const char* strName)
{
	SetNamingService(tns);
	Init(strName);
}

TaoEventChannel::~TaoEventChannel()
{
	Destroy();
}

bool TaoEventChannel::SetNamingService(TaoNamingService* tns)
{
	m_tns = tns;
	return NULL != m_tns;
}

bool TaoEventChannel::Init(const char* strName)
{
	m_EventChannel = NARROW_VAR(CosEventChannelAdmin::EventChannel, m_tns->ResolveObject(strName));
	m_bChannelLoaded = true;

	return true;
}

bool TaoEventChannel::IsLoaded()
{
	return m_bChannelLoaded;
}

void TaoEventChannel::Destroy()
{
	if (m_bChannelLoaded)
		m_bChannelLoaded = false;
}

bool TaoEventChannel::GetPushConsumer(TaoEventChannel::PushConsumer* tpc)
{
	if (tpc->IsLoaded())
		return false;

	CosEventChannelAdmin::ConsumerAdmin_var consumer_admin = m_EventChannel->for_consumers();
	CosEventChannelAdmin::ProxyPushSupplier_var ppsv = consumer_admin->obtain_push_supplier();

	tpc->SetSupplier(ppsv);
	return true;
}

bool TaoEventChannel::GetPushSupplier(TaoEventChannel::PushSupplier* tps)
{
	if (tps->IsLoaded())
		return false;

	
	CosEventChannelAdmin::SupplierAdmin_var supplier_admin = m_EventChannel->for_suppliers();
	CosEventChannelAdmin::ProxyPushConsumer_var ppcv = supplier_admin->obtain_push_consumer();

	tps->SetConsumer(ppcv);
	return true;
}



void TaoEventChannel::PushConsumer::push(const CORBA::Any& data ACE_ENV_ARG_DECL_NOT_USED)
{
	this->OnPush(data);
}

void TaoEventChannel::PushConsumer::disconnect_push_consumer(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
{
	this->OnDisconnect();
}

void TaoEventChannel::PushConsumer::OnConnect()
{
	//empty
}

void TaoEventChannel::PushConsumer::OnDisconnect()
{
	this->m_ProxyPushSupplier = CosEventChannelAdmin::ProxyPushSupplier::_nil();
}

TaoEventChannel::PushConsumer::PushConsumer()
:m_bLoaded(false)
{
}

void TaoEventChannel::PushConsumer::SetSupplier(CosEventChannelAdmin::ProxyPushSupplier_var supplier)
{
	m_ProxyPushSupplier = supplier;
	m_bLoaded = true;
}

bool TaoEventChannel::PushConsumer::Connect()
{
	CosEventComm::PushConsumer_var self = this->_this();
	this->m_ProxyPushSupplier->connect_push_consumer(self.in ());

	this->OnConnect();
	return true;
}

void TaoEventChannel::PushConsumer::Disconnect()
{
	this->m_ProxyPushSupplier->disconnect_push_supplier();
}

void TaoEventChannel::PushSupplier::disconnect_push_supplier()
{
	this->OnDisconnect();
}

void TaoEventChannel::PushSupplier::OnConnect()
{
	//empty
}

void TaoEventChannel::PushSupplier::OnDisconnect()
{
	this->m_ProxyPushConsumer = CosEventChannelAdmin::ProxyPushConsumer::_nil();
}

TaoEventChannel::PushSupplier::PushSupplier()
:m_bLoaded(false)
{
}

void TaoEventChannel::PushSupplier::SetConsumer(CosEventChannelAdmin::ProxyPushConsumer_var consumer)
{
	m_ProxyPushConsumer = consumer;
	m_bLoaded = true;
}

bool TaoEventChannel::PushSupplier::Connect()
{
	CosEventComm::PushSupplier_var self = this->_this();
	this->m_ProxyPushConsumer->connect_push_supplier(self.in ());

	this->OnConnect();
	return true;
}

void TaoEventChannel::PushSupplier::Disconnect()
{
	this->m_ProxyPushConsumer->disconnect_push_consumer();
}

void TaoEventChannel::PushSupplier::Push(const CORBA::Any& data)
{
	m_ProxyPushConsumer->push(data);
}
