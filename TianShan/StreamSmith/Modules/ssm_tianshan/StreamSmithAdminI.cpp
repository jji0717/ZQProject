
#include "public.h"
#include <Log.h>
#include <Guid.h>
#include <TianShanDefines.h>

#include "StreamSmithAdminI.h"
#include <TianShanIceHelper.h>

#ifdef _DEBUG
	#include <adebugmem.h>
#endif

#define SSADMINI(x)		"[SuperPlugin]Stream[%s] UserSess[%s]"##x,guid.c_str(),userSessId.c_str()

namespace ZQ
{
namespace StreamSmith
{
using namespace ZQ::common;

TianShanIce::Variant		GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
											   const TianShanIce::SRM::ResourceType& type,
											   const std::string& strkey)
{
	TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
	if(itResMap==rcMap.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() type %d not found",type);
		
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1001,szBuf );
	}
	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if(it==itResMap->second.resourceData.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() value with key=%s not found",strkey.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1002,szBuf );
	}
	return it->second;	
}


StreamSmithI::StreamSmithI(IPlaylistManager* pMan,ZQADAPTER_DECLTYPE adapter,
						   char* srvNetID,const std::vector<int>& edgeCards):
					_pAdpter(adapter)
{
	_plManager=pMan;
	if(!srvNetID)
	{
		char szBuf[1024];
		ZeroMemory(szBuf,sizeof(szBuf));
		gethostname (szBuf,sizeof(szBuf));
		_srvNetID=szBuf;
	}
	else if( strlen(srvNetID)<=0)
	{
		_srvNetID="";
	}
	else
	{
		_srvNetID=srvNetID;
	}
	_edgeCards=edgeCards;
}
StreamSmithI::~StreamSmithI()
{
}

