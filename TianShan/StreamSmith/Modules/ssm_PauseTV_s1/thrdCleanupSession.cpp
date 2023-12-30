// thrdCleanupSession.cpp: implementation of the thrdCleanupSession class.
//
//////////////////////////////////////////////////////////////////////

#include	"stdafx.h"
#include	"thrdCleanupSession.h"
#include	"ssm_PauseTV_s1.h"
#include	"ZQResource.h"
#pragma		warning(disable: 4018)

//ZQ::common::ScLog*		thrdCleanupSession::pLog = NULL;
ZQ::common::FileLog*		thrdCleanupSession::pLog = NULL;
#define	myGlog	(*thrdCleanupSession::pLog)
#define PSLOG(_X) "	" _X
#define PSLOG2(_X) "		" _X

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


thrdCleanupSession::thrdCleanupSession()
{

}

//thrdCleanupSession::thrdCleanupSession(ZQ::common::ScLog * log)
thrdCleanupSession::thrdCleanupSession(ZQ::common::FileLog * log)
{
	pLog = log;
}

thrdCleanupSession::~thrdCleanupSession()
{
	stop();
}

bool thrdCleanupSession::init(void)
{
	myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Cleanup Session Thread Starts"));

	_hExit = CreateEvent(NULL, TRUE, FALSE, NULL);

	return true;
}

void thrdCleanupSession::final(void)
{
	myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Cleanup Session Thread Ends"));
	delete(this);
}

void thrdCleanupSession::stop()
{
	waitHandle(2000);	

	if (_hExit)
	{
		CloseHandle(_hExit);
		_hExit = NULL;
	}
	
	CSsm_PauseTV_s1::pConnServiceThrd = NULL;
}

