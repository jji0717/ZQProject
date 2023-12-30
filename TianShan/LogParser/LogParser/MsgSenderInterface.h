#ifndef _MASSEAGE_SENDER_INTERFACE_H_
#define _MASSEAGE_SENDER_INTERFACE_H_
#pragma warning (disable:4786)
#pragma warning(disable:4503)
#include <ZQ_common_conf.h>
#include <map>
#include <string>


struct MessageIdentity {
    std::string source;
    int64 position;
    int64 stamp;
    MessageIdentity():position(0), stamp(0) {}
};

class IMsgSender;

typedef struct _tagMsgStructure
{
	int										id;				//Event ID
	std::string								category;		//Event Category
	std::string								timestamp;		//timestamp
	std::string								eventName;		//Event name such as 'SessionInService'
	std::string								sourceNetId;	//source net id,host name is recommended
	std::map<std::string,std::string>		property;	//Event properties
}MSGSTRUCT;


//void post(string category, int eventId, string eventName, string stampUTC, string sourceNetId, Properties params);

///initialize the plugin with input parameter
///@return true if initialize ok,false if failed
///@param pEventDispatcher  the event dispatcher
///@param type event sender type such as 'JMS' 'ICE' 'TEXT' 'SNMP'
///@param pText the extra data for plugin use,it can be used as configuration file path
///NOTE:the entry for this must be "InitModuleEntry"
typedef bool (*InitModule)( IMsgSender* pEventDispatcher, const char *type, const char* pText);

///uninitialize the plugin with input parameter
///@return void
///@param pEventDispatcher  the event dispatcher
///@param pText the extra data for plugin use,it can be used as configuration file path
///NOTE:the entry for this must be "UninitModuleEntry"
typedef void (*UninitModule)( IMsgSender* pEventDispatcher );

///@param mstStruct the struct about message
///NOTE:use this function to send message
typedef void (*OnNewMessage)(const MSGSTRUCT& mstStruct, const MessageIdentity& mid, void* ctx);


class IMsgSender
{
public:
	virtual ~IMsgSender(){}
	///register event routine with type
	///@param pMsg the event sink routine address
	///@param type event sender type
	virtual	bool	regist(const OnNewMessage& pMsg ,const char* type ) =0;
	
	///un-register event routine with type
	///@param pMsg the event sink routine address
	///@param type event sender type
	virtual	void	unregist( const OnNewMessage& pMsg , const char* type) =0;

    /// acknowledge the sent message
    virtual void ack(const MessageIdentity& mid, void* ctx) = 0;
};


#endif //MASSEAGE_SENDER_INTERFACE_H

