
// EGH_MQTT.cpp : Defines the entry point for the DLL application.
//
#include <ZQ_common_conf.h>
#include <EventGwHelper.h>
#include "MQTTEventHelper.h"
#include "MQTTMessageTransporter.h"
#include <FileLog.h>
#include <TsEvents.h>
#include "FileSystemOp.h"

static void reset();
static void clear(); // clear the global resource in the module
#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		reset();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		clear();
		break;
	}
	return TRUE;
}
#endif

#ifdef EGH_MQTT_EXPORTS
#define EGH_MQTT_API __EXPORT
#else
#define EGH_MQTT_API __DLLRTL
#endif


#define THIS_MODULE_NAME "EGH_MQTT"

ZQ::common::Config::Loader<EventGateway::MQTT::MQTTConfig> gConfig(THIS_MODULE_NAME ".xml");

static EventGateway::IEventGateway* _gw;
static ZQ::common::FileLog* _plog;

// target manager
class TargetManager
{
public:
	explicit TargetManager(const EventGateway::MQTT::MQTTConfig::Servers& servers);
	~TargetManager();

	typedef std::pair< std::string, std::string > Target;
	static Target parse(const std::string& target);

	EventGateway::MQTT::MessageChannel* get(const std::string& target);
	void clear();
private:
	EventGateway::MQTT::MQTTConfig::Servers _serversConf;
	EventGateway::MQTT::TransportManager* _transportMgr;
};
static TargetManager* _targetMgr;

struct HelperInfo
{
	EventGateway::IGenericEventHelper* helper;
	std::string topic;
	std::string name;
	HelperInfo(EventGateway::IGenericEventHelper* h, const std::string& t, const std::string& n)
		:helper(h), topic(t), name(n)
	{
	}
};

static std::vector<HelperInfo> _eventHelpers;

static void reset()
{
	_gw = NULL;
	_plog = NULL;
	_targetMgr = NULL;
	_eventHelpers.clear();
}
static void clear() // clear the global resource in the module
{
	if(_targetMgr) {
		try{ delete _targetMgr; }catch(...){}
		_targetMgr = NULL;
	}
	if(_plog)
	{
		try {
			_plog->flush();
			delete _plog;
		} catch(...){}
		_plog = NULL;
	}

	reset();
}

static std::string pathCat(const std::string &dir, const std::string& sub)
{
	std::string path;
	path.reserve(dir.size() + 1 + sub.size());
	if(!dir.empty() && dir[dir.size() - 1] != FNSEPC)
	{
		path = dir + FNSEPS;
	}
	else
	{
		path = dir;
	}

	path += sub;

	return path;
}

