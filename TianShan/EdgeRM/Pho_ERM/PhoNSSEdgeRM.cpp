 #include "PhoNSSEdgeRM.h"
#include "Log.h"
#include "strHelper.h"
#include "TianShanIceHelper.h"
#include "public.h"
#include <time.h>
#include <algorithm>
namespace ZQTianShan {
namespace AccreditedPath {

static ConfItem NSSEdge_IP[] = {
	{ "EdgeRMEndpoint",			::TianShanIce::vtStrings,	false,	"EdgeRM:tcp -h host -p port",	false},
	{ "Qam.ModulationFormat",	::TianShanIce::vtInts,		false,	"8",		false },
	{ "QAM-ZONE",			    ::TianShanIce::vtStrings,	false,	"",	        false},//device of zone, if the zone is not null, the QAM-IDs must be set NULL
	{ "QAM-IDs",				::TianShanIce::vtStrings,	false,	"",			false }, //use ; as a splitter
	{ "SopName",				::TianShanIce::vtStrings,	false,	"",			false }, 
	{ NULL,						::TianShanIce::vtInts,		true,	"",			false },
	};


void NSSEdgeRMPHO::dumpLine(const char* line, void* pCtx)
{
	envlog(ZQ::common::Log::L_DEBUG, line);
}
NSSEdgeRMPHO::NSSEdgeRMPHO(IPHOManager& mgr, ZQTianShan::EdgeRM::PhoEdgeRMEnv& env, ::ZQTianShan::AccreditedPath::EdgeRMPHO& edgermpho)
: IStreamLinkPHO(mgr), _env(env), _edgermpho(edgermpho)
{
	_phoManager=&mgr;
}

NSSEdgeRMPHO::~NSSEdgeRMPHO()
{
	_helperMgr.unregisterStreamLinkHelper( STRMLINK_TYPE_TianShanNSSERM );
}

bool NSSEdgeRMPHO::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;

	// address the schema definition
	if ( 0 == strcmp(type, STRMLINK_TYPE_TianShanNSSERM) )
		config = NSSEdge_IP;

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
void NSSEdgeRMPHO::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
	{
		type = "NULL";
	}
	if (0 == strcmp( STRMLINK_TYPE_TianShanNSSERM, type))
	{
		validate_NSSEdgeRMStrmLinkConfig( identStr, configPD );
	}
	else
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("ERMEdgePHO",1001,"unsupported type [%s] in NSSEdgeRMPHO", type);
	}
}
//no problem
void NSSEdgeRMPHO::validate_NSSEdgeRMStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSEdgeRMPHO, " enter validate_EdgeRMStrmLinkConfig() %s Configuration: link[%s]"), STRMLINK_TYPE_TianShanNSSERM, identStr);
	EdgeRMLinkAttr lia;
	lia._streamLinkID	= identStr;

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
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSSEdgeRMPHO,"StreamLink[%s] get EdgeRM service Endpoint[%s]"), identStr, edgeRMEndpoint.c_str());
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get EdgeRMEndpoint caught (%s:%d:%s)", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get EdgeRM service Endpoint parameter", identStr);
		}
		//get QAM modulationFormat
		::Ice::Int modulationFormat = 0;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "Qam.ModulationFormat", modulationFormat);
			lia._modulationFormat = modulationFormat;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSSEdgeRMPHO,"StreamLink[%s] get ModulationFormat[%d]"), identStr, modulationFormat);
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get caught exception(%s:%d:%s)", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{ 
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get ModulationFormat parameter", identStr);
		}

		//get QAM-ZONE
		::std::string edgeRMZONE;
		try
		{
			::ZQTianShan::Util::getValueMapDataWithDefault(configPD, "QAM-ZONE","", edgeRMZONE);
			lia._strQAMZONE = edgeRMZONE;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSSEdgeRMPHO,"StreamLink[%s] get QAM-ZONE[%s]"), identStr, edgeRMZONE.c_str());
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get QAM-ZONE caught exception(%s:%d:%s)", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get QAM-ZONE parameter", identStr);
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
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSSEdgeRMPHO,"StreamLink[%s] get QAM-IDs[%s]"), identStr, strQAMIDList.c_str());
			}
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get QAM-IDs caught exception(%s:%d:%s)", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get QAM-IDs parameter", identStr);
		}

		//get SopName
		::std::string sopname;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "SopName", sopname);
			lia._strSopName = sopname;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSSEdgeRMPHO,"StreamLink[%s] get SopName[%s]"), identStr, sopname.c_str());
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get SopName caught exception(%s:%d:%s)", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"StreamLink[%s] failed to get SopName parameter", identStr);
		}

		TianShanIce::Transport::StreamLinkPrx strmLinkPrx = NULL;
		try
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSSEdgeRMPHO,"open StreamLink[%s]"), identStr);
			strmLinkPrx =_phoManager->openStreamLink(identStr);  
		}
		catch (Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSSEdgeRMPHO",1011,"failed to open StreamLink[%s]", identStr);
		}

		if(strmLinkPrx)
		{
			IdentCollection idc;
			try
			{
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSSEdgeRMPHO,"list PathTickets by StreamLink[%s]"), identStr);
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
						envlog(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEdgeRMPHO,"validate ticket [%s] caught exception [%s]"),itID->name.c_str(), e.message.c_str());
					}
					catch (...)
					{
					}
				}
			}
		}
		
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
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSSEdgeRMPHO,"validate_NSSEdgeRMStrmLinkConfig() update SeaChange.NSS.EdgeRM streamlink with streamlink id[%s]"), identStr);
	}	
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	catch (const ::Ice::Exception& e)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEdgeRMPHO,"validate_EdgeRMStrmLinkConfig() catch ice exception :%s"),e.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEdgeRMPHO,"validate_EdgeRMStrmLinkConfig() caught unknown exception(%d)"), SYS::getLastErr());
	}
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSEdgeRMPHO, "Leave validate_EdgeRMStrmLinkConfig() %s Configuration: link[%s]"), STRMLINK_TYPE_TianShanNSSERM, identStr);
}

