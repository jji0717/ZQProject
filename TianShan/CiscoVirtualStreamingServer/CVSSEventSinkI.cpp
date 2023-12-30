#include <winsock2.h>
#include <TianShanDefines.h>
#include "CVSSEventSinkI.h"

namespace ZQTianShan{
namespace CVSS{

#define MYLOG (_fileLog)

CVSSEventSinkI::CVSSEventSinkI(::ZQ::common::FileLog &fileLog, const char * topicManagerEndpoint)
:_fileLog(fileLog)
{
	_bQuit=false;
	_bNetworkOK=false;
	if (NULL == topicManagerEndpoint || strlen(topicManagerEndpoint) <=0)
		_strTopicProxy = SERVICE_NAME_TopicManager":"DEFAULT_ENDPOINT_TopicManager;
	else
		_strTopicProxy = SERVICE_NAME_TopicManager":" + ::std::string(topicManagerEndpoint);
	_hBadConn=CreateEvent(NULL,FALSE,FALSE,NULL);
	//connectToStormService();
}

CVSSEventSinkI::CVSSEventSinkI(::ZQ::common::FileLog &fileLog, const char * topicManagerEndpoint, ZQADAPTER_DECLTYPE  adpater)
:_fileLog(fileLog),
_objAdapter(adpater)
{
	_bQuit=false;
	_bNetworkOK=false;
	if (NULL == topicManagerEndpoint || strlen(topicManagerEndpoint) <=0)
		_strTopicProxy = SERVICE_NAME_TopicManager":"DEFAULT_ENDPOINT_TopicManager;
	else
		_strTopicProxy = SERVICE_NAME_TopicManager":" + ::std::string(topicManagerEndpoint);
	_hBadConn=CreateEvent(NULL,FALSE,FALSE,NULL);
	connectToStormService();
}
			
CVSSEventSinkI::~CVSSEventSinkI()
{
	_bQuit=true;
	SetEvent(_hBadConn);
	Sleep(1);
	waitHandle(1000);
	CloseHandle(_hBadConn);
	exit();
}

void CVSSEventSinkI::registerEventSink()
{

}

bool EventDispatchIceStormI(DWORD eventType,ZQ::common::Variant& params,void* pExtraData)
{
	CVSSEventSinkI* pThis=(CVSSEventSinkI*)pExtraData;
	return pThis->dispatchEvent(eventType,params);
}

TianShanIce::Streamer::StreamState ConvertVstrmStateToTianshanIceState(ULONG plstate)
{
	TianShanIce::Streamer::StreamState ret=TianShanIce:: Streamer::stsStop;

	return ret;		 
}

bool CVSSEventSinkI::dispatchEvent(DWORD eventType,::ZQ::common::Variant& params)
{
	if(_strTopicProxy.empty())
	{
		//SUPERLOG(Log::L_DEBUG,SPLUGIN("Don't send out message because no ice storm service is connected"));
		return true;
	}
	if(!_bNetworkOK)
	{
		//glog(Log::L_WARNING,"Net work not accessible,event is discarded!");
		return false;
	}	
	//E_PLAYLIST_INPROGRESS|E_PLAYLIST_ITEMDONE|E_PLAYLIST_DONE
	try
	{
		switch(eventType)
		{
		case E_PLAYLIST_INPROGRESS:
			{//get var				
				//get playlist guid
				std::string	strGuid=(std::string)params[EventField_PlaylistGuid];
				//retrieve playlist ice proxy string
				std::string	strProxy;
				Ice::Identity	idTemp;
				idTemp.category=PL_CATALOG;
				idTemp.name=strGuid;
				Ice::ObjectPrx prx=_objAdapter->createProxy(idTemp);
				strProxy=_objAdapter->getCommunicator()->proxyToString(prx);
				prx=NULL;
				ULONGLONG	done,total;
				int			step,totalstep;				
				params.getQuadword(params,EventField_runningByteOffset,done);
				params.getQuadword(params,EventField_TotalbyteOffset,total);
				step=(int)params[EventField_currentStep];
				totalstep=(int)params[EventField_totalStep];
				int	CtrlNumber=(int)params[EventField_UserCtrlNum];
				std::string	strItemName=(std::string)params[EventField_ItemFileName];
				char	szbuf[1024];
				sprintf(szbuf,"CtrlNum=%d&ItemName=%s",CtrlNumber,strItemName.c_str());
				long	lSeq = (long)params[EventField_EventCSEQ];
				char szSeq[16];
				sprintf(szSeq,"%d",lSeq);
				try
				{
					Ice::Context ctx;					
					ctx[EVENTCSEQ] = szSeq ; 
					_objProgressEventPrx->OnProgress(	strProxy,strGuid,
						done,total,
						(Ice::Int)step,(Ice::Int)totalstep,
						szbuf,
						ctx );
				}
				catch (...) 
				{
					NotifyBadConnection();
					return false;
				}
			}
			break;
		case E_PLAYLIST_ITEMDONE:
			{
				std::string	strProxy;

				std::string	strGuid				= (std::string)params[EventField_PlaylistGuid];
				std::string strPrevFileName		= (std::string)params[EventField_ItemFileName];;
				std::string strnextFileName		= (std::string)params[EventField_ItemOtherFileName];;
				std::string	strStampUTC			= (std::string)params[EventField_StampUTC];

				int			ctrlPrevItem		= (int)params[EventField_UserCtrlNum];
				int			ctrlNextItem		= (int)params[EventField_NextUserCtrlNum];

				int			timeoffsetCurItem	= (int)params[EventField_CurrentItemTimeOffset];


				TianShanIce::Properties	pro;
				pro.insert(std::make_pair<std::string,std::string>("prevItemName",strPrevFileName));
				pro.insert(std::make_pair<std::string,std::string>("currentItemName",strnextFileName));
				pro.insert(std::make_pair<std::string,std::string>("stampUTC",strStampUTC));
				char szBuf[256];
				pro.insert(std::make_pair<std::string,std::string>("currentItemTimeOffset", std::string(itoa(timeoffsetCurItem,szBuf,10))));

				Ice::Identity id;
				id.category=PL_CATALOG;
				id.name=strGuid;
				Ice::ObjectPrx prx=_objAdapter->createProxy(id);
				strProxy=_objAdapter->getCommunicator()->proxyToString(prx);
				prx=NULL;

				long	lSeq = (long)params[EventField_EventCSEQ];
				char szSeq[16];
				sprintf(szSeq,"%d",lSeq);
				pro.insert(std::make_pair<std::string,std::string>(EVENTCSEQ,szSeq));
				try
				{
					Ice::Context ctx;					
					ctx[EVENTCSEQ] = szSeq ; 
					_objPlaylistEventPrx->OnItemStepped(strProxy,strGuid,
						ctrlNextItem,ctrlPrevItem,
						pro,
						ctx);
				}
				catch (...) 
				{
					NotifyBadConnection();
					return false;
				}

			}
			break;
		case E_PLAYLIST_END:
			{
				std::string	strGuid;
				std::string	strProxy;
				strGuid=(std::string)params[EventField_PlaylistGuid];				
				Ice::Identity id;
				id.category=PL_CATALOG;
				id.name=strGuid;
				Ice::ObjectPrx prx=_objAdapter->createProxy(id);
				strProxy=_objAdapter->getCommunicator()->proxyToString(prx);
				prx=NULL;
				long	lSeq = (long)params[EventField_EventCSEQ];
				char szSeq[16];
				sprintf(szSeq,"%d",lSeq);
				try
				{
					Ice::Context ctx;					
					ctx[EVENTCSEQ] = szSeq;
					_objStreamEventPrx->OnEndOfStream( strProxy , strGuid , ctx);
				}
				catch (...) 
				{
					NotifyBadConnection();
					return false;
				}
			}
			break;
		case E_PLAYLIST_BEGIN:
			{
				std::string	strGuid;
				std::string	strProxy;
				strGuid=(std::string)params[EventField_PlaylistGuid];				
				Ice::Identity id;
				id.category=PL_CATALOG;
				id.name=strGuid;
				Ice::ObjectPrx prx=_objAdapter->createProxy(id);
				strProxy=_objAdapter->getCommunicator()->proxyToString(prx);
				prx=NULL;

				long	lSeq = (long)params[EventField_EventCSEQ];
				char szSeq[16];
				sprintf(szSeq,"%d",lSeq);

				try
				{
					Ice::Context ctx;					
					ctx[EVENTCSEQ] = szSeq ;
					_objStreamEventPrx->OnBeginningOfStream( strProxy , strGuid ,ctx );
				}
				catch (...) 
				{
					NotifyBadConnection();
					return false;
				}
			}
			break;
		case E_PLAYLIST_SPEEDCHANGED:
			{
				std::string	strGuid;
				std::string	strProxy;
				strGuid=(std::string)params[EventField_PlaylistGuid];				
				Ice::Identity id;
				id.category=PL_CATALOG;
				id.name=strGuid;
				Ice::ObjectPrx prx=_objAdapter->createProxy(id);
				strProxy=_objAdapter->getCommunicator()->proxyToString(prx);
				prx=NULL;
				ZQ::common::Variant	varSpeedPrev=(ZQ::common::Variant&)params[EventField_PrevSpeed];
				float	prevSpeed=(float)((int)varSpeedPrev[EventField_SpeedNumer])/(float)((int)varSpeedPrev[EventField_SpeedDenom]);
				ZQ::common::Variant varSpeedCurrent=(ZQ::common::Variant&)params[EventField_CurrentSpeed];
				float	currentSpeed=(float)((int)varSpeedCurrent[EventField_SpeedNumer])/(float)((int)varSpeedCurrent[EventField_SpeedDenom]);

				std::string strContentName = (std::string)params[EventField_ItemFileName];
				int iCtrlNum = (int)params[EventField_UserCtrlNum];

				long	lSeq = (long)params[EventField_EventCSEQ];
				char szSeq[16];
				sprintf(szSeq,"%d",lSeq);

				try
				{
					Ice::Context ctx;					
					ctx[EVENTCSEQ] = szSeq ;
					_objStreamEventPrx->OnSpeedChanged(strProxy, strGuid, 
						prevSpeed, currentSpeed,
						ctx );
				}
				catch (...) 
				{
					NotifyBadConnection();
					return false;
				}
			}
			break;
		case E_PLAYLIST_STATECHANGED:
			{
				std::string	strGuid;
				std::string	strProxy;
				strGuid=(std::string)params[EventField_PlaylistGuid];				
				Ice::Identity id;
				id.category=PL_CATALOG;
				id.name=strGuid;
				Ice::ObjectPrx prx=_objAdapter->createProxy(id);
				strProxy=_objAdapter->getCommunicator()->proxyToString(prx);
				prx=NULL;
				TianShanIce::Streamer::StreamState		PrevState;
				TianShanIce::Streamer::StreamState		CurrentState;
				PrevState	=	ConvertVstrmStateToTianshanIceState((long)params[EventField_PrevState]);
				CurrentState=	ConvertVstrmStateToTianshanIceState((long)params[EventField_CurrentState]);
				std::string strContentName = (std::string)params[EventField_ItemFileName];
				int iCtrlNum = (int)params[EventField_UserCtrlNum];

				long	lSeq = (long)params[EventField_EventCSEQ];
				char szSeq[16];
				sprintf(szSeq,"%d",lSeq);

				try
				{
					Ice::Context ctx;					
					ctx[EVENTCSEQ] = szSeq ;
					_objStreamEventPrx->OnStateChanged( strProxy , strGuid , 
						PrevState , CurrentState,
						ctx);
				}
				catch (...) 
				{
					NotifyBadConnection();
					return false;
				}
			}
			break;
		case E_PLAYLIST_DESTROYED:
			{
				std::string strGuid;
				std::string strProxy;
				std::string	strReason;
				strReason=(std::string)params[EventField_ExiReason];
				strGuid=(std::string)params[EventField_PlaylistGuid];
				Ice::Identity id;
				id.category=PL_CATALOG;
				id.name=strGuid;
				Ice::ObjectPrx prx=_objAdapter->createProxy(id);
				strProxy=_objAdapter->getCommunicator()->proxyToString(prx);
				prx=NULL;
				int exitCode=(int)params[EventField_ExitCode];
				long	lSeq = (long)params[EventField_EventCSEQ];
				char szSeq[16];
				sprintf(szSeq,"%d",lSeq);
				try
				{
					Ice::Context ctx;					
					ctx[EVENTCSEQ] = szSeq ;
					_objStreamEventPrx->OnExit(strProxy , strGuid , exitCode , strReason ,ctx );
				}
				catch (...) 
				{
					NotifyBadConnection();
					return false;
				}
			}
			break;
		default:
			break;
		}
	}
	catch (...) 
	{
		return false;
	}
	return true;
}

int	 CVSSEventSinkI::run()
{
	do 
	{
		WaitForSingleObject(_hBadConn,20000);
		if(!_bQuit)
		{
			if(!_bNetworkOK)
			{
				if (!_strTopicProxy.empty()) 
				{
					MYLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(CVSSEventSinkI, "no connection to ice storm service,connect it now"));
					connectToStormService();
				}
			}
			else
			{
				try
				{
					topicManagerPrx->ice_ping();
					if (!_paramsList.Empty())
					{
						::ZQTianShan::CVSS::listmem tmpMem = _paramsList.First();
						//::ZQ::common::Variant tmp = tmpMem.param;
						
						bool b = dispatchEvent(tmpMem.type, tmpMem.param);
					}
				}
				catch (...) 
				{
					_bNetworkOK=false;
					SetEvent(_hBadConn);
				}
			}
		}
	} while(!_bQuit);

