// PlaylistEventSinkI.cpp: implementation of the PlaylistEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlaylistEventSinkI.h"
#include "StreamSmithModule.h"
#include "ZQResource.h"
//ZQ::common::ScLog *		PlaylistEventSinkI::pLog = NULL;
ZQ::common::FileLog *		PlaylistEventSinkI::pLog = NULL;
#define	myGlog	(*PlaylistEventSinkI::pLog)
#define PSLOG(_X)	"	" _X 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//PlaylistEventSinkI::PlaylistEventSinkI(ZQ::common::ScLog * log)
PlaylistEventSinkI::PlaylistEventSinkI(ZQ::common::FileLog * log)
{
	pLog = log;
}

PlaylistEventSinkI::~PlaylistEventSinkI()
{

}

void PlaylistEventSinkI::ping(::Ice::Long lv, const ::Ice::Current& ic)
{
	/// 
}

void PlaylistEventSinkI::OnItemStepped(const ::std::string& proxy, 
									   const ::std::string& playlistId,
									   ::Ice::Int curUserCtrlNum,
									   ::Ice::Int prevUserCtrlNum, 
									   const ::TianShanIce::Properties& prty,
									   const ::Ice::Current& ic) const
{
	::Ice::Int userCtrlNum=prevUserCtrlNum;
	::std::string itemName;
	::TianShanIce::Properties tempMap;
	tempMap = prty;
	itemName = tempMap["ItemName"];
	
	myGlog(ZQ::common::Log.L_DEBUG,PSLOG("EVENT-TYPE:end-of-item"
		SEPARATOR_LINE "	PlaylistID: \"%s\""
		SEPARATOR_LINE "	UserCtrlNum: %d"
		SEPARATOR_LINE "	Before find stream in StreamToSessionMap")
		,playlistId.c_str(),userCtrlNum);

	try{
		Ice::CommunicatorPtr icm = ic.adapter->getCommunicator();
		{
			IceUtil::RecMutex::Lock lock(CSsm_PauseTV_s1::streamToSessionMapMutex);
			::CSsm_PauseTV_s1::STREAM_TO_SESSION_MAP::const_iterator itStm = ::CSsm_PauseTV_s1::streamToSessionMap.find(proxy);
			if(itStm != ::CSsm_PauseTV_s1::streamToSessionMap.end())
			{
				/// find the client session id.
				char gBuff[MY_BUFFER_SIZE];
				strcpy(gBuff,itStm->second.c_str());
				
				::std::string sClientSession;
				sClientSession = gBuff;
				
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success find stream in StreamToSessionMap"
					SEPARATOR_LINE "	Before create server request"));
				
				IServerRequest *pSrvrRequest = ::CSsm_PauseTV_s1::globalSite->newServerRequest(sClientSession.c_str());
				if(!pSrvrRequest)
				{
					myGlog(ZQ::common::Log.L_ERROR,PSLOG("Fail to create server request"));
					return;
				}
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success to create server request"));
				
				{
					IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::clientInfoMapMutex);
					::CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::const_iterator it = ::CSsm_PauseTV_s1::clientInfoMap.find(sClientSession);
					if(it != ::CSsm_PauseTV_s1::clientInfoMap.end())
					{
						const ::CSsm_PauseTV_s1::CLIENT_INFO& clientInfo = it->second;
						
						/// write the header of ANNOUCE request.
						::std::string responseHead;
						responseHead = "ANNOUNCE rtsp://" + clientInfo.site + "/" + clientInfo.path + "?channelId=" + clientInfo.channelId + " RTSP/1.0";
						pSrvrRequest->printCmdLine(responseHead.c_str());
						
						/// write the head of client session
						/// here gBuff stores the session id.
						pSrvrRequest->printHeader(HEADER_SESSION,gBuff);
						
						/// write the head of server.
						strcpy(gBuff,"ssm_PauseTV_s1 ");
						strcat(gBuff,SSM_PAUSETV_PLUGIN_VER);
						pSrvrRequest->printHeader(HEADER_SERVER,gBuff);
						::std::string srvrData = gBuff;
						
						/// write the head of Seachange-Notice.
						strcpy(gBuff,ZQ_ANNOUNCE_ENDOFITEM);
						strcat(gBuff," \"End-of-Item Reached\" ");
						SYSTEMTIME time;
						GetLocalTime(&time);
						char t[50];
						sprintf(t,"%04d%02d%02dT%02d%02d%02dZ \"",time.wYear,time.wMonth,time.wDay,
							time.wHour,time.wMinute,time.wSecond);
						strcat(gBuff,t);
						strcat(gBuff,itemName.c_str());
						strcat(gBuff,"\"");
						pSrvrRequest->printHeader(HEADER_SC_NOTICE,gBuff);
						::std::string notice = gBuff;
						
						pSrvrRequest->post();
						
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("**************************************************************************"
							SEPARATOR_LINE "	%s"
							SEPARATOR_LINE "	Session: %s"
							SEPARATOR_LINE "	Server: %s"
							SEPARATOR_LINE "	Seachange-Notice: %s"
							SEPARATOR_LINE "	**************************************************************************")
							,responseHead.c_str(),sClientSession.c_str(),srvrData.c_str(),notice.c_str());
						
						if(!pSrvrRequest)
							pSrvrRequest->release();
					}
					else
					{
						myGlog(ZQ::common::Log.L_ERROR,PSLOG("%s"),CANNOT_FIND_SESSION);
					}
				}
			}
			else
			{
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Can't find Stream in StreamToSessionMap, it was not created by this application."));
			}
		}
	}
	catch(...)
	{
		myGlog(ZQ::common::Log.L_ERROR,PSLOG("Catch an exception when process end-of-item event"));
	}
}
