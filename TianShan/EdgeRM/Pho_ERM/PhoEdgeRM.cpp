 #include "PhoEdgeRM.h"
#include "Log.h"
#include "strHelper.h"
#include "TianShanIceHelper.h"
#include <time.h>
#include <algorithm>

namespace ZQTianShan {
namespace AccreditedPath {

static ConfItem ERMEdge_IP[] = {
	{ "EdgeRMEndpoint",			::TianShanIce::vtStrings,	false,	"EdgeRM:tcp -h host -p port",	false},
//	{ "RoutingMode",			::TianShanIce::vtInts,		false,	"1",		false }, // 1 for Optimistic, 2 for Pessimistic
	{ "Qam.ModulationFormat",	::TianShanIce::vtInts,		false,	"8",		false },
	{ "QAM-ZONE",			    ::TianShanIce::vtStrings,	false,	"",	        false},//device of zone, if the zone is not null, the QAM-IDs must be set NULL
	{ "QAM-IDs",				::TianShanIce::vtStrings,	false,	"",			false }, //use ; as a splitter
//	{ "MaxStreamCount",			::TianShanIce::vtInts,		false,	"80",		false },
//	{ "TotalBandWidth",			::TianShanIce::vtLongs,		false,	"20000",	false },
	{ NULL,						::TianShanIce::vtInts,		true,	"",			false },
	};

class MyRandom 
{
public:
	ptrdiff_t operator() (ptrdiff_t max) 
	{
		static bool bInit = false;
		if( !bInit )
		{
			srand(time(NULL));
			bInit = true;
		}
		if( max  > 0 )
			return rand()%max;
		else 
			return 0;
	}
};

EdgeRMPHO::EdgeRMPHO(IPHOManager& mgr, ZQTianShan::EdgeRM::PhoEdgeRMEnv& env)
: IStreamLinkPHO(mgr), _env(env)
{
	_phoManager=&mgr;
}

EdgeRMPHO::~EdgeRMPHO()
{
	_helperMgr.unregisterStreamLinkHelper( STRMLINK_TYPE_TianShanERM );
}
/// no problem
bool EdgeRMPHO::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;

	// address the schema definition
	if ( 0 == strcmp(type, STRMLINK_TYPE_TianShanERM) )
		config = ERMEdge_IP;

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
/// no problem
void EdgeRMPHO::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
	{
		type = "NULL";
	}
	if (0 == strcmp( STRMLINK_TYPE_TianShanERM, type))
	{
		validate_EdgeRMStrmLinkConfig( identStr, configPD );
	}
	else
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("ERMEdgePHO",1001,"unsupported type [%s] in EdgeRMPHO", type);
	}
}
//no problem
void EdgeRMPHO::validate_EdgeRMStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, " enter validate_EdgeRMStrmLinkConfig() %s Configuration: link[%s]"), STRMLINK_TYPE_TianShanERM, identStr);
	EdgeRMLinkAttr lia;
	lia._streamLinkID	= identStr;
//	lia._usedStreamCount = 0;
//	lia._usedBandwidth = 0;

	TianShanIce::Variant var;
	var.bRange			= false;
	try
	{
		//get EdgeRMEndpoint
		::std::string edgeRMEndpoint;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "EdgeRMEndpoint", edgeRMEndpoint);
			lia._strEdgeRMEndpoint = edgeRMEndpoint;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"StreamLink[%s] get EdgeRM service Endpoint[%s]"), identStr, edgeRMEndpoint.c_str());
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get EdgeRM service Endpoint caught exception(%s:%d:%s)", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get EdgeRM service Endpoint parameter", identStr);
		}
		//get QAM modulationFormat
		::Ice::Int modulationFormat = 0;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "Qam.ModulationFormat", modulationFormat);
			lia._modulationFormat = modulationFormat;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"StreamLink[%s] get ModulationFormat[%d]"), identStr, modulationFormat);
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get ModulationFormat caught exception(%s:%d:%s)", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get ModulationFormat parameter", identStr);
		}

