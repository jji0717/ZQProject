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
// $Log: /ZQProjs/TianShan/ContentStore/ClusterCS.cpp $
// 
// 15    6/18/15 4:16p Li.huang
// fix bug 21380
// 
// 14    2/02/15 3:53p Hui.shao
// merged from V1.15
// 
// 14    2/02/15 3:40p Hui.shao
// merged from 1.15
// 
// 13    4/16/14 10:28a Li.huang
// fix bug 18963
// 
// 12    1/16/14 12:44p Hui.shao
// 
// 11    1/02/14 4:27p Hui.shao
// 
// 10    1/02/14 2:28p Hui.shao
// bug#18785  clusterCS should response 0 when slave replica is at state
// OutService or Cleaning
// 
// 9     12/25/13 5:50p Zonghuan.xiao
// contentstore refector to support native contentstore
// 
// 8     5/30/12 6:06p Hui.shao
// 
// 8     5/30/12 6:05p Hui.shao
// 
// 9     5/30/12 6:04p Hui.shao
// 
// 8     5/22/12 2:56p Hui.shao
// ticket#11165
// 
// 7     4/18/12 3:14p Hui.shao
// bug#16317
// 
// 6     3/09/12 12:11p Li.huang
// fix bug 15933
// 
// 5     9/16/11 8:39a Li.huang
// add WishedTrickSpeeds metedata
// 
// 4     5/26/11 6:04p Hui.shao
// 
// 3     3/17/11 5:44p Jie.zhang
// 
// 2     2/23/11 7:36p Jie.zhang
// support subfolder for mediaclustercs
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 15    10-10-28 15:22 Li.huang
// 
// 14    10-09-24 16:55 Li.huang
// 
// 13    10-09-24 10:35 Li.huang
// add AugmentationPID
// 
// 12    10-05-07 11:09 Xia.chen
// 
// 11    10-05-06 15:11 Xia.chen
// add config on-off for generating vvc index as default type
// 
// 10    10-04-29 16:30 Xia.chen
// 
// 9     10-04-27 13:45 Xia.chen
// 
// 8     10-03-05 15:40 Fei.huang
// * added NoResourceException to excpetion spec of submitProvision
// 
// 7     10-03-03 15:50 Xia.chen
// multiple dir support and support for vvc type setting
// 
// 5     09-12-22 14:06 Jie.zhang
// merge from TianShan1.10
// 
// 4     09-07-29 13:49 Jie.zhang
// merge from TianShan1.10
// 
// 3     09-07-24 15:03 Xia.chen
// change getExportUrl interface
// 
// 2     09-07-16 12:21 Jie.zhang
// forgot the mount path
// 
// 1     09-06-05 10:45 Jie.zhang
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
#include "ClusterCS.h"

#ifdef ZQ_OS_MSWIN
#   define ATOI64 _atoi64
#else
#   define ATOI64 atoll
#endif


extern "C"
{
//	#include <io.h>
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
		:cpWrapper(pContentProvisionWrapper)
	{
	};

	ContentProvisionWrapper::Ptr  cpWrapper;
};


void ClusterCS::initializePortal(ContentStoreImpl& store)
{
	_ctx = new PortalCtx(new ContentProvisionWrapper(store._log));
	if (NULL == _ctx)
		return;

	ContentProvisionWrapper::Ptr& cpWrapper = _ctx->cpWrapper;
	Ice::Identity csIdent = store._adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);
	::TianShanIce::Storage::ContentStoreExPrx  csPrx = ::TianShanIce::Storage::ContentStoreExPrx::
		uncheckedCast(store._adapter->createProxy(csIdent));		
	
	if (!cpWrapper->init(store._adapter->getCommunicator(), csPrx, configGroup.mccsConfig.cpcEndPoint, configGroup.mccsConfig.registerInterval))
	{		
		return;
	}	

	cpWrapper->setTrickSpeeds(configGroup.mccsConfig.trickSpeedCollection);
	cpWrapper->setNoTrickSpeedFileRegex(configGroup.mccsConfig.noTrickSpeeds.enable, configGroup.mccsConfig.noTrickSpeeds.expressionList);
}

