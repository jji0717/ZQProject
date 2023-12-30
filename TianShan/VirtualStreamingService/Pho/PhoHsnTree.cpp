
#include <ZQ_common_conf.h>

#include "public.h"
#include "PhoHsnTree.h"
#include "Log.h"
#include <time.h>
#include <algorithm>
#include "TianShanIceHelper.h"
#include "Configuration.h"
#include <assert.h>

extern ZQ::common::Config::Loader<PhoVssConf>		phoConfig;

namespace ZQTianShan {
namespace AccreditedPath {

static ConfItem IpEdge_HsnTree[] = {
	//{ HsnTreeMaxStreamCount,		::TianShanIce::vtInts,		false,	"80",	false },
	{ HsnTreeQammodulationFormat,	::TianShanIce::vtInts,		false,	"1",	false },
	{ HsnTreeQamIP,					::TianShanIce::vtStrings,	false,	"",		false },
	{ HsnTreeQamport,				::TianShanIce::vtInts,		false,	"1",	false },
	{ HsnTreeQamMac,				::TianShanIce::vtStrings,	false,	"",		false },
	{ HsnTreeQamsymbolRate,			::TianShanIce::vtInts,		false,	"1",	false },
	{ HsnTreeQamfrequency,			::TianShanIce::vtInts,		false,	"1",	false },
	{ HsnTreePN,					::TianShanIce::vtInts,		false,	"1",	false },
	{ NULL,							::TianShanIce::vtInts,		true,	"",		false },
	};

PHOHsnTree::PHOHsnTree(IPHOManager& mgr)
: IStreamLinkPHO(mgr)
//,_usePN(0)
{
	_phoManager=&mgr;	
}

PHOHsnTree::~PHOHsnTree()
{
	_helperMgr.unregisterStreamLinkHelper( STRMLINK_TYPE_HsnTree );	
}

bool PHOHsnTree::init( )
{
	if( !phoConfig.phoReplicaSubscriberEndpoint.empty() )
	{
		glog( ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree, "start replica subscriber") );
		mStremerReplicaManager = new ReplicaSubscriberI( *this );
		assert( mStremerReplicaManager);
		
		//add replica subscriber into object adapter

		try
		{
			int i = 0 ;
			mIc = Ice::initialize( i, NULL );
			assert( mIc);
			mAdapter = mIc->createObjectAdapterWithEndpoints( "phoVssSubscriber",phoConfig.phoReplicaSubscriberEndpoint);
			mAdapter->add( mStremerReplicaManager, mIc->stringToIdentity("ReplicaSubscriber"));
			mAdapter->activate( );
		}
		catch( const Ice::Exception& ex )
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"init() failed to start replica subscriber due to [%s] when create object adapter"),ex.ice_name().c_str() );
			return false;
		}

		mStremerReplicaManager->start();		
	}
	return true;
}

void PHOHsnTree::uninit( )
{
	if( mStremerReplicaManager )
	{
		mStremerReplicaManager->stop();
		mStremerReplicaManager = NULL;
		try
		{
			mAdapter->deactivate( );
			mIc->destroy();
		}
		catch(...){}
	}
}

bool PHOHsnTree::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;

	// address the schema definition
	if ( 0 == strcmp(type, STRMLINK_TYPE_HsnTree) )
		config = IpEdge_HsnTree;

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

void PHOHsnTree::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
	{
		type = "NULL";
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("PHOHsnTree",1001,"unsupported type [%s] in PHOHsnTree", type);
	}
	if (0 == strcmp( STRMLINK_TYPE_HsnTree, type))
	{
		validate_IPStrmLinkConfig( identStr, configPD );
	}
	configPD.erase( _SERVICE_STATE_KEY_ );
}

