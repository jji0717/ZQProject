// MRTStreamService.cpp: implementation of the MRTStreamSmith class.
//
//////////////////////////////////////////////////////////////////////

#include "MRTStreamService.h"
#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#endif

#include <TianShanDefines.h>
#include <TianShanIceHelper.h>

#if ICE_INT_VERSION/100 < 303
	#include <Ice/IdentityUtil.h>
#endif

#include "Guid.h"
#include "Log.h"
#include <time.h>
#include "FileSystemOp.h"
#include "Variant.h"
#include "MRTClient.h"

extern MRTProxy*	_serviceInstance;

ZQ::common::NativeThreadPool gPool(30);

using namespace ZQ::common;

void tempPause( )
{
	uint dwPauseTime = pauseMin + rand()%( pauseMax - pauseMin );
	SYS::sleep(dwPauseTime);
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///DBStore
#define TRY_BEGIN()	try {
#define TRY_END(_PROMPT)	}	catch(const Ice::Exception& ex) \
{ std::ostringstream s; s << _PROMPT;ex.ice_print(s);glog(Log::L_ERROR,"%s",s.str().c_str());}\
catch(...){ std::ostringstream s; s << _PROMPT;glog(Log::L_ERROR,"%s Unexpect error",s.str().c_str());return false;}

#define ICEDBFMT(x)	"[IceDB] "x

DBStore::DBStore(Ice::ObjectAdapterPtr& objAdapter)
{
	m_pCommunicatorPtr=objAdapter->getCommunicator();
	m_pAdpter=objAdapter;
}
DBStore::~DBStore()
{
}
#ifdef ZQ_OS_MSWIN
#define		PATHDELIMETER '\\'
#else
#define		PATHDELIMETER '/'
#endif
bool DBStore::Init(const std::string& dbPath)
{
	std::string	db=dbPath;
	if(db.empty ())
	{
#ifdef ZQ_OS_MSWIN
		  db="c:\\db\\";
#else
		  db = "/opt/TianShan/data";
#endif
	}
	if(db.at(db.length()-1) != PATHDELIMETER)
	{
		db+=PATHDELIMETER;
	}
	db+="Stream";
	FS::createDirectory(db.c_str());
	TRY_BEGIN();
	m_pConnectionPtr=Freeze::createConnection (m_pCommunicatorPtr,db);	
	if(!m_pConnectionPtr)
		return false;
	{
		try
		{
#if ICE_INT_VERSION / 100 >= 303			
			m_DBEvictor=Freeze::createBackgroundSaveEvictor(m_pAdpter,db,PURCHASE_CATAGORY);
#else
			m_DBEvictor=Freeze::createEvictor(m_pAdpter,db,PURCHASE_CATAGORY);
#endif
			
		}
		catch(...)
		{
			glog(Log::L_ERROR,ICEDBFMT("Unexpect error when create evictor"));
		}
		m_pAdpter->addServantLocator(m_DBEvictor,PURCHASE_CATAGORY);
		m_DBEvictor->setSize(1000);
	}
	
	TRY_END("bool DBStore::Init##");
	return true;
}
bool DBStore::UpdateStreamInstance( TianShanIce::Streamer::StreamPtr& streamPtr,Ice::Identity& id)
{	
	if(!streamPtr)
	{
		glog(Log::L_ERROR,ICEDBFMT("null purchase pointer passed in"));
		return false;
	}
	TRY_BEGIN()
		ZQ::common::MutexGuard gd(m_dbLocks,__MSGLOC__);
		Freeze::TransactionHolder holder(m_pConnectionPtr);
		if(!m_DBEvictor->hasObject(id))
		{		
			streamPtr->ident=id;
			Ice::ObjectPrx prx= m_DBEvictor->add(streamPtr,id);			
			glog(ZQ::common::Log::L_DEBUG,ICEDBFMT("add new record with category=%s name=%s"), id.category.c_str(),id.name.c_str());		
		}
		else
		{			
			TianShanIce::Streamer::StreamPrx prx=TianShanIce::Streamer::StreamPrx::checkedCast(m_pAdpter->createProxy(id));			
		}		
		holder.commit ();
	TRY_END("bool DBStore::UpdateStreamInstance()");
	return true;
}
bool DBStore::ClearStreamInstance(const Ice::Identity& id)
{
	TRY_BEGIN()
		ZQ::common::MutexGuard gd(m_dbLocks,__MSGLOC__);
		Freeze::TransactionHolder holder(m_pConnectionPtr);
		
		m_DBEvictor->remove(id);
	
		holder.commit();
	TRY_END("bool DBStore::ClearStreamInstance()");
	return true;		
}
bool DBStore::ClearAll()
{
	TRY_BEGIN()
		ZQ::common::MutexGuard gdpl(m_dbLocks,__MSGLOC__);		
		Freeze::TransactionHolder holder(m_pConnectionPtr);
				
		typedef std::vector<Ice::Identity>	RESULT;
		::Freeze::EvictorIteratorPtr itPtr=m_DBEvictor->getIterator("",1);
		RESULT r;
		r.clear();
		if(itPtr)
		{
			while (itPtr->hasNext()) 
			{
				r.push_back(itPtr->next());
			}
			for(RESULT::iterator it=r.begin();it!=r.end();it++)
			{
				m_DBEvictor->remove(*it);				
			}
		}
		holder.commit();
	TRY_END("bool DBStore::ClearAll()##");
	return true;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


MRTProxy::MRTProxy(Ice::ObjectAdapterPtr& Adapter,const std::string& dbPath ,const std::string& netId,const std::vector<std::string>& streamers, const StreamNetIDToMRTEndpoints& mrtEndpintInfos, ZQ::common::Log& log,int target )
	:mWatchDog(this),m_Adapter(Adapter),_db(Adapter),mNetId(netId),mStreamers(streamers), _maxPenalty(100), _outOfServicePenalty(20), _mrtEndpintInfos(mrtEndpintInfos), _log(log)
{
	mWatchDog.start();
	RLock sunc(*this);
	_db.Init(dbPath);
	_nextWakeup=ZQTianShan::now()+0xffff;
	srand((unsigned int)time(NULL));
	//_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	_bQuit = false;
	start();
	_factory= new FailOverFactory(m_Adapter,this);
	m_Adapter->getCommunicator()->addObjectFactory(_factory,TianShanIce::Streamer::Stream::ice_staticId());
	m_Adapter->getCommunicator()->addObjectFactory(_factory,TianShanIce::Streamer::Playlist::ice_staticId());
	m_Adapter->getCommunicator()->addObjectFactory(_factory,TianShanIce::Streamer::PlaylistEx::ice_staticId());
	targetTime = target;
//	_stream=new MRTStream;
//	m_Adapter->add(_stream,Ice::stringToIdentity("stream"));
}

MRTProxy::~MRTProxy()
{
	mWatchDog.stop();
	RLock sunc(*this);
	_bQuit= true;
	//SetEvent(_hEvent);
	_sigleEvent.signal();
	waitHandle(1000);
	//CloseHandle(_hEvent);
	_stream=NULL;
}
bool MRTProxy::UpdateStreamInstance(TianShanIce::Streamer::StreamPtr& streamPtr,
										Ice::Identity& id,
										const TianShanIce::Transport::PathTicketPrx& ticket)
{
	_db.UpdateStreamInstance(streamPtr,id);
	{
		ZQ::common::MutexGuard gd(_mapMutex);
		int tempTime = rand()%100+2;
		Ice::Long nextWakeup = ZQTianShan::now() + tempTime;
		TimerProperty p;
		p._nextWakeup=nextWakeup;
		p._ticketPrx = ticket;
		_map[id.name] = p;
		if(nextWakeup < _nextWakeup)
			_sigleEvent.signal();
			//SetEvent(_hEvent);
	}
	return true;
}

bool MRTProxy::ClearStreamInstance(const Ice::Identity& id)
{
	{
		ZQ::common::MutexGuard gd(_mapMutex);
		_map.erase(id.name);
	}
	_db.ClearStreamInstance(id);
	return true;
}

int	 MRTProxy::run()
{
	//updateInterval = 0;
	int		waittime=0xFF;	
	Ice::Long current=ZQTianShan::now();
	_nextWakeup = current+waittime;
	while (!_bQuit) 
	{
		//modified by lxm for report dummySS status
		//if (waittime > updateInterval)
		//	waittime = updateInterval;
		//WaitForSingleObject(_hEvent,waittime);	
		_sigleEvent.wait(waittime);
		if(_bQuit)
			break;

		//reportSpigot();

		{
			ZQ::common::MutexGuard gd(_mapMutex);
			current = ZQTianShan::now();
			_nextWakeup=current+0xffff;
			TimerMap::iterator it=_map.begin();
			for( ; it!=_map.end() ;it++)
			{
				if(it->second._nextWakeup <= current)
				{
					int renewTime=rand()%150000+20000;
					(new RenewTicket(it->second._ticketPrx,renewTime,gPool))->start();
					it->second._nextWakeup=renewTime-10000+current;
				}
				::Ice::Long lTemp= it->second._nextWakeup;
				_nextWakeup =_nextWakeup > it->second._nextWakeup ? it->second._nextWakeup : _nextWakeup;			
			}
			waittime=(int)(_nextWakeup- ZQTianShan::now());
			if(waittime<100)
				waittime=100;
		}
	}
	return 1;
}
TianShanIce::Streamer::StreamPrx MRTProxy::createStream(const ::TianShanIce::Transport::PathTicketPrx& ticket,
															const ::Ice::Current& )
{	
	//add some thing into database
	///PurchseID或者BcastChID作为Stream的ID
	std::string streamID = "";

	//check the resource first	
	try
	{
		if (ticket) 
		{	
			std::string  strTicketID = ticket->getIdent().name;
			TianShanIce::ValueMap data = ticket->getPrivateData();
			TianShanIce::SRM::ResourceMap  resources = ticket->getResources();

			if(resources.find(TianShanIce::SRM::rtServiceGroup) != resources.end() 
				&& resources[TianShanIce::SRM::rtServiceGroup].resourceData.find("deliveryId") != resources[TianShanIce::SRM::rtServiceGroup].resourceData.end() 
				&& !resources[TianShanIce::SRM::rtServiceGroup].resourceData["deliveryId"].strs.empty())
			{
				streamID = resources[TianShanIce::SRM::rtServiceGroup].resourceData["deliveryId"].strs[0];
			}
		}
	}
	catch (const Ice::Exception& ex)
	{
		
	}
	catch (...)
	{
	}

	tempPause();
	MRTStream* s = new MRTStream(m_Adapter,this, targetTime);
	
	TianShanIce::Streamer::StreamPtr streamptr = s; 
	Ice::Identity id;	

	if(streamID.empty())
	{
		ZQ::common::Guid uid;
		uid.create();
		char	szBuf[128];
		memset(szBuf,0,sizeof(szBuf));
		uid.toString(szBuf,127);
		streamID = szBuf;
	}
	id.name = streamID;
	id.category = PURCHASE_CATAGORY;
	_serviceInstance->UpdateStreamInstance(streamptr,id,ticket);
	
	TianShanIce::Streamer::StreamPrx prx=TianShanIce::Streamer::StreamPrx::checkedCast(m_Adapter->createProxy(id));	
	return prx;
}
::TianShanIce::Streamer::StreamPrx MRTProxy::createStreamByResource(const ::TianShanIce::SRM::ResourceMap&, const TianShanIce::Properties &,const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	//check the resource first
	//WLock sync(*this);	
	tempPause();
	
	TianShanIce::Streamer::StreamPtr streamptr= new MRTStream(m_Adapter,this, targetTime);	
	//add some thing into database

	///PurchseID或者BcastChID作为Stream的ID
	std::string streamID = "";
	Ice::Identity id;	
	id.name = streamID;
    id.category = PURCHASE_CATAGORY;
	_serviceInstance->UpdateStreamInstance(streamptr,id,NULL);
	TianShanIce::Streamer::StreamPrx prx=TianShanIce::Streamer::StreamPrx::checkedCast(m_Adapter->createProxy(id));	
	return prx;
}	

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
MRTStream::MRTStream( Ice::ObjectAdapterPtr adapter, MRTProxy* service,  int targettime)
:m_Adapter(adapter),
m_Service(service){	
	RLock sunc(*this);
	m_lastState = TianShanIce::Streamer::stsStreaming;
	m_lastSpeed = 1.0;
	m_targetTime = targettime;
}

MRTStream::~MRTStream()
{
	RLock sunc(*this);
}

void MRTStream::sendSpeedChange( ) {
	try {
		float nextSpeed = fabs(m_lastSpeed - 1.0001)<=0.001 ? 2.0 : 1.0;
		std::string proxyString = m_Adapter->getCommunicator()->proxyToString( m_Adapter->createProxy(ident) );
		m_Service->mStreamEventPrx->OnSpeedChanged( proxyString, ident.name, m_lastSpeed, nextSpeed,	TianShanIce::Properties() );	
		m_lastSpeed=nextSpeed;
	}
	catch( const Ice::Exception& ex ) {
		printf("caught [%s] while send speed change for [%s]\n",
			ex.ice_name().c_str(), ident.name.c_str() );
	}
}

void MRTStream::sendStateChange( ) {
	try {
		TianShanIce::Streamer::StreamState nextState = m_lastState == TianShanIce::Streamer::stsStreaming ? 
			TianShanIce::Streamer::stsPause : TianShanIce::Streamer::stsStreaming;

		std::string proxyString = m_Adapter->getCommunicator()->proxyToString( m_Adapter->createProxy(ident) );
		m_Service->mStreamEventPrx->OnStateChanged( proxyString , ident.name , 
			m_lastState,
			nextState,
			TianShanIce::Properties() );
		m_lastState = nextState;
	}
	catch( const Ice::Exception& ex) {
		printf("caught [%s] while send speed change for [%s]\n",
			ex.ice_name().c_str(), ident.name.c_str() );
	}
}

void MRTStream::onTimer( const ::Ice::Current& /*= ::Ice::Current() */) {
	{
		WLock sync(*this);
		sendSpeedChange();
		sendStateChange();
	}
	if (m_targetTime > 0)
		m_Service->mWatchDog.watch(ident.name, ZQ::common::now() + 10 * m_targetTime + rand()% 30 * 1000);
}

void MRTStream::allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx&, const ::Ice::Current&/* = ::Ice::Current()*/) 
{
	RLock sunc(*this);
}
::Ice::Long MRTStream::seekStream(::Ice::Long, ::Ice::Int, const ::Ice::Current& /* = ::Ice::Current( */)
{
	RLock sunc(*this);
	tempPause();
	return 1;
}
void MRTStream::destroy(const ::Ice::Current& c) 
{
	::TianShanIce::Properties feedback; // simply throw away feedback returned from destory2()
	 destroy2(feedback, c);
}

void MRTStream::destroy2(::TianShanIce::Properties& feedback, const ::Ice::Current& c)
{
	{
		WLock sunc(*this);
		_serviceInstance->ClearStreamInstance(ident);
	}

	m_Service->mWatchDog.unwatch(ident.name);
	std::string streamID = ident.name;

	std::string mrtSessionId;
	std::string srmSessionId;
	std::string streamNetId;
	bool bRet = m_Service->findMRTSessionIdByStreamId(streamID, mrtSessionId, srmSessionId,streamNetId);
	if(!bRet)
	{
		return;
	}

	MRTEndpointInfo mrtEndpointInfo;
	bRet = m_Service->findMRTEnpoint(streamNetId, mrtEndpointInfo);
	if(!bRet)
	{
		return;
	}

	ZQMRT::MRTClient mrtClient(m_Service->getLogger());
	std::string  errorMsg;
	bRet = mrtClient.teardown(mrtEndpointInfo, mrtSessionId, srmSessionId, errorMsg);
	if(!bRet)
	{
		return;
	}
}

::std::string MRTStream::lastError(const ::Ice::Current&/* = ::Ice::Current()*/) const 
{
	RLock sunc(*this);
	return "Nothing";
}


::Ice::Identity MRTStream::getIdent(const ::Ice::Current&/* = ::Ice::Current()*/) const
{
	RLock sunc(*this);

	return ident;
}


void MRTStream::setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask&, const ::TianShanIce::Streamer::ConditionalControlPrx&, const ::Ice::Current& /*= ::Ice::Current()*/)
{
	RLock sunc(*this);

}


