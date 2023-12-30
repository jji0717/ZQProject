
#include <ZQ_common_conf.h>
#include <algorithm>
#include <vector>
#include "Playlist.h"
#include <Log.h>
#include <Guid.h>
#include "StreamSmithSite.h"
#include <TianShanIceHelper.h>

#include "vvxParser/VstrmProc.h"
#include "VV2Parser.h"

#ifndef _RTSP_PROXY
	#include <StreamSmithConfig.h>
#endif

#ifdef _DEBUG
	#include "adebugmem.h"
#endif


#define DB_PLAYLIST_PROXY_STRING	"playlistProxyString"
#define DB_TICKET_PROXY_STRING		"ticketProxyString"

#define DB_VSTRM_BANDWIDTH_TICKET_EDGE	"VstrmEdgeTicket"
#define DB_VSTRM_BANDWIDTH_TICKET_FILE	"VstrmFileTicket"


// 30 sec max timeout for playlists' timer, don't let the timer loop sleep too much
#define TIMER_MAX_TIMEOUT        (10000) 

#define	LOGFORMAT(className,x)		"["##className"]"##x

using namespace ZQ::common;
namespace ZQ{
namespace StreamSmith {
		

// -----------------------------
// class TimerRequest
// -----------------------------

class TimerRequest : public ZQ::common::ThreadRequest
{
public:

	TimerRequest(ZQ::common::Guid& uid,PlaylistManager& plMan, ZQ::common::NativeThreadPool& pool)
		: _guid(uid),ThreadRequest(pool),_plMan(plMan) {}

protected:
	bool init(void)
	{
		//glog(Log::L_DEBUG,"Create Timer request %p",this);
		return true;
	}
	virtual int run(void)
	{
		Playlist* p=_plMan.find(_guid);
		if(p!=NULL)
		{
			p->OnTimer();
		}
		return 0;
	}

	virtual void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}

	ZQ::common::Guid	_guid;
	PlaylistManager&	_plMan;
};



//////////////////////////////////////////////////////////////////////////

#ifdef _ICE_INTERFACE_SUPPORT
void PlaylistManager::UpdatePlaylistFailStore(Playlist* pList,bool bPlaylistAttr,bool bItemAttr)
{
	if(!pList)
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","enter UpdatePlaylistFailStore but playlist pointer is NULL"));
		return ;
	}
	m_pFailStoreThread->UpdateFailStore(pList,bPlaylistAttr,bItemAttr);
}

