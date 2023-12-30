#include <winsock2.h>
#include "TMVSStreamServiceImpl.h"
#include <IceUtil/UUID.h>
#include <sstream>
#include "TMVSSCfgLoader.h"
#include "TMVSSCommitThrd.h"

extern ZQ::common::Config::Loader< ::ZQTianShan::VSS::TM::TMVSSCfg > pConfig;

namespace ZQTianShan{
namespace VSS{
namespace TM{

	static void dummyprint1(const char* line,void* pVoid = NULL)
	{
		printf("%s\n", line);
	}
/***********************
//TMVStreamServiceImpl
************************/
TMVStreamServiceImpl::
TMVStreamServiceImpl(ZQ::common::FileLog &LogFile,
					  //::ZQ::common::NativeThreadPool &pool,
					  ::Ice::CommunicatorPtr& communicator,
					  std::string &strServerPath,
					  uint16 &uServerPort,
					  std::string &strNotifyServer,
					  uint16 &usNotifyPort,
					  int32 &evictorSize,
					  TMVSSEnv &env)
:_env(env)
,_logFile(LogFile)
//,_pool(pool)
,_strServerPath(strServerPath)
,_uServerPort(uServerPort)
,_strNotifyServer(strNotifyServer)
,_usNotifyPort(usNotifyPort)
{
	_guid.create();

	//active adapter
	try
	{
		_env._adapter->ZQADAPTER_ADD(_env._communicator, this, ADAPTER_NAME_TMVSS);
		//_env._adapter->activate();
	} 
	catch (const Ice::Exception& e) 
	{
		envlog(::ZQ::common::Log::L_ERROR ,CLOGFMT(TMVStreamServiceImpl, "add adapter catch Ice:Exception(%s)"), e.what());
	} 
	catch (const char* msg) 
	{
		envlog(::ZQ::common::Log::L_ERROR ,CLOGFMT(TMVStreamServiceImpl, "add adapter catch UnknowException(%s)"), msg);
	}

	//initialize environment for TMVSS evictor
	_env._adapter->findServantLocator(DBFILENAME_TMVSession);
	if (evictorSize <= 0)
		_env._eTMVStream->setSize(1000);
	else
		_env._eTMVStream->setSize(evictorSize);
	Freeze::EvictorIteratorPtr evicIter = _env._eTMVStream->getIterator("", _env._eTMVStream->getSize());
	const ::Ice::Current c;
	while (evicIter && evicIter->hasNext())
	{
		::Ice::Identity _ident = evicIter->next();
		try
		{
			::TianShanIce::Streamer::TMVStreamServer::TMVStreamPrx streamPrx = TMVSSIdentityToObjEnv(_env, TMVStream, _ident);

			//use proxy to restore data
			std::string strSessionId = streamPrx->getSessionId();
			std::string strOnDemandSessId = streamPrx->getOnDemandSessionId();

			//initialize session
			TMVSoapClientSession *tmvssProxy = new TMVSoapClientSession(_env._logFile, _strServerPath.c_str(), _uServerPort);
			//TMVSSProxy *tmvssProxy = new TMVSSProxy();
			//memcpy(tmvssProxy->endpoint, _env._urlStr.getPath(), strlen(_env._urlStr.getPath()));

			//add to map
			_env._pTMVSSSoapServer->_soapClientMap.addSoapClient(strSessionId, tmvssProxy);
		}
		catch (::Ice::UnmarshalOutOfBoundsException &ex)
		{
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(TMVStreamServiceImpl, "catch UnmarshalOutOfBoundsException when restore group from db error(%s)"), ex.what());
			_env._eTMVStream->remove(_ident);
		}
		catch(...)
		{
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(TMVStreamServiceImpl, "restore group from db error(%s)"), _ident.name.c_str());
			_env._eTMVStream->remove(_ident);
		}
	}
}

TMVStreamServiceImpl::
~TMVStreamServiceImpl()
{
	terminate();
}

::TianShanIce::Streamer::StreamPrx TMVStreamServiceImpl::
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

		//initialize session
		//TMVSoapClientSession *tmvssProxy = new TMVSoapClientSession(_env._logFile, _strServerPath.c_str(), _uServerPort);