void PHOHsnTree::validate_IPStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	glog(ZQ::common::Log::L_DEBUG,
		CLOGFMT(PHOHsnTree, " enter validate a IP StreamLink's Configuration: link[%s]"), 
		identStr );
	LinkIPAttr lia;
	lia._streamLinkID	= identStr;

	TianShanIce::Variant var;
	var.bRange			= false;
	try
	{
		//get Qam.modulationFormat
		var = PDField(configPD, HsnTreeQammodulationFormat);
		if( var.type!=::TianShanIce::vtInts )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1011,"Invalid %s type,should be vtInts", HsnTreeQammodulationFormat);
		}
		if ( var.ints.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1012,"Invalid %s ,strs size() == 0", HsnTreeQammodulationFormat);
		}
		lia._QammodulationFormat	= var.ints[0];
		glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"stream IP link[%s] get %s[%d]"), identStr, HsnTreeQammodulationFormat, lia._QammodulationFormat);

		//get Qam.IP
		var = PDField(configPD, HsnTreeQamIP);
		if( var.type!=::TianShanIce::vtStrings )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1011,"Invalid %s type,should be vtStrings", HsnTreeQamIP);
		}
		if ( var.strs.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1012,"Invalid %s ,strs size() == 0", HsnTreeQamIP);
		}
		lia._QamIP	= var.strs[0];
		glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"stream IP link[%s] get %s[%s]"), identStr, HsnTreeQamIP, lia._QamIP.c_str());

		//get Qam.port
		var = PDField(configPD, HsnTreeQamport);
		if( var.type!=::TianShanIce::vtInts )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1011,"Invalid %s type,should be vtInts", HsnTreeQamport);
		}
		if ( var.ints.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1012,"Invalid %s ,strs size() == 0", HsnTreeQamport);
		}
		lia._Qamport	= var.ints[0];
		glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"stream IP link[%s] get %s[%d]"), identStr, HsnTreeQamport, lia._Qamport);

		//get Qam.mac
		var = PDField(configPD, HsnTreeQamMac);
		if( var.type!=::TianShanIce::vtStrings )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1011,"Invalid %s type,should be vtStrings", HsnTreeQamMac);
		}
		if ( var.strs.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1012,"Invalid %s ,strs size() == 0", HsnTreeQamMac);
		}
		lia._QamMac	= var.strs[0];
		glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"stream IP link[%s] get %s[%s]"), identStr, HsnTreeQamMac, lia._QamMac.c_str());

		//get Qam.symbolRate
		var = PDField(configPD, HsnTreeQamsymbolRate);
		if( var.type!=::TianShanIce::vtInts )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1011,"Invalid %s type,should be vtInts", HsnTreeQamsymbolRate);
		}
		if ( var.ints.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1012,"Invalid %s ,strs size() == 0", HsnTreeQamsymbolRate);
		}
		lia._QamsymbolRate	= var.ints[0];
		glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"stream IP link[%s] get %s[%d]"), identStr, HsnTreeQamsymbolRate, lia._QamsymbolRate);

		//get Qam.frequency
		var = PDField(configPD, HsnTreeQamfrequency);
		if( var.type!=::TianShanIce::vtInts )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1011,"Invalid %s type,should be vtInts", HsnTreeQamfrequency);
		}
		if ( var.ints.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1012,"Invalid %s ,strs size() == 0", HsnTreeQamfrequency);
		}
		lia._Qamfrequency	= var.ints[0];
		glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"stream IP link[%s] get %s[%d]"), identStr, HsnTreeQamfrequency, lia._Qamfrequency);

		//get PN
		var = PDField(configPD, HsnTreePN);
		if( var.type!=::TianShanIce::vtInts )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1011,"Invalid %s type,should be vtInts", HsnTreePN);
		}
		if ( var.ints.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1012,"Invalid %s ,strs size() == 0", HsnTreePN);
		}
		lia._PN	= var.ints[0];
	/*	if (lia._PN != 1)
		{
			lia._PN = 1;
			glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"stream IP link[%s] reset %s[%d]"), identStr, HsnTreePN, lia._PN);
		}
		else
			*/glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"stream IP link[%s] get %s[%d]"), identStr, HsnTreePN, lia._PN);

		lia._usedPN = 0;

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
								glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PHOHsnTree,"validate_IPStrmLinkConfig ticket %s but no bandwidth resource"), itID->name.c_str());
							}
							else
							{	
								//TianShanIce::ValueMap val=it->second.resourceData;
								bandwidth = PDField( PDData , PathTicketPD_Field(bandwidth) );
								if (bandwidth.lints.size()==0) 
								{
									glog(ZQ::common::Log::L_WARNING, CLOGFMT(PHOHsnTree,"validate_IPStrmLinkConfig() ticket %s found a invalid bandwidth,maybe db error"), itID->name.c_str());
									continue;
								}
								glog(ZQ::common::Log::L_DEBUG,CLOGFMT(PHOHsnTree,"validate_IPStrmLinkConfig() ticket %s used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);
								//lia._usedStreamCount++;	
								lia._usedPN++;

								ResourceIPData rid;
								rid._usedPN = 0;
								{
									_ipResourceDataMap[ itID->name ] = rid;
								}

							}
						}
						catch(::TianShanIce::InvalidParameter& e)
						{
							glog(ZQ::common::Log::L_ERROR, CLOGFMT(PHOHsnTree,"validate_IPStrmLinkConfig() invalidParameter exception is caught:%s"), e.message.c_str());
						}
						catch (...)
						{
						}
					}
				}//for
			}
			if ( itIpAttr != _StreamLinkIPAttrmap.end() )
			{				
				itIpAttr->second = lia;
			}
			else
			{
				_StreamLinkIPAttrmap[ identStr ] = lia;
			}

		}
		else
		{
			LinkIPAttrMap::iterator itIpAttr = _StreamLinkIPAttrmap.find( identStr );
			if ( itIpAttr != _StreamLinkIPAttrmap.end() )
			{
				lia._usedPN = itIpAttr->second._usedPN;
				itIpAttr->second = lia;
			}
			else
			{
				_StreamLinkIPAttrmap[ identStr ] = lia;
			}
		}
		//lia._usedPN = 0;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"validate_IPStrmLinkConfig() update  ip streamlink with streamlink id [%s] PN[%d]"), identStr, lia._PN);
	}	
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	catch (const ::Ice::Exception& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"validate_IPStrmLinkConfig() catch a ice exception :%s"),e.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"validate_IPStrmLinkConfig() unexpect error"));
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PHOHsnTree, "leave validate a IP StreamLink's Configuration: link[%s]"), identStr);
}

