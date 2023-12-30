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
// Ident : $Id: PathHelperMgr.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathHelperMgr.h $
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
// 23    08-01-03 15:48 Hui.shao
// 
// 22    07-09-18 14:02 Hongquan.zhang
// 
// 20    07-03-21 16:06 Hui.shao
// added entry to commit path usage
// 
// 19    07-03-14 12:33 Hongquan.zhang
// 
// 18    07-01-12 12:08 Hongquan.zhang
// 
// 17    07-01-09 15:14 Hongquan.zhang
// 
// 16    06-12-28 16:45 Hongquan.zhang
// 
// 15    06-12-25 16:22 Hongquan.zhang
// 
// 14    06-09-19 11:42 Hui.shao
// 
// 12    06-09-18 19:26 Hui.shao
// use plugin for PHO
// ===========================================================================

#ifndef __ZQTianShan_PathHelperMgr_H__
#define __ZQTianShan_PathHelperMgr_H__

#include "ZQ_common_conf.h"
#include "Exception.h"
#include "Locks.h"

#include "TsPathAdmin.h"
#include "../common/IPathHelperObj.h"

namespace ZQTianShan {
namespace AccreditedPath {

class PathSvcEnv;
class ADPathTicketImpl;

class PathHelperMgr : public ::Ice::LocalObject, public IPHOManager
{
public:

	PathHelperMgr(PathSvcEnv& env);
	virtual ~PathHelperMgr();
	void	setExtraData(const char* configFile,const char* logFolder);
	int populate(const char* pathToPHO = NULL);
    bool getStorageLinkPHOInfo(const ::std::string& type, ::ZQ::common::DynSharedObj::ImageInfo& info);
    bool getStreamLinkPHOInfo(const ::std::string& type, ::ZQ::common::DynSharedObj::ImageInfo& info);

public: // impl of IPHOManager

	virtual IStorageLinkPHO* registerStorageLinkHelper(const char* type, IStorageLinkPHO& helper, void* pCtx);
	virtual IStorageLinkPHO* unregisterStorageLinkHelper(const char* type);
	virtual IStorageLinkPHO* findStorageLinkHelper(const char* type);

	virtual IStreamLinkPHO* registerStreamLinkHelper(const char* type, IStreamLinkPHO& helper, void* pCtx);
	virtual IStreamLinkPHO* unregisterStreamLinkHelper(const char* type);
	virtual IStreamLinkPHO* findStreamLinkHelper(const char* type);

    virtual ::TianShanIce::StrValues listSupportedStorageLinkTypes();
    virtual ::TianShanIce::StrValues listSupportedStreamLinkTypes();

    virtual bool getStorageLinkSchema(const ::std::string& type, ::TianShanIce::PDSchema& schema);
    virtual bool getStreamLinkSchema(const ::std::string& type, ::TianShanIce::PDSchema& schema);

	virtual void validateStorageLinkConfig(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD);
	virtual void validateStreamLinkConfig(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD);

	virtual IdentCollection listPathTicketsByLink(::TianShanIce::Transport::StorageLinkPrx& link);
	virtual IdentCollection listPathTicketsByLink(::TianShanIce::Transport::StreamLinkPrx& link);

	virtual ::TianShanIce::Transport::PathTicketPrx openPathTicket(::Ice::Identity ident);
	virtual ::TianShanIce::Transport::StorageLinkPrx openStorageLink(::Ice::Identity ident);
	virtual ::TianShanIce::Transport::StreamLinkPrx openStreamLink(::Ice::Identity ident);

	virtual ::TianShanIce::Transport::PathTicketPrx openPathTicket(const char* name);
	virtual ::TianShanIce::Transport::StorageLinkPrx openStorageLink(const char* name);
	virtual ::TianShanIce::Transport::StreamLinkPrx openStreamLink(const char* name);

	virtual Ice::Int doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost);

	virtual bool	doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);
	virtual bool	doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	virtual void	doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket);
protected:

	PathSvcEnv& _env;

	typedef struct _PHONode
	{
		IPathHelperObject* pPHO;
		std::string        fnPlugin;
	} PHONode;

	typedef std::map<std::string, PHONode > PathHelperObjMap; ///< a map of link-type to StorageLinkHelper object

	PathHelperObjMap	_storageLinkPHOMap;
	ZQ::common::Mutex   _lockStorageLinkMap;

	PathHelperObjMap	_streamLinkPHOMap;
	ZQ::common::Mutex   _lockStreamLinkMap;

protected:

	ZQ::common::DynSharedObj* loadPHO(const char* filename);
	typedef std::map<std::string, ZQ::common::DynSharedObj* > PHOMap; ///< a map of link-type to StorageLinkHelper object
	PHOMap  _phoMap;
	ZQ::common::Mutex   _lockPhoMap;
	std::string			_strConfigFile;
	std::string			_strLogFolder;
};

}} // namespace

#endif // __ZQTianShan_PathHelperMgr_H__