		TMVStreamImplPtr stream = new TMVStreamImpl(_env);
		stream->pathTicketStr = _env._communicator->proxyToString(pathTicket);
		printf("pathticket string=%s\n", stream->pathTicketStr.c_str());
		if (!stream)
			return NULL;
		//std::stringstream ss;
		//ss << GetTickCount();
		stream->ident.category = "TMVSS";
		//stream->ident.name = "TMVSS" + ss.str();
		stream->ident.name = ident.name;

		DWORD sTime = GetTickCount();
		stream->sessKey = stream->ident.name;
		_env._eTMVStream->add(stream, stream->ident);
		envlog(::ZQ::common::Log::L_INFO, CLOGFMT(TMVStreamServiceImpl, "add session(%s) cost %dms"), stream->sessKey.c_str(), GetTickCount() - sTime);

		::TianShanIce::Streamer::StreamPrx streamProxy = TMVSSIdentityToObjEnv(_env, TMVStream, stream->ident);
		return streamProxy;
	}
	catch (Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(TMVStreamServiceImpl, 306, "createSession() catch ice exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(TMVStreamServiceImpl, 307, "createSession() catch unknown exception"));
	}
	return NULL;
}

::TianShanIce::Streamer::StreamerDescriptors
TMVStreamServiceImpl::listStreamers(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	::TianShanIce::Streamer::StreamerDescriptors _Descriptors;

	::TianShanIce::Streamer::StreamerDescriptor _Descriptor;
	char tmpGUid[128];

	memset(tmpGUid, 0 , 128);
	gethostname(tmpGUid, 127);
	_Descriptor.deviceId = tmpGUid;
	_Descriptor.type = "SeaChange."DBFILENAME_TMVSession;
	_Descriptors.push_back(_Descriptor);

	return _Descriptors;
}

::std::string TMVStreamServiceImpl::
getNetId(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError)
{
	return std::string(DBFILENAME_TMVSession);
}

void TMVStreamServiceImpl::
getStreamStat_async(const::TianShanIce::Streamer::TMVStreamServer::AMD_TMVStreamService_getStreamStatPtr& amdCB, 
					const ::TianShanIce::StrValues&, 
					const ::std::string&, 
					const ::Ice::Current& c)
{
	//TODO:
	::TianShanIce::Streamer::TMVStreamServer::StreamStatCollection streamStateCollection;
	amdCB->ice_response(streamStateCollection);
}

::TianShanIce::Streamer::TMVStreamServer::TMVStreamPrx TMVStreamServiceImpl::findStreamByOnDemandSession(const ::std::string& onDemandSessionId, const ::Ice::Current& c)
{
	::std::vector<::Ice::Identity> identities = _env._idxSessionIdx->find(onDemandSessionId);

	if (identities.size() > 0)
		return TMVSSIdentityToObjEnv(_env, TMVStream, identities[0]);
	else
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(TMVStreamServiceImpl, 306, "could not find streamingSession use IdentId(%s)"), onDemandSessionId.c_str());
		return NULL;
	}
}

::TianShanIce::Streamer::TMVStreamServer::TMVStreamPrx TMVStreamServiceImpl::findStreamBySession(const ::std::string& sessionId, const ::Ice::Current& c)
{
	TMVSoapClientSession  *pSess = _env._pTMVSSSoapServer->_soapClientMap.getSoapClient(const_cast<std::string &>(sessionId));
	if (pSess == NULL)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(TMVStreamServiceImpl, 307, "could not find streamingSession use sessionId(%s)"), sessionId.c_str());
		return NULL;
	}
	else
	{
		return findStreamByOnDemandSession(pSess->strOndemandSessionId, c);
	}
}

::std::string TMVStreamServiceImpl::
getAdminUri(const ::Ice::Current& c)
{
	return std::string("");
}

::TianShanIce::State TMVStreamServiceImpl::
getState(const ::Ice::Current& c)
{
	return ::TianShanIce::stNotProvisioned;
}

//don't need impl
void TMVStreamServiceImpl::
queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdReplicaQuery, const ::std::string& category, const ::std::string& groupId, bool Only, const ::Ice::Current& c)
{
	::TianShanIce::Replicas replicas;
	amdReplicaQuery->ice_response(replicas);
}