PlaylistFailStoreThread::PlaylistFailStoreThread(PlaylistInfoStore& plinfoStore):m_PlayListStore(plinfoStore)
{
	m_hUpdatePlaylistFailStoreEvent	=	CreateEvent(NULL,FALSE,FALSE,NULL);
	m_bQuit	=	false;	
}
PlaylistFailStoreThread::~PlaylistFailStoreThread()
{
	CloseHandle( m_hUpdatePlaylistFailStoreEvent );
}
void PlaylistFailStoreThread::Stop()
{
	m_bQuit	=	true;
}
void PlaylistFailStoreThread::AddClearPlaylistInfo(const std::string& uid,bool bplAttr,bool bpliAttr)
{
	FailStoreInfo info;
	info.uid				= uid;
	info.bPlAttr			= bplAttr;
	info.bPliAttr			= bpliAttr;
	{
		{
			ZQ::common::MutexGuard gd(m_failClearMutex,__MSGLOC__);
			m_PlaylistClearList.push_back(info);
		}
		{
			ZQ::common::MutexGuard gd(m_failStoreMutex,__MSGLOC__);
			FailStoreMap::iterator it=m_PlaylistFailStoreMap.find(uid);
			if(it!=m_PlaylistFailStoreMap.end())
				m_PlaylistFailStoreMap.erase(it);
		}		
		SetEvent(m_hUpdatePlaylistFailStoreEvent);
	}
}
void PlaylistFailStoreThread::UpdateFailStore(Playlist* pList,bool bplAttr,bool bpliAttr)
{
	char szBuf[128];
	
	FailStoreInfo info;
	info.pList				= pList;
	info.bPlAttr			= bplAttr;
	info.bPliAttr			= bpliAttr;

	ZeroMemory(szBuf,sizeof(szBuf));
	pList->_guid.toString(szBuf,sizeof(szBuf));
	info.uid				= szBuf;
	
	if(bplAttr)
	{
		ChangePlaylistToAttr(pList,info.plAttr);
	}
	if(bpliAttr)
	{
		ChangeItemToAttr(pList,info.plivecAttr);
	}
	{
		ZQ::common::MutexGuard gd(m_failStoreMutex,__MSGLOC__);
		//m_PlaylistFailStoreList.push_back(info);		
		ZeroMemory(szBuf,sizeof(szBuf));
		pList->_guid.toString(szBuf,sizeof(szBuf));
		FailStoreMap::iterator it;
		std::string	strGuid	=	szBuf;			

		if((it=m_PlaylistFailStoreMap.find(strGuid))!=m_PlaylistFailStoreMap.end())
		{
			it->second	=	info;
		}
		else
		{
			m_PlaylistFailStoreMap.insert(std::make_pair<std::string,FailStoreInfo>(strGuid,info));
		}
		SetEvent( m_hUpdatePlaylistFailStoreEvent );
	}
}
int PlaylistFailStoreThread::run()
{
	glog(Log::L_DEBUG,"int PlaylistFailStoreThread::run()##Entering");
	while (!m_bQuit) 
	{
		DWORD dwResult=WaitForSingleObject(m_hUpdatePlaylistFailStoreEvent,500);
		switch(dwResult)
		{
		case WAIT_OBJECT_0:
			{
				while(m_PlaylistFailStoreMap.size()>0)
				{
					FailStoreInfo	info;
					{
						ZQ::common::MutexGuard gd(m_failStoreMutex,__MSGLOC__);
						FailStoreMap::iterator it=m_PlaylistFailStoreMap.begin();
						if( it == m_PlaylistFailStoreMap.end() )
							break;
						info=it->second;
						m_PlaylistFailStoreMap.erase(it);
						//m_PlaylistFailStoreList.pop_back();
					}
					//prepare data
					if(info.bPlAttr)
					{
						m_PlayListStore.UpdatePlaylistAttr(info.uid,info.plAttr);
					}
					if(info.bPliAttr)
					{
						m_PlayListStore.UpdatePlaylistItemAttr(info.uid,info.plivecAttr);
					}
				}
				//是否可有能是需要update的东西太多导致clear不能及时执行???????
				while (m_PlaylistClearList.size()>0) 
				{
					FailStoreInfo	info;
					{
						ZQ::common::MutexGuard gd(m_failClearMutex,__MSGLOC__);
						info=m_PlaylistClearList.back();
						m_PlaylistClearList.pop_back();
					}
					{
						if(info.bPliAttr)
						{
							//ZQ::common::MutexGuard	gd(m_addNewPlMutex,__MSGLOC__);
							m_PlayListStore.ClearPlaylistItemAttr(info.uid);
						}
						if(info.bPlAttr)
						{
							//ZQ::common::MutexGuard	gd(m_addNewPlMutex,__MSGLOC__);
							m_PlayListStore.ClearPlaylistAttr(info.uid);
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}
	m_PlayListStore.ClearAll();
	glog(Log::L_DEBUG,"int PlaylistFailStoreThread::run()##Leaving");
	return 1;
}

void PlaylistFailStoreThread::AddNewPlaylistIntoFailOver(const std::string& strGuid, const ::PlaylistAttr& attr)
{
//	ZQ::common::MutexGuard	gd(m_addNewPlMutex,__MSGLOC__);
	TianShanIce::Streamer::PlaylistAttr plAttr;

	plAttr.ClientSessionID		=	attr.ClientSessionID;
	plAttr.currentCtrlNum		=	attr.currentCtrlNum;
	plAttr.destIP				=	attr.destIP;
	plAttr.destMac				=	attr.destMac;
	plAttr.destPort				=	attr.destPort;
	plAttr.endPoint				=	attr.endPoint;
	plAttr.Guid					=	attr.Guid;
	plAttr.MaxRate				=	attr.MaxRate;
	plAttr.MinRate				=	attr.MinRate;
	plAttr.NowRate				=	attr.NowRate;
	plAttr.playlistState		=	attr.playlistState;
	plAttr.programNumber		=	attr.programNumber;
	plAttr.ResourceGuid			=	attr.ResourceGuid;
	plAttr.StreamSmithSiteName	=	attr.StreamSmithSiteName;
	plAttr.vstrmPort			=	attr.vstrmPort;
	plAttr.vstrmSessID			=	attr.vstrmSessID;
	plAttr.streamPID			=	attr.streamPID ;

	plAttr.property.clear();

	m_PlayListStore.AddPlaylistAttr( strGuid , plAttr );
}

void PlaylistFailStoreThread::final()
{
}

void	PlaylistFailStoreThread::ChangeItemToAttr( Playlist* pList, ZQ::StreamSmith::VECPlaylistItemAttr& vecAttr )
{
	Playlist::List	itemList;
	pList->copyList(itemList);
	Playlist::List::iterator it;
	vecAttr.clear();
	TianShanIce::Streamer::PlaylistItemAttr	attr;
	char	strGuid[128];
	ZeroMemory(strGuid,128);
	pList->_guid.toString(strGuid,127);	
	int		i=0;
	for( it=itemList.begin() ; it != itemList.end() ; it++ )
	{
		attr.PlaylistGuid					=	strGuid;
		attr.InternalCtrlNum				=	(int)(it-itemList.begin());
		attr.itemSetupInfo.contentName		=	(*it).objectName.string;
		attr.CtrlNumber						=	(*it).userCtrlNum;
		attr.itemSetupInfo.inTimeOffset		=	(*it).inTimeOffset;
		attr.itemSetupInfo.outTimeOffset	=	(*it).outTimeOffset;
		attr.itemSetupInfo.criticalStart	=	(*it).criticalStart;
		attr.itemSetupInfo.spliceIn			=	((*it).spliceIn==1);
		attr.itemSetupInfo.spliceOut		=	((*it).spliceOut==1);
		attr.itemSetupInfo.forceNormal		=	((*it).forceNormalSpeed==1);
		attr.vStrmSessID					=	(*it).sessionId;
		attr.SessState						=	(Ice::Short)(*it).state;
		
		int	iUrlCount = (int)(*it)._itemLibraryUrls.size( );
		char	szBuf[128];
		for( int i = 0 ; i < iUrlCount ; i++ )
		{
		
			sprintf(szBuf,"nasUrl%03d",i);
			attr.property[szBuf] = (*it)._itemLibraryUrls[i];
		}
		sprintf( szBuf, "%d" , iUrlCount );
		attr.property["nasUrlCount"] = szBuf;

		vecAttr.push_back(attr);
	}
}
void	PlaylistFailStoreThread::ChangePlaylistToAttr(Playlist* pList , TianShanIce::Streamer::PlaylistAttr& attr)
{
	char	szBuf[128];

	ZeroMemory(szBuf,128);	
	pList->_guid.toString(szBuf,127);
	attr.Guid=szBuf;
	if(pList->m_pStreamSite)
		attr.StreamSmithSiteName	=	pList->m_pStreamSite->getSiteName();
	else
		attr.StreamSmithSiteName	=	"";
	ZeroMemory(szBuf,128);
	pList->_ResourceGuid.toString(szBuf,128);
    attr.ResourceGuid				=	szBuf;
    attr.ClientSessionID			=	pList->m_strClientSessionID;
    attr.endPoint					=	pList->_authEndpoint;
    attr.MaxRate					=	pList->_dvbAttrs.MaxMuxRate;
    attr.MinRate					=	pList->_dvbAttrs.MinMuxRate;
    attr.NowRate					=	pList->_dvbAttrs.NowMuxRate;
    attr.destIP						=	pList->_destIP;
    attr.destMac					=	pList->m_strDestinationMac;
    attr.destPort					=	pList->_dvbAttrs.udpPort;
    attr.vstrmPort					=	pList->_vstrmPortNum;
    attr.programNumber				=	pList->_dvbAttrs.ProgramNumber;	
    attr.playlistState				=	(::Ice::Short)(pList->_currentStatus);   
	//use userCtrlNum as the identity to indicate the current streaming item
	
	//_currentItemDist
	 //_currentItemDist;
	if( pList->_itCurrent != pList->_list.end() )
		attr.currentCtrlNum			=	pList->_itCurrent->userCtrlNum;
	else
		attr.currentCtrlNum			=	(CtrlNum)INVALID_CTRLNUM;

	if( pList->_itCurrent != pList->_list.end() )
		attr.vstrmSessID			=	pList->_itCurrent->sessionId;
	else
		attr.vstrmSessID			=	0;

	attr.streamPID					=	pList->_perStreamPID;


	///add extern attribute into property
	attr.property[DB_PLAYLIST_PROXY_STRING]		= pList->m_plePxStr;
	attr.property[DB_TICKET_PROXY_STRING]		= pList->m_ticketPrx;

	std::ostringstream	oss;

	oss << pList->mVstrmBwTcikets.FileTicket;
	attr.property[DB_VSTRM_BANDWIDTH_TICKET_FILE] = oss.str();

	oss.str("");
	oss << pList->mVstrmBwTcikets.EdgeTicket;
	attr.property[DB_VSTRM_BANDWIDTH_TICKET_EDGE] = oss.str();

}
void	PlaylistFailStoreThread::ChangeAttrToItem(ZQ::StreamSmith::VECPlaylistItemAttr& vecAttr,
													Playlist::List& itemList)
{

}
void	PlaylistFailStoreThread::ChangeAttrToPlaylist(TianShanIce::Streamer::PlaylistAttr& attr,Playlist* pList )
{
}


#endif//_ICE_INTERFACE_SUPPORT
//////////////////////////////////////////////////////////////////////////

// -----------------------------
// class PlaylistManager
// -----------------------------
void	vstrmSpigotStatusCallback( const VstrmClass::VstrmSpigotChar& spigotChar , const int spigotIndex , void* pUserData )
{
	assert( pUserData != NULL );
	PlaylistManager *pThis = reinterpret_cast<PlaylistManager*>(pUserData);
	pThis->spigotStatusCallback( spigotChar , spigotIndex);

}
void bandwidthStatusCallback( const BandwidthUsage& bwUsage , void* pUserData )
{
	assert( pUserData != NULL );
	PlaylistManager *pThis = reinterpret_cast<PlaylistManager*>(pUserData);
	pThis->bandwidthUsageCallback( bwUsage );
}

void PlaylistManager::bandwidthUsageCallback( const BandwidthUsage& bwUsage )
{
	//glog(ZQ::common::Log::L_INFO,CLOGFMT(spigotStatusCallback,"bandwidth usage changed "));
	//send out the status report
	//step 1 
	// connect to the ReplicaSubscriber
#pragma message(__MSGLOC__"TODO: call the replica subscriber here")
	std::string& strReplicaSubscriber = GAPPLICATIONCONFIGURATION.spigotReplicaConfig.listenerEndpoint;
	if( !strReplicaSubscriber.empty() )
	{
		try
		{
			if(strReplicaSubscriber.find(":") == std::string::npos)
			{
				strReplicaSubscriber = "" + strReplicaSubscriber;
			}
			Ice::CommunicatorPtr ic = m_Adapter->getCommunicator( );
			TianShanIce::ReplicaSubscriberPrx repSubPrx = TianShanIce::ReplicaSubscriberPrx::checkedCast( ic->stringToProxy( strReplicaSubscriber ) );

			TianShanIce::Replicas reps;
			TianShanIce::Replica rep;
			rep.category	=	"BandwidthUsage";
			rep.groupId		=	GAPPLICATIONCONFIGURATION.spigotReplicaConfig.groupId;
			rep.replicaState=	TianShanIce::stInService;
			rep.replicaId	=	"";
			rep.priority	=	255;
			rep.maxPrioritySeenInGroup = 0;
			rep.obj			=	NULL;
			rep.stampBorn	=	0;
			rep.stampChanged=	0;

			std::ostringstream oss;
			oss << bwUsage.cdnUsedBandiwidth;
			rep.props["UsedReplicaImportBandwidth"] = oss.str();
			oss.str("");

			oss << bwUsage.cdnTotalBandwidth;
			rep.props["TotalReplicaImportBandwidth"] = oss.str();
			oss.str("");

			oss << bwUsage.cdnImportSessionCount;
			rep.props["runningImportSessionCount"]	= oss.str();
			oss.str("");

			reps.push_back(rep);
			repSubPrx->updateReplica(reps);

		}
		catch( const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR,
				"failed to report the status to bandwidth replica subscriber[%s] with ice exception[%s]",
				strReplicaSubscriber.c_str() , ex.ice_name().c_str()  );
			return;
		}
	}
}
void nodeStatusEventCallback( const std::string& nodeName , bool bUp, void* pUserData )
{
	PlaylistManager* pThis = (PlaylistManager*)pUserData;
	pThis->nodeStatusCallback(nodeName,bUp);
}
class ReplicaUpdateCallback: public TianShanIce::AMI_ReplicaSubscriber_updateReplica
{
public:
	ReplicaUpdateCallback(){}
	virtual ~ReplicaUpdateCallback(){}

protected:
	virtual void ice_response(::Ice::Int) {}
	virtual void ice_exception(const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(ReplicaUpdateCallback,"caught exception:%s"),ex.ice_name().c_str());
	}
};
void PlaylistManager::nodeStatusCallback( const std::string& nodeName , bool bUp )
{
	if( bUp)
		return;//do not care about node up event
	glog(ZQ::common::Log::L_INFO,CLOGFMT(nodeStatusCallback,"node[%s] is [%s]"),nodeName.c_str(), bUp?"UP":"DOWN");

	std::string& strReplicaSubscriber = GAPPLICATIONCONFIGURATION.spigotReplicaConfig.listenerEndpoint;
	if( strReplicaSubscriber.empty() )
	{
		glog(ZQ::common::Log::L_INFO,"No available replica subscriber , do not report spigot status");
		return;
	}
	
	try
	{
		if(strReplicaSubscriber.find(":") == std::string::npos)
		{
			strReplicaSubscriber = "" + strReplicaSubscriber;
		}
		Ice::CommunicatorPtr ic = m_Adapter->getCommunicator( );
		TianShanIce::ReplicaSubscriberPrx repSubPrx = TianShanIce::ReplicaSubscriberPrx::checkedCast( ic->stringToProxy( strReplicaSubscriber ) );

		TianShanIce::Replicas reps;

		TianShanIce::Replica rep;
		rep.category	= "StreamService";/*GAPPLICATIONCONFIGURATION.spigotReplicaConfig.category;*/		
		rep.groupId		= nodeName;
		rep.replicaState = (bUp) ? TianShanIce::stInService : TianShanIce::stOutOfService;
		rep.maxPrioritySeenInGroup = 0;
		rep.priority	= 0;

		//set reportingNode so that subscribe can get to know which node report this event
		ZQTianShan::Util::updatePropertyData(rep.props,"reportingNode",GAPPLICATIONCONFIGURATION.spigotReplicaConfig.groupId);
				
		Ice::Identity id;
		id.category		= "";
		id.name			= "StreamSmith";
		rep.obj			= m_Adapter->createProxy(id);
		reps.push_back(rep);		
		repSubPrx->updateReplica_async((new ReplicaUpdateCallback()),reps);
	}
	catch( const Ice::Exception& ex)
	{
		//if I failed to report the event to subscriber
		//what should I do ?
		//just ignore the event or save it and resend in the future
		glog(ZQ::common::Log::L_ERROR,"failed to report the status to replica subscriber[%s] with ice exception[%s]",
			strReplicaSubscriber.c_str() , ex.ice_name().c_str()  );
		return;
	}
}

void PlaylistManager::spigotStatusCallback( const VstrmClass::VstrmSpigotChar& spigotChar , const int spigotIndex)
{
	glog(ZQ::common::Log::L_INFO,CLOGFMT(spigotStatusCallback,"spigot status changed "));
	//send out the status report
	//step 1 
	// connect to the ReplicaSubscriber
#pragma message(__MSGLOC__"TODO: call the replica subscriber here")
	std::string& strReplicaSubscriber = GAPPLICATIONCONFIGURATION.spigotReplicaConfig.listenerEndpoint;
	if( !strReplicaSubscriber.empty() )
	{
		try
		{
			if(strReplicaSubscriber.find(":") == std::string::npos)
			{
				strReplicaSubscriber = "" + strReplicaSubscriber;
			}
			Ice::CommunicatorPtr ic = m_Adapter->getCommunicator( );
			TianShanIce::ReplicaSubscriberPrx repSubPrx = TianShanIce::ReplicaSubscriberPrx::checkedCast( ic->stringToProxy( strReplicaSubscriber ) );

			TianShanIce::Replicas reps;
			TianShanIce::Replica rep;		
			rep.category	= "Streamer";/*GAPPLICATIONCONFIGURATION.spigotReplicaConfig.category;*/
			rep.groupId		= GAPPLICATIONCONFIGURATION.spigotReplicaConfig.groupId;
			//rep.disabled	= static_cast<bool>( spigotChar.status != STATUS_READY );
			rep.replicaState = (spigotChar.status != STATUS_READY) ? TianShanIce::stOutOfService : TianShanIce::stInService;
			rep.maxPrioritySeenInGroup = 0;
			rep.priority	= 0;

			char szReplicaId[128];
			sprintf( szReplicaId , "Spigot%d" , spigotIndex);
			rep.replicaId	= szReplicaId;

			rep.stampBorn	= spigotChar.firstUp;
			rep.stampChanged= spigotChar.lastUpdate;

			Ice::Identity id;
			id.category		= "";
			id.name			= "StreamSmith";
			rep.obj			= m_Adapter->createProxy(id);
			reps.push_back(rep);		

			/////////////////////////////////////////////////////////////
			rep.category	= "Streamer";/*GAPPLICATIONCONFIGURATION.spigotReplicaConfig.category;*/
			rep.groupId		= GAPPLICATIONCONFIGURATION.spigotReplicaConfig.groupId;
			//rep.disabled	= static_cast<bool>(  spigotChar.status != STATUS_READY );
			rep.replicaState = (spigotChar.status != STATUS_READY) ? TianShanIce::stOutOfService : TianShanIce::stInService;
			rep.maxPrioritySeenInGroup = 0;
			rep.priority	= 0;

			/*char szReplicaId[128];*/
			sprintf( szReplicaId , "BoardNumber%d" , spigotIndex);
			rep.replicaId	= szReplicaId;

			rep.stampBorn	= spigotChar.firstUp;
			rep.stampChanged= spigotChar.lastUpdate;

			/*Ice::Identity id;*/
			id.category		= "";
			id.name			= "StreamSmith";
			rep.obj			= m_Adapter->createProxy(id);
			reps.push_back(rep);		
			repSubPrx->updateReplica(reps);
		}
		catch( const Ice::Exception& ex)
		{
			//if I failed to report the event to subscriber
			//what should I do ?
			//just ignore the event or save it and resend in the future
			glog(ZQ::common::Log::L_ERROR,"failed to report the status to replica subscriber[%s] with ice exception[%s]",
				strReplicaSubscriber.c_str() , ex.ice_name().c_str()  );
			return;
		}
	}
	else
	{
		glog(ZQ::common::Log::L_INFO,"No available replica subscriber , do not report spigot status");
	}
}

#ifndef _ICE_INTERFACE_SUPPORT
PlaylistManager::PlaylistManager(VstrmClass& cls):
_cls(cls), ThreadRequest(cls.getThreadPool()), _sessmon(cls),
_hEvtWakeup(NULL), _timeout(TIMEOUT_INF), _bQuit(false)
#else
PlaylistManager::PlaylistManager(VstrmClass& cls,Ice::CommunicatorPtr& ic,const char* endPoint): _cls(cls), ThreadRequest(cls.getThreadPool()), _sessmon(cls),
_hEvtWakeup(NULL), _timeout(TIMEOUT_INF), _bQuit(false),m_ic(ic),m_contentChecker(ic)
#endif
,m_spigotReplicaReporter(*this),
mbDBRecordValid(true),
mPendingRequestLastCheckTime(0),
mbServiceAvailable(true)
{
	_staticPlaylistId = 0;
	_cls.registerSpigotStatusCallback( vstrmSpigotStatusCallback , this);
	_cls.registerBandwidthUsageCallback( bandwidthStatusCallback , this );
	_cls.registerNodeStatusCallback(nodeStatusEventCallback,this);
	_bTimeoutOnPause=true;
#ifdef WITH_ICESTORM
	_pEventDispatch=NULL;
	_pEventDispatchExtraData=NULL;
#endif//WITH_ICESTORM
	_hEvtWakeup = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	//_sessmon.start();	
	m_pKillPlayList=new PlaylistSuicide;
	m_pKillPlayList->start();
#pragma message(__MSGLOC__"After merge NCS into streamsmith,leave NCS call vsm_initialize() ")
#ifdef _ICE_INTERFACE_SUPPORT

	try
	{
		if(endPoint==NULL || strlen(endPoint) <= 0 )
		{
			m_Adapter = ZQADAPTER_CREATE(m_ic,"PLInfoStore","default -p 10000",glog);
			//m_Adapter=m_ic->createObjectAdapterWithEndpoints("PLInfoStore","default -p 10000");
		}
		else
		{
			m_Adapter = ZQADAPTER_CREATE(m_ic,"PLInfoStore",endPoint,glog);
			//m_Adapter=m_ic->createObjectAdapterWithEndpoints("PLInfoStore",endPoint);
		}
#ifndef _INDEPENDENT_ADAPTER		
		std::vector<MonitoredLog>::const_iterator iter = gStreamSmithConfig.monitoredLogs.begin();		
		for (; iter != gStreamSmithConfig.monitoredLogs.end(); ++iter ) 
		{			
			if (!m_Adapter->publishLogger(iter->name.c_str(), iter->syntax.c_str() ,iter->key.c_str() , iter->type.c_str() ) )				
			{				
				glog(ZQ::common::Log::L_ERROR, "Failed to publish logger name[%s] synax[%s] key[%s] type[%s]",
					iter->name.c_str(), iter->syntax.c_str() , iter->key.c_str() , iter->type.c_str() );
			}			
			else				
			{				
				glog(ZQ::common::Log::L_INFO, "Publish logger name[%s] synax[%s] key[%s] type[%s] successful", 					
					iter->name.c_str(), iter->syntax.c_str() , iter->key.c_str() , iter->type.c_str() );
			}
		}		
#endif
		
	}
	catch(::Ice::Exception& ex)
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","CreateObject Adapter failed with ice exception is %s and EndPoint is %s"),
					ex.ice_name().c_str(),endPoint);
		throw ex;
	}
	m_pPlayListStore = new PlaylistInfoStore( m_Adapter , *this);
	m_pFailStoreThread = new PlaylistFailStoreThread(*m_pPlayListStore);
	
#endif

	int32 amdThreadPoolSize = gStreamSmithConfig.lServiceAmdThreadCount;
	amdThreadPoolSize = amdThreadPoolSize < 3 ? 3 : amdThreadPoolSize;
	mAmdThreadpool.resize(amdThreadPoolSize);
	mSysLogger.open( "StreamSmith" , ZQ::common::Log::L_DEBUG);
}

bool PlaylistManager::setContentStoreProxy( Ice::ObjectPrx objPrx )
{
	mContentStoreProxy = objPrx;
	return m_contentChecker.setContentStoreProxy( objPrx );
}

PlaylistManager::~PlaylistManager()
{
	m_spigotReplicaReporter.stop();
	
	_sessmon.stop();
	WaitForSingleObject(_sessmon.GetQuittedHandle(),INFINITE);
	_bQuit = true;

	if (_hEvtWakeup)
	{
		::SetEvent(_hEvtWakeup);
		Sleep(10);
		::CloseHandle(_hEvtWakeup);
	}
	_hEvtWakeup = NULL;
#ifdef _ICE_INTERFACE_SUPPORT
	if(m_pFailStoreThread)
	{
		m_pFailStoreThread->Stop();
		delete m_pFailStoreThread;
		m_pFailStoreThread=NULL;
	}
#pragma message(__MSGLOC__"DO NOT clear stream when stop")
	
	if(m_pPlayListStore!=NULL)
	{
		delete m_pPlayListStore;
		m_pPlayListStore=NULL;
	}

#ifdef _ICE_INTERFACE_SUPPORT
	if(strlen(gStreamSmithConfig.szSuperPluginPath)> 0 && gStreamSmithConfig.lEnableSuperPlugin)
	{//ReleaseSuperPlugin will deactivate adapter, so  can I deactivate adapter twice ?
		ReleaseSuperPlugin();
	}
#endif//_ICE_INTERFACE_SUPPORT


#endif

	m_pKillPlayList->SetquitFlag(true);
	WaitForSingleObject(m_pKillPlayList->GetQuittedHandle(),INFINITE);
	delete m_pKillPlayList;
	m_pKillPlayList=NULL;

}
#ifdef WITH_ICESTORM
bool SSMH_EventChannelEventSink(IStreamSmithSite* pSite, EventType Type, ZQ::common::Variant& params)
{
	PlaylistManager* pMan=StreamSmithSite::m_pPlayListManager;
	if(!pMan)
	{
		glog(Log::L_ERROR,"No playlistmanager instance,return without send out event message");
		return true;//That is no more sending out of current message
	}
	return pMan->postEventSink(Type,params);
}
void PlaylistManager::registerEventSink(DWORD type,EventDispatchEvent eDispatch,void* pExtraData)
{
	IStreamSmithSite* pDefaultSite=StreamSmithSite::getDefaultSite();
	if(!pDefaultSite)
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","registerEventSink() Can't get default streamsmith site"));
		return;
	}
	if(eDispatch==NULL)
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","Invalid event dispatch function address"));
		return;
	}
	_pEventDispatch=eDispatch;
	_pEventDispatchExtraData=pExtraData;

	pDefaultSite->SinkEvent(type,(SSMH_EventSink)SSMH_EventChannelEventSink);
}
bool PlaylistManager::postEventSink(DWORD eventType,ZQ::common::Variant& params)
{
	if(!_pEventDispatch)
	{
		return true;
	}
	return _pEventDispatch(eventType,params,_pEventDispatchExtraData);
}
#endif//WITH_ICESTORM

