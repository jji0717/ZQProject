#include "C2LocatorImpl.h"
#include "strHelper.h"
#include <Text.h>
#ifdef ZQ_OS_LINUX
#include <sys/wait.h>
#endif

static Ice::Long StringToLong(const std::string& s)
{
#ifdef ZQ_OS_MSWIN
	return ::_strtoi64(s.c_str(), NULL, 10);
#elif defined(__x86_64)
	return strtol(s.c_str(), NULL, 10);
#else
	return strtoll(s.c_str(), NULL, 10);
#endif
}

namespace TianShanIce{
namespace SCS{

C2LocatorImpl::C2LocatorImpl(::ZQTianShan::CDN::C2Env &env) //, PortSnmpManager& portSnmpMgr)
:_env(env), _clientMgr(env), _portMgr(env), _quit(false) //, portSnmpMgr_(portSnmpMgr)
{
    _env.setClientManager(&_clientMgr);
    _env.setPortManager(&_portMgr);

//	portSnmpMgr_.reset();
#ifdef ZQ_OS_LINUX
	if(_env._conf.checkRH.script.empty()) {
		envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "No CheckRemoteHealth script be configured."));
	} else if(0 != access(_env._conf.checkRH.script.c_str(), X_OK)) {
		envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "The script file (%s) don't have execute permission"), _env._conf.checkRH.script.c_str());
		_env._conf.checkRH.script.clear();
	}
#else
	// disable the checkRH in windows
	_env._conf.checkRH.script.clear();
#endif
}

C2LocatorImpl::~C2LocatorImpl()
{
	if(streamEventHandler_)
	{
		streamEventHandler_->releaseLocator();
		streamEventHandler_ = NULL;
	}

	if(isRunning())
	{
		_quit = true;
		_env.hExpiredSessionNotifier.signal();
		envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "~C2LocatorImpl The maintenance thread is quiting..."));
		waitHandle(-1); // the thread need quit first
		envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "~C2LocatorImpl The maintenance thread quit"));
	}
	else
	{
		envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "~C2LocatorImpl The maintenance thread is not running."));
	}
}

TransferSessionPrx C2LocatorImpl::openSessionByTransferId(const ::std::string& transferId, const ::Ice::Current& c)
{
	Ice::Identity sessId;
	{
		ZQ::common::MutexGuard gd(_indexLock);
		TransferIdMap::iterator iter = _transferIdMap.find(transferId);
		if (iter == _transferIdMap.end())
		{
			envExtLog(ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "openSession() session(TransferId:%s) not find"), transferId.c_str());
			return NULL;
		}
		else//find in map
		{
			envExtLog(ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "openSession() session(TransferId:%s) find"), transferId.c_str());
			sessId = iter->second;
		}
	}
	return C2IdentityToObjEnv(_env, TransferSession, sessId);
}
ClientTransfers C2LocatorImpl::listClients(const Ice::Current& c)
{
	return _clientMgr.snapshot();
}

TransferPorts C2LocatorImpl::listTransferPorts(const Ice::Current& c)
{
	return _portMgr.snapshot();
}

TransferSessions C2LocatorImpl::listSessionsByClient(const std::string& client, const Ice::Current& c)
{
	TransferSessions ret;

	Freeze::EvictorIteratorPtr evicIter = _env._eC2TransferSession->getIterator("", _env._eC2TransferSession->getSize());
	const Ice::Context context;
	while (evicIter && evicIter->hasNext())
	{
		::std::string strTransferId;
		::Ice::Identity ident = evicIter->next();
		try
		{
			TransferSessionPrx transferSessionPrx = C2IdentityToObjEnv(_env, TransferSession, ident);
			if (!transferSessionPrx)
				continue;

			TransferSessionProp prop = transferSessionPrx->getProps(context);
			if(client == prop.clientTransfer)
				ret.push_back(prop);
		}
		catch(::Ice::UnmarshalOutOfBoundsException &)
		{
			envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "listSessionsByClient() with [client: %s] caught UnmarshalOutOfBoundsException when processing session: [%s]"), client.c_str(), ident.name.c_str());
		}
		catch(::Ice::Exception &ex)
		{
			envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "listSessionsByClient() with [client: %s] caught [%s] when processing session: [%s]"), client.c_str(), ex.ice_name().c_str(), ident.name.c_str());
		}
		catch(...)
		{
			envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "listSessionsByClient() with [client: %s] caught exception when processing session: [%s]"), client.c_str(), ident.name.c_str());
		}
	}
	return ret;
}

