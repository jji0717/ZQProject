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
// Ident : $Id: PathHelperMgr.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathHelperMgr.cpp $
// 
// 8     1/11/16 4:55p Dejian.fei
// 
// 7     6/18/15 11:11a Hui.shao
// ticket#17865 to export serviceGroup usage via csv
// 
// 6     8/21/14 4:20p Hui.shao
// 
// 6     8/21/14 4:18p Hui.shao
// 
// 5     3/18/11 1:48p Fei.huang
// 
// 4     3/17/11 4:23p Hongquan.zhang
// 
// 3     3/09/11 4:26p Hongquan.zhang
// 
// 2     3/07/11 4:54p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 38    08-03-18 14:50 Hongquan.zhang
// must lock servant when using it
// 
// 37    08-01-03 15:48 Hui.shao
// 
// 36    07-12-14 11:37 Hongquan.zhang
// Check in for updating ErrorCode
// 
// 35    07-10-17 12:21 Hongquan.zhang
// 
// 34    07-09-18 12:55 Hongquan.zhang
// 
// 32    07-08-30 15:39 Hongquan.zhang
// 
// 31    07-08-01 11:04 Hongquan.zhang
// 
// 30    07-06-06 16:07 Hongquan.zhang
// 
// 29    07-05-24 11:18 Hongquan.zhang
// 
// 28    07-03-21 16:06 Hui.shao
// added entry to commit path usage
// 
// 27    07-03-14 12:33 Hongquan.zhang
// 
// 26    07-03-07 14:47 Hongquan.zhang
// 
// 25    07-02-26 17:51 Hongquan.zhang
// 
// 24    07-01-12 12:08 Hongquan.zhang
// 
// 23    07-01-09 15:14 Hongquan.zhang
// 
// 22    06-12-28 16:45 Hongquan.zhang
// 
// 21    06-12-25 19:30 Hui.shao
// add warnning when pho overwrite prev registration
// 
// 20    06-12-25 16:58 Hui.shao
// fixed glog to envlog; _throw with envlog
// 
// 17    06-09-18 19:26 Hui.shao
// use plugin for PHO
// 
// 11    06-08-10 12:46 Hui.shao
// moved bandwidth into the privateData
// 
// 1     06-06-14 11:01 Hui.shao
// initial check in
// ===========================================================================

#include "PathHelperMgr.h"
#include "PathSvcEnv.h"
#include "PathManagerImpl.h"
#include "Log.h"
#include "FileSystemOp.h"
#include <TianShanIceHelper.h>

extern "C"
{
#ifdef ZQ_OS_MSWIN
	#include <io.h>
#endif
};

namespace ZQTianShan {
namespace AccreditedPath {

PathHelperMgr::PathHelperMgr(PathSvcEnv& env)
: _env(env)
{
}

PathHelperMgr::~PathHelperMgr()
{
	ZQ::common::MutexGuard sync(_lockPhoMap);
	for (PHOMap::iterator it = _phoMap.begin(); it != _phoMap.end(); it++)
	{
		if (!it->second)
			continue;

		try {
			PHOPluginFacet	phoFacet(*(it->second));
			phoFacet.UninitPHO();
		}catch(...) {}
		
		try{delete it->second;}catch (...) {}
	}
	_phoMap.clear();
}


IStorageLinkPHO* PathHelperMgr::registerStorageLinkHelper(const char* type, IStorageLinkPHO& helper, void* pCtx)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		PHONode node;
		node.pPHO = &helper; node.fnPlugin = (const char*) (NULL == pCtx ? "<Internal>" : pCtx);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(PathHelperMgr, "associate StorageLink type \"%s\" with PHO: \"%s\""), type, node.fnPlugin.c_str());
		ZQ::common::MutexGuard gd(_lockStorageLinkMap);
		_storageLinkPHOMap.insert(PathHelperObjMap::value_type(type, node));
	}
	
	return findStorageLinkHelper(type);
}

IStorageLinkPHO* PathHelperMgr::unregisterStorageLinkHelper(const char* type)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		ZQ::common::MutexGuard gd(_lockStorageLinkMap);
		PathHelperObjMap::iterator it = _storageLinkPHOMap.find(type);
		if (_storageLinkPHOMap.end() == it)
			return NULL;

		IStorageLinkPHO* ret = (IStorageLinkPHO*) it->second.pPHO;
		
		_storageLinkPHOMap.erase(it);
		return ret;
	}
}