void ClusterCS::uninitializePortal(ContentStoreImpl& store)
{
	if (NULL != _ctx)
	{
		PortalCtx* pCtx = _ctx;
		_ctx = NULL;
		ContentProvisionWrapper::Ptr& cpWrapper = pCtx->cpWrapper;
		cpWrapper->unInit();

		delete pCtx;
	}	
}

bool ClusterCS::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume,const std::string& volname)
{
	std::string subfolder;
	{
		std::string::size_type bpos = pathOfVolume.find_last_of("\\/");
		if (bpos!=std::string::npos)
			subfolder = pathOfVolume.substr(bpos+1);
	}

	ContentStoreImpl::NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);

	if (nrqueue.empty())
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ClusterCS, "createPathOfVolume() fail, no node replica available"));
		return false;
	}

	for(; !nrqueue.empty(); nrqueue.pop())
	{
		const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;
		try 
		{
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "createPathOfVolume() on replica[%s] pathofVolume[%s] basevolume[%s]"), replica.replicaId.c_str(),pathOfVolume.c_str(),volname.c_str());
			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::uncheckedCast(replica.obj);		
			::TianShanIce::Storage::VolumePrx volume = nodeStore->openVolume(volname);
			if (volume && !subfolder.empty())
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "Invoke openSubFolder interface"));
				::TianShanIce::Storage::FolderPrx  folderPrx = volume->openSubFolder(subfolder,true,0);
			}
		}
		catch(const ::Ice::ObjectNotExistException&)
		{
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "createPathOfVolume() on replica[%s] pathOfVolume[%s] caught ObjectNotExistException"), 
				replica.replicaId.c_str(), pathOfVolume.c_str());
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "createPathOfVolume() on replica[%s] pathOfVolume[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), pathOfVolume.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "createPathOfVolume() on replica[%s] pathOfVolume[%s] caught exception[%s]"), replica.replicaId.c_str(), pathOfVolume.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "createPathOfVolume() on replica[%s] pathOfVolume[%s] caught unknown exception"), replica.replicaId.c_str(), pathOfVolume.c_str());
		}
	}

	return true;
}

bool ClusterCS::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{

	return true;
}

void ClusterCS::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath)
{
	freeMB = totalMB = 0;

	ContentStoreImpl::NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);

	if (nrqueue.empty())
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ClusterCS, "getStorageSpace() fail, no node replica available"));
		return;
	}

	for(; !nrqueue.empty() && totalMB <=0; nrqueue.pop())
	{
		const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try {
			::Ice::Long lFreeMB=0, lTotalMB=0;

			MOLOG(Log::L_DEBUG, CLOGFMT(ClusterCS, "to getStorageSpace() from replica[%s]"), replica.replicaId.c_str());
			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::checkedCast(replica.obj);
			nodeStore->getCapacity(lFreeMB, lTotalMB);
			freeMB = (uint32) lFreeMB; totalMB = (uint32) lTotalMB;

			MOLOG(Log::L_DEBUG, CLOGFMT(ClusterCS, "getStorageSpace() from replica[%s], freeMB=%d, totalMB=%d"), 
				replica.replicaId.c_str(), freeMB, totalMB);
			break;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "getStorageSpace() replica[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "getStorageSpace() replica[%s] caught exception[%s]"), replica.replicaId.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "getStorageSpace() replica[%s] caught unknown exception"), replica.replicaId.c_str());
		}
	}
}

