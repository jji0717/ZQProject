#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#endif
#include "PhoNGOD_DVBC.h"
#include "public.h"
#include "Log.h"
#include <time.h>
#include <algorithm>
#include "TianShanIceHelper.h"

namespace ZQTianShan {
namespace AccreditedPath {

///schema for STRMLINK_TYPE_NGOD_DVBC
static ::ZQTianShan::ConfItem IpEdge_DVBC[] = {	
{ "Qam.modulationFormat",	::TianShanIce::vtInts,	false, "0x10",				false },
{ "Qam.IP",			::TianShanIce::vtStrings,		false, "192.168.80.138",	false },
{ "Qam.Mac",		::TianShanIce::vtStrings,		false, "a:b:c:d:e:f",		false },
{ "Qam.basePort",	::TianShanIce::vtInts,			false, "4001",				false },
{ "Qam.portMask",	::TianShanIce::vtInts,			false, "65280",				false },
{ "Qam.portStep",	::TianShanIce::vtInts,			false,	"1",				false },
{ "Qam.symbolRate",	::TianShanIce::vtInts,			false, "50000",				false },
{ "Qam.frequency",	::TianShanIce::vtInts,			false, "1150",				false },
{ "PN",          	::TianShanIce::vtInts,			false, " 5 ~ 20 ",			true  },
{ "TotalBandwidth",	::TianShanIce::vtLongs,			false, "20000",				false }, // in Kbps
{ "SopName",		::TianShanIce::vtStrings,		true,  "",				false },
{ "SopGroup",		::TianShanIce::vtStrings,		true,  "",				false },
{ NULL,			::TianShanIce::vtInts,			true,  "",				false },
};

static ::ZQTianShan::ConfItem IpEdge_DVBC_ShareLink[] = {
	{"LinkId",	::TianShanIce::vtStrings,	false , "" , false },
	{"SopName", ::TianShanIce::vtStrings,	true , "" , false },
	{"SopGroup", ::TianShanIce::vtStrings,	true , "" , false },
	{NULL, ::TianShanIce::vtInts, true, "",false },
};

IpEdgePHO_DVBC::IpEdgePHO_DVBC(IPHOManager& mgr)
: IStreamLinkPHO(mgr)
{
	_phoManager = &mgr;

}

IpEdgePHO_DVBC::~IpEdgePHO_DVBC()
{
	_helperMgr.unregisterStreamLinkHelper( STRMLINK_TYPE_NGOD_DVBC );
	_helperMgr.unregisterStorageLinkHelper( STRMLINK_TYPE_NGOD_DVBC_SHARELINK );
}
bool IpEdgePHO_DVBC::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;

	// address the schema definition
	if (0 == strcmp(type, STRMLINK_TYPE_NGOD_DVBC))
	{
		config = IpEdge_DVBC;
	}
	else if( 0 == strcmp( type , STRMLINK_TYPE_NGOD_DVBC_SHARELINK ))
	{
		config = IpEdge_DVBC_ShareLink;
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

void IpEdgePHO_DVBC::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
	{
		type = "NULL";
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC",1001,"unsupported type [%s] in NgodSop_DVBC", type);
	}
	if (0 == strcmp( STRMLINK_TYPE_NGOD_DVBC, type))
	{
		validate_DvbcStrmLinkConfig( identStr, configPD );
	}
	else if( 0 == strcmp( STRMLINK_TYPE_NGOD_DVBC_SHARELINK , type ) )
	{
		validate_DvbcStrmShareLinkConfig( identStr , configPD );
	}
	configPD.erase( _SERVICE_STATE_KEY_ );
}

void IpEdgePHO_DVBC::validate_DvbcStrmShareLinkConfig( const char* identStr, ::TianShanIce::ValueMap& configPD )
{
	glog(ZQ::common::Log::L_DEBUG , CLOGFMT(NgodSop_DVBC , "enter validate a DVBC shared streamlink's configuration: link[%s]" ),identStr);
	DVBCSharedAttr dsa;
	try
	{		
		TianShanIce::Variant var ;
		var.bRange = false;
		var = PDField(configPD , "LinkId");
		if ( var.type != ::TianShanIce::vtStrings ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC", 1051 ,"Invalid LinkId type,should be vtString");
		}
		if (var.strs.size() == 0) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC", 1052 ,"Invalid LinkId with size() == 0 ");
		}
		std::string& strTempId = var.strs[0];
		std::string::size_type slashPos = strTempId.find("/");
		if ( slashPos != std::string::npos ) 
		{
			strTempId = strTempId.substr( slashPos+1 );
		}
		dsa._streamLinkId = strTempId;

		// validate that SopName or SopGroup must be specified
		try 
		{
			var	= PDField(configPD,"SopName");
		} 
		catch ( const TianShanIce::InvalidParameter&) 
		{
			var.type = TianShanIce::vtStrings;
			var.strs.push_back("");
			configPD["SopName"] = var;			
		}
		
		TianShanIce::Variant var2;
		var2.bRange	= false;
		try
		{
			var2 = PDField(configPD, "SopGroup");
		}
		catch( const TianShanIce::InvalidParameter& )
		{
			var2.type = TianShanIce::vtStrings;
			var2.strs.push_back("");
			configPD["SopGroup"] = var2;
		}
		
		if( var.type != TianShanIce::vtStrings || var.strs.size() <= 0 || var2.type != TianShanIce::vtStrings || var2.strs.size() <= 0)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC", 1053, "neither SopName nor SopGroup is specified");
		}		
		if( var.strs[0].empty() && var2.strs[0].empty() )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC", 1012, "both SopName and SopGroup are empty");
		}
		dsa._strSOPName	= var.strs[0];
		dsa._strSOPGroup= var2.strs[0];

