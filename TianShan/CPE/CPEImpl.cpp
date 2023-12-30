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
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/CPEImpl.cpp $
// 
// 8     6/12/16 4:50p Li.huang
// 
// 7     1/12/16 8:55a Dejian.fei
// 
// 6     8/05/13 4:32p Ketao.zhang
// 
// 5     4/18/13 5:27p Li.huang
// fix bug 18035
// 
// 4     3/12/12 3:11p Li.huang
// fix bug 15933
// 
// 3     10-12-08 18:06 Fei.huang
// * reimplement getExposeUrl, discard old full path scheme, use volume
// name instead.
// 
// 2     10-11-15 18:46 Fei.huang
// - comment out FtpOverVstrm flag on linux
// 
// 51    10-11-11 17:28 Fei.huang
// * no ftpovervstrm on linux
// 
// 50    10-10-28 13:44 Fei.huang
// * merge from 1.10
// 
// 49    10-04-07 15:24 Jie.zhang
// add parameter on updateProgress
// 
// 48    10-03-23 13:34 Xia.chen
// 
// 47    10-03-23 10:27 Xia.chen
// 
// 46    10-03-22 18:31 Xia.chen
// 
// 45    09-11-11 15:48 Xia.chen
// 
// 44    09-11-11 13:46 Xia.chen
// add maxPecentageStep  control
// 
// 43    09-07-24 15:07 Xia.chen
// add volume info for url
// 
// 41    09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 43    09-05-14 14:45 Jie.zhang
// 
// 42    09-05-13 21:52 Jie.zhang
// 
// 41    09-05-08 16:15 Xia.chen
// 
// 40    09-02-11 17:52 Jie.zhang
// add try catch on init()
// 
// 39    09-02-06 10:41 Jie.zhang
// 
// 38    09-01-22 16:01 Yixin.tian
// 
// 37    09-01-21 14:30 Jie.zhang
// the canceled provision would not send "stop event"
// 
// 36    09-01-20 17:59 Jie.zhang
// 
// 35    09-01-09 11:31 Yixin.tian
// modify warning
// 
// 34    08-12-12 13:45 Yixin.tian
// modify for Linux OS
// 
// 33    08-12-08 15:23 Jie.zhang
// 
// 32    08-11-18 10:59 Jie.zhang
// merge from TianShan1.8
// 
// 38    08-10-24 14:48 Jie.zhang
// fixed "cancelProvision" return error status
// 
// 37    08-10-24 10:56 Xia.chen
// 
// 36    08-09-17 15:42 Jie.zhang
// getExportUrl add maxBitrate limit
// 
// 35    08-09-17 14:39 Jie.zhang
// added some log in getExportUrl
// 
// 34    08-09-11 17:42 Jie.zhang
// getExportUrl parameter changed
// 
// 33    08-08-15 17:37 Xia.chen
// 
// 32    08-08-14 18:46 Jie.zhang
// add getExportUrl
// 
// 31    08-07-24 12:16 Jie.zhang
// fixed sometime the progress too frequncy.
// 
// 30    08-07-16 14:40 Jie.zhang
// _bSucc initialize must be set to true
// 
// 29    08-07-11 13:50 Jie.zhang
// add export content support
// 
// 28    08-06-24 14:56 Jie.zhang
// changestatecmd removed, "setup","ontimer","onrestore" process in ice
// server dispatch thread.
// 
// 27    08-06-19 18:57 Jie.zhang
// change the inital value 
// 
// 26    08-06-10 19:04 Jie.zhang
// 
// 25    08-06-04 12:33 Jie.zhang
// 
// 24    08-05-30 16:44 Jie.zhang
// 
// 23    08-05-30 15:56 Jie.zhang
// 
// 22    08-05-30 14:11 Jie.zhang
// 
// 21    08-05-17 19:09 Jie.zhang
// 
// 20    08-05-14 22:11 Jie.zhang
// 
// 19    08-05-13 11:31 Jie.zhang
// 
// 18    08-04-25 16:08 Jie.zhang
// 
// 17    08-04-09 18:16 Jie.zhang
// 
// 16    08-04-09 15:37 Hui.shao
// impl listMethods
// 
// 15    08-04-09 11:48 Hui.shao
// added ProvisionCost
// 
// 14    08-04-02 15:47 Hui.shao
// per CPC ICE changes
// 
// 13    08-03-28 16:11 Build
// 
// 17    08-04-01 18:53 Jie.zhang
// 
// 16    08-03-24 19:41 Jie.zhang
// 
// 15    08-03-17 19:56 Jie.zhang
// 
// 13    08-03-07 21:29 Jie.zhang
// 
// 12    08-03-07 18:13 Jie.zhang
// 
// 11    08-02-28 16:17 Jie.zhang
// 
// 10    08-02-21 15:08 Hui.shao
// added paged list
// 
// 9     08-02-20 16:13 Jie.zhang
// 
// 8     08-02-20 16:02 Hongquan.zhang
// lock CPESess at OnTimer and forcetoStart function because we can't
// change the sess' stte simultaneously
// 
// 7     08-02-18 20:59 Hui.shao
// replaced priviate data with property
// 
// 6     08-02-18 18:48 Jie.zhang
// changes check in
// 
// 5     08-02-18 18:44 Hui.shao
// 
// 4     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 3     08-02-14 18:47 Hui.shao
// impled ProvisionSessionBind callbacks
// 
// 2     08-02-14 16:26 Hui.shao
// added ami callbacks
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================
#include <time.h>

#include "CPEImpl.h"
#include "CPEEnv.h"
#include "ProvisionCmds.h"
#include "ProvisionState.h"
#include "CPHInc.h"
#include "CPECfg.h"

#include "Log.h"
#include "FTPAccount.h"
#include "urlstr.h"
#include "HostToIP.h"




#ifdef _WITH_EVENTSENDER_
	#include "eventsendermanager.h"
	extern ZQTianShan::Site::EventSenderManager*	g_pEventSinkMan;
#endif //_WITH_EVENTSENDER_


void GenerateFtpUrl(const char* orginUrl, std::string& newRootUrl)
{
	std::string ip;
	std::string temp;
    char portBuff[16];
	
	ZQ::common::URLStr url(orginUrl);
	ZQ::common::URLStr newUrl;
	if (!HostToIP::translateHostIP(url.getHost(),ip))
		ip = url.getHost();
	newUrl.setProtocol(url.getProtocol());
	newUrl.setHost(ip.c_str());
	newUrl.setPort(url.getPort());
	newUrl.setPath(url.getPath());
		
	itoa(newUrl.getPort(), portBuff, 10);
	newRootUrl= std::string(newUrl.getProtocol()) + "://"+ std::string(newUrl.getHost()) + ":" + portBuff + "/" + std::string(newUrl.getPath());
}

