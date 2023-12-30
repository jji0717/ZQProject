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

#include "ContentImpl.h"
#include "CPCImpl.h"
#include "ContentProvisionWrapper.h"
#include "ContentState.h"
#include "DispatchCSInterface.h"
#include "ClusterCS.h"
#include "NativeCS.h"
#include "MCCSCfg.h"

using namespace ::TianShanIce::Storage;
using namespace ::TianShanIce;
using namespace ZQ::common;

namespace ZQTianShan {
namespace ContentStore {

void ContentStoreImpl::initializePortal(ContentStoreImpl& store)
{
	if (NULL != store._ctxPortal)
		return;

	IDispatchCS* pCtx = NULL;
	if(configGroup.mccsConfig.rootPath.empty())
		pCtx = new (std::nothrow) ClusterCS;
	else
		pCtx = new (std::nothrow) NativeCS;

	store._ctxPortal = (void*) pCtx;
	if (NULL == store._ctxPortal)
	{
		store._log(Log::L_WARNING, CLOGFMT(ContentStoreImpl, "initializePortal() create portal failed, ctxPortal[NULL]"));
		return;
	}

	return pCtx->initializePortal(store);
}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store)
{
	if (NULL != store._ctxPortal)
	{
		IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
		store._ctxPortal = NULL;
		pCtx->uninitializePortal(store);
		delete pCtx;
	}	
}

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume,const std::string& volname)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->createPathOfVolume(store, pathOfVolume, volname);
}

bool ContentStoreImpl::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->deletePathOfVolume(store, pathOfVolume);
}

void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->getStorageSpace(store, freeMB,totalMB, rootPath);
}

bool ContentStoreImpl::validateMainFileName(ContentStoreImpl& store, std::string& contentName, const std::string& contentType)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->validateMainFileName(store, contentName, contentType);
}

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->listMainFiles(store, rootPath);
}

std::string ContentStoreImpl::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->memberFileNameToContentName(store, memberFilename);
}


bool ContentStoreImpl::deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->deleteFileByContent(store, content, mainFilePathname);
}

uint64 ContentStoreImpl::checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->checkResidentialStatus(store, flagsToTest, pContent, contentFullName, mainFilePathname);
}

bool ContentStoreImpl::populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->populateAttrsFromFile(store, content, mainFilePathname);
}

bool ContentStoreImpl::completeRenaming(ContentStoreImpl& store, const ::std::string& oldName, const ::std::string& newName)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->completeRenaming(store, oldName, newName);
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::submitProvision(
	ContentStoreImpl& store, 
	ContentImpl& content, 
	const ::std::string& contentName,
	const ::std::string& sourceUrl, 
	const ::std::string& sourceType, 
	const ::std::string& startTimeUTC, 
	const ::std::string& stopTimeUTC, 
	const int maxTransferBitrate)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->submitProvision(store, content, contentName, sourceUrl, sourceType, startTimeUTC, stopTimeUTC, maxTransferBitrate);
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& contentName,
	::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->bookPassiveProvision(store, content, contentName, pushUrl, sourceType, startTimeUTC, stopTimeUTC, maxTransferBitrate);
}

std::string ContentStoreImpl::getExportURL(ContentStoreImpl& store, ContentImpl& content, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->getExportURL(store, content, contentkey, transferProtocol, transferBitrate, ttl, params);
}

void ContentStoreImpl::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->cancelProvision(store, content, provisionTaskPrx);
}

void ContentStoreImpl::notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew)
{
	IDispatchCS* pCtx = (IDispatchCS*)(store._ctxPortal);
	return pCtx->notifyReplicasChanged(store, replicasOld, replicasNew);
}


}} // namespace