		glog(ZQ::common::Log::L_INFO,
			CLOGFMT( IpEdgePHO , "Update DVBC shared stream link with id[%s] and target LinkId[%s], sopName[%s] sopGroup[%s]" ),
			identStr , dsa._streamLinkId.c_str(), dsa._strSOPName.c_str(), dsa._strSOPGroup.c_str());
		{
			ZQ::common::MutexGuard gd(_sharedDVBCLinkAttrLocker);
			_sharedDVBCLinkAttrmap[identStr] = dsa;		
		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	glog(ZQ::common::Log::L_DEBUG , CLOGFMT(NgodSop_DVBC , "leave validate a DVBC shared streamlink's configuration: link[%s]" ),identStr);
}

void IpEdgePHO_DVBC::validate_DvbcStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodSop_DVBC, "enter validate a DVBC StreamLink's Configuration: link[%s]"), identStr);

	//retrieve data from configPD	
	//setup a link dvbc attribute instance
	LinkDVBCAttr la;
	la._streamLinkID=identStr;
	::TianShanIce::Variant val;
	val.bRange=false;
	try
	{
		
		//get qam'name ,if there is no qam name ,take qam ip as it's name

		///get total bandwidth and set available bandwidth
		val=PDField(configPD,"TotalBandwidth");
		if( val.type!=::TianShanIce::vtLongs )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC",1031,"invalid TotalBandwidth type,should be vtLongs");
		}
		if (val.lints.size()==0) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC",1032,"invalid TotalBandwidth type,no bandwidth is found");
		}
		la._totalBandWidth			=	val.lints[0]*1000;
		la._availableBandwidth		=	la._totalBandWidth;//Available Bandwidth is useless
		la._usedBandwidth			=	0;//initialize usedBandwidth as 0 

		glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] get totalBW[%lld]"),identStr,la._totalBandWidth);

		///get program number and set total pn count
		val=PDField(configPD,"PN");
		if(val.type!=::TianShanIce::vtInts)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC",1033,"Invalid PN type,should be vtInts");
		}
		if (val.bRange)
		{
			if (val.ints.size() < 2) 
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC",1034,"Invalid PN,range=true but content size < 2");
			}
			::Ice::Int minPn = val.ints[0],  maxPn = val.ints[1];
			if (minPn > maxPn)
			{
				Ice::Int temp = minPn;
				minPn = maxPn ;
				maxPn = temp;
				//std::swap<::Ice::Int> (minPn, maxPn);
			}

			la._pnMin = minPn;
			la._pnMax = maxPn;
			for (; minPn <= maxPn; minPn++)
				la._availablePN.push_front(minPn);
			glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] get ProgramNumber [%d,%d] count=[%d]"),
				identStr,la._pnMin,la._pnMax,la._pnMax+1-la._pnMin);
		}
		else
		{
			if (val.ints.size()==0) 
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC",1035,"Invalid PN data,data size is 0");
			}
			else if (val.ints.size() == 1)
			{
				//How to deal with multiple values
				la._pnMax = la._pnMin = val.ints[0];
				la._availablePN.push_back(val.ints[0]);
				glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] get ProgramNumber [%d,%d] count=[%d]"),
					identStr,la._pnMin,la._pnMax,la._pnMax+1-la._pnMin);
			}
			else 
			{
				la._pnMax = la._pnMin = -1;
				for (int  i = 0 ; i < (int) val.ints.size()  ; i++ ) 
				{
					std::list<Ice::Int>::const_iterator itBackup = la._backupPN.begin();					
					for ( ;itBackup != la._backupPN.end() ; itBackup ++ ) 
					{
						if ( *itBackup == val.ints[i] ) 
						{
							glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] with duplicate PN [%d]"),identStr,val.ints[i]);
							ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC",1036,"streamLink [%s] input has duplicate PN %d",identStr , val.ints[i]);
						}
					}
					la._backupPN.push_back(val.ints[i]);
					la._availablePN.push_back(val.ints[i]);
					glog(ZQ::common::Log::L_INFO , CLOGFMT(NgodSop_DVBC , "Stream DVBC link[%s] with input PN [%d]"),identStr , val.ints[i]);
				}
				glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] get ProgramNumber count=[%d]"),
					identStr, la._availablePN.size() );
			}
		}
		la._totalPNCount	= la._availablePN.size();

		//get port mask default is 65280 ==>0xFF00		
		try
		{
			val=PDField(configPD,"Qam.portMask");
			if (val.type == TianShanIce::vtInts &&val.ints.size()>0) 
			{
				la._portMask = val.ints[0];
				glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] get portmask [%x]"),identStr,la._portMask);
			}
			else
			{
				la._portMask = 0xFF00;
				glog(ZQ::common::Log::L_WARNING,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] invalid portmask type or data,set it to [%x]"),identStr,0xFF00);
			}
		}
		catch (const TianShanIce::InvalidParameter&) 
		{
			la._portMask = 0xFF00;
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] can't get portmask,setit to [%x]"),identStr,0xFF00);
		}

		try
		{
			val = PDField(configPD,"Qam.portStep");
			if (val.type == TianShanIce::vtInts && val.ints.size() >0) 
			{
				la._portStep = val.ints[0];
				glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] get portstep [%d]"),identStr,la._portStep);
			}
			else
			{
				la._portStep = 1;
				glog(ZQ::common::Log::L_WARNING,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] invalid portstep type or data,set it to [%d]"),identStr,la._portStep);
			}
		}
		catch (const TianShanIce::InvalidParameter&) 
		{
			la._portStep = 1;
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] can't get portstep,set it to [%d]"),identStr,la._portStep);
		}


		///get base port and format it
		val=PDField(configPD,"Qam.basePort");
		if(val.type!=::TianShanIce::vtInts)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1037,"Invalid Qam.baseport type,should be vtInts");
		}
		la._basePort	= val.ints[0] & (unsigned int)la._portMask;	//最后八位置0
		glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"Stream DVBC link [%s] get baseport[%d] and change it to [%d] with mask[%x]"),
			identStr,val.ints[0],la._basePort,la._portMask);

		// validate sop name and sop group
		try 
		{
			val	= PDField(configPD,"SopName");
		} 
		catch ( const TianShanIce::InvalidParameter&) 
		{
			val.type = TianShanIce::vtStrings;
			val.strs.push_back("");
			configPD["SopName"] = val;			
		}
		
		TianShanIce::Variant var2;
		var2.bRange	= false;
		try
		{
			var2 = PDField(configPD, "SopGroup");
		}
		catch( const TianShanIce::InvalidParameter& )
		{
			var2.type = TianShanIce::vtStrings;
			var2.strs.push_back("");
			configPD["SopGroup"] = var2;
		}
		
		if( val.type != ::TianShanIce::vtStrings || var2.type != TianShanIce::vtStrings)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC", 1011, "Invalid type of SopName or SopGroup, should be vtStrings");
		}

		if ( val.strs.size() <= 0 || var2.strs.size() <= 0) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC", 1012, "neither SopName nor SopGroup is specified");
		}
		if( val.strs[0].empty() && var2.strs[0].empty() )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NgodSop_DVBC", 1012, "both SopName and SopGroup are empty");
		}
		la._strSOPName	= val.strs[0];
		la._strSOPGroup	= var2.strs[0];
		glog(ZQ::common::Log::L_INFO, CLOGFMT(NgodSop_DVBC,"stream IP link[%s] get sopName[%s] sopGroup[%s]"),
			identStr, la._strSOPName.c_str(), la._strSOPGroup.c_str());
		
		Ice::Int iServiceState = TianShanIce::stNotProvisioned;
		ZQTianShan::Util::getValueMapDataWithDefault( configPD, _SERVICE_STATE_KEY_, TianShanIce::stNotProvisioned, iServiceState );
		
		if( iServiceState != TianShanIce::stInService )
		{
			///find out the used bandwidth and program number,then remove it from available resource
			::TianShanIce::Transport::StreamLinkPrx strmLinkPrx=_phoManager->openStreamLink(identStr);
			if(strmLinkPrx)
			{
				//_dvbcResourceDataMap.clear();

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
				for(;itID!=idc.end();itID++)
				{
					::TianShanIce::Transport::PathTicketPrx ticketprx=_phoManager->openPathTicket(*itID);
					if(ticketprx)
					{
						TianShanIce::ValueMap PDData=ticketprx->getPrivateData();
						TianShanIce::SRM::ResourceMap dataMap=ticketprx->getResources();

						::TianShanIce::Variant bandwidth;
						::TianShanIce::Variant pn;

						Ice::Long	lBandwidth;
						Ice::Int	lPn;
						try
						{
							//check the bandwidth resource
							TianShanIce::SRM::ResourceMap::const_iterator it=dataMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
							if(it==dataMap.end())
							{
								glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"validate_DvbcStrmLinkConfig ticket %s but no bandwidth resource"),itID->name.c_str());
							}
							else
							{	
								TianShanIce::ValueMap val=it->second.resourceData;
								bandwidth=PDField(PDData,PathTicketPD_Field(bandwidth));
								glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"validate_DvbcStrmLinkConfig() ticket %s used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);
								lBandwidth = bandwidth.lints[0];
								la._availableBandwidth -= lBandwidth;								
								la._usedBandwidth += lBandwidth;
							}

							//check the program number resource
							it=dataMap.find(TianShanIce::SRM::rtMpegProgram);
							if(it==dataMap.end())
							{
								glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"validate_DvbcStrmLinkConfig() ticket %s but no porgram number resource"),itID->name.c_str());
							}
							else
							{
								TianShanIce::ValueMap val=it->second.resourceData;
								pn=PDField(PDData,PathTicketPD_Field(PN));

								//remove the used pn from attr
								int iPn=pn.ints[0];
								std::list<int>::iterator itAvailPN=la._availablePN.begin();
								while (itAvailPN!=la._availablePN.end() && iPn!=*itAvailPN)
								{
									itAvailPN++;
								}								
								if(itAvailPN==la._availablePN.end())
								{
									glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"validate_DvbcStrmLinkConfig() can't find the programnumber %d in available pn list"),pn.ints[0]);
								}
								else
								{
									glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"validate_DvbcStrmLinkConfig() used porgram %d from available program number list"),pn.ints[0]);
									lPn = iPn;
									la._availablePN.erase(itAvailPN);
								}
							}

							ResourceDVBCData rdData;
							rdData._usedBandWidth	=	lBandwidth;
							rdData._usedPN			=	lPn;
							_dvbcResourceDataMap.insert(ResourceDVBCDataMap::value_type(itID->name,rdData));
						}
						catch(::TianShanIce::InvalidParameter& e)
						{
							glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"validate_DvbcStrmLinkConfig() invalidParameter exception is caught:%s"),e.message.c_str());
						}
						catch (...)
						{

						}
					}
				}
			}
			
			ZQ::common::MutexGuard gd(_dvbcResourceLocker);
			LinkDVBCAttrMap::iterator itAttr = _StreamLinkDVBCAttrmap.find( la._streamLinkID );
			if(itAttr==_StreamLinkDVBCAttrmap.end())
			{
				_StreamLinkDVBCAttrmap.insert(std::make_pair<std::string,LinkDVBCAttr>( la._streamLinkID , la ));
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(NgodSop_DVBC,"insert a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedBandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());
			}
			else
			{
				itAttr->second = la;
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(NgodSop_DVBC,"update a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedbandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());
			}
		}
		else
		{			
			ZQ::common::MutexGuard gd(_dvbcResourceLocker);
			LinkDVBCAttrMap::iterator itAttr = _StreamLinkDVBCAttrmap.find( la._streamLinkID );

			if(itAttr==_StreamLinkDVBCAttrmap.end())
			{
				_StreamLinkDVBCAttrmap.insert(std::make_pair<std::string,LinkDVBCAttr>( la._streamLinkID , la ));
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(NgodSop_DVBC,"insert a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedBandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());
			}
			else
			{
				la._usedBandwidth		= itAttr->second._usedBandwidth;
				la._availableBandwidth	= itAttr->second._totalBandWidth - itAttr->second._usedBandwidth;

				//TODO: filter out the used program number
				std::vector<Ice::Int> tmpTotal;
				LinkDVBCAttr& attr = itAttr->second;
				for ( Ice::Int iPort = attr._pnMin ; iPort <= attr._pnMax ; iPort ++ )
				{
					tmpTotal.push_back(iPort);
				}

				std::vector<Ice::Int> availOld;
				ZQTianShan::Util::copyListToVector<Ice::Int>( attr._availablePN , availOld );
				std::vector<Ice::Int> availNew;
				ZQTianShan::Util::copyListToVector<Ice::Int>( la._availablePN , availNew );


				std::sort( availOld.begin(),  availOld.end() , std::less<Ice::Int>( ));
				std::sort( availNew.begin(),  availNew.end() , std::less<Ice::Int>( ));
				
				std::vector<Ice::Int> tmpUsed;
				std::set_difference( tmpTotal.begin() , tmpTotal.end() , availOld.begin() , availOld.end(), std::inserter< std::vector<Ice::Int> >( tmpUsed , tmpUsed.begin( ) ) );
				
				std::vector<Ice::Int> tmpNew;
				std::set_difference( availNew.begin() , availNew.end() , tmpUsed.begin() , tmpUsed.end() , std::inserter< std::vector<Ice::Int> >( tmpNew , tmpNew.begin() ) );

				la._availablePN.clear();
				ZQTianShan::Util::copyVectorToList<Ice::Int>( tmpNew , la._availablePN );

				itAttr->second = la;
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(NgodSop_DVBC,"update a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedbandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());

			}
		}
	}
	catch(::TianShanIce::BaseException& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"tianshan expection caught :%s"),e.message.c_str());		
		e.ice_throw();
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"validate_DvbcStrmLinkConfig() unexpect error"));
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodSop_DVBC, "leave validate a DVBC StreamLink's Configuration: link[%s]"), identStr);
}