::TianShanIce::Streamer::PlaylistIDs StreamSmithI::listPlaylists(const ::Ice::Current&)
{
	SUPERLOG(Log::L_DEBUG,SPLUGIN("List playlist start"));	
	::TianShanIce::Streamer::PlaylistIDs IDs;
#ifdef _ICE_INTERFACE_SUPPORT
	_plManager->listPlaylistGuid(IDs);
#endif
	SUPERLOG(Log::L_DEBUG,SPLUGIN("List playlist end"));
	return IDs;
}
::TianShanIce::Streamer::StreamPrx StreamSmithI::openStream(const ::std::string& guid, const ::Ice::Current& /* = ::Ice::Current */)
{
	::Ice::Identity id;
	id.category	= PL_CATALOG;
	id.name		= guid;
	::TianShanIce::Streamer::StreamPrx prx;
	try
	{
		prx = ::TianShanIce::Streamer::StreamPrx::checkedCast(_pAdpter->createProxy(id));
		if(prx)
			prx->lastError();
	}
	catch ( const Ice::ObjectNotExistException&) 
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1012,"stream[%s] not exist",guid.c_str() );
	}
	catch( const Ice::Exception& ex )
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("StreamSmith",1012,"failed to open stream[%s] due to [%s]",
			guid.c_str(), ex.ice_name().c_str() );
	}
	return prx;
}
::TianShanIce::Streamer::PlaylistPrx StreamSmithI::openPlaylist(const ::std::string& guid,
																const ::TianShanIce::Streamer::SpigotBoards& edgeCards,
																bool bCreate,const ::Ice::Current& c )
{
	std::string userSessId;

	ZQTianShan::Util::getPropertyDataWithDefault( c.ctx , "CLIENTSESSIONID" , "" , userSessId );

	SUPERLOG(Log::L_DEBUG,SSADMINI("Open playlist with bCreate=%d") , bCreate);
	for(int i=0;i<(int)edgeCards.size();i++)
		SUPERLOG(Log::L_DEBUG,SSADMINI("Open playlist   with spigot board id=%d"), edgeCards[i]);

	ZQ::common::Guid uid(guid.c_str());
	IPlaylistEx* pl = NULL;
	if (!bCreate)
	{
		pl =_plManager->find(guid);
		SUPERLOG(Log::L_DEBUG,SSADMINI("Open playlist after find in playlist manager") );
	}
	
	if(!pl)
	{
		if(!bCreate)
		{
			SUPERLOG(Log::L_DEBUG,SSADMINI("Can't open the playlist"));
			return NULL;
		}
		else
		{	
			DWORD dwStartTime  = GetTickCount();
			std::vector<int> BoardIDs;
			for(int i=0;i<(int)edgeCards.size();i++)
			{
				BoardIDs.push_back(edgeCards[i]);
			}
			IPlaylistEx	*pl=_plManager->CreatePlaylist( guid, BoardIDs, userSessId );			
			
			if(!pl)
			{
				SUPERLOG(Log::L_ERROR,SSADMINI("Create playlist fail,maybe no suitable stream port"));
				return NULL;//TianShanIce::Streamer::PlaylistExPrx();
			}
			else
			{				
				::PlaylistAttr attr;
				attr.playlistState=IPlaylist::PLAYLIST_SETUP;
				attr.Guid=guid;
				attr.currentCtrlNum=0;
#ifdef _ICE_INTERFACE_SUPPORT
				DWORD dwAddDbStart = GetTickCount();
				//glog(Log::L_DEBUG,"Add new playlist into fail over with guid=%s",guid.c_str());
				_plManager->addNewPlaylistIntoFailOver(guid,attr);
				SUPERLOG(Log::L_DEBUG,SSADMINI("Add playlist instance into DB use time count [%u]"),
									GetTickCount()-dwAddDbStart);
#endif
				::Ice::Identity id;
				id.category=PL_CATALOG;
				id.name=guid;					
				TianShanIce::Streamer::InternalPlaylistExPrx prx=TianShanIce::Streamer::InternalPlaylistExPrx::uncheckedCast(_pAdpter->createProxy(id));
				//if(prx)
				//	prx->lastError();
				SUPERLOG(Log::L_DEBUG,SSADMINI("Create playlist OK with time [%u]"),
									GetTickCount()-dwStartTime);
				return prx;
				/*std::string strTemp=_pAdpter->getCommunicator()->proxyToString(prx);*/		
			}
		}
	}
	else
	{
		::Ice::Identity id;
		id.category=PL_CATALOG;
		id.name=guid;
		TianShanIce::Streamer::InternalPlaylistExPrx prx;
		try
		{
			prx=TianShanIce::Streamer::InternalPlaylistExPrx::checkedCast(_pAdpter->createProxy(id));
			if(prx)
				prx->lastError();
		}
		catch (...) 
		{			
		}
		return prx;
	}

}
::TianShanIce::Streamer::StreamPrx StreamSmithI::createStreamByResource(const ::TianShanIce::SRM::ResourceMap& res,
																		const TianShanIce::Properties& props,
																		const ::Ice::Current& c)
{
	std::string guid;
	std::string userSessId; //client user sessionId

	//Step 1.check if there is a specified streamer or not
	try
	{
	
	bool bHasStreamer =false;
	TianShanIce::Variant varStreamer ; 
	try
	{
		varStreamer = GetResourceMapData(res,TianShanIce::SRM::rtStreamer,"NetworkId");
		if (varStreamer.type == TianShanIce::vtStrings && varStreamer.strs.size() >= 0) 
		{
			bHasStreamer = true;
		}
	}
	catch (const TianShanIce::InvalidParameter&) 
	{//No streamer is found		
	}
	
	::TianShanIce::Streamer::StreamPrx retStreamPrx;
	if (bHasStreamer) 
	{
		retStreamPrx = createStreamByStreamer(res,varStreamer,c);
	}
	else
	{
		retStreamPrx = createStreamByServiceGroup(res,c);
	}
	SUPERLOG(Log::L_DEBUG, SSADMINI("there are [%d] sessions currently"), listPlaylists(c).size());
	return retStreamPrx;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		SUPERLOG(Log::L_DEBUG,SSADMINI("Catch TianShan Exception [%s] when createStream"),ex.message.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1011,"Catch TianShan Exception [%s] when createStream",ex.message.c_str());
	}
	catch (const Ice::Exception& ex) 
	{
		SUPERLOG(Log::L_DEBUG,SSADMINI("Catch Ice Exception [%s] when createStream"),ex.ice_name().c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1012,"Catch Ice Exception [%s] when createStream",ex.ice_name().c_str());
	}
	catch (...) 
	{
		SUPERLOG(Log::L_DEBUG,SSADMINI("Catch unknown exception when createStream"));
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1013,"Catch unknown exception when create Stream");
	}
	SUPERLOG(Log::L_DEBUG,SSADMINI("there are [%d] sessions currently"), listPlaylists(c).size());
	return NULL;
}
TianShanIce::Streamer::StreamPrx StreamSmithI::createStreamByStreamer(const TianShanIce::SRM::ResourceMap& res,
																	  TianShanIce::Variant& varStreamer,
																	  const ::Ice::Current& c)
{
	ZQ::common::Guid uid;
	std::string userSessId;

	//CLIENTSESSIONID
	ZQTianShan::Util::getPropertyDataWithDefault( c.ctx , "CLIENTSESSIONID" , "" , userSessId );
	uid.create();
	char szGuid[128];
	ZeroMemory(szGuid,sizeof(szGuid));
		uid.toString(szGuid,sizeof(szGuid)-1);
	std::string guid = szGuid;
	SUPERLOG(Log::L_DEBUG, SSADMINI("entering createStreamByStreamer"));
	DWORD dwCreateStart = GetTickCount();
	//get resource,bandwidth destIP destPort destMac serverPort boarddNumber
	TianShanIce::Variant varBandwidth;
	TianShanIce::Variant varDestIP;
	TianShanIce::Variant varDestPort;
	TianShanIce::Variant varDestMac;
	TianShanIce::Variant varServerPort;

	std::string			strDestMac;
	std::string			strDestIP;	
	long				lBandwidth = 0;
	int					iServerPort = 0 ;
	int					iDestPort = 0;
	int					iBoardNumber = -1;//all spigots
	std::string			strNetId;

	TianShanIce::Variant	valPokeHoleSessID;
	bool					bEnableNATPenetrating =  false;
	std::string				strPokeHoldeSessionID;
	
	{
		/*
		if (sscanf(spiGotStr.c_str(),"Spigot%02d",&iSpigot)!=1)
		{
		SUPERLOG(Log::L_DEBUG,SSADMINI("Can't find spigotid in  %s,maybe it is old format with ticket[%s]"),
		spiGotStr.c_str(),strTicket.c_str());
		sscanf(spiGotStr.c_str(),"BoardNumber%d",&iSpigot);
		}
		*/
		if (varStreamer.type != TianShanIce::vtStrings && varStreamer.strs.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1020,"no streamer netId is found");
		}
		strNetId =varStreamer.strs[0];
		std::string::size_type iPos;
		if ( (iPos=strNetId.rfind("/") )!=std::string::npos )
		{
			std::string spiGotStr=strNetId.substr(iPos+1);			
			if ( sscanf(spiGotStr.c_str(),"Spigot%02d",&iBoardNumber) != 1 )
			{
				SUPERLOG(Log::L_DEBUG,SSADMINI("can't find spigotId in[%s],try \'BoardNumber\'"),spiGotStr.c_str() );
				sscanf(spiGotStr.c_str(),"BoardNumber%d",&iBoardNumber);
			}
		}
		else
		{
			SUPERLOG(Log::L_WARNING,SSADMINI("invalid streamer netId format [%s]"),strNetId.c_str());
			iPos=-1;						
			iBoardNumber = -1;
			//ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0,"Invalid Streamer info");
		}
		SUPERLOG(Log::L_INFO,SSADMINI("got boardNumber[%d] from netID[%s]"),iBoardNumber,strNetId.c_str());
	}
	
	try
	{
		varBandwidth	= GetResourceMapData(res,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
		varDestIP		= GetResourceMapData(res,TianShanIce::SRM::rtEthernetInterface,"destIP");
		varDestPort		= GetResourceMapData(res,TianShanIce::SRM::rtEthernetInterface,"destPort");

		if (varBandwidth.type == TianShanIce::vtLongs && varBandwidth.lints.size() > 0) 
		{
			lBandwidth = (long)varBandwidth.lints[0];
			SUPERLOG(Log::L_INFO,SSADMINI("Get BandWidth [%d]"),lBandwidth);
		}
		else
		{
			SUPERLOG(Log::L_INFO,SSADMINI("No bandwidth is found"));
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1021,"No bandwidth is found");
		}

		if (varDestIP.type == TianShanIce::vtStrings && varDestIP.strs.size() > 0) 
		{
			strDestIP = varDestIP.strs[0];
			SUPERLOG(Log::L_INFO,SSADMINI("get destIP [%s]"),strDestIP.c_str());
		}
		else
		{
			SUPERLOG(Log::L_INFO,SSADMINI("No destIP is found"));
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1022,"No destIP is found");
		}

		if (varDestPort.type == TianShanIce::vtInts && varDestPort.ints.size() > 0) 
		{
			iDestPort = varDestPort.ints[0];
			SUPERLOG(Log::L_INFO,SSADMINI("get destPort [%d]"),iDestPort);
		}
		else
		{
			SUPERLOG(Log::L_ERROR,SSADMINI("no destPort is found"));
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1023,"No destPort is found");
		}
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		SUPERLOG(Log::L_ERROR,SSADMINI("resource insufficient [%s]"),ex.message.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1024,"resource insufficient [%s]",ex.message.c_str());
	}
	try
	{
		varDestMac =  GetResourceMapData(res,TianShanIce::SRM::rtEthernetInterface,"destMac");
		if (varDestMac.type == TianShanIce::vtStrings && varDestMac.strs.size() > 0) 
		{
			strDestMac = varDestMac.strs[0];
			SUPERLOG(Log::L_INFO,SSADMINI("got destMac [%s]"),strDestMac.c_str());
		}
		else
		{
			if (varDestMac.type == TianShanIce::vtLongs && varDestMac.lints.size() > 0) 
			{
				Ice::Long lMac = varDestMac.lints[0];
				char szBuf[64];
				sprintf(szBuf,"%012llX",lMac);
				strDestMac = szBuf;
				SUPERLOG(Log::L_INFO,SSADMINI("got destMac [%s]"),strDestMac.c_str());
			}
			else
			{
				SUPERLOG(Log::L_INFO,SSADMINI("no destMac is found"));
			}
		}
	}
	catch (const TianShanIce::InvalidParameter&) 
	{
		strDestMac = "";
		SUPERLOG(Log::L_INFO,SSADMINI("no destMac is found"));
	}
	try
	{
		varServerPort = GetResourceMapData(res,TianShanIce::SRM::rtEthernetInterface,"srcPort");
		if (varServerPort.type == TianShanIce::vtInts && varServerPort.ints.size() > 0) 
		{
			iServerPort = varServerPort.ints[0];
			SUPERLOG(Log::L_INFO,SSADMINI("got serverPort [%d]"),iServerPort);
		}
		else
		{
			SUPERLOG(Log::L_INFO,SSADMINI("no serverPort is found"));
			iServerPort = 0;
		}
	}
	catch (const TianShanIce::InvalidParameter&) 
	{
		SUPERLOG(Log::L_INFO,SSADMINI("no serverPort is found"));
		iServerPort = 0;
	}
	//get NATPenetrating and poke hole session ID
	try
	{
		TianShanIce::Variant valNATEnable;
		valNATEnable = GetResourceMapData (res ,  TianShanIce::SRM::rtEthernetInterface,"natPenetrating");
		if ( valNATEnable.type != TianShanIce::vtInts || valNATEnable.ints.size () == 0  ) 
		{
			bEnableNATPenetrating = false;
			SUPERLOG(Log::L_INFO,SSADMINI("natPenetrating disabled"));
		}
		else
		{
			if ( valNATEnable.ints[0] != 0 )
			{
				SUPERLOG(Log::L_INFO,SSADMINI("natPenetrating enabled"));
				bEnableNATPenetrating = true;
			}
			else
			{
				bEnableNATPenetrating =  false;
				SUPERLOG(Log::L_INFO,SSADMINI("natPenetrating disabled"));
			}
		}
	}
	catch (TianShanIce::InvalidParameter&) 
	{
		bEnableNATPenetrating = false;
		SUPERLOG(Log::L_INFO,SSADMINI("natPenetrating disabled"));
	}
	if (bEnableNATPenetrating) 
	{
		valPokeHoleSessID = GetResourceMapData (res , TianShanIce::SRM::rtEthernetInterface,"pokeholeSession");
		if(valPokeHoleSessID.type != TianShanIce::vtStrings || valPokeHoleSessID.strs.size () == 0)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(SUPERLOG,"StreamSmith",1047,SSADMINI("NATPenetrating Enabled but NO Poke Hole Session ID"));
			return NULL; 
		}
		strPokeHoldeSessionID = valPokeHoleSessID.strs[0];
