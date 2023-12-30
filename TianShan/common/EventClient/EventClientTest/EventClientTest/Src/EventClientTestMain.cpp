#include "EventClientTestMain.h"
#include "EventClientTest.h"
#include "Log.h"

static int listByTopicName(EventClient::sessionPropertysMap::iterator &it, ::ZQ::common::FileLog& zqlog);

EventClientTestMain::EventClientTestMain(int argc, char* argv[])
{
	_pClientHandle = NULL;
	setRunning(true);
	std::string prgName = argv[0];
	std::string strFileDest;
	_programName = prgName.substr(prgName.rfind("\\") + 1);

	if (argc == 3)
	{
		strFileDest = argv[2];
	}

	int nRev = true;
	_fileDest.clear();
	_fileDest = strFileDest;
	if (strFileDest.empty())
	{
		char szPathTemp[512] = {0};
		::GetModuleFileNameA(NULL, szPathTemp, 512);
		_fileDest = szPathTemp;
		_fileDest.replace(_fileDest.size() - 3, 3, "log");
	}

	_zqlog.open(_fileDest.c_str(), ::ZQ::common::Log::L_DEBUG);
	
	_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "create logfile [%s] succeed, create eventclient succeed"), _fileDest.c_str());

	_ic = Ice::initialize();
	printCommandLine();
}

EventClientTestMain::~EventClientTestMain()
{
	if (NULL != _pClientHandle)
	{
		EventClient * pClientHandleTemp = _pClientHandle;
		_pClientHandle = NULL;
		delete pClientHandleTemp;
	}
}

int EventClientTestMain::createAdapter(const std::string & adapterName, const std::string & defaultProtocal)
{
	int nRev = true;
	try
	{
		_properties["reliability"] = "ordered";
		if (!defaultProtocal.empty())
		{
			_defaultEndpoint.clear();
			_defaultProtocal = defaultProtocal;
		}else
			_defaultProtocal = "default -p 10105";

		if (adapterName.empty())		
			_adapterName = "DemoIceEndpoint";
		else
			_adapterName = adapterName;

		_adapter = _ic->createObjectAdapterWithEndpoints(_adapterName, _defaultProtocal);
		_pClientHandle = new EventClient(_zqlog, _adapter, _properties);
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "creating adapterName[%s], endpoint[%s]"), _adapterName.c_str(), _defaultProtocal.c_str());
	}
	catch (...)
	{
		nRev = false;
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "creating adapterName[%s], endpoint[%s], failed"), _adapterName.c_str(), _defaultProtocal.c_str());
	}

    return nRev;
}

int EventClientTestMain::connectToEndpoint(const std::string & strEndpoint)
{
	int nRev = true;

	_defaultEndpoint.clear();
	_defaultEndpoint = "TianShanEvents/TopicManager:tcp ";

#define topicManagerEndpoint "TianShanEvents/TopicManager:tcp -h 192.168.81.117 -p 11000"

	if (strEndpoint.empty())
	{
		_defaultEndpoint +=  "-h 192.168.81.117 ";
	}else{
		_defaultEndpoint +=strEndpoint;
	}

	_defaultEndpoint += " -p 11000";

	if(!_pClientHandle->connect(_defaultEndpoint))
	{
		nRev = false;
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "connecting to EventChannel[%s] failed "), _defaultEndpoint.c_str());
	}else
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "connecting to EventChannel[%s]"), _defaultEndpoint.c_str());

	return nRev;
}

int EventClientTestMain::createSubscriberForTopic(const std::string & topicName)
{
	int nRev = true;
	MyCppSession *pSessionHandle = NULL;
	if (topicName.empty())
		return nRev;

	try
	{
		pSessionHandle = new MyCppSession(_zqlog, *_pClientHandle, topicName, _properties, false, true, this);
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "create Subscriber topicName [%s], sessionId[%s] succeed"), topicName.c_str(), pSessionHandle->getSessionId().c_str());
	}
	catch (EventClientException &)
	{
		nRev = false;
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "create Subscriber topicName [%s] failed"), topicName.c_str());
	}

	return nRev;
}

int EventClientTestMain::createPublisherForTopic(const std::string & topicName)
{
	int nRev = true;
	MyCppSession *pSessionHandle = NULL;
	if (topicName.empty())
	{
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "create pulisher  failed, topicname is empty"));
		std::cout<<"Error, topic name is empty"<<std::endl;
		return nRev;
	}

	try
	{
		pSessionHandle = NULL;
		pSessionHandle = new MyCppSession(_zqlog, *_pClientHandle, topicName, _properties, true, false, this);
		_mapPublish[topicName] = pSessionHandle; 
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "create pulisher for topicname [%s], sessionId[%s]  succeed"),topicName.c_str(), pSessionHandle->getSessionId().c_str());
	}
	catch (EventClientException &)
	{
		nRev = false;
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "create pulisher for topicname [%s]  failed"),topicName.c_str());
	}

	return nRev;
}