void EatWhite(std::string& str)
{	
	int iPos=0;
	int iLastPos=0;
	int iSize=str.size();
	if(iSize<=0)
		return;
	for (;iPos<iSize;iPos++) 
	{
		if(!(str[iPos]==' ' || str[iPos]=='\t'))
			break;
	}
	iLastPos=iSize-1;
	for(;iLastPos>=0;iLastPos--)
	{
		if(!(str[iLastPos]==' ' || str[iLastPos]=='\t'))
			break;
	}

	str=str.substr(iPos,iLastPos-iPos+1);
	return;
}

bool	PlaylistManager::initIceInterface( ZQ::common::Log* pLog,char* strSuperPluginPath,
											 char* eventChannelEndPoint,char* netId,char* spigotsID )
{
	_sessmon.SetLogInstance(pLog);
	std::vector<int> vecSpigotsIDs;
	int iStrLen=strlen(spigotsID);
	int iLastPos=-1;
	int iPos=0;
	char szBuf[128];
	for(iPos=0;iPos<iStrLen;iPos++)
	{
		if(spigotsID[iPos]!=';')
			continue;
		if(iLastPos==iPos)
			continue;
		if(iPos-iLastPos>0)
		{
			ZeroMemory(szBuf,sizeof(szBuf));
			strncpy(szBuf,&spigotsID[iLastPos+1],iPos-iLastPos-1);
			std::string strTemp=szBuf;
			EatWhite(strTemp);
			if(!strTemp.empty())
			{
				int iTemp=atoi(strTemp.c_str());
				glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","parse default spigotsIDs ##get a new spigot board number %d with string=%s"),iTemp,szBuf);
				vecSpigotsIDs.push_back(iTemp);
			}
		}
		iLastPos=iPos;
	}
	if((iPos)-iLastPos>0)
	{
		ZeroMemory(szBuf,sizeof(szBuf));
		strncpy(szBuf,&spigotsID[iLastPos+1],iPos-iLastPos-1);
		std::string strTemp=szBuf;
		EatWhite(strTemp);
		if(!strTemp.empty())
		{
			int iTemp=atoi(strTemp.c_str());
			glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","parse default spigotsIDs ##get a new spigot board number %d with string=%s"),iTemp,szBuf);
			vecSpigotsIDs.push_back(iTemp);
		}			
	}
	m_pFailStoreThread->start();
	if( strlen(gStreamSmithConfig.szSuperPluginPath)>0 && 
		gStreamSmithConfig.lEnableSuperPlugin)
	{		
		LoadSuperPlugin(strSuperPluginPath,eventChannelEndPoint,netId,vecSpigotsIDs);
		//m_Adapter->activate();
		//glog(Log::L_INFO,LOGFORMAT("PlaylistManager","activate the ice adapter"));
	}	
	return true;

}
void PlaylistManager::StartSessionMonitor( char* spigotsID )
{	
	//在这之前将所有的FailOver的PlayList重新建立起来等待VstrmSession的检测
	//

	_sessmon.ScanSessWhenStartup();
	//set properties to adpater and communicator

	_sessmon.SetProgressGranularity(gStreamSmithConfig.lProgressEventSendoutInterval);

	_sessmon.start();
	std::vector<int> vecSpigotsIDs;
	int iStrLen=strlen(spigotsID);
	int iLastPos=-1;
	int iPos=0;
	char szBuf[128];
	for(iPos=0;iPos<iStrLen;iPos++)
	{
		if(spigotsID[iPos]!=';')
			continue;
		if(iLastPos==iPos)
			continue;
		if(iPos-iLastPos>0)
		{
			ZeroMemory(szBuf,sizeof(szBuf));
			strncpy(szBuf,&spigotsID[iLastPos+1],iPos-iLastPos-1);
			std::string strTemp=szBuf;
			EatWhite(strTemp);
			if(!strTemp.empty())
			{
				int iTemp=atoi(strTemp.c_str());
				glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","parse default spigotsIDs ##get a new spigot board number %d with string=%s"),iTemp,szBuf);
				vecSpigotsIDs.push_back(iTemp);
			}
		}
		iLastPos=iPos;
	}
	if((iPos)-iLastPos>0)
	{
		ZeroMemory(szBuf,sizeof(szBuf));
		strncpy(szBuf,&spigotsID[iLastPos+1],iPos-iLastPos-1);
		std::string strTemp=szBuf;
		EatWhite(strTemp);
		if(!strTemp.empty())
		{
			int iTemp=atoi(strTemp.c_str());
			glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","parse default spigotsIDs ##get a new spigot board number %d with string=%s"),iTemp,szBuf);
			vecSpigotsIDs.push_back(iTemp);
		}			
	}
	
	StreamSmithSite::_pDefaultSite->setDefaultSpigotsBoards(vecSpigotsIDs);

	IceUtil::Time interval = IceUtil::Time::milliSeconds(2000);
	int32 defaultUpdateInterval = GAPPLICATIONCONFIGURATION.spigotReplicaConfig.defaultUpdateInterval;
	std::string& litenerEndpoint = GAPPLICATIONCONFIGURATION.spigotReplicaConfig.listenerEndpoint;
// 	m_spigotReplicaReporter->schedule( new spigotReplicaReport(*this, defaultUpdateInterval, litenerEndpoint ) ,
// 									interval);
	m_spigotReplicaReporter.set(defaultUpdateInterval,litenerEndpoint);
	m_spigotReplicaReporter.start();

	mIdxParserEnv.InitVstrmEnv();
	mIdxParserEnv.AttchLogger(&glog);
	if (gStreamSmithConfig.embededContenStoreCfg.ctntAttr.attrFromVstm != 0)
	{
		mIdxParserEnv.setUseVstrmIndexParseAPI(true);
		glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","use vstrm api anyway, due to attrFromVstm is unequal to 0"));
	}
	else
	{
		mIdxParserEnv.setUseVstrmIndexParseAPI( ( gStreamSmithConfig.serverMode == 2 ) );//EdgeServer
	}
	mIdxParserEnv.setUseVsOpenAPI( gStreamSmithConfig.embededContenStoreCfg.ctntAttr.useVsOpenAPI >= 1 );
	mIdxParserEnv.setSkipZeroByteFile( gStreamSmithConfig.embededContenStoreCfg.ctntAttr.skipZeroByteFiles >= 1 );
}
#ifdef _ICE_INTERFACE_SUPPORT



