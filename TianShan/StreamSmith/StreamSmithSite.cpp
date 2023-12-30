#include "StreamSmithSite.h"
#include "global.h"
#include <Log.h>
#include "RtspUtil.hxx"
#include <descCode.h>
#include <RtspDialog.h>
#include <math.h>
#include "SystemUtils.h"
#include "StreamSmithService.h"

#if defined(ZQ_OS_MSWIN)
#include <BaseZQServiceApplication.h>
#elif defined(ZQ_OS_LINUX)
#include <ZQDaemon.h>
#endif//ZQ_OS_MSWIN

extern BaseZQServiceApplication	*Application;

#ifdef TEST
#include <ScLog.h>
#endif

#ifndef _RTSP_PROXY
	#include <streamsmithconfig.h>
#else
	#include <rtspProxyConfig.h>
#endif

#if defined (ZQ_OS_MSWIN) && defined(_DEBUG)
	#include "ADebugMem.h"
#endif

extern bool gServiceHealthy;


class KillService: public ZQ::common::NativeThread
{
protected:
	int run()
	{
		extern ZQ::common::BaseZQServiceApplication* Application;
		if(!Application)
			return -1;
		StreamSmithService* svc = (StreamSmithService*)Application;
		svc->OnStop();
		svc->OnUnInit();
		return 0;
	}

	void final()
	{
		delete this;
	}
};

volatile int64 lastIdleStamp = 0;
ZQ::common::Mutex idleStampLOcker;

const std::string url_in_setup("url_in_setup");

int64 getBusyTime( ZQ::common::NativeThreadPool& pool)
{
	if( pool.activeCount() != pool.size() || lastIdleStamp<= 0 )
		return 0;
	int64 delta = 0;
	{
		ZQ::common::MutexGuard gd(idleStampLOcker);
		delta = ZQ::common::now() - lastIdleStamp;;
	}
	return delta;
}

void updateLastIdleStamp( )
{
	ZQ::common::MutexGuard gd(idleStampLOcker);
	lastIdleStamp = ZQ::common::now();
}

namespace ZQ{
namespace StreamSmith {
		


#define		SESSID(x)	"[defaultPlugin]SESSID(%s)  "#x,pSessionID

#define		SSREQ(x)	"[defaultPlugin]SESSID(%s)REQ(%p)  "#x,pSessionID,pReq

#define		REQ(x)		"[defaultPlugin]REQ(%p)  "#x,pReq



template<class classType>
class auto_release
{
public:
	auto_release(classType _p):_ptr(_p)
	{;}
	~auto_release()
	{
		if(_ptr!=NULL)
			_ptr->release();
	}	
public:
	classType& operator->()
	{
		return _ptr;
	}
	classType& operator=(const classType t)
	{
		_ptr=t;
	}
	
//	classType& operator!()
//	{
//		return _ptr;
//	}
	operator classType()
	{
		return _ptr;
	}
	bool operator==(classType& t)
	{
		if(t==_ptr)
			return true;
		else
			return false;
	}
	bool operator!()
	{
		return !_ptr;
	}
public:
	classType	_ptr;
};


using namespace ZQ;
using namespace ZQ::common;

ZQ::common::NativeThreadPool					g_ThreadPool(20);

ZQ::common::NativeThreadPool&					_gThreadPool=g_ThreadPool;




StreamSmithSite*								StreamSmithSite::_pDefaultSite	=NULL;
#ifndef _RTSP_PROXY
VstrmClass*										StreamSmithSite::m_pvstrmClass=NULL;
PlaylistManager*								StreamSmithSite::m_pPlayListManager=NULL;
#endif

StreamSmithSite::EventFire*						StreamSmithSite::m_pEventFire=NULL;
ZQ::StreamSmith::StreamSmithUtilityImpl			ultility;
ZQ::StreamSmith::StreamSmithUtilityImpl*		pUtility = (ZQ::StreamSmith::StreamSmithUtilityImpl* )(&ultility);
#ifndef TEST
	RtspSessionMgr*								StreamSmithSite:: m_pSessionMgr=NULL;
#endif

std::string										StreamSmithSite::_strApplicationLogFolder;
std::vector< StreamSmithSite* >					StreamSmithSite::_stackStreamSmithSite;

std::vector<DynSharedObj*>					    StreamSmithSite::_stackPluginModuleHandle;

StreamSmithSite::EventSinkStack					StreamSmithSite::_stackEventSink;
StreamSmithSite::FixupRequestHandler::Stack		StreamSmithSite::_stackFixupRequest;
StreamSmithSite::FixupResponseHandler::Stack	StreamSmithSite::_stackFixupResponse;
StreamSmithSite::FixupServerRequestHandler::Stack	StreamSmithSite::_stackFixupServerRequest;

SSMH_ContentHandle								StreamSmithSite::_defaultContentHandler=NULL;
std::string										StreamSmithSite::_strIDsServerAddr;
ZQ::common::Log*								StreamSmithSite::_PluginLog=NULL;
std::vector<int>								StreamSmithSite::_defaultSpigotsBoardIDs;

StreamSmithSite									defaultSite;

#define	STR_URI_KEY_PRE  "Item"

const char GLOBAL_GUID[]="Guid"; 

#define STR_CLIENT_RTSP_URI	"Client RTSP URI"


RequestProcessResult SSMH_DefaultContentHandleSetup (IStreamSmithSite* pSite, IClientRequest* pReq)
{
	pSite->CoreLog(Log::L_DEBUG,REQ("Enter SETUP"));
	IServerResponse* pResponse=pReq->getResponse();
	if(!pResponse)
	{
		pSite->CoreLog(Log::L_ERROR,REQ("Can't get IServerResponse through IClientRequest"));
		return RequestError;
	}
		
	char				szBuf[1024];
	uint16	iStrLen=sizeof(szBuf);
	memset(szBuf, 0, iStrLen);

	pReq->getUri(szBuf,iStrLen);
	pSite->CoreLog(Log::L_DEBUG,REQ("URI is % s"),szBuf);
	ZQ::common::URLStr	parser(szBuf,true);
	
	/*	
	{//Check URI
		if(strlen(parser.GetContent())<=0)
		{//Invalid content
			pSite->CoreLog(Log::L_ERROR,REQ("No content value in the uri string=%s"),szBuf);
			pResponse->printf_preheader(RESPONSE_BAD_REQUEST);
			pResponse->post();			
			return RequestError;
		}
	}
	*/
	
	//Get target IP and port
	tstring strDestination;
	int		destPort;

	
	iStrLen=sizeof(szBuf);
	memset(szBuf, 0, iStrLen);
	
	pReq->getTransportParam(KEY_TRANS_DEST,szBuf,&iStrLen);
	
	strDestination=szBuf;
	pSite->CoreLog(Log::L_DEBUG,REQ("Get destination =%s in Transport"),szBuf);
	
	iStrLen=sizeof(szBuf);
	
	pReq->getTransportParam(KEY_TRANS_CPORTA,szBuf,&iStrLen);
	
	destPort=atoi(szBuf);	
	pSite->CoreLog(Log::L_DEBUG,REQ("Get Port=%d in transport"),destPort);

	
	char	*pDestinationIP=NULL;
	char	szDestinationIPBuffer[32];
	memset(szDestinationIPBuffer, 0, 32);
	
	try
	{
		
		
		if(strDestination.size()<=0)
		{//Get peer IP
			pSite->CoreLog(Log::L_DEBUG,REQ("No Destination in transport,get it from peer IP"));
			//******************************************************************************//
			//Get remote IP from request
			//******************************************************************************//
			IClientRequest::RemoteInfo ri;
			ri.size = sizeof(ri);
			ri.ipaddr = szDestinationIPBuffer;
			ri.addrlen = sizeof(szDestinationIPBuffer);
			if (!pReq->getRemoteInfo(ri))
			{
				pSite->CoreLog(Log::L_ERROR,REQ("Can't Get remote IP from request and return ip is %s"),ri.ipaddr);			
				return RequestError;
			}
			pDestinationIP = szDestinationIPBuffer;			

			pSite->CoreLog(Log::L_DEBUG,REQ("Get destination IP=%s from peer IP"),pDestinationIP);
		}
		else
		{
			pDestinationIP=(char*)strDestination.c_str();
		}
		//check the IP
		int a,b,c,d;
		sscanf(pDestinationIP,("%d.%d.%d.%d"),&a,&b,&c,&d);
		if(!((a>=0&&a<=255)&&(a>=0&&a<=255)&&(b>=0&&b<=255)&&(c>=0&&c<=255)&&(d>=0&&d<=255)))
		{//It's not good IP address,reject it!HAHA
			pSite->CoreLog(Log::L_ERROR,REQ("It's a unavailable IP = %s"),pDestinationIP);
			pResponse->printf_preheader(RESPONSE_BAD_REQUEST); 
			pResponse->post();			
			return RequestError;
		}

	}
	catch(VariantException e)
	{
		pSite->CoreLog(Log::L_ERROR,REQ("%s"),e.what());
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR); 
		pResponse->post();		
		return RequestError;
	}
	catch (...)
	{
		pSite->CoreLog(Log::L_ERROR,REQ("Unexpect exception was threw out when Get remote IP and port"));
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR); 
		pResponse->post();		
		return RequestError;

	}
	

	ZQ::common::Guid	uid;
	uid.create();
	
	IPlaylist* pList=pSite->openPlaylist(uid,true);
	uid.toString(szBuf,sizeof(szBuf)-1);
	pSite->CoreLog(Log::L_DEBUG,REQ("Open Playlist=%p with UID=%s"),pList,szBuf);
	if(!pList)
	{
		pSite->CoreLog(Log::L_ERROR,REQ("No playlist and create playlist fail"));
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR); 
		pResponse->post();		
		return RequestError;
	}
	IPlaylist::Item pi;
	memset(&pi, 0, sizeof(IPlaylist::Item));
	pi._criticalStart=0;
	pi._flags=0;
	pi._forceNormal=false;
	pi._inTimeOffset=0;
	pi._outTimeOffset=0;
	pi._spliceIn=false;
	pi._spliceOut=false;
	
	bool	bOK=true;
	char	szItemNameBuf[32];
	int ItemCount=0;

	while (bOK)
	{
		sprintf(szItemNameBuf,"%s%d",STR_URI_KEY_PRE,ItemCount);
		const char * pElement=parser.getVar(szItemNameBuf);
		if(!pElement)
		{
			bOK=false;
			break;
		}
		ItemCount++;
		strcpy(pi._fileName,"\\vod\\");
		strcat(pi._fileName,pElement);
		
		pSite->CoreLog(Log::L_DEBUG,REQ("file name=%s"),pi._fileName);
		pList->push_back(pi);		
	}
	if(ItemCount<=0)
	{
		pSite->CoreLog(Log::L_ERROR,REQ("No item push back into playlist"));
		pResponse->printf_preheader(RESPONSE_BAD_REQUEST); 
		pResponse->post();	
		return RequestError;	
	}
	
	const char* pSessionID=pReq->getClientSessionId();
	pSite->CoreLog(Log::L_DEBUG,REQ("Get client Session ID=%s"),pSessionID);

	memset(szBuf, 0, sizeof(szBuf));
	pReq->getUri(szBuf,sizeof(szBuf)-1);
	IClientSession* pNewSession=pSite->createClientSession(NULL,szBuf);

	pSite->CoreLog(Log::L_DEBUG,REQ("Open client Session=%p and now Session iD=%s"),pNewSession,pNewSession->getSessionID());
	if(pNewSession==NULL)
	{
		pList->destroy();
		pSite->CoreLog(Log::L_ERROR,REQ("Can't open client Session and create ClientSession fail"));
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR); 
		pResponse->post();		
		return RequestError;
	}
	
	uid.toString(szBuf,sizeof(szBuf)-1);
	pNewSession->set(GLOBAL_GUID,szBuf);
	


	