namespace ZQTianShan {
namespace CPE {

	
using namespace ZQ::common;

typedef ::std::vector< Ice::Identity > IdentCollection;
#define IdentityToObj(_CLASS, _ID) ::TianShanIce::ContentProvision::##_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))
#define CATEG_Txn			"Txn"

// -----------------------------
// service CPEImpl
// -----------------------------
CPEImpl::CPEImpl (CPEEnv& env)
: _env(env)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CPE, "start watchdog"));
	_env._watchDog.start();
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CPE, "restore all the sessions from last run"));
	IdentCollection identities;
	try	{
		ZQ::common::MutexGuard gd(_env._lockProvisionSession);
		::Freeze::EvictorIteratorPtr itptr = _env._eProvisionSession->getIterator("", 100);
		while (itptr->hasNext())
			identities.push_back(itptr->next());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(CPE, "failed to list all the existing sessions"));
	}
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CPE, "add the interface \"%s\" on to the adapter"), SERVICE_NAME_ContentProvisionService);

	try	
	{
		_env._adapter->ZQADAPTER_ADD(_env._communicator, this, SERVICE_NAME_ContentProvisionService);
	}
	catch (Ice::Exception& ex) 
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(CPE, "ZQADAPTER_ADD() caught ice exception[%s]"), ex.ice_name().c_str());		
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(CPE, "ZQADAPTER_ADD() caught unknown exception"));
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CPE, "%d session(s) found need to restore"), identities.size());
	
	for (IdentCollection::iterator it = identities.begin(); it !=identities.end(); it ++)
	{
		::TianShanIce::ContentProvision::ProvisionSessionExPrx sess;
		std::string sessid = it->name;
		
		try {
			(new RestoreCmd(_env, *it))->start();
		}
		catch(...) {}
	}	
}

CPEImpl::~CPEImpl()
{
//	_env._adapter->remove(_thisPrx);
}

::std::string CPEImpl::getAdminUri(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	::ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (envlog, EXPFMT(CPE, 101, __MSGLOC__ "TODO: impl here"));
	return ""; // dummy statement to avoid compiler error
}

 ::TianShanIce::State CPEImpl::getState(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	::ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (envlog, EXPFMT(CPE, 201, __MSGLOC__ "TODO: impl here"));
	return ::TianShanIce::stInService; // dummy statement to avoid compiler error
}

::std::string CPEImpl::getNetId(const ::Ice::Current& c) const 
{
	return _gCPECfg._cpeNetId;
}

::TianShanIce::ContentProvision::ProvisionSessionPrx CPEImpl::createSession(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, const ::std::string& methodType, ::TianShanIce::ContentProvision::ProvisionOwnerType ownerType, const ::TianShanIce::Storage::ContentPrx& primaryContent, const ::TianShanIce::ContentProvision::ProvisionSessionBindPrx& owner, const ::Ice::Current& c)
{
	//create purchase
	::TianShanIce::ContentProvision::ProvisionSessionPrx sess;

	Ice::Identity ident = ProvisionSessImpl::ContentKeyToIdent(contentKey);
	if (_env._eProvisionSession->hasObject(ident) || (_env._provisionFactory && _env._provisionFactory->findHelperSession((char*)ident.name.c_str())))
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(CPE, "createSession() provision session for content[%s] on volume[%s] of ContentStore[%s] already exist"), contentKey.content.c_str(), contentKey.volume.c_str(), contentKey.contentStoreNetId.c_str());
		::ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog, EXPFMT(CPE, 310, "session[%s] already exists"), ident.name.c_str());
	}

	try
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CPE, "createSession() creating a provision session for content[%s] on volume[%s] of ContentStore[%s]"), contentKey.content.c_str(), contentKey.volume.c_str(), contentKey.contentStoreNetId.c_str());
		ProvisionSessImpl::Ptr pSess = new ProvisionSessImpl(_env);
		if (!pSess)
			return NULL;
		
		pSess->contentKey = contentKey;
		pSess->ident = ProvisionSessImpl::ContentKeyToIdent(contentKey);
		pSess->state = ::TianShanIce::ContentProvision::cpsCreated;
		pSess->provType = ::TianShanIce::ContentProvision::ptUnknown;
		pSess->ownerType = ownerType;
		pSess->streamable = false;
		
		pSess->primaryContent = primaryContent;
		pSess->secondaryContent = NULL;
		pSess->owner = owner;

		// initial with default mask
		static ::TianShanIce::ContentProvision::ProvisionSubscribeMask DEFAULT_SUBMASK = {0, false, true, true, true, true};
		pSess->subMask = DEFAULT_SUBMASK;
		
		pSess->methodType = methodType;
		
		pSess->scheduledStart = 0;
		pSess->scheduledEnd =0;
		pSess->preload = pSess->linger = 0;
		ProvStateCreated(_env, *pSess.get()).enter();
		
		envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(CPE, "createSession() add the new provision session[%s] into DB"), pSess->ident.name.c_str());
		_env._eProvisionSession->add(pSess, pSess->ident);	
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(CPE, "created a new provision session[%s]"), pSess->ident.name.c_str()); 

		sess = IdentityToObjEnv(_env, ProvisionSession, pSess->ident);
	}
	catch (Ice::Exception& ex) 
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(CPE, 306, "createSession() ice exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(CPE, 307, "createSession() unknown exception"));
	}

	return sess;
}

::TianShanIce::ContentProvision::ProvisionSessionPrx CPEImpl::openSession(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, const ::Ice::Current& c)
{
	::TianShanIce::ContentProvision::ProvisionSessionPrx sess;
	Ice::Identity ident = ProvisionSessImpl::ContentKeyToIdent(contentKey);

	try
	{
		sess = ::TianShanIce::ContentProvision::ProvisionSessionPrx::checkedCast(_env._adapter->createProxy(ident));
	}
	catch (Ice::Exception&)
	{
		sess = NULL;
	}	

	return sess;
}