#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
	if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else envlog(ZQ::common::Log::L_WARNING, CLOGFMT(NSSEdgeRMPHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}
//no problem
Ice::Int NSSEdgeRMPHO::doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& ticketPrivateData, const ::Ice::Int oldCost)
{
//	std::string &linktype =linkInfo.linkType;
	EdgeRMLinkAttr linktattr;
	{
		ZQ::common::MutexGuard gd(_linkAttrMapMutex);
		EdgeRMLinkAttrMap::iterator iter = _linkAttrMap.find( linkInfo.linkIden.name );
		if(iter==_linkAttrMap.end())
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSEdgeRMPHO,"doEvaluation() session[%s] can't find the streamlink attr through the id [%s]"),
				sessCtx.sessId.c_str(), linkInfo.linkIden.name.c_str() );
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		linktattr	= iter->second;

		::TianShanIce::Variant var_SopName;
		///set QAM mac
		var_SopName.type = ::TianShanIce::vtStrings;
		var_SopName.bRange = false;
		var_SopName.strs.clear();
		var_SopName.strs.push_back(linktattr._strSopName);
		ticketPrivateData[PathTicketPD_Field(sop_name)] = var_SopName;
	}

	return _edgermpho.eval_EdgeRMStrmLink(linkInfo, sessCtx, ticketPrivateData, linkInfo.rcMap,oldCost, linktattr);
}
///
IPathHelperObject::NarrowResult NSSEdgeRMPHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, 
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

		ZQ::common::MutexGuard gd(_linkAttrMapMutex);
		EdgeRMLinkAttrMap::iterator iter = _linkAttrMap.find(ticket->streamLinkIden.name);
		if(iter==_linkAttrMap.end())
		{
			//envlog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSEdgeRMPHO,"doEvaluation() can't find the streamlink attr through the id [%s]" sessCtx.sessId.c_str(), ticket->streamLinkIden.name);
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSEdgeRMPHO,"doEvaluation() can't find the streamlink attr through the id [%s]"), sessCtx.sessId.c_str());
			return IPathHelperObject::NR_Unrecognized;
		}
		EdgeRMLinkAttr linktattr = iter->second;

		::TianShanIce::Variant var_SopName;
		///set QAM mac
		var_SopName.type = ::TianShanIce::vtStrings;
		var_SopName.bRange = false;
		var_SopName.strs.clear();
		var_SopName.strs.push_back(linktattr._strSopName);
		ticket->privateData[PathTicketPD_Field(sop_name)] = var_SopName;
	}
	catch(...) 
	{
		strmLink = NULL;
		type = "";
	}
	if ( 0 == type.compare( STRMLINK_TYPE_TianShanNSSERM ) )
	{
		return _edgermpho.narrow_EdgeRMStrmLink(strmLink , sessCtx , ticket);
	}

	envlog( ZQ::common::Log::L_ERROR, CLOGFMT(NSSEdgeRMPHO, "doNarrow() unrecognized stream link type [%s]"), type.c_str() );
	return IPathHelperObject::NR_Unrecognized;

}

void NSSEdgeRMPHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
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
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEdgeRMPHO,"doFreeResource() ticketId[%s] no ticket owner link type is found"), ticketID.name.c_str());
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];

		///get streamlinkID
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkId->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEdgeRMPHO,"doFreeResource() ticketId[%s] can' get streamlink from ticket"), ticketID.name.c_str());
			return ;
		}
		strStreamlinkID = itLinkId->second.strs[0];

		TianShanIce::ValueMap::const_iterator itorPD = ticketPD.find(PathTicketPD_Field(OnDemandSessionId));
		if(itorPD == ticketPD.end() || itorPD->second.strs.size() < 1)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT (NSSEdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] can't find OnDemandSessionId key" ), strStreamlinkID.c_str(), ticketID.name.c_str());
			return;
		}
		std::string  OnDemondSessionId= itorPD->second.strs[0];

		_env.removeOnDemandSession(OnDemondSessionId);

		if ( strcmp( STRMLINK_TYPE_TianShanERM , strLinkType.c_str() ) == 0 ) 
		{
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT (NSSEdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] free resource" ), strStreamlinkID.c_str(), ticketID.name.c_str());

			//free allocation
			ZQTianShan::Util::getValueMapData(ticketPD, "AllocationSessionId", allocationSessKey);
			ZQTianShan::Util::getValueMapData(ticketPD, "AllocationProxyString", strAllocProxyString);

			try
			{
				::TianShanIce::EdgeResource::AllocationPrx allocPrx = NULL;
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSEdgeRMPHO,"doFreeResources()free alloction[%s]"), allocationSessKey.c_str());

				allocPrx = TianShanIce::EdgeResource::AllocationPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(strAllocProxyString));
				allocPrx->destroy();
			}
			catch(::TianShanIce::BaseException& ex)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(NSSEdgeRMPHO, "ticketId[%s] destroy allocation[%s] caught exception(%s,%d,%s)"),
					ticketID.name.c_str(), allocationSessKey.c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
			}
			catch (Ice::Exception& ex)
			{	
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSEdgeRMPHO,"ticketId[%s]destroy allocation[%s] caught exception (%s)"), 
					ticketID.name.c_str(), allocationSessKey.c_str(), ex.ice_name().c_str());
			}
			catch (...)
			{	
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSEdgeRMPHO,"ticketId[%s]destroy allocation[%s] caught unknown exception (%d)"), 
					ticketID.name.c_str(), allocationSessKey.c_str(), SYS::getLastErr());
			}
			////////////////////////////////////////////////////////////
		}
		else
		{
			envlog(ZQ::common::Log::L_WARNING , CLOGFMT(NSSEdgeRMPHO,"unrecognized stream link type [%s]"), strLinkType.c_str() );
		}
	}
	catch( const Ice::Exception& e)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEdgeRMPHO,"doFreeResource() ticketId[%s] caught exception(%s)"),ticketID.name.c_str(), e.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEdgeRMPHO,"doFreeResource() ticketId[%s] caught unknown exception(%d)"), ticketID.name.c_str(), SYS::getLastErr());
	}	

	envlog(ZQ::common::Log::L_INFO, CLOGFMT (NSSEdgeRMPHO , "doFreeResource() ticketId[%s] leave free resource alloction[%s]took %d ms" ), ticketID.name.c_str(), allocationSessKey.c_str(), ZQTianShan::now() - lstart);
}
void NSSEdgeRMPHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{	
	try
	{
      TianShanIce::ValueMap& ticketPD = ticket->privateData;  

	  ///get streamlinkID
	  TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
	  if ( itLinkId ==ticketPD.end() || itLinkId->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
	  {
		  envlog(ZQ::common::Log::L_WARNING,CLOGFMT(NSSEdgeRMPHO,"doCommit() can' get streamlink from ticket"));
		  return ;
	  }
	  std::string strStreamlinkID = itLinkId->second.strs[0];
	  std::string sessionId = sessCtx.sessId;

	  envlog(ZQ::common::Log::L_INFO, CLOGFMT (NSSEdgeRMPHO , "doCommit() streamLink[%s] sessionId[%s]" ), strStreamlinkID.c_str(), sessionId.c_str());
	}
	catch (...){}
}

}}//namespace ZQTianShan::AccreditedPath