IStreamLinkPHO* PathHelperMgr::registerStreamLinkHelper(const char* type, IStreamLinkPHO& helper, void* pCtx)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		PHONode node;
		node.pPHO = &helper; node.fnPlugin = (const char*) (NULL == pCtx ? "<Internal>" : pCtx);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(PathHelperMgr, "associate StreamLink type \"%s\" with PHO: \"%s\""), type, node.fnPlugin.c_str());
		ZQ::common::MutexGuard gd(_lockStreamLinkMap);
		if (_streamLinkPHOMap.end() != _streamLinkPHOMap.find(type))
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(PathHelperMgr, "Pre-associated StreamLink type \"%s\" is being overwritted by PHO: \"%s\""), type, node.fnPlugin.c_str());
		_streamLinkPHOMap.insert(PathHelperObjMap::value_type(type, node));
	}
	
	return findStreamLinkHelper(type);
}

IStreamLinkPHO* PathHelperMgr::unregisterStreamLinkHelper(const char* type)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		ZQ::common::MutexGuard gd(_lockStreamLinkMap);
		PathHelperObjMap::iterator it = _streamLinkPHOMap.find(type);
		if (_streamLinkPHOMap.end() == it)
			return NULL;

		IStreamLinkPHO* ret = (IStreamLinkPHO*) it->second.pPHO;
		
		_streamLinkPHOMap.erase(it);
		return ret;
	}
}

::TianShanIce::StrValues PathHelperMgr::listSupportedStorageLinkTypes()
{
	::TianShanIce::StrValues types;
	ZQ::common::MutexGuard gd(_lockStorageLinkMap);
	for (PathHelperObjMap::const_iterator it = _storageLinkPHOMap.begin(); it != _storageLinkPHOMap.end(); it++)
	{
		if (it->first.empty() || NULL == it->second.pPHO)
			continue;
		types.push_back(it->first);
	}

	return types;
}

::TianShanIce::StrValues PathHelperMgr::listSupportedStreamLinkTypes()
{
	::TianShanIce::StrValues types;
	ZQ::common::MutexGuard gd(_lockStreamLinkMap);
	for (PathHelperObjMap::const_iterator it = _streamLinkPHOMap.begin(); it != _streamLinkPHOMap.end(); it++)
	{
		if (it->first.empty() || NULL == it->second.pPHO)
			continue;
		types.push_back(it->first);
	}

	return types;
}

IStorageLinkPHO* PathHelperMgr::findStorageLinkHelper(const char* type)
{
	ZQ::common::MutexGuard gd(_lockStorageLinkMap);
	PathHelperObjMap::iterator it = _storageLinkPHOMap.find(type);
	if (_storageLinkPHOMap.end() == it)
		return NULL;
	
	return (IStorageLinkPHO*) it->second.pPHO;
}

IStreamLinkPHO* PathHelperMgr::findStreamLinkHelper(const char* type)
{
	ZQ::common::MutexGuard gd(_lockStreamLinkMap);
	PathHelperObjMap::iterator it = _streamLinkPHOMap.find(type);
	if (_streamLinkPHOMap.end() == it)
		return NULL;
	
	return (IStreamLinkPHO*) it->second.pPHO;
}

bool PathHelperMgr::getStorageLinkSchema(const ::std::string& type, TianShanIce::PDSchema& schema)
{
	IStorageLinkPHO* pHelper = findStorageLinkHelper(type.c_str());
	if (NULL == pHelper)
		return false;

	// call the helper to evaluate the link
	return pHelper->getSchema(type.c_str(), schema);
}

bool PathHelperMgr::getStreamLinkSchema(const ::std::string& type, TianShanIce::PDSchema& schema)
{
	IStreamLinkPHO* pHelper = findStreamLinkHelper(type.c_str());
	if (NULL == pHelper)
		return false;

	// call the helper to evaluate the link
	return pHelper->getSchema(type.c_str(), schema);
}

bool PathHelperMgr::getStorageLinkPHOInfo(const ::std::string& type, ::ZQ::common::DynSharedObj::ImageInfo& info)
{
	std::string filename;
	{
		ZQ::common::MutexGuard gd(_lockStorageLinkMap);
		PathHelperObjMap::iterator it = _storageLinkPHOMap.find(type);
		if (_storageLinkPHOMap.end() == it)
			return false;
		filename = it->second.fnPlugin;
	}
	{
		ZQ::common::MutexGuard gd(_lockPhoMap);
		PHOMap::iterator it = _phoMap.find(filename);
		memcpy(&info, it->second->getImageInfo(), sizeof(info));
	}
	return true;
}

