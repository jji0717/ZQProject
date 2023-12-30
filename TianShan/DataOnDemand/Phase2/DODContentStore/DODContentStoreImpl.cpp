// ============================================================================================
// Copyright (c) 2006, 2007 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Ken Qian
// Desc  : DODContentStore Interface implementation class
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/DODContentStoreImpl.cpp 1     10-11-12 16:05 Admin $
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/DODContentStoreImpl.cpp $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 5     10-02-02 14:22 Li.huang
// 
// 4     09-12-29 12:02 Li.huang
// 
// 3     09-06-03 15:56 Li.huang
// 
// 3     09-05-14 10:25 Build
// add method adjustProvisionSchedule()
// 
// 2     08-12-09 17:01 Li.huang
// 
// 1     08-12-08 11:11 Li.huang
// 
// 19    08-10-29 15:24 Li.huang
// 
// 18    08-10-29 10:10 Ken.qian
// compatile with new ContentStore interface
// 
// 17    07-11-27 19:42 Ken.qian
// 
// 16    07-11-23 18:06 Ken.qian
// pass dataType to request
// 
// 15    07-11-21 11:22 Ken.qian
// Support VersionNumber in TS private data packet
// 
// 14    07-11-20 13:52 Ken.qian
// Changes according to ProvisionRequest base class
// 
// 13    07-11-14 11:52 Ken.qian
// have corresponding change since request base class construction changed
// 
// 12    07-05-24 11:36 Li.huang
// 
// 11    07-05-23 18:43 Li.huang
// 
// 10    07-05-18 11:20 Li.huang
// 
// 9     07-05-17 12:15 Li.huang
// 
// 8     07-05-17 10:43 Li.huang
// 
// 7     07-05-11 14:54 Li.huang
// 
// 6     07-04-26 19:50 Ken.qian
// 
// 5     07-04-20 16:32 Ken.qian
// 
// 4     07-04-17 10:54 Ken.qian
// 
// 3     07-04-16 15:07 Ken.qian
// 
// 2     07-04-16 11:03 Ken.qian
// 
// 1     07-04-11 20:10 Ken.qian


#include "DODContentStoreImpl.h"
#include "DODContentStoreServ.h"
#include "DODContentStoreCfg.h"
extern DODContentStoreServ g_dodCSServ;
extern ZQ::common::Config::Loader<DODContentStoreCfg> config;

#define CONTENT_CATEGORY      "content"
#define FREEZE_ENV_NAME       "EvictorDB"
#define FREEZE_ENV_EVICTOR    "DODContentObjs"
#define DEF_EVICTOR_SIZE      10

//////////////////////////////////////////////////////////////////////////
//      Implementation of DODDODContentFactory                                //
//////////////////////////////////////////////////////////////////////////

Ice::ObjectPtr DODContentFactory::create(const std::string& type)
{
	if(type == DODContentI::ice_staticId())
	{
		return new DODContentI();
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR, "Invalid DODContentFactory type[%s]", type.c_str());
		throw TianShanIce::InvalidStateOfArt("DODContentStore service" ,0,"Invalid type for DODContentFactory: " + type);
	}
}

void DODContentFactory::destroy()
{
	// do nothing here
}

//////////////////////////////////////////////////////////////////
//          Implementation of DODContentI                       //
//////////////////////////////////////////////////////////////////

DODContentI::DODContentI(const ::std::string& name, std::string destinationContentType, bool createIfNotExist)
{
	_paramConfiged = false;
	this->name = name;
}

DODContentI::DODContentI()
{
	_paramConfiged = false;
}

DODContentI::~DODContentI()
{
}


TianShanIce::Storage::ContentStorePrx DODContentI::getStore(const ::Ice::Current& current)
{
	return NULL;
}

::std::string DODContentI::getName(const ::Ice::Current& current) const
{
	return "";
}

bool DODContentI::isProvisioned(const ::Ice::Current& current) const
{
	return false;
}

::std::string DODContentI::getProvisionTime(const ::Ice::Current& current) const
{
	return "";
}

std::string DODContentI::getLocaltype(const ::Ice::Current& current) const
{
	return "";
}

std::string DODContentI::getSubtype(const ::Ice::Current& current) const
{
	return "";
}

::Ice::Float DODContentI::getFramerate(const ::Ice::Current& current) const
{
	throw TianShanIce::NotImplemented();
}

