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
// Ident : $Id: IPathHelperObject.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/IPathHelperObj.h $
// 
// 4     3/18/11 1:47p Fei.huang
// 
// 3     3/09/11 2:43p Hongquan.zhang
// 
// 2     3/08/11 2:14p Fei.huang
// migrate to linux
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 24    09-12-16 15:16 Li.huang
// modify include path
// 
// 24    09-12-09 12:11 Build
// modify include path
// 
// 23    08-04-03 10:41 Build
// 
// 22    08-01-03 15:50 Hui.shao
// 
// 21    07-09-17 17:55 Hui.shao
// 
// 21    07-09-10 17:47 Hui.shao
// reduced invocations of link->getId() and getType()
// 
// 20    07-09-10 11:58 Hui.shao
// 
// 20    07-09-10 11:58 Hui.shao
// correct comments of SessCtx parameter
// 
// 19    07-09-10 11:53 Hui.shao
// checkin-TSmain: use session ctx snapshot in the PHOs instead let them
// call remotely every time
// 
// 19    07-09-10 11:21 Hui.shao
// checkin-TS1.6: use session ctx snapshot in the PHOs instead let them
// call remotely every time
// 
// 18    07-03-21 16:06 Hui.shao
// added entry to commit path usage
// 
// 17    07-03-13 17:11 Hongquan.zhang
// 
// 16    07-01-12 12:08 Hongquan.zhang
// 
// 15    07-01-09 15:10 Hongquan.zhang
// 
// 14    06-12-28 16:43 Hongquan.zhang
// 
// 13    06-12-21 14:04 Hui.shao
// removed param sess from PHO::doFreeResource
// 
// 11    9/21/06 4:34p Hui.shao
// batch checkin on 20060921
// 
// 10    06-09-18 19:26 Hui.shao
// use plugin for PHO
// ===========================================================================

#ifndef __ZQTianShan_PathHelper_H__
#define __ZQTianShan_PathHelper_H__

#include "ZQ_common_conf.h"
#include "Exception.h"
#include "Locks.h"
#include "DynSharedObj.h"

#include "TianShanDefines.h"
#include "TsTransport.h"
#include "TsSRM.h"

namespace ZQTianShan {
namespace AccreditedPath {

class IPathHelperObject;
class IPHOManager;
class IStorageLinkPHO;
class IStreamLinkPHO;

#define PathTicketPD_Field(_FIELD)  (::TianShanIce::Transport::PathTicketPDPrefix + #_FIELD).c_str()

#ifdef ZQ_OS_MSWIN
#define PHO_FILENAME_PREFIX		"PHO_"
#define PHO_EXT ".dll"
#else
#define PHO_FILENAME_PREFIX		"libPHO_"
#define PHO_EXT ".so"
#endif

#define _SERVICE_STATE_KEY_		"_service._state"
// -----------------------------
// struct SessCtx
// -----------------------------
/// A SRM session context for internal use within PathService. It keeps a snapshot of SRM session to reduce
/// remote invocations and speed up the processing.
///@note be aware this context is usually be used as READONLY, updates on this structure will not be flushed
/// into SRM session normally
typedef struct _SessCtx
{
	std::string					sessId;
	TianShanIce::State		state;
	
	TianShanIce::SRM::ResourceMap resources;
	TianShanIce::ValueMap	privdata;
	std::string strPrx;
	
} SessCtx;

// -----------------------------
// struct LinkInfo
// -----------------------------
typedef struct _LinkInfo
{
	::Ice::Identity linkIden;
	::std::string	streamerId;
	::std::string	otherNetId;
	::Ice::Int		otherIntId;
	::Ice::Int		cost;
	TianShanIce::SRM::ResourceMap rcMap; // ticket resource map
	::Ice::ObjectPrx linkPrx;
	::std::string    linkType;
	bool			 isStreamLink;
} LinkInfo;

// -----------------------------
// interface IPHOManager
// -----------------------------
/// A path helper object assistants AccreditPath on one specific type of StorageLink or StreamLink.
/// interface IStorageLinkPHO defines the basic entries of a StorageLink helper object
class IPHOManager
{
public:
	virtual ~IPHOManager() {}
	/// register a StorageLinkHelper object, called by a PathHelper plugin during InitPathHelper() entry
	///@param[in] type	 the string type of StorageLink about to associated by this helper object
	///@param[in] helper the helper object
	///@return    pointer to the helper if succeeded, otherwise NULL
	virtual IStorageLinkPHO* registerStorageLinkHelper(const char* type, IStorageLinkPHO& helper, void* pCtx) =0;