#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
else glog(ZQ::common::Log::L_WARNING, CLOGFMT(NgodSop_DVBC, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}

Ice::Int IpEdgePHO_DVBC::doEvaluation(LinkInfo& linkInfo, 
const SessCtx& sessCtx,
TianShanIce::ValueMap& hintPD,
const ::Ice::Int oldCost)
{
	std::string &linktype =linkInfo.linkType;
	if (0 == linktype.compare(STRMLINK_TYPE_NGOD_DVBC))
	{
		return eval_DvbcStrmLink(linkInfo, sessCtx, hintPD, linkInfo.rcMap,oldCost);
	}
	else if( 0 == linktype.compare( STRMLINK_TYPE_NGOD_DVBC_SHARELINK ) )
	{
		return eval_DvbcStrmShareLink(linkInfo, sessCtx, hintPD, linkInfo.rcMap,oldCost);
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR , CLOGFMT(NgodSop_DVBC , "unrecognized streamlink type[%s]" ),linktype.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
}

Ice::Int IpEdgePHO_DVBC::eval_DvbcStrmShareLink(LinkInfo& linkInfo, 
											   const SessCtx& sessCtx, 
											   TianShanIce::ValueMap& hintPD, 
											   TianShanIce::SRM::ResourceMap& rcMap ,
											   const ::Ice::Int oldCost)
{
	glog(ZQ::common::Log::L_DEBUG,
		CLOGFMT(NgodSop_DVBC,"eval_DvbcShareStrmLink() using shared link with linkID[%s]"),
		linkInfo.linkIden.name.c_str());
	std::string strLinkId = "";
	{
		ZQ::common::MutexGuard gd(_sharedDVBCLinkAttrLocker);
		//_sharedDVBCLinkAttrmap
		LinkDVBCShareAttrMap::const_iterator it = _sharedDVBCLinkAttrmap.find( linkInfo.linkIden.name );
		if ( it != _sharedDVBCLinkAttrmap.end() ) 
		{
			strLinkId = it->second._streamLinkId;
			glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC , "eval_DvbcShareStrmLink() find the target linkID [%s]" ),
				strLinkId.c_str() );
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC , "eval_DvbcShareStrmLink() no shared dvbc link with ID [%s] ,return with OutOfServiceCost" ),
				linkInfo.linkIden.name.c_str() );
			return 	::TianShanIce::Transport::OutOfServiceCost;			 
		}
	}

	LinkInfo lInfo = linkInfo;
	{
		ZQ::common::MutexGuard lockDVBCres(_dvbcResourceLocker);
		LinkDVBCAttrMap::const_iterator itDvbc = _StreamLinkDVBCAttrmap.find( strLinkId );
		if (itDvbc == _StreamLinkDVBCAttrmap.end( ) ) 
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NgodSop_DVBC , "eval_DvbcShareStrmLink() target streamlink [%s] is not found" ),
				strLinkId.c_str());
			return 	::TianShanIce::Transport::OutOfServiceCost;
		}	
		try
		{			
			lInfo.linkIden.name = strLinkId ;
			lInfo.linkPrx = _helperMgr.openStreamLink(strLinkId.c_str() );		
		}
		catch ( const Ice::Exception& ex )
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NgodSop_DVBC , "eval_DvbcShareStrmLink() catch ice exception [%s] when open streamlink [%s]"),
				ex.ice_name().c_str() , strLinkId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NgodSop_DVBC , "eval_DvbcShareStrmLink() catch unknown exception when open streamlink [%s]"),
				strLinkId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	Ice::Int retCost =  eval_DvbcStrmLink(lInfo , sessCtx , hintPD , rcMap , oldCost );
	return retCost;
}