bool MRTStream::play(const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	RLock sunc(*this);

	if(m_targetTime > 0)
		m_Service->mWatchDog.watch(ident.name, ZQ::common::now()+m_targetTime);

	std::string errorMsg;
	bool bret = setupMRTStream(errorMsg);
	if(!bret)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(m_Service->getLogger(), "MRTStream", 5003, CLOGFMT(MRTStream,
			"[%s]%s"), ident.name.c_str(), errorMsg.c_str());
	}
	return bret;
}


bool MRTStream::setSpeed(::Ice::Float, const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	RLock sunc(*this);
	tempPause();
	return true;
}


bool MRTStream::pause(const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	RLock sunc(*this);
	tempPause();
	if(m_targetTime > 0)
		m_Service->mWatchDog.watch(ident.name,ZQ::common::now() + 2 * m_targetTime);
	return true;
}


bool MRTStream::resume(const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	RLock sunc(*this);
	tempPause();
	return true;
}


::TianShanIce::Streamer::StreamState MRTStream::getCurrentState(const ::Ice::Current&/* = ::Ice::Current()*/) const 
{
	RLock sunc(*this);
	TianShanIce::Streamer::StreamState t = TianShanIce::Streamer::stsSetup;

	std::string streamID = ident.name;
	std::string mrtSessionId;
	std::string srmSessionId;
	std::string streamNetId;
	bool bRet = m_Service->findMRTSessionIdByStreamId(streamID, mrtSessionId, srmSessionId,streamNetId);
	if(!bRet)
	{
		return TianShanIce::Streamer::stsStop;
	}

	MRTEndpointInfo mrtEndpointInfo;
	bRet = m_Service->findMRTEnpoint(streamNetId, mrtEndpointInfo);
	if(!bRet)
	{
		return TianShanIce::Streamer::stsStop;
	}

	ZQMRT::MRTClient mrtClient(m_Service->getLogger());
	std::string errorMsg;
	bRet = mrtClient.getStatus(mrtEndpointInfo, mrtSessionId, srmSessionId, errorMsg);

	if(!bRet)
	{
		return TianShanIce::Streamer::stsStop;
	}

	t =TianShanIce::Streamer::stsStreaming;
	return t;
}