extern "C"
{
	EGH_MQTT_API bool EventGw_Module_Entry_init(EventGateway::IEventGateway* gateway)
	{
		using namespace EventGateway::MQTT;
 		reset();
		_gw = gateway;

		_gw->superLogger()(ZQ::common::Log::L_INFO, "====================EGH_MQTT INIT====================");

		// load config
		gConfig.setLogger(&_gw->superLogger());
		if(!gConfig.loadInFolder(_gw->getConfigFolder().c_str()))
		{
			return false;
		}
		// validate the safe store path
		if(!FS::createDirectory(gConfig.safeStorePath))
		{
			_gw->superLogger()(ZQ::common::Log::L_ERROR, "Bad safe store path [%s]", gConfig.safeStorePath.c_str());
			return false;
		}

		// create module's log
		std::string modLogFilePath = _gw->getLogFolder() + THIS_MODULE_NAME ".log";
		try{
			//            _plog = NULL;
			_plog = new ZQ::common::FileLog(
				modLogFilePath.c_str(),
				gConfig.logLevel,
				gConfig.logCount,
				gConfig.logFileSize
				);
		}catch (...) {
			_gw->superLogger()(ZQ::common::Log::L_ERROR, "Caught unknown exception during create FileLog [%s]", modLogFilePath.c_str());
			return false;
		}
		

		// target manager
		_targetMgr = new TargetManager(gConfig.servers);

		{ // create event helpers

			for(MQTTConfig::HandlersConfig::const_iterator it = gConfig.handlers.begin(); it != gConfig.handlers.end(); ++it)
			{
				if(!it->enabled)
					continue;

				MessageChannel* msgChannel = _targetMgr->get(it->target);
				if(msgChannel)
				{
					EventHelper* helper = new EventHelper((*_plog), msgChannel);
					if(helper->init(it->filter, it->msgTemplate))
					{
						_eventHelpers.push_back(HelperInfo(helper, it->source, it->name));
						(*_plog)(ZQ::common::Log::L_INFO, CLOGFMT(ModuleInit, "Event handler [%s/%p] is setup."), it->name.c_str(), helper);
					}
					else
					{
						(*_plog)(ZQ::common::Log::L_ERROR, CLOGFMT(ModuleInit, "Failed to init event handler [%s]."), it->name.c_str());
						delete helper;
					}
				}
				else // no channel availble
				{
					(*_plog)(ZQ::common::Log::L_WARNING, CLOGFMT(ModuleInit, "No channel found for target [%s]."), it->target.c_str());
				}
			}
		}

		// subscribe
		{ // content event helper
			for(size_t i = 0; i < _eventHelpers.size(); ++i)
			{
				_gw->subscribe(_eventHelpers[i].helper, _eventHelpers[i].topic);
				(*_plog)(ZQ::common::Log::L_INFO, CLOGFMT(ModuleInit, "Event handler [%s/%p] subscribe [%s]"), _eventHelpers[i].name.c_str(), _eventHelpers[i].helper, _eventHelpers[i].topic.c_str());
			}

			(*_plog)(ZQ::common::Log::L_INFO, CLOGFMT(ModuleInit, "Subscribe %d event helpers."), _eventHelpers.size());
		}
		return true;
	}

	EGH_MQTT_API void EventGw_Module_Entry_uninit()
	{
		// unsubscribe
		{ // content event helper
			for(size_t i = 0; i < _eventHelpers.size(); ++i)
			{
				_gw->unsubscribe(_eventHelpers[i].helper, _eventHelpers[i].topic);
				(*_plog)(ZQ::common::Log::L_INFO, CLOGFMT(ModuleUninit, "Event handler [%s/%p] unsubscribe [%s]"), _eventHelpers[i].name.c_str(), _eventHelpers[i].helper, _eventHelpers[i].topic.c_str());
			}
		}

		// we need uninit jvm manually
		if(_targetMgr) {
			try{ delete _targetMgr; }catch(...){}
			_targetMgr = NULL;
		}
        clear();
	}
}

/// definition of TargetManager

TargetManager::TargetManager(const EventGateway::MQTT::MQTTConfig::Servers& servers)
: _serversConf(servers), _transportMgr(NULL)
{
}
TargetManager::~TargetManager()
{
	clear();
}

typedef std::pair< std::string, std::string > Target;
Target TargetManager::parse(const std::string& target)
{
	Target t;
	// parse the string target into structured
	// format: 'server.channel'
	std::string::size_type pos = target.find('.');
	if(0 == pos)
	{ // .channel
		t.second = target.substr(1); // channel
	}
	else if(std::string::npos == pos)
	{ // server.
		t.first = target; // server
	}
	else
	{ // server.channel
		t.first = target.substr(0, pos); // server
		t.second = target.substr(pos + 1); // channel
	}
	return t;
}

