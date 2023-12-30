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
// $Log: /ZQProjs/TianShan/ContentStore/cspMediaCluster.cpp $
// 
// 2     10-11-17 18:32 Hui.shao
// resort the node queue
// 
// 31    10-10-27 15:51 Hui.shao
// double checked the changes of V1.10 since 3/19/2009
// 
// 29    09-12-22 14:06 Jie.zhang
// 
// 34    09-09-15 21:40 Jie.zhang
// 
// 33    09-09-15 17:55 Jie.zhang
// change openContentByFullName to openContent
// 
// 32    09-07-16 15:37 Jie.zhang
// forgot the mountpath of the volume
// 
// 31    09-07-14 16:50 Jie.zhang
// add protection on populate from node on FileSize and PlayTime
// 
// 30    09-06-29 18:22 Jie.zhang
// add wakeupContent interface to internal contentstore interface
// 
// 29    09-06-05 11:38 Jie.zhang
// 
// 28    09-05-13 21:19 Jie.zhang
// adjustProvisionSchedule() implementation added
// 
// 27    09-02-27 12:53 Jie.zhang
// 
// 26    09-02-20 17:10 Jie.zhang
// 
// 25    08-12-25 22:53 Jie.zhang
// 
// 24    08-12-25 18:37 Jie.zhang
// 
// 23    08-12-25 17:59 Jie.zhang
// 
// 22    08-12-24 16:50 Jie.zhang
// 
// 21    08-12-24 16:49 Jie.zhang
// 
// 20    08-12-24 11:21 Jie.zhang
// 
// 19    08-12-22 17:23 Jie.zhang
// 
// 18    08-12-22 12:17 Jie.zhang
// 
// 17    08-12-19 16:27 Jie.zhang
// 
// 16    08-12-18 19:21 Jie.zhang
// change content destroy-->destroy2
// 
// 15    08-12-17 21:23 Jie.zhang
// 
// 
// 14    08-12-17 15:19 Jie.zhang
// 
// 13    08-12-10 18:38 Jie.zhang
// 
// 12    08-12-03 17:39 Jie.zhang
// 
// 11    08-11-24 12:29 Jie.zhang
// add a parameter on checkResidencialStatus
// 
// 10    08-11-18 11:57 Jie.zhang
// 
// 9     08-11-13 16:10 Jie.zhang
// 
// 8     08-11-13 11:57 Jie.zhang
// 
// 7     08-11-12 12:06 Jie.zhang
// add volumePath to populateAttrsFromFile
// 
// 6     08-11-11 15:32 Jie.zhang
// 
// 5     08-11-10 11:37 Jie.zhang
// 
// 4     08-11-03 11:19 Jie.zhang
// getExportUrl changed
// 
// 3     08-09-10 11:47 Jie.zhang
// changes check in
// 
// 2     08-08-14 18:01 Hui.shao
// 
// 2     08-08-13 15:56 Hui.shao
// 
// 1     08-08-13 15:35 Hui.shao
// initial draft
// ===========================================================================
#include "ContentImpl.h"
#include "Guid.h"
#include "Log.h"
#include "CombString.h"
#include "CPCImpl.h"
#include "strHelper.h"
#include "urlstr.h"
#include "CPHInc.h"
#include "MCCSCfg.h"
#include "ContentProvisionWrapper.h"
#include "ContentState.h"
#include "TimeUtil.h"
#include "CPHInc.h"
#include "ContentUser.h"

#  include "D4Update.h"

extern "C"
{
	#include <io.h>
};

using namespace ::TianShanIce::Storage;
using namespace ::TianShanIce;
using namespace ZQ::common;

#define MOLOG	(store._log)

namespace ZQTianShan {
namespace ContentStore {

#define READ_DIR_CHANGE_BUFFER_SIZE 4096
#define READ_DIR_CHANGE_FILETER	 (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE \
		| FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY )
#define READ_DIR_CHANGE_SUBDIR	false
#define READ_DIR_CHANGE_INTERVAL (60*1000) // 1min


struct PortalCtx
{
	PortalCtx(ContentProvisionWrapper* pContentProvisionWrapper)
		:cpWrapper(pContentProvisionWrapper),_d4Speaker(NULL)
	{
	};

	ContentProvisionWrapper::Ptr  cpWrapper;
	D4Speaker*                    _d4Speaker;
};

void ContentStoreImpl::initializePortal(ContentStoreImpl& store)
{
	if (NULL != store._ctxPortal)
		return;

	PortalCtx* pCtx = new PortalCtx(new ContentProvisionWrapper(MOLOG));
	store._ctxPortal = (void*) pCtx;
	if (NULL == store._ctxPortal)
		return;

	ContentProvisionWrapper::Ptr& cpWrapper = pCtx->cpWrapper;
	Ice::Identity csIdent = store._adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);
	::TianShanIce::Storage::ContentStoreExPrx  csPrx = ::TianShanIce::Storage::ContentStoreExPrx::
		uncheckedCast(store._adapter->createProxy(csIdent));		