void CPEImpl::listSessions_async(const ::TianShanIce::ContentProvision::AMD_ContentProvisionService_listSessionsPtr& amdCB, const ::std::string& methodType, const ::TianShanIce::StrValues& paramNames, const ::std::string& startId, ::Ice::Int maxCount, const ::Ice::Current& c) const
{
	try {
		(new ListProvisionCmd(amdCB, _env, methodType, paramNames, startId, maxCount))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(CPE,"listSessions_async() failed to initial ListProvisionCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("CPE", 500, "failed to generate ListProvisionCmd"));
	}
}

void CPEImpl::listMethods_async(const ::TianShanIce::ContentProvision::AMD_ContentProvisionService_listMethodsPtr& amdCB, const ::Ice::Current& c) const
{
	try {
		(new ListMethodCmd(amdCB, _env))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(CPE,"listMethods_async() failed to initial ListMethodCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("CPE", 501, "failed to generate ListMethodCmd"));
	}
}

::TianShanIce::ContentProvision::ExportMethods CPEImpl::listExportMethods(const ::Ice::Current& ic) const 
{
	::TianShanIce::ContentProvision::ExportMethods methods;
	::TianShanIce::ContentProvision::ExportMethod meth;

#ifdef ZQ_OS_MSWIN
	if (_gCPECfg._dwEnableFtpOverVstrm)
	{
#endif
		meth.protocal = ::TianShanIce::Storage::potoFTP;
		std::string tempUrl;
		GenerateFtpUrl(_gCPECfg._ftpRootUrl,tempUrl);
		meth.rootUrl=tempUrl;
		meth.maxBwKbps = _gCPECfg._ftpMaxBandWidth;
		methods.push_back(meth);
#ifdef ZQ_OS_MSWIN
	}
#endif

	return methods;
}

::std::string CPEImpl::getExportURL(const ::std::string& protocal,const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, ::Ice::Int transferBitrate, ::Ice::Int& nTTL, ::Ice::Int& permittedBitrate, const ::Ice::Current& ic)
{
	std::string strUrl;
	std::string username;
	std::string password;
	std::string rootUrl;

	envlog(ZQ::common::Log::L_INFO, 
		CLOGFMT(CPE,"getExportUrl() protocal[%s] content[%s] volume[%s] bitrate[%d]"), 
		protocal.c_str(),contentKey.content.c_str(),contentKey.volume.c_str(),transferBitrate);

	if (!stricmp(protocal.c_str(), TianShanIce::Storage::potoFTP.c_str()))
	{
#ifdef ZQ_OS_MSWIN
		if (_gCPECfg._dwEnableFtpOverVstrm)
#endif
		    GenerateFtpUrl(_gCPECfg._ftpRootUrl,rootUrl);


		if (rootUrl.size() == 0)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(CPE,"The root URL is NULL"));
			::ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(envlog, EXPFMT(CPE, 307, "The root URL is NULL"));
		}

		nTTL = _gCPECfg._timeForLive;
		std::string partUrl = rootUrl.substr(rootUrl.find_first_of("://")+3,rootUrl.size()-rootUrl.find_first_of("://")-1);
		if (partUrl[partUrl.size()-1] != '/')
			partUrl += "/";

		if (!transferBitrate)
			transferBitrate = _gCPECfg._dwExportBitrate;

		if (transferBitrate > _gCPECfg._dwMaxBitrate)
		{
			permittedBitrate = _gCPECfg._dwMaxBitrate;
		}
		else
		{
			permittedBitrate = transferBitrate;
		}

		int nExpiredTime = (int)time(0) + nTTL;
		ExportAccountGen::generate(contentKey.content,nExpiredTime,permittedBitrate, username,password);

#ifdef ZQ_OS_MSWIN
		strUrl = protocal + "://" + username + ":" + password + "@" + partUrl + contentKey.content;
#else

		if (contentKey.volume.empty()) {
			envlog(ZQ::common::Log::L_ERROR, 
				CLOGFMT(CPE,"Unsupported volume %s"), 
				contentKey.volume.c_str());

			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
				envlog, EXPFMT(CPE, 307, "Unsupported volume %s"), 
				contentKey.volume.c_str());
		} 
		std::string volumeName = "";
		std::string strFolder = "";
		std::string tmpVolume = contentKey.volume.substr(1, contentKey.volume.size()-1);
		int nPos = tmpVolume.find('/');
		if(nPos > 0)
		{
			volumeName = tmpVolume.substr(1, nPos -1);
			strFolder =  tmpVolume.substr(nPos) + "/";
		}
		else
			volumeName = tmpVolume;

		if (volumeName == "$")
			volumeName = "";

		std::string strFileVolume = volumeName + strFolder;

		if(strFileVolume.size() > 0 && strFileVolume[0] == '/')
		{
			strFileVolume = strFileVolume.substr(1);
		}


		strUrl = protocal + "://" + username + ":" + password + "@" + partUrl + strFileVolume + contentKey.content;
#endif
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(CPE,"getExportUrl(%s) return url[%s]"), contentKey.content.c_str(), strUrl.c_str());
	}
	else
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(CPE,"Unsupported protocal %s"), protocal.c_str());
		::ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(envlog, EXPFMT(CPE, 307, "Unsupported protocal %s"), protocal.c_str());
	}

	return strUrl;
}




// -----------------------------
// class ProvisionSessImpl
// -----------------------------
#define PROVISIONSESSLOGFMT(_X) CLOGFMT(ProvisionSession, "provision[%s:%s(%d)] " _X), ident.name.c_str(), ProvisionStateBase::stateStr(state), state
#define PROVISIONSESSEXPFMT(_ERRCODE, _X) EXPFMT(ProvisionSession, _ERRCODE, "provision[%s:%s(%d)] " _X), ident.name.c_str(), ProvisionStateBase::stateStr(state), state

Ice::Identity ProvisionSessImpl::ContentKeyToIdent(const ::TianShanIce::ContentProvision::ProvisionContentKey& key)
{
	Ice::Identity ret;
	ret.category = DBFILENAME_ProvisionSession;
	ret.name = key.content + "|" + key.contentStoreNetId + "|" + key.volume;
	return ret;
}

const char* ProvisionSessImpl::typeStr(const ::TianShanIce::ContentProvision::ProvisionType type)
{
#define SWITCH_CASE_TYPE(_ST)	case ::TianShanIce::ContentProvision::pt##_ST: return #_ST
	switch(type)
	{
		SWITCH_CASE_TYPE(Unknown);
		SWITCH_CASE_TYPE(Catcher);
		SWITCH_CASE_TYPE(Pitcher);
		SWITCH_CASE_TYPE(PassThru);
	default:
		return "Unknown";
	}
#undef SWITCH_CASE_TYPE
}

const char* ProvisionSessImpl::startTypeStr(const ::TianShanIce::ContentProvision::StartType stype)
{
#define SWITCH_CASE_TYPE(_ST)	case ::TianShanIce::ContentProvision::st##_ST: return #_ST
	switch(stype)
	{
		SWITCH_CASE_TYPE(Scheduled);
		SWITCH_CASE_TYPE(ScheduledRestorable);
		SWITCH_CASE_TYPE(PushTrigger);
	default:
		return "Unknown";
	}
#undef SWITCH_CASE_TYPE
}

