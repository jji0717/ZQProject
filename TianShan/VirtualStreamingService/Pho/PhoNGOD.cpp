#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#endif
#include "PhoNGOD.h"
#include "Log.h"
#include <time.h>
#include <algorithm>
#include "TianShanIceHelper.h"


namespace ZQTianShan {
namespace AccreditedPath {

static ConfItem IpEdge_IP[] = {
	{ "TotalBandwidth",		::TianShanIce::vtLongs,		false,	"20000",	false }, // in Kbps
	{ "MaxStreamCount",		::TianShanIce::vtInts,		false,	"80",		false },
	{ "SopName",			::TianShanIce::vtStrings,	false,	"",			false },
	{ "DestMac" ,			::TianShanIce::vtStrings,	false,	"",			false },
	{ NULL,					::TianShanIce::vtInts,		true,	"",			false },
	};

static ::ZQTianShan::ConfItem IpEdge_IP_ShareLink[]= {
	{ "LinkId",             ::TianShanIce::vtStrings,	false , "" ,        false },
//	{ "SopName",			::TianShanIce::vtStrings,	false,	"",			false },
	{NULL, ::TianShanIce::vtInts, true, "",false },
};

IpEdgePHO::IpEdgePHO(IPHOManager& mgr)
: IStreamLinkPHO(mgr)
{
	_phoManager=&mgr;
}

IpEdgePHO::~IpEdgePHO()
{
	_helperMgr.unregisterStreamLinkHelper( STRMLINK_TYPE_NGOD );
	_helperMgr.unregisterStreamLinkHelper( STRMLINK_TYPE_NGOD_SHARELINK );
}
bool IpEdgePHO::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;

	// address the schema definition
	if ( 0 == strcmp(type, STRMLINK_TYPE_NGOD) )
	{
		config = IpEdge_IP;
	}
	else if( 0 == strcmp(type ,STRMLINK_TYPE_NGOD_SHARELINK ))
	{
		config = IpEdge_IP_ShareLink;
	}

	// no matches
	if (NULL == config)
		return false;

	// copy the schema to the output
	for (::ZQTianShan::ConfItem *item = config; item && item->keyname; item ++)
	{
		::TianShanIce::PDElement elem;
		elem.keyname			= item->keyname;
		//elem.optional			= item->optional;
		elem.optional2			= item->optional2;
		elem.defaultvalue.type	= item->type;
		elem.defaultvalue.bRange= item->bRange;
		switch(item->type) 
		{
		case TianShanIce::vtInts:
			{				
				elem.defaultvalue.ints.clear();
				if(!elem.defaultvalue.bRange)
				{
					elem.defaultvalue.ints.push_back(atoi(item->hintvalue));
				}				
				else
				{
					int a,b;
					sscanf(item->hintvalue,"%d ~ %d",&a,&b);
					elem.defaultvalue.ints.push_back(a);
					elem.defaultvalue.ints.push_back(b);
				}
			}
			break;
		case TianShanIce::vtLongs:
			{
				elem.defaultvalue.lints.clear();
				if (!elem.defaultvalue.bRange)
				{					
					elem.defaultvalue.lints.push_back( _atoi64(item->hintvalue));
				}
				else
				{
					Ice::Long a,b;
					sscanf(item->hintvalue,FMT64" ~ "FMT64,&a,&b);
					elem.defaultvalue.lints.push_back(a);
					elem.defaultvalue.lints.push_back(b);
				}
			}
			break;
		case TianShanIce::vtStrings:
			{
				elem.defaultvalue.strs.clear();
				elem.defaultvalue.strs.push_back(item->hintvalue);
			}
			break;
		default:
			break;
		}
		schema.push_back(elem);
	}

	return true;
}

void IpEdgePHO::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
	{
		type = "NULL";
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop",1001,"unsupported type [%s] in IpEdgePHO", type);
	}
	if (0 == strcmp( STRMLINK_TYPE_NGOD, type))
	{
		validate_IPStrmLinkConfig( identStr, configPD );
	}
	else if ( 0 == strcmp( STRMLINK_TYPE_NGOD_SHARELINK , type ) )
	{
		validate_IPStrmShareLinkConfig( identStr , configPD );
	}
	configPD.erase( _SERVICE_STATE_KEY_ );
}

void IpEdgePHO::validate_IPStrmShareLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	glog(ZQ::common::Log::L_DEBUG , CLOGFMT(NGODPHO,"enter validate a IP shared StreamLink's configuration :link[%s]") ,identStr );
	IPSharedAttr lsa;
	try
	{
		ZQ::common::MutexGuard gd(_sharedIPLinkAttrLocker);
		TianShanIce::Variant var;
		var.bRange = false;
		var = PDField(configPD,"LinkId");
		if ( var.type != TianShanIce::vtStrings ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog,"NgodSop", 1051 ,"Invalid LinkId type,should be vtString");
		}
		if ( var.strs.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop", 1052 ,"Invalid LinkId with size() == 0 ");
		}
		std::string& strTempId = var.strs[0];
		std::string::size_type slashPos = strTempId.find("/");
		if ( slashPos != std::string::npos ) 
		{
			strTempId = strTempId.substr( slashPos+1 );
		}
		lsa._streamLinkId = strTempId;

		var.bRange	= false;
		var = PDField(configPD,"SopName");
		if( var.type != TianShanIce::vtStrings || var.strs.size() <= 0 )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop", 1052 ,"Invalid SopName");
		}
		lsa._strSOPName	= var.strs[0];

		glog(ZQ::common::Log::L_INFO,
			CLOGFMT(NGODPHO, "Update IP shared stream link with id[%s] and target LinkId[%s],sopName[%s]"),
			identStr , lsa._streamLinkId.c_str(), lsa._strSOPName.c_str() );
		_sharedIPLinkAttrmap[identStr] = lsa;

	}
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	glog(ZQ::common::Log::L_DEBUG , CLOGFMT( NgodSop , "leave validate a IP shared streamlink's configuration: link[%s]" ),identStr);
}

