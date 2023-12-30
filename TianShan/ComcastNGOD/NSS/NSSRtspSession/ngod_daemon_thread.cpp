#include "ngod_daemon_thread.h"
#include "ngod_rtsp_action.h"
#include "ngod_rtsp_parser/RTSPMessage/RTSPMessageParser.h"
#include "NSS.h"
#include "NSSEx.h"

#define SessionSyncTime 200000

int iClientNumber = 0;
string strLocalIP="192.168.81.103";

//const variables for r2 interaction
const char* pComPath = "com.comcast.org";
const char* pInterfaceId = "r2";
const char* pTransportType = "MP2T/DVBC/UDP";
const char* pSDPProviderId = "schange.com";
const char* pSDPRange = "0-";
const char* pStreamControlProtoType = "rtsp";

ngod_daemon_thread::ngod_daemon_thread(ZQ::common::FileLog *logfile,
									   ::ZQTianShan::NSS::NssEventList &eventList, 
									   ZQ::common::NativeThreadPool &pool,
									   ::Ice::CommunicatorPtr communicator,
									   ZQADAPTER_DECLTYPE adapter)
:m_NSSSessionGroupList(NULL),
m_iMaxSessionNum(NSSSessionMapMaxSize),
m_bRunToken(false),
m_pPool(&pool),
m_pLogFile(logfile),
_eventList(eventList),
_communicator(communicator),
_adapter(adapter)
{
	InitEvent();
	InitializeCriticalSection(&_CS);
}

ngod_daemon_thread::ngod_daemon_thread(ZQ::common::FileLog *logfile,
									   ::ZQTianShan::NSS::NssEventList &eventList, 
									   NSSSessionGroupList &_NSSSessionGroupList,
									   ZQ::common::NativeThreadPool &pool,
									   ::Ice::CommunicatorPtr communicator,
									   ZQADAPTER_DECLTYPE adapter)
:m_NSSSessionGroupList(&_NSSSessionGroupList),
m_iMaxSessionNum(NSSSessionMapMaxSize),
m_bRunToken(false),
m_pPool(&pool),
m_pLogFile(logfile),
_eventList(eventList),
_communicator(communicator),
_adapter(adapter)
{
	InitEvent();
	InitializeCriticalSection(&_CS);
}

ngod_daemon_thread::~ngod_daemon_thread()
{
	//exit thread
	terminate(0);

	//stop rtsp message process
	delete m_pngod_recv_thread;
	delete m_pngod_chop_thread;
	delete m_pngod_parse_thread;

	//reset member variables
	m_NSSSessionGroupList = NULL;
	m_pPool = NULL;
	m_pLogFile = NULL;

	::CloseHandle(m_Event);
	DeleteCriticalSection(&_CS);

	_communicator = NULL;
	_adapter = NULL;
	exit();
}

void ngod_daemon_thread::setNSSSessionGroupList(NSSSessionGroupList &_NSSSessionGroupList)
{
	m_NSSSessionGroupList = &_NSSSessionGroupList;
}

bool ngod_daemon_thread::initialize(void)
{
	m_pngod_recv_thread = new ngod_recv_thread(m_pLogFile, *m_NSSSessionGroupList);
	m_pngod_chop_thread = new ngod_chop_thread(m_pLogFile, *m_NSSSessionGroupList);
	m_pngod_parse_thread= new ngod_parse_thread(m_pLogFile, *m_pPool, _eventList, *m_NSSSessionGroupList, _streamerStrList);

	m_pngod_recv_thread->start();
	m_pngod_chop_thread->start();
	m_pngod_parse_thread->start();
	return true;
}