TransferSessions C2LocatorImpl::listSessionsByPort(const std::string& port, const Ice::Current& c)
{
	TransferSessions ret;

	Freeze::EvictorIteratorPtr evicIter = _env._eC2TransferSession->getIterator("", _env._eC2TransferSession->getSize());
	const Ice::Context context;
	while (evicIter && evicIter->hasNext())
	{
		::std::string strTransferId;
		::Ice::Identity ident = evicIter->next();
		try
		{
			TransferSessionPrx transferSessionPrx = C2IdentityToObjEnv(_env, TransferSession, ident);
			if (!transferSessionPrx)
				continue;

			TransferSessionProp prop = transferSessionPrx->getProps(context);
			if(port == prop.transferPort)
				ret.push_back(prop);
		}
		catch(::Ice::UnmarshalOutOfBoundsException &)
		{
			envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "listSessionsByPort() with [port: %s] caught UnmarshalOutOfBoundsException when processing session: [%s]"), port.c_str(), ident.name.c_str());
		}
		catch(::Ice::Exception &ex)
		{
			envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "listSessionsByPort() with [port: %s] caught [%s] when processing session: [%s]"), port.c_str(), ex.ice_name().c_str(), ident.name.c_str());
		}
		catch(...)
		{
			envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "listSessionsByPort() with [port: %s] caught exception when processing session: [%s]"), port.c_str(), ident.name.c_str());
		}
	}
	return ret;
}

void C2LocatorImpl::updatePortsAvailability(const TianShanIce::StrValues& ports, bool enabled, const Ice::Current& c)
{
	TransferPort port;
	port.isUp = false;
	port.capacity = 0;
	port.activeBandwidth = 0;
	port.activeTransferCount = 0;

	port.enabled = enabled;

	for(TianShanIce::StrValues::const_iterator it = ports.begin(); it != ports.end(); ++it)
	{
		port.name = (*it);
		_portMgr.updatePortAvailability(port);
	}
}

bool C2LocatorImpl::commit(const std::string& reqId, ::TianShanIce::SCS::TransferSessionPrx transferSession, int& err)
{
	err = 0;
	::std::string strTransferId;
	Ice::Identity sessIdent = transferSession->getIdent();
	try
	{
		int64 sTime = SYS::getTickCount();

		//commit stream
		::TianShanIce::Streamer::StreamPrx streamPrx = transferSession->getStream();
		streamPrx->commit();
		streamPrx->play();

		ZQTianShan::Util::TimeSpan tsSetProps;tsSetProps.start();
		TianShanIce::Properties props = streamPrx->getProperties();
		envExtLog(ZQ::common::Log::L_DEBUG, CLOGFMT(TESTXXX,"%s"), ZQTianShan::Util::dumpStringMap(props).c_str());
		transferSession->setProps3( props );
		tsSetProps.stop();

		//get transfer ID and set index
		//::TianShanIce::SCS::TransferSessionProp prop = transferSession->getProps();
		//strTransferId = prop.transferId;
		ZQTianShan::Util::getPropertyDataWithDefault(props,CDN_TRANSFERID,"",strTransferId);
		if (strTransferId.empty())
		{
			::TianShanIce::SCS::TransferSessionProp prop = transferSession->getProps();
			strTransferId = prop.transferId;
		}
		envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "[%s] commit TransferSession(%s) get transferId(%s), setProps cost us [%lld] ms"), 
			reqId.c_str(), sessIdent.name.c_str(), strTransferId.c_str(), tsSetProps.span() );

		Ice::Identity streamIdent = streamPrx->getIdent();
		//add to memory map
		{
			ZQ::common::MutexGuard gd(_indexLock);
			_transferIdMap[strTransferId] = sessIdent;
			addStreamIndex(streamIdent.name, sessIdent);
		}

		transferSession->setState(::TianShanIce::stInService);
		_env._watch.watch(sessIdent, 0);
		envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "[%s] commit TransferSession(%s) success cost %dms"), reqId.c_str(), strTransferId.c_str(), SYS::getTickCount() - sTime);
		return true;
	}
	catch( const Ice::SocketException& ex)
	{
		err = Ice_SocketException;
		envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "[%s] caught [%s] when commit TransferSession(%s)"), reqId.c_str(), ex.ice_name().c_str(), strTransferId.c_str());
	}
	catch( const Ice::TimeoutException& ex)
	{
		err = Ice_TimeoutException;
		envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "[%s] caught [%s] when commit TransferSession(%s)"), reqId.c_str(), ex.ice_name().c_str(), strTransferId.c_str());
	}
    catch (const TianShanIce::BaseException& e)
    {
        err = e.errorCode;
		if (err <0)
			err = 500; // force to server-side error
        envExtLog(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocatorImpl, "[%s] err(%d) due to %s when committing TransferSession(%s). category=%s code=%d message=%s."), reqId.c_str(), err, e.ice_name().c_str(), sessIdent.name.c_str(), e.category.c_str(), e.errorCode, e.message.c_str());
    }
    catch (::Ice::Exception& ex)
    {
		err = 500; // force to server-side error
        envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "[%s] err(%d) due to %s when committing TransferSession(%s)"), reqId.c_str(), err, ex.ice_name().c_str(), strTransferId.c_str());
    }
    catch (...)
    {
		err = Unknown_Exception;
        envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "[%s] caught Exception when committing TransferSession(%s)"), reqId.c_str(), strTransferId.c_str());
    }

    return false;
}

