#ifndef __ZQ_EventClientTestMain_H__
#define __ZQ_EventClientTestMain_H__

#include "EventClientTest.h"
#include "FileLog.h"

class EventClientTestMain
{
public:
	EventClientTestMain(int argc, char* argv[]);
	~EventClientTestMain();

	int createAdapter(const std::string & adapterName, const std::string & defaultProtocal);
	int connectToEndpoint(const std::string & strEndpoint);
	int createSubscriberForTopic(const std::string & topicName);
	int createPublisherForTopic(const std::string & topicName);
	int createPubAndSub(const std::string & topicName);
	int publishMessage(const std::string & topicName, const std::string & PubMessage);
	int disconnectToEndpoint(const std::string & strEndpoint);

	int  listHelpCmd(void);
	int  printCommandLine(void);
	
public:
	int  getRuning(void){return _nRuning;}
	int  setRunning(int nRuning){return (_nRuning = nRuning);}

private:
	Ice::CommunicatorPtr     _ic;
	Ice::ObjectAdapterPtr    _adapter;
	std::string              _adapterName; 
	std::string              _defaultProtocal;
	std::string              _defaultEndpoint;
	::TianShanIce::Properties  _properties;

private:

	std::string            _fileDest;
	std::string            _programName;
	int                   _nRuning;
	::ZQ::common::FileLog   _zqlog;
	EventClient *          _pClientHandle;
	std::map<std::string, MyCppSession * > _mapPublish;
};


#endif//__ZQ_EventClientTestMain_H__