ProvisionSessImpl::ProvisionSessImpl(CPEEnv& env)
:_bSucc(true), _env(env), _progressProcessed(0), _progressTotal(0), _progressLatest(0)
{
	_progressLatestStamp.time = 0;
	_progressLatestStamp.millitm = 0;
	provType = TianShanIce::ContentProvision::ptCatcher;
	stType = TianShanIce::ContentProvision::stScheduled;
}

ZQTianShan::ContentProvision::ICPHSession* ProvisionSessImpl::getCPHSession() const
{
	RLock sync(*this);

	if (state > ::TianShanIce::ContentProvision::cpsAccepted)
	{
		ZQTianShan::ContentProvision::ICPHelper* pHelper = _env._provisionFactory->findHelper(methodType.c_str());
		if (NULL != pHelper)
			return pHelper->find(ident.name.c_str());
	}

	return NULL;
}

bool ProvisionSessImpl::isStreamable(const Ice::Current& c)
{
	WLock sync(*this);

	try {
		if (::TianShanIce::ContentProvision::cpsProvisioning == state)
		{
			envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("isStreamable() query helper session for latest status"));
			ZQTianShan::ContentProvision::ICPHSession* pCPHSess = getCPHSession();
			if (NULL != pCPHSess)
			{
				streamable = pCPHSess->isStreamable();
			}
		}
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_WARNING, PROVISIONSESSLOGFMT("isStreamable() caugt exception when query helper session"));
	}
	
	return streamable;
}

::TianShanIce::ContentProvision::ProvisionState ProvisionSessImpl::getState(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return state;
}

::TianShanIce::ContentProvision::ProvisionContentKey ProvisionSessImpl::getContentKey(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return contentKey;
}

::TianShanIce::ContentProvision::ProvisionType ProvisionSessImpl::getProvisionType(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return provType;
}

::std::string ProvisionSessImpl::getMethodType(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return methodType;
}

::TianShanIce::Properties ProvisionSessImpl::_getProperties() const
{
	return props;
}

::TianShanIce::Properties ProvisionSessImpl::getProperties(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return props;
}

void ProvisionSessImpl::_setProperty(const ::std::string& key, const ::std::string& val)
{
	::TianShanIce::Properties::const_iterator it = props.find(key);
	if (props.end() == it)
		props.insert(::TianShanIce::Properties::value_type(key, val));
	else
		props[key] = val;

	envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("setProperty %s=[%s]"), key.c_str(), val.c_str());		
}

void ProvisionSessImpl::setProperty(const ::std::string& key, const ::std::string& val, const ::Ice::Current& c)
{
	WLock sync(*this);	

	_setProperty(key, val);		
}

void ProvisionSessImpl::setTrickSpeedCollection(const ::TianShanIce::ContentProvision::TrickSpeedCollection& col, const ::Ice::Current& c)
{
	WLock sync(*this);	
	if (col.size())
		trickSpeeds = col;
	else
		trickSpeeds.push_back(7.5);

	envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("setTrickSpeedCollection"));		
}

void ProvisionSessImpl::addResource(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::ValueMap& resData, const ::Ice::Current& c)
{
	WLock sync(*this);		

	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("addResource() type=%s, entering"), ZQTianShan::ResourceTypeStr(type));
	switch(state)
	{
	case ::TianShanIce::ContentProvision::cpsCreated:
		ProvStateCreated(_env, *this).doAddResource(type, resData, c); break;
	case ::TianShanIce::ContentProvision::cpsAccepted:
		ProvStateAccepted(_env, *this).doAddResource(type, resData, c); break;
	case ::TianShanIce::ContentProvision::cpsWait:
		ProvStateWait(_env, *this).doAddResource(type, resData, c); break;
	case ::TianShanIce::ContentProvision::cpsReady:
		ProvStateReady(_env, *this).doAddResource(type, resData, c); break;
	case ::TianShanIce::ContentProvision::cpsProvisioning:
		ProvStateProvisioning(_env, *this).doAddResource(type, resData, c); break;
	case ::TianShanIce::ContentProvision::cpsStopped:
		ProvStateStopped(_env, *this).doAddResource(type, resData, c); break;
	}

	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("addResource() type=%s, leaving"), ZQTianShan::ResourceTypeStr(type));
}

::TianShanIce::SRM::ResourceMap ProvisionSessImpl::getResources(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return resources;
}

void ProvisionSessImpl::getScheduledTime(::std::string& startTimeUTC, ::std::string& endTimeUTC, const ::Ice::Current& c) const
{
	startTimeUTC = endTimeUTC = "";
	RLock sync(*this);
	char buf[64];
	if (scheduledStart>0)
		startTimeUTC = ZQTianShan::TimeToUTC(scheduledStart, buf, sizeof(buf)-2);
	
	if (scheduledEnd>0)
		endTimeUTC = ZQTianShan::TimeToUTC(scheduledEnd, buf, sizeof(buf)-2);
}

void ProvisionSessImpl::updateScheduledTime(const ::std::string& startTimeUTC, const ::std::string& endTimeUTC, const ::Ice::Current& c)
{
	char szTime[64];
	::Ice::Long startTime, endTime;
	startTime = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
	endTime = ZQTianShan::ISO8601ToTime(endTimeUTC.c_str());

	if (endTime>0 && endTime <= startTime)
	{
		::ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT(ProvisionSess, 310, __MSGLOC__ "invalid endTime[%s]"), endTimeUTC.c_str());
	}

	WLock sync(*this);		
	if (state <TianShanIce::ContentProvision::cpsReady)
	{
		// not started yet, could change
		if (startTime>0 && scheduledStart != startTime)
		{
			TimeToUTC(scheduledStart, szTime, sizeof(szTime));
			scheduledStart = startTime;
			ProvStateWait(_env, *this).OnTimer(c);
			envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("updateScheduledTime() scheduledStart changed from [%s] to [%s]"), szTime, startTimeUTC.c_str());
		}
		
		if (endTime>0 && scheduledEnd != endTime)
		{
			TimeToUTC(scheduledEnd, szTime, sizeof(szTime));
			scheduledEnd = endTime;
			envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("updateScheduledTime() scheduledEnd changed from [%s] to [%s]"), szTime, endTimeUTC.c_str());
		}
	}
	else if (state == ::TianShanIce::ContentProvision::cpsStopped)
	{
		//refuse to change at this state
		envlog(ZQ::common::Log::L_WARNING, PROVISIONSESSLOGFMT("updateScheduledTime() refuse to change schedule time at current state[cpsStopped]"));			
		::ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, EXPFMT(ProvisionSess, 311, __MSGLOC__ "can not change schedule time at state[stopped]"));
	}
	else
	{
		if (endTime>0 && endTime != scheduledEnd)
		{
			TimeToUTC(scheduledEnd, szTime, sizeof(szTime));
			scheduledEnd = endTime;
			envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("updateScheduledTime() scheduledEnd changed from [%s] to [%s]"), szTime, endTimeUTC.c_str());

			if (state ==::TianShanIce::ContentProvision::cpsProvisioning)
			{
				ZQTianShan::ContentProvision::ICPHSession* pCPHSess = _env._provisionFactory->findHelperSession(ident.name.c_str());
				if (NULL == pCPHSess)
				{
					envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("updateScheduledTime()  fail to findHelperSession[%s]"),ident.name.c_str());
				}
				else
				{
					char timeBuffer[64] = {0};
					TimeToUTC(scheduledStart, timeBuffer, sizeof(timeBuffer));
					std::string startUTC =  timeBuffer;
					pCPHSess->updateScheduledTime(startUTC,endTimeUTC);
				}
				ProvStateProvisioning(_env, *this).OnTimer(c);
			}
			else
			{
				// cpsReady state, we do not need do anything
			}
		}		
	}	

	_env._pProvStore.updateProvisionStore(ident, methodType, scheduledStart/1000, scheduledEnd/1000, _getBandwidth()/1000);
}