bool ClusterCS::validateMainFileName(ContentStoreImpl& store, std::string& contentName, const std::string& contentType)
{
	std::string name = store.fixupPathname(store, contentName);
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

ContentStoreImpl::FileInfos ClusterCS::listMainFiles(ContentStoreImpl& store, const char* rootPath)
//    throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt)
{
	ContentStoreImpl::NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);

	// get volumename, foldername
	std::string volumeName, folderName;
	std::string contentName;
	std::string strTmp = std::string(rootPath) + LOGIC_FNSEPS + "xxx";
	store.chopPathname(strTmp, volumeName, folderName, contentName);

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "ContentStore require listMainFiles() on volume[%s] folder[%s], replica queue size(%d)"),
		volumeName.c_str(), folderName.c_str(), nrqueue.size());

	ContentStoreImpl::FileInfos filenames;
	for(; !nrqueue.empty(); nrqueue.pop())
	{
		const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;
		filenames.clear();

		try 
		{
			TianShanIce::StrValues metaDataNames;
			metaDataNames.push_back(SYS_PROP(StampLastUpdated));

			
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "ContentStore listMainFiles() on replica[%s]"), replica.replicaId.c_str());
			
			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::uncheckedCast(replica.obj);
			::TianShanIce::Storage::VolumePrx prxVol = nodeStore->openVolume(volumeName);
			::TianShanIce::Storage::FolderPrx prxFolder;
			if (folderName.empty())
			{
				prxFolder = prxVol;
			}
			else
			{
				prxFolder = prxVol->openSubFolder(folderName, false, 0);
			}
			::TianShanIce::Storage::ContentInfos contents = prxFolder->listContents(metaDataNames, "", -1);

			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "ContentStore listMainFiles() on volume[%s] folder[%s] found %d contents on replica[%s]"),
				volumeName.c_str(), folderName.c_str(), contents.size(), replica.replicaId.c_str());

			for (::TianShanIce::Storage::ContentInfos::iterator it = contents.begin(); it < contents.end(); it++)
			{
				ContentStoreImpl::FileInfo fi;
				fi.filename = it->name;
				fi.stampLastWrite = it->metaData[SYS_PROP(StampLastUpdated)];
				
				filenames.push_back(fi);
			}

			return filenames;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "listMainFiles() replica[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());			
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "listMainFiles() replica[%s] caught exception[%s]"), replica.replicaId.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "listMainFiles() replica[%s] caught unknown exception"), replica.replicaId.c_str());
		}
	}

	ZQTianShan::_IceThrow<TianShanIce::Storage::NoResourceException>(MOLOG, EXPFMT(ClusterCS, 0, "no node replica available for listMainFiles()"));
	return filenames;
}

std::string ClusterCS::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename)
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


bool ClusterCS::deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname)
{
	ContentStoreImpl::NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);

	std::string strVolumeName = content.identVolume.name;
	std::string strContentName = content._name();
	std::string strContentType = "";

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "Content[%s] deleteFileByContent(), replica queue size(%d)"), 
		strContentName.c_str(), nrqueue.size());

	int c =0;
	for(; !nrqueue.empty(); nrqueue.pop())
	{
		const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try 
		{			
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "destroy content[%s] on replica[%s] volume[%s]"), 
				strContentName.c_str(), replica.replicaId.c_str(), strVolumeName.c_str());

			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::uncheckedCast(replica.obj);
			::TianShanIce::Storage::VolumePrx volumePrx = nodeStore->openVolume(strVolumeName);
			::TianShanIce::Storage::ContentPrx content = volumePrx->openContent(strContentName, strContentType, false);

			if (content)
			{
				content->destroy2(true);
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "content[%s] destroyed on replica[%s]"), strContentName.c_str(), replica.replicaId.c_str());
			}

			c++;
		}
		catch(const ::Ice::ObjectNotExistException&)
		{
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "deleteByContent() on replica[%s] content[%s] caught ObjectNotExistException"), 
				replica.replicaId.c_str(), strContentName.c_str());

			c++;	//not exist also works 
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "deleteByContent() on replica[%s] content[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "deleteByContent() on replica[%s] content[%s] caught exception[%s]"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "deleteByContent() on replica[%s] content[%s] caught unknown exception"), strContentName.c_str(), replica.replicaId.c_str());
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