#ifdef _TEST_PLAY_WITH_MAC_	
	pList->setDestMac("1:2:3:4:5:6");	
	pDestinationIP="127.0.0.1";
	pList->setDestination("127.0.0.1",destPort);
	pSite->CoreLog(Log::L_DEBUG,REQ("set destination=%s and port=%d"),"127.0.0.1",destPort);
#else
	pSite->CoreLog(Log::L_DEBUG,REQ("set destination=%s and port=%d"),pDestinationIP,destPort);
	pList->setDestination(pDestinationIP,destPort);
#endif

	


		
	pResponse->printf_preheader(RESPONSE_OK);

	//pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf)-1);	
	pResponse->setHeader(HEADER_MTHDCODE, "Setup");

	pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");	
	uint16 sLen=sizeof(szBuf);
	pReq->getHeader(HEADER_TRANSPORT,szBuf,&sLen);
	
	pResponse->setHeader(HEADER_SESSION,pNewSession->getSessionID());
		
	if(strstr(szBuf,"destination")==NULL)
	{
		strcat(szBuf,";destination=");
		strcat(szBuf,pDestinationIP);
	}

	pResponse->setHeader(HEADER_TRANSPORT,szBuf);		
	pResponse->post();

	pSite->CoreLog(Log::L_DEBUG,REQ("Leave SETUP"));
	return RequestDone;
}
RequestProcessResult SSMH_DefaultContentHandlePlay(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	const char* pSessionID=pReq->getClientSessionId();	
	pSite->CoreLog(Log::L_DEBUG,SSREQ("Enter Play"));	
	
	IServerResponse* pResponse=pReq->getResponse();
	if(!pResponse)
	{		
		pSite->CoreLog(Log::L_DEBUG,SSREQ("Can't get IServerResponse through IClientRequest"));
		return RequestError;
	}
	char				szBuf[1024];
	uint16	iStrLen=sizeof(szBuf);
	memset(szBuf, 0, iStrLen);

	auto_release<IClientSession*> pClientSession=pSite->findClientSession(pSessionID);
	if(pClientSession==NULL)
	{		
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't open client session"));
		pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND); 
		pResponse->post();		
		return RequestError;
	}
	
	ZQ::common::Variant varGuid=pClientSession->get(GLOBAL_GUID);

	tstring GuidStr=(tstring&)varGuid;


	ZQ::common::Guid playListGuid(GuidStr.c_str());
	IPlaylist* pList=pSite->openPlaylist(playListGuid,false);			
	if(!pList)
	{	
		playListGuid.toString(szBuf,sizeof(szBuf)-1);
		pSite->CoreLog(Log::L_ERROR,SSREQ("Get playlist fail with VERB==PLAY with Guid=%s"),szBuf);
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR); 
		pResponse->post();		
		return RequestError;		
	}


	uint16	iStrSize=sizeof(szBuf)-1;
	double fScale=1.0f;
	if(NULL==pReq->getHeader(HEADER_SCALE,szBuf,&iStrSize))
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("No Scale value,Set it to 1"));
	}
	else
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("Get Scale is %s"),szBuf);
		fScale=atof(szBuf);
	}	
	
	if(fabs(fScale)-0.01f<0)
			fScale=1.0f;
	if(pList->getCurrentState()!=IPlaylist::PLAYLIST_SETUP)
	pList->setSpeed((float)fScale);
	
	
	if(pList->getCurrentState()==IPlaylist::PLAYLIST_SETUP)
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("playlist status=SETUP"));		
		pList->setUserCtxIdx(pSessionID);
		if(!pList->play())
		{
			pSite->CoreLog(Log::L_ERROR,SSREQ("Play fail,delete it"));
			pList->destroy();
		
			pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
			pResponse->post();
		}
		else
		{	
			pSite->CoreLog(Log::L_DEBUG,SSREQ("Play OK"));			
			pResponse->printf_preheader(RESPONSE_OK);
			
			/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf)-1);*/
			pResponse->setHeader(HEADER_MTHDCODE, "Play");
			pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
			//pResponse->setHeader(HEADER_RANGE, "npt=10.400-530.000");
			pResponse->setHeader(HEADER_SCALE, "1.0000");
			pResponse->post();			
		}
		
	}
	else if(pList->getCurrentState()==IPlaylist::PLAYLIST_PAUSE)
	{//resume
		pSite->CoreLog(Log::L_DEBUG,SSREQ("playlist status=PAUSE"));
		if(!pList->resume())
		{
			pSite->CoreLog(Log::L_ERROR,SSREQ("resume playlist fail"));
			pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
			pResponse->post();
		}
		else
		{
			pSite->CoreLog(Log::L_DEBUG,SSREQ("resume playlist success"));
			pResponse->printf_preheader(RESPONSE_OK);

			/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf)-1);*/
			pResponse->setHeader(HEADER_MTHDCODE, "Play");
			pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
			pResponse->setHeader(HEADER_RANGE, "npt=10.400-530.000");
			pResponse->setHeader(HEADER_SCALE, "1.0000");

			pResponse->post();
		}
		
	}
	else if(pList->getCurrentState()==IPlaylist::PLAYLIST_PLAY)
	{//playing
		pSite->CoreLog(Log::L_DEBUG,SSREQ("playlist status=PLAY"));
		pResponse->printf_preheader(RESPONSE_OK);		
		
		/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf));*/
		pResponse->setHeader(HEADER_MTHDCODE, "Play");
		pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
		pResponse->setHeader(HEADER_RANGE, "npt=10.400-530.000");
		pResponse->setHeader(HEADER_SCALE, "1.0000");

		pResponse->post();
	}
	else if(pList->getCurrentState()==IPlaylist::PLAYLIST_STOP)
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("playlist status=STOP"));
		pResponse->printf_preheader(RESPONSE_BAD_REQUEST);		
		
		/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf));*/
		pResponse->setHeader(HEADER_MTHDCODE, "play");
		pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
		pResponse->post();		
	}
	pSite->CoreLog(Log::L_DEBUG,SSREQ("Leave Play"));
	return RequestDone;
}

RequestProcessResult SSMH_DefaultContentHandlePause (IStreamSmithSite* pSite, IClientRequest* pReq)
{
	const char*			pSessionID=pReq->getClientSessionId();
	pSite->CoreLog(Log::L_DEBUG,SSREQ("Enter Pause"));	

	IServerResponse* pResponse=pReq->getResponse();
	if(!pResponse)
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("Can't get IServerResponse through IClientRequest"));
		return RequestError;
	}

	char				szBuf[1024];
	uint16	iStrLen=sizeof(szBuf);
	memset(szBuf, 0, iStrLen);

	auto_release<IClientSession*>		pClientSession=pSite->findClientSession(pSessionID);
	if(pClientSession==NULL)
	{
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't find ClientSession"));
		pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND);
		pResponse->post();		
		return RequestError;
	}
	
	ZQ::common::Variant varGuid = pClientSession->get(GLOBAL_GUID);
	tstring GuidStr=(tstring&)varGuid;
	ZQ::common::Guid playListGuid(GuidStr.c_str());
	IPlaylist* pList=pSite->openPlaylist(playListGuid,false);
	if(!pList)
	{		
		playListGuid.toString(szBuf,sizeof(szBuf));
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't find PlayList with GUID=%s"),szBuf);
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
		pResponse->post();		
		return RequestError;
	}
	if(pList->getCurrentState()==IPlaylist::PLAYLIST_STOP)
	{		
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't pause when playlist stop"));
		pResponse->printf_preheader(RESPONSE_BAD_REQUEST);
		pResponse->post();		
		return RequestDone;
	}
	if(!pList->pause())
	{
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't pause playlist"));
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
		pResponse->post();
	}
	else
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("pause the playlist success "));
		pResponse->printf_preheader(RESPONSE_OK);
		/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf));*/
		pResponse->setHeader(HEADER_MTHDCODE, "Pause");
		pResponse->post();
	}	
	pSite->CoreLog(Log::L_DEBUG,SSREQ("Leave Pause"));
	return RequestDone;
}

RequestProcessResult SSMH_DefaultContentHandleTearDown (IStreamSmithSite* pSite, IClientRequest* pReq)
{
	const char* pSessionID=pReq->getClientSessionId();
	pSite->CoreLog(Log::L_DEBUG,SSREQ("Enter TearDown"));
	
	IServerResponse* pResponse=pReq->getResponse();
	if(!pResponse)
	{
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't get IServerResponse through IClientRequest"));
		return RequestError;
	}

	char				szBuf[1024];
	uint16	iStrLen=sizeof(szBuf);
	memset(szBuf, 0, iStrLen);

			
 	auto_release<IClientSession*> pClientSession=pSite->findClientSession(pSessionID);
	if(pClientSession==NULL)
	{		
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't open client session"));
		pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND); 
		pResponse->post();		
		// pClientSession->release();
		return RequestError;
	}
	ZQ::common::Variant varTemp=pClientSession->get(GLOBAL_GUID);
	tstring GuidStr=(tstring&)varTemp;
	//tstring GuidStr=(tstring&)pClientSession->get(GLOBAL_GUID);
	
	ZQ::common::Guid playListGuid(GuidStr.c_str());
	IPlaylist* pList=pSite->openPlaylist(playListGuid,false);	
	
	if(pList)
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("find the play list ,Destroy it now!"));
		pList->destroy();
	}
	else
	{				
		pSite->CoreLog(Log::L_DEBUG,SSREQ("No available PlayList need to be destroyed "));
	}	
	pResponse->printf_preheader(RESPONSE_OK);
	
	/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf));*/
	pResponse->setHeader(HEADER_MTHDCODE, "Teardown");	

	pResponse->post();	
	
	/*pClientSession->release();*/
	pSite->destroyClientSession((char*)pReq->getClientSessionId());
