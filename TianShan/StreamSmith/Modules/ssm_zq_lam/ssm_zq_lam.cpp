// SSM_Dummy.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#pragma warning(disable:4786)

#include <StreamSmithModule.h>
#include "URIParser.h"
#include <descCode.h>
#include <vector>
#include <math.h>
/*#include <AssetDict_def.h>*/
#include <Log.h>
#include <ScLog.h>

#ifdef _TEST_PLAY_WITH_MAC_
	#include "InitInfo.h"
#endif//_TEST_PLAY_WITH_MAC_

#include "InitInfo.h"

#include <ice\ice.h>
#include <IceUtil\iceutil.h>

#include "LAMFacade.h"


//#define _LOCAL_TEST

//#pragma message(__MSGLOC__"%%%%%%LOCAL TEST%%%%%%%%%%%%")



#define MY_BUFFER_SIZE	1024

using namespace ZQ::StreamSmith;
using namespace com::izq::am::facade::servicesForIce;

#ifdef HANDLER_TEST
	RequestProcessResult ContentHandleSetup (IStreamSmithSite* pSite, IClientRequest* pReq);
	RequestProcessResult ContentHandlePlay(IStreamSmithSite* pSite, IClientRequest* pReq);
	RequestProcessResult ContentHandleTearDown (IStreamSmithSite* pSite, IClientRequest* pReq);
#endif//HANDLER_TEST


#define		SESSID(x)	"<<SSM_ZQ_LAM>>SESSID(%s)\t\t"#x,pSessionID

#define		SSREQ(x)	"<<SSM_ZQ_LAM>>SESSID(%s)REQ(%p) \t\t"#x,pSessionID,pReq

#define		REQ(x)		"<<SSM_ZQ_LAM>>REQ(%p)\t\t"#x,pReq

//#define PLUGIN_PATH "SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\RtspProxy"
//
//typedef ::std::vector<::std::string>			CONFIG_ARRAY;
//typedef ::std::map<::std::string,::std::string> CONFIG_MAP;
//CONFIG_ARRAY									configArray;
//CONFIG_MAP										configMap;
//::std::string									configFilePath;
//bool											bConnICEOK = false;
//bool											iceInitialized = false;



Ice::CommunicatorPtr	_iceCommunicator=NULL;
Ice::ObjectAdapterPtr	_AssetAdpterPtr=NULL;
LAMFacadePrx			_LAMFacadeprx=NULL; 


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

using namespace ZQ::common;

//using namespace ZQ::AssetDict;
//for asset
//Dictionary		gAssetDict;

HMODULE			g_CurrentModuleHandle=0;

#ifdef _TEST_PLAY_WITH_MAC_
	std::string		gstrItemName;
	int				gRepeatCount=0;
	bool			gbFindIDS=false;
#endif

	

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	g_CurrentModuleHandle=(HMODULE)hModule;
    return TRUE; 
}
//const URI key 
//such as rtsp://10.11.0.250/SChange?Item0=0000&Item1=1111
#define	STR_URI_KEY_PRE  "Item"

const char GLOBAL_GUID[]="Guid"; 

#define	RTSP_MSG_MAX_RATE_HEADER	"MuxRate_LAM"
//StreamSmithUtility	*pUtility=NULL;