uint64 ClusterCS::checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname)
{
	ContentStoreImpl::NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);

	std::string strCheckRes = showCheckResidentialFlag(flagsToTest);
	std::string strContentType = "";
	std::string strVolumeName;
	std::string strContentName;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "Content[%s] checkResidentialStatus(%s), replica queue size(%d)"), 
		contentFullName.c_str(), strCheckRes.c_str(), nrqueue.size());

	if (pContent)
	{
		strVolumeName = pContent->identVolume.name;
		strContentName = pContent->_name();
	}
	else
	{
		//get the volume and content name from contentfull name
		std::string::size_type bpos = contentFullName.find_last_of(LOGIC_FNSEPS);
		if (bpos!=std::string::npos)
		{
			strContentName = contentFullName.substr(bpos +1);
			strVolumeName = contentFullName.substr(0, bpos);

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "checkResidentialStatus() identify the volume[%s] content[%s] from fullname[%s]"), 
				strVolumeName.c_str(), strContentName.c_str(), contentFullName.c_str());
		}
		else
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ClusterCS, "checkResidentialStatus() could not identify the volume and content from fullname[%s]"), 
				contentFullName.c_str());
			
			return RSDFLAG(frfAbsence);
		}
	}

	for(; !nrqueue.empty(); nrqueue.pop())
	{
		const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try 
		{
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "Content[%s] checkResidentialStatus() on replica[%s] volume[%s]"), 
				strContentName.c_str(), replica.replicaId.c_str(), strVolumeName.c_str());

			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::uncheckedCast(replica.obj);
			::TianShanIce::Storage::VolumePrx volumePrx = nodeStore->openVolume(strVolumeName);
			::TianShanIce::Storage::ContentPrx content = volumePrx->openContent(strContentName, strContentType, false);
			::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(content);

			if (!uniContent)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "Could not find content[%s] on replica[%s]"), 
					strContentName.c_str(), replica.replicaId.c_str());

				if (nrqueue.size()> 1)
					continue;

				return RSDFLAG(frfAbsence);		//not exist at all
			}
			
			// Ticket#11165 trust the slave NotProvisioned as non-residential and all member file absence
			::TianShanIce::Storage::ContentState slaveState = uniContent->getState();
			if (::TianShanIce::Storage::csNotProvisioned == slaveState)
				return RSDFLAG(frfAbsence);
			else if (::TianShanIce::Storage::csOutService == slaveState || ::TianShanIce::Storage::csCleaning == slaveState)
			{
				MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "checkResidentialStatus() replica[%s] content[%s]'s state[%s(%d)], yield for a while"),
					replica.replicaId.c_str(), strContentName.c_str(), ContentStateBase::stateStr(slaveState), slaveState);

#ifdef ZQ_OS_MSWIN
				::Sleep(OUTSERVICE_TIMEOUT >>1);
#else
				usleep((OUTSERVICE_TIMEOUT >>1)*1000);
#endif
				return 0;
			}
				
			uint64 flagsRet = uniContent->checkResidentialStatus(flagsToTest);
			std::string strRet = showCheckResidentialFlag(flagsRet);

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "Content[%s] checkResidentialStatus() on replica[%s] return [%s]"), 
				strContentName.c_str(), replica.replicaId.c_str(), strRet.c_str());

			return flagsRet;
		}
		catch(const ::Ice::ObjectNotExistException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "checkResidentialStatus() replica[%s] content[%s] caught exception[%s]"),
				replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str());

			if (nrqueue.size()> 1)
				continue;

			return RSDFLAG(frfAbsence);		//not exist at all
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "checkResidentialStatus() replica[%s] content[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "checkResidentialStatus() replica[%s] content[%s] caught exception[%s]"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "deleteByContent() replica[%s] content[%s] caught unknown exception"), strContentName.c_str(), replica.replicaId.c_str());
		}
	}

	ZQTianShan::_IceThrow<TianShanIce::ServerError>(MOLOG, EXPFMT(ClusterCS, 0, "Content[%s] could not checkResidentialStatus() at the moment"), contentFullName.c_str());

	return 0;
}