	virtual IStorageLinkPHO* unregisterStorageLinkHelper(const char* type) =0;
	virtual IStorageLinkPHO* findStorageLinkHelper(const char* type) =0;

	/// register a StreamLinkHelper object, called by a PathHelper plugin during InitPathHelper() entry
	///@param[in] type	 the string type of StreamLink about to associated by this helper object
	///@param[in] helper the helper object
	///@return    pointer to the helper if succeeded, otherwise NULL
	virtual IStreamLinkPHO* registerStreamLinkHelper(const char* type, IStreamLinkPHO& helper, void* pCtx) =0;
	virtual IStreamLinkPHO* unregisterStreamLinkHelper(const char* type) =0;
	virtual IStreamLinkPHO* findStreamLinkHelper(const char* type) =0;

    ///list all supported storage link types
	///@return a list of storage link string types
    virtual ::TianShanIce::StrValues listSupportedStorageLinkTypes() =0;

    ///list all supported stream link types
	///@return a list of stream link string types
    virtual ::TianShanIce::StrValues listSupportedStreamLinkTypes() =0;

    ///list the private data schema of a specific storage link type
	///@param[in] type the string type of the StorageLink to query
	///@param[out] schema the collection of schema definition
	///@return true if succeeded
    virtual bool getStorageLinkSchema(const ::std::string& type, ::TianShanIce::PDSchema& schema) =0;

    ///list the private data schema of a specific stream link type
	///@param[in] type the string type of the StreamLink to query
	///@param[out] schema the collection of schema definition
	///@return true if succeeded
    virtual bool getStreamLinkSchema(const ::std::string& type, ::TianShanIce::PDSchema& schema) =0;

	/// validate a StorageLink configuration with the schema of the type
	///@param[in] type	the string type of link
	///@param[in] identStr the unique key to identify this link
	///@param[in, out]  configPD the configuration set based on the schema, some value may be adjusted during validation
	///@throw ::TianShanIce::InvalidParameter if some unacceptable configurations are given
	virtual void validateStorageLinkConfig(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD) =0;

	/// validate a StorageLink configuration with the schema of the type
	///@param[in] type	the string type of link
	///@param[in] identStr the unique key to identify this link
	///@param[in, out]  configPD the configuration set based on the schema, some value may be adjusted during validation
	///@throw ::TianShanIce::InvalidParameter if some unacceptable configurations are given
	virtual void validateStreamLinkConfig(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD) =0;

	/// list all the allocated PathTicket with allocation on the specific storage link
	///@param[in] link	access to the StroageLink instance
	///@return    a collection of PathTicket identities
	virtual IdentCollection listPathTicketsByLink(::TianShanIce::Transport::StorageLinkPrx& link) =0;

	/// list all the allocated PathTicket with allocation on the specific stream link
	///@param[in] link	access to the StreamLink instance
	///@return    a collection of PathTicket identities
	virtual IdentCollection listPathTicketsByLink(::TianShanIce::Transport::StreamLinkPrx& link) =0;

	/// open a path ticket by its identity/name
	///@param[in] ident	the identity of the path ticket
	///@param[in] name ticket name
	///@return    access to the ticket
	virtual ::TianShanIce::Transport::PathTicketPrx openPathTicket(::Ice::Identity ident) =0;
	