void PlaylistManager::LoadSuperPlugin(const char* strPath,const char* eventChannelEndPoint,char* strNetID,const std::vector<int>& SpigotsIDs)
{
	glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","Load super plugin %s "),strPath);
	m_hSuperPlugin=LoadLibraryA(strPath);
	if(m_hSuperPlugin==NULL)
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","Load Library %s fail and error code is %d"),
																	strPath,GetLastError());
		return;
	}
	initializeSuper Init=(initializeSuper)GetProcAddress(m_hSuperPlugin,"Initialize");
	if(!Init)
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","Get SuperPluginInitialize fail from %s"),strPath);
		return;
	}
	UninitializeSuper uninit=(UninitializeSuper)GetProcAddress(m_hSuperPlugin,"UnInitialize");
	if(!uninit)
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","Get SuperPluginUnIntialize fail from %s"),strPath);
		return;
	}
	try
	{
		std::string	strEndpoint;
		if(eventChannelEndPoint&&strlen(eventChannelEndPoint)>0)
		{
			strEndpoint=SERVICE_NAME_TopicManager;
			strEndpoint+=":";
			strEndpoint+=eventChannelEndPoint;
		}
		Init(this,m_Adapter,strEndpoint,strNetID,&glog,SpigotsIDs);
		glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","loaded superplugin %s successfully"),strPath);
	}
	catch (...) 
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","Initialize superplugin failed due to some unexpect error"));
	}
}

void PlaylistManager::ReleaseSuperPlugin()
{
	if(m_hSuperPlugin!=NULL)
	{
		UninitializeSuper uninit=(UninitializeSuper)GetProcAddress(m_hSuperPlugin,"UnInitialize");
		if(uninit)
		{
			try
			{
				m_Adapter->deactivate();		
			}
			catch(...)
			{
			}
			try
			{
				mAmdThreadpool.stop();
			}
			catch(...){}

			try
			{
				uninit(m_Adapter);
			}
			catch (...) 
			{
			}
			
			try
			{				
				m_ic->destroy();
				int iRef = m_ic->__getRef();
				m_ic = NULL;
			}
			catch(...)
			{
			}
		}
		FreeLibrary(m_hSuperPlugin);
	}
	else
	{
		try
		{
			m_Adapter->deactivate();		
		}
		catch(...)
		{
		}
		try
		{			
			m_ic->destroy();
			m_ic = NULL;
		}
		catch(...)
		{
		}
	}
}
#endif//_ICE_INTERFACE_SUPPORT
Playlist* PlaylistManager::find(const ZQ::common::Guid& uid)
{
	if (uid.isNil())
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","find() Guid is NULL"));
		return NULL;
	}

	ZQ::common::MutexGuard gd(_mapLocker,__MSGLOC__);
	Map::iterator it = _map.find(uid);
	if (_map.end() == it)
	{
		return NULL;
	}

	return it->second.pl;
}

Playlist* PlaylistManager::plExist( long id )
{
	ZQ::common::MutexGuard gd(_mapLocker,__MSGLOC__);
	MapPLExist::const_iterator it = _existPl.find(id);
	if( it != _existPl.end() )
		return it->second;
	else 
		return NULL;
}
void PlaylistManager::ServiceDown()
{
	m_spigotReplicaReporter.setServiceState( false );//service is down	
	return;
}

long PlaylistManager::reg(Playlist& playlist)
{
	Playlist* oldpl = NULL;
	
	{
		ZQ::common::MutexGuard gd(_mapLocker,__MSGLOC__);
		Map::iterator it = _map.find(playlist._guid);
		_staticPlaylistId ++;
		{
			char	szBuf[128];
			ZeroMemory(szBuf,128);
			playlist._guid.toString(szBuf,127);
			glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","Reg a new playlist with guid=%s"),szBuf);
		}
		
		PlaylistInfo info;
		info.pl		= &playlist;
		info.id		= _staticPlaylistId;
		_map.insert(Map::value_type(playlist._guid, info));
		_existPl.insert( MapPLExist::value_type(_staticPlaylistId,info.pl) );
		return _staticPlaylistId;
	}	
	return true;
}

