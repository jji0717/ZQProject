#include <winsock2.h>
#include "VLCStreamServiceImpl.h"
#include <IceUtil/UUID.h>
#include <sstream>
#include "VLCVSSCfgLoader.h"
#include "VLCVSSCommitThrd.h"

extern ZQ::common::Config::Loader< ::ZQTianShan::VSS::VLC::VLCVSSCfg > pConfig;

namespace ZQTianShan{
namespace VSS{
namespace VLC{

static void dummyprint1(const char* line,void* pVoid = NULL)
{
	printf("%s\n", line);
}
/***********************
//VLCStreamServiceImpl
************************/
VLCStreamServiceImpl::
VLCStreamServiceImpl(ZQ::common::FileLog &LogFile,
					  //::ZQ::common::NativeThreadPool &pool,
					  ::Ice::CommunicatorPtr& communicator,
					  int32 &evictorSize,
					  VLCVSSEnv &env)
:_env(env)
,_logFile(LogFile)
{
	_quitHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	_guid.create();

	//active adapter
	try
	{
		_env._adapter->ZQADAPTER_ADD(_env._communicator, this, ADAPTER_NAME_VLCVSS);
		//_env._adapter->activate();
	} 
	catch (const Ice::Exception& e) 
	{
		envlog(::ZQ::common::Log::L_ERROR ,CLOGFMT(VLCStreamServiceImpl, "add adapter catch Ice:Exception(%s)"), e.what());
	} 
	catch (const char* msg) 
	{
		envlog(::ZQ::common::Log::L_ERROR ,CLOGFMT(VLCStreamServiceImpl, "add adapter catch UnknowException(%s)"), msg);
	}

	//initialize environment for TMVSS evictor
	_env._adapter->findServantLocator(DBFILENAME_VLCSession);
	if (evictorSize <= 0)
		_env._eVLCStream->setSize(1000);
	else
		_env._eVLCStream->setSize(evictorSize);
	Freeze::EvictorIteratorPtr evicIter = _env._eVLCStream->getIterator("", _env._eVLCStream->getSize());
	const ::Ice::Current c;
	while (evicIter && evicIter->hasNext())
	{
		::Ice::Identity _ident = evicIter->next();
		try
		{
			::TianShanIce::Streamer::VLCStreamServer::VLCStreamPrx streamPrx = VLCVSSIdentityToObjEnv(_env, VLCStream, _ident);

			//use proxy to restore data
			std::string strSessionId = streamPrx->getSessionId();
			std::string strOnDemandSessId = streamPrx->getOnDemandSessionId();
			::TianShanIce::Streamer::VLCStreamServer::VLCPlayList tmpPL = streamPrx->getPlayList();

			//initialize session
			VLCTelnetSession *tmvssProxy = new VLCTelnetSession(_env._logFile, strSessionId, streamPrx->getDestIp(), streamPrx->getDestPort());
			//TMVSSProxy *tmvssProxy = new TMVSSProxy();
			//memcpy(tmvssProxy->endpoint, _env._urlStr.getPath(), strlen(_env._urlStr.getPath()));

			//add to map
			_env._vlcTelnetSessionPool.addSession(strSessionId, tmvssProxy);
		}
		catch (::Ice::UnmarshalOutOfBoundsException &ex)
		{
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(VLCStreamServiceImpl, "catch UnmarshalOutOfBoundsException when restore group from db error(%s)"), ex.what());
			_env._eVLCStream->remove(_ident);
		}
		catch(...)
		{
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(VLCStreamServiceImpl, "restore group from db error(%s)"), _ident.name.c_str());
			_env._eVLCStream->remove(_ident);
		}
	}
}

VLCStreamServiceImpl::
~VLCStreamServiceImpl()
{
	terminate();
	CloseHandle(_quitHandle);
}

::TianShanIce::Streamer::StreamPrx VLCStreamServiceImpl::
createStream(const ::TianShanIce::Transport::PathTicketPrx& pathTicket,
			 const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	//get information from pathTicket
	//catch exception, return null
	try
	{
		/// read from ticket: stream->onDemandSessId = 
		//printf("pathTicket string %s\n",_env._communicator->proxyToString(pathTicket).c_str());
		::Ice::Identity ident = pathTicket->getIdent();

		VLCStreamImplPtr stream = new VLCStreamImpl(_env);
		stream->pathTicketStr = _env._communicator->proxyToString(pathTicket);
		stream->ident.name = ident.name;
		stream->ident.category = DBFILENAME_VLCSession;
		stream->sessKey = ident.name;
	
		//get dest ip and port
		TianShanIce::Variant	valDestAddr;
		TianShanIce::Variant	valDestPort;
		::TianShanIce::SRM::ResourceMap resMap = pathTicket->getResources();

		//get destination IP
		stream->_pl._ouputItem._mux = "ts";
		stream->_pl._ouputItem._access = "udp";
		valDestAddr = GetResourceMapData (resMap,TianShanIce::SRM::rtEthernetInterface,"destIP");
		if (valDestAddr.type != TianShanIce::vtStrings || valDestAddr.strs.size () == 0)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("VLCVSS",1043,"invalid destAddress type should be vtString or no destAddress data with ticket [%s]"), stream->pathTicketStr.c_str());		
			return NULL;
		}
		else
		{
			stream->_pl._ouputItem._dstIp = valDestAddr.strs[0];
			stream->destIp = valDestAddr.strs[0];
			envlog(::ZQ::common::Log::L_INFO, CLOGFMT("VLCVSS", "Get DestIP[%s] through ticket[%s]"),stream->_pl._ouputItem._dstIp.c_str(),stream->pathTicketStr.c_str());
		}