/*		//get RoutingMode
		Ice::Int iRoutingMode = 0;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "RoutingMode", iRoutingMode);
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"StreamLink[%s] get RoutingMode[%d]"), identStr, iRoutingMode);
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get RoutingMode caught exception: %s:%d:%s", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get RoutingMode", identStr);
		}
		if (iRoutingMode == 1)
		{
			lia._routingMode = OptimisticMode;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"StreamLink[%s] get RoutingMode[Optimistic]"), identStr);
		}
		else
		{
			lia._routingMode = PessimisticMode;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"StreamLink[%s] get RoutingMode[Pessimistic]"), identStr);
		}
*/
		//get QAM-ZONE
		::std::string edgeRMZONE;
		try
		{
			::ZQTianShan::Util::getValueMapDataWithDefault(configPD, "QAM-ZONE","", edgeRMZONE);
			lia._strQAMZONE = edgeRMZONE;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"StreamLink[%s] get QAM-ZONE[%s]"), identStr, edgeRMZONE.c_str());
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get QAM-ZONE caught exception(%s:%d:%s)", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get QAM-ZONE parameter", identStr);
		}

	    //get QAM-IDs
		//::std::string strQAMIDs;
		try
		{
			bool bRange = false;
			::ZQTianShan::Util::getValueMapData(configPD, "QAM-IDs", lia._vecQAMID, bRange);
			if (!lia._vecQAMID.empty())
			{
				::std::string strQAMIDList = "";

				int iSize = lia._vecQAMID.size();
				for (int i = 0; i < iSize; i++)
				{
					strQAMIDList += lia._vecQAMID[i];
					if (i < iSize - 1)
						strQAMIDList += ::std::string(", ");
				}
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"StreamLink[%s] QAM-IDs[%s]"), identStr, strQAMIDList.c_str());
			}
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get QAM-IDs caught exception(%s:%d:%s)", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"StreamLink[%s] failed to get QAM-IDs parameter", identStr);
		}

		TianShanIce::Transport::StreamLinkPrx strmLinkPrx = NULL;
		try
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"open StreamLink[%s]"), identStr);
			strmLinkPrx =_phoManager->openStreamLink(identStr);  
		}
		catch (Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("EdgeRMPHO",1011,"failed to open StreamLink[%s]", identStr);
		}

		if(strmLinkPrx)
		{
			IdentCollection idc;
			try
			{
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"list PathTickets by StreamLink[%s]"), identStr);
				idc=_phoManager->listPathTicketsByLink(strmLinkPrx);
			}
			catch (Ice::ObjectNotExistException&)
			{
				idc.clear();
			}
			IdentCollection::const_iterator itID=idc.begin();
			for(;itID!=idc.end();itID++)
			{
				::TianShanIce::Transport::PathTicketPrx ticketprx=_phoManager->openPathTicket(*itID);
				if(ticketprx)
				{
					//TianShanIce::ValueMap data=ticketprx->getPrivateData();
					TianShanIce::Variant    onDemandSession;

					try
					{
						TianShanIce::ValueMap	PDData	=	ticketprx->getPrivateData();
						onDemandSession = PDField(PDData, PathTicketPD_Field(OnDemandSessionId));
						if (onDemandSession.strs.size()!=0 ) 
						{
							_env.addOnDemandSession(onDemandSession.strs[0]);
						}
					}
					catch(::TianShanIce::InvalidParameter& e)
					{
						envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"validate ticket [%s] caught exception [%s]"),itID->name.c_str(), e.message.c_str());
					}
					catch (...)
					{
					}
				}
			}
		}
		//update stream link attribute map
		{
			::ZQ::common::MutexGuard guard(_linkAttrMapMutex);
			EdgeRMLinkAttrMap::iterator iter = _linkAttrMap.find(identStr);
			if ( iter != _linkAttrMap.end() )
			{
				iter->second = lia;
			}
			else
			{
				_linkAttrMap[identStr] = lia;
			}
		}
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"[%s]update EdgeRM PHO streamlink"), identStr);
	}	
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	catch (const ::Ice::Exception& e)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"[%s]validate Config caught exception [%s]"), identStr, e.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"[%s]validate Config caught unknown exception(%d)"), identStr, SYS::getLastErr());
	}
}

#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
	if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMPHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}
//no problem
Ice::Int EdgeRMPHO::doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& ticketPrivateData, const ::Ice::Int oldCost)
{
	EdgeRMLinkAttr linktattr;
	{
		ZQ::common::MutexGuard gd(_linkAttrMapMutex);
		EdgeRMLinkAttrMap::iterator iter = _linkAttrMap.find( linkInfo.linkIden.name );
		if(iter==_linkAttrMap.end())
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO," doEvaluation() session[%s] can't find the streamlink attr through the id[%s]"),
				sessCtx.sessId.c_str(), linkInfo.linkIden.name.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		linktattr = iter->second;
	}
	return eval_EdgeRMStrmLink(linkInfo, sessCtx, ticketPrivateData, linkInfo.rcMap,oldCost, linktattr);
}
///
Ice::Int EdgeRMPHO::eval_EdgeRMStrmLink(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& ticketPrivateData, TianShanIce::SRM::ResourceMap& resourceMap, const ::Ice::Int oldCost, EdgeRMLinkAttr& linktattr)
{
	Ice::Long lstart  = ZQTianShan::now();
	Ice::Int	newCost = oldCost;
	const std::string sessId = sessCtx.sessId;

	::TianShanIce::Transport::StreamLinkPrx strmLinkPrx = NULL;
	try
	{
		strmLinkPrx = ::TianShanIce::Transport::StreamLinkPrx::checkedCast(linkInfo.linkPrx);
	}
	catch (::Ice::Exception &ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"eval_EdgeRMStrmLink()SessID[%s] failed to get Streamlink proxy caught exception(%s)"), sessId.c_str(), ex.ice_name().c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"eval_EdgeRMStrmLink()SessID[%s] failed to get Streamlink proxy caught unknown exception"), sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	::std::string phoAllocName = linkInfo.linkIden.name + PHOALLOCATION_KEY_SEPARATOR + sessCtx.sessId;

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO , "eval_EdgeRMStrmLink()[%s] enter evaluation with oldCost[%d]" ), phoAllocName.c_str(),  oldCost);
	if ( newCost > ::TianShanIce::Transport::MaxCost )
	{	
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "eval_EdgeRMStrmLink()[%s] return oldCost[%d] because oldCost >= MaxCost"),phoAllocName.c_str(), newCost);
		return newCost;
	}

	//step 1. create allocation

	::TianShanIce::EdgeResource::AllocationPrx allocationPrx;
	allocationPrx = createAllocation(linkInfo, sessCtx, ticketPrivateData, resourceMap, linktattr);
	if(!allocationPrx)
	{
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
    
	//step2. doEvaluation for this allocation
	try
	{	
		std::string strAllocProxyString = allocationPrx->ice_toString();
		std::string strAllocSessionId = allocationPrx->getId();

		envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"eval_EdgeRMStrmLink()[%s] create allocation object successful"), phoAllocName.c_str());
		{
			::TianShanIce::Variant var;
			var.type = ::TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.push_back(strAllocSessionId);
			MAPSET(TianShanIce::ValueMap, ticketPrivateData, "AllocationSessionId", var);
		}
		{
			::TianShanIce::Variant var;
			var.type = ::TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.push_back(strAllocProxyString);
			MAPSET(TianShanIce::ValueMap, ticketPrivateData, "AllocationProxyString", var);
		}

		::TianShanIce::SRM::ResourceMap resources = allocationPrx->getResources();
				::TianShanIce::SRM::ResourceMap::iterator resItor; 