int IpEdgePHO_DVBC::evalDVBCStreamLinkCost(const std::string& streamLinkID,Ice::Long bw2Alloc,const std::string& sessId)
{	
	ZQ::common::MutexGuard gd(_dvbcResourceLocker);
	LinkDVBCAttrMap::iterator it=_StreamLinkDVBCAttrmap.find(streamLinkID);
	if(it == _StreamLinkDVBCAttrmap.end())
	{
		std::string strErr="no streamlink attr is found through the streamlinkID=";
		strErr+=streamLinkID;
		throw strErr;
	}
	//should I put a mutex here???
	int bandwidthCost= ::TianShanIce::Transport::OutOfServiceCost;

	int pnCost=( ( it->second._totalPNCount - (int)it->second._availablePN.size() ) *
		(int)::TianShanIce::Transport::MaxCost) / 
		(it->second._totalPNCount);
	glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,
		"evalDVBCStreamLinkCost() Session[%s] streamlink [%s] has pnCount=[%d] and UsedPnCount=[%d] "
		"totalBW=[%lld] UsedBW=[%lld] ,pnCost=[%d]"),
		sessId.c_str(),streamLinkID.c_str(),
		it->second._totalPNCount ,it->second._totalPNCount- (int)it->second._availablePN.size(),
		it->second._totalBandWidth,it->second._usedBandwidth,pnCost);

	Ice::Long usedBW=it->second._usedBandwidth;
	Ice::Long total=it->second._totalBandWidth;
	if( ( total-usedBW ) >= bw2Alloc)	
	{		
		bandwidthCost=(int) (  (usedBW)*(::TianShanIce::Transport::MaxCost) / total );		
		glog(ZQ::common::Log::L_INFO,
			CLOGFMT(NgodSop_DVBC,"evalDVBCStreamLinkCost() Session[%s] streamlink [%s] allocbandwidth [%lld] "
			"totalBW=[%lld] UsedBW=[%lld] ,bwCost=[%d]"),
			sessId.c_str(),streamLinkID.c_str(),
			bw2Alloc,it->second._totalBandWidth,
			it->second._usedBandwidth,bandwidthCost);
	}
	int returnCost=max(pnCost,bandwidthCost);
	glog(ZQ::common::Log::L_INFO,
		CLOGFMT(NgodSop_DVBC,"evalDVBCStreamLinkCost() Session[%s] streamlink [%s] "
		" return cost=[%d] (pnCost=[%d],BWCost=[%d]) "),
		sessId.c_str(),streamLinkID.c_str(),returnCost,pnCost,bandwidthCost);

	return returnCost;
}