void ProvisionSessImpl::queryProgress(::Ice::Long& processed, ::Ice::Long& total, const ::Ice::Current& c) const
{
	RLock sync(*this);		

	processed = _progressProcessed;
	total = _progressTotal;
}

void ProvisionSessImpl::setup_async(const ::TianShanIce::ContentProvision::AMD_ProvisionSession_setupPtr& amdCB, const ::std::string& startTimeUTC, const ::std::string& endTimeUTC, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("schedule start[%s] end[%s]"), startTimeUTC.c_str(), endTimeUTC.c_str());
	if (::TianShanIce::ContentProvision::cpsCreated != state)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("invalid state to setup"));
		amdCB->ice_exception(::TianShanIce::InvalidStateOfArt("ProvisionSetup", 1001, "invalid state to setup"));
		return;
	}

	{
		WLock sync(*this);		
		scheduledStart = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
		scheduledEnd = ZQTianShan::ISO8601ToTime(endTimeUTC.c_str());
	}

	if (scheduledEnd < ZQTianShan::now())
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("schedule end[%s] is at a passed time"), endTimeUTC.c_str());
		amdCB->ice_exception(::TianShanIce::InvalidParameter("ProvisionSetup", 1002, "end at a passed time"));
		return;
	}

	if (::TianShanIce::ContentProvision::ptCatcher == provType && scheduledStart < ZQTianShan::now() - _env._scheduleErrorWindow)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("schedule start[%s] is at a passed time, check window[%d]ms"), startTimeUTC.c_str(), _env._scheduleErrorWindow);
		amdCB->ice_exception(::TianShanIce::InvalidParameter("ProvisionSetup", 1003, "schedule start is at a passed time"));
		return;
	}
	
	try 
	{
		if (stType == ::TianShanIce::ContentProvision::stPushTrigger)
		{
			std::string strPushUrl;
			GenerateFtpUrl(_gCPECfg._ftpRootUrl,strPushUrl);
			if(!(strPushUrl[strPushUrl.length()-1]=='/'))
				strPushUrl+="/";
			
			if (!contentKey.contentStoreNetId.empty())
				strPushUrl += contentKey.contentStoreNetId + "/" + contentKey.content;
			else
				strPushUrl += contentKey.content;
			
			envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("fixed up the pushURL as %s"), strPushUrl.c_str());
			setProperty(PROPTY_PUSHURL, strPushUrl, ::Ice::Current());
		}

		ProvStateAccepted(_env, *this).enter();
		amdCB->ice_response();
		return;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("forward exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
		amdCB->ice_exception(ex);
	}
	catch (const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("forward exception[%s]"), ex.ice_name().c_str());
		amdCB->ice_exception(ex);
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("forward unknown exception"));
		amdCB->ice_exception(::TianShanIce::ServerError("ProvisionSetup", 0, "unknown exception"));
	}

	//
	// error happen
	//
	try
	{
		ProvStateStopped(_env, *this).enter();
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("setup() failed, change state to Stopped(%d) exception[%s] %s"), ::TianShanIce::ContentProvision::cpsStopped, ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("setup() failed, change state to Stopped(%d) exception[%s]"), ::TianShanIce::ContentProvision::cpsStopped, ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("setup() failed, change state to Stopped(%d) unknown exception"), ::TianShanIce::ContentProvision::cpsStopped);
	}
}

void ProvisionSessImpl::commit(const ::Ice::Current& c)
{
	WLock sync(*this);		

	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("commit() entering"));
	switch(state)
	{
	case ::TianShanIce::ContentProvision::cpsCreated:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVISIONSESSEXPFMT(404, "commit() not allowed in this state"));

	case ::TianShanIce::ContentProvision::cpsAccepted:
		ProvStateWait(_env, *this).enter(); break;

// 		state =::TianShanIce::ContentProvision::cpsWait;
// 		_env._watchDog.watchSession(ident, (long) (scheduledStart - preload - now()));
// 		break;
	case ::TianShanIce::ContentProvision::cpsWait:
	case ::TianShanIce::ContentProvision::cpsReady:
	case ::TianShanIce::ContentProvision::cpsProvisioning:
	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVISIONSESSEXPFMT(405, "commit() session already committed"));
	}

	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("commit() executed"));
}

void ProvisionSessImpl::cancel(::Ice::Int clientErrorCode, const ::std::string& reason, const ::Ice::Current& c)
{
	WLock sync(*this);
	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("cancel() entering"));

	//
	// make as failure of user canceled, it is important for CPE that know this session is failure status
	//
	_setProvisionCanceled();	
	_bSucc = false;

	switch(state)
	{
	case ::TianShanIce::ContentProvision::cpsCreated:
	case ::TianShanIce::ContentProvision::cpsAccepted:
	case ::TianShanIce::ContentProvision::cpsWait:
	case ::TianShanIce::ContentProvision::cpsReady:
	case ::TianShanIce::ContentProvision::cpsProvisioning:
		ProvStateStopped(_env, *this).enter();
		break;

	case ::TianShanIce::ContentProvision::cpsStopped:
	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVISIONSESSEXPFMT(405, "cancel() session already stopped"));
	}
	
	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("cancel() executed"));
}

