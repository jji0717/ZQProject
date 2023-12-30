#include "RpcGeekEvent.h"
extern "C"
{
#include <time.h>
}

namespace ZQ {
namespace RpcGeek {

/// -----------------------------
/// class EventHandlerQueue
/// -----------------------------
class EventHandlerQueue
{
	friend class EventHelper;
	friend class EventHandler;

	EventHandlerQueue(EventHelper& helper) : _helper(helper) {}
	~EventHandlerQueue();

	bool insert(EventHandler& handler);
	bool erase(EventHandler& handler);

	EventHandler::RET_e processEvent(const char* type, const char* instance, ZQ::common::Variant& params);
	EventHandler::RET_e processCallback(const char* type, const char* instance, ZQ::common::Variant& params, ZQ::common::Variant& result);

	EventHelper& _helper;

    typedef std::vector <EventHandler*> Queue;
	Queue _EHQueue;
	ZQ::common::Mutex _EHQueue_locker;
};

EventHandlerQueue::~EventHandlerQueue()
{
	ZQ::common::MutexGuard guard(_EHQueue_locker);
	_EHQueue.clear();
}

bool EventHandlerQueue::insert(EventHandler& handler)
{
	// add it into the queue for future operations
	ZQ::common::MutexGuard guard(_EHQueue_locker);
	Queue::iterator it= _EHQueue.begin();
	uint8 prio = handler.getPriority();

	for(; it < _EHQueue.end(); it++)
	{
		if ((*it) == &handler)
			return false;

		if ((*it) && (*it)->getPriority() > prio)
			break;
	}

	_EHQueue.insert(it, &handler);
	return true;
}

bool EventHandlerQueue::erase(EventHandler& handler)
{
	ZQ::common::MutexGuard guard(_EHQueue_locker);
	Queue::iterator it= _EHQueue.begin();
	for(; it < _EHQueue.end(); it++)
	{
		if (&handler == *it)
		{
			_EHQueue.erase(it);
			return true;
		}
	}
	return false;
}

EventHandler::RET_e EventHandlerQueue::processEvent(const char* type, const char* instance, ZQ::common::Variant& params)
{
	EventHandler::RET_e ret = EventHandler::E_UNRECOG;
	for(Queue::iterator it= _EHQueue.begin(); it < _EHQueue.end(); it++)
	{
		if (*it)
		{
			EventHandler::RET_e ret1 = (*it)->OnEvent(type, instance, params);
			ret = (ret < ret1) ? ret1 : ret;
		}
		
		if (EventHandler::E_DONE <= ret)
			break;
	}
	return ret;
}

EventHandler::RET_e EventHandlerQueue::processCallback(const char* type, const char* instance, ZQ::common::Variant& params, ZQ::common::Variant& result)
{
	EventHandler::RET_e ret = EventHandler::E_UNRECOG;
	for(Queue::iterator it= _EHQueue.begin(); it < _EHQueue.end(); it++)
	{
		if (*it)
		{
			ret = (*it)->OnCallback(type, instance, params, result);
			if (EventHandler::E_UNRECOG != ret)
				break;
		}
	}
	return ret;
}

/// -----------------------------
/// class EventHelper
/// -----------------------------
EventHelper::EventHelper(ZQ::RpcGeek::Server& server)
: ServerHelper(server), _pQueue(NULL)
{
	_pQueue = new EventHandlerQueue(*this);
}

EventHelper::~EventHelper()
{
	if (_pQueue)
		delete _pQueue;
	_pQueue=NULL;
}

bool EventHelper::execMethod(const char* host, const char* method_name, ZQ::common::Variant& params, ZQ::common::Variant& result)
{
	if (NULL ==method_name || NULL == _pQueue)
		return false;

	const char* evttype =NULL;
	tstring objid;

	EventHandler::RET_e ret = EventHandler::E_UNRECOG;

	if (0 == strncmp(method_name, TAG_EVENT, strlen(TAG_EVENT)))
	{
		result = ZQ::common::Variant();

		const char* evttype = method_name + strlen(TAG_EVENT);
		try{
			if (params.size() <2)
				return false;
		}
		catch(...) { return false; }

		try{
			if (params[0].has(OBJ_GUID))
				objid = params[0][OBJ_GUID];
		}
		catch(...) {}

		ret = _pQueue->processEvent(evttype, objid.empty()?NULL:objid.c_str(), params[1]);

		// prepare the response
		switch(ret)
		{
		case EventHandler::E_UNRECOG: // event unrecognized by all the EventHandlers
			ServerObjectHelper::setError(result, true, ERROR_UNKNOWN_EVENT, "Event(%s) is not recognized", evttype);
			break;
			
		case EventHandler::E_PROCESSED:
		case EventHandler::E_DONE: // event handled
			ServerObjectHelper::setError(result, false, ERROR_SUCC, "Event(%s) handled", evttype);
			break;
			
		default:
			break;
		}
		
		return true;

	}
	else if (0 == strncmp(method_name, TAG_CALLBACK, strlen(TAG_CALLBACK)))
	{
		
		const char* callbacktype = method_name + strlen(TAG_CALLBACK);
		try{
			if (params.size() <2)
				return false;
		}
		catch(...) { return false; }

		try{
			if (params[0].has(OBJ_GUID))
				objid = params[0][OBJ_GUID];
		}
		catch(...) {}


		ret = _pQueue->processCallback(evttype, objid.empty()?NULL:objid.c_str(), params[1], result);

		// prepare the response
		switch(ret)
		{
		case EventHandler::E_UNRECOG: // event unrecognized by all the EventHandlers
			ServerObjectHelper::setError(result, true, ERROR_UNKNOWN_EVENT, "Callback(%s) is not recognized", callbacktype);
			break;
			
		case EventHandler::E_PROCESSED:
		case EventHandler::E_DONE: // event handled
			ServerObjectHelper::setError(result, false, ERROR_SUCC, "Callback(%s) executed", callbacktype);
			break;
			
		default:
			break;
		}
		
		return true;
	}
	
	return false;
}

/// -----------------------------
/// class EventHandler
/// -----------------------------
EventHandler::EventHandler(EventHelper& helper)
:_helper(helper)
{
	if (_helper._pQueue)
		_helper._pQueue->insert(*this);
}

EventHandler::~EventHandler()
{
	if (_helper._pQueue)
		_helper._pQueue->erase(*this);
}

/// -----------------------------
/// class EventTrigger
/// -----------------------------
bool EventTrigger::fireEvent(const char* type, const char* instance, ZQ::common::Variant& params)
{
	if (NULL == type)
		return false;

	std::string method = TAG_EVENT;
	method +=type;

	ZQ::common::Variant method_params, header;
	time_t stamp;
	stamp = time(&stamp);
	header.set("timeStamp", (int) stamp);

	if (NULL != instance)
		header.set(OBJ_GUID, instance);

	method_params.set(0, header);
	method_params.set(1, params);

	printf("firing event %s: ", type);
	TRACE_VAR(method_params);
	return call_async(method.c_str(), method_params);
}

bool EventTrigger::callback (const char* type, const char* instance, ZQ::common::Variant& params, ZQ::common::Variant& result)
{
	if (NULL == type)
		return false;

	std::string method = TAG_CALLBACK;
	method +=type;

	ZQ::common::Variant method_params, header;
	time_t stamp;
	stamp = time(&stamp);
	header.set("timeStamp", (int) stamp);

	if (NULL != instance)
		header.set(OBJ_GUID, instance);

	method_params.set(0, header);
	method_params.set(1, params);

	return call_sync(method.c_str(), method_params, result);
}

} // namespace RpcGeek
} // namespace ZQ