Ice::Int IpEdgePHO_DVBC::eval_DvbcStrmLink(LinkInfo& linkInfo,
											const SessCtx& sessCtx,
											TianShanIce::ValueMap& hintPD,
											TianShanIce::SRM::ResourceMap& rcMap ,
											const ::Ice::Int oldCost)
{
	int	newCost = oldCost;
	std::string sessId=sessCtx.sessId;

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodSop_DVBC, "[Session[%s] enter eval_DvbcStrmLink() with oldCost=%d"),sessId.c_str(),oldCost);

	if (oldCost > ::TianShanIce::Transport::MaxCost)
	{
		glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"Session[%s] oldCost cost is bigger than MaxCost,return with OldCost=%d"),sessId.c_str(),oldCost);
		return oldCost;
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
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"Session[%s] ice exception is caught:%s"),sessId.c_str(),e.ice_name().c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"Session[%s] unexpect error is threw out when call streamlink's getPrivateData"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	if (1)
	{
		try
		{
			// try to get the bandwidth requirement from the session context
			if (resourceMap.end() != resourceMap.find(::TianShanIce::SRM::rtTsDownstreamBandwidth))
			{
				::TianShanIce::SRM::Resource& tsDsBw = resourceMap[::TianShanIce::SRM::rtTsDownstreamBandwidth];
				if (tsDsBw.resourceData.end() != tsDsBw.resourceData.find("bandwidth") && 
					!tsDsBw.resourceData["bandwidth"].bRange &&
					tsDsBw.resourceData["bandwidth"].lints.size() !=0)
				{
					bw2Alloc = tsDsBw.resourceData["bandwidth"].lints[0];
				}
				else
				{
					glog(ZQ::common::Log::L_WARNING, CLOGFMT(NgodSop_DVBC, "eval_DvbcStrmLink() unacceptable rtTsDownstreamBandwidth in session: bandwidth(range=%d, size=%d)"),
						tsDsBw.resourceData["bandwidth"].bRange,
						tsDsBw.resourceData["bandwidth"].lints.size());
				}
			}
			else
			{//error if no bandwidth parameter???
				glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"eval_DvbcStrmLink() Session[%s] unacceptable resource without bandwidth"),sessId.c_str());
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(EXPFMT("IpEdgePHO_DVBC",1041,"eval_DvbcStrmLink() no 'bandwidth' is found in resoucemap"));
			}
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(NgodSop_DVBC, "eval_DvbcStrmLink() Session[%s] can not query the given session for resource info, stop evaluation"),sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	else
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(NgodSop_DVBC, "eval_DvbcStrmLink() Session[%s] no session specified, use hinted private data only"),sessId.c_str());

	// step 2, adjust if the hintPD also specify the bandwidth to the max of them
	if (hintPD.end() != hintPD.find(PathTicketPD_Field(bandwidth)))
	{
		::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(bandwidth)];
		if (var.type == TianShanIce::vtLongs && var.lints.size() >0)
			bw2Alloc = (bw2Alloc >0) ? max(bw2Alloc, var.lints[0]) : var.lints[0];
	}

	// step 2.1, double check if the bandwidth is valid
	if (bw2Alloc <= 0)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NgodSop_DVBC, "eval_DvbcStrmLink() Session[%s] 0 bandwidth has been specified, quit evaluation"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodSop_DVBC, "eval_DvbcStrmLink() Session[%s] required bandwidth:%lldbps"),sessId.c_str(), bw2Alloc);

	// 	TianShanIce::Variant valPN;
	// 	valPN.bRange=false;

	TianShanIce::Variant valBandwidth;
	valBandwidth.lints.clear();
	valBandwidth.lints.push_back(bw2Alloc);
	valBandwidth.type=TianShanIce::vtLongs;
	valBandwidth.bRange=false;
	try
	{
		newCost=evalDVBCStreamLinkCost(linkInfo.linkIden.name , bw2Alloc , sessId);
		if(newCost > ::TianShanIce::Transport::MaxCost)
		{
			glog(ZQ::common::Log::L_INFO,
				CLOGFMT(NgodSop_DVBC,"eval_DvbcStrmLink() Session[%s] streamLink=%s return with outofServiceCost with cost=%d"),
				sessId.c_str(),linkInfo.linkIden.name.c_str(),newCost);
			return newCost;
		}

	}
	catch (std::string& str)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"%s"),str.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"Session[%s] unexpect error when eval dvbc streamlink cost"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	//hintPD[PathTicketPD_Field(PN)] = valPN;
	hintPD[PathTicketPD_Field(bandwidth)] = valBandwidth;
	hintPD[PathTicketPD_Field(Qam.mode)] = linkPD["Qam.modulationFormat"];
	hintPD[PathTicketPD_Field(Qam.IP)] = linkPD["Qam.IP"];
	hintPD[PathTicketPD_Field(Qam.basePort)] = linkPD["Qam.basePort"];
	hintPD[PathTicketPD_Field(Qam.symbolRate)] = linkPD["Qam.symbolRate"];
	hintPD[PathTicketPD_Field(Qam.frequency)] = linkPD["Qam.frequency"];
	//hintPD[PathTicketPD_Field(Streamer.SpigotID)]=linkPD["Streamer.SpigotID"];
	hintPD[PathTicketPD_Field(destAddr)] = linkPD["Qam.IP"];
	#pragma message(__MSGLOC__"需要加入数据到resourceMap for ticket")	



	//fill rtEthernetInterface
	PutResourceMapData(rcMap,TianShanIce::SRM::rtEthernetInterface,"destIP",linkPD["Qam.IP"],sessId);
	//port 需要等到narrow的时候才能确定
	PutResourceMapData(rcMap,TianShanIce::SRM::rtEthernetInterface,"destMac",linkPD["Qam.Mac"],sessId);

	///fill rtTsDownstreamBandwidth	
	PutResourceMapData(rcMap,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth",valBandwidth,sessId);


	//fill rtPhysicalChannel
	PutResourceMapData(rcMap,TianShanIce::SRM::rtPhysicalChannel,"channelId",linkPD["Qam.frequency"],sessId);

	//fill rtAtscModulationMode
	PutResourceMapData(rcMap,TianShanIce::SRM::rtAtscModulationMode,"symbolRate",linkPD["Qam.symbolRate"],sessId);

	PutResourceMapData(rcMap,TianShanIce::SRM::rtAtscModulationMode,"modulationFormat",linkPD["Qam.modulationFormat"],sessId);


	glog(ZQ::common::Log::L_DEBUG,
		CLOGFMT(NgodSop_DVBC,"Session[%s] StreamLink=%s return with cost=%d"),
		sessId.c_str(),linkInfo.linkIden.name.c_str(),max(oldCost,newCost));
	return max(oldCost,newCost);
}

