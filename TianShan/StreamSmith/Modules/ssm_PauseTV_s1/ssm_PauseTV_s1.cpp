// ssm_PauseTV_s1.cpp : Defines the entry point for the DLL application.
//

#include	"ssm_PauseTV_s1.h"
#include	"StreamEventSinkI.h"
#include	"PlaylistEventSinkI.h"
#include	"purchaseProperty.h"
#include	"Locks.h"
#include	"Winsock2.h"
#include	<fstream>
#include	"exptHandle.h"
#pragma		comment(lib,"Ws2_32.lib")

#define	myGlog	(CSsm_PauseTV_s1::ssm_PauseTV_s1_Log)
#define PSLOG(_X) "	" _X
#define PSLOG2(_X) "		" _X


template<class classType>
class auto_free
{
public:
	auto_free(classType _p)
	{
		_ptr=_p;
	}
	~auto_free()
	{
		if(_ptr!=NULL)
		{
			_ptr->free();
			_ptr=NULL;
		}
	}	
public:
	classType& operator->()
	{
		return _ptr;
	}
	classType& operator=(const classType t)
	{
		if(_ptr!=NULL)
		{
			_ptr->free();
			_ptr=NULL;
		}
		_ptr=t;
		return _ptr;
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

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				//
			}
		case DLL_THREAD_ATTACH:
			{
				//
			}
		case DLL_THREAD_DETACH:
			{
				//
			}
		case DLL_PROCESS_DETACH:
			{
				//
			}
    }
    return TRUE;
}

IceUtil::RecMutex										CSsm_PauseTV_s1::clientInfoMapMutex;
IceUtil::RecMutex										CSsm_PauseTV_s1::streamToSessionMapMutex;
CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP				CSsm_PauseTV_s1::clientInfoMap;
CSsm_PauseTV_s1::STREAM_TO_SESSION_MAP					CSsm_PauseTV_s1::streamToSessionMap;
CSsm_PauseTV_s1::CONFIGURATION_MAP						CSsm_PauseTV_s1::configMap;
::std::vector<::std::string>							CSsm_PauseTV_s1::configString;

::Ice::CommunicatorPtr									CSsm_PauseTV_s1::ic;
::ChannelOnDemand::ChannelOnDemandAppPrx				CSsm_PauseTV_s1::codApp;
::TianShanIce::Streamer::StreamSmithAdminPrx			CSsm_PauseTV_s1::stmservice;

bool										CSsm_PauseTV_s1::bModuleInit = false;
bool										CSsm_PauseTV_s1::iceInitialized;
bool										CSsm_PauseTV_s1::bConnChodsvcOK;
bool										CSsm_PauseTV_s1::bConnStreamSmithOK;
bool										CSsm_PauseTV_s1::bConnIceStormOK;
thrdConnService*							CSsm_PauseTV_s1::pConnServiceThrd;
thrdCleanupSession*							CSsm_PauseTV_s1::pCleanSessionThrd;
bool										_objectCreated = false;
IStreamSmithSite*							CSsm_PauseTV_s1::globalSite;
::std::string								CSsm_PauseTV_s1::_configFilePath;
Ice::ObjectAdapterPtr						CSsm_PauseTV_s1::_topicAdapter=NULL;
TianShanIce::Events::EventChannelImpl::Ptr  CSsm_PauseTV_s1::_eventChannel=NULL;

//#define SessLOGFMT(_C, _X, _Sess) CLOGFMT(_C, "sess[%s] " _X), _Sess.c_str()		/// 

void ReplaceChar(::std::string& inStr,TCHAR from,TCHAR to)
{
	int nLen = inStr.size();
	for(int i=0;i<nLen;i++)
	{
		if(inStr[i] == from)
		{
			inStr[i] = to;
		}
	}
}

void CSsm_PauseTV_s1::DestroyPurchaseStream(::std::string sStream,::std::string sPurchase)
{
	if(sPurchase.size())
	{
		try
		{
			Ice::ObjectPrx basePurchase = CSsm_PauseTV_s1::ic->stringToProxy(sPurchase);
			::ChannelOnDemand::ChannelPurchaseExPrx purPrx = ::ChannelOnDemand::ChannelPurchaseExPrx::checkedCast(basePurchase);
			purPrx->destroy();
			myGlog(ZQ::common::Log::L_DEBUG,PSLOG("Destroy Purchase: \"%s\""),sPurchase.c_str());
		}
		catch(TianShanIce::BaseException& ex)
		{
			myGlog(ZQ::common::Log::L_ERROR,PSLOG("Catch an %s:%s when destroy purchase \"%s\""),ex.ice_name().c_str(),ex.message.c_str(),sPurchase.c_str());
		}
		catch(Ice::Exception& ex)
		{
			myGlog(ZQ::common::Log::L_ERROR,PSLOG("Catch an %s when destroy purchase \"%s\""),ex.ice_name().c_str(),sPurchase.c_str());
		}
		catch(...)
		{
			myGlog(ZQ::common::Log::L_ERROR,PSLOG("Catch an unknown Exception when destroy purchase \"%s\""),sPurchase.c_str());
		}
	}
	if(sStream.size())
	{
		try
		{
			Ice::ObjectPrx baseStm = CSsm_PauseTV_s1::ic->stringToProxy(sStream);
			TianShanIce::Streamer::StreamPrx stmPrx = TianShanIce::Streamer::StreamPrx::checkedCast(baseStm);
			stmPrx->destroy();
			myGlog(ZQ::common::Log::L_DEBUG,PSLOG("Destroy Stream: \"%s\""),sStream.c_str());
		}
		catch(TianShanIce::BaseException& ex)
		{
			myGlog(ZQ::common::Log::L_ERROR,PSLOG("Catch an %s:%s when destroy stream \"%s\""),ex.ice_name().c_str(),ex.message.c_str(),sStream.c_str());
		}
		catch(Ice::Exception& ex)
		{
			myGlog(ZQ::common::Log::L_ERROR,PSLOG("Catch an %s when destroy stream \"%s\""),ex.ice_name().c_str(),sStream.c_str());
		}
		catch(...)
		{
			myGlog(ZQ::common::Log::L_ERROR,PSLOG("Catch an unknown Exception when destroy stream \"%s\""),sStream.c_str());
		}
	}
}

void CSsm_PauseTV_s1::DestroyAllPurchase()
{
	CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::const_iterator it;
	::std::map<std::string,::std::string> SessPurchaseMap;
	::std::map<std::string,::std::string>::const_iterator itSessPurchase;

	// Add all purchase string to a vector
	{
		IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::clientInfoMapMutex);
		for(it = CSsm_PauseTV_s1::clientInfoMap.begin(); it != CSsm_PauseTV_s1::clientInfoMap.end(); it++)
		{
			SessPurchaseMap[it->first] = it->second.purString;
		}
	}

	// Destroy every purchase in the vector
	for(itSessPurchase = SessPurchaseMap.begin(); itSessPurchase != SessPurchaseMap.end(); itSessPurchase++)
	{
		// Delete session
		{
			IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::clientInfoMapMutex);
			CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::iterator itDel = CSsm_PauseTV_s1::clientInfoMap.find(itSessPurchase->first);
			if(itDel != CSsm_PauseTV_s1::clientInfoMap.end())
			{
				CSsm_PauseTV_s1::clientInfoMap.erase(itDel);
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Delete Session: %s"),itSessPurchase->first.c_str());
			}
		}

		// Delete purchase
		try
		{
			Ice::ObjectPrx basePurchase = CSsm_PauseTV_s1::ic->stringToProxy(itSessPurchase->second);
			::ChannelOnDemand::ChannelPurchaseExPrx purPrx = ::ChannelOnDemand::ChannelPurchaseExPrx::checkedCast(basePurchase);
			purPrx->destroy();
			myGlog(ZQ::common::Log::L_DEBUG,PSLOG("Destroy Purchase: %s"),itSessPurchase->second.c_str());
		}
		catch(TianShanIce::BaseException& ex)
		{
			myGlog(ZQ::common::Log::L_DEBUG,PSLOG("Catch an %s:%s when destroy purchase \"%s\""),ex.ice_name().c_str(),ex.message.c_str(),itSessPurchase->second.c_str());
		}
		catch(Ice::Exception& ex)
		{
			myGlog(ZQ::common::Log::L_DEBUG,PSLOG("Catch an %s when destroy purchase \"%s\""),ex.ice_name().c_str(),itSessPurchase->second.c_str());
		}
		catch(...)
		{
			myGlog(ZQ::common::Log::L_DEBUG,PSLOG("Catch an unknown Exception when destroy purchase \"%s\""),itSessPurchase->second.c_str());
		}
	}
}

/// Write client session, site, path, channel id, stream string and purchase string to a file
//void CSsm_PauseTV_s1::StoreStreamPurchase(const ::std::string resumePath)
//{
//	ofstream ofile;
//	/// open file to write message, if the file exists, delete it.
//	ofile.open(resumePath.c_str());
//	if(ofile.is_open())
//	{
//		CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::const_iterator itClientInfo;
//		::std::string strSite,strPath,strChannelId,strStream,strPurchase;
//		{
//			IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::clientInfoMapMutex);
//			for(itClientInfo = CSsm_PauseTV_s1::clientInfoMap.begin();itClientInfo != CSsm_PauseTV_s1::clientInfoMap.end();itClientInfo++)
//			{
//				strSite = ((CSsm_PauseTV_s1::CLIENT_INFO)(itClientInfo->second)).site;
//				if(!strSite.size())
//					continue;
//				strPath = ((CSsm_PauseTV_s1::CLIENT_INFO)(itClientInfo->second)).path;
//				if(!strPath.size())
//					continue;
//				strChannelId = ((CSsm_PauseTV_s1::CLIENT_INFO)(itClientInfo->second)).channelId;
//				if(!strChannelId.size())
//					continue;
//				strStream = ((CSsm_PauseTV_s1::CLIENT_INFO)(itClientInfo->second)).stmString;
//				if(!strStream.size())
//					continue;
//				strPurchase= ((CSsm_PauseTV_s1::CLIENT_INFO)(itClientInfo->second)).purString;
//				if(!strPurchase.size())
//					continue;
//				ReplaceChar(strStream,' ','\\');
//				ReplaceChar(strPurchase,' ','\\');
//				
//				/// Any item is not empty here, then write then into a file.
//				ofile<<itClientInfo->first<<" ";
//				ofile<<strSite<<" ";
//				ofile<<strPath<<" ";
//				ofile<<strChannelId<<" ";
//				ofile<<strStream<<" ";
//				ofile<<strPurchase<<endl;
//				myGlog(ZQ::common::Log::L_DEBUG,"		Save session %s to file",itClientInfo->first);
//			}
//		}
//		ofile.close();
//	}
//	else
//	{
//		myGlog(ZQ::common::Log::L_DEBUG,"		fail to open file (%s) to store message",resumePath.c_str());
//	}
//}