#ifdef _DEBUG
void dumpProperty(ContentStoreImpl& store,const std::string& content, const std::string& tag, const ::TianShanIce::Properties & props)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "======================= dump [%d] properties for content %s %s ===========================\n"), props.size(), content.c_str(), tag.c_str());
	
	::TianShanIce::Properties::const_iterator it = props.begin();
	for(;it!=props.end();it++)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "key [%s] = [%s]\n"), it->first.c_str(), it->second.c_str());
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

bool ClusterCS::populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname)
{
	std::string strVolumeName = content.identVolume.name;
	std::string strContentName = content._name();
	std::string strContentType = "";

	ContentStoreImpl::NodeReplicaQueue nrqueue;
	store.buildNodeReplicaQueue(nrqueue);
	int nNodeRepCount = nrqueue.size();

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "Content[%s] populateAttrsFromFile(), replica queue size(%d)"), 
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
		const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try 
		{
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "Content[%s] populateAttrsFromFile() on replica[%s]"), strContentName.c_str(), replica.replicaId.c_str());

			::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::uncheckedCast(replica.obj);
			::TianShanIce::Storage::VolumePrx volumePrx = nodeStore->openVolume(strVolumeName);
			::TianShanIce::Storage::ContentPrx cont = volumePrx->openContent(strContentName, strContentType, false);

			if (!cont)
			{
				continue;
			}

			::TianShanIce::Properties metaDatas = cont->getMetaData();
			::TianShanIce::Properties metaDataToNode = content.metaData;

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "Content[%s] found (%d) metadatas on replica[%s]"), strContentName.c_str(), 
				metaDatas.size(), replica.replicaId.c_str());

			for (::TianShanIce::Properties::const_iterator it = metaDatas.begin(); it!= metaDatas.end(); it++)
			{
				// we should take the system properties from node as the bible, but
				// take the user's properties from the cluster as bible
				static const int prefixLen = strlen(SYS_PROP_PREFIX);
				if (0 != it->first.substr(0, prefixLen).compare(SYS_PROP_PREFIX) || it->first == METADATA_ScheduledProvisonStart || it->first == METADATA_ScheduledProvisonEnd)
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
						int64 nSavedFileSize = ATOI64(itp->second.c_str());
						int64 nNodeFileSize = ATOI64(it->second.c_str());

						if (nNodeFileSize <= nSavedFileSize)
						{
							bUseMetaData = false;							
						}						
					}

					if (it->first == METADATA_PlayTime)
					{
						int64 nSavedPlayTime = ATOI64(itp->second.c_str());
						int64 nNodePlayTime = ATOI64(it->second.c_str());

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
					MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "Content[%s] setMetaData(%s)=%s"), 
						strContentName.c_str(), it->first.c_str(), it->second.c_str());
				}
				else
				{
					MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "Content[%s], the current MetaData(%s)=%s, skip MetaData(%s)=%s from node"), 
						strContentName.c_str(), it->first.c_str(), itp->second.c_str(), it->first.c_str(), it->second.c_str());
				}
			}

			// flush the user's metadata and node-unknown meta
			if (metaDataToNode.size() >0)
			{
				MOLOG(Log::L_DEBUG, CLOGFMT(ClusterCS, "setMetaData to replica[%s] content[%s]"), replica.replicaId.c_str(), strContentName.c_str());
				::TianShanIce::Storage::UnivContentPrx::checkedCast(cont)->setMetaData(metaDataToNode);
			}

#ifdef _DEBUG
			dumpProperty(store, contentName, "after", content.metaData);
#endif

			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "Content[%s] populateAttrsFromFile() on replica[%s] successful"), strContentName.c_str(), replica.replicaId.c_str());
			return true;		//verifyCriticalMetaData(content.metaData);
		}
		catch(const ::Ice::ObjectNotExistException&)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "populateAttrsFromFile() on replica[%s] content[%s] caught ObjectNotExistException"), 
				replica.replicaId.c_str(), strContentName.c_str());
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "populateAttrsFromFile() on replica[%s] content[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "populateAttrsFromFile() on replica[%s] content[%s] caught exception[%s]"), replica.replicaId.c_str(), strContentName.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "populateAttrsFromFile() on replica[%s] content[%s] caught unknown exception"), strContentName.c_str(), replica.replicaId.c_str());
		}
	}

	return false;
}