int ngod_daemon_thread::run(void)
{
	m_bRunToken = true;

	int iStartTime = GetTickCount();
	int iEndTime = GetTickCount();
	while (m_bRunToken)
	{
		//todo daemon thread
#ifdef DAEMONTHRDDEBUG
		//cout << "thread running" << endl;
#endif
		Sleep(1000);
		iEndTime = GetTickCount();
		if (iEndTime - iStartTime >= SessionSyncTime)
		{
			//to send get parameter
			for (NSSSessionGroupList::iterator iter = m_NSSSessionGroupList->begin();
			iter != m_NSSSessionGroupList->end(); iter++)
			{
				if ((*iter)->m_NSSSessionMap.empty())
					continue;
#ifdef DAEMONTHRDDEBUG
		//cout << "send get_parameter to sync " << (*iter)->strSessionGroup << endl;
#endif
				ngod_rtsp_action::GetParameterAction((*iter)->strSessionGroup, *m_NSSSessionGroupList, *m_pPool, *m_pLogFile);
#ifdef DAEMONTHRDDEBUG
		//cout << "send get_parameter over" << endl;
#endif
				for (NSSSessionMap::iterator iter1 = (*iter)->m_NSSSessionMap.begin(); iter1 != (*iter)->m_NSSSessionMap.end(); iter1++)
				{
					::Ice::Identity ident;
					try
					{
						ident = _communicator->stringToIdentity(iter1->second->strStreamName);
						::TianShanIce::Streamer::StreamPrx streamObj = ::TianShanIce::Streamer::NGODStreamServer::NGODStreamExPrx::uncheckedCast(_adapter->createProxy(ident));
						streamObj->destroy();
					}
					catch (Ice::Exception& ex)
					{
						(*m_pLogFile)(ZQ::common::Log::L_ERROR, CLOGFMT(ngod_daemon_thread, "caught Ice::Exception(%s) when destroy session(%s) by sync list"), ex.ice_name().c_str(), ident.name.c_str());
					}
					catch (...)
					{
						(*m_pLogFile)(ZQ::common::Log::L_ERROR, CLOGFMT(ngod_daemon_thread, "caught unknown exception when destroy session(%s)"), ident.name.c_str());
					}
				}
			}//for(...)	
			iStartTime = GetTickCount();
		}//if (iEndTime - iStartTime >= SessionSyncTime)
		else
		{
			if (_streamerStrList.m_StreamStrList.empty() == false)
			{
				::std::string strStreamer = _streamerStrList.First();
				::Ice::Identity ident;
				try
				{
					ident = _communicator->stringToIdentity(strStreamer);
					::TianShanIce::Streamer::StreamPrx streamObj = ::TianShanIce::Streamer::NGODStreamServer::NGODStreamExPrx::uncheckedCast(_adapter->createProxy(ident));
					streamObj->destroy();
				}
				catch (Ice::Exception& ex)
				{
					(*m_pLogFile)(ZQ::common::Log::L_ERROR, CLOGFMT(ngod_daemon_thread, "caught Ice::Exception(%s) when destroy session(%s) by sync list"), ex.ice_name().c_str(), ident.name.c_str());
				}
				catch (...)
				{
					(*m_pLogFile)(ZQ::common::Log::L_ERROR, CLOGFMT(ngod_daemon_thread, "caught unknown exception when destroy session(%s)"), ident.name.c_str());
				}
			}
		}
	}//while (m_bRunToken)

	::SetEvent(m_Event);
	cout << "exit daemon thread" << endl;
	return 1;
}

int ngod_daemon_thread::terminate(int code )
{
	if (m_bRunToken == false)
		return 1;
	m_bRunToken = false;

	//wait until the run function exit
	::WaitForSingleObject(m_Event, INFINITE);
	return 1;
}

void ngod_daemon_thread::addGroup(NSSSessionGroup *SessionGroup)
{
	EnterCriticalSection(&_CS);
	m_NSSSessionGroupList->push_back(SessionGroup);
	LeaveCriticalSection(&_CS);
	ngod_rtsp_action::SetParameterAction(SessionGroup->strSessionGroup, *m_NSSSessionGroupList, *m_pPool, *m_pLogFile);
	//todo
}

void ngod_daemon_thread::removeGroup(string &strSessionGroup)
{
	//todo
}

void ngod_daemon_thread::updateGroup(string &strSessionGroup)
{
	//todo
}

bool ngod_daemon_thread::addSession(RTSPClientSession *sess)
{
	//create key
	SessionMapKey key;
	key.strOnDemandSessionId = sess->m_RTSPR2Header.strOnDemandSessionID;
	key.strSessionId = sess->m_RTSPR2Header.strSessionID;


	//add session to group
	EnterCriticalSection(&_CS);
	for (NSSSessionGroupList::iterator iter = m_NSSSessionGroupList->begin();
		iter != m_NSSSessionGroupList->end();
		iter++)
	{
		if ((*iter)->m_NSSOnDemandSessionMap.size() < (*iter)->uMaxSession && (*iter)->m_NSSSessionMap.size() < (*iter)->uMaxSession)
		{
			//initialize session parameters
			initRTSPClientSession(*sess, *(*iter));

			NSSSessionMap::iterator sessIter = (*iter)->m_NSSSessionMap.find(key.strSessionId);
			if (sessIter == (*iter)->m_NSSSessionMap.end())
			{
				if (!key.strSessionId.empty())
					(*iter)->m_NSSSessionMap[key.strSessionId] = sess;
			}

			sessIter = (*iter)->m_NSSOnDemandSessionMap.find(key.strOnDemandSessionId);
			if (sessIter == (*iter)->m_NSSOnDemandSessionMap.end())
			{
				if (!key.strOnDemandSessionId.empty())
					(*iter)->m_NSSOnDemandSessionMap[key.strOnDemandSessionId] = sess;
			}

			LeaveCriticalSection(&_CS);
			return true;
		}
	}
	LeaveCriticalSection(&_CS);
	return false;
}