#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
	if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else glog(ZQ::common::Log::L_WARNING, CLOGFMT(PHOHsnTree, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}

Ice::Int PHOHsnTree::doEvaluation(LinkInfo& linkInfo, 
								 const SessCtx& sessCtx,
								 TianShanIce::ValueMap& hintPD,
								 const ::Ice::Int oldCost)
{
	std::string &linktype =linkInfo.linkType;
	if (0 == linktype.compare(STRMLINK_TYPE_HsnTree))
	{
		return eval_IPStrmLink(linkInfo, sessCtx, hintPD, linkInfo.rcMap,oldCost);
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR , CLOGFMT(PHOHsnTree , "unrecognized streamlink type[%s]" ),linktype.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
}
Ice::Int PHOHsnTree::eval_IPStrmLink(LinkInfo& linkInfo,
									const SessCtx& sessCtx, 
									TianShanIce::ValueMap& hintPD,
									TianShanIce::SRM::ResourceMap& rcMap ,
									const ::Ice::Int oldCost)
{
	Ice::Int	newCost = oldCost;
	const std::string& sessId = sessCtx.sessId;
	if (!linkInfo.linkPrx )
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(PHOHsnTree,"eval_IPStreamLink() Session [%s] no Streamlink is attached,return with OutOfServiceCost"), sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	glog(ZQ::common::Log::L_DEBUG , CLOGFMT(PHOHsnTree , "eval_IPStreamLink() session[%s] enter evaluation with oldCost[%d]" ), sessId.c_str(), oldCost );
	if ( newCost > ::TianShanIce::Transport::MaxCost )
	{	
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(PHOHsnTree, "eval_IPStreamLink() Session[%s] return oldCost[%d] because oldCost>=MaxCost"), sessId.c_str(),newCost);
		return newCost;
	}
	
	{
		ZQ::common::MutexGuard gd(_streamerMapLock);

		if ( mStremerReplicaManager && !mStremerReplicaManager->isStreamerAvailable( linkInfo.streamerId ) )
		{
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(PHOHsnTree,"streamer [%s] is not available now, return with out-of-service cost"),linkInfo.streamerId.c_str() );
			return TianShanIce::Transport::OutOfServiceCost;
		}
	}

	// get resource information from session
	::TianShanIce::SRM::ResourceMap resourceMap =sessCtx.resources;	//sess->getReources();
	::Ice::Long bw2Alloc = 0;
	TianShanIce::ValueMap linkPD;
	try
	{
		linkPD = ::TianShanIce::Transport::StreamLinkPrx::checkedCast(linkInfo.linkPrx)->getPrivateData();		
	}
	catch (Ice::Exception& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"Session[%s] ice exception is caught:%s"),sessId.c_str(),e.ice_name().c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"Session[%s] unexpect error is threw out when call streamlink's getPrivateData"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	
	// try to get the bandwidth requirement from the session context
	READ_RES_FIELD(bw2Alloc, resourceMap, rtTsDownstreamBandwidth, bandwidth, lints);

	if (bw2Alloc <= 0)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(PHOHsnTree, "eval_IPStreamLink() Session[%s] 0 bandwidth has been specified, quit evaluation"), sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	Ice::Int	PN				= 0;
	Ice::Int	UsedPN			= 0;

	{
		ZQ::common::MutexGuard gd(_ipResourceLocker);
		LinkIPAttrMap::iterator it = _StreamLinkIPAttrmap.find( linkInfo.linkIden.name );
		if(it==_StreamLinkIPAttrmap.end())
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(PHOHsnTree,"Session[%s] Can't find the streamlink attr through the id [%s]"), sessId.c_str(), linkInfo.linkIden.name.c_str() );
			return ::TianShanIce::Transport::OutOfServiceCost;
		}	
		//_usePN += it->second._PN;
		//it->second._usedPN++;
		PN = it->second._PN;
		UsedPN = it->second._usedPN;	
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PHOHsnTree,"Session[%s] current PN[%d]"), sessId.c_str() , PN);

	if ( ::TianShanIce::Transport::MaxCost > newCost)
	{
		if(UsedPN >= 1)
			newCost = ::TianShanIce::Transport::OutOfServiceCost;
		else
			newCost = (::Ice::Int) ( UsedPN * ::TianShanIce::Transport::MaxCost / 1 );
	}
	
	glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"session[%s] PN cost[%d] with usedPN[%d] totalPN[%d]"), sessId.c_str(), newCost, UsedPN, PN);

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
				CLOGFMT(IpEdgePHO,"eval_IPStrmLink() Session[%s] set bandwidth to %lld"),
				sessId.c_str(),bw2Alloc);
			hintPD[PathTicketPD_Field(bandwidth)] = value;


			res.resourceData["bandwidth"]=value;
			res.status=TianShanIce::SRM::rsAssigned;
			rcMap[TianShanIce::SRM::rtTsDownstreamBandwidth]=res;

			hintPD[PathTicketPD_Field(Qam.mode)] = linkPD["Qam.modulationFormat"];
			hintPD[PathTicketPD_Field(Qam.IP)] = linkPD["Qam.IP"];
			hintPD[PathTicketPD_Field(Qam.basePort)] = linkPD["Qam.port"];
			hintPD[PathTicketPD_Field(Qam.symbolRate)] = linkPD["Qam.symbolRate"];
			hintPD[PathTicketPD_Field(Qam.frequency)] = linkPD["Qam.frequency"];

			//fill rtEthernetInterface
			PutResourceMapData(rcMap,TianShanIce::SRM::rtEthernetInterface,"destIP",linkPD["Qam.IP"],sessId);
			//port 需要等到narrow的时候才能确定
			PutResourceMapData(rcMap,TianShanIce::SRM::rtEthernetInterface,"destMac",linkPD["Qam.Mac"],sessId);

			///fill rtTsDownstreamBandwidth	
			PutResourceMapData(rcMap,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth",value,sessId);


			//fill rtPhysicalChannel
			PutResourceMapData(rcMap,TianShanIce::SRM::rtPhysicalChannel,"channelId",linkPD["Qam.frequency"],sessId);

			//fill rtAtscModulationMode
			PutResourceMapData(rcMap,TianShanIce::SRM::rtAtscModulationMode,"symbolRate",linkPD["Qam.symbolRate"],sessId);

			PutResourceMapData(rcMap,TianShanIce::SRM::rtAtscModulationMode,"modulationFormat",linkPD["Qam.modulationFormat"],sessId);

		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(IpEdgePHO, "eval_IPStreamLink() Session[%s] exception caught while filling in the ticket private data"), sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PHOHsnTree,"eval_IPStreamLink() Session[%s] return with newCost=%d"), sessId.c_str(),max(oldCost,newCost));
	// step 4. return the higher as the cost
	return max(oldCost, newCost);
}