int thrdCleanupSession::run()
{
	long inteval = atol(DEFAULT_SESSION_TIME_OUT);
	CSsm_PauseTV_s1::CONFIGURATION_MAP::const_iterator itConfig = 
		CSsm_PauseTV_s1::configMap.find("SESSION_TIME_OUT");
	if(itConfig != CSsm_PauseTV_s1::configMap.end())
		inteval = atol(itConfig->second.c_str());
	unsigned long minisecond = inteval*1000;
	while(1)
	{
		DWORD dwRet = WaitForSingleObject(_hExit, minisecond);
		if (dwRet == WAIT_OBJECT_0)
			break;		

		time_t tmNow;
		tmNow = time(NULL);

		myGlog(ZQ::common::Log::L_DEBUG, PSLOG("########Cleanup Session Starts########"));
		::std::vector<::std::string> sessionArray;
		CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::const_iterator itClient;
		{
			IceUtil::RecMutex::Lock lock(CSsm_PauseTV_s1::clientInfoMapMutex);
			
			for(itClient = CSsm_PauseTV_s1::clientInfoMap.begin();itClient != CSsm_PauseTV_s1::clientInfoMap.end();itClient++)
			{
				//TODO:find the session whose time is out.
				time_t tmLastAccess = ((CSsm_PauseTV_s1::CLIENT_INFO)(itClient->second)).lastAccessTime;
				long dis = tmNow-tmLastAccess;
				if(dis >= inteval)
				{
					tm* tmStru = localtime(&tmLastAccess);
					char strLastAccess[50];
					sprintf(strLastAccess,"%04d-%02d-%02d %02d:%02d:%02d",tmStru->tm_year+1900,tmStru->tm_mon+1,tmStru->tm_mday
						,tmStru->tm_hour,tmStru->tm_min,tmStru->tm_sec);
					myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Session:%s should be deleted		Last access time:%s")
						,itClient->first.c_str(),strLastAccess);
					sessionArray.push_back(itClient->first);
				}
				else if(dis < 0)
				{
					CSsm_PauseTV_s1::UpdateLastAccessTime(itClient->first,"TimeRollBack");
				}
			}
		}
		
		//
		// process
		//
		::std::vector<::std::string>::const_iterator itSession;
		for(itSession = sessionArray.begin();itSession != sessionArray.end();itSession++)
		{
			IceUtil::RecMutex::Lock lock(CSsm_PauseTV_s1::clientInfoMapMutex);
			CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::iterator itDelete = CSsm_PauseTV_s1::clientInfoMap.find(*itSession);
			if(itDelete != CSsm_PauseTV_s1::clientInfoMap.end())
			{
				try
				{
					const CSsm_PauseTV_s1::CLIENT_INFO clientInfo = ((CSsm_PauseTV_s1::CLIENT_INFO)(itDelete->second));

					//DO: Delete Session from ClientInfoMap
					{
						CSsm_PauseTV_s1::clientInfoMap.erase(itDelete);
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Remove %s from ClientInfoMap"),(*itSession).c_str());
					}
					
					//DO: Delete Stream from StreamToSessionMap
					{
						IceUtil::RecMutex::Lock lock1(CSsm_PauseTV_s1::streamToSessionMapMutex);
						CSsm_PauseTV_s1::STREAM_TO_SESSION_MAP::iterator itS = CSsm_PauseTV_s1::streamToSessionMap.find(clientInfo.stmString);
						if(itS != CSsm_PauseTV_s1::streamToSessionMap.end())
						{
							CSsm_PauseTV_s1::streamToSessionMap.erase(itS);
							myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Remove (%s) from StreamToSessionMap"),(clientInfo.stmString).c_str());
						}
					}

					::Ice::ObjectPrx basePur = 
						CSsm_PauseTV_s1::ic->stringToProxy(clientInfo.purString);
					::ChannelOnDemand::ChannelPurchaseExPrx purchasePrx = 
						::ChannelOnDemand::ChannelPurchaseExPrx::checkedCast(basePur);

					myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Before Create Server Request"));
					IServerRequest* pSrvrRequest = ::CSsm_PauseTV_s1::globalSite->newServerRequest((*itSession).c_str());
					
					if(pSrvrRequest)
					{
						myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Success Create Server Request"));

						char gBuff[MY_BUFFER_SIZE];
						
						/// write the header of ANNOUCE request.
						::std::string responseHead;
						responseHead = "ANNOUNCE rtsp://" + clientInfo.site + "/" + clientInfo.path + "?channelId=" + clientInfo.channelId + " RTSP/1.0";
						pSrvrRequest->printCmdLine(responseHead.c_str());
						
						/// write the head of client session
						/// here gBuff stores the session id.
						strcpy(gBuff,(*itSession).c_str());
						pSrvrRequest->printHeader(HEADER_SESSION,gBuff);
						
						/// write the head of server.
						strcpy(gBuff,"ssm_PauseTV_s1 ");
						strcat(gBuff,SSM_PAUSETV_PLUGIN_VER);
						pSrvrRequest->printHeader(HEADER_SERVER,gBuff);
						::std::string srvrData = gBuff;
						
						/// write the head of Seachange-Notice.
						strcpy(gBuff,SC_ANNOUNCE_ENDOFSTREAM);
						strcat(gBuff," \"End-of-Stream Reached\" ");
						SYSTEMTIME time;
						GetLocalTime(&time);
						char t[50];
						sprintf(t,"%04d%02d%02dT%02d%02d%02dZ ",time.wYear,time.wMonth,time.wDay,
							time.wHour,time.wMinute,time.wSecond);
						strcat(gBuff,t);
						strcat(gBuff,"\"Session Timeout\"");
						pSrvrRequest->printHeader(HEADER_SC_NOTICE,gBuff);
						::std::string notice = gBuff;
						
						pSrvrRequest->post();
						myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Session Timeout Announce has been sent out"));
						
						if(!pSrvrRequest)
							pSrvrRequest->release();
					}
					else
					{
						myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Fail To Create Server Request"));
					}
					
					if(purchasePrx)
					{
						// do bookmark...
						::std::string title;

						//save bookmark
						purchasePrx->bookmark("timeout",NULL);
						myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Save Bookmark %s"),clientInfo.purString.c_str());

						// do teardown...
						purchasePrx->destroy();
						myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Destroy Purchase:%s"
							SEPARATOR_LINE "	Reason: Session Timeout")
							,clientInfo.purString.c_str());
					}
				}
				catch(::TianShanIce::BaseException& ex)
				{
					myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Cacth an %s:%s"),ex.ice_name().c_str(),ex.message.c_str());
				}
				catch(Ice::Exception& ex)
				{
					myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Cacth an %s"),ex.ice_name().c_str());
				}
				catch(...)
				{
					myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Cacth an unknown exception"));
				}	
				
				// Delete RTSP Session
				{
					CSsm_PauseTV_s1::globalSite->destroyClientSession((*itSession).c_str());
					myGlog(ZQ::common::Log::L_DEBUG, PSLOG("Delete RTSP Connection and Session %s"),(*itSession).c_str());
				}
			}
		}
		
		myGlog(ZQ::common::Log::L_DEBUG, PSLOG("########Cleanup Session End########"));
	}
	return 0;
}