//	pCon->close();	
	
	pSite->CoreLog(Log::L_DEBUG,SSREQ("Leave Teardown"));
	return RequestDone;
}
RequestProcessResult SSMH_DefaultContentHandleOption (IStreamSmithSite* pSite, IClientRequest* pReq)
{
	pSite->CoreLog(Log::L_DEBUG,REQ("Enter OPTION"));
	
	IServerResponse* pResponse=pReq->getResponse();
	if(!pResponse)
	{
		pSite->CoreLog(Log::L_DEBUG,REQ("Can't get IServerResponse through IClientRequest"));
		return RequestError;
	}

	char	szBuf[1024];
	memset(szBuf, 0, 1024);

	uint16	iLen=sizeof(szBuf);
	pSite->CoreLog(Log::L_DEBUG,REQ("uri is %s"),pReq->getUri(szBuf,sizeof(szBuf)-1));

	pResponse->printf_preheader(RESPONSE_OK);
	
	pReq->getHeader(HEADER_SEQ,szBuf,&iLen);
	pResponse->setHeader(HEADER_SEQ,szBuf);

	pResponse->setHeader("Public"," DESCRIBLE, OPTION, SETUP, TEARDOWN, PLAY, PAUSE");
	
	/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf));*/
	pResponse->setHeader(HEADER_MTHDCODE, "Option");

	pResponse->post();
	pSite->CoreLog(Log::L_DEBUG,REQ("Leave OPTION"));
	return RequestProcessed;
}
RequestProcessResult SSMH_DefaultContentHandleDescribe(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	pSite->CoreLog(Log::L_DEBUG,REQ("Enter Describle"));
	IServerResponse* pResponse=pReq->getResponse();
	if(!pResponse)
	{
		pSite->CoreLog(Log::L_DEBUG,REQ("Can't get IServerResponse through IClientRequest"));
		return RequestError;
	}

	char szBuf[1024];
	memset(szBuf, 0, 1024);
	

	pResponse->printf_preheader(RESPONSE_OK);
	pResponse->printf_postheader("m=video 0 RAW/RAW/UDP 33\r\nc=IN IP4 0.0.0.0/255\r\na=control:basic");
	pResponse->post();	

	pSite->CoreLog(Log::L_DEBUG,REQ("Leave Describle"));
	return RequestDone;
}
//Default content Handler
RequestProcessResult SSMH_DefaultContentHandle (IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	//should never be here
	IServerResponse* pResponse =  pReq->getResponse();
	if(!pResponse)
	{
		pSite->CoreLog(ZQ::common::Log::L_ERROR,REQ("SSMH_DefaultContentHandle() Can't get Server Response interface "));
		return RequestError;
	}
	pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
	pResponse->printf_postheader("default function is not allowed to enter,maybe configuration error");
	pResponse->post();
	return RequestDone;
	
	switch(pReq->getVerb()) 
	{
	case RTSP_MTHD_SETUP:
		{	
#ifdef HANDLER_TEST
			return	ContentHandleSetup(pSite,pReq);
#endif
			return SSMH_DefaultContentHandleSetup(pSite,pReq);
		}
		break;
	case RTSP_MTHD_PLAY:
		{
#ifdef HANDLER_TEST
			return ContentHandlePlay(pSite,pReq);
#endif
			return SSMH_DefaultContentHandlePlay(pSite,pReq);
		}
		break;
	case RTSP_MTHD_PAUSE:
		{
			return SSMH_DefaultContentHandlePause(pSite,pReq);
		}
		break;		
	case RTSP_MTHD_TEARDOWN:
		{
#ifdef HANDLER_TEST
			return ContentHandleTearDown(pSite,pReq);
#endif
			return SSMH_DefaultContentHandleTearDown(pSite,pReq);
		}
		break;
	case RTSP_MTHD_OPTIONS:
		{
			return SSMH_DefaultContentHandleOption(pSite,pReq);
		}
		break;
	case RTSP_MTHD_DESCRIBE:
		{
			return SSMH_DefaultContentHandleDescribe(pSite,pReq);
		}
		break;
	case RTSP_MTHD_GET_PARAMETER:
		{
			//find the client session at first
			const char *pSessId=pReq->getClientSessionId();
			auto_release<IClientSession*> pSession=pSite->findClientSession(pSessId);
			if(!pSession)
			{
				IServerResponse *pResponse = pReq->getResponse();
				pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND);
				pResponse->setHeader(HEADER_MTHDCODE,"GetParameter");
				pResponse->post();
				return RequestDone;//return done because this is the last phase
			}
		}
		break;
	default:
		{
			pSite->CoreLog(Log::L_DEBUG,REQ("Enter RequestProcessResult SSMH_DummyContentHandle():: but VERB was not supported"));
			IServerResponse* pResponse=pReq->getResponse();
			if(!pResponse)
			{
				pSite->CoreLog(Log::L_DEBUG,REQ("Can't get IServerResponse through IClientRequest"));
				return RequestError;
			}

			pSite->CoreLog(Log::L_DEBUG,REQ("####NOT IMPLEMENT####"));
		
			//pResponse->printf_preheader(RESPONSE_NOT_IMPLEMENT); 
			pResponse->printf_preheader(RESPONSE_OK); 
			pResponse->post();			
			return RequestDone;
		}
		break;
	}	
	return RequestDone;	
}


void SSMH_fixupURIWithPlayStar(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	try
	{
		const char* pSessID=pReq->getClientSessionId();
		if ( !(pSessID&&strlen(pSessID)>0) )
		{
			glog(Log::L_DEBUG,"[ SSMH_fixupURIWithPlayStar ] can't get session id from client request,No uri will be fixed");
			return;
		}
		auto_release<IClientSession*> pSession=pSite->findClientSession(pSessID);
		if(!pSession)
		{
			glog(Log::L_ERROR,"[ SSMH_fixupURIWithPlayStar ] can't find client session through sessid=%s, no URI will be fixed",pSessID);
			return;
		}
		ZQ::common::Variant var=pSession->get(RESERVED_ATTR(RtspURI));
		std::string uri=(std::string)var;
//		std::string uri=(std::string)(pSession->get(RESERVED_ATTR(RtspURI)));
		
		char szProtocol[256];
		memset(szProtocol, 0, sizeof(szProtocol));
		pReq->getProtocol(szProtocol,sizeof(szProtocol)-1);
		pReq->setArgument(pReq->getVerb(),uri.c_str(),szProtocol);
	}
	catch (...)
	{
		glog(Log::L_ERROR,"unexpect error when SSMH_fixupURIWithPlayStar");
	}
}
///default fix up request
RequestProcessResult SSMH_DefaultFixupRequest (IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	char	szBuf[1024];
	memset(szBuf, 0, 1024);
	if(pReq->getVerb()!=RTSP_MTHD_SETUP)
	{
		SSMH_fixupURIWithPlayStar(pSite,pReq);
	}
//	pReq->getUri(szBuf,1023);
//	glog(Log::L_DEBUG,REQ("Default fixup request phase with URI=%s"),szBuf);
	return RequestProcessed;
}
///default fix up response 
RequestProcessResult SSMH_DefaultFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	glog(Log::L_DEBUG,REQ("Default Fixup Response phase was called"));
	return RequestDone;
}


// -----------------------------
// class StreamSmithSitework
// -----------------------------
#define SSSITE(x)	"[StreamSmithSite] "x
StreamSmithSite::StreamSmithSite(const char* pSitename)
{
	glog(Log::L_DEBUG,SSSITE("Create a new Site with site name=%s"),pSitename);
	_siteName=pSitename;
}
StreamSmithSite::StreamSmithSite()
{

}
StreamSmithSite::~StreamSmithSite()
{

}


//////////////////////////////////////////////////////////////////////////
// added by Cary

void StreamSmithSite::RegisterSessionDrop(SSMH_SessionDrop sessionDropHandle)
{
	_sessionDropHandlers.push_back(sessionDropHandle);
}

void StreamSmithSite::onSessionRemoved(const std::string& sessionId)
{
	SessionDropHandlers::iterator itor = _sessionDropHandlers.begin();
	for (; itor != _sessionDropHandlers.end(); itor ++) {
		SSMH_SessionDrop sessionDropHandler = *itor;
		try
		{
			sessionDropHandler(sessionId.c_str());
		}
		catch (...) {
		}
	}
}

//////////////////////////////////////////////////////////////////////////
#define STROK(x) (x!=NULL && x[0]!=0)
bool StreamSmithSite::checkRequestWithSessionPriority( IClientRequestWriter * pReq ,const char* sessId ,int& priority)
{
	IClientRequestWriterInternal* req =(IClientRequestWriterInternal*)pReq;
	priority = -1; //set to -1 as default
	if( !STROK(sessId))
		return true;//no session id attached
	//get KEY_REQUEST_PRIORITY
	IClientSession* clientSess = findClientSession(sessId);
	if(!clientSess )
		return true;

	ZQ::common::Variant varPriority = clientSess->get(KEY_REQUEST_PRIORITY);
	clientSess->release();

	if(!varPriority.valid())
		return true;
	priority = (int)varPriority;

	if( priority < 0 )
		return true;

	int64 reqInitTime = req->getRequestInitTime();
	int64 curr = ZQ::common::now();

	char			szSeq[32];
	uint16 iSeqSize =sizeof(szSeq)-1;
	memset(szSeq, 0, sizeof(szSeq));
	req->getHeader("CSeq",szSeq,&iSeqSize);

#ifdef _RTSP_PROXY
	if( GAPPLICATIONCONFIGURATION.lLowPriorityRequestTimeout < ( curr - reqInitTime) )
	{		
		glog(ZQ::common::Log::L_WARNING, SSSITE("postRequest() sess[%s]seq[%s]priority[%d] timed out, created at[%lld] now[%lld] delta[%lld]"),
			sessId,szSeq,priority,reqInitTime,curr , curr-reqInitTime);

		IServerResponse* pResponse = req->getResponse();
		if(pResponse)
		{
			pResponse->printf_preheader("RTSP/1.0 503 Service Unavailable");
			pResponse->post();
			return false;
		}
		return false;
	}

#endif//_RTSP_PROXY
	glog(ZQ::common::Log::L_DEBUG,SSSITE("postRequest() sess[%s]seq[%s] set priority to[%d]"),
		sessId,szSeq,priority);
	return true;
}


void ThreadPoolBusyToDie( ZQ::common::NativeThreadPool& pool)
{
#ifdef _RTSP_PROXY

	int64 busyInterval = GAPPLICATIONCONFIGURATION.lRestartAtBusyHang;
	if( busyInterval <= 10000)
		busyInterval = 10000;

	_GlobalObject::pendingRequest = (int32)pool.pendingRequestSize();
	_GlobalObject::activeThreads = (int32)pool.activeCount();
	_GlobalObject::lastIdleStampDelta = lastIdleStamp >0 ? (ZQ::common::now() - lastIdleStamp) : 0;

	if( getBusyTime(pool) >= busyInterval )
	{
		glog(ZQ::common::Log::L_EMERG,CLOGFMT(ThreadPoolBusyToDie,"the service is out-of-service due to thread blocked, force to quit the program in order to clean up"));
		if( GAPPLICATIONCONFIGURATION.lRestartViaCrash >= 1 )
		{
#ifdef ZQ_OS_MSWIN
			RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
#else

#endif
		}
		else
		{
			
			KillService* k = new KillService();
			k->start();
		}
	}

#endif
}

