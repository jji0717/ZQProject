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
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
//
// ===========================================================================


#include "TianShanDefines.h"
#include "Log.h"
#include "Locks.h"
#include "CPCImpl.h"
#include <vector>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include "CPHInc.h"
#include <algorithm>
#include "ContentImpl.h"
#include "SystemUtils.h"
#include "D4Update.h"

#undef max
#include <boost/regex.hpp>

using namespace std;
using namespace ZQ::common;

#define CPC			"CPC"
#define MOLOG					(_log)

#ifdef ZQ_OS_LINUX
#define stricmp strcasecmp
#endif


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//
CPCImpl::CPCImpl(ZQ::common::Log& log)
	:_log(log)
{
	_dwInstanceLeaseTermSecs = 8;

#ifdef ZQ_OS_MSWIN
	_stopEvent = NULL;
#endif

	//default 7.5
	_trickSpeeds.push_back(7.5);

	_enableNoTrickSpeedFile = false;
	_fileRegexs.clear();
}

CPCImpl::~CPCImpl()
{
//	uninit();
}


TianShanIce::ContentProvision::ProvisionSessionPrx CPCImpl::createCPEProvision(::TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx,
								 const std::string& methodType, 
								 TianShanIce::ContentProvision::ProvisionOwnerType nProvisionOwnerType,
								 TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
								 const std::string& timestart,
								 const std::string& timestop,
								 const ResItemSet&	resItems,
								 TianShanIce::ContentProvision::ProvisionSessionBindPrx provEvtSink,
								 ::TianShanIce::Storage::ContentPrx	contentPrx,
								 ::TianShanIce::Properties& prop)
{
	unsigned long dwTimeStart = SYS::getTickCount();
	TianShanIce::ContentProvision::ProvisionSessionPrx sessPrx;
	std::string strContentProvisionSvc = _ic->proxyToString(cpePrx);
	std::string strCPENetId;

	try
	{
		strCPENetId = cpePrx->getNetId();

		sessPrx = cpePrx->createSession(contentKey, methodType, nProvisionOwnerType, contentPrx, provEvtSink);
		MOLOG(Log::L_INFO, CLOGFMT(CPC, "[%s|%s|%s] createSession successful"), 
			contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str());	
		
		::TianShanIce::ContentProvision::ProvisionSubscribeMask mask = {1, false, false, true, true, true};
		sessPrx->setSubscribeMask(mask);
		sessPrx->setTrickSpeedCollection(_trickSpeeds);

		for (::TianShanIce::Properties::const_iterator it = prop.begin(); it != prop.end(); it++)
		{
			sessPrx->setProperty(it->first, it->second);
			MOLOG(Log::L_INFO, CLOGFMT(CPC, "[%s|%s|%s] setProperty[%s|%s]"), 
				contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(),it->first.c_str(),it->second.c_str());
		}

		MOLOG(Log::L_INFO, CLOGFMT(CPC, "[%s|%s|%s] resource added"), 
			contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str());	

		for(ResItemSet::const_iterator it=resItems.begin();it!=resItems.end();it++)
		{
			sessPrx->addResource(it->resType, it->resVars);
		}
		MOLOG(Log::L_INFO, CLOGFMT(CPC, "[%s|%s|%s] all session resource added"), 
			contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str());	
				
		sessPrx->setup(timestart, timestop);
		MOLOG(Log::L_INFO, CLOGFMT(CPC, "[%s|%s|%s] session setup successful"), 
			contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str());	
		sessPrx->commit();	
	}
	catch(const::TianShanIce::InvalidParameter& ex)
	{	
		MOLOG(Log::L_ERROR, CLOGFMT(CPC, "Failed to createSession on CPE[%s], caught InvalidParameter exception: %s"), 
			strContentProvisionSvc.c_str(), ex.message.c_str());	
		ex.ice_throw();
	}
	catch(const::TianShanIce::ServerError& ex)
	{	
		MOLOG(Log::L_ERROR, CLOGFMT(CPC, "Failed to createSession on CPE[%s], caught ServerError exception: %s"), 
			strContentProvisionSvc.c_str(), ex.message.c_str());	
		ex.ice_throw();
	}
	catch(const::TianShanIce::ContentProvision::OutOfResource& ex)
	{	
		MOLOG(Log::L_ERROR, CLOGFMT(CPC, "Failed to createSession on CPE[%s], caught OutOfResource exception: %s"), 
			strContentProvisionSvc.c_str(), ex.message.c_str());	

		ZQTianShan::_IceThrow<TianShanIce::Storage::NoResourceException>(CPC, 0, "out of resource");
	}
	catch(const Ice::AlreadyRegisteredException&)
	{
		//
		//session already exist, wait to be destroyed
		//
		MOLOG(Log::L_ERROR, CLOGFMT(CPC, "Failed to createSession on CPE[%s], session already exist"), 
			strContentProvisionSvc.c_str());	
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(CPC, 0, "provision session already exist");
	}
	catch(const Ice::Exception& ex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPC, "Failed to createSession on CPE[%s], caught ice exception: %s"), 
			strContentProvisionSvc.c_str(), ex.ice_name().c_str());	
		ex.ice_throw();		
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPC, "[%s|%s|%s] createSession on CPE[%s] with method type [%s] successful, spent[%d]ms"), 
		contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(),
		strContentProvisionSvc.c_str(), methodType.c_str(), SYS::getTickCount()-dwTimeStart);					

	prop["sys.CPE.NetId"] = strCPENetId;
	return sessPrx;
}