		////get destination port
		valDestPort = GetResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destPort");
		if ( valDestPort.type != TianShanIce::vtInts || valDestPort.ints.size() == 0 )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("VLCVSS",1044,"invalid dest por type,should be vtInts,or no destPort data with ticket [%s]"),stream->pathTicketStr.c_str());
			return NULL;
		}
		else
		{
			stream->_pl._ouputItem._dstPort = valDestPort.ints[0];
			stream->destPort = valDestPort.ints[0];
			envlog(::ZQ::common::Log::L_INFO,CLOGFMT("VLCVSS", "Get destPort[%d] through ticket[%s]"),stream->_pl._ouputItem._dstPort,stream->pathTicketStr.c_str());
		}

		//add stream into evictor map
		DWORD sTime = GetTickCount();

		_env._eVLCStream->add(stream, stream->ident);
		envlog(::ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamServiceImpl, "add session(%s) evictor size(%d) cost %dms"), stream->sessKey.c_str(), _env._eVLCStream->getSize(), GetTickCount() - sTime);

		::TianShanIce::Streamer::StreamPrx streamProxy = VLCVSSIdentityToObjEnv(_env, VLCStream, stream->ident);
		if (streamProxy == NULL)
		{
			::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(VLCStreamServiceImpl, 306, "createSession() NULL proxy"));
		}

		//initialize session
		VLCTelnetSession *pSess = new VLCTelnetSession(_env._logFile, stream->sessKey, _env._strServerPath.c_str(), _env._uServerPort);
		pSess->_strStreamName = _env._communicator->proxyToString(streamProxy);
		_env._vlcTelnetSessionPool.addSession(stream->sessKey, pSess);

		//return
		return streamProxy;
	}
	catch (Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(VLCStreamServiceImpl, 306, "createSession() catch ice exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(VLCStreamServiceImpl, 307, "createSession() catch unknown exception"));
	}
	return NULL;
}

::TianShanIce::Streamer::StreamerDescriptors
VLCStreamServiceImpl::listStreamers(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	::TianShanIce::Streamer::StreamerDescriptors _Descriptors;

	::TianShanIce::Streamer::StreamerDescriptor _Descriptor;
	char tmpGUid[128];

	memset(tmpGUid, 0 , 128);
	gethostname(tmpGUid, 127);
	_Descriptor.deviceId = tmpGUid;
	_Descriptor.type = "SeaChange."DBFILENAME_VLCSession;
	_Descriptors.push_back(_Descriptor);

	return _Descriptors;
}

::std::string VLCStreamServiceImpl::
getNetId(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError)
{
	return std::string(DBFILENAME_VLCSession);
}

void VLCStreamServiceImpl::
getStreamStat_async(const::TianShanIce::Streamer::VLCStreamServer::AMD_VLCStreamService_getStreamStatPtr& amdCB, 
					const ::TianShanIce::StrValues&, 
					const ::std::string&, 
					const ::Ice::Current& c)
{
	//TODO:
	::TianShanIce::Streamer::VLCStreamServer::StreamStatCollection streamStateCollection;
	amdCB->ice_response(streamStateCollection);
}

