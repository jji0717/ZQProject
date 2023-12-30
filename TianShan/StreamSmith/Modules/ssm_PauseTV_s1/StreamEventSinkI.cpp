// StreamEventSinkI.cpp: implementation of the StreamEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StreamEventSinkI.h"
#include "StreamSmithModule.h"
#include "TianShanIce.h"
#include "ZQResource.h"
#include <string>
//ZQ::common::ScLog *			StreamEventSinkI::pLog = NULL;
ZQ::common::FileLog *			StreamEventSinkI::pLog = NULL;
#define myGlog			(*StreamEventSinkI::pLog)
#define PSLOG(_X) "	" _X	

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//StreamEventSinkI::StreamEventSinkI(ZQ::common::ScLog * log)
StreamEventSinkI::StreamEventSinkI(ZQ::common::FileLog * log)
{
	pLog = log;
}

StreamEventSinkI::~StreamEventSinkI()
{
	/// 
}

void StreamEventSinkI::ping(::Ice::Long lv, const ::Ice::Current& ic)
{
	/// 
}

void StreamEventSinkI::OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic)const
{
	try{
		Ice::CommunicatorPtr icm = ic.adapter->getCommunicator();
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("EVENT-TYPE:	End-of-Stream"
			SEPARATOR_LINE "	STREAM: %s"
			SEPARATOR_LINE "	Before find Stream in StreamToSessionMap")
			,proxy.c_str());
		
		{
			IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::streamToSessionMapMutex);
			::CSsm_PauseTV_s1::STREAM_TO_SESSION_MAP::iterator itStm = ::CSsm_PauseTV_s1::streamToSessionMap.find(proxy);
			if(itStm != ::CSsm_PauseTV_s1::streamToSessionMap.end())
			{
				/// find the client session id.
				char buff[MY_BUFFER_SIZE];
				strcpy(buff,itStm->second.c_str());
				
				::std::string sessionid;
				sessionid = buff;
				
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success find Stream in StreamToSessionMap"
					SEPARATOR_LINE "	Before Create Server Request"));
				
				IServerRequest *pSrvrRequest = ::CSsm_PauseTV_s1::globalSite->newServerRequest(buff);
				if(!pSrvrRequest)
				{
					myGlog(ZQ::common::Log.L_ERROR,PSLOG("Fail to Create Server Request"));
					return;
				}
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success to Create Server Request"));
				
				{
					IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::clientInfoMapMutex);
					::CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::iterator itClient = ::CSsm_PauseTV_s1::clientInfoMap.find(sessionid);
					if(itClient != ::CSsm_PauseTV_s1::clientInfoMap.end())
					{
						const CSsm_PauseTV_s1::CLIENT_INFO& clientInfo = ((CSsm_PauseTV_s1::CLIENT_INFO)(itClient->second));
						
						/// write the header of ANNOUCE request.
						::std::string responseHead;
						responseHead = "ANNOUNCE rtsp://" + clientInfo.site + "/" + clientInfo.path + "?channelId=" + clientInfo.channelId + " RTSP/1.0";
						pSrvrRequest->printCmdLine(responseHead.c_str());
						
						/// write the head of client session.
						pSrvrRequest->printHeader(HEADER_SESSION,buff);
						
						/// write the head of server.
						::std::string srvrData;
						strcpy(buff,"ssm_PauseTV_s1 ");
						strcat(buff,SSM_PAUSETV_PLUGIN_VER);
						pSrvrRequest->printHeader(HEADER_SERVER,buff);
						srvrData = buff;
						
						/// write the head of Seachange-Notice.
						::std::string notice;
						strcpy(buff,SC_ANNOUNCE_ENDOFSTREAM);
						strcat(buff," \"End-of-Stream Reached\" ");
						SYSTEMTIME time;
						GetLocalTime(&time);
						char t[50];
						sprintf(t,"%04d%02d%02dT%02d%02d%02dZ ",time.wYear,time.wMonth,time.wDay,
							time.wHour,time.wMinute,time.wSecond);
						strcat(buff,t);
						strcat(buff,"\"Normal End\"");
						pSrvrRequest->printHeader(HEADER_SC_NOTICE,buff);
						notice = buff;
						
						pSrvrRequest->post();
						
						//DO: Delete Session from ClientInfoMap
//						{
//							::CSsm_PauseTV_s1::clientInfoMap.erase(itClient);
//							myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Remove %s from ClientInfoMap"),sessionid.c_str());
//						}
						
						//DO: Delete Stream from StreamToSessionMap
//						{
//							::CSsm_PauseTV_s1::streamToSessionMap.erase(itStm);
//							myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Remove (%s) from StreamToSessionMap"),proxy.c_str());
//						}
						
						//DO: Delete RTSP Session
//						{
//							::CSsm_PauseTV_s1::globalSite->destroyClientSession(sessionid.c_str());
//							myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success Delete RTSP Connection and Session"));
//						}
						
						if(pSrvrRequest)
							pSrvrRequest->release();
						
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("**************************************************************************"
							SEPARATOR_LINE "	%s"
							SEPARATOR_LINE "	Session: %s"
							SEPARATOR_LINE "	Server: %s"
							SEPARATOR_LINE "	Seachange-Notice: %s"
							SEPARATOR_LINE "	**************************************************************************")
							,responseHead.c_str(),sessionid.c_str(),srvrData.c_str(),notice.c_str());
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
		myGlog(ZQ::common::Log.L_ERROR,PSLOG("Catch An Exception When Process end-of-stream Event"));
	}
}