RequestProcessResult SSMH_DefaultContentHandleSetup (IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	/// new added line
	::std::vector<::std::string>	strItems;
	DWORD	dwTestTime=GetTickCount();
	
	pSite->CoreLog(Log::L_DEBUG,REQ("Enter SETUP"));
	IServerResponse* pResponse=pReq->getResponse();
//#ifdef _LOCAL_TEST
//	pResponse->printf_preheader(RESPONSE_OK);
//	pResponse->setHeader(HEADER_TRANSPORT,"MP2T/DVBC/QAM;unicast");	
//	char szBuf111[1204];
//	sprintf(szBuf111,"program-number=%d;frequency=%d;qam-mode=%d",10,55667788,256);
//	pResponse->setHeader(HEADER_SC_TRANSPORT,szBuf111);
//	pResponse->setHeader(HEADER_SESSION,"123456789");
//	pResponse->post();
//	return RequestDone;
//#endif
	if(!pResponse)
	{
		pSite->CoreLog(Log::L_ERROR,REQ("Can't get IServerResponse through IClientRequest"));
		return RequestError;
	}
	pSite->CoreLog(Log::L_DEBUG,REQ("After get ServerResponse %p"),pResponse);

	char				szBuf[1024];
	unsigned __int16	iStrLen=sizeof(szBuf);
	ZeroMemory(szBuf,iStrLen);

	pReq->getUri(szBuf,iStrLen);
	pSite->CoreLog(Log::L_DEBUG,REQ("get URI is % s"),szBuf);
	URIParser	parser(szBuf);

	
	{//Check URI
		if(strlen(parser.GetContent())<=0)
		{//Invalid content
			pSite->CoreLog(Log::L_ERROR,REQ("No content value in the uri string=%s"),szBuf);
			pResponse->printf_preheader(RESPONSE_BAD_REQUEST);
			pResponse->post();
			return RequestError;
		}
	}	
	ZQ::common::Guid	uid;
	uid.create();	
	
	//////////////////////////////////////////////////////////////////////////
	int muxBitrate=4000000;
	if(pReq->getHeader(RTSP_MSG_MAX_RATE_HEADER,szBuf,&iStrLen)==NULL)
	{
		pSite->CoreLog(Log::L_DEBUG,REQ("can't get RTSP_MSG_MAX_RATE_HEADER in rtsp message,set it to 4000(kbps)"));
	}
	else
	{
		muxBitrate=atoi(szBuf);
		pSite->CoreLog(Log::L_DEBUG,REQ("get RTSP_MSG_MAX_RATE_HEADER from rtsp message and result is %d(kbps)"),
			muxBitrate/1000);
	}

	bool	bQam=false;	
	int		NodeGroupID=0;
	bool	bHasNodeGroupID=true;
	iStrLen=sizeof(szBuf)-1;
	if(pReq->getHeader(HEADER_TRANSPORT,szBuf,&iStrLen)==NULL)
	{
		pSite->CoreLog(Log::L_ERROR,REQ("Can't get Transport information from client request"));
		pResponse->printf_preheader(RESPONSE_BAD_REQUEST);
		pResponse->post();
		return RequestError;
	}
	if(strstr(szBuf,"MP2T")!=NULL && strstr(szBuf,"DVB")!=NULL && strstr(szBuf,"QAM")!=NULL )
	{//QAM
		pSite->CoreLog(Log::L_DEBUG,REQ("the request is DVBC mode"));
		bQam=true;
		iStrLen=sizeof(szBuf)-1;
		//if(pReq->getHeader("SeaChange-Version",szBuf,&iStrLen)!=NULL)
		if(pReq->getHeader(HEADER_SC_SERVERDATA,szBuf,&iStrLen)!=NULL)
		{
			std::string	strTemp=szBuf;
			int			iPos=0;
			int			iTokenPos=0;
			if((iPos=strTemp.find("node-group-id="))!=std::string::npos)
			{
				if((iTokenPos=strTemp.find(";",iPos+1))!=std::string::npos)
				{
					strTemp=strTemp.substr(iPos,iTokenPos-iPos);
				}
				else
				{
					strTemp=strTemp.substr(iPos);
				}
				//int	GroupID=atoi(strTemp.c_str());
				sscanf(strTemp.c_str(),"node-group-id=%d",&NodeGroupID);
				pSite->CoreLog(Log::L_DEBUG,REQ("Get service group id %d "),NodeGroupID);		
				bHasNodeGroupID=true;
			}
			else
			{//NO service group ID	
				pSite->CoreLog(Log::L_DEBUG,REQ("No nodeGroup is found"));
				bHasNodeGroupID=false;
			}
		}
		else
		{
			pSite->CoreLog(Log::L_ERROR,REQ("Can't get server data from client request"));
			pResponse->printf_preheader(RESPONSE_BAD_REQUEST);
			pResponse->post();
			return RequestError;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IPlaylist* pList=NULL;
	if(bQam && bHasNodeGroupID)
	{
		pSite->CoreLog(Log::L_INFO,REQ("create playlist with NodeGroupID %d"),NodeGroupID);
		pList=pSite->openPlaylist(uid,true,NodeGroupID,muxBitrate);
	}
	else
	{		
		pList=pSite->openPlaylist(uid,true);
	}

	
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
	ZeroMemory(&pi,sizeof(IPlaylist::Item));
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
		const char * pElement=parser.GetValue(szItemNameBuf);
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
		pList->destroy();
		pSite->CoreLog(Log::L_ERROR,REQ("No item push back into playlist"));
		pResponse->printf_preheader(RESPONSE_BAD_REQUEST); 
		pResponse->post();	
		return RequestError;	
	}
	
	ZeroMemory(szBuf,sizeof(szBuf));
	iStrLen=strlen(szBuf);
	//check the bitrate

	pList->setMuxRate(muxBitrate,muxBitrate,0);

	//Check client request type Qam or IP
	iStrLen=sizeof(szBuf)-1;
	if(pReq->getHeader(HEADER_TRANSPORT,szBuf,&iStrLen)==NULL)
	{
		pList->destroy();
		pSite->CoreLog(Log::L_ERROR,REQ("Can't get Transport information from client request"));
		pResponse->printf_preheader(RESPONSE_BAD_REQUEST);
		pResponse->post();
		return RequestError;
	}
	
	//Get target IP and port
	tstring strDestination;
	tstring	strDestMac;
	int		destPort;
	char	*pDestinationIP=NULL;

	int		ProNum;
	int		Frequency;
	int		Qammode;
	if(strstr(szBuf,"MP2T")!=NULL && strstr(szBuf,"DVB")!=NULL && strstr(szBuf,"QAM")!=NULL )
	{//QAM
		pSite->CoreLog(Log::L_DEBUG,REQ("the request is DVBC mode"));
		bQam=true;
		iStrLen=sizeof(szBuf)-1;
		//if(pReq->getHeader("SeaChange-Version",szBuf,&iStrLen)!=NULL)
		if(pReq->getHeader(HEADER_SC_SERVERDATA,szBuf,&iStrLen)!=NULL)
		{
			std::string	strTemp=szBuf;
			int			iPos=0;
			int			iTokenPos=0;
			if((iPos=strTemp.find("node-group-id="))!=std::string::npos)
			{
				if((iTokenPos=strTemp.find(";",iPos+1))!=std::string::npos)
				{
					strTemp=strTemp.substr(iPos,iTokenPos-iPos);
				}
				else
				{
					strTemp=strTemp.substr(iPos);
				}
				//int	GroupID=atoi(strTemp.c_str());
				int		GroupID;
				sscanf(strTemp.c_str(),"node-group-id=%d",&GroupID);
				pSite->CoreLog(Log::L_DEBUG,REQ("Get service group id %d and need BandWidth=%d(kbps)"),
										GroupID,muxBitrate/1000);
				ZQ::common::Variant	varOut;
				if(!pList->allocateResource(GroupID,varOut,muxBitrate/1000))
				{
					pList->destroy();
					pSite->CoreLog(Log::L_ERROR,REQ("Can't get resource for service group ID=%d and need bandwidth=%d(kbps)"),
														GroupID,muxBitrate/1000);
					pResponse->printf_preheader(RESPONSE_NOT_ENOUGH_BW);
					pResponse->post();
					return RequestError;
				}
				Qammode=(int&)varOut[IPlaylist::RES_QAMMODE];
				ProNum=(int&)varOut[IPlaylist::RES_PROGRAMNUMBER];
				Frequency=(int&)varOut[IPlaylist::RES_FRENQENCY];
				strDestination=(tstring&)varOut[IPlaylist::RES_DESTIP];
				destPort=(int&)varOut[IPlaylist::RES_DESTPORT];
				strDestMac=(tstring&)varOut[IPlaylist::RES_DESTMAC];
				pDestinationIP=(char*)strDestination.c_str();
			}
			else
			{//NO service group ID
				pList->destroy();
				pSite->CoreLog(Log::L_ERROR,REQ("Can't get service group id from client request"));
				pResponse->printf_preheader(RESPONSE_BAD_REQUEST);
				pResponse->post();
				return RequestError;
			}
		}
		else
		{
			pList->destroy();
			pSite->CoreLog(Log::L_ERROR,REQ("Can't get server data from client request"));
			pResponse->printf_preheader(RESPONSE_BAD_REQUEST);
			pResponse->post();
			return RequestError;
		}
	}
	else
	{//IP
		pSite->CoreLog(Log::L_DEBUG,REQ("he request is IP mode"));
		iStrLen=sizeof(szBuf);
		ZeroMemory(szBuf,iStrLen);
		
		pReq->getTransportParam(KEY_TRANS_DEST,szBuf,&iStrLen);
		
		strDestination=szBuf;
		pSite->CoreLog(Log::L_DEBUG,REQ("Get destination =%s in Transport"),szBuf);
		
		iStrLen=sizeof(szBuf);
		
		pReq->getTransportParam(KEY_TRANS_CPORTA,szBuf,&iStrLen);
		
		destPort=atoi(szBuf);	
		pSite->CoreLog(Log::L_DEBUG,REQ("Get Port=%d in transport"),destPort);
		
		
		
		char	szDestinationIPBuffer[32];
		ZeroMemory(szDestinationIPBuffer,32);
		
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
			pSite->CoreLog(Log::L_ERROR,REQ("exception is throw out with error description is %s"),e.what());
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
	}
		

	const char* pSessionID=pReq->getClientSessionId();
	pSite->CoreLog(Log::L_DEBUG,REQ("Get client Session ID=%s"),pSessionID);

	//get uri
	pReq->getUri(szBuf,sizeof(szBuf)-1);
	IClientSession* pNewSession=pSite->createClientSession(NULL,szBuf);
	
	if(pNewSession==NULL)
	{
		pSite->CoreLog(Log::L_ERROR,REQ(" create ClientSession fail"));
		pList->destroy();		
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR); 
		pResponse->post();		
		return RequestError;
	}
	pSite->CoreLog(Log::L_DEBUG,REQ("Open client Session=%p success and now Session ID=%s"),
									pNewSession,pNewSession->getSessionID());
	
	uid.toString(szBuf,sizeof(szBuf)-1);
	pNewSession->set(GLOBAL_GUID,szBuf);
	

	pList->setUserCtxIdx(pNewSession->getSessionID());
#ifdef _TEST_PLAY_WITH_MAC_	
	pList->setDestMac("01:02:03:04:05:06");	
	pDestinationIP="127.0.0.1";
	pList->setDestination("127.0.0.1",destPort);
	pSite->CoreLog(Log::L_DEBUG,REQ("set destination=%s and port=%d"),"127.0.0.1",destPort);
#else
	pSite->CoreLog(Log::L_DEBUG,REQ("set destination=%s and port=%d"),pDestinationIP,destPort);
	pList->setDestination(pDestinationIP,destPort);
	if(bQam)
	{
		pList->setDestMac((char*)strDestMac.c_str());
	}	
#endif

			
	pResponse->printf_preheader(RESPONSE_OK);

	//pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf)-1);	
	pResponse->setHeader(HEADER_MTHDCODE, "Setup");

	pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");	
	pResponse->setHeader(HEADER_SESSION,pNewSession->getSessionID());
	if(!bQam)
	{
		unsigned __int16 sLen=sizeof(szBuf);
		pReq->getHeader(HEADER_TRANSPORT,szBuf,&sLen);
		
		if(strstr(szBuf,"destination")==NULL)
		{
			strcat(szBuf,";destination=");
			strcat(szBuf,pDestinationIP);
		}		
		pResponse->setHeader(HEADER_TRANSPORT,szBuf);
		
	}
	else
	{//Qam
		pResponse->setHeader(HEADER_TRANSPORT,"MP2T/DVBC/QAM;unicast");
		ZeroMemory(szBuf,sizeof(szBuf));
		sprintf(szBuf,"program-number=%d;frequency=%d;qam-mode=%d",ProNum,Frequency,Qammode);
		pResponse->setHeader(HEADER_SC_TRANSPORT,szBuf);
	}
	pResponse->post();
	pReq->setHeader(HEADER_SESSION,(char*)(pSessionID==NULL?"":pSessionID));

	pSite->CoreLog(Log::L_DEBUG,REQ("SETUP TIME COST=%d"),GetTickCount()-dwTestTime);
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
	unsigned __int16	iStrLen=sizeof(szBuf);
	ZeroMemory(szBuf,iStrLen);

	auto_release<IClientSession*> pClientSession=pSite->findClientSession(pSessionID);
	pSite->CoreLog(Log::L_DEBUG,SSREQ("Get Client Session %p"),pClientSession);
	if(pClientSession==NULL)
	{		
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't open client session"));
		pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND); 
		pResponse->post();		
		return RequestError;
	}
	
	ZQ::common::Variant varGuid=pClientSession->get(GLOBAL_GUID);

	tstring GuidStr=(tstring&)varGuid;
	
	pSite->CoreLog(Log::L_DEBUG,SSREQ("Get playlist guid %s"),GuidStr.c_str());

	ZQ::common::Guid playListGuid(GuidStr.c_str());
	IPlaylist* pList=pSite->openPlaylist(playListGuid,false);			
	if(!pList)
	{	
		playListGuid.toString(szBuf,sizeof(szBuf)-1);
		pSite->CoreLog(Log::L_ERROR,SSREQ("Get playlist fail with VERB==PLAY with Guid=%s"),szBuf);
		pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND); 
		pResponse->post();		
		return RequestError;		
	}

	pSite->CoreLog(Log::L_DEBUG,SSREQ("Get playlist %p with guid=%s"),pList,GuidStr.c_str());
	
	unsigned __int16	iStrSize=sizeof(szBuf)-1;
	double fScale=1.0f;
	if(NULL==pReq->getHeader(HEADER_SCALE,szBuf,&iStrSize))
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("No Scale value was found,Set it to 1.0"));
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
	
	
	if(pList->getCurrentState()==IPlaylist::PLAYLIST_SETUP || pList->getCurrentState()==IPlaylist::PLAYLIST_STOP )
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("playlist status=SETUP || STOP"));		
		
		if(!pList->play())
		{
			pSite->CoreLog(Log::L_ERROR,SSREQ("Play fail,err(%s) delete playlist object with guid=%s"),pList->lastError(), GuidStr.c_str());
			pList->destroy();
		
			pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
			pResponse->post();
			return RequestError;
		}
		else
		{	
			pSite->CoreLog(Log::L_DEBUG,SSREQ("Play OK"));			
			pResponse->printf_preheader(RESPONSE_OK);
			
			/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf)-1);*/
			pResponse->setHeader(HEADER_MTHDCODE, "Play");
			pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
			//pResponse->setHeader(HEADER_RANGE, "npt=0.000-");
			//pResponse->setHeader(HEADER_SCALE, "1.0000");
			//pResponse->post();			
		}
		
	}
	else if(pList->getCurrentState()==IPlaylist::PLAYLIST_PAUSE)
	{//resume
		pSite->CoreLog(Log::L_DEBUG,SSREQ("playlist status=PAUSE,resume it"));
		if(!pList->resume())
		{
			pSite->CoreLog(Log::L_ERROR,SSREQ("resume playlist fail,err(%s)"),pList->lastError());
			pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
			pResponse->post();
			return RequestError;
		}
		else
		{
			pSite->CoreLog(Log::L_DEBUG,SSREQ("resume playlist success"));
			pResponse->printf_preheader(RESPONSE_OK);

			/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf)-1);*/
			pResponse->setHeader(HEADER_MTHDCODE, "Play");
			pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
			//pResponse->setHeader(HEADER_RANGE, "npt=0.000-");
			//pResponse->setHeader(HEADER_SCALE, "1.0000");
			
		}
		
	}
	else if(pList->getCurrentState()==IPlaylist::PLAYLIST_PLAY)
	{//playing
		pSite->CoreLog(Log::L_DEBUG,SSREQ("playlist status=PLAY"));
		pResponse->printf_preheader(RESPONSE_OK);		
		
		/*pUtility->getVerbString(pReq->getVerb(),szBuf,sizeof(szBuf));*/
		pResponse->setHeader(HEADER_MTHDCODE, "Play");
		pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
		//pResponse->setHeader(HEADER_RANGE, "npt=0.000-");
		//pResponse->setHeader(HEADER_SCALE, "1.0000");
		
	}
	
	ZQ::common::Variant var;
	pList->getInfo(IPlaylist::infoPLAYPOSITION,var);
	tstring strScale=(tstring&)var[EventField_PlayScale];
	pResponse->setHeader(HEADER_SCALE,strScale.c_str());

	int timeOffset=(int)var[EventField_CurrentTimeOffset];
	int totalTime=(int)var[EventField_TotalTimeOffset];
	char szNptBuffer[1024];
	ZeroMemory(szNptBuffer,sizeof(szNptBuffer));