// 		if ( strPokeHoldeSessionID.length () != 10 ) 
// 		{
// 			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(SUPERLOG,"StreamSmith",1048,SSADMINI("NATPenetrating Enabled and get PokeHoleSessID [%d] but size [%d] is wrong "),
// 				strPokeHoldeSessionID.c_str (),strPokeHoldeSessionID.length ());
// 		}
		SUPERLOG(Log::L_INFO,SSADMINI("got poke hole sessionId [%s] "),strPokeHoldeSessionID.c_str());
	}
	
	try
	{
		
		::TianShanIce::Streamer::SpigotBoards	edgeCards ;
		edgeCards.push_back(iBoardNumber);
		
		::TianShanIce::Streamer::InternalPlaylistExPrx plPrx=
				TianShanIce::Streamer::InternalPlaylistExPrx::uncheckedCast(openPlaylist(szGuid,edgeCards,true,c));

		if(!plPrx)
		{		
			SUPERLOG(Log::L_ERROR,SSADMINI("failed to create playlist"));
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("StreamSmith",1025,"failed to create playlist");
			return NULL;
		}
		plPrx->setDestination(strDestIP,iDestPort);
		if(!strDestMac.empty())
		{
			plPrx->setDestMac(strDestMac);
		}
		plPrx->setSourceStrmPort(iServerPort);
		plPrx->setMuxRate(lBandwidth,lBandwidth,0);
		
		//get client Session ID ( maybe it is OnDemandSessionID ) if available
		{
			const Ice::Context& ctx = c.ctx;
			Ice::Context::const_iterator itCtx = ctx.find("CLIENTSESSIONID");
			if( itCtx != ctx.end() )
			{
				plPrx->attachClientSessionId(itCtx->second);
			}
			else
			{
				plPrx->attachClientSessionId( std::string());
			}
		}
		if (bEnableNATPenetrating)
		{
			TianShanIce::ValueMap valmap;
			valmap.clear ();
			valmap["PokeSessionID"] = valPokeHoleSessID;
			SUPERLOG(Log::L_INFO,SSADMINI("got poke hole session Id[%s]"),strPokeHoldeSessionID.c_str() );
			plPrx->setPLaylistData(valmap);
		}
		SUPERLOG(Log::L_DEBUG,SSADMINI("create playlist [%s] successfully, used[%u]ms"),	szGuid,GetTickCount()-dwCreateStart);
		return plPrx;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	catch (const Ice::Exception& ex)
	{
		SUPERLOG(Log::L_ERROR,SSADMINI("caught ice exception:%s"),ex.ice_name().c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("StreamSmith",1026,"ice exception when createStream:%s",ex.ice_name().c_str());
		return NULL;
	}
	catch (...) 
	{
		SUPERLOG(Log::L_ERROR,SSADMINI("caught unkown exception"));
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("StreamSmith",1027,"Unexpect error when create stream");
		return NULL;
	}
	return NULL;
}
TianShanIce::Streamer::StreamPrx StreamSmithI::createStreamByServiceGroup(const ::TianShanIce::SRM::ResourceMap& res,
																		  const ::Ice::Current& c)
{
	ZQ::common::Guid uid;
	uid.create();
	char szGuid[128];
	ZeroMemory(szGuid,sizeof(szGuid));
	uid.toString(szGuid,sizeof(szGuid)-1);
	std::string guid = szGuid;
	std::string userSessId;

	ZQTianShan::Util::getPropertyDataWithDefault( c.ctx, "CLIENTSESSIONID", "", userSessId );

	//get serviceGroup and MaxBitRate
	SUPERLOG(Log::L_DEBUG,SSADMINI("enter createStreamByResource"));
	int serviceGroupIDs;
	int	maxBitRate;
	::TianShanIce::SRM::ResourceMap sessRes=res;
	::TianShanIce::SRM::Resource& resServiceGroup=sessRes[::TianShanIce::SRM::rtServiceGroup];
	if(resServiceGroup.resourceData.find("servicegroup")==resServiceGroup.resourceData.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1031,"No servicegroup was found");
	}
	serviceGroupIDs=resServiceGroup.resourceData["servicegroup"].ints[0];
	SUPERLOG(Log::L_DEBUG,SSADMINI("Get servicegroup =%d"),serviceGroupIDs);

	::TianShanIce::SRM::Resource& resBandWdith=sessRes[::TianShanIce::SRM::rtTsDownstreamBandwidth];
	if(resBandWdith.resourceData.find("bandwidth")==resBandWdith.resourceData.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",1032,"no bandwidth was found");
	}
	maxBitRate=resBandWdith.resourceData["bandwidth"].ints[0];
	SUPERLOG(Log::L_DEBUG,SSADMINI("get bandwith =%d"),maxBitRate);

	
	//at first query resource manager to get the spigot id through the servicegroup id
	//GetSpigotIDsFromResource
	std::vector<int>	vecSpigotIDs;
	if(!_plManager->GetSpigotIDsFromResource(serviceGroupIDs,maxBitRate,vecSpigotIDs))
	{
		SUPERLOG(Log::L_ERROR,SSADMINI("Can't find the linked spigots id through serviceGroupd ID=%d"),serviceGroupIDs);
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("StreamSmith",1033,"can't find the linked spigots di through servicegroupdid %s",serviceGroupIDs);
		return NULL;
	}
	if(vecSpigotIDs.size()<=0)
	{
		SUPERLOG(Log::L_DEBUG,SSADMINI("no linked spigots,set it to default"));
		vecSpigotIDs=_edgeCards;
	}
	for(int i=0;i<(int)vecSpigotIDs.size();i++)
	{
		SUPERLOG(Log::L_DEBUG,SSADMINI("create playlist with spigot=%d"),vecSpigotIDs[i]);
	}
	::TianShanIce::Streamer::InternalPlaylistExPrx pl=::TianShanIce::Streamer::InternalPlaylistExPrx::uncheckedCast(
																				openPlaylist(szGuid,vecSpigotIDs,true)
																													);
	if(!pl)
	{		
		SUPERLOG(Log::L_ERROR,SSADMINI("Can't create playlist "));
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("StreamSmith",1034,"Create playlist failed");
		return NULL;
	}

	try
	{
		if(!pl->allocDVBCResource(serviceGroupIDs,maxBitRate))
		{
			SUPERLOG(Log::L_ERROR,SSADMINI("alloc dvbc resource fail"));
			return NULL;			 
		}
		else
		{
			SUPERLOG(Log::L_DEBUG,SSADMINI("create playlist ok with guid=%s"),szGuid);
			return pl;
		}
	}
	catch (TianShanIce::ServerError& err)
	{
		pl->destroy();
		throw err;//does this work?
	}
	
	return NULL;
}
::TianShanIce::Streamer::StreamPrx StreamSmithI::createStream(const ::TianShanIce::Transport::PathTicketPrx& ticket,
															   const ::Ice::Current& c)
{
	std::string userSessId;

	ZQTianShan::Util::getPropertyDataWithDefault( c.ctx,"CLIENTSESSIONID","",userSessId );

	DWORD dwCreateStart = GetTickCount ();
	ZQ::common::Guid uid;
	uid.create ();
	char szGuid[128];
	uid.toString (szGuid,sizeof(szGuid)-1);
	std::string	guid= szGuid;
	SUPERLOG(Log::L_DEBUG,SSADMINI("Comes a new request with"));
	TianShanIce::ValueMap	privData;
	TianShanIce::SRM::ResourceMap resMap;
	TianShanIce::Streamer::SpigotBoards	edgeCards = _edgeCards;
	std::string		strDestIP;
	int				iDestPort;
	std::string		strDestMac;
	long			lBandwidth = 4000000;
	unsigned short	usStreamServPort = 0;
	std::string		strStreamServIP;
	std::string		strPokeHoldeSessionID;
	:: TianShanIce :: Streamer :: InternalPlaylistExPrx plPrx = NULL;
	if ( ticket != NULL )
	{
		try
		{			
			DWORD dwGetStreamLinkTime = GetTickCount ();
			std::string	strTicket = _pAdpter->getCommunicator ()->proxyToString (ticket);
			SUPERLOG(Log::L_DEBUG , SSADMINI("Before get stream link through ticket[%s]"),strTicket.c_str ());
			TianShanIce::Transport::StreamLinkPrx streamLink = ticket->getStreamLink ();			
			if (!streamLink)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(SUPERLOG,"StreamSmith",1041,SSADMINI("Can't get stream link through ticket [%s]"),strTicket.c_str());return NULL;
			}
			SUPERLOG(Log::L_INFO , SSADMINI("After get stream link through ticket[%s] using time[%u]"),
									strTicket.c_str () , GetTickCount ()-dwGetStreamLinkTime);

			DWORD dwGetResourceTime = GetTickCount ();
			resMap = ticket->getResources ();
			SUPERLOG(Log::L_INFO,SSADMINI("Get ticket's resource through ticket[%s] using time[%u]"),
						strTicket.c_str (),GetTickCount ()-dwGetResourceTime);			
			TianShanIce::Variant	valBandwidth;
			TianShanIce::Variant	valDestAddr;
			TianShanIce::Variant	valDestPort;
			TianShanIce::Variant	valDestMac;
			TianShanIce::Variant	valServerIP;
			TianShanIce::Variant	valServerPort;
			TianShanIce::Variant	valNATEnable;
			TianShanIce::Variant	valPokeHoleSessID;
			bool					bEnableNATPenetrating =  false;
			try
			{
				//get bandwidth
				valBandwidth = GetResourceMapData (resMap , TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
				if (valBandwidth.type != TianShanIce::vtLongs || valBandwidth.lints.size () == 0 )
				{
					ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(SUPERLOG,"StreamSmith",1042, SSADMINI("Invalid BandWidth type should be vtLongs , or no bandwidth data with ticket [%s]"),
								strTicket.c_str ());return NULL;
				}
				lBandwidth = (long)valBandwidth.lints[0];
				SUPERLOG(Log::L_INFO,SSADMINI("Get bandwidth [%d]bps through ticket[%s]"),
						lBandwidth,strTicket.c_str ());
				//get destination IP
				valDestAddr = GetResourceMapData (resMap,TianShanIce::SRM::rtEthernetInterface,"destIP");
				if (valDestAddr.type != TianShanIce::vtStrings || valDestAddr.strs.size () == 0)
				{
					ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(SUPERLOG,"StreamSmith",1043,SSADMINI("invalid destAddress type should be vtString or no destAddress data with ticket [%s]"),
										strTicket.c_str());return NULL;
				}
				strDestIP = valDestAddr.strs[0];
				SUPERLOG(Log::L_INFO,SSADMINI("Get DestIP[%s] through ticket[%s]"),
								strDestIP.c_str (),strTicket.c_str ());

				//get destination port
				valDestPort = GetResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destPort");
				if ( valDestPort.type != TianShanIce::vtInts || valDestPort.ints.size() == 0 )
				{
					ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(SUPERLOG,"StreamSmith",1044,SSADMINI("invalid dest por type,should be vtInts,or no destPort data with ticket [%s]"),
									strTicket.c_str());return NULL;
				}
				iDestPort = valDestPort.ints[0];
				SUPERLOG(Log::L_INFO,SSADMINI("Get destPort[%d] through ticket[%s]"),iDestPort,strTicket.c_str ());
				
				//get dest mac ,and mac may not available
				try
				{
					valDestMac = GetResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destMac");
					if ( valDestMac.type != TianShanIce::vtStrings || valDestMac.strs.size () == 0 )
					{
						if ( valDestMac.type == TianShanIce::vtLongs && valDestMac.lints.size() > 0) 
						{
							Ice::Long lMac = valDestMac.lints[0];
							char szBuf[64];
							sprintf(szBuf,"%012llX",lMac);
							strDestMac = szBuf;
							SUPERLOG(Log::L_INFO,SSADMINI("got destMac [%s]"),strDestMac.c_str());
						}
						else
						{					
						
							ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(SUPERLOG,"StreamSmith",1045,SSADMINI("invalid dest mac type,should be vtStrings,or no destmac data with ticket [%s]"),
									strTicket.c_str());return NULL;
						}
					}
					else
					{
						strDestMac = valDestMac.strs[0];
						SUPERLOG(Log::L_INFO,SSADMINI("Get dest mac [%s] through ticket[%s]"), strDestMac.c_str (),strTicket.c_str ());
					}
				}
				catch (TianShanIce::InvalidParameter&)
				{
					strDestMac = "";
					SUPERLOG(Log::L_DEBUG,SSADMINI("no dest mac information in ticket[%s]"),strTicket.c_str());
				}
				//get source port
				try
				{
					valServerPort = GetResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"srcPort");
					if ( valServerPort.type != TianShanIce::vtInts || valServerPort.ints.size () == 0 )
					{
						ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(SUPERLOG,"StreamSmith",1046,SSADMINI("invalid server port type or no data with ticket [%s]"),
							strTicket.c_str ());return NULL;
					}
					usStreamServPort = valServerPort.ints[0];
					SUPERLOG(Log::L_INFO,SSADMINI("Get server port[%us] through ticket[%s]"),
							usStreamServPort,strTicket.c_str ());
				}
				catch (TianShanIce::InvalidParameter&)
				{
					usStreamServPort = 0;
					SUPERLOG(Log::L_DEBUG,SSADMINI("no server port information in ticket[%s]"),strTicket.c_str());
				}
				//get NATPenetrating and poke hole session ID
				try
				{
					valNATEnable = GetResourceMapData (resMap ,  TianShanIce::SRM::rtEthernetInterface,"natPenetrating");
					if ( valNATEnable.type != TianShanIce::vtStrings || valNATEnable.ints.size () == 0  ) 
					{
						bEnableNATPenetrating = false;
						SUPERLOG(Log::L_INFO,SSADMINI("NO natPenetrating is enabled"));
					}
					else
					{
						if ( valNATEnable.ints[0] != 0 )
						{
							SUPERLOG(Log::L_INFO,SSADMINI("natPenetrating Enabled"));
							bEnableNATPenetrating = true;
						}
						else
						{
							bEnableNATPenetrating =  false;
							SUPERLOG(Log::L_INFO,SSADMINI("NO natPenetrating is enabled"));
						}
					}
				}
				catch (TianShanIce::InvalidParameter&) 
				{
					bEnableNATPenetrating = false;
					SUPERLOG(Log::L_INFO,SSADMINI("NO natPenetrating is enabled"));
				}
				if (bEnableNATPenetrating) 
				{
					valPokeHoleSessID = GetResourceMapData (resMap , TianShanIce::SRM::rtEthernetInterface,"pokeholeSession");
					if(valPokeHoleSessID.type != TianShanIce::vtStrings || valPokeHoleSessID.strs.size () == 0)
					{
						ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(SUPERLOG,"StreamSmith",1047,SSADMINI("NATPenetrating Enabled but NO Poke Hole Session ID"));
						return NULL; 
					}
					strPokeHoldeSessionID = valPokeHoleSessID.strs[0];
// 					if ( strPokeHoldeSessionID.length () != 10 ) 
// 					{
// 						ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(SUPERLOG,"StreamSmith",1048,SSADMINI("NATPenetrating Enabled and get PokeHoleSessID [%d] but size [%d] is wrong "),
// 								strPokeHoldeSessionID.c_str (),strPokeHoldeSessionID.length ());
// 					}
					SUPERLOG(Log::L_INFO,SSADMINI("get PokeHoleSessionID [%s] "),strPokeHoldeSessionID.c_str());
				}

#pragma message(__MSGLOC__"TODO:DO I need Source stream IP ? ")
				//get source IP

				//get spigot
				DWORD dwGetStreamInfoTime = GetTickCount ();
				TianShanIce::Transport::Streamer strmInfo = streamLink->getStreamerInfo ();
				SUPERLOG(Log::L_DEBUG,SSADMINI("Get streamInfo using time [%d] "),GetTickCount () - dwGetStreamInfoTime);
				std::string&	strStrmerNetId = strmInfo.netId;
				std::string::size_type iPos;
				if ( (iPos=strStrmerNetId.rfind("/") )==std::string::npos )
				{
					SUPERLOG(Log::L_ERROR,SSADMINI("Invalid streamer netID [%s]  format in ticket[%s]"),
								strStrmerNetId.c_str (),strTicket.c_str());
					iPos=-1;
				}
				std::string spiGotStr=strStrmerNetId.substr(iPos+1);
				int iSpigot;
				if (sscanf(spiGotStr.c_str(),"Spigot%02d",&iSpigot)!=1)
				{
					SUPERLOG(Log::L_DEBUG,SSADMINI("Can't find spigotid in  %s,maybe it is old format with ticket[%s]"),
						spiGotStr.c_str(),strTicket.c_str());
					sscanf(spiGotStr.c_str(),"BoardNumber%d",&iSpigot);
				}
				//sscanf(spiGotStr.c_str(),"BoardNumber%d",&iSpigot);
				//Spigot%02d					
				
				SUPERLOG(Log::L_DEBUG,SSADMINI("find spigot [%s] and convert it to integer [%d] in ticket[%s]"),
					spiGotStr.c_str(),iSpigot,strTicket.c_str());
				edgeCards.clear();
				edgeCards.push_back(iSpigot);	

				
			}			
			catch(TianShanIce::InvalidParameter& e)
			{
				SUPERLOG(Log::L_ERROR,SSADMINI("invalidParameter exception was caught:%s with ticket[%s]"),
					e.message.c_str(),strTicket.c_str());
				e.ice_throw();
				return NULL;
			}
			catch (TianShanIce::BaseException& e)
			{
				SUPERLOG(Log::L_ERROR,SSADMINI("TianShanIce::BaseException was caught:%s with ticket[%s]"),
					e.message.c_str(),strTicket.c_str());
				e.ice_throw();
				return NULL;
			}
			catch (Ice::Exception& e)
			{
				SUPERLOG(Log::L_ERROR,SSADMINI("Ice::Exception was caught:%s with ticket[%s]"),
					e.ice_name().c_str(),strTicket.c_str());
				e.ice_throw();
				return NULL;
			}
			SUPERLOG(Log::L_DEBUG,SSADMINI("create playlist with ticket [%s]"),strTicket.c_str());
			plPrx = ::TianShanIce::Streamer::InternalPlaylistExPrx::uncheckedCast(
														openPlaylist(guid,edgeCards,true,c)
																				  );
			if ( !plPrx )
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(SUPERLOG,"StreamSmith",1049,SSADMINI("Can't create new stream with ticket [%s]"),
							strTicket.c_str());return NULL;
			}
			plPrx->setPathTicketProxy (ticket,plPrx);
			plPrx->setDestination (strDestIP,iDestPort);
			if (!strDestMac.empty ())plPrx->setDestMac (strDestMac);
			plPrx->setSourceStrmPort (usStreamServPort);
			plPrx->setMuxRate (lBandwidth,lBandwidth,0);
			if (bEnableNATPenetrating)
			{
				TianShanIce::ValueMap valmap;
				valmap.clear ();
				valmap["PokeSessionID"] = valPokeHoleSessID;
				plPrx->setPLaylistData(valmap);
			}
			
			SUPERLOG(Log::L_DEBUG,SSADMINI("create playlist success with ticket [%s] and time count=[%u]"),
						strTicket.c_str(),GetTickCount()-dwCreateStart);
			return plPrx;
		}
		catch (TianShanIce::BaseException& ex) 
		{
			ex.ice_throw();
		}
		catch (Ice::Exception& ex)
		{			
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(SUPERLOG,"StreamSmith",2041,SSADMINI("ice exception when createStream:%s"),ex.ice_name().c_str());
			return NULL;
		}
		catch (...) 
		{			
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(SUPERLOG,"StreamSmith",2042,SSADMINI("Unexpect error when create stream"));
			return NULL;
		}
		return NULL;
	}	
	else
	{
		//Just for test use
		const Ice::Context& ctx = c.ctx;
		plPrx=::TianShanIce::Streamer::InternalPlaylistExPrx::uncheckedCast(
																			openPlaylist(guid,edgeCards,true)
																			);	
		if(!plPrx)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("StreamSmith",2043,plPrx->lastError().c_str());
			return  NULL;
		}	
		plPrx->lastError();
		plPrx->setPathTicketProxy(NULL,plPrx);
		Ice::Context::const_iterator itCtx = ctx.end ();
		itCtx = ctx.find ("SrcPort");
		if ( itCtx != ctx.end() )
		{
			unsigned short usSrcPort = (unsigned short) atoi(itCtx->second.c_str());
			SUPERLOG(Log::L_DEBUG,SSADMINI("Get SrcPort [%d]"),usSrcPort );						
			plPrx->setSourceStrmPort (usSrcPort);
		}
		return plPrx;
	}
}