/// Load client session, site, path, channel id, stream string and purchase string from a file,
/// then rebuilt the map, such as STREAM_MAP,CLIENTINFO_MAP,PURCHASE_MAP.
//void CSsm_PauseTV_s1::LoadStreamPurchase(const ::std::string resumePath)
//{
//	ifstream ifile;
//	ifile.open(resumePath.c_str());
//	if(ifile.is_open())
//	{
//		IceUtil::RecMutex::Lock lock1(::CSsm_PauseTV_s1::streamToSessionMapMutex);
//		IceUtil::RecMutex::Lock lock2(::CSsm_PauseTV_s1::clientInfoMapMutex);
//		int nCount=0;
//		while(ifile)
//		{
//			::std::string sClientSession,sSite,sPath,sChannelId,sStream,sPurchase;
//			ifile>>sClientSession;
//			if(sClientSession.size() == 0)
//				break;
//			ifile>>sSite;
//			ifile>>sPath;
//			ifile>>sChannelId;
//			ifile>>sStream;
//			ifile>>sPurchase;
//			ReplaceChar(sStream,'\\',' ');
//			ReplaceChar(sPurchase,'\\',' ');
//			{
//				CSsm_PauseTV_s1::streamToSessionMap[sStream] = sClientSession;
//				CSsm_PauseTV_s1::CLIENT_INFO cltInfo;
//				cltInfo.channelId = sChannelId;
//				cltInfo.path = sPath;
//				cltInfo.site = sSite;
//				cltInfo.stmString = sStream;
//				cltInfo.purString = sPurchase;
//				CSsm_PauseTV_s1::clientInfoMap[sClientSession] = cltInfo;
//				UpdateLastAccessTime(sClientSession,"LoadSession");
//				myGlog(ZQ::common::Log::L_DEBUG,"		Success to load session %s",sClientSession);
//				nCount++;
// 			}
//		}
//		ifile.close();
//		myGlog(ZQ::common::Log::L_DEBUG,"		Success to load %d sessions",nCount);
//	}
//}

void SplitStringByChar(const ::std::string& contString,char vChar,::std::vector<::std::string> &strArray)
{
	int cur=0,nConStringLen=0;
	int tempCur=0;

	nConStringLen = contString.size();
	if(nConStringLen==0)
	{
		strArray.clear();	
		return;
	}
	
	::std::string strTemp = "";

	for(cur=0;cur<nConStringLen;cur++)
	{
		if(contString[cur]!=vChar)
		{
			strTemp.push_back(contString[cur]);
		}
		else
		{
			strArray.push_back(strTemp);
			strTemp = "";
		}
	}
	strArray.push_back(strTemp);
}

// 获得conString中右边第一个vChar后面的字符串，若没有vChar则返回空字符串。
void GetStringRightAtChar(const ::std::string& contString,char vChar,::std::string &strTemp)
{
	int strLen = contString.size();
	int cur;
	for(cur = strLen-1;cur >= 0;cur--)
	{
		if(contString[cur] == vChar)
			break;
	}
	if(cur < 0)
	{
		strTemp = "";
		return;
	}
	else
	{
		cur++;
		while(cur<strLen)
		{
			strTemp.push_back(contString[cur]);
			cur++;
		}
	}
}

void ChangeToReturn(::std::string& vString)
{
	::std::string sTemp;
	sTemp = vString;
	vString = "clock=";
	vString = vString + sTemp;
	vString = vString + "-";
}

// 获得contString中在第一个vChar左边的字符串, 若没有vChar则返回空字符串.
void GetStringLeftAtChar(const ::std::string& contString,char vChar,::std::string &strTemp)
{
	int strLen = contString.size();
	int cur;
	for(cur = 0;cur < strLen;cur++)
	{
		if(contString[cur] == vChar)
			break;
	}
	if(cur >= strLen)
	{
		strTemp = "";
		return;
	}
	else
	{
		int tempCur=0;
		while(tempCur<cur)
		{
			strTemp.push_back(contString[tempCur]);
			tempCur++;
		}
	}
}

void CSsm_PauseTV_s1::ErrorResponse(IServerResponse* pResponse,const char* sHeader,const char* sVerbCode,const char* sSCNotice,const char* sSessionID)
{
	pResponse->printf_preheader(sHeader);
	pResponse->setHeader(HEADER_MTHDCODE,sVerbCode);
	pResponse->setHeader(HEADER_SC_NOTICE,sSCNotice);
	pResponse->setHeader(HEADER_SESSION,sSessionID);
	pResponse->post();
	myGlog(ZQ::common::Log.L_ERROR,PSLOG("(%s:%s)\"%s\" has been sent to client."),sSessionID,sVerbCode,sHeader);
}

void CSsm_PauseTV_s1::UpdateLastAccessTime(const ::std::string& strSession,const ::std::string& strMethod)
{
	time_t tmNow;
	tmNow= time(NULL);
	{
		IceUtil::RecMutex::Lock lock(CSsm_PauseTV_s1::clientInfoMapMutex);
		CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::iterator itClient = CSsm_PauseTV_s1::clientInfoMap.find(strSession);
		if(itClient == CSsm_PauseTV_s1::clientInfoMap.end())
		{
			return;
		}
		(itClient->second).lastAccessTime = tmNow;
	}
	
	tm* tmStru = localtime(&tmNow);
	char strLastAccess[50];
	sprintf(strLastAccess,"%d-%02d-%02d %02d:%02d:%02d",tmStru->tm_year+1900,tmStru->tm_mon+1,tmStru->tm_mday
		,tmStru->tm_hour,tmStru->tm_min,tmStru->tm_sec);
	myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Store Last Access Time: %s"),strSession.c_str(),strMethod.c_str(),strLastAccess);
}

bool CSsm_PauseTV_s1::HdlGetParameterReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID)
{
	IServerResponse *pResponse = pReq->getResponse();
	::std::string sMethod = "GetParameter";
	char gBuff[MY_BUFFER_SIZE];
	unsigned __int16 gBuffLen = MY_BUFFER_SIZE-1;
	const char* pSeq = pReq->getHeader(HEADER_SEQ, gBuff, &gBuffLen);
	if (pSeq != NULL && strlen(pSeq))
	{
		myGlog(ZQ::common::Log::L_DEBUG, PSLOG("CSeq: %s"), pSeq);
	}
	::std::string retContentBody="";
	::std::string sPosition="",sScale="";
    try
	{
		::std::string	strPur;
		retSessionID = pReq->getClientSessionId();
		
		UpdateLastAccessTime(retSessionID,sMethod);
		
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("RTSP REQUEST: GET_PARAMETER"
			SEPARATOR_LINE "	SESSION_ID: %s"
			SEPARATOR_LINE "	(%s:%s)%s")
			,retSessionID.c_str()
			,retSessionID.c_str(),sMethod.c_str(),BEFORE_FIND_SESSION);
		
		{
			IceUtil::RecMutex::Lock lock(clientInfoMapMutex);
			SESSION_TO_CLIENT_INFO_MAP::const_iterator it = clientInfoMap.find(retSessionID);
			if(it == clientInfoMap.end())
			{
				myGlog(ZQ::common::Log.L_ERROR,PSLOG("(%s:%s)%s"),retSessionID.c_str(),sMethod.c_str(),CANNOT_FIND_SESSION);
				pResponse->printf_preheader(RESPONSE_SESSION_NOTFOUND);
				pResponse->setHeader(HEADER_MTHDCODE, "GET_PARAMETER");
				sprintf(gBuff,"%s",CANNOT_FIND_SESSION);
				pResponse->setHeader(HEADER_SC_NOTICE,gBuff);
				pResponse->setHeader(HEADER_SESSION,retSessionID.c_str());
				pResponse->post();
				return false;
			}
			strPur = ((CLIENT_INFO)(it->second)).purString;
		}
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)%s"),retSessionID.c_str(),sMethod.c_str(),SUCCESS_FIND_SESSION);

		
		//////////////////////////////////////////////////////////////////////////
		//TODO: Get Content-Body

		::std::vector<::std::string> strArray;
		::std::vector<::std::string>::const_iterator itString;
		unsigned char contentBody[MY_BUFFER_SIZE];
		unsigned __int32 contentBodyLen = MY_BUFFER_SIZE-1;
		const char* pBuff = pReq->getContent(contentBody,&contentBodyLen);
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)%s:%s"),retSessionID.c_str(),sMethod.c_str(),HEADER_CONTENT_BODY,contentBody);
		SplitStringByChar(pBuff,' ',strArray);
		
		//////////////////////////////////////////////////////////////////////////
		
		
		bool bHasPos=false,bHasScale=false;
		for(itString = strArray.begin();itString != strArray.end();itString++)
		{
			if(stricmp((*itString).c_str(),"bcastpos") == 0)
			{
				bHasPos = true;
				continue;
			}

			if(stricmp((*itString).c_str(),"scale") == 0)
			{
				bHasScale = true;
				continue;
			}
		}

		
		//////////////////////////////////////////////////////////////////////////
		//DO: Find purchase

		if(bHasPos || bHasScale)
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before find Purchase(%s)"),retSessionID.c_str(),sMethod.c_str(),strPur.c_str());
			Ice::ObjectPrx basePur = CSsm_PauseTV_s1::ic->stringToProxy(strPur);
			ChannelOnDemand::ChannelPurchaseExPrx purPrx = ChannelOnDemand::ChannelPurchaseExPrx::checkedCast(basePur);
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success find Purchase"
				SEPARATOR_LINE "	Before Get Position And Scale")
				,retSessionID.c_str(),sMethod.c_str());
			
			purPrx->getCurPosAndScale(sPosition,sScale);
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Get Position And Scale"),retSessionID.c_str(),sMethod.c_str());
			
			if(bHasPos)
			{
				if(!retContentBody.empty())
					retContentBody = retContentBody + "\n";
				retContentBody = retContentBody + "BcastPos:" + sPosition;
			}

			if(bHasScale)
			{
				if(!retContentBody.empty())
					retContentBody = retContentBody + "\n";
				retContentBody = retContentBody + "Scale:" + sScale;
			}
		}
		
		//////////////////////////////////////////////////////////////////////////
		

		// Before response OK
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before Response OK."),retSessionID.c_str(),sMethod.c_str());
		pResponse->printf_preheader(RESPONSE_OK);
		pResponse->setHeader(HEADER_SESSION,retSessionID.c_str());
		pResponse->setHeader(HEADER_MTHDCODE,sMethod.c_str());
		pResponse->printf_postheader(retContentBody.c_str());
		pResponse->post();

		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("************************************************************"
			SEPARATOR_LINE "	%s"
			SEPARATOR_LINE "	%s: %s"
			SEPARATOR_LINE "	%s: %s"
			SEPARATOR_LINE "	BcastPos: %s"
			SEPARATOR_LINE "	Scale: %s"
			SEPARATOR_LINE "	************************************************************")
			,RESPONSE_OK,HEADER_MTHDCODE,sMethod.c_str()
			,HEADER_SESSION,retSessionID.c_str()
			,sPosition.c_str(),sScale.c_str());
	}
	TIANSHANICE_INVALID_PARAMETER_HANDLE
	TIANSHANICE_INVALID_STATE_OF_ART_HANDLE
	TIANSHANICE_SERVER_ERROR_HANDLE
	TIANSHANICE_BASE_EXCEPTION_HANDLE
	ICE_PROXY_PARSE_EXCEPTION_HANDLE
	ICE_EXCEPTION_HANDLE
	ALL_EXCEPTION_HANDLE
	return true;
}