void DODContentI::getResolution(::Ice::Int& pixelHorizontal, ::Ice::Int& pixelVertical, const ::Ice::Current& current) const
{
	throw TianShanIce::NotImplemented();
}

::Ice::Long DODContentI::getFilesize(const ::Ice::Current& current) const
{
	return 0;
}

::Ice::Long DODContentI::getSupportFileSize(const ::Ice::Current& current) const
{
	return 0;
}

::Ice::Long DODContentI::getPlayTime(const ::Ice::Current& current) const
{
	return 0;
}

::Ice::Int DODContentI::getBitRate(const ::Ice::Current& current) const
{
	return 0;
}

void DODContentI::getTrickSpeedCollection(TianShanIce::Storage::TrickSpeedCollection& speeds, const ::Ice::Current& current) const
{
	throw TianShanIce::NotImplemented();
}

::std::string DODContentI::getSourceUrl(const ::Ice::Current& current) const
{
	return "";
}

void DODContentI::setDataWrappingParam(const ::DataOnDemand::DataWrappingParam& param, const ::Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, "setDataWrappingParam() enter");

    _wrapParam.esPID = param.esPID;
    _wrapParam.streamType = param.streamType ;
    _wrapParam.subStreamCount = param.subStreamCount;
    _wrapParam.dataType = param.dataType;
    _wrapParam.withObjIndex = param.withObjIndex;
    _wrapParam.objTag = param.objTag;
    _wrapParam.encryptType = param.encryptType;
	_wrapParam.versionNumber = param.versionNumber;
	
	_paramConfiged = true;

	glog(ZQ::common::Log::L_DEBUG, "setDataWrappingParam() enter");
}

::std::string DODContentI::getMD5Checksum(const ::Ice::Current& current) const
{
	return "";
}


::std::string DODContentI::getExportURL(const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, 
								   ::TianShanIce::Properties& exppro, const ::Ice::Current& current)
{
	std::string url = name;
	
	int len = config.desURLPrefix.size();
	if(len > 0 && config.desURLPrefix[len-1] != '/' && config.desURLPrefix[len-1] != '\\')
	{
		url = "\\" + name;
	}
	return config.desURLPrefix + url;
}

void DODContentI::provision(const ::std::string& sourceUrl, const std::string& sourceContentType, bool overwrite, 
						const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, 
						::Ice::Int maxTransferBitrate, const ::Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, "[%s] provision() enter", name.c_str());

	if(!_paramConfiged)
	{
		throw TianShanIce::InvalidStateOfArt("DODContentStore service",0,"setDataWrappingParam() was not invoked before provision");
	}

	// sync provision request
	DODProvisionRequest* request = new DODProvisionRequest(g_dodCSServ.m_pReporter, 
														   g_dodCSServ.getGraphPool(), 
														   true, 
		                                                   sourceUrl, name, 
														   0, // no maxbps limitation
														   config.progressReportInterval*1000);
	
	request->setParameters(_wrapParam.esPID, _wrapParam.streamType, _wrapParam.subStreamCount, 
		_wrapParam.withObjIndex, _wrapParam.objTag, _wrapParam.encryptType, _wrapParam.versionNumber, 
		_wrapParam.dataType);

	// start the request
	request->start();

	// waiting for provision completion
	glog(ZQ::common::Log::L_DEBUG, "wait for the completion of provision");

	std::string provErrStr;
	bool result = request->waitForCompletion(provErrStr);

	if(!result)
	{
		glog(ZQ::common::Log::L_ERROR, "[%s] provision failed with error: %s", name.c_str(), provErrStr.c_str());
		
		provErrStr = "provision failed with reason: " + provErrStr; 
		throw TianShanIce::InvalidStateOfArt(provErrStr,0,"");
	}

	glog(ZQ::common::Log::L_DEBUG, "[%s] provision() leave", name.c_str());
}

::std::string DODContentI::provisionPassive(const std::string& sourceContentType, bool overwrite, 
						const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC,
						::Ice::Int maxTransferBitrate, const ::Ice::Current& current)
{
	throw TianShanIce::NotImplemented();
}

