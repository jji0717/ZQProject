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


#ifndef _ZQTianShan_CPCIMPL_H_
#define _ZQTianShan_CPCIMPL_H_

#include "TsContentProv.h"

class CPCImpl;
class ContentStoreImpl;

class D4Speaker;

using namespace ::TianShanIce::ContentProvision;

class CPCImpl:public TianShanIce::ContentProvision::ContentProvisionCluster, ZQ::common::NativeThread
{
public:
	typedef ::IceInternal::Handle<CPCImpl> Ptr;
	
	struct CPEInst
	{
		::Ice::Long stampLastChange;
		::TianShanIce::ContentProvision::ContentProvisionServicePrx		cpePrx;
		::Ice::Long stampLastReport;
		::TianShanIce::ContentProvision::MethodInfos	methodInfos;
		::TianShanIce::ContentProvision::ExportMethods	exportMethods;
	};
	
	struct ResItem
	{
		TianShanIce::SRM::ResourceType	resType;
		TianShanIce::ValueMap			resVars;
	};
	typedef std::vector<ResItem>	ResItemSet;
	
	CPCImpl(ZQ::common::Log& log);
	virtual ~CPCImpl();

	virtual std::string getAdminUri(const ::Ice::Current& ) { return ""; }
	virtual TianShanIce::State getState(const ::Ice::Current&) { return TianShanIce::stOutOfService; }

	bool init(Ice::CommunicatorPtr& ic, int nRegisterInterval, D4Speaker* d4Speaker=NULL);

	void uninit();

	typedef std::vector<float> TrickSpeeds;

	void setTrickSpeeds(const TrickSpeeds& trickSpeeds);
	void setNoTrickSpeedFileRegex(bool enable, const TianShanIce::StrValues& fileRegexs );
	
    virtual void reportEngine_async(const ::TianShanIce::ContentProvision::AMD_ContentProvisionCluster_reportEnginePtr&, const ::std::string&, const ::TianShanIce::ContentProvision::ContentProvisionServicePrx&, ::Ice::Long, const ::Ice::Current& = ::Ice::Current());
	
    virtual ::TianShanIce::ContentProvision::ProvisionTaskPrx openTask(const ::std::string&, bool, const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::ContentProvision::CPEInsts listRegisteredCPE(const ::Ice::Current& = ::Ice::Current()) const ;


	std::string getExposeUrl(const std::string& protocal, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, int transferBitrate, int& nTTL, int& permittedBitrate);


	TianShanIce::ContentProvision::ProvisionSessionPrx provision(const std::string& methodType,
		const std::string& sourceUrl,
		TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
		const ::std::string& filePathName,
		const std::string& timestart,
		const std::string& timestop,
		unsigned int		bandwidth,
		TianShanIce::ContentProvision::ProvisionSessionBindPrx provEvtSink,
		::TianShanIce::Storage::ContentPrx	contentPrx,
		::TianShanIce::Properties& prop,
		bool bAudioFlag = false);

	TianShanIce::ContentProvision::ProvisionSessionPrx provision_NasRTI(const std::string& sourceUrl,
		const std::string& outputUrl,
		TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
		const ::std::string& filePathName,
		const std::string& timestart,
		const std::string& timestop,
		unsigned int		bandwidth,
		TianShanIce::ContentProvision::ProvisionSessionBindPrx provEvtSink,
		::TianShanIce::Storage::ContentPrx	contentPrx);


protected:

	TianShanIce::ContentProvision::ProvisionSessionPrx createProvision(const std::string& methodType, 
		TianShanIce::ContentProvision::ProvisionOwnerType nProvisionOwnerType,
		TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
		const std::string& timestart,
		const std::string& timestop,
		unsigned int		bandwidth,
		const ResItemSet&	resItems,
		TianShanIce::ContentProvision::ProvisionSessionBindPrx provEvtSink,
		::TianShanIce::Storage::ContentPrx	contentPrx,
		::TianShanIce::Properties& prop);

protected:

	bool queryCPEInfo(const ::TianShanIce::ContentProvision::ContentProvisionServicePrx& prx, CPEInst& inst, const ::std::string& netId);
	void checkInstance();

	TianShanIce::ContentProvision::ProvisionSessionPrx createCPEProvision(::TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx,
		const std::string& methodType, 
		TianShanIce::ContentProvision::ProvisionOwnerType nProvisionOwnerType,
		TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
		const std::string& timestart,
		const std::string& timestop,
		const ResItemSet&	resItems,
		TianShanIce::ContentProvision::ProvisionSessionBindPrx provEvtSink,
		::TianShanIce::Storage::ContentPrx	contentPrx,
		::TianShanIce::Properties& prop);

	int run();
	
	TianShanIce::ContentProvision::ProvisionSessionBindPrx _cpeEventBindPrx;
	
	ZQ::common::Mutex	_lock;
	typedef std::map<std::string, CPEInst>  CPEMAP;
	CPEMAP	_cpes;

#ifdef ZQ_OS_MSWIN
	HANDLE _stopEvent;
#else
    sem_t _stopEvent;
#endif

	bool								_bStop;
	uint32								_dwInstanceLeaseTermSecs;
	Ice::CommunicatorPtr				_ic;

	TrickSpeeds							_trickSpeeds;
	uint32								_dwRegisterInterval;	

	ZQ::common::Log&					_log;

	D4Speaker*                          _d4Speaker;

	TianShanIce::StrValues				_fileRegexs;
	bool								_enableNoTrickSpeedFile;
};



#endif

