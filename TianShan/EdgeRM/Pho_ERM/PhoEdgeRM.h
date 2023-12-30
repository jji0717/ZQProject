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
// Ident : $Id: PhoEdgeRM.h $
// Branch: $Name:  $
// Author: Li Xiao Ming
// Desc  : 
//
// Revision History: 

#ifndef __ZQTianShan_PathHelper_EdgeRM_H__
#define __ZQTianShan_PathHelper_EdgeRM_H__

#include "ZQ_common_conf.h"
#include "Exception.h"
#include "Locks.h"
#include "NativeThread.h"
#include "SystemUtils.h"

#include <vector>
#include <string>
#include <list>
#include "public.h"
#include "IPathHelperObj.h"

#include "PhoEdgeRMEnv.h"

namespace ZQTianShan {
namespace AccreditedPath {

#define STRMLINK_TYPE_TianShanERM  "XOR-media.IpEdge.EdgeRM"

typedef enum
{
	OptimisticMode	= 1,
	PessimisticMode	= 2
}RoutingMode;

class EdgeRMPHO : public ::Ice::LocalObject, public IStreamLinkPHO
{
public:

	EdgeRMPHO(IPHOManager& mgr, ZQTianShan::EdgeRM::PhoEdgeRMEnv& env);
	virtual ~EdgeRMPHO();
public:
	
	/// Implementations of IPathHelperObject
	///@{
	/// exports the private data schema of a specific StorageLink or StreamLink type from an IPathHelperObject object
	///@param[in] type	the string type of link
	///@param[out] schema the schema of the acceptable private data bound with this type of StorageLink
	///@return true if succeeded
	virtual bool			getSchema(const char* type, ::TianShanIce::PDSchema& schema);

	/// validate the configuration with the schema of the type
	///@param[in] type	the string type of link
	///@param[in] identStr the unique key to identify this link
	///@param[in, out]  configPD the configuration set based on the schema, some value may be adjusted during validation
	///@throw ::TianShanIce::InvalidParameter if some unacceptable configurations are given
	virtual void			validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD);

	/// Implementaions of IStreamLinkPHO
	///@{

	/// evaluates a given StreamLink for its cost. The evaluation is based on the resources collection and other
	/// recognizable context in the given session, when some required fields are not available in the session
	/// context, it will then refer to the hinted private data.\n
	///@param[in] link	access to the StreamLink that about to evaluate
	///@param[in] sess	access to the session context that evaluate for
	///@param[in] hintedPD 	hinted private data in the case they are short in the given session context
	///@param[in] oldCost	the initial cost
	///@return the higher value of the old cost and cost result of this evaluation
	///@note the resource and context in the session may also be updated during evaluation
	///@note when a NULL session has been given, all necessary context updates will be applied into hintedPD instead
	///@sa getStorageLinkSchema() for hintedPD fields
	//virtual Ice::Int doEvaluation(::TianShanIce::Transport::StreamLinkPrx& link, const TianShanIce::SRM::SessionPrx& sess, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap , const ::Ice::Int oldCost);

	virtual Ice::Int		doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& ticketPrivateData, const ::Ice::Int oldCost);

	/// commit the path that has been employed, the helper manager will dispatch the routine to the doCommit() method
	/// of the related path helper objects
	///@param[in] sess	access to the session context that narrow for
	///@param[in] ticketIdent identity to open the allocated ticket
	///@return true if succeeded
	///@sa IPathHelperObject::doCommit()
	virtual void			doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	/// narrow the negotiable resources and context bound within a session based on this specific type
	///@param[in] ticket	access to the ticket that narrow for
	///@param[in] sess	access to the session context that narrow for
	///@return NarrowResult types
	///@sa IPHOManager::doNarrow()
	virtual NarrowResult	doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	/// free a ticket, the PHO must perform the steps to free the resource allocated for this ticket
	///@param[in, out] ticket	access to the ticket of which PHO should free the allocated resource	
	///@sa IPHOManager::doFreeResource()
	///@note this invocation is from ::TianShanIce::Transport::PathTicket::destroy()
	///@note PHO may test ::TianShanIce::Transport::PathTicket::stampComitted, which will be stamped by PHOManager if doNarrow() succeeds,
	///      to check if the resource allocation has been committed	
	virtual void			doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket);

protected:
	virtual void			validate_EdgeRMStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);
	::TianShanIce::EdgeResource::AllocationPrx
		createAllocation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& ticketPrivateData, TianShanIce::SRM::ResourceMap& resourceMap, EdgeRMLinkAttr& linktattr);

	::TianShanIce::EdgeResource::EdgeResouceManagerPrx createEdgeRMPrx(const char* endpoint);

public:
	virtual Ice::Int		eval_EdgeRMStrmLink(LinkInfo& linkInfo, const SessCtx& sessCtx, 
		                                       TianShanIce::ValueMap& ticketPrivateData, 
		                                       TianShanIce::SRM::ResourceMap& resourceMap, 
											   const ::Ice::Int oldCost, 
		                                       EdgeRMLinkAttr& linktattr);

	NarrowResult			narrow_EdgeRMStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, const SessCtx& sessCtx, const TianShanIce::Transport::PathTicketPtr& ticket);

protected:
	IPHOManager*	_phoManager;
	::ZQTianShan::EdgeRM::PhoEdgeRMEnv& _env;

private:
	typedef struct
	{		
		Ice::Long			_bandwidth;
		std::string			_channel;
		Ice::Int			_modulationFormat;
		Ice::Int			_FEC;
		::Ice::Int          _symbolRate;
		::Ice::Long         _PnId;
		::std::string       _edgeDeviceIP;
		::std::string       _edgeDeviceMac;
		::std::string       _edgeDeviceGroup;
		::std::string       _edgeDeviceName;
		::Ice::Int          _destPort;

	}TicketAttr;

	typedef std::map<std::string, TicketAttr>	TicketAttrMap;	//<ticketid,TicketAttr>

	EdgeRMLinkAttrMap	_linkAttrMap;
	TicketAttrMap	    _ticketAttrMap;
	
	::ZQ::common::Mutex		_linkAttrMapMutex;
	//::ZQ::common::Mutex		_ticketAttrMapMutex;
};

#define NGOD_RES_PREFIX(X)	"NGOD.R2."#X

}} // namespace

#endif // __ZQTianShan_PathHelper_EdgeRM_H__