bool CSsm_PauseTV_s1::HdlSetupReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID,::std::string &retStreamString,::std::string &retPurchaseString)
{
	myGlog(ZQ::common::Log.L_DEBUG,PSLOG("RTSP REQUEST: SETUP"));
	::std::string retSessionHead;
	IServerResponse *pResponse = pReq->getResponse();	
	::std::string sMethod = "Setup";
	char gBuff[MY_BUFFER_SIZE];
	unsigned __int16 gBuffLen = MY_BUFFER_SIZE-1;
	const char* pSeq = pReq->getHeader(HEADER_SEQ, gBuff, &gBuffLen);
	if (pSeq != NULL && strlen(pSeq))
	{
		myGlog(ZQ::common::Log::L_DEBUG, PSLOG("CSeq: %s"), pSeq);
	}
	try{
		::TianShanIce::Properties cltProperties;
 		::TianShanIce::Properties siteProperties;

		//////////////////////////////////////////////////////////////////////////
		//TODO: create client session

		pReq->getUri(gBuff,gBuffLen);
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Before Create ClientSession."));
		IClientSession *pClientSession = pSite->createClientSession(NULL,gBuff);
		if(pClientSession==NULL)
		{
			myGlog(ZQ::common::Log.L_ERROR,PSLOG("Fail Create ClientSession"));
			pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR); 
			pResponse->setHeader(HEADER_MTHDCODE, sMethod.c_str());
			pResponse->setHeader(HEADER_SC_NOTICE,"Fail Create ClientSession");
			pResponse->post();
			return false;
		}
		retSessionID = pClientSession->getSessionID();
		char tempSess[MAX_PATH];
		strcpy(tempSess,retSessionID.c_str());
		pReq->setHeader(HEADER_SESSION,tempSess);
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success create clientSession %s"),retSessionID.c_str());
		//////////////////////////////////////////////////////////////////////////
		

		//////////////////////////////////////////////////////////////////////////
		//TODO: analyze url string

		::std::string chanid,sPath,sSite;
		std::string strTemp="rtsp://";
		strTemp+=gBuff;
		URLStr url(strTemp.c_str(),true);
		chanid = url.getVar("channelId");
		sSite = url.getHost();
		sPath = url.getPath();
		//////////////////////////////////////////////////////////////////////////
		

		//////////////////////////////////////////////////////////////////////////
		//TODO: add some values to map cltProperties
		
		cltProperties[CHANNEL_ID] = chanid;
		cltProperties[URL_SITE] = sSite;
		cltProperties[URL_PATH] = sPath;
		cltProperties[CLIENT_SESSION_ID] = retSessionID;

		const char *pTemp = NULL;
		pTemp = pReq->getHeader(CSEQ,gBuff,&gBuffLen);
		if(pTemp&&strlen(pTemp)>0)
		{
			cltProperties[CSEQ]=pTemp;
		}

		pTemp = NULL;
		pTemp = pReq->getHeader(TRANSPORT,gBuff,&gBuffLen);
		if(pTemp&&strlen(pTemp)>0)
		{
			cltProperties[TRANSPORT]=pTemp;
		}

		pTemp = NULL;
		pTemp = pReq->getHeader(USER_AGENT,gBuff,&gBuffLen);
		if(pTemp&&strlen(pTemp)>0)
		{
			cltProperties[USER_AGENT]=pTemp;
		}

		pTemp = NULL;
		pTemp = pReq->getHeader(SEACHANGE_VERSION,gBuff,&gBuffLen);
		if(pTemp&&strlen(pTemp)>0)
		{
			cltProperties[SEACHANGE_VERSION]=pTemp;
		}

		pTemp = NULL;
		pTemp = pReq->getHeader(SEACHANGE_MAYNOTIFY,gBuff,&gBuffLen);
		if(pTemp&&strlen(pTemp)>0)
		{
			cltProperties[SEACHANGE_MAYNOTIFY]=pTemp;
		}

		pTemp = NULL;
		pTemp = pReq->getHeader(SEACHANGE_SERVER_DATA,gBuff,&gBuffLen);
		if(pTemp&&strlen(pTemp)>0)
		{
			cltProperties[SEACHANGE_SERVER_DATA]=pTemp;
		}

		pTemp = NULL;
		pTemp = pReq->getHeader(SEACHANGE_TRANSPORT,gBuff,&gBuffLen);
		if(pTemp&&strlen(pTemp)>0)
		{
			cltProperties[SEACHANGE_TRANSPORT]=pTemp;
		}

		pTemp = NULL;
		pTemp = pReq->getHeader(SEACHANGE_NOTICE,gBuff,&gBuffLen);
		if(pTemp&&strlen(pTemp)>0)
		{
			cltProperties[SEACHANGE_NOTICE]=pTemp;
		}

		//////////////////////////////////////////////////////////////////////////
		

		::std::string _ipAddress,_port;
		
		//////////////////////////////////////////////////////////////////////////
		//TODO:

		bool _bQam = false;
		if(strstr(cltProperties[TRANSPORT].c_str(),"DVBC") == NULL)
		{
			_bQam = false;
			pReq->getTransportParam(KEY_TRANS_DEST,gBuff,&gBuffLen);
			_ipAddress = gBuff;
			pReq->getTransportParam(KEY_TRANS_CPORTA,gBuff,&gBuffLen);
			_port = gBuff;
			if(_ipAddress.size() == 0)
			{
				pReq->getHeader(TRANSPORT,gBuff,&gBuffLen);
				if(strstr(gBuff,"unicast") != NULL)
				{
					char addr[64];
					IClientRequest::RemoteInfo info;
					info.size = sizeof(IClientRequest::RemoteInfo);
					info.addrlen = sizeof(addr);
					info.ipaddr = addr;
					if(pReq->getRemoteInfo(info))
					{
						_ipAddress=info.ipaddr;
						char sPort[MAX_PATH];
						sprintf(sPort,"%d",info.port);
						_port = sPort;
					}
				}
			}
			cltProperties[CLIENT_IP] = _ipAddress;
			cltProperties[CLIENT_PORT] = _port;
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)IP: %s; Port: %s"),retSessionID.c_str(),sMethod.c_str(),_ipAddress.c_str(),_port.c_str());
		}
		else
		{
			_bQam = true;
		}
		//////////////////////////////////////////////////////////////////////////
		

		//////////////////////////////////////////////////////////////////////////
		//TODO: analyze field Seachange-Server-Data

		char vChar = ';';
		::std::vector<::std::string> strArray;
		SplitStringByChar(cltProperties[SEACHANGE_SERVER_DATA],vChar,strArray);
		::std::vector<::std::string>::const_iterator it;
		for(it = strArray.begin();it != strArray.end();it++)
		{
			::std::string strItem,strKey,strValue;
			strItem = *it;
			GetStringRightAtChar(strItem,'=',strValue);
			GetStringLeftAtChar(strItem,'=',strKey);
			cltProperties[strKey] = strValue;
		}
		//////////////////////////////////////////////////////////////////////////

		
		//////////////////////////////////////////////////////////////////////////
		//DO: show values from client request

		::std::string showValue;
		showValue = "Get values from Client Request";
		::TianShanIce::Properties::const_iterator itProty;
		for(itProty = cltProperties.begin();itProty != cltProperties.end();itProty++)
		{
			::std::string tempString = itProty->first + "=" + itProty->second;
			showValue = showValue + SEPARATOR_LINE + "		" + tempString;
		}
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("%s"),showValue.c_str());
		//////////////////////////////////////////////////////////////////////////
		

		//////////////////////////////////////////////////////////////////////////
		//TODO: create purchase

		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before Create Purchase."),retSessionID.c_str(),sMethod.c_str());
 		::ChannelOnDemand::ChannelPurchasePrx objPrx = codApp->createPurchaseByCR(cltProperties,siteProperties);
		::ChannelOnDemand::ChannelPurchaseExPrx purPrx = ::ChannelOnDemand::ChannelPurchaseExPrx::checkedCast(objPrx);
	    retPurchaseString = ic->proxyToString(purPrx);
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success to Create Purchase \"%s\""
			SEPARATOR_LINE "	Before Create Stream")
			,retSessionID.c_str(),sMethod.c_str(),retPurchaseString.c_str());
		//////////////////////////////////////////////////////////////////////////
		
		
		//////////////////////////////////////////////////////////////////////////
		//TODO: create stream

		int bandWidth = purPrx->getMaxBitrate();
		bandWidth = bandWidth/1024;
		::TianShanIce::SRM::ResourceMap rsMap;
		::TianShanIce::SRM::ResourceType rsType;
		::TianShanIce::SRM::Resource rs;
		rsType = ::TianShanIce::SRM::rtServiceGroup;
		::TianShanIce::Variant vGroup,vBandwidth;
		vGroup.type = ::TianShanIce::vtInts;
		vBandwidth.type = ::TianShanIce::vtInts;
		vGroup.ints.push_back(atoi(cltProperties["node-group-id"].c_str()));
		
		rs.resourceData["servicegroup"] = vGroup;
		rsMap[rsType] = rs;

		rsType=TianShanIce::SRM::rtTsDownstreamBandwidth;
		vBandwidth.ints.push_back(bandWidth);
		rs.resourceData["bandwidth"] = vBandwidth;
		rsMap[rsType] = rs;
		TianShanIce::Streamer::StreamPrx stmPrx = stmservice->createStreamByResource(rsMap);
		retStreamString = ic->proxyToString(stmPrx);		
		TianShanIce::Streamer::PlaylistExPrx playlistPrx = TianShanIce::Streamer::PlaylistExPrx::checkedCast(stmPrx);
		if(!playlistPrx)
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Can't Change Stream to Playlist."),retSessionID.c_str(),sMethod.c_str());
			pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
			pResponse->setHeader(HEADER_MTHDCODE, "Setup");
			pResponse->setHeader(HEADER_SC_NOTICE,"Can't Change Stream to Playlist.");
			pResponse->setHeader(HEADER_SESSION,retSessionID.c_str());
			pResponse->post();
			return false;
		}
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success to Create Stream \"%s\""
			SEPARATOR_LINE "	(%s:%s)Before Render Purchase")
			,retSessionID.c_str(),sMethod.c_str(),retStreamString.c_str());
				
		/// render the stream on purchase.
		purPrx->render(stmPrx,NULL);
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Render Purchase"),retSessionID.c_str(),sMethod.c_str());

		TianShanIce::ValueMap varMap;
		Ice::Int intPort;
		/// set the destination of stream.
		if(!_bQam)
		{
			intPort = atoi(_port.c_str());
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before Set Stream's Destination"),retSessionID.c_str(),sMethod.c_str());
			playlistPrx->setDestination(_ipAddress,intPort);
			playlistPrx->setDestMac("11:22:33:44:55:66");
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Set Stream's Destination(%s:%d)"),retSessionID.c_str(),sMethod.c_str(),_ipAddress.c_str(),intPort);
		}
		else
		{
			unsigned long Mask=TianShanIce::Streamer::infoDVBCRESOURCE;
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)%s"),retSessionID.c_str(),sMethod.c_str(),BEFORE_GET_STREAM_INFO);
			bool bRet = playlistPrx->getInfo(Mask,varMap);
			if(!bRet)
			{
				myGlog(ZQ::common::Log.L_ERROR,PSLOG("(%s:%s)%s"),retSessionID.c_str(),sMethod.c_str(),CANNOT_GET_STREAM_INFO);
				sprintf(gBuff,"%s",CANNOT_GET_STREAM_INFO);
				ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());
				return false;
			}
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)%s"),retSessionID.c_str(),sMethod.c_str(),SUCCESS_GET_STREAM_INFO);
			_ipAddress = varMap["DestIP"].strs[0];
			intPort = varMap["DestPort"].ints[0];
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before Set Stream's Destination"),retSessionID.c_str(),sMethod.c_str());
			playlistPrx->setDestination(_ipAddress,intPort);
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Set Stream's Destination(%s:%d)"),retSessionID.c_str(),sMethod.c_str(),_ipAddress.c_str(),intPort);
		}
				
		/// Build maps which can associate the client session to corresponding stream and purchase.
		CLIENT_INFO clientInfo;	
		clientInfo.channelId = url.getVar("channelId");
		clientInfo.path = sPath;
		clientInfo.site = sSite;
		clientInfo.purString = retPurchaseString;
		clientInfo.stmString = retStreamString;
		
		{
			IceUtil::RecMutex::Lock lock(clientInfoMapMutex);
			clientInfoMap[retSessionID] = clientInfo;
		}

		{
			IceUtil::RecMutex::Lock lock(streamToSessionMapMutex);
			streamToSessionMap[retStreamString] = retSessionID;
		}

		UpdateLastAccessTime(retSessionID,sMethod.c_str());

		/// tell client that the setup request has been processed successfully.
		pResponse->printf_preheader(RESPONSE_OK);	
	    pResponse->setHeader(HEADER_MTHDCODE, sMethod.c_str());
		pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
		retSessionHead = retSessionID + ";timeout=" + configMap["SESSION_TIME_OUT"];
		pResponse->setHeader(HEADER_SESSION,retSessionHead.c_str());
		
		::std::string sTransport;
		pReq->getHeader(HEADER_TRANSPORT,gBuff,&gBuffLen);
		if(strstr(gBuff,"destination")==NULL)
		{
			strcat(gBuff,";destination=");
			strcat(gBuff,_ipAddress.c_str());
		}
		pResponse->setHeader(HEADER_TRANSPORT,gBuff);
		sTransport = gBuff;

		::std::string sSeaTransport;
		if(_bQam)
		{
			int ProgramNumber = varMap["ProgramNumber"].ints[0];
			int frequency = varMap["Frequency"].ints[0];
			int qamMode = varMap["QamMode"].ints[0];
			sprintf(gBuff,"program-number=%d;frequency=%d;qam-mode=%d",ProgramNumber,frequency,qamMode);
			pResponse->setHeader(HEADER_SC_TRANSPORT,gBuff);
			sSeaTransport = gBuff;
		}
		pResponse->post();

		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("************************************************************"
			SEPARATOR_LINE "	%s"
			SEPARATOR_LINE "	%s: %s"
			SEPARATOR_LINE "	%s: %s"
			SEPARATOR_LINE "	%s: %s"
			SEPARATOR_LINE "	%s: %s"
			SEPARATOR_LINE "	************************************************************")
			,RESPONSE_OK,HEADER_MTHDCODE,sMethod.c_str(),HEADER_SESSION,retSessionHead.c_str()
			,HEADER_TRANSPORT,sTransport.c_str(),HEADER_SC_TRANSPORT,sSeaTransport.c_str());
	}
	TIANSHANICE_INVALID_PARAMETER_HANDLE
	TIANSHANICE_SERVER_ERROR_HANDLE
	TIANSHANICE_INVALID_STATE_OF_ART_HANDLE
	TIANSHANICE_BASE_EXCEPTION_HANDLE
	ICE_EXCEPTION_HANDLE
	ALL_EXCEPTION_HANDLE

	return true;
}