::TianShanIce::SRM::SessionPrx MRTStream::getSession(const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	RLock sunc(*this);
	return NULL;
}


void MRTStream::setMuxRate(::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	RLock sunc(*this);
}


bool MRTStream::allocDVBCResource(::Ice::Int, ::Ice::Int, const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	RLock sunc(*this);
	return true;
}
::std::string MRTStream::getId(const ::Ice::Current& /*= ::Ice::Current()*/) const 
{
	return ident.name;
}

bool MRTStream::getInfo(::Ice::Int mask , ::TianShanIce::ValueMap& varOut, const ::Ice::Current& /*= ::Ice::Current()*/)
{	
	#if  ICE_INT_VERSION / 100 >= 306
		RLock sync(*this);
	#else
		IceUtil::RWRecMutex::RLock sync(*this);
	#endif
	tempPause();
	ZQ::common::Variant var;
	switch(mask)
	{
	case TianShanIce::Streamer::infoSTREAMSOURCE:
		{
			//if(!var.valid())
			//	return false;
			TianShanIce::Variant	res;
			varOut.clear();

			res.strs.clear();
			res.bRange = false;
			res.strs.push_back("1.2.63.4");
			res.type = TianShanIce::vtStrings;
			varOut["StreamingSourceIp"] = res;

			res.ints.clear();
			res.bRange = false;
			res.ints.push_back( 5689 );
			res.type = TianShanIce::vtInts;
			varOut["StreamingSourcePort"] = res;

		}
		break;
	case TianShanIce::Streamer::infoDVBCRESOURCE:
		{
			//check if the var is right 	
			
			TianShanIce::Variant	res;
			varOut.clear();

			///push playlist guid
			res.strs.clear();
			res.strs.push_back(ident.name);
			res.type=TianShanIce::vtStrings;
			varOut["Guid"]=res;
			
			res.ints.clear();
			res.ints.push_back(int(5678910));
			res.type=TianShanIce::vtInts;
			varOut["Frequency"]=res;

			res.ints.clear();
			res.ints.push_back(int(5545));
			res.type=TianShanIce::vtInts;
			varOut["ProgramNumber"]=res;
			
			res.strs.clear();
			res.strs.push_back(std::string("224.0.0.100"));
			res.type=TianShanIce::vtStrings;
			varOut["DestIP"]=res;

			res.ints.clear();
			res.ints.push_back(int(7896));
			res.type=TianShanIce::vtInts;
			varOut["DestPort"]=res;
			
			res.ints.clear();
			res.ints.push_back(int( 2 ));
			res.type=TianShanIce::vtInts;
			varOut["Channel"]=res;

			res.ints.clear();
			res.ints.push_back(16);
			res.type=TianShanIce::vtInts;
			varOut["QamMode"]=res;
		}
		break;
	case TianShanIce::Streamer::infoPLAYPOSITION:
		{
			TianShanIce::Variant res;
			res.ints.clear();
			res.ints.push_back( 10 );
			res.type=TianShanIce::vtInts;
			varOut["ctrlnumber"]=res;

			ZQTianShan::Util::updateValueMapData( varOut, "index" , (Ice::Int)1);
			ZQTianShan::Util::updateValueMapData( varOut, "itemOffset" , (Ice::Int)200);

			res.ints.clear();
			res.ints.push_back( 1800 );
			res.type=TianShanIce::vtInts;
			varOut["playposition"]=res;		
			
			res.ints.clear();
			res.ints.push_back(int( 1800000 ));
			res.type=TianShanIce::vtInts;
			varOut["totalplaytime"]=res;

			res.strs.clear();
			res.strs.push_back("1.0");
			res.type=TianShanIce::vtStrings;
			varOut["scale"]=res;
		}
		break;
	case TianShanIce::Streamer::infoSTREAMNPTPOS:
		{			
			TianShanIce::Variant res;
			res.strs.clear();
			res.strs.push_back( ident.name );
			res.type=TianShanIce::vtStrings;
			varOut["playlistid"]=res;
			
			res.ints.clear();
			res.ints.push_back( 1800 );
			res.type=TianShanIce::vtInts;
			varOut["playposition"]=res;		
			
			res.ints.clear();
			res.ints.push_back( 180000 );
			res.type=TianShanIce::vtInts;
			varOut["totalplaytime"]=res;

			ZQTianShan::Util::updateValueMapData( varOut, "index" , (Ice::Int)1);
			ZQTianShan::Util::updateValueMapData( varOut, "itemOffset" , (Ice::Int)200);
			
			res.strs.clear();
			res.strs.push_back( "1.0" );
			res.type=TianShanIce::vtStrings;
			varOut["scale"]=res;			
		}
		break;
	default:
		glog(Log::L_ERROR,"invalid mask =%d when get info",mask);
		break;
	}	
	return true;
	
}