	virtual ::TianShanIce::Transport::PathTicketPrx openPathTicket(const char* name) =0;
	

	/// open a storage link from the server core database
	///@param[in] ident	the identity of the StorageLink
	///@param[in] name storage link name
	///@return    access to the StorageLink instance
	virtual ::TianShanIce::Transport::StorageLinkPrx openStorageLink(::Ice::Identity ident) =0;

	virtual ::TianShanIce::Transport::StorageLinkPrx openStorageLink(const char* name) =0;

	/// open a StreamLink from the server core database
	///@param[in] ident	the identity of the StreamLink
	///@param[in] stream link name
	///@return    access to the StreamLink instance
	virtual ::TianShanIce::Transport::StreamLinkPrx openStreamLink(::Ice::Identity ident) =0;

	virtual ::TianShanIce::Transport::StreamLinkPrx openStreamLink(const char* name) =0;

	/// evalutes a given Link for its cost. The evaluation is based on the resources collection and other
	/// recognizable context in the given session, when some required fields are not available in the session
	/// context, it will then refer to the hinted private data.\n
	///@param[in] link	access to the StroageLink that about to evaluate, must be able to cast to StroageLinkEx
	///@param[in] sessCtx	readonly snapshot of the session context that evaluate for
	///@param[in] hintedPD 	hinted private data in the case they are short in the given session context
	///@param[in out] rcMap useful ticket resource data
	///@param[in] oldCost	the initial cost
	///@return the higher value of the old cost and cost result of this evaluation
	///@note the resource and context in the session may also be updated during evaluation
	///@note when a NULL session has been given, all necessary context updates will be applied into hintedPD instead
	///@sa IStorageLinkPHO::doEvaluation()
	virtual Ice::Int doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost) =0;

	/// narrow the negotiable resources and context bound within a session, the helper manager will dispatch the routine
	/// to the doNarrow() method of the related path helper objects
	///@param[in] sessCtx	readonly snapshot of the session context that narrow for
	///@param[in] ticketIdent identity to open the allocated ticket
	///@return true if succeeded
	///@sa IPathHelperObject::doNarrow()
	virtual bool doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx) =0;

	/// commit the path that has been employed, the helper manager will dispatch the routine to the doCommit() method
	/// of the related path helper objects
	///@param[in] sessCtx	readonly snapshot of the session context that narrow for
	///@param[in] ticketIdent identity to open the allocated ticket
	///@return true if succeeded
	///@sa IPathHelperObject::doCommit()
	virtual bool doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx) =0;

	/// Free resources bound on a ticket, the PHO must perform the steps to free the resource allocated for this ticket
	///@param[in, out] ticket	access to the ticket of which PHO should free the allocated resource
	///@sa IPathHelperObject::doFreeResource()
	///@note this invocation is from ::TianShanIce::Transport::PathTicket::destroy()
	virtual void doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket) =0;

};

// -----------------------------
// plugin facet IPathHelperObject
// -----------------------------
/// A path helper object assistants AccreditPath on one specific type of StorageLink or StreamLink.
/// interface IStorageLinkPHO defines the basic entries of a StorageLink helper object
class IPathHelperObject
{
public:
	
	IPathHelperObject(IPHOManager& mgr) : _helperMgr(mgr) {}
	virtual ~IPathHelperObject() {}

	typedef enum _NarrowResult
	{
		// don't change the order
		NR_Unrecognized,
		NR_RetryLater,
		NR_Narrowed,
		NR_Completed,
		NR_Error,
	} NarrowResult;

	/// exports the private data schema of a specific StorageLink or StreamLink type from an IPathHelperObject object
	///@param[in] type	the string type of link
	///@param[out] schema the schema of the acceptable private data bound with this type of StorageLink
	///@return true if succeeded
	virtual bool getSchema(const char* type, ::TianShanIce::PDSchema& schema) =0;