EventGateway::MQTT::MessageChannel* TargetManager::get(const std::string& target)
{
	using namespace EventGateway::MQTT;
	using namespace ZQ::common;

	(*_plog)(Log::L_DEBUG, CLOGFMT(TargetManager, "try get target [%s]"), target.c_str());
	if(NULL == _transportMgr)
	{
		try
		{
			_transportMgr = new EventGateway::MQTT::TransportManager((*_plog));
		}
		catch(...)
		{
			(*_plog)(Log::L_ERROR, CLOGFMT(TargetManager, "Failed to create TransportManager"));
			return NULL;
		}
	}
	std::string server, channel;
	// target -> server, channel
	Target t = parse(target);
	server = t.first;
	channel = t.second;

	MessageTransporter* transporter = _transportMgr->getTransporter(server);
	if(transporter)
	{ // server been connected
		MessageChannel* msgChannel = transporter->getChannel(channel);
		if(msgChannel)
		{ // Got the channel;
			(*_plog)(Log::L_DEBUG, CLOGFMT(TargetManager, "Found target [%s] at [%p]"), target.c_str(), msgChannel);
			return msgChannel;
		}
		else
		{ // try create the channel
			// get the config firstly
			if(_serversConf.find(server) != _serversConf.end())
			{
				ServerConfig& serverConf = _serversConf[server];
				if(serverConf.channels.find(channel) != serverConf.channels.end())
				{
					ChannelConfig& channelConf = serverConf.channels[channel];
					msgChannel = transporter->createChannel(channelConf);
					// log the result
					(*_plog)(Log::L_DEBUG, CLOGFMT(TargetManager, "Create target [%s] at [%p]"), target.c_str(), msgChannel);
					return msgChannel;
				}
				else
				{ // no channel config availble
					(*_plog)(Log::L_DEBUG, CLOGFMT(TargetManager, "No channel config for target [%s]"), target.c_str());
					return NULL;
				}
			}
			else
			{ // something wrong badly! corrupt data
				(*_plog)(Log::L_ERROR, CLOGFMT(TargetManager, "Bad configuration. No config of server [%s]"), server.c_str());
				return NULL;
			}
		}
	}
	else
	{
		// try connect the server and the channel
		if(_serversConf.find(server) != _serversConf.end())
		{
			ServerConfig& serverConf = _serversConf[server];
			if( serverConf.channels.find(channel) != serverConf.channels.end())
			{
				ChannelConfig& channelConf = serverConf.channels[channel];

				std::string safeStorePath = pathCat(gConfig.safeStorePath, server);
				FS::createDirectory(safeStorePath);
				try
				{
					transporter = _transportMgr->createTransporter(serverConf, safeStorePath);
					if(transporter)
					{
						MessageChannel* msgChannel = transporter->createChannel(channelConf);
						(*_plog)(Log::L_DEBUG, CLOGFMT(TargetManager, "Create target [%s] at [%p]"), target.c_str(), msgChannel);
						return msgChannel;
					}
					else
					{
						(*_plog)(Log::L_ERROR, CLOGFMT(TargetManager, "Can't create Messagetransporter for target[%s]"), target.c_str());
						return NULL;
					}
				}
				catch(const Ice::Exception& e)
				{
					(*_plog)(Log::L_ERROR, CLOGFMT(TargetManager, "Caught %s during initializing MQTT MesssageBase."), e.ice_name().c_str());
					return NULL;
				}
				catch(...)
				{
					(*_plog)(Log::L_ERROR, CLOGFMT(TargetManager, "Caught unknown exception during initializing MQTT MesssageBase."));
					return NULL;
				}
			}
			else
			{ // no channel config
				(*_plog)(Log::L_DEBUG, CLOGFMT(TargetManager, "No channel config  for target [%s]"), target.c_str());
				return NULL;
			}
		}
		else
		{ // no server config
			(*_plog)(Log::L_DEBUG, CLOGFMT(TargetManager, "No server config  for target [%s]"), target.c_str());
			return NULL;
		}
	}
}
void TargetManager::clear()
{
	if(_transportMgr)
	{
		try{ delete _transportMgr; }catch(...){}
		_transportMgr = NULL;
	}
}
