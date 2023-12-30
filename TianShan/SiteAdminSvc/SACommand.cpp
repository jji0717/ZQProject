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
// Ident : $Id: SessionCommand.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/SiteAdminSvc/SACommand.cpp $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 6     08-07-21 13:53 Hongquan.zhang
// 
// 5     08-03-18 17:37 Xiaohui.chai
// changed interface of ListLiveTxn
// 
// 4     07-12-13 18:27 Hui.shao
// 
// 3     07-12-13 17:00 Hui.shao
// 
// 2     07-12-10 18:47 Hui.shao
// moved event out of txn
// ===========================================================================

#include "SACommand.h"

namespace ZQTianShan {
namespace Site {

// -----------------------------
// class SaveEventCommand
// -----------------------------
SaveEventCommand::SaveEventCommand(const ::TianShanIce::Site::AMD_TxnService_tracePtr& amdCB, SiteAdminSvcEnv& env, const ::std::string& sessId, const ::std::string& category, const ::std::string& eventCode, const ::std::string& eventMsg)
		: _env(env), ThreadRequest(env._thpool), _event(NULL), _amdCB(amdCB)
{
	_stamp = ::ZQTianShan::now();

	_event = new TxnEventImpl(_env);
	if (_event)
	{
		_event->identTxn.category = DBFILENAME_TxnEvent; _event->identTxn.name = sessId;
		_event->category = category;
		_event->eventCode = eventCode;
		_event->eventMsg = eventMsg;
	}
}

SaveEventCommand::SaveEventCommand(const ::Ice::Long stamp, SiteAdminSvcEnv& env, const ::std::string& sessId, const ::std::string& category, ::std::string& eventCode, const ::std::string& eventMsg)
	:_env(env), ThreadRequest(env._thpool), _event(NULL), _amdCB(NULL), _stamp(stamp)
{
	if (_stamp <=0)
		_stamp = now();
	
	_event = new TxnEventImpl(_env);
	if (_event)
	{
		_event->identTxn.category = DBFILENAME_Txn; _event->identTxn.name = sessId;
		_event->category = category;
		_event->eventCode = eventCode;
		_event->eventMsg = eventMsg;
	}
}

bool SaveEventCommand::init(void)
{
	return (NULL != _event);
}

int SaveEventCommand::run(void)
{
	std::string lastError;

	try {

		/// convert the stamp to the UTC formatted time string
		char stampbuf[32] = "";
		_event->stampUTC = ::ZQTianShan::TimeToUTC(_stamp, stampbuf, sizeof(stampbuf) -2);

		::Ice::Identity ident;
		ident.name = ::IceUtil::generateUUID(); ident.category = DBFILENAME_TxnEvent;
		_env._eTxnEvent->add(_event, ident);	

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin,"SaveEventCommand() saved: %s sess[%s] %s:%s %s"),
			_event->stampUTC.c_str(), _event->identTxn.name.c_str(), _event->category.c_str(), _event->eventCode.c_str(), _event->eventMsg.c_str());

		if (_amdCB)
			_amdCB->ice_response();

		return 0;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "SaveEventCommand (sess[%s] %s:%s) caught exception:%s", 
			_event->identTxn.name.c_str(), _event->category.c_str(), _event->eventCode.c_str(), ex.ice_name().c_str());
		lastError = buf;
	}

	if (_amdCB)
	{
		TianShanIce::ServerError ex("SiteAdmin", 500, lastError);
		_amdCB->ice_exception(ex);
	}
	else
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SiteAdmin,"%s"), lastError.c_str());

	return 1;
}
	

// -----------------------------
// class ListTxnCommand
// -----------------------------
ListTxnCommand::ListTxnCommand(const ::TianShanIce::Site::AMD_TxnService_listLiveTxnPtr& amdCB, SiteAdminSvcEnv& env, const ::std::string& siteName, const ::std::string& appMount, const ::TianShanIce::StrValues& paramNames, const ::std::string& startId, int maxCount)
: _env(env), ThreadRequest(env._thpool), _amdCB(amdCB),
 _siteName(siteName), _appMount(appMount), _paramNames(paramNames),
 _startId(startId), _maxCount(maxCount)
{
}