bool PathHelperMgr::getStreamLinkPHOInfo(const ::std::string& type, ::ZQ::common::DynSharedObj::ImageInfo& info)
{
	std::string filename;
	{
		ZQ::common::MutexGuard gd(_lockStreamLinkMap);
		PathHelperObjMap::iterator it = _streamLinkPHOMap.find(type);
		if (_streamLinkPHOMap.end() == it)
			return false;
		
		filename = it->second.fnPlugin;
	}
	{
		ZQ::common::MutexGuard gd(_lockPhoMap);
		PHOMap::iterator it = _phoMap.find(filename);
		memcpy(&info, it->second->getImageInfo(), sizeof(info));
	}
	return true;
}

void PathHelperMgr::validateStorageLinkConfig(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	//add current service state into configPD value map
	Ice::Int iServiceState = _env._serviceState;
	ZQTianShan::Util::updateValueMapData( configPD, _SERVICE_STATE_KEY_, iServiceState );

	IStorageLinkPHO* pHelper = findStorageLinkHelper(type);
	if (NULL == pHelper)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"PathHelperMgr",1001, "no such type of link [%s] to validate the configruation", type);

	pHelper->validateConfiguration(type, identStr, configPD);
	configPD.erase( _SERVICE_STATE_KEY_ );
}

void PathHelperMgr::validateStreamLinkConfig(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	Ice::Int iServiceState = _env._serviceState;
	ZQTianShan::Util::updateValueMapData( configPD, _SERVICE_STATE_KEY_, iServiceState );

	IStreamLinkPHO* pHelper = findStreamLinkHelper(type);
	if (NULL == pHelper)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"PathHelperMgr",1011,"no such type of link [%s] to validate the configruation", type);

	pHelper->validateConfiguration(type, identStr, configPD);
	configPD.erase( _SERVICE_STATE_KEY_ );
}

Ice::Int PathHelperMgr::doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost)
{
	if (!linkInfo.linkPrx || linkInfo.linkType.empty())
		return oldCost;

	// address the IPathHelperObject object
	std::string& linktype = linkInfo.linkType;
	IPathHelperObject* pHelper = NULL;

	if (linkInfo.isStreamLink)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathHelperMgr, "doEvaluation() look for \"%s\" helper for StreamLink[%s]"), linktype.c_str(), linkInfo.linkIden.name.c_str());
		pHelper = findStreamLinkHelper(linktype.c_str());
	}
	else
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathHelperMgr, "doEvaluation() look for \"%s\" helper for StorageLink[%s]"), linktype.c_str(), linkInfo.linkIden.name.c_str());
		pHelper = findStorageLinkHelper(linktype.c_str());
	}

	if (NULL == pHelper)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr, "doEvaluation() failed to find the helper for type \"%s\", return OutOfServiceCost"), linktype.c_str());
		return TianShanIce::Transport::OutOfServiceCost;
	}

	// call the helper to evaluate the link
	return pHelper->doEvaluation(linkInfo, sessCtx, hintPD, oldCost);
}

IdentCollection PathHelperMgr::listPathTicketsByLink(TianShanIce::Transport::StorageLinkPrx& link)
{
	IdentCollection ticketIds = _env._idxStorageLinkToTicket->find(link->getIdent());
	return ticketIds;
}

IdentCollection PathHelperMgr::listPathTicketsByLink(TianShanIce::Transport::StreamLinkPrx& link)
{
	IdentCollection ticketIds = _env._idxStreamLinkToTicket->find(link->getIdent());
	return ticketIds;
}

TianShanIce::Transport::PathTicketPrx PathHelperMgr::openPathTicket(::Ice::Identity ident)
{
	return IdentityToObj(PathTicket, ident);
}

TianShanIce::Transport::StorageLinkPrx PathHelperMgr::openStorageLink(::Ice::Identity ident)
{
	return IdentityToObj(StorageLink, ident);
}

TianShanIce::Transport::StreamLinkPrx PathHelperMgr::openStreamLink(::Ice::Identity ident)
{
	return IdentityToObj(StreamLink, ident);
}