::std::string StreamSmithI::getAdminUri(const ::Ice::Current& /* = ::Ice::Current( */)
{
	return "what uri should I return";
}
::TianShanIce::State StreamSmithI::getState(const ::Ice::Current& /* = ::Ice::Current( */)
{
	RLock sync(*this);
	return TianShanIce::stInService;
}
::TianShanIce::Streamer::StreamerDescriptors StreamSmithI::listStreamers(const ::Ice::Current& ic)
{
	RLock sync(*this);
	
	SUPERLOG(Log::L_DEBUG,SPLUGIN("Enter ListStreamer"));
	::TianShanIce::Streamer::StreamerDescriptors SDs;
	std::vector<int> IDs;
	_plManager->listStreamers(IDs);
	::TianShanIce::Streamer::StreamerDescriptor sd;
	char	szBuf[512];
	for(int i=0;i<(int)IDs.size();i++)
	{
		//sd.deviceId=itoa(IDs[i],szBuf,10);
		//sprintf(szBuf,"BoardNumber%02d",IDs[i]);
		sprintf(szBuf,"Spigot%02d",IDs[i]);
		sd.deviceId=szBuf;
		sd.type="IPEdge";
		SDs.push_back(sd);
	}	
	SUPERLOG(Log::L_DEBUG,SPLUGIN("Leave ListStreamer"));
	return SDs;
}