	pCtx->_d4Speaker = NULL;

	if (configGroup.mccsConfig._d4messsage.enableD4)
	{
		D4MessageConfig D4msgCfg;
		D4msgCfg.advInterval = configGroup.mccsConfig._d4messsage.advInterval;

		D4msgCfg.listener = configGroup.mccsConfig._d4messsage.listener;
		D4msgCfg.enableD4 = configGroup.mccsConfig._d4messsage.enableD4;
		D4msgCfg.advInterval = configGroup.mccsConfig._d4messsage.advInterval;
		D4msgCfg.strA3Interface = configGroup.mccsConfig._d4messsage.strA3Interface;
		D4msgCfg.strStreamZone = configGroup.mccsConfig._d4messsage.strStreamZone;
		D4msgCfg.strRouteAddr = configGroup.mccsConfig._d4messsage.strRouteAddr;
		if(configGroup.mccsConfig._d4messsage.strVolumeId.empty())
		{
			D4msgCfg.strVolumeId = configGroup.mccsConfig.netId;
		}
		else
		{
			D4msgCfg.strVolumeId = configGroup.mccsConfig._d4messsage.strVolumeId;
		}

		D4msgCfg.portId = configGroup.mccsConfig._d4messsage.portId;
		D4msgCfg.ServerName = configGroup.mccsConfig._d4messsage.serverName;
		D4msgCfg.ServerIp = configGroup.mccsConfig._d4messsage.serverIp;

		for(std::vector<AdvertiseMethod>::iterator itorAM =  configGroup.mccsConfig._d4messsage.AdMethod.begin(); itorAM != configGroup.mccsConfig._d4messsage.AdMethod.end(); itorAM++) 
			D4msgCfg.AdMethod.push_back(itorAM->method);

		pCtx->_d4Speaker = new D4Speaker(MOLOG,store._thpool, csPrx,D4msgCfg);
		if (!pCtx->_d4Speaker)
			return;
		pCtx->_d4Speaker->start();
	}

	if (!cpWrapper->init(store._adapter->getCommunicator(), csPrx, configGroup.mccsConfig.cpcEndPoint, configGroup.mccsConfig.registerInterval,pCtx->_d4Speaker))
	{		
		return;
	}	

	cpWrapper->setTrickSpeeds(configGroup.mccsConfig.trickSpeedCollection);
}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store)
{
	if (NULL != store._ctxPortal)
	{
		PortalCtx* pCtx = (PortalCtx*)(store._ctxPortal);
		ContentProvisionWrapper::Ptr& cpWrapper = pCtx->cpWrapper;
		cpWrapper->unInit();

		if (configGroup.mccsConfig._d4messsage.enableD4)
		{
			if (pCtx->_d4Speaker)
			{
				pCtx->_d4Speaker->stop();
				delete pCtx->_d4Speaker;
				pCtx->_d4Speaker = NULL;
			}
		}

		delete pCtx;
		store._ctxPortal = NULL;
	}	
}

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{

	return true;
}

bool ContentStoreImpl::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{

	return true;
}

void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath)
{
	freeMB = totalMB = 0;

	NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);

	if (nrqueue.empty())
	{
		MOLOG(Log::L_WARNING, CLOGFMT(MediaClusterCS, "getStorageSpace() fail, no node replica available"));
		return;
	}

	for(; !nrqueue.empty() && totalMB <=0; nrqueue.pop())
	{
		::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try {
			::Ice::Long lFreeMB=0, lTotalMB=0;

			MOLOG(Log::L_DEBUG, CLOGFMT(MediaClusterCS, "to getStorageSpace() from replica[%s]"), replica.replicaId.c_str());
			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::checkedCast(replica.obj);
			nodeStore->getCapacity(lFreeMB, lTotalMB);
			freeMB = (uint32) lFreeMB; totalMB = (uint32) lTotalMB;

			MOLOG(Log::L_DEBUG, CLOGFMT(MediaClusterCS, "getStorageSpace() from replica[%s], freeMB=%d, totalMB=%d"), 
				replica.replicaId.c_str(), freeMB, totalMB);
			break;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "getStorageSpace() replica[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "getStorageSpace() replica[%s] caught exception[%s]"), replica.replicaId.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "getStorageSpace() replica[%s] caught unknown exception"), replica.replicaId.c_str());
		}
	}
}