bool ClusterCS::completeRenaming(ContentStoreImpl& store, const ::std::string& oldName, const ::std::string& newName)
{
	return false;
}

std::string ClusterCS::fixupPathname(ContentStoreImpl& store, const std::string& pathname)
{
	return pathname;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ClusterCS::submitProvision(
            ContentStoreImpl& store, 
            ContentImpl& content, 
            const ::std::string& contentName,
		    const ::std::string& sourceUrl, 
            const ::std::string& sourceType, 
            const ::std::string& startTimeUTC, 
            const ::std::string& stopTimeUTC, 
            const int maxTransferBitrate)
//		    throw (::TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException,  
//                   ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
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

			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
				contentName.c_str(), it->first.c_str(), it->second.c_str());

			it = metaDatas.find(METADATA_ProviderId);
			if (it!=metaDatas.end())
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
					contentName.c_str(), it->first.c_str(), it->second.c_str());
			}

			it = metaDatas.find(METADATA_ProviderAssetId);
			if (it!=metaDatas.end())
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
					contentName.c_str(), it->first.c_str(), it->second.c_str());
			}

			it = metaDatas.find(METADATA_SubscriberId);
			if (it!=metaDatas.end())
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
					contentName.c_str(), it->first.c_str(), it->second.c_str());
			}
		}
	}

	::TianShanIce::Properties sessMdata;
	{
		::TianShanIce::Properties metaDatas = content.getMetaData(Ice::Current());
		::TianShanIce::Properties::const_iterator it = metaDatas.find(METADATA_IndexType);
		if (it!=metaDatas.end())
		{
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, it->second.c_str()));
		}
		else
		{
			if (stricmp(configGroup.mccsConfig.strDefaultIndexType.c_str(),"VVC") == 0)
				sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, "VVC"));
			else
				sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, "VVX"));
		}
		it = metaDatas.find(METADATA_ProviderId);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PROVIDERID, it->second));
		

		it = metaDatas.find(METADATA_ProviderAssetId);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PROVIDERASSETID, it->second));
				
		it = metaDatas.find(METADATA_AugmentationPIDs);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_AUGMENTATIONPIDS, it->second));
		
		it = metaDatas.find(METADATA_Preencryption);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PREENCRYPTION, it->second));

		it = metaDatas.find(METADATA_WishedTrickSpeeds);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_WISHEDTRICKSPEEDS, it->second));
	}

	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

#ifndef CDNCS_SERVICE
	//remove the first possible '\' or "\\"
	if (strFilePathName[0]=='\\' || strFilePathName[0]=='/')
		strFilePathName = strFilePathName.substr(1);
#endif

	::TianShanIce::Storage::ContentPrx	contentPrx;
	{
		// get the content proxy
		contentPrx = ::TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));
	}

	TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper::Ptr& cpWrapper = _ctx->cpWrapper;;

	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = cpWrapper->activeProvision(
		contentPrx,
		contentKey,
		strFilePathName,	
		sourceUrl,
		sourceType, 
		startTimeUTC,
		stopTimeUTC, 
		transferBitrate,
		sessMdata,
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

		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ClusterCS, "[%s] nPVR lead session copy [%s]"), 
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

	std::string strCPENetId = sessMdata["sys.CPE.NetId"];
	if (!strCPENetId.empty())
	{
		::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(contentPrx);
		TianShanIce::Properties metaData;
		metaData["sys.CPE.NetId"] = strCPENetId;
		try
		{
			uniContent->setMetaData(metaData);
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "[%s] set content metadata CPE netid[%s]"), 
				strFilePathName.c_str(), strCPENetId.c_str());
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