class DelayedStreamDelCmd : public ZQ::common::ThreadRequest
{
public:
	DelayedStreamDelCmd(::ZQTianShan::CDN::C2Env& env, ::TianShanIce::Streamer::StreamPrx& strm, const std::string& sessDesc)
		:ThreadRequest(env._pool), _env(env), _strm(strm), _sessDesc(sessDesc)
	{
		ThreadRequest::setPriority(200); // quite low priority to yield other operations

		try
		{
			_strmEP = _env._communicator->proxyToString(_strm);
		}
		catch(...) {}
	}

protected:
	void final(int retcode =0, bool bCancelled =false) { delete this; }

	virtual int run()
	{
		int64 stampStart = ZQ::common::now();
		try
		{
			_strm->destroy();
			envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(DelayedStreamDelCmd, "strm[%s] destroyed took[%lld]msec: %s"),
				_strmEP.c_str(), ZQ::common::now()-stampStart, _sessDesc.c_str());

			return 0;
		}
		catch (const Ice::ObjectNotExistException&)
		{
			envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(DelayedStreamDelCmd, "stream[%s] has gone, took[%lld]msec: %s"), _strmEP.c_str(), ZQ::common::now()-stampStart, _sessDesc.c_str());
		}
		catch (const ::Ice::Exception &ex)
		{
			envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(DelayedStreamDelCmd, "destroying stream[%s] took[%lld]msec caught [%s]: %s"), _strmEP.c_str(), ZQ::common::now()-stampStart, ex.ice_name().c_str(), _sessDesc.c_str());
		}
		catch(...)
		{
			envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(DelayedStreamDelCmd, "destroying stream[%s] took[%lld]msec caught exception: %s"), _strmEP.c_str(), ZQ::common::now()-stampStart, _sessDesc.c_str());
		}

		return 1;
	}

	::ZQTianShan::CDN::C2Env& _env;
	::TianShanIce::Streamer::StreamPrx _strm;
	std::string _strmEP;
	std::string _sessDesc;
};

void C2LocatorImpl::destroy(::Ice::Identity &sessIdent)
{
	int64 stampStart = ZQ::common::now();
	ZQTianShan::CDN::ClientManager::ClientInfo clientStatus;
	TianShanIce::SCS::TransferSessionProp prop;
	std::string sessDesc = sessIdent.name.c_str();
	try
	{
		TransferSessionPrx transferSessionPrx = C2IdentityToObjEnv(_env, TransferSession, sessIdent);
		if (!transferSessionPrx)
		{
			envExtLog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2LocatorImpl, "destroy() stream of transfer session is not exist now. session(%s)"), sessIdent.name.c_str());
			return;
		}

		// remove the reservation first
		prop = transferSessionPrx->getProps();
		_portMgr.removeReservation(prop.transferPort, sessIdent.name, prop.allocatedBW);
		_clientMgr.removeReservation(prop.clientTransfer, sessIdent.name, prop.allocatedBW, &clientStatus);

		// post a delayed command to ensure the stream be cleaned up at StreamService side
		char buf[100];
		snprintf(buf, sizeof(buf)-2, "sess[%s] txfId[%s] bw[%lld] client[%s] status[%lld(%lld)/%lld]",
			sessIdent.name.c_str(), prop.transferId.c_str(), prop.allocatedBW, prop.clientTransfer.c_str(), clientStatus.consumedBandwidth, clientStatus.activeTransferCount, clientStatus.ingressCapacity);
		sessDesc = buf;
		::TianShanIce::Streamer::StreamPrx strm =transferSessionPrx->getStream();
		(new DelayedStreamDelCmd(_env, strm, sessDesc))->start();
		envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "destroy() destroyed transfer session(%s)"),sessIdent.name.c_str());
	}
	catch(const ::Ice::Exception &ex)
	{
		envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "destroy() caught [%s] when destroy transfer session(%s)"), ex.ice_name().c_str(), sessIdent.name.c_str());
	}
	catch(...)
	{
		envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "destroy() caught exception when destroy transfer session(%s)"), sessIdent.name.c_str());
	}

	removeTransferSession(sessIdent);
	envExtLog(::ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "destroy() took[%lld]msec to remove: %s"), ZQ::common::now()-stampStart, sessDesc.c_str());
}