IPathHelperObject::NarrowResult PHOHsnTree::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, 
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
	if ( 0 == type.compare( STRMLINK_TYPE_HsnTree ) )
	{
		return narrow_IPStrmLink( strmLink , sessCtx , ticket );
	}
	else
	{
		glog( ZQ::common::Log::L_ERROR,
			CLOGFMT(PHOHsnTree , "doNarrow() unrecognized stream link type [%s]" ),
			type.c_str() );
		return IPathHelperObject::NR_Unrecognized;
	}
}

IPathHelperObject::NarrowResult PHOHsnTree::narrow_IPStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink,
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
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(PHOHsnTree,"narrow_IPStrmLink() caught[%s] when get streamlink's identity"), ex.ice_name().c_str());
		return NR_Error;
	}
	catch (...) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"narrow_IPStrmLink() get streamlink's ID failed"));
		return NR_Error;
	}
	std::string sessId = sessCtx.sessId;

	std::string	ticketID = ticket->ident.name;

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PHOHsnTree,"Session[%s] narrowed ticket with ticketid[%s]"), sessId.c_str(),ticketID.c_str());

	ResourceIPData rid;
	
	TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
	Ice::Long	bw2Alloc		= 0;
	Ice::Long	bwUsed			= 0;
	Ice::Long	bwTotal			= 0;
	//Ice::Int	totalStrmCount	= 0;
	//Ice::Int	usedStrmCount	= 0;
	Ice::Int	PN				= 0;