TianShanIce::Transport::PathTicketPrx PathHelperMgr::openPathTicket(const char* name)
{
	Ice::Identity ident;
	ident.name=name;
	ident.category=DBFILENAME_PathTicket;
	return IdentityToObj(PathTicket, ident);
}

TianShanIce::Transport::StorageLinkPrx PathHelperMgr::openStorageLink(const char* name)
{
	Ice::Identity ident;
	ident.name=name;
	ident.category=DBFILENAME_StorageLink;
	return IdentityToObj(StorageLink, ident);
}

TianShanIce::Transport::StreamLinkPrx PathHelperMgr::openStreamLink(const char* name)
{
	Ice::Identity ident;
	ident.name=name;
	ident.category=DBFILENAME_StreamLink;
	return IdentityToObj(StreamLink, ident);
}


typedef std::vector<IPathHelperObject* > PHOStack;


bool PathHelperMgr::doNarrow(const TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{
	if (!ticket)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr, "doNarrow() null ticket"));
		return false;
	}

	// step 1. let's start with the IPathHelperObject object of the StorageLink and StreamLink bound on this ticket
	PHOStack narrowStack;

	IPathHelperObject* pHelper = NULL;
	try 
	{
		TianShanIce::Transport::StreamLinkPrx strmLink = ticket->getStreamLink();
		if (strmLink)
			pHelper =  findStreamLinkHelper(strmLink->getType().c_str());
		
		if (NULL != pHelper)
			narrowStack.push_back(pHelper);
	}
	catch (const TianShanIce::BaseException&) 
	{
	}
	catch (const Ice::Exception& ex) 
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr, "doNarrow() ice expcetion [%s] when find streamlink helper"),ex.ice_name().c_str());
	}
	catch(...) 
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr, "doNarrow() unknown expcetion when find streamlink helper"));
	}
	
	try 
	{
		pHelper = NULL;
		TianShanIce::Transport::StorageLinkPrx storeLink = ticket->getStorageLink();
		if (storeLink)
			pHelper =  findStorageLinkHelper(storeLink->getType().c_str());
		
		if (NULL != pHelper)
			narrowStack.push_back(pHelper);
	}
	catch(...) {}

	IPathHelperObject::NarrowResult result = IPathHelperObject::NR_Unrecognized;

	for (PHOStack::iterator it= narrowStack.begin(); it < narrowStack.end(); it++)
	{
		if (NULL == (*it))
			continue;

		try 
		{

#if ICE_INT_VERSION / 100 >= 306
			WLockT<ADPathTicketImpl> gd( *( ( ADPathTicketImpl*) ticket.get()));
#else
			IceUtil::WLockT<ADPathTicketImpl> gd( *( ( ADPathTicketImpl*) ticket.get()));
#endif
			IPathHelperObject::NarrowResult ret = (*it)->doNarrow(ticket, sessCtx);
			if (IPathHelperObject::NR_Unrecognized != ret)
				result = ret;
		}
		catch(...) 
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(PathHelperMgr,"doNarrow() unexpect error when doNarrow"));
			continue; 
		}

		if(result==IPathHelperObject::NR_Completed || result==IPathHelperObject::NR_Narrowed )
		{
//			ticket->stampComitted=now();
		}
		
		if (result >= IPathHelperObject::NR_Completed)
			break;

#pragma message ( __MSGLOC__ "TODO: retry narrow magic")
//		if (NR_RetryLater == result && narrowStack.size() < MAX_NARROW_STEP)
//			narrowStack.push_back(pHelper);

	}
	
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(PathHelperMgr, "doNarrow() end of narrow stack: size=%d, result=%d"), narrowStack.size(), result);
	narrowStack.clear();

	if (IPathHelperObject::NR_Completed == result || IPathHelperObject::NR_Narrowed == result)
		return true;

	if (IPathHelperObject::NR_Error == result)
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog,"PathHelperMgr",1021, "PathHelperMgr::doNarrow(%s) error occured, please refer to the PathHelper object", ticket->ident.name.c_str());

	if (IPathHelperObject::NR_RetryLater == result)
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog,"PathHelperMgr",1022, "PathHelperMgr::doNarrow(%s) exceed the max narrow stack", ticket->ident.name.c_str());

	if (IPathHelperObject::NR_Unrecognized == result)
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog,"PathHelperMgr",1023, "PathHelperMgr::doNarrow(%s) no PathHelper is able to narrow the ticket", ticket->ident.name.c_str());

	return false;
}