void DODContentI::destroy(const ::Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, "[%s] DODContentI::destroy enter", name.c_str());

	std::string filename = config.homeDirectory + name;

	bool ret = DeleteFile(filename.c_str());
	if(!ret)
	{
		glog(ZQ::common::Log::L_WARNING, "[%s] failed to delete file %s", name.c_str(), filename.c_str());
	}

	glog(ZQ::common::Log::L_DEBUG, "[%s] DODContentI::destroy leave", name.c_str());	
}

void DODContentI::destroy2(bool mandatory, const ::Ice::Current& current)
{
	destroy();
}

void DODContentI::cancelProvision(const ::Ice::Current& current)
{
	throw TianShanIce::NotImplemented("DODContentStore service",0,"DODContentStore does not support cancel operation");
}
TianShanIce::Storage::ContentStorePrx DODContentI::getStore(const Ice::Current &) const
{
  return NULL;
}
TianShanIce::Properties DODContentI::getMetaData(const Ice::Current &) const
{
	TianShanIce::Properties prop;
	return prop;
}
void DODContentI::setUserMetaData(const std::string &,const std::string &,const Ice::Current &)
{

}
void DODContentI::setUserMetaData2(const TianShanIce::Properties &,const Ice::Current &)
{

}
TianShanIce::Storage::ContentState DODContentI::getState(const Ice::Current &) const
{
  TianShanIce::Storage::ContentState  state;
  return state;
}
Ice::Long DODContentI::getPlayTimeEx(const Ice::Current &) const
{
  return 0;
}
TianShanIce::Storage::VolumePrx DODContentI::theVolume(const Ice::Current &) const
{
	return NULL;
}
TianShanIce::Storage::TrickSpeedCollection DODContentI::getTrickSpeedCollection(const Ice::Current &) const
{
	TianShanIce::Storage::TrickSpeedCollection trickcollection;
	return trickcollection;
}
void DODContentI::adjustProvisionSchedule(const std::string& startTimeUTC, const std::string& stopTimeUTC, const Ice::Current &)
{
	throw TianShanIce::NotImplemented();
}
//////////////////////////////////////////////////////////////////
//          Implementation of DODContentStoreI               //
//////////////////////////////////////////////////////////////////

DODContentStoreI::DODContentStoreI(std::string netId)

{
	//
	// create evictor for persistent data
	//
	std::string dbpath = config.dbPath;
	try
	{
		Ice::PropertiesPtr properties = g_dodCSServ._communicator->getProperties();

		//
		// Create and install a factory for Content.
		//
		_contentFactory = new DODContentFactory();
		g_dodCSServ._communicator->addObjectFactory(_contentFactory, DODContentI::ice_staticId());

		//
		// Create content name Index
		//
		dbpath += FREEZE_ENV_NAME FNSEPS;
		CreateDirectoryA(dbpath.c_str(), NULL);

#if ICE_INT_VERSION / 100 == 302
		_evictor = Freeze::createEvictor(g_dodCSServ._adapter, dbpath, FREEZE_ENV_EVICTOR);
#else
		_evictor = Freeze::createBackgroundSaveEvictor(g_dodCSServ._adapter, dbpath, FREEZE_ENV_EVICTOR);  //FREEZE_ENV_NAME
#endif

		g_dodCSServ._adapter->addServantLocator(_evictor, CONTENT_CATEGORY);

		Ice::Int evictorSize = properties->getPropertyAsInt("ContentStore.EvictorSize");

		if(evictorSize <= 0)
		{
			evictorSize = DEF_EVICTOR_SIZE;
		}
		_evictor->setSize(evictorSize);
	}
	catch(const Ice::Exception&)
	{
		std::string errmsg = "Failed to create Freeze evictor on " + dbpath;
		throw TianShanIce::InvalidStateOfArt("DODContentStore service",0,errmsg);
	}
}

DODContentStoreI::~DODContentStoreI()
{
}

::std::string DODContentStoreI::getAdminUri(const ::Ice::Current& current)
{
	throw TianShanIce::NotImplemented();
}

::TianShanIce::State DODContentStoreI::getState(const ::Ice::Current& current)
{
	throw TianShanIce::NotImplemented();
}

::std::string DODContentStoreI::getNetId(const ::Ice::Current& current) const
{
	return config.netId;
}

std::string DODContentStoreI::type(const ::Ice::Current& current) const
{
	return TianShanIce::Storage::ctDODTS;
}