bool PlaylistManager::unreg( Playlist& playlist)
{

	ZQ::common::MutexGuard gd(_mapLocker,__MSGLOC__);
	Map::iterator it = _map.find(playlist._guid);
	if (_map.end() == it)
		return false;
	{
		char	szBuf[128];
		ZeroMemory(szBuf,128);
		playlist._guid.toString(szBuf,127);
		glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","UnReg playlist with guid=%s"),szBuf);
	}
	if( _map.size() > 250 )
	{
		glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","There are [%u] playlists in PlaylistManager"), _map.size() );
	}

	Playlist* pl=(it->second.pl);
	long id = (it->second.id);
	_map.erase(playlist._guid);
	MapPLExist::iterator itexistPl=_existPl.find(id);
	_existPl.erase(id);
	m_pKillPlayList->PushPlaylist(pl);
	return true;
}

bool PlaylistManager::init(void)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
#ifdef _ICE_INTERFACE_SUPPORT
IPlaylistEx*	PlaylistManager::CreatePlaylist(const std::string& guid,const std::vector<int>& boardIDs , const std::string& userSessId )
{
	ZQ::common::Guid uid(guid.c_str());
	Playlist* pl=new Playlist(NULL,*this,uid,false,boardIDs);
	if(!pl->PlayListInitOK())
	{
		pl->destroy();
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","playlist initialize fail  with guid=%s"),guid.c_str());
		return NULL;
	}
	return (IPlaylistEx*)pl;
}
void	PlaylistManager::plSetupCallback(const std::string& guidStr , 
										 const TianShanIce::Streamer::PlaylistAttr& plAttr,void* pData)
{
	PlaylistManager* pMan=(PlaylistManager*)pData;
	pMan->PlaylistReConstruct(guidStr,plAttr);
}
void	PlaylistManager::pliSetupCallback(const std::string& guidStr , 
										  ZQ::StreamSmith::VECPlaylistItemAttr& pliAttr,void* pData)
{
	PlaylistManager* pMan=(PlaylistManager*)pData;
	pMan->PlaylistItemReConstruct(guidStr,pliAttr);
}
void PlaylistManager::PlaylistReConstruct(const std::string& guidStr ,
										  const TianShanIce::Streamer::PlaylistAttr& plAttr)
{
	ZQ::common::Guid	uid(guidStr.c_str());
	if(find(uid)!=NULL)
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","void PlaylistManager::PlaylistReConstruct() already exist a playlist with guid = %s"),guidStr.c_str());
		return;
	}

	StreamSmithSite* pSite=StreamSmithSite::findSite(plAttr.StreamSmithSiteName.c_str());

	Playlist *pList=new Playlist(pSite,*this,uid,true);
	if(!pList)
	{
		glog(Log::L_ERROR,"貌似内存不够>>>>这么夸张");
		return;
	}
	
    pList->_ResourceGuid = ZQ::common::Guid(plAttr.ResourceGuid.c_str());
    
    pList->m_strClientSessionID = plAttr.ClientSessionID;
#ifdef VSTRM_GUID
    if (!pList->m_strClientSessionID.empty())
    {
       ZQ::common::Guid::GUID guid= (ZQ::common::Guid::GUID) ZQ::common::Guid(plAttr.ClientSessionID.c_str());
       memcpy(&pList->_userSessGuid, &guid, sizeof(pList->_userSessGuid));
    }
    else
    {
	// eventIS doesn't provide an OnDemandSessionId, so take the guid of playlist by default
	ZQ::common::Guid::UUID guid = (ZQ::common::Guid::UUID) ZQ::common::Guid(pList->_strGuid.c_str());
	memcpy(&pList->_userSessGuid, &guid, sizeof(pList->_userSessGuid));
    }
#endif // VSTRM_GUID

	pList->setAuth(plAttr.endPoint.c_str());
	pList->setMuxRate(plAttr.NowRate,plAttr.MaxRate,plAttr.MinRate);
	pList->setDestination((char*)plAttr.destIP.c_str(),plAttr.destPort);
	pList->setDestMac((char*) plAttr.destMac.c_str());
	//怎么处理VstrmPort呢？    
	pList->setProgramNumber(plAttr.programNumber);
	glog(Log::L_DEBUG,"status=%d",plAttr.playlistState);
	pList->_currentStatus=(IPlaylist::State)plAttr.playlistState;	
	if((ULONG)plAttr.vstrmPort!=_cls.GetUnUsePort(plAttr.vstrmPort))
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","Can't get vstrm port %d"),plAttr.vstrmPort);
		if( plAttr.vstrmPort > ( _cls.getPortCount() * 2 ) )
		{
			glog(ZQ::common::Log::L_ERROR,LOGFORMAT("PlaylistManager","Invalid vstrm port , database maybe corrupt"));
		}
	}
	else
	{
		pList->_vstrmPortNum=plAttr.vstrmPort;
		glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","get vtrsm port =%d when failover startup"),plAttr.vstrmPort);
	}
	//set streamPID
	glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","get streamPID =%d when failover startup"),plAttr.streamPID);
	pList->_perStreamPID = plAttr.streamPID;
	//现在的问题是如何设置当前播放的iterator

	pList->_currentItemDist=plAttr.currentCtrlNum;	
	pList->_vstrmSessioIDForFailOver=plAttr.vstrmSessID;
	glog(Log::L_DEBUG,"PLaylist[%s] failover with current internal ctrlNum[%u] current vstrmSessId[%u] status=%d",
			plAttr.Guid.c_str(),plAttr.currentCtrlNum,plAttr.vstrmSessID,plAttr.playlistState);
    //attr.currentCtrlNum=pList->_itCurrent->userCtrlNum;
	std::map<std::string,std::string>::const_iterator itProperty ;

	std::string strPlistProxyString	 ="";
	itProperty =  plAttr.property.find(DB_PLAYLIST_PROXY_STRING);
	if( itProperty != plAttr.property.end() )
	{
		strPlistProxyString = itProperty->second;
	}
	std::string strTicketProxyString	= "";
	
	itProperty =  plAttr.property.find(DB_TICKET_PROXY_STRING);
	if ( itProperty != plAttr.property.end() )
	{
		strTicketProxyString= itProperty->second;
	}
	pList->m_ticketPrx	= strTicketProxyString;
	pList->m_plePxStr	= strPlistProxyString;

	
	itProperty = plAttr.property.find(DB_VSTRM_BANDWIDTH_TICKET_FILE);
	if( itProperty != plAttr.property.end())
	{
		ULONGLONG ticketId = 0;
		sscanf( itProperty->second.c_str() , "%llu",&ticketId );
		pList->mVstrmBwTcikets.FileTicket = ticketId;
		ticketsInDb.push_back(ticketId);
	}

	itProperty = plAttr.property.find(DB_VSTRM_BANDWIDTH_TICKET_EDGE);
	if( itProperty != plAttr.property.end())
	{
		ULONGLONG ticketId = 0;
		sscanf( itProperty->second.c_str() , "%llu",&ticketId );
		pList->mVstrmBwTcikets.EdgeTicket = ticketId;
		ticketsInDb.push_back(ticketId);
	}

	glog(ZQ::common::Log::L_DEBUG, "PLaylist[%s] failover with ticketproxyString[%s] ",
				plAttr.Guid.c_str( ) , strTicketProxyString.c_str() );
}

void PlaylistManager::PlaylistItemReConstruct(const std::string& guidStr ,
											  ZQ::StreamSmith::VECPlaylistItemAttr& vecPliAttr)
{
	ZQ::common::Guid uid(guidStr.c_str());
	Playlist* pList;
	if(NULL== (pList=find(uid)) )
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","PlaylistItemReConstruct()  Can't find playlist\
										instance through uid=%s"),guidStr.c_str());
		return;
	}
	ZQ::StreamSmith::VECPlaylistItemAttr::iterator	it;
	Playlist::Item	item;
	Playlist::List& itemList=pList->_list;
	int i=0;
	bool bFound=false;
	while (vecPliAttr.size()>0) 
	{
		bFound=false;
		for( it = vecPliAttr.begin(); it != vecPliAttr.end(); it++ )
		{
			if( i == it->InternalCtrlNum /*item index*/)
			{
				glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","found item index=%d and guid=%s"),
					it->InternalCtrlNum,guidStr.c_str());
				{
					item.intlCtrlNum			= i;
					item.criticalStart			= (time_t)it->itemSetupInfo.criticalStart;
					item.userCtrlNum			= it->CtrlNumber;
					strncpy(item.objectName.string , it->itemSetupInfo.contentName.c_str() , 255);
					item.forceNormalSpeed		= it->itemSetupInfo.forceNormal;
					item.inTimeOffset			= (ULONG)it->itemSetupInfo.inTimeOffset;
					item.outTimeOffset			= (ULONG)it->itemSetupInfo.outTimeOffset;
					item.spliceIn				= it->itemSetupInfo.spliceIn;
					item.spliceOut				= it->itemSetupInfo.spliceOut;
					item.sessionId				= it->vStrmSessID;
					item.state					= it->SessState;
					std::string rawItemName		= it->itemSetupInfo.contentName;
					std::string::size_type iSlashPos ;
					if ( ( iSlashPos = rawItemName.rfind ('\\') ) != std::string::npos )
					{
						rawItemName = rawItemName.substr (iSlashPos+1);
						strncpy( item._rawItemName , rawItemName.c_str () , 255 );
					}
					/*
					int	iUrlCount = (int)(*it)->_itemLibraryUrls.size( );
					char	szBuf[128];
					for( int i = 0 ; i < iUrlCount ; i++ )
					{

					sprintf(szBuf,"nasUrl%03d",i);
					attr.property[szBuf] = (*it)->_itemLibraryUrls[i];
					}
					sprintf( szBuf, "%d" , iUrlCount );
					attr.property["nasUrlCount"] = szBuf;
					*/
					//get nas urls count first
					std::map<std::string,std::string>::const_iterator itProperty;
					itProperty = it->property.find("nasUrlCount");
					bool bHasUrl = false;
					if(  itProperty != it->property.end() )
					{
						int iCount = atoi(itProperty->second.c_str() );
						char	szBuf[128];
						for( int i = 0; i <iCount ; i++ )
						{
							sprintf(szBuf, "nasUrl%03d" , i);
							itProperty = it->property.find(szBuf);
							if ( itProperty != it->property.end() )
							{
								item._itemLibraryUrls.push_back( itProperty->second );
								bHasUrl = true;
							}
						}
					}
					item._bEnableItemLibrary = bHasUrl;
				}
				itemList.push_back(item);
				i++;
				vecPliAttr.erase(it);
				bFound=true;
				break;
			}
		}
		if(!bFound)
		{
			glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","void PlaylistManager::PlaylistItemReConstruct()##Invalid playlist item data"));
			break;
		}
	}
}
int	PlaylistManager::getIceEvitorPLaylistSize()
{
	return m_pPlayListStore->GetPlaylistSize();
}