bool StreamSmithSite::postRequest(IClientRequestWriter* pReq, const IClientRequest::ProcessPhase Procphase)
{
	ThreadPoolBusyToDie(_gThreadPool);

	//glog(Log::L_DEBUG,"new request with Method is %d URI is %s ",pReq->getVerb(),pReq->getUri(szBuf,sizeof(szBuf)-1));
	//
	IClientRequest::ProcessPhase phase= Procphase;
	if (NULL == pReq)
	{
		glog(Log::L_ERROR,SSSITE("postRequest() null pReq pass in"));
		return false;
	}
	const char* pSessId = pReq->getClientSessionId();
	char szSeq[256];
	memset(szSeq,0,sizeof(szSeq));
	uint16 seqBufLen = sizeof(szSeq);
	const char* pSeq = pReq->getHeader("CSeq",szSeq,&seqBufLen);
	
	IStreamSmithSite* pSite = pReq->getSite();
	
	if (NULL == pSite)
	{
		glog(Log::L_CRIT,SSSITE("Get Site fail from client request"));
		return false;
	}
	char szURI[1024];
	pReq->getUri(szURI,sizeof(szURI)-1);

	int curRequestPendingSize	= _gThreadPool.pendingRequestSize();
	int curActiveThreadCount	= _gThreadPool.activeCount();
	int curTotalThreadCount		= _gThreadPool.size();
	int limitedPendingSize		= GAPPLICATIONCONFIGURATION.lMaxPendingRequest;
	
	//check with session to see if relative session has KEY_REQUEST_PRIORITY
	int newPriority = -1;
	if( !checkRequestWithSessionPriority( pReq , pSessId, newPriority) )
		return false;
	
	if (limitedPendingSize> 0 ) 
	{	
		if (curRequestPendingSize > 0 ) 
		{
			glog( (curRequestPendingSize > (limitedPendingSize>>1)) ? ZQ::common::Log::L_WARNING : ZQ::common::Log::L_DEBUG, SSSITE("postRequest() server busy with pending requests[%d/%d]"),
					curRequestPendingSize, limitedPendingSize );

			if( newPriority >= 0 )
				limitedPendingSize = limitedPendingSize>>1;
			
			if ( curRequestPendingSize > limitedPendingSize )
			{
				char			szSeq[32];
				uint16 iSeqSize =sizeof(szSeq)-1;
				memset(szSeq, 0, sizeof(szSeq));
				pReq->getHeader("CSeq",szSeq,&iSeqSize);
				glog(ZQ::common::Log::L_ERROR, SSSITE("postRequest() pending requests[%d] exceeded limition[%d], uri[%s] Seq[%s]"),
					curRequestPendingSize,limitedPendingSize,szURI,szSeq);
				
				IServerResponse* pResponse = pReq->getResponse();
				if(pResponse)
				{
					pResponse->printf_preheader("RTSP/1.0 503 Service Unavailable");
					pResponse->post();
					return false;
				}
			}

			int enableGP2ping = 0;
#ifdef _RTSP_PROXY
			enableGP2ping = GAPPLICATIONCONFIGURATION.lGetParaToPingAtBusy >= 1;
#endif
			if(   enableGP2ping && curRequestPendingSize > (limitedPendingSize / 2 ) )
			{
				REQUEST_VerbCode requestVerb = pReq->getVerb();
				if( requestVerb == RTSP_MTHD_GET_PARAMETER)
				{					
					glog(ZQ::common::Log::L_WARNING,SSSITE("postRequest()  sess[%s] seq[%s]: system busy, converted GET_PARAMETER to PING,"),
						pSessId , szSeq);
					unsigned char* tmp = (unsigned char*)"";
					pReq->setContent( tmp , 0 );					
				}
			}

		}
	}//StreamSmithSite::_stackFixupRequest;
	if (phase==IClientRequest::FixupRequest) 
	{
		if (StreamSmithSite::_stackFixupRequest.size()==0) 
		{
			glog(ZQ::common::Log::L_DEBUG,SSSITE("No fixup request handle,turn current request to contentHandler"));
			phase = IClientRequest::ContentHandle;
		}
	}
	else if(phase == IClientRequest::FixupResponse)
	{
		if (StreamSmithSite::_stackFixupResponse.size() == 0) 
		{
			glog(ZQ::common::Log::L_DEBUG,SSSITE("No fixup response handle,turn current request to PostResponse(That is dismiss it)"));
			phase = IClientRequest::PostResponse;
		}
	}


	switch(phase)
	{
	case IClientRequest::PostRequestRead:
		{
			glog(Log::L_DEBUG,SSSITE("Now URI(%s) in phase PostRequestRead"),szURI);
		}
		//break;

	case IClientRequest::AuthUser:
		{
//			AuthUserHandler* handler = new AuthUserHandler(_gThreadPool, _stackAuthUser, pReq);
//			if (NULL != handler)
//				handler->start();
			glog(Log::L_DEBUG,SSSITE("Now URI(%s) in phase AuthUser"),szURI);
		}
		//break;
		
		/// fix up the request, the last chance to change the request
	case IClientRequest::FixupRequest:
		{			
			glog(Log::L_DEBUG,SSSITE("Request(%p) In phase FixupRequest:[%s][%s][%s]; threadPool: [%d/%d];"
				" pending request [%d]"), 
				pReq, 
				pSessId,
				pSeq,
				szURI,
				curActiveThreadCount,
				curTotalThreadCount,
				curRequestPendingSize );

			if (pReq->getVerb() == RTSP_TEARDOWN_MTHD)
				newPriority = 40;

			FixupRequestHandler* handler = new FixupRequestHandler(_gThreadPool, _stackFixupRequest, pReq,newPriority);			
			if (NULL != handler)				
			{
				handler->start();
			}
			else
			{
				glog(Log::L_ERROR,SSSITE("postRequest() can't create a new FixuprequestHanlder object"));
			}
		}
		break;		
		/// handle the content
	case IClientRequest::ContentHandle:
		{
			if(pReq->getVerb() != RTSP_MTHD_SETUP) {
				const char* pSessId = pReq->getClientSessionId();
				if(pSessId&& pSessId[0]) {
					IClientSession* pClientSession = pSite->findClientSession(pSessId);
					if(pClientSession) {
						const std::string& strUrl  = pClientSession->getProp(url_in_setup);
						if(!strUrl.empty()) {
							strncpy(szURI, strUrl.c_str(), sizeof(szURI)-1);
							szURI[sizeof(szURI)-1] = '\0';
							glog(Log::L_DEBUG, SSSITE("get url[%s] from url_in_setup"), szURI);						
						}
						pClientSession->release();
					}
				}
			}
			glog(Log::L_DEBUG,SSSITE("Now (%s) in phase ContentHandle"),szURI);
			URLStr uri( (std::string("rtsp://")+szURI ).c_str() , true );
			//////////////////////////////////////////////////////////////////////////			
			glog(Log::L_DEBUG,SSSITE("get Site=%s from URI"),uri.getHost());
			StreamSmithSite* pVSSite=findSite(uri.getHost());			
			if(NULL==pVSSite)
			{				
				glog(Log::L_DEBUG,SSSITE("Can't find VSSite with host=%s,turn to default VSSsite"),uri.getHost() );
				pVSSite=findSite(STR_SITE_DEFAULT);				
			}
			glog(Log::L_DEBUG,SSSITE("Find contentHandler=[%s] from URI"),uri.getPath());
			SSMH_ContentHandle contentHandler = pVSSite->findContentHandler(uri.getPath());
			glog(Log::L_DEBUG,SSSITE("Request(%p) In phase ContentHandle:[%s][%s][%s]; threadPool: [%d/%d];"
				" pending request [%d]"), 
				pReq, 
				pSessId,
				pSeq,
				szURI, 
				curActiveThreadCount,
				curTotalThreadCount,
				curRequestPendingSize);
			if (NULL != contentHandler)
			{
				if (pReq->getVerb() == RTSP_TEARDOWN_MTHD)
					newPriority = 40;
				ContentHandler* handler = new ContentHandler(_gThreadPool, contentHandler, pReq,newPriority);
				if (NULL != handler)
					handler->start();
			}
			else
			{
#pragma message(__MSGLOC__"Should I post a BAD REQUEST to client?")
				glog(Log::L_ERROR,SSSITE("can't find the contenthandler with path = [%s]"),uri.getPath());
			}
		}
		break;
		
		/// fixup the response before it is sent thru the connection
	case IClientRequest::FixupResponse:
		{
			//glog(Log::L_DEBUG,"bool StreamSmithSite::postRequest()##Now, in phase FixupResponse");
			glog(Log::L_DEBUG,SSSITE("Request(%p) In phase FixupResponse:[%s][%s][%s]; threadPool: [%d/%d]; "
				"pending request [%d]"), 
				pReq,
				pSessId,
				pSeq,
				szURI, 
				curActiveThreadCount,
				curTotalThreadCount,
				curRequestPendingSize);
			FixupResponseHandler* handler=new FixupResponseHandler(_gThreadPool,_stackFixupResponse,pReq);
			if(NULL!=handler)
			{
				handler->start();
			}
			else
			{
				glog(Log::L_ERROR,SSSITE("StreamSmithSite::postRequest()  Can't create a new FixupResponseHandler object"));
			}
		}
		break;
	case IClientRequest::PostResponse:
		{
			glog(Log::L_DEBUG,SSSITE("StreamSmithSite::postRequest()  Now,in phase PostResponse"));
		}
		break;
	default:
		{
			glog(Log::L_ERROR,SSSITE("StreamSmithSite::postRequest()  Invalid phase"));
		}
		break;
	}
	
	return true;
}
StreamSmithSite::EventSinkManager::EventSinkManager(ZQ::common::NativeThreadPool& pool, 
													StreamSmithSite* pSite, 
													uint32 dwEventCode,
													ZQ::common::Variant& var,
													EventFire& ef,
													const std::string& strGuid ):ThreadRequest(pool),_EventFire(ef)
{
	_var=var;
	_eventCode=dwEventCode;
	_pSite=pSite;
	_strGuid=strGuid;
}
void StreamSmithSite::EventSinkManager::final(int retcode , bool bCancelled )
{
	delete this;
}
int StreamSmithSite::EventSinkManager::run()
{
	try
	{
		_EventFire.addNewEvent(_eventCode,_var,_pSite,_strGuid);
	}
	catch (...) 
	{
		glog(Log::L_ERROR,SSSITE("(EventSinkManager) error ouucrred when add new event into eventfire"));
		return 1;
	}
	return 1;
}

StreamSmithSite::EventFire::EventFire(ZQ::common::NativeThreadPool& pool):ThreadRequest(pool)
{
	_bQuit=false;
	m_SleepTime=60*1000;
}
StreamSmithSite::EventFire::~EventFire()
{
	_bQuit=true;
    m_hNewSinkData.signal();
}

void StreamSmithSite::EventFire::SetQuit()
{
	_bQuit=true;
    m_hNewSinkData.signal();
}

void StreamSmithSite::EventFire::final(int retcode , bool bCancelled )
{
	delete this;
}

#define	CAN_OVERIDE	10000
char geventCodeString[][32]={
	{"E_PLAYLIST_INPROGRESS"},
	{"E_PLAYLIST_STATECHANGED"},
	{"E_PLAYLIST_SPEEDCHANGED"},
	{"E_PLAYLIST_ITEMDONE"},
	{"E_PLAYLIST_END"},
	{"E_PLAYLIST_BEGIN"},
	{"E_PLAYLIST_SESSEXPIRED"},
	{"E_PLAYLIST_DESTROYED"},
	{"E_PLAYLIST_PAUSETIMEOUT"},
	{"E_PLAYLIST_REPOSITION"},	
	{"UnKown Event"}
};

int StreamSmithSite::EventFire::run()
{
	int		sleepTime=10*1000; //10seconds

    while(!_bQuit) {
        SYS::SingleObject::STATE result = m_hNewSinkData.wait();
        if(result == SYS::SingleObject::SIGNALED) {
            while (m_listData.size()>0) 
            {
                StreamSmithSite::EventFire::EventData  data;
                {
                    ZQ::common::MutexGuard gd(m_mutex);
                    bool bValid = true;
                    do
                    {
                        if((int)m_listData.size()<=0) break;							

                        data=m_listData.back();
                        m_listData.pop_back();
                        if(m_listData.size()>50)
                        {
                            glog(Log::L_DEBUG,SSSITE("(EventFire) Now There are %d messages in queue"),m_listData.size());
                        }
                        if (!data._isValid) 
                        {							
                            continue;
                        }
                        int64 dwCur = ZQ::common::now();
                        int64 dwDif=0;
                        if(dwCur >= data._ttl)
                        {
                            dwDif=dwCur - data._ttl;
                        }
                        else
                        {
                            dwDif=0xFFFFFFFFFFFFFFFFll-data._ttl+dwCur;
                        }
                        if(dwDif >= (int64)GAPPLICATIONCONFIGURATION.lEventSinkTimeout)
                        {
                            long lSeq = -1;
                            if (data._var.has(EventField_EventCSEQ)) 
                            {
                                lSeq = (long)data._var[EventField_EventCSEQ];
                            }
                            //glog(Log::L_DEBUG,"message is deleted because ttl <= 0");
                            glog(ZQ::common::Log::L_DEBUG,"EventSinkDetail discard event with type =[%s] playlistId=[%s] Seq=[%d] because expired",
                                geventCodeString[data._msgIndex],data._strGuid.c_str(),lSeq);
                            bValid = false;
                            continue;
                        }
                        else
                        {
                            bValid = true;
                        }
                    }while(!bValid) ;
                }
                try
                {
                    EventSinkStack::iterator	iter;
                    for(iter=_stackEventSink.begin();iter!=_stackEventSink.end();iter++)
                    {
                        if ((iter->first&data._eventCode) && data._pSite) 
                        { 
//								DWORD DWStart=GetTickCount();
                            if(!iter->second(data._pSite,(EventType)data._eventCode,data._var))
                            {
                                glog(Log::L_DEBUG,SSSITE("(EventFire) message  with ttl=%lld and priority=%d can't be sent out,sleep for %dms"),
												data._ttl,data._priority,	sleepTime);									
                                SYS::sleep(sleepTime);									
                                sleepTime*=2;
                                if(sleepTime>m_SleepTime) {
                                    sleepTime=m_SleepTime;									
                                }
                                addNewEvent(data._eventCode,data._var,data._pSite,data._strGuid,false);
                            }
                            else
                            {
                                sleepTime=10*1000;//10seconds;									
#ifndef _RTSP_PROXY
                                long lSeq = -1;
                                if (data._var.has(EventField_EventCSEQ)) 
                                {
                                    lSeq = (long)data._var[EventField_EventCSEQ];
                                }
                                if(gStreamSmithConfig.lEnableShowEventSinkDetail >= 1)
                                {
                                    glog(ZQ::common::Log::L_DEBUG,
                                        "EventSinkDetail Send out event with type =[%s] playlistId=[%s] seq=[%d]",
                                        geventCodeString[data._msgIndex],
                                        data._strGuid.c_str(),
                                        lSeq);
                                }
#endif
                                //if(data._priority==(CAN_OVERIDE+1000))
                                //	glog(Log::L_DEBUG,"$$$$$$Playlist SPEED CHANGED message was sent out now$$$$$$");							
                            }								
                        }
                    }
                }
                catch(...)
                {
                    glog(Log::L_ERROR,SSSITE(" EventFire::run() Unexpect exception was thew out"));
                }
            //glog(Log::L_DEBUG,"One loop run time=%lld", RtspMilliTimeStamp::now()-dwoneLoop);
            }
        }
		if(_bQuit) {
			break;
        }
	}
	return 1;
}