//	Ice::Int	UsedPN			= 0;
	Ice::Int	PN2Alloc		= 1;
	Ice::Int	port			= 1;
	std::string strSopName		= "";	

	READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
	std::string&	strLinkIdent = strmLinkId ; //strmLink->getIdent().name;

	{
		ZQ::common::MutexGuard gd(_ipResourceLocker);
		LinkIPAttrMap::iterator itIPmap = _StreamLinkIPAttrmap.find(strLinkIdent);
		if ( itIPmap == _StreamLinkIPAttrmap.end())
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"narrow_IPStrmLink() Session[%s] No streamlink with ID[%s] is found"), sessId.c_str(),strLinkIdent.c_str());
			return NR_Error;
		}
		LinkIPAttr& ipAttr	= itIPmap->second;
		if ( ipAttr._usedPN + PN2Alloc > 1) 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(PHOHsnTree,"narrow_IPStrmLink() Session[%s] not enough PNfor strmLink[%s], usedPN[%d], totalPN[%d]"), sessId.c_str(), strLinkIdent.c_str(), ipAttr._usedPN, ipAttr._PN);
			return NR_Error;
		}

		PN = ipAttr._PN;
		port = ipAttr._Qamport;
		rid._usedPN		= PN2Alloc;
		ipAttr._usedPN	+= PN2Alloc;
	}

	TianShanIce::Variant valPN;
	TianShanIce::Variant valPort;
	TianShanIce::Variant valBandwidth;


	valPort.bRange=false;
	valPort.type = TianShanIce::vtInts;
	valPort.ints.clear();
	valPort.ints.push_back(port);
	valPort.type=TianShanIce::vtInts;
	ticket->privateData[PathTicketPD_Field(destPort)]=valPort;
	PutResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destPort",valPort,sessId);

	valPN.bRange=false;
	valPN.type = TianShanIce::vtInts;
	valPN.ints.clear();
	valPN.ints.push_back(PN);
	valPN.type=TianShanIce::vtInts;
	ticket->privateData[PathTicketPD_Field(PN)]=valPN;
	PutResourceMapData(resMap,TianShanIce::SRM::rtMpegProgram,"Id",valPN,sessId);

	TianShanIce::Variant varLinkType;
	TianShanIce::Variant varLinkId;
	TianShanIce::Variant varSopName;

	varLinkType.bRange = false;
	varLinkType.type = TianShanIce::vtStrings;
	varLinkType.strs.clear();
	varLinkType.strs.push_back( STRMLINK_TYPE_HsnTree );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkType)] = varLinkType;

	varLinkId.bRange = false;
	varLinkId.type = TianShanIce::vtStrings;
	varLinkId.strs.clear();
	varLinkId.strs.push_back( strLinkIdent );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varLinkId;

	glog(ZQ::common::Log::L_INFO,CLOGFMT(PHOHsnTree,"narrow_IPStrmLink() Session[%s] narrowed with NeedBW[%lld] and UsedBW[%lld] TotalBW[%lld]"), sessId.c_str() , bw2Alloc, bwUsed , bwTotal);
	{
		ZQ::common::MutexGuard gd(_ipResourceLocker);
		_ipResourceDataMap[ticketID]=rid;
	}
	return NR_Narrowed;
}