void C2LocatorImpl::InitEventSink()
{
	envExtLog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2LocatorImpl, "InitEventSink() enter."));
	try {
		if(!streamEventHandler_) {
			streamEventHandler_ = new StreamEventHandler(_env);
		}
		streamEventHandler_->bindLocator(this);
	} catch (const Ice::Exception& ex) {
		envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "InitEventSink() Failed to init StreamEventHandler, exception(%s)"), ex.ice_name().c_str());
	} catch (...) {
		envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "InitEventSink() Failed to init StreamEventHandler, exception(Unknown)"));
	}

	try
	{
		const TianShanIce::Properties qos;
		TianShanIce::Streamer::StreamEventSinkPtr sinkPtr(streamEventHandler_);
		_env._pEventChannel->sink(sinkPtr, qos);
		envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "InitEventSink() stream event handler is registered."));
	}
	catch(const TianShanIce::BaseException& ex)
	{
		envExtLog(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocatorImpl, "register to eventchannel caught exception(%s)"),ex.ice_name().c_str());
	}
	catch(const Ice::Exception& ex)
	{
		envExtLog(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocatorImpl, "register to eventchannel caught exception(%s)"),ex.ice_name().c_str());
	}
}

void C2LocatorImpl::OnRestore(const ::Ice::Current& c)
{
	Freeze::EvictorIteratorPtr evicIter = _env._eC2TransferSession->getIterator("", _env._eC2TransferSession->getSize());
	const Ice::Context context;
	while (evicIter && evicIter->hasNext())
	{
		::Ice::Identity ident = evicIter->next();
		::std::string strTransferId;
		try
		{
			TransferSessionPrx transferSessionPrx = C2IdentityToObjEnv(_env, TransferSession, ident);
			if (!transferSessionPrx)
				continue;

			TransferSessionProp tmpProp = transferSessionPrx->getProps(context);
			strTransferId = tmpProp.transferId;

			if (strTransferId.empty())
			{
				envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(C2LocatorImpl, "OnRestore() TransferId is null. session(%s)"), ident.name.c_str());
			}
			else//restore the memory map
			{
				ZQ::common::MutexGuard gd(_indexLock);
				_transferIdMap[strTransferId] = ident;
			}

			TianShanIce::Streamer::StreamPrx stream = transferSessionPrx->getStream();
			if(stream)
			{
				std::string streamId = stream->getIdent().name;
				ZQ::common::MutexGuard gd(_indexLock);
				addStreamIndex(streamId, ident);
			}
			else
			{
				envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(C2LocatorImpl, "OnRestore() stream is null. session(%s)"), ident.name.c_str());
			}

			//add to watch
			long expirationTime = transferSessionPrx->getLeaseLeft(context);
			envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "restore TransferSession(%s) from db and add to watch(%dms)"), strTransferId.c_str(), expirationTime);
			_env._watch.watch(ident, expirationTime);
		}
		catch(::Ice::UnmarshalOutOfBoundsException &)
		{
			envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "caught UnmarshalOutOfBoundsException when restore TransferSession(%s) from db"), strTransferId.c_str());
			removeTransferSession(ident);
		}
		catch(::Ice::Exception &ex)
		{
			envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "caught [%s] when restore TransferSession(%s) from db"), ex.ice_name().c_str(), strTransferId.c_str());
			removeTransferSession(ident);
		}
		catch(...)
		{
			envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "restore TransferSession(%s) from db caught error"), strTransferId.c_str());
			removeTransferSession(ident);
		}
	}

	_portMgr.restore();
	_clientMgr.restore();
}

//impl of BaseService
::std::string C2LocatorImpl::getAdminUri(const ::Ice::Current& c)
{
	return "";
}

::TianShanIce::State C2LocatorImpl::getState(const ::Ice::Current& c)
{
	return ::TianShanIce::stInService;
}