IPathHelperObject::NarrowResult IpEdgePHO_DVBC::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, 
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
	if ( 0 == type.compare( STRMLINK_TYPE_NGOD_DVBC ) )
	{
		return narrow_DvbcStrmLink( strmLink , sessCtx , ticket );
	}
	else if( 0 == type.compare(STRMLINK_TYPE_NGOD_DVBC_SHARELINK))
	{
		return narrow_DvbcShareStrmLink( strmLink , sessCtx , ticket );
	}
	else
	{
		glog( ZQ::common::Log::L_ERROR,
			CLOGFMT(NgodSop_DVBC , "doNarrow() unrecognized stream link type [%s]" ),
			type.c_str() );
		return IPathHelperObject::NR_Unrecognized;
	}
}

IPathHelperObject::NarrowResult IpEdgePHO_DVBC::narrow_DvbcShareStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
																	 const SessCtx& sessCtx,
																	 const TianShanIce::Transport::PathTicketPtr&  ticket)
{
	try
	{
		std::string strLinkType = strmLink->getType();
		if (strcmp( strLinkType.c_str() , STRMLINK_TYPE_NGOD_DVBC_SHARELINK ) !=0 ) 
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NgodSop_DVBC,"narrow_DvbcSharedStrmLink() the streamLink type is [%s] and does not match [%s]"),
				strLinkType.c_str()	,
				STRMLINK_TYPE_NGOD_DVBC_SHARELINK );
			return NR_Error;
		}
		std::string strLinkId = strmLink->getIdent().name;
		std::string strTargetLinkId = "";
		std::string strSopName;
		std::string strSopGroup;
		{
			ZQ::common::MutexGuard gd( _sharedDVBCLinkAttrLocker );
			LinkDVBCShareAttrMap::const_iterator it = _sharedDVBCLinkAttrmap.find(strLinkId);
			if ( it == _sharedDVBCLinkAttrmap.end()) 
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(NgodSop_DVBC , "narrow_DvbcSharedStrmLink() Can't find target linkId through link[%s]" ),
					strLinkId.c_str());
				return NR_Error;					 
			}
			strTargetLinkId = it->second._streamLinkId;	
			strSopName     	= it->second._strSOPName;
			strSopGroup     = it->second._strSOPGroup;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(NgodSop_DVBC , "narrow_DvbcSharedStrmLink() find a target link[%s] with LinkId[%s], sopName[%s] sopGroup[%s]"),
			strTargetLinkId.c_str() , strLinkId.c_str(), strSopName.c_str(), strSopGroup.c_str());
		NarrowResult ret =  inner_narrow_DvbcStrmLink(strTargetLinkId,sessCtx,ticket);

		TianShanIce::Variant var;
		var.bRange	= false;
		var.type		= TianShanIce::vtStrings;
		var.strs.clear();
		var.strs.push_back(strSopName);
		ticket->privateData[PathTicketPD_Field(sop_name)] = var;
		
		var.strs.clear();
		var.strs.push_back(strSopGroup);
		ticket->privateData[PathTicketPD_Field(sop_group)] = var;
		glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"narrow_DvbcSharedStrmLink() overwrite with sopName[%s] and sopGroup[%s] for ticket[%s],streamLink[%s]"),
			strSopName.c_str(), strSopGroup.c_str(), ticket->ident.name.c_str(), strLinkId.c_str() );
		return ret;
	}
	catch ( const Ice::Exception& ex ) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC , "narrow_DvbcSharedStrmLink() catch ice exception:%s" ),ex.ice_name().c_str());
		return NR_Error;
	}
	catch (... ) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC , "narrow_DvbcSharedStrmLink() catch unknown exception " ) );
		return NR_Error;
	}
}