::std::string ProvisionSessImpl::getProvisionId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident.name;
}

::Ice::Long ProvisionSessImpl::_getBandwidth() const
{
	::TianShanIce::SRM::ResourceMap::const_iterator itResBw = resources.find(::TianShanIce::SRM::rtProvisionBandwidth);
	if (resources.end() == itResBw)
		return 0;

	::TianShanIce::ValueMap::const_iterator itVal = itResBw->second.resourceData.find("bandwidth");
	if (itResBw->second.resourceData.end() == itVal)
		return 0;

	if (TianShanIce::vtLongs == itVal->second.type && itVal->second.lints.size() >0)
		return itVal->second.lints[0];

	return 0;
}

::Ice::Long ProvisionSessImpl::getBandwidth(const ::Ice::Current& c) const
{
	RLock sync(*this);

	return _getBandwidth();
}

::TianShanIce::ContentProvision::ProvisionSubscribeMask ProvisionSessImpl::getSubscribeMask(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return subMask;
}

void ProvisionSessImpl::setSubscribeMask(const ::TianShanIce::ContentProvision::ProvisionSubscribeMask& mask, const ::Ice::Current& c)
{
	WLock sync(*this);
	subMask = mask;
}


void ProvisionSessImpl::updateProgress(::Ice::Long processed, ::Ice::Long total, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
	WLock sync(*this);
	_progressProcessed = processed;
	_progressTotal = total;
	
	std::string ownerPrxStr;
	try {
		if (!owner || 0 == subMask.psmInProgress)
			return;

		struct timeb stampNow;
		ftime(&stampNow);
		if (1 == subMask.psmInProgress)
		{
			//if processed == total, maybe the 100%
			if (processed != total)
			{
				::Ice::Long stepped = processed - _progressLatest;

				// ignore the progess that less than 25% AND has interval shorter than _dwMinProgressInterval
				if(stampNow.time >= _progressLatestStamp.time && (stampNow.time - _progressLatestStamp.time)*1000 + (stampNow.millitm - _progressLatestStamp.millitm)  < _gCPECfg._dwMinProgressInterval)
				{
					if (total <= 0)
						return;

					if (_gCPECfg._dwMaxPecentageStep > 0 && _gCPECfg._dwMaxPecentageStep < 100)
					{
						if (stepped * 100/(_gCPECfg._dwMaxPecentageStep) < total)
							return;
					}
					else
						return;

					if (stampNow.time - _progressLatestStamp.time<10)  //ignore if no more than 10 seconds
						return;
				}
			}

			_progressLatest = processed;
			_progressLatestStamp.time = stampNow.time;
			_progressLatestStamp.millitm = stampNow.millitm;
		}
		
		ownerPrxStr = _env._communicator->proxyToString(owner);
		TianShanIce::ContentProvision::ProvisionSessionBindPrx prx =
			TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(owner->ice_collocationOptimized(false));
		
		envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("notify progress to the owner[%s]: %lld / %lld"),
			ownerPrxStr.c_str(), processed, total);	
		
		Ice::Long timeStampNow = ZQTianShan::now();
#if ICE_INT_VERSION / 100 >= 306
		OnProvisionStateCBPtr onProStateCbPtr = new OnProvisionStateCB(_env);
		Ice::CallbackPtr genericCB = Ice::newCallback(onProStateCbPtr, &OnProvisionStateCB::OnProvisionProgress);
		prx->begin_OnProvisionProgress(contentKey, timeStampNow, processed, total, params , genericCB);
#else
		prx->OnProvisionProgress_async(new OnProvisionProgressAmiCBImpl(_env), contentKey, timeStampNow, processed, total, params);
#endif
	}
	catch(const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("updateProgress() error occurs when nodify owner[%s], exception[%s]"), ownerPrxStr.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("updateProgress() error occurs when nodify owner[%s], unknown exception"), ownerPrxStr.c_str());
	}
}

void ProvisionSessImpl::OnTimer(const ::Ice::Current& c)
{
	WLock sync(*this);
	switch(state)
	{
	case ::TianShanIce::ContentProvision::cpsCreated:
		ProvStateCreated(_env, *this).OnTimer(c);
		break;

	case ::TianShanIce::ContentProvision::cpsAccepted:
		ProvStateAccepted(_env, *this).OnTimer(c); 
		break;

	case ::TianShanIce::ContentProvision::cpsWait:
		ProvStateWait(_env, *this).OnTimer(c); 
		break;

	case ::TianShanIce::ContentProvision::cpsReady:
		ProvStateReady(_env, *this).OnTimer(c); 
		break;

	case ::TianShanIce::ContentProvision::cpsProvisioning:
		ProvStateProvisioning(_env, *this).OnTimer(c); 
		break;

	case ::TianShanIce::ContentProvision::cpsStopped:
		ProvStateStopped(_env, *this).OnTimer(c); 
		break;

	}

	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("OnTimer() executed"));
}

void ProvisionSessImpl::forceToStart(const ::Ice::Current& c)
{
	WLock sync(*this);
	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("forceToStart() entering"));

	switch(state)
	{
	case ::TianShanIce::ContentProvision::cpsCreated:
		ProvStateAccepted(_env, *this).enter(); // no break statement here
	case ::TianShanIce::ContentProvision::cpsAccepted:
		scheduledStart = now();
		ProvStateWait(_env, *this).enter(); // no break statement here
	case ::TianShanIce::ContentProvision::cpsWait:
		ProvStateReady(_env, *this).enter(); // no break statement here
	case ::TianShanIce::ContentProvision::cpsReady:
		ProvStateProvisioning(_env, *this).enter();
		break;

	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVISIONSESSEXPFMT(405, "commit() session already committed"));
	}

	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("forceToStart() executed"));
}


void ProvisionSessImpl::setSessionType(::TianShanIce::ContentProvision::ProvisionType ptype, ::TianShanIce::ContentProvision::StartType stype, const ::Ice::Current& c)
{
	WLock(*this);

	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("setSessionType() entering"));

	// this entry is only allowed to be called from CPH during validateSetup()
	if (::TianShanIce::ContentProvision::cpsCreated != state)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVISIONSESSEXPFMT(1105, "setSessionType() not allowed in this state"));

	provType = ptype;
	stType = stype;

	envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("setSessionType() type=%s, startType=%s"), typeStr(provType), startTypeStr(stType));
}