TianShanIce::ContentProvision::ProvisionSessionPrx CPCImpl::createProvision(const std::string& methodType, 
							  TianShanIce::ContentProvision::ProvisionOwnerType nProvisionOwnerType,
							  TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
							  const std::string& timestart,
							  const std::string& timestop,
							  unsigned int		bandwidth,
							  const ResItemSet&	resItems,
							  TianShanIce::ContentProvision::ProvisionSessionBindPrx provEvtSink,
							  ::TianShanIce::Storage::ContentPrx	contentPrx,
							  ::TianShanIce::Properties& prop)
{
	TianShanIce::ContentProvision::ProvisionSessionPrx sessPrx;
	std::vector<TianShanIce::ContentProvision::ContentProvisionServicePrx>	cpeProxies;
	std::vector<TianShanIce::ContentProvision::ContentProvisionServicePrx>::iterator itc;
		
	{
		ZQ::common::Guard<ZQ::common::Mutex> op(_lock);
		
		CPEMAP::iterator it=_cpes.begin();
		for(;it != _cpes.end();it++)
		{
			::TianShanIce::ContentProvision::MethodInfos::iterator mit = it->second.methodInfos.begin();
			for(;mit!=it->second.methodInfos.end();mit++)
			{
				::TianShanIce::ContentProvision::MethodInfo& mInfo = *mit;
				if (mInfo.methodType == methodType && bandwidth/1000 <= mInfo.maxKbps && 1 <= mInfo.maxsessions)
				{				
					cpeProxies.push_back(it->second.cpePrx);
					break;
				}
			}
		}
	}	

	if (!cpeProxies.size())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPC, "[%s]failed to createProvision with error: method type %s not found"), contentKey.content.c_str(),methodType.c_str());
		ZQTianShan::_IceThrow<TianShanIce::Storage::NoResourceException>(CPC, 0, "[%s]No CPE available for the provision", contentKey.content.c_str());		
	}

	if (cpeProxies.size()>1)
	{
		std::random_shuffle(cpeProxies.begin(), cpeProxies.end());
	}

	for(itc=cpeProxies.begin();itc!=cpeProxies.end();itc++)
	{		
		try
		{
			 sessPrx = createCPEProvision((*itc), 
				methodType, 
				nProvisionOwnerType,
				contentKey,
				timestart,
				timestop,
				resItems,
				provEvtSink,
				contentPrx,
				prop);

			return sessPrx;
		}
		catch(const::TianShanIce::InvalidParameter& ex)
		{
			ex.ice_throw();
		}
		catch(Ice::Exception& ex)
		{
			//last one
			if (itc+1==cpeProxies.end())
			{
				ex.ice_throw();
			}
		}
	}
	
	//should not go here
	return sessPrx;
}

bool CPCImpl::queryCPEInfo(const ::TianShanIce::ContentProvision::ContentProvisionServicePrx& prx, CPCImpl::CPEInst& inst, const ::std::string& netId)
{
	try
	{
		inst.methodInfos = prx->listMethods();
		inst.exportMethods = prx->listExportMethods();
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(CPC, "failed to listMethods(), caught exception[%s]"), ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(CPC, "failed to listMethods(), caught unknown exception"));
		return false;
	}

	//print log


 	return true;
}