bool CSsm_PauseTV_s1::HdlPlayReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID)
{
	IServerResponse *pResponse = pReq->getResponse();
	::std::string sMethod = "Play";
	char gBuff[MY_BUFFER_SIZE];
	unsigned __int16 gBuffLen = MY_BUFFER_SIZE-1;
	const char* pSeq = pReq->getHeader(HEADER_SEQ, gBuff, &gBuffLen);
	if (pSeq != NULL && strlen(pSeq))
	{
		myGlog(ZQ::common::Log::L_DEBUG, PSLOG("CSeq: %s"), pSeq);
	}
    try
	{
        ::std::string	strStrm;
		::std::string	strPur;
		retSessionID = pReq->getClientSessionId();
		
		// Update last access time
		UpdateLastAccessTime(retSessionID,sMethod);
		
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("RTSP REQUEST: PLAY"
			SEPARATOR_LINE "	SESSION_ID: %s"
			SEPARATOR_LINE "	%s")
			,retSessionID.c_str(),BEFORE_FIND_SESSION);
		
		{
			IceUtil::RecMutex::Lock lock(clientInfoMapMutex);
			SESSION_TO_CLIENT_INFO_MAP::const_iterator it = clientInfoMap.find(retSessionID);
			if(it == clientInfoMap.end())
			{
				myGlog(ZQ::common::Log.L_ERROR,PSLOG("(%s:%s)%s"),retSessionID.c_str(),sMethod.c_str(),CANNOT_FIND_SESSION);
				sprintf(gBuff,"%s",CANNOT_FIND_SESSION);
				ErrorResponse(pResponse,RESPONSE_SESSION_NOTFOUND,sMethod.c_str(),gBuff,retSessionID.c_str());
				return false;
			}
			strStrm = ((CLIENT_INFO)(it->second)).stmString;
			strPur = ((CLIENT_INFO)(it->second)).purString;
		}

		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)%s"
			SEPARATOR_LINE "	Before Find Stream(%s)")
			,retSessionID.c_str(),sMethod.c_str(),SUCCESS_FIND_SESSION,strStrm.c_str());

		//////////////////////////////////////////////////////////////////////////
		//TODO: find the proxies on server
		TianShanIce::Streamer::PlaylistPrx playlistPrx = NULL;
		::ChannelOnDemand::ChannelPurchaseExPrx purchasePrx = NULL;
		try
		{
			// Get proxy of stream
			::Ice::ObjectPrx basePL = ic->stringToProxy(strStrm);
			playlistPrx = TianShanIce::Streamer::PlaylistPrx::checkedCast(basePL);
			if(!playlistPrx)
			{
				myGlog(ZQ::common::Log.L_ERROR,PSLOG("(%s:%s)Can't Find Stream"),retSessionID.c_str(),sMethod.c_str());
				sprintf(gBuff,"Can't find stream");
				ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());
				return false;
			}
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Find The Stream"
				SEPARATOR_LINE "	Before Find Purchase(%s)")
				,retSessionID.c_str(),sMethod.c_str(),strPur.c_str());

			// Get proxy of purchase
			::Ice::ObjectPrx basePur = ic->stringToProxy(strPur);
			purchasePrx = ::ChannelOnDemand::ChannelPurchaseExPrx::checkedCast(basePur);
			if(!purchasePrx)
			{
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Can't Find Purchase"),retSessionID.c_str(),sMethod.c_str());
				sprintf(gBuff,"Can't find purchase");
				ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());
				return false;
			}
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Find the Purchase"),retSessionID.c_str(),sMethod.c_str());
		}
		TIANSHANICE_BASE_EXCEPTION_HANDLE
		ICE_EXCEPTION_HANDLE
		ALL_EXCEPTION_HANDLE
		//////////////////////////////////////////////////////////////////////////
		
			
		//////////////////////////////////////////////////////////////////////////
		//TODO:seek according to Range field
        uint16 bufLen = MY_BUFFER_SIZE-1;
		const char *pTemp = NULL;
		pTemp = pReq->getHeader("Range",gBuff,&gBuffLen);
		::std::string retRange;
		if(pTemp&&strlen(pTemp)>0)
		{
			/// have the range field.
			::std::string retTime;
			::std::string strTime;

			if(stricmp(pTemp,"clock=") != 0)
			{
				if(stricmp(pTemp,"now") == 0)
				{
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Range: now, need to seek"
						SEPARATOR_LINE "	Before seek to %s")
						,retSessionID.c_str(),sMethod.c_str(),pTemp);
					strTime = "now";
				}
				else
				{
					GetStringRightAtChar(pTemp,'=',strTime);
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Range: %s, need to seek"
						SEPARATOR_LINE "	Before seek to %s")
						,retSessionID.c_str(),sMethod.c_str(),pTemp,pTemp);
				}

				try
				{
					retTime = purchasePrx->seekToPosition(strTime);
					ChangeToReturn(retTime);
					retRange = retTime;//如果Seek的话，retRange.empty()为false;
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success seek to \"%s\""),retSessionID.c_str(),sMethod.c_str(),retTime.c_str());
				}
				TIANSHANICE_SERVER_ERROR_HANDLE
				TIANSHANICE_INVALID_STATE_OF_ART_HANDLE
				TIANSHANICE_INVALID_PARAMETER_HANDLE
				TIANSHANICE_BASE_EXCEPTION_HANDLE
				ICE_EXCEPTION_HANDLE
				ALL_EXCEPTION_HANDLE
			}
			else
			{
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Range: clock=, need not to seek"),retSessionID.c_str(),sMethod.c_str());
			}
		}
		else
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Don't have Range field, need not to seek"),retSessionID.c_str(),sMethod.c_str());
		}
		//////////////////////////////////////////////////////////////////////////
		
		// if(retRange.empty())表示没有Seek，在返回给客户端Range是要调用purchase->getCurrentPosition();

		//////////////////////////////////////////////////////////////////////////
		//TODO: Set speed

		::Ice::Float speed = 0;
		::std::string sSpeed = "";
		pTemp = NULL;
		pTemp = pReq->getHeader("Scale",gBuff,&gBuffLen);
		if(pTemp != NULL && strlen(gBuff) > 0)
		{
			// have scale field, set the speed
			speed = atof(gBuff);
			sSpeed = gBuff;

			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Speed Wanted: %f"
				SEPARATOR_LINE "	Before Get Stream State")
				,retSessionID.c_str(),sMethod.c_str(),speed);
			
			// Get stream state
			TianShanIce::Streamer::StreamState	strmState;
			try
			{
				strmState=playlistPrx->getCurrentState();
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Get Stream State(STATE:%d)"),retSessionID.c_str(),sMethod.c_str(),strmState);
			}
			TIANSHANICE_BASE_EXCEPTION_HANDLE
			ICE_EXCEPTION_HANDLE
			ALL_EXCEPTION_HANDLE
			
			// if the stream state is stsStreaming or stsPause, set the speed
			if((strmState==TianShanIce::Streamer::stsStreaming) || (strmState==TianShanIce::Streamer::stsPause))
			{
				try
				{
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before Set Speed to %f"),retSessionID.c_str(),sMethod.c_str(),speed);
					playlistPrx->setSpeed(speed);
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Set Speed"),retSessionID.c_str(),sMethod.c_str());
				}
				catch(::TianShanIce::BaseException& ex)
				{
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Catch An %s:%s When Set Speed"),retSessionID.c_str(),sMethod.c_str(),ex.ice_name().c_str(),ex.message.c_str());
					::Sleep(50);
					
					//TODO: Get current speed, and if the speed is not 0, it indicates that the stream is streaming
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before Get Position And Scale"),retSessionID.c_str(),sMethod.c_str());
					purchasePrx->getCurPosAndScale(retRange,sSpeed);
					ChangeToReturn(retRange);					
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Get Position And Scale"
						SEPARATOR_LINE "	Pos: %s,Scale: %s")
						,retSessionID.c_str(),sMethod.c_str()
						,retRange.c_str(),sSpeed.c_str());

					float lScale = atof(sSpeed.c_str());
					if(lScale != 0)
					{
						pResponse->printf_preheader(RESPONSE_OK);
						pResponse->setHeader(HEADER_MTHDCODE, sMethod.c_str());
						pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
						pResponse->setHeader(HEADER_RANGE, retRange.c_str());
						// if current speed not equal to speed want
						if(atof(sSpeed.c_str()) != speed)
						{
							sprintf(gBuff,"%s:%s",ex.ice_name().c_str(),ex.message.c_str());
							pResponse->setHeader(HEADER_SC_NOTICE, gBuff);
 						}
						pResponse->setHeader(HEADER_SCALE,sSpeed.c_str());
						pResponse->post();
						
						// response OK log
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("************************************************************"
							SEPARATOR_LINE "	%s"
							SEPARATOR_LINE "	%s: Play"
							SEPARATOR_LINE "	%s: %s"
							SEPARATOR_LINE "	%s: NPT"
							SEPARATOR_LINE "	%s: %s"
							SEPARATOR_LINE "	%s: %s"
							SEPARATOR_LINE "	************************************************************")
							,RESPONSE_OK,HEADER_MTHDCODE,HEADER_SCALE,sSpeed.c_str(),HEADER_ACCEPTRANGE
							,HEADER_SC_NOTICE,gBuff,HEADER_RANGE,retRange.c_str());

						return true;
					}
					else
					{
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Fail To Set Speed,Speed=0"),retSessionID.c_str(),sMethod.c_str());
						sprintf(gBuff,"%s:%s",ex.ice_name().c_str(),ex.message.c_str());
						ErrorResponse(pResponse,RESPONSE_BAD_REQUEST,sMethod.c_str(),gBuff,retSessionID.c_str());
						return false;
					}
				}
				
				// if the stream state is stsStreaming, return OK without play
				if(strmState==TianShanIce::Streamer::stsStreaming)
				{
					// response OK to client
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Prepare Response OK"),retSessionID.c_str(),sMethod.c_str());
					
					// Get current play position and scale
					::Sleep(50);
					purchasePrx->getCurPosAndScale(retRange,sSpeed);
					ChangeToReturn(retRange);
					
					pResponse->printf_preheader(RESPONSE_OK);
					pResponse->setHeader(HEADER_MTHDCODE, sMethod.c_str());
					pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
					pResponse->setHeader(HEADER_RANGE, retRange.c_str());
					pResponse->setHeader(HEADER_SCALE, sSpeed.c_str());
					pResponse->post();
					
					// response OK log
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("************************************************************"
						SEPARATOR_LINE "	%s"
						SEPARATOR_LINE "	%s: Play"
						SEPARATOR_LINE "	%s: %s"
						SEPARATOR_LINE "	%s: NPT"
						SEPARATOR_LINE "	%s: %s"
						SEPARATOR_LINE "	************************************************************")
						,RESPONSE_OK,HEADER_MTHDCODE,HEADER_SCALE,sSpeed.c_str(),HEADER_ACCEPTRANGE
						,HEADER_RANGE,retRange.c_str());
					
					return true;
				}
			}
			else
			{
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Stream State is Neither \"Streaming\" nor \"Pause\", Needn't To Set Speed"),retSessionID.c_str(),sMethod.c_str());
			}
		}
		else
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Don't Have Scale Field, Needn't To Set Speed"),retSessionID.c_str(),sMethod.c_str());
		}

		/// play the stream.
 		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before Play The Stream(%s)"),retSessionID.c_str(),sMethod.c_str(),strStrm.c_str());
		if(playlistPrx->play())
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Play The Stream"
				SEPARATOR_LINE "	(%s:%s)Prepare Response OK")
				,retSessionID.c_str(),sMethod.c_str()
				,retSessionID.c_str(),sMethod.c_str());

			//Get current position and scale
			::Sleep(50);
			purchasePrx->getCurPosAndScale(retRange,sSpeed);
			ChangeToReturn(retRange);

			pResponse->printf_preheader(RESPONSE_OK);
			pResponse->setHeader(HEADER_MTHDCODE, sMethod.c_str());
			pResponse->setHeader(HEADER_ACCEPTRANGE, "NPT");
			pResponse->setHeader(HEADER_RANGE, retRange.c_str());
			pResponse->setHeader(HEADER_SCALE, sSpeed.c_str());
			pResponse->post();

			// response OK log
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("************************************************************"
				SEPARATOR_LINE "	%s"
				SEPARATOR_LINE "	%s: Play"
				SEPARATOR_LINE "	%s: %s"
				SEPARATOR_LINE "	%s: NPT"
				SEPARATOR_LINE "	%s: %s"
				SEPARATOR_LINE "	************************************************************")
				,RESPONSE_OK,HEADER_MTHDCODE,HEADER_SCALE,sSpeed.c_str(),HEADER_ACCEPTRANGE
				,HEADER_RANGE,retRange.c_str());

			return true;
		}
		else
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Can't Play The Stream"),retSessionID.c_str(),sMethod.c_str());
			sprintf(gBuff,"Can't Play The Stream");
			ErrorResponse(pResponse,RESPONSE_BAD_REQUEST,sMethod.c_str(),gBuff,retSessionID.c_str());
			return false;
		}
	}
	TIANSHANICE_BASE_EXCEPTION_HANDLE
	ICE_EXCEPTION_HANDLE
	ALL_EXCEPTION_HANDLE
}

