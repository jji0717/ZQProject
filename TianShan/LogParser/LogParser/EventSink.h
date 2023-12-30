#ifndef __ZQTianShan_EventSink_H__
#define __ZQTianShan_EventSink_H__

#include <ZQ_common_conf.h>
#include "MsgSenderInterface.h"
#include <vector>
#include <Pointer.h>

class IEventSender
{
public:
	virtual ~IEventSender(){}
    struct Event
    {
        std::string category;
        int eventId;
        std::string eventName;
        std::string stampUTC;
        std::string sourceNetId;
        typedef std::map<std::string, std::string> Properties;
        Properties params;
    };
    virtual void SendEvent(const std::vector<std::string>& targets,const Event& evnt, const MessageIdentity& mid) = 0;
};

typedef IEventSender::Event EventTemplate;


class IRawMessageSource
{
public:

	virtual ~IRawMessageSource(){}
    virtual bool open(const MessageIdentity& recoverPoint) = 0;
    // return the writen data size
    // len(IO) input as the buffer length, output as the required buffer size
    // terminal null will be add for every successful invocation.
    virtual int fetchNext(char* buf, int* len, MessageIdentity& mid) = 0;
    virtual void close() = 0;
	
private:
	std::string _key;
	MessageIdentity _mid;
public:
	void SetMessageIdentity(MessageIdentity mid){ _mid = mid;}
	void SetKey(const std::string& key){_key = key;}
	MessageIdentity& getMessageIdentity(){return _mid;}
	std::string getKey(){ return _key;}
};

#endif