::std::string StreamSmithI::getNetId(const ::Ice::Current& ic) const
{
	RLock sync(*this);

	if ( _srvNetID.empty() ) 
	{
		char szBuf[1024];
		memset( szBuf , 0, sizeof(szBuf) );
		gethostname(szBuf,sizeof(szBuf));
		SUPERLOG(Log::L_DEBUG,SPLUGIN("Service netId is empty ,return as local machine name [%s] "),szBuf);	
		return std::string(szBuf);
	}
	else
	{	
		return _srvNetID;
	}
}

std::string StreamSmithI::ShowMemory(const ::Ice::Current& /* = ::Ice::Current( */)
{
#ifdef _DEBUG
//	ShowMemoryDifference();
#endif
	int plSize=_plManager->getIceEvitorPLaylistSize();
	int itemSize=_plManager->getIceEvitorPLaylistItemSize();
	int alivePlCount=_plManager->getPlaylistCount();
	int suicidePlCount=_plManager->getSuicidePlCount();
	char szBuf[2048];
	sprintf(szBuf,"current EvictorPlaylistCount is %d and EvictorItem count is %d\n"
		"current alive playlistcount is %d and suicide playlist count is %d",
		plSize,itemSize,alivePlCount,suicidePlCount);
	return std::string(szBuf);
	
}
void StreamSmithI::queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& callback, 
									   const ::std::string& category, 
									   const ::std::string& groupId,
									   bool bLocalOnly,
									   const ::Ice::Current&/* = ::Ice::Current()*/) 
{
	TianShanIce::Replicas res;	
	_plManager->getReplicaInfo( category , groupId , bLocalOnly , res   );
	callback->ice_response(res);
	return;

// 	//now I am not care about the localOnly parameter
// 	TianShanIce::Replica rep;
// 	SpigotsInfo::const_iterator it = spInfos.begin();
// 	for( ; it != spInfos.end() ; it ++ )
// 	{
// 		rep.category	= category;
// 		rep.groupId		= groupId;
// 		rep.disabled	= static_cast<bool>( it->bDisable );
// 		rep.maxPrioritySeenInGroup = 0;
// 		rep.priority	= 0;
// 
// 		char szReplicaId[128];
// 		sprintf( szReplicaId , "Spigot%02d" ,it->spigotIndex);
// 		rep.replicaId	= szReplicaId;
// 
// 		rep.stampKnew	= it->stampFirstUp;
// 		rep.stampUpdated= it->stampUpdate;
// 
// 		Ice::Identity id;
// 		id.category		= "";
// 		id.name			= "StreamSmith";
// 		rep.obj			= _pAdpter->createProxy(id);
// 
// 		res.push_back(rep);
// 
// 	}
}