void StreamSmithSite::EventFire::addNewEvent(uint32& dwEventCode,ZQ::common::Variant& EventData,
											 StreamSmithSite* pSite,const std::string& strGuid,bool bNew)
{
	
	StreamSmithSite::EventFire::EventData	data;
	data._eventCode=dwEventCode;
	data._strGuid=strGuid;
	data._var=EventData;
	data._isValid=true;
	data._pSite=pSite;
	if(bNew) {
        data._ttl = ZQ::common::now();
	}
	
	//如果高优先级的消息到来的话，低优先级的消息就会被设定为Invalid
	//也就是这个消息不会被发送
	//理由是：如果ItemDone到来那么Progress就没有意义了！但是下一个Progress的到来可以覆盖掉当前这个Progress
	//而StreamDone的到来肯定意味着ItemDone和Progress都失去了意义
	switch(dwEventCode)
	{
	case E_PLAYLIST_INPROGRESS:
		{
			data._msgIndex	= 0;
			data._priority=CAN_OVERIDE-9000;
		}
		break;
	case E_PLAYLIST_STATECHANGED:
		{
			data._msgIndex = 1;
			data._priority=CAN_OVERIDE+5500;
		}
		break;
	case E_PLAYLIST_SPEEDCHANGED:
		{
			data._msgIndex = 2;
			data._priority=CAN_OVERIDE+1000;
		}
		break;
	case E_PLAYLIST_ITEMDONE:
		{
			data._msgIndex = 3;
			data._priority=CAN_OVERIDE+3000;
		}
		break;
	case E_PLAYLIST_END:
		{
			data._msgIndex = 4;
			data._priority=CAN_OVERIDE+9000;
		}
		break;
	case E_PLAYLIST_BEGIN:
		{	
			data._msgIndex = 5;
			data._priority=CAN_OVERIDE+9000;
		}
		break;
	case E_PLAYLIST_SESSEXPIRED:
		{
			data._msgIndex = 6;
			data._priority=CAN_OVERIDE+10000;
		}
		break;
	case E_PLAYLIST_DESTROYED:
		{
			data._msgIndex = 7;
			data._priority=CAN_OVERIDE+11000;
		}
		break;
	case E_PLAYLIST_PAUSETIMEOUT:
		{
			data._msgIndex = 8;
			data._priority=CAN_OVERIDE+12000;
		}
		break;
	case E_PLAYLIST_REPOSITION:
		{
			data._msgIndex = 9;
			data._priority=CAN_OVERIDE+13000;
		}
		break;
	default:
		{
			data._msgIndex = 8;
			data._priority=0;
		}		
		break;
	}

	ZQ::common::MutexGuard gd(m_mutex);

	EVENTDATALST::iterator it=m_listData.begin();
	
	//////////////////////////////////////////////////////////////////////////
	while (it!=m_listData.end()) 
	{
		if( it->_priority < data._priority )
		{//如果当前的优先级小于新数据的
//			if( it->_strGuid==data._strGuid )
//			{//如果是同一个guid的事件，那么把低优先级的事件直接置为false，不需要发送这个事件
//				it->_isValid=true;
//			}
			it++;
			continue;
		}
		if(it->_priority==data._priority)
		{
			//如果是可以覆盖的事件
			if(it->_priority<CAN_OVERIDE)//该数据可以被覆盖
			{
				EVENTDATALST::iterator itTemp=it;
				for(;it!=m_listData.end();it++)
				{
					if(it->_priority >data._priority)
					{//如果当前的优先级高于新数据的优先级，那么就是没有找到可以覆盖的数据
						break;
					}
					if ( it->_eventCode==data._eventCode && it->_strGuid==data._strGuid ) 
					{//找到可以被覆盖的数据，干掉之!
						*it=data;
						m_hNewSinkData.signal();
						return;
					}
				}
				//没有找到可以被覆盖的数据，那么直接插入新的数据到itTemp
				it=itTemp;
				break;
			}
			else
			{
				//数据不能被覆盖
				break;
			}
		}
		else
		{
			//如果当前的优先级比新数据的优先高，那么就直接插入新的数据
			break;
		}
	}
	m_listData.insert(it,data);
	m_hNewSinkData.signal();
	return;	

}
#undef CAN_OVERIDE

void StreamSmithSite::PostEventSink(uint32 dwEventCode,ZQ::common::Variant& EventData,const std::string& strGuid)
{
	EventSinkStack::iterator	iter;

//#pragma message(__MSGLOC__"TEST HERE %&%(*&#(@(&$&@^$(@*&$(^@($&$(@*&$(*@$&(*@&$(^$^")
//	return ;
//#pragma message(__MSGLOC__"TEST HERE %&%(*&#(@(&$&@^$(@*&$(^@($&$(@*&$(*@$&(*@&$(^$^")

	try
	{
		for(iter=_stackEventSink.begin();iter!=_stackEventSink.end();iter++)
		{
			if (iter->first&dwEventCode) 
			{ 				
				StreamSmithSite::EventSinkManager *pSink=new StreamSmithSite::EventSinkManager(_gThreadPool,this,dwEventCode,EventData,*StreamSmithSite::m_pEventFire,strGuid);
				if(!pSink)
				{
					glog(Log::L_ERROR,SSSITE("PostEventSink()  Create event sink object fail"));
					return;
				}
				pSink->start();				
				return;
			}
		}
	}
	catch (...)
	{
		glog(Log::L_ERROR,SSSITE(" :PostEventSink()  Unexpect exception was threw out"));
	}
}
void StreamSmithSite::SetIdsServerAddr(std::string& strAddr)
{
	StreamSmithSite::_strIDsServerAddr=strAddr;
}
const char* StreamSmithSite::getAssetDictoryUrl()
{
	return StreamSmithSite::_strIDsServerAddr.c_str();
}
void StreamSmithSite::setDefaultSpigotsBoards(const std::vector<int>& bdIDs)
{
	StreamSmithSite::_defaultSpigotsBoardIDs=bdIDs;
}
IPlaylist* StreamSmithSite::openPlaylist(const ZQ::common::Guid& playlistGuid, const bool bCreateIfNotExist,const char* pClientSessionID)
{	

#ifndef _RTSP_PROXY
	char szBuf[128];
	memset(szBuf, 0, 128);
	playlistGuid.toString(szBuf,128);	
	if(m_pPlayListManager)
	{		
		IPlaylist* pTemp=(IPlaylist*)m_pPlayListManager->find(playlistGuid);
		if(NULL==pTemp)
		{
			if(bCreateIfNotExist)
			{
				glog(Log::L_DEBUG,SSSITE("openPlaylist() Can't find the playlist with Guid=%s"),szBuf);
				//find sipgotID from sourceManager first
				//std::vector<int> SpigotIDs;
				//m_pPlayListManager->GetSpigotIDsFromResource();
				for(int i=0;i<(int)StreamSmithSite::_defaultSpigotsBoardIDs.size();i++)
				{
					glog(Log::L_DEBUG,SSSITE("openPlaylist() create playlist using spigots board %d"),StreamSmithSite::_defaultSpigotsBoardIDs[i]);
				}
				Playlist* p=new Playlist(this,*m_pPlayListManager,playlistGuid,false,StreamSmithSite::_defaultSpigotsBoardIDs);
				glog(Log::L_DEBUG,SSSITE("Create a playlist %p successfully"),p);
				if(!p)
				{
					glog(Log::L_ERROR,SSSITE("openPlaylist()  Can't create a new playlistwith guid %s"),szBuf);
					return NULL;
				}
				else
				{
					if(!p->PlayListInitOK())
					{
						p->destroy();
						glog(Log::L_ERROR,SSSITE("openPlaylist()  Playlist initialize error and was deleted with guid %s"),szBuf);
						return NULL;
					}
					else
					{
						glog(Log::L_DEBUG,SSSITE("openPlaylist() Create playList OK with guid %s"),szBuf);
						return (IPlaylist*)p;
					}
				}
			}
			else
			{
				glog(Log::L_DEBUG,SSSITE(":openPlaylist()  Can't find the Playlist with Guid %s OK"),szBuf);
				return NULL;
			}
		}
		else
		{
			glog(Log::L_DEBUG,SSSITE("openPlaylist()  find the Playlist with Guid %s OK"),szBuf);
			return pTemp;
		}
	}
	else
#endif // ifdef _RTSP_PROXY
	{
		glog(Log::L_ERROR,"No PlayListManager ");
		return NULL;
	}

}


IPlaylist* StreamSmithSite::openPlaylist(const ZQ::common::Guid& playlistGuid,const bool bCreateIfNotExist,
										const int serviceGroupID,const int maxBitRate,
										const char* pCLientSessionID)
{
#ifndef _RTSP_PROXY
	char szGuid[128];
	memset(szGuid, 0, sizeof(szGuid));
	playlistGuid.toString(szGuid,sizeof(szGuid)-1);
	glog(Log::L_DEBUG,SSSITE("openPlaylist() enter with guid=%s svcGroupID=%d MaxBitRate=%d(bps)"),
									szGuid,serviceGroupID,maxBitRate);
	Playlist* pList=NULL;
	if ( (pList=m_pPlayListManager->find(playlistGuid)) == NULL)
	{
		glog(Log::L_DEBUG,SSSITE("openPlaylist()  can't find playlist with guid=%s"),szGuid);
		if(bCreateIfNotExist)
		{
			glog(Log::L_DEBUG,SSSITE("openPlaylist()  create a playlist with guid=%s"),szGuid);
			//query the spigots id from resource manager
			std::vector<int> vecSpigots;
			if(!m_pPlayListManager->GetSpigotIDsFromResource(serviceGroupID,maxBitRate/1000,vecSpigots))
			{
				glog(Log::L_ERROR,SSSITE("Can't get spigot id from resource,create playlist failed"));
				return NULL;
			}
			if(vecSpigots.size()<=0)
			{
				vecSpigots=StreamSmithSite::_defaultSpigotsBoardIDs;
			}
			for(int i=0;i<(int)vecSpigots.size();i++)
			{
				glog(Log::L_DEBUG,SSSITE("openPlaylist() create playlist using spigots board %d"),vecSpigots[i]);
				}
			Playlist* pList=new Playlist(this,*m_pPlayListManager,playlistGuid,false,vecSpigots);
			if(!pList)
			{
				glog(Log::L_CRIT,SSSITE("Can't create playlist due to some os error"));
				return NULL;
			}
			if(!pList->PlayListInitOK())
			{
				pList->destroy();
				glog(Log::L_ERROR,SSSITE("can't createplaylist successfully,maybe dvbc resource problem"));
				return NULL;
			}
			ZQ::common::Variant var;
			if(!pList->allocateResource(serviceGroupID,var,maxBitRate/1000))
			{
				pList->destroy();
				glog(Log::L_ERROR,SSSITE("can't alloc resource for serviceGroupID=%d and maxBitRate=%d(bps)"),
										serviceGroupID,maxBitRate);				
				return NULL;
			}
			else
			{
				glog(Log::L_DEBUG,SSSITE("create playlist ok with guid=%s"),szGuid);
				return (IPlaylist*)pList;
			}
		}
	}
	else
	{
		glog(Log::L_DEBUG,SSSITE("find a playlist with guid=%s"),szGuid);
		return (IPlaylist*)pList;
	}
#endif//_RTSP_PROXY
	return NULL;
}