bool CSsm_PauseTV_s1::HdlPauseReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID)
{
	IServerResponse *pResponse = pReq->getResponse();
	::std::string sMethod = "Pause";
	char gBuff[MY_BUFFER_SIZE];
	unsigned __int16 gBuffLen = MY_BUFFER_SIZE-1;
	const char* pSeq = pReq->getHeader(HEADER_SEQ, gBuff, &gBuffLen);
	if (pSeq != NULL && strlen(pSeq))
	{
		myGlog(ZQ::common::Log::L_DEBUG, PSLOG("CSeq: %s"), pSeq);
	}
    try
	{
        ::std::string strStm,strPur;
		retSessionID = pReq->getClientSessionId();
		UpdateLastAccessTime(retSessionID,sMethod);
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("RTSP REQUEST: PAUSE"
			SEPARATOR_LINE "	SESSION_ID: %s"
			SEPARATOR_LINE "	%s")
			,retSessionID.c_str(),BEFORE_FIND_SESSION);

		{
			IceUtil::RecMutex::Lock lock(clientInfoMapMutex);
			SESSION_TO_CLIENT_INFO_MAP::const_iterator it = clientInfoMap.find(retSessionID);
			if(it == clientInfoMap.end())
			{
				myGlog(ZQ::common::Log.L_ERROR,PSLOG("(%s:%s)%s"),retSessionID.c_str(),sMethod.c_str(),CANNOT_FIND_SESSION);
				sprintf(gBuff,"%s",CANNOT_FIND_SESSION);
				ErrorResponse(pResponse,RESPONSE_SESSION_NOTFOUND,sMethod.c_str(),gBuff,retSessionID.c_str());
				return false;
			}
			strPur = ((CLIENT_INFO)(it->second)).purString;
			strStm = ((CLIENT_INFO)(it->second)).stmString;
		}

		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)%s"
			SEPARATOR_LINE "	Before Find Stream(%s)")
			,retSessionID.c_str(),sMethod.c_str(),SUCCESS_FIND_SESSION,strStm.c_str());
		
		//////////////////////////////////////////////////////////////////////////
		//DO: Find purchase and stream on server

		::Ice::ObjectPrx prxObjStream = ic->stringToProxy(strStm);
		TianShanIce::Streamer::StreamPrx prxStream = 
			TianShanIce::Streamer::StreamPrx::checkedCast(prxObjStream);
		
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Find Stream"
			SEPARATOR_LINE "	Before Find Purchase(%s)")
			,retSessionID.c_str(),sMethod.c_str(),strPur.c_str());

		::Ice::ObjectPrx prxObjPurchase = ic->stringToProxy(strPur);
		ChannelOnDemand::ChannelPurchaseExPrx prxPurchase = 
			ChannelOnDemand::ChannelPurchaseExPrx::checkedCast(prxObjPurchase);		
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Find Purchase"),retSessionID.c_str(),sMethod.c_str());
	    
		if(!prxStream || !prxPurchase)
		{
			myGlog(ZQ::common::Log.L_ERROR,PSLOG("(%s:%s)Can't Find Stream or Purchase"),retSessionID.c_str(),sMethod.c_str());
			sprintf(gBuff,"Can't Find Stream or Purchase");
			ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());
			return false;
		}
		
		TianShanIce::Streamer::StreamState	strmState;
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before Get Stream State"),retSessionID.c_str(),sMethod.c_str());
		strmState=prxStream->getCurrentState();
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Get Stream State STATE:%d"),retSessionID.c_str(),sMethod.c_str(),strmState);
		
		if(strmState != TianShanIce::Streamer::stsStreaming)
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Bad request! You Can Only Pause A Streaming Stream"),retSessionID.c_str(),sMethod.c_str());
			sprintf(gBuff,"You Can Only Pause A Streaming Stream");
			ErrorResponse(pResponse,RESPONSE_BAD_REQUEST,sMethod.c_str(),gBuff,retSessionID.c_str());
			return false;
		}
		
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Before Pause Stream."),retSessionID.c_str(),sMethod.c_str());
		
		//DO: Pause the stream
		if(prxStream->pause())
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Pause Stream"
				SEPARATOR_LINE "	Before Get Current Position")
				,retSessionID.c_str(),sMethod.c_str());

			::std::string curPos = prxPurchase->getCurrentPosition();
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Pause Position: %s"
				SEPARATOR_LINE "	Before Response OK")
				,retSessionID.c_str(),sMethod.c_str(),curPos.c_str());
			
			// Before response OK
			pResponse->printf_preheader(RESPONSE_OK);
			pResponse->setHeader(HEADER_MTHDCODE, sMethod.c_str());
			pResponse->setHeader(HEADER_SESSION, retSessionID.c_str());
			pResponse->post();

			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("************************************************************"
				SEPARATOR_LINE "	%s"
				SEPARATOR_LINE "	%s: Pause"
				SEPARATOR_LINE "	%s: %s"
				SEPARATOR_LINE "	Position: %s"
				SEPARATOR_LINE "	************************************************************")
				,RESPONSE_OK,HEADER_MTHDCODE,HEADER_SESSION,retSessionID.c_str(),curPos.c_str());
		}
		else
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Fail To Pause Stream"),retSessionID.c_str(),sMethod.c_str());
			sprintf(gBuff,"Fail To Pause Stream");
			ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());
			return false;
		}
	}
	TIANSHANICE_INVALID_PARAMETER_HANDLE
	TIANSHANICE_INVALID_STATE_OF_ART_HANDLE
	TIANSHANICE_SERVER_ERROR_HANDLE
	TIANSHANICE_BASE_EXCEPTION_HANDLE
	ICE_PROXY_PARSE_EXCEPTION_HANDLE
	ICE_EXCEPTION_HANDLE
	ALL_EXCEPTION_HANDLE
	return true;
}