void IpEdgePHO::validate_IPStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	glog(ZQ::common::Log::L_DEBUG,
		CLOGFMT(NGODPHO, " enter validate a IP StreamLink's Configuration: link[%s]"), 
		identStr );
	LinkIPAttr lia;
	lia._streamLinkID	= identStr;

	TianShanIce::Variant var;
	var.bRange			= false;
	try
	{					
		//get TotalBandwidth
		var = PDField( configPD , "TotalBandwidth" );
		if( var.type!=::TianShanIce::vtLongs)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop",1011,"Invalid TotalBandwidth type,should be vtLongs");
		}
		if ( var.lints.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop",1012,"Invalid TotalBandwidth ,lints size() == 0");
		}
		lia._totalBandwidth		= var.lints[0]*1000;
		lia._usedStreamCount	= 0 ;
		lia._usedBandwidth		= 0;
		glog(ZQ::common::Log::L_INFO , 
					CLOGFMT(NGODPHO,"stream IP link[%s] get totalBW[%lld]"),
					identStr, lia._totalBandwidth);

		//try to get max stream count
		try
		{
			var = PDField(configPD,"MaxStreamCount");
			if ( var.type == TianShanIce::vtInts && var.ints.size() > 0) 
			{
				lia._maxStreamCount		= var.ints[0];
				lia._usedStreamCount	= 0;
				glog(ZQ::common::Log::L_INFO,
							CLOGFMT(NGODPHO,"stream IP link[%s] get totalStreamCount[%d]"),
							identStr,
							lia._maxStreamCount);
			}
			else
			{
				lia._maxStreamCount		= 80;
				lia._usedStreamCount		= 0;
				glog(ZQ::common::Log::L_WARNING,
							CLOGFMT(NGODPHO,"stream IP link[%s] ,invalid maxStreamCount type or data,set it to default [80]"),
							identStr);
			}
		}
		catch( const ::TianShanIce::InvalidParameter&  )
		{
			lia._maxStreamCount = 80;//hard code the default value
			lia._usedStreamCount = 0;
			glog(ZQ::common::Log::L_WARNING,
							CLOGFMT(NGODPHO,"stream IP link[%s] ,MaxStreamCount is not found,set it to default [80]"),
							identStr);
		}

		//get sop name
		var = PDField(configPD, "SopName");
		if( var.type!=::TianShanIce::vtStrings )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop",1011,"Invalid SopName type,should be vtStrings");
		}
		if ( var.strs.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop",1012,"Invalid SopName ,strs size() == 0");
		}

		lia._strSOPName	= var.strs[0];
		glog(ZQ::common::Log::L_INFO , CLOGFMT(NGODPHO,"stream IP link[%s] get sopName[%s]"), identStr, lia._strSOPName.c_str() );

		//get DestMac
		lia._destMacAddress = "";
		try
		{
			var = PDField(configPD, "DestMac");
			if ( var.type == TianShanIce::vtStrings && var.strs.size() > 0 ) 
			{
				lia._destMacAddress = var.strs[0];
				glog(ZQ::common::Log::L_INFO, CLOGFMT(NGODPHO, "IP StreamLink[%s], set DestMac[%s]"), identStr, lia._destMacAddress.c_str());
			}
			else
				glog(ZQ::common::Log::L_INFO, CLOGFMT(NGODPHO , "IP StreamLink[%s], DestMac not found, set it to empty"), identStr);
		}
		catch ( const ::TianShanIce::InvalidParameter& ) 
		{
			glog( ZQ::common::Log::L_INFO, CLOGFMT(NGODPHO, "IP StreamLink[%s], DestMac not found, set it to empty"), identStr);
		}
		
		Ice::Int iServiceState = TianShanIce::stNotProvisioned;
		ZQTianShan::Util::getValueMapDataWithDefault( configPD, _SERVICE_STATE_KEY_, TianShanIce::stNotProvisioned, iServiceState );

		if( iServiceState != TianShanIce::stInService )
		{
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			LinkIPAttrMap::iterator itIpAttr = _StreamLinkIPAttrmap.find( identStr );

			::TianShanIce::Transport::StreamLinkPrx strmLinkPrx = _phoManager->openStreamLink( identStr );
			if ( strmLinkPrx )
			{
				IdentCollection idc;
				try
				{
					idc=_phoManager->listPathTicketsByLink(strmLinkPrx);
				}
				catch (Ice::ObjectNotExistException&)
				{
					idc.clear();
				}
				IdentCollection::const_iterator itID=idc.begin();
				for( ; itID != idc.end() ; itID ++ )
				{
					::TianShanIce::Transport::PathTicketPrx ticketprx=_phoManager->openPathTicket( *itID );
					if ( ticketprx )
					{
						TianShanIce::ValueMap	PDData					=	ticketprx->getPrivateData();
						TianShanIce::SRM::ResourceMap ticketResource	=	ticketprx->getResources();
						::TianShanIce::Variant	bandwidth;
						try
						{
							TianShanIce::SRM::ResourceMap::const_iterator it = ticketResource.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
							if( it == ticketResource.end())
							{
								glog(ZQ::common::Log::L_DEBUG,
									CLOGFMT(NGODPHO,"validate_IPStrmLinkConfig ticket %s but no bandwidth resource"),
									itID->name.c_str());
							}
							else
							{
								//get owner streamlink id
								//ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varLinkId;
								std::string ownerStreamlinkId;
								ZQTianShan::Util::getValueMapDataWithDefault(PDData,PathTicketPD_Field(ownerStreamLinkId),"",ownerStreamlinkId );
								if( ownerStreamlinkId.empty() )
								{
									glog(ZQ::common::Log::L_WARNING,
										CLOGFMT(NGODPHO,"validate_IPStrmLinkConfig() ticket %s found, but no onwer stream link id, skip"),
										itID->name.c_str());
									continue;
								}

								//TianShanIce::ValueMap val=it->second.resourceData;
								bandwidth = PDField( PDData , PathTicketPD_Field(bandwidth) );
								if (bandwidth.lints.size()==0) 
								{
									glog(ZQ::common::Log::L_WARNING,
										CLOGFMT(NGODPHO,"validate_IPStrmLinkConfig() ticket %s found an invalid bandwidth,maybe db error"),
										itID->name.c_str());
									continue;
								}
								glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NGODPHO,"validate_IPStrmLinkConfig() ticket %s used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);
								lia._usedBandwidth += bandwidth.lints[0];
								lia._usedStreamCount ++;	

								ResourceIPData rid;
								rid._usedBandwidth = bandwidth.lints[0];
								{
									_ipResourceDataMap[ itID->name ] = rid;
								}

							}
						}
						catch(::TianShanIce::InvalidParameter& e)
						{
							glog(ZQ::common::Log::L_ERROR,
								CLOGFMT(NGODPHO,"validate_IPStrmLinkConfig() invalidParameter exception is caught:%s"),
								e.message.c_str());
						}
						catch (...)
						{
						}
					}
				}//for
			}
			if ( itIpAttr != _StreamLinkIPAttrmap.end() )
			{				
				itIpAttr->second					= lia;
			}
			else
			{
				_StreamLinkIPAttrmap[ identStr ]	= lia;
			}

		}		
		else
		{
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			LinkIPAttrMap::iterator itIpAttr = _StreamLinkIPAttrmap.find( identStr );
			if ( itIpAttr != _StreamLinkIPAttrmap.end() )
			{
				lia._usedStreamCount	= itIpAttr->second._usedStreamCount;
				lia._usedBandwidth		= itIpAttr->second._usedBandwidth;
				itIpAttr->second		= lia;
			}
			else
			{
				_StreamLinkIPAttrmap[ identStr ]	= lia;
			}
		}
		glog(ZQ::common::Log::L_INFO,
			CLOGFMT(NGODPHO,"validate_IPStrmLinkConfig() update  ip streamlink with streamlink id [%s] "
								"totalBW[%lld] usedBW[%lld] maxStreamCount[%d] usedStrmCount[%d] sopName[%s]"),
								identStr,   
								lia._totalBandwidth, lia._usedBandwidth,
								lia._maxStreamCount, lia._usedStreamCount , 
								lia._strSOPName.c_str());
	}	
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	catch (const ::Ice::Exception& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO,"validate_IPStrmLinkConfig() catch a ice exception :%s"),e.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO,"validate_IPStrmLinkConfig() unexpect error"));
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODPHO, "leave validate a IP StreamLink's Configuration: link[%s]"), identStr);
}