void PHOHsnTree::decreasePenalty( const std::string& streamerId )
{
	ZQ::common::MutexGuard gd( _streamerMapLock );
	streamerPenaltyAttrMap::iterator it = _streamerMap.find ( streamerId );
	if( it == _streamerMap.end() )
		return;
	if ( it->second.penaltyValue > 0 )
	{
		it->second.penaltyValue --;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(PHOHsnTree,"decrease streamer[%s]'s penalty to[%d]"),
			streamerId.c_str() , it->second.penaltyValue );
	}
}

void PHOHsnTree::increasePenalty( const std::string& streamerId , int maxPenaltyValue )
{
	ZQ::common::MutexGuard gd( _streamerMapLock );
	streamerPenaltyAttrMap::iterator it = _streamerMap.find ( streamerId );
	if ( it != _streamerMap.end ( ) ) 
	{
		it->second.penaltyValue = maxPenaltyValue;
	}
	else
	{
		streamerPenaltyAttr attr;
		attr.streamerId		= streamerId;
		attr.penaltyValue	= maxPenaltyValue;		

		_streamerMap.insert ( streamerPenaltyAttrMap::value_type( streamerId , attr ) );

	}
	glog(ZQ::common::Log::L_INFO ,
		CLOGFMT( PHOHsnTree , "set streamer[%s]'s penalty value to max penalty value[%d]" ),
		streamerId.c_str () ,  maxPenaltyValue );
}

void PHOHsnTree::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	try
	{
		::Ice::Int penaltyValue = 0;		

		TianShanIce::ValueMap& ticketPD = ticket->privateData;
		ZQTianShan::Util::getValueMapDataWithDefault( ticket->privateData , "StreamerPenalty_value", 0 , penaltyValue );
		if( penaltyValue > 0 )
		{
			try
			{
				TianShanIce::Transport::StreamLinkPrx streamLink = _phoManager->openStreamLink ( ticket->streamLinkIden );
				std::string streamerId = streamLink->getStreamerId();
				if(!streamerId.empty())
					increasePenalty( streamerId , penaltyValue );
			}
			catch(...){}
		}

		TianShanIce::ValueMap::const_iterator itLinkType = ticketPD.find( PathTicketPD_Field(ownerStreamLinkType) ) ;
		if ( itLinkType == ticketPD.end()  || itLinkType->second.type != TianShanIce::vtStrings ||itLinkType->second.strs.size() == 0  ) 
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"no ticket owner link type is found"));
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkType->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"doFreeResource() can' get streamlink from ticket"));
			return ;
		}
		std::string strStreamlinkID = itLinkId->second.strs[0];
		Ice::Identity& ticketID = ticket->ident;

		if ( strcmp( STRMLINK_TYPE_HsnTree , strLinkType.c_str() ) == 0 ) 
		{
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			ResourceIPDataMap::iterator itAlloc =_ipResourceDataMap.find( ticketID.name );
			if(itAlloc == _ipResourceDataMap.end())
			{
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(PHOHsnTree,"doFreeResource() streamlink [%s] ticket[%s] no allocated IP resource"),
					strStreamlinkID.c_str() , ticketID.name.c_str() );
				return;
			}
			LinkIPAttrMap::iterator it=_StreamLinkIPAttrmap.find( strStreamlinkID );
			if( it == _StreamLinkIPAttrmap.end() )
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(PHOHsnTree,"doFreeResource() streamlink [%s] ticket[%s] can't find strmlink attr "),
					strStreamlinkID.c_str(),ticketID.name.c_str());
				return;
			}
			LinkIPAttr& ipAttr		= it->second;
