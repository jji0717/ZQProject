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
// Ident : $Id: cachepNSS.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/BaseCS/cachepNSS_draft.cpp $
// 
// 23    7/11/13 11:49a Build
// 
// 22    9/18/12 2:25p Li.huang
// 
// 21    7/25/12 4:01p Li.huang
// 
// 20    7/06/12 12:41p Li.huang
// 
// 19    6/28/12 12:36p Li.huang
// 
// 18    6/26/12 2:41p Li.huang
// 
// 17    6/19/12 2:26p Hui.shao
// 
// 16    6/15/12 11:53a Li.huang
// 
// 15    6/14/12 5:34p Li.huang
// 
// 14    6/14/12 5:16p Hui.shao
// 
// 13    6/08/12 10:22a Li.huang
// 
// 12    6/01/12 5:14p Li.huang
// 
// 11    6/01/12 3:08p Li.huang
// 
// 10    4/27/12 6:57p Hui.shao
// 
// 9     4/27/12 3:40p Li.huang
// 
// 8     4/27/12 1:53p Li.huang
// 
// 7     4/17/12 1:16p Li.huang
// 
// 6     4/16/12 4:12p Hui.shao
// 
// 5     4/09/12 2:34p Hui.shao
// 
// 4     1/17/12 11:36a Hui.shao
// 
// 3     1/06/12 2:16p Li.huang
// modify portalLocateContent function
// 
// 2     1/06/12 11:55a Hui.shao
// 
// 1     1/06/12 10:46a Hui.shao
// created, including HuangLi's impl of locateContent()
// ===========================================================================

#include "CacheStoreImpl.h"
#include "Guid.h"
#include "ContentUser.h"
#include "../../CPE/common/C2HttpClient.h"
#include "../../CPE/common/ParseIndexFile.h"

#include "urlstr.h"
#include "CDNDefines.h"
using namespace ZQTianShan::ContentProvision;

namespace ZQTianShan {
namespace ContentStore {

#define storelog (store._log)

void CacheStoreImpl::initializePortal(CacheStoreImpl& store)
{
#pragma message ( __TODO__ "impl here")
}

void CacheStoreImpl::uninitializePortal(CacheStoreImpl& store)
{
#pragma message ( __TODO__ "impl here")
}

std::string CacheStoreImpl::portalSubfileToFileExtname(const CacheStoreImpl& store, const std::string& subfile)
{
	std::string extName = subfile;

	size_t pos = subfile.find_first_not_of(". \t");
	if (pos >=0)
		extName = subfile.substr(pos);

	if (0 == extName.compare("forward/1"))
		extName ="";

	return extName;
}

int CacheStoreImpl::portalContentNameToken(const CacheStoreImpl& store, const std::string& contentName, TianShanIce::Properties& contentNameField)
{
	size_t pos = contentName.find_last_of("\\/");
	std::string paid =  (std::string::npos == pos) ? contentName : contentName.substr(pos+1);
	std::string pid;

	ZQ::common::Guid guid(paid.c_str());

	if (guid.isNil() && paid.length() > store._paidLength)
	{
		pid  = paid.substr(store._paidLength);
		paid = paid.substr(0, store._paidLength);

		pos = pid.find_first_not_of(".-_ \t@");
		if (std::string::npos == pos)
			pid = "";
		else if (0 != pos)
			pid = pid.substr(pos);
	}

	MAPSET(TianShanIce::Properties, contentNameField, METADATA_ProviderAssetId, paid);
	MAPSET(TianShanIce::Properties, contentNameField, METADATA_ProviderId,  pid);
	return contentNameField.size();
}

// A common function called by CacheStoreImpl::portalBookTransferSession() or CacheStoreImpl::portalLocateContent()
// up to the portal's local context about the C2 client, such as the c2client instance and transferInfo struct
static int LocateSourceSessionEx(const CacheStoreImpl& store, const std::string& pid, const std::string& paid, const std::string& subFile, const std::string& extSessionInterface, long transferBitrate, ZQTianShan::ContentProvision::LocateResponse& locateResponse)
{
	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreImpl, "LocateSourceSessionEx()"));
	
