#include "EventClientTest.h"
#include "EventClientTestMain.h"

int parserCmd(const std::string & strGetFromCmd, EventClientTestMain& stTestMain);

int main(int argc, char* argv[])
{
	std::string strGetFromCmd;
	EventClientTestMain stTest(argc, argv);

	while (stTest.getRuning())
	{
		strGetFromCmd.clear();
		std::getline(std::cin, strGetFromCmd);
        parserCmd(strGetFromCmd, stTest);
	    stTest.printCommandLine();
	}

	return true;
}

int parserCmd(const std::string & strGetFromCmd, EventClientTestMain& stTestMain)
{
	const std::string delims(" \t,.;");
	std::string strCommand;
	std::string strOption;
	std::string strAttributeTopic;
	std::string strAttribute;
	const int nCompOK = 0;
	std::string::size_type begIdex, endIdx = 0;
	int nRev = true;

	for (int nLoop = 0; nLoop < 4; ++nLoop)
	{

		//first word
		begIdex = strGetFromCmd.find_first_not_of(delims, endIdx);
		if (begIdex == std::string::npos)
		{
			nRev = false;
			break;
		}

		endIdx = strGetFromCmd.find_first_of(delims, begIdex);
		if (endIdx == std::string::npos)
		{
			endIdx = strGetFromCmd.length();
		}

		if (0 == nLoop)
			strCommand = strGetFromCmd.substr(begIdex, endIdx - begIdex);
		else if(1 == nLoop)
		{
			if (nCompOK == strCommand.compare("connect"))
			{
				strOption = strGetFromCmd.substr(begIdex, strGetFromCmd.length() - begIdex);
				break;
			}
			else
				strOption = strGetFromCmd.substr(begIdex, endIdx - begIdex);
		}
		else if(2 == nLoop)
			if (nCompOK == strCommand.compare("publish"))
			{
				strAttribute = strGetFromCmd.substr(begIdex, strGetFromCmd.length() - begIdex);
				break;
			}else 
				strAttribute = strGetFromCmd.substr(begIdex, endIdx - begIdex);
		else
			strAttributeTopic = strGetFromCmd.substr(begIdex, strGetFromCmd.length() - begIdex);
	}

	if (nCompOK == strCommand.compare("create"))//create command
	{
		if (nCompOK == strOption.compare("adapter") || nCompOK == strOption.compare("-a"))
			nRev = stTestMain.createAdapter(strAttribute, strAttributeTopic);
		else if(nCompOK == strOption.compare("subscriber") || nCompOK == strOption.compare("-s"))
			nRev = stTestMain.createSubscriberForTopic(strAttribute);
		if(nCompOK == strOption.compare("pubscriber") || nCompOK == strOption.compare("-p"))
			nRev = stTestMain.createPublisherForTopic(strAttribute);//leave number alone ,temporary.
	}
	else if (nCompOK == strCommand.compare("createps"))	//createps command
	{
		nRev = stTestMain.createPubAndSub(strOption);
	}
	else if (nCompOK == strCommand.compare("publish"))	//publish command
	{
		nRev = stTestMain.publishMessage(strOption, strAttribute);
	}
	else if (nCompOK== strCommand.compare("help"))	//help command
	{
		nRev = stTestMain.listHelpCmd();
	}
	else if (nCompOK == strCommand.compare("connect"))	//connect command
	{
		nRev = stTestMain.connectToEndpoint(strOption);
	}
	else if (nCompOK == strCommand.compare("disconnect"))//disconnect command
	{
		nRev = stTestMain.disconnectToEndpoint(strOption);
	}
	else if (nCompOK == strCommand.compare("quit"))//quit command
	{
		stTestMain.setRunning(false);
		nRev = true;
	}

	cout<<endl;
	if (nRev)
		std::cout<<"execute\t"<<strGetFromCmd<<"\t succeed\n"<<std::ends<<endl;
	else
		std::cout<<"execute\t"<<strGetFromCmd<<"\t failed\n"<<std::ends<<std::endl;

	cout<<endl;
	return nRev;
}

MyCppSession::MyCppSession(ZQ::common::Log& log, EventClient& client, const std::string& topicName, TianShanIce::Properties& properties, bool asProducer/* =true*/, bool asConsumer/*=false*/, EventClientTestMain *testMain /*= NULL*/)
throw(::ZQ::EventClient::EventClientException)
:GenericEventSession(log, client, topicName, properties, asProducer, asConsumer), _log(log)
{
	_testMain = testMain;
	_sessionId = getSessionId();
	_topicName = getSessionTopicName();
	if(_asProducer)
		_strId += "Publisher ";

	if (_asConsumer && _asProducer)
		_strId += "and Subscriber ";
	else if(_asConsumer && !_asProducer)
		_strId += "Subscriber ";
}

MyCppSession::~MyCppSession(){}

void MyCppSession::OnConnected(const std::string& notice)
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "[%s] OnConnected called, notice [%s], sessionId[%s], _topicName[%s]"), _strId.c_str(), notice.c_str(), _sessionId.c_str(), _topicName.c_str());
	_testMain->printCommandLine();
}

void MyCppSession::OnConnectionLost(const std::string& notice)
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "[%s] OnConnectionLost called, notice [%s], sessionId[%s], _topicName[%s]"), _strId.c_str(), notice.c_str(), _sessionId.c_str(), _topicName.c_str());
	_testMain->printCommandLine();
}

void MyCppSession::OnMessage(Message& message)
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(EventClientTest, "[%s] OnMessage called, message.category [%s], message.id [%d],sessionId[%s], _topicName[%s]"), _strId.c_str(), message.category.c_str(), message.id, _sessionId.c_str(), _topicName.c_str());
}