bool ContentStoreImpl::validateMainFileName(ContentStoreImpl& store, std::string& contentName, const std::string& contentType)
{
	std::string name = fixupPathname(store, contentName);
    int bpos = name.find_last_of("\\/");
    int epos = name.find_last_of(".");

	if (epos < bpos)
	{
		contentName = name.substr(bpos +1);
		return true;
	}

	std::string extfn = name.substr(epos+1, 2);
	if (0 == extfn.compare("VV") || 0 == extfn.compare("FF") || 0 == extfn.compare("FR"))
	{
		contentName = name.substr(bpos+1, epos - bpos - 1);
	}
	else contentName = name.substr(bpos +1);

	return true;
}


// bool ContentStoreImpl::fileExists(ContentStoreImpl& store, const std::string& filename)
// {
// 	NodeReplicaQueue nrqueue;
// 	store.buildNodeReplicaQueue(nrqueue);
// 
// 	for(; !nrqueue.empty(); nrqueue.pop())
// 	{
// 		::TianShanIce::Replica& replica = nrqueue.top().replicaData;
// 
// 		try {
// 			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::checkedCast(replica.obj);
// 			::TianShanIce::StrValues contents = nodeStore->listContent(filename);
// 
// 			return contents.size() >0;
// 		}
// 		catch(const ::TianShanIce::BaseException& ex)
// 		{
// 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "fileExists() replica[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
// 		}
// 		catch(const ::Ice::Exception& ex)
// 		{
// 			std::string strRepObj = store._adapter->getCommunicator()->proxyToString(replica.obj);
// 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "fileExists() replica[%s][%s] caught exception[%s]"),
// 				replica.replicaId.c_str(), strRepObj.c_str(), ex.ice_name().c_str());
// 		}
// 		catch(...)
// 		{
// 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "fileExists() replica[%s] caught unknown exception"), replica.replicaId.c_str());
// 		}
// 	}
// 
// 	return false;
// }

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath)
{
	NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "ContentStore require listMainFiles(), replica queue size(%d)"), nrqueue.size());

	ContentStoreImpl::FileInfos filenames;
	for(; !nrqueue.empty(); nrqueue.pop())
	{
		::TianShanIce::Replica& replica = nrqueue.top().replicaData;
		filenames.clear();

		try 
		{
			TianShanIce::StrValues metaDataNames;
			metaDataNames.push_back(SYS_PROP(StampLastUpdated));
			
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "ContentStore listMainFiles() on replica[%s]"), replica.replicaId.c_str());
			
			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::uncheckedCast(replica.obj);
			::TianShanIce::Storage::ContentInfos contents = nodeStore->listContents(metaDataNames, "", -1);
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "ContentStore listMainFiles() found %d contents on replica[%s]"),
				contents.size(), replica.replicaId.c_str());

			for (::TianShanIce::Storage::ContentInfos::iterator it = contents.begin(); it < contents.end(); it++)
			{
				FileInfo fi;
				fi.filename = it->name;
				fi.stampLastWrite = it->metaData[SYS_PROP(StampLastUpdated)];
				
				filenames.push_back(fi);
			}

			return filenames;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "listMainFiles() replica[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());			
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "listMainFiles() replica[%s] caught exception[%s]"), replica.replicaId.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "listMainFiles() replica[%s] caught unknown exception"), replica.replicaId.c_str());
		}
	}

	ZQTianShan::_IceThrow<::TianShanIce::Storage::NoResourceException>(MOLOG, EXPFMT(MediaClusterCS, 0, "no node replica available for listMainFiles()"));
	return filenames;
}

std::string ContentStoreImpl::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename)
{
#if 1
	std::string contentName = store.fixupPathname(store, memberFilename);
	
	//remove the "/\"
	{
		std::string::size_type bpos = contentName.find_last_of("\\/");
		if (bpos!=std::string::npos)
			contentName = contentName.substr(bpos +1);
	}

	std::string::size_type epos = contentName.find_last_of(".");
	if (epos==std::string::npos)
		return contentName;

	std::string extfn = contentName.substr(epos+1, 2);
	if (0 == extfn.compare("VV") || 0 == extfn.compare("FF") || 0 == extfn.compare("FR"))
		return contentName.substr(0, epos);

	return contentName;

#else
	//
	// test the 
	//
	std::string contentName = store.fixupPathname(store, memberFilename);
	
	//remove the "/\"
	{
		std::string::size_type bpos = contentName.find_last_of("\\/");
		if (bpos!=std::string::npos)
			contentName = contentName.substr(bpos +1);
	}
	
	//process the "."
	std::string::size_type epos = contentName.find_last_of(".");
	if (epos==std::string::npos)
		return contentName;

	std::string extfn = contentName.substr( epos+1 );
	std::vector<std::string> subFileExtNames;
	subFileExtNames.push_back("vv2");
	subFileExtNames.push_back("vvx");
	subFileExtNames.push_back("ff");
	subFileExtNames.push_back("ff1");
	subFileExtNames.push_back("ff2");
	subFileExtNames.push_back("ff3");
	subFileExtNames.push_back("fr");
	subFileExtNames.push_back("fr1");
	subFileExtNames.push_back("fr2");
	subFileExtNames.push_back("fr3");
	subFileExtNames.push_back("ffr");
	subFileExtNames.push_back("ffr1");
	subFileExtNames.push_back("ffr2");
	subFileExtNames.push_back("ffr3");

	std::vector<std::string>::const_iterator itFileExt = subFileExtNames.begin( );
	for( ; itFileExt != subFileExtNames.end() ; itFileExt ++ )
	{
		if( stricmp( itFileExt->c_str() , extfn.c_str() ) == 0 )
		{
			return contentName.substr(0, epos);
		}
	}

	return contentName;
#endif
}