::Ice::Int MRTStream::insert(::Ice::Int, const ::TianShanIce::Streamer::PlaylistItemSetupInfo&,
							   ::Ice::Int, const ::Ice::Current&/* = ::Ice::Current()*/)
{
	return 1;
}

::Ice::Int MRTStream::pushBack(::Ice::Int, const ::TianShanIce::Streamer::PlaylistItemSetupInfo&,
					const ::Ice::Current&/* = ::Ice::Current()*/) 
{
	return 1;
}

::Ice::Int MRTStream::size(const ::Ice::Current&/* = ::Ice::Current()*/) const
{
	return 1;
}

::Ice::Int MRTStream::left(const ::Ice::Current&/* = ::Ice::Current()*/) const
{
	return 1;
}

bool MRTStream::empty(const ::Ice::Current&/* = ::Ice::Current()*/) const 
{
	return true;
}

::Ice::Int MRTStream::current(const ::Ice::Current&/* = ::Ice::Current()*/) const 
{
	return 1;
}

void MRTStream::erase(::Ice::Int, const ::Ice::Current&/* = ::Ice::Current()*/) 
{
	
}

::Ice::Int MRTStream::flushExpired(const ::Ice::Current&/* = ::Ice::Current()*/) 
{
	return 1;
}

bool MRTStream::clearPending(bool, const ::Ice::Current&/* = ::Ice::Current()*/) 
{
	return true;
}