#if  0
		//dump allocation resource data
		for(resItor = resources.begin(); resItor != resources.end(); resItor++)
		{
			TianShanIce::ValueMap & resourcedata = resItor->second.resourceData;
			char szBuf[1024];
			memset(szBuf, 0, 1024);
			snprintf(szBuf, sizeof(szBuf) - 1, "[%s]session ",strAllocSessionId.c_str());
			ZQTianShan::dumpValueMap(resourcedata, szBuf, dumpLine);
		}
#endif

		//step2.1. get resource of TianShanIce::SRM::rtPhysicalChannel
		resItor = resources.find(TianShanIce::SRM::rtPhysicalChannel);
		if(resItor == resources.end())
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"eval_EdgeRMStrmLink()[%s] missing resource of 'rtPhysicalChannel'"), phoAllocName.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}

		::TianShanIce::SRM::Resource& res = resItor->second;
		::TianShanIce::ValueMap::iterator itorvmap;
		itorvmap = res.resourceData.find("channelId");
		if(itorvmap == res.resourceData.end() || itorvmap->second.lints.size() < 1)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"eval_EdgeRMStrmLink()[%s] missing 'channelId' key from resource of rtPhysicalChannel"), phoAllocName.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		::TianShanIce::Variant& varChannelId = itorvmap->second;
		newCost = 1* COST_MAX / varChannelId.lints.size();

		//step 2.2. add strmlink_type and streamlinkId to ticket privatedata
		//set ownerStreamLinkType to ticket privatedata
		TianShanIce::Variant varStreamLinkType;
		varStreamLinkType.bRange = false;
		varStreamLinkType.type = TianShanIce::vtStrings;
		varStreamLinkType.strs.clear();
		varStreamLinkType.strs.push_back(linkInfo.linkType);
		ticketPrivateData[PathTicketPD_Field(ownerStreamLinkType)] = varStreamLinkType;

		//set ownerStreamLinkId to ticket privatedata	
		TianShanIce::Variant varStreamLinkId;
		varStreamLinkId.bRange = false;
		varStreamLinkId.type = TianShanIce::vtStrings;
		varStreamLinkId.strs.clear();
		varStreamLinkId.strs.push_back(linkInfo.linkIden.name);
		ticketPrivateData[PathTicketPD_Field(ownerStreamLinkId)] = varStreamLinkId;
	}
	catch (::Ice::Exception &ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"eval_EdgeRMStrmLink()[%s] caught exception(%s)"), phoAllocName.c_str(), ex.ice_name().c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"eval_EdgeRMStrmLink()[%s] caught unknown exception(%d)"), phoAllocName.c_str(), SYS::getLastErr());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	//step 3. create pho allocation according to LinkId#SessionID

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO, "eval_EdgeRMStrmLink()[%s] do Evaluation with newCost=%d took %dms"), phoAllocName.c_str(), max(oldCost,newCost), ZQTianShan::now() - lstart);

	// step 5. return the higher as the cost
	return max(oldCost, newCost);
}

IPathHelperObject::NarrowResult EdgeRMPHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, 
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
	if ( 0 == type.compare( STRMLINK_TYPE_TianShanERM ) )
	{
		return narrow_EdgeRMStrmLink( strmLink , sessCtx , ticket );
	}

	envlog( ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "doNarrow() unrecognized stream link type [%s]"), type.c_str() );
	return IPathHelperObject::NR_Unrecognized;
}