#ifdef ZQ_OS_MSWIN
int CPCImpl::run()
{
	MOLOG(Log::L_DEBUG, CLOGFMT(CPC, "CPC thread enter, thread id [%04x]"), GetCurrentThreadId());
	
	int	nTimeWait;
	DWORD dwTimeLast = GetTickCount();
	int nErrorOutputCount = 0;	
	
	while(!_bStop)
	{
		dwTimeLast = GetTickCount();
		
		checkInstance();
		
		nTimeWait = _dwInstanceLeaseTermSecs * 1000;		
		
		DWORD dwRet = WaitForSingleObject(_stopEvent, nTimeWait);
		if (dwRet == WAIT_OBJECT_0)
		{
			// exist the thread
			break;
		}
		else if (dwRet != WAIT_TIMEOUT)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPC, "WaitForSingleObject(_stopEvent) failed with code %d"), dwRet);
			break;
		}
	}
	
	MOLOG(Log::L_DEBUG, CLOGFMT(CPC, "CPC thread leave"));
	
	return 0;
}
#else
int CPCImpl::run() {
//	MOLOG(Log::L_DEBUG, CLOGFMT(CPC, "CPC thread enter, thread id [%04x]"), pthread_self());
	//MOLOG(Log::L_DEBUG, "CPC thread enter, thread id [%d]", pthread_self());
	
	struct timespec	nTimeWait;
	
	while(!_bStop) {
		
        if(clock_gettime(CLOCK_REALTIME, &nTimeWait) == (-1)) {
			MOLOG(Log::L_ERROR, CLOGFMT(CPC, "failed to get real clock %s"), strerror(errno));
            nTimeWait.tv_sec = time(0);
            nTimeWait.tv_nsec = 0;
        }

		checkInstance();
		
		nTimeWait.tv_sec += _dwInstanceLeaseTermSecs;		
		
		int res = 0; 
        while((res = sem_timedwait(&_stopEvent, &nTimeWait)) == (-1) && errno == EINTR) {
            continue;
        }

		if (res == (-1)) {
            if(errno != ETIMEDOUT){
                MOLOG(Log::L_ERROR, CLOGFMT(CPC, "failed to wait for stop event %s"), strerror(errno));
                _bStop = true;
            }
		}
		else {
			MOLOG(Log::L_ERROR, CLOGFMT(CPC, "somebody requesting a stop"));
            _bStop = true;
		}
	}
	
	MOLOG(Log::L_DEBUG, CLOGFMT(CPC, "CPC thread leave"));
	
	return 0;
}
#endif

void CPCImpl::checkInstance()
{
	//
	// remove those instances after it has not reported for 3* leaseTerm
	// 
	::Ice::Long llNow = ZQTianShan::now();

	{
		ZQ::common::Guard<ZQ::common::Mutex> op(_lock);

		vector<string> listToDelete;	//netId list to remove.
		vector<string>::iterator sit;
		
		CPEMAP::iterator it=_cpes.begin();
		for(;it != _cpes.end();it++)
		{
			::Ice::Long diff = llNow - it->second.stampLastReport;
			if (diff > _dwRegisterInterval * 3 )
			{
				listToDelete.push_back(it->first);
				MOLOG(Log::L_WARNING, CLOGFMT(CPC, "CPE: [%s] has not been updated for %lld ms, to remove from CPE list"), 
					it->first.c_str(), diff);
			}
		}			
		
		for(sit=listToDelete.begin();sit<listToDelete.end();sit++)
		{
			it = _cpes.find(*sit);
			
			if (it == _cpes.end())
			{
				continue;
			}
			
			// this CPE instance have down
			MOLOG(Log::L_INFO, CLOGFMT(CPC, "CPE: [%s] has lost connection, removed from CPE list"), 
				it->first.c_str());
			
			_cpes.erase(it);
		}
	}
}