bool CSsm_PauseTV_s1::HdlTeardownReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID)
{
	IServerResponse *pResponse = pReq->getResponse();
	::std::string sMethod = "Teardown";
	char gBuff[MY_BUFFER_SIZE];	
	unsigned __int16 gBuffLen = MY_BUFFER_SIZE-1;
	const char* pSeq = pReq->getHeader(HEADER_SEQ, gBuff, &gBuffLen);
	if (pSeq != NULL && strlen(pSeq))
	{
		myGlog(ZQ::common::Log::L_DEBUG, PSLOG("CSeq: %s"), pSeq);
	}
	try
	{
        ::std::string strPur,strStm;
		SESSION_TO_CLIENT_INFO_MAP::const_iterator it;
		retSessionID = pReq->getClientSessionId();
		
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("RTSP REQUEST: TEARDOWN"
			SEPARATOR_LINE "	SESSION_ID: %s"
			SEPARATOR_LINE "	%s")
			,retSessionID.c_str(),BEFORE_FIND_SESSION);

		{
			IceUtil::RecMutex::Lock lock(clientInfoMapMutex);
			it = clientInfoMap.find(retSessionID);
			if(it == clientInfoMap.end())
			{
				myGlog(ZQ::common::Log.L_ERROR,PSLOG("(%s:%s)%s"),retSessionID.c_str(),sMethod.c_str(),CANNOT_FIND_SESSION);
				sprintf(gBuff,"%s",CANNOT_FIND_SESSION);
				ErrorResponse(pResponse,RESPONSE_SESSION_NOTFOUND,sMethod.c_str(),gBuff,retSessionID.c_str());
				return false;
			}
			strPur = ((CLIENT_INFO)(it->second)).purString;
			strStm = ((CLIENT_INFO)(it->second)).stmString;
		}
		
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)%s"
			SEPARATOR_LINE "	Before Find Purchase(%s)")
			,retSessionID.c_str(),sMethod.c_str(),SUCCESS_FIND_SESSION,strPur.c_str());

		::Ice::ObjectPrx prxObjPur = ic->stringToProxy(strPur);		
		ChannelOnDemand::ChannelPurchaseExPrx prxPurchase = 
			ChannelOnDemand::ChannelPurchaseExPrx::checkedCast(prxObjPur);
		if(prxPurchase)
		{
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Find Purchase"
				SEPARATOR_LINE "	Before Destroy Purchase(%s)"
				SEPARATOR_LINE "	Reason: Client request teardown")
				,retSessionID.c_str(),sMethod.c_str(),strPur.c_str());

			// Destroy purchase
			prxPurchase->destroy();
			
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Destroy Purchase"),retSessionID.c_str(),sMethod.c_str());
						
			pResponse->printf_preheader(RESPONSE_OK);
			pResponse->setHeader(HEADER_MTHDCODE, sMethod.c_str());
			pResponse->setHeader(HEADER_SESSION,retSessionID.c_str());
			pResponse->post();

			//DO: Delete RTSP Session
			{
				pSite->destroyClientSession(retSessionID.c_str());
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Success Delete RTSP Connection and Session"),retSessionID.c_str(),sMethod.c_str());
			}
			
			//DO: Delete Session from ClientInfoMap
			{
				IceUtil::RecMutex::Lock lock(clientInfoMapMutex);
				CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::iterator itClient = clientInfoMap.find(retSessionID);
				if(itClient != clientInfoMap.end())
				{
					clientInfoMap.erase(itClient);
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Remove %s from ClientInfoMap"),retSessionID.c_str(),sMethod.c_str(),retSessionID.c_str());
				}
			}

			//DO: Delete Stream from StreamToSessionMap
			{
				IceUtil::RecMutex::Lock lock(streamToSessionMapMutex);
				CSsm_PauseTV_s1::STREAM_TO_SESSION_MAP::iterator itStm = streamToSessionMap.find(strStm);
				if(itStm != streamToSessionMap.end())
				{
					streamToSessionMap.erase(itStm);
					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("(%s:%s)Remove (%s) from StreamToSessionMap"),retSessionID.c_str(),sMethod.c_str(),strStm.c_str());
				}
			}
			
			myGlog(ZQ::common::Log.L_DEBUG,PSLOG("************************************************************"
				SEPARATOR_LINE "	%s"
				SEPARATOR_LINE "	%s: %s"
				SEPARATOR_LINE "	%s: %s"
				SEPARATOR_LINE "	************************************************************")
				,RESPONSE_OK,HEADER_MTHDCODE,sMethod.c_str(),HEADER_SESSION,retSessionID.c_str());
		}
		else
		{
			myGlog(ZQ::common::Log.L_ERROR,PSLOG("(%s:%s)Can't Find Purchase"),retSessionID.c_str(),sMethod.c_str());
			sprintf(gBuff,"Can't Find Purchase");
			ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());
			return false;
		}
	}
	TIANSHANICE_INVALID_PARAMETER_HANDLE
	TIANSHANICE_INVALID_STATE_OF_ART_HANDLE
	TIANSHANICE_SERVER_ERROR_HANDLE
	TIANSHANICE_BASE_EXCEPTION_HANDLE
	ICE_PROXY_PARSE_EXCEPTION_HANDLE
	ICE_EXCEPTION_HANDLE
	ALL_EXCEPTION_HANDLE
	return true;
}