IPathHelperObject::NarrowResult EdgeRMPHO::narrow_EdgeRMStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink,
															 const SessCtx& sessCtx,
															 const TianShanIce::Transport::PathTicketPtr&  ticket)
{
	Ice::Long lstart  = ZQTianShan::now();

	///setp1. get session Id
	std::string sessId = sessCtx.sessId;
	std::string ticketID = ticket->ident.name;

	///setp2. get StreamLink Id
	::std::string	strmLinkId = "", strmLinkType = "";
	IPathHelperObject::NarrowResult narrowresult = NR_Error;
	try
	{
		strmLinkId = strmLink->getIdent().name;
		strmLinkType = strmLink->getType();
	}
	catch (const Ice::Exception& ex) 
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "session[%s]ticketId[%s] failed to get streamID caught expception(%s)"), sessId.c_str(), ticketID.c_str(),ex.ice_name().c_str());
		return narrowresult;
	}
	catch (...) 
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "session[%s]ticketId[%s] failed to get streamID caught expception"), sessId.c_str(), ticketID.c_str());
		return narrowresult;
	}
	std::string strAllocId="";
    ///setp3. allocation serve
	::TianShanIce::EdgeResource::AllocationPrx allocPrx = NULL;
	try
	{
		TianShanIce::ValueMap& ticketPD = ticket->privateData;

		TianShanIce::ValueMap::iterator itorASK = ticketPD.find("AllocationSessionId");
		if(itorASK == ticketPD.end() || itorASK->second.strs.size() < 1)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT (S6EdgeRMPHO , "session[%s]ticketId[%s]  can't find AllocationSessKey key" ), sessId.c_str(), ticketID.c_str());
			return narrowresult;
		}
		strAllocId = itorASK->second.strs[0];

		itorASK = ticketPD.find("AllocationProxyString");
		if(itorASK == ticketPD.end() || itorASK->second.strs.size() < 1)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT (S6EdgeRMPHO , "session[%s]ticketId[%s]  can't find AllocationSessKey key" ), sessId.c_str(), ticketID.c_str());
			return narrowresult;
		}
		std::string strAllocProxyString = itorASK->second.strs[0];

		allocPrx = TianShanIce::EdgeResource::AllocationPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(strAllocProxyString));
		allocPrx->serve();
	}
	catch(::TianShanIce::BaseException &ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "session[%s]ticketId[%s] failed to serve allocation[%s] caught exception(%s:%d:%s)"),
			 sessId.c_str(),ticketID.c_str(),strAllocId.c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		return narrowresult;
	}
	catch(::Ice::Exception &ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "session[%s]ticketId[%s] failed to serve allocation[%s] caught exception(%s)"),
			sessId.c_str(),ticketID.c_str(), strAllocId.c_str(), ex.ice_name().c_str());
		return narrowresult;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "session[%s]ticketId[%s] failed to serve allocation[%s] caught unknown exception"), 
			sessId.c_str(),ticketID.c_str(), strAllocId.c_str());
		return narrowresult;
	}

	///setp6. get resource from allocation

	Ice::Long channelId =-1;
	Ice::Byte	FEC = -1;
	Ice::Int symbolRate = -1;
	Ice::Byte modulationFormat = -1;
	Ice::Long PnId = -1;
	std::string edgeDeviceIP = "";
	std::string edgeDeviceMac = "";
	std::string edgeDeviceZone ="";
	std::string edgeDeviceName ="";
	Ice::Int destPort = -1;
	Ice::Long	bw2Alloc = 0;
	try
	{
//		::std::string strAllocId = allocPrx->getId();
		::TianShanIce::StatedObjInfo allocInfo = allocPrx->getInfo();
		::TianShanIce::SRM::ResourceMap allocResourceMap = allocPrx->getResources();

		::TianShanIce::Variant var;
#if  0
		///dump allocation resource data
		::TianShanIce::SRM::ResourceMap::iterator resItor; 
		for(resItor = allocResourceMap.begin(); resItor != allocResourceMap.end(); resItor++)
		{
			TianShanIce::ValueMap & resourcedata = resItor->second.resourceData;
			char szBuf[1024];
			memset(szBuf, 0, 1024);
			snprintf(szBuf, sizeof(szBuf) - 1, "[%s] ",strAllocId.c_str());
			ZQTianShan::dumpValueMap(resourcedata, szBuf, dumpLine);
		}
#endif
		READ_RES_FIELD(edgeDeviceIP, allocResourceMap, rtPhysicalChannel, edgeDeviceIP, strs);
		READ_RES_FIELD(edgeDeviceMac, allocResourceMap, rtPhysicalChannel, edgeDeviceMac, strs);
		READ_RES_FIELD(edgeDeviceZone, allocResourceMap, rtPhysicalChannel, edgeDeviceZone, strs);
		READ_RES_FIELD(edgeDeviceName, allocResourceMap, rtPhysicalChannel, edgeDeviceName, strs);
		READ_RES_FIELD(destPort, allocResourceMap, rtPhysicalChannel, destPort, ints);
		READ_RES_FIELD(FEC, allocResourceMap, rtAtscModulationMode, FEC, bin);
		READ_RES_FIELD(symbolRate, allocResourceMap, rtAtscModulationMode, symbolRate, ints);
		READ_RES_FIELD(channelId, allocResourceMap, rtPhysicalChannel, channelId, lints);
		READ_RES_FIELD(modulationFormat, allocResourceMap, rtAtscModulationMode, modulationFormat, bin);
		READ_RES_FIELD(PnId, allocResourceMap, rtMpegProgram, Id, lints);

#pragma message ( __MSGLOC__ "TODO: check each parameter")
		//set bandwidth
		::std::string strProp;
		::ZQTianShan::Util::getPropertyData(allocInfo.props, SYS_PROP(Bandwidth), strProp);
		bw2Alloc = atol(strProp.c_str());
		var.type = ::TianShanIce::vtLongs;
		var.bRange = false;
		var.lints.clear();
		var.lints.push_back(bw2Alloc);
		ticket->privateData[PathTicketPD_Field(bandwidth)] = var;
		PutResourceMapData(ticket->resources, ::TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", var, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set bandwidth to [%d]"), sessId.c_str(), strAllocId.c_str(), bw2Alloc);

		::TianShanIce::Variant var_QamIp;
		//set QAM ip
		var_QamIp.type = ::TianShanIce::vtStrings;
		var_QamIp.bRange = false;
		var_QamIp.strs.clear();
		var_QamIp.strs.push_back(edgeDeviceIP);
		ticket->privateData[PathTicketPD_Field(Qam.IP)] = var_QamIp;
		ticket->privateData[PathTicketPD_Field(destAddr)] = var_QamIp;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtEthernetInterface, "destIP", var_QamIp, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set destAddr to [%s]"), sessId.c_str(), strAllocId.c_str(), var_QamIp.strs[0].c_str());

		::TianShanIce::Variant var_QamMac;
		///set QAM mac
		var_QamMac.type = ::TianShanIce::vtStrings;
		var_QamMac.bRange = false;
		var_QamMac.strs.clear();
		var_QamMac.strs.push_back(edgeDeviceMac);
		ticket->privateData[PathTicketPD_Field(Qam.Mac)] = var_QamMac;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtEthernetInterface, "destMac", var_QamMac, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set destMac to [%s]"), sessId.c_str(), strAllocId.c_str(), var_QamMac.strs[0].c_str());

		::TianShanIce::Variant var_QambasePort;
		//set QAM.basePort
		var_QambasePort.type = ::TianShanIce::vtInts;
		var_QambasePort.bRange = false;
		var_QambasePort.ints.clear();
		var_QambasePort.ints.push_back(destPort);
		ticket->privateData[PathTicketPD_Field(Qam.basePort)] = var_QambasePort;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtEthernetInterface, "destPort", var_QambasePort, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set destPort to [%d]"), sessId.c_str(), strAllocId.c_str(), destPort);

		::TianShanIce::Variant var_QamFec;
		//set QAM FEC
		var_QamFec.type = ::TianShanIce::vtBin;
		var_QamFec.bRange = false;
		var_QamFec.bin.clear();
		var_QamFec.bin.push_back(FEC);
		ticket->privateData[PathTicketPD_Field(Qam.fec)] = var_QamFec;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtPhysicalChannel, "FEC", var_QamFec, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set FEC to [%d]"), sessId.c_str(), strAllocId.c_str(), FEC);

		::TianShanIce::Variant var_QamSymbolRate;
		//set QAM symbolRate
		var_QamSymbolRate.type = ::TianShanIce::vtInts;
		var_QamSymbolRate.bRange = false;
		var_QamSymbolRate.ints.clear();
		var_QamSymbolRate.ints.push_back(symbolRate);
		ticket->privateData[PathTicketPD_Field(Qam.symbolRate)] = var_QamSymbolRate;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtAtscModulationMode, "symbolRate", var_QamSymbolRate, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set symbolRate to [%d]"), sessId.c_str(), strAllocId.c_str(), symbolRate);

		::TianShanIce::Variant var_QamFrequency;
		//set QAM frequency
		var_QamFrequency.type = ::TianShanIce::vtInts;
		var_QamFrequency.bRange = false;
		var_QamFrequency.lints.clear();
		var_QamFrequency.lints.push_back(channelId);
		var_QamFrequency.ints.clear();
		var_QamFrequency.ints.push_back((Ice::Int)channelId);
		ticket->privateData[PathTicketPD_Field(Qam.frequency)] = var_QamFrequency;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtPhysicalChannel , "channelId", var_QamFrequency, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set channelId to [%d]"), sessId.c_str(), strAllocId.c_str(), channelId);

        ::TianShanIce::Variant var_QamName;
        //set QAM name
		var_QamName.type = ::TianShanIce::vtStrings;
		var_QamName.bRange = false;
		var_QamName.strs.clear();
		var_QamName.strs.push_back(edgeDeviceName);
		ticket->privateData[PathTicketPD_Field(Qam.name)] = var_QamName;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceName", var_QamName, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set edgeDeviceName to [%s]"), sessId.c_str(), strAllocId.c_str(), var_QamName.strs[0].c_str());

		 ::TianShanIce::Variant var_QamZone;
		//set QAM devicegroup
		//这里有问题 需要确认
		var_QamZone.type = ::TianShanIce::vtStrings;
		var_QamZone.bRange = false;
		var_QamZone.strs.clear();
		var_QamZone.strs.push_back(edgeDeviceZone);
		ticket->privateData[PathTicketPD_Field(Qam.Group)] = var_QamZone;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceZone", var_QamZone, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set edgeDeviceZone to [%s]"), sessId.c_str(), strAllocId.c_str(), var_QamZone.strs[0].c_str());

		::TianShanIce::Variant var_QamModulationFormat;
		//set QAM modulationformat
		var_QamModulationFormat.type = ::TianShanIce::vtInts;
		var_QamModulationFormat.bRange = false;
		var_QamModulationFormat.bin.clear();
		var_QamModulationFormat.bin.push_back(modulationFormat);
		var_QamModulationFormat.ints.clear();
		var_QamModulationFormat.ints.push_back((int)modulationFormat);
		ticket->privateData[PathTicketPD_Field(Qam.mode)] = var_QamModulationFormat;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtAtscModulationMode, "modulationFormat", var_QamModulationFormat, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set modulationFormat to [%d]"), sessId.c_str(), strAllocId.c_str(), modulationFormat);

		::TianShanIce::Variant var_QamPN;
		//set PN
		var_QamPN.type = ::TianShanIce::vtInts;
		var_QamPN.bRange = false;
		var_QamPN.lints.clear();
		var_QamPN.lints.push_back(PnId);
		var_QamPN.ints.clear();
		var_QamPN.ints.push_back((Ice::Int)PnId);
		ticket->privateData[PathTicketPD_Field(PN)] = var_QamPN;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtMpegProgram, "Id", var_QamPN, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]allocId[%s] set pn to [%d]"), sessId.c_str(), strAllocId.c_str(), PnId);

		{
			::TianShanIce::Variant var;
			var.type = ::TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.push_back(sessId);
			ticket->privateData[PathTicketPD_Field(OnDemandSessionId)] = var;
		}

		{
			//set ownerStreamLinkType to ticket privatedata
			TianShanIce::Variant varStreamLinkType;
			varStreamLinkType.bRange = false;
			varStreamLinkType.type = TianShanIce::vtStrings;
			varStreamLinkType.strs.clear();
			varStreamLinkType.strs.push_back(strmLinkType);
			ticket->privateData[PathTicketPD_Field(ownerStreamLinkType)] = varStreamLinkType;
		}
		{
			//set ownerStreamLinkId to ticket privatedata	
			TianShanIce::Variant varStreamLinkId;
			varStreamLinkId.bRange = false;
			varStreamLinkId.type = TianShanIce::vtStrings;
			varStreamLinkId.strs.clear();
			varStreamLinkId.strs.push_back(strmLinkId);
			ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varStreamLinkId;
		}
		_env.addOnDemandSession(sessId);
		narrowresult = NR_Narrowed;
	}
	catch(::TianShanIce::BaseException &ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "session[%s]ticketId[%s] failed to get resource from allocation[%s] caught exception(%s:%d:%s)"),
			sessId.c_str(),ticketID.c_str(), strAllocId.c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		narrowresult =  NR_Error;
	}
	catch(::Ice::Exception &ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "session[%s]ticketId[%s] failed to get resource from allocation[%s]caught exception(%s)"),
			sessId.c_str(),ticketID.c_str(), strAllocId.c_str(), ex.ice_name().c_str());
		narrowresult =  NR_Error;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "session[%s]ticketId[%s] failed to get resource from allocation[%s] caught unknown exception"), 
			sessId.c_str(),ticketID.c_str(), strAllocId.c_str());
		narrowresult = NR_Error;
	}
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO, "session[%s]ticketId[%s]AllocationSessKey[%s] do narrowed took %d ms"),
		sessId.c_str(),ticketID.c_str(), strAllocId.c_str(), ZQTianShan::now() - lstart);

	return narrowresult;
}