SSMH_ContentHandle StreamSmithSite::findContentHandler(const char* path)
{
	if(! (path&&strlen(path)>0))
	{
		glog(ZQ::common::Log::L_DEBUG,"null path when findContentHandler,return with default ContentHandler");		
		return _defaultContentHandler;
	}

	
	IterContentHanldermap it=_mapContentHandle.find(path);
	if (it!=_mapContentHandle.end()) 
	{
		return it->second;
	}
	if (_defaultContentHandler!=NULL) 
	{
		glog(Log::L_DEBUG,SSSITE("No ContentHandler with path=%s was found,return with default ContentHandler"),path);
		return _defaultContentHandler;
	}
	else
	{
		glog(Log::L_DEBUG,SSSITE("No ContentHandler with path=%s was found,return NULL"),path);
		return NULL;
	}
}

StreamSmithSite* StreamSmithSite::findSite(const char* sitename)
{
	std::vector< StreamSmithSite*>::iterator	iter;
	for(iter=_stackStreamSmithSite.begin();iter!=_stackStreamSmithSite.end();iter++)
	{
		if(strcmp((*iter)->getSiteName(),sitename)==0)
		{
			return *iter;
		}
	}
	return NULL;
}
void StreamSmithSite::StartService(IClientRequest* pReq)
{
	if(!pReq)
	{
		glog(Log::L_ERROR,SSSITE("StreamSmithSite::StartService(IClientRequest* pReq)##NUll pReq pass in"));
		return;
	}	
}

// modified by Cary 2006-11-02
IClientSession* 
StreamSmithSite::createClientSession(const char* sessId,const char* uri,
									 const IClientSession::SessType sessType)
{
	IClientSession *pSession = NULL;
	int tryCount = 0;
	do {
// Han Guan modified
		glog(ZQ::common::Log::L_DEBUG,SSSITE("Create session with sessID(%s)(bNull=%d)"),sessId,sessId==NULL);
		pSession=m_pSessionMgr->createSession(sessId,sessType,uri);
// End modified
// 		pSession=m_pSessionMgr->createSession(sessType,uri);
		tryCount ++;
	} while(pSession == NULL && tryCount < 5);

	if(NULL == pSession)
	{
		glog(Log::L_ERROR, SSSITE("Create Client Session fail"));
		return NULL;
	}
	else
	{
		glog(Log::L_DEBUG, 
			SSSITE("Create Client Session = %p with Client sessionID = %s success"), 
			pSession, pSession->getSessionID());		
		return pSession;
	}
}

IClientSession* StreamSmithSite::findClientSession(const char* sessId, 												   
												   const IClientSession::SessType sessType)
{
	IClientSession* pSession = m_pSessionMgr->findSession(sessId);	
	if(NULL == pSession)
	{
		glog(Log::L_DEBUG ,SSSITE("Can't find clientSession with SessionID=%s"),sessId);
		return NULL;	
	}
	else
	{	
		glog(Log::L_DEBUG,SSSITE("return Session=%X with SessionID=%s"),pSession,sessId);
		return pSession;
	}
	return NULL;
}

bool	StreamSmithSite::destroyClientSession(const char* sessionID)
{
#ifndef TEST
	glog(Log::L_DEBUG,"Delete Session with session ID=%s",sessionID);
	return m_pSessionMgr->deleteSession(sessionID,false);
#else
	return true;
#endif
}

IServerRequest*	StreamSmithSite::newServerRequest(const char* sessionID,const std::string& ConnectionIDentity)
{
	return m_pSessionMgr->getServerRequest(sessionID,ConnectionIDentity);
}
void	StreamSmithSite::PostServerRequest(IServerRequest* pReq) 
{
	glog(ZQ::common::Log::L_DEBUG,"ClientSession %s is in FixupServerRequest phase",pReq->getClientSessionID());
	FixupServerRequestHandler*	pHandler = new FixupServerRequestHandler(_gThreadPool,_stackFixupServerRequest,pReq);
	if(pHandler)		
	{

		pHandler->start();
	}
	else
	{
		glog(ZQ::common::Log::L_DEBUG,"Can't Post FixupServerRequest because no ThreadRequest is made");
	}
}
void StreamSmithSite::DestroyStreamSmithSite()
{	
	//delete m_pEventFire;
	if(m_pEventFire)
		m_pEventFire->SetQuit();
	StreamSmithSite* pSite=NULL;
	while (_stackPluginModuleHandle.size()>0) 
	{
		DynSharedObj* share = _stackPluginModuleHandle.back();
		_stackPluginModuleHandle.pop_back();

		if(share->isLoaded())
		{
			SSM_ModuleUninit	unInit=NULL;
			unInit=(SSM_ModuleUninit)SYS::getProcAddr(share->getLib(),"ModuleUninit");
			try
			{
				
				if(unInit)
				{
					try
					{
						unInit(StreamSmithSite::getDefaultSite());
					}
					catch (...) 
					{
						glog(ZQ::common::Log::L_ERROR,SSSITE("DestroyStreamSmithSite() unexpect error when invoke unInit"));						
					}
				}
				int64 DwTest=ZQ::common::now();
                share->free();
				glog(Log::L_DEBUG,"Free library time count="FMT64, ZQ::common::now()-DwTest);				
			}
			catch(...)
			{
				glog(Log::L_ERROR,SSSITE("unexpect error when freelibrary"));
			}
		}
        delete share;
	}
	while (_stackStreamSmithSite.size()>0)
	{
		pSite=_stackStreamSmithSite.back();
		_stackStreamSmithSite.pop_back();
		if(pSite)
		{
			if(strcmp(pSite->getSiteName(),STR_SITE_DEFAULT)!=0)
			{
				delete pSite;
				pSite=NULL;
			}
		}
	}
//	if(_PluginLog)
//	{
//		delete _PluginLog;
//		_PluginLog=NULL;
//	}	
	_stackFixupRequest.clear();
	_stackFixupResponse.clear();
	_stackEventSink.clear();
}

bool StreamSmithSite::SetupStreamSmithSite(const char *pConfPath,std::vector<std::string>& path)
{
//	if(!pConfPath)
//	{
//		glog(Log::L_ERROR,SSSITE("SetupStreamSmithSite() null pConfPath pass in"));
//		return false;
//	}
//	CConfigParser parser(pConfPath);
//	if(!parser.ParseConfig(path))
//	{
//		glog(Log::L_ERROR,SSSITE("SetupStreamSmithSite()  can't parse the configuration file %s"),pConfPath);
//		return false;
//	}	


	std::vector<std::string> vecSiteName= GAPPLICATIONCONFIGURATION.reqesutHanlders.GetSiteName();


	
	
	int i=0;
	for(i=0;i<(int)vecSiteName.size();i++)
	{
		if(strcmp(vecSiteName[i].c_str(),STR_SITE_DEFAULT)==0)
		{
			_pDefaultSite->setSitename(STR_SITE_DEFAULT);
			glog(Log::L_DEBUG,SSSITE("SetupStreamSmithSite() create Default site"));
			_pDefaultSite->m_pRequestHandler = &GAPPLICATIONCONFIGURATION.reqesutHanlders;
			_stackStreamSmithSite.push_back(_pDefaultSite);
		}
		else
		{
			StreamSmithSite* pSite=new StreamSmithSite(vecSiteName[i].c_str());
			if(!pSite)
			{
				glog(Log::L_CRIT,SSSITE("SetupStreamSmithSite() create StreamSmithSite with sitename=%s fail"),vecSiteName[i].c_str());
				return false;
			}

			pSite->m_pRequestHandler = &GAPPLICATIONCONFIGURATION.reqesutHanlders;
			_stackStreamSmithSite.push_back(pSite);
		}
	}
	//load plugin
	glog.flush();
	std::vector<RtspRequestHandler::PluginAttr> vecPluginPathAndInfo;
	GAPPLICATIONCONFIGURATION.reqesutHanlders.GetPluginPathAndInfo(vecPluginPathAndInfo);	
//	vecPluginPath.push_back("lalala");
	SSHM_ModuleInit				ModuleInit=NULL;	
	SSHM_ModuleInitEx			ModuleInitEx=NULL;
	glog(Log::L_DEBUG,SSSITE("there are %d Plugin(s) need to be loaded"),vecPluginPathAndInfo.size());

	for(i=0;i<(int)vecPluginPathAndInfo.size();i++)
	{		
		glog(Log::L_DEBUG,SSSITE("Load plugin %s"),vecPluginPathAndInfo[i].filePath.c_str());

        DynSharedObj* share = new DynSharedObj(vecPluginPathAndInfo[i].filePath.c_str());
		if(!share->isLoaded())
		{
			glog(Log::L_ERROR,SSSITE("SetupStreamSmithSite()  can't load the plugin %s and Last error =%s"),
								vecPluginPathAndInfo[i].filePath.c_str(), share->getErrMsg());
			// continue;
			return false;
		}
        void* hdl = share->getLib();
		ModuleInit=  (SSHM_ModuleInit)  SYS::getProcAddr(hdl, "ModuleInit");
		ModuleInitEx=(SSHM_ModuleInitEx)SYS::getProcAddr(hdl, "ModuleInitEx");

		if(ModuleInitEx!=NULL)
		{
			glog(Log::L_DEBUG,SSSITE("SetupStreamSmithSite()  Find ModuleInitEx, call it"));
			for(int j=0;j<(int)_stackStreamSmithSite.size();j++)
			{
				try
				{
					ModuleInitEx( _stackStreamSmithSite[j], vecPluginPathAndInfo[i].configFile.c_str() );
				}
				catch (...) 
				{
					glog(ZQ::common::Log::L_ERROR,SSSITE("SetupStreamSmithSite() Unexpect error when invoke ModuleInitEx for module %s"),
								vecPluginPathAndInfo[i].filePath.c_str());
					try
					{
						SSM_ModuleUninit	unInit=NULL;
						unInit=(SSM_ModuleUninit)SYS::getProcAddr(hdl, "ModuleUninit");
						if(unInit)
						{
							unInit(_stackStreamSmithSite[j]);
						}
					}
					catch (...) 
					{
					}
					return false;
				}
			}
		}
		else
		{
			glog(Log::L_DEBUG,SSSITE("SetupStreamSmithSite()  Find ModuleInit, call it"));
			if(!ModuleInit)
			{
				glog(Log::L_ERROR,SSSITE("SetupStreamSmithSite()  can't find the initialize entry in the plugin"));
				continue;
			}
			for(int j=0;j<(int)_stackStreamSmithSite.size();j++)
			{
				try
				{
					ModuleInit(_stackStreamSmithSite[j]);
				}
				catch (...) 
				{
					glog(ZQ::common::Log::L_ERROR,SSSITE("SetupStreamSmithSite() Unexpect error when invoke ModuleInit for module %s"),
								vecPluginPathAndInfo[i].filePath.c_str());
						try
					{
						SSM_ModuleUninit	unInit=NULL;
						unInit=(SSM_ModuleUninit)SYS::getProcAddr(hdl,"ModuleUninit");
						if(unInit)
						{
							unInit(_stackStreamSmithSite[j]);
						}
					}
					catch (...) 
					{
					}
					return false;
				}
			}
		}
		_stackPluginModuleHandle.push_back(share);
	}	
	//register default handler
#ifdef _RTSP_PROXY
#ifndef _LOCAL_TEST
	if ( gRtspProxyConfig.reqesutHanlders.GetDefaultContenHandler().empty() || _defaultContentHandler==NULL ) 
	{
		glog(ZQ::common::Log::L_ERROR,"Default content Handler [%s] is specified, but no such handler, service quit",
			gRtspProxyConfig.reqesutHanlders.GetDefaultContenHandler().c_str());
		return false;
	}
#endif
#endif//_RTSP_PROXY

	StreamSmithSite::_pDefaultSite->RegisterFixupRequest((SSMH_FixupRequest)SSMH_DefaultFixupRequest);
//	StreamSmithSite::_pDefaultSite->RegisterFixupResponse((SSMH_FixupResponse)SSMH_DefaultFixupResponse);


#ifndef _RTSP_PROXY
	//Start playList Manager	
	m_pPlayListManager->start();
#endif	
	m_pEventFire=new StreamSmithSite::EventFire(_gThreadPool);//,eventSinkTimeout);
	m_pEventFire->start();
	return true;
}