void StreamEventSinkI::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic)const
{
	try{
		Ice::CommunicatorPtr icm = ic.adapter->getCommunicator();
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("EVENT-TYPE:	Beginning-of-Stream"
			SEPARATOR_LINE "	STREAM: %s"
			SEPARATOR_LINE "	Before find Stream in StreamToSessionMap")
			,proxy.c_str());

		{
			IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::streamToSessionMapMutex);
			::CSsm_PauseTV_s1::STREAM_TO_SESSION_MAP::const_iterator itStm = ::CSsm_PauseTV_s1::streamToSessionMap.find(proxy);
			if(itStm != ::CSsm_PauseTV_s1::streamToSessionMap.end())
			{
				/// find the client session id.
				char buff[MY_BUFFER_SIZE];
				strcpy(buff,itStm->second.c_str());
				
				::std::string sessionid;
				sessionid = buff;
				
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success find Stream in StreamToSessionMap"
					SEPARATOR_LINE "	Before create server request"));
				
				IServerRequest *pSrvrRequest = ::CSsm_PauseTV_s1::globalSite->newServerRequest(buff);
				if(!pSrvrRequest)
				{
					myGlog(ZQ::common::Log.L_ERROR,PSLOG("Fail to create server request"));
					return;
				}
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success to create server request"));
				
				{
					IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::clientInfoMapMutex);
					::CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::const_iterator it = ::CSsm_PauseTV_s1::clientInfoMap.find(sessionid);
					if(it != ::CSsm_PauseTV_s1::clientInfoMap.end())
					{
						const CSsm_PauseTV_s1::CLIENT_INFO& clientInfo = ((CSsm_PauseTV_s1::CLIENT_INFO)(it->second));
						
						/// write the header of ANNOUCE request.
						::std::string responseHead;
						responseHead = "ANNOUNCE rtsp://" + clientInfo.site + "/" + clientInfo.path + "?channelId=" + clientInfo.channelId + " RTSP/1.0";
						pSrvrRequest->printCmdLine(responseHead.c_str());
						
						/// write the head of client session
						pSrvrRequest->printHeader(HEADER_SESSION,buff);
						
						/// write the head of server.
						::std::string srvrData;
						strcpy(buff,"ssm_PauseTV_s1 ");
						strcat(buff,SSM_PAUSETV_PLUGIN_VER);
						pSrvrRequest->printHeader(HEADER_SERVER,buff);
						srvrData = buff;
						
						/// write the head of Seachange-Notice.
						::std::string notice;
						strcpy(buff,SC_ANNOUNCE_BEGINOFSTREAM);
						strcat(buff," \"Beginning-of-Stream Reached\" ");
						SYSTEMTIME time;
						GetLocalTime(&time);
						char t[50];
						sprintf(t,"%04d%02d%02dT%02d%02d%02dZ",time.wYear,time.wMonth,time.wDay,
							time.wHour,time.wMinute,time.wSecond);
						strcat(buff,t);
						pSrvrRequest->printHeader(HEADER_SC_NOTICE,buff);
						notice = buff;
						
						pSrvrRequest->post();
						
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("**************************************************************************"
							SEPARATOR_LINE "	%s"
							SEPARATOR_LINE "	Session: %s"
							SEPARATOR_LINE "	Server: %s"
							SEPARATOR_LINE "	Seachange-Notice: %s"
							SEPARATOR_LINE "	**************************************************************************")
							,responseHead.c_str(),sessionid.c_str(),srvrData.c_str(),notice.c_str());
						
						if(pSrvrRequest)
							pSrvrRequest->release();
					}
					else
					{
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("%s"),CANNOT_FIND_SESSION);
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
		myGlog(ZQ::common::Log.L_ERROR,PSLOG("Catch An Exception When Process beginning-of-stream Event")); 
	}
}