RequestProcessResult CSsm_PauseTV_s1::FixupRequest(IStreamSmithSite* pSite
									  , IClientRequestWriter* pReq)
{
	return RequestProcessed;
}

RequestProcessResult CSsm_PauseTV_s1::ContentHandle(IStreamSmithSite* pSite
									   , IClientRequestWriter* pReq)
{
	RTSP_VerbCode verbcode;
	verbcode = pReq->getVerb();
	
	switch(verbcode)
	{
	case RTSP_MTHD_SETUP:
		{
			::std::string		retSessionID = "";
			::std::string		retStreamString,retPurchaseString;
			if(!HdlSetupReq(pSite,pReq,retSessionID,retStreamString,retPurchaseString))
			{
				DestroyPurchaseStream(retStreamString,retPurchaseString);
				pSite->destroyClientSession(retSessionID.c_str());
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success Delete RTSP Connection and Session"));
				return RequestError;
			}
			break;
		}
	case RTSP_MTHD_PLAY:
		{
			::std::string	retSessionID = "";
			if(!HdlPlayReq(pSite,pReq,retSessionID))
			{
				return RequestError;
			}
			break;
		}
	case RTSP_MTHD_TEARDOWN:
		{
			::std::string	retSessionID = "";
			if(!HdlTeardownReq(pSite,pReq,retSessionID))
			{
				return RequestError;
			}
			break;
		}
	case RTSP_MTHD_PAUSE:
		{
			::std::string	retSessionID = "";
			if(!HdlPauseReq(pSite,pReq,retSessionID))
			{
				return RequestError;
			}
			break;
		}
	case RTSP_MTHD_GET_PARAMETER:
		{
			::std::string	retSessionID = "";
			if(!HdlGetParameterReq(pSite,pReq,retSessionID))
			{
				return RequestError;
			}
			break;
		}
	default:
		{
			myGlog(ZQ::common::Log.L_ERROR,PSLOG("unknown RTSP request, the RequestError will be returned to RtspProxy."));
			return RequestUnrecognized;
		}
	}

	/// request has been processed but need further process at the same phase.
	return RequestProcessed;
}

RequestProcessResult CSsm_PauseTV_s1::FixupResponse(IStreamSmithSite* pSite
													   , IClientRequest* pReq)
{
	return RequestProcessed;
}

void CSsm_PauseTV_s1::ConnChodsvc()
{
	try
	{
		Ice::ObjectPrx baseApp;
		::std::string strAppEndPoints;

		CONFIGURATION_MAP::const_iterator it = configMap.find("CHANNELONDEMANDAPP_NAME");
		if(it != configMap.end())
			strAppEndPoints = strAppEndPoints + it->second + ":tcp -h ";
		else
			strAppEndPoints = strAppEndPoints + DEFAULT_CHANNELONDEMANDAPP_NAME + ":tcp -h ";
		
		it = configMap.find("CHANNELONDEMANDAPP_IP");
		if(it != configMap.end())
			strAppEndPoints = strAppEndPoints + it->second;
		else
			strAppEndPoints = strAppEndPoints + DEFAULT_CHANNELONDEMANDAPP_IP;
		
		it = configMap.find("CHANNELONDEMANDAPP_PORT");
		if(it != configMap.end())
			strAppEndPoints = strAppEndPoints + " -p " + it->second;
		else
			strAppEndPoints = strAppEndPoints + " -p " + DEFAULT_CHANNELONDEMANDAPP_PORT;

		myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Connecting Service Chodsvc.exe ..."));
		baseApp = ic->stringToProxy(strAppEndPoints);
		codApp = ::ChannelOnDemand::ChannelOnDemandAppPrx::checkedCast(baseApp);
		if(!codApp)
		{
			bConnChodsvcOK =false;
			myGlog(ZQ::common::Log.L_EMERG,PSLOG2("Can't Connect Service Chodsvc.exe"));
			return;
		}
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Success Connect to Chodsvc.exe"));
		bConnChodsvcOK = true;
		return;
	}
	catch(const Ice::ProxyParseException& ex)
	{
		bConnChodsvcOK =false;
		myGlog(ZQ::common::Log.L_EMERG,PSLOG2("Catch An %s When Connect Chodsvc.exe"),ex.ice_name().c_str());
		return;
	}
	catch(::Ice::Exception& ex)
	{
		bConnChodsvcOK =false;
		myGlog(ZQ::common::Log::L_EMERG,PSLOG2("Catch An %s When Connect Chodsvc.exe"), ex.ice_name().c_str());
		return;
	}
	catch(...)
	{
		bConnChodsvcOK =false;
		myGlog(ZQ::common::Log.L_EMERG,PSLOG2("Catch An Unknown Exception When Connect Chodsvc.exe"));
		return;
	}
}

void CSsm_PauseTV_s1::ConnStreamSmith()
{
	try
	{
		Ice::ObjectPrx baseStream;
		::std::string strStmEndPoints;
		
		CONFIGURATION_MAP::const_iterator it = configMap.find("CHANNELONDEMANDAPP_NAME");
		it = configMap.find("STREAMSERVICE_NAME");
		if(it != configMap.end())
			strStmEndPoints = strStmEndPoints + it->second + ":tcp -h ";
		else
			strStmEndPoints = strStmEndPoints + DEFAULT_STREAMSERVICE_NAME + ":tcp -h ";

		it = configMap.find("STREAMSERVICE_IP");
		if(it != configMap.end())
			strStmEndPoints = strStmEndPoints + it->second;
		else
			strStmEndPoints = strStmEndPoints + DEFAULT_STREAMSERVICE_IP;

		it = configMap.find("STREAMSERVICE_PORT");
		if(it != configMap.end())
			strStmEndPoints = strStmEndPoints + " -p " + it->second;
		else
			strStmEndPoints = strStmEndPoints + " -p " + DEFAULT_STREAMSERVICE_PORT;

		myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Connecting Service StreamSmith.exe ..."));
		baseStream = ic->stringToProxy(strStmEndPoints);
		stmservice = ::TianShanIce::Streamer::StreamSmithAdminPrx::checkedCast(baseStream);
    	if(!stmservice)
		{
			bConnStreamSmithOK =false;
			myGlog(ZQ::common::Log.L_EMERG,PSLOG2("Can't Connecting Service StreamSmith.exe"));
			return;
		}
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Success Connecting Service StreamSmith.exe"));
		bConnStreamSmithOK = true;
	}
	catch(const Ice::ProxyParseException& ex)
	{
		bConnStreamSmithOK =false;
		myGlog(ZQ::common::Log.L_EMERG,PSLOG2("Catch an %s When Connect Service StreamSmith.exe"),ex.ice_name().c_str());
		return;
	}
	catch(::Ice::Exception& ex)
	{
		bConnStreamSmithOK =false;
		myGlog(ZQ::common::Log::L_EMERG,PSLOG2("Catch an %s When Connect Service StreamSmith.exe"), ex.ice_name().c_str());
		return;
	}
	catch(...)
	{
		bConnStreamSmithOK =false;
		myGlog(ZQ::common::Log.L_EMERG,PSLOG("Catch An Unknown Exception When Connect Service StreamSmith.exe"));
		return;
	}
}

void CSsm_PauseTV_s1::InitIce()
{
	try
	{
		int value=0;
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Initialize ICE Environment ..."));
		ic = ::Ice::initialize(value,NULL);
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Success Initialize ICE Environment"));
		iceInitialized =true;
	}
	catch(::Ice::Exception& ex)
	{
		iceInitialized =false;
		myGlog(ZQ::common::Log::L_EMERG,PSLOG2("Catch An %s When Initialize ICE Environment"),ex.ice_name().c_str());
		return;
	}
	catch(...)
	{
		iceInitialized =false;
		myGlog(ZQ::common::Log.L_EMERG,PSLOG("Catch An Unknown Exception When Initialize ICE Environment"));
		return;
	}
}

bool CSsm_PauseTV_s1::LoadConfig(::std::string &sPath)
{	
	try
	{
		ComInitializer init;
		XMLPrefDoc xDoc(init);
		
		if(!xDoc.open(sPath.c_str()))
		{
			return false;
		}
		
		auto_free<IPreference*>	pRoot=(IPreference *)xDoc.root();
		if(!pRoot)
			return false;

		/// Only retrieve the first configure item which has name of configure，
		/// and then retrieve all the pairs of key/value.
		/// 只查找第一个名字为configure的配置项，并遍历里面的键值对
		auto_free<IPreference*> pChild = pRoot->firstChild("configure");
		if(!pChild)
		{
			return false;
		}	
		char cArr[MY_BUFFER_SIZE];
		configMap.clear();

		/// 将键值对加入configMap中，若configString数组中没有该键名，则该键值对不能加入configMap。
		/// configString在构造函数中初始化，里面应该存放配置文件里的所有项目，否则该配置项无法生效。
		for(::std::vector<::std::string>::const_iterator it=configString.begin();it!=configString.end();it++)
		{
			if(pChild->has(it->c_str()))
			{
				configMap[it->c_str()] = pChild->get(it->c_str(),cArr);
			}
		}
		xDoc.close();
	}
	catch(...)
	{
		return false;
	}
	return true;
}