IPathHelperObject::NarrowResult IpEdgePHO_DVBC::inner_narrow_DvbcStrmLink(const std::string strStrmLinkID , 
																		const SessCtx& sessCtx,
																		const TianShanIce::Transport::PathTicketPtr&  ticket)
{
	//如何narrow呢?
	//step 1.find a unused program number
	int pn=0;
	int port=0;
	std::string strSopName;
	std::string strSopGroup;
	TianShanIce::SRM::ResourceMap::iterator itResMap=ticket->resources.find(TianShanIce::SRM::rtEthernetInterface);
	std::string sessId= sessCtx.sessId;

	if(itResMap==ticket->resources.end())
	{
		glog(ZQ::common::Log::L_ERROR,
			CLOGFMT(NgodSop_DVBC,"narrow_DvbcStrmLink() Session[%s] can't find "
			"rtEthernetInterface from ticket resources"),
			sessId.c_str());
		return 	NR_Error;		 
	}
	std::string	strTicketID = ticket->ident.name;

	TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
	int AvailPnCount=0;
	Ice::Long  bw2Alloc;
	{
		ZQ::common::MutexGuard gd(_dvbcResourceLocker);	
		READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
		LinkDVBCAttrMap::iterator it=_StreamLinkDVBCAttrmap.find(  strStrmLinkID );
		if(it==_StreamLinkDVBCAttrmap.end())
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NgodSop_DVBC,"narrow_DvbcStrmLink() Session[%s] can't find the streamlink attribute through streamlink id %s"),
				sessId.c_str(),strStrmLinkID.c_str());
			return NR_Error;
		}
		if(it->second._availablePN.size()<=0)
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NgodSop_DVBC,"Session[%s] streamlink=%s not enough available program number,totalPN[%d] usedPN[%d]"),
				sessId.c_str(),strStrmLinkID.c_str(),it->second._totalPNCount,it->second._totalPNCount-it->second._availablePN.size());
			return NR_Error;				 
		}
		if (it->second._totalBandWidth <= it->second._usedBandwidth) 
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(NgodSop_DVBC,"narrow_DvbcStrmLink() Session[%s] strmLink[%s] no available Badnwidth,totalBW[%lld] usedBW[%lld]"),
				sessId.c_str(),strStrmLinkID.c_str(),it->second._totalBandWidth,it->second._usedBandwidth);
			return NR_Error;
		}

		//modify pn allocation algorithm
		std::list<int>::iterator itPn=it->second._availablePN.begin();
		static bool bRandInit=false;
		if(!bRandInit)
		{
			srand(time(NULL));
			bRandInit=true;
		}

		int iOffset= 0;
		if( (int)it->second._availablePN.size() > 1 )
		{
			iOffset = rand() % ( it->second._availablePN.size() -1 );
			iOffset=iOffset>5?5:iOffset;
		}
		for(int i=0;i<iOffset ;i++)
			itPn++;

		pn =*itPn;
		it->second._availablePN.erase(itPn);

		//it->second._availableBandwidth-=bw2Alloc;
		it->second._usedBandwidth += bw2Alloc;

		//step 2.calculate the udp port
		port=it->second._basePort + it->second._portStep * (pn);

		AvailPnCount=it->second._availablePN.size();

		strSopName	= it->second._strSOPName;
		strSopGroup	= it->second._strSOPGroup;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(NgodSop_DVBC,"Session[%s] streamlink=[%s] ticketID=[%s] narrowed with BWAlloc=[%lld] PN=[%d]"
			" Now totalBW=[%lld] usedBW=[%lld] totalPn=[%d] usedPn=[%d]"),
			sessId.c_str(),strStrmLinkID.c_str(),strTicketID.c_str(),
			bw2Alloc,pn,it->second._totalBandWidth , it->second._usedBandwidth,
			it->second._totalPNCount,it->second._totalPNCount-(int)it->second._availablePN.size() );
	}	

	TianShanIce::Variant valPN;
	TianShanIce::Variant valPort;
	TianShanIce::Variant valBandwidth;

	valPN.bRange=false;
	valPN.type = TianShanIce::vtInts;
	valPN.ints.clear();
	valPN.ints.push_back(pn);
	valPN.type=TianShanIce::vtInts;
	ticket->privateData[PathTicketPD_Field(PN)]=valPN;
	PutResourceMapData(resMap,TianShanIce::SRM::rtMpegProgram,"Id",valPN,sessId);

	valPort.bRange=false;
	valPort.type = TianShanIce::vtInts;
	valPort.ints.clear();
	valPort.ints.push_back(port);
	valPort.type=TianShanIce::vtInts;
	ticket->privateData[PathTicketPD_Field(destPort)]=valPort;
	PutResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destPort",valPort,sessId);

	valBandwidth.bRange = false;
	valBandwidth.lints.clear();
	valBandwidth.type =TianShanIce::vtLongs;
	valBandwidth.lints.push_back(bw2Alloc);
	ticket->privateData[PathTicketPD_Field(bandwidth)] = valBandwidth;

	TianShanIce::Variant varLinkType;
	TianShanIce::Variant varLinkId;
	TianShanIce::Variant varSopName;
	TianShanIce::Variant varSopGroup;

	varLinkType.bRange = false;
	varLinkType.type = TianShanIce::vtStrings;
	varLinkType.strs.clear();
	varLinkType.strs.push_back( STRMLINK_TYPE_NGOD_DVBC );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkType)] = varLinkType;

	varLinkId.bRange = false;
	varLinkId.type = TianShanIce::vtStrings;
	varLinkId.strs.clear();
	varLinkId.strs.push_back( strStrmLinkID );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varLinkId;

	varSopName.bRange	= false;
	varSopName.type		= TianShanIce::vtStrings;
	varSopName.strs.clear();
	varSopName.strs.push_back(strSopName);
	ticket->privateData[PathTicketPD_Field(sop_name)] = varSopName;

	varSopGroup.bRange	= false;
	varSopGroup.type	= TianShanIce::vtStrings;
	varSopGroup.strs.clear();
	varSopGroup.strs.push_back(strSopGroup);
	ticket->privateData[PathTicketPD_Field(sop_group)] = varSopGroup;

	{
		ZQ::common::MutexGuard gd(_dvbcResourceLocker);
		ResourceDVBCData rdd;
		rdd._usedBandWidth = bw2Alloc;
		rdd._usedPN = pn;
		_dvbcResourceDataMap[strTicketID]=rdd;
	}

	glog(ZQ::common::Log::L_INFO,
		CLOGFMT(NgodSop_DVBC,"narrow_DvbcStrmLink() Session[%s] strmLink[%s] get dvbcresource with"
		" pn=[%d] port=[%d] bw=[%lld] with ticketID=[%s]"),
		sessId.c_str(),strStrmLinkID.c_str(), pn,port,bw2Alloc ,strTicketID.c_str());

	return NR_Narrowed;
}

IPathHelperObject::NarrowResult IpEdgePHO_DVBC::narrow_DvbcStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink,
																		const SessCtx& sessCtx,
																		const TianShanIce::Transport::PathTicketPtr&  ticket)
{	
	std::string	strStrmLinkID;
	try
	{
		strStrmLinkID=strmLink->getIdent().name;
	}
	catch (...) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"narrow_DvbcStrmLink() get streamlink's ID failed"));
		return NR_Error;
	}
	return inner_narrow_DvbcStrmLink(strStrmLinkID , sessCtx , ticket );
}