::TianShanIce::Streamer::VLCStreamServer::VLCStreamPrx VLCStreamServiceImpl::findStreamByOnDemandSession(const ::std::string& onDemandSessionId, const ::Ice::Current& c)
{
	::std::vector<::Ice::Identity> identities = _env._idxSessionIdx->find(onDemandSessionId);

	if (identities.size() > 0)
		return VLCVSSIdentityToObjEnv(_env, VLCStream, identities[0]);
	else
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(VLCStreamServiceImpl, 306, "could not find streamingSession use IdentId(%s)"), onDemandSessionId.c_str());
		return NULL;
	}
}

::TianShanIce::Streamer::VLCStreamServer::VLCStreamPrx VLCStreamServiceImpl::findStreamBySession(const ::std::string& sessionId, const ::Ice::Current& c)
{
	VLCTelnetSession  *pSess = _env._vlcTelnetSessionPool.findSession(const_cast<std::string &>(sessionId));
	if (pSess == NULL)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(VLCStreamServiceImpl, 307, "could not find streamingSession use sessionId(%s)"), sessionId.c_str());
		return NULL;
	}
	else
	{
		return findStreamByOnDemandSession(pSess->_strKey, c);
	}
}

::std::string VLCStreamServiceImpl::
getAdminUri(const ::Ice::Current& c)
{
	return std::string("");
}

::TianShanIce::State VLCStreamServiceImpl::
getState(const ::Ice::Current& c)
{
	return ::TianShanIce::stNotProvisioned;
}

//don't need impl
void VLCStreamServiceImpl::
queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdReplicaQuery, const ::std::string& category, const ::std::string& groupId, bool Only, const ::Ice::Current& c)
{
	::TianShanIce::Replicas replicas;
	amdReplicaQuery->ice_response(replicas);
}