void StreamEventSinkI::OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const ::Ice::Current& ic)const
{
	myGlog(ZQ::common::Log.L_DEBUG,PSLOG("EVENT-TYPE:	Scale Changed"
		SEPARATOR_LINE "	STREAM: %s"
		SEPARATOR_LINE "	Current: %f, Previous: %f")
		,proxy.c_str(),currentSpeed,prevSpeed);

	if(prevSpeed == 0)
	{
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Previous speed is zero, need not to send out"));
		return;
	}
	try{
		Ice::CommunicatorPtr icm = ic.adapter->getCommunicator();
		/// find the client session id.
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Before find Stream in StreamToSessionMap"));

		{
			IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::streamToSessionMapMutex);
			::CSsm_PauseTV_s1::STREAM_TO_SESSION_MAP::const_iterator itStm = ::CSsm_PauseTV_s1::streamToSessionMap.find(proxy);
			if(itStm != ::CSsm_PauseTV_s1::streamToSessionMap.end())
			{
				/// find the client session id.
				char buff[MY_BUFFER_SIZE];
				strcpy(buff,itStm->second.c_str());
				
				::std::string sessionid;
				sessionid = buff;
				
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success find Stream in StreamToSessionMap"
					SEPARATOR_LINE "	Before create server request"));
				
				IServerRequest *pSrvrRequest = ::CSsm_PauseTV_s1::globalSite->newServerRequest(buff);
				if(!pSrvrRequest)
				{
					myGlog(ZQ::common::Log.L_ERROR,PSLOG("Fail to create server request"));
					return;
				}
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success to create server request"));
				
				/// find the client information by search map.
				{
					IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::clientInfoMapMutex);
					::CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::const_iterator it = ::CSsm_PauseTV_s1::clientInfoMap.find(sessionid);
					if(it != ::CSsm_PauseTV_s1::clientInfoMap.end())
					{
						const CSsm_PauseTV_s1::CLIENT_INFO& clientInfo = ((CSsm_PauseTV_s1::CLIENT_INFO)(it->second));
						
						/// write the header of ANNOUCE request.
						::std::string responseHead;
						responseHead = "ANNOUNCE rtsp://" + clientInfo.site + "/" + clientInfo.path + "?channelId=" + clientInfo.channelId + " RTSP/1.0";
						pSrvrRequest->printCmdLine(responseHead.c_str());
						
						/// write the head of client session
						pSrvrRequest->printHeader(HEADER_SESSION,buff);
						
						/// write the head of server.
						::std::string srvrData;
						strcpy(buff,"ssm_PauseTV_s1 ");
						strcat(buff,SSM_PAUSETV_PLUGIN_VER);
						pSrvrRequest->printHeader(HEADER_SERVER,buff);
						srvrData = buff;
						
						/// write the head of Seachange-Notice.
						::std::string notice;
						strcpy(buff,ZQ_ANNOUNCE_SPEEDCHANGE);
						strcat(buff," \"Scale Changed\" ");
						SYSTEMTIME time;
						GetLocalTime(&time);
						char t[50];
						sprintf(t,"%04d%02d%02dT%02d%02d%02dZ",time.wYear,time.wMonth,time.wDay,
							time.wHour,time.wMinute,time.wSecond);
						strcat(buff,t);
						pSrvrRequest->printHeader(HEADER_SC_NOTICE,buff);
						notice = buff;
						
						/// write the head of Scale.
						::std::string sCurSpeed;
						sprintf(buff,"%f",currentSpeed);
						pSrvrRequest->printHeader(HEADER_SCALE,buff);
						sCurSpeed = buff;
						
						{
							pSrvrRequest->post();
						}
						
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("**************************************************************************"
							SEPARATOR_LINE "	%s"
							SEPARATOR_LINE "	Session: %s"
							SEPARATOR_LINE "	Scale: %s"
							SEPARATOR_LINE "	Server: %s"
							SEPARATOR_LINE "	Seachange-Notice: %s"
							SEPARATOR_LINE "	**************************************************************************")
							,responseHead.c_str(),sessionid.c_str(),sCurSpeed.c_str(),srvrData.c_str(),notice.c_str());
						
						if(pSrvrRequest)
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
		myGlog(ZQ::common::Log.L_ERROR,PSLOG("Catch An Exception When Process speed-changed Event"));
	}
}