bool PathHelperMgr::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{
	if (!ticket)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr, "doCommit() null ticket"));
		return false;
	}

	IPathHelperObject* pHelper = NULL;
	Ice::Int svcGrp = -1;
	std::string ticketId;
	try 
	{
		TianShanIce::Transport::StreamLinkPrx strmLink = ticket->getStreamLink();
		ticketId = ticket->getIdent().name;
		if (strmLink)
		{
			svcGrp  = strmLink->getServiceGroupId();
			pHelper = findStreamLinkHelper(strmLink->getType().c_str());
		}
		
		if (NULL != pHelper)
		{
#if ICE_INT_VERSION / 100 >= 306
			WLockT<ADPathTicketImpl> gd( *((ADPathTicketImpl*)ticket.get()));
#else
			IceUtil::WLockT<ADPathTicketImpl> gd( *((ADPathTicketImpl*)ticket.get()));
#endif
			pHelper->doCommit(ticket, sessCtx);
		}
	}
	catch(...) {}
	
	try 
	{
		pHelper = NULL;
		TianShanIce::Transport::StorageLinkPrx storeLink = ticket->getStorageLink();
		if (storeLink)
			pHelper =  findStorageLinkHelper(storeLink->getType().c_str());
		
		{			
#if ICE_INT_VERSION / 100 >= 306
			WLockT<ZQTianShan::AccreditedPath::ADPathTicketImpl> gd( *((ZQTianShan::AccreditedPath::ADPathTicketImpl*)ticket.get()));
#else
			IceUtil::WLockT<ZQTianShan::AccreditedPath::ADPathTicketImpl> gd( *((ZQTianShan::AccreditedPath::ADPathTicketImpl*)ticket.get()));
#endif
			if (NULL != pHelper)
			{
				pHelper->doCommit(ticket, sessCtx);
			}
			
			ticket->stampComitted =now();
		}
	}
	catch(...) {}

	try {
		if (svcGrp >0 && ticket->stampComitted >0)
		{
			Ice::Long bandwidth =0;
			ZQTianShan::Util::getResourceDataWithDefault(ticket->resources, TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth", 0, bandwidth);
			_env.commitUsage(svcGrp, bandwidth);
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathHelperMgr, "doCommit() ticket[%s] committed serviceGroup[%d] usage [%lld]bps"), ticketId.c_str(), svcGrp, bandwidth);
		}
	}
	catch(...) {}

	return true;
}

void PathHelperMgr::setExtraData(const char* configFile,const char* logFolder)
{
	if(configFile&&strlen(configFile)>0)
	{
		_strConfigFile=configFile;
	}
	else
	{
		_strConfigFile="";
	}
	if(logFolder&&strlen(logFolder)>0)
	{
		_strLogFolder=logFolder;
	}
	else
	{
		_strLogFolder="";
	}
}

int PathHelperMgr::populate(const char* pathToPHO)
{
	if (NULL == pathToPHO)
		return _phoMap.size();


	std::string wkpath = pathToPHO;
	std::string testwkpath =wkpath ;
	std::string searchpath = ::getenv("PATH");
	searchpath +=PHSEPS;

#ifdef WIN32
	// in windows, the pathnames are case-insensitive, make all lower case before comparing
	std::transform(testwkpath.begin(), testwkpath.end(), testwkpath.begin(), (int(*)(int)) tolower);
	std::transform(searchpath.begin(), searchpath.end(), searchpath.begin(), (int(*)(int)) tolower);
#endif

	int pos =searchpath.find(testwkpath);
	if (pos<0)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathHelperMgr, "populate() append %s into evironment variable PATH"), wkpath.c_str());
		searchpath = "PATH=";
		searchpath += ::getenv("PATH");
		searchpath += PHSEPS;
		searchpath += wkpath;
		::putenv(const_cast<char*>(searchpath.c_str()));
	}

	if (wkpath[wkpath.length()-1] != FNSEPC)
		wkpath +=FNSEPS;

	std::vector<std::string> result = FS::searchFiles(wkpath, PHO_FILENAME_PREFIX "*" PHO_EXT);	
	std::vector<std::string>::iterator file = result.begin();
	for(; file != result.end(); ++file) {
		loadPHO(file->c_str());
	}

	return _phoMap.size();
}