#ifdef _LOCAL_TEST
	sprintf(szNptBuffer,"npt=%d.%d-%d.%d",
		timeOffset/1000,timeOffset-timeOffset/1000*1000,
		totalTime/1000,totalTime-totalTime/1000*1000);
#else
	sprintf(szNptBuffer,"%d.%d-%d.%d",
		timeOffset/1000,timeOffset-timeOffset/1000*1000,
		totalTime/1000,totalTime-totalTime/1000*1000);
#endif	
	pResponse->setHeader(HEADER_RANGE,szNptBuffer);

	pResponse->post();

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
	unsigned __int16	iStrLen=sizeof(szBuf);
	ZeroMemory(szBuf,iStrLen);

	auto_release<IClientSession*>		pClientSession=pSite->findClientSession(pSessionID);
	if(pClientSession==NULL)
	{
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't find ClientSession"));
		pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND);
		pResponse->post();		
		return RequestError;
	}
	
	
	ZQ::common::Variant var=pClientSession->get(GLOBAL_GUID);
	tstring GuidStr=(tstring&)var;
	ZQ::common::Guid playListGuid(GuidStr.c_str());
	IPlaylist* pList=pSite->openPlaylist(playListGuid,false);
	if(!pList)
	{		
		playListGuid.toString(szBuf,sizeof(szBuf));
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't find PlayList with GUID=%s"),szBuf);
		pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND);
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
		pSite->CoreLog(Log::L_ERROR,SSREQ("pause playlist %s failed,err(%s)"),GuidStr.c_str(),pList->lastError());
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
		pResponse->post();
	}
	else
	{
		pSite->CoreLog(Log::L_DEBUG,SSREQ("pause the playlist %s successfully"),GuidStr.c_str());
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
	unsigned __int16	iStrLen=sizeof(szBuf);
	ZeroMemory(szBuf,iStrLen);

	pSite->CoreLog(Log::L_DEBUG,SSREQ("find playlist through client session id=%s"),pSessionID);
 	auto_release<IClientSession*> pClientSession=pSite->findClientSession(pSessionID);
	if(pClientSession==NULL)
	{		
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't open client session with sessionId=%s"),pSessionID);
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
		pSite->CoreLog(Log::L_DEBUG,SSREQ("find the play list %s ,Destroy it now!"),GuidStr.c_str());
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
	ZeroMemory(szBuf,1024);

	unsigned __int16	iLen=sizeof(szBuf);

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
	ZeroMemory(szBuf,1024);
	

	pResponse->printf_preheader(RESPONSE_OK);
	pResponse->printf_postheader("m=video 0 RAW/RAW/UDP 33\r\nc=IN IP4 0.0.0.0/255\r\na=control:basic");
	pResponse->post();	

	pSite->CoreLog(Log::L_DEBUG,REQ("Leave Describle"));
	return RequestDone;
}
RequestProcessResult SSMH_DefaultContentHandleGetParameter(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	pSite->CoreLog(Log::L_DEBUG,REQ("Enter GetParameter"));

	IServerResponse* pResponse=pReq->getResponse();
	if(!pResponse)
	{
		pSite->CoreLog(Log::L_DEBUG,REQ("Can't get IServerResponse through IClientRequest"));
		return RequestError;
	}
	char *pSessionID=(char*)pReq->getClientSessionId();
	if(!pSessionID || strlen(pSessionID)<=0 )
	{
		pSite->CoreLog(Log::L_ERROR,REQ("Can't get client session ID from client request"));
		pResponse->printf_preheader(RESPONSE_BAD_REQUEST);
		pResponse->post();
		return RequestError;
	}	
	
	pSite->CoreLog(Log::L_DEBUG,SSREQ("find playlist through client session id=%s"),pSessionID);
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
	

	ZQ::common::Guid uid(GuidStr.c_str());
	IPlaylist* pList=pSite->openPlaylist(uid,false);
	if(!pList)
	{
		pSite->CoreLog(Log::L_ERROR,SSREQ("Can't find playlist with session ID=%s"),pSessionID);
		pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND);
		pResponse->post();
		return RequestError;
	}
	//Get information from play list
	ZQ::common::Variant var;
	if(!pList->getInfo(IPlaylist::infoPLAYPOSITION,var))
	{
		pSite->CoreLog(Log::L_ERROR,SSREQ("get play position failed"));
		pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
		pResponse->post();
		return RequestError;
	}
	pResponse->printf_preheader(RESPONSE_OK);
	pResponse->setHeader(HEADER_MTHDCODE, "GET_PARAMETER");	
	pResponse->setHeader(HEADER_CONTENT_TYPE,"text/parameters");

	int timeOffset=(int)var[EventField_CurrentTimeOffset];
	tstring strScale=(tstring&)var[EventField_PlayScale];
	int totalTime=(int)var[EventField_TotalTimeOffset];
	std::string	strCntn;
	strCntn="presentation_state: ";
	IPlaylist::State st=pList->getCurrentState();
	switch(st)
	{
	case IPlaylist::PLAYLIST_SETUP:
		{
			strCntn+="setup";
		}
		break;
	case IPlaylist::PLAYLIST_PLAY:
		{
			strCntn+="playing";
		}
		break;
	case IPlaylist::PLAYLIST_PAUSE:
		{
			strCntn+="paused";
		}
		break;
	case IPlaylist::PLAYLIST_STOP:
		{
			strCntn+="stoped";
		}
		break;
	default:
		break;
	}
	strCntn+="\r\n";
	strCntn+="position: ";
	char szNptBuffer[1024];
	ZeroMemory(szNptBuffer,sizeof(szNptBuffer));
	sprintf(szNptBuffer,"%d.%d-%d.%d",
		timeOffset/1000,timeOffset-timeOffset/1000*1000,
		totalTime/1000,totalTime-totalTime/1000*1000);
	strCntn+=szNptBuffer;
	strCntn+="\r\n";
	strCntn+="scale: ";
	strCntn+=strScale;
	pResponse->printf_postheader(strCntn.c_str());
	pResponse->post();	

	pSite->CoreLog(Log::L_DEBUG,REQ("Leave GetParamter"));
	return RequestDone;
}
//Default content Handler
RequestProcessResult SSMH_DummyContentHandle (IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
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
			return SSMH_DefaultContentHandleGetParameter(pSite,pReq);
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
			const char* pSessionID=pReq->getClientSessionId();
			//pSite->CoreLog(Log::L_DEBUG,REQ("####NOT IMPLEMENT####"));
			char	szBuf[128];
			ZeroMemory(szBuf,sizeof(szBuf));
			//pResponse->printf_preheader(RESPONSE_NOT_IMPLEMENT); 			
			switch(pReq->getVerb())
			{
			case RTSP_MTHD_RECORD:				
				{
					strcpy(szBuf,"RECORD");
					pSite->CoreLog(Log::L_DEBUG,SSREQ("Verb=RECORD"));
				}
				break;
			case RTSP_MTHD_GET_PARAMETER:				
				{
					strcpy(szBuf,"GET_PARAMETER");					
					pSite->CoreLog(Log::L_DEBUG,SSREQ("Verb=GET_PARAMETER"));
				}
				break;
			case RTSP_MTHD_REDIRECT:				
				{
					strcpy(szBuf,"REDIRECT");
					pSite->CoreLog(Log::L_DEBUG,SSREQ("Verb=REDIRECT"));
				}
				break;
			case RTSP_MTHD_SET_PARAMETER:				
				{
					strcpy(szBuf,"SET_PARAMETER");
					pSite->CoreLog(Log::L_DEBUG,SSREQ("Verb=SET_PARAMETER"));
				}
				break;		
			default:
				{
					strcpy(szBuf,"Unknown");
					pSite->CoreLog(Log::L_DEBUG,SSREQ("Verb=Unknown"));
				}
				break;
			}
			pResponse->setHeader(HEADER_MTHDCODE,szBuf);
			pResponse->printf_preheader(RESPONSE_OK); 
			pResponse->post();			
			return RequestDone;
		}
		break;
	}	
	return RequestDone;	
}