void ProvisionSessImpl::notifyStarted(const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
	WLock sync(*this);		

	std::string ownerPrxStr;
	try {
		if (!owner || !subMask.psmStarted)
			return;

		char buf[32];
		::Ice::Long stampNow = ZQTianShan::now();

		int  bitrate = (int)resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData[CPHPM_BANDWIDTH].lints[0];

		ownerPrxStr = _env._communicator->proxyToString(owner);

		TianShanIce::ContentProvision::ProvisionSessionBindPrx prx = 
			TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(owner->ice_collocationOptimized(false));
		
		envlog(ZQ::common::Log::L_NOTICE, PROVISIONSESSLOGFMT("notify started to the owner[%s], stampNow[%s], content[%s|%s], sourceUrl[%s], transferBitrate[%d]"),
			ownerPrxStr.c_str(),ZQTianShan::TimeToUTC(stampNow,buf,sizeof(buf)-2),contentKey.content.c_str(),contentKey.contentStoreNetId.c_str(),resources[::TianShanIce::SRM::rtURI].resourceData[CPHPM_SOURCEURL].strs[0].c_str(),bitrate);	
#if ICE_INT_VERSION / 100 >= 306
		OnProvisionStateCBPtr onProStateCbPtr = new OnProvisionStateCB(_env);
		Ice::CallbackPtr genericCB = Ice::newCallback(onProStateCbPtr, &OnProvisionStateCB::OnProvisionStarted);
		prx->begin_OnProvisionStarted(contentKey, stampNow, params, genericCB);
#else		
		prx->OnProvisionStarted_async(new OnProvisionStartedAmiCBImpl(_env), contentKey, stampNow, params);
#endif
	}
	catch(const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("notifyStarted() error occurs when nodify owner[%s], exception[%s]"), ownerPrxStr.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("notifyStarted() error occurs when nodify owner[%s], unknown exception"), ownerPrxStr.c_str());
	}

	// this entry is only allowed to be called while entering cpsProvisioning
	switch(state)
	{
	case ::TianShanIce::ContentProvision::cpsReady:
	case ::TianShanIce::ContentProvision::cpsProvisioning:
		envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("notifyStarted() provision execution started in the helper"));
		break;
	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVISIONSESSEXPFMT(1105, "setSessionType() not allowed in this state"));
	}	
}

bool ProvisionSessImpl::_isProvisionCanceled()
{
	::TianShanIce::Properties propers = _getProperties();
	::TianShanIce::Properties::const_iterator it = propers.find(PROPTY_PROVISIONCANCELLED);
	if (it!=propers.end())
	{
		return (atoi(it->second.c_str())>0);
	}

	return false;
}

void ProvisionSessImpl::_setProvisionCanceled()
{
	_setProperty(PROPTY_PROVISIONCANCELLED, "1");
}

void ProvisionSessImpl::notifyStopped(bool errorOccurred, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
	std::string errmsg, errcode;

	{
		WLock sync(*this);		

		if (owner && subMask.psmStopped && !_isProvisionCanceled())
		{
			std::string ownerPrxStr;
			try 
			{
				::Ice::Long stampNow = ZQTianShan::now();

				ownerPrxStr = _env._communicator->proxyToString(owner);
				TianShanIce::ContentProvision::ProvisionSessionBindPrx prx =
					TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(owner->ice_collocationOptimized(false));

				envlog(ZQ::common::Log::L_NOTICE, PROVISIONSESSLOGFMT("notify stopped to the owner[%s]"),
					ownerPrxStr.c_str());	
#if ICE_INT_VERSION / 100 >= 306
				OnProvisionStateCBPtr onProStateCbPtr = new OnProvisionStateCB(_env);
				Ice::CallbackPtr genericCB = Ice::newCallback(onProStateCbPtr, &OnProvisionStateCB::OnProvisionStopped);
				prx->begin_OnProvisionStopped( contentKey, stampNow, errorOccurred, params, genericCB);
#else	
				prx->OnProvisionStopped_async(new OnProvisionStoppedAmiCBImpl(_env), contentKey, stampNow, errorOccurred, params);
#endif
			}
			catch(const Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("notifyStopped() error occurs when nodify owner[%s], exception[%s]"), ownerPrxStr.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("notifyStopped() error occurs when nodify owner[%s], unknown exception"), ownerPrxStr.c_str());
			}
		}
		else
		{
			envlog(ZQ::common::Log::L_INFO, PROVISIONSESSLOGFMT("notifyStopped() OnProvisionStopped event skipped"));
		}

		const char* szStatus;
		if (errorOccurred)
		{
			_bSucc = false;	
			szStatus = "failure";

			::TianShanIce::Properties::const_iterator it = params.find(EVTPM_ERRORMESSAGE);
			if (it!=params.end())
				errmsg = it->second;
			
			it = params.find(EVTPM_ERRORCODE);
			if (it!=params.end())
				errcode = it->second;
		}
		else
		{
			_bSucc = true;
			szStatus = "success";
		}

		envlog(ZQ::common::Log::L_NOTICE, PROVISIONSESSLOGFMT("notifyStopped() provision stopped, status[%s], error[%s], code[%s]"),
		 szStatus, errmsg.c_str(), errcode.c_str());		

		ProvStateStopped(_env, *this).enter();
	}

	_env.processProvisionError(!errorOccurred, errmsg, errcode);
}

void ProvisionSessImpl::notifyStreamable(bool streamable, const ::Ice::Current& c)
{
	WLock sync(*this);		

	this->streamable = streamable;

	std::string ownerPrxStr;
	try {
		if (!owner || !subMask.psmStreamable)
			return;
		
		::Ice::Long stampNow = ZQTianShan::now();

		ownerPrxStr = _env._communicator->proxyToString(owner);
		TianShanIce::ContentProvision::ProvisionSessionBindPrx prx =
			TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(owner->ice_collocationOptimized(false));

		static const ::TianShanIce::Properties params;
		envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("notify streamable to the owner[%s]: %s"),
			ownerPrxStr.c_str(), (streamable? "true":"false"));	
#if ICE_INT_VERSION / 100 >= 306
		OnProvisionStateCBPtr onProStateCbPtr = new OnProvisionStateCB(_env);
		Ice::CallbackPtr genericCB = Ice::newCallback(onProStateCbPtr, &OnProvisionStateCB::OnProvisionStreamable);
		prx->begin_OnProvisionStreamable( contentKey, stampNow, streamable, params, genericCB);
#else	
		prx->OnProvisionStreamable_async(new OnProvisionStreamableAmiCBImpl(_env), contentKey, stampNow, streamable, params);
#endif
	}
	catch(const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("notifyStreamable() error occurs when nodify owner[%s], exception[%s]"), ownerPrxStr.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVISIONSESSLOGFMT("notifyStreamable() error occurs when nodify owner[%s], unknown exception"), ownerPrxStr.c_str());
	}
}