//impl of ReplicaSubscriber
void C2LocatorImpl::updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amd, const ::TianShanIce::Replicas& replicas, const ::Ice::Current& c)
{
	std::string caller = (c.con)->toString();
	caller = caller.substr(caller.find_last_of("=")+1, caller.size());

	// monitor the capacity changing
	Ice::Long currentTotalBw = _env.portPerf.totalBw;

	std::vector<ZQTianShan::CDN::TransferPortManager::PortInfo> availablePorts; // for the status of the ports
	TianShanIce::Replicas::const_iterator it;
	for(it = replicas.begin(); it != replicas.end(); ++it)
	{
		TianShanIce::SCS::TransferPort port;

		port.isUp					= false;
		port.capacity				= 0;
		port.activeBandwidth		= 0 ;
		port.activeTransferCount	= 0;
		port.enabled				= 0;
		port.penalty				= 0;

		port.name = it->groupId + "/" + it->replicaId;
		port.isUp = (it->replicaState == TianShanIce::stInService);
		TianShanIce::Properties::const_iterator itProp;

		itProp = it->props.find(STREAMERPROP_VOLUMENETID);
		if(itProp != it->props.end() && !itProp->second.empty())
		{
			port.name = itProp->second + "/" + port.name;
		}

		itProp = it->props.find(STREAMERPROP_ADDRESSIPV4);
		if(itProp != it->props.end())
		{
			ZQ::common::stringHelper::SplitString(itProp->second, port.addressListIPv4, " ");
		}

		itProp = it->props.find(STREAMERPROP_ADDRESSIPV6);
		if(itProp != it->props.end())
		{
			ZQ::common::stringHelper::SplitString(itProp->second, port.addressListIPv6, " ");
		}

		itProp = it->props.find(STREAMERPROP_CAPACITY);
		if(itProp == it->props.end())
		{
			envExtLog(ZQ::common::Log::L_WARNING, CLOGFMT(C2LocatorImpl, "updateReplica() prop[%s] of port[%s] missed"), STREAMERPROP_CAPACITY, port.name.c_str());
			continue;
		}

		port.capacity = StringToLong(itProp->second);

		itProp = it->props.find(STREAMERPROP_ACTIVETRANSFERCOUNT);
		if(itProp != it->props.end())
		{
			port.activeTransferCount = StringToLong(itProp->second);
		}
		else
		{
			port.activeTransferCount = 0;
		}

		itProp = it->props.find(STREAMERPROP_ACTIVEBANDWIDTH);
		if(itProp != it->props.end())
		{
			port.activeBandwidth = StringToLong(itProp->second);
		}
		else
		{
			port.activeTransferCount = 0;
		}

		port.streamService = _env._communicator->proxyToString(it->obj);

		port.enabled = true;
		std::string hostName;
		if(!port.streamService.empty()) {
			std::vector<std::string> epItems;
			ZQ::common::stringHelper::SplitString(port.streamService, epItems, " \t\n\r", " \t\n\r");
			for(size_t i = 0; i < epItems.size(); ++i)
			{
				if(epItems[i] == "-h")
				{
					// check the next item
					if(i + 1 < epItems.size())
						hostName = epItems[i + 1];

					break;
				}
			}
		}

		if(!hostName.empty()) 
		{
			if(!checkRemoteHealth(hostName))
			{
				envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "updateReplica() host[%s] is unhealthy, set port[%s] down"), hostName.c_str(), port.name.c_str());
				port.isUp = false;
			}
		} 
		else 
		{
			envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "updateReplica() failed to parse the proxystring[%s] of port[%s]"), port.streamService.c_str(), port.name.c_str());
		}

		_portMgr.updatePortStatus(port);
		//portSnmpMgr_.updatePort(port.name);
		_env.updatePort(port);
		if (port.isUp && port.capacity > 0)
		{
			ZQTianShan::CDN::TransferPortManager::PortInfo pInfo;
			if(_portMgr.queryPort(port.name, pInfo))
				availablePorts.push_back(pInfo);
		}
	}

    _env.refreshTransferPortTable();
    _env.refreshClientTransferTable();

	// report the ports status
	std::ostringstream buf;
	for(size_t i = 0; i < availablePorts.size(); ++i)
	{
		ZQTianShan::CDN::TransferPortManager::PortInfo& pInfo = availablePorts[i];
		buf << pInfo.name << "["
			<< ZQ::common::Text::join(pInfo.addressListIPv4) << "/" << ZQ::common::Text::join(pInfo.addressListIPv6) << ";"
			<< "usage" << pInfo.activeBandwidth << "@" << pInfo.activeTransferCount << "/" << pInfo.capacity 
			<< "], ";
	}

	envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "updateReplica() %d port(s) updated by %s: %s"), availablePorts.size(), caller.c_str(), buf.str().c_str());
	
	// monitor the capacity changing
	Ice::Long bwDelta = _env.portPerf.totalBw - currentTotalBw;
	if (bwDelta != 0)
		envExtLog(ZQ::common::Log::L_INFO, CLOGFMT(C2LocatorImpl, "updateReplica() total bandwidth %s: current(%lld) delta(%lld)"), (bwDelta > 0 ? "increased" : "decreased"), _env.portPerf.totalBw, bwDelta);
	else
		envExtLog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2LocatorImpl, "updateReplica() total bandwidth: %lld"), _env.portPerf.totalBw);

	amd->ice_response(_env.replicaReportIntervalSec);
}