#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
	if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else glog(ZQ::common::Log::L_WARNING, CLOGFMT(NGODPHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}

Ice::Int IpEdgePHO::doEvaluation(LinkInfo& linkInfo, 
								 const SessCtx& sessCtx,
								 TianShanIce::ValueMap& hintPD,
								 const ::Ice::Int oldCost)
{
	std::string &linktype =linkInfo.linkType;
	if (0 == linktype.compare( STRMLINK_TYPE_NGOD ))
	{
		return eval_IPStrmLink(linkInfo, sessCtx, hintPD, linkInfo.rcMap,oldCost);
	}
	else if( 0 == linktype.compare( STRMLINK_TYPE_NGOD_SHARELINK ) )
	{
		return eval_IPStrmShareLink( linkInfo , sessCtx , hintPD , linkInfo.rcMap , oldCost );
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR , CLOGFMT(NGODPHO , "unrecognized streamlink type[%s]" ),linktype.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
}

Ice::Int IpEdgePHO::eval_IPStrmShareLink(LinkInfo& linkInfo, 
											 const SessCtx& sessCtx, 
											 TianShanIce::ValueMap& hintPD, 
											 TianShanIce::SRM::ResourceMap& rcMap , 
											 const ::Ice::Int oldCost )
{
	glog(ZQ::common::Log::L_DEBUG , CLOGFMT(NGODPHO,"eval_IPShareStrmLink() using share link with LinkId[%s]"),
		linkInfo.linkIden.name.c_str());
	std::string strLinkId = "";

	{
		ZQ::common::MutexGuard gd(_sharedIPLinkAttrLocker);
		LinkIPShareAttrMap::const_iterator it = _sharedIPLinkAttrmap.find( linkInfo.linkIden.name );
		if( it != _sharedIPLinkAttrmap.end( ) )
		{
			strLinkId = it->second._streamLinkId;
			glog(ZQ::common::Log::L_INFO , CLOGFMT(NGODPHO , "eval_IPShareStrmLink() find target linkId[%s]" ),
				strLinkId.c_str() );
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR , 
				CLOGFMT(NGODPHO , "eval_IPShareStrmLink() no shared IP link with ID [%s] ,return with OutOfServiceCost"),
				linkInfo.linkIden.name.c_str() );
			return TianShanIce::Transport::OutOfServiceCost;
		}
	}

	LinkInfo lInfo = linkInfo;
	{
		ZQ::common::MutexGuard lockIpRes(_ipResourceLocker);
		LinkIPAttrMap::const_iterator itIP = _StreamLinkIPAttrmap.find(strLinkId);
		if( itIP == _StreamLinkIPAttrmap.end() )
		{
			glog(ZQ::common::Log::L_ERROR, 
				CLOGFMT(NGODPHO , "eval_IPShareStrmLink() target streamlink [%s] is not found" ),
				strLinkId.c_str() );
			return 	::TianShanIce::Transport::OutOfServiceCost;
		}

		try
		{
			lInfo.linkIden.name = strLinkId;
			lInfo.linkPrx = _helperMgr.openStreamLink( strLinkId.c_str() );
		}
		catch (const Ice::Exception& ex) 
		{
			glog(ZQ::common::Log::L_ERROR , 
				CLOGFMT(NGODPHO , "eval_IPShareStrmLink() catch ice exception[%s] when open streamLink [%s]"),
				ex.ice_name().c_str() , strLinkId.c_str() );
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		catch (...) 
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NGODPHO,"eval_IPShareStrmLink() catch an unknown exception when open streamLink[%s]"),
				strLinkId.c_str() );
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}

	Ice::Int retCost = eval_IPStrmLink(lInfo, sessCtx, hintPD, rcMap, oldCost);
	return retCost;	
}

