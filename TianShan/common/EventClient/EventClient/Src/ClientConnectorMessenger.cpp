#include "ClientConnectorMessenger.h"

namespace ZQ {
	namespace EventClient {

///-----------------------------------------
///start of class ClientConnectorMessenger                               
///-----------------------------------------
ClientConnectorMessenger::ClientConnectorMessenger(ZQ::common::Log& log)
	:_log(log)
{
	Ice::InitializationData initData;
	_communicator  =  Ice::initialize(initData);
	_topicName     =  IceUtil::generateUUID();//make it unique
	_adapterId     =  IceUtil::generateUUID();
	_messagerId    =  IceUtil::generateUUID();
	_pAdapter      =  _communicator->createObjectAdapterWithEndpoints(_adapterId, "tcp");
	_messengerPrx  =  _pAdapter->addWithUUID(this);
	_publisherPrx  =  NULL;
	_waitBeatTime  =  1000;
	_minBeat      =   6;
	_pAdapter->activate();
	_hEvent.signal();
};

ClientConnectorMessenger::~ClientConnectorMessenger()
{
	try
	{
		_pAdapter->deactivate();
	}catch (::Ice::UserException & ex) {
	}
	catch (...){
	}
};

void ClientConnectorMessenger::ping(::Ice::Long timestamp, const ::Ice::Current& c)
{
	Ice::Context::const_iterator it = c.ctx.find("from");
	if(it != c.ctx.end() && it->second == _messagerId)
	{
		_hEvent.signal();
	}
};

int ClientConnectorMessenger::checkMessengerState(void)
{
	Ice::ObjectPrx prx = _communicator->stringToProxy(_topicManagerEndpoint);
	_topicMgr = IceStorm::TopicManagerPrx::uncheckedCast(prx);
	IceStorm::TopicPrx  topic;
	int nRev = true;
	Ice::Long stamp = ZQTianShan::now(); // generate the stamp
	Ice::Context ctx;
	ctx["from"] = _messagerId;

	_pAdapter->activate();
	try{
		_publisherPrx->ping(stamp, ctx);// first time _publisherPrx is null, and then get value
	}
	catch (::IceUtil::Exception & ex)
	{
		try{
			topic = _topicMgr->retrieve(_topicName);
		}catch(const IceStorm::NoSuchTopic& ){
			topic = _topicMgr->create(_topicName);
		}catch (...) {
		}

		try{
			Ice::ObjectPrx obj = topic->subscribeAndGetPublisher(IceStorm::QoS(), _messengerPrx);
			_publisherPrx = TianShanIce::Events::BaseEventSinkPrx::uncheckedCast(obj);
			_publisherPrx->ping(stamp, ctx);
		}catch (...){					
		}
	}catch (...){
	}

	nRev = checkMessengerResponse();
	return nRev;
};

int ClientConnectorMessenger::checkMessengerResponse(void)
{			
	SYS::SingleObject::STATE st = SYS::SingleObject::UNKNOWN;
	int nRev = true;
	st = _hEvent.wait(_waitBeatTime);

	switch(st)
	{
	case SYS::SingleObject::SIGNALED:
		resetLoseHeartBeat(0);
		break;

	case SYS::SingleObject::TIMEDOUT:
	default:
		++_loseHeartBeat;
		break;
	}

	if (_loseHeartBeat > _minBeat)
	{
		nRev = false;
		resetLoseHeartBeat(0);//next sequence check
	}

	_hEvent.wait(_waitBeatTime);//make next sequence be locked;
	return nRev;
}

long  ClientConnectorMessenger::resetLoseHeartBeat(long nLoseHeartBeat/* = 0*/)
{
	return (_loseHeartBeat = nLoseHeartBeat);
};

int ClientConnectorMessenger::setMessengerEndpoint(::std::string topicManagerEndpoint)
{
	_topicManagerEndpoint.empty();
	_topicManagerEndpoint = topicManagerEndpoint;

	return true;
}
//-------------------------
//end of class ClientConnectorMessenger                                  
//-------------------------

	}///endnamespace ZQ::EventClient
}///endnamespace ZQ