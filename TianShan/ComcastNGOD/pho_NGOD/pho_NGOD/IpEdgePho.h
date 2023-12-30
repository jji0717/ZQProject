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
// Ident : $Id: IpEdgePHO.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 

#ifndef __ZQTianShan_PathHelper_IpEdge_H__
#define __ZQTianShan_PathHelper_IpEdge_H__

#include "ZQ_common_Conf.h"
#include "Exception.h"
#include "Locks.h"

#include <vector>
#include <string>
#include <list>


#include "IPathHelperObj.h"

namespace ZQTianShan {
namespace AccreditedPath {


#define STRMLINK_TYPE_NGOD  "SeaChange.VSS.NGOD"

class IpEdgePHO : public ::Ice::LocalObject, public IStreamLinkPHO
{
public:

	IpEdgePHO(IPHOManager& mgr);
	virtual ~IpEdgePHO();

public:
	
	virtual bool			getSchema( const char* type, ::TianShanIce::PDSchema& schema);

	virtual void			validateConfiguration( const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD);

	virtual NarrowResult	doNarrow( const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	virtual void			doFreeResources( const ::TianShanIce::Transport::PathTicketPtr& ticket);

	virtual Ice::Int		doEvaluation( LinkInfo& linkInfo, 
											const SessCtx& sessCtx, 
											TianShanIce::ValueMap& hintPD, 
											const ::Ice::Int oldCost);

	virtual void			doCommit( const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);


protected:

	virtual Ice::Int		eval_IPStrmLink(LinkInfo& linkInfo, 
											const SessCtx& sessCtx, 
											TianShanIce::ValueMap& hintPD, 
											TianShanIce::SRM::ResourceMap& rcMap , 
											const ::Ice::Int oldCost );
	
	virtual void			validate_IPStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);


	NarrowResult			narrow_IPStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
													const SessCtx& sessCtx,
													const TianShanIce::Transport::PathTicketPtr& ticket);


	IPHOManager*	_phoManager;
private:
	typedef struct _tagLinkIPAttr
	{
		std::string			_streamLinkID;
		Ice::Long			_totalBandwidth;		
		Ice::Long			_usedBandwidth;
		Ice::Int			_maxStreamCount;
		Ice::Int			_usedStreamCount;
		std::string			_strSOPName;		
	}LinkIPAttr;

	typedef struct _tagResourceIPData
	{		
		Ice::Long			_usedBandwidth;
	}ResourceIPData;


	typedef std::map<std::string,LinkIPAttr>		LinkIPAttrMap;
	typedef std::map<std::string,ResourceIPData>	ResourceIPDataMap;//<ticketid,ResourceIPData>

	LinkIPAttrMap						_StreamLinkIPAttrmap;
	ResourceIPDataMap					_ipResourceDataMap;
	
	ZQ::common::Mutex					_ipResourceLocker;
};

#define NGOD_RES_PREFIX(X)	"NGOD.R2."#X

}} // namespace

#endif // __ZQTianShan_PathHelper_IpEdge_H__