ZQ::common::DynSharedObj* PathHelperMgr::loadPHO(const char* filename)
{
	if (NULL == filename)
		return NULL;

	ZQ::common::DynSharedObj* pDHO = new ZQ::common::DynSharedObj(filename);

	if (NULL == pDHO)
		return NULL;

	ZQ::common::MutexGuard sync(_lockPhoMap);
	PHOMap::iterator it = _phoMap.find(pDHO->getImageInfo()->filename);
	if (_phoMap.end() != it)
	{
		delete pDHO;
		return it->second;
	}
	
	bool bValidPHO = false;
	try
	{
		PHOPluginFacet	phoFacet(*pDHO);
		bValidPHO = phoFacet.isValid();
		if (bValidPHO)
		{
			phoFacet.InitPHO(*this, (void*)pDHO->getImageInfo()->filename,_strConfigFile.c_str(),_strLogFolder.c_str());
		}
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr, "loadPHO() failed to load %s"), pDHO->getImageInfo()->filename);
		delete pDHO;
		return NULL;
	}

	if (!bValidPHO)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr, "loadPHO() invalid PHO: %s"), pDHO->getImageInfo()->filename);
		delete pDHO;
		return NULL;
	}

	_phoMap[pDHO->getImageInfo()->filename] = pDHO;
	return pDHO;
}

void PathHelperMgr::doFreeResources(const TianShanIce::Transport::PathTicketPtr& ticket)
{
	if (!ticket)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr, "doFreeResource() null ticket"));
		return ;
	}

	// step 1. let's start with the IPathHelperObject object of the StorageLink and StreamLink bound on this ticket
	PHOStack freeStack;

	IPathHelperObject* pHelper = NULL;
	Ice::Int svcGrp = -1;

	try {
		TianShanIce::Transport::StreamLinkPrx strmLink = ticket->getStreamLink();
		if (strmLink)
		{
			svcGrp  = strmLink->getServiceGroupId();
			pHelper =  findStreamLinkHelper(strmLink->getType().c_str());
		}
		
		if (NULL != pHelper)
			freeStack.push_back(pHelper);			
		else
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(PathHelperMgr, "failed to associate PHO to free resource of ticket[%s] with strmlink[%s]"),
				ticket->ident.name.c_str() , strmLink->getType().c_str() );
		}
	}
	catch(...) {}
	
	try {
		pHelper = NULL;
		TianShanIce::Transport::StorageLinkPrx storeLink = ticket->getStorageLink();
		if (storeLink)
			pHelper =  findStorageLinkHelper(storeLink->getType().c_str());
		
		if (NULL != pHelper)
			freeStack.push_back(pHelper);
		else
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(PathHelperMgr, "failed to associate pho to free resource of ticket[%s] with storelink[%s]"),
				ticket->ident.name.c_str() , storeLink->getType().c_str() );
		}
	}
	catch(...) {}	

	if( freeStack.size() <= 0 )
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr, "no pho is found to free resource of ticket[%s]"), ticket->ident.name.c_str());
	else
	{
		for (PHOStack::iterator it= freeStack.begin(); it < freeStack.end(); it++)
		{
			if (NULL == (*it))
				continue;
			
			try 
			{
#if ICE_INT_VERSION / 100 >= 306
				WLockT<ADPathTicketImpl> gd( *((ADPathTicketImpl*)ticket.get()));
#else
				IceUtil::WLockT<ADPathTicketImpl> gd( *((ADPathTicketImpl*)ticket.get()));
#endif
				(*it)->doFreeResources(ticket);			
			}
			catch(...) 
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PathHelperMgr,"error occured when FreeResource for ticket[%s]"),ticket->ident.name.c_str() );
				continue; 
			}
		}	
//		try
//		{
//			ticket->privateData.clear();
//			ticket->resources.clear();
//		}
//		catch(...)
//		{
//		}
	}

	try {
		if (svcGrp >0 && ticket->stampComitted >0)
		{
			Ice::Long bandwidth =0;
			ZQTianShan::Util::getResourceDataWithDefault(ticket->resources, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", 0, bandwidth);
			_env.withdrawUsage(svcGrp, bandwidth);
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathHelperMgr, "doFreeResources() ticket[%s] withdrawn serviceGroup[%d] usage [%lld]bps"), ticket->ident.name.c_str(), svcGrp, bandwidth);
		}
	}
	catch(...) {}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(PathHelperMgr, "doFreeResource() end "));

	freeStack.clear();
}


}} // namespace