	/// validate the configuration with the schema of the type
	///@param[in] type	the string type of link
	///@param[in] identStr the unique key to identify this link
	///@param[in, out]  configPD the configuration set based on the schema, some value may be adjusted during validation
	///@throw ::TianShanIce::InvalidParameter if some unacceptable configurations are given
	virtual void validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD) =0;

	/// evalutes a given Link for its cost. The evaluation is based on the resources collection and other
	/// recognizable context in the given session, when some required fields are not available in the session
	/// context, it will then refer to the hinted private data.\n
	///@param[in] link	access to the StroageLink that about to evaluate
	///@param[in] sessCtx	readonly snapshot of the session context that evaluate for
	///@param[in] hintedPD 	hinted private data in the case they are short in the given session context
	///@param[in] oldCost	the initial cost
	///@return the higher value of the old cost and cost result of this evaluation
	///@note the resource and context in the session may also be updated during evaluation
	///@note when a NULL session has been given, all necessary context updates will be applied into hintedPD instead
	///@sa getStorageLinkSchema() for hintedPD fields
	virtual Ice::Int doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost) =0;

	/// narrow the negotiable resources and context bound within a session based on this specific type
	///@param[in, out] sessCtx	readonly snapshot of the session context that narrow for
	///@param[in, out] ticket	access to the ticket that narrow for
	///@return NarrowResult types
	///@sa IPHOManager::doNarrow()
	virtual NarrowResult doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx) =0;

	/// commit the path that has been employed, a stream instance has already been created at this moment
	///@param[in] sessCtx	readonly snapshot of the session context that narrow for
	///@param[in, out] ticket	access to the ticket that narrow for
	///@return NarrowResult types
	///@sa IPHOManager::doCommit()
	virtual void doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx) =0;

	/// Free resources bound on a ticket, the PHO must perform the steps to free the resource allocated for this ticket
	///@param[in, out] ticket	access to the ticket of which PHO should free the allocated resource
	///@sa IPHOManager::doFreeResource()
	///@note this invocation is from ::TianShanIce::Transport::PathTicket::destroy()
	///@note PHO may test ::TianShanIce::Transport::PathTicket::stampComitted, which will be stamped by PHOManager if doNarrow() succeeds,
	///      to check if the resource allocation has been committed
	virtual void doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket) =0;

protected:

	IPHOManager& _helperMgr;

};

// -----------------------------
// plugin facet IStorageLinkPHO
// -----------------------------
/// A path helper object assistants AccreditPath on one specific type of StorageLink.
/// interface IStorageLinkPHO defines the basic entries of a StorageLink helper object
class IStorageLinkPHO : public IPathHelperObject
{
public:

	IStorageLinkPHO(IPHOManager& mgr) : IPathHelperObject(mgr) {}
};

// -----------------------------
// plugin facet IStreamLinkPHO
// -----------------------------
/// A path helper object assistants AccreditPath on one specific type of StreamLink.
/// interface IPathHelperObject defines the basic entries of a StreamLink helper object
class IStreamLinkPHO : public IPathHelperObject
{
public:

	IStreamLinkPHO(IPHOManager& mgr) : IPathHelperObject(mgr) {}
};

class PHOPluginFacet : public ZQ::common::DynSharedFacet
{
	// declare this Facet object as a child of DynSharedFacet
	DECLARE_DSOFACET(PHOPluginFacet, DynSharedFacet);

	// declare the API prototypes
	DECLARE_PROC(void, InitPHO, (IPHOManager& mgr, void* pCtx ,const char* configFile,const char* logFolder));
	DECLARE_PROC(void, UninitPHO, (void));

	// map the external APIs
	DSOFACET_PROC_BEGIN();
		DSOFACET_PROC(InitPHO);
		DSOFACET_PROC(UninitPHO);
	DSOFACET_PROC_END();
};


}} // namespace

#endif // __ZQTianShan_ADPTicketHelper_H__
