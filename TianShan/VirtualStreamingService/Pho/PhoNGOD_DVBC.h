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

#ifndef __ZQTianShan_PathHelper_IpEdgeDVBC_H__
#define __ZQTianShan_PathHelper_IpEdgeDVBC_H__

#include "ZQ_common_conf.h"
#include "Exception.h"
#include "Locks.h"

#include <vector>
#include <string>
#include <list>


#include "IPathHelperObj.h"

namespace ZQTianShan {
namespace AccreditedPath {

#define STRMLINK_TYPE_NGOD_DVBC					"SeaChange.VSS.NGOD.DVBC"
#define STRMLINK_TYPE_NGOD_DVBC_SHARELINK		"SeaChange.VSS.NGOD.DVBC.ShareLink"

class IpEdgePHO_DVBC : public ::Ice::LocalObject, public IStreamLinkPHO
{
public:

	IpEdgePHO_DVBC(IPHOManager& mgr);
	virtual ~IpEdgePHO_DVBC();

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

	virtual Ice::Int		eval_DvbcStrmLink(LinkInfo& linkInfo, 
											const SessCtx& sessCtx, 
											TianShanIce::ValueMap& hintPD, 
											TianShanIce::SRM::ResourceMap& rcMap ,
											const ::Ice::Int oldCost);
	virtual Ice::Int		eval_DvbcStrmShareLink(LinkInfo& linkInfo, 
											const SessCtx& sessCtx, 
											TianShanIce::ValueMap& hintPD, 
											TianShanIce::SRM::ResourceMap& rcMap ,
											const ::Ice::Int oldCost);

	virtual void			validate_DvbcStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);

	virtual void			validate_DvbcStrmShareLinkConfig( const char* identStr, ::TianShanIce::ValueMap& configPD );


	NarrowResult			narrow_DvbcStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
												const SessCtx& sessCtx,
												const TianShanIce::Transport::PathTicketPtr&  ticket);

	NarrowResult			narrow_DvbcShareStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
												const SessCtx& sessCtx,
												const TianShanIce::Transport::PathTicketPtr&  ticket);

private:
	int			evalDVBCStreamLinkCost(const std::string& streamLinkID,
										Ice::Long bw2Alloc,
										const std::string& sessID) ;

	NarrowResult inner_narrow_DvbcStrmLink(const std::string strmLinkId , 
		const SessCtx& sessCtx,
		const TianShanIce::Transport::PathTicketPtr&  ticket);

	IPHOManager*	_phoManager;

private:
	/*------------------------------------------------------------------------------------
	DVBC format
	------------------------------------------------------------------------------------*/
	typedef struct _tagLinkDVBCAttr
	{
		std::string				_streamLinkID;
		Ice::Long				_totalBandWidth;		//total bandwidth in bytes
		Ice::Long				_availableBandwidth;	//available bandwidth in bytes
		Ice::Long				_usedBandwidth;
		Ice::Int				_pnMin;
		Ice::Int				_pnMax;
		Ice::Int				_totalPNCount;
		Ice::Int				_basePort;				//这个是经过计算的basePort
		Ice::Long				_portMask;
		Ice::Int				_portStep;
		Ice::Int				_basePN;		
		std::string				_strSOPName;
		std::string				_strSOPGroup;
		std::list<Ice::Int>		_availablePN;			//current available program number
		std::list<Ice::Int>		_backupPN;				//if _pnMin and _pnMax <0 ,this list contain the original PN data

	}LinkDVBCAttr;

	typedef std::map<std::string,LinkDVBCAttr>	LinkDVBCAttrMap;


	typedef struct _tagResourceDVBCData					//this data is used to associated with stream link 
	{
		Ice::Long			_usedBandWidth;			//used bandwidth
		Ice::Int			_usedPN;				//used program number		
	}ResourceDVBCData;

	//NOTE: if target streamlink is deleted,the linked shared link must be deleted too !!!!
	typedef std::map< std::string , ResourceDVBCData >	ResourceDVBCDataMap;//<TicketID,ResourceDVBCData>	

	LinkDVBCAttrMap						_StreamLinkDVBCAttrmap;
	ResourceDVBCDataMap					_dvbcResourceDataMap;

	typedef struct _tagDVBCSharedAttr 
	{
		std::string					_streamLinkId;	//point to real link 
		std::string					_strSOPName;
		std::string					_strSOPGroup;
	}DVBCSharedAttr;
	typedef std::map< std::string  , DVBCSharedAttr > LinkDVBCShareAttrMap;
	LinkDVBCShareAttrMap				_sharedDVBCLinkAttrmap;
	ZQ::common::Mutex					_sharedDVBCLinkAttrLocker;
	ZQ::common::Mutex					_dvbcResourceLocker;

};

#define NGOD_RES_PREFIX(X)	"NGOD.R2."#X

}} // namespace

#endif // __ZQTianShan_PathHelper_IpEdgeDVBC_H__