bool MRTStream::isCompleted(const ::Ice::Current&/* = ::Ice::Current()*/) 
{
	return true;
}

::TianShanIce::IValues MRTStream::getSequence(const ::Ice::Current&/* = ::Ice::Current()*/) const 
{
	::TianShanIce::IValues lv;
	return lv;
}

::Ice::Int MRTStream::findItem(::Ice::Int, ::Ice::Int, const ::Ice::Current&/* = ::Ice::Current()*/)
{
	return 1;
}

bool MRTStream::distance(::Ice::Int, ::Ice::Int, ::Ice::Int&, const ::Ice::Current&/* = ::Ice::Current()*/)
{
	return true;
}

bool MRTStream::skipToItem(::Ice::Int, bool, const ::Ice::Current& /*= ::Ice::Current()*/)
{
	return true;
}

bool MRTStream::seekToPosition(::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current&/* = ::Ice::Current()*/)
{
	return true;
}
void MRTStream::play_async(const ::TianShanIce::Streamer::AMD_Stream_playPtr& callback, const ::Ice::Current& ) 
{
//	tempPause();
	
	if(m_targetTime > 0)
		m_Service->mWatchDog.watch(ident.name, ZQ::common::now()+ m_targetTime);

	std::string errorMsg;
	if(!setupMRTStream(errorMsg))
	{
		callback->ice_exception(TianShanIce::ServerError("MRTStream", 5003, errorMsg));
	}
	else
		callback->ice_response(1);
}