void CPCImpl::reportEngine_async(const ::TianShanIce::ContentProvision::AMD_ContentProvisionCluster_reportEnginePtr& amdResp, const ::std::string& netId, const ::TianShanIce::ContentProvision::ContentProvisionServicePrx& cpe, ::Ice::Long stampLastChange, const ::Ice::Current& ic)
{
	{
		std::string strCPEPrx = _ic->proxyToString(cpe);

		ZQ::common::Guard<ZQ::common::Mutex> op(_lock);

		CPEMAP::iterator it = _cpes.find(netId);
		if (it!=_cpes.end())
		{
			if (it->second.stampLastChange != stampLastChange)
			{
				// do query
				MOLOG(Log::L_INFO, CLOGFMT(CPC, "CPE: [%s][%s] time stamp changed, to update info"), netId.c_str(), strCPEPrx.c_str());

				if (queryCPEInfo(cpe, it->second, netId))
				{
					it->second.cpePrx = cpe;
					it->second.stampLastChange = stampLastChange;
					MOLOG(Log::L_INFO, CLOGFMT(CPC, "CPE: [%s][%s] info updated"), netId.c_str(), strCPEPrx.c_str());
				}
				else
				{
					MOLOG(Log::L_WARNING, CLOGFMT(CPC, "CPE: [%s][%s] failed to update info, retry on next"), netId.c_str(), strCPEPrx.c_str());
				}				
			}

			it->second.stampLastReport = ZQTianShan::now();
		}
		else
		{
			CPEInst inst;

			MOLOG(Log::L_INFO, CLOGFMT(CPC, "CPE: [%s][%s] discovered"), netId.c_str(), strCPEPrx.c_str());
			if (queryCPEInfo(cpe, inst, netId))
			{
				inst.cpePrx = cpe;
				inst.stampLastChange = stampLastChange;
				inst.stampLastReport = ZQTianShan::now();
				MOLOG(Log::L_INFO, CLOGFMT(CPC, "CPE: [%s][%s] info updated"), netId.c_str(), strCPEPrx.c_str());
			}
			else
			{
				inst.stampLastReport = 0;
				MOLOG(Log::L_WARNING, CLOGFMT(CPC, "CPE: [%s][%s] failed to update info, retry on next"), netId.c_str(), strCPEPrx.c_str());
			}
			_cpes.insert(std::pair<std::string, CPEInst>(netId, inst));
		}

		if (_d4Speaker)
			_d4Speaker->onSpigotStateChange(_cpes,true);
	}

	amdResp->ice_response( _dwRegisterInterval /1000);
}

::TianShanIce::ContentProvision::CPEInsts CPCImpl::listRegisteredCPE(const ::Ice::Current& ic) const
{
	ZQ::common::Guard<const ZQ::common::Mutex> op(_lock);

	::TianShanIce::ContentProvision::CPEInsts cpeInsts;
	CPEMAP::const_iterator it = _cpes.begin();
	for(;it!=_cpes.end();it++)
	{
		cpeInsts.push_back(it->second.cpePrx);
	}

	return cpeInsts;
}

::TianShanIce::ContentProvision::ProvisionTaskPrx CPCImpl::openTask(const ::std::string&, bool, const ::Ice::Current& ic)
{
	TianShanIce::NotImplemented ex;
	throw ex;

	return NULL;
}

bool CPCImpl::init(Ice::CommunicatorPtr& ic, int nRegisterInterval,  D4Speaker* d4Speaker)
{
	_dwRegisterInterval = nRegisterInterval;
	_d4Speaker = d4Speaker;


	try 
	{	
		_ic = ic;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPC, "caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPC, "caught exception[%s]"), ex.ice_name().c_str());
		return false;
	}

#ifdef ZQ_OS_MSWIN
	_stopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    if(sem_init(&_stopEvent, 0, 0) == (-1)) {
		MOLOG(Log::L_INFO, CLOGFMT(CPC, "failed to init stop event [%s]"), strerror(errno));
        return false;
    }
#endif
	_bStop = false;
	start();

	if (_d4Speaker)
	{

		::TianShanIce::ContentProvision::CPEInsts cpeinsts = listRegisteredCPE();
		CPEMAP tempCPEM;
		for (::TianShanIce::ContentProvision::CPEInsts::iterator iter = cpeinsts.begin(); iter!= cpeinsts.end(); iter++)
		{
			CPEInst cpeinst;
			queryCPEInfo((*iter),cpeinst,(*iter).get()->getNetId());
			tempCPEM.insert(std::pair<std::string, CPEInst>((*iter).get()->getNetId(), cpeinst));	
		}

		if (!tempCPEM.empty())
			_d4Speaker->onSpigotStateChange(tempCPEM,true);
	}

	return true;
}