void StreamEventSinkI::OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState currentState, const ::Ice::Current& /*ic = ::Ice::Current()*/) const
{
	/// 
}
void StreamEventSinkI::OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic /*= ::Ice::Current()*/) const
{
	myGlog(ZQ::common::Log.L_DEBUG,PSLOG("EVENT-TYPE:	Stream Exit"
		SEPARATOR_LINE "	STREAM: %s")
		,proxy.c_str());

	try{
		Ice::CommunicatorPtr icm = ic.adapter->getCommunicator();
		myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Before find Stream in StreamToSessionMap"));

		{
			IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::streamToSessionMapMutex);
			::CSsm_PauseTV_s1::STREAM_TO_SESSION_MAP::iterator itStm = ::CSsm_PauseTV_s1::streamToSessionMap.find(proxy);
			if(itStm != ::CSsm_PauseTV_s1::streamToSessionMap.end())
			{
				/// find the client session id.
				char buff[MY_BUFFER_SIZE];
				strcpy(buff,itStm->second.c_str());
				
				::std::string sessionid;
				sessionid = buff;

				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success find Stream in StreamToSessionMap"));
				
				// If normal exit
				if(nExitCode == 0)
				{
					//DO: Delete Session from ClientInfoMap
					{
						IceUtil::RecMutex::Lock lock(CSsm_PauseTV_s1::clientInfoMapMutex);
						::CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::iterator itC = ::CSsm_PauseTV_s1::clientInfoMap.find(sessionid);
						if(itC != ::CSsm_PauseTV_s1::clientInfoMap.end())
						{
							::CSsm_PauseTV_s1::clientInfoMap.erase(itC);
							myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Delete Session from ClientInfoMap"));
						}
					}
					
					//DO: Delete Stream from StreamToSessionMap
					{
						::CSsm_PauseTV_s1::streamToSessionMap.erase(itStm);
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Delete Stream from StreamToSessionMap"));
					}

					//DO: Delete RTSP Session
					{
						::CSsm_PauseTV_s1::globalSite->destroyClientSession(sessionid.c_str());
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success Delete RTSP Connection and Session"));
					}

					myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Normal exit, Need not send out"));
					return;
				}
				
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Before Create Server Request"));
				
				IServerRequest *pSrvrRequest = ::CSsm_PauseTV_s1::globalSite->newServerRequest(buff);
				if(!pSrvrRequest)
				{
					myGlog(ZQ::common::Log.L_ERROR,PSLOG("Fail to Create Server Request"));
					return;
				}
				myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success to Create Server Request"));
				
				/// find the client information by search map.
				{
					IceUtil::RecMutex::Lock lock(::CSsm_PauseTV_s1::clientInfoMapMutex);
					::CSsm_PauseTV_s1::SESSION_TO_CLIENT_INFO_MAP::iterator it = ::CSsm_PauseTV_s1::clientInfoMap.find(sessionid);
					if(it != ::CSsm_PauseTV_s1::clientInfoMap.end())
					{
						const CSsm_PauseTV_s1::CLIENT_INFO& clientInfo = ((CSsm_PauseTV_s1::CLIENT_INFO)(it->second));
//						
						/// write the header of ANNOUCE request.
						::std::string responseHead;
						responseHead = "ANNOUNCE rtsp://" + clientInfo.site + "/" + clientInfo.path + "?channelId=" + clientInfo.channelId + " RTSP/1.0";
						pSrvrRequest->printCmdLine(responseHead.c_str());
						
						/// write the head of client session
						pSrvrRequest->printHeader(HEADER_SESSION,buff);
						
						/// write the head of server.
						::std::string srvrData;
						strcpy(buff,"ssm_PauseTV_s1 ");
						strcat(buff,SSM_PAUSETV_PLUGIN_VER);
						pSrvrRequest->printHeader(HEADER_SERVER,buff);
						srvrData = buff;
						
						/// write the head of Seachange-Notice.
						::std::string notice;
						strcpy(buff,ZQ_ANNOUNCE_EXCEPTION_EXIT);
						strcat(buff," \"Stream Exit\" ");
						SYSTEMTIME time;
						GetSystemTime(&time);
						char t[50];
						sprintf(t,"%04d%02d%02dT%02d%02d%02dZ",time.wYear,time.wMonth,time.wDay,
							time.wHour,time.wMinute,time.wSecond);
						strcat(buff,t);
						pSrvrRequest->printHeader(HEADER_SC_NOTICE,buff);
						notice = buff;
						pSrvrRequest->post();
						
						//DO: Delete Session from ClientInfoMap
						{
							::CSsm_PauseTV_s1::clientInfoMap.erase(it);
							myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Remove %s from ClientInfoMap"),sessionid.c_str());
						}
						
						//DO: Delete Stream from StreamToSessionMap
						{
							::CSsm_PauseTV_s1::streamToSessionMap.erase(itStm);
							myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Remove (%s) from StreamToSessionMap"),proxy.c_str());
						}
						
						//DO: Delete RTSP Session
						{
							::CSsm_PauseTV_s1::globalSite->destroyClientSession(sessionid.c_str());
							myGlog(ZQ::common::Log.L_DEBUG,PSLOG("Success Delete RTSP Connection and Session"));
						}
						
						myGlog(ZQ::common::Log.L_DEBUG,PSLOG("**************************************************************************"
							SEPARATOR_LINE "	%s"
							SEPARATOR_LINE "	Session: %s"
							SEPARATOR_LINE "	Server: %s"
							SEPARATOR_LINE "	Seachange-Notice: %s"
							SEPARATOR_LINE "	**************************************************************************")
							,responseHead.c_str(),sessionid.c_str(),srvrData.c_str(),notice.c_str());
						
						if(pSrvrRequest)
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
		myGlog(ZQ::common::Log.L_ERROR,PSLOG("Catch An Exception When Process Stream Exit Event"));
	} 
}