bool DODContentStoreI::isValid(const ::Ice::Current& current) const
{
	return true;
}

void DODContentStoreI::getCapacity(::Ice::Long& freeMB, ::Ice::Long& totalMB, const ::Ice::Current& current)
{
	freeMB = 100;
	totalMB = 100;
}

Ice::Byte DODContentStoreI::getCacheLevel(const ::Ice::Current& current) const
{
	return 0;
}

::TianShanIce::StrValues DODContentStoreI::listContent(const ::std::string& condition, const ::Ice::Current& current) const
{
	throw TianShanIce::NotImplemented();
}

TianShanIce::Storage::ContentPrx DODContentStoreI::openContent(const ::std::string& name, const std::string& destinationContentType, bool createIfNotExist, const ::Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, "[%s] openContent enter", name.c_str());

	ZQ::common::MutexGuard guard(_openMutex);

	// create the ident for the content object
    Ice::Identity ident;
    ident.name = name;
    ident.category = CONTENT_CATEGORY;
		
	/* if there is an object cached for the request. */
	if(_evictor->hasObject(ident))
	{
		glog(ZQ::common::Log::L_DEBUG, "[%s] openContent() leave", name.c_str());

		return DataOnDemand::DODContentPrx::uncheckedCast(g_dodCSServ._adapter->createProxy(ident));	
	}
	
	/* needs a new one, then create the servant for this request. possible exception here. */
	DODContentIPtr contentI = new DODContentI(name, destinationContentType, createIfNotExist);
	
	/* cache this object. */
	_evictor->add(contentI, ident);

	glog(ZQ::common::Log::L_DEBUG, "[%s] openContent() leave", name.c_str());

	return DataOnDemand::DODContentPrx::uncheckedCast(g_dodCSServ._adapter->createProxy(ident));
}

/*bool DODContentStoreI::sinkStateChangeEvent(const TianShanIce::Storage::ProvisionStateChangeSinkPrx& event, const ::Ice::Current& current)
{
	return false;
}

bool DODContentStoreI::sinkProgressEvent(const TianShanIce::Storage::ProvisionProgressSinkPrx&, const ::Ice::Current& current)
{
	return false;
}*/
TianShanIce::Storage::VolumePrx DODContentStoreI::openVolume(const std::string &,const Ice::Current &) const
{
	return NULL;
}
void  DODContentStoreI::listVolumes_async(const TianShanIce::Storage::AMD_ContentStore_listVolumesPtr & amdlistvolumes,const std::string &,bool,const Ice::Current &) const
{
	::TianShanIce::Storage::VolumeInfos volumeinfos;
   amdlistvolumes->ice_response(volumeinfos);
}


bool DODContentStoreI::unsinkEvent(const ::TianShanIce::Events::BaseEventSinkPrx&, const ::Ice::Current& current)
{
	return false;
}
TianShanIce::Storage::ContentPrx 
DODContentStoreI::openContentByFullname(const std::string &,const Ice::Current &) const
{
	return NULL;
}
std::string DODContentStoreI::getVolumeName(const Ice::Current &) const
{
    return "";
}
TianShanIce::Replicas DODContentStoreI::getStreamServices(const Ice::Current &) const
{
   TianShanIce::Replicas replicas;
   return replicas;
}
void DODContentStoreI::listContents_async(const ::TianShanIce::Storage::AMD_Folder_listContentsPtr&, const ::TianShanIce::StrValues&, const ::std::string&, ::Ice::Int, const ::Ice::Current& ) const
{

}
TianShanIce::Storage::VolumePrx DODContentStoreI::openSubVolume(const std::string &,bool,Ice::Long,const Ice::Current &)
{
	return NULL;
}
::TianShanIce::Storage::FolderPrx DODContentStoreI::openSubFolder(const ::std::string&, bool, ::Ice::Long, const ::Ice::Current&)
{
	return NULL;
}
::TianShanIce::Storage::FolderPrx DODContentStoreI::parent(const Ice::Current &) const
{
	return NULL;
}
::std::string DODContentStoreI::getName(const ::Ice::Current& ) const
{
	return "";
}
void DODContentStoreI::destroy(const Ice::Current &)
{

}

TianShanIce::Storage::FolderInfos 
DODContentStoreI::listSubFolders(const Ice::Current &) const
{
   throw TianShanIce::NotImplemented();
}




	