bool ngod_daemon_thread::addSession(RTSPClientSession *sess, string &strSessGroup)
{
	//find session group first
	NSSSessionGroupList::iterator iter = find_if(m_NSSSessionGroupList->begin(), 
												 m_NSSSessionGroupList->end(),
												 FindBySessionGroup(sess->m_RTSPR2Header.SessionGroup.strToken));
	
	//create key
	SessionMapKey key;
	key.strOnDemandSessionId = sess->m_RTSPR2Header.strOnDemandSessionID;
	key.strSessionId = sess->m_RTSPR2Header.strSessionID;

	//add session to group
	EnterCriticalSection(&_CS);
	if (iter == m_NSSSessionGroupList->end())
	{
		LeaveCriticalSection(&_CS);
		return false;
	}
	else
	{
		//initialize session parameters
		initRTSPClientSession(*sess, *(*iter));
		NSSSessionMap::iterator sessIter;

		
		sessIter = (*iter)->m_NSSSessionMap.find(key.strSessionId);
		if (sessIter != (*iter)->m_NSSSessionMap.end())
		{
			(*m_pLogFile)(ZQ::common::Log::L_WARNING, CLOGFMT(ngod_daemon_thread, "error when join NSS Session Map with Session([%s])(already in map)"), key.strSessionId.c_str());
			//LeaveCriticalSection(&_CS);
			//return false;
		}
		else
		{
			if ((*iter)->m_NSSSessionMap.size() < (*iter)->uMaxSession)
			{
				if (!key.strSessionId.empty())
					(*iter)->m_NSSSessionMap[key.strSessionId] = sess;
			}
			else
			{
				(*m_pLogFile)(ZQ::common::Log::L_WARNING, CLOGFMT(ngod_daemon_thread, "error when join NSS Session Map with Session([%s]), Session Map Size(%d) over Max(%d)"), key.strSessionId.c_str(), (*iter)->m_NSSSessionMap.size(), (*iter)->uMaxSession);
				LeaveCriticalSection(&_CS);
				return false;
			}
		}

		sessIter = (*iter)->m_NSSOnDemandSessionMap.find(key.strOnDemandSessionId);
		if (sessIter != (*iter)->m_NSSOnDemandSessionMap.end())
		{
			(*m_pLogFile)(ZQ::common::Log::L_WARNING, CLOGFMT(ngod_daemon_thread, "error when join NSS Ondemand Session Map with Session([%s])(already in map)"), key.strOnDemandSessionId.c_str());
			//LeaveCriticalSection(&_CS);
			//return false;
		}
		else
		{
			if ((*iter)->m_NSSOnDemandSessionMap.size() < (*iter)->uMaxSession)
			{
				if (!key.strOnDemandSessionId.empty())
					(*iter)->m_NSSOnDemandSessionMap[key.strOnDemandSessionId] = sess;
			}
			else
			{
				(*m_pLogFile)(ZQ::common::Log::L_WARNING, CLOGFMT(ngod_daemon_thread, "error when join NSS Ondemand Session Map with Session([%s]), Session Map Size(%d) over Max(%d)"), key.strOnDemandSessionId.c_str(), (*iter)->m_NSSOnDemandSessionMap.size(), (*iter)->uMaxSession);
				LeaveCriticalSection(&_CS);
				return false;
			}
		}
		

		LeaveCriticalSection(&_CS);
		return true;
	}
}

bool ngod_daemon_thread::removeSession(RTSPClientSession *sess)
{
	//find session group first
	NSSSessionGroupList::iterator iter = find_if(m_NSSSessionGroupList->begin(), 
												 m_NSSSessionGroupList->end(),
												 FindBySessionGroup(sess->m_RTSPR2Header.SessionGroup.strToken));
	EnterCriticalSection(&_CS);
	if (iter == m_NSSSessionGroupList->end())
	{
		LeaveCriticalSection(&_CS);
		return false;
	}
	else
	{
		SessionMapKey key;
		key.strOnDemandSessionId = sess->m_RTSPR2Header.strOnDemandSessionID;
		key.strSessionId = sess->m_RTSPR2Header.strSessionID;
		//remove on-demand-session map
		NSSSessionMap::iterator sessIter =  (*iter)->m_NSSOnDemandSessionMap.find(key.strOnDemandSessionId);
		if (sessIter == (*iter)->m_NSSOnDemandSessionMap.end())
		{
			LeaveCriticalSection(&_CS);
			(*m_pLogFile)(ZQ::common::Log::L_DEBUG, CLOGFMT(ngod_daemon_thread,"fail to find OnDemandSessionId(%s)"), key.strOnDemandSessionId.c_str());
			return false;
		}
		else
			(*iter)->m_NSSOnDemandSessionMap.erase(sessIter);

		//remove session map
		sessIter =  (*iter)->m_NSSSessionMap.find(key.strSessionId);
		if (sessIter == (*iter)->m_NSSSessionMap.end())
		{
			LeaveCriticalSection(&_CS);
			(*m_pLogFile)(ZQ::common::Log::L_DEBUG, CLOGFMT(ngod_daemon_thread,"fail to find SessionId(%s)"), key.strSessionId.c_str());
			return false;
		}
		else
			(*iter)->m_NSSSessionMap.erase(sessIter);
	}
	LeaveCriticalSection(&_CS);
	return true;
}