// -----------------------------
// class AuthUserHandler
// -----------------------------
StreamSmithSite::AuthUserHandler::AuthUserHandler(ZQ::common::NativeThreadPool& pool, Stack& _sstack, IClientRequestWriter* pRequest)
: ThreadRequest(pool), _stack(_sstack), _pReq(pRequest)
{
	if( _pReq )
		_pReq->addRef();
}
StreamSmithSite::AuthUserHandler::~AuthUserHandler( )
{
	if(_pReq)
		_pReq->release();
}

bool StreamSmithSite::AuthUserHandler::init(void)
{
	//glog(Log::L_DEBUG,"bool StreamSmithSite::AuthUserHandler::init()##create site request %p",this);
	return (NULL != _pReq && NULL != _pReq->getSite());
}

int StreamSmithSite::AuthUserHandler::run(void)
{
	IStreamSmithSite* pSite = _pReq->getSite();
	bool continueInPhase = true;
	
	for (Stack::iterator it = _stack.begin(); NULL !=pSite && continueInPhase && it< _stack.end(); it++)
	{
		if (NULL == *it)
			continue;
		try
		{
			RequestProcessResult ret = (*it)(pSite, _pReq);
			switch (ret)
			{
			case RequestError: // error occured during processing the request
			case RequestDone:  // request has been done, no further process needed
// 				// clean up the request
// 				if(_pReq)
// 				{
// 					const char* pDisableAutoDelete = _pReq->getContext(CLIENT_REQUEST_DISABLE_AUTO_DELETE);
// 					if( pDisableAutoDelete && stricmp(CLIENT_REQUEST_DISABLE_AUTO_DELETE,"1"))
// 					{
// 						//do nothing
// 					}
// 					else
// 					{
// 						_pReq->release();
// 					}
// 				}
				return 0;
				
			case RequestUnrecognized:
			case RequestProcessed:
				break; // continue for the next handler in this phase
				
			case RequestPhaseDone:  // request has been done for this phase, ready to be processed in next phase
				continueInPhase = false;
				break;
			}			
		}
		catch (...)  
		{
			glog(Log::L_ERROR,SSSITE("AuthUserHandler::run() unexpect exception was threw out"));
		}
	}
	
	
	if (!pSite->postRequest(_pReq, IClientRequest::FixupRequest))
	{
		// error failed to forward the requet to FixupRequest phase
	}
	return 0;
}
void StreamSmithSite::AuthUserHandler::final(int retcode /* =0 */, bool bCancelled /* =false */)
{
	updateLastIdleStamp();
	//glog(Log::L_DEBUG,"void StreamSmithSite::AuthUserHandler::final()##Delete site request %p",this);
	delete this;
	return;
}

//class FixupServerRequestHandler : public ZQ::common::ThreadRequest
//{
//	friend class StreamSmithSite;
//protected:
//	typedef std::vector < SSMH_FixupServerRequest > Stack;
//	virtual ~FixupServerRequestHandler();
//	Stack&				_stack;
//	IServerRequest*		_pReq;
//	
//	bool	init();
//	int		run();
//	void	final(int retcode =0, bool bCancelled =false);	
//};

StreamSmithSite::FixupServerRequestHandler::FixupServerRequestHandler(ZQ::common::NativeThreadPool& pool,Stack& s,IServerRequest* pReq)
:ZQ::common::ThreadRequest(pool),_stack(s)
{
	_pReq=pReq;
}

StreamSmithSite::FixupServerRequestHandler::~FixupServerRequestHandler( )
{
}

bool StreamSmithSite::FixupServerRequestHandler::init()
{
	return (NULL != _pReq );
}
int StreamSmithSite::FixupServerRequestHandler::run()
{
	IStreamSmithSite* pSite =StreamSmithSite::getDefaultSite();
	bool continueInPhase = true;
	for(Stack::iterator it=_stack.begin() ;  NULL !=pSite && continueInPhase && it!=_stack.end()  ;   it++   )
	{
		if(NULL==*it)
			continue;
		try
		{
// 			const char* pSessId = _pReq->getClientSessionID();
// 			char szSeq[256];
// 			memset(szSeq,0,sizeof(szSeq));
// 			uint16 seqBufLen = sizeof(szSeq);
// 			const char* pSeq = _pReq->getHeader("CSeq",szSeq,&seqBufLen);			
// 
// 			glog(ZQ::common::Log::L_DEBUG,"Enter FixupServerRequestHandler with [%s][%s] ",pSessId,pSeq );
			RequestProcessResult ret = (*it)(pSite, _pReq);
//			glog(ZQ::common::Log::L_DEBUG,"Leave FixupServerRequestHandler with [%s][%s] ",pSessId,pSeq );
			switch (ret)
			{
			case RequestError: // error occured during processing the request
			case RequestDone:  // request has been done, no further process needed
				// clean up the request
				continueInPhase = false;
				if(_pReq)
					_pReq->release();
				return 0;
				
			case RequestUnrecognized:
			case RequestProcessed:
				break; // continue for the next handler in this phase
				
			case RequestPhaseDone:  // request has been done for this phase, ready to be processed in next phase
				continueInPhase = false;
				break;
			}
		}
		catch (...)
		{
			glog(Log::L_ERROR, SSSITE("ThreadID[%d] FixupServerRequestHandler::run() unexpect exception was threw out"), 
            #ifdef ZQ_OS_MSWIN 
            GetCurrentThreadId());
            #else
            pthread_self());
            #endif
			return 0;
		}
		return 1;
	}
	//如果没有任何的routinue注册，就会直接release掉这个资源然后返回
	try
	{
		_pReq->release();
	}
	catch (...)
	{
	}
	return 1;
}
void StreamSmithSite::FixupServerRequestHandler::final(int retcode /* =0 */, bool bCancelled /* =false */)
{
	updateLastIdleStamp();
// 	if( _pReq )
// 		_pReq->release();
	delete this;
}

StreamSmithSite::FixupRequestHandler::FixupRequestHandler(ZQ::common::NativeThreadPool& pool, Stack& _sstack, IClientRequestWriter* pRequest, int priority)
: ThreadRequest(pool), _stack(_sstack), _pReq(pRequest)
{
	if( _pReq )
		_pReq->addRef( );
	if( priority >= 0 )
		setPriority(priority);
	else
		setPriority(70);
}
StreamSmithSite::FixupRequestHandler::~FixupRequestHandler( )
{
	if( _pReq )
		_pReq->release();
}

bool StreamSmithSite::FixupRequestHandler::init(void)
{
	//glog(Log::L_DEBUG,"bool StreamSmithSite::FixupRequestHandler::init()##create site request %p",this);
	return (NULL != _pReq && NULL != _pReq->getSite());
}

int StreamSmithSite::FixupRequestHandler::run(void)
{
	IStreamSmithSite* pSite = _pReq->getSite();
	// parse the host from uri, then assign the site for the request

	bool continueInPhase = true;
	for (Stack::iterator it = _stack.begin(); NULL !=pSite && continueInPhase && it< _stack.end(); it++)
	{
		if (NULL == *it)
			continue;
		try
		{
			const char* pSessId = dynamic_cast<IClientRequest*>(_pReq)->getClientSessionId();
			char szSeq[256];
			memset(szSeq,0,sizeof(szSeq));
			uint16 seqBufLen = sizeof(szSeq);
			const char* pSeq = _pReq->getHeader("CSeq", szSeq, &seqBufLen);


			glog(ZQ::common::Log::L_DEBUG,"Enter FixupRequestHandler with [%s][%s] ",pSessId,pSeq );
			RequestProcessResult ret = (*it)(pSite, _pReq);
			
			pSessId = dynamic_cast<IClientRequest*>(_pReq)->getClientSessionId();
			glog(ZQ::common::Log::L_DEBUG,"Leave FixupRequestHandler with [%s][%s] ",pSessId,pSeq );
			
			switch (ret)
			{
			case RequestError: // error occured during processing the request
			case RequestDone:  // request has been done, no further process needed
				// clean up the request
				continueInPhase = false;
				return 0;
				
			case RequestUnrecognized:
			case RequestProcessed:
				break; // continue for the next handler in this phase
				
			case RequestPhaseDone:  // request has been done for this phase, ready to be processed in next phase
				continueInPhase = false;
				break;
			}
		}
		catch (...)  
		{
			glog(Log::L_ERROR,SSSITE("ThreadID[%d] FixupRequestHandler::run() unexpect exception was threw out"), 
            #ifdef ZQ_OS_MSWIN 
            GetCurrentThreadId());
            #else
            pthread_self());
            #endif
			return 0;
		}
	}	
	
	if (!pSite->postRequest(_pReq, IClientRequest::ContentHandle))
	{
		// error failed to forward the request to ContentHandle phase
	}
	return 0;
}
void StreamSmithSite::FixupRequestHandler::final(int retcode /* =0 */, bool bCancelled /* =false */)
{
	updateLastIdleStamp();
	//glog(Log::L_DEBUG,"void StreamSmithSite::FixupRequestHandler::final()##delete site request %p",this);
	delete this;
	return;		 
}
// -----------------------------
// class ContentHandler
// -----------------------------
StreamSmithSite::ContentHandler::ContentHandler(ZQ::common::NativeThreadPool& pool, 
												SSMH_ContentHandle funcHandle, IClientRequestWriter* pRequest, int priority)
: ThreadRequest(pool), _func(funcHandle), _pReq(pRequest)
{
	if( _pReq )
		_pReq->addRef();
	if( priority >= 0 )
		setPriority(priority);
	else
		setPriority(50);
}
StreamSmithSite::ContentHandler::~ContentHandler( )
{
	if( _pReq )
		_pReq->release();
}

bool StreamSmithSite::ContentHandler::init(void)
{
	//glog(Log::L_DEBUG,"bool StreamSmithSite::ContentHandler::init()##Create site request %p",this);


	return (NULL != _pReq && NULL != _pReq->getSite());
}