void ProvisionSessImpl::OnRestore(const ::Ice::Current& c)
{
	WLock sync(*this);		

	switch(state)
	{
	case ::TianShanIce::ContentProvision::cpsCreated:
		ProvStateCreated(_env, *this).OnRestore(c); break;
	case ::TianShanIce::ContentProvision::cpsAccepted:
		ProvStateAccepted(_env, *this).OnRestore(c); break;
	case ::TianShanIce::ContentProvision::cpsWait:
		ProvStateWait(_env, *this).OnRestore(c); break;
	case ::TianShanIce::ContentProvision::cpsReady:
		ProvStateReady(_env, *this).OnRestore(c); break;
	case ::TianShanIce::ContentProvision::cpsProvisioning:
		ProvStateProvisioning(_env, *this).OnRestore(c); break;
	case ::TianShanIce::ContentProvision::cpsStopped:
		ProvStateStopped(_env, *this).OnRestore(c); break;
	}

	envlog(ZQ::common::Log::L_DEBUG, PROVISIONSESSLOGFMT("OnRestore() executed"));
}

void ProvisionSessImpl::notifyError(int nErrCode, const char* szErrMsg)
{
	_bSucc = false;
	char tmp[64];
	sprintf(tmp, "%d", nErrCode);

	::TianShanIce::Properties params;
	params[EVTPM_ERRORCODE] = tmp;
	params[EVTPM_ERRORMESSAGE] = (szErrMsg?szErrMsg:"");

	notifyStopped(true, params, Ice::Current());
}
#if  ICE_INT_VERSION / 100 >= 306

OnProvisionStateCB::OnProvisionStateCB(CPEEnv& env): _env(env)
{

}

void OnProvisionStateCB::handleException(const std::string& name, const Ice::Exception& ex)
{
   //_env.logProvisionSessionBindAmiCBException(name, ex);
}

void OnProvisionStateCB::OnProvisionStarted(const Ice::AsyncResultPtr& r)	
{
	TianShanIce::ContentProvision::ProvisionSessionBindPrx provBindProxy = TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(r->getProxy());
	try
	{
		provBindProxy->end_OnProvisionStarted(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException("OnProvisionStarted",ex);
	}
}
void OnProvisionStateCB::OnProvisionStopped(const Ice::AsyncResultPtr& r)
{
	TianShanIce::ContentProvision::ProvisionSessionBindPrx provBindProxy = TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(r->getProxy());
	try
	{
		 provBindProxy->end_OnProvisionStopped(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException("OnProvisionStopped",ex);
	}
}
void OnProvisionStateCB::OnProvisionProgress(const Ice::AsyncResultPtr& r)
{
	TianShanIce::ContentProvision::ProvisionSessionBindPrx provBindProxy = TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(r->getProxy());
	try
	{
		provBindProxy->end_OnProvisionProgress(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException("OnProvisionProgress",ex);
	}
}
void OnProvisionStateCB::OnProvisionStateChanged(const Ice::AsyncResultPtr& r)
{
	TianShanIce::ContentProvision::ProvisionSessionBindPrx provBindProxy = TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(r->getProxy());
	try
	{
		 provBindProxy->end_OnProvisionStateChanged(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException("OnProvisionStateChanged",ex);
	}
}
void OnProvisionStateCB::OnProvisionStreamable(const Ice::AsyncResultPtr& r)
{
	TianShanIce::ContentProvision::ProvisionSessionBindPrx provBindProxy = TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(r->getProxy());
	try
	{
		provBindProxy->end_OnProvisionStreamable(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException("OnProvisionStreamable",ex);
	}
}
void OnProvisionStateCB::OnProvisionDestroyed(const Ice::AsyncResultPtr& r)
{
	TianShanIce::ContentProvision::ProvisionSessionBindPrx provBindProxy = TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(r->getProxy());
	try
	{
		provBindProxy->end_OnProvisionDestroyed(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException("OnProvisionDestroyed",ex);
	}
}

#else
// -----------------------------
// callback OnProvisionStartedAmiCBImpl
// -----------------------------
OnProvisionStartedAmiCBImpl::OnProvisionStartedAmiCBImpl(CPEEnv& env)
:_env(env)
{
}

void OnProvisionStartedAmiCBImpl::ice_exception(const ::Ice::Exception& ex)
{
	_env.logProvisionSessionBindAmiCBException("OnProvisionStarted", ex);
}

// -----------------------------
// callback OnProvisionProgressAmiCBImpl
// -----------------------------
OnProvisionStoppedAmiCBImpl::OnProvisionStoppedAmiCBImpl(CPEEnv& env)
:_env(env)
{
}

void OnProvisionStoppedAmiCBImpl::ice_exception(const ::Ice::Exception& ex)
{
	_env.logProvisionSessionBindAmiCBException("OnProvisionStopped", ex);
}

// -----------------------------
// callback OnProvisionProgressAmiCBImpl
// -----------------------------
OnProvisionProgressAmiCBImpl::OnProvisionProgressAmiCBImpl(CPEEnv& env)
:_env(env)
{
}

void OnProvisionProgressAmiCBImpl::ice_exception(const ::Ice::Exception& ex)
{
	_env.logProvisionSessionBindAmiCBException("OnProvisionProgress", ex);
}

// -----------------------------
// callback OnProvisionStateChangedAmiCBImpl
// -----------------------------
OnProvisionStateChangedAmiCBImpl::OnProvisionStateChangedAmiCBImpl(CPEEnv& env)
:_env(env)
{
}

void OnProvisionStateChangedAmiCBImpl::ice_exception(const ::Ice::Exception& ex)
{
	_env.logProvisionSessionBindAmiCBException("OnProvisionStateChanged", ex);
}

// -----------------------------
// callback OnProvisionStreamableAmiCBImpl
// -----------------------------
OnProvisionStreamableAmiCBImpl::OnProvisionStreamableAmiCBImpl(CPEEnv& env)
:_env(env)
{
}

void OnProvisionStreamableAmiCBImpl::ice_exception(const ::Ice::Exception& ex)
{
	_env.logProvisionSessionBindAmiCBException("OnProvisionStreamable", ex);
}

// -----------------------------
// callback OnProvisionDestroyedAmiCBImpl
// -----------------------------
OnProvisionDestroyedAmiCBImpl::OnProvisionDestroyedAmiCBImpl(CPEEnv& env)
:_env(env)
{
}

void OnProvisionDestroyedAmiCBImpl::ice_exception(const ::Ice::Exception& ex)
{
	_env.logProvisionSessionBindAmiCBException("OnProvisionDestroyed", ex);
}
#endif
}} // namespace