bool ngod_daemon_thread::updateSession(RTSPClientSession *sess,
									   SDPRequestContent &pSDPReqContent,
										RTSPTransportUdpHeader &pTransportHeader)
{
	removeSession(sess);
	addSession(sess);
	return true;
}

RTSPClientSession *ngod_daemon_thread
::findSessionByOnDemandSessionId(const string &strOnDemandSessionId)
{
	//find session group first
	EnterCriticalSection(&_CS);
	for (NSSSessionGroupList::iterator iter = m_NSSSessionGroupList->begin();
		iter != m_NSSSessionGroupList->end();iter++)
	{
		//create key
		//SessionMapKey key;
		//key.strOnDemandSessionId = strOnDemandSessionId;

		//NSSSessionMap::iterator sessIter =  (*iter)->m_NSSOnDemandSessionMap.find(key.strOnDemandSessionId);
		NSSSessionMap::iterator sessIter =  (*iter)->m_NSSOnDemandSessionMap.find(strOnDemandSessionId);
		if (sessIter != (*iter)->m_NSSOnDemandSessionMap.end())
		{
			LeaveCriticalSection(&_CS);
			return (*sessIter).second;
		}
	}
	LeaveCriticalSection(&_CS);
	return NULL;
}

RTSPClientSession *ngod_daemon_thread
::findSessionBySessionId(const string	&strSessionId)
{
	//find session group first
	EnterCriticalSection(&_CS);
	for (NSSSessionGroupList::iterator iter = m_NSSSessionGroupList->begin();
		iter != m_NSSSessionGroupList->end();iter++)
	{
		//create key
		SessionMapKey key;
		key.strSessionId = strSessionId;

		NSSSessionMap::iterator sessIter =  (*iter)->m_NSSSessionMap.find(key.strSessionId);
		if (sessIter != (*iter)->m_NSSSessionMap.end())
		{
			LeaveCriticalSection(&_CS);
			return (*sessIter).second;
		}
	}
	LeaveCriticalSection(&_CS);
	return NULL;
}

void ngod_daemon_thread::syncSessionGroup(string &strSessionGroup)
{
	//find session group first
	NSSSessionGroupList::iterator iter = find_if(m_NSSSessionGroupList->begin(), 
												 m_NSSSessionGroupList->end(),
												 FindBySessionGroup(strSessionGroup));
	
	if (iter == m_NSSSessionGroupList->end())
		return;
	else
	{
		//set group status
		while ((*iter)->m_SessionGroupStatus.getStatus() == Sync)
			Sleep(1);

		//(*iter)->m_SessionGroupStatus.setStatus(Sync);
		
		//todo
		//to create thread pool request
		//wait until the request return
		ngod_rtsp_action::GetParameterAction(strSessionGroup, *m_NSSSessionGroupList, *m_pPool, *m_pLogFile);
	}
}

void ngod_daemon_thread::InitEvent()
{
	//initialize event
	m_Event = ::CreateEvent(NULL, true, false, NULL);
	::ResetEvent(m_Event);
}

void ngod_daemon_thread::initRTSPClientSession(RTSPClientSession &sess, 
											   NSSSessionGroup& pNSSSessionGroup)
{
	//init socket
	sess.RTSPSocket = &pNSSSessionGroup.m_SessionSocket.m_Socket;

	//init critical section
	sess.m_pCS = &pNSSSessionGroup.m_CS;
	sess.m_pCS_ClientSeq = &pNSSSessionGroup.m_CS_ClientSeq;
	sess.m_pCS_ServerSeq = &pNSSSessionGroup.m_CS_ServerSeq;

	//init rtsp request path and port
	sess.strServerPath = pNSSSessionGroup.strServerPath;
	sess.uServerPort = pNSSSessionGroup.uServerPort;

	//init session group number
	sess.m_RTSPR2Header.SessionGroup.strToken = pNSSSessionGroup.strSessionGroup;
	
	sess.m_RTSPR2Header.Require.strComPath = pComPath;
	sess.m_RTSPR2Header.Require.strInterface_id = pInterfaceId;
	
	//following is optional header content
	sess.m_RTSPR2Header.StreamControlProto.strType = pStreamControlProtoType;
}