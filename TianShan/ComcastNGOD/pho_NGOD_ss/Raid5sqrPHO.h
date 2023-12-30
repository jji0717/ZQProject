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
// Ident : $Id: Raid5sqrPHO.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 

#ifndef __ZQTianShan_PathHelper_Raid5sql_H__
#define __ZQTianShan_PathHelper_Raid5sql_H__

#include "ZQ_common_Conf.h"
#include "Exception.h"
#include "Locks.h"

#include "../../common/IPathHelperObj.h"

namespace ZQTianShan {
namespace AccreditedPath {

#define STORLINK_TYPE_RAID5SQR  "SeaChange.Raid5^2.forNGOD"

class Raid5sqrPHO : public ::Ice::LocalObject, public IStorageLinkPHO
{
public:

	Raid5sqrPHO(IPHOManager& mgr);
	virtual ~Raid5sqrPHO();

public:

	/// Implementaions of IPathHelperObject
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
	///@param[in, out] ticket	access to the ticket that narrow for
	///@param[in] sess	access to the session context that narrow for
	///@return NarrowResult types
	///@sa IPHOManager::doNarrow()
	virtual NarrowResult doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const TianShanIce::SRM::SessionPrx& sess);

	virtual void doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket);
	///@}

	/// Implementations of IStorageLinkPHO
	///@{

	/// evalutes a given StorageLink for its cost. The evaluation is based on the resources collection and other
	/// recognizable context in the given session, when some required fields are not available in the session
	/// context, it will then refer to the hinted private data.\n
	///@param[in] link	access to the StroageLink that about to evaluate
	///@param[in] sess	access to the session context that evaluate for
	///@param[in] hintedPD 	hinted private data in the case they are short in the given session context
	///@param[in] oldCost	the initial cost
	///@return the higher value of the old cost and cost result of this evaluation
	///@note the resource and context in the session may also be updated during evaluation
	///@note when a NULL session has been given, all necessary context updates will be applied into hintedPD instead
	///@sa getStorageLinkSchema() for hintedPD fields
	virtual Ice::Int doEvaluation(::TianShanIce::Transport::StorageLinkPrx& link, const TianShanIce::SRM::SessionPrx& sess, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap , const ::Ice::Int oldCost);

	/// commit the path that has been employed, the helper manager will dispatch the routine to the doCommit() method
	/// of the related path helper objects
	///@param[in] sess	access to the session context that narrow for
	///@param[in] ticketIdent identity to open the allocated ticket
	///@return true if succeeded
	///@sa IPathHelperObject::doCommit()
	virtual void doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const TianShanIce::SRM::SessionPrx& sess) ;

	///@}

protected:

	Ice::Int eval_Raid5sLink(::TianShanIce::Transport::StorageLinkPrx& link, const TianShanIce::SRM::SessionPrx& sess, TianShanIce::ValueMap& ticketPrivateData,TianShanIce::SRM::ResourceMap& rcMap , const ::Ice::Int oldCost);
	void validate_Raid5sLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);
private:
	IPHOManager*	_phoManager;

	typedef struct _tagRaid5sAttr
	{
		std::string		_storageLinkID;
		Ice::Long		_totalBandwidth;
		Ice::Long		_availBandwidth;
	}Raid5sAttr;

	typedef struct _tagResourceRaidData
	{
		Ice::Long		_usedBandwidth;
	}ResourceRaidData;

	typedef std::map<std::string,Raid5sAttr>		Raid5sAttrMap;
	typedef std::map<std::string,ResourceRaidData>	ResourceRaidMap;

	Raid5sAttrMap		_raid5sAttrmap;
	ResourceRaidMap		_raidResourceMap;
	ZQ::common::Mutex	_raid5sattrmapLocker;
	
};

}} // namespace

#endif // __ZQTianShan_PathHelper_Raid5sql_H__