Ice::Int IpEdgePHO::eval_IPStrmLink(LinkInfo& linkInfo,
									const SessCtx& sessCtx, 
									TianShanIce::ValueMap& hintPD,
									TianShanIce::SRM::ResourceMap& rcMap ,
									const ::Ice::Int oldCost)
{
	Ice::Int	newCost = oldCost;
	const std::string& sessId = sessCtx.sessId;
	if (!linkInfo.linkPrx )
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODPHO,"eval_IPStreamLink() Session [%s] no Streamlink is attached,return with OutOfServiceCost"),
			sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODPHO , "eval_IPStreamLink() session[%s] enter evaluation with oldCost[%d]" ) , 
			sessId.c_str() , oldCost );
	if ( newCost > ::TianShanIce::Transport::MaxCost )
	{	
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODPHO, "eval_IPStreamLink() Session[%s] return oldCost[%d] because oldCost>=MaxCost"),
			sessId.c_str(),newCost);
		return newCost;
	}
	
	Ice::Long	bw2Alloc	= 0;
	Ice::Int	destPort	= 0;
	std::string destAddr, destMac;
	
	::TianShanIce::SRM::ResourceMap resourceMap = sessCtx.resources;
	
	// try to get the bandwidth requirement from the session context
	READ_RES_FIELD(bw2Alloc, resourceMap, rtTsDownstreamBandwidth, bandwidth, lints);

	if (bw2Alloc <= 0)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODPHO, "eval_IPStreamLink() Session[%s] 0 bandwidth has been specified, quit evaluation"),
			sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	// try to get the IP requirement from the session context
	READ_RES_FIELD(destAddr, resourceMap, rtEthernetInterface, destIP, strs);
	READ_RES_FIELD(destPort, resourceMap, rtEthernetInterface, destPort, ints);
	// try to get the EthernetInterface requirement from the session context
	READ_RES_FIELD(destMac, resourceMap, rtEthernetInterface, destMac, strs);

	if (destPort <= 0 || destAddr.empty())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODPHO, "eval_IPStreamLink() Session[%s]  illegal destination [%s]:[%d] has been specified, quit evaluation"), 
			sessId.c_str(),destAddr.c_str(), destPort);
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	//adjust if the hintPD also specify parameters

	{
		if ( hintPD.end() != hintPD.find(PathTicketPD_Field(bandwidth)) )
		{
			::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(bandwidth)];
			if (var.type == TianShanIce::vtLongs && var.lints.size() >0)
				bw2Alloc = (bw2Alloc >0) ? max(bw2Alloc, var.lints[0]) : var.lints[0];
		}

		if ( destAddr.empty() && hintPD.end() != hintPD.find( PathTicketPD_Field(destAddr) ) )
		{
			::TianShanIce::Variant& var = hintPD[ PathTicketPD_Field(destAddr) ];
			if (var.strs.size() >0)
				destAddr = var.strs[0];
		}

		if ( destMac.empty() && hintPD.end() != hintPD.find( PathTicketPD_Field(destMac) ) )
		{
			::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(destMac)];
			if (var.strs.size() >0)
				destMac = var.strs[0];
		}

		if (destPort<=0 && hintPD.end() != hintPD.find(PathTicketPD_Field(destPort)))
		{
			::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(destPort)];
			if (var.ints.size() >0)
				destPort = var.ints[0];
		}
	}
	
	glog(ZQ::common::Log::L_INFO, CLOGFMT(NGODPHO, "eval_IPStreamLink() Session[%s] requested allocation: [%s][%s]:[%d]; bandwidth:[%lld]bps"),
		sessId.c_str(),
		destAddr.c_str(),
		destMac.c_str(), 
		destPort, 
		bw2Alloc );

	Ice::Int	totalStrmCount	= 0;
	Ice::Int	usedStrmCount	= 0;
	Ice::Long	totalBW			= 0;
	Ice::Long	usedBW			= 0;
	std::string destMacOfStrmLnk;


	{
		ZQ::common::MutexGuard gd(_ipResourceLocker);
		LinkIPAttrMap::iterator it = _StreamLinkIPAttrmap.find( linkInfo.linkIden.name );
		if(it==_StreamLinkIPAttrmap.end())
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODPHO, "Session[%s] Can't find the streamlink attr through the id [%s]"),
				sessId.c_str(), linkInfo.linkIden.name.c_str() );
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		totalBW				= it->second._totalBandwidth;
		usedBW				= it->second._usedBandwidth;		
		usedStrmCount		= it->second._usedStreamCount;
		totalStrmCount		= it->second._maxStreamCount;
		destMacOfStrmLnk    = it->second._destMacAddress;
	}

	if( bw2Alloc > ( totalBW - usedBW ) )
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODPHO,"Session[%s] no enough bandwidth ,Required bw[%lld] and used BW[%lld] totalBW[%lld]"),
			sessId.c_str(),bw2Alloc,usedBW,totalBW);
		return TianShanIce::Transport::OutOfServiceCost;
	}

	if ( totalStrmCount <= usedStrmCount ) 
	{
		glog(ZQ::common::Log::L_ERROR,
			CLOGFMT(NGODPHO,"Session[%s] not enough StreamCount, totalStrmCount[%d] usedStrmCount[%d],no available strmCount "),
			sessId.c_str(), totalStrmCount ,usedStrmCount);
		return TianShanIce::Transport::OutOfServiceCost;
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODPHO,"Session[%s] current totalBW[%lld] usedBW[%lld] MaxStreamCount[%d] usedStreamCount[%d] and required BW[%lld]"),
		sessId.c_str() , totalBW , usedBW ,  totalStrmCount ,usedStrmCount, bw2Alloc );

	if ( ::TianShanIce::Transport::MaxCost > newCost && totalBW > 0 )
	{
		if(totalBW - usedBW < bw2Alloc)
			newCost = ::TianShanIce::Transport::OutOfServiceCost;
		else if (usedBW >0)
			newCost = (::Ice::Int) ( usedBW * ::TianShanIce::Transport::MaxCost / totalBW );
		else 
			newCost=0;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(NGODPHO,"Session[%s] bandwidth cost[%d] with usedBandwidth[%lld] totalBandwidth[%lld]"),
		sessId.c_str() , newCost, usedBW, totalBW );
	int		strmCountCost = usedStrmCount*::TianShanIce::Transport::MaxCost / totalStrmCount;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(NGODPHO,"Session[%s] streamcount cost[%d] with usedStreamCount[%d] totalStreamCount[%d]"),
		sessId.c_str(), strmCountCost, usedStrmCount, totalStrmCount );
	newCost = newCost > strmCountCost ? strmCountCost : newCost;
	
	// end of the evaluation
	newCost = max(oldCost, newCost);

	if ( newCost <= ::TianShanIce::Transport::MaxCost )
	{
		try
		{
			::TianShanIce::SRM::Resource res;
			res.attr	= TianShanIce::SRM::raMandatoryNegotiable;
			res.status	= TianShanIce::SRM::rsInProgress;

			::TianShanIce::Variant value;
			value.bRange = false;
			//////////////////////////////////////////////////////////////////////////			
			// a. bandwidth
			res.resourceData.clear();
			value.type = TianShanIce::vtLongs;
			value.lints.clear();
			value.lints.push_back(bw2Alloc);
			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(NGODPHO,"eval_IPStrmLink() Session[%s] set bandwidth to %lld"),
				sessId.c_str(),bw2Alloc);
			hintPD[PathTicketPD_Field(bandwidth)] = value;

			res.resourceData["bandwidth"]=value;
			res.status=TianShanIce::SRM::rsAssigned;
			rcMap[TianShanIce::SRM::rtTsDownstreamBandwidth]=res;


			//////////////////////////////////////////////////////////////////////////
			// b. dest ip and port
			res.resourceData.clear();
			value.type = TianShanIce::vtInts;
			value.ints.clear();
			value.ints.push_back(destPort);
			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(NGODPHO,"eval_IPStrmLink() Session[%s] set destPort to %d"),
				sessId.c_str(),destPort);
			hintPD[PathTicketPD_Field(destPort)] = value;
			res.resourceData["destPort"]=value;


			value.type = TianShanIce::vtStrings;
			value.strs.clear();
			value.strs.push_back(destAddr);
			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(NGODPHO,"eval_IPStrmLink() Session[%s] set destAddr to %s"),
				sessId.c_str(),destAddr.c_str());
			hintPD[PathTicketPD_Field(destAddr)] = value;
			res.resourceData["destIP"]=value;

			//////////////////////////////////////////////////////////////////////////
			// d. dest mac address
			if (destMacOfStrmLnk.length() >=6) // always take DestMac of StreamLink if available
				destMac = destMacOfStrmLnk;

			if (!destMac.empty())
			{
				value.type = TianShanIce::vtStrings;
				value.strs.clear();
				value.strs.push_back(destMac);
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODPHO, "eval_IPStrmLink() Session[%s] set destMac to %s"),
					sessId.c_str(),destMac.c_str());
				hintPD[PathTicketPD_Field(destMac)] = value;
				res.resourceData["destMac"]=value;				
			}
			else
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODPHO,"eval_IPStrmLink() Session[%s] no destMac"),
					sessId.c_str() );
			}

			res.status=TianShanIce::SRM::rsAssigned;
			rcMap[TianShanIce::SRM::rtEthernetInterface]=res;	
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR, 
				CLOGFMT(NGODPHO, "eval_IPStreamLink() Session[%s] exception caught while filling in the ticket private data"),
				sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODPHO, "eval_IPStreamLink() Session[%s] return with newCost=%d"),
		sessId.c_str(), max(oldCost,newCost));

	// step 4. return the higher as the cost
	return max(oldCost, newCost);
}