	int64 stampStart = ZQ::common::now();

	ZQ::common::URLStr url(extSessionInterface.c_str());
	std::string proto = url.getProtocol();
	if (0 == proto.compare("c2http")) // convert the c2http:// to http:// to pass into HttpClient
		url.setProtocol("http");
	else if (0 != proto.compare("http"))
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "LocateSourceSessionEx() unsupported import protocol[%s]"), proto.c_str());
		return 400;
	}

	url.setPath("vodadi.cgi");

	ZQTianShan::ContentProvision::LocateRequest locateRequest;

	locateRequest.pid  = pid;
	locateRequest.paid = paid;
	locateRequest.subFile = subFile;
	locateRequest.beginPos = -1;
	locateRequest.endPos = -1;
	locateRequest.bitRate = transferBitrate;
	locateRequest.transferDelay = 0;

	std::string locateUrl = url.generate();

	/// need configration following parameter
	std::string locateBindIp = "";
	std::string transferFileBindIp = "";

	ZQTianShan::ContentProvision::C2HttpClient* pC2HttpClient = new C2HttpClient(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE, &storelog , 20, locateBindIp, transferFileBindIp);
    std::auto_ptr<ZQTianShan::ContentProvision::C2HttpClient>	pHttpDownloader;
	pHttpDownloader.reset(pC2HttpClient);
	pHttpDownloader->setIngressCapacity(store._totalProvisionKbps * 1000);
	pHttpDownloader->setPIdAndPaid(pid, paid);
	pHttpDownloader->setSubfile(subFile);

	if (!pHttpDownloader->prepareLocateFile(locateUrl, locateRequest, locateResponse))
	{
		int nRetCode;
		std::string errMsg;
		pHttpDownloader->getLastErrorMsg(nRetCode, errMsg);
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "LocateSourceSessionEx() failed to prepare locate index file with error:(%d) %s"), nRetCode, errMsg.c_str());
		return nRetCode;
	}
	return 0;
}

int CacheStoreImpl::portalBookTransferSession(const CacheStoreImpl& store, std::string& sessionURL, const TianShanIce::Properties& contentNameToken, const std::string& subFile, const std::string& extSessionInterface, long transferBitrate, ::TianShanIce::Properties& params)
{
	int64 stampStart = ZQ::common::now();

	/// get ProvideId ProvideAssetId
	std::string strProvideId, strProvideAssetId;
	TianShanIce::Properties::const_iterator itorNameField;

	itorNameField = contentNameToken.find(METADATA_ProviderId);
	if(itorNameField ==contentNameToken.end() && itorNameField->second.empty())
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "LocateSourceSessionEx() failed to get parameter %s"), METADATA_ProviderId);
		return 400;
	}
	strProvideId = itorNameField->second;

	itorNameField = contentNameToken.find(METADATA_ProviderAssetId);
	if (itorNameField == contentNameToken.end() && itorNameField->second.empty())
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "LocateSourceSessionEx() failed to get parameter %s"),METADATA_ProviderAssetId );
		return 400;
	}

	strProvideAssetId = itorNameField->second;

    ZQTianShan::ContentProvision::LocateResponse locateResponse;
    int nRetCode = LocateSourceSessionEx(store, strProvideId, strProvideAssetId, subFile, extSessionInterface, transferBitrate, locateResponse);
	if(nRetCode > 0)
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "failed to BookTransferSession(errorcode:%d)"), nRetCode);
		return nRetCode;
	}

	char urlPort[65] = "";
	snprintf(urlPort, sizeof(urlPort) -1, "%u", store._defaultTransferServerPort);

	if (locateResponse.transferportnum.length() <= 0)
		locateResponse.transferportnum = urlPort;
	
	char buf[256] = "";
	snprintf(buf, sizeof(buf)-2, "http://%s:%s/%s", locateResponse.transferHost.c_str(), locateResponse.transferportnum.c_str(), locateResponse.transferId.c_str());
	sessionURL = buf;

	//cdn.transferId, cdn.transferPort, cdn.transferTimeout, cdn.availableRange, cdn.openForWrite
	char strTimeout[33];
	memset(strTimeout, 0, sizeof(strTimeout));
	itoa(locateResponse.transferTimeout, strTimeout,10);
	MAPSET(TianShanIce::Properties, params, CDN_TRANSFERID , locateResponse.transferId);
	MAPSET(TianShanIce::Properties, params, CDN_TRANSFERPORT, locateResponse.transferHost);
	MAPSET(TianShanIce::Properties, params, CDN_TRANSFERTIMEOUT, std::string(strTimeout));
	MAPSET(TianShanIce::Properties, params, CDN_AVAILRANGE, locateResponse.availableRange);
	MAPSET(TianShanIce::Properties, params, CDN_OPENFORWRITE, locateResponse.OpenForWrite == true ? "yes" : "no");
	MAPSET(TianShanIce::Properties, params, CDN_TRANSFERPORTNUM, locateResponse.transferportnum);

	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreImpl, "Book TransferSession for subFile[%s] sessionURL[%s] took %dms"), subFile.c_str(),sessionURL.c_str(), (int)(ZQ::common::now() -  stampStart));

	return 0;
}