//			ResourceIPData& rid		= itAlloc->second;
			//_usePN -= ipAttr._PN;
			ipAttr._usedPN--;
			ipAttr._usedPN			= ipAttr._usedPN > 0 ? ipAttr._usedPN : 0;

			_ipResourceDataMap.erase(itAlloc);

			glog(ZQ::common::Log::L_INFO ,
				CLOGFMT (PHOHsnTree , "doFreeResource() streamLink[%s] free resource and now " ),
				strStreamlinkID.c_str());
		}
		else
		{
			glog(ZQ::common::Log::L_WARNING , CLOGFMT(PHOHsnTree,"unrecognized stream link type [%s]") , strLinkType.c_str() );
			return ; 
		}

	}
	catch( const TianShanIce::BaseException& ex )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"doFreeResource() catch a tianshan exception:%s"),ex.message.c_str());
		return;
	}
	catch( const Ice::Exception& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"doFreeResource() catch a ice exception:%s"),e.ice_name().c_str());
		return;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(PHOHsnTree,"doFreeResource() catch a unknown exception"));
		return;
	}	
}
void PHOHsnTree::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{

}

///ReplicaSubscriberI
ReplicaSubscriberI::ReplicaSubscriberI( PHOHsnTree& phoTree )
:mPhoTree(phoTree)
{
	mMaxUpdateInterval = phoConfig.replicaUpdaterInterval;
	mbQuit = false;
}
ReplicaSubscriberI::~ReplicaSubscriberI()
{
}
void ReplicaSubscriberI::stop( )
{
	mbQuit = true;
	mEventHandle.signal();
	waitHandle( 100 * 1000 );
}

int ReplicaSubscriberI::run( )
{
	glog(ZQ::common::Log::L_INFO,CLOGFMT(ReplicaSubscriberI,"running..."));
	int64 defaultInterval = mMaxUpdateInterval/2;
	int64 waitInterval = defaultInterval;
	while ( !mbQuit )
	{//check if any replica timed out
		{
			waitInterval = defaultInterval;
			ZQ::common::MutexGuard gd( mPhoTree._streamerMapLock );
			PHOHsnTree::streamerPenaltyAttrMap& penaltyMap = mPhoTree._streamerMap;
			int64 curTime = ZQTianShan::now();
			PHOHsnTree::streamerPenaltyAttrMap::iterator itAttr = penaltyMap.begin();
			for( ; itAttr != penaltyMap.end() ; itAttr ++ )
			{
				PHOHsnTree::streamerPenaltyAttr& attr = itAttr->second;
				if( (curTime - attr.lastUpdateTime ) > mMaxUpdateInterval )
				{
					if( attr.bStatus )
					{
						glog(ZQ::common::Log::L_INFO,CLOGFMT(ReplicaSubscriberI,"streamer[%s] replica information timed out, set it to [DOWN]"),
							itAttr->first.c_str() );
					}
					attr.bStatus = false;
				}
				else
				{
					waitInterval = attr.lastUpdateTime + mMaxUpdateInterval - curTime;
				}
			}
		}
		waitInterval = max( waitInterval , 10 );
		mEventHandle.wait(waitInterval);
	}
	return 0;
}