bool ContentStoreImpl::deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname)
{
	NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);

	std::string strVolumeName = content.identVolume.name;
	std::string strContentName = content._name();
	std::string strContentType = "";

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "Content[%s] deleteFileByContent(), replica queue size(%d)"), 
		strContentName.c_str(), nrqueue.size());

	int c =0;
	for(; !nrqueue.empty(); nrqueue.pop())
	{
		::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try 
		{			
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "destroy content[%s] on replica[%s] volume[%s]"), 
				strContentName.c_str(), replica.replicaId.c_str(), strVolumeName.c_str());

			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::uncheckedCast(replica.obj);
			::TianShanIce::Storage::VolumePrx volumePrx = nodeStore->openVolume(strVolumeName);
			::TianShanIce::Storage::ContentPrx contentPrx = volumePrx->openContent(strContentName, strContentType, false);

			if (contentPrx)
			{
				::TianShanIce::Properties::const_iterator itMD = content.metaData.find(SYS_PROP(DestroySignature));
				if (content.metaData.end() == itMD || itMD->second.empty()) // ignore to call contentPrx->destroy2(true) if there is no DestroySignature
					continue;
					
				contentPrx->destroy2(true);
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "content[%s] destroyed on replica[%s]"), strContentName.c_str(), replica.replicaId.c_str());
			}

			c++;
		}
		catch(const ::Ice::ObjectNotExistException&)
		{
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "deleteByContent() on replica[%s] content[%s] caught ObjectNotExistException"), 
				replica.replicaId.c_str(), strContentName.c_str());

			c++;	//not exist also works 
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "deleteByContent() on replica[%s] content[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "deleteByContent() on replica[%s] content[%s] caught exception[%s]"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "deleteByContent() on replica[%s] content[%s] caught unknown exception"), strContentName.c_str(), replica.replicaId.c_str());
		}
	}

	return (c>0);
}