void MRTStream::seekStream_async(const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr& callback, ::Ice::Long, ::Ice::Int, const ::Ice::Current& )
{
	tempPause();
	callback->ice_response(1);
}
bool MRTStream::setupMRTStream(std::string& errorMsg)
{
	std::string streamID = ident.name;
	bool bret = false;
	std::string AssetName;
	std::string destIp;
	int destPort;
	int bitrate;
	std::string srmSessionId;
	std::string streamNetId;
	bool bRet = m_Service->findMRTSetupInfoByStreamId(streamID, AssetName, destIp,destPort,bitrate, srmSessionId,streamNetId);
	if(!bRet)
	{
		m_Service->increaseStreamPenalty(streamNetId);
		errorMsg = streamID  + " failed to get setup info by streamID: " + streamID;
		return false;
	}

	MRTEndpointInfo mrtEndpointInfo;
	bRet = m_Service->findMRTEnpoint(streamNetId, mrtEndpointInfo);
	if(!bRet)
	{
		m_Service->increaseStreamPenalty(streamNetId);
		errorMsg = streamID  + " setup MRT stream with error: can't find MRT endpoint by streamNetId: " + streamNetId;
		return false;
	}

	ZQMRT::MRTClient mrtClient(m_Service->getLogger());
	std::string mrtSessionId, mrtErrorMsg;
	bRet = mrtClient.setup(mrtEndpointInfo, AssetName, destIp, destPort, bitrate, srmSessionId,mrtSessionId, mrtErrorMsg);

	if(!bRet)
	{
		m_Service->increaseStreamPenalty(streamNetId);
		errorMsg = streamID  + " setup MRT stream with error:  " + mrtErrorMsg;
		return false;
	}

	m_Service->updateMRTSessionIdToStore(streamID, mrtSessionId, streamNetId);
	m_Service->decreaseStreamPenalty(streamNetId);
	return true;

}
void MRTStream::playEx_async(const ::TianShanIce::Streamer::AMD_Stream_playExPtr& callback, ::Ice::Float, ::Ice::Long, ::Ice::Short, const ::TianShanIce::StrValues&, const ::Ice::Current&)
{
//	tempPause();
	::TianShanIce::Streamer::StreamInfo info;
	info.state = TianShanIce::Streamer::stsStreaming;
	info.props["CURRENTPOS"]	=	"100";
	info.props["ITEM_CURRENTPOS"] = "100";
	info.props["TOTALPOS"] = "10000";
	info.props["ITEM_TOTALPOS"] = "10000";
	info.props["SPEED"] = "1.0";
	info.props["USERCTRLNUM"] = "1";

	if(m_targetTime > 0)
		m_Service->mWatchDog.watch(ident.name, ZQ::common::now()+m_targetTime);

	std::string errorMsg;
	if(!setupMRTStream(errorMsg))
	{
		callback->ice_exception(TianShanIce::ServerError("MRTStream", 5003, errorMsg));
	}
	else
		callback->ice_response(info);
}

void MRTStream::skipToItem_async(const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr& callback, ::Ice::Int, bool, const ::Ice::Current& )
{
	tempPause();
	callback->ice_response(true);
}

void MRTStream::seekToPosition_async(const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr& callback, ::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& )
{
	tempPause();
	callback->ice_response(true);
}

void MRTStream::playItem_async(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr& callback, ::Ice::Int, ::Ice::Int, ::Ice::Short, ::Ice::Float, const ::TianShanIce::StrValues&, const ::Ice::Current& )
{
	tempPause();
	::TianShanIce::Streamer::StreamInfo info;
	callback->ice_response(info);
}

::TianShanIce::Streamer::StreamerDescriptors MRTProxy::listStreamers(const ::Ice::Current& )
{
	RLock sync(*this);
	::TianShanIce::Streamer::StreamerDescriptors ids;
	
	std::vector<std::string>::const_iterator it = mStreamers.begin( );
	for( ; it != mStreamers.end() ; it++ )
	{
		::TianShanIce::Streamer::StreamerDescriptor id;
		id.deviceId = *it;
		id.type="MRTSS";
		ids.push_back(id);
	}
	return ids;
}

bool MRTProxy::connectToEventChannel( const std::string& endpoint )
{
	mStrTopicProxy = endpoint;
	try
	{
		if( mStrTopicProxy.empty() )
		{			
			return true;
		}
		printf("connect to event channel service with %s\n",mStrTopicProxy.c_str());
		Ice::CommunicatorPtr ic=m_Adapter->getCommunicator( );
		Ice::ObjectPrx base=ic->stringToProxy( mStrTopicProxy );
		if(!base)
		{		
			return false;
		}

		mTopicManagerPrx = IceStorm::TopicManagerPrx::checkedCast( base );
		if(!mTopicManagerPrx)
		{		
			return false;
		}

		Ice::ObjectPrx topic = NULL;
		topic = getTopic( TianShanIce::Streamer::TopicOfPlaylist );
		if( !topic ) return false;
		mPlaylistEventPrx	= TianShanIce::Streamer::PlaylistEventSinkPrx::uncheckedCast( topic );

		topic = getTopic( TianShanIce::Streamer::TopicOfStream );
		if(!topic ) return false;
		mStreamEventPrx		= TianShanIce::Streamer::StreamEventSinkPrx::uncheckedCast( topic );

		topic = getTopic( TianShanIce::Streamer::TopicOfStreamProgress );
		if(!topic) return false;
		mProgressEventPrx	= TianShanIce::Streamer::StreamProgressSinkPrx::uncheckedCast( topic );

		topic = getTopic( TianShanIce::Events::TopicStreamRepositionEvent );
		if(!topic) return false;
		mRepostionEventPrx = TianShanIce::Events::GenericEventSinkPrx::uncheckedCast( topic );

		topic = getTopic( TianShanIce::Events::TopicStreamPauseTimeoutEvent );
		if(! topic ) return false;
		mPauseTimeoutEventPrx = TianShanIce::Events::GenericEventSinkPrx::uncheckedCast( topic );

		printf("successfully connected to event channel service %s \n",mStrTopicProxy.c_str());
	}
	catch ( const Ice::Exception& ex) 
	{
		printf("caught %s when connect to event channel %s\n",ex.ice_name().c_str(),mStrTopicProxy.c_str());
		return false;
	}
	catch (...) 
	{
		printf("Connect to event channel service %s failed\n", mStrTopicProxy.c_str() );
		return false;
	}

	return true;
}