void IpEdgePHO_DVBC::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	try
	{
		TianShanIce::ValueMap& ticketPD = ticket->privateData;
		TianShanIce::ValueMap::const_iterator itLinkType = ticketPD.find( PathTicketPD_Field(ownerStreamLinkType) ) ;
		if ( itLinkType == ticketPD.end()  || itLinkType->second.type != TianShanIce::vtStrings ||itLinkType->second.strs.size() == 0  ) 
		{
			glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSop_DVBC,"no ticket owner link type is found"));
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkType->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"doFreeResource() can' get streamlink from ticket"));
			return ;
		}
		std::string strStreamlinkID = itLinkId->second.strs[0];
		Ice::Identity& ticketID = ticket->ident;

		//hack for STRMLINK_TYPE_NGOD_DVBC_SHARELINK
		if ( strcmp( STRMLINK_TYPE_NGOD_DVBC_SHARELINK , strLinkType.c_str() ) == 0 ) 
		{
			{
				ZQ::common::MutexGuard gd(_sharedDVBCLinkAttrLocker);
				LinkDVBCShareAttrMap::const_iterator it  = _sharedDVBCLinkAttrmap.find( strStreamlinkID );
				if ( it == _sharedDVBCLinkAttrmap.end() ) 
				{
					glog(ZQ::common::Log::L_ERROR,
						CLOGFMT(NgodSop_DVBC,"doFreeResources() can't find target link with DVBC shared linkId [%s] for ticket[%s]"),
						strStreamlinkID.c_str() ,ticketID.name.c_str() );
					return ;
				}

				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(NgodSop_DVBC , "doFreeResources() find target StreamLinkId[%s] through DVBC shared link Id[%s] for ticket[%s]" ),
					it->second._streamLinkId.c_str(),strStreamlinkID.c_str() , ticketID.name.c_str() );
				strStreamlinkID = it->second._streamLinkId;
				strLinkType = STRMLINK_TYPE_NGOD_DVBC;
			}			
		}
		
		if( strcmp(STRMLINK_TYPE_NGOD_DVBC,strLinkType.c_str())==0 )
		{//dvbc mode
			ZQ::common::MutexGuard gd(_dvbcResourceLocker);

			//find if there is a allocated dvbc resource
			ResourceDVBCDataMap::iterator itAlloc=_dvbcResourceDataMap.find(ticketID.name);
			if(itAlloc==_dvbcResourceDataMap.end())
			{//if not,return without free any resource
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(NgodSop_DVBC,"doFreeResource() no allocated dvbc resource with tickID=[%s]"),
					ticketID.name.c_str());
				return;
			}

			LinkDVBCAttrMap::iterator it=_StreamLinkDVBCAttrmap.find(strStreamlinkID);
			if(it==_StreamLinkDVBCAttrmap.end())
			{
				glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"doFreeResource() can't find strmlink attr through streamlink id=%s"),strStreamlinkID.c_str());
				return;
			}

			try
			{
//				TianShanIce::ValueMap& value=ticket->privateData;

				TianShanIce::Variant val;
				TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
				try
				{
					val=GetResourceMapData(resMap,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
				}
				catch (TianShanIce::InvalidParameter&)
				{
					glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() can't find bandwith in resources,invalid resources"));
				}
				if (val.type!=TianShanIce::vtLongs || val.lints.size()<=0|| val.lints[0]!= itAlloc->second._usedBandWidth ) 
				{
					glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() can't find bandwith in resources or invalid bandwidth data"));
				}				
				try
				{
					val=GetResourceMapData(resMap,TianShanIce::SRM::rtMpegProgram,"Id");
				}
				catch(TianShanIce::InvalidParameter&)
				{
					glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() can't find program number in resources"));
				}
				if (val.type!=TianShanIce::vtInts || val.ints.size()==0 ||val.ints[0]!=itAlloc->second._usedPN) 
				{
					glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() can't find program number in resources or invalid pn data"));
				}
			}
			catch (Ice::Exception& ex) 
			{
				glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() ice exception [%s] error when validate the ticket private data"),
					ex.ice_name().c_str());
			}
			catch (...) 
			{
				glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() unexpetc error when validate the ticket private data"));
			}

			//free resource allocated by narrow function
			Ice::Long bandwidth=itAlloc->second._usedBandWidth;
			it->second._availableBandwidth+=bandwidth;
			it->second._usedBandwidth -= bandwidth;

			it->second._usedBandwidth = it->second._usedBandwidth > 0 ? it->second._usedBandwidth : 0 ;//adjust to 0

			{
				int pn=itAlloc->second._usedPN;
				if ( it->second._pnMax < 0 && it->second._pnMin < 0  ) 
				{
					bool bReleased = false;
					std::list<Ice::Int>& backLst = it->second._backupPN;
					std::list<Ice::Int>::const_iterator itBackup = backLst.begin();
					for (  ; itBackup != backLst.end() ; itBackup++) 
					{
						if ( pn == *itBackup ) 
						{
							it->second._availablePN.push_back(pn);
							glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() streamlink [%s]"
								" [ticket %s] with PN=[%d] BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
								strStreamlinkID.c_str(),ticketID.name.c_str(),
								pn,bandwidth,
								it->second._totalBandWidth,it->second._usedBandwidth,
								it->second._backupPN.size(),it->second._backupPN.size()-(int)it->second._availablePN.size());
							bReleased = true;
						}
					}
					if ( !bReleased) 
					{
						glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() streamlink [%s]"
							" [ticket %s] with PN=[%d](discard pnMax[%d] pnMin[%d]) BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
							strStreamlinkID.c_str(),ticketID.name.c_str(),
							pn, it->second._pnMax,it->second._pnMin, bandwidth,
							it->second._totalBandWidth,it->second._usedBandwidth,
							it->second._backupPN.size() , it->second._backupPN.size() - (int)it->second._availablePN.size());
					}
				}
				else if (pn<=it->second._pnMax &&pn>=it->second._pnMin) 
				{
					it->second._availablePN.push_back(pn);
					glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() streamlink [%s]"
						" [ticket %s] with PN=[%d] BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
						strStreamlinkID.c_str(),ticketID.name.c_str(),
						pn,bandwidth,
						it->second._totalBandWidth,it->second._usedBandwidth,
						it->second._totalPNCount,it->second._totalPNCount-(int)it->second._availablePN.size());

				}	
				else
				{
					glog(ZQ::common::Log::L_INFO,CLOGFMT(NgodSop_DVBC,"doFreeResource() streamlink [%s]"
						" [ticket %s] with PN=[%d](discard pnMax[%d] pnMin[%d]) BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
						strStreamlinkID.c_str(),ticketID.name.c_str(),
						pn, it->second._pnMax,it->second._pnMin, bandwidth,
						it->second._totalBandWidth,it->second._usedBandwidth,
						it->second._totalPNCount,it->second._totalPNCount-(int)it->second._availablePN.size());

				}	
			}			
			_dvbcResourceDataMap.erase(itAlloc);
		}
	}
	catch( const TianShanIce::BaseException& ex )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"doFreeResource() catch a tianshan exception:%s"),ex.message.c_str());
		return;
	}
	catch( const Ice::Exception& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"doFreeResource() catch a ice exception:%s"),e.ice_name().c_str());
		return;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSop_DVBC,"doFreeResource() catch a unknown exception"));
		return;
	}	
}
void IpEdgePHO_DVBC::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{

}


}}//namespace ZQTianShan::AccreditedPath