void CPCImpl::uninit()
{
    MOLOG(Log::L_INFO, CLOGFMT(CPC, "uninitialize enter"));

#ifdef ZQ_OS_MSWIN
	if (_stopEvent) {
		SetEvent(_stopEvent);
		_bStop = true;
	
		waitHandle(5000);
		CloseHandle(_stopEvent);
		_stopEvent = NULL;
	}
#else
    sem_post(&_stopEvent);
    _bStop = true;

    waitHandle(5000);
    sem_destroy(&_stopEvent);
#endif

    MOLOG(Log::L_INFO, CLOGFMT(CPC, "uninitialize leave"));
}

bool regexFileName(const std::string& filename, const TianShanIce::StrValues& expressionList)
{
	bool bret = false;
	for(uint i = 0; i < expressionList.size(); ++i)
	{
		boost::regex fileRegex(expressionList[i]);

		if (boost::regex_match(filename.c_str(), fileRegex))
		{
			bret = true;
			break;
		}
	}
	return bret;
}

TianShanIce::ContentProvision::ProvisionSessionPrx CPCImpl::provision(const std::string& methodType,
															 const std::string& sourceUrl,
															 TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
															 const ::std::string& filePathName,
															 const std::string& timestart,
															 const std::string& timestop,
															 unsigned int		bandwidth,
															 TianShanIce::ContentProvision::ProvisionSessionBindPrx provEvtSink,
															 ::TianShanIce::Storage::ContentPrx	contentPrx,
															 ::TianShanIce::Properties& prop,
															 bool bAudioFlag)
{
	ResItemSet resItems;
	ResItem	resIt;
	TianShanIce::Variant	var;
	resIt.resType = ::TianShanIce::SRM::rtURI;
	var.type = TianShanIce::vtStrings;
	var.bRange = false;
	var.strs.clear();
	var.strs.push_back(filePathName);
	resIt.resVars[CPHPM_FILENAME] = var;
	var.strs.clear();
	var.strs.push_back(sourceUrl);
	resIt.resVars[CPHPM_SOURCEURL] = var;	
	resItems.push_back(resIt);
	var.strs.clear();
	char strSubtype[10];
	itoa(bAudioFlag,strSubtype,10);
	var.strs.push_back(strSubtype);
	resIt.resVars[CPHPM_SUBTYPE] = var;
	resItems.push_back(resIt);

	var.strs.clear();
	itoa(bAudioFlag,strSubtype,10);
	var.strs.push_back(strSubtype);
	resIt.resVars[CPHPM_SUBTYPE] = var;
	resItems.push_back(resIt);

	var.strs.clear();	
	resIt.resVars.clear();		
	resIt.resType = ::TianShanIce::SRM::rtProvisionBandwidth;
	var.type = TianShanIce::vtLongs;
	var.lints.push_back(bandwidth);
	resIt.resVars[CPHPM_BANDWIDTH] = var;
	resItems.push_back(resIt);

	if(_enableNoTrickSpeedFile && regexFileName(contentKey.content, _fileRegexs))
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPC, "[%s|%s|%s] No Trick Speed file Flag "), contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str());	
        prop[CPHPM_NOTRICKSPEEDS] = "1";
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPC, "[%s|%s|%s] create [%s] provision, sourceurl(%s), starttime(%s), stoptime(%s), bandwith(%d)bps,filepath(%s) "), 
		contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), methodType.c_str(), sourceUrl.c_str(),
		timestart.c_str(), timestop.c_str(),bandwidth,filePathName.c_str());	

	return createProvision(methodType, TianShanIce::ContentProvision::potDirect, contentKey, 
		timestart, timestop, bandwidth, resItems, provEvtSink, contentPrx,prop);
}