Ice::ObjectPrx MRTProxy::getTopic( const std::string& topicName )
{
	if( !mTopicManagerPrx )
		return NULL;

	IceStorm::TopicPrx			iceTopic = NULL;	
	Ice::ObjectPrx				iceObj = NULL;

	try
	{
		iceTopic = mTopicManagerPrx->retrieve( topicName );
	}
	catch (const IceStorm::NoSuchTopic& ) 
	{	
		try
		{
			iceTopic = mTopicManagerPrx->create( topicName );
		}
		catch( const Ice::Exception&)
		{
			iceTopic = NULL;
		}
	}
	if( !iceTopic )
		return NULL;
	try
	{
		iceObj = iceTopic->getPublisher();
		if(!iceObj->ice_isDatagram())
		{//set it to one way schema
			iceObj->ice_oneway();
		}
	}
	catch( const Ice::Exception&)
	{
		iceObj = NULL;
	}
	return iceObj;
}

int MRTProxy::reportSpigot()
{
	if( listenerEndpoint.empty() )
	{
		//printf("no listen spigot point\n");
		return 1000*1000;
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
		strListenerEndpoint = "ReplicaSubscriber:" + listenerEndpoint;
	}

	try
	{
		subscriber	= TianShanIce::ReplicaSubscriberPrx::checkedCast(m_Adapter->getCommunicator()->stringToProxy(strListenerEndpoint));
		if( !subscriber )
		{
			printf("can't connect to replica listener[%s]\n", strListenerEndpoint.c_str());
		}
	}
	catch( const Ice::Exception& ex)
	{//If I catch an exception
		//I should quit the report process
		printf("caught exception:[%s] when connect to replica listener[%s]\n", ex.ice_name().c_str(), strListenerEndpoint.c_str());
		subscriber = NULL;
		return 60*1000;
	}

	//collect replicas
	::TianShanIce::Replicas reps;
	
	std::vector<std::string>::const_iterator it = mStreamers.begin();
	for( ; it != mStreamers.end() ; it ++ )
	{
		::TianShanIce::Replica  rep;
		rep.replicaId				= *it;
		rep.groupId					= mNetId;
		rep.replicaState			= TianShanIce::stInService;

		std::string streamNetId = mNetId + "/" + *it;
		int penalty = geStreamPenalty(streamNetId);

		if(penalty > _outOfServicePenalty)
			rep.replicaState			= TianShanIce::stOutOfService;		

		rep.category				= "streamer";
		reps.push_back(rep);
	}

	int defaultUpdateInterval = 10*1000;
	//report it to subscriber
	int iNextReportInterval = defaultUpdateInterval;
	try
	{
		if( subscriber )
		{
			iNextReportInterval = subscriber->updateReplica( reps ) * 700;
			if( iNextReportInterval <= 0 )
			{
				printf("subscriber return the update interval is [%d] , adjust it to [%d]\n" , iNextReportInterval , defaultUpdateInterval);
				iNextReportInterval = defaultUpdateInterval;
			}
			printf("update replica to [%s] ok, and interval for next report is [%d]\n",strListenerEndpoint.c_str() ,iNextReportInterval);
		}
		else
		{
			printf("failed to update replica to [%s] , and interval for next report is [%d]\n",strListenerEndpoint.c_str(), iNextReportInterval);
		}
	}
	catch( const Ice::Exception& ex )
	{
		printf("failed to report replicas status to subscriber and caught ice exception:[%s] and interval for next report is [%d]\n",ex.ice_name().c_str() ,iNextReportInterval);
	}
	if( iNextReportInterval < 10 * 1000 )
		iNextReportInterval = 10 * 1000;
	updateInterval = iNextReportInterval;
	return updateInterval;
}
/*
bool MRTProxy::setupMRTStream(const std::string& streamID)
{
	std::string AssetName;
	std::string destIp;
	int destPort;
	int bitrate;
	std::string srmSessionId;
	std::string streamNetId;
	bool bRet = findMRTSetupInfoByStreamId(streamID, AssetName, destIp,destPort,bitrate, srmSessionId,streamNetId);
	if(!bRet)
	{
		return false;
	}

	ZQMRT::MRTEndpointInfo mrtEndpointInfo;
	bRet = findMRTEnpoint(streamNetId, mrtEndpointInfo);
	if(!bRet)
	{
		return false;
	}

	std::string mrtSessionId, errorMsg;
	bRet = setup(mrtEndpointInfo, AssetName, destIp, destPort, bitrate, srmSessionId,mrtSessionId, errorMsg);

	if(!bRet)
	{
		return false;
	}

	updateMRTSessionIdToStore(streamID, mrtSessionId);
	return bRet;
}
bool MRTProxy::getMRTStreamStatus(const std::string& streamID)
{
	std::string mrtSessionId;
	std::string srmSessionId;
	std::string streamNetId;
	bool bRet = findMRTSessionIdByStreamId(streamID, mrtSessionId, srmSessionId,streamNetId);
	if(!bRet)
	{
		return false;
	}

	MRTEndpointInfo mrtEndpointInfo;
	bRet = findMRTEnpoint(streamNetId, mrtEndpointInfo);
	if(!bRet)
	{
		return false;
	}

	std::string mrtSessionId, errorMsg;
	bRet = getStatus(mrtEndpointInfo, mrtSessionId, srmSessionId, errorMsg);

	if(!bRet)
	{
		return false;
	}

	return bRet;
}
bool MRTProxy::teardownMRTStream(const std::string& streamID)
{
	std::string mrtSessionId;
	std::string srmSessionId;
	std::string streamNetId;
	bool bRet = findMRTSessionIdByStreamId(streamID, mrtSessionId, srmSessionId,streamNetId);
	if(!bRet)
	{
		return false;
	}

	MRTEndpointInfo mrtEndpointInfo;
	bRet = findMRTEnpoint(streamNetId, mrtEndpointInfo);
	if(!bRet)
	{
		return false;
	}

	std::string mrtSessionId, errorMsg;
	bRet = teardown(mrtEndpointInfo, mrtSessionId, srmSessionId, errorMsg);

	if(!bRet)
	{
		return false;
	}

	return bRet;
}
*/
//////////////////////////////////////////////////////////////////////////
ReplicaUpdater::ReplicaUpdater( MRTProxy& svc )
:mService(svc)
{
	//mEvent = CreateEvent( NULL , FALSE, FALSE , NULL );
}