void CSsm_PauseTV_s1::ConnIceStorm()
{
	::std::string _strAdapterEndPoints,_strEventChannelEndPoints;

	/// construct endpoints string.
	try{
		_strAdapterEndPoints = "default -p ";
		CONFIGURATION_MAP::const_iterator it = configMap.find("LISTEN_EVENT_PORT");
		if(it == configMap.end())
			_strAdapterEndPoints = _strAdapterEndPoints + DEFAULT_ICESTORM_PORT;
		else
			_strAdapterEndPoints = _strAdapterEndPoints + it->second;
		
		_strEventChannelEndPoints = "tcp -h ";
		it = configMap.find("ICESTORM_IP");
		if(it != configMap.end())
			_strEventChannelEndPoints = _strEventChannelEndPoints + it->second;
		else
			_strEventChannelEndPoints = _strEventChannelEndPoints + DEFAULT_ICESTORM_IP;
		it = configMap.find("ICESTORM_PORT");
		_strEventChannelEndPoints = _strEventChannelEndPoints + " -p ";
		if(it != configMap.end())
			_strEventChannelEndPoints = _strEventChannelEndPoints + it->second;
		else
			_strEventChannelEndPoints = _strEventChannelEndPoints + DEFAULT_ICESTORM_PORT;

		myGlog(ZQ::common::Log::L_DEBUG,PSLOG2("Connecting IceStorm ..."));
		if(!_objectCreated)
		{
			_topicAdapter = ic->createObjectAdapterWithEndpoints("EventManager",_strAdapterEndPoints);
			_objectCreated=true;
		}
		_eventChannel = new TianShanIce::Events::EventChannelImpl(_topicAdapter,_strEventChannelEndPoints.c_str());
		StreamEventSinkPtr _stmEvent = new StreamEventSinkI(&ssm_PauseTV_s1_Log);
		PlaylistEventSinkPtr _plEvent = new PlaylistEventSinkI(&ssm_PauseTV_s1_Log);
		::TianShanIce::Properties qos;
		TianShanIce::Properties _protys;
		_eventChannel->sink(_stmEvent,qos);
		_eventChannel->sink(_plEvent,_protys);
		_topicAdapter->activate();
		bConnIceStormOK = true;
		myGlog(ZQ::common::Log::L_DEBUG,PSLOG("Success Connect IceStorm"));
	}
	catch(...)
	{
		myGlog(ZQ::common::Log.L_ERROR,PSLOG2("Catch An Exception When Connect IceStorm.exe"));
		bConnIceStormOK = false;
	}
}

void CSsm_PauseTV_s1::ShowConfiguration()
{
	CSsm_PauseTV_s1::CONFIGURATION_MAP::const_iterator it;
	for(it = CSsm_PauseTV_s1::configMap.begin();it != CSsm_PauseTV_s1::configMap.end();it++)
	{
		myGlog(ZQ::common::Log::L_DEBUG,PSLOG2("%s = %s"),it->first.c_str(),it->second.c_str());	
	}
}

void CSsm_PauseTV_s1::createCleanupSessionThrd()
{
	CSsm_PauseTV_s1::pCleanSessionThrd = new thrdCleanupSession(&ssm_PauseTV_s1_Log);
	CSsm_PauseTV_s1::pCleanSessionThrd->start();
}

void CSsm_PauseTV_s1::createConnServiceThrd()
{
	pConnServiceThrd = new thrdConnService(&ssm_PauseTV_s1_Log);
	pConnServiceThrd->start();
}

void ModuleInitEx(IStreamSmithSite* pSite, const char* cfgPath)
{
	if(!CSsm_PauseTV_s1::bModuleInit)
	{
		CSsm_PauseTV_s1::bModuleInit = true;
		CSsm_PauseTV_s1 plugin;
		CSsm_PauseTV_s1::globalSite = pSite;
		
		/// load configuration file.
		CSsm_PauseTV_s1::_configFilePath = cfgPath;
		bool bRet = CSsm_PauseTV_s1::LoadConfig(CSsm_PauseTV_s1::_configFilePath);
		if(!bRet)
		{
			return;
		}
		
		::std::string logFileName;
		//int logLevel;
		int logFileSize;
		CSsm_PauseTV_s1::CONFIGURATION_MAP::const_iterator it;
		
		it = CSsm_PauseTV_s1::configMap.find("LOGFILE_NAME");
		if(it != CSsm_PauseTV_s1::configMap.end())
			logFileName = it->second;
		else
			logFileName = DEFAULT_LOGFILE_NAME;
		
		it = CSsm_PauseTV_s1::configMap.find("LOGFILE_SIZE");
		if(it != CSsm_PauseTV_s1::configMap.end())
			logFileSize = atoi(it->second.c_str());
		else
			logFileSize = atoi(DEFAULT_LOGFILE_SIZE);
		
		try
		{
			CSsm_PauseTV_s1::ssm_PauseTV_s1_Log.open(logFileName.c_str(), \
				ZQ::common::Log::L_DEBUG, \
				ZQLOG_DEFAULT_FILENUM*2, \
				logFileSize, \
				ZQLOG_DEFAULT_BUFFSIZE, \
				ZQLOG_DEFAULT_FLUSHINTERVAL, \
				ZQLOG_DEFAULT_EVENTLOGLEVEL, \
				"ssm_PauseTV_s1");
		}
		catch(ZQ::common::FileLogException& ex)
		{
			myGlog(Log::L_EMERG, PSLOG("Catch an %s when create log file"), ex.getString());
		}
		
		myGlog(ZQ::common::Log::L_DEBUG,PSLOG("****************************************	BEGIN	************************************"));
		CSsm_PauseTV_s1::ShowConfiguration();
		
		// do create thread to complete services
		CSsm_PauseTV_s1::createConnServiceThrd();
		
		// do create thread cleanup session
		CSsm_PauseTV_s1::createCleanupSessionThrd();

	}

	/// register mothed.
	LPCTSTR handlerName = "PauseTV_s1";
	pSite->RegisterFixupRequest((SSMH_FixupRequest)CSsm_PauseTV_s1::FixupRequest);
	pSite->RegisterContentHandle(handlerName,(SSMH_ContentHandle)CSsm_PauseTV_s1::ContentHandle);
	pSite->RegisterFixupResponse((SSMH_FixupResponse)CSsm_PauseTV_s1::FixupResponse);
}

void ModuleUninit(IStreamSmithSite* pSite)
{
	CSsm_PauseTV_s1::bModuleInit = false;

	myGlog(ZQ::common::Log::L_DEBUG,PSLOG("****************************************	END		************************************"));
	
	CSsm_PauseTV_s1::DestroyAllPurchase();

	if(CSsm_PauseTV_s1::pConnServiceThrd != NULL && CSsm_PauseTV_s1::pConnServiceThrd->isRunning())
	{
		delete(CSsm_PauseTV_s1::pConnServiceThrd);
	}
	
	if(CSsm_PauseTV_s1::pCleanSessionThrd != NULL && CSsm_PauseTV_s1::pCleanSessionThrd->isRunning())
	{
		delete(CSsm_PauseTV_s1::pCleanSessionThrd);
	}

	if(CSsm_PauseTV_s1::ic)
	{
		try{
			CSsm_PauseTV_s1::ic->destroy();
		}
		catch(const std::string &)
		{
			///
		}
	}

	myGlog(ZQ::common::Log::L_DEBUG,PSLOG("****************************************	END		************************************"));
}

void ConvertToPath(char *vString)
{
	int len = strlen(vString);
	for(int i = len-1;i >= 0;i--)
	{
		if(vString[i] == '\\')
		{
			vString[i] = '\0';
			break;
		}
	}
}

// This is the constructor of a class that has been exported.
// see ssm_PauseTV_s1.h for the class definition
CSsm_PauseTV_s1::CSsm_PauseTV_s1()
{
	/// 添加配置项，XML文件中的配置项必须在此出现，否则无效。
	/// 也就是说，当在XML中添加新的可配项时，必须在此添加对应的项目。
	/// 如：在XML中添加新的键值对CLIENT_IP="127.0.0.1";
	///	则：必须在此添加configString.push_back("CLIENT_IP").LOGFILE_NAME
	configString.push_back("LOGFILE_NAME");
	configString.push_back("LOGFILE_SIZE");
	configString.push_back("CHANNELONDEMANDAPP_NAME");
	configString.push_back("CHANNELONDEMANDAPP_IP");
	configString.push_back("CHANNELONDEMANDAPP_PORT");
	configString.push_back("STREAMSERVICE_NAME");
	configString.push_back("STREAMSERVICE_IP");
	configString.push_back("STREAMSERVICE_PORT");
	configString.push_back("LISTEN_EVENT_PORT");
	configString.push_back("ICESTORM_IP");
	configString.push_back("ICESTORM_PORT");
	configString.push_back("RECONNECT_INTERNAL");
	configString.push_back("SESSION_TIME_OUT");
	globalSite = NULL;
	iceInitialized = false;
	bConnChodsvcOK = false;
	bConnStreamSmithOK = false;
	bConnIceStormOK = false;
	pConnServiceThrd = NULL;
	pCleanSessionThrd = NULL;
	
	/// obtain configuration file path by reading register table.
	/// 获得配置文件的路径
	HKEY hKey;
	TCHAR pluginConfigFilePath[MY_BUFFER_SIZE];
	unsigned long bufferLen=MY_BUFFER_SIZE;
	memset(pluginConfigFilePath,0,MY_BUFFER_SIZE);
	LONG result = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,PLUGIN_PATH, 0, KEY_QUERY_VALUE, &hKey);
	if (result == ERROR_SUCCESS)
	{
	    DWORD vType=REG_SZ;
		result = ::RegQueryValueEx (hKey, "pluginConfigFilePath", NULL, &vType,
			(LPBYTE)pluginConfigFilePath, &bufferLen);
		if(result == ERROR_SUCCESS)
			_configFilePath = pluginConfigFilePath;
	}
	::RegCloseKey(hKey);
	if(_configFilePath.size() == 0)
	{
		::GetModuleFileName(NULL,pluginConfigFilePath,MY_BUFFER_SIZE);
		ConvertToPath(pluginConfigFilePath);
		_configFilePath += pluginConfigFilePath;
		_configFilePath += "\\ssm_PauseTV_s1.xml";
	}
}

CSsm_PauseTV_s1::~CSsm_PauseTV_s1()
{
	/// 
}