void ReplicaSubscriberI::updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& callback,
											 const ::TianShanIce::Replicas& reps, 
											 const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	TianShanIce::Replicas::const_iterator itRep = reps.begin();
	for( ; itRep != reps.end() ; itRep++ )
	{
		if( itRep->category.compare("streamer") != 0 )
		{
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(ReplicaSubscriberI,"got replica update information, but with unknown category[%s]"),itRep->category.c_str() );
			continue;
		}
		const TianShanIce::Replica rep = *itRep;
		std::string		streamerId = rep.groupId + "/" + rep.replicaId;
		{
			ZQ::common::MutexGuard gd( mPhoTree._streamerMapLock );
			PHOHsnTree::streamerPenaltyAttrMap& penaltyMap = mPhoTree._streamerMap;
			PHOHsnTree::streamerPenaltyAttrMap::iterator itAttr = penaltyMap.find(streamerId);
			if( itAttr != penaltyMap.end() )
			{
				bool bChanged = itAttr->second.bStatus != (itRep->replicaState == TianShanIce::stInService);
				
				if( bChanged )
				{
					itAttr->second.bStatus = (itRep->replicaState == TianShanIce::stInService);
					glog(ZQ::common::Log::L_INFO,CLOGFMT(ReplicaSubscriberI,"set streamer[%s] to [%s] due to replica information update"), 
						streamerId.c_str() , itAttr->second.bStatus ? "UP" : "DOWN" );
				}

				if( itRep->replicaState == TianShanIce::stInService )
				{
					itAttr->second.lastUpdateTime = ZQTianShan::now();
				}

				if( (itRep->replicaState == TianShanIce::stInService) && (itAttr->second.penaltyValue > 0) )
				{
					itAttr->second.penaltyValue --;
					glog(ZQ::common::Log::L_INFO,CLOGFMT(ReplicaSubscriberI,"decrease penalty to [%d] for streamer[%s]"), 
						itAttr->second.penaltyValue,
						itAttr->second.streamerId.c_str() );
				}				
			}
			else
			{
				PHOHsnTree::streamerPenaltyAttr attr;
				
				attr.streamerId			= streamerId;
				attr.bStatus			= itRep->replicaState == TianShanIce::stInService;
				attr.lastUpdateTime		= attr.bStatus ? ZQTianShan::now() : 0;
				attr.penaltyValue		= 0;
				penaltyMap.insert( PHOHsnTree::streamerPenaltyAttrMap::value_type( streamerId, attr ) );
				glog(ZQ::common::Log::L_INFO,CLOGFMT(ReplicaSubscriberI,"set streamer[%s] to [%s] due to replica information update"), 
					streamerId.c_str() , attr.bStatus ? "UP" : "DOWN" );
			}
		}
	}
	try
	{
		if( callback )
			callback->ice_response( mMaxUpdateInterval / 1000 );//response in seconds
	}
	catch(...)
	{

	}
}

void ReplicaSubscriberI::checkStreamer( const std::string& streamerId , const std::string& streamerEndpoint)
{
	Ice::CommunicatorPtr ic = mPhoTree.mIc;
	if( !ic ) {		return;  }

	if( isStreamerAvailable( streamerId ) )
		return;

	try
	{
		//check the streamer		
		TianShanIce::ReplicaQueryPrx queryPrx = TianShanIce::ReplicaQueryPrx::uncheckedCast( ic->stringToProxy( streamerEndpoint ) );
		TianShanIce::Replicas reps = queryPrx->queryReplicas( "Streamer" , "*" , true );
		updateReplica_async( NULL , reps ,Ice::Current() );
	}
	catch( const Ice::Exception& ex )
	{
		glog(ZQ::common::Log::L_WARNING,CLOGFMT(ReplicaSubscriberI,"checkStreamer() failed to get streamer replica information according to endpoint[%s] due to [%s]"),
			streamerEndpoint.c_str() , ex.ice_name().c_str() );
	}
}

bool ReplicaSubscriberI::isStreamerAvailable( const std::string& streamerId )
{
	ZQ::common::MutexGuard gd( mPhoTree._streamerMapLock );

	PHOHsnTree::streamerPenaltyAttrMap& penaltyMap = mPhoTree._streamerMap;
	PHOHsnTree::streamerPenaltyAttrMap::iterator itAttr = penaltyMap.find(streamerId);
	if( itAttr == penaltyMap.end() )
		return false;
	
	if( itAttr->second.bStatus&& itAttr->second.penaltyValue <= 0  )
		return true;
	else
		return false;
}

}}//namespace ZQTianShan::AccreditedPath