void StreamSmithI::createStreamBatch_async(const ::TianShanIce::Streamer::AMD_StreamSmithAdmin_createStreamBatchPtr& callback, 
										   const ::TianShanIce::Streamer::StreamBatchRequest& req, 
										   const ::Ice::Current& c)
{
	SUPERLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamSmithI, "createStreamBatch_async() enter"));
	::TianShanIce::Streamer::StreamBatchResponse res;

	for (::std::vector< ::TianShanIce::Streamer::StreamDuplicateReq>::const_iterator it = req.begin();
		it != req.end(); it++)
	{
		try
		{
			::TianShanIce::Streamer::StreamDuplicateResp dupRes;
			::TianShanIce::Streamer::StreamPrx stream = createStreamByResource(it->resources, it->properties, c);
			if (NULL != stream)
			{
				::TianShanIce::Streamer::PlaylistPrx playlist = ::TianShanIce::Streamer::PlaylistPrx::uncheckedCast(stream);
				playlist = ::TianShanIce::Streamer::PlaylistPrx::uncheckedCast(playlist->ice_collocationOptimized(false));

				int usrCtrlNum = 1;
				for (::std::vector< ::TianShanIce::Streamer::PlaylistItemSetupInfo>::const_iterator playIter = it->items.begin();
					playIter != it->items.end(); playIter++)
				{
					playlist->pushBack(usrCtrlNum++, *playIter);
				}

				::TianShanIce::Streamer::StreamInfo streamInfo;
				::TianShanIce::StrValues expectValues;	
				expectValues.push_back("CURRENTPOS");
				expectValues.push_back("TOTALPOS");
				expectValues.push_back("SPEED");
				expectValues.push_back("STATE");
				expectValues.push_back("USERCTRLNUM");

				switch (it->state)
				{
				case ::TianShanIce::Streamer::StreamState::stsPause:

					streamInfo = playlist->playEx(it->scale, it->timeoffset, 1, expectValues);
					streamInfo = playlist->pauseEx(expectValues);
					break;
				case ::TianShanIce::Streamer::StreamState::stsStreaming:
					streamInfo = playlist->playEx(it->scale, it->timeoffset, 1, expectValues);
					break;
				default:
					streamInfo.state = it->state;

					char szBuf[128];
					sprintf( szBuf , "%u" , 0);
					streamInfo.props["CURRENTPOS"]	=	szBuf;

					sprintf( szBuf , "%u" , 0);
					streamInfo.props["TOTALPOS"]	=	szBuf;

					sprintf( szBuf , "%u" , 0);
					streamInfo.props["SPEED"]	=	szBuf;

					sprintf( szBuf , "%u" , 0);
					streamInfo.props["USERCTRLNUM"]	=	szBuf;

					break;
				}

				dupRes.reqTag			=	it->reqTag;
				dupRes.streamSessionId  =	stream->getIdent().name;
				dupRes.state			=	streamInfo.state;
				dupRes.timeoffset		=	::ZQTianShan::Util::getStreamTimeOffset(streamInfo);
				dupRes.scale			=	::ZQTianShan::Util::getSpeedFromStreamInfo(streamInfo);
				dupRes.streamSession	=	playlist;
			}
			else	//stream == NULL
			{
				dupRes.reqTag			=	it->reqTag;
				dupRes.streamSessionId  =	"";
			}

			res.push_back(dupRes);		
		}
		catch (const ::TianShanIce::BaseException& e)
		{
			//e.ice_throw();
			SUPERLOG(Log::L_ERROR,"TianShanIce exception when createStreamBatch: %s",e.message.c_str());	
			e.ice_throw();
			//ZQTianShan::_IceThrow<TianShanIce::ServerError>(SUPERLOG,"StreamSmith",2041,SSADMINI("TianShanIce exception when createStreamBatch:%s"),e.ice_name().c_str());
		}
		catch (const ::Ice::Exception& e)
		{
			SUPERLOG(Log::L_ERROR,"Ice exception when createStreamBatch: %s ",e.ice_name().c_str());		
			e.ice_throw();
		}
		catch(...)
		{
			SUPERLOG(Log::L_ERROR,"Unknown exception when createStreamBatch ");				
		}
	}
	
	SUPERLOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamSmithI, "createStreamBatch_async() Receive number: %d, Response number: %d"), req.size(), res.size());
	callback->ice_response(res);	
	return;
}

}}//namespace