	return 1;
}

void CVSSEventSinkI::NotifyBadConnection()
{
	//MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CVSSEventSinkI,"Ice storm down detected,connect it again"));
	_bNetworkOK=false;
	SetEvent(_hBadConn);
}

bool CVSSEventSinkI::connectToStormService()
{
	try
	{
		if(_strTopicProxy.empty())
		{
			_bNetworkOK=true;
			return true;
		}
		//MYLOG(::ZQ::common::Log::L_INFO,CLOGFMT(CVSSEventSinkI,"connect to ice storm service with %s"),_strTopicProxy.c_str());
		Ice::CommunicatorPtr ic =_objAdapter->getCommunicator();
		Ice::ObjectPrx base = ic->stringToProxy(_strTopicProxy);
		if(!base)
		{
			return false;
		}
		topicManagerPrx=IceStorm::TopicManagerPrx::checkedCast(base);
		if(!topicManagerPrx)
		{
			return false;
		}
		//get topic
		IceStorm::TopicPrx			playlistTopic;
		IceStorm::TopicPrx			streamTopic;
		IceStorm::TopicPrx			progressTopic;

		Ice::ObjectPrx				obj;
		try
		{
			playlistTopic=topicManagerPrx->retrieve(TianShanIce::Streamer::TopicOfPlaylist);
		}
		catch (const IceStorm::NoSuchTopic& e)
		{
			playlistTopic=topicManagerPrx->create(TianShanIce::Streamer::TopicOfPlaylist);
		}
		obj=playlistTopic->getPublisher();
		if(!obj->ice_isDatagram())
		{
			obj->ice_oneway();
		}
		_objPlaylistEventPrx=TianShanIce::Streamer::PlaylistEventSinkPrx::uncheckedCast(obj);

		try
		{
			streamTopic=topicManagerPrx->retrieve(TianShanIce::Streamer::TopicOfStream);
		}
		catch (const IceStorm::NoSuchTopic& e)
		{
			streamTopic=topicManagerPrx->create(TianShanIce::Streamer::TopicOfStream);
		}
		obj=streamTopic->getPublisher();
		if(!obj->ice_isDatagram())
		{
			obj->ice_oneway();
		}
		_objStreamEventPrx=TianShanIce::Streamer::StreamEventSinkPrx::uncheckedCast(obj);

		try
		{
			progressTopic=topicManagerPrx->retrieve(TianShanIce::Streamer::TopicOfStreamProgress);
		}
		catch (const IceStorm::NoSuchTopic& e) 
		{
			progressTopic=topicManagerPrx->create(TianShanIce::Streamer::TopicOfStreamProgress);
		}
		obj=progressTopic->getPublisher();
		if(!obj->ice_isDatagram())
		{
			obj->ice_oneway();
		}
		_objProgressEventPrx=TianShanIce::Streamer::StreamProgressSinkPrx::uncheckedCast(obj);		
		MYLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(CVSSEventSinkI, "connect to ice storm service %s successfully!"),_strTopicProxy.c_str());
	}
	catch (Ice::Exception& ex) 
	{
		MYLOG(::ZQ::common::Log::L_ERROR,CLOGFMT(CVSSEventSinkI, "Catch %s when connect to ice Storm %s"),	ex.ice_name().c_str(),_strTopicProxy.c_str());
		return false;
	}
	catch (...)
	{
		MYLOG(::ZQ::common::Log::L_ERROR,CLOGFMT(CVSSEventSinkI, "Connect to Ice Storm service %s failed"),_strTopicProxy.c_str());
		return false;
	}
	_bNetworkOK=true;
	return true;
}

bool CVSSEventSinkI::SetHandle()
{
	return SetEvent(_hBadConn);
}

}//namespace CVSS
}//namespace ZQTianShan