//impl of TimeoutObj
//void C2LocatorImpl::OnTimer(const ::Ice::Current& c)

//impl of ZQ NativeThread
int C2LocatorImpl::run()
{
	try {
		InitEventSink();
	} catch (const Ice::Exception& ex) {
		envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "run() Failed to register StreamEventHandler to event channel, exception(%s)"), ex.ice_name().c_str());
	} catch (...) {
		envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "run() Failed to register StreamEventHandler to event channel, exception(Unknown)"));
	}

	// do a full check
	envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "run() do the full check for the expired sessions."));
	int nExpired = 0;
	Freeze::EvictorIteratorPtr evicIter = _env._eC2TransferSession->getIterator("", _env._eC2TransferSession->getSize());
	const Ice::Context context;
	while (evicIter && evicIter->hasNext())
	{
		::Ice::Identity ident = evicIter->next();
		try {
			TransferSessionPrx transferSessionPrx = C2IdentityToObjEnv(_env, TransferSession, ident);
			if (!transferSessionPrx)
			{
				envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "run() transfer session(%s) not exist."), ident.name.c_str());
				continue;
			}
			//check session state
			::TianShanIce::State sessState = transferSessionPrx->getState(context);
			if (sessState == ::TianShanIce::stOutOfService)//destroy session
			{
				envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "transfer session(%s) is out of service now, destroy it."), ident.name.c_str());
				try {
					destroy(ident);
				} catch(...) {
				}
				++nExpired;
			}
		}
		catch(::Ice::Exception &ex)
		{
			envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "run() caught [%s] when check the state of transfer session(%s)"), ex.ice_name().c_str(), ident.name.c_str());
		}
		catch(...)
		{
			envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "run() Unexpected exception when check the state of transfer session(%s)"), ident.name.c_str());
		}
	}
	envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "run() Full check complete. %d sessions expired."), nExpired);

	while (!_quit)
	{
		SYS::SingleObject::STATE ret = _env.hExpiredSessionNotifier.wait(5000);
		if(_quit) { // thread exit
			break;
		} 

		if (ret == SYS::SingleObject::SIGNALED) { // session expired
			std::vector<Ice::Identity> expiredSessions = _env.getExpiredSessions();
			if(!expiredSessions.empty()) {
				envExtLog(::ZQ::common::Log::L_DEBUG, CLOGFMT(C2LocatorImpl, "run() %d sessions are reported expired."), expiredSessions.size());
				for(size_t i = 0; i < expiredSessions.size(); ++i)
				{
					try { destroy(expiredSessions[i]); } catch (...) {}
				}
			}
			continue;
		} 
		else if (ret == SYS::SingleObject::UNKNOWN) { // unexpected, but ignore it
			continue;
		}
	} // while(1)
	envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(C2LocatorImpl, "run() thread exit"));
	return 1;
}

ZQTianShan::CDN::ClientManager& C2LocatorImpl::getClientManager()
{
	return _clientMgr;
}
ZQTianShan::CDN::TransferPortManager& C2LocatorImpl::getTransferPortManager()
{
	return _portMgr;
}