int TMVStreamServiceImpl::run(void)
{
	DWORD sTime = GetTickCount();
	while(1)
	{
		DWORD eTime = GetTickCount();
		if (eTime - sTime < pConfig._rtspProp.timeOut * 1000)
		{
			Sleep(pConfig._rtspProp.timeOut * 1000 - (eTime - sTime));
			continue;
		}

		sTime = GetTickCount();
		//synce status
		Freeze::EvictorIteratorPtr evicIter = _env._eTMVStream->getIterator("", _env._eTMVStream->getSize());
		const ::Ice::Current c;
		while (evicIter && evicIter->hasNext())
		{
			::Ice::Identity _ident = evicIter->next();
			try
			{
				::TianShanIce::Streamer::TMVStreamServer::TMVStreamPrx streamPrx = TMVSSIdentityToObjEnv(_env, TMVStream, _ident);

				//use proxy to restore data
 				std::string strSessionId = streamPrx->getSessionId();
				envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(TMVStreamServiceImpl, "streamobj(sess:%d) set to check status"), strSessionId.c_str());

				//std::string strOnDemandSessId = streamPrx->getOnDemandSessionId();

				TMVSoapClientSession *tmpSess = NULL;
				if (strSessionId.empty() == false)
					tmpSess = _env._pTMVSSSoapServer->_soapClientMap.getSoapClient(strSessionId);

				if (tmpSess != NULL)
				{
					envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(TMVStreamServiceImpl, "streamobj(sess:%d) status=%d"), strSessionId.c_str(), tmpSess->_bDestroy);
					if (tmpSess->_bDestroy)
					{
						DWORD ssTime = GetTickCount();
						try
						{
							streamPrx->destroy();
							envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(TMVStreamServiceImpl, "streamobj(sess:%d) set to destroy cost %dms"), strSessionId.c_str(), GetTickCount() - ssTime);

							::ZQTianShan::VSS::TM::listmem params;
							params.type = ::ZQTianShan::VSS::TM::E_PLAYLIST_STATECHANGED;
							params.param[EventField_PlaylistGuid] = streamPrx->getOnDemandSessionId();
							params.param[EventField_PrevState] = TianShanIce::Streamer::stsSetup;
							params.param[EventField_CurrentState] = TianShanIce::Streamer::stsStop;
							_env._tmvssEventSinkI._paramsList.PushBack(params);
						}
						catch(...)
						{

						}
					}
				}
			}
			catch(...)
			{
				envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(TMVStreamServiceImpl, "check stream status caught exception"));
			}
		}
	}
	return 1;
}

//void* getVariantValue(::TianShanIce::Variant &val)
//{
//	switch (val.type)
//	{
//		case ::TianShanIce::vtStrings:
//			return (void *)&(val.strs[0]);
//		case ::TianShanIce::vtBin:
//			return (void *)&(val.bin);
//		case ::TianShanIce::vtFloats:
//			return (void *)&(val.floats);
//		case ::TianShanIce::vtInts:
//			return (void *)&(val.ints);
//		case ::TianShanIce::vtLongs:
//			return (void *)&(val.lints[0]);
//		default:
//			return "";
//	}
//}
//
//void* getPrivateDataValue(const ::std::string &str, ::TianShanIce::ValueMap &pVal)
//{
//	::TianShanIce::ValueMap::iterator iter;
//	iter = pVal.find(str);
//	if (iter == pVal.end())
//		return "";
//	else
//		return getVariantValue((*iter).second);
//}
//
//TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
//										const TianShanIce::SRM::ResourceType& type,
//										const std::string& strkey)
//{
//	TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
//	if(itResMap==rcMap.end())
//	{
//		char szBuf[1024];
//		sprintf(szBuf,"GetResourceMapData() type %d not found",type);
//		
//		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSS",1001,szBuf );
//	}
//	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
//	if(it==itResMap->second.resourceData.end())
//	{
//		char szBuf[1024];
//		sprintf(szBuf,"GetResourceMapData() value with key=%s not found",strkey.c_str());
//		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSS",1002,szBuf);
//	}
//	return it->second;
//}

}//namespace TM
}//namespace VSS
}//namespace ZQTianShan