ReplicaUpdater::~ReplicaUpdater( )
{
//	CloseHandle(mEvent);
}

int ReplicaUpdater::report( )
{
	return mService.reportSpigot();
}

int ReplicaUpdater::run( )
{
	mbQuit = false;
	int interval = 60*1000;
	do
	{
		interval = report();
		//WaitForSingleObject(mEvent,interval);
		_replicaUpdaterEvent.wait(interval);
	}while(!mbQuit);
	return 1;
}

void ReplicaUpdater::stop()
{
	mbQuit = true;
	_replicaUpdaterEvent.signal();
	//SetEvent(mEvent);
}


//////////////////////////////////////////////////////////////////////
bool TimerWatch::start( ) {

	return ZQ::common::NativeThread::start();
}
void TimerWatch::stop( ) {
	if(!mbRunning)
		return;
	mbRunning = false;
	mSem.post();
	waitHandle(-1);
}
void TimerWatch::watch( const std::string& id, long long target) {
	bool bSignal = false;
	{
		ZQ::common::MutexGuard gd(mLocker);
		unwatch(id);
		TimerInfo info;
		info.id = id;
		info.target = target;
		mTimerInfos.insert(info);
		mInstance2Infos[id] = info;
		if( target < mNextWakeup ) 
			bSignal = true;
	}
	if( bSignal)
		mSem.post();
}
void TimerWatch::unwatch( const std::string& id ) {
	ZQ::common::MutexGuard gd(mLocker);
	std::map<std::string,TimerInfo>::iterator it = mInstance2Infos.find(id);
	if(it != mInstance2Infos.end()) {
		mTimerInfos.erase(it->second);
		mInstance2Infos.erase(it);
	}		
}

TianShanIce::Streamer::PlaylistExPrx TimerWatch::openStream( const std::string& id ) {
	try{
		return TianShanIce::Streamer::PlaylistExPrx::uncheckedCast( mMRTProxy->openPlaylist(id,::TianShanIce::Streamer::SpigotBoards(),true) );
	}
	catch( Ice::ObjectNotExistException& ){
		return NULL;
	}
}

int TimerWatch::run( ) {
	mbRunning = true;
	mNextWakeup = ZQ::common::now();
	long long delta = 100;
	while(mbRunning) {
		delta = 60 * 1000;		
		long long tnow = ZQ::common::now();
		mNextWakeup = tnow + delta;
		while( true ) {				
			std::string id;
			{
				ZQ::common::MutexGuard gd(mLocker);
				if(mTimerInfos.empty())
					break;
				std::set<TimerInfo>::iterator it = mTimerInfos.begin();
				const TimerInfo& info = *it;
				if( info.target > tnow ) {
					delta= (info.target -  tnow);
					if( delta <= 5)
						delta = 5;
					mNextWakeup = ZQ::common::now() + delta;
					break;
				} else {
					id = info.id;
					mTimerInfos.erase(it);
				}
			}
			TianShanIce::Streamer::PlaylistExPrx stream = openStream(id);
			if(!stream)
				continue;
			try {
				stream->onTimer();
			}
			catch( const Ice::Exception& ) {}
		}
		mSem.timedWait((timeout_t)delta);
	}
	return 0;
}