int EventClientTestMain::createPubAndSub(const std::string & topicName)
{
	int nRev = true;
	MyCppSession *pSessionHandle = NULL;
	if (topicName.empty())
	{
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "create pulisher  and subscriber  failed, topicname is empty"));
		std::cout<<"Error, topic name is empty"<<std::endl;
		return nRev;
	}

	try
	{
		pSessionHandle = NULL;
		pSessionHandle = new MyCppSession(_zqlog, *_pClientHandle, topicName, _properties, true, true, this);
		_mapPublish[topicName] = pSessionHandle; 
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "create pulisher and subscriber for topicname [%s], sessionId[%s]  succeed"),topicName.c_str(), pSessionHandle->getSessionId().c_str());
	}
	catch (EventClientException &)
	{
		nRev = false;
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "create pulisher for topicname [%s]  failed"), topicName.c_str());
	}

	return nRev;
}


static long g_nId = 0;
int EventClientTestMain::publishMessage(const std::string & topicName, const std::string & PubMessage)
{
	GenericEventSession::Message semdMessage = {10, "", "timestamp", "eventname"};
	semdMessage.property = _properties;
	MyCppSession *pSessionHandle = NULL;
	int nRev = true;

	std::map<std::string, typename MyCppSession* >::iterator pos = _mapPublish.find(topicName);
	if(pos == _mapPublish.end())
	{
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "publish Message [%s], topic [%s], failed, no such topic"), PubMessage.c_str(), topicName.c_str());
		return false;
	}

	int nThreadID = ::GetCurrentThreadId();
	char buf[256] = {0};
	sprintf(buf,"\t threadID=%d\t message=%s", nThreadID, PubMessage.c_str());
	semdMessage.category += buf;
	semdMessage.id = g_nId++;

	pSessionHandle = (*pos).second;
	nRev = pSessionHandle->sendMessage(semdMessage);

	std::string strSessionId = pSessionHandle->getSessionId();
	if (nRev)
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "publish Message [%s], topic [%s], sessionId[%s], threadID[%d] succeed "), PubMessage.c_str(), topicName.c_str(), strSessionId.c_str(), nThreadID);
	else
		_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "publish Message [%s], topic [%s], sessionId[%s], threadID[%d] failed "), PubMessage.c_str(), topicName.c_str(), strSessionId.c_str(), nThreadID);

	return nRev;
}

int EventClientTestMain::disconnectToEndpoint(const std::string & strEndpoint)
{
	int nRev = true;
	_pClientHandle->disconnect();

	_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "disconnect to tndpoint [%s] succeed "), _defaultEndpoint.c_str());
	return nRev;
}

int EventClientTestMain::listHelpCmd(void)
{
	int nRev = true;

	//create command
	std::string strListHelp = "create  [option]  [name]  [attributes]\n"\
		" option    \-a   adapter                     create EventClient adapter\n"\
		"           \-s   subscriber                   create the number of subscribers\n"\
		"           \-p   pubscriber                   create the number of pubscribers\n"\
		" name         \"adapter name\"           the name for EventClient adapter\n"\
		"              \"topic name\"             the topic of subscribers or pubscribers\n"\
		" attributes    \"number\"             the number for pubscribers or subscribers\n"\
		"               \"default protocal\"     the default protocal for adapter\n"\
		"                                         such as \" default -p 10105\"\n"\
		"\n";

	//createps command
	strListHelp +="createps  [topicname]\n"\
		" topicname      the topic of subscribers or pubscribers\n"\
		"                 createps topicname"\
		"\n";


	//connect command
	strListHelp +="connect  [destination]\n"\
		" destination      this property is used by client to connect to EventChannel\n"\
		"                  TianShanEvents\/TopicManager\:tcp \-h 192.168.81.113 \-p 11000\n"\
		"\n";

	//disconnect command
	strListHelp +="disconnect  [destination]\n"\
		" destination      this property is used by client to disconnected EventChannel\n"\
		"                  TianShanEvents\/TopicManager\:tcp \-h 192.168.81.113 \-p 11000\n"\
		"\n";

	//help command
	strListHelp +="help  [type]\n"\
		" type      this property is used to list support command\n"\
		"\n";

	//publish command
	strListHelp +="publish   [topicname]   [message]\n"\
		" topicname  \"topicname\"  the topicname for publish\n"\
		" message    \"message\"    the message for publish\n"\
		"\n";

	//quit command
	strListHelp +="quit               exit this program\n"\
		"\n";

	std::cout<<strListHelp<<ends;
	_zqlog(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "help command succeed"));
	return nRev;
}

int EventClientTestMain::printCommandLine(void)
{
	int nRev = true;
	cout<<endl;
	std::cout<<_programName<<">"<<ends;
	return nRev;
}