std::string showCheckResidentialFlag( uint64 flag )
{
	char	szTemp[1024];
	char	*p = szTemp;
	size_t	lenTemp = sizeof(szTemp)-1;	
	szTemp[lenTemp] = 0;
	if( flag & ( 1 << TianShanIce::Storage::frfResidential) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfResidential" );
		p += iRet;
		lenTemp -= iRet ;
	}
	if( flag & ( 1 << TianShanIce::Storage::frfReading ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfReading" );
		p += iRet;
		lenTemp -= iRet ;
	}
	if( flag & ( 1 << TianShanIce::Storage::frfWriting ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfWriting" );
		p += iRet;
		lenTemp -= iRet ;
	}

	if( flag & ( 1 << TianShanIce::Storage::frfAbsence ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfAbsence" );
		p += iRet;
		lenTemp -= iRet ;
	}

	if( flag & ( 1 << TianShanIce::Storage::frfCorrupt ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfCorrupt" );
		p += iRet;
		lenTemp -= iRet ;
	}

	if( flag & ( 1 << TianShanIce::Storage::frfDirectory ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfDirectory" );
		p += iRet;
		lenTemp -= iRet ;
	}

	return szTemp;
}

uint64 ContentStoreImpl::checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname)
{
	NodeReplicaQueue tmpQ;
	NodeReplicaQueue2 nrqueue;
	store.buildNodeReplicaQueue(tmpQ);

	std::string strCheckRes = showCheckResidentialFlag(flagsToTest);
	std::string strContentType = "";
	std::string strVolumeName, strContentName;
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "Content[%s] checkResidentialStatus(%s), replica queue size(%d)"), 
		contentFullName.c_str(), strCheckRes.c_str(), tmpQ.size());

	if (pContent)
	{
		strVolumeName = pContent->identVolume.name;
		strContentName = pContent->_name();

		store.resortNodeQueuePerContentReplica(tmpQ, nrqueue, *pContent);
	}
	else
	{
		//get the volume and content name from contentfull name
		std::string::size_type bpos = contentFullName.find_last_of(LOGIC_FNSEPS);
		if (bpos!=std::string::npos)
		{
			strContentName = contentFullName.substr(bpos +1);
			strVolumeName = contentFullName.substr(0, bpos);

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "checkResidentialStatus() identify the volume[%s] content[%s] from fullname[%s]"), 
				strVolumeName.c_str(), strContentName.c_str(), contentFullName.c_str());
		}
		else
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(MediaClusterCS, "checkResidentialStatus() could not identify the volume and content from fullname[%s]"), 
				contentFullName.c_str());
			
			return RSDFLAG(frfAbsence);
		}
	}

	int c =0;
	for(; !nrqueue.empty(); nrqueue.pop())
	{
		::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try 
		{
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "Content[%s] checkResidentialStatus() on replica[%s] volume[%s]"), 
				strContentName.c_str(), replica.replicaId.c_str(), strVolumeName.c_str());

			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::uncheckedCast(replica.obj);
			::TianShanIce::Storage::VolumePrx volumePrx = nodeStore->openVolume(strVolumeName);
			::TianShanIce::Storage::ContentPrx content = volumePrx->openContent(strContentName, strContentType, false);
			::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(content);

			if (!uniContent)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "Could not find content[%s] on replica[%s]"), 
					strContentName.c_str(), replica.replicaId.c_str());

				if (nrqueue.size()> 1)
					continue;

				return RSDFLAG(frfAbsence);		//not exist at all
			}

			uint64 flagsRet = uniContent->checkResidentialStatus(flagsToTest);
			std::string strRet = showCheckResidentialFlag(flagsRet);

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "Content[%s] checkResidentialStatus() on replica[%s] return [%s]"), 
				strContentName.c_str(), replica.replicaId.c_str(), strRet.c_str());

			return flagsRet;
		}
		catch(const ::Ice::ObjectNotExistException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "checkResidentialStatus() replica[%s] content[%s] caught exception[%s]"),
				replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str());

			if (nrqueue.size()> 1)
			{
				continue;
			}
			else
			{
				return RSDFLAG(frfAbsence);		//not exist at all
			}
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "checkResidentialStatus() replica[%s] content[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "checkResidentialStatus() replica[%s] content[%s] caught exception[%s]"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "deleteByContent() replica[%s] content[%s] caught unknown exception"), strContentName.c_str(), replica.replicaId.c_str());
		}
	}

	ZQTianShan::_IceThrow<::TianShanIce::ServerError>(MOLOG, EXPFMT(MediaClusterCS, 0, "Content[%s] could not checkResidentialStatus() at the moment"), contentFullName.c_str());

	return 0;
}

#ifdef _DEBUG
void dumpProperty(ContentStoreImpl& store,const std::string& content, const std::string& tag, const ::TianShanIce::Properties & props)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "======================= dump [%d] properties for content %s %s ===========================\n"), props.size(), content.c_str(), tag.c_str());
	
	::TianShanIce::Properties::const_iterator it = props.begin();
	for(;it!=props.end();it++)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "key [%s] = [%s]\n"), it->first.c_str(), it->second.c_str());
	}
}
#endif

bool verifyCriticalMetaData(const ::TianShanIce::Properties& metaDatas)
{
	::TianShanIce::Properties::const_iterator it;
	it = metaDatas.find(METADATA_PlayTime);
	if (metaDatas.end() == it)
		return false;

	if (!atoi(it->second.c_str()))
		return false;

	it = metaDatas.find(METADATA_BitRate);
	if (metaDatas.end() == it)
		return false;

	if (!atoi(it->second.c_str()))
		return false;

	return true;
}

bool ContentStoreImpl::populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname)
{
	std::string strVolumeName = content.identVolume.name;
	std::string strContentName = content._name();
	std::string strContentType = "";

	NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);
	int nNodeRepCount = nrqueue.size();

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "Content[%s] populateAttrsFromFile(), replica queue size(%d)"), 
		strContentName.c_str(), nNodeRepCount);

	if (!nNodeRepCount)
	{
		return false;
	}

#ifdef _DEBUG
	dumpProperty(store, strContentName, "before", content.metaData);