TianShanIce::ContentProvision::ProvisionSessionPrx CPCImpl::provision_NasRTI(
								const std::string& sourceUrl,
								const std::string& outputUrl,
								TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
								const ::std::string& filePathName,
								const std::string& timestart,
								const std::string& timestop,
								unsigned int		bandwidth,
								TianShanIce::ContentProvision::ProvisionSessionBindPrx provEvtSink,
								::TianShanIce::Storage::ContentPrx	contentPrx)
{
	std::string methodType = METHODTYPE_RTIVSNAS;

	ResItemSet resItems;
	ResItem	resIt;
	TianShanIce::Variant	var;
	resIt.resType = ::TianShanIce::SRM::rtURI;
	var.type = TianShanIce::vtStrings;
	var.bRange = false;
	
	var.strs.clear();
	var.strs.push_back(filePathName);
	resIt.resVars[CPHPM_FILENAME] = var;
	
	var.strs.clear();
	var.strs.push_back(sourceUrl);
	resIt.resVars[CPHPM_SOURCEURL] = var;

	var.strs.clear();
	var.strs.push_back(outputUrl);
	resIt.resVars[CPHPM_OUTPUTURL] = var;

	resItems.push_back(resIt);

	var.strs.clear();	
	resIt.resVars.clear();		
	resIt.resType = ::TianShanIce::SRM::rtProvisionBandwidth;
	var.type = TianShanIce::vtLongs;
	var.lints.push_back(bandwidth);
	resIt.resVars[CPHPM_BANDWIDTH] = var;
	resItems.push_back(resIt);

	MOLOG(Log::L_INFO, CLOGFMT(CPC, "[%s|%s|%s] create [%s] provision, sourceurl(%s), starttime(%s), stoptime(%s), bandwith(%d)bps"), 
		contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), methodType.c_str(), sourceUrl.c_str(),
		timestart.c_str(), timestop.c_str(),bandwidth);	
    ::TianShanIce::Properties prop;
	return createProvision(methodType, TianShanIce::ContentProvision::potDirect, contentKey,
		timestart, timestop, bandwidth, resItems, provEvtSink, contentPrx,prop);
}


std::string CPCImpl::getExposeUrl(const std::string& protocal, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, int transferBitrate, int& nTTL, int& permittedBitrate)
{
	std::vector<TianShanIce::ContentProvision::ContentProvisionServicePrx>	cpeProxies;
	std::vector<TianShanIce::ContentProvision::ContentProvisionServicePrx>::iterator itc;
	std::vector<std::string>	exportUrls;

	{
		ZQ::common::Guard<ZQ::common::Mutex> op(_lock);

		CPEMAP::iterator it=_cpes.begin();
		for(;it != _cpes.end();it++)
		{
			::TianShanIce::ContentProvision::ExportMethods::iterator mit = it->second.exportMethods.begin();
			for(;mit!=it->second.exportMethods.end();mit++)
			{
				::TianShanIce::ContentProvision::ExportMethod& mInfo = *mit;
				if (!stricmp(mInfo.protocal.c_str(), protocal.c_str()))
				{				
					cpeProxies.push_back(it->second.cpePrx);
					break;
				}
			}
		}
	}	

	if (!cpeProxies.size())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPC, "No CPE available for export protocal %s"), protocal.c_str());
		ZQTianShan::_IceThrow<TianShanIce::Storage::NoResourceException>(CPC, 0, "No CPE available for export");		
	}

	if (cpeProxies.size()>1)
	{
		std::random_shuffle(cpeProxies.begin(), cpeProxies.end());
	}

	std::string strUrl;
	for(itc=cpeProxies.begin();itc!=cpeProxies.end();itc++)
	{		
		::TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx = *itc;

		try
		{
			strUrl = cpePrx->getExportURL(protocal, contentkey, transferBitrate, nTTL, permittedBitrate);
			break;
		}
		catch(const::TianShanIce::InvalidStateOfArt& ex)
		{
			ex.ice_throw();
		}
		catch(Ice::Exception& ex)
		{
			//last one
			if (itc+1==cpeProxies.end())
			{
				ex.ice_throw();
			}
		}
	}

	return strUrl;
}

void CPCImpl::setTrickSpeeds( const TrickSpeeds& trickSpeeds )
{
	_trickSpeeds = trickSpeeds;
}

void CPCImpl::setNoTrickSpeedFileRegex(bool enable, const TianShanIce::StrValues& fileRegexs)
{
	_enableNoTrickSpeedFile = enable;
    _fileRegexs = fileRegexs;
}