void C2LocatorImpl::post(const ::std::string& category, ::Ice::Int eventId, const ::std::string& eventName, const ::std::string& stampUTC, const ::std::string& sourceNetId, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{ // to be removed
}

void C2LocatorImpl::removeTransferSession(const Ice::Identity &ident)
{
	TransferSessionPrx transferSessionPrx = C2IdentityToObjEnv(_env, TransferSession, ident);
	if (!transferSessionPrx)
		return;

	::Ice::Context context;
	TransferSessionProp prop = transferSessionPrx->getProps(context);
	::std::string strTransferId = prop.transferId;
	if (strTransferId.empty())
	{
		envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(C2LocatorImpl, "removeTransferSession() session not in map"));
	}
	else
	{
		{//remove from memory map
			ZQ::common::MutexGuard gd(_indexLock);
			_transferIdMap.erase(strTransferId);
			removeStreamIndexBySession(ident);
		}

		//remove from evictor map
		_env._eC2TransferSession->remove(ident);
		envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(C2LocatorImpl, "removeTransferSession() session(%s) remove from map"), strTransferId.c_str());
	}
}

void C2LocatorImpl::addStreamIndex(const std::string& streamId, const Ice::Identity& sess)
{
	streamToSessionIndex_[streamId] = sess;
	envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(C2LocatorImpl, "addStreamIndex() build index:stream(%s) to session(%s)."), streamId.c_str(), sess.name.c_str());
}
void C2LocatorImpl::removeStreamIndexBySession(const Ice::Identity& sess)
{
	StreamToSessionMap::iterator it = streamToSessionIndex_.begin();
	for(; it != streamToSessionIndex_.end(); ++it)
	{
		if(it->second == sess) {
			envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(C2LocatorImpl, "removeStreamIndexBySession() remove index:stream(%s) to session(%s)."), it->first.c_str(), it->second.name.c_str());
			streamToSessionIndex_.erase(it);
			return;
		}
	}
	envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "removeSessionFromStreamIndex() no stream was bound to session(%s)"), sess.name.c_str());
}

bool C2LocatorImpl::querySessionByStream(const std::string& streamId, Ice::Identity& sess) const
{
	ZQ::common::MutexGuard gd(_indexLock);
	StreamToSessionMap::const_iterator it = streamToSessionIndex_.find(streamId);
	if(it != streamToSessionIndex_.end())
	{
		sess = it->second;
		return true;
	}
	else
	{
		return false;
	}
}
class SystemCommandThread: public ZQ::common::ThreadRequest {
public:
	SystemCommandThread(const std::string& cmd,const std::string& arg, ZQ::common::NativeThreadPool& pool)
		:_cmd(cmd), _arg(arg), _bDone(false),ZQ::common::ThreadRequest(pool) 
#ifdef ZQ_OS_LINUX
		,_childPid(0)
#endif
	{
	}

	virtual int run() 
	{
		_bDone = false;
		_result = 0;
#ifdef ZQ_OS_LINUX
		//system(_cmd.c_str());
		_childPid = fork();
		if( _childPid == 0 )
		{//child process
			execlp(_cmd.c_str(),_cmd.c_str(),_arg.c_str());
			_result = errno;

		}
		else if( _childPid < 0 )
		{
			return -1;//error occurred
		}
		//parent process
		bool bChildExit = false;
		while(!_bDone)
		{
			if( waitpid(_childPid,&_result,WNOHANG) == _childPid )
			{
				bChildExit = true;
				break;
			}
			ZQ::common::delay(100);
		}
		if(!bChildExit)
		{
			kill(_childPid,SIGKILL);
			_result = -1;
		}
#endif

		int result = _result;
		_bDone = true;		
		_event.signal();
		return result;
	}

	void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}

	void stopRunning() 
	{
		_bDone = true;
		_event.wait();		
	}
	int getResult() const {
		return _bDone ? _result : -10000;
	}

	bool wait(timeout_t timeout)
	{
		return SYS::SingleObject::SIGNALED == _event.wait(timeout);
	}
private:
	std::string _cmd;
	std::string _arg;
	int _result;
	bool _bDone;
	SYS::SingleObject _event;
#ifdef ZQ_OS_LINUX
	pid_t	_childPid;