#endif

	for(; !nrqueue.empty(); nrqueue.pop())
	{
		::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try 
		{
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "Content[%s] populateAttrsFromFile() on replica[%s]"), strContentName.c_str(), replica.replicaId.c_str());

			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::uncheckedCast(replica.obj);
			::TianShanIce::Storage::VolumePrx volumePrx = nodeStore->openVolume(strVolumeName);
			::TianShanIce::Storage::ContentPrx cont = volumePrx->openContent(strContentName, strContentType, false);

			if (!cont)
			{
				continue;
			}

			::TianShanIce::Properties metaDatas = cont->getMetaData();
			::TianShanIce::Properties metaDataToNode = content.metaData;

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "Content[%s] found (%d) metadatas on replica[%s]"), strContentName.c_str(), 
				metaDatas.size(), replica.replicaId.c_str());

			for (::TianShanIce::Properties::const_iterator it = metaDatas.begin(); it!= metaDatas.end(); it++)
			{
				// we should take the system properties from node as the bible, but
				// take the user's properties from the cluster as bible
				static const int prefixLen = strlen(SYS_PROP_PREFIX);
				if (0 != it->first.substr(0, prefixLen).compare(SYS_PROP_PREFIX))
					continue;

				metaDataToNode.erase(it->first);

				bool bUseMetaData = true;
				::TianShanIce::Properties::const_iterator itp = content.metaData.find(it->first);
				if (content.metaData.end() == itp)
					content.metaData.insert(*it);
				else 
				{	
					if (it->first == METADATA_FileSize)
					{
						int64 nSavedFileSize = _atoi64(itp->second.c_str());
						int64 nNodeFileSize = _atoi64(it->second.c_str());

						if (nNodeFileSize <= nSavedFileSize)
						{
							bUseMetaData = false;							
						}						
					}

					if (it->first == METADATA_PlayTime)
					{
						int64 nSavedPlayTime = _atoi64(itp->second.c_str());
						int64 nNodePlayTime = _atoi64(it->second.c_str());

						if (nNodePlayTime <= nSavedPlayTime)
						{
							bUseMetaData = false;							
						}						
					}

					if (bUseMetaData)
					{
						content.metaData[it->first] = it->second;
					}
				}

				if (bUseMetaData)
				{
					MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "Content[%s] setMetaData(%s)=%s"), 
						strContentName.c_str(), it->first.c_str(), it->second.c_str());
				}
				else
				{
					MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "Content[%s], the current MetaData(%s)=%s, skip MetaData(%s)=%s from node"), 
						strContentName.c_str(), it->first.c_str(), itp->second.c_str(), it->first.c_str(), it->second.c_str());
				}
			}

			// flush the user's metadata and node-unknown meta
			if (metaDataToNode.size() >0)
			{
				MOLOG(Log::L_DEBUG, CLOGFMT(MediaClusterCS, "setMetaData to replica[%s] content[%s]"), replica.replicaId.c_str(), strContentName.c_str());
				::TianShanIce::Storage::UnivContentPrx::checkedCast(cont)->setMetaData(metaDataToNode);
			}

#ifdef _DEBUG
			dumpProperty(store, contentName, "after", content.metaData);
#endif

			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "Content[%s] populateAttrsFromFile() on replica[%s] successful"), strContentName.c_str(), replica.replicaId.c_str());
			return true;		//verifyCriticalMetaData(content.metaData);
		}
		catch(const ::Ice::ObjectNotExistException&)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "populateAttrsFromFile() on replica[%s] content[%s] caught ObjectNotExistException"), 
				replica.replicaId.c_str(), strContentName.c_str());
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "populateAttrsFromFile() on replica[%s] content[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "populateAttrsFromFile() on replica[%s] content[%s] caught exception[%s]"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "populateAttrsFromFile() on replica[%s] content[%s] caught unknown exception"), strContentName.c_str(), replica.replicaId.c_str());
		}
	}

	return false;
}

bool ContentStoreImpl::completeRenaming(ContentStoreImpl& store, const ::std::string& oldName, const ::std::string& newName)
{
	return false;
}


TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::submitProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& contentName,
						const ::std::string& sourceUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	int transferBitrate = maxTransferBitrate;
	if (!transferBitrate)
	{
		transferBitrate = configGroup.mccsConfig.defaultProvisionBW;
	}

	//
	// find out if it is a NPVR session
	//
	bool bNPVRSession = false;
	{
		::TianShanIce::Properties metaDatas = content.getMetaData(Ice::Current());
		::TianShanIce::Properties::const_iterator it = metaDatas.find(METADATA_nPVRCopy);
		if (it!=metaDatas.end())
		{
			if (atoi(it->second.c_str()))
			{
				bNPVRSession = true;				
			}

			//
			// dump the NPVR parameters
			//

			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
				contentName.c_str(), it->first.c_str(), it->second.c_str());

			it = metaDatas.find(METADATA_ProviderId);
			if (it!=metaDatas.end())
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
					contentName.c_str(), it->first.c_str(), it->second.c_str());
			}
			
			it = metaDatas.find(METADATA_ProviderAssetId);
			if (it!=metaDatas.end())
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
					contentName.c_str(), it->first.c_str(), it->second.c_str());
			}

			it = metaDatas.find(METADATA_SubscriberId);
			if (it!=metaDatas.end())
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
					contentName.c_str(), it->first.c_str(), it->second.c_str());
			}
		}
	}

	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

	//remove the first possible '\' or "\\"
	if (strFilePathName[0]=='\\' || strFilePathName[0]=='/')
		strFilePathName = strFilePathName.substr(1);

	::TianShanIce::Storage::ContentPrx	contentPrx;
	{
		// get the content proxy
		contentPrx = ::TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));
	}

	TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper::Ptr& cpWrapper = ((PortalCtx*)store._ctxPortal)->cpWrapper;;
	
	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = cpWrapper->activeProvision(
		contentPrx,
		contentKey,
		strFilePathName,	
		sourceUrl,
		sourceType, 
		startTimeUTC,
		stopTimeUTC, 
		transferBitrate,
		bNPVRSession);

	if (bNPVRSession)
	{
		//get property and set to content 
		std::string strnPVRLeadCopy;

		::TianShanIce::Properties propers = pPrx->getProperties();
		::TianShanIce::Properties::const_iterator it = propers.find(METADATA_nPVRLeadCopy);
		if (it!=propers.end())
		{
			strnPVRLeadCopy = it->second;
		}

		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "[%s] nPVR lead session copy [%s]"), 
			strFilePathName.c_str(), strnPVRLeadCopy.c_str());

		::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(contentPrx);
		TianShanIce::Properties metaData;
		metaData[METADATA_nPVRLeadCopy] = strnPVRLeadCopy;
		try
		{
			uniContent->setMetaData(metaData);
		}
		catch (const Ice::Exception& ex) 
		{
		}
		catch (...) 
		{
		}
	}

	return pPrx;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& contentName,
						::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

	//remove the first possible '\' or "\\"
	if (strFilePathName[0]=='\\' || strFilePathName[0]=='/')
		strFilePathName = strFilePathName.substr(1);

	::TianShanIce::Storage::ContentPrx	contentPrx;
	{
		// get the content proxy
		contentPrx = ::TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));
	}

	TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper::Ptr& cpWrapper = ((PortalCtx*)store._ctxPortal)->cpWrapper;;

	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = cpWrapper->passiveProvision(
		contentPrx,
		contentKey,
		strFilePathName,					  
		sourceType, 
		startTimeUTC,
		stopTimeUTC, 
		maxTransferBitrate,
		pushUrl);

	return pPrx;
}

std::string ContentStoreImpl::getExportURL(ContentStoreImpl& store, ContentImpl& content, const std::string& contentName, const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params)
{
	ContentProvisionWrapper::Ptr& cpWrapper = ((PortalCtx*)store._ctxPortal)->cpWrapper;;

	//const std::string& protocal, const std::string& filename, int transferBitrate, int& nTTL, int& permittedBitrate
	int transBitrate = transferBitrate;
	int nTTL = 0;
	int permittedBitrate;

	/* invalidate the protocol. */
	if(transferProtocol != TianShanIce::Storage::potoFTP){
		ZQTianShan::_IceThrow<InvalidStateOfArt>(
			MOLOG,
			EXPFMT(MediaClusterCS, csexpUnsupportProto, "protocol (%s) not supported"), transferProtocol.c_str()
			);
	}

#pragma message ( __MSGLOC__ "TODO: change getExportUrl interface to add transfer bitrate etc.")
	std::string strExposeUrl = cpWrapper->getExposeUrl(transferProtocol, contentName, transBitrate, nTTL, permittedBitrate);

	ttl = nTTL;

	{
		std::ostringstream oss;
		oss << ttl;		
		params[expTTL] = oss.str();

		oss.str("");
		oss << permittedBitrate;
		params[expTransferBitrate] = oss.str();		
	}

	time_t now = time(0);
	char window[255];

	ZQ::common::TimeUtil::Time2Iso(now, window, 255);
	std::string stStart = window;
	params[expTimeWindowStart] = stStart;

	ZQ::common::TimeUtil::Time2Iso(now+ttl, window, 255);
	std::string stEnd = window;
	params[expTimeWindowEnd] = stEnd;


	MOLOG(ZQ::common::Log::L_DEBUG, 
		LOGFMT("(%s) getExportURL [URL: (%s) ttl: (%d) timeWindowStart: (%s) timeWindowEnd: (%s) bitrate: (%d)"), 
		contentName.c_str(), strExposeUrl.c_str(), ttl, stStart.c_str(), stEnd.c_str(), permittedBitrate);

	return strExposeUrl;
}

void ContentStoreImpl::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
{
	ContentProvisionWrapper::Ptr& cpWrapper = ((PortalCtx*)store._ctxPortal)->cpWrapper;
	
	std::string contentName = content._name();
	cpWrapper->cancelProvision(contentName, provisionTaskPrx);
}