bool SSMH_DummyEventSink(IStreamSmithSite* pSite,EventType dEventType,ZQ::common::Variant& params)
{
	switch(dEventType)
	{
	case E_PLAYLIST_END:
	case E_PLAYLIST_BEGIN:
	case E_PLAYLIST_SESSEXPIRED:
		{		
			tstring ClientSessionID=params[EventField_ClientSessId];
			pSite->CoreLog(Log::L_DEBUG,"void SSMH_DummyEventSink() event sink come with session id =%s and event Type=%d",
								ClientSessionID.c_str(),dEventType);
			
			auto_release<IServerRequest*> pRequest=pSite->newServerRequest(ClientSessionID.c_str());
			if(pRequest==NULL)
			{
				pSite->CoreLog(Log::L_ERROR,"Can't get server request through client session ID=%s \
							and this message won't be sent out again",ClientSessionID.c_str());				
				
				return true;
			}
			pRequest->printCmdLine(RESPONSE_ANNOUCE);
			pRequest->printHeader("Session",(char*)ClientSessionID.c_str());
			char	szBuf[1024];
			ZeroMemory(szBuf,1024);
			
			SYSTEMTIME	st;
			GetSystemTime(&st);
			if(dEventType==E_PLAYLIST_BEGIN)
			{
				sprintf(szBuf,"2104 Start-of-Stream Reached %04d%02d%02dT%02d%02d%02d.%03dZ",
					st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			else if(dEventType==E_PLAYLIST_END)
			{
				sprintf(szBuf,"2101 End-of-Stream Reached %04d%02d%02dT%02d%02d%02d.%03dZ",
					st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			else if(dEventType==E_PLAYLIST_SESSEXPIRED)
			{
				sprintf(szBuf,"5502 Internal Server Error %04d%02d%02dT%02d%02d%02d.%03dZ",
					st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			else
			{
				
			}
			pSite->CoreLog(Log::L_DEBUG,"[ClientSessionID(%s)] stream event was sent out with {%s}",
					ClientSessionID.c_str(),szBuf);
			pRequest->printHeader("SeaChange-Notice",szBuf);			
			pRequest->post();
#ifdef _TEST_PLAY_WITH_MAC_

#else
			//pRequest->closeConnection();
#endif
		}
		break;
	default:
		break;
	}
	return true;
}

#include <vector>
#include <string>
typedef	std::vector<std::string>  VECUID;


#define	STR_ASSET_UID	"assetuid"
#define	STR_ASSET_NAME	"assetname"

void CheckAssetID(IStreamSmithSite* pSite,const char* AssetID,int type,VECUID& vUID,IClientRequestWriter* pReq)
{
	vUID.clear();
	try
	{
#ifdef _LOCAL_TEST
		vUID.push_back("good");
		vUID.push_back("good");
		pReq->setHeader(RTSP_MSG_MAX_RATE_HEADER,"5000000");
		return;
#endif//_LOCAL_TEST
		if(!_LAMFacadeprx)
		{
			pSite->CoreLog(Log::L_ERROR,"No available LAMFacade,return with failure");
			return;
		}	
			
		if(!AssetID||strlen(AssetID)<0)
		{
			pSite->CoreLog(Log::L_ERROR,"CheckAssetID()##No asset id pass in");
		}
		int aeid=0;
		sscanf(AssetID,"%x",&aeid);
		pSite->CoreLog(Log::L_DEBUG,REQ("get AE list from ice interface with asset id=%d"),aeid);	
		
		int MuxRate=0;
		AssetElementCollection	aCl=_LAMFacadeprx->getAEList(aeid);

		pSite->CoreLog(Log::L_DEBUG,REQ("get ae list and size=%d"),aCl.size());
		for(int i=0;i<(int)aCl.size();i++)
		{
			vUID.push_back(aCl[i].aeUID);
			pSite->CoreLog(Log::L_DEBUG,REQ("Get a UID=%s and bandWidth =%d"),aCl[i].aeUID.c_str(),aCl[i].bandWidth);
			if(aCl[i].bandWidth>MuxRate)
				MuxRate=aCl[i].bandWidth;
		}

		char szBufMuxRate[128];
		ZeroMemory(szBufMuxRate,sizeof(szBufMuxRate));
		if(MuxRate>0)
			sprintf(szBufMuxRate,"%d",MuxRate);
		else
		{
			pSite->CoreLog(Log::L_DEBUG,REQ("no bandwidth available,set it to 4000000(4000KBPS)"));
			sprintf(szBufMuxRate,"%d",4000000);
		}
		pSite->CoreLog(Log::L_DEBUG,REQ("set maxBitRate to %s(bps)"),szBufMuxRate);
		pReq->setHeader(RTSP_MSG_MAX_RATE_HEADER,szBufMuxRate);		
	}
	catch (Ice::Exception& e)
	{
		pSite->CoreLog(Log::L_ERROR,("Find Asset UID fail with ice error description is %s"),e.ice_name().c_str());		
	}
	catch (...)
	{
		pSite->CoreLog(Log::L_ERROR,("void CheckAssetID()## Unexcept error"));
	}
}


RequestProcessResult SSMH_DummyFixupRequest (IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	pSite->CoreLog(Log::L_DEBUG,REQ("Enter SSMH_DummyFixupRequest"));

#ifdef _TEST_PLAY_WITH_MAC_
	if(!gbFindIDS)
	{
		std::string strUri="rtsp://./basic?";
		char szBUf[10];
		for(int i=0;i<gRepeatCount;i++)
		{
			strUri=strUri+"Item";			
			strUri=strUri+itoa(i,szBUf,10);
			strUri=strUri+'=';
			strUri=strUri+gstrItemName;
			strUri=strUri+"&";
		}
		pReq->setArgument(pReq->getVerb(),strUri.c_str(),"RTSP");
		pSite->CoreLog(Log::L_DEBUG,REQ("Leave SSMH_DummyFixupRequest"));
		return RequestProcessed;
	}
	else
	{	
#endif//_TEST_WITHOUT_FINDASSET__

	IServerResponse* pResponse=pReq->getResponse();
	if(!pResponse)
	{
		pSite->CoreLog(Log::L_ERROR,REQ("Can't get IServerResponse through IClientRequest"));
		return RequestError;
	}
	
	char strURI[2048];
	ZeroMemory(strURI,2048);
	pReq->getUri(strURI,2047);
	pSite->CoreLog(Log::L_DEBUG,REQ("Before fix up the URI, uri is %s"),strURI);
	
	URIParser parser(strURI);
	parser.SetDefaultApplication("basic");
	
	parser.SetDefaultSite(STR_SITE_DEFAULT);
	
	std::string	strTContent=parser.GetContent();
	if( (stricmp("SeaChange/ITV",parser.GetPath())==0 ||stricmp(parser.GetPath(),"mediacluster")==0 ) && (strTContent.find('.')!=std::string::npos) )
	{//How to check the uri is old format?		
		pSite->CoreLog(Log::L_DEBUG,REQ("Old formmat set path to basic"));
		parser.SetPath("basic");		
	}
	else
	{
		pSite->CoreLog(Log::L_DEBUG,REQ("can't recognise ,return with RequestProcessed"));
		return RequestProcessed;
	}
	
	//fill the uri
	const char* pInitURI=parser.FillUri();
	pSite->CoreLog(Log::L_DEBUG,REQ("Fix up the uri first and now URI is %s"),pInitURI);

	if(pReq->getVerb()==RTSP_MTHD_SETUP)
	{
		pSite->CoreLog(Log::L_DEBUG,REQ("VERB==SETUP"));
		std::string	strTempContent=parser.GetContent();

		std::string	strAsset;
		int			iTempPos=0;
		VECUID		vUID;
		
		if((iTempPos=strTContent.find('.'))!=std::string::npos )
		{//How to check the uri is old format?
			strAsset=strTempContent.substr(iTempPos+1);
//			if(strAsset.find("0x")!=0)
//				strAsset="0x"+strAsset;
			LONG tickCount = GetTickCount();
			CheckAssetID(pSite,strAsset.c_str(),0,vUID,pReq);
			pSite->CoreLog(Log::L_DEBUG, REQ("CheckAssetID() run time count %d"),GetTickCount() - tickCount);
		}
		else
		{
			strAsset=strTempContent;
			
			if(parser.GetValue(STR_ASSET_UID)==NULL)
			{
				pSite->CoreLog(Log::L_DEBUG,REQ("NO asset ID"));
				if(parser.GetValue(STR_ASSET_NAME)==NULL)
				{
					char szBuf1[32];
					sprintf(szBuf1,"%s%d",STR_URI_KEY_PRE,0);
					if(parser.GetValue(szBuf1)!=NULL)
					{
						pReq->setArgument(pReq->getVerb(),parser.FillUri(),pReq->getProtocol(strURI,2047));
						pSite->CoreLog(Log::L_DEBUG,REQ("Leave SSMH_DummyFixupRequest"));
						return RequestProcessed;
					}
					pSite->CoreLog(Log::L_ERROR,REQ("RequestProcessResult SSMH_DummyFixupRequest()## NO assetUID or ASsetName found"));
//					pResponse->printf_preheader(RESPONSE_BAD_REQUEST); 
//					char szBuf[256];
//					unsigned __int16 sLen=256;
//					pReq->getHeader(HEADER_SEQ,szBuf,&sLen);
//					pResponse->setHeader(HEADER_SEQ,szBuf);
//					pResponse->post();
					return RequestProcessed;				
				}
				else
				{
					pSite->CoreLog(Log::L_DEBUG,REQ("Process with ASSet name=%s"),parser.GetValue(STR_ASSET_NAME));
					CheckAssetID(pSite,parser.GetValue(STR_ASSET_NAME),0,vUID,pReq);
				}
			}
			else
			{
				pSite->CoreLog(Log::L_DEBUG,REQ("Process with Asset UID=%s"),parser.GetValue(STR_ASSET_UID));
				std::string	tempAsset=parser.GetValue(STR_ASSET_UID);
				if(tempAsset.find("0x")!=0)
					tempAsset="0x"+tempAsset;
				CheckAssetID(pSite,tempAsset.c_str(),0,vUID,pReq);
			}
		}
		
		parser.SetContent("");
		
		char	szBuf[32];
		/*char	szUID[32];*/
		for(int i=0;i<(int)vUID.size();i++)
		{
			sprintf(szBuf,"%s%d",STR_URI_KEY_PRE,i);
			//sprintf(szUID,"%08X",vUID[i]);
			pSite->CoreLog(Log::L_DEBUG,REQ("Get Key=%s UID=%s"),szBuf,vUID[i]);
			parser.SetValue(szBuf,vUID[i]);
		}
		if(vUID.size()<=0)
		{
			pSite->CoreLog(Log::L_ERROR,REQ("No Asset element ID was found"));
		}
		const char *pURI=parser.FillUri();
		pSite->CoreLog(Log::L_DEBUG,REQ("fix request uri=%s "),pURI);
		pReq->setArgument(pReq->getVerb(),pURI,pReq->getProtocol(strURI,2047));
	}
	else
	{
		pSite->CoreLog(Log::L_DEBUG,REQ("VERB != SETUP"));
		pReq->setArgument(pReq->getVerb(),pInitURI,pReq->getProtocol(strURI,2047));
	}
	
	pSite->CoreLog(Log::L_DEBUG,REQ("Leave SSMH_DummyFixupRequest"));
#ifdef _TEST_PLAY_WITH_MAC_
	}
#endif
	return RequestProcessed;
}



bool InitIce(const std::string& configFile,IStreamSmithSite* pSite)
{
#ifdef _LOCAL_TEST
	return true;
#endif//_LOCAL_TEST
	try
	{
		InitInfo ini;
		if(!ini.init(configFile.c_str()))
		{
			pSite->CoreLog(Log::L_ERROR,"can't find configuration %s",configFile.c_str());
			return false;
		}
		ini.setCurrent("IceConf");
		std::string	strEndPoint;
		if(!ini.getValue("LAMFacadeEndPoint",strEndPoint,""))
		{
			pSite->CoreLog(Log::L_ERROR,"can't find LAMFacadeEndPoint");
			return false;
		}
		pSite->CoreLog(Log::L_DEBUG,"get LAMFacadeEndPoint is %s  and connect it",strEndPoint.c_str());
		//Ice::CommunicatorPtr	_iceCommunicator=NULL;
		//Ice::ObjectAdapterPtr	_AssetAdpterPtr=NULL;
		//LAMFacadePrx			_LAMFacadeprx=NULL; 

		int value=0;
		_iceCommunicator = ::Ice::initialize(value,NULL);
		_LAMFacadeprx=LAMFacadePrx::checkedCast(_iceCommunicator->stringToProxy(strEndPoint));
		if(_LAMFacadeprx==NULL)
		{
			pSite->CoreLog(Log::L_ERROR,"connect to %s failed",strEndPoint.c_str());
			return false;
		}		
	}
	catch(::Ice::Exception& ex)
	{
		pSite->CoreLog(Log::L_ERROR,"catch a exception is %s",ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{	
		pSite->CoreLog(Log::L_ERROR,"catch an unexpect error");
		return false;
	}
	return true;
}


bool _gbInitialized=false;
void ModuleInit(IStreamSmithSite* pSite)
{
	pSite->CoreLog(Log::L_DEBUG,"Enter ModuleInit");
	pSite->RegisterContentHandle("basic",(SSMH_ContentHandle)SSMH_DummyContentHandle);
	pSite->RegisterFixupRequest((SSMH_FixupRequest)SSMH_DummyFixupRequest);
	pSite->SinkEvent(E_PLAYLIST_END|E_PLAYLIST_BEGIN,(SSMH_EventSink)SSMH_DummyEventSink);
	pSite->CoreLog(Log::L_DEBUG,"Leave ModuleInit");
}
void ModuleInitEx(IStreamSmithSite* pSite,const char* pConf)
{
	pSite->CoreLog(Log::L_DEBUG,"Enter ModuleInit");
	pSite->RegisterContentHandle("MOD",(SSMH_ContentHandle)SSMH_DummyContentHandle);
	pSite->RegisterFixupRequest((SSMH_FixupRequest)SSMH_DummyFixupRequest);
	pSite->SinkEvent(E_PLAYLIST_END|E_PLAYLIST_BEGIN|E_PLAYLIST_SESSEXPIRED,
						(SSMH_EventSink)SSMH_DummyEventSink);

	if(!_gbInitialized)
	{
		pSite->CoreLog(Log::L_DEBUG,"ice  initlializing");
		std::string	strConf=pConf!=NULL?pConf:"";
		if(!InitIce(strConf,pSite))
		{
			pSite->CoreLog(Log::L_CRIT,"init ice fail");
		}
		else
		{
			_gbInitialized=true;
			pSite->CoreLog(Log::L_DEBUG,"ice initialize ok");
		}
	}
	else
	{
		pSite->CoreLog(Log::L_DEBUG,"ice has been initlialized");
	}
	pSite->CoreLog(Log::L_DEBUG,"Leave ModuleInit");
}


void ModuleUninit(IStreamSmithSite* pSite)
{
		//Ice::CommunicatorPtr	_iceCommunicator=NULL;
		//Ice::ObjectAdapterPtr	_AssetAdpterPtr=NULL;
		//LAMFacadePrx			_LAMFacadeprx=NULL; 
	try
	{
		if(_LAMFacadeprx)
			_LAMFacadeprx=NULL;
	}
	catch(...){ }

	try
	{
		if(_AssetAdpterPtr)
		{
			_AssetAdpterPtr->deactivate();
			_AssetAdpterPtr=NULL;
		}
	}
	catch (...)
	{
	}
	try
	{
		if(_iceCommunicator)
			_iceCommunicator->destroy();
	}
	catch(...){ }
	_gbInitialized=false;
}