int	PlaylistManager::getIceEvitorPLaylistItemSize()
{
	return m_pPlayListStore->GetPlaylistItemSize();
}
int	PlaylistManager::getPlaylistCount()
{
	ZQ::common::MutexGuard gd(_mapLocker);
	return (int)_map.size();
}

int PlaylistManager::getSuicidePlCount()
{
	return m_pKillPlayList->getPlCount();
}

void PlaylistManager::getSpigotReplicaInfo( TianShanIce::Replicas& res )
{

	if( strlen( GAPPLICATIONCONFIGURATION.szServiceID ) <= 0 )
	{
		char szBuf[1024];
		memset( szBuf , 0, sizeof(szBuf) );
		gethostname(szBuf,sizeof(szBuf));
		strcpy( GAPPLICATIONCONFIGURATION.szServiceID , szBuf );
	}
	VstrmClass::VSTRMSPIGOTS spigots;
	_cls.getSpigotInfo( spigots );
	
	///use a separated thread pool to serve AMD only
	long maxPendingSize = gStreamSmithConfig.outOfServiceConf.maxPendingRequest;
	if( maxPendingSize <= 0 )
		maxPendingSize = mAmdThreadpool.size() * 2;	
	maxPendingSize = maxPendingSize < 10 ? 10 : maxPendingSize;
	long pendingSize = mAmdThreadpool.pendingRequestSize();
	
	bool bServiceOk = ( pendingSize < maxPendingSize );
	
	if( gStreamSmithConfig.outOfServiceConf.timeout  > 0 )
	{//OutOfService timeout configuration
		if( !bServiceOk )
		{
			if( mbDBRecordValid || (mPendingRequestLastCheckTime <= 0) )
			{
				mPendingRequestLastCheckTime	= ZQTianShan::now();
				mbDBRecordValid					= false;
			}
			else
			{
				Ice::Long cur = ZQTianShan::now();

				if( (cur - mPendingRequestLastCheckTime) > gStreamSmithConfig.outOfServiceConf.timeout )
				{				
					m_pPlayListStore->updateDbHealth( false );
					glog(ZQ::common::Log::L_EMERG,CLOGFMT(playlistManager,"the service timeout at state out-of-service due to thread leaks blocked by underlayer Vstream, force to quit the program in order to clean up"));
					ZQTianShan::Util::suicide();//kill StreamSmith
				}
			}		
		}
		else
		{
			if( !mbDBRecordValid )
			{
				mPendingRequestLastCheckTime	= ZQTianShan::now();
				mbDBRecordValid					= true;
				m_pPlayListStore->updateDbHealth( true );
			}
		}
	}


	if( !bServiceOk )
	{
		mbServiceAvailable = false;
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(PlaylistManager,"current threadpool pending requests[%ld], maxPendingSize[%ld], set all streamers as OutOfService"),
			pendingSize,maxPendingSize );
		mSysLogger(ZQ::common::Log::L_WARNING, CLOGFMT(PlaylistManager,"current threadpool pending requests[%ld], maxPendingSize[%ld], set all streamers as OutOfService"),
			pendingSize,maxPendingSize );
	}
	else
	{
		if( !mbServiceAvailable )
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(PlaylistManager,"stream service recovered"));
			mSysLogger(ZQ::common::Log::L_INFO, CLOGFMT(PlaylistManager,"stream service recovered"));
			mbServiceAvailable = true;
		}
	}

	TianShanIce::Replica rep;
	VstrmClass::VSTRMSPIGOTS::const_iterator it = spigots.begin();
	for( ; it != spigots.end() ; it ++ )
	{
		rep.category	= "Streamer";/*GAPPLICATIONCONFIGURATION.spigotReplicaConfig.category;*/
		
		rep.groupId		= GAPPLICATIONCONFIGURATION.szServiceID;//GAPPLICATIONCONFIGURATION.spigotReplicaConfig.groupId;
		//rep.disabled	= static_cast<bool>(  it->second.status != STATUS_READY );
		if( !bServiceOk )
		{
			rep.replicaState = TianShanIce::stOutOfService;			
		}
		else
		{
			rep.replicaState = (it->second.status != STATUS_READY) ? TianShanIce::stOutOfService : TianShanIce::stInService;
		}

		rep.maxPrioritySeenInGroup = 0;
		rep.priority	= 0;
		
		char szReplicaId[128];
		sprintf( szReplicaId , "Spigot%02d" , it->first);
		rep.replicaId	= szReplicaId;

		rep.stampBorn	= it->second.firstUp;
		rep.stampChanged= it->second.lastUpdate;

		Ice::Identity id;
		id.category		= "";
		id.name			= "StreamSmith";
		rep.obj			= m_Adapter->createProxy(id);
		rep.props		= it->second.props;
		res.push_back(rep);		
		
		/////////////////////////////////////////////////////////////
		rep.category	= "Streamer";/*GAPPLICATIONCONFIGURATION.spigotReplicaConfig.category;*/
		rep.groupId		= GAPPLICATIONCONFIGURATION.szServiceID;//GAPPLICATIONCONFIGURATION.spigotReplicaConfig.groupId;
		//rep.disabled	= static_cast<bool>(  it->second.status != STATUS_READY );
		rep.replicaState = (it->second.status != STATUS_READY) ? TianShanIce::stOutOfService : TianShanIce::stInService;
		rep.maxPrioritySeenInGroup = 0;
		rep.priority	= 0;

		/*char szReplicaId[128];*/
		sprintf( szReplicaId , "BoardNumber%02d" , it->first);
		rep.replicaId	= szReplicaId;

		rep.stampBorn	= it->second.firstUp;
		rep.stampChanged= it->second.lastUpdate;

		/*Ice::Identity id;*/
		id.category		= "";
		id.name			= "StreamSmith";
		rep.obj			= m_Adapter->createProxy(id);

		ZQTianShan::Util::mergeProperty( rep.props ,it->second.props );

		ZQTianShan::Util::updatePropertyData(rep.props,"sourceIp",it->second.sourceIp);
		ZQTianShan::Util::updatePropertyData(rep.props,"sourceBasePort",it->second.sourceBasePort);

		res.push_back(rep);		
	}
	
}

void PlaylistManager::getReplicaInfo(const std::string& category , const std::string& groupId ,
									 bool bLocalOnly , TianShanIce::Replicas& res )
{
	if( category.compare("Streamer") == 0 || category.compare("*") == 0 )
	{
		getSpigotReplicaInfo(res);
	}
	else if( category.compare("BandwidthUsage") == 0 )
	{
		BandwidthUsage bwUsage = _cls.getBwUsage();
		TianShanIce::Replicas reps;
		TianShanIce::Replica rep;
		rep.category	=	"BandwidthUsage";
		rep.groupId		=	GAPPLICATIONCONFIGURATION.spigotReplicaConfig.groupId;
		rep.replicaState=	TianShanIce::stInService;
		rep.replicaId	=	"";
		rep.priority	=	255;
		rep.obj			=	NULL;
		rep.stampBorn	=	0;
		rep.stampChanged=	0;



		std::ostringstream oss;

		oss << bwUsage.cdnUsedBandiwidth;
		rep.props["UsedReplicaImportBandwidth"] = oss.str();
		oss.str("");

		oss << bwUsage.cdnTotalBandwidth;
		rep.props["TotalReplicaImportBandwidth"] = oss.str();
		oss.str("");

		oss << bwUsage.cdnImportSessionCount;
		rep.props["runningImportSessionCount"]	= oss.str();
		oss.str("");

		res.push_back(rep);
	}
	if( category.compare("StreamService") == 0 || category.compare("*") == 0  )
	{
		TianShanIce::Replica rep;
		rep.category	= "StreamService";
		//rep.disabled	= false;
		rep.replicaState = TianShanIce::stInService;//(it->second.status != STATUS_READY) ? TianShanIce::stOutOfService : TianShanIce::stInService;
		char szClusterId[128];
		sprintf(szClusterId,"%d",GAPPLICATIONCONFIGURATION.mediaClusterId);
		rep.groupId		= szClusterId;
		rep.maxPrioritySeenInGroup = 0;
		Ice::Identity id;
		id.category		= "";
		id.name			= "StreamSmith";
		rep.obj			= m_Adapter->createProxy(id);
		rep.replicaId	= "";
		rep.stampBorn	= 0;
		rep.stampChanged= ZQTianShan::now();
		
		res.push_back(rep);
	}
}

bool PlaylistManager::StartFailOver(/*const std::string& dbPath,*/ZQ::common::Log* log)
{
	_sessmon.SetLogInstance(log);
	
	if(!m_pPlayListStore)
	{
		glog(Log::L_ERROR,"说什么好呢！");

		return false;
	}
	if(!m_pPlayListStore->Init(gStreamSmithConfig.szFailOverDatabasePath))
	{
		glog(Log::L_ERROR,"initliaze failover database fail");
		return false;
	}
	bool	bOK=false;
	try
	{
		bOK=m_pPlayListStore->SetupPlaylistInfo(PlaylistManager::plSetupCallback,
												PlaylistManager::pliSetupCallback,
												this);
	}
	catch (...) 
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","Unexpect exception occurred when \
										re-setup playlist info from failover database"));
		return false;
	}
	if(!bOK)
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","re-setup playlist info from failover database fail"));

		return false;
	}

#ifndef kVSTRM_BANDWIDTH_CLIENTID_ZQ_STREAMSMITH
	#define kVSTRM_BANDWIDTH_CLIENTID_ZQ_STREAMSMITH	0x04000000
