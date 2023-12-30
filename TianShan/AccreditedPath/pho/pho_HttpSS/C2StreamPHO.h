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
// Ident : $Id: C2StreamPHO.h $
// Branch: $Name:  $
// Author: l.huang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/pho/pho_HttpSS/C2StreamPHO.h $
// 
// 1     5/08/14 3:32p Li.huang
// ===========================================================================

#ifndef __ZQTianShan_PathHelper_HTTPSS_C2Stream_H__
#define __ZQTianShan_PathHelper_HTTPSS_C2Stream_H__

#include "ZQ_common_conf.h"
#include "Exception.h"
#include "Locks.h"
#include "NativeThreadPool.h"
#include "SystemUtils.h"

#include <vector>
#include <string>
#include <list>


#include "../../common/IPathHelperObj.h"

namespace ZQTianShan {
namespace AccreditedPath {

#define STRMLINK_TYPE_HTTPSS_C2STREAM				"xor-media.HttpSS.C2Stream"

class C2StreamPHO : public ::Ice::LocalObject, public IStreamLinkPHO 
{
public:

	C2StreamPHO(IPHOManager& mgr);
	virtual ~C2StreamPHO();

public:

	/// Implementations of IPathHelperObject
	///@{

	/// exports the private data schema of a specific StorageLink or StreamLink type from an IPathHelperObject object
	///@param[in] type	the string type of link
	///@param[out] schema the schema of the acceptable private data bound with this type of StorageLink
	///@return true if succeeded
	virtual bool getSchema(const char* type, ::TianShanIce::PDSchema& schema);

	/// validate the configuration with the schema of the type
	///@param[in] type	the string type of link
	///@param[in] identStr the unique key to identify this link
	///@param[in, out]  configPD the configuration set based on the schema, some value may be adjusted during validation
	///@throw ::TianShanIce::InvalidParameter if some unacceptable configurations are given
	virtual void validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD);

	/// narrow the negotiable resources and context bound within a session based on this specific type
	///@param[in] ticket	access to the ticket that narrow for
	///@param[in] sess	access to the session context that narrow for
	///@return NarrowResult types
	///@sa IPHOManager::doNarrow()
	virtual IPathHelperObject::NarrowResult doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	/// free a ticket, the PHO must perform the steps to free the resource allocated for this ticket
	///@param[in, out] ticket	access to the ticket of which PHO should free the allocated resource	
	///@sa IPHOManager::doFreeResource()
	///@note this invocation is from ::TianShanIce::Transport::PathTicket::destroy()
	///@note PHO may test ::TianShanIce::Transport::PathTicket::stampComitted, which will be stamped by PHOManager if doNarrow() succeeds,
	///      to check if the resource allocation has been committed	
	virtual void doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket);
	///@}

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
	virtual Ice::Int doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost) ;

	/// commit the path that has been employed, the helper manager will dispatch the routine to the doCommit() method
	/// of the related path helper objects
	///@param[in] sess	access to the session context that narrow for
	///@param[in] ticketIdent identity to open the allocated ticket
	///@return true if succeeded
	///@sa IPathHelperObject::doCommit()
	virtual void doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

protected:

	virtual Ice::Int eval_C2StreamLink(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap ,const ::Ice::Int oldCost);
	
	virtual void validate_C2StreamLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);
	

	NarrowResult narrow_C2StreamLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
									const SessCtx& sessCtx,
									const TianShanIce::Transport::PathTicketPtr& ticket);
protected:
	typedef struct
	{
		std::string                _streamLinkID;
		std::vector<std::string>   _C2LocateURLs;
		Ice::Long                  _totalBandwidth;
		Ice::Int                   _maxSessionsCount;

		Ice::Long                  _availableBandwidth;
		Ice::Long                  _usedBandwidth;

		Ice::Int                  _usedSessionCount;

	}C2StreamLinkAttr;

	typedef std::map<std::string, C2StreamLinkAttr>		C2StreamLinkAttrMap;	//<linkId, LinAttr>

	C2StreamLinkAttrMap _c2StreamLinkMap;
	ZQ::common::Mutex   _lockC2StreamLinkMap;

	typedef struct
	{		
		Ice::Long				   _usedBandwidth; //bps
		std::vector<std::string>   _C2LocateURLs;

	}C2TicketAttr;

	typedef std::map<std::string, C2TicketAttr>	C2StreamTicketAttrMap;	//<ticketid,TicketAttr>
	C2StreamTicketAttrMap	    _c2ticketAttrMap;

private:	
	IPHOManager*	_phoManager;
private:
	
};

}} // namespace

#endif // __ZQTianShan_PathHelper_HTTPSS_C2Stream_H__