int StreamSmithSite::ContentHandler::run(void)
{
	IStreamSmithSite* pSite = _pReq->getSite();
	bool continueInPhase = true;	
	if (NULL != _func)
	{
		RequestProcessResult ret;
		try
		{
			const char* pSessId = dynamic_cast<IClientRequest*>(_pReq)->getClientSessionId();
			char szSeq[256];
			memset(szSeq,0,sizeof(szSeq));
			uint16 seqBufLen = sizeof(szSeq);
			const char* pSeq = _pReq->getHeader("CSeq",szSeq,&seqBufLen);

			glog(ZQ::common::Log::L_DEBUG,"Enter ContentHandler with [%s][%s] ",pSessId,pSeq );
			 ret= _func(pSite, _pReq);
			glog(ZQ::common::Log::L_DEBUG,"leave ContentHandler with [%s][%s] ",pSessId,pSeq );
			 if(ret!=RequestError)
			 {
				 StreamSmithSite::m_pSessionMgr->onAccessingSession(_pReq);
			 }
			 if( _pReq->getVerb() == RTSP_MTHD_SETUP && ret != RequestError ) {
				 char szUrl[256];
				 memset(szUrl,0,sizeof(szUrl));
				 uint16 urlLen = sizeof(szUrl) - 1;
				 _pReq->getUri(szUrl, urlLen);
				 const char* pSessId = _pReq->getClientSessionId();

				 IClientSession* pClientSession = pSite->findClientSession(pSessId);
				 if(pClientSession) {
					 pClientSession->setProp(url_in_setup, std::string(szUrl));
					 pClientSession->release();
					 glog(ZQ::common::Log::L_DEBUG, "set url [%s] in session[%s]'s context", szUrl, pSessId );
				 }
			 }
			switch (ret)
			{
			case RequestError: // error occured during processing the request
			case RequestDone:  // request has been done, no further process needed

				return 0;
				
			case RequestUnrecognized:
			case RequestProcessed:
				break; // continue for the next handler in this phase
				
			case RequestPhaseDone:  // request has been done for this phase, ready to be processed in next phase
				continueInPhase = false;
				break;
			}
			//Add uri into session for future use!
		
		}
		catch (...)  
		{
			glog(Log::L_ERROR,SSSITE("ThreadID[%d] ContentHandler::run() Unexpect exception was threw out"), 
            #ifdef ZQ_OS_MSWIN 
            GetCurrentThreadId());
            #else
            pthread_self());
            #endif
			return 0;
		}
	}
	
	if (!pSite->postRequest(_pReq, IClientRequest::FixupResponse))
	{
		// error failed to forward the requet to ContentHandle phase
		glog(Log::L_ERROR,"StreamSmithSite::ContentHandler::run(void)## Post request to phase FixupResponse fail");
	}	
	
	return 0;
}
void StreamSmithSite::ContentHandler::FillURIIntoClientSession()
{
	if(_pReq==NULL)
	{
		glog(Log::L_ERROR,SSSITE("Oh!My god,client request pointer is NULL,what should I do?"));
		return;
	}
	IStreamSmithSite* pSite=_pReq->getSite();
	IClientRequest* pReq=_pReq;
	//at first get the client session ID
	const	char* pSessID=pReq->getClientSessionId();
	if( ! (pSessID&&strlen(pSessID)>0) )
	{
		glog(Log::L_ERROR,SSSITE("no session id is found in clientrequest instance"));
		return ;
	}
	auto_release<IClientSession*> pSession=pSite->findClientSession(pSessID);
	if(! pSession )
	{
		glog(Log::L_ERROR,SSSITE("can't get the client session through sessID=%s"),pSessID);
		return;
	}
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	pReq->getUri(szBuf,sizeof(szBuf)-1);
	ZQ::common::Variant var=std::string(szBuf);
	if(!pSession->set(STR_CLIENT_RTSP_URI,var,true))
	{
		glog(Log::L_ERROR,SSSITE("something wrong when push client uri into cleint session"));
		return;
	}
	
}
void StreamSmithSite::ContentHandler::final(int retcode /* =0 */, bool bCancelled /* =false */)
{
	updateLastIdleStamp();
	//glog(Log::L_DEBUG,"void StreamSmithSite::ContentHandler::final()##delete site request %p",this);
	delete this;
	return;
}
// -----------------------------
// class FixupResponseHandler
// -----------------------------
StreamSmithSite::FixupResponseHandler::FixupResponseHandler(ZQ::common::NativeThreadPool& pool, Stack& _sstack, IClientRequestWriter* pRequest,int priority)
: ThreadRequest(pool), _stack(_sstack), _pReq(pRequest)
{
	if( _pReq )
	{
		_pReq->addRef();
	}
	if( priority >= 0 )
		setPriority(priority);
	else
		setPriority(30);
}
StreamSmithSite::FixupResponseHandler::~FixupResponseHandler( )
{
	if( _pReq )
	{
		_pReq->release();
	}
}
bool StreamSmithSite::FixupResponseHandler::init(void)
{
	
	glog(Log::L_DEBUG,"StreamSmithSite::FixupResponseHandler::init(void)##create site request %p",this);
	return (NULL != _pReq && NULL != _pReq->getSite());
}

int StreamSmithSite::FixupResponseHandler::run(void)
{
	IStreamSmithSite* pSite = _pReq->getSite();
	bool continueInPhase = true;
	for (Stack::iterator it = _stack.begin(); NULL !=pSite && continueInPhase && it< _stack.end(); it++)
	{
		if (NULL == *it)
			continue;
		try
		{
			const char* pSessId = dynamic_cast<IClientRequest*>(_pReq)->getClientSessionId();
			char szSeq[256];
			memset(szSeq,0,sizeof(szSeq));
			uint16 seqBufLen = sizeof(szSeq);
			const char* pSeq = _pReq->getHeader("CSeq",szSeq,&seqBufLen);

			glog(ZQ::common::Log::L_DEBUG,"Enter FixupResponseHandler [%s][%s] ",pSessId,pSeq );
			RequestProcessResult ret = (*it)(pSite, _pReq);
			glog(ZQ::common::Log::L_DEBUG,"leave FixupResponseHandler [%s][%s] ",pSessId,pSeq );
			switch (ret)
			{
			case RequestError: // error occurred during processing the request
			case RequestDone:  // request has been done, no further process needed
				// clean up the request
// 				if(_pReq)
// 				{
// 					const char* pDisableAutoDelete = _pReq->getContext(CLIENT_REQUEST_DISABLE_AUTO_DELETE);
// 					if( pDisableAutoDelete && stricmp(CLIENT_REQUEST_DISABLE_AUTO_DELETE,"1"))
// 					{
// 						//do nothing
// 					}
// 					else
// 					{
// 						_pReq->release();
// 					}
// 				}
				return 0;
				
			case RequestUnrecognized:
			case RequestProcessed:
				break; // continue for the next handler in this phase
				
			case RequestPhaseDone:  // request has been done for this phase, ready to be processed in next phase
				continueInPhase = false;
				break;
			}
		}
		catch (...)  
		{
			glog(Log::L_ERROR,SSSITE("FixupResponseHandler::run()  Unexpect exception was threw out"));
			return 0;
		}
	}
	
	if (!pSite->postRequest(_pReq, IClientRequest::PostResponse))
	{
		// error failed to forward the request to ContentHandle phase
		glog(Log::L_ERROR,SSSITE("FixupResponseHandler::run() post request to phase PostResponse fail"));
	}
	return 0;
}
void StreamSmithSite::FixupResponseHandler::final(int retcode /* =0 */, bool bCancelled /* =false */)
{
	updateLastIdleStamp();
	//glog(Log::L_DEBUG,"void StreamSmithSite::FixupResponseHandler::final()##Delete site request %p",this);
	delete this;
	return;
}

ZQ::common::Variant StreamSmithSite::getInfo( int32 infoType ) 
{
	ZQ::common::Variant var;
	switch( infoType ) 
	{
	case INFORMATION_LSCP_PORT:
		var = GAPPLICATIONCONFIGURATION.lLscpPort;
		break;
	case INFORMATION_RTSP_PORT:
		var = GAPPLICATIONCONFIGURATION.lRtspPort;
		break;
	case INFORMATION_SNMP_PROCESS:
		{
			long instanceId = Application->getInstanceId();
			glog(ZQ::common::Log::L_DEBUG,"StreamSmithSite::getInfo() infoType[%d], instanceId[%d]", infoType, instanceId);
			ZQ::common::Variant vInStanceId = (long)instanceId;
			var["sys.snmp.instanceId"] = vInStanceId;
		}
		break;
	default:
		break;
	}
	return var;
}

#ifdef TEST
#include "dllmanager/ClientClass.h"

#include <iostream>
using namespace ZQ::common;
using namespace std;

int main()
{
	ZQ::common::ScLog log("log.txt",ZQ::common::Log::L_DEBUG,1024*1024*1);

	ZQ::common::setGlogger(&log);
	StreamSmithSite	defaultSite;
	VstrmClass		strClass(_gThreadPool);
	PlaylistManager	listMan(strClass);


	StreamSmithSite::_pDefaultSite=&defaultSite;
	StreamSmithSite::m_pvstrmClass=&strClass;
	StreamSmithSite::m_pPlayListManager=&listMan;
	StreamSmithSite::_defaultContentHandler=SSMH_DefaultContentHandle;


	//StreamSmithSite::SetupStreamSmithSite("D:\\work\\project\\ZQProjs\\StreamNow\\StreamSmith\\DLLManager\\configuration.xml");
	if(!StreamSmithSite::SetupStreamSmithSite("configuration.xml"))
	{
		glog(Log::L_ERROR,"SetUp Stream Smith Site fail");
	}

	StreamSmithSite* pSite=NULL;
	
	

	pSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	
	ClientRequestWriter* pWriter=new ClientRequestWriter;
	pWriter->setSite(pSite);
	pWriter->SetVerb(RTSP_MTHD_SETUP);
	pWriter->setArgument(RTSP_MTHD_NULL,"/?\\vod\\0038055f","1.0");
	
	pSite->postRequest(pWriter,IClientRequest::_ProcessPhase::FixupRequest);
	Sleep(3000);
	glog(Log::L_DEBUG,"Play request Start");
	pWriter->SetVerb(RTSP_MTHD_PLAY);
	pSite->postRequest(pWriter,IClientRequest::_ProcessPhase::FixupRequest);
	
	glog(Log::L_DEBUG,"over before");

	//Sleep(10*1000);
	Sleep(-1);
	glog(Log::L_DEBUG,"over");
	
	
	delete pWriter;
	StreamSmithSite::DestroyStreamSmithSite();	
	
	ZQ::common::setGlogger(NULL);
	::SleepEx(1, false);

	return 1;
}
#endif//TEST

/* expected test main()
main()
{
	// step 0, create a default site instance
	// step 1. load a dll with filename start with "ssm_", maybe based on cofiguation <Module file="ssm_mod.dll" />
	//      1.1 list all the names of exported content handlers based on the configuration
	//      1.2 insert its PostRequestRead, FixupRequest, AuthUser and FixupResponse entires into the stack of the default site
	// step 2. repeat step 1 to build up an aggregate of the exported content handler names
	// step 3. parse a virtual site configuration
	//            <VirtualSite name = "mysite" >
	//               <Application path= "MovieOnDemand" handler="MovieOnDemand" />
	//               <Application path= "ScheduledTV" handler="STV" />
	//            </VirtualSite>
	//      3.1 create a virtual site instance if the name does not pre-exists
	//      3.2 find the SSMH_ContentHandle from the name map built in 2 and 1.1
	//      3.3 create a map row <path, SSMH_ContentHandle> within the virtual site
	//      3.4 repeat 3.1-3.3 for each application in the virtual site
	// step 4 repeat 3 for each <VirtualSite>
	// step 6 repeat 3.1-3.3 for the <Application> out-of-range of any virtual site as the applications available for the default site
	// step 5 clean up the temp data structure created in step 1 and 2

	// the final stacks and request process routine will be
	// 1. stacks
	//    1.1 global stacks: DefaultSite::_stackPostRequestRead, DefaultSite::_stackAuthUser, DefaultSite::_stackFixupRequest
	//                       DefaultSite::_stackFixupResponse, DefaultSite::_stackPostResponse
	//    1.2 per-site hander map: StreamSmithSite::_mapContentHandler<path, SSMH_ContentHandle>
	// 2. process routine:
	// DefaultSite::_stackPostRequestRead [=> DefaultSite::_stackAuthUser) => DefaultSite::_stackFixupRequest]
	// /=> virtualsite = IStreamSite::findSite(Urlstr(IClientRequest::getUri())::gethost) => virtualsite::findContentHandler(Urlstr(IClientRequest::getUri())::getPath)(...)
	// => DefaultSite::_stackFixupResponse => DefaultSite::_stackPostResponse

    // clean up the sites when abort
}
*/
}
}