#endif

	{
		//collect current in-use tickets and remove all idle tickets
		VSTRM_BANDWIDTH_BCB_WRAPPER		bcb;
		std::vector<ULONG64>			runningTickets;
		VSTATUS							vret = VSTRM_SUCCESS;
		ULONG							curIndex = 0;
		ULONG64							ticket = 0;
		while( vret == VSTRM_SUCCESS )
		{
			memset(&bcb,0,sizeof(bcb));

			vret =  VstrmClassGetNextBandwidthTicketHolder( _cls.handle() , &bcb  , sizeof(bcb) ,&ticket, &curIndex );
			if( ticket == (ULONG64)-1 )
				break;
			if( VSTRM_SUCCESS != vret )
			{				
				break;
			}
			else if ( bcb.Bcb.ClientId == kVSTRM_BANDWIDTH_CLIENTID_ZQ_STREAMSMITH )
			{
				runningTickets.push_back( bcb.Bcb.Ticket );				
			}
		}
		std::sort( runningTickets.begin() , runningTickets.end() );
		std::sort( ticketsInDb.begin() , ticketsInDb.end() );
		std::vector<ULONG64> orphanTickets;
		std::set_difference(	runningTickets.begin() , runningTickets.end(),
								ticketsInDb.begin(),ticketsInDb.end(),
								std::inserter< std::vector<ULONG64> >( orphanTickets , orphanTickets.begin( ) ));
		ticketsInDb.clear();
		std::vector<ULONG64>::const_iterator it = orphanTickets.begin();
		for( ; it != orphanTickets.end() ; it ++ )
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(PlaylistManager,"remove orphan ticket[%llx]"),*it);
			VstrmClassReleaseBandwidth(_cls.handle(),*it);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	return true;
}
void PlaylistManager::DeleteFailOverDbPath(std::string dbPath)
{
	WIN32_FIND_DATAA	fData;
	ZeroMemory(&fData,sizeof(fData));
	std::string	strFind=dbPath;
	if(strFind[strFind.length()-1]=='\\')
		strFind=strFind+"*.*";
	else
		strFind=strFind+"\\*.*";
	if(dbPath[dbPath.length()-1]!='\\')
		dbPath+="\\";		
	HANDLE	hFile=FindFirstFileA(strFind.c_str(),&fData);
	if(INVALID_HANDLE_VALUE!=hFile)
	{
		bool bOK=false;
		do 
		{
			bOK=FindNextFileA(hFile,&fData);
			if(bOK)
			{
				if(fData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
					continue;
				
				if(!DeleteFileA((dbPath+fData.cFileName).c_str()))
				{
					DWORD dwTestErr=GetLastError();
				}
			}
			
		} while(bOK);
		FindClose(hFile);
	}
}
void PlaylistManager::ClearFailOverInfo(ZQ::common::Guid& uid)
{
	char	szBuf[128];
	ZeroMemory(szBuf,128);
	uid.toString(szBuf,127);
	//Delete playlistEx servant directly	
	m_pFailStoreThread->AddClearPlaylistInfo(szBuf,true,true);
//	m_pPlayListStore->ClearPlaylistAttr(szBuf);
//	m_pPlayListStore->ClearPlaylistItemAttr(szBuf);
}
IPlaylistEx*	PlaylistManager::find(const std::string uidString)
{
	return (IPlaylistEx*)find(ZQ::common::Guid(uidString.c_str()));
}
void PlaylistManager::listPlaylistGuid(::std::vector<::std::string>& IDs)
{
	ZQ::common::MutexGuard gd(_mapLocker,__MSGLOC__);
	IDs.clear();
	char szBuf[128];
	for(Map::iterator it=_map.begin();it!=_map.end();it++)
	{
		ZeroMemory(szBuf,sizeof(szBuf));
		it->first.toString(szBuf,sizeof(szBuf));
		IDs.push_back(szBuf);
	}
}
void PlaylistManager::addNewPlaylistIntoFailOver(const std::string& strGuid ,	const PlaylistAttr& attr)
{
	m_pFailStoreThread->AddNewPlaylistIntoFailOver(strGuid,attr);
	//m_pPlayListStore->UpdatePlaylistAttr(strGuid,attr);
}

#endif//_ICE_INTERFACE_SUPPORT

int PlaylistManager::run(void)
{
	while (_cls.isValid() && !_bQuit && NULL != _hEvtWakeup)
	{
		if (_timeout!=_UI64_MAX && _timeout <0)
			_timeout =0;
		switch(WaitForSingleObject(_hEvtWakeup, (int)_timeout-1))
		{
		case WAIT_OBJECT_0:
		case WAIT_TIMEOUT:
			{
				if (_bQuit)
					return 0;

				_timeout = TIMER_MAX_TIMEOUT;
				ZQ::common::MutexGuard gd(_mapLocker,__MSGLOC__);
				for (Map::iterator it =_map.begin(); it != _map.end(); it++)
				{
					Playlist* pl = it->second.pl;
					if (_bQuit)
						return 0;

					if (NULL != pl)
					{						
						timeout64_t to = pl->getTimer64();
						if (_UI64_MAX == to)
							continue;
						else if (0 == to)
						{
							glog(ZQ::common::Log::L_DEBUG, LOGFORMAT("PlaylistManager","create a TimerRequest for playlist %s"),pl->_strGuid.c_str());
							TimerRequest* treq = new TimerRequest((*pl)._guid, *this,_cls.getThreadPool());
							
							//在这里停止当前的Timer
							pl->endTimer64();
							//////////////////////////////////////////////////////////////////////////							
							if (treq)
								treq->start();
						}
						else if ((int)_timeout > to)
							_timeout = to;
					}
				}
			}

			break;
		case WAIT_ABANDONED:
		default:
			return -1;
		}
	}

	return 0;
}
	
void PlaylistManager::final(int retcode, bool bCancelled)
{	
}

void PlaylistManager::wakeup(void)
{
	if (_hEvtWakeup)
		::SetEvent(_hEvtWakeup);
}

VHANDLE PlaylistManager::classHandle() const
{
	return _cls.handle();
}
bool	PlaylistManager::GetSpigotIDsFromResource(int serviceGroupID,int maxBitRate,std::vector<int>& SpigotIDs)
{
	return m_ResourceManager.GetSpigotIDsFromResource(serviceGroupID,maxBitRate,SpigotIDs);
}
bool	PlaylistManager::GetResource(int serviceGroupdID,long needBW, 
							std::string&	strIP,
							int&			Port,
							std::string&	strMac,
							int&			ProNum,
							int&			Frequency,
							int&			ChannelID,
							ZQ::common::Guid& uid,
							int& qamMode)
{
	ResourceMan::ResourceAlloc rOut;
	if(!m_ResourceManager.GetQamResource(serviceGroupdID,needBW,rOut,uid))
	{
		glog(Log::L_ERROR,LOGFORMAT("PlaylistManager","Can't Alloc resource for service \
							group iD=%d need band width=%d(kbps)"),serviceGroupdID,needBW);
		return false;
	}
	strIP=rOut._QamIP;
	Port=rOut._Port;
	ProNum=rOut._ProgramNumber;
	Frequency=rOut._Frequency;
	ChannelID=rOut._ChannelID;
	strMac=rOut._QamMac;
	qamMode=rOut._QamMode;

	char	szBuf[128];
	ZeroMemory(szBuf,128);
	uid.toString(szBuf,127);
	glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","get a resource for service group ID=%d and need BW=%d\
						result(IP:%s Port:%d ProNum(%d) Frequency:%d qamMode:%d guid:%s"),
						serviceGroupdID,needBW,strIP.c_str(),Port,ProNum,Frequency,qamMode,szBuf);
	return true;
}
bool PlaylistManager::FreeResource(ZQ::common::Guid& uid)
{
	char	szBuf[128];
	ZeroMemory(szBuf,128);
	uid.toString(szBuf,127);
	glog(Log::L_DEBUG,"free resource with uid=%s",szBuf);
	return m_ResourceManager.FreeQamResource(uid);
}
#ifdef _ICE_INTERFACE_SUPPORT
void PlaylistManager::listStreamers(std::vector<int >& SpigotIDs)
{
	_cls.listStreamer(SpigotIDs);
}
#endif//_ICE_INTERFACE_SUPPORT


#ifdef _ICE_INTERFACE_SUPPORT
//void PlaylistManager::PlayListFailOverCallback(const std::string& guidStr , const PlaylistAttr& plAttr,void* pData)
//{
//	PlaylistManager* pThis=(PlaylistManager*)pData;
//	pThis->GetPlaylistInfoFromIce(guidStr,plAttr);
//}
//void PlaylistManager::PlaylistItemFailOverCallback(const std::string& guidStr , const PlayListItemAttr& pliAttr,void* pData)
//{
//	PlaylistManager* pThis=(PlaylistManager*)pData;
//	pThis->GetPlaylistItemInfoFromIce(guidStr,pliAttr);
//}
//void PlaylistManager::GetPlaylistInfoFromIce(const std::string& guidStr , const PlaylistAttr& plAttr)
//{
//	ZQ::common::Guid	uid(guidStr.c_str());
//	Playlist* pl=find(guidStr.c_str());
//	if(NULL==pl)
//	{//there is already a playlist with the guid
//		return;
//	}
//	else
//	{		
//		StreamSmithSite* pSite=StreamSmithSite::findSite(plAttr.StreamSmithSiteName.c_str());
//		if(NULL==pSite)
//		{
//			glog(Log::L_ERROR,"void PlaylistManager::GetPlaylistInfoFromIce()##Can't find streamsmith site with name=%s",plAttr.StreamSmithSiteName.c_str());
//			return;
//		}
//		pl= new Playlist(pSite,*this,uid);
//		if(!pl)
//		{
//			glog(Log::L_CRIT,"void PlaylistManager::GetPlaylistInfoFromIce()##Can't create playlist");
//			return;
//		}
//		///copy attribute
//		pl->_ResourceGuid=ZQ::common::Guid(plAttr.ResourceGuid.c_str());
//		pl->setUserCtxIdx(plAttr.ClientSessionID.c_str());		
//		pl->setAuth(plAttr.endPoint.c_str());
//		pl->setMuxRate((long)plAttr.NowRate,(long)plAttr.MaxRate,(long)plAttr.MinRate);
//		pl->setDestination((char*)plAttr.destIP.c_str(),(long)plAttr.destPort);
//		if(plAttr.destMac.length()>1)
//		{
//			pl->setDestMac((char*)plAttr.destMac.c_str());
//		}
//		
//		///may be wrong
//		pl->_vstrmPortNum=(long)plAttr.vstrmPort;
//		pl->_currentStatus=(Playlist::State)plAttr.playlistState;		
//	}
//}
//void PlaylistManager::GetPlaylistItemInfoFromIce(const std::string& guidStr , const PlayListItemAttr& pliAttr)
//{
//	ZQ::common::Guid  uid(guidStr.c_str());
//	IPlaylist::Item	item;
//	ZeroMemory(&item,sizeof(item));
//	IPlaylist* pList=StreamSmithSite::getDefaultSite()->openPlaylist(uid,false);
//	if(!pList)
//	{
//		glog(Log::L_ERROR,"void PlaylistManager::GetPlaylistItemInfoFromIce()##Can't open playlist with guid=%s",guidStr.c_str());
//		return;
//	}
//	strncpy(item._fileName,pliAttr.FileName.c_str(),sizeof(item._fileName)-1);
//	item._criticalStart=(time_t)pliAttr.CriticalStart;
//	item._forceNormal=pliAttr.foreceNormalSpeed;
//	item._inTimeOffset=(uint32)pliAttr.InTimeOffset;
//	item._outTimeOffset=(uint32)pliAttr.OutTimeOffset;
//	item._spliceIn=pliAttr.SpliceIn;
//	item._spliceOut=pliAttr.SpliceOut;
//	item._whereInsert=(CtrlNum)pliAttr.InternalCtrlNum;//Use this var to control the sequence of the play list item
//	pList->push_back(item);
//}
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
PlaylistSuicide::PlaylistSuicide()
{
	m_pStreamSite=StreamSmithSite::_pDefaultSite;
	m_hQuitHandle=CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hQuittedHanlde=CreateEvent(NULL,FALSE,FALSE,NULL);
	_KillerThreadInterval=100000;
}
PlaylistSuicide::~PlaylistSuicide()
{
	CloseHandle(m_hQuitHandle);
	CloseHandle(m_hQuittedHanlde);
}
int PlaylistSuicide::run()
{
	DWORD	dwResult=0;
	bool	bQuit=false;
	while (!bQuit) 
	{
		dwResult=WaitForSingleObject(m_hQuitHandle,_KillerThreadInterval);
		switch(dwResult)
		{
		case WAIT_OBJECT_0:
			{
				bQuit=true;
			}			
		default:
			{
				//Kill playlist
				if(!m_pStreamSite)
				{
					glog(Log::L_ERROR,"NO StreamSmithSite for using");
					break;
				}
				VECSuicidePlayList::iterator it=m_vecList.begin();				
				bool bSearchAll=false;
				DWORD	dwTestTime=GetTickCount();
				do 
				{
					if(it==m_vecList.end())
					{
						bSearchAll=true;
					}
					else
					{
						ZQ::common::MutexGuard gd(m_ListLocker,__MSGLOC__);
						glog(ZQ::common::Log::L_DEBUG, LOGFORMAT("PlaylistManager","current idle playlist count[%u]"), m_vecList.size() );
						for(it=m_vecList.begin();it!=m_vecList.end();)
						{
							IClientSession* pSession=m_pStreamSite->findClientSession( (it->_pl)->m_strClientSessionID.c_str() );
							if(pSession)
							{//Session was found
								pSession->release();
								it++;
								if(it==m_vecList.end())
								{
									bSearchAll=true;
								}
							}
							else
							{//NO session
								
								//if the ref > 0,delete it in the next run							
								
								if(it+1==m_vecList.end())
								{
									bSearchAll=true;
								}
								if(!bQuit && !it->_bCleanOver)
								{
									it->_bCleanOver=true;
									it++;
								}
								else
								{
									if (it->_pl->ref() >0 )
									{
										glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","delete playlist %p but ref is %d, delete later"),it->_pl, it->_pl->ref());
										it++;
										continue;
									}
#pragma message(__MSGLOC__"现在需要扫描两次才能kill一个playlist")
									glog(Log::L_DEBUG,LOGFORMAT("PlaylistManager","delete playlist %p"),it->_pl);
									try
									{
										if(it->_pl)
											delete (it->_pl);								
									}
									catch (...)
									{
									}
									int iTemp=it-m_vecList.begin();
									m_vecList.erase(it);
									it=m_vecList.begin()+iTemp;										
								}
							}
						}					
					}
				} while(!bSearchAll);

				if(m_vecList.size()>0)
					glog(Log::L_INFO,"killing playlist thread run time=%ld and current suicide list size is %d",GetTickCount()-dwTestTime,m_vecList.size());
				if(m_vecList.size()>0)
				{
					for(it=m_vecList.begin();it!=m_vecList.end();it++)
					{
						glog(Log::L_DEBUG,"play list with Session iD=%s is still alive",(it->_pl)->m_strClientSessionID.c_str() );
					}
				}
			}
			break;
		}
	}
	
	SetEvent(m_hQuittedHanlde);
	return 1;
}
void PlaylistSuicide::PushPlaylist(Playlist* pl)
{
	ZQ::common::MutexGuard gd(m_ListLocker,__MSGLOC__);
	VECSuicidePlayList::iterator it;
	for(it=m_vecList.begin();it!=m_vecList.end();it++)
	{
		if(it->_pl==pl)
		{
			//The playlist is already in the vector
			return;
		}
	}
	plPropery p;
	p._bCleanOver=false;
	p._pl=pl;
	m_vecList.push_back(p);
}

///spigotReplicaReport

spigotReplicaReport::spigotReplicaReport( PlaylistManager& plMan )
:_plManager(plMan)
{
	bServiceUp = true;
	_bQuit = false;
	_hQuitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}
spigotReplicaReport::~spigotReplicaReport()
{
	if(_hQuitEvent)
	{
		CloseHandle(_hQuitEvent);
		_hQuitEvent = NULL;
	}
}
void spigotReplicaReport::stop()
{
	if( !_bQuit )
	{
		_bQuit = true;
		SetEvent(_hQuitEvent);
		waitHandle(5000);
	}
}

void spigotReplicaReport::setServiceState( bool serviceStatus )
{
	bServiceUp = serviceStatus;
	if(!bServiceUp)
	{
		stop();
		//report replica status before quit
		runTimerTask();
	}
}

int spigotReplicaReport::run()
{
	bServiceUp = true;
	updateInterval = defaultUpdateInterval;
	while( !_bQuit )
	{
		runTimerTask();
		if(_bQuit)
			break;
		WaitForSingleObject( _hQuitEvent ,  updateInterval );
	}
	return 1;
}

void spigotReplicaReport::runTimerTask( )
{
	//collect replicas
	::TianShanIce::Replicas  reps;
	_plManager.getSpigotReplicaInfo(reps);

	if( listenerEndpoint.empty() )
	{
		updateInterval = 60 * 60 * 1000;
		glog(ZQ::common::Log::L_WARNING,CLOGFMT(spigotReplicaReport,"no listener endpoint , do not update replica information to subscriber"));
		return;
	}
	//step 1
	//connect to replica listener
	TianShanIce::ReplicaSubscriberPrx	subscriber = NULL;

	std::string		strListenerEndpoint ;
	if( listenerEndpoint.find(":") != std::string::npos )
	{
		strListenerEndpoint = listenerEndpoint;
	}
	else
	{
		strListenerEndpoint = ADAPTER_NAME_PathManager":" + listenerEndpoint;
	}

	try
	{
		subscriber	= TianShanIce::ReplicaSubscriberPrx::checkedCast( 
			_plManager.m_Adapter->getCommunicator()->stringToProxy(strListenerEndpoint) 
			);
		if( !subscriber )
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT( spigotReplicaReport , "can't connect to replica listener[%s]"),
				strListenerEndpoint.c_str() );
		}
	}
	catch( const Ice::Exception& ex)
	{//If I catch an exception
		//I should quit the report process
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(spigotReplicaReport,"caught Ice Exception:%s when connect to replica listener[%s]"),
			ex.ice_name().c_str() ,
			strListenerEndpoint.c_str()	);
		subscriber = NULL;
	}

	
	if( !bServiceUp )
	{
		::TianShanIce::Replicas::iterator it = reps.begin();
		for( ; it != reps.end() ; it ++ )
		{
			it->replicaState = TianShanIce::stOutOfService;
		}
	}

	//report it to subscriber
	int iNextReportInterval = defaultUpdateInterval;
	std::map<std::string,std::string> replicaStatusMap;

	TianShanIce::Replicas::const_iterator itRep =  reps.begin();
	for( ; itRep != reps.end() ; itRep++ )
	{
		const TianShanIce::Replica& rep = *itRep;
		replicaStatusMap[rep.groupId+'/'+rep.replicaId] = rep.replicaState == TianShanIce::stInService ? "UP" : "DOWN";
	}

	try
	{
		if( subscriber )
		{
			iNextReportInterval = subscriber->updateReplica( reps ) * 500;
			if( iNextReportInterval <= 0 )
			{
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(spigotReplicaReport,"subscriber return the update interval is [%d] , adjust it to [%d]") ,
					iNextReportInterval ,
					defaultUpdateInterval );
				iNextReportInterval = defaultUpdateInterval;
			}
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(spigotReplicaReport,"update replica status {%s} to [%s] ok, and interval for next report is [%d]"),
				ZQTianShan::Util::dumpStringMap(replicaStatusMap).c_str(),  strListenerEndpoint.c_str() ,iNextReportInterval);
		}
		else
		{
			glog(ZQ::common::Log::L_INFO,
				CLOGFMT(spigotReplicaReport,"failed to update replica to [%s] , and interval for next report is [%d] "),
				strListenerEndpoint.c_str(), iNextReportInterval );
		}
	}
	catch( const Ice::Exception& ex )
	{
		glog(ZQ::common::Log::L_ERROR,
			CLOGFMT(spigotReplicaReport,"failed to report replicas status to subscriber and caught ice exception:[%s] and interval for next report is [%d] "),
			ex.ice_name().c_str() ,iNextReportInterval);
	}

	///update timer so that we can report the replicas again
// 	if( iNextReportInterval <= 10*1000 )
// 		iNextReportInterval = 10 * 1000;

	//IceUtil::Time interval= IceUtil::Time::milliSeconds( iNextReportInterval );
	updateInterval = iNextReportInterval;

	//_plManager.m_spigotReplicaTimer->schedule( new spigotReplicaReport( _plManager,defaultUpdateInterval, listenerEndpoint ) , interval );
}


}}//namespace