int CacheStoreImpl::portalLocateContent(const CacheStoreImpl& store, const std::string& extSessionInterface, CacheTaskImpl& cacheTask)
{
	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreImpl, "portalLocateContent() content[%s]"), cacheTask.ident.name.c_str());

	int64 stampStart = ZQ::common::now();

	//init tempDir for save index file
	std::string tempDir = store._cacheDir ; ///temp directory to save index file

#ifdef ZQ_OS_MSWIN
	if(tempDir.empty())
		tempDir = "c:\\temp\\"; ///temp directory to save index file
	if(tempDir.size() > 0 && tempDir[tempDir.size() -1] != '\\')
		tempDir += "\\";
#else
	if(tempDir.empty())
		tempDir = "/tmp/"; ///temp directory to save index file
	if(tempDir.size() > 0 && tempDir[tempDir.size() -1] != '/')
		tempDir += "/";
#endif

	/// get ProvideId ProvideAssetId
	std::string strProvideId, strProvideAssetId;
	TianShanIce::Properties::const_iterator itorNameField;

	itorNameField = cacheTask.nameFields.find(METADATA_ProviderId);
	if(itorNameField == cacheTask.nameFields.end() && itorNameField->second.empty())
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to get parameter 'ProvideId'"), cacheTask.ident.name.c_str());
		return 400;
	}
	strProvideId = itorNameField->second;

	itorNameField = cacheTask.nameFields.find(METADATA_ProviderAssetId);
	if (itorNameField == cacheTask.nameFields.end() && itorNameField->second.empty())
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to get parameter 'ProvideAssetId'"), cacheTask.ident.name.c_str());
		return 400;
	}

	strProvideAssetId = itorNameField->second;

   ///book Book TransferSession
	ZQTianShan::ContentProvision::LocateResponse locateResponse;

	int nRetCode = LocateSourceSessionEx(store, strProvideId, strProvideAssetId, "index", extSessionInterface, cacheTask.bwMax, locateResponse);
	if(nRetCode > 0)
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to BookTransferSession(errorcode:%d)"), cacheTask.ident.name.c_str(), nRetCode);
		return nRetCode;
	}

	cacheTask.isSrcPWE = locateResponse.OpenForWrite;
	cacheTask.bwCommitted =  cacheTask.bwMax;

	//delete transfer url
	ZQ::common::URLStr url(extSessionInterface.c_str());
	std::string proto = url.getProtocol();
	if (0 == proto.compare("c2http")) // convert the c2http:// to http:// to pass into HttpClient
		url.setProtocol("http");

	std::string deleteURL(url.generate());

	///organize transfer URL
	char urlPort[65]="";
	snprintf(urlPort, sizeof(urlPort) -1, "%u", store._defaultTransferServerPort);

	if (locateResponse.transferportnum.length() <= 0)
		locateResponse.transferportnum = urlPort;

	char buf[256] = "";
	snprintf(buf, sizeof(buf)-2, "http://%s:%s/%s", locateResponse.transferHost.c_str(), locateResponse.transferportnum.c_str(), locateResponse.transferId.c_str());
	std::string transferURL = buf;

	/// need configration following parameter
	std::string locateBindIp= "";
	std::string transferFileBindIp = "";

	ZQTianShan::ContentProvision::C2HttpClient* pC2HttpClient = new C2HttpClient(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE, &storelog , 20, locateBindIp, transferFileBindIp);
	std::auto_ptr<ZQTianShan::ContentProvision::C2HttpClient>	pHttpDownloader;
	pHttpDownloader.reset(pC2HttpClient);
	pHttpDownloader->setIngressCapacity(store._totalProvisionKbps * 1000);
	pHttpDownloader->setPIdAndPaid(strProvideId, strProvideAssetId);
	pHttpDownloader->setSubfile("index");

		std::string indexFilename = tempDir + strProvideAssetId + strProvideId +".index";

	int64 maxLen = -1;
	if(locateResponse.OpenForWrite)
	{
		int64 nBeginPos = 0;
		int64 nEndPos = 0;

		if(pHttpDownloader->parserAavailableRange(locateResponse.availableRange, nBeginPos, nEndPos))
			maxLen = nEndPos;
		else
			maxLen = 65536;
	}
	int64 byteRecved = pHttpDownloader->downloadFile(transferURL, indexFilename, 0, maxLen,false);

	if(byteRecved < 0)
	{
		int nRetCode;
		std::string errMsg;
		pHttpDownloader->getLastErrorMsg(nRetCode, errMsg);
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s]failed to download index file with error: (%d)%s"), cacheTask.ident.name.c_str(), nRetCode, errMsg.c_str());
		
		pHttpDownloader->deleteTransferId(deleteURL, locateResponse.transferId, locateResponse.transferHost);

		return nRetCode;
		}

		pHttpDownloader->deleteTransferId(deleteURL, locateResponse.transferId, locateResponse.transferHost);

		//get IndexFile extension
		int npos = indexFilename.rfind('.');
		if(npos <  0)
		{
			storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s]failed to get index file %s extension"), cacheTask.ident.name.c_str(), indexFilename.c_str());
		return 400;
		}

		std::string strIndexExt = indexFilename.substr(npos + 1);

	///parse IndexFile
		std::vector<SubFileInfo> subFiles;
		ZQTianShan::ContentProvision::MediaInfo				 mediaInfo;
		if(!ParseIndexFileInfo::getIdxSubFileInfo(indexFilename.c_str(), strIndexExt, subFiles, mediaInfo))
		{
			//failed to parse the index file
			storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to parse VVX/VV2/INDEX index file"), cacheTask.ident.name.c_str());

		return 400;
		}