::TianShanIce::EdgeResource::AllocationPrx
 EdgeRMPHO::createAllocation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& ticketPrivateData, TianShanIce::SRM::ResourceMap& resourceMap, EdgeRMLinkAttr& linktattr)
{	
	Ice::Long lstart  = ZQTianShan::now();
	::std::string phoAllocName = linkInfo.linkIden.name + PHOALLOCATION_KEY_SEPARATOR + sessCtx.sessId;

	envlog(ZQ::common::Log::L_INFO,CLOGFMT(EdgeRMPHO, "createAllocation()[%s] enter"),phoAllocName.c_str());

	::TianShanIce::EdgeResource::AllocationPrx allocPrx = NULL;
	try
	{
		//step 1. create Allocation object 
		::std::string strAllocSessionId = "";
		Ice::Long	bw2Alloc	= 0;
		Ice::Int	modulationFormat  = -1;
		::std::string QamZone="";

		::TianShanIce::SRM::ResourceMap sessResourceMap = sessCtx.resources;

		//step2.1. try to get the bandwidth requirement from the session context	
		READ_RES_FIELD(bw2Alloc, sessResourceMap, rtTsDownstreamBandwidth, bandwidth, lints);

		//step2.2 adjust if the ticketPrivateData also specify the bandwidth to the max of them
		if ( ticketPrivateData.end() != ticketPrivateData.find(PathTicketPD_Field(bandwidth)) )
		{
			::TianShanIce::Variant& var = ticketPrivateData[PathTicketPD_Field(bandwidth)];
			if (var.type == TianShanIce::vtLongs && var.lints.size() >0)
				bw2Alloc = (bw2Alloc >0) ? max(bw2Alloc, var.lints[0]) : var.lints[0];
		}	
		//step2.3. double check if the bandwidth is valid
		if (bw2Alloc <= 0)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "[%s] session[%s] 0 bandwidth has been specified, quit evaluation"), phoAllocName.c_str(), sessCtx.sessId.c_str());
			return NULL;
		}
		//step2.4. get streamLink info
		::std::string strEdgeRMEndpoint = "";
		modulationFormat	= linktattr._modulationFormat;
		strEdgeRMEndpoint	= linktattr._strEdgeRMEndpoint;
		QamZone = linktattr._strQAMZONE;

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO,"[%s] required BW[%lld]"),phoAllocName.c_str(),bw2Alloc);

		//step2.5. get EdgeResouceManager proxy
		::TianShanIce::EdgeResource::EdgeResouceManagerPrx edgeRMPrx = createEdgeRMPrx(strEdgeRMEndpoint.c_str());
		if(!edgeRMPrx)
		{
			return NULL;
		}

		//step2.6. create allocation object 
		///****create Edge Resource Manager Allocation
		///****   1.create 
		///****   2.addResource
		///****   3.provision
		::TianShanIce::SRM::ResourceMap resRequirement;
		///add resource to Allocation	
		::TianShanIce::SRM::Resource allocResource;
		allocResource.status = TianShanIce::SRM::rsRequested;
		allocResource.attr = TianShanIce::SRM::raMandatoryNonNegotiable;

		///add serviceGroup:id
		::TianShanIce::Variant var_servicegroup;
		var_servicegroup.type = ::TianShanIce::vtStrings;
		var_servicegroup.bRange = false;
		char temp[20] = "";
		snprintf(temp, sizeof(temp)-2, "%d", linkInfo.otherIntId);
		var_servicegroup.strs.push_back(temp);		    
		MAPSET(::TianShanIce::ValueMap, allocResource.resourceData, "routeName",  var_servicegroup);
		MAPSET(::TianShanIce::SRM::ResourceMap, resRequirement, TianShanIce::SRM::rtServiceGroup , allocResource);
		allocResource.resourceData.clear();

		if(QamZone.size() > 0)
		{ 
			///add rtPhysicalChannel:edgeDeviceZone
			::TianShanIce::Variant var_edgeDeviceGroup;
			var_edgeDeviceGroup.type = ::TianShanIce::vtStrings;
			var_edgeDeviceGroup.bRange = false;
			var_edgeDeviceGroup.strs.push_back(QamZone);
			MAPSET(::TianShanIce::ValueMap, allocResource.resourceData, "edgeDeviceZone",  var_edgeDeviceGroup);
		}
		if(linktattr._vecQAMID.size() > 0)
		{
			///add rtPhysicalChannel:edgeDeviceName
			::TianShanIce::Variant var_edgeDeviceName;
			var_edgeDeviceName.type = ::TianShanIce::vtStrings;
			var_edgeDeviceName.bRange = false;
			var_edgeDeviceName.strs.clear();
			{
				//std::random_shuffle(linktattr._vecQAMID.begin(),linktattr._vecQAMID.end(), MyRandom());
				for (size_t i = 0; i < linktattr._vecQAMID.size(); i++)
				{
					if(linktattr._vecQAMID[i].size() > 0)
						var_edgeDeviceName.strs.push_back(linktattr._vecQAMID[i]);
				}
			}	
			if(var_edgeDeviceName.strs.size() > 0)
				MAPSET(::TianShanIce::ValueMap, allocResource.resourceData, "edgeDeviceName",  var_edgeDeviceName);
		}

		if(allocResource.resourceData.size() > 0)
		{
			MAPSET(::TianShanIce::SRM::ResourceMap, resRequirement, TianShanIce::SRM::rtPhysicalChannel , allocResource);
		}

		allocResource.resourceData.clear();

		///rtTsDownstreamBandwidth:bandwidth
		::TianShanIce::Variant var_bandwidth;
		var_bandwidth.type = ::TianShanIce::vtLongs;
		var_bandwidth.bRange = false;
		var_bandwidth.lints.push_back(bw2Alloc);
		MAPSET(::TianShanIce::ValueMap, allocResource.resourceData, "bandwidth",  var_bandwidth);
		MAPSET(::TianShanIce::SRM::ResourceMap, resRequirement, TianShanIce::SRM::rtTsDownstreamBandwidth , allocResource);

		allocResource.resourceData.clear();

		///rtAtscModulationMode:modulationFormat
		::TianShanIce::Variant var_modulationFormat;
		var_modulationFormat.bRange = false;
		var_modulationFormat.type = ::TianShanIce::vtBin;
		var_modulationFormat.bin.push_back(modulationFormat);
		MAPSET(::TianShanIce::ValueMap, allocResource.resourceData, "modulationFormat",  var_modulationFormat);
		MAPSET(::TianShanIce::SRM::ResourceMap, resRequirement, TianShanIce::SRM::rtAtscModulationMode , allocResource);
		allocResource.resourceData.clear();

		Ice::Long lbegin  = ZQTianShan::now();
		allocPrx = edgeRMPrx->createAllocation(resRequirement, 60*1000*10, _env._allocOwnerPrx, sessCtx.sessId);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"[%s] create Allocation took %dms"), phoAllocName.c_str(), ZQTianShan::now() - lbegin);

		strAllocSessionId = allocPrx->getId();

		lbegin  = ZQTianShan::now();
		allocPrx->provision(_env._maxCandidates, true);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMPHO,"[%s] allocation[%s] provision took %dms"), phoAllocName.c_str(), strAllocSessionId.c_str(), ZQTianShan::now() - lbegin);
	}
	catch (::TianShanIce::BaseException &ex)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"[%s] create allocation caught exception(%s,%d,%s)"), 
			phoAllocName.c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		allocPrx =  NULL;
	}
	catch (const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"[%s] create allocation caught exception(%s)"), phoAllocName.c_str(), ex.ice_name().c_str());
	    allocPrx =  NULL;
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"[%s] create allocation caught unknown exception(%d)"), phoAllocName.c_str(), SYS::getLastErr());
		allocPrx =  NULL;
	}
	return allocPrx;
}

void EdgeRMPHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	Ice::Long lstart  = ZQTianShan::now();
	std::string strStreamlinkID="";
	std::string allocationSessKey = "", strAllocProxyString;		
	Ice::Identity& ticketID = ticket->ident;
	try
	{
		///get ticket Identity;
		TianShanIce::ValueMap& ticketPD = ticket->privateData;
        ///get streamlinkType
		TianShanIce::ValueMap::const_iterator itLinkType = ticketPD.find( PathTicketPD_Field(ownerStreamLinkType) ) ;
		if ( itLinkType == ticketPD.end()  || itLinkType->second.type != TianShanIce::vtStrings ||itLinkType->second.strs.size() == 0  ) 
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"doFreeResource() ticketId[%s] no ticket owner link type is found"), ticketID.name.c_str());
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];

        ///get streamlinkID
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkId->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"doFreeResource() ticketId[%s] can' get streamlink from ticket"), ticketID.name.c_str());
			return ;
		}
		strStreamlinkID = itLinkId->second.strs[0];

		TianShanIce::ValueMap::const_iterator itorPD = ticketPD.find(PathTicketPD_Field(OnDemandSessionId));
		if(itorPD == ticketPD.end() || itorPD->second.strs.size() < 1)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT (S6EdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] can't find OnDemandSessionId key" ), strStreamlinkID.c_str(), ticketID.name.c_str());
			return;
		}
		std::string  OnDemondSessionId= itorPD->second.strs[0];

		_env.removeOnDemandSession(OnDemondSessionId);

		if ( strcmp( STRMLINK_TYPE_TianShanERM , strLinkType.c_str() ) == 0 ) 
		{
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT (EdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] free resource" ), strStreamlinkID.c_str(), ticketID.name.c_str());

			//free allocation
			ZQTianShan::Util::getValueMapData(ticketPD, "AllocationSessionId", allocationSessKey);
			ZQTianShan::Util::getValueMapData(ticketPD, "AllocationProxyString", strAllocProxyString);

			try
			{
				::TianShanIce::EdgeResource::AllocationPrx allocPrx = NULL;
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMPHO,"doFreeResources()free alloction[%s]"), allocationSessKey.c_str());

				allocPrx = TianShanIce::EdgeResource::AllocationPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(strAllocProxyString));
				allocPrx->destroy();
			}
			catch(::TianShanIce::BaseException& ex)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMPHO, "ticketId[%s] destroy allocation[%s] caught exception(%s,%d,%s)"),
					ticketID.name.c_str(), allocationSessKey.c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
			}
			catch (Ice::Exception& ex)
			{	
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"ticketId[%s]destroy allocation[%s] caught exception (%s)"), 
					ticketID.name.c_str(), allocationSessKey.c_str(), ex.ice_name().c_str());
			}
			catch (...)
			{	
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"ticketId[%s]destroy allocation[%s] caught unknown exception (%d)"), 
					ticketID.name.c_str(), allocationSessKey.c_str(), SYS::getLastErr());
			}
			////////////////////////////////////////////////////////////
		}
		else
		{
			envlog(ZQ::common::Log::L_WARNING , CLOGFMT(EdgeRMPHO,"unrecognized stream link type [%s]"), strLinkType.c_str() );
		}
	}
	catch( const Ice::Exception& e)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"doFreeResource() ticketId[%s] caught exception(%s)"),ticketID.name.c_str(), e.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeRMPHO,"doFreeResource() ticketId[%s] caught unknown exception(%d)"), ticketID.name.c_str(), SYS::getLastErr());
	}	

	envlog(ZQ::common::Log::L_INFO, CLOGFMT (EdgeRMPHO , "doFreeResource() ticketId[%s] leave free resource alloction[%s]took %d ms" ), ticketID.name.c_str(), allocationSessKey.c_str(), ZQTianShan::now() - lstart);
}
void EdgeRMPHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{	
	try
	{
      TianShanIce::ValueMap& ticketPD = ticket->privateData;  

	  ///get streamlinkID
	  TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
	  if ( itLinkId ==ticketPD.end() || itLinkId->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
	  {
		  envlog(ZQ::common::Log::L_WARNING,CLOGFMT(EdgeRMPHO,"doCommit() failed to get streamlink from ticket"));
		  return ;
	  }
	  std::string strStreamlinkID = itLinkId->second.strs[0];
	  std::string sessionId = sessCtx.sessId;

	  envlog(ZQ::common::Log::L_INFO, CLOGFMT (EdgeRMPHO , "doCommit() [%s#%s]" ), strStreamlinkID.c_str(), sessionId.c_str());
	}
	catch (...){}
}

::TianShanIce::EdgeResource::EdgeResouceManagerPrx EdgeRMPHO::createEdgeRMPrx(const char* endpoint)
{	
	::TianShanIce::EdgeResource::EdgeResouceManagerPrx edgeRMPrx = NULL;
	try 
	{
		edgeRMPrx = TianShanIce::EdgeResource::EdgeResouceManagerPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(::std::string(endpoint)));
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "failed to get Edge Resource Manager proxy at endpoint '%s' caught exception(%s)"), endpoint,  ex.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO, "failed to get Edge Resource Manager proxy at endpoint '%s' caught unknown exception(%d)"), endpoint, SYS::getLastErr());
	}
	return edgeRMPrx;
}

}}//namespace ZQTianShan::AccreditedPath

