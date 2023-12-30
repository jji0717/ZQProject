#ifndef __RpcGeek_Event_H__
#define __RpcGeek_Event_H__

#include "RpcGeekServer.h"
#include "RpcGeekClient.h"

namespace ZQ {
namespace RpcGeek {

class RPCGEEK_API EventHelper;
class RPCGEEK_API EventHandler;
class RPCGEEK_API EventTrigger;

#define TAG_EVENT       "#Event."
#define TAG_CALLBACK    "#Callback."

#define RPCGEEK_TRIGGER "RPCGEEK_TRIGGER"

/// -----------------------------
/// class EventHandler
/// -----------------------------
class EventHandler
{
	friend class EventHelper;
	friend class EventHandlerQueue;

public:

	typedef enum
	{
		E_UNRECOG = -1, // unrecognized event of this EventHandler
		E_PROCESSED = 0, // processed but other EventHandler object may also interested
		E_DONE, // processed, will not bother any other EventHandler in the queue
	} RET_e;

	EventHandler(EventHelper& helper);
	virtual ~EventHandler();

	virtual RET_e OnEvent(const char* type, const char* instance, ZQ::common::Variant& params)
	{ return E_UNRECOG; }

	virtual RET_e OnCallback(const char* callback, const char* instance, ZQ::common::Variant& params, ZQ::common::Variant& result)
	{ return E_UNRECOG; }

	virtual uint8 getPriority(void) { return 0; }

protected:

	EventHelper& _helper;
};

/// -----------------------------
/// class EventHelper
/// -----------------------------
class EventHelper : public ZQ::RpcGeek::ServerHelper
{
	friend class EventHandler;
public:

	EventHelper(ZQ::RpcGeek::Server& server);
	virtual ~EventHelper();

	virtual bool execMethod(const char* host, const char* method_name, ZQ::common::Variant& params, ZQ::common::Variant& result);

protected:

	EventHandlerQueue* _pQueue;

};

/// -----------------------------
/// class EventTrigger
/// -----------------------------
class EventTrigger : public Client
{
public:
	EventTrigger()
		:Client(RPCGEEK_TRIGGER, PROTOCOL_VERSION) {}
	
	virtual ~EventTrigger() {}

	virtual bool fireEvent(const char* type, const char* instance, ZQ::common::Variant& params);
	virtual bool callback (const char* type, const char* instance, ZQ::common::Variant& params, ZQ::common::Variant& result);

protected:
	virtual void OnDefaultAsynResponse(const char* methodName, ZQ::common::Variant& paramArray, ZQ::common::Variant& result, const int faultcode)
	{
	}
};


} // namespace RpcGeek
} // namespace ZQ

#endif __RpcGeek_Event_H__