IPathHelperObject::NarrowResult IpEdgePHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, 
													const SessCtx& sessCtx)
{
	if (!ticket)
		return NR_Unrecognized;

	std::string		type;
	::TianShanIce::Transport::StreamLinkPrx strmLink;
	try 
	{
		strmLink = _helperMgr.openStreamLink(ticket->streamLinkIden);
		type = strmLink->getType();
	}
	catch(...) 
	{
		strmLink = NULL;
		type = "";
	}
	if ( 0 == type.compare( STRMLINK_TYPE_NGOD ) )
	{
		return narrow_IPStrmLink( strmLink , sessCtx , ticket );
	}
	else if( 0 == type.compare( STRMLINK_TYPE_NGOD_SHARELINK ) )
	{
		return narrow_IPStrmShareLink( strmLink , sessCtx , ticket  );
	}
	else
	{
		glog( ZQ::common::Log::L_ERROR,
			CLOGFMT(NGODPHO , "doNarrow() unrecognized stream link type [%s]" ),
			type.c_str() );
		return IPathHelperObject::NR_Unrecognized;
	}
}

IPathHelperObject::NarrowResult	IpEdgePHO::narrow_IPStrmShareLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
											   const SessCtx& sessCtx,
											   const TianShanIce::Transport::PathTicketPtr& ticket)
{
	try
	{
		std::string	strLinkType = strmLink->getType( );
		if ( strcmp( strLinkType.c_str() , STRMLINK_TYPE_NGOD_SHARELINK) != 0 ) 
		{
			glog(ZQ::common::Log::L_ERROR, 
				CLOGFMT( IpEdgePHO , "narrow_IPSharedStrmLink() the strmLink type is[%s] and does not match [%s]" ),
				strLinkType.c_str(),
				STRMLINK_TYPE_NGOD_SHARELINK);
			return NR_Error;
		}
		std::string strLinkId = strmLink->getIdent().name;
		std::string strTragetLinkId = "";
		std::string strSopName;
		{
			ZQ::common::MutexGuard gd(_sharedIPLinkAttrLocker);
			LinkIPShareAttrMap::const_iterator it = _sharedIPLinkAttrmap.find(strLinkId);
			if ( it == _sharedIPLinkAttrmap.end() )
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(NGODPHO,"narrow_IPSharedStrmLink() can't find target linkId through link[%s]"),
					strLinkId.c_str() );
				return NR_Error;					 
			}
			strTragetLinkId = it->second._streamLinkId;
			strSopName = it->second._strSOPName;
		}
		glog(ZQ::common::Log::L_INFO , 
			CLOGFMT(NGODPHO , "narrow_IPSharedStrmLink() find a target link[%s] with linkId[%s],sopName[%s]"),
			strTragetLinkId.c_str() , strLinkId.c_str(), strSopName.c_str()	);
		NarrowResult ret =  inner_narrow_IPStrmLink( strTragetLinkId , sessCtx , ticket );
		//rewrite the sopname with sharelink's attributes
		TianShanIce::Variant varSopName;
		varSopName.bRange	= false;
		varSopName.type		= TianShanIce::vtStrings;
		varSopName.strs.clear();
		varSopName.strs.push_back(strSopName);
		ticket->privateData[PathTicketPD_Field(sop_name)] = varSopName;
		glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NGODPHO,"narrow_IPSharedStrmLink() overwrite with sopName[%s] for ticket[%s],streamLink[%s]"),
			strSopName.c_str(),ticket->ident.name.c_str(),strLinkId.c_str() );
		return ret;
	}
	catch ( const Ice::Exception& ex ) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO , "narrow_IPSharedStrmLink() catch ice exception:%s" ),ex.ice_name().c_str());
		return NR_Error;
	}
	catch (... ) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO , "narrow_IPSharedStrmLink() catch unknown exception " ) );
		return NR_Error;
	}
}