TianShanIce::ContentProvision::ProvisionSessionPrx ClusterCS::bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& contentName,
						::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
//			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
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

	ContentProvisionWrapper::Ptr& cpWrapper = _ctx->cpWrapper;;

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

std::string ClusterCS::getExportURL(ContentStoreImpl& store, ContentImpl& content, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params)
{
	ContentProvisionWrapper::Ptr& cpWrapper = _ctx->cpWrapper;;

	//const std::string& protocal, const std::string& filename, int transferBitrate, int& nTTL, int& permittedBitrate
	int transBitrate = transferBitrate;
	int nTTL = 0;
	int permittedBitrate = transferBitrate; // bug#16584, to initialize with the inputted transferBitrate

	// validate and determin the url per inputted protocol
	std::string strExposeUrl;
	
#pragma message ( __MSGLOC__ "TODO: change getExportUrl interface to add transfer bitrate etc.")
	if (!configGroup.mccsConfig.exportLocatorInterface.empty() && (transferProtocol.empty() || 0 == transferProtocol.compare("c2http")))
	{
		strExposeUrl = std::string("c2http://") + configGroup.mccsConfig.exportLocatorInterface + "/";
		nTTL = 10*60*1000;
	}
	else if (0 == transferProtocol.compare(TianShanIce::Storage::potoFTP))
	{
		strExposeUrl = cpWrapper->getExposeUrl(transferProtocol, contentkey, transBitrate, nTTL, permittedBitrate);
		std::string mainFilename  = "";
		::TianShanIce::Properties metadatas = content.getMetaData(Ice::Current());
		if (metadatas.find(SYS_PROP(MainFileExtentionName)) != metadatas.end())
		{
			strExposeUrl +=  metadatas[SYS_PROP(MainFileExtentionName)];
		}
		else if (metadatas.find(SYS_PROP(MainFileName)) != metadatas.end()) 
		{
			mainFilename = metadatas[SYS_PROP(MainFileName)];
			int npos = mainFilename.rfind('/');
			if(npos  >= 0)
				mainFilename = mainFilename.substr(npos + 1);

			npos = strExposeUrl.rfind('/');
			if(npos >= 0)
				strExposeUrl = strExposeUrl.substr(0, npos +1) + mainFilename;
		}
	}
	else ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(store._log, EXPFMT(MediaClusterCS, csexpUnsupportProto, "protocol[%s] not supported"), transferProtocol.c_str()	);

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


	MOLOG(ZQ::common::Log::L_DEBUG, LOGFMT("(%s) getExportURL [URL: (%s) ttl: (%d) timeWindowStart: (%s) timeWindowEnd: (%s) bitrate: (%d)"), 
		contentkey.content.c_str(), strExposeUrl.c_str(), ttl, stStart.c_str(), stEnd.c_str(), permittedBitrate);

	return strExposeUrl;
}

void ClusterCS::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
//    throw (::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	ContentProvisionWrapper::Ptr& cpWrapper = _ctx->cpWrapper;
	
	std::string contentName = content._name();
	cpWrapper->cancelProvision(contentName, provisionTaskPrx);
}

void ClusterCS::notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew)
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
	 			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "notifyReplicasChanged() try to sync with replica[%s][%s]"), replica.replicaId.c_str(), prxstr.c_str());
	 
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
				MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "notifyReplicasChanged() replica[%s] caught ObjectNotExistException"), replica.replicaId.c_str());
	 		}
	 		catch(const ::TianShanIce::BaseException& ex)
	 		{
	 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "notifyReplicasChanged() replica[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	 		}
	 		catch(const ::Ice::Exception& ex)
	 		{
	 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "notifyReplicasChanged() replica[%s] caught exception[%s]"), replica.replicaId.c_str(), ex.ice_name().c_str());
	 		}
	 		catch(...)
	 		{
	 			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ClusterCS, "notifyReplicasChanged() replica[%s] caught unknown exception"), replica.replicaId.c_str());
	 		}
		}
	}
}


}} // namespace