#endif
};
bool C2LocatorImpl::checkRemoteHealth(const std::string& hostName) {
	if(_env._conf.checkRH.script.empty()) {
		return true;
	}
	int64 thisCheckStamp = ZQ::common::now();
	RemoteHealthRecord::RHStatus lastCheckResult;
	if(_rhRecord.get(hostName, lastCheckResult)) {
		if(thisCheckStamp - lastCheckResult.stamp > _env._conf.checkRH.interval * _env.replicaReportIntervalSec * 1000) {
			//need check
		} else {
			return lastCheckResult.status == 0;
		}
	}
	lastCheckResult.hostName = hostName;
	lastCheckResult.stamp = thisCheckStamp;
	lastCheckResult.status = -1;

	static ZQ::common::NativeThreadPool checkHealthPool(1);
	std::string cmd = _env._conf.checkRH.script;// + " " + hostName;
	SystemCommandThread *pChecker = new SystemCommandThread(cmd,hostName,checkHealthPool);
	pChecker->start();
	envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(C2LocatorImpl, "checkRemoteHealth() Executing command (%s)..."), cmd.c_str());
	
	if(pChecker->wait(_env._conf.checkRH.timeout)) {
		lastCheckResult.status = pChecker->getResult();
		envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "checkRemoteHealth() Execute command (%s) got result:%d"), cmd.c_str(), lastCheckResult.status);
	} else {
		envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "checkRemoteHealth() The command (%s) didn't return in %d msec. Kill it."), cmd.c_str(), _env._conf.checkRH.timeout);
		pChecker->stopRunning();
		lastCheckResult.status = -10000;
	}
	_rhRecord.set(lastCheckResult);
	return lastCheckResult.status == 0;
}
// impl of StreamEventHandler
StreamEventHandler::StreamEventHandler(ZQTianShan::CDN::C2Env& env)
:_env(env)
{
}

StreamEventHandler::~StreamEventHandler()
{
	releaseLocator();
}

void StreamEventHandler::bindLocator(C2LocatorImpl* loc)
{
	ZQ::common::MutexGuard guard(lockLocator_);
	locator_ = loc;
}
void StreamEventHandler::releaseLocator()
{
	ZQ::common::MutexGuard guard(lockLocator_);
	locator_ = NULL;
}
void StreamEventHandler::ping(::Ice::Long timestamp, const ::Ice::Current&)
{ // ignore
}
void StreamEventHandler::OnEndOfStream(const std::string &proxy, const std::string &id, const TianShanIce::Properties &, const ::Ice::Current&) const
{
	ZQ::common::MutexGuard guard(lockLocator_);
	if(!locator_)
	{
		envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(StreamEventHandler, "OnEndOfStream() EndOfStream detected but no locator is bound with stream(%s)"), id.c_str());
		return;
	}
	Ice::Identity ident;
	if(locator_->querySessionByStream(id, ident))
	{
		envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(C2LocatorImpl, "sess[%s] OnEndOfStream() stream(%s)"), ident.name.c_str(), id.c_str());
		try{
			TianShanIce::SCS::TransferSessionPrx sess = C2IdentityToObjEnv(_env, TransferSession, ident);
			if(sess)
			{
				// destroy transfer session immediately
				envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(C2LocatorImpl, "sess[%s] OnEndOfStream() removed transfer session immediately. stream(%s)"), ident.name.c_str(), id.c_str());
				sess->renew(0);
			}
			else
			{
				envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "OnEndOfStream() Got EndOfStream event but session(%s) is not exist. stream(%s)"), ident.name.c_str(), id.c_str());
			}
		}
		catch(::Ice::Exception &ex)
		{
			envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "OnEndOfStream() caught [%s] when process the EndOfStream event. session(%s), stream(%s)"), ex.ice_name().c_str(), ident.name.c_str(), id.c_str());
		}
		catch(...)
		{
			envExtLog(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "OnEndOfStream() Unexpected exception when process the EndOfStream event. session(%s), stream(%s)"), ident.name.c_str(), id.c_str());
		}
	}
	else
	{
		envExtLog(::ZQ::common::Log::L_WARNING,CLOGFMT(C2LocatorImpl, "OnEndOfStream() Got EndOfStream event but no session is bound with stream(%s)."), id.c_str());
	}
}
void StreamEventHandler::OnBeginningOfStream(const std::string &proxy, const std::string &id, const TianShanIce::Properties &, const ::Ice::Current&) const
{ // ignore
}
void StreamEventHandler::OnStateChanged(const std::string &proxy, const std::string &id, TianShanIce::Streamer::StreamState prevState, TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties &, const ::Ice::Current&) const
{ // ignore
}
void StreamEventHandler::OnSpeedChanged(const std::string &proxy, const std::string &id, Ice::Float prevSpeed, Ice::Float currentSpeed, const TianShanIce::Properties &, const ::Ice::Current&) const
{ // ignore
}
void StreamEventHandler::OnExit(const std::string &proxy, const std::string &id, Ice::Int exitCode, const std::string &reason, const ::Ice::Current&) const
{ // ignore
}
void StreamEventHandler::OnExit2(const std::string &proxy, const std::string &id, Ice::Int exitCode, const std::string &reason ,const ::TianShanIce::Properties& props, const ::Ice::Current&) const
{ // ignore
}

}//namespace SCS
}// namespace TianShanIce