int VLCStreamServiceImpl::run(void)
{
	DWORD sTime = GetTickCount();
	int nRetry = 0;
	while(1)
	{
		DWORD eTime = GetTickCount();
		if (eTime - sTime < pConfig._streamServiceProp.synInterval * 1000)
		{
			DWORD dwResult = WaitForSingleObject(_quitHandle, pConfig._streamServiceProp.synInterval * 1000 - (eTime - sTime));
			if (WAIT_OBJECT_0 == dwResult)
			{
				envlog(::ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamServiceImpl, "run() : Stream Service is terminated"));
				break;
			}
		}

		sTime = GetTickCount();
		//synce status
		Freeze::EvictorIteratorPtr evicIter = _env._eVLCStream->getIterator("", _env._eVLCStream->getSize());
		const ::Ice::Current c;

		// get vlc server play list 
		std::string strMsg("");
		if (!_env._vlcTelnetSessionPool.ControlVLC("show media", strMsg, 1))
		{
			// fail to show list
			nRetry++;
			if (nRetry < pConfig._streamServiceProp.retry)
			{
				envlog(::ZQ::common::Log::L_WARNING, CLOGFMT(VLCStreamServiceImpl, "run() : Fail to show play list, retry[%d]"), nRetry);
				continue;
			}
		}
		ZQ::common::TelnetParser parser(NULL);
		std::list<std::string> lst;
		if (!parser.getList(strMsg, lst))
		{
			// fail to parser list
			envlog(::ZQ::common::Log::L_WARNING, CLOGFMT(VLCStreamServiceImpl, "run() : Fail to parser [%s] to get play list"), strMsg.c_str());
			if (nRetry < pConfig._streamServiceProp.retry)
			{
				continue;
			}
		}
		nRetry = 0;
		while (evicIter && evicIter->hasNext())
		{
			::Ice::Identity _ident = evicIter->next();
			try
			{
				::TianShanIce::Streamer::VLCStreamServer::VLCStreamPrx streamPrx = VLCVSSIdentityToObjEnv(_env, VLCStream, _ident);

				//use proxy to restore data
				std::string strKey = streamPrx->getOnDemandSessionId();
				envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(VLCStreamServiceImpl, "run() : streamobj(iden:%s) set to check status"), strKey.c_str());
				
				//std::string strOnDemandSessId = streamPrx->getOnDemandSessionId();

				//VLCTelnetSession *tmpSess = NULL;
				//if (strKey.empty() == false)
				//	tmpSess = _env._vlcTelnetSessionPool.findSession(strKey);


				//DWORD ssTime = GetTickCount();
				//if (tmpSess != NULL)
				//{
				//	envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(VLCStreamServiceImpl, "streamobj(ident:%d) status=%d"), strKey.c_str(), tmpSess->_bDestroy);
				//	
				//	if (tmpSess->_bDestroy)
				//	{
				//		try
				//		{
				//			streamPrx->destroy();
				//			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(VLCStreamServiceImpl, "streamobj(ident:%d) set to destroy status cost %dms"), strKey.c_str(), GetTickCount() - ssTime);

				//			::ZQTianShan::VSS::VLC::listmem params;
				//			params.type = ::ZQTianShan::VSS::VLC::E_PLAYLIST_STATECHANGED;
				//			params.param[EventField_PlaylistGuid] = strKey;
				//			params.param[EventField_PrevState] = TianShanIce::Streamer::stsSetup;
				//			params.param[EventField_CurrentState] = TianShanIce::Streamer::stsStop;
				//			_env._VLCVSSEventSink._paramsList.PushBack(params);
				//		}
				//		catch(...)
				//		{

				//		}
				//	}
				//}
				try
				{

					DWORD ssTime = GetTickCount();
					if (!lst.empty() && std::find(lst.begin(), lst.end(), strKey) != lst.end())
					{
						lst.remove(strKey);
						if (streamPrx->getCurrentState() == ::TianShanIce::Streamer::stsStop)
						{
							envlog(::ZQ::common::Log::L_DEBUG, CLOGFMT(VLCStreamServiceImpl, "run() : [delete] streamobj(ident:%s) is stopped and to be deleting..........."), strKey.c_str());
							streamPrx->destroy();
							envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(VLCStreamServiceImpl, "run() : [delete] streamobj(ident:%s) is destoryed, cost %dms"), strKey.c_str(), GetTickCount() - ssTime);
						}
						else
						{
							streamPrx->renewPathTicket();
						}
					}
					else
					{
						streamPrx->destroy();
						envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(VLCStreamServiceImpl, "run() : [delete] streamobj(ident:%s) fail to synce from VLC server and to be destroy, cost %dms"), strKey.c_str(), GetTickCount() - ssTime);
					}
				}
				catch(::TianShanIce::ServerError &ex)
				{
					envlog(::ZQ::common::Log::L_NOTICE, CLOGFMT(VLCStreamServiceImpl, "run() : streamobj(ident:%d) get ice exception(%s) when sync stream status with VLC"), strKey.c_str(), ex.what());
					_env._eVLCStream->remove(_ident);
				}
				catch(...)
				{
					envlog(::ZQ::common::Log::L_NOTICE, CLOGFMT(VLCStreamServiceImpl, "run() : streamobj(ident:%d) get unknown exception when sync stream status with VLC"));
					_env._eVLCStream->remove(_ident);
				}
			}
			catch(...)
			{
				envlog(::ZQ::common::Log::L_NOTICE, CLOGFMT(VLCStreamServiceImpl, "run() : check stream status caught exception"));
			}
		}
		if (!lst.empty())
		{
			std::list<std::string>::iterator p = lst.begin();
			char szMsg[1024];
			while (p != lst.end())
			{
				envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(VLCStreamServiceImpl,
					"run() : [delete] playlist [%s] only in VLC service and will be delete"), p->c_str());
				sprintf(szMsg, "del %s", p->c_str());
				_env._vlcTelnetSessionPool.ControlVLC(szMsg, std::string(""), 0);
				p++;
			}
		}
	}
	return 1;
}

void* getVariantValue(::TianShanIce::Variant &val)
{
	switch (val.type)
	{
		case ::TianShanIce::vtStrings:
			return (void *)&(val.strs[0]);
		case ::TianShanIce::vtBin:
			return (void *)&(val.bin);
		case ::TianShanIce::vtFloats:
			return (void *)&(val.floats);
		case ::TianShanIce::vtInts:
			return (void *)&(val.ints);
		case ::TianShanIce::vtLongs:
			return (void *)&(val.lints[0]);
		default:
			return "";
	}
}

void* getPrivateDataValue(const ::std::string &str, ::TianShanIce::ValueMap &pVal)
{
	::TianShanIce::ValueMap::iterator iter;
	iter = pVal.find(str);
	if (iter == pVal.end())
		return "";
	else
		return getVariantValue((*iter).second);
}

TianShanIce::Variant VLCStreamServiceImpl::GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
										const TianShanIce::SRM::ResourceType& type,
										const std::string& strkey)
{
	TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
	if(itResMap==rcMap.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() type %d not found",type);
		
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("VLCVSS",1001,szBuf );
	}
	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if(it==itResMap->second.resourceData.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() value with key=%s not found",strkey.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("VLCVSS",1002,szBuf);
	}
	return it->second;
}

}//namespace TM
}//namespace VSS
}//namespace ZQTianShan