IPathHelperObject::NarrowResult IpEdgePHO::inner_narrow_IPStrmLink(const std::string& strmLinkId, 
													 const SessCtx& sessCtx,
													 const TianShanIce::Transport::PathTicketPtr& ticket)
{
	std::string sessId = sessCtx.sessId;

	std::string	ticketID = ticket->ident.name;

	glog(ZQ::common::Log::L_DEBUG,
		CLOGFMT(NGODPHO,"Session[%s] narrowed ticket with ticketid[%s]"),
		sessId.c_str(),ticketID.c_str());

	ResourceIPData rid;

	TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
	Ice::Long	bw2Alloc		= 0;
	Ice::Long	bwUsed			= 0;
	Ice::Long	bwTotal			= 0;
	Ice::Int	totalStrmCount	= 0;
	Ice::Int	usedStrmCount	= 0;	
	std::string strSopName		= "";	

	READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
	const std::string&	strLinkIdent = strmLinkId ; //strmLink->getIdent().name;

	{
		ZQ::common::MutexGuard gd(_ipResourceLocker);
		LinkIPAttrMap::iterator itIPmap = _StreamLinkIPAttrmap.find(strLinkIdent);
		if ( itIPmap == _StreamLinkIPAttrmap.end())
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO,"narrow_IPStrmLink() Session[%s] No streamlink with ID[%s] is found"),
				sessId.c_str(),strLinkIdent.c_str());
			return NR_Error;
		}
		LinkIPAttr& ipAttr	= itIPmap->second;		
		if ( ipAttr._usedStreamCount >= ipAttr._maxStreamCount ) 
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NGODPHO,"narrow_IPStrmLink() Session[%s] not enough streamcount,"
				"totalStrmCount[%d] usedStrmCount[%d] for strmLink[%s]"),
				sessId.c_str() ,
				ipAttr._maxStreamCount,
				ipAttr._usedStreamCount, 
				strLinkIdent.c_str() );
			return NR_Error;
		}
		if ( ipAttr._usedBandwidth + bw2Alloc > ipAttr._totalBandwidth) 
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NGODPHO,"narrow_IPStrmLink() Session[%s] not enough bandwidth,totalBandwidth[%lld] usedBandwidth[%lld] for strmLink[%s]"),
				sessId.c_str() , ipAttr._totalBandwidth ,
				ipAttr._usedBandwidth , strLinkIdent.c_str()	);
			return NR_Error;
		}

		bwTotal					= ipAttr._totalBandwidth;
		rid._usedBandwidth		= bw2Alloc;
		ipAttr._usedStreamCount ++ ;		
		ipAttr._usedBandwidth	+= bw2Alloc;
		totalStrmCount			= ipAttr._maxStreamCount;
		usedStrmCount			= ipAttr._usedStreamCount;
		bwUsed					= ipAttr._usedBandwidth;
		strSopName				= ipAttr._strSOPName;
	}

	TianShanIce::Variant varLinkType;
	TianShanIce::Variant varLinkId;
	TianShanIce::Variant varSopName;

	varLinkType.bRange = false;
	varLinkType.type = TianShanIce::vtStrings;
	varLinkType.strs.clear();
	varLinkType.strs.push_back( STRMLINK_TYPE_NGOD );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkType)] = varLinkType;

	varLinkId.bRange = false;
	varLinkId.type = TianShanIce::vtStrings;
	varLinkId.strs.clear();
	varLinkId.strs.push_back( strLinkIdent );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varLinkId;

	varSopName.bRange	= false;
	varSopName.type		= TianShanIce::vtStrings;
	varSopName.strs.clear();
	varSopName.strs.push_back(strSopName);
	ticket->privateData[PathTicketPD_Field(sop_name)] = varSopName;
	glog(ZQ::common::Log::L_INFO , 
		CLOGFMT(NGODPHO , "narrow_IPStrmLink() Session[%s] set sopname[%s]" ),
		sessId.c_str() ,
		strSopName.c_str() );

	glog(ZQ::common::Log::L_INFO,CLOGFMT(NGODPHO,"narrow_IPStrmLink() Session[%s] narrowed with NeedBW[%lld]"
		" and UsedBW[%lld] TotalBW[%lld] TotalStreamCount[%d] usedStreamCount[%d]  streamLink[%s]"),
		sessId.c_str() , bw2Alloc, 
		bwUsed , bwTotal , 
		totalStrmCount, usedStrmCount ,
		strmLinkId.c_str() );

	{
		ZQ::common::MutexGuard gd(_ipResourceLocker);
		_ipResourceDataMap[ticketID]=rid;
	}
	return NR_Narrowed;
}
IPathHelperObject::NarrowResult IpEdgePHO::narrow_IPStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink,
															 const SessCtx& sessCtx,
															 const TianShanIce::Transport::PathTicketPtr&  ticket)
{
	std::string	strmLinkId;
	try
	{
		strmLinkId = strmLink->getIdent().name;
	}
	catch (const Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR , 
			CLOGFMT(NGODPHO,"narrow_IPStrmLink() catch an ice exception when get streamlink's identity" ),
			ex.ice_name().c_str() );
		return NR_Error;
	}
	catch (...) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO,"narrow_IPStrmLink() get streamlink's ID failed"));
		return NR_Error;
	}
	return inner_narrow_IPStrmLink(strmLinkId,sessCtx,ticket);
	
}

void IpEdgePHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	try
	{
		TianShanIce::ValueMap& ticketPD = ticket->privateData;
		TianShanIce::ValueMap::const_iterator itLinkType = ticketPD.find( PathTicketPD_Field(ownerStreamLinkType) ) ;
		if ( itLinkType == ticketPD.end()  || itLinkType->second.type != TianShanIce::vtStrings ||itLinkType->second.strs.size() == 0  ) 
		{
			glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NGODPHO,"no ticket owner link type is found"));
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkType->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO,"doFreeResource() can' get streamlink from ticket"));
			return ;
		}
		std::string strStreamlinkID = itLinkId->second.strs[0];
		Ice::Identity& ticketID = ticket->ident;

		if ( strcmp( STRMLINK_TYPE_NGOD_SHARELINK , strLinkType.c_str() ) == 0 ) 
		{
			{
				ZQ::common::MutexGuard gd(_sharedIPLinkAttrLocker);
				LinkIPShareAttrMap::const_iterator it = _sharedIPLinkAttrmap.find(strStreamlinkID);
				if( it == _sharedIPLinkAttrmap.end() )
				{

					glog(ZQ::common::Log::L_ERROR , 
						CLOGFMT(NGODPHO ,"doFreeResources() can't find target link with IP shared linkId [%s]" ),
						strStreamlinkID.c_str() );
					return;
				}
				glog(ZQ::common::Log::L_INFO ,
					CLOGFMT(NGODPHO,"doFreeResources() find target streamLinkId[%s] through IP shared linkid[%s] for ticket[%s]"),
					it->second._streamLinkId.c_str() , strStreamlinkID.c_str(),ticketID.name.c_str() );
				strStreamlinkID = it->second._streamLinkId;
				strLinkType = STRMLINK_TYPE_NGOD;
			}
		}

		if ( strcmp( STRMLINK_TYPE_NGOD , strLinkType.c_str() ) == 0 ) 
		{
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			ResourceIPDataMap::iterator itAlloc =_ipResourceDataMap.find( ticketID.name );
			if(itAlloc == _ipResourceDataMap.end())
			{
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(NGODPHO,"doFreeResource() streamlink [%s] ticket[%s] no allocated IP resource"),
					strStreamlinkID.c_str() , ticketID.name.c_str() );
				return;
			}
			LinkIPAttrMap::iterator it=_StreamLinkIPAttrmap.find( strStreamlinkID );
			if( it == _StreamLinkIPAttrmap.end() )
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(NGODPHO,"doFreeResource() streamlink [%s] ticket[%s] can't find strmlink attr "),
					strStreamlinkID.c_str(),ticketID.name.c_str());
				return;
			}
			LinkIPAttr& ipAttr		= it->second;
			ResourceIPData& rid		= itAlloc->second;
			ipAttr._usedBandwidth	-= rid._usedBandwidth;
			ipAttr._usedStreamCount -- ;
			
			ipAttr._usedStreamCount = ipAttr._usedStreamCount > 0 ? ipAttr._usedStreamCount : 0 ;
			ipAttr._usedBandwidth	= ipAttr._usedBandwidth > 0 ? ipAttr._usedBandwidth : 0 ;	

			_ipResourceDataMap.erase(itAlloc);

			glog(ZQ::common::Log::L_INFO ,
				CLOGFMT (IpEdgePHO , "doFreeResource() streamLink[%s] free resource and now "
				"totalBW[%lld] usedBW[%lld] totalStreamCount[%d] usedStreamCount[%d]" ),
				strStreamlinkID.c_str() ,
				ipAttr._totalBandwidth,
				ipAttr._usedBandwidth , 
				ipAttr._maxStreamCount,
				ipAttr._usedStreamCount);
		}
		else
		{
			glog(ZQ::common::Log::L_WARNING , CLOGFMT(NGODPHO,"unrecognized stream link type [%s]") , strLinkType.c_str() );
			return ; 
		}

	}
	catch( const TianShanIce::BaseException& ex )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO,"doFreeResource() catch a tianshan exception:%s"),ex.message.c_str());
		return;
	}
	catch( const Ice::Exception& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO,"doFreeResource() catch a ice exception:%s"),e.ice_name().c_str());
		return;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NGODPHO,"doFreeResource() catch a unknown exception"));
		return;
	}	
}
void IpEdgePHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{

}


}}//namespace ZQTianShan::AccreditedPath