/*	cacheTask.fileSizeMB = 0;
	for(std::vector<SubFileInfo>::iterator itor = subFiles.begin(); itor != subFiles.end(); itor++)
	{
		if(itor->finalOffset > 0)
			cacheTask.fileSizeMB += itor->finalOffset  -  itor->firstOffset + 1;
	}

	cacheTask.fileSizeMB +=1024*1024-1;
	cacheTask.fileSizeMB >>=20;*/

	if(!subFiles.empty())
	{
		cacheTask.startOffset = subFiles[0].firstOffset;
		cacheTask.endOffset = subFiles[0].finalOffset;
	}

   
   if(locateResponse.OpenForWrite)
	{
		cacheTask.bwCommitted = mediaInfo.bitrate * 1.1; //bps;
	}

		remove(indexFilename.c_str());

	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreImpl, "[%s]completed portal Locate Content took %dms"), cacheTask.ident.name.c_str(), (int)(ZQ::common::now() - stampStart));

#pragma message ( __TODO__ "the test portal simply set cacheTask.urlSrcStream = extSessionInterface, should be corrected")
	cacheTask.urlSrcStream = extSessionInterface ;//+ cacheTask.ident.name;
	return 0;
	return true;
}

}} ///end namespace ZQTianShan::ContentStore