void ContentStoreImpl::notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew)
{
	::TianShanIce::Replicas reps1, reps2;
	
	////* step 1, filter out the replicas not belongs to this cluster
	{
		::TianShanIce::Replicas::const_iterator it;
		for(it=replicasOld.begin();it!=replicasOld.end();it++)
		{
			if (0 != it->category.compare(CATEGORY_ContentStore) || 0 != it->groupId.compare(store._replicaGroupId))
				continue;

			reps1.push_back(*it);
		}

		for(it=replicasNew.begin();it!=replicasNew.end();it++)
		{
			if (0 != it->category.compare(CATEGORY_ContentStore) || 0 != it->groupId.compare(store._replicaGroupId))
				continue;

			reps2.push_back(*it);
		}
	}

/* merged from V1.10
	//// step 2, if this is the first nodeCS ever know, 
	if (reps1.size() == 0 && reps2.size() > 0)
	{
		////   step 2.1 pick one of the replica and do listVolumes()
		////          for each volume not is virtual
		////          {
		//				volumename = volumePath = volume.name;
		//				if (volumename matches spec)
		//				{
		//					trust it as the only single volume on the storage
		//					adjust the value of volumename and volumepath
		//				}
		///*             _store.mountVolume(volume.name, volume.name);
		////           }
		::TianShanIce::Replicas::iterator it;
		for(it=reps2.begin();it!=reps2.end();it++)
		{
			::TianShanIce::Replica& replica = *it;

	 		try 
			{
	 			std::string prxstr = store._adapter->getCommunicator()->proxyToString(replica.obj);
	 			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() try to sync with replica[%s][%s]"), replica.replicaId.c_str(), prxstr.c_str());
	 
	 			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::checkedCast(replica.obj);
	 			::TianShanIce::Storage::VolumeInfos volumeInfos = nodeStore->listVolumes("", false);
	 			
	 			::TianShanIce::Storage::VolumeInfos::iterator itv = volumeInfos.begin();
	 			for(;itv!=volumeInfos.end();itv++)
	 			{
	 				::TianShanIce::Storage::VolumeInfo& vInfo = *itv;
	 				//::TianShanIce::Storage::VolumePrx	vPrx = nodeStore->openVolume(vInfo.name);
					std::string mountPath = vInfo.metaData[SYS_PROP(MountPath)];
			
					 store.mountStoreVolume(vInfo.name, mountPath, true);
				}

				break;
			}
	 		catch(const ::Ice::ObjectNotExistException&)
	 		{
				MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() replica[%s] caught ObjectNotExistException"), replica.replicaId.c_str());
	 		}
	 		catch(const ::TianShanIce::BaseException& ex)
	 		{
	 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() replica[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	 		}
	 		catch(const ::Ice::Exception& ex)
	 		{
	 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() replica[%s] caught exception[%s]"), replica.replicaId.c_str(), ex.ice_name().c_str());
	 		}
	 		catch(...)
	 		{
	 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() replica[%s] caught unknown exception"), replica.replicaId.c_str());
	 		}
		}
	}
*/
	::TianShanIce::Replicas::iterator it;
	for(it=reps2.begin();it!=reps2.end();it++)
	{
		::TianShanIce::Replica& replica = *it;

 		try 
		{
 			std::string prxstr = store._adapter->getCommunicator()->proxyToString(replica.obj);
 			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() try to sync with replica[%s][%s]"), replica.replicaId.c_str(), prxstr.c_str());
 
 			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::checkedCast(replica.obj);
 			::TianShanIce::Storage::VolumeInfos volumeInfos = nodeStore->listVolumes("", false);

 			::TianShanIce::Storage::VolumeInfos::iterator itv = volumeInfos.begin();
 			for(;itv!=volumeInfos.end();itv++)
 			{
 				::TianShanIce::Storage::VolumeInfo& vInfo = *itv;
				::TianShanIce::Storage::VolumePrx	vPrx = store.openVolume(vInfo.name,Ice::Current());
				if(!vPrx)
				{
					std::string mountPath = vInfo.metaData[SYS_PROP(MountPath)];
			
					 store.mountStoreVolume(vInfo.name, mountPath, true);
				}
			}

			break;
		}
 		catch(const ::Ice::ObjectNotExistException&)
 		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() replica[%s] caught ObjectNotExistException"), replica.replicaId.c_str());
 		}
 		catch(const ::TianShanIce::BaseException& ex)
 		{
 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() replica[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
 		}
 		catch(const ::Ice::Exception& ex)
 		{
 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() replica[%s] caught exception[%s]"), replica.replicaId.c_str(), ex.ice_name().c_str());
 		}
 		catch(...)
 		{
 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "notifyReplicasChanged() replica[%s] caught unknown exception"), replica.replicaId.c_str());
 		}
	}

}


}} // namespace