bool ListTxnCommand::init(void)
{
	return (NULL != _amdCB);
}

int ListTxnCommand::run(void)
{
	std::string lastError;
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListTxnCommand, "list live transaction by site[%s] and app[%s]"), _siteName.c_str(), _appMount.c_str());
	
	if ( "*" == _siteName)
		_siteName = "";
	
	if ( "*" == _appMount)
		_appMount = "";
	
	IdentCollection txnIdents;
	
	try	{
		IdentCollection identsBySite, identsByMount;
        bool bSorted = false;

		// always search by site no matter if it is empty
		if (_siteName.empty())
		{
			::Freeze::EvictorIteratorPtr itptr = _env._eLiveTxn->getIterator("", 100);
			while (itptr && itptr->hasNext())
				identsBySite.push_back(itptr->next());
		}
		else
            identsBySite = _env._idxSiteToTxn->find(_siteName);
		
		if (_appMount.empty())
		{
			txnIdents = identsBySite;
		}
		else
		{
			identsByMount = _env._idxMountToTxn->find(_appMount);
			
			if (_siteName.empty())
				txnIdents = identsByMount;
			else
			{
				// need merge while neither site and app is empty
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListTxnCommand, "merging results by site[%s]%d and app[%s]%d"),
					_siteName.c_str(), identsBySite.size(), _appMount.c_str(), identsByMount.size());
				::std::sort(identsBySite.begin(), identsBySite.end());
				::std::sort(identsByMount.begin(), identsByMount.end());
				for (IdentCollection::iterator it1= identsBySite.begin(), it2= identsByMount.begin(); it1 < identsBySite.end(); it1++)
				{
					int comp= -1;
					while (it2 < identsByMount.end() && (comp= (it2->name).compare(it1->name)) <0)
						it2++;
					
					if (0==comp)
					{
						txnIdents.push_back(*it1);
						it2++; 
					}
				}
			}

            bSorted = true;
		}

        if (!bSorted)
            ::std::sort(txnIdents.begin(), txnIdents.end());

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListTxnCommand, "found %d matched transactions"), txnIdents.size());

		// build up the transaction info collection based on the search result
		::TianShanIce::Site::TxnInfos results;

		for (IdentCollection::iterator it= txnIdents.begin(); it < txnIdents.end() && results.size() < _maxCount; it++)
		{
            if (it->name.compare(_startId) <0)
                continue;

			try {
				::TianShanIce::Site::LiveTxnPrx txn = IdentityToObjEnv(_env, LiveTxn, *it);
				::TianShanIce::Site::TxnInfo txnInfo;
				txnInfo.sessId = it->name;
				::TianShanIce::Properties props = txn->getProperties();

#define TXNPARAM_BEGIN  if (0)
#define TXNPARAM_ITEM(FIELD, SVALUE)   else if (0 == pit->compare(FIELD)) txnInfo.params.insert(::TianShanIce::Properties::value_type(FIELD, SVALUE))
#define TXNPARAM_END else if (props.end() !=props.find(*pit)) txnInfo.params.insert(::TianShanIce::Properties::value_type(*pit, props[*pit]))

				for (::TianShanIce::StrValues::iterator pit= _paramNames.begin(); pit < _paramNames.end(); pit++)
				{
					TXNPARAM_BEGIN;
						TXNPARAM_ITEM(SYS_PROP(siteName), txn->getSitename());
						TXNPARAM_ITEM(SYS_PROP(path),     txn->getPath());
						TXNPARAM_ITEM(SYS_PROP(state),    ZQTianShan::ObjStateStr(txn->getState()));
					TXNPARAM_END;
				}

				results.push_back(txnInfo);
			}
			catch (...) {}
		}

		_amdCB->ice_response(results);
		return 1;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListTxnCommand site[%s] app[%s] caught exception:%s", 
			_siteName.c_str(), _appMount.c_str(), ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListTxnCommand site[%s] app[%s] caught unknown exception", 
			_siteName.c_str(), _appMount.c_str());
		lastError = buf;
	}

	TianShanIce::ServerError ex("SiteAdmin", 500, lastError);
	_amdCB->ice_exception(ex);
	
	return 1;
}

